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

#include "connectiontools.hxx"
#include <comphelper/componentmodule.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL sdbt_component_getFactory(
                    const sal_Char* pImplementationName, SAL_UNUSED_PARAMETER void*, SAL_UNUSED_PARAMETER void*)
{
    Reference< XInterface > xRet;

    if (sdbtools::ConnectionTools::getImplementationName_static() == OUString::createFromAscii(pImplementationName))
    {
        xRet = ::cppu::createSingleComponentFactory(sdbtools::ConnectionTools::Create,
                    sdbtools::ConnectionTools::getImplementationName_static(),
                    sdbtools::ConnectionTools::getSupportedServiceNames_static(), nullptr);
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
