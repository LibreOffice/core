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

#include <drawpage.hxx>
#include <drwlayer.hxx>
#include <pageuno.hxx>

ScDrawPage::ScDrawPage(ScDrawLayer& rNewModel, bool bMasterPage)
:   FmFormPage(rNewModel, bMasterPage)
{
    SetSize( Size( SAL_MAX_INT32, SAL_MAX_INT32 ) );
        // largest size supported by sal_Int32 SdrPage::mnWidth/Height
}

ScDrawPage::~ScDrawPage()
{
}

rtl::Reference<SdrPage> ScDrawPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    ScDrawLayer& rScDrawLayer(static_cast< ScDrawLayer& >(rTargetModel));
    rtl::Reference<ScDrawPage> pClonedScDrawPage(
        new ScDrawPage(
            rScDrawLayer,
            IsMasterPage()));
    pClonedScDrawPage->FmFormPage::lateInit(*this);
    return pClonedScDrawPage;
}

css::uno::Reference< css::uno::XInterface > ScDrawPage::createUnoPage()
{
    return static_cast<cppu::OWeakObject*>( new ScPageObj( this ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
