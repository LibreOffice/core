/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
Context_Comment::ReadCharChain( CharacterSource &   io_rText )
{
    // KORR_FUTURE
    //      Counting of lines must be implemented.
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
            cNext = static_cast<char>( tolower(io_rText.MoveOn()) );
        } while ( (cNext != 'e' AND isalnum(cNext)) OR cNext == '.');
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
    jumpToWhite(io_rText);
    SetNewToken(new Tok_UnblockMacro( io_rText.CutToken() + strlen("#unblock-") ));
}

}   // namespace cpp
