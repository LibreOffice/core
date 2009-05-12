/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NeonHeadRequest.cxx,v $
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
#include "precompiled_ucb.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include "NeonHeadRequest.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

namespace {

#if NEON_VERSION >= 0x0250
void process_headers(ne_request *req,
                     DAVResource &rResource,
                     const std::vector< ::rtl::OUString > &rHeaderNames)
{
    void *cursor = NULL;
    const char *name, *value;

    while ((cursor = ne_response_header_iterate(req, cursor,
                                                &name, &value)) != NULL) {
        rtl::OUString aHeaderName( rtl::OUString::createFromAscii( name ) );
        rtl::OUString aHeaderValue( rtl::OUString::createFromAscii( value ) );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( rHeaderNames.size() == 0 );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< ::rtl::OUString >::const_iterator it(
                rHeaderNames.begin() );
            const std::vector< ::rtl::OUString >::const_iterator end(
                rHeaderNames.end() );

            while ( it != end )
            {
                if ( (*it) == aHeaderName )
                    break;

                ++it;
            }

            if ( it != end )
                bIncludeIt = true;
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            DAVPropertyValue thePropertyValue;
            thePropertyValue.Name = aHeaderName;
            thePropertyValue.IsCaseSensitive = false;
            thePropertyValue.Value <<= aHeaderValue;

            // Add the newly created PropertyValue
            rResource.properties.push_back( thePropertyValue );
        }
    }
}
#else
struct NeonHeadRequestContext
{
    DAVResource * pResource;
    const std::vector< ::rtl::OUString > * pHeaderNames;

    NeonHeadRequestContext( DAVResource * p,
                            const std::vector< ::rtl::OUString > * pHeaders )
    : pResource( p ), pHeaderNames( pHeaders ) {}
};

extern "C" void NHR_ResponseHeaderCatcher( void * userdata,
                                           const char * value )
{
    rtl::OUString aHeader( rtl::OUString::createFromAscii( value ) );
    sal_Int32 nPos = aHeader.indexOf( ':' );

    if ( nPos != -1 )
    {
        rtl::OUString aHeaderName( aHeader.copy( 0, nPos ) );

        NeonHeadRequestContext * pCtx
            = static_cast< NeonHeadRequestContext * >( userdata );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( pCtx->pHeaderNames->size() == 0 );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< ::rtl::OUString >::const_iterator it(
                pCtx->pHeaderNames->begin() );
            const std::vector< ::rtl::OUString >::const_iterator end(
                pCtx->pHeaderNames->end() );

            while ( it != end )
            {
                if ( (*it) == aHeaderName )
                    break;

                ++it;
            }

            if ( it != end )
                bIncludeIt = true;
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            DAVPropertyValue thePropertyValue;
            thePropertyValue.Name = aHeaderName;
            thePropertyValue.IsCaseSensitive = false;

            if ( nPos < aHeader.getLength() )
                thePropertyValue.Value <<= aHeader.copy( nPos + 1 ).trim();

            // Add the newly created PropertyValue
            pCtx->pResource->properties.push_back( thePropertyValue );
        }
    }
}
#endif

} // namespace

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

NeonHeadRequest::NeonHeadRequest( HttpSession* inSession,
                                  const rtl::OUString & inPath,
                                  const std::vector< ::rtl::OUString > &
                                    inHeaderNames,
                                  DAVResource & ioResource,
                                  int & nError )
{
    ioResource.uri = inPath;
    ioResource.properties.clear();

    // Create and dispatch HEAD request. Install catcher for all response
    // header fields.
    ne_request * req = ne_request_create( inSession,
                                          "HEAD",
                                          rtl::OUStringToOString(
                                            inPath,
                                            RTL_TEXTENCODING_UTF8 ) );

#if NEON_VERSION < 0x0250
    NeonHeadRequestContext aCtx( &ioResource, &inHeaderNames );
    ne_add_response_header_catcher( req, NHR_ResponseHeaderCatcher, &aCtx );
#endif

    nError = ne_request_dispatch( req );

#if NEON_VERSION >= 0x0250
    process_headers(req, ioResource, inHeaderNames);
#endif

    if ( nError == NE_OK && ne_get_status( req )->klass != 2 )
        nError = NE_ERROR;

    ne_request_destroy( req );
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonHeadRequest::~NeonHeadRequest()
{
}

