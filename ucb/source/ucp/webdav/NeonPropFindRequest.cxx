/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NeonPropFindRequest.cxx,v $
 * $Revision: 1.24 $
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
#include "precompiled_ucb.hxx"
#include <osl/diagnose.h>
#include "NeonTypes.hxx"
#include "DAVException.hxx"
#include "DAVProperties.hxx"
#include "NeonPropFindRequest.hxx"
#ifndef _LINKSEQUENCE_HXX_
#include "LinkSequence.hxx"
#endif
#include "LockSequence.hxx"
#include "LockEntrySequence.hxx"
#include "UCBDeadPropertyValue.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace std;
using namespace webdav_ucp;

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

    DAVProperties::createUCBPropName( pname->nspace,
                                        pname->name,
                                        thePropertyValue.Name );
    bool bHasValue = false;
    if ( DAVProperties::isUCBDeadProperty( *pname ) )
    {
        // DAV dead property added by WebDAV UCP?
        if ( UCBDeadPropertyValue::createFromXML(
                                        value, thePropertyValue.Value ) )
            OSL_ENSURE( thePropertyValue.Value.hasValue(),
                        "NeonPropFindRequest::propfind_iter - No value!" );
            bHasValue = true;
    }

    if ( !bHasValue )
    {
        if ( rtl_str_compareIgnoreAsciiCase(
                                    pname->name, "resourcetype" ) == 0 )
        {
            OString aValue( value );
            aValue = aValue.trim(); // #107358# remove leading/trailing spaces
            if ( aValue.getLength() )
            {
                aValue = aValue.toAsciiLowerCase();
                if ( aValue.compareTo(
                    RTL_CONSTASCII_STRINGPARAM( "<dav:collection" ) ) == 0 )
                {
                    thePropertyValue.Value
                        <<= OUString::createFromAscii( "collection" );
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
            LockEntrySequence::createFromXML( value, aEntries );
            thePropertyValue.Value <<= aEntries;
        }
        else if ( rtl_str_compareIgnoreAsciiCase(
                                    pname->name, "lockdiscovery" ) == 0 )
        {
            Sequence< Lock > aLocks;
            LockSequence::createFromXML( value, aLocks );
            thePropertyValue.Value <<= aLocks;
        }
        else if ( rtl_str_compareIgnoreAsciiCase( pname->name, "source" ) == 0 )
        {
            Sequence< Link > aLinks;
            LinkSequence::createFromXML( value, aLinks );
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
#if NEON_VERSION >= 0x0260
                                       const ne_uri* uri,
#else
                                       const char* href,
#endif
                                       const NeonPropFindResultSet* set )
{
    // @@@ href is not the uri! DAVResource ctor wants uri!

#if NEON_VERSION >= 0x0260
    DAVResource theResource(
                        OStringToOUString( uri->path, RTL_TEXTENCODING_UTF8 ) );
#else
    DAVResource theResource(
                        OStringToOUString( href, RTL_TEXTENCODING_UTF8 ) );
#endif

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
#if NEON_VERSION >= 0x0260
                                        const ne_uri* uri,
#else
                                        const char* href,
#endif
                                        const NeonPropFindResultSet* results )
{
    // @@@ href is not the uri! DAVResourceInfo ctor wants uri!
    // Create entry for the resource.
#if NEON_VERSION >= 0x0260
    DAVResourceInfo theResource(
                        OStringToOUString( uri->path, RTL_TEXTENCODING_UTF8 ) );
#else
    DAVResourceInfo theResource(
                        OStringToOUString( href, RTL_TEXTENCODING_UTF8 ) );
#endif

    // Fill entry.
    ne_propset_iterate( results, NPFR_propnames_iter, &theResource );

    // Add entry to resources list.
    vector< DAVResourceInfo > * theResources
        = static_cast< vector< DAVResourceInfo > * >( userdata );
    theResources->push_back( theResource );
}

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

        nError = ne_simple_propfind( inSession,
                                     inPath,
                                     inDepth,
                                     thePropNames,
                                     NPFR_propfind_results,
                                     &ioResources );

        for ( theIndex = 0; theIndex < thePropCount; theIndex ++ )
            free( (void *)thePropNames[ theIndex ].name );

        delete [] thePropNames;
    }
    else
    {
        // ALLPROP
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
    nError = ne_propnames( inSession,
                           inPath,
                           inDepth,
                           NPFR_propnames_results,
                           &ioResInfo );

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
