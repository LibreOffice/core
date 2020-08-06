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


#include <string.h>
#include "DAVProperties.hxx"

using namespace webdav_ucp;

const OUStringLiteral DAVProperties::CREATIONDATE("DAV:creationdate");
const OUStringLiteral DAVProperties::DISPLAYNAME("DAV:displayname");
const OUStringLiteral DAVProperties::GETCONTENTLANGUAGE("DAV:getcontentlanguage");
const OUStringLiteral DAVProperties::GETCONTENTLENGTH("DAV:getcontentlength");
const OUStringLiteral DAVProperties::GETCONTENTTYPE("DAV:getcontenttype");
const OUStringLiteral DAVProperties::GETETAG("DAV:getetag");
const OUStringLiteral DAVProperties::GETLASTMODIFIED("DAV:getlastmodified");
const OUStringLiteral DAVProperties::LOCKDISCOVERY("DAV:lockdiscovery");
const OUStringLiteral DAVProperties::RESOURCETYPE("DAV:resourcetype");
const OUStringLiteral DAVProperties::SOURCE("DAV:source");
const OUStringLiteral DAVProperties::SUPPORTEDLOCK("DAV:supportedlock");
const OUStringLiteral DAVProperties::EXECUTABLE("http://apache.org/dav/props/executable");

void DAVProperties::createNeonPropName( const OUString & rFullName,
                                        NeonPropName & rName )
{
    if ( rFullName.startsWith( "DAV:" ) )
    {
        rName.nspace = "DAV:";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy( RTL_CONSTASCII_LENGTH( "DAV:" ) ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    else if ( rFullName.startsWith( "http://apache.org/dav/props/" ) )
    {
        rName.nspace = "http://apache.org/dav/props/";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://apache.org/dav/props/" ) ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    else if ( rFullName.startsWith( "http://ucb.openoffice.org/dav/props/" ) )
    {
        rName.nspace = "http://ucb.openoffice.org/dav/props/";
        rName.name
            = strdup( OUStringToOString(
                        rFullName.copy(
                            RTL_CONSTASCII_LENGTH(
                                "http://ucb.openoffice.org/dav/props/" ) ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }
    else if ( rFullName.startsWith( "<prop:" ) )
    {
        // Support for 3rd party namespaces/props

        OString aFullName
            = OUStringToOString( rFullName, RTL_TEXTENCODING_UTF8 );

        // Format: <prop:the_propname xmlns:prop="the_namespace">

        sal_Int32 nStart = RTL_CONSTASCII_LENGTH( "<prop:" );
        sal_Int32 nLen = aFullName.indexOf( ' ' ) - nStart;
        rName.name = strdup( aFullName.copy( nStart, nLen ).getStr() );

        nStart = aFullName.indexOf( '=', nStart + nLen ) + 2; // after ="
        nLen = aFullName.getLength() - RTL_CONSTASCII_LENGTH( "\">" ) - nStart;
        rName.nspace = strdup( aFullName.copy( nStart, nLen ).getStr() );
    }
    else
    {
        // Add our namespace to our own properties.
        rName.nspace = "http://ucb.openoffice.org/dav/props/";
        rName.name
            = strdup( OUStringToOString( rFullName,
                                              RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

void DAVProperties::createUCBPropName( const char * nspace,
                                       const char * name,
                                       OUString & rFullName )
{
    OUString aNameSpace
        = OStringToOUString( nspace, RTL_TEXTENCODING_UTF8 );
    OUString aName
        = OStringToOUString( name,   RTL_TEXTENCODING_UTF8 );

    if ( aNameSpace.isEmpty() )
    {
        // Some servers send XML without proper namespaces. Assume "DAV:"
        // in this case, if name is a well-known dav property name.
        // Although this is not 100% correct, it solves many problems.
        if ( aName.equalsIgnoreAsciiCaseAscii(DAVProperties::RESOURCETYPE.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::RESOURCETYPE.data + 4)  ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::SUPPORTEDLOCK.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::LOCKDISCOVERY.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::CREATIONDATE.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::DISPLAYNAME.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::GETCONTENTLANGUAGE.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::GETCONTENTLENGTH.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::GETCONTENTTYPE.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::GETETAG.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::GETLASTMODIFIED.data + 4) ||
             aName.equalsIgnoreAsciiCaseAscii(DAVProperties::SOURCE.data + 4) )
            aNameSpace = "DAV:";
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
        rFullName  = "<prop:" + aName + " xmlns:prop=\"" + aNameSpace + "\">";
    }
}

bool DAVProperties::isUCBDeadProperty( const NeonPropName & rName )
{
    return ( rName.nspace &&
             ( rtl_str_compareIgnoreAsciiCase(
                 rName.nspace, "http://ucb.openoffice.org/dav/props/" )
               == 0 ) );
}

bool DAVProperties::isUCBSpecialProperty(
    const OUString& rFullName, OUString& rParsedName)
{
    if ( !rFullName.startsWith( "<prop:" ) || !rFullName.endsWith( "\">" ) )
        return false;

    sal_Int32 nStart = strlen( "<prop:" );
    sal_Int32 nEnd = rFullName.indexOf( ' ', nStart );
    if ( nEnd <= nStart ) // incl. -1 for "not found"
        return false;

    OUString sPropName( rFullName.copy( nStart, nEnd - nStart ) );

    // TODO skip whitespaces?
    if ( !rFullName.match( "xmlns:prop=\"", ++nEnd ) )
        return false;

    nStart = nEnd + strlen( "xmlns:prop=\"" );
    nEnd = rFullName.indexOf( '"', nStart );
    if ( nEnd != rFullName.getLength() - sal_Int32( strlen( "\">" ) )
         || nEnd == nStart )
    {
        return false;
    }

    rParsedName = rFullName.copy( nStart, nEnd - nStart );
    if ( !rParsedName.endsWith( "/" ) )
        rParsedName += "/";
    rParsedName += sPropName;

    return rParsedName.getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
