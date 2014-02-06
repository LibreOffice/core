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

#include "SerfPropFindReqProcImpl.hxx"
#include "SerfTypes.hxx"
#include "DAVProperties.hxx"

#include "webdavresponseparser.hxx"
#include <comphelper/seqstream.hxx>
#include <rtl/ustrbuf.hxx>


using namespace com::sun::star;

namespace http_dav_ucp
{

SerfPropFindReqProcImpl::SerfPropFindReqProcImpl( const char* inPath,
                                                  const DAVRequestHeaders& inRequestHeaders,
                                                  const Depth inDepth,
                                                  const std::vector< OUString > & inPropNames,
                                                  std::vector< DAVResource > & ioResources )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mDepthStr( 0 )
    , mpPropNames( &inPropNames )
    , mpResources( &ioResources )
    , mpResInfo( 0 )
    , mbOnlyPropertyNames( false )
    , xInputStream( new SerfInputStream() )
{
    init( inDepth );
}

SerfPropFindReqProcImpl::SerfPropFindReqProcImpl( const char* inPath,
                                                  const DAVRequestHeaders& inRequestHeaders,
                                                  const Depth inDepth,
                                                  std::vector< DAVResourceInfo > & ioResInfo )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mDepthStr( 0 )
    , mpPropNames( 0 )
    , mpResources( 0 )
    , mpResInfo( &ioResInfo )
    , mbOnlyPropertyNames( true )
    , xInputStream( new SerfInputStream() )
{
    init( inDepth );
}

void SerfPropFindReqProcImpl::init( const Depth inDepth )
{
    switch ( inDepth )
    {
        case DAVZERO:
            mDepthStr = "0";
            break;
        case DAVONE:
            mDepthStr = "1";
            break;
        case DAVINFINITY:
            mDepthStr = "infinity";
            break;
    }
}

SerfPropFindReqProcImpl::~SerfPropFindReqProcImpl()
{
}

#define PROPFIND_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\"?><propfind xmlns=\"DAV:\">"
#define PROPFIND_TRAILER "</propfind>"

serf_bucket_t * SerfPropFindReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // body bucket - certain properties OR all properties OR only property names
    serf_bucket_t* body_bkt = 0;
    OString aBodyText;
    {
        // TODO is it really needed a Unicode string buffer?
        // All properties and property names aren't supposed to be ASCII?
        rtl::OUStringBuffer aBuffer;
        aBuffer.append( PROPFIND_HEADER );

        // create and fill body bucket with requested properties
        const int nPropCount = ( !mbOnlyPropertyNames && mpPropNames )
                               ? mpPropNames->size()
                               : 0;
        if ( nPropCount > 0 )
        {
            aBuffer.append( "<prop>" );
            SerfPropName thePropName;
            for ( int theIndex = 0; theIndex < nPropCount; theIndex ++ )
            {
                // split fullname into namespace and name!
                DAVProperties::createSerfPropName( (*mpPropNames)[ theIndex ],
                                                   thePropName );

                /* <*propname* xmlns="*propns*" /> */
                aBuffer.append( "<" );
                aBuffer.append( thePropName.name );
                aBuffer.append( " xmlnx=\"" );
                aBuffer.append( thePropName.nspace );
                aBuffer.append( "\"/>" );
            }

            aBuffer.append( "</prop>" );
        }
        else
        {
            if ( mbOnlyPropertyNames )
            {
                aBuffer.append( "<propname/>" );
            }
            else
            {
                aBuffer.append( "<allprop/>" );
            }
        }

        aBuffer.append( PROPFIND_TRAILER );
        aBodyText = rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
        body_bkt = serf_bucket_simple_copy_create( aBodyText.getStr(),
                                                   aBodyText.getLength(),
                                                   pSerfBucketAlloc );
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PROPFIND",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc );
    handleChunkedEncoding(req_bkt, aBodyText.getLength());

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    if (hdrs_bkt != NULL)
    {
        // general header fields provided by caller
        setRequestHeaders( hdrs_bkt );

        // request specific header fields
        serf_bucket_headers_set( hdrs_bkt, "Depth", mDepthStr );
        if (hdrs_bkt!=NULL && body_bkt != 0 && aBodyText.getLength() > 0 )
        {
            serf_bucket_headers_set( hdrs_bkt, "Content-Type", "application/xml" );
        }
    }
    else
    {
        OSL_ASSERT("Headers Bucket missing");
    }

    return req_bkt;
}

void SerfPropFindReqProcImpl::processChunkOfResponseData( const char* data,
                                                          apr_size_t len )
{
    if ( xInputStream.is() )
    {
        xInputStream->AddToStream( data, len );
    }
}

void SerfPropFindReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    if ( mbOnlyPropertyNames )
    {
        const std::vector< DAVResourceInfo > rResInfo( parseWebDAVPropNameResponse( xInputStream.get() ) );
        *mpResInfo = rResInfo;
    }
    else
    {
        const std::vector< DAVResource > rResources( parseWebDAVPropFindResponse( xInputStream.get() ) );
        *mpResources = rResources;
    }
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
