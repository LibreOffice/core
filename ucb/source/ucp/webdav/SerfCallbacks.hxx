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


#ifndef INCLUDED_CALLBACKS_HXX
#define INCLUDED_CALLBACKS_HXX

#include <serf.h>

extern "C" apr_status_t Serf_ConnectSetup( apr_socket_t *skt,
                                           serf_bucket_t **read_bkt,
                                           serf_bucket_t **write_bkt,
                                           void *setup_baton,
                                           apr_pool_t *pool );

extern "C" apr_status_t Serf_Credentials( char **username,
                                          char **password,
                                          serf_request_t *request,
                                          void *baton,
                                          int code,
                                          const char *authn_type,
                                          const char *realm,
                                          apr_pool_t *pool );

extern "C" apr_status_t Serf_CertificateChainValidation(
    void* pSerfSession,
    int nFailures,
    const char** pCertificateChainBase64Encoded,
    int nCertificateChainLength);

extern "C" apr_status_t Serf_SetupRequest( serf_request_t *request,
                                           void *setup_baton,
                                           serf_bucket_t **req_bkt,
                                           serf_response_acceptor_t *acceptor,
                                           void **acceptor_baton,
                                           serf_response_handler_t *handler,
                                           void **handler_baton,
                                           apr_pool_t * pool );

extern "C" serf_bucket_t* Serf_AcceptResponse( serf_request_t *request,
                                               serf_bucket_t *stream,
                                               void *acceptor_baton,
                                               apr_pool_t *pool );

extern "C" apr_status_t Serf_HandleResponse( serf_request_t *request,
                                             serf_bucket_t *response,
                                             void *handler_baton,
                                             apr_pool_t *pool );

#endif // INCLUDED_CALLBACKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
