/*************************************************************************
 *
 *  $RCSfile: docu_pe2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:43:23 $
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
#include <s2_dsapi/docu_pe2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/codeinf2.hxx>
#include <ary_i/d_token.hxx>
#include <s2_dsapi/dsapitok.hxx>
#include <s2_dsapi/tk_atag2.hxx>
#include <s2_dsapi/tk_html.hxx>
#include <s2_dsapi/tk_docw2.hxx>
#include <s2_dsapi/tk_xml.hxx>


#ifdef UNX
#define strnicmp strncasecmp
#endif


namespace csi
{
namespace dsapi
{


const char *        AtTagTitle(
                        const Tok_AtTag &   i_rToken );


SapiDocu_PE::SapiDocu_PE()
    :   pDocu(0),
        eState(e_none),
        fCurTokenAddFunction(&SapiDocu_PE::AddDocuToken2Void),
        pCurAtTag(0)
{
}

SapiDocu_PE::~SapiDocu_PE()
{
}

void
SapiDocu_PE::ProcessToken( DYN csi::dsapi::Token & let_drToken )
{
    if (IsComplete())
    {
        pDocu = 0;
        eState = e_none;
    }

    if ( eState == e_none )
    {
        pDocu = new ary::info::CodeInformation;
        eState = st_short;
        fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2Short;
    }

    csv_assert(pDocu);

    let_drToken.Trigger(*this);
    delete &let_drToken;
}

void
SapiDocu_PE::Process_AtTag( const Tok_AtTag & i_rToken )
{
    if (NOT pCurAtTag)
    {
        eState = st_attags;
        fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
    }
    else
    {
        csv_assert(eState = st_attags);
        pDocu->AddAtTag(*pCurAtTag.Release());
    }

    if (i_rToken.Id() == Tok_AtTag::param)
    {
        pCurAtTag = new DT_ParameterAtTag;
        fCurTokenAddFunction = &SapiDocu_PE::SetCurParameterAtTagName;
    }
    else if (i_rToken.Id() == Tok_AtTag::see)
    {
        pCurAtTag = new DT_SeeAlsoAtTag;
        fCurTokenAddFunction = &SapiDocu_PE::SetCurSeeAlsoAtTagLinkText;
    }
    else if (i_rToken.Id() == Tok_AtTag::deprecated)
    {
        pDocu->SetDeprecated();
        pCurAtTag = new DT_StdAtTag("");    // Dummy that will not be used.
        fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2Deprecated;
    }
    else if (i_rToken.Id() == Tok_AtTag::since)
    {
        pCurAtTag = new DT_SinceAtTag;
        fCurTokenAddFunction = &SapiDocu_PE::SetCurSinceAtTagVersion;
    }
    else
    {
        pCurAtTag = new DT_StdAtTag( AtTagTitle(i_rToken) );
        fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
    }
}

void
SapiDocu_PE::Process_HtmlTag( const Tok_HtmlTag & i_rToken )
{
    if (eState == st_short AND i_rToken.IsParagraphStarter())
    {
        eState = st_description;
        fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2Description;
    }

    // Workaround special for some errors in API docu:
    if ( strnicmp("<true",i_rToken.Text(),5 ) == 0 )
    {
        Cerr() << "Invalid const symbol: " << i_rToken.Text() << Endl();
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>true</b>") );
        return;
    }
    else if ( strnicmp("<false",i_rToken.Text(),6 ) == 0 )
    {
        Cerr() << "Invalid const symbol: " << i_rToken.Text() << Endl();
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>false</b>") );
        return;
    }
    else if ( strnicmp("<NULL",i_rToken.Text(),5 ) == 0 )
    {
        Cerr() << "Invalid const symbol: " << i_rToken.Text() << Endl();
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>null</b>") );
        return;
    }
    else if ( strnicmp("<void",i_rToken.Text(),5 ) == 0 )
    {
        Cerr() << "Invalid const symbol: " << i_rToken.Text() << Endl();
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>void</b>") );
        return;
    }

    (this->*fCurTokenAddFunction)( *new DT_Style(i_rToken.Text(),false) );
}

void
SapiDocu_PE::Process_XmlConst( const Tok_XmlConst & i_rToken )
{
    (this->*fCurTokenAddFunction)(*new DT_MupConst(i_rToken.Text()));
}

void
SapiDocu_PE::Process_XmlLink_BeginTag( const Tok_XmlLink_BeginTag & i_rToken )
{
    switch (i_rToken.Id())
    {
        case Tok_XmlLink_Tag::e_const:
            (this->*fCurTokenAddFunction)(*new DT_Style("<b>",false));
            break;
        case Tok_XmlLink_Tag::member:
            (this->*fCurTokenAddFunction)(*new DT_MupMember(i_rToken.Scope()));
            break;
        case Tok_XmlLink_Tag::type:
            (this->*fCurTokenAddFunction)(*new DT_MupType(i_rToken.Scope()));
            break;
        // default:
        //          Do nothing.
    }

    if ( i_rToken.Dim().length() > 0 )
        sCurDimAttribute = i_rToken.Dim();
    else
        sCurDimAttribute.clear();
}

void
SapiDocu_PE::Process_XmlLink_EndTag( const Tok_XmlLink_EndTag & i_rToken )
{
    switch (i_rToken.Id())
    {
        case Tok_XmlLink_Tag::e_const:
            (this->*fCurTokenAddFunction)(*new DT_Style("</b>",false));
            break;
        case Tok_XmlLink_Tag::member:
            (this->*fCurTokenAddFunction)(*new DT_MupMember(true));
            break;
        case Tok_XmlLink_Tag::type:
            (this->*fCurTokenAddFunction)(*new DT_MupType(true));
            break;
        // default:
        //          Do nothing.
    }
    if ( sCurDimAttribute.length() > 0 )
    {
        (this->*fCurTokenAddFunction)( *new DT_TextToken(sCurDimAttribute.c_str()) );
        sCurDimAttribute.clear();
    }
}

void
SapiDocu_PE::Process_XmlFormat_BeginTag( const Tok_XmlFormat_BeginTag & i_rToken )
{
    switch (i_rToken.Id())
    {
        case Tok_XmlFormat_Tag::code:
            (this->*fCurTokenAddFunction)(*new DT_Style("<code>",false));
            break;
        case Tok_XmlFormat_Tag::listing:
            (this->*fCurTokenAddFunction)(*new DT_Style("<pre>",true));
            break;
        case Tok_XmlFormat_Tag::atom:
            (this->*fCurTokenAddFunction)(*new DT_Style("<code>",true));
            break;
        // default:
        //          Do nothing.
    }
    if ( i_rToken.Dim().length() > 0 )
        sCurDimAttribute = i_rToken.Dim();
    else
        sCurDimAttribute.clear();
}

void
SapiDocu_PE::Process_XmlFormat_EndTag( const Tok_XmlFormat_EndTag & i_rToken )
{
    switch (i_rToken.Id())
    {
        case Tok_XmlFormat_Tag::code:
            (this->*fCurTokenAddFunction)(*new DT_Style("</code>",false));
            break;
        case Tok_XmlFormat_Tag::listing:
            (this->*fCurTokenAddFunction)(*new DT_Style("</pre>",true));
            break;
        case Tok_XmlFormat_Tag::atom:
            (this->*fCurTokenAddFunction)(*new DT_Style("</code>",true));
            break;
        // default:
        //          Do nothing.
    }
    if ( sCurDimAttribute.length() > 0 )
    {
        (this->*fCurTokenAddFunction)( *new DT_TextToken(sCurDimAttribute.c_str()) );
        sCurDimAttribute.clear();
    }
}

void
SapiDocu_PE::Process_Word( const Tok_Word & i_rToken )
{
    (this->*fCurTokenAddFunction)(*new DT_TextToken(i_rToken.Text()));
}

void
SapiDocu_PE::Process_Comma()
{
    csv_assert(1==7);
//  (this->*fCurTokenAddFunction)(*new DT_Comma(i_rToken.Text()));
}

void
SapiDocu_PE::Process_DocuEnd()
{
    eState = st_complete;
    if (pCurAtTag)
        pDocu->AddAtTag(*pCurAtTag.Release());
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2Void;
}

void
SapiDocu_PE::Process_EOL()
{
    (this->*fCurTokenAddFunction)(*new DT_EOL);
}


DYN ary::info::CodeInformation *
SapiDocu_PE::ReleaseJustParsedDocu()
{
    if (IsComplete())
    {
        eState = e_none;
        return pDocu.Release();
    }
    return 0;
}


bool
SapiDocu_PE::IsComplete() const
{
    return eState == st_complete;
}

void
SapiDocu_PE::AddDocuToken2Void( DYN ary::info::DocuToken & let_drNewToken )
{
    delete &let_drNewToken;
}

void
SapiDocu_PE::AddDocuToken2Short( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2Short(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2Description( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2Description(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2Deprecated( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2DeprecatedText(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2CurAtTag( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);
    pCurAtTag->AddToken(let_drNewToken);
}

void
SapiDocu_PE::SetCurParameterAtTagName( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);
    DT_TextToken * dpText = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (dpText != 0)
        pCurAtTag->SetName(dpText->GetText());
    else
        pCurAtTag->SetName("parameter ?");
    delete &let_drNewToken;
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
}

void
SapiDocu_PE::SetCurSeeAlsoAtTagLinkText( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);
    DT_TextToken * dpText = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (dpText != 0)
        pCurAtTag->SetName(dpText->GetText());
    else
        pCurAtTag->SetName("unknown ?");
    delete &let_drNewToken;
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
}


void
SapiDocu_PE::SetCurSinceAtTagVersion( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    DT_TextToken * pToken = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pToken == 0)
    {
        delete &let_drNewToken;
        return;
    }

    char cFirst = *pToken->GetText();
    const char cCiphersend = '9' + 1;
    if ( NOT csv::in_range('0', cFirst, cCiphersend) )
    {
        delete &let_drNewToken;
        return;
    }

    pCurAtTag->AddToken(let_drNewToken);
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2SinceAtTag;
}

void
SapiDocu_PE::AddDocuToken2SinceAtTag( DYN ary::info::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    DT_TextToken * pToken = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pToken != 0)
    {
        String & sValue = pCurAtTag->Access_Text().Access_TextOfFirstToken();
        StreamLock sHelp(100);
        sValue = sHelp() << sValue << pToken->GetText() << c_str;
    }

      delete &let_drNewToken;
}

const char *
AtTagTitle( const Tok_AtTag & i_rToken )
{
    switch (i_rToken.Id())
    {
        case Tok_AtTag::author:     return "";
        case Tok_AtTag::see:        return "See also";
        case Tok_AtTag::param:      return "Parameters";
        case Tok_AtTag::e_return:   return "Returns";
        case Tok_AtTag::e_throw:    return "Throws";
        case Tok_AtTag::example:    return "Example";
        case Tok_AtTag::deprecated: return "Deprecated";
        case Tok_AtTag::suspicious: return "";
        case Tok_AtTag::missing:    return "";
        case Tok_AtTag::incomplete: return "";
        case Tok_AtTag::version:    return "";
        case Tok_AtTag::guarantees: return "Guarantees";
        case Tok_AtTag::exception:  return "Exception";
        case Tok_AtTag::since:      return "Since version";
    }
    return i_rToken.Text();
}



}   // namespace dsapi
}   // namespace csi

