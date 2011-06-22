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
#include <s2_dsapi/docu_pe2.hxx>


// NOT FULLY DEFINED SERVICES
#include <cctype>
#include <ary/doc/d_oldidldocu.hxx>
#include <ary_i/d_token.hxx>
#include <parser/parserinfo.hxx>
#include <adc_cl.hxx>
#include <adc_msg.hxx>
#include <../parser/inc/x_docu.hxx>
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


SapiDocu_PE::SapiDocu_PE(ParserInfo & io_rPositionInfo)
    :   pDocu(0),
        eState(e_none),
        pPositionInfo(&io_rPositionInfo),
        fCurTokenAddFunction(&SapiDocu_PE::AddDocuToken2Void),
        pCurAtTag(0),
        sCurDimAttribute(),
        sCurAtSeeType_byXML(200)
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
        pDocu = new ary::doc::OldIdlDocu;
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
        csv_assert(eState == st_attags);
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
        fCurTokenAddFunction = &SapiDocu_PE::SetCurSinceAtTagVersion_OOo;
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
        if ( strcmp("<TRUE/>",i_rToken.Text()) != 0 )
            TheMessages().Out_InvalidConstSymbol( i_rToken.Text(),
                                              pPositionInfo->CurFile(),
                                              pPositionInfo->CurLine() );
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>true</b>") );
        return;
    }
    else if ( strnicmp("<false",i_rToken.Text(),6 ) == 0 )
    {
        if ( strcmp("<FALSE/>",i_rToken.Text()) != 0 )
            TheMessages().Out_InvalidConstSymbol( i_rToken.Text(),
                                              pPositionInfo->CurFile(),
                                              pPositionInfo->CurLine() );
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>false</b>") );
        return;
    }
    else if ( strnicmp("<NULL",i_rToken.Text(),5 ) == 0 )
    {
        if ( strcmp("<NULL/>",i_rToken.Text()) != 0 )
            TheMessages().Out_InvalidConstSymbol( i_rToken.Text(),
                                              pPositionInfo->CurFile(),
                                              pPositionInfo->CurLine() );
        (this->*fCurTokenAddFunction)( *new DT_TextToken("<b>null</b>") );
        return;
    }
    else if ( strnicmp("<void",i_rToken.Text(),5 ) == 0 )
    {
        if ( strcmp("<void/>",i_rToken.Text()) != 0 )
            TheMessages().Out_InvalidConstSymbol( i_rToken.Text(),
                                              pPositionInfo->CurFile(),
                                              pPositionInfo->CurLine() );
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
        default:
            //  Do nothing.
            ;
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
        default:
            //  Do nothing.
            ;
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
        default:
            //  Do nothing.
            ;
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
        default:
            //  Do nothing.
            ;
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

void
SapiDocu_PE::Process_White()
{
    (this->*fCurTokenAddFunction)(*new DT_White);
}

DYN ary::doc::OldIdlDocu *
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
SapiDocu_PE::AddDocuToken2Void( DYN ary::inf::DocuToken & let_drNewToken )
{
    delete &let_drNewToken;
}

void
SapiDocu_PE::AddDocuToken2Short( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2Short(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2Description( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2Description(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2Deprecated( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pDocu);
    pDocu->AddToken2DeprecatedText(let_drNewToken);
}

void
SapiDocu_PE::AddDocuToken2CurAtTag( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);
    pCurAtTag->AddToken(let_drNewToken);
}

void
SapiDocu_PE::SetCurParameterAtTagName( DYN ary::inf::DocuToken & let_drNewToken )
{
    if (let_drNewToken.IsWhiteOnly())
    {
        delete &let_drNewToken;
        return;
    }

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
SapiDocu_PE::SetCurSeeAlsoAtTagLinkText( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    if (let_drNewToken.IsWhiteOnly())
    {
        delete &let_drNewToken;
        return;
    }

    DT_TextToken * pText = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pText != 0)
        pCurAtTag->SetName(pText->GetText());
    else
    {
        DT_MupType *
            pTypeBegin = dynamic_cast< DT_MupType* >(&let_drNewToken);
        DT_MupMember *
            pMemberBegin = dynamic_cast< DT_MupMember* >(&let_drNewToken);
        if (pTypeBegin != 0 OR pMemberBegin != 0)
        {
            sCurAtSeeType_byXML.reset();

            sCurAtSeeType_byXML
                << ( pTypeBegin != 0
                        ?   pTypeBegin->Scope()
                        :   pMemberBegin->Scope() );

            if (sCurAtSeeType_byXML.tellp() > 0)
            {
                sCurAtSeeType_byXML
                    << "::";
            }
            delete &let_drNewToken;
            fCurTokenAddFunction = &SapiDocu_PE::SetCurSeeAlsoAtTagLinkText_2;
            return;
        }
        else
        {
            pCurAtTag->SetName("? (no identifier found)");
        }
    }
    delete &let_drNewToken;
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
}

void
SapiDocu_PE::SetCurSeeAlsoAtTagLinkText_2( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    if (let_drNewToken.IsWhiteOnly())
    {
        delete &let_drNewToken;
        return;
    }

    DT_TextToken *
        pText = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pText != 0)
    {
        sCurAtSeeType_byXML
            << pText->GetText();
        pCurAtTag->SetName(sCurAtSeeType_byXML.c_str());
    }
    else
    {
        pCurAtTag->SetName("? (no identifier found)");
    }
    sCurAtSeeType_byXML.reset();
    delete &let_drNewToken;
    fCurTokenAddFunction = &SapiDocu_PE::SetCurSeeAlsoAtTagLinkText_3;
}

void
SapiDocu_PE::SetCurSeeAlsoAtTagLinkText_3( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    if (let_drNewToken.IsWhiteOnly())
    {
        delete &let_drNewToken;
        return;
    }

    /// Could emit warning, but don't because this parser is obsolete.
//  Tok_XmlLink_BeginTag *
//      pLinkEnd = dynamic_cast< Tok_XmlLink_EndTag* >(&let_drNewToken);
//  if (pLinkEnd == 0)
//  {
//      warn_aboutMissingClosingTag();
//  }

    delete &let_drNewToken;
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2CurAtTag;
}

const String
    C_sSinceFormat("Correct version format: \"OOo <major>.<minor>[.<micro> if micro is not 0]\".");

void
SapiDocu_PE::SetCurSinceAtTagVersion_OOo( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    DT_TextToken * pToken = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pToken == 0)
    {
        delete &let_drNewToken;
        return;
    }

    const String
        sVersion(pToken->GetText());
    if (NOT CheckVersionSyntax_OOo(sVersion))
    {
        Cerr() << "Version information in @since tag has incorrect format.\n"
               << "Found: \"" << sVersion << "\"\n"
               << C_sSinceFormat
               << Endl();
        exit(1);
    }

    const autodoc::CommandLine &
        rCommandLine = autodoc::CommandLine::Get_();
    if (NOT rCommandLine.DoesTransform_SinceTag())
        pCurAtTag->AddToken(let_drNewToken);

    fCurTokenAddFunction = &SapiDocu_PE::SetCurSinceAtTagVersion_Number;
}

void
SapiDocu_PE::SetCurSinceAtTagVersion_Number( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);

    DT_TextToken * pToken = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pToken == 0)
    {
        if (dynamic_cast< DT_White* >(&let_drNewToken) != 0)
        {
            String &
                sValue = pCurAtTag->Access_Text().Access_TextOfFirstToken();
            StreamLock
                sHelp(1000);
            sValue = sHelp() << sValue << " " << c_str;
        }

        delete &let_drNewToken;
        return;
    }

    const String
        sVersion(pToken->GetText());
    if (NOT CheckVersionSyntax_Number(sVersion))
    {
        Cerr() << "Version information in @since tag has incorrect format.\n"
               << "Found: \"" << sVersion << "\"\n"
               << C_sSinceFormat
               << Endl();
        exit(1);
    }

    const autodoc::CommandLine &
        rCommandLine = autodoc::CommandLine::Get_();
    if ( rCommandLine.DoesTransform_SinceTag())
    {
        pCurAtTag->AddToken(let_drNewToken);

        if (rCommandLine.DisplayOf_SinceTagValue(sVersion).empty())
        {
            // This is the numbered part, but we don't know it.
            delete &let_drNewToken;

            StreamLock
                sl(200);
            sl()
                << "Since-value '"
                << sVersion
                << "' not found in translation table.";
            throw X_Docu("since", sl().c_str());
        }
    }
    else
    {
        AddDocuToken2SinceAtTag(let_drNewToken);
    }
    fCurTokenAddFunction = &SapiDocu_PE::AddDocuToken2SinceAtTag;
}

void
SapiDocu_PE::AddDocuToken2SinceAtTag( DYN ary::inf::DocuToken & let_drNewToken )
{
    csv_assert(pCurAtTag);
    String &
        sValue = pCurAtTag->Access_Text().Access_TextOfFirstToken();
    StreamLock
        sHelp(1000);

    DT_TextToken *
        pToken = dynamic_cast< DT_TextToken* >(&let_drNewToken);
    if (pToken != 0)
    {
        sValue = sHelp() << sValue << pToken->GetText() << c_str;
    }
    else if (dynamic_cast< DT_White* >(&let_drNewToken) != 0)
    {
        sValue = sHelp() << sValue << " " << c_str;
    }
      delete &let_drNewToken;
}

bool
SapiDocu_PE::CheckVersionSyntax_OOo(const String & i_versionPart1)
{
    return      i_versionPart1 == "OOo"
            OR  i_versionPart1 == "OpenOffice.org";
}

bool
SapiDocu_PE::CheckVersionSyntax_Number(const String & i_versionPart2)
{
    if (i_versionPart2.length () == 0)
        return false;

    const char
        pt = '.';
    unsigned int countDigit = 0;
    unsigned int countPoint = 0;
    const char *
        pFirstPoint = 0;
    const char *
        pLastPoint = 0;

    for ( const char * p = i_versionPart2.begin();
          *p != 0;
          ++p )
    {
        if ( std::isdigit(*p) )
            ++countDigit;
        else if (*p == pt)
        {
            if (countPoint == 0)
                pFirstPoint = p;
            pLastPoint = p;
            ++countPoint;
        }
    }

    if (    countDigit + countPoint == i_versionPart2.length()         // only digits and points
        AND pFirstPoint != 0 AND countPoint < 3                         // 1 or 2 points
        AND pFirstPoint + 1 != pLastPoint                               // there are digits between two points
        AND *i_versionPart2.begin() != pt AND *(pLastPoint + 1) != 0    // points are surrounded by digits
        AND (*(pLastPoint + 1) != '0' OR pLastPoint == pFirstPoint) )   // the first micro-digit is not 0
    {
        return true;
    }
    return false;
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
        default:
            //  See below.
            ;
    }
    return i_rToken.Text();
}



}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
