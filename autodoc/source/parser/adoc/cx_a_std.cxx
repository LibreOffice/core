/*************************************************************************
 *
 *  $RCSfile: cx_a_std.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:37:45 $
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
#include <adoc/cx_a_std.hxx>


// NOT FULLY DEFINED SERVICES
#include <adoc/cx_a_sub.hxx>
#include <x_parse.hxx>
#include <tools/tkpchars.hxx>
#include <adoc/tk_attag.hxx>
#include <adoc/tk_docw.hxx>
#include <tokens/tokdeal.hxx>



namespace adoc {


const intt C_nStatusSize = 128;
const intt C_nCppInitialNrOfStati = 400;


const uintt nF_fin_Error = 1;
const uintt nF_fin_Ignore = 2;
const uintt nF_fin_LineStart = 3;
const uintt nF_fin_Eol = 4;
const uintt nF_fin_Eof = 5;
const uintt nF_fin_AnyWord = 6;
const uintt nF_fin_Whitespace = 7;

const uintt nF_goto_AtTag = 20;
const uintt nF_goto_CheckStar = 21;

DYN TextToken * TCF_DocWord(const char * text) { return new Tok_DocWord(text); }

DYN TextToken * TCF_atstd_ATT(const char * text) { return new Tok_at_std(ary::info::atid_ATT); }
DYN TextToken * TCF_atstd_author(const char * text) { return new Tok_at_std(ary::info::atid_author); }
DYN TextToken * TCF_atstd_change(const char * text) { return new Tok_at_std(ary::info::atid_change); }
DYN TextToken * TCF_atstd_collab(const char * text) { return new Tok_at_std(ary::info::atid_collab); }
DYN TextToken * TCF_atstd_contact(const char * text) { return new Tok_at_std(ary::info::atid_contact); }
DYN TextToken * TCF_atstd_copyright(const char * text) { return new Tok_at_std(ary::info::atid_copyright); }
DYN TextToken * TCF_atstd_descr(const char * text) { return new Tok_at_std(ary::info::atid_descr); }
DYN TextToken * TCF_atstd_docdate(const char * text) { return new Tok_at_std(ary::info::atid_docdate); }
DYN TextToken * TCF_atstd_derive(const char * text) { return new Tok_at_std(ary::info::atid_derive); }
DYN TextToken * TCF_atstd_instance(const char * text) { return new Tok_at_std(ary::info::atid_instance); }
DYN TextToken * TCF_atstd_life(const char * text) { return new Tok_at_std(ary::info::atid_life); }
DYN TextToken * TCF_atstd_multi(const char * text) { return new Tok_at_std(ary::info::atid_multi); }
DYN TextToken * TCF_atstd_onerror(const char * text) { return new Tok_at_std(ary::info::atid_onerror); }
DYN TextToken * TCF_atstd_persist(const char * text) { return new Tok_at_std(ary::info::atid_persist); }
DYN TextToken * TCF_atstd_postcond(const char * text) { return new Tok_at_std(ary::info::atid_postcond); }
DYN TextToken * TCF_atstd_precond(const char * text) { return new Tok_at_std(ary::info::atid_precond); }
DYN TextToken * TCF_atstd_responsibility(const char * text) { return new Tok_at_std(ary::info::atid_resp); }
DYN TextToken * TCF_atstd_return(const char * text) { return new Tok_at_std(ary::info::atid_return); }
DYN TextToken * TCF_atstd_short(const char * text) { return new Tok_at_std(ary::info::atid_short); }
DYN TextToken * TCF_atstd_todo(const char * text) { return new Tok_at_std(ary::info::atid_todo); }
DYN TextToken * TCF_atstd_version(const char * text) { return new Tok_at_std(ary::info::atid_version); }

DYN TextToken * TCF_at_base(const char *) { return new Tok_at_base; }
DYN TextToken * TCF_at_exception(const char *) { return new Tok_at_exception; }
DYN TextToken * TCF_at_impl(const char *) { return new Tok_at_impl; }
DYN TextToken * TCF_at_interface(const char *) { return new Tok_at_interface; }
DYN TextToken * TCF_at_key(const char *) { return new Tok_at_key; }
DYN TextToken * TCF_at_param(const char *) { return new Tok_at_param; }
DYN TextToken * TCF_at_see(const char *) { return new Tok_at_see; }
DYN TextToken * TCF_at_template(const char *) { return new Tok_at_template; }
DYN TextToken * TCF_at_internal(const char *) { return new Tok_at_internal; }
DYN TextToken * TCF_at_obsolete(const char *) { return new Tok_at_obsolete; }
DYN TextToken * TCF_at_module(const char *) { return new Tok_at_module; }
DYN TextToken * TCF_at_file(const char *) { return new Tok_at_file; }
DYN TextToken * TCF_at_gloss(const char *) { return new Tok_at_gloss; }
DYN TextToken * TCF_at_global(const char *) { return new Tok_at_global; }
DYN TextToken * TCF_at_include(const char *) { return new Tok_at_include; }
DYN TextToken * TCF_at_label(const char *) { return new Tok_at_label; }
DYN TextToken * TCF_at_since(const char *) { return new Tok_at_since; }
DYN TextToken * TCF_at_HTML(const char *) { return new Tok_at_HTML; }
DYN TextToken * TCF_at_NOHTML(const char *) { return new Tok_at_NOHTML; }
DYN TextToken * TCF_Whitespace(const char * i_sText);
DYN TextToken * TCF_EoDocu(const char *) { return new Tok_EoDocu; }
DYN TextToken * TCF_EoLine(const char *) { return new Tok_Eol; }
DYN TextToken * TCF_Eof(const char *) { return new Tok_Eof; }




Context_AdocStd::Context_AdocStd()
    :   aStateMachine(C_nStatusSize, C_nCppInitialNrOfStati),
        pDealer(0),
        pParentContext(0),
        pFollowUpContext(0),
        pCx_LineStart(0),
        pCx_CheckStar(0),
        pCx_AtTagCompletion(0),
        pNewToken(0),
        bIsMultiline(false)
{
    pCx_LineStart = new Cx_LineStart(*this);
    pCx_CheckStar = new Cx_CheckStar(*this);
    pCx_AtTagCompletion = new Cx_AtTagCompletion(*this);

    SetupStateMachine();
}

void
Context_AdocStd::SetParentContext( TkpContext & io_rParentContext,
                                   const char * i_sMultiLineEndToken )
{
    pFollowUpContext = pParentContext = &io_rParentContext;
    pCx_CheckStar->Set_End_FollowUpContext(io_rParentContext);
}

Context_AdocStd::~Context_AdocStd()
{
}

void
Context_AdocStd::AssignDealer( TokenDealer & o_rDealer )
{
    pDealer = &o_rDealer;
    pCx_LineStart->AssignDealer(o_rDealer);
    pCx_CheckStar->AssignDealer(o_rDealer);
    pCx_AtTagCompletion->AssignDealer(o_rDealer);
}

void
Context_AdocStd::ReadCharChain( CharacterSource & io_rText )
{
    csv_assert(pParentContext != 0);
    pNewToken = 0;

    TextToken::F_CRTOK  fTokenCreateFunction = 0;
    StmBoundsStatus & rBound = aStateMachine.GetCharChain(fTokenCreateFunction, io_rText);

    // !!!
    //   The order of the next two lines is essential, because
    //   pFollowUpContext may be changed by PerformStatusFunction() also,
    //   which then MUST override the previous assignment.
    pFollowUpContext = rBound.FollowUpContext();
    PerformStatusFunction(rBound.StatusFunctionNr(), fTokenCreateFunction, io_rText);
}

bool
Context_AdocStd::PassNewToken()
{
    if (pNewToken)
    {
        pNewToken.Release()->DealOut(*pDealer);
        return true;
    }
    return false;
}

TkpContext &
Context_AdocStd::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Context_AdocStd::PerformStatusFunction( uintt               i_nStatusSignal,
                                        F_CRTOK             i_fTokenCreateFunction,
                                        CharacterSource &   io_rText )
{
    switch (i_nStatusSignal)
    {
        case nF_fin_Error:
        {
            char cCC = io_rText.CurChar();
            udmstri sChar( &cCC, 1 );
            throw X_Parser(X_Parser::x_InvalidChar, sChar, udmstri::Null_(), 0);
        }   break;
        case nF_fin_Ignore:
            io_rText.CutToken();
            pNewToken = 0;
            break;
        case nF_fin_LineStart:
            csv_assert(i_fTokenCreateFunction != 0);
            pNewToken = (*i_fTokenCreateFunction)(io_rText.CutToken());
            break;
        case nF_fin_Eol:
            io_rText.CutToken();
            pDealer->Deal_Eol();
            if ( bIsMultiline )
            {
                pNewToken = TCF_EoLine(0);
                pFollowUpContext = pCx_LineStart.Ptr();
            }
            else
            {
                pNewToken = TCF_EoDocu(0);
                pFollowUpContext = pParentContext;
            }
            break;
        case nF_fin_Eof:
            pNewToken = TCF_Eof(0);
            break;
        case nF_fin_AnyWord:
            if (i_fTokenCreateFunction != 0)
                pNewToken = (*i_fTokenCreateFunction)(io_rText.CutToken());
            else
                pNewToken = TCF_DocWord(io_rText.CutToken());
            break;
        case nF_fin_Whitespace:
            pNewToken = TCF_Whitespace(io_rText.CutToken());
            break;
        case nF_goto_AtTag:
            pNewToken = 0;
            pCx_AtTagCompletion->SetCurToken(i_fTokenCreateFunction);
            break;
        case nF_goto_CheckStar:
            pNewToken = 0;
            pCx_CheckStar->SetCanBeEnd( bIsMultiline );
            break;
        default:
        {
            char cCC = io_rText.CurChar();
            udmstri sChar( &cCC, 1 );
            throw X_Parser(X_Parser::x_InvalidChar, sChar, udmstri::Null_(), 0);
        }
    }   // end switch (i_nStatusSignal)
}

void
Context_AdocStd::SetupStateMachine()
{
    // Besondere Array-Stati (kein Tokenabschluss oder Kontextwechsel):
    const INT16 bas = 0;        // Base-Status
    const INT16 wht = 1;        // Whitespace-Status
    const INT16 awd = 2;        // Any-Word-Read-Status

    // Kontextwechsel-Stati:
    const INT16 goto_CheckStar = 3;
    const INT16 goto_AtTag = 4;

    // Tokenfinish-Stati:
    const INT16 finError = 5;
    const INT16 finIgnore = 6;
    const INT16 finEol = 7;
    const INT16 finEof = 8;
    const INT16 finAnyWord = 9;
    const INT16 finWhitespace = 10;

    // Konstanten zur Benutzung in der Tabelle:
    const INT16 fof = finEof;
    const INT16 err = finError;
    const INT16 faw = finAnyWord;
    const INT16 fig = finIgnore;
    const INT16 fwh = finWhitespace;

    /// The '0's  will be replaced by calls of AddToken().

    const INT16 A_nTopStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {fof,err,err,err,err,err,err,err,err,wht,  0,wht,wht,  0,err,err,
     err,err,err,err,err,err,err,err,err,err,fof,err,err,err,err,err, // ... 31
     wht,awd,awd,awd,awd,awd,awd,awd,awd,awd,  0,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 63
       0,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
    };

    const INT16 A_nWhitespaceStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
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
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 63
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
    };

    const INT16 A_nAtTagDefStatus[C_nStatusSize] =
    //  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    {faw,err,err,err,err,err,err,err,err,faw,faw,faw,faw,faw,err,err,
     err,err,err,err,err,err,err,err,err,err,faw,err,err,err,err,err, // ... 31
     faw,awd,awd,awd,awd,awd,awd,awd,awd,awd,faw,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 63
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd, // ... 95
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,
     awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd,awd  // ... 127
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

    DYN StmArrayStatus * dpStatusTop
            = new StmArrayStatus( C_nStatusSize, A_nTopStatus, 0, true);
    DYN StmArrayStatus * dpStatusWhite
            = new StmArrayStatus( C_nStatusSize, A_nWhitespaceStatus, 0, true);
    DYN StmArrayStatus * dpStatusWord
            = new StmArrayStatus( C_nStatusSize, A_nWordStatus, TCF_DocWord, true);

    DYN StmBoundsStatus *   dpBst_goto_CheckStar
            = new StmBoundsStatus( *this, *pCx_CheckStar, nF_goto_CheckStar, true );
    DYN StmBoundsStatus *   dpBst_goto_AtTag
            = new StmBoundsStatus( *this, *pCx_AtTagCompletion, nF_goto_AtTag, true );

    DYN StmBoundsStatus *   dpBst_finError
            = new StmBoundsStatus( *this, TkpContext::Null_(), nF_fin_Error, true );
    DYN StmBoundsStatus *   dpBst_finIgnore
            = new StmBoundsStatus( *this, *this, nF_fin_Ignore, true);
    DYN StmBoundsStatus *   dpBst_finEol
            = new StmBoundsStatus( *this, *pCx_LineStart, nF_fin_Eol, false);
    DYN StmBoundsStatus *   dpBst_finEof
            = new StmBoundsStatus(  *this, TkpContext::Null_(), nF_fin_Eof, false);
    DYN StmBoundsStatus *   dpBst_finAnyWord
            = new StmBoundsStatus( *this, *this, nF_fin_AnyWord, true);
    DYN StmBoundsStatus *   dpBst_finWhitespace
            = new StmBoundsStatus( *this, *this, nF_fin_Whitespace, true);

    // dpMain aufbauen:
    aStateMachine.AddStatus(dpStatusTop);
    aStateMachine.AddStatus(dpStatusWhite);
    aStateMachine.AddStatus(dpStatusWord);

    aStateMachine.AddStatus(dpBst_goto_CheckStar);
    aStateMachine.AddStatus(dpBst_goto_AtTag);

    aStateMachine.AddStatus(dpBst_finError);
    aStateMachine.AddStatus(dpBst_finIgnore);
    aStateMachine.AddStatus(dpBst_finEol);
    aStateMachine.AddStatus(dpBst_finEof);
    aStateMachine.AddStatus(dpBst_finAnyWord);
    aStateMachine.AddStatus(dpBst_finWhitespace);

    aStateMachine.AddToken( "*",        0,                  A_nPunctDefStatus, goto_CheckStar );
    aStateMachine.AddToken( "@ATT",     TCF_atstd_ATT,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@att",     TCF_atstd_ATT,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@ATTENTION",
                                        TCF_atstd_ATT,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@attention",
                                        TCF_atstd_ATT,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@author",  TCF_atstd_author,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@change",  TCF_atstd_change,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@collab",  TCF_atstd_collab,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@collaborator",
                                        TCF_atstd_collab,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@contact", TCF_atstd_contact,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@copyright",TCF_atstd_copyright, A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@descr",   TCF_atstd_descr,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@docdate", TCF_atstd_docdate,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@derive",  TCF_atstd_derive,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@instance",TCF_atstd_instance, A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@life",    TCF_atstd_life,     A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@lifecycle",
                                        TCF_atstd_life,     A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@multi",   TCF_atstd_multi,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@multiplicity",
                                        TCF_atstd_multi,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@onerror", TCF_atstd_onerror,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@persist", TCF_atstd_persist,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@postcond",TCF_atstd_postcond,A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@precond", TCF_atstd_precond,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@resp",    TCF_atstd_responsibility,
                                                            A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@responsibility",
                                        TCF_atstd_return,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@return",  TCF_atstd_return,   A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@short",   TCF_atstd_short,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@todo",    TCF_atstd_todo,     A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@version", TCF_atstd_version,  A_nAtTagDefStatus, goto_AtTag );

    aStateMachine.AddToken( "@base",    TCF_at_base,        A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@exception",TCF_at_exception,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@impl",    TCF_at_impl,        A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@key",     TCF_at_key,         A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@param",   TCF_at_param,       A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@see",     TCF_at_see,         A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@seealso", TCF_at_see,         A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@since",   TCF_at_since,       A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@tpl",     TCF_at_template,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@tplparam",
                                        TCF_at_template,    A_nAtTagDefStatus, goto_AtTag );

    aStateMachine.AddToken( "@interface",TCF_at_interface,  A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@internal",TCF_at_internal,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@obsolete",TCF_at_obsolete,    A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@deprecated",TCF_at_obsolete,  A_nAtTagDefStatus, goto_AtTag );

    aStateMachine.AddToken( "@module",  TCF_at_module,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@file",    TCF_at_file,        A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@gloss",   TCF_at_gloss,       A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@global#", TCF_at_global,      A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@include#",TCF_at_include,     A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@#",       TCF_at_label,       A_nAtTagDefStatus, goto_AtTag );

    aStateMachine.AddToken( "@HTML",    TCF_at_HTML,        A_nAtTagDefStatus, goto_AtTag );
    aStateMachine.AddToken( "@NOHTML",  TCF_at_NOHTML,      A_nAtTagDefStatus, goto_AtTag );

    aStateMachine.AddToken( "\r\n",     0,                  A_nPunctDefStatus, finEol );
    aStateMachine.AddToken( "\n",       0,                  A_nPunctDefStatus, finEol );
    aStateMachine.AddToken( "\r",       0,                  A_nPunctDefStatus, finEol );
};

void
Context_AdocStd::SetMode_IsMultiLine( bool  i_bTrue )
{
    bIsMultiline = i_bTrue;
}

DYN TextToken *
TCF_Whitespace(const char * i_sText)
{
    UINT8 nSize = strlen(i_sText);
    for ( const char * pTab = strchr(i_sText,'\t');
          pTab != 0;
          pTab = strchr(pTab+1,'\t') )
    {
         nSize += 3;
    }

    return new Tok_Whitespace(nSize);
}


}   // namespace adoc


/*
@ATT[ENTION]
@author
@change[s]
@collab[orators]
@contact
@copyright
@descr
@devstat[e]
@docdate
@derive
@instance
@life[cycle]
@multi[plicity]
@onerror
@persist[ence]
@postcond
@precond
@return
@short
@todo

@module
@file
@gloss[ary]


@base <BasisklassenName>
@exception <ExceptionName>
@impl[ements] <IDL-Construct>
@key[words]|[s]
@param <FunctionParameterName> [<Range of valid values>]
@see[also]
@templ[ate] <FormalTemplateParameterName>

@internal
@obsolete

@#<Label>

@global#<Label> Global comment.
@include#<Label>


*/

