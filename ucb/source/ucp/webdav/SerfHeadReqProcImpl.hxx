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

#ifndef INCLUDED_SERFHEADREQPROCIMPL_HXX
#define INCLUDED_SERFHEADREQPROCIMPL_HXX

#include <SerfRequestProcessorImpl.hxx>

#include <vector>
#include <rtl/ustring.hxx>
#include <DAVResource.hxx>

#include <SerfInputStream.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

namespace http_dav_ucp
{

class SerfHeadReqProcImpl : public SerfRequestProcessorImpl
{
public:
    SerfHeadReqProcImpl( const char* inPath,
                         const DAVRequestHeaders& inRequestHeaders,
                         const std::vector< ::rtl::OUString > & inHeaderNames,
                         DAVResource & ioResource );

    virtual ~SerfHeadReqProcImpl();

    virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest );

    void processSingleResponseHeader( const char* inHeaderName,
                                      const char* inHeaderValue );

protected:
    virtual
    void processChunkOfResponseData( const char* data, apr_size_t len );

    virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket );

private:
    const std::vector< ::rtl::OUString > * mpHeaderNames;
    DAVResource* mpResource;
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFHEADREQPROCIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
