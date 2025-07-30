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

#include "DAVProperties.hxx"
#include <rtl/ustrbuf.hxx>
#include <o3tl/string_view.hxx>
#include <string.h>

using namespace http_dav_ucp;


// static
void DAVProperties::createSerfPropName( ::std::u16string_view const rFullName,
                                        SerfPropName & rName )
{
    if (o3tl::starts_with(rFullName, u"DAV:"))
    {
        rName.nspace = "DAV:"_ostr;
        rName.name
            = OUStringToOString(
                rFullName.substr(RTL_CONSTASCII_LENGTH("DAV:")),
                                RTL_TEXTENCODING_UTF8 );
    }
    else if (o3tl::starts_with(rFullName, u"http://apache.org/dav/props/"))
    {
        rName.nspace = "http://apache.org/dav/props/"_ostr;
        rName.name =
            OUStringToOString(
              rFullName.substr(
                  RTL_CONSTASCII_LENGTH(
                      "http://apache.org/dav/props/" ) ),
                  RTL_TEXTENCODING_UTF8 );
    }
    else if (o3tl::starts_with(rFullName, u"http://ucb.openoffice.org/dav/props/"))
    {
        rName.nspace = "http://ucb.openoffice.org/dav/props/"_ostr;
        rName.name =
            OUStringToOString(
              rFullName.substr(
                  RTL_CONSTASCII_LENGTH(
                      "http://ucb.openoffice.org/dav/props/" ) ),
                  RTL_TEXTENCODING_UTF8 );
    }
    else if (o3tl::starts_with(rFullName, u"<prop:"))
    {
        // Support for 3rd party namespaces/props

        OString aFullName
            = OUStringToOString( rFullName, RTL_TEXTENCODING_UTF8 );

        // Format: <prop:the_propname xmlns:prop="the_namespace">

        sal_Int32 nStart = RTL_CONSTASCII_LENGTH( "<prop:" );
        sal_Int32 nLen = aFullName.indexOf( ' ' ) - nStart;
        rName.name = aFullName.copy( nStart, nLen );

        nStart = aFullName.indexOf( '=', nStart + nLen ) + 2; // after ="
        nLen = aFullName.getLength() - RTL_CONSTASCII_LENGTH( "\">" ) - nStart;
        rName.nspace = aFullName.copy( nStart, nLen );
    }
    else
    {
        // this must not be a URI - WebDAVResponseParser must have converted it
        // to the "<prop:" form above
        assert(rFullName.find(':') == ::std::u16string_view::npos);
        // Add our namespace to our own properties.
        rName.nspace = "http://ucb.openoffice.org/dav/props/"_ostr;
        rName.name =
            OUStringToOString( rFullName,
                                    RTL_TEXTENCODING_UTF8 );
    }
}


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

        if (o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::RESOURCETYPE).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::SUPPORTEDLOCK).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::LOCKDISCOVERY).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::CREATIONDATE).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::DISPLAYNAME).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::GETCONTENTLANGUAGE).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::GETCONTENTLENGTH).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::GETCONTENTTYPE).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::GETETAG).substr(4)) ||
            o3tl::equalsIgnoreAsciiCase(aName, std::u16string_view(DAVProperties::GETLASTMODIFIED).substr(4)))
        {
            aNameSpace = "DAV:";
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
        rFullName  = "<prop:";
        rFullName += aName;
        rFullName += " xmlns:prop=\"";
        rFullName += aNameSpace;
        rFullName += "\">";
    }
}


// static
bool DAVProperties::isUCBDeadProperty( const SerfPropName & rName )
{
    return rName.nspace.equalsIgnoreAsciiCase("http://ucb.openoffice.org/dav/props/");
}

bool DAVProperties::isUCBSpecialProperty(std::u16string_view rFullName, OUString& rParsedName)
{
    size_t nLen = rFullName.size();
    if ( nLen <= 0 ||
            !o3tl::starts_with(rFullName, u"<prop:" ) ||
            !o3tl::starts_with(rFullName, u"\">" ) )
        return false;

    sal_Int32 nStart = RTL_CONSTASCII_LENGTH( "<prop:" );
    size_t nEnd = rFullName.find( ' ', nStart );
    if ( nEnd == std::u16string_view::npos )
        return false;

    std::u16string_view sPropName = rFullName.substr( nStart, nEnd - nStart );
    if ( sPropName.empty() )
        return false;

    // TODO skip whitespaces?
    ++nEnd;
    if ( !o3tl::starts_with(rFullName.substr(nEnd), u"xmlns:prop=\"" ) )
        return false;

    nStart = nEnd + RTL_CONSTASCII_LENGTH( "xmlns:prop=\"" );
    nEnd = rFullName.find( '"', nStart );
    if ( nEnd != nLen - RTL_CONSTASCII_LENGTH( "\">" ) )
        return false;

    std::u16string_view sNamesp = rFullName.substr( nStart, nEnd - nStart );
    nLen = sNamesp.size();
    if ( !nLen )
        return false;

    OUStringBuffer aBuff( sNamesp );
    if ( sNamesp[nLen - 1] != '/' )
        aBuff.append( '/' );
    aBuff.append( sPropName );
    rParsedName = aBuff.makeStringAndClear();

    return rParsedName.getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
