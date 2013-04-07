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


#include "swabstdlg.hxx"

#include <osl/module.hxx>
#include <tools/string.hxx>
#include <vcl/unohelp.hxx>

typedef SwAbstractDialogFactory* (SAL_CALL *SwFuncPtrCreateDialogFactory)();

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" SwAbstractDialogFactory* SwCreateDialogFactory();

#endif

SwAbstractDialogFactory* SwAbstractDialogFactory::Create()
{
#ifndef DISABLE_DYNLOADING
    SwFuncPtrCreateDialogFactory fp = 0;
    static ::osl::Module aDialogLibrary;
    static const OUString sLibName(::vcl::unohelper::CreateLibraryName("swui", sal_True));
    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, String( sLibName ),
                                                             SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY ) )
        fp = ( SwAbstractDialogFactory* (SAL_CALL*)() )
            aDialogLibrary.getFunctionSymbol( OUString("CreateDialogFactory"));
    if ( fp )
        return fp();
    return 0;
#else
    return SwCreateDialogFactory();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
