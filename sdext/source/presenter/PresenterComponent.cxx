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

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "PresenterProtocolHandler.hxx"
#include "PresenterScreen.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;

namespace sdext { namespace presenter {

static const struct ImplementationEntry gServiceEntries[] =
{
    {
        PresenterProtocolHandler::Create,
        PresenterProtocolHandler::getImplementationName_static,
        PresenterProtocolHandler::getSupportedServiceNames_static,
        createSingleComponentFactory, 0, 0
    },
    {
        PresenterScreenJob::Create,
        PresenterScreenJob::getImplementationName_static,
        PresenterScreenJob::getSupportedServiceNames_static,
        createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL presenter_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , gServiceEntries);
}

} } // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
