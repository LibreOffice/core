/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_a_sub.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:12:20 $
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

#include <precomp.h>
#include <adoc/cx_a_sub.hxx>


// NOT FULLY DEFINED SERVICES
#include <tokens/parseinc.hxx>
#include <x_parse.hxx>
#include <adoc/tk_docw.hxx>


namespace adoc {

//************************    Cx_LineStart      ************************//

Cx_LineStart::Cx_LineStart( TkpContext &    i_rFollowUpContext )
    :   pDealer(0),
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


//************************    Cx_CheckStar      ************************//

Cx_CheckStar::Cx_CheckStar( TkpContext &        i_rFollowUpContext )
    :   pDealer(0),
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
            throw X_Parser(X_Parser::x_UnexpectedEOF, "", udmstri::Null_(), 0);
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


//************************    Cx_AtTagCompletion        ************************//

Cx_AtTagCompletion::Cx_AtTagCompletion( TkpContext &    i_rFollowUpContext )
    :   pDealer(0),
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



/*
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
Cx_EoHtml::ReadCharChain( CharacterSource & io_rText )
{
    if ( NULCH == jumpTo(io_rText,'>') )
        throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    io_rText.MoveOn();
    SetToken(new Tok_HtmlTag(io_rText.CutToken(),bToken_IsStartOfParagraph));
}

void
Cx_EoXmlConst::ReadCharChain( CharacterSource & io_rText )
{
    if ( NULCH == jumpTo(io_rText,'>') )
        throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlConst(eTokenId));
}

void
Cx_EoXmlLink_BeginTag::ReadCharChain( CharacterSource & io_rText )
{
    udmstri sScope;

    switch ( jumpTo(io_rText,'"','>') )
    {
        case '"':
            io_rText.MoveOn();
            io_rText.CutToken();
            if ( NULCH == jumpTo(io_rText,'"') )
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
            sScope = io_rText.CutToken();
            if ( NULCH == jumpTo(io_rText,'>') )
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
            break;
        case '>':
            break;
        default:
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    }
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlLink_BeginTag(eTokenId,sScope));
}

void
Cx_EoXmlLink_EndTag::ReadCharChain( CharacterSource & io_rText )
{
    if ( NULCH == jumpTo(io_rText,'>') )
        throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlLink_EndTag(eTokenId));
}

void
Cx_EoXmlFormat_BeginTag::ReadCharChain( CharacterSource & io_rText )
{
    if ( NULCH == jumpTo(io_rText,'>') )
        throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlFormat_BeginTag(eTokenId));
}

void
Cx_EoXmlFormat_EndTag::ReadCharChain( CharacterSource & io_rText )
{
    if ( NULCH == jumpTo(io_rText,'>') )
        throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlFormat_EndTag(eTokenId));
}

void
Cx_CheckStar::ReadCharChain( CharacterSource & io_rText )
{
    bEndTokenFound = false;
    if (bIsEnd)
    {
        char cNext = jumpOver(io_rText,'*');
        if ( NULCH == cNext )
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        if (cNext == '/')
        {
            io_rText.MoveOn();
            SetToken(new Tok_DocuEnd);
            bEndTokenFound = true;
        }
        else
        {
            SetToken( new Tok_Word(io_rText.CutToken()) );
        }
    }
    else
    {
        jumpToWhite(io_rText);
        SetToken( new Tok_Word(io_rText.CutToken()) );
    }
}

TkpContext &
Cx_CheckStar::FollowUpContext()
{
    if (bEndTokenFound)
        return *pEnd_FollowUpContext;
    else
        return Cx_Base::FollowUpContext();
}
*/

}   // namespace adoc








