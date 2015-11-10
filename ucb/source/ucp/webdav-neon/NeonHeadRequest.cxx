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

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include "NeonHeadRequest.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

namespace {

void process_headers( ne_request * req,
                      DAVResource & rResource,
                      const std::vector< OUString > & rHeaderNames )
{
    void * cursor = nullptr;
    const char * name, *value;

    while ( ( cursor = ne_response_header_iterate( req, cursor,
                                                   &name, &value ) ) != nullptr ) {
        OUString aHeaderName( OUString::createFromAscii( name ) );
        OUString aHeaderValue( OUString::createFromAscii( value ) );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( rHeaderNames.empty() );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< OUString >::const_iterator it(
                rHeaderNames.begin() );
            const std::vector< OUString >::const_iterator end(
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

} // namespace

extern osl::Mutex aGlobalNeonMutex;

NeonHeadRequest::NeonHeadRequest( HttpSession * inSession,
                                  const OUString & inPath,
                                  const std::vector< OUString > &
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
                                          OUStringToOString(
                                            inPath,
                                            RTL_TEXTENCODING_UTF8 ).getStr() );

    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        nError = ne_request_dispatch( req );
    }

    process_headers( req, ioResource, inHeaderNames );

    if ( nError == NE_OK && ne_get_status( req )->klass != 2 )
        nError = NE_ERROR;

    ne_request_destroy( req );
}

NeonHeadRequest::~NeonHeadRequest()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
