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

#include "ucpext_provider.hxx"

#include <cppuhelper/implementationentry.hxx>


namespace ucb::ucp::ext
{




    //= descriptors for the services implemented in this component

    struct ::cppu::ImplementationEntry const s_aServiceEntries[] =
    {
        {
            ContentProvider::Create,
            ContentProvider::getImplementationName_static,
            ContentProvider::getSupportedServiceNames_static,
            ::cppu::createOneInstanceComponentFactory, nullptr, 0
        },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
    };


}   // namespace ucb::ucp::ext


extern "C"
{

    SAL_DLLPUBLIC_EXPORT void * ucpext_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , ::ucb::ucp::ext::s_aServiceEntries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
