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

#include <rtl/ustring.hxx>
#include <DAVProperties.hxx>
#include <UCBDeadPropertyValue.hxx>

#include <SerfPropPatchReqProcImpl.hxx>
#include <SerfTypes.hxx>

namespace http_dav_ucp
{

SerfPropPatchReqProcImpl::SerfPropPatchReqProcImpl( const char* inPath,
                                                    const std::vector< ProppatchValue > & inProperties )
    : SerfRequestProcessorImpl( inPath )
    , mpProperties( &inProperties )
{
}

SerfPropPatchReqProcImpl::~SerfPropPatchReqProcImpl()
{
}

#define PROPPATCH_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\"?><propertyupdate xmlns=\"DAV:\">"
#define PROPPATCH_TRAILER "</propertyupdate>"

serf_bucket_t * SerfPropPatchReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // body bucket
    serf_bucket_t* body_bkt = 0;
    rtl::OUString aBodyText;
    {
        // create and fill body bucket with properties to be set or removed
        static const char* OpCodes[2] = { "set", "remove" };
        const int nPropCount = ( mpProperties != 0 )
                               ? mpProperties->size()
                               : 0;
        if ( nPropCount > 0 )
        {
            // <*operation code*><prop>
            ProppatchOperation lastOp = (*mpProperties)[ 0 ].operation;
            aBodyText += rtl::OUString::createFromAscii( "<" );
            aBodyText += rtl::OUString::createFromAscii( OpCodes[lastOp] );
            aBodyText += rtl::OUString::createFromAscii( "><prop>" );

            SerfPropName thePropName;
            for ( int n = 0; n < nPropCount; ++n )
            {
                const ProppatchValue & rProperty = (*mpProperties)[ n ];
                // split fullname into namespace and name!
                DAVProperties::createSerfPropName( rProperty.name,
                                                   thePropName );

                if ( rProperty.operation != lastOp )
                {
                    // </prop></*last operation code*><*operation code><prop>
                    aBodyText += rtl::OUString::createFromAscii( "</prop></" );
                    aBodyText += rtl::OUString::createFromAscii( OpCodes[lastOp] );
                    aBodyText += rtl::OUString::createFromAscii( "><" );
                    aBodyText += rtl::OUString::createFromAscii( OpCodes[rProperty.operation] );
                    aBodyText += rtl::OUString::createFromAscii( "><prop>" );
                }

                // <*propname* xmlns="*propns*"
                aBodyText += rtl::OUString::createFromAscii( "<" );
                aBodyText += rtl::OUString::createFromAscii( thePropName.name );
                aBodyText += rtl::OUString::createFromAscii( " xmlns=\"" );
                aBodyText += rtl::OUString::createFromAscii( thePropName.nspace );
                aBodyText += rtl::OUString::createFromAscii( "\"" );

                if ( rProperty.operation == PROPSET )
                {
                    // >*property value*</*propname*>
                    aBodyText += rtl::OUString::createFromAscii( ">" );

                    rtl::OUString aStringValue;
                    if ( DAVProperties::isUCBDeadProperty( thePropName ) )
                    {
                        UCBDeadPropertyValue::toXML( rProperty.value,
                                                     aStringValue );
                    }
                    else
                    {
                        rProperty.value >>= aStringValue;
                    }
                    aBodyText += aStringValue;
                    aBodyText += rtl::OUString::createFromAscii( "</" );
                    aBodyText += rtl::OUString::createFromAscii( thePropName.name );
                    aBodyText += rtl::OUString::createFromAscii( ">" );
                }
                else
                {
                    // />
                    aBodyText += rtl::OUString::createFromAscii( "/>" );
                }

                lastOp = rProperty.operation;
            }

            // </prop></*last operation code*>
            aBodyText += rtl::OUString::createFromAscii( "</prop></" );
            aBodyText += rtl::OUString::createFromAscii( OpCodes[lastOp] );
            aBodyText += rtl::OUString::createFromAscii( ">" );

            // add PropPatch xml header in front
            aBodyText = rtl::OUString::createFromAscii( PROPPATCH_HEADER ) + aBodyText;

            // add PropPatch xml trailer at end
            aBodyText += rtl::OUString::createFromAscii( PROPPATCH_TRAILER );

            const char* pTestValue = rtl::OUStringToOString( aBodyText, RTL_TEXTENCODING_UTF8 );
            body_bkt = SERF_BUCKET_SIMPLE_STRING( rtl::OUStringToOString( aBodyText, RTL_TEXTENCODING_UTF8 ),
                                                  pSerfBucketAlloc );
        }
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PROPPATCH",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // TODO - correct header data
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    serf_bucket_headers_setn( hdrs_bkt, "User-Agent", "www.openoffice.org/ucb/" );
    serf_bucket_headers_setn( hdrs_bkt, "Accept-Encoding", "gzip");

    // request specific header fields
    if ( body_bkt != 0 && aBodyText.getLength() > 0 )
    {
        serf_bucket_headers_setn( hdrs_bkt, "Content-Type", "application/xml" );
        serf_bucket_headers_setn( hdrs_bkt, "Content-Length",
                                  rtl::OUStringToOString( rtl::OUString::valueOf( aBodyText.getLength() ), RTL_TEXTENCODING_UTF8 ) );
    }

    return req_bkt;
}


bool SerfPropPatchReqProcImpl::processSerfResponseBucket( serf_request_t * /*inSerfRequest*/,
                                                         serf_bucket_t * inSerfResponseBucket,
                                                         apr_pool_t * /*inAprPool*/,
                                                         apr_status_t & outStatus )
{
    const char* data;
    apr_size_t len;

    while (1) {
        outStatus = serf_bucket_read(inSerfResponseBucket, 2048, &data, &len);
        if (SERF_BUCKET_READ_ERROR(outStatus))
        {
            return true;
        }

        /* are we done yet? */
        if (APR_STATUS_IS_EOF(outStatus))
        {
            outStatus = APR_EOF;
            return true;
        }

        /* have we drained the response so far? */
        if ( APR_STATUS_IS_EAGAIN( outStatus ) )
        {
            return false;
        }
    }

    /* NOTREACHED */
    return true;
}

} // namespace http_dav_ucp
