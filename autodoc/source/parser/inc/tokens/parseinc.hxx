/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
