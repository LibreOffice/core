/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <WriterFilter.hxx>
#include <WriterFilterDetection.hxx>
#include <RtfFilter.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

/*-- 22.02.2007 12:19:23---------------------------------------------------

  -----------------------------------------------------------------------*/
WriterFilter::WriterFilter( const uno::Reference< uno::XComponentContext >& rxContext)  :
    m_xContext( rxContext )
{
}
/*-- 22.02.2007 12:19:23---------------------------------------------------

  -----------------------------------------------------------------------*/
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

void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_writeInfoHelper( xMgr, xRegistry, s_component_entries );
}

void * SAL_CALL component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );
}

} //extern "C"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
