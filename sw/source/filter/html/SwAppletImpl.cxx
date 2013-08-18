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
#include <com/sun/star/embed/EmbedStates.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svtools/embedhlp.hxx>

using namespace com::sun::star;

namespace {

static char const sHTML_O_archive[] = "ARCHIVE";
static char const sHTML_O_Archives[] = "ARCHIVES";
static char const sHTML_O_Object[] = "OBJECT";

}

sal_uInt16 SwApplet_Impl::GetOptionType( const OUString& rName, sal_Bool bApplet )
{
    sal_uInt16 nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.toChar() )
    {
    case 'A':
    case 'a':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_align ) ||
            rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.equalsIgnoreAsciiCaseAscii( sHTML_O_archive ) ||
                 rName.equalsIgnoreAsciiCaseAscii( sHTML_O_Archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_class ) ||
            (bApplet && (rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_code ) ||
                         rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_height ) )
            nType = SWHTML_OPTTYPE_SIZE;
        else if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_hspace ) ||
            (!bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SW_HTML_O_Hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.equalsIgnoreAsciiCaseAscii( sHTML_O_Object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_style ) ||
            (!bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_O_width ) )
            nType = SWHTML_OPTTYPE_SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
{
}

void SwApplet_Impl::CreateApplet( const OUString& rCode, const OUString& rName,
                                  sal_Bool bMayScript, const OUString& rCodeBase,
                                  const OUString& rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    OUString aName;

    // create Applet; it will be in running state
    xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    ::svt::EmbeddedObjectRef::TryRunningState( xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    OUString sDocBase = aUrlBase.GetMainURL(INetURLObject::NO_DECODE);
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue("AppletCode", uno::makeAny( rCode ) );
        xSet->setPropertyValue("AppletName", uno::makeAny( rName ) );
        xSet->setPropertyValue("AppletIsScript", uno::makeAny( sal_Bool(bMayScript) ) );
        xSet->setPropertyValue("AppletDocBase", uno::makeAny( sDocBase ) );
        if ( !rCodeBase.isEmpty() )
            xSet->setPropertyValue("AppletCodeBase", uno::makeAny( rCodeBase ) );
        else
            xSet->setPropertyValue("AppletCodeBase", uno::makeAny( sDocBase ) );
    }
}
#if HAVE_FEATURE_JAVA
sal_Bool SwApplet_Impl::CreateApplet( const OUString& rBaseURL )
{
    OUString aCode, aName, aCodeBase;
    sal_Bool bMayScript = sal_False;

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
            bMayScript = sal_True;
    }

    if( aCode.isEmpty() )
        return sal_False;
    CreateApplet( aCode, aName, bMayScript, aCodeBase, rBaseURL );
    return sal_True;
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
