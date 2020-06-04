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

#include <scabstdlg.hxx>

#include <osl/module.hxx>
#include <tools/svlibrary.h>
#include <rtl/ustrbuf.hxx>

typedef ScAbstractDialogFactory* (*ScFuncPtrCreateDialogFactory)();

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

#else

extern "C" ScAbstractDialogFactory* ScCreateDialogFactory();

#endif

ScAbstractDialogFactory* ScAbstractDialogFactory::Create()
{
    ScFuncPtrCreateDialogFactory fp = nullptr;
#ifndef DISABLE_DYNLOADING
    static ::osl::Module aDialogLibrary;

    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, SVLIBRARY("scui"),
                                                             SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY ) )
        fp = reinterpret_cast<ScAbstractDialogFactory* (SAL_CALL*)()>(
            aDialogLibrary.getFunctionSymbol( "ScCreateDialogFactory" ));
#else
    fp = ScCreateDialogFactory;
#endif
    if ( fp )
        return fp();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
