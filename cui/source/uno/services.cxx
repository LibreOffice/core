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

#include <com/sun/star/registry/XRegistryKey.hpp>
#include "sal/types.h"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implementationentry.hxx>

using namespace com::sun::star;

namespace cui {
extern OUString SAL_CALL ColorPicker_getImplementationName();
extern uno::Reference< uno::XInterface > SAL_CALL ColorPicker_createInstance( uno::Reference< uno::XComponentContext > const & ) SAL_THROW( (uno::Exception) );
extern uno::Sequence< OUString > SAL_CALL ColorPicker_getSupportedServiceNames() throw( uno::RuntimeException );
}

namespace
{
    cppu::ImplementationEntry entries[] = {
        { &::cui::ColorPicker_createInstance, &::cui::ColorPicker_getImplementationName, &::cui::ColorPicker_getSupportedServiceNames, &cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL cui_component_getFactory( char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
