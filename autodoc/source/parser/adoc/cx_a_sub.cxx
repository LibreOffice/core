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
#include <adoc/cx_a_sub.hxx>


// NOT FULLY DEFINED SERVICES
#include <tokens/parseinc.hxx>
#include <x_parse.hxx>
#include <adoc/tk_docw.hxx>


namespace adoc {

//************************	  Cx_LineStart		************************//

Cx_LineStart::Cx_LineStart( TkpContext &	i_rFollowUpContext )
    : 	pDealer(0),
        pFollowUpContext(&i_rFollowUpContext)
{
}

void
Cx_LineStart::ReadCharChain( CharacterSource & io_rText )
{
    uintt nCount = 0;
    for ( char cNext = io_rText.CurChar(); cNext == 32 OR cNext == 9; cNext = io_rText.MoveOn() )
    {
        if (cNext == 32)
            nCount++;
        else if (cNext == 9)
            nCount += 4;
    }
    io_rText.CutToken();

    if (nCount < 50)
        pNewToken = new Tok_LineStart(UINT8(nCount));
    else
        pNewToken = new Tok_LineStart(0);
}

bool
Cx_LineStart::PassNewToken()
{
    if (pNewToken)
    {
        pNewToken.Release()->DealOut(*pDealer);
        return true;
    }
    return false;
}

TkpContext &
Cx_LineStart::FollowUpContext()
{
    return *pFollowUpContext;
}


//************************	  Cx_CheckStar		************************//

Cx_CheckStar::Cx_CheckStar(	TkpContext &		i_rFollowUpContext )
    : 	pDealer(0),
        pFollowUpContext(&i_rFollowUpContext),
        pEnd_FollowUpContext(0),
        bCanBeEnd(false),
        bEndTokenFound(false)
{
}


void
Cx_CheckStar::ReadCharChain( CharacterSource & io_rText )
{
    bEndTokenFound = false;
    if (bCanBeEnd)
    {
        char cNext = jumpOver(io_rText,'*');
        if ( NULCH == cNext )
            throw X_Parser(X_Parser::x_UnexpectedEOF, "", String::Null_(), 0);
        if (cNext == '/')
        {
            io_rText.MoveOn();
            pNewToken = new Tok_EoDocu;
            bEndTokenFound = true;
        }
        else
        {
            pNewToken = new Tok_DocWord(io_rText.CutToken());
        }
    }
    else
    {
        jumpToWhite(io_rText);
        pNewToken = new Tok_DocWord(io_rText.CutToken());
    }
}

bool
Cx_CheckStar::PassNewToken()
{
    if (pNewToken)
    {
        pNewToken.Release()->DealOut(*pDealer);
        return true;
    }
    return false;
}

TkpContext &
Cx_CheckStar::FollowUpContext()
{
    if (bEndTokenFound)
        return *pEnd_FollowUpContext;
    else
        return *pFollowUpContext;
}


//************************	  Cx_AtTagCompletion		************************//

Cx_AtTagCompletion::Cx_AtTagCompletion( TkpContext &	i_rFollowUpContext )
    : 	pDealer(0),
        pFollowUpContext(&i_rFollowUpContext)
{
}

void
Cx_AtTagCompletion::ReadCharChain( CharacterSource & io_rText )
{
    jumpToWhite(io_rText);
    csv_assert(fCur_TokenCreateFunction != 0);
    pNewToken = (*fCur_TokenCreateFunction)(io_rText.CutToken());
}

bool
Cx_AtTagCompletion::PassNewToken()
{
    if (pNewToken)
    {
        pNewToken.Release()->DealOut(*pDealer);
        return true;
    }
    return false;
}

TkpContext &
Cx_AtTagCompletion::FollowUpContext()
{
    return *pFollowUpContext;
}




}   // namespace adoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
