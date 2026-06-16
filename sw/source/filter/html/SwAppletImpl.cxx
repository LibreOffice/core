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

#include <SwAppletImpl.hxx>
#include <svtools/htmlkywd.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <o3tl/string_view.hxx>
#include <svtools/embedhlp.hxx>
#include <tools/globname.hxx>
#include <tools/urlobj.hxx>
#include <hintids.hxx>

using namespace com::sun::star;

SwHtmlOptType SwApplet_Impl::GetOptionType( std::u16string_view rName, bool bApplet )
{
    SwHtmlOptType nType = bApplet ? SwHtmlOptType::PARAM : SwHtmlOptType::TAG;

    switch( rName[0] )
    {
    case 'A':
    case 'a':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_align ) ||
            o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_alt ) )
            nType = SwHtmlOptType::IGNORE;
        else if( bApplet &&
                 (rName == u"ARCHIVE" || rName == u"ARCHIVES" ) )
            nType = SwHtmlOptType::TAG;
        break;
    case 'C':
    case 'c':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_class ) ||
            (bApplet && (o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_code ) ||
                         o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_codebase ))) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'H':
    case 'h':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_height ) )
            nType = SwHtmlOptType::SIZE;
        else if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_hspace ) ||
            (!bApplet && o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SW_HTML_O_Hidden )) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'I':
    case 'i':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_id ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'N':
    case 'n':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_name ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName == u"OBJECT" )
            nType = SwHtmlOptType::TAG;
        break;
    case 'S':
    case 's':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_style ) ||
            (!bApplet && o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_src )) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_type ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'V':
    case 'v':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_vspace ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'W':
    case 'w':
        if( o3tl::equalsIgnoreAsciiCase( rName, OOO_STRING_SVTOOLS_HTML_O_width ) )
            nType = SwHtmlOptType::SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool ) :
        m_aItemSet( rPool, svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1> )
{
}

void SwApplet_Impl::CreateApplet( const OUString& rCode, const OUString& rName,
                                  bool bMayScript, const OUString& rCodeBase,
                                  std::u16string_view rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aName;

    // create Applet; it will be in running state
    m_xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    (void)::svt::EmbeddedObjectRef::TryRunningState( m_xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    OUString sDocBase = aUrlBase.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    uno::Reference < beans::XPropertySet > xSet( m_xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue(u"AppletCode"_ustr, uno::Any( rCode ) );
        xSet->setPropertyValue(u"AppletName"_ustr, uno::Any( rName ) );
        xSet->setPropertyValue(u"AppletIsScript"_ustr, uno::Any( bMayScript ) );
        xSet->setPropertyValue(u"AppletDocBase"_ustr, uno::Any( sDocBase ) );
        if ( !rCodeBase.isEmpty() )
            xSet->setPropertyValue(u"AppletCodeBase"_ustr, uno::Any( rCodeBase ) );
        else
            xSet->setPropertyValue(u"AppletCodeBase"_ustr, uno::Any( sDocBase ) );
    }
}
#if HAVE_FEATURE_JAVA
bool SwApplet_Impl::CreateApplet( std::u16string_view rBaseURL )
{
    OUString aCode, aName, aCodeBase;
    bool bMayScript = false;

    size_t nArgCount = m_aCommandList.size();
    for( size_t i = 0; i < nArgCount; i++ )
    {
        const SvCommand& rArg = m_aCommandList[i];
        const OUString& rName = rArg.GetCommand();
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_code ) )
            aCode = rArg.GetArgument();
        else if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_codebase ) )
            aCodeBase = INetURLObject::GetAbsURL( rBaseURL, rArg.GetArgument() );
        else if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_name ) )
            aName = rArg.GetArgument();
        else if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            bMayScript = true;
    }

    if( aCode.isEmpty() )
        return false;
    CreateApplet( aCode, aName, bMayScript, aCodeBase, rBaseURL );
    return true;
}
#endif

SwApplet_Impl::~SwApplet_Impl()
{
}
void SwApplet_Impl::FinishApplet()
{
    uno::Reference < beans::XPropertySet > xSet( m_xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        uno::Sequence < beans::PropertyValue > aProps;
        m_aCommandList.FillSequence( aProps );
        xSet->setPropertyValue(u"AppletCommands"_ustr, uno::Any( aProps ) );
    }
}

#if HAVE_FEATURE_JAVA
void SwApplet_Impl::AppendParam( const OUString& rName, const OUString& rValue )
{
    m_aCommandList.Append( rName, rValue );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
