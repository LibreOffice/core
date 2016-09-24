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

#include <cppuhelper/implementationentry.hxx>

#include <services.hxx>

using namespace ::com::sun::star::uno;

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, nullptr, 0 }

//TODO: QuickDiagrammingImport and QuickDiagrammingLayout are not listed in
// oox/util/oox.component (and not directly referenced from anywhere in the code
// either); it is unclear whether they are dead code or whether
// a81327ff2faaf21c22f1a902bea170942d5207e6 "Import SmartArt graphics to
// Impress" would actually want to make use of them:
static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::oox::core::FastTokenHandler ),
    IMPLEMENTATION_ENTRY( ::oox::core::FilterDetect ),
    IMPLEMENTATION_ENTRY( ::oox::docprop::DocumentPropertiesImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::PowerPointImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::QuickDiagrammingImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::QuickDiagrammingLayout ),
    IMPLEMENTATION_ENTRY( ::oox::shape::ShapeContextHandler ),
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

#undef IMPLEMENTATION_ENTRY

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL oox_component_getFactory( const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, spServices );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
