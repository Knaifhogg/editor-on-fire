#include <allegro.h>
#include "main.h"
#include "beat.h"
#include "event.h"

#ifdef USEMEMWATCH
#include "memwatch.h"
#endif

char eof_event_list_text[EOF_MAX_TEXT_EVENTS][256] = {{0}};

EOF_TEXT_EVENT * eof_song_add_text_event(EOF_SONG * sp, unsigned long beat, char * text, unsigned long track, char is_temporary)
{
// 	eof_log("eof_song_add_text_event() entered");

	if(sp && text && (sp->text_events < EOF_MAX_TEXT_EVENTS) && (beat < sp->beats))
	{	//If the maximum number of text events hasn't already been defined, and the specified beat number is valid
		sp->text_event[sp->text_events] = malloc(sizeof(EOF_TEXT_EVENT));
		if(sp->text_event[sp->text_events])
		{
			ustrcpy(sp->text_event[sp->text_events]->text, text);
			sp->text_event[sp->text_events]->beat = beat;
			if(track >= sp->tracks)
			{	//If this is an invalid track
				track = 0;	//Make this a global text event
			}
			sp->text_event[sp->text_events]->track = track;
			sp->text_event[sp->text_events]->is_temporary = is_temporary;
			sp->beat[beat]->flags |= EOF_BEAT_FLAG_EVENTS;	//Set the events flag for the beat
			sp->text_events++;
			return sp->text_event[sp->text_events-1];	//Return successfully created text event
		}
	}
	return NULL;	//Return failure
}

void eof_move_text_events(EOF_SONG * sp, unsigned long beat, unsigned long offset, int dir)
{
	eof_log("eof_move_text_events() entered", 1);

	unsigned long i;

	if(!sp)
	{
		return;
	}
	for(i = 0; i < sp->text_events; i++)
	{
		if(sp->text_event[i]->beat >= beat)
		{
			if(sp->text_event[i]->beat >= sp->beats)
				continue;	//Do not allow an out of bound access
			sp->beat[sp->text_event[i]->beat]->flags &= ~(EOF_BEAT_FLAG_EVENTS);	//Clear the event flag
			if(dir < 0)
			{
				if(offset > sp->text_event[i]->beat)
					continue;	//Do not allow an underflow
				sp->text_event[i]->beat -= offset;
			}
			else
			{
				if(sp->text_event[i]->beat + offset >= sp->beats)
					continue;	//Do not allow an overflow
				sp->text_event[i]->beat += offset;
			}
			sp->beat[sp->text_event[i]->beat]->flags |= EOF_BEAT_FLAG_EVENTS;	//Set the event flag
		}
	}
}

void eof_song_delete_text_event(EOF_SONG * sp, unsigned long event)
{
 	eof_log("eof_song_delete_text_event() entered", 1);

	unsigned long i;
	if(sp)
	{
		free(sp->text_event[event]);
		for(i = event; i < sp->text_events - 1; i++)
		{
			sp->text_event[i] = sp->text_event[i + 1];
		}
		sp->text_events--;
		eof_cleanup_beat_flags(sp);	//Rebuild event flags for all beats to ensure they're valid
	}
}

int eof_song_resize_text_events(EOF_SONG * sp, unsigned long events)
{
	unsigned long i;
	unsigned long oldevents;

	if(!sp)
	{
		return 0;
	}
	oldevents = sp->text_events;
	if(events > oldevents)
	{
		for(i = oldevents; i < events; i++)
		{
			if(!eof_song_add_text_event(sp, 0, "", 0, 0))
			{
				return 0;	//Return failure
			}
		}
	}
	else if(events < oldevents)
	{
		for(i = events; i < oldevents; i++)
		{
			free(sp->text_event[i]);
			sp->text_events--;
		}
	}
	return 1;	//Return succes
}

void eof_sort_events(EOF_SONG * sp)
{
 	eof_log("eof_sort_events() entered", 1);

	if(sp)
	{
		qsort(sp->text_event, sp->text_events, sizeof(EOF_TEXT_EVENT *), eof_song_qsort_events);
	}
}

int eof_song_qsort_events(const void * e1, const void * e2)
{
	EOF_TEXT_EVENT ** thing1 = (EOF_TEXT_EVENT **)e1;
	EOF_TEXT_EVENT ** thing2 = (EOF_TEXT_EVENT **)e2;

	if((*thing1)->beat < (*thing2)->beat)
	{
		return -1;
	}
	else if((*thing1)->beat == (*thing2)->beat)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

char eof_song_contains_event(EOF_SONG *sp, const char *text, unsigned long track)
{
	unsigned long i;

	if(sp && text)
	{
		for(i = 0; i < sp->text_events; i++)
		{
			if(track && (sp->text_event[i]->track != track))
			{	//If searching for events in a specific track, and this event isn't in that track
				continue;	//Skip this event
			}
			if(!ustrcmp(sp->text_event[i]->text, text))
			{
				return 1;	//Return match found
			}
		}
	}
	return 0;	//Return no match found
}

char eof_song_contains_event_beginning_with(EOF_SONG *sp, const char *text, unsigned long track)
{
	unsigned long i;

	if(sp && text)
	{
		for(i = 0; i < sp->text_events; i++)
		{
			if(track && (sp->text_event[i]->track != track))
			{	//If searching for events in a specific track, and this event isn't in that track
				continue;	//Skip this event
			}
			if(ustrstr(sp->text_event[i]->text, text) == sp->text_event[i]->text)
			{	//If a substring search returns the beginning of an existing text event as a match
				return 1;	//Return match found
			}
		}
	}
	return 0;	//Return no match found
}

int eof_is_section_marker(const char *text)
{
	if(text)
	{
		if((ustrstr(text, "[section") == text) ||
			(ustrstr(text, "section") == text) ||
			(ustrstr(text, "[prc_") == text))
			return 1;
	}
	return 0;
}
