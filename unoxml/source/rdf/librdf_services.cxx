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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include "CNodes.hxx"
#include "librdf_repository.hxx"


using namespace ::com::sun::star;


extern "C"
{

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_CBlankNode::_create,
      &comp_CBlankNode::_getImplementationName,
      &comp_CBlankNode::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, nullptr, 0 },
    { &comp_CURI::_create,
      &comp_CURI::_getImplementationName,
      &comp_CURI::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, nullptr, 0 },
    { &comp_CLiteral::_create,
      &comp_CLiteral::_getImplementationName,
      &comp_CLiteral::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, nullptr, 0 },
    { &comp_librdf_Repository::_create,
      &comp_librdf_Repository::_getImplementationName,
      &comp_librdf_Repository::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, nullptr, 0 },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

SAL_DLLPUBLIC_EXPORT void * SAL_CALL unordf_component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
