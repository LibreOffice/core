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

#include <SwAppletImpl.hxx>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <sot/clsids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svtools/embedhlp.hxx>

using namespace com::sun::star;

namespace {

static char const sHTML_O_archive[] = "ARCHIVE";
static char const sHTML_O_Archives[] = "ARCHIVES";
static char const sHTML_O_Object[] = "OBJECT";

}

sal_uInt16 SwApplet_Impl::GetOptionType( const ::rtl::OUString& rName, sal_Bool bApplet )
{
    sal_uInt16 nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.toChar() )
    {
    case 'A':
    case 'a':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_align ) ||
            rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.equalsIgnoreAsciiCaseAscii( sHTML_O_archive ) ||
                 rName.equalsIgnoreAsciiCaseAscii( sHTML_O_Archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_class ) ||
            (bApplet && (rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_code ) ||
                         rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_height ) )
            nType = SWHTML_OPTTYPE_SIZE;
        else if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_hspace ) ||
            (!bApplet && rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SW_HTML_O_Hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.equalsIgnoreAsciiCaseAscii( sHTML_O_Object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_style ) ||
            (!bApplet && rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_width ) )
            nType = SWHTML_OPTTYPE_SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
{
}

void SwApplet_Impl::CreateApplet( const ::rtl::OUString& rCode, const ::rtl::OUString& rName,
                                  sal_Bool bMayScript, const ::rtl::OUString& rCodeBase,
                                  const ::rtl::OUString& rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    ::rtl::OUString aName;

    // create Applet; it will be in running state
    xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    ::svt::EmbeddedObjectRef::TryRunningState( xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    ::rtl::OUString sDocBase = aUrlBase.GetMainURL(INetURLObject::NO_DECODE);
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCode")), uno::makeAny( rCode ) );
        xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletName")), uno::makeAny( rName ) );
        xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletIsScript")), uno::makeAny( sal_Bool(bMayScript) ) );
        xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletDocBase")), uno::makeAny( sDocBase ) );
        if ( !rCodeBase.isEmpty() )
            xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCodeBase")), uno::makeAny( rCodeBase ) );
        else
            xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCodeBase")), uno::makeAny( sDocBase ) );
    }
}
#ifdef SOLAR_JAVA
sal_Bool SwApplet_Impl::CreateApplet( const ::rtl::OUString& rBaseURL )
{
    ::rtl::OUString aCode, aName, aCodeBase;
    sal_Bool bMayScript = sal_False;

    size_t nArgCount = aCommandList.size();
    for( size_t i = 0; i < nArgCount; i++ )
    {
        const SvCommand& rArg = aCommandList[i];
        const ::rtl::OUString& rName = rArg.GetCommand();
        if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_code ) )
            aCode = rArg.GetArgument();
        else if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_codebase ) )
            aCodeBase = INetURLObject::GetAbsURL( rBaseURL, rArg.GetArgument() );
        else if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_name ) )
            aName = rArg.GetArgument();
        else if( rName.equalsIgnoreAsciiCaseAscii( OOO_STRING_SVTOOLS_HTML_O_mayscript ) )
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
        xSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AppletCommands")), uno::makeAny( aProps ) );
    }
}

#ifdef SOLAR_JAVA
void SwApplet_Impl::AppendParam( const ::rtl::OUString& rName, const ::rtl::OUString& rValue )
{
    aCommandList.Append( rName, rValue );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
