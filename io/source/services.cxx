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

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

#include "services.hxx"

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

static const struct ImplementationEntry g_entries[] =
{
    {
        io_acceptor::acceptor_CreateInstance, io_acceptor::acceptor_getImplementationName ,
        io_acceptor::acceptor_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {
        stoc_connector::connector_CreateInstance, stoc_connector::connector_getImplementationName ,
        stoc_connector::connector_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {
        io_stm::OPipeImpl_CreateInstance, io_stm::OPipeImpl_getImplementationName ,
        io_stm::OPipeImpl_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {
        io_stm::OPumpImpl_CreateInstance, io_stm::OPumpImpl_getImplementationName ,
        io_stm::OPumpImpl_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {
        io_stm::ODataInputStream_CreateInstance, io_stm::ODataInputStream_getImplementationName,
        io_stm::ODataInputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_stm::ODataOutputStream_CreateInstance, io_stm::ODataOutputStream_getImplementationName,
        io_stm::ODataOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_stm::OObjectInputStream_CreateInstance, io_stm::OObjectInputStream_getImplementationName,
        io_stm::OObjectInputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_stm::OObjectOutputStream_CreateInstance, io_stm::OObjectOutputStream_getImplementationName,
        io_stm::OObjectOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_stm::OMarkableInputStream_CreateInstance, io_stm::OMarkableInputStream_getImplementationName,
        io_stm::OMarkableInputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_stm::OMarkableOutputStream_CreateInstance, io_stm::OMarkableOutputStream_getImplementationName,
        io_stm::OMarkableOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    {
        io_TextInputStream::TextInputStream_CreateInstance, io_TextInputStream::TextInputStream_getImplementationName ,
        io_TextInputStream::TextInputStream_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {
        io_TextOutputStream::TextOutputStream_CreateInstance, io_TextOutputStream::TextOutputStream_getImplementationName ,
        io_TextOutputStream::TextOutputStream_getSupportedServiceNames, createSingleComponentFactory ,
        nullptr, 0
    },
    {nullptr, nullptr, nullptr, nullptr, nullptr, 0}
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL io_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
