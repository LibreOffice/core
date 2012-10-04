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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>
#include "DAVProperties.hxx"

using namespace http_dav_ucp;

const ::rtl::OUString DAVProperties::CREATIONDATE =
                ::rtl::OUString::createFromAscii( "DAV:creationdate" );
const ::rtl::OUString DAVProperties::DISPLAYNAME =
                ::rtl::OUString::createFromAscii( "DAV:displayname" );
const ::rtl::OUString DAVProperties::GETCONTENTLANGUAGE =
                ::rtl::OUString::createFromAscii( "DAV:getcontentlanguage" );
const ::rtl::OUString DAVProperties::GETCONTENTLENGTH =
                ::rtl::OUString::createFromAscii( "DAV:getcontentlength" );
const ::rtl::OUString DAVProperties::GETCONTENTTYPE =
                ::rtl::OUString::createFromAscii( "DAV:getcontenttype" );
const ::rtl::OUString DAVProperties::GETETAG =
                ::rtl::OUString::createFromAscii( "DAV:getetag" );
const ::rtl::OUString DAVProperties::GETLASTMODIFIED =
                ::rtl::OUString::createFromAscii( "DAV:getlastmodified" );
const ::rtl::OUString DAVProperties::LOCKDISCOVERY =
                ::rtl::OUString::createFromAscii( "DAV:lockdiscovery" );
const ::rtl::OUString DAVProperties::RESOURCETYPE =
                ::rtl::OUString::createFromAscii( "DAV:resourcetype" );
const ::rtl::OUString DAVProperties::SUPPORTEDLOCK =
                ::rtl::OUString::createFromAscii( "DAV:supportedlock" );

const ::rtl::OUString DAVProperties::EXECUTABLE =
                ::rtl::OUString::createFromAscii(
                                "http://apache.org/dav/props/executable" );

// -------------------------------------------------------------------
// static
void DAVProperties::createSerfPropName( const rtl::OUString & rFullName,
                                        SerfPropName & rName )
{
    if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "DAV:" ) ) == 0 )
    {
        rName.nspace = "DAV:";
        rName.name
            = strdup( rtl::OUStringToOString(
                        rFullName.copy( RTL_CONSTASCII_LENGTH( "DAV:" ) ),
                                        RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "http://apache.org/dav/props/" ) ) == 0 )
    {
        rName.nspace = "http://apache.org/dav/props/";
        rName.name
            = strdup( rtl::OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://apache.org/dav/props/" ) ),
                            RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "http://ucb.openoffice.org/dav/props/" ) ) == 0 )
    {
        rName.nspace = "http://ucb.openoffice.org/dav/props/";
        rName.name
            = strdup( rtl::OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://ucb.openoffice.org/dav/props/" ) ),
                            RTL_TEXTENCODING_UTF8 ) );
    }
    else if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "<prop:" ) ) == 0 )
    {
        // Support for 3rd party namespaces/props

        rtl::OString aFullName
            = rtl::OUStringToOString( rFullName, RTL_TEXTENCODING_UTF8 );

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
            = strdup( rtl::OUStringToOString( rFullName,
                                              RTL_TEXTENCODING_UTF8 ) );
    }
}

// -------------------------------------------------------------------
// static
void DAVProperties::createUCBPropName( const char * nspace,
                                       const char * name,
                                       rtl::OUString & rFullName )
{
    rtl::OUString aNameSpace
        = rtl::OStringToOUString( nspace, RTL_TEXTENCODING_UTF8 );
    rtl::OUString aName
        = rtl::OStringToOUString( name,   RTL_TEXTENCODING_UTF8 );

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
            aNameSpace = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DAV:" ) );
        }
    }

    // Note: Concatenating strings BEFORE comparing against known namespaces
    //       is important. See RFC 2815 ( 23.4.2 Meaning of Qualified Names ).
    rFullName  = aNameSpace;
    rFullName += aName;

    if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "DAV:" ) ) == 0 )
    {
        // Okay, Just concat strings.
    }
    else if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "http://apache.org/dav/props/" ) ) == 0 )
    {
        // Okay, Just concat strings.
    }
    else if ( rFullName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(
                "http://ucb.openoffice.org/dav/props/" ) ) == 0 )
    {
        // Remove namespace from our own properties.
        rFullName = rFullName.copy(
                        RTL_CONSTASCII_LENGTH(
                            "http://ucb.openoffice.org/dav/props/" ) );
    }
    else
    {
        // Create property name that encodes, namespace and name ( XML ).
        rFullName  = rtl::OUString::createFromAscii( "<prop:" );
        rFullName += aName;
        rFullName += rtl::OUString::createFromAscii( " xmlns:prop=\"" );
        rFullName += aNameSpace;
        rFullName += rtl::OUString::createFromAscii( "\">" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
