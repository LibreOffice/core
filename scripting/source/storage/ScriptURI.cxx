/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptURI.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include <osl/diagnose.h>
#include <osl/file.h>

#ifdef _DEBUG
#include <stdio.h>
#endif

//Local headers
#include "ScriptURI.hxx"
#include <util/util.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


namespace scripting_impl {

static const OUString schema = OUString::createFromAscii( "vnd.sun.star.script://" );

/**
 *  Constructor
 *
 *  @param scriptURI the string script URI
 */
ScriptURI::ScriptURI( const ::rtl::OUString& scriptURI )
    throw ( IllegalArgumentException ) : m_uri( scriptURI )
{
    OSL_TRACE( "received uri: %s\n",::rtl::OUStringToOString( m_uri, RTL_TEXTENCODING_ASCII_US).pData->buffer );
    set_values( parseIt() );
    if( !isValid() )
    {
        OSL_TRACE( "ScriptURI ctor: throwing IllegalArgException" );
        throw IllegalArgumentException(
            OUSTR( "Failed to parse invalid URI: " ).concat( scriptURI ),
            Reference < XInterface > (), 1 );
    }
}

/**
 *  Destuctor
 *
 */
// dtor should never throw exceptions, so ensure this by specifying it
ScriptURI::~ScriptURI() SAL_THROW( () )
{
    OSL_TRACE( "< ScriptURI dtor called >\n" );
}

/**
 *  This function is used to determine if this object represents a valid URI
 *
 */
bool ScriptURI::isValid(  ) {
    return ( m_valid == sal_True );
}

/**
 *  This function returns the location of the script
 *
 */
::rtl::OUString ScriptURI::getLocation(  )
{
    return m_location;
}

/**
 *  This function returns the language of the script, eg. java, StarBasic,...
 *
 */
::rtl::OUString ScriptURI::getLanguage(  )
{
    return m_language;
}

/**
 *  This function returns the language dependent function name of the script
 *
 */
::rtl::OUString ScriptURI::getFunctionName(  )
{
    return m_functionName;
}

/**
 *  This function returns the language independent logical name of the script
 *
 */
::rtl::OUString ScriptURI::getLogicalName(  )
{
    return m_logicalName;
}

/**
 *  This function returns the full URI
 *
 */
::rtl::OUString ScriptURI::getURI(  )
{
    return m_uri;
}

//Private mutex guarded method for setting the members
void ScriptURI::set_values( scripting_impl::Uri values )
{
    osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    m_valid = values.valid;
    m_location = values.location;
    m_language = values.language;
// format is vnd.sun.star.script://[function_name]?language=[languge]&location=[location]
// LogicalName is now not used anymore, further more the ScriptURI class
// will be retired also and a new UNO service will be used. Additionally the
// parcel-description will also need to be modified to remove logical name
// In order to temporarly support the existing code functionname is
// set to the logica name parsed by this class. So getLogicalName() and
// getFunctionName() return identical string.
//

    m_functionName = values.logicalName;
    m_logicalName = values.logicalName;

}
/**
 *  This is a private method used for parsing the URI received by the
 * initialization.
 *
 */
// rather naive parsing?
Uri ScriptURI::parseIt()
{
    sal_Int32 schemaLen = schema.getLength();
    scripting_impl::Uri results;
    results.valid = sal_True;
    //attempt to parse
    // check that it starts vnd.sun.star.script
    // better check for OBO errors here
    if( m_uri.indexOf( schema ) != 0 )
    {
        OSL_TRACE( "wrong schema" );
        results.valid=sal_False;
        return results;
    }

    // substr from here to the '?' and set the logical name
    sal_Int32 len = m_uri.indexOf( '?' );
    if( len == -1 )
    {
        // no queries so just set the logical name
        results.logicalName = m_uri.copy( schemaLen );
        return results;
    }
    results.logicalName = m_uri.copy( schemaLen, len-schemaLen );
    OSL_TRACE( "log name: %s\n", ::rtl::OUStringToOString( results.logicalName,
        RTL_TEXTENCODING_ASCII_US ).pData->buffer );

    len++;
    // now get the attributes
    OUString attr;
    do
    {
        attr = m_uri.getToken( 0, '&', len );
        OSL_TRACE( "chunk: %s\n", ::rtl::OUStringToOString( attr,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        if( attr.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "language" ) )
            == sal_True )
        {
            sal_Int32 len2 = attr.indexOf('=');
            results.language = attr.copy( len2 + 1 );
            continue;
        }
        else if ( attr.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "location" ) )
            == sal_True )
        {
            sal_Int32 len2 = attr.indexOf( '=' );
            results.location = attr.copy( len2 + 1 );
            continue;
        }
        else if ( attr.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "function" ) )
            == sal_True )
        {
            sal_Int32 len2 = attr.indexOf( '=' );
            results.functionName = attr.copy( len2 + 1 );
            continue;
        }
        else
        {
            OSL_TRACE( "Unknown attribute?\n" );
        }
    }
    while ( len >= 0 );

    // parse is good
    return results;
}

} // namespace script_uri
