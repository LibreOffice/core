/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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


inline char	// Finds a matching closing bracket after the opening one is passed
jumpToMatchingBracket( CharacterSource & io_rText,
                       char 			 in_cBegin,
                       char 			 in_cEnd )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
