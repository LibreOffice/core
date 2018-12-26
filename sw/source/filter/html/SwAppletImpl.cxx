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
#include <svl/urihelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svtools/embedhlp.hxx>
#include <tools/globname.hxx>
#include <hintids.hxx>

using namespace com::sun::star;

SwHtmlOptType SwApplet_Impl::GetOptionType( const OUString& rName, bool bApplet )
{
    SwHtmlOptType nType = bApplet ? SwHtmlOptType::PARAM : SwHtmlOptType::TAG;

    switch( rName.toChar() )
    {
    case 'A':
    case 'a':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_align ) ||
            rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_alt ) )
            nType = SwHtmlOptType::IGNORE;
        else if( bApplet &&
                 (rName == "ARCHIVE" || rName == "ARCHIVES" ) )
            nType = SwHtmlOptType::TAG;
        break;
    case 'C':
    case 'c':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_class ) ||
            (bApplet && (rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_code ) ||
                         rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_codebase ))) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_height ) )
            nType = SwHtmlOptType::SIZE;
        else if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_hspace ) ||
            (!bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SW_HTML_O_Hidden )) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_id ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_name ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName == "OBJECT" )
            nType = SwHtmlOptType::TAG;
        break;
    case 'S':
    case 's':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_style ) ||
            (!bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_src )) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_type ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_vspace ) )
            nType = SwHtmlOptType::IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_width ) )
            nType = SwHtmlOptType::SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool ) :
        aItemSet( rPool, svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} )
{
}

void SwApplet_Impl::CreateApplet( const OUString& rCode, const OUString& rName,
                                  bool bMayScript, const OUString& rCodeBase,
                                  const OUString& rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aName;

    // create Applet; it will be in running state
    xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    ::svt::EmbeddedObjectRef::TryRunningState( xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    OUString sDocBase = aUrlBase.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue("AppletCode", uno::makeAny( rCode ) );
        xSet->setPropertyValue("AppletName", uno::makeAny( rName ) );
        xSet->setPropertyValue("AppletIsScript", uno::makeAny( bMayScript ) );
        xSet->setPropertyValue("AppletDocBase", uno::makeAny( sDocBase ) );
        if ( !rCodeBase.isEmpty() )
            xSet->setPropertyValue("AppletCodeBase", uno::makeAny( rCodeBase ) );
        else
            xSet->setPropertyValue("AppletCodeBase", uno::makeAny( sDocBase ) );
    }
}
#if HAVE_FEATURE_JAVA
bool SwApplet_Impl::CreateApplet( const OUString& rBaseURL )
{
    OUString aCode, aName, aCodeBase;
    bool bMayScript = false;

    size_t nArgCount = aCommandList.size();
    for( size_t i = 0; i < nArgCount; i++ )
    {
        const SvCommand& rArg = aCommandList[i];
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
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        uno::Sequence < beans::PropertyValue > aProps;
        aCommandList.FillSequence( aProps );
        xSet->setPropertyValue("AppletCommands", uno::makeAny( aProps ) );
    }
}

#if HAVE_FEATURE_JAVA
void SwApplet_Impl::AppendParam( const OUString& rName, const OUString& rValue )
{
    aCommandList.Append( rName, rValue );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
