/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parseinc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:39:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADC_PARSEINC_HXX
#define ADC_PARSEINC_HXX


#include <tools/tkpchars.hxx>

inline char
jumpOver( CharacterSource & io_rText,
          char in_c )
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext == in_c;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpTo( CharacterSource & io_rText,
        char              in_c )
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != in_c AND cNext != 0;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpTo( CharacterSource & io_rText,
        char              in_c1,
        char              in_c2 )
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != in_c1 AND cNext != in_c2 AND cNext != 0;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpTo( CharacterSource & io_rText,
        char              in_c1,
        char              in_c2,
        char              in_c3 )
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != in_c1 AND cNext != in_c2 AND cNext != in_c3 AND cNext != 0;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpTo( CharacterSource & io_rText,
        char              in_c1,
        char              in_c2,
        char              in_c3,
        char              in_c4 )
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != in_c1 AND cNext != in_c2 AND cNext != in_c3
                         AND cNext != in_c4 AND cNext != 0;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpOverWhite(CharacterSource & io_rText)
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          static_cast<UINT8>(cNext) < 33
              AND cNext != 0 AND cNext != 13 AND cNext != 10;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpToWhite(CharacterSource & io_rText)
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          static_cast<UINT8>(cNext) > 32;
          cNext = io_rText.MoveOn() )
    { }

    return cNext;
}

inline char
jumpToEol(CharacterSource & io_rText, int & o_rCount_BackslashedLineBreaks )
{
    o_rCount_BackslashedLineBreaks = 0;
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != 13 AND cNext != 10 AND cNext != NULCH;
          cNext = io_rText.MoveOn() )
    {
        if ( cNext == '\\')
        {
            cNext = io_rText.MoveOn();
            if ( cNext == 13 )
                io_rText.MoveOn();
            if ( cNext == 10 )
                ++o_rCount_BackslashedLineBreaks;
        }
    }
    return cNext;
}

inline char
jumpToEol(CharacterSource & io_rText)
{
    char cNext;
    for ( cNext = io_rText.CurChar();
          cNext != 13 AND cNext != 10 AND cNext != NULCH;
          cNext = io_rText.MoveOn() )
    {
        if ( cNext == '\\')
            io_rText.MoveOn();
    }
    return cNext;
}

inline char
jumpOverEol(CharacterSource & io_rText)
{
    char cNext = io_rText.CurChar();

    if (cNext == 13)
        io_rText.MoveOn();
    if (cNext == 10)
        io_rText.MoveOn();
    return cNext;
}


inline char // Finds a matching closing bracket after the opening one is passed
jumpToMatchingBracket( CharacterSource & io_rText,
                       char              in_cBegin,
                       char              in_cEnd )
{
    intt nCounter = 1;
    char cNext;
    for ( cNext = io_rText.CurChar();
          nCounter - (cNext == in_cEnd ? 1 : 0) > 0 AND cNext != NULCH;
          cNext = io_rText.MoveOn() )
    {
        if (cNext == in_cEnd)
           nCounter++;
        else if (cNext == in_cBegin)
           nCounter--;
    }

    return cNext;
}




#endif

