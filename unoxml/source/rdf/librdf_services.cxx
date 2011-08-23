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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include "CNodes.hxx"
#include "librdf_repository.hxx"


using namespace ::com::sun::star;


extern "C"
{

void SAL_CALL
component_getImplementationEnvironment(const sal_Char **o_ppEnvironmentTypeName,
    uno_Environment ** /* ppEnvironment */)
{
    *o_ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_CBlankNode::_create,
      &comp_CBlankNode::_getImplementationName,
      &comp_CBlankNode::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_CURI::_create,
      &comp_CURI::_getImplementationName,
      &comp_CURI::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_CLiteral::_create,
      &comp_CLiteral::_getImplementationName,
      &comp_CLiteral::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_librdf_Repository::_create,
      &comp_librdf_Repository::_getImplementationName,
      &comp_librdf_Repository::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C" void * SAL_CALL component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return ::cppu::component_writeInfoHelper(serviceManager, registryKey,
        entries);
}

} // extern "C"

