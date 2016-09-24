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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_SERFPOSTREQPROCIMPL_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_SERFPOSTREQPROCIMPL_HXX

#include "SerfRequestProcessorImpl.hxx"

#include "SerfInputStream.hxx"
#include <com/sun/star/io/XOutputStream.hpp>

namespace http_dav_ucp
{

class SerfPostReqProcImpl : public SerfRequestProcessorImpl
{
public:
    SerfPostReqProcImpl( const char* inPath,
                         const DAVRequestHeaders& inRequestHeaders,
                         const char* inData,
                         apr_size_t inDataLen,
                         const char* inContentType,
                         const char* inReferer,
                         const css::uno::Reference< SerfInputStream > & xioInStrm );

    SerfPostReqProcImpl( const char* inPath,
                         const DAVRequestHeaders& inRequestHeaders,
                         const char* inData,
                         apr_size_t inDataLen,
                         const char* inContentType,
                         const char* inReferer,
                         const css::uno::Reference< css::io::XOutputStream > & xioOutStrm );

    virtual ~SerfPostReqProcImpl();

    virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest ) override;

protected:
    virtual
    void processChunkOfResponseData( const char* data, apr_size_t len ) override;

    virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket ) override;

private:
    const char* mpPostData;
    apr_size_t mnPostDataLen;
    const char* mpContentType;
    const char* mpReferer;
    css::uno::Reference< SerfInputStream > xInputStream;
    css::uno::Reference< css::io::XOutputStream > xOutputStream;

};

} // namespace http_dav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_SERFPOSTREQPROCIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
