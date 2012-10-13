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


#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "NeonTypes.hxx"
#include "DAVException.hxx"
#include "DAVProperties.hxx"
#include "NeonPropFindRequest.hxx"
#include "LinkSequence.hxx"
#include "LockSequence.hxx"
#include "LockEntrySequence.hxx"
#include "UCBDeadPropertyValue.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace std;
using namespace webdav_ucp;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;

// -------------------------------------------------------------------
namespace
{
    // strip "DAV:" namespace from XML snippets to avoid
    // parser error (undeclared namespace) later on.
    rtl::OString stripDavNamespace( const rtl::OString & in )
    {
        const rtl::OString inXML( in.toAsciiLowerCase() );

        rtl::OStringBuffer buf;
        sal_Int32 start = 0;
        sal_Int32 end = inXML.indexOf( "dav:" );
        while ( end != -1 )
        {
            if ( inXML[ end - 1 ] == '<' ||
                 inXML[ end - 1 ] == '/' )
            {
                // copy from original buffer - preserve case.
                buf.append( in.copy( start, end - start ) );
            }
            else
            {
                // copy from original buffer - preserve case.
                buf.append( in.copy( start, end - start + 4 ) );
            }
            start = end + 4;
            end = inXML.indexOf( "dav:", start );
        }
        buf.append( inXML.copy( start ) );

        return rtl::OString( buf.makeStringAndClear() );
    }
}

// -------------------------------------------------------------------
extern "C" int NPFR_propfind_iter( void* userdata,
                                   const NeonPropName* pname,
                                   const char* value,
                                   const HttpStatus* status )
{
    /*
        HTTP Response Status Classes:

        - 1: Informational - Request received, continuing process

        - 2: Success - The action was successfully received,
          understood, and accepted

        - 3: Redirection - Further action must be taken in order to
          complete the request

        - 4: Client Error - The request contains bad syntax or cannot
          be fulfilled

        - 5: Server Error - The server failed to fulfill an apparently
          valid request
    */

    if ( status->klass > 2 )
        return 0; // Error getting this property. Go on.

    // Create & set the PropertyValue
    DAVPropertyValue thePropertyValue;
    thePropertyValue.IsCaseSensitive = true;

    OSL_ENSURE( pname->nspace, "NPFR_propfind_iter - No namespace!" );

    DAVProperties::createUCBPropName( pname->nspace,
                                      pname->name,
                                      thePropertyValue.Name );
    bool bHasValue = false;
    if ( DAVProperties::isUCBDeadProperty( *pname ) )
    {
        // DAV dead property added by WebDAV UCP?
        if ( UCBDeadPropertyValue::createFromXML(
                 value, thePropertyValue.Value ) )
        {
            OSL_ENSURE( thePropertyValue.Value.hasValue(),
                        "NPFR_propfind_iter - No value!" );
            bHasValue = true;
        }
    }

    if ( !bHasValue )
    {
        if ( rtl_str_compareIgnoreAsciiCase(
                                    pname->name, "resourcetype" ) == 0 )
        {
            OString aValue( value );
            aValue = aValue.trim(); // #107358# remove leading/trailing spaces
            if ( !aValue.isEmpty() )
            {
                aValue = stripDavNamespace( aValue ).toAsciiLowerCase();
                if ( aValue.compareTo(
                         RTL_CONSTASCII_STRINGPARAM( "<collection" ) ) == 0 )
                {
                    thePropertyValue.Value
                        <<= OUString("collection");
                }
            }

            if ( !thePropertyValue.Value.hasValue() )
            {
                // Take over the value exactly as supplied by the server.
                thePropertyValue.Value <<= OUString::createFromAscii( value );
            }
        }
        else if ( rtl_str_compareIgnoreAsciiCase(
                                    pname->name, "supportedlock" ) == 0 )
        {
            Sequence< LockEntry > aEntries;
            LockEntrySequence::createFromXML(
                stripDavNamespace( value ), aEntries );
            thePropertyValue.Value <<= aEntries;
        }
        else if ( rtl_str_compareIgnoreAsciiCase(
                                    pname->name, "lockdiscovery" ) == 0 )
        {
            Sequence< Lock > aLocks;
            LockSequence::createFromXML(
                stripDavNamespace( value ), aLocks );
            thePropertyValue.Value <<= aLocks;
        }
        else if ( rtl_str_compareIgnoreAsciiCase( pname->name, "source" ) == 0 )
        {
            Sequence< Link > aLinks;
            LinkSequence::createFromXML(
                stripDavNamespace( value ), aLinks );
            thePropertyValue.Value <<= aLinks;
        }
        else
        {
            thePropertyValue.Value
                <<= OStringToOUString( value, RTL_TEXTENCODING_UTF8 );
        }
    }

    // Add the newly created PropertyValue
    DAVResource* theResource = static_cast< DAVResource * >( userdata );
    theResource->properties.push_back( thePropertyValue );

    return 0; // Go on.
}

// -------------------------------------------------------------------
extern "C" void NPFR_propfind_results( void* userdata,
                                       const ne_uri* uri,
                                       const NeonPropFindResultSet* set )
{
    // @@@ href is not the uri! DAVResource ctor wants uri!

    DAVResource theResource(
        OStringToOUString( uri->path, RTL_TEXTENCODING_UTF8 ) );

    ne_propset_iterate( set, NPFR_propfind_iter, &theResource );

    // Add entry to resources list.
    vector< DAVResource > * theResources
        = static_cast< vector< DAVResource > * >( userdata );
    theResources->push_back( theResource );
}
// -------------------------------------------------------------------
extern "C" int NPFR_propnames_iter( void* userdata,
                                    const NeonPropName* pname,
                                    const char* /*value*/,
                                    const HttpStatus* /*status*/ )
{
    OUString aFullName;
    DAVProperties::createUCBPropName( pname->nspace,
                                      pname->name,
                                      aFullName );

    DAVResourceInfo* theResource = static_cast< DAVResourceInfo * >( userdata );
    theResource->properties.push_back( aFullName );
    return 0;
}

// -------------------------------------------------------------------
extern "C" void NPFR_propnames_results( void* userdata,
                                        const ne_uri* uri,
                                        const NeonPropFindResultSet* results )
{
    // @@@ href is not the uri! DAVResourceInfo ctor wants uri!
    // Create entry for the resource.
    DAVResourceInfo theResource(
        OStringToOUString( uri->path, RTL_TEXTENCODING_UTF8 ) );

    // Fill entry.
    ne_propset_iterate( results, NPFR_propnames_iter, &theResource );

    // Add entry to resources list.
    vector< DAVResourceInfo > * theResources
        = static_cast< vector< DAVResourceInfo > * >( userdata );
    theResources->push_back( theResource );
}

extern osl::Mutex aGlobalNeonMutex;

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

NeonPropFindRequest::NeonPropFindRequest( HttpSession* inSession,
                                          const char* inPath,
                                          const Depth inDepth,
                                          const vector< OUString >& inPropNames,
                                          vector< DAVResource >& ioResources,
                                          int & nError )
{
    // Generate the list of properties we're looking for
    int thePropCount = inPropNames.size();
    if ( thePropCount > 0 )
    {
        NeonPropName* thePropNames = new NeonPropName[ thePropCount + 1 ];
        int theIndex;

        for ( theIndex = 0; theIndex < thePropCount; theIndex ++ )
        {
            // Split fullname into namespace and name!
            DAVProperties::createNeonPropName(
                inPropNames[ theIndex ], thePropNames[ theIndex ] );
        }
        thePropNames[ theIndex ].nspace = NULL;
        thePropNames[ theIndex ].name   = NULL;

        {
            osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
            nError = ne_simple_propfind( inSession,
                                         inPath,
                                         inDepth,
                                         thePropNames,
                                         NPFR_propfind_results,
                                         &ioResources );
        }

        for ( theIndex = 0; theIndex < thePropCount; theIndex ++ )
            free( (void *)thePropNames[ theIndex ].name );

        delete [] thePropNames;
    }
    else
    {
        // ALLPROP
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        nError = ne_simple_propfind( inSession,
                                     inPath,
                                     inDepth,
                                     NULL, // 0 == allprop
                                     NPFR_propfind_results,
                                     &ioResources );
    }

    // #87585# - Sometimes neon lies (because some servers lie).
    if ( ( nError == NE_OK ) && ioResources.empty() )
        nError = NE_ERROR;
}

// -------------------------------------------------------------------
// Constructor
// - obtains property names
// -------------------------------------------------------------------

NeonPropFindRequest::NeonPropFindRequest(
                            HttpSession* inSession,
                            const char* inPath,
                            const Depth inDepth,
                            std::vector< DAVResourceInfo > & ioResInfo,
                            int & nError )
{
    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        nError = ne_propnames( inSession,
                            inPath,
                            inDepth,
                            NPFR_propnames_results,
                            &ioResInfo );
    }

    // #87585# - Sometimes neon lies (because some servers lie).
    if ( ( nError == NE_OK ) && ioResInfo.empty() )
        nError = NE_ERROR;
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonPropFindRequest::~NeonPropFindRequest( )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
