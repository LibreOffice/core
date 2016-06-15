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

#include <config_features.h>

#include <rtl/ustring.hxx>
#include <osl/module.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/bitmap.hxx>

typedef VclAbstractDialogFactory* (SAL_CALL *FuncPtrCreateDialogFactory)();

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }
#else
extern "C" VclAbstractDialogFactory* CreateDialogFactory();
#endif

VclAbstractDialogFactory* VclAbstractDialogFactory::Create()
{
    FuncPtrCreateDialogFactory fp = nullptr;
#if HAVE_FEATURE_DESKTOP
#ifndef DISABLE_DYNLOADING
    static ::osl::Module aDialogLibrary;
    if (aDialogLibrary.is() ||
        aDialogLibrary.loadRelative(&thisModule, CUI_DLL_NAME,
                SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY))
    {
        fp = reinterpret_cast<VclAbstractDialogFactory* (SAL_CALL*)()>(
            aDialogLibrary.getFunctionSymbol( "CreateDialogFactory" ) );
    }
#else
    fp = CreateDialogFactory;
#endif
#endif
    if ( fp )
        return fp();
    return nullptr;
}

VclAbstractDialog::~VclAbstractDialog()
{
}

std::vector<OString> VclAbstractDialog::getAllPageUIXMLDescriptions() const
{
    // default has no pages
    return std::vector<OString>();
}

bool VclAbstractDialog::selectPageByUIXMLDescription(const OString& /*rUIXMLDescription*/)
{
    // default cannot select a page (which is okay, return true)
    return true;
}

Bitmap VclAbstractDialog::createScreenshot() const
{
    // default returns empty bitmap
    return Bitmap();
}

// virtual
VclAbstractDialog2::~VclAbstractDialog2()
{
}

VclAbstractDialogFactory::~VclAbstractDialogFactory()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
