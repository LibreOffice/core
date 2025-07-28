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

#include <vcl/abstdlg.hxx>
#include <vcl/bitmapex.hxx>
#include <com/sun/star/cui/GetCreateDialogFactoryService.hpp>
#include <comphelper/processfactory.hxx>

typedef VclAbstractDialogFactory*(SAL_CALL* FuncPtrCreateDialogFactory)();

VclAbstractDialogFactory* VclAbstractDialogFactory::Create()
{
    auto xService
        = css::cui::GetCreateDialogFactoryService::create(comphelper::getProcessComponentContext());
    return reinterpret_cast<VclAbstractDialogFactory*>(xService->getSomething({}));
}

VclAbstractDialog::~VclAbstractDialog() {}

bool VclAbstractDialog::StartExecuteAsync(AsyncContext&)
{
    assert(false);
    return false;
}

std::vector<OUString> VclAbstractDialog::getAllPageUIXMLDescriptions() const
{
    // default has no pages
    return {};
}

bool VclAbstractDialog::selectPageByUIXMLDescription(const OUString& /*rUIXMLDescription*/)
{
    // default cannot select a page (which is okay, return true)
    return true;
}

Bitmap VclAbstractDialog::createScreenshot() const
{
    // default returns empty bitmap
    return Bitmap();
}

VclAbstractDialogFactory::~VclAbstractDialogFactory() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
