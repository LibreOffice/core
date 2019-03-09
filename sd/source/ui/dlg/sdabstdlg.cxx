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

#include <sdabstdlg.hxx>

#include <osl/module.hxx>
#include <rtl/ustring.hxx>

typedef SdAbstractDialogFactory* (*SdFuncPtrCreateDialogFactory)();

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

#else

extern "C" SdAbstractDialogFactory* SdCreateDialogFactory();

#endif

SdAbstractDialogFactory* SdAbstractDialogFactory::Create()
{
    SdFuncPtrCreateDialogFactory fp = nullptr;
#ifndef DISABLE_DYNLOADING
    static ::osl::Module aDialogLibrary;
    static const OUString sLibName(SDUI_DLL_NAME);
    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, sLibName ) )
        fp = reinterpret_cast<SdAbstractDialogFactory* (SAL_CALL*)()>(
            aDialogLibrary.getFunctionSymbol( "SdCreateDialogFactory" ));
#else
    fp = SdCreateDialogFactory;
#endif
    if ( fp )
        return fp();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
