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

#include <SerfRequestProcessorImpl.hxx>

namespace http_dav_ucp
{

SerfRequestProcessorImpl::SerfRequestProcessorImpl( const char* inPath,
                                                    const DAVRequestHeaders& inRequestHeaders )
    : mPathStr( inPath )
    , mrRequestHeaders( inRequestHeaders )
    , mbUseChunkedEncoding( false )
{
}

SerfRequestProcessorImpl::~SerfRequestProcessorImpl()
{
}

const char* SerfRequestProcessorImpl::getPathStr() const
{
    return mPathStr;
}

void SerfRequestProcessorImpl::activateChunkedEncoding()
{
    mbUseChunkedEncoding = true;
}

const bool SerfRequestProcessorImpl::useChunkedEncoding() const
{
    return mbUseChunkedEncoding;
}

void SerfRequestProcessorImpl::setRequestHeaders( serf_bucket_t* inoutSerfHeaderBucket )
{
    DAVRequestHeaders::const_iterator aHeaderIter( mrRequestHeaders.begin() );
    const DAVRequestHeaders::const_iterator aEnd( mrRequestHeaders.end() );

    while ( aHeaderIter != aEnd )
    {
        const rtl::OString aHeader = rtl::OUStringToOString( (*aHeaderIter).first,
                                                               RTL_TEXTENCODING_UTF8 );
        const rtl::OString aValue = rtl::OUStringToOString( (*aHeaderIter).second,
                                                            RTL_TEXTENCODING_UTF8 );

        serf_bucket_headers_set( inoutSerfHeaderBucket,
                                 aHeader.getStr(),
                                 aValue.getStr() );

        ++aHeaderIter;
    }

    serf_bucket_headers_set( inoutSerfHeaderBucket, "Accept-Encoding", "gzip");
}

bool SerfRequestProcessorImpl::processSerfResponseBucket( serf_request_t * /*inSerfRequest*/,
                                                          serf_bucket_t * inSerfResponseBucket,
                                                          apr_pool_t * /*inAprPool*/,
                                                          apr_status_t & outStatus )
{
    const char* data;
    apr_size_t len;

    while (1) {
        outStatus = serf_bucket_read(inSerfResponseBucket, 8096, &data, &len);
        if (SERF_BUCKET_READ_ERROR(outStatus))
        {
            return true;
        }

        if ( len > 0 )
        {
            processChunkOfResponseData( data, len );
        }

        /* are we done yet? */
        if (APR_STATUS_IS_EOF(outStatus))
        {
            handleEndOfResponseData( inSerfResponseBucket );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
