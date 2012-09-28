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
#include <s2_luidl/cx_idlco.hxx>


// NOT FULLY DECLARED SERVICES
#include <s2_luidl/cx_sub.hxx>
#include <s2_dsapi/cx_dsapi.hxx>
#include <tools/tkpchars.hxx>
#include <tokens/tkpstam2.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_luidl/tokrecv.hxx>
#include <x_parse2.hxx>


namespace csi
{
namespace uidl
{


const intt C_nCppInitialNrOfStati = 400;
const intt C_nStatusSize = 128;



const uintt nF_fin_Error = 1;
const uintt nF_fin_Ignore = 2;
const uintt nF_fin_Identifier = 3;
const uintt nF_fin_Keyword = 4;
const uintt nF_fin_Punctuation = 5;
const uintt nF_fin_EOL = 6;
const uintt nF_fin_EOF = 7;

const uintt nF_goto_MLDocu = 10;
const uintt nF_goto_SLDocu = 11;
const uintt nF_goto_MLComment = 12;
const uintt nF_goto_SLComment = 13;
const uintt nF_goto_Praeprocessor = 14;
const uintt nF_goto_Assignment = 15;



const UINT16 nTok_bty_any = 100 + TokBuiltInType::bty_any;
const UINT16 nTok_bty_boolean = 100 + TokBuiltInType::bty_boolean;
const UINT16 nTok_bty_byte = 100 + TokBuiltInType::bty_byte;
const UINT16 nTok_bty_char = 100 + TokBuiltInType::bty_char;
const UINT16 nTok_bty_double = 100 + TokBuiltInType::bty_double;
const UINT16 nTok_bty_hyper = 100 + TokBuiltInType::bty_hyper;
const UINT16 nTok_bty_long = 100 + TokBuiltInType::bty_long;
const UINT16 nTok_bty_short = 100 + TokBuiltInType::bty_short;
const UINT16 nTok_bty_string = 100 + TokBuiltInType::bty_string;
const UINT16 nTok_bty_void = 100 + TokBuiltInType::bty_void;
const UINT16 nTok_bty_ellipse = 100 + TokBuiltInType::bty_ellipse;

const UINT16 nTok_tmod_unsigned = 200 + TokTypeModifier::tmod_unsigned;
const UINT16 nTok_tmod_sequence = 200 + TokTypeModifier::tmod_sequence;

const UINT16 nTok_ph_in = 250 + TokParameterHandling::ph_in;
const UINT16 nTok_ph_out = 250 + TokParameterHandling::ph_out;
const UINT16 nTok_ph_inout = 250 + TokParameterHandling::ph_inout;

const UINT16 nTok_mt_attribute = 300 + TokMetaType::mt_attribute;
const UINT16 nTok_mt_constants = 300 + TokMetaType::mt_constants;
const UINT16 nTok_mt_enum = 300 + TokMetaType::mt_enum;
const UINT16 nTok_mt_exception = 300 + TokMetaType::mt_exception;
const UINT16 nTok_mt_ident = 300 + TokMetaType::mt_ident;
const UINT16 nTok_mt_interface = 300 + TokMetaType::mt_interface;
const UINT16 nTok_mt_module = 300 + TokMetaType::mt_module;
const UINT16 nTok_mt_property = 300 + TokMetaType::mt_property;
const UINT16 nTok_mt_service = 300 + TokMetaType::mt_service;
const UINT16 nTok_mt_singleton = 300 + TokMetaType::mt_singleton;
const UINT16 nTok_mt_struct = 300 + TokMetaType::mt_struct;
const UINT16 nTok_mt_typedef = 300 + TokMetaType::mt_typedef;
const UINT16 nTok_mt_uik = 300 + TokMetaType::mt_uik;

const UINT16 nTok_ste_bound = 400 + TokStereotype::ste_bound;
const UINT16 nTok_ste_constrained = 400 + TokStereotype::ste_constrained;
const UINT16 nTok_ste_const = 400 + TokStereotype::ste_const;
const UINT16 nTok_ste_maybeambiguous = 400 + TokStereotype::ste_maybeambiguous;
const UINT16 nTok_ste_maybedefault = 400 + TokStereotype::ste_maybedefault;
const UINT16 nTok_ste_maybevoid = 400 + TokStereotype::ste_maybevoid;
const UINT16 nTok_ste_oneway = 400 + TokStereotype::ste_oneway;
const UINT16 nTok_ste_optional = 400 + TokStereotype::ste_optional;
const UINT16 nTok_ste_readonly = 400 + TokStereotype::ste_readonly;
const UINT16 nTok_ste_removable = 400 + TokStereotype::ste_removable;
const UINT16 nTok_ste_virtual = 400 + TokStereotype::ste_virtual;
const UINT16 nTok_ste_transient = 400 + TokStereotype::ste_transient;
const UINT16 nTok_ste_published = 400 + TokStereotype::ste_published;

const UINT16 nTok_raises = 501;
const UINT16 nTok_needs = 502;
const UINT16 nTok_observes = 503;

const UINT16 nTok_assignment = 550;

const UINT16 nTok_ignore = 600;
const UINT16 nTok_none_MLCommentBegin = 601;
const UINT16 nTok_none_SLCommentBegin = 602;
const UINT16 nTok_none_MLDocuBegin = 603;
const UINT16 nTok_none_SLDocuBegin = 604;
const UINT16 nTok_none_PraeprocessorBegin = 605;


const UINT16 nTok_punct_BracketOpen = 700 + TokPunctuation::BracketOpen;
const UINT16 nTok_punct_BracketClose = 700 + TokPunctuation::BracketClose;
const UINT16 nTok_punct_ArrayBracketOpen = 700 + TokPunctuation::ArrayBracketOpen;
const UINT16 nTok_punct_ArrayBracketClose = 700 + TokPunctuation::ArrayBracketClose;
const UINT16 nTok_punct_CurledBracketOpen = 700 + TokPunctuation::CurledBracketOpen;
const UINT16 nTok_punct_CurledBracketClose = 700 + TokPunctuation::CurledBracketClose;
const UINT16 nTok_punct_Semicolon = 700 + TokPunctuation::Semicolon;
const UINT16 nTok_punct_Colon = 700 + TokPunctuation::Colon;
const UINT16 nTok_punct_DoubleColon = 700 + TokPunctuation::DoubleColon;
const UINT16 nTok_punct_Comma = 700 + TokPunctuation::Comma;
const UINT16 nTok_punct_Minus = 700 + TokPunctuation::Minus;
const UINT16 nTok_punct_Fullstop = 700 + TokPunctuation::Fullstop;
const UINT16 nTok_punct_Lesser = 700 + TokPunctuation::Lesser;
const UINT16 nTok_punct_Greater = 700 + TokPunctuation::Greater;

const UINT16 nTok_EOL = 801;
const UINT16 nTok_EOF = 802;



Context_UidlCode::Context_UidlCode( Token_Receiver &     o_rReceiver,
                                    DYN TkpDocuContext & let_drContext_Docu )
    :   aStateMachine(C_nStatusSize,C_nCppInitialNrOfStati),
        pReceiver(&o_rReceiver),
        pDocuContext(&let_drContext_Docu),
        dpContext_MLComment(0),
        dpContext_SLComment(0),
        dpContext_Preprocessor(0),
        dpContext_Assignment(0),
        pNewToken(0),
        pFollowUpContext(0)
{
    dpContext_MLComment = new Context_MLComment(o_rReceiver,*this),
    dpContext_SLComment = new Context_SLComment(o_rReceiver,*this),
    dpContext_Preprocessor = new Context_Praeprocessor(o_rReceiver,*this),
    dpContext_Assignment = new Context_Assignment(o_rReceiver,*this),

    pDocuContext->SetParentContext(*this,"*/");
    SetupStateMachine();
}

Context_UidlCode::~Context_UidlCode()
{
}

void
Context_UidlCode::ReadCharChain( CharacterSource & io_rText )
{
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
Context_UidlCode::PassNewToken()
{
    if (pNewToken)
    {
        pReceiver->Receive(*pNewToken.Release());
        return true;
    }
    return false;
}

TkpContext &
Context_UidlCode::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Context_UidlCode::PerformStatusFunction( uintt              i_nStatusSignal,
                                         UINT16             i_nTokenId,
                                         CharacterSource &  io_rText )
{
    switch (i_nStatusSignal)
    {
        case nF_fin_Error:
            // KORR_FUTURE
            throw X_AutodocParser(X_AutodocParser::x_InvalidChar);
            // no break, because of throw
        case nF_fin_Ignore:
            pNewToken = 0;
            io_rText.CutToken();
            break;
        case nF_fin_Identifier:
            pNewToken = new TokIdentifier(io_rText.CutToken());
            break;
        case nF_fin_Keyword:
            io_rText.CutToken();
            switch ( i_nTokenId / 50 )
            {
                case 2:
                    pNewToken = new TokBuiltInType(i_nTokenId - 100);
                    break;
                case 4:
                    pNewToken = new TokTypeModifier(i_nTokenId - 200);
                    break;
                case 5:
                    pNewToken = new TokParameterHandling(i_nTokenId - 250);
                    break;
                case 6:
                    pNewToken = new TokMetaType(i_nTokenId - 300);
                    break;
                case 8:
                    pNewToken = new TokStereotype(i_nTokenId - 400);
                    break;
                case 10:
                    switch (i_nTokenId-500)
                    {
                        case 1:
                            pNewToken = new TokRaises;
                            break;
                        case 2:
                            pNewToken = new TokNeeds;
                            break;
                        case 3:
                            pNewToken = new TokObserves;
                            break;
                        default:
                            csv_assert(false);
                    }
                    break;
                default:
                    csv_assert(false);
            }   // end switch ( i_nTokenId / 50 )
            break;
        case nF_fin_Punctuation:
            io_rText.CutToken();
            if (i_nTokenId == nTok_punct_DoubleColon)
                pNewToken = new TokNameSeparator;
            else
                pNewToken = new TokPunctuation(i_nTokenId - 700);
            break;
        case nF_fin_EOL:
            io_rText.CutToken();
            pNewToken = new Tok_EOL;
            pReceiver->Increment_CurLine();
            break;
        case nF_fin_EOF:
            pNewToken = new Tok_EOF;
            break;
        case nF_goto_MLDocu:
            while ( io_rText.CurChar() == '*')
                io_rText.MoveOn();
            io_rText.CutToken();
            pDocuContext->SetMode_IsMultiLine(true);
            break;
        case nF_goto_SLDocu:
            io_rText.CutToken();
            pDocuContext->SetMode_IsMultiLine(false);
            break;
        case nF_goto_MLComment:
            break;
        case nF_goto_SLComment:
            break;
        case nF_goto_Praeprocessor:
            break;
        case nF_goto_Assignment:
            break;
        default:
            csv_assert(false);
    }   // end switch (i_nStatusSignal)
}

void
Context_UidlCode::SetupStateMachine()
{
    // special array statuses (no tokenfinish or change of context):
//  const INT16 top = 0;        // top status
    const INT16 wht = 1;        // skip whitespace status
    const INT16 bez = 2;        // read identifier status

    // tokenfinish statuses:
    const INT16 finErr = 3;
    const INT16 finIgn = 4;
    const INT16 finBez = 5;
    const INT16 finKeyw = 6;
    const INT16 finPunct = 7;
    const INT16 finEOL = 8;
    const INT16 finEOF = 9;

    // change of context statuses:
    const INT16 gotoMld = 10;
    const INT16 gotoSld = 11;
    const INT16 gotoMlc = 12;
    const INT16 gotoSlc = 13;
    const INT16 gotoPrp = 14;
    const INT16 gotoAsg = 15;

    // constants for use in the table:
    const INT16 err = finErr;
    const INT16 fbz = finBez;
    const INT16 fig = finIgn;
    const INT16 fof = finEOF;
//  const INT16 fkw = finKeyw;
//  const INT16 fpc = finPunct;

    /// '0's are going to be replaced by AddToken() later

    const INT16 A_nTopStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,  0,wht,wht,  0,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // 16 ...
     wht,err,wht,  0,err,err,err,err,  0,  0,err,err,  0,  0,  0,err,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,err,err,err,err,err,err, // 48 ...
     err,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,  0,err,  0,err,bez, // 80 ...
     err,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,  0,err,  0,err,err, // 112 ...
    };

    const INT16 A_nWhitespaceStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,fig,wht,wht,fig,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // 16 ...
     wht,fig,wht,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig, // 48 ...
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig, // 80 ...
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,err  // 112 ...
    };

    const INT16 A_nBezeichnerStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fbz,err,err,err,err,err,err,err,err,fbz,fbz,fbz,fbz,fbz,err,err,
     err,err,err,err,err,err,err,err,err,err,fbz,err,err,err,err,err, // 16 ...
     fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,fbz,fbz, // 48 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,bez, // 80 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,err  // 112 ...
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

    const INT16 A_nKeywordDefStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fbz,err,err,err,err,err,err,err,err,fbz,fbz,fbz,fbz,fbz,err,err,
     err,err,err,err,err,err,err,err,err,err,fbz,err,err,err,err,err, // 16 ...
     fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,fbz,fbz, // 48 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,bez, // 80 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,err  // 112 ...
    };

    DYN StmArrayStatu2 * dpStatusTop
            = new StmArrayStatu2( C_nStatusSize, A_nTopStatus, 0, true);
    DYN StmArrayStatu2 * dpStatusWhite
            = new StmArrayStatu2( C_nStatusSize, A_nWhitespaceStatus, 0, true);
    DYN StmArrayStatu2 * dpStatusBez
            = new StmArrayStatu2( C_nStatusSize, A_nBezeichnerStatus, 0, true);

    DYN StmBoundsStatu2 *   dpBst_finErr
            = new StmBoundsStatu2( TkpContext_Null2_(), nF_fin_Error, true );
    DYN StmBoundsStatu2 *   dpBst_finIgn
            = new StmBoundsStatu2( *this, nF_fin_Ignore, true );
    DYN StmBoundsStatu2 *   dpBst_finBez
            = new StmBoundsStatu2( *this, nF_fin_Identifier, true );
    DYN StmBoundsStatu2 *   dpBst_finKeyw
            = new StmBoundsStatu2( *this, nF_fin_Keyword, false );
    DYN StmBoundsStatu2 *   dpBst_finPunct
            = new StmBoundsStatu2( *this, nF_fin_Punctuation, false );
    DYN StmBoundsStatu2 *   dpBst_finEOL
            = new StmBoundsStatu2( *this, nF_fin_EOL, false );
    DYN StmBoundsStatu2 *   dpBst_finEOF
            = new StmBoundsStatu2( TkpContext_Null2_(), nF_fin_EOF, false );

    DYN StmBoundsStatu2 *   dpBst_gotoMld
            = new StmBoundsStatu2( *pDocuContext, nF_goto_MLDocu, false );
    DYN StmBoundsStatu2 *   dpBst_gotoSld
            = new StmBoundsStatu2( *pDocuContext, nF_goto_SLDocu, false );
    DYN StmBoundsStatu2 *   dpBst_gotoMlc
            = new StmBoundsStatu2( *dpContext_MLComment, nF_goto_MLComment, false );
    DYN StmBoundsStatu2 *   dpBst_gotoSlc
            = new StmBoundsStatu2( *dpContext_SLComment, nF_goto_SLComment, false );
    DYN StmBoundsStatu2 *   dpBst_gotoPrp
            = new StmBoundsStatu2( *dpContext_Preprocessor, nF_goto_Praeprocessor, false );
    DYN StmBoundsStatu2 *   dpBst_gotoAsg
            = new StmBoundsStatu2( *dpContext_Assignment, nF_goto_Assignment, false );

    // construct dpMain:
    aStateMachine.AddStatus(dpStatusTop);

    aStateMachine.AddStatus(dpStatusWhite);
    aStateMachine.AddStatus(dpStatusBez);

    aStateMachine.AddStatus(dpBst_finErr);
    aStateMachine.AddStatus(dpBst_finIgn);
    aStateMachine.AddStatus(dpBst_finBez);
    aStateMachine.AddStatus(dpBst_finKeyw);
    aStateMachine.AddStatus(dpBst_finPunct);
    aStateMachine.AddStatus(dpBst_finEOL);
    aStateMachine.AddStatus(dpBst_finEOF);

    aStateMachine.AddStatus(dpBst_gotoMld);
    aStateMachine.AddStatus(dpBst_gotoSld);
    aStateMachine.AddStatus(dpBst_gotoMlc);
    aStateMachine.AddStatus(dpBst_gotoSlc);
    aStateMachine.AddStatus(dpBst_gotoPrp);
    aStateMachine.AddStatus(dpBst_gotoAsg);

    aStateMachine.AddToken("any",       nTok_bty_any,           A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("attribute", nTok_mt_attribute,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("boolean",   nTok_bty_boolean,       A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("bound",     nTok_ste_bound,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("byte",      nTok_bty_byte,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("char",      nTok_bty_char,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("const",     nTok_ste_const,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("constants", nTok_mt_constants,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("constrained",
                                        nTok_ste_constrained,   A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("double",    nTok_bty_double,        A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("enum",      nTok_mt_enum,           A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("exception", nTok_mt_exception,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("hyper",     nTok_bty_hyper,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("ident",     nTok_mt_ident,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("in",        nTok_ph_in,             A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("inout",     nTok_ph_inout,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("interface", nTok_mt_interface,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("long",      nTok_bty_long,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("maybeambiguous",
                                        nTok_ste_maybeambiguous,A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("maybedefault",
                                        nTok_ste_maybedefault,  A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("maybevoid", nTok_ste_maybevoid,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("module",    nTok_mt_module,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("needs",     nTok_needs,             A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("observes",  nTok_observes,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("oneway",    nTok_ste_oneway,        A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("optional",  nTok_ste_optional,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("out",       nTok_ph_out,            A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("property",  nTok_mt_property,       A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("published", nTok_ste_published,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("raises",    nTok_raises,            A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("readonly",  nTok_ste_readonly,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("removable", nTok_ste_removable,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("sequence",  nTok_tmod_sequence,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("service",   nTok_mt_service,        A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("short",     nTok_bty_short,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("singleton", nTok_mt_singleton,      A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("string",    nTok_bty_string,        A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("struct",    nTok_mt_struct,         A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("transient", nTok_ste_transient,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("typedef",   nTok_mt_typedef,        A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("uik",       nTok_mt_uik,            A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("unsigned",  nTok_tmod_unsigned,     A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("virtual",   nTok_ste_virtual,       A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("void",      nTok_bty_void,          A_nKeywordDefStatus,    finKeyw);
    aStateMachine.AddToken("...",       nTok_bty_ellipse,       A_nPunctDefStatus,      finKeyw);

    aStateMachine.AddToken("=",         nTok_assignment,        A_nPunctDefStatus,      gotoAsg);

    aStateMachine.AddToken("(",         nTok_punct_BracketOpen, A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(")",         nTok_punct_BracketClose,A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("[",         nTok_punct_ArrayBracketOpen,
                                                                A_nPunctDefStatus,      finIgn);
    aStateMachine.AddToken("]",         nTok_punct_ArrayBracketClose,
                                                                A_nPunctDefStatus,      finIgn);
    aStateMachine.AddToken("{",         nTok_punct_CurledBracketOpen,
                                                                A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("}",         nTok_punct_CurledBracketClose,
                                                                A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("<",         nTok_punct_Lesser,      A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(">",         nTok_punct_Greater,     A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(";",         nTok_punct_Semicolon,   A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(":",         nTok_punct_Colon,       A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("::",        nTok_punct_DoubleColon, A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(",",         nTok_punct_Comma,       A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("-",         nTok_punct_Minus,       A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken(".",         nTok_punct_Fullstop,    A_nPunctDefStatus,      finPunct);
    aStateMachine.AddToken("/**",       nTok_none_MLDocuBegin,  A_nPunctDefStatus,      gotoMld);
    aStateMachine.AddToken("///",       nTok_none_SLDocuBegin,  A_nPunctDefStatus,      gotoSld);
    aStateMachine.AddToken("/*",        nTok_none_MLCommentBegin,
                                                                A_nPunctDefStatus,      gotoMlc);
    aStateMachine.AddToken("//",        nTok_none_SLCommentBegin,
                                                                A_nPunctDefStatus,      gotoSlc);
    aStateMachine.AddToken("/**/",      nTok_ignore,            A_nPunctDefStatus,      finIgn);
    aStateMachine.AddToken("#",         nTok_none_PraeprocessorBegin,
                                                                A_nPunctDefStatus,      gotoPrp);
    aStateMachine.AddToken("\r\n",      nTok_EOL,               A_nPunctDefStatus,      finEOL);
    aStateMachine.AddToken("\r",        nTok_EOL,               A_nPunctDefStatus,      finEOL);
    aStateMachine.AddToken("\n",        nTok_EOL,               A_nPunctDefStatus,      finEOL);
    aStateMachine.AddToken("\n\r",      nTok_EOL,               A_nPunctDefStatus,      finEOL);
};


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
