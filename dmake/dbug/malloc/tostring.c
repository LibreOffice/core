/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include "tostring.h"

/*
 * Function:    tostring()
 *
 * Purpose: to convert an integer to an ascii display string
 *
 * Arguments:   buf - place to put the
 *      val - integer to convert
 *      len - length of output field (0 if just enough to hold data)
 *      base    - base for number conversion (only works for base <= 16)
 *      fill    - fill char when len > # digits
 *
 * Returns: length of string
 *
 * Narrative:   IF fill character is non-blank
 *          Determine base
 *              If base is HEX
 *                  add "0x" to beginning of string
 *              IF base is OCTAL
 *                  add "0" to beginning of string
 *
 *      While value is greater than zero
 *          use val % base as index into xlation str to get cur char
 *          divide val by base
 *
 *      Determine fill-in length
 *
 *      Fill in fill chars
 *
 *      Copy in number
 *
 *
 * Mod History:
 *   90/01/24   cpcahil     Initial revision.
 */

#ifndef lint
static
char rcs_hdr[] = "$Id: tostring.c,v 1.2 2006-07-25 10:10:17 rt Exp $";
#endif

#define T_LEN 10

int
tostring(buf,val,len,base,fill)
    int   base;
    char    * buf;
    char      fill;
    int   len;
    int   val;

{
    char    * bufstart = buf;
    int   i = T_LEN;
    char    * xbuf = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char      tbuf[T_LEN];

    /*
     * if we are filling with non-blanks, make sure the
     * proper start string is added
     */
    if( fill != ' ' )
    {
        switch(base)
        {
            case B_HEX:
                *(buf++) = '0';
                *(buf++) = 'x';
                if( len )
                {
                    len -= 2;
                }
                break;
            case B_OCTAL:
                *(buf++) = fill;
                if( len )
                {
                    len--;
                }
                break;
            default:
                break;
        }
    }

    while( val > 0 )
    {
        tbuf[--i] = xbuf[val % base];
        val = val / base;
    }

    if( len )
    {
        len -= (T_LEN - i);

        if( len > 0 )
        {
            while(len-- > 0)
            {
                *(buf++) = fill;
            }
        }
        else
        {
            /*
             * string is too long so we must truncate
             * off some characters.  We do this the easiest
             * way by just incrementing i.  This means the
             * most significant digits are lost.
             */
            while( len++ < 0 )
            {
                i++;
            }
        }
    }

    while( i < T_LEN )
    {
        *(buf++) = tbuf[i++];
    }

    return( (int) (buf - bufstart) );

} /* tostring(... */

