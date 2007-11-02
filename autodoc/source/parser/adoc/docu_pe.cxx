/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docu_pe.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:47:02 $
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
    :   pCurDocu(0),
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
Adoc_PE::Hdl_at_std( const Tok_at_std & i_rTok )
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
Adoc_PE::Hdl_at_key( const Tok_at_key & )
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
Adoc_PE::Hdl_at_see( const Tok_at_see & )
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

//  pCurAtTag = CurDocu().Assign2_ModuleTag();
//  nCurSpecialMeaningTokens = pCurAtTag->NrOfSpecialMeaningTokens();
}

void
Adoc_PE::Hdl_at_file( const Tok_at_file & )
{
    // KORR_FUTURE

//  pCurAtTag = CurDocu().Assign2_FileTag();
//  nCurSpecialMeaningTokens = pCurAtTag->NrOfSpecialMeaningTokens();
}

void
Adoc_PE::Hdl_at_gloss( const Tok_at_gloss & )
{
    // KORR_FUTURE

//  Create_GlossaryEntry();
}

void
Adoc_PE::Hdl_at_global( const Tok_at_global & )
{
    // KORR_FUTURE
//  Create_GlobalTextComponent();
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
Adoc_PE::Hdl_Whitespace( const Tok_Whitespace & i_rTok )
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
Adoc_PE::Hdl_EoDocu( const Tok_EoDocu & )
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





