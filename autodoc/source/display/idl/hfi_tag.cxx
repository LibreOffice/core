/*************************************************************************
 *
 *  $RCSfile: hfi_tag.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:29:59 $
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
#include "hfi_tag.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary_i/ci_text2.hxx>
#include <ary_i/d_token.hxx>
#include <toolkit/out_tree.hxx>
#include <adc_cl.hxx>
#include "hfi_typetext.hxx"
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include "hi_linkhelper.hxx"


using ary::info::DocuTex2;


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
HF_IdlTag::PutText_Out( const ary::info::DocuTex2 & i_rText ) const
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
                           const ary::info::AtTag2 &   i_rTag ) const
{
    pTitleOut = &o_rTitle;
    Enter_TextOut(o_rText);
    i_rTag.DisplayAt( const_cast< HF_IdlTag& >(*this) );
    Leave_TextOut();
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

    HF_IdlTypeText aLinkText(Env(),aTextOut.CurOut(),true, &aTextOut.ScopeGivingCe());
    aLinkText.Produce_byData( i_rTag.LinkText() );

    aTextOut.CurOut() << new Html::LineBreak;
    PutText_Out( i_rTag.Text() );
}

void
HF_IdlTag::Display_ParameterAtTag( const csi::dsapi::DT_ParameterAtTag & i_rTag )
{
    csv_assert( pTitleOut != 0 );
    *pTitleOut
        << ( StreamLock(100)() << "Parameter " << i_rTag.Title() << c_str );
    PutText_Out( i_rTag.Text() );
}

void
HF_IdlTag::Display_SinceAtTag( const csi::dsapi::DT_SinceAtTag & i_rTag )
{
    csv_assert(pTitleOut != 0);

    if ( i_rTag.Text().IsEmpty()
         OR
         NOT autodoc::CommandLine::Get_().Display_SinceTag() )
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
    if (i_rCe.Docu() == 0)
        return;

    const ce_info &
        rDocu = *i_rCe.Docu();
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
            Cerr() << "Error in documentation: Too many or too few tokens for a link in <member> or <type>." << Endl();
            Cerr() << "  Link won't be created, but all tokens shown plain." << Endl();
            Cerr() << "  \"" << sLinkToken << "\"";
            if ( pScopeGivingCe != 0 )
                Cerr() << " in " << pScopeGivingCe->LocalName();
            Cerr() << Endl();
            StopLinkGathering();
        }
    }   // endif (bGatherLink)

    CurOut() << " " << new Xml::XmlCode( i_rToken.GetText() );
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
            CurOut()
                << " ";
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
            CurOut()
                << " ";
            CreateMemberLink();
            StopLinkGathering();
        }
    }
}

void
HF_IdlDocuTextDisplay::Display_MupConst( const csi::dsapi::DT_MupConst & i_rToken )
{
    CurOut()
        << " "
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
         Cerr() << "Warning: Qualified name (probably member) \""
                << sLinkToken
                << "\" found in <type> tag in "
               << Env().CurPageCe_AsText()
               << Endl();
        CurOut() << sLinkToken;
        return;
    }
    HF_IdlTypeText aLink(Env(), CurOut(), true, &ScopeGivingCe());
    aLink.Produce_LinkInDocu(sScope, sLinkToken, String::Null_());
}

void
HF_IdlDocuTextDisplay::CreateMemberLink()
{

    HF_IdlTypeText aLink(Env(), CurOut(), true, &ScopeGivingCe());

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
