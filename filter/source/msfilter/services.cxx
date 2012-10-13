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

using ::rtl::OUString;
using namespace ::com::sun::star::uno;

// Declare static functions providing service information =====================

#define DECLARE_FUNCTIONS( className )                                                  \
extern OUString SAL_CALL className##_getImplementationName() throw();                   \
extern Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();    \
extern Reference< XInterface > SAL_CALL className##_createInstance(                     \
    const Reference< XComponentContext >& rxContext ) throw (Exception)

namespace ooo { namespace vba { DECLARE_FUNCTIONS( VBAMacroResolver ); } }

#undef DECLARE_FUNCTIONS

// ============================================================================

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 }

static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::ooo::vba::VBAMacroResolver ),
    { 0, 0, 0, 0, 0, 0 }
};

#undef IMPLEMENTATION_ENTRY

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL msfilter_component_getFactory( const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, spServices );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
