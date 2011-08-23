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
#include <adoc/docu_pe.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include <ary/info/ci_attag.hxx>
#include <ary/info/ci_text.hxx>
#include <adoc/adoc_tok.hxx>
#include <adoc/tk_attag.hxx>
#include <adoc/tk_docw.hxx>


namespace adoc
{


inline bool
Adoc_PE::UsesHtmlInDocuText()
{
    return bUsesHtmlInDocuText;
}




Adoc_PE::Adoc_PE()
    :	pCurDocu(0),
        pCurAtTag(0),
        nLineCountInDocu(0),
        nCurSpecialMeaningTokens(0),
        nCurSubtractFromLineStart(0),
        eCurTagState(ts_new),
        eDocuState(ds_wait_for_short),
        bIsComplete(false),
        bUsesHtmlInDocuText(false)
{
}

Adoc_PE::~Adoc_PE()
{
}

void
Adoc_PE::Hdl_at_std( const Tok_at_std &	i_rTok )
{
    InstallAtTag(
        CurDocu().Create_StdTag(i_rTok.Id()) );
}

void
Adoc_PE::Hdl_at_base( const Tok_at_base & )
{
    InstallAtTag(
        CurDocu().CheckIn_BaseTag() );
}

void
Adoc_PE::Hdl_at_exception( const Tok_at_exception & )
{
    InstallAtTag(
        CurDocu().CheckIn_ExceptionTag() );
}

void
Adoc_PE::Hdl_at_impl( const Tok_at_impl & )
{
    InstallAtTag(
        CurDocu().Create_ImplementsTag() );
}

void
Adoc_PE::Hdl_at_key( const Tok_at_key &	)
{
    InstallAtTag(
        CurDocu().Create_KeywordTag() );
}

void
Adoc_PE::Hdl_at_param( const Tok_at_param & )
{
    InstallAtTag(
        CurDocu().CheckIn_ParameterTag() );
}

void
Adoc_PE::Hdl_at_see( const Tok_at_see &	)
{
    InstallAtTag(
        CurDocu().CheckIn_SeeTag() );
}

void
Adoc_PE::Hdl_at_template( const Tok_at_template & )
{
    InstallAtTag(
        CurDocu().CheckIn_TemplateTag() );
}

void
Adoc_PE::Hdl_at_interface( const Tok_at_interface & )
{
    CurDocu().Set_Interface();
}

void
Adoc_PE::Hdl_at_internal( const Tok_at_internal & )
{
    CurDocu().Set_Internal();
}

void
Adoc_PE::Hdl_at_obsolete( const Tok_at_obsolete & )
{
    CurDocu().Set_Obsolete();
}

void
Adoc_PE::Hdl_at_module( const Tok_at_module & )
{
    // KORR_FUTURE

//	pCurAtTag = CurDocu().Assign2_ModuleTag();
//	nCurSpecialMeaningTokens = pCurAtTag->NrOfSpecialMeaningTokens();
}

void
Adoc_PE::Hdl_at_file( const Tok_at_file & )
{
    // KORR_FUTURE

//	pCurAtTag = CurDocu().Assign2_FileTag();
//	nCurSpecialMeaningTokens = pCurAtTag->NrOfSpecialMeaningTokens();
}

void
Adoc_PE::Hdl_at_gloss( const Tok_at_gloss & )
{
    // KORR_FUTURE

//	Create_GlossaryEntry();
}

void
Adoc_PE::Hdl_at_global( const Tok_at_global & )
{
    // KORR_FUTURE
//	Create_GlobalTextComponent();
}

void
Adoc_PE::Hdl_at_include( const Tok_at_include & )
{
    // KORR_FUTURE
}

void
Adoc_PE::Hdl_at_label( const Tok_at_label & )
{
    InstallAtTag(
        CurDocu().Create_LabelTag() );
}

void
Adoc_PE::Hdl_at_since( const Tok_at_since & )
{
    InstallAtTag(
        CurDocu().Create_SinceTag() );
}

void
Adoc_PE::Hdl_at_HTML( const Tok_at_HTML &  )
{
    bUsesHtmlInDocuText = true;
}

void
Adoc_PE::Hdl_at_NOHTML( const Tok_at_NOHTML & )
{
    bUsesHtmlInDocuText = false;
}

void
Adoc_PE::Hdl_DocWord( const Tok_DocWord & i_rTok )
{
    bool bIsSpecial = false;
    if ( nCurSpecialMeaningTokens > 0 )
    {
        bIsSpecial = CurAtTag().Add_SpecialMeaningToken(
                                    i_rTok.Text(),
                                    CurAtTag().NrOfSpecialMeaningTokens()
                                        - (--nCurSpecialMeaningTokens) );
    }

    if ( NOT bIsSpecial )
    {
        if ( eDocuState == ds_wait_for_short OR eDocuState == ds_1newline_after_short )
            eDocuState = ds_in_short;
        if (nLineCountInDocu == 0)
            nLineCountInDocu = 1;

        uintt nLength = i_rTok.Length();
        if ( nLength > 2 )
        {
            bool bMaybeGlobalLink = strncmp( "::", i_rTok.Text(), 2 ) == 0;
            bool bMayBeFunction = *(i_rTok.Text() + nLength - 2) == '('
                                  AND *(i_rTok.Text() + nLength - 1) == ')';
            if ( bMaybeGlobalLink OR bMayBeFunction )
            {
                CurAtTag().Add_PotentialLink( i_rTok.Text(),
                                              bMaybeGlobalLink,
                                              bMayBeFunction );
                return;
            }
        }

        CurAtTag().Add_Token( i_rTok.Text() );
        eCurTagState = ts_std;
    }
}

void
Adoc_PE::Hdl_Whitespace( const Tok_Whitespace &	i_rTok )
{
    if ( eCurTagState == ts_std )
    {

        CurAtTag().Add_Whitespace(i_rTok.Size());
    }
}

void
Adoc_PE::Hdl_LineStart( const Tok_LineStart & i_rTok )
{
    if ( pCurAtTag == 0 )
        return;

    if ( nLineCountInDocu == 2 )
    {
        nCurSubtractFromLineStart = i_rTok.Size();
        eCurTagState = ts_std;
    }
    else if ( nLineCountInDocu > 2 )
    {
        if ( i_rTok.Size() > nCurSubtractFromLineStart )
        {
            CurAtTag().Add_Whitespace( i_rTok.Size()
                                       - nCurSubtractFromLineStart );
        }
        // else do nothing, because there is no whitespace.
    }
}

void
Adoc_PE::Hdl_Eol( const Tok_Eol & )
{
    if ( pCurAtTag == 0 )
        return;

    nLineCountInDocu++;

    if ( nCurSpecialMeaningTokens == 0 )
    {
        CurAtTag().Add_Eol();

        switch ( eDocuState )
        {
            case ds_wait_for_short:         break;
             case ds_in_short:               if ( nLineCountInDocu < 4 )
                                                eDocuState = ds_1newline_after_short;
                                            else
                                            {
                                                RenameCurShortTag();
                                                eDocuState = ds_in_descr;
                                            }
                                            break;
            case ds_1newline_after_short:   FinishCurShortTag();
                                            eDocuState = ds_in_descr;
                                            break;
            default:
                                            ;   // Do noting.
        }
    }
    else
    {
        nCurSpecialMeaningTokens = 0;
    }


}

void
Adoc_PE::Hdl_EoDocu( const Tok_EoDocu &	)
{
    bIsComplete = true;
}

DYN ary::doc::OldCppDocu *
Adoc_PE::ReleaseJustParsedDocu()
{
    pCurAtTag = 0;
    nLineCountInDocu = 0;
    nCurSpecialMeaningTokens = 0;
    nCurSubtractFromLineStart = 0;
    eCurTagState = ts_new;
    eDocuState = ds_wait_for_short;
    bIsComplete = false;
    return pCurDocu.Release();
}

void
Adoc_PE::InstallAtTag( DYN ary::info::AtTag * let_dpTag,
                       bool                   i_bImplicit )
{
    pCurAtTag = let_dpTag;
    if ( pCurAtTag != 0 )
    {
        nCurSpecialMeaningTokens = pCurAtTag->NrOfSpecialMeaningTokens();
        pCurAtTag->Set_HtmlUseInDocuText( bUsesHtmlInDocuText );
    }

    eCurTagState = ts_new;
    if ( NOT i_bImplicit )
        eDocuState = ds_std;
}

ary::doc::OldCppDocu &
Adoc_PE::CurDocu()
{
    if (NOT pCurDocu)
        pCurDocu = new ary::doc::OldCppDocu;
    return *pCurDocu;
}

ary::info::AtTag &
Adoc_PE::CurAtTag()
{
    if (NOT pCurAtTag)
    {
        if ( int(eDocuState) <  int(ds_in_descr) )
        {
            InstallAtTag(
                CurDocu().Create_StdTag(ary::info::atid_short),
                true );
        }
        else
        {
            InstallAtTag(
                CurDocu().Create_StdTag(ary::info::atid_descr),
                true );
        }
    }
    return *pCurAtTag;
}

void
Adoc_PE::RenameCurShortTag()
{
    CurDocu().Replace_AtShort_By_AtDescr();
}

void
Adoc_PE::FinishCurShortTag()
{
    InstallAtTag(
        CurDocu().Create_StdTag(ary::info::atid_descr),
        true );
}


}   // namespace adoc





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
