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

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "DAVProperties.hxx"
#include "UCBDeadPropertyValue.hxx"

#include "SerfPropPatchReqProcImpl.hxx"

namespace http_dav_ucp
{

SerfPropPatchReqProcImpl::SerfPropPatchReqProcImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders,
                                                    const std::vector< ProppatchValue > & inProperties )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
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
    OString aBodyText;
    {
        // create and fill body bucket with properties to be set or removed
        static const struct
        {
            const char *str;
            sal_Int32   len;
        }
        OpCode [] = {
            { RTL_CONSTASCII_STRINGPARAM( "set" ) },
            { RTL_CONSTASCII_STRINGPARAM( "remove" ) }
        };
        const int nPropCount = ( mpProperties != 0 )
                               ? mpProperties->size()
                               : 0;
        if ( nPropCount > 0 )
        {
            rtl::OUStringBuffer aBuffer;
            // add PropPatch xml header in front
            aBuffer.append( PROPPATCH_HEADER );

            // <*operation code*><prop>

            ProppatchOperation lastOp = (*mpProperties)[ 0 ].operation;
            aBuffer.append( "<" );
            aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
            aBuffer.append( "><prop>" );

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
                    aBuffer.append( "</prop></" );
                    aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
                    aBuffer.append( "><" );
                    aBuffer.appendAscii( OpCode[rProperty.operation].str, OpCode[rProperty.operation].len );
                    aBuffer.append( "><prop>" );
                }

                // <*propname* xmlns="*propns*"
                aBuffer.append( "<" );
                aBuffer.appendAscii( thePropName.name );
                aBuffer.append( " xmlns=\"" );
                aBuffer.appendAscii( thePropName.nspace );
                aBuffer.append( "\"" );

                if ( rProperty.operation == PROPSET )
                {
                    // >*property value*</*propname*>
                    aBuffer.append( ">" );

                    OUString aStringValue;
                    if ( DAVProperties::isUCBDeadProperty( thePropName ) )
                    {
                        UCBDeadPropertyValue::toXML( rProperty.value,
                                                     aStringValue );
                    }
                    else
                    {
                        rProperty.value >>= aStringValue;
                    }
                    aBuffer.append( aStringValue );
                    aBuffer.append( "</" );
                    aBuffer.appendAscii( thePropName.name );
                    aBuffer.append( ">" );
                }
                else
                {
                    aBuffer.append( "/>" );
                }

                lastOp = rProperty.operation;
            }

            // </prop></*last operation code*>
            aBuffer.append( "</prop></" );
            aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
            aBuffer.append( ">" );

            // add PropPatch xml trailer at end
            aBuffer.append( PROPPATCH_TRAILER );

            aBodyText = rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
            body_bkt = serf_bucket_simple_copy_create( aBodyText.getStr(),
                                                       aBodyText.getLength(),
                                                       pSerfBucketAlloc );
        }
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PROPPATCH",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc ) ;
    handleChunkedEncoding(req_bkt, aBodyText.getLength());

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    if (hdrs_bkt != NULL)
    {
        // general header fields provided by caller
        setRequestHeaders( hdrs_bkt );

        // request specific header fields
        if ( body_bkt != 0 && aBodyText.getLength() > 0 )
        {
            serf_bucket_headers_set( hdrs_bkt, "Content-Type", "application/xml" );
        }
    }
    else
    {
        assert("Headers Bucket missing");
    }

    return req_bkt;
}

void SerfPropPatchReqProcImpl::processChunkOfResponseData( const char* /*data*/,
                                                           apr_size_t /*len*/ )
{
    // nothing to do;
}

void SerfPropPatchReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    // nothing to do;
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
