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
 *                  add "0x" to begining of string
 *              IF base is OCTAL
 *                  add "0" to begining of string
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
char rcs_hdr[] = "$Id: tostring.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  1997/09/22 14:51:11  hjs
 * dmake 4.1 orginal sourcen
 *
 * Revision 1.1.1.1  1997/07/15 16:02:26  dvadura
 * dmake gold 4.1.00 initial import
 *
 * Revision 1.1.1.1  1996/10/27 07:30:15  dvadura
 * Dmake 4.1 Initial Import
 *
 * Revision 1.1.1.1  1996/10/24 05:33:14  dvadura
 * Initial import for final release of dmake 4.1
 *
 * Revision 1.1  1994/10/06  17:43:20  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:29  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:16  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.4  90/05/11  00:13:11  cpcahil
 * added copyright statment
 *
 * Revision 1.3  90/02/24  21:50:33  cpcahil
 * lots of lint fixes
 *
 * Revision 1.2  90/02/24  17:29:42  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs
 * id string
 *
 * Revision 1.1  90/02/22  23:17:44  cpcahil
 * Initial revision
 *
 */
