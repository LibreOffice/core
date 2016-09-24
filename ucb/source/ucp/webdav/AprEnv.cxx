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

#include "AprEnv.hxx"

namespace apr_environment
{

AprEnv::AprEnv()
    : mpAprPool( 0 )
{
    apr_initialize();

    apr_pool_create(&mpAprPool, NULL);

    mpSerfLockStore = new http_dav_ucp::SerfLockStore();
}

AprEnv::~AprEnv()
{
    delete mpSerfLockStore;

    apr_pool_destroy(mpAprPool);

    apr_terminate();
}

/* static */
AprEnv* AprEnv::getAprEnv()
{
    static AprEnv rAprEnv;

    return &rAprEnv;
}

apr_pool_t* AprEnv::getAprPool()
{
    return mpAprPool;
}

http_dav_ucp::SerfLockStore* AprEnv::getSerfLockStore()
{
    return mpSerfLockStore;
}

} // namespace apr_environment

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
