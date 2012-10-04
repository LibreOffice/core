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

#ifndef INCLUDED_SERFPUTREQPROCIMPL_HXX
#define INCLUDED_SERFPUTREQPROCIMPL_HXX

#include <SerfRequestProcessorImpl.hxx>

namespace http_dav_ucp
{

class SerfPutReqProcImpl : public SerfRequestProcessorImpl
{
public:
    SerfPutReqProcImpl( const char* inPath,
                        const DAVRequestHeaders& inRequestHeaders,
                        const char* inData,
                        apr_size_t inDataLen );


    virtual ~SerfPutReqProcImpl();

    virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest );

protected:
    virtual
    void processChunkOfResponseData( const char* data, apr_size_t len );

    virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket );

private:
    const char* mpData;
    apr_size_t mnDataLen;

};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFPUTREQPROCIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
