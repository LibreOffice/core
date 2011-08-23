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
#include <s2_dsapi/cx_docu2.hxx>


// NOT FULLY DEFINED SERVICES
#include <../../parser/inc/tokens/parseinc.hxx>
#include <s2_dsapi/tokrecv.hxx>
#include <s2_dsapi/tk_html.hxx>
#include <s2_dsapi/tk_xml.hxx>
#include <s2_dsapi/tk_docw2.hxx>
#include <x_parse2.hxx>



namespace csi
{
namespace dsapi
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
Cx_Base::Handle_DocuSyntaxError( CharacterSource & io_rText )
{
    // KORR_FUTURE
    // Put this into Error Log File

    Cerr() << "Error:  Syntax error in documentation within "
              << "this text:\n\""
              << io_rText.CutToken()
              << "\"."
              << Endl();
    SetToken( new Tok_Word(io_rText.CurToken()) );
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
    char c = jumpTo(io_rText,'>','*');
    if ( NULCH == c OR '*' == c )
    {
        Handle_DocuSyntaxError(io_rText);
        return;
    }

    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlConst(eTokenId));
}

void
Cx_EoXmlLink_BeginTag::ReadCharChain( CharacterSource & io_rText )
{
    String 	sScope;
    String 	sDim;

    do {
        char cReached = jumpTo(io_rText,'"','>','*');
        switch (cReached)
        {
            case '"':
            {
                io_rText.MoveOn();
                io_rText.CutToken();
                char c = jumpTo(io_rText,'"','*', '>');
                if ( NULCH == c OR '*' == c OR '>' == c)
                {
                    if ( '>' == c )
                        io_rText.MoveOn();
                    Handle_DocuSyntaxError(io_rText);
                    return;
                }

                const char * pAttribute = io_rText.CutToken();
                if ( *pAttribute != '[' )
                    sScope = pAttribute;
                else
                    sDim = pAttribute;

                io_rText.MoveOn();
                break;
            }
            case '>':
                break;
            case '*':
                Handle_DocuSyntaxError(io_rText);
                return;
            default:
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        }   // end switch
    }   while ( io_rText.CurChar() != '>' );

    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken( new Tok_XmlLink_BeginTag(eTokenId, sScope.c_str(), sDim.c_str()) );
}

void
Cx_EoXmlLink_EndTag::ReadCharChain( CharacterSource & io_rText )
{
    char c = jumpTo(io_rText,'>','*');
    if ( NULCH == c OR '*' == c )
    {
        Handle_DocuSyntaxError(io_rText);
        return;
    }

    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlLink_EndTag(eTokenId));
}

void
Cx_EoXmlFormat_BeginTag::ReadCharChain( CharacterSource & io_rText )
{
    String 	sDim;

    char cReached = jumpTo(io_rText,'"','>','*');
    switch (cReached)
    {
        case '"':
        {
            io_rText.MoveOn();
            io_rText.CutToken();

            char c = jumpTo(io_rText,'"','*','>');
            if ( NULCH == c OR '*' == c OR '>' == c )
            {
                if ('>' == c )
                    io_rText.MoveOn();
                Handle_DocuSyntaxError(io_rText);
                return;
            }

            sDim = io_rText.CutToken();

            c = jumpTo(io_rText,'>','*');
            if ( NULCH == c OR '*' == c )
            {
                Handle_DocuSyntaxError(io_rText);
                return;
            }
            break;
        }
        case '>':
               break;
        case '*':
            Handle_DocuSyntaxError(io_rText);
            return;
        default:
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    }   // end switch

    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(new Tok_XmlFormat_BeginTag(eTokenId, sDim));
}

void
Cx_EoXmlFormat_EndTag::ReadCharChain( CharacterSource & io_rText )
{
    char c = jumpTo(io_rText,'>','*');
    if ( NULCH == c OR '*' == c )
    {
        Handle_DocuSyntaxError(io_rText);
        return;
    }

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

}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
