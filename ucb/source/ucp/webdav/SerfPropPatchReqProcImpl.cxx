/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "DAVProperties.hxx"
#include "UCBDeadPropertyValue.hxx"

#include "SerfPropPatchReqProcImpl.hxx"
#include "SerfTypes.hxx"

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

    
    serf_bucket_t* body_bkt = 0;
    OString aBodyText;
    {
        
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
            
            aBuffer.append( PROPPATCH_HEADER );

            

            ProppatchOperation lastOp = (*mpProperties)[ 0 ].operation;
            aBuffer.append( "<" );
            aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
            aBuffer.append( "><prop>" );

            SerfPropName thePropName;
            for ( int n = 0; n < nPropCount; ++n )
            {
                const ProppatchValue & rProperty = (*mpProperties)[ n ];
                
                DAVProperties::createSerfPropName( rProperty.name,
                                                   thePropName );

                if ( rProperty.operation != lastOp )
                {
                    
                    aBuffer.append( "</prop></" );
                    aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
                    aBuffer.append( "><" );
                    aBuffer.appendAscii( OpCode[rProperty.operation].str, OpCode[rProperty.operation].len );
                    aBuffer.append( "><prop>" );
                }

                
                aBuffer.append( "<" );
                aBuffer.appendAscii( thePropName.name );
                aBuffer.append( " xmlns=\"" );
                aBuffer.appendAscii( thePropName.nspace );
                aBuffer.append( "\"" );

                if ( rProperty.operation == PROPSET )
                {
                    
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

            
            aBuffer.append( "</prop></" );
            aBuffer.appendAscii( OpCode[lastOp].str, OpCode[lastOp].len );
            aBuffer.append( ">" );

            
            aBuffer.append( PROPPATCH_TRAILER );

            aBodyText = rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
            body_bkt = serf_bucket_simple_copy_create( aBodyText.getStr(),
                                                       aBodyText.getLength(),
                                                       pSerfBucketAlloc );
        }
    }

    
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PROPPATCH",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc ) ;

    
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    
    setRequestHeaders( hdrs_bkt );

    
    if ( body_bkt != 0 && aBodyText.getLength() > 0 )
    {
        if ( useChunkedEncoding() )
        {
            serf_bucket_headers_set( hdrs_bkt, "Transfer-Encoding", "chunked");
        }
        serf_bucket_headers_set( hdrs_bkt, "Content-Type", "application/xml" );
        serf_bucket_headers_set( hdrs_bkt, "Content-Length",
                                 OUStringToOString( OUString::valueOf( aBodyText.getLength() ), RTL_TEXTENCODING_UTF8 ) );
    }

    return req_bkt;
}

void SerfPropPatchReqProcImpl::processChunkOfResponseData( const char* /*data*/,
                                                           apr_size_t /*len*/ )
{
    
}

void SerfPropPatchReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
