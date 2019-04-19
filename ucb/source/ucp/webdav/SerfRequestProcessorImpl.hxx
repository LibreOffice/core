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

#pragma once

#include <serf.h>

#include <sal/types.h>
#include "DAVRequestEnvironment.hxx"

namespace http_dav_ucp
{

class SerfRequestProcessorImpl
{
public:
    SerfRequestProcessorImpl( const char* inPath,
                              const DAVRequestHeaders& inRequestHeaders );

    virtual ~SerfRequestProcessorImpl();

    /*pure*/ virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest ) = 0;

    bool processSerfResponseBucket( serf_request_t * inSerfRequest,

                                    serf_bucket_t * inSerfResponseBucket,
                                    apr_pool_t * inAprPool,
                                    apr_status_t & outStatus );

    void activateChunkedEncoding();

    /** Turn chunked encoding on or off, depending on the result of
        useChunkedEncoding().
    */
    void handleChunkedEncoding (
        serf_bucket_t* pRequestBucket,
        apr_int64_t nLength) const;

protected:
    void setRequestHeaders( serf_bucket_t* inoutSerfHeaderBucket );

    /*pure*/ virtual
    void processChunkOfResponseData( const char* data, apr_size_t len ) = 0;

    /*pure*/ virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket ) = 0;

    const char* getPathStr() const;
    bool useChunkedEncoding() const;

private:
    const char* mPathStr;
    const DAVRequestHeaders& mrRequestHeaders;
    bool mbUseChunkedEncoding;
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
