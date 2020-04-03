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

#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include "NeonHeadRequest.hxx"
#include "NeonSession.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

namespace {

void process_headers( ne_request * req,
                      DAVResource & rResource,
                      const std::vector< OUString > & rHeaderNames )
{
    void * cursor = nullptr;
    const char * name, *value;

#if defined SAL_LOG_INFO
    {
        for ( const auto& rHeader : rHeaderNames )
        {
            SAL_INFO( "ucb.ucp.webdav", "HEAD - requested header: " << rHeader );
        }
    }
#endif
    while ( ( cursor = ne_response_header_iterate( req, cursor,
                                                   &name, &value ) ) != nullptr ) {
        // The HTTP header `field-name` must be a `token`, which can only contain a subset of ASCII;
        // assume that Neon will already have rejected any invalid data, so that it is guaranteed
        // that `name` is ASCII-only:
        OUString aHeaderName( OUString::createFromAscii( name ) );
        // The HTTP header `field-value` may contain obsolete (as per RFC 7230) `obs-text` non-ASCII
        // %x80-FF octets, lets preserve them as individual characters in `aHeaderValue` by treating
        // `value` as ISO 8859-1:
        OUString aHeaderValue(value, strlen(value), RTL_TEXTENCODING_ISO_8859_1);

        SAL_INFO( "ucb.ucp.webdav", "HEAD - received header: " << aHeaderName << ":" << aHeaderValue);

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = rHeaderNames.empty();

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            auto it = std::find_if(rHeaderNames.begin(), rHeaderNames.end(),
                [&aHeaderName](const OUString& rName) {
                    // header names are case insensitive
                    return rName.equalsIgnoreAsciiCase( aHeaderName );
                });

            if ( it != rHeaderNames.end() )
            {
                aHeaderName = *it;
                bIncludeIt = true;
            }
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            DAVPropertyValue thePropertyValue;
            // header names are case insensitive, so are the
            // corresponding property names
            thePropertyValue.Name = aHeaderName.toAsciiLowerCase();
            thePropertyValue.IsCaseSensitive = false;
            thePropertyValue.Value <<= aHeaderValue;

            // Add the newly created PropertyValue
            rResource.properties.push_back( thePropertyValue );
        }
    }
}

} // namespace

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
        osl::Guard< osl::Mutex > theGlobalGuard(getGlobalNeonMutex());
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
