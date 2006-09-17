/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVProperties.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:04:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>

#ifndef _DAVPROPERTIES_HXX_
#include "DAVProperties.hxx"
#endif

using namespace webdav_ucp;

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
const ::rtl::OUString DAVProperties::SOURCE =
                ::rtl::OUString::createFromAscii( "DAV:source" );
const ::rtl::OUString DAVProperties::SUPPORTEDLOCK =
                ::rtl::OUString::createFromAscii( "DAV:supportedlock" );

const ::rtl::OUString DAVProperties::EXECUTABLE =
                ::rtl::OUString::createFromAscii(
                                "http://apache.org/dav/props/executable" );

// -------------------------------------------------------------------
// static
void DAVProperties::createNeonPropName( const rtl::OUString & rFullName,
                                        NeonPropName & rName )
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
bool DAVProperties::isUCBDeadProperty( const NeonPropName & rName )
{
    return ( rtl_str_compareIgnoreAsciiCase(
                rName.nspace, "http://ucb.openoffice.org/dav/props/" ) == 0 );
}
