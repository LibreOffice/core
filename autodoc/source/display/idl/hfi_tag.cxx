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
#include "hfi_tag.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary_i/ci_text2.hxx>
#include <ary_i/d_token.hxx>
#include <toolkit/out_tree.hxx>
#include <adc_cl.hxx>
#include <adc_msg.hxx>
#include "hfi_typetext.hxx"
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include "hi_linkhelper.hxx"


using ary::inf::DocuTex2;


inline void
HF_IdlTag::Enter_TextOut( Xml::Element & o_rText ) const
{
    aTextOut.Out().Enter(o_rText);
}

inline void
HF_IdlTag::Leave_TextOut() const
{
    aTextOut.Out().Leave();
}

inline void
HF_IdlTag::PutText_Out( const ary::inf::DocuTex2 & i_rText ) const
{
    i_rText.DisplayAt( const_cast< HF_IdlDocuTextDisplay& >(aTextOut) );
}



HF_IdlTag::HF_IdlTag( Environment &                 io_rEnv,
                      const ary::idl::CodeEntity &  i_rScopeGivingCe )
    :   HtmlFactory_Idl( io_rEnv, 0 ),
        pTitleOut(0),
        aTextOut(io_rEnv, 0, i_rScopeGivingCe)
{
}

HF_IdlTag::~HF_IdlTag()
{
}

void
HF_IdlTag::Produce_byData( Xml::Element &              o_rTitle,
                           Xml::Element &              o_rText,
                           const ary::inf::AtTag2 &   i_rTag ) const
{
    pTitleOut = &o_rTitle;
    Enter_TextOut(o_rText);
    i_rTag.DisplayAt( const_cast< HF_IdlTag& >(*this) );
    Leave_TextOut();
}

void
HF_IdlTag::Produce_byData( Xml::Element &      o_rTitle,
                           Xml::Element &      o_rText,
                           const std::vector< csi::dsapi::DT_SeeAlsoAtTag* > &
                                                i_seeAlsoVector ) const
{
    o_rTitle << "See also";
    for ( std::vector< csi::dsapi::DT_SeeAlsoAtTag* >::const_iterator
            it = i_seeAlsoVector.begin();
          it != i_seeAlsoVector.end();
          ++it )
    {
        if (it != i_seeAlsoVector.begin())
        {
            o_rText << ", ";
        }
        HF_IdlTypeText
            aLinkText(Env(), o_rText, &aTextOut.ScopeGivingCe());
        aLinkText.Produce_byData( (*it)->LinkText() );
    }
}

void
HF_IdlTag::Display_StdAtTag( const csi::dsapi::DT_StdAtTag & i_rTag )
{
    if ( i_rTag.Text().IsEmpty() )
        return;

    csv_assert( pTitleOut != 0 );
    *pTitleOut << i_rTag.Title();
    PutText_Out( i_rTag.Text() );
}

void
HF_IdlTag::Display_SeeAlsoAtTag( const csi::dsapi::DT_SeeAlsoAtTag & i_rTag )
{
    if ( i_rTag.Text().IsEmpty() )
        return;

    csv_assert( pTitleOut != 0 );
    *pTitleOut << "See also";

    HF_IdlTypeText aLinkText(Env(),aTextOut.CurOut(), &aTextOut.ScopeGivingCe());
    aLinkText.Produce_byData( i_rTag.LinkText() );

    aTextOut.CurOut() << new Html::LineBreak;
    PutText_Out( i_rTag.Text() );
}

void
HF_IdlTag::Display_ParameterAtTag( const csi::dsapi::DT_ParameterAtTag & i_rTag )
{
    csv_assert( pTitleOut != 0 );
    StreamLock sl(100);
    *pTitleOut
        << ( sl() << "Parameter " << i_rTag.Title() << c_str );
    PutText_Out( i_rTag.Text() );
}

void
HF_IdlTag::Display_SinceAtTag( const csi::dsapi::DT_SinceAtTag & i_rTag )
{
    csv_assert(pTitleOut != 0);

    if ( i_rTag.Text().IsEmpty() )
    {
         return;
    }

    // Transform the value of the @since tag into the text to be displayed.
    String sDisplay =
        autodoc::CommandLine::Get_().DisplayOf_SinceTagValue(
                                        i_rTag.Text().TextOfFirstToken() );
    if (sDisplay.empty())
        return;

    *pTitleOut << "Since ";
    DocuTex2 aHelp;
    aHelp.AddToken(* new csi::dsapi::DT_TextToken(sDisplay));
    PutText_Out(aHelp);
}


//********************      HF_IdlShortDocu     *********************/

HF_IdlShortDocu::HF_IdlShortDocu( Environment &         io_rEnv,
                                  Xml::Element &        o_rOut )
    :   HtmlFactory_Idl( io_rEnv, &o_rOut )
{
}

HF_IdlShortDocu::~HF_IdlShortDocu()
{
}

void
HF_IdlShortDocu::Produce_byData( const ary::idl::CodeEntity & i_rCe )
{
    const ce_info *
        pDocu = Get_IdlDocu(i_rCe.Docu());
    if (pDocu == 0)
        return;

    const ce_info &
        rDocu = *pDocu;
    if ( rDocu.IsDeprecated() )
    {
        CurOut()
            >> *new Html::Bold
                << "[ DEPRECATED ]" << new Html::LineBreak;
    }
    if ( rDocu.IsOptional() )
    {
        CurOut()
            >> *new Html::Bold
                << "[ OPTIONAL ]" << new Html::LineBreak;
    }

    HF_IdlDocuTextDisplay
        aText( Env(), &CurOut(), i_rCe);
    rDocu.Short().DisplayAt(aText);
}


//********************      HF_IdlDocuTextDisplay       *********************/


HF_IdlDocuTextDisplay::HF_IdlDocuTextDisplay( Environment &                 io_rEnv,
                                              Xml::Element *                o_pOut,
                                              const ary::idl::CodeEntity &  i_rScopeGivingCe )
    :   HtmlFactory_Idl(io_rEnv, o_pOut),
        sScope(),
        sLinkToken(),
        bGatherLink(false),
        pScopeGivingCe(&i_rScopeGivingCe)
{
}

HF_IdlDocuTextDisplay::~HF_IdlDocuTextDisplay()
{
}

void
HF_IdlDocuTextDisplay::Display_TextToken( const csi::dsapi::DT_TextToken & i_rToken )
{
    if (bGatherLink)
    {
        if (sLinkToken.length() == 0)
        {
            sLinkToken = i_rToken.GetText();
            return;
        }
        else
        {
            if ( pScopeGivingCe == 0 )
            {   // only in original file
                TheMessages().Out_TypeVsMemberMisuse(sLinkToken, Env().CurPageCe_AsText(), 0);
            }

            StopLinkGathering();
        }
    }   // endif (bGatherLink)

    CurOut() << new Xml::XmlCode( i_rToken.GetText() );
}

void
HF_IdlDocuTextDisplay::Display_White()
{
    CurOut() << " ";
}

void
HF_IdlDocuTextDisplay::Display_MupType( const csi::dsapi::DT_MupType & i_rToken )
{
    if (i_rToken.IsBegin())
    {
        StartLinkGathering(i_rToken.Scope());
    }
    else
    {
        if (bGatherLink)
        {
            CreateTypeLink();
            StopLinkGathering();
        }
    }
}

void
HF_IdlDocuTextDisplay::Display_MupMember( const csi::dsapi::DT_MupMember & i_rToken )
{
    if (i_rToken.IsBegin())
    {
        StartLinkGathering(i_rToken.Scope());
    }
    else
    {
        if (bGatherLink)
        {
            CreateMemberLink();
            StopLinkGathering();
        }
    }
}

void
HF_IdlDocuTextDisplay::Display_MupConst( const csi::dsapi::DT_MupConst & i_rToken )
{
    CurOut()
        >> *new Html::Bold
           << i_rToken.GetText();
}

void
HF_IdlDocuTextDisplay::Display_Style( const csi::dsapi::DT_Style & i_rToken )
{
    CurOut() << new Xml::XmlCode( i_rToken.GetText() );
}

void
HF_IdlDocuTextDisplay::Display_EOL()
{
    CurOut() << "\n";
}

void
HF_IdlDocuTextDisplay::CreateTypeLink()
{
    if (strchr(sLinkToken,':') != 0)
    {
        TheMessages().Out_TypeVsMemberMisuse(sLinkToken, Env().CurPageCe_AsFile(".idl"), 0);
        CurOut() << sLinkToken;
        return;
    }
    HF_IdlTypeText aLink(Env(), CurOut(), &ScopeGivingCe());
    aLink.Produce_LinkInDocu(sScope, sLinkToken, String::Null_());
}

void
HF_IdlDocuTextDisplay::CreateMemberLink()
{

    HF_IdlTypeText aLink(Env(), CurOut(), &ScopeGivingCe());

    const char *
        sSplit = strchr(sLinkToken,':');

    if (sSplit != 0)
    {
        String sCe(sLinkToken.c_str(), sSplit - sLinkToken.c_str());
        String sMember(sSplit+2);

        if (NOT sScope.empty() OR ScopeGivingCe().LocalName() != sCe )
            aLink.Produce_LinkInDocu(sScope, sCe, sMember);
        else
            aLink.Produce_LocalLinkInDocu(sMember);
    }
    else
    {
        aLink.Produce_LocalLinkInDocu(sLinkToken);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
