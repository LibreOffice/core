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

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <WriterFilter.hxx>
#include <WriterFilterDetection.hxx>
#include <RtfFilter.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;



WriterFilter::WriterFilter( const uno::Reference< uno::XComponentContext >& rxContext)  :
    m_xContext( rxContext )
{
}


WriterFilter::~WriterFilter()
{
}

extern "C"
{
/* shared lib exports implemented with helpers */
static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    { WriterFilter_createInstance, WriterFilter_getImplementationName, WriterFilter_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 },
    { WriterFilterDetection_createInstance, WriterFilterDetection_getImplementationName, WriterFilterDetection_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0} ,
    { RtfFilter_createInstance, RtfFilter_getImplementationName, RtfFilter_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 } // terminate with NULL
};

SAL_DLLPUBLIC_EXPORT void * SAL_CALL writerfilter_component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );
}

} //extern "C"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
