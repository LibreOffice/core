/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include "NeonHeadRequest.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

namespace {

void process_headers( ne_request * req,
                      DAVResource & rResource,
                      const std::vector< ::rtl::OUString > & rHeaderNames )
{
    void * cursor = NULL;
    const char * name, *value;

    while ( ( cursor = ne_response_header_iterate( req, cursor,
                                                   &name, &value ) ) != NULL ) {
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

} // namespace

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

NeonHeadRequest::NeonHeadRequest( HttpSession * inSession,
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

    nError = ne_request_dispatch( req );

    process_headers( req, ioResource, inHeaderNames );

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

