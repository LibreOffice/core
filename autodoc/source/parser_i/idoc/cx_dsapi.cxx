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

#include <precomp.h>
#include <s2_dsapi/cx_dsapi.hxx>


// NOT FULLY DEFINED SERVICES
#include <x_parse2.hxx>
#include <tools/tkpchars.hxx>
#include <s2_dsapi/tk_atag2.hxx>
#include <s2_dsapi/tk_docw2.hxx>
#include <s2_dsapi/tk_xml.hxx>
#include <s2_dsapi/cx_docu2.hxx>
#include <s2_dsapi/tokrecv.hxx>


namespace csi
{
namespace dsapi
{


const intt C_nStatusSize = 128;
const intt C_nCppInitialNrOfStati = 400;


const uintt nF_fin_Error = 1;
const uintt nF_fin_Ignore = 2;
const uintt nF_fin_Eof = 3;
const uintt nF_fin_AnyWord = 4;
const uintt nF_fin_AtTag = 5;
const uintt nF_fin_EndSign = 6;
const uintt nF_goto_EoHtml = 7;
const uintt nF_goto_EoXmlConst = 8;
const uintt nF_goto_EoXmlLink_BeginTag = 9;
const uintt nF_goto_EoXmlLink_EndTag = 10;
const uintt nF_goto_EoXmlFormat_BeginTag = 11;
const uintt nF_goto_EoXmlFormat_EndTag = 12;
const uintt nF_goto_CheckStar = 13;
const uintt nF_fin_Comma = 14;
const uintt nF_fin_White = 15;

const UINT16 nTok_at_author = 100 + Tok_AtTag::author;
const UINT16 nTok_at_see = 100 + Tok_AtTag::see;
const UINT16 nTok_at_param = 100 + Tok_AtTag::param;
const UINT16 nTok_at_return = 100 + Tok_AtTag::e_return;
const UINT16 nTok_at_throws = 100 + Tok_AtTag::e_throw;
const UINT16 nTok_at_example = 100 + Tok_AtTag::example;
const UINT16 nTok_at_deprecated = 100 + Tok_AtTag::deprecated;
const UINT16 nTok_at_suspicious = 100 + Tok_AtTag::suspicious;
const UINT16 nTok_at_missing = 100 + Tok_AtTag::missing;
const UINT16 nTok_at_incomplete = 100 + Tok_AtTag::incomplete;
const UINT16 nTok_at_version = 100 + Tok_AtTag::version;
const UINT16 nTok_at_guarantees = 100 + Tok_AtTag::guarantees;
const UINT16 nTok_at_exception = 100 + Tok_AtTag::exception;
const UINT16 nTok_at_since = 100 + Tok_AtTag::since;

const UINT16 nTok_const_TRUE = 200 + Tok_XmlConst::e_true;
const UINT16 nTok_const_FALSE = 200 + Tok_XmlConst::e_false;
const UINT16 nTok_const_NULL = 200 + Tok_XmlConst::e_null;
const UINT16 nTok_const_void = 200 + Tok_XmlConst::e_void;

const UINT16 nTok_link_typeB = 300 + Tok_XmlLink_BeginTag::type;
const UINT16 nTok_link_typeE = 325 + Tok_XmlLink_EndTag::type;
const UINT16 nTok_link_memberB = 300 + Tok_XmlLink_BeginTag::member;
const UINT16 nTok_link_membeE = 325 + Tok_XmlLink_EndTag::member;
const UINT16 nTok_link_constB = 300 + Tok_XmlLink_BeginTag::e_const;
const UINT16 nTok_link_constE = 325 + Tok_XmlLink_EndTag::e_const;

const UINT16 nTok_format_listingB = 350 + Tok_XmlFormat_BeginTag::listing;
const UINT16 nTok_format_listingE = 375 + Tok_XmlFormat_EndTag::listing;
const UINT16 nTok_format_codeB = 350 + Tok_XmlFormat_BeginTag::code;
const UINT16 nTok_format_codeE = 375 + Tok_XmlFormat_EndTag::code;
const UINT16 nTok_format_atomB = 350 + Tok_XmlFormat_BeginTag::atom;
const UINT16 nTok_format_atomE = 375 + Tok_XmlFormat_EndTag::atom;


const UINT16 nTok_html_parastart = 400;

const UINT16 nTok_MLDocuEnd = 501;
const UINT16 nTok_EOL = 502;


Context_Docu::Context_Docu( Token_Receiver & o_rReceiver )
    :   aStateMachine(C_nStatusSize, C_nCppInitialNrOfStati),
        pReceiver(&o_rReceiver),
        pParentContext(0),
        pCx_EoHtml(0),
        pCx_EoXmlConst(0),
        pCx_EoXmlLink_BeginTag(0),
        pCx_EoXmlLink_EndTag(0),
        pCx_EoXmlFormat_BeginTag(0),
        pCx_EoXmlFormat_EndTag(0),
        pCx_CheckStar(0),
        pNewToken(0),
        pFollowUpContext(0),
        bIsMultiline(false)
{
    pCx_EoHtml = new Cx_EoHtml(o_rReceiver, *this);
    pCx_EoXmlConst = new Cx_EoXmlConst(o_rReceiver, *this);
    pCx_EoXmlLink_BeginTag = new Cx_EoXmlLink_BeginTag(o_rReceiver, *this);
    pCx_EoXmlLink_EndTag = new Cx_EoXmlLink_EndTag(o_rReceiver, *this);
    pCx_EoXmlFormat_BeginTag = new Cx_EoXmlFormat_BeginTag(o_rReceiver, *this);
    pCx_EoXmlFormat_EndTag = new Cx_EoXmlFormat_EndTag(o_rReceiver, *this);
    pCx_CheckStar = new Cx_CheckStar(*pReceiver,*this);

    SetupStateMachine();
}

void
Context_Docu::SetParentContext( TkpContext & io_rParentContext,
                                const char * )
{
    pFollowUpContext = pParentContext = &io_rParentContext;
    pCx_CheckStar->Set_End_FolloUpContext(io_rParentContext);
}

Context_Docu::~Context_Docu()
{
}

void
Context_Docu::ReadCharChain( CharacterSource & io_rText )
{
    csv_assert(pParentContext != 0);

    pNewToken = 0;

    UINT16 nTokenId = 0;
    StmBoundsStatu2 & rBound = aStateMachine.GetCharChain(nTokenId, io_rText);

    // !!!
    //   The order of the next two lines is essential, because
    //   pFollowUpContext may be changed by PerformStatusFunction() also,
    //   which then MUST override the previous assignment.
    pFollowUpContext = rBound.FollowUpContext();
    PerformStatusFunction(rBound.StatusFunctionNr(), nTokenId, io_rText);
}

bool
Context_Docu::PassNewToken()
{
    if (pNewToken)
    {
        pReceiver->Receive(*pNewToken.Release());
        return true;
    }
    return false;
}

TkpContext &
Context_Docu::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Context_Docu::PerformStatusFunction( uintt              i_nStatusSignal,
                                     UINT16             i_nTokenId,
                                     CharacterSource &  io_rText )
{
    switch (i_nStatusSignal)
    {
        case nF_fin_White:
            io_rText.CutToken();
            pNewToken = new Tok_White;
            break;
        case nF_fin_Error:
            throw X_AutodocParser(X_AutodocParser::x_InvalidChar);
            // no break because of throw
        case nF_fin_Ignore:
            pNewToken = 0;
            io_rText.CutToken();
            break;
        case nF_fin_Eof:
            if (bIsMultiline)
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
            else
                io_rText.CutToken();
                pNewToken = new Tok_EOF;
            break;
        case nF_fin_AnyWord:
            pNewToken = new Tok_Word(io_rText.CutToken());
            break;
        case nF_fin_AtTag:
            io_rText.CutToken();
            pNewToken = new Tok_AtTag( i_nTokenId - 100 );
            break;
        case nF_fin_Comma:
            io_rText.CutToken();
            pNewToken = new Tok_Comma;
            break;
        case nF_fin_EndSign:
            io_rText.CutToken();
            switch (i_nTokenId)
            {
                case nTok_MLDocuEnd:
                    if (bIsMultiline)
                    {
                        pNewToken = new Tok_DocuEnd;
                        pFollowUpContext = pParentContext;
                    }
                    else
                    {
                        pNewToken = new Tok_Word(io_rText.CutToken());
                        pFollowUpContext = this;
                    }
                    break;
                case nTok_EOL:
                    if (bIsMultiline)
                    {
                        pNewToken = new Tok_EOL;
                        pFollowUpContext = this;
                    }
                    else
                    {
                        pNewToken = new Tok_DocuEnd;
                        pFollowUpContext = pParentContext;
                    }
                    pReceiver->Increment_CurLine();
                    break;
                default:
                    csv_assert(false);
            }
            break;
        case nF_goto_EoHtml:
            pCx_EoHtml->SetIfIsStartOfParagraph(i_nTokenId == nTok_html_parastart);
            break;
        case nF_goto_EoXmlConst:
            pCx_EoXmlConst->SetTokenId(i_nTokenId - 200);
            break;
        case nF_goto_EoXmlLink_BeginTag:
            pCx_EoXmlLink_BeginTag->SetTokenId(i_nTokenId - 300);
            break;
        case nF_goto_EoXmlLink_EndTag:
            pCx_EoXmlLink_EndTag->SetTokenId(i_nTokenId - 325);
            break;
        case nF_goto_EoXmlFormat_BeginTag:
            pCx_EoXmlFormat_BeginTag->SetTokenId(i_nTokenId - 350);
            break;
        case nF_goto_EoXmlFormat_EndTag:
            pCx_EoXmlFormat_EndTag->SetTokenId(i_nTokenId - 375);
            break;
        case nF_goto_CheckStar:
            pCx_CheckStar->SetIsEnd( bIsMultiline );
            break;
        default:
            csv_assert(false);
    }   // end switch (i_nStatusSignal)
}

void
Context_Docu::SetupStateMachine()
{
    // special array statuses (no tokenfinish or change of context):
//  const INT16 bas = 0;        // base status
    const INT16 wht = 1;        // skip whitespace status
    const INT16 awd = 2;        // any word read status

    // change of context statuses:
    const INT16 goto_EoHtml = 3;
    const INT16 goto_EoXmlConst = 4;
    const INT16 goto_EoXmlLink_BeginTag = 5;
    const INT16 goto_EoXmlLink_EndTag = 6;
    const INT16 goto_EoXmlFormat_BeginTag = 7;
    const INT16 goto_EoXmlFormat_EndTag = 8;
    const INT16 goto_CheckStar = 9;

    // tokenfinish statuses:
    const INT16 finError = 10;
//  const INT16 finIgnore = 11;
    const INT16 finEof = 12;
    const INT16 finAnyWord = 13;
    const INT16 finAtTag = 14;
    const INT16 finEndSign = 15;
//  const INT16 finComma = 16;
    const INT16 finWhite = 17;

    // constants for use in the table:
    const INT16 ght = goto_EoHtml;
    const INT16 err = finError;
    const INT16 faw = finAnyWord;
//  const INT16 fig = finIgnore;
//  const INT16 fes = finEndSign;
    const INT16 fof = finEof;
//  const INT16 fat = finAtTag;
    const INT16 fwh = finWhite;

    /// The '0's  will be replaced by calls of AddToken().

    const INT16 A_nTopStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,  0,wht,wht,  0,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // ... 31
     wht,awd,awd,awd,awd,awd,awd,awd,awd,awd,  0,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,  0,awd,awd,awd, // ... 63
       0,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
    };

    const INT16 A_nWhitespaceStatus[C_nStatusSize] =
  //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,fwh,wht,wht,fwh,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // ... 31
     wht,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,
     fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh, // ... 63
     fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,
     fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh, // ... 95
     fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,
     fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh,fwh  // ... 127
    };

    const INT16 A_nWordStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {faw,err,err,err,err,err,err,err,err,faw,faw,faw,faw,faw,err,err,
     err,err,err,err,err,err,err,err,err,err,faw,err,err,err,err,err, // ... 31
     faw,awd,awd,awd,awd,awd,awd,awd,awd,awd,faw,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,faw,awd,awd,awd, // ... 63
     faw,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
    };

    const INT16 A_nAtTagDefStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {faw,err,err,err,err,err,err,err,err,faw,faw,faw,faw,faw,err,err,
     err,err,err,err,err,err,err,err,err,err,faw,err,err,err,err,err, // ... 31
     faw,awd,awd,awd,awd,awd,awd,awd,awd,awd,faw,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,faw,awd,faw,awd,awd,awd, // ... 63
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
    };

    const INT16 A_nHtmlDefStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {ght,err,err,err,err,err,err,err,err,ght,ght,ght,ght,ght,err,err,
     err,err,err,err,err,err,err,err,err,err,ght,err,err,err,err,err, // ... 31
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght, // ... 63
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght, // ... 95
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,
     ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght,ght  // ... 127
    };

    const INT16 A_nPunctDefStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 16 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 48 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 80 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err  // 112 ...
    };

    DYN StmArrayStatu2 * dpStatusTop
            = new StmArrayStatu2( C_nStatusSize, A_nTopStatus, 0, true);
    DYN StmArrayStatu2 * dpStatusWhite
            = new StmArrayStatu2( C_nStatusSize, A_nWhitespaceStatus, 0, true);
    DYN StmArrayStatu2 * dpStatusWord
            = new StmArrayStatu2( C_nStatusSize, A_nWordStatus, 0, true);

    DYN StmBoundsStatu2 *   dpBst_goto_EoHtml
            = new StmBoundsStatu2( *pCx_EoHtml, nF_goto_EoHtml, true );
    DYN StmBoundsStatu2 *   dpBst_goto_EoXmlConst
            = new StmBoundsStatu2( *pCx_EoXmlConst, nF_goto_EoXmlConst, true );
    DYN StmBoundsStatu2 *   dpBst_goto_EoXmlLink_BeginTag
            = new StmBoundsStatu2( *pCx_EoXmlLink_BeginTag, nF_goto_EoXmlLink_BeginTag, true );
    DYN StmBoundsStatu2 *   dpBst_goto_EoXmlLink_EndTag
            = new StmBoundsStatu2( *pCx_EoXmlLink_EndTag, nF_goto_EoXmlLink_EndTag, true );
    DYN StmBoundsStatu2 *   dpBst_goto_EoXmlFormat_BeginTag
            = new StmBoundsStatu2( *pCx_EoXmlFormat_BeginTag, nF_goto_EoXmlFormat_BeginTag, true );
    DYN StmBoundsStatu2 *   dpBst_goto_EoXmlFormat_EndTag
            = new StmBoundsStatu2( *pCx_EoXmlFormat_EndTag, nF_goto_EoXmlFormat_EndTag, true );
    DYN StmBoundsStatu2 *   dpBst_goto_CheckStar
            = new StmBoundsStatu2( *pCx_CheckStar, nF_goto_CheckStar, true );


    DYN StmBoundsStatu2 *   dpBst_finError
            = new StmBoundsStatu2( TkpContext_Null2_(), nF_fin_Error, true );
    DYN StmBoundsStatu2 *   dpBst_finIgnore
            = new StmBoundsStatu2( *this, nF_fin_Ignore, true);
    DYN StmBoundsStatu2 *   dpBst_finEof
            = new StmBoundsStatu2( TkpContext_Null2_(), nF_fin_Eof, false);
    DYN StmBoundsStatu2 *   dpBst_finAnyWord
            = new StmBoundsStatu2( *this, nF_fin_AnyWord, true);
    DYN StmBoundsStatu2 *   dpBst_finAtTag
            = new StmBoundsStatu2( *this, nF_fin_AtTag, false);
    DYN StmBoundsStatu2 *   dpBst_finEndSign
            = new StmBoundsStatu2( *pParentContext, nF_fin_EndSign, false);
    DYN StmBoundsStatu2 *   dpBst_fin_Comma
            = new StmBoundsStatu2( *this, nF_fin_Comma, false );
    DYN StmBoundsStatu2 *   dpBst_finWhite
            = new StmBoundsStatu2( *this, nF_fin_White, false);


    // construct dpMain:
    aStateMachine.AddStatus(dpStatusTop);
    aStateMachine.AddStatus(dpStatusWhite);
    aStateMachine.AddStatus(dpStatusWord);

    aStateMachine.AddStatus(dpBst_goto_EoHtml);
    aStateMachine.AddStatus(dpBst_goto_EoXmlConst);
    aStateMachine.AddStatus(dpBst_goto_EoXmlLink_BeginTag);
    aStateMachine.AddStatus(dpBst_goto_EoXmlLink_EndTag);
    aStateMachine.AddStatus(dpBst_goto_EoXmlFormat_BeginTag);
    aStateMachine.AddStatus(dpBst_goto_EoXmlFormat_EndTag);
    aStateMachine.AddStatus(dpBst_goto_CheckStar);

    aStateMachine.AddStatus(dpBst_finError);
    aStateMachine.AddStatus(dpBst_finIgnore);
    aStateMachine.AddStatus(dpBst_finEof);
    aStateMachine.AddStatus(dpBst_finAnyWord);
    aStateMachine.AddStatus(dpBst_finAtTag);
    aStateMachine.AddStatus(dpBst_finEndSign);
    aStateMachine.AddStatus(dpBst_fin_Comma);
    aStateMachine.AddStatus(dpBst_finWhite);


    aStateMachine.AddToken( "@author",  nTok_at_author,      A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@param",   nTok_at_param,       A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@throws",  nTok_at_throws,      A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@see",     nTok_at_see,         A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@since",   nTok_at_since,       A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@example", nTok_at_example,     A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@return",  nTok_at_return,      A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@returns", nTok_at_return,      A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@deprecated",
                                        nTok_at_deprecated,  A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@suspicious",
                                        nTok_at_suspicious,  A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@missing", nTok_at_missing,     A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@incomplete",
                                        nTok_at_incomplete,  A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@version", nTok_at_version,     A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@guarantees",
                                        nTok_at_guarantees,  A_nAtTagDefStatus, finAtTag );
    aStateMachine.AddToken( "@exception",
                                        nTok_at_exception,   A_nAtTagDefStatus, finAtTag );

    aStateMachine.AddToken( "<",        0,                   A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "*",        0,                   A_nPunctDefStatus, goto_CheckStar );
//  aStateMachine.AddToken( ",",        0,                   A_nPunctDefStatus, finComma );

    aStateMachine.AddToken( "<type",    nTok_link_typeB,     A_nHtmlDefStatus,  goto_EoXmlLink_BeginTag );
    aStateMachine.AddToken( "</type",   nTok_link_typeE,     A_nHtmlDefStatus,  goto_EoXmlLink_EndTag );
    aStateMachine.AddToken( "<member",  nTok_link_memberB,   A_nHtmlDefStatus,  goto_EoXmlLink_BeginTag );
    aStateMachine.AddToken( "</member", nTok_link_membeE,    A_nHtmlDefStatus,  goto_EoXmlLink_EndTag );
    aStateMachine.AddToken( "<const",   nTok_link_constB,    A_nHtmlDefStatus,  goto_EoXmlLink_BeginTag );
    aStateMachine.AddToken( "</const",  nTok_link_constE,    A_nHtmlDefStatus,  goto_EoXmlLink_EndTag );

    aStateMachine.AddToken( "<listing", nTok_format_listingB,A_nHtmlDefStatus,  goto_EoXmlFormat_BeginTag );
    aStateMachine.AddToken( "</listing",nTok_format_listingE,A_nHtmlDefStatus,  goto_EoXmlFormat_EndTag );
    aStateMachine.AddToken( "<code",    nTok_format_codeB,   A_nHtmlDefStatus,  goto_EoXmlFormat_BeginTag  );
    aStateMachine.AddToken( "</code",   nTok_format_codeE,   A_nHtmlDefStatus,  goto_EoXmlFormat_EndTag );
    aStateMachine.AddToken( "<atom",    nTok_format_atomB,   A_nHtmlDefStatus,  goto_EoXmlFormat_BeginTag  );
    aStateMachine.AddToken( "</atom",   nTok_format_atomE,   A_nHtmlDefStatus,  goto_EoXmlFormat_EndTag );

    aStateMachine.AddToken( "<TRUE/",   nTok_const_TRUE,     A_nHtmlDefStatus,  goto_EoXmlConst );
    aStateMachine.AddToken( "<true/",   nTok_const_TRUE,     A_nHtmlDefStatus,  goto_EoXmlConst );
    aStateMachine.AddToken( "<FALSE/",  nTok_const_FALSE,    A_nHtmlDefStatus,  goto_EoXmlConst );
    aStateMachine.AddToken( "<false/",  nTok_const_FALSE,    A_nHtmlDefStatus,  goto_EoXmlConst );
    aStateMachine.AddToken( "<NULL/",   nTok_const_NULL,     A_nHtmlDefStatus,  goto_EoXmlConst );
    aStateMachine.AddToken( "<void/",   nTok_const_void,     A_nHtmlDefStatus,  goto_EoXmlConst );

    aStateMachine.AddToken( "<p",       nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<pre",     nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<dl",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<ul",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<ol",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<table",   nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<P",       nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<PRE",     nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<DL",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<UL",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<OL",      nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );
    aStateMachine.AddToken( "<TABLE",   nTok_html_parastart, A_nHtmlDefStatus,  goto_EoHtml );

    aStateMachine.AddToken( "\r\n",     nTok_EOL,            A_nPunctDefStatus, finEndSign );
    aStateMachine.AddToken( "\n",       nTok_EOL,            A_nPunctDefStatus, finEndSign );
    aStateMachine.AddToken( "\r",       nTok_EOL,            A_nPunctDefStatus, finEndSign );
};

void
Context_Docu::SetMode_IsMultiLine( bool i_bTrue )
{
    bIsMultiline = i_bTrue;
}


}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
