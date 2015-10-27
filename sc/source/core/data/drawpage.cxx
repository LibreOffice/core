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

#include <sfx2/objsh.hxx>

#include "drawpage.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "pageuno.hxx"

// STATIC DATA -----------------------------------------------------------

ScDrawPage::ScDrawPage(ScDrawLayer& rNewModel, bool bMasterPage) :
    FmFormPage(rNewModel, bMasterPage)
{
    SetSize( Size( LONG_MAX, LONG_MAX ) );
}

ScDrawPage::ScDrawPage(const ScDrawPage& rSrcPage)
    : FmFormPage(rSrcPage)
{
}

ScDrawPage::~ScDrawPage()
{
}

ScDrawPage* ScDrawPage::Clone() const
{
    return Clone(0);
}

ScDrawPage* ScDrawPage::Clone(SdrModel* const pNewModel) const
{
    ScDrawPage* const pNewPage = new ScDrawPage(*this);
    FmFormModel* pScDrawModel = 0;
    if (pNewModel)
    {
        pScDrawModel = dynamic_cast<FmFormModel*>(pNewModel);
        assert(pScDrawModel);
    }
    pNewPage->lateInit(*this, pScDrawModel);
    return pNewPage;
}

css::uno::Reference< css::uno::XInterface > ScDrawPage::createUnoPage()
{
    return static_cast<cppu::OWeakObject*>( new ScPageObj( this ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
