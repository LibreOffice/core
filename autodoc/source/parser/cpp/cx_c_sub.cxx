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

#include <precomp.h>
#include "cx_c_sub.hxx"



// NOT FULLY DECLARED SERVICES
#include <ctype.h>
#include "c_dealer.hxx"
#include <tokens/parseinc.hxx>
#include <x_parse.hxx>
#include "all_toks.hxx"


namespace cpp {



void
Context_Comment::ReadCharChain( CharacterSource &	io_rText )
{
    // KORR_FUTURE
    //		Counting of lines must be implemented.
    if (bCurrentModeIsMultiline)
    {
        char cNext = NULCH;

        do {
            do {
                cNext = jumpTo( io_rText,'*',char(10) );
                if (cNext == NULCH)
                    throw X_Parser( X_Parser::x_UnexpectedEOF, "", String::Null_(), 0 );
                else if ( cNext == char(10) )
                {
                    jumpOverEol(io_rText);
                    Dealer().Deal_Eol();
                }
            }   while ( cNext != '*');
            cNext = jumpOver(io_rText,'*');
            if (cNext == NULCH)
                throw X_Parser( X_Parser::x_UnexpectedEOF, "", String::Null_(), 0 );
        } while (cNext != '/');
        io_rText.MoveOn();
        io_rText.CutToken();
        SetNewToken(0);
    }
    else //
    {
        int o_rCount_BackslashedLineBreaks = 0;
        jumpToEol(io_rText,o_rCount_BackslashedLineBreaks);
        for ( ; o_rCount_BackslashedLineBreaks > 0; --o_rCount_BackslashedLineBreaks )
            Dealer().Deal_Eol();

        if (io_rText.CurChar() != NULCH)
            jumpOverEol(io_rText);
        io_rText.CutToken();
        Dealer().Deal_Eol();
        SetNewToken(0);
    }  // endif
}


void
Context_ConstString::ReadCharChain( CharacterSource &	io_rText )
{
    char cNext = io_rText.MoveOn();

    while (cNext != '"')
    { 	// Get one complete string constant:  "...."
        while (cNext != '"' AND cNext != '\\')
        {	// Get string till next '\\'
            cNext = io_rText.MoveOn();
        }
        if (cNext == '\\')
        {
            io_rText.MoveOn();
            cNext = io_rText.MoveOn();
        }
    }
    io_rText.MoveOn();
    SetNewToken(new Tok_Constant(io_rText.CutToken()));
}

void
Context_ConstChar::ReadCharChain( CharacterSource &	io_rText )
{
    char cNext = io_rText.MoveOn();

    while (cNext != '\'')
    { 	// Get one complete char constant:  "...."
        while (cNext != '\'' AND cNext != '\\')
        {	// Get string till next '\\'
            cNext = io_rText.MoveOn();
        }
        if (cNext == '\\')
        {
            io_rText.MoveOn();
            cNext = io_rText.MoveOn();
        }
    }
    io_rText.MoveOn();
    SetNewToken(new Tok_Constant(io_rText.CutToken()));
}

void
Context_ConstNumeric::ReadCharChain(CharacterSource & io_rText)
{
    char cNext = 0;

    do {
        do {
            cNext = static_cast<char>( tolower(io_rText.MoveOn()) );
        } while ( (cNext != 'e' AND isalnum(cNext)) OR cNext == '.');
        if (cNext == 'e')
        {
            cNext = io_rText.MoveOn();
            if (cNext == '+' OR cNext == '-')
                cNext = io_rText.MoveOn();
        }	// endif
    } while (isalnum(cNext) OR cNext == '.');     // Reicht aus, wenn Zahlen korrekt geschrieben sind
    SetNewToken(new Tok_Constant(io_rText.CutToken()));
}

void
Context_UnblockMacro::ReadCharChain(CharacterSource & io_rText)
{
    jumpToWhite(io_rText);
    SetNewToken(new Tok_UnblockMacro( io_rText.CutToken() + strlen("#unblock-") ));
}

}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
