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

#include <string.h>
#include "DAVProperties.hxx"
#include <rtl/ustrbuf.hxx>

using namespace http_dav_ucp;

const OUString DAVProperties::CREATIONDATE =
                OUString::createFromAscii( "DAV:creationdate" );
const OUString DAVProperties::DISPLAYNAME =
                OUString::createFromAscii( "DAV:displayname" );
const OUString DAVProperties::GETCONTENTLANGUAGE =
                OUString::createFromAscii( "DAV:getcontentlanguage" );
const OUString DAVProperties::GETCONTENTLENGTH =
                OUString::createFromAscii( "DAV:getcontentlength" );
const OUString DAVProperties::GETCONTENTTYPE =
                OUString::createFromAscii( "DAV:getcontenttype" );
const OUString DAVProperties::GETETAG =
                OUString::createFromAscii( "DAV:getetag" );
const OUString DAVProperties::GETLASTMODIFIED =
                OUString::createFromAscii( "DAV:getlastmodified" );
const OUString DAVProperties::LOCKDISCOVERY =
                OUString::createFromAscii( "DAV:lockdiscovery" );
const OUString DAVProperties::RESOURCETYPE =
                OUString::createFromAscii( "DAV:resourcetype" );
const OUString DAVProperties::SUPPORTEDLOCK =
                OUString::createFromAscii( "DAV:supportedlock" );

const OUString DAVProperties::EXECUTABLE =
                OUString::createFromAscii(
                                "http://apache.org/dav/props/executable" );

// -------------------------------------------------------------------
// static
void DAVProperties::createSerfPropName( const OUString & rFullName,
                                        SerfPropName & rName )
{
    if ( rFullName.startsWith( "DAV:" ) )
    {
        rName.nspace = "DAV:";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy( RTL_CONSTASCII_LENGTH( "DAV:" ) ),
                                        RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.startsWith( "http://apache.org/dav/props/" ) )
    {
        rName.nspace = "http://apache.org/dav/props/";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://apache.org/dav/props/" ) ),
                            RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.startsWith( "http://ucb.openoffice.org/dav/props/" ) )
    {
        rName.nspace = "http://ucb.openoffice.org/dav/props/";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://ucb.openoffice.org/dav/props/" ) ),
                            RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.startsWith( "<prop:" ) )
    {
        // Support for 3rd party namespaces/props

        OString aFullName
            = OUStringToOString( rFullName, RTL_TEXTENCODING_UTF8 );

        // Format: <prop:the_propname xmlns:prop="the_namespace">

        sal_Int32 nStart = RTL_CONSTASCII_LENGTH( "<prop:" );
        sal_Int32 nLen = aFullName.indexOf( ' ' ) - nStart;
        rName.name = strdup( aFullName.copy( nStart, nLen ) );

        nStart = aFullName.indexOf( '=', nStart + nLen ) + 2; // after ="
        nLen = aFullName.getLength() - RTL_CONSTASCII_LENGTH( "\">" ) - nStart;
        rName.nspace = strdup( aFullName.copy( nStart, nLen ) );
    }
    else
    {
        // Add our namespace to our own properties.
        rName.nspace = "http://ucb.openoffice.org/dav/props/";
        rName.name
            = strdup( OUStringToOString( rFullName,
                                              RTL_TEXTENCODING_UTF8 ) );
    }
}

// -------------------------------------------------------------------
// static
void DAVProperties::createUCBPropName( const char * nspace,
                                       const char * name,
                                       OUString & rFullName )
{
    OUString aNameSpace
        = OStringToOUString( nspace, RTL_TEXTENCODING_UTF8 );
    OUString aName
        = OStringToOUString( name,   RTL_TEXTENCODING_UTF8 );

    if ( !aNameSpace.getLength() )
    {
        // Some servers send XML without proper namespaces. Assume "DAV:"
        // in this case, if name is a well-known dav property name.
        // Although this is not 100% correct, it solves many problems.

        if ( DAVProperties::RESOURCETYPE.matchIgnoreAsciiCase( aName, 4 )  ||
             DAVProperties::SUPPORTEDLOCK.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::LOCKDISCOVERY.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::CREATIONDATE.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::DISPLAYNAME.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::GETCONTENTLANGUAGE.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::GETCONTENTLENGTH.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::GETCONTENTTYPE.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::GETETAG.matchIgnoreAsciiCase( aName, 4 ) ||
             DAVProperties::GETLASTMODIFIED.matchIgnoreAsciiCase( aName, 4 ) )
        {
            aNameSpace = OUString( "DAV:" );
        }
    }

    // Note: Concatenating strings BEFORE comparing against known namespaces
    //       is important. See RFC 2815 ( 23.4.2 Meaning of Qualified Names ).
    rFullName  = aNameSpace;
    rFullName += aName;

    if ( rFullName.startsWith( "DAV:" ) )
    {
        // Okay, Just concat strings.
    }
    else if ( rFullName.startsWith( "http://apache.org/dav/props/" ) )
    {
        // Okay, Just concat strings.
    }
    else if ( rFullName.startsWith( "http://ucb.openoffice.org/dav/props/" ) )
    {
        // Remove namespace from our own properties.
        rFullName = rFullName.copy(
                        RTL_CONSTASCII_LENGTH(
                            "http://ucb.openoffice.org/dav/props/" ) );
    }
    else
    {
        // Create property name that encodes, namespace and name ( XML ).
        rFullName  = OUString::createFromAscii( "<prop:" );
        rFullName += aName;
        rFullName += OUString::createFromAscii( " xmlns:prop=\"" );
        rFullName += aNameSpace;
        rFullName += OUString::createFromAscii( "\">" );
    }
}

// -------------------------------------------------------------------
// static
bool DAVProperties::isUCBDeadProperty( const SerfPropName & rName )
{
    return ( rName.nspace &&
             ( rtl_str_compareIgnoreAsciiCase(
                 rName.nspace, "http://ucb.openoffice.org/dav/props/" )
               == 0 ) );
}

bool DAVProperties::isUCBSpecialProperty(const rtl::OUString& rFullName, rtl::OUString& rParsedName)
{
    sal_Int32 nLen = rFullName.getLength();
    if ( nLen <= 0 ||
            !rFullName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "<prop:" ) ) ||
            !rFullName.endsWithAsciiL( RTL_CONSTASCII_STRINGPARAM( "\">" ) ) )
        return false;

    sal_Int32 nStart = RTL_CONSTASCII_LENGTH( "<prop:" );
    sal_Int32 nEnd = rFullName.indexOf( sal_Unicode( ' ' ), nStart );
    if ( nEnd == -1 )
        return false;

    rtl::OUString sPropName = rFullName.copy( nStart, nEnd - nStart );
    if ( !sPropName.getLength() )
        return false;

    // TODO skip whitespaces?
    if ( !rFullName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "xmlns:prop=\"" ), ++nEnd ) )
        return false;

    nStart = nEnd + RTL_CONSTASCII_LENGTH( "xmlns:prop=\"" );
    nEnd = rFullName.indexOf( sal_Unicode( '"' ), nStart );
    if ( nEnd != nLen - RTL_CONSTASCII_LENGTH( "\">" ) )
        return false;

    rtl::OUString sNamesp = rFullName.copy( nStart, nEnd - nStart );
    if ( !( nLen = sNamesp.getLength() ) )
        return false;

    rtl::OUStringBuffer aBuff( sNamesp );
    if ( sNamesp[nLen - 1] != '/' )
        aBuff.append( sal_Unicode( '/' ) );
    aBuff.append( sPropName );
    rParsedName = aBuff.makeStringAndClear();

    return rParsedName.getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
