/*************************************************************************
 *
 *  $RCSfile: cx_c_sub.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include "cx_c_sub.hxx"



// NOT FULLY DECLARED SERVICES
#include "c_dealer.hxx"
#include <tokens/parseinc.hxx>
#include <x_parse.hxx>
#include "all_toks.hxx"


namespace cpp {



void
Context_Comment::ReadCharChain( CharacterSource &   io_rText )
{
    // KORR
    //      Counting of lines must be implemented.
    if (bCurrentModeIsMultiline)
    {
        char cNext = NULCH;

        do {
            do {
                cNext = jumpTo( io_rText,'*',char(10) );
                if (cNext == NULCH)
                    throw X_Parser( X_Parser::x_UnexpectedEOF, "", udmstri::Null_(), 0 );
                else if ( cNext == char(10) )
                {
                    jumpOverEol(io_rText);
                    Dealer().Deal_Eol();
                }
            }   while ( cNext != '*');
            cNext = jumpOver(io_rText,'*');
            if (cNext == NULCH)
                throw X_Parser( X_Parser::x_UnexpectedEOF, "", udmstri::Null_(), 0 );
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
Context_ConstString::ReadCharChain( CharacterSource &   io_rText )
{
    char cNext = io_rText.MoveOn();

    while (cNext != '"')
    {   // Get one complete string constant:  "...."
        while (cNext != '"' AND cNext != '\\')
        {   // Get string till next '\\'
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
Context_ConstChar::ReadCharChain( CharacterSource & io_rText )
{
    char cNext = io_rText.MoveOn();

    while (cNext != '\'')
    {   // Get one complete char constant:  "...."
        while (cNext != '\'' AND cNext != '\\')
        {   // Get string till next '\\'
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
            cNext = tolower(io_rText.MoveOn());
        } while (cNext != 'e' AND isalnum(cNext) OR cNext == '.');
        if (cNext == 'e')
        {
            cNext = io_rText.MoveOn();
            if (cNext == '+' OR cNext == '-')
                cNext = io_rText.MoveOn();
        }   // endif
    } while (isalnum(cNext) OR cNext == '.');     // Reicht aus, wenn Zahlen korrekt geschrieben sind
    SetNewToken(new Tok_Constant(io_rText.CutToken()));
}

void
Context_UnblockMacro::ReadCharChain(CharacterSource & io_rText)
{
    char cNext = jumpToWhite(io_rText);
    SetNewToken(new Tok_UnblockMacro( io_rText.CutToken() + strlen("#unblock-") ));
}

}   // namespace cpp








