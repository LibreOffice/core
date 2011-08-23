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
#include <s2_luidl/cx_sub.hxx>



// NOT FULLY DECLARED SERVICES
#include <s2_luidl/tokrecv.hxx>
#include <../../parser/inc/tokens/parseinc.hxx>
#include <x_parse2.hxx>
#include <s2_luidl/tk_const.hxx>



namespace csi
{
namespace uidl
{

bool
Cx_Base::PassNewToken()
{
    if (pNewToken)
    {
        rReceiver.Receive(*pNewToken.Release());
        return true;
    }
    return false;
}

TkpContext &
Cx_Base::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Context_MLComment::ReadCharChain( CharacterSource &	io_rText )
{
    char cNext = NULCH;

    do {
        do {
            cNext = jumpTo(io_rText,'*','\n');
            if (cNext == '\n')
            {
                Receiver().Increment_CurLine();
                cNext = io_rText.MoveOn();
            }
            else if (cNext == NULCH)
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        } while (cNext != '*');

        cNext = jumpOver(io_rText,'*');
        if (cNext == NULCH)
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    } while (cNext != '/');
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(0);
}

void
Context_SLComment::ReadCharChain( CharacterSource &	io_rText )
{
    jumpToEol(io_rText);
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    SetToken(0);

    Receiver().Increment_CurLine();
}

void
Context_Praeprocessor::ReadCharChain( CharacterSource &	io_rText )
{
    jumpToEol(io_rText);
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    SetToken(0);

    Receiver().Increment_CurLine();
}

void
Context_Assignment::ReadCharChain( CharacterSource &	io_rText )
{
    // KORR_FUTURE
    // How to handle new lines within this, so he y get realised by
    //  ParserInfo?

    char cNext = NULCH;
    do {
        if ( (cNext = jumpTo(io_rText,';',',','"','}')) == NULCH )
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        if (cNext == '"')
        {
            cNext = io_rText.MoveOn();
            while (cNext != '"')
            {
                if ( (cNext = jumpTo(io_rText,'"','\\')) == NULCH )
                    throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
                if (cNext == '\\')
                    io_rText.MoveOn();
            }
            cNext = io_rText.MoveOn();
        }	// endif (cNext == '"')
    } while (cNext != ';' AND cNext != ',' AND cNext != '}');

    if (cNext == ',' OR cNext == ';')
        io_rText.MoveOn();
    SetToken(new TokAssignment(io_rText.CutToken()));
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
