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
#include "cx_c_std.hxx"


// NOT FULLY DECLARED SERVICES
#include "all_toks.hxx"
#include "cx_c_pp.hxx"
#include "cx_c_sub.hxx"
#include <tools/tkpchars.hxx>
#include <tokens/tkpstama.hxx>
#include <x_parse.hxx>
#include "c_dealer.hxx"


namespace cpp {


const intt C_nCppInitialNrOfStati = 600;
const intt C_nStatusSize = 128;



const uintt nF_fin_Error = 1;
const uintt nF_fin_CreateWithoutText = 2;
const uintt nF_fin_CreateWithText = 3;
const uintt nF_fin_Ignore = 4;
const uintt nF_fin_EOL = 5;
const uintt nF_fin_EOF = 6;
const uintt nF_fin_Bezeichner = 7;

const uintt nF_goto_Preprocessor = 10;
const uintt nF_goto_Comment = 11;
const uintt nF_goto_Docu = 12;
const uintt nF_goto_Const = 13;
const uintt nF_goto_UnblockMacro = 14;

// Token create functions for the state machine:
DYN TextToken * TCF_Identifier(const char * text) { return new Tok_Identifier(text); }

DYN TextToken * TCF_operator(const char *) { return new Tok_operator; }
DYN TextToken * TCF_class(const char *) { return new Tok_class; }
DYN TextToken * TCF_struct(const char *) { return new Tok_struct; }
DYN TextToken * TCF_union(const char *) { return new Tok_union; }
DYN TextToken * TCF_enum(const char *) { return new Tok_enum; }
DYN TextToken * TCF_typedef(const char *) { return new Tok_typedef; }
DYN TextToken * TCF_public(const char *) { return new Tok_public; }
DYN TextToken * TCF_protected(const char *) { return new Tok_protected; }
DYN TextToken * TCF_private(const char *) { return new Tok_private; }
DYN TextToken * TCF_template(const char *) { return new Tok_template; }
DYN TextToken * TCF_virtual(const char *) { return new Tok_virtual; }
DYN TextToken * TCF_friend(const char *) { return new Tok_friend; }
DYN TextToken * TCF_Tilde(const char *) { return new Tok_Tilde; }
DYN TextToken * TCF_const(const char *) { return new Tok_const; }
DYN TextToken * TCF_volatile(const char *) { return new Tok_volatile; }
DYN TextToken * TCF_extern(const char *) { return new Tok_extern; }
DYN TextToken * TCF_static(const char *) { return new Tok_static; }
DYN TextToken * TCF_mutable(const char *) { return new Tok_mutable; }
DYN TextToken * TCF_register(const char *) { return new Tok_register; }
DYN TextToken * TCF_inline(const char *) { return new Tok_inline; }
DYN TextToken * TCF_explicit(const char *) { return new Tok_explicit; }
DYN TextToken * TCF_namespace(const char *) { return new Tok_namespace; }
DYN TextToken * TCF_using(const char *) { return new Tok_using; }
DYN TextToken * TCF_throw(const char *) { return new Tok_throw; }
DYN TextToken * TCF_SwBracketOpen(const char *) { return new Tok_SwBracket_Left; }
DYN TextToken * TCF_SwBracketClose(const char *) { return new Tok_SwBracket_Right; }
DYN TextToken * TCF_ArBracketOpen(const char *) { return new Tok_ArrayBracket_Left; }
DYN TextToken * TCF_ArBracketClose(const char *) { return new Tok_ArrayBracket_Right; }
DYN TextToken * TCF_BracketOpen(const char *) { return new Tok_Bracket_Left; }
DYN TextToken * TCF_BracketClose(const char *) { return new Tok_Bracket_Right; }
DYN TextToken * TCF_DblColon(const char *) { return new Tok_DoubleColon; }
DYN TextToken * TCF_Semikolon(const char *) { return new Tok_Semicolon; }
DYN TextToken * TCF_Komma(const char *) { return new Tok_Comma; }
DYN TextToken * TCF_Colon(const char *) { return new Tok_Colon; }
DYN TextToken * TCF_Zuweisung(const char *) { return new Tok_Assign; }
DYN TextToken * TCF_Smaller(const char *) { return new Tok_Less; }
DYN TextToken * TCF_Bigger(const char *) { return new Tok_Greater; }
DYN TextToken * TCF_Stern(const char *) { return new Tok_Asterix; }
DYN TextToken * TCF_Ampersand(const char *) { return new Tok_AmpersAnd; }
DYN TextToken * TCF_Ellipse(const char *) { return new Tok_Ellipse; }
DYN TextToken * TCF_typename(const char *) { return new Tok_typename; }

    // Operators
DYN TextToken * TCF_Operator(const char * text) { return new Tok_Operator(text); }

DYN TextToken * TCF_BuiltInType(const char * text) { return new Tok_BuiltInType(text); }
DYN TextToken * TCF_TypeModifier(const char * text) { return new Tok_TypeSpecializer(text); }

DYN TextToken * TCF_Eof(const char *) { return new Tok_Eof; }



Context_CppStd::Context_CppStd( DYN autodoc::TkpDocuContext & let_drContext_Docu )
    :   Cx_Base(0),
        aStateMachine(C_nStatusSize,C_nCppInitialNrOfStati),
        pDocuContext(&let_drContext_Docu),
        pContext_Comment(0),
        pContext_Preprocessor(0),
        pContext_ConstString(0),
        pContext_ConstChar(0),
        pContext_ConstNumeric(0),
        pContext_UnblockMacro(0)
{
    pDocuContext->SetParentContext(*this,"*/");

    pContext_Comment = new Context_Comment(*this);
    pContext_Preprocessor = new Context_Preprocessor(*this);
    pContext_ConstString = new Context_ConstString(*this);
    pContext_ConstChar = new Context_ConstChar(*this);
    pContext_ConstNumeric = new Context_ConstNumeric(*this);
    pContext_UnblockMacro = new Context_UnblockMacro(*this);

    SetupStateMachine();
}

Context_CppStd::~Context_CppStd()
{
}

void
Context_CppStd::ReadCharChain( CharacterSource & io_rText )
{
    SetNewToken(0);

    TextToken::F_CRTOK fTokenCreateFunction = 0;
    StmBoundsStatus & rBound = aStateMachine.GetCharChain(fTokenCreateFunction, io_rText);

    // !!!
    // 	 The order of the next two lines is essential, because
    //   pFollowUpContext may be changed by PerformStatusFunction() also,
    //   which then MUST override the previous assignment.
    SetFollowUpContext(rBound.FollowUpContext());
    PerformStatusFunction(rBound.StatusFunctionNr(), fTokenCreateFunction, io_rText);
}

void
Context_CppStd::AssignDealer( Distributor & o_rDealer )
{
    Cx_Base::AssignDealer(o_rDealer);

    pDocuContext->AssignDealer(o_rDealer);
    pContext_Comment->AssignDealer(o_rDealer);
    pContext_Preprocessor->AssignDealer(o_rDealer);
    pContext_ConstString->AssignDealer(o_rDealer);
    pContext_ConstChar->AssignDealer(o_rDealer);
    pContext_ConstNumeric->AssignDealer(o_rDealer);
    pContext_UnblockMacro->AssignDealer(o_rDealer);
}

void
Context_CppStd::PerformStatusFunction( uintt					i_nStatusSignal,
                                       F_CRTOK 	                i_fTokenCreateFunction,
                                       CharacterSource &	    io_rText )
{
    switch (i_nStatusSignal)
    {
        case nF_fin_CreateWithoutText:
            io_rText.CutToken();
            csv_assert(i_fTokenCreateFunction != 0);
            SetNewToken( (*i_fTokenCreateFunction)(0) );
            break;
        case nF_fin_CreateWithText:
            csv_assert(i_fTokenCreateFunction != 0);
            SetNewToken( (*i_fTokenCreateFunction)(io_rText.CutToken()) );
            break;
        case nF_fin_Ignore:
            io_rText.CutToken();
            SetNewToken(0);
            break;
        case nF_fin_EOL:
            io_rText.CutToken();
            SetNewToken(0);
            Dealer().Deal_Eol();
            break;
        case nF_fin_EOF:
            io_rText.CutToken();
            SetNewToken( TCF_Eof(0) );
            break;
        case nF_fin_Bezeichner:
            SetNewToken( TCF_Identifier(io_rText.CutToken()) );
            break;

        case nF_goto_Preprocessor:
            io_rText.CutToken();
            SetNewToken(0);
            break;
        case nF_goto_Comment:
            SetNewToken(0);
            pContext_Comment->SetMode_IsMultiLine( io_rText.CutToken()[1] == '*' );
            break;
        case nF_goto_Docu:
            SetNewToken(0);
            pDocuContext->SetMode_IsMultiLine( io_rText.CutToken()[1] == '*' );
            break;
        case nF_goto_Const:
            SetNewToken(0);
            break;
        case nF_goto_UnblockMacro:
            SetNewToken(0);
            break;

        case nF_fin_Error:
        default:
        {
            char cCC = io_rText.CurChar();
            String  sCurChar( &cCC, 1 );
            throw X_Parser( X_Parser::x_InvalidChar, sCurChar, String::Null_(), 0 );
        }
    }	// end switch (i_nStatusSignal)
}

void
Context_CppStd::SetupStateMachine()
{
    // Besondere Array-Stati (kein Tokenabschluss oder Kontextwechsel):
//	const INT16	top = 0;		// Top-Status
    const INT16	wht = 1;		// Whitespace-überlese-Status
    const INT16	bez = 2;        // Bezeichner-lese-Status

    // Tokenfinish-Stati:
    const INT16	finError = 3;
    const INT16 finIgnore = 4;
    const INT16 finBezeichner = 5;
    const INT16 finKeyword = 6;
    const INT16 finPunctuation = 7;
    const INT16 finBiType = 8;
    const INT16 finTypeModifier = 9;
    const INT16 finEOL = 10;
    const INT16 finEOF = 11;

    // Kontextwechsel-Stati:
    const INT16	gotoComment = 12;
    const INT16	gotoDocu = 13;
    const INT16 gotoPreprocessor = 14;
    const INT16	gotoConstString = 15;
    const INT16	gotoConstChar = 16;
    const INT16	gotoConstNumeric = 17;
    const INT16 gotoUnblockMacro = 18;

    // Abbreviations to be used in status tables:
    const INT16 err = finError;
    const INT16 fig = finIgnore;
    const INT16 fbz = finBezeichner;
    const INT16 fof = finEOF;
    const INT16 cst = gotoConstString;
    const INT16 cch = gotoConstChar;
    const INT16 cnr = gotoConstNumeric;


    /// Zeros - '0' - will be replaced by AddToken().

    const INT16 A_nTopStatus[C_nStatusSize] =
    //  0	1	2	3	4	5	6	7	8	9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,  0,wht,wht,  0,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // 16 ...
     wht,  0,cst,  0,err,  0,  0,cch,  0,  0,  0,  0,  0,  0,  0,  0,
     cnr,cnr,cnr,cnr,cnr,cnr,cnr,cnr,cnr,cnr,  0,  0,  0,  0,  0,  0, // 48 ...
       0,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,  0,  0,  0,  0,bez, // 80 ...
       0,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,  0,  0,  0,  0,err, // 80 ...
    };

    const INT16 A_nWhitespaceStatus[C_nStatusSize] =
    // 	0	1	2	3	4	5	6	7	8	9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,fig,wht,wht,fig,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // 16 ...
     wht,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig, // 48 ...
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig, // 80 ...
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,
     fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,fig,err
    };

    const INT16 A_nBezeichnerStatus[C_nStatusSize] =
    // 	0	1	2	3	4	5	6	7	8	9  10  11  12  13  14  15
    {fbz,err,err,err,err,err,err,err,err,fbz,fbz,fbz,fbz,fbz,err,err,
     err,err,err,err,err,err,err,err,err,err,fbz,err,err,err,err,err, // 16 ...
     fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,fbz,fbz, // 48 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,bez, // 80 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,err
    };


    const INT16 A_nOperatorDefStatus[C_nStatusSize] =
    // 	0	1	2	3	4	5	6	7	8	9  10  11  12  13  14  15
    {err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 16 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 48 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, // 80 ...
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,
     err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err
    };

    const INT16 A_nBezDefStatus[C_nStatusSize] =
    // 	0	1	2	3	4	5	6	7	8	9  10  11  12  13  14  15
    {fbz,err,err,err,err,err,err,err,err,fbz,fbz,fbz,fbz,fbz,err,err,
     err,err,err,err,err,err,err,err,err,err,fbz,err,err,err,err,err, // 16 ...
     fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,fbz,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,fbz,fbz, // 48 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,bez, // 80 ...
     fbz,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,
     bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,bez,fbz,fbz,fbz,fbz,err
    };

    DYN StmArrayStatus * dpStatusTop
            = new StmArrayStatus( C_nStatusSize, A_nTopStatus, 0, true);
    DYN StmArrayStatus * dpStatusWhite
            = new StmArrayStatus( C_nStatusSize, A_nWhitespaceStatus, 0, true);
    DYN StmArrayStatus * dpStatusBez
            = new StmArrayStatus( C_nStatusSize, A_nBezeichnerStatus, TCF_Identifier, true);

    DYN StmBoundsStatus *  	dpBst_finError
            = new StmBoundsStatus( *this, TkpContext::Null_(), nF_fin_Error, true );
    DYN StmBoundsStatus *  	dpBst_finIgnore
            = new StmBoundsStatus( *this, *this, nF_fin_Ignore, true );
    DYN StmBoundsStatus *  	dpBst_finBezeichner
            = new StmBoundsStatus( *this, *this, nF_fin_Bezeichner, true );
    DYN StmBoundsStatus *  	dpBst_finKeyword
            = new StmBoundsStatus( *this, *this, nF_fin_CreateWithoutText, false );
    DYN StmBoundsStatus *  	dpBst_finPunctuation
            = new StmBoundsStatus( *this, *this, nF_fin_CreateWithText, false );
    DYN StmBoundsStatus *  	dpBst_finBiType
            = new StmBoundsStatus( *this, *this, nF_fin_CreateWithText, false );
    DYN StmBoundsStatus *  	dpBst_finTypeModifier
            = new StmBoundsStatus( *this, *this, nF_fin_CreateWithText, false );
    DYN StmBoundsStatus *  	dpBst_finEOL
            = new StmBoundsStatus( *this, *this, nF_fin_EOL, false );
    DYN StmBoundsStatus *  	dpBst_finEOF
            = new StmBoundsStatus( *this, TkpContext::Null_(), nF_fin_EOF, false );

    DYN StmBoundsStatus *  	dpBst_gotoComment
            = new StmBoundsStatus( *this, *pContext_Comment, nF_goto_Comment, false );
    DYN StmBoundsStatus *  	dpBst_gotoDocu
            = new StmBoundsStatus( *this, *pDocuContext, nF_goto_Docu, false );
    DYN StmBoundsStatus *  	dpBst_gotoPreprocessor
            = new StmBoundsStatus( *this, *pContext_Preprocessor, nF_goto_Preprocessor, false );
    DYN StmBoundsStatus *  	dpBst_gotoConstString
            = new StmBoundsStatus( *this, *pContext_ConstString, nF_goto_Const, false );
    DYN StmBoundsStatus *  	dpBst_gotoConstChar
            = new StmBoundsStatus( *this, *pContext_ConstChar, nF_goto_Const, false );
    DYN StmBoundsStatus *  	dpBst_gotoConstNumeric
            = new StmBoundsStatus( *this, *pContext_ConstNumeric, nF_goto_Const, false );
    DYN StmBoundsStatus *  	dpBst_gotoUnblockMacro
            = new StmBoundsStatus( *this, *pContext_UnblockMacro, nF_goto_UnblockMacro, false );

    // dpMain aufbauen:
    aStateMachine.AddStatus(dpStatusTop);

    aStateMachine.AddStatus(dpStatusWhite);
    aStateMachine.AddStatus(dpStatusBez);

    aStateMachine.AddStatus(dpBst_finError);
    aStateMachine.AddStatus(dpBst_finIgnore);
    aStateMachine.AddStatus(dpBst_finBezeichner);
    aStateMachine.AddStatus(dpBst_finKeyword);
    aStateMachine.AddStatus(dpBst_finPunctuation);
    aStateMachine.AddStatus(dpBst_finBiType);
    aStateMachine.AddStatus(dpBst_finTypeModifier);
    aStateMachine.AddStatus(dpBst_finEOL);
    aStateMachine.AddStatus(dpBst_finEOF);

    aStateMachine.AddStatus(dpBst_gotoComment);
    aStateMachine.AddStatus(dpBst_gotoDocu);
    aStateMachine.AddStatus(dpBst_gotoPreprocessor);
    aStateMachine.AddStatus(dpBst_gotoConstString);
    aStateMachine.AddStatus(dpBst_gotoConstChar);
    aStateMachine.AddStatus(dpBst_gotoConstNumeric);
    aStateMachine.AddStatus(dpBst_gotoUnblockMacro);

    // Identifier

    // Keywords and other unique Tokens
    aStateMachine.AddToken("operator",TCF_operator,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("class",TCF_class,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("struct",TCF_struct,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("union",TCF_union,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("enum",TCF_enum,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("typedef",TCF_typedef,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("public",TCF_public,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("protected",TCF_protected,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("private",TCF_private,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("template",TCF_template,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("virtual",TCF_virtual,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("friend",TCF_friend,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("~",TCF_Tilde,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("const",TCF_const,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("volatile",TCF_volatile,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("extern",TCF_extern,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("static",TCF_static,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("mutable",TCF_mutable,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("register",TCF_register,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("inline",TCF_inline,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("explicit",TCF_explicit,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("namespace",TCF_namespace,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("using",TCF_using,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("throw",TCF_throw,A_nBezDefStatus,finKeyword);
    aStateMachine.AddToken("{",TCF_SwBracketOpen,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("}",TCF_SwBracketClose,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("[",TCF_ArBracketOpen,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("]",TCF_ArBracketClose,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("(",TCF_BracketOpen,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken(")",TCF_BracketClose,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("::",TCF_DblColon,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken(";",TCF_Semikolon,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken(",",TCF_Komma,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken(":",TCF_Colon,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("=",TCF_Zuweisung,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("<",TCF_Smaller,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken(">",TCF_Bigger,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("*",TCF_Stern,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("&",TCF_Ampersand,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("...",TCF_Ellipse,A_nOperatorDefStatus,finKeyword);
    aStateMachine.AddToken("typename",TCF_typename,A_nOperatorDefStatus,finKeyword);

    // Operators
    aStateMachine.AddToken("==",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("!=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("<=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken(">=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("&&",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("||",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("!",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("new",TCF_Operator,A_nBezDefStatus,finPunctuation);
    aStateMachine.AddToken("delete",TCF_Operator,A_nBezDefStatus,finPunctuation);
    aStateMachine.AddToken("sizeof",TCF_Operator,A_nBezDefStatus,finPunctuation);
    aStateMachine.AddToken("typeid",TCF_Operator,A_nBezDefStatus,finPunctuation);
    aStateMachine.AddToken("+",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("-",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("/",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("%",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("^",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("|",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("<<",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken(">>",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken(".",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("->",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("?",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("+=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("-=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("*=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("/=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("%=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("&=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("|=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("^=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken("<<=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);
    aStateMachine.AddToken(">>=",TCF_Operator,A_nOperatorDefStatus,finPunctuation);

    // Builtin types
    aStateMachine.AddToken("char",	TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("short",	TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("int",	TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("long",	TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("float",	TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("double",TCF_BuiltInType,	A_nBezDefStatus,	finBiType);
    aStateMachine.AddToken("wchar_t",TCF_BuiltInType,	A_nBezDefStatus,    finBiType);
    aStateMachine.AddToken("size_t",TCF_BuiltInType,	A_nBezDefStatus,    finBiType);

    // Type modifiers
    aStateMachine.AddToken("signed",	TCF_TypeModifier,	A_nBezDefStatus,	finTypeModifier);
    aStateMachine.AddToken("unsigned",	TCF_TypeModifier,	A_nBezDefStatus,	finTypeModifier);

    // To ignore
    aStateMachine.AddToken("auto",		0,	A_nBezDefStatus,  	finIgnore);
    aStateMachine.AddToken("_cdecl",	0,	A_nBezDefStatus,	finIgnore);
    aStateMachine.AddToken("__cdecl",	0,	A_nBezDefStatus,	finIgnore);
    aStateMachine.AddToken("__stdcall", 0,	A_nBezDefStatus,	finIgnore);
    aStateMachine.AddToken("__fastcall",0,	A_nBezDefStatus,	finIgnore);
    aStateMachine.AddToken("/**/",	    0,	A_nOperatorDefStatus,finIgnore);

    // Context changers
    aStateMachine.AddToken("#",		0,	A_nOperatorDefStatus,   gotoPreprocessor);
    aStateMachine.AddToken("#undef",0,	A_nOperatorDefStatus,   gotoPreprocessor);
    aStateMachine.AddToken("#unblock-",
                                    0,	A_nOperatorDefStatus,   gotoUnblockMacro);
    aStateMachine.AddToken("/*",	0,	A_nOperatorDefStatus,	gotoComment);
    aStateMachine.AddToken("//",	0,	A_nOperatorDefStatus,	gotoComment);
    aStateMachine.AddToken("/**",   0,	A_nOperatorDefStatus,	gotoDocu);
    aStateMachine.AddToken("///",   0,	A_nOperatorDefStatus,	gotoDocu);

    // Line ends
        //	regular
    aStateMachine.AddToken("\r\n",	0,	A_nOperatorDefStatus,	finEOL);
    aStateMachine.AddToken("\n",	0,	A_nOperatorDefStatus,	finEOL);
    aStateMachine.AddToken("\r",    0,	A_nOperatorDefStatus,	finEOL);
        //	To ignore in some cases(may be only at preprocessor?), but
        //		linecount has to be incremented.
    aStateMachine.AddToken("\\\r\n",0,	A_nOperatorDefStatus,	finEOL);
    aStateMachine.AddToken("\\\n",	0,	A_nOperatorDefStatus,	finEOL);
    aStateMachine.AddToken("\\\r",	0,	A_nOperatorDefStatus,	finEOL);
};


}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
