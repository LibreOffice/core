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

#include <vcl/builderfactory.hxx>

#include <map>
#include <set>

#include "strings.hrc"
#include "dlgctrls.hxx"
#include "sdresid.hxx"
#include "fadedef.h"
#include "sdpage.hxx"

using namespace ::sd;

struct FadeEffectLBImpl
{
};

FadeEffectLB::FadeEffectLB(vcl::Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
    , mpImpl(new FadeEffectLBImpl)
{
}

FadeEffectLB::~FadeEffectLB()
{
    disposeOnce();
}

void FadeEffectLB::dispose()
{
    delete mpImpl;
    ListBox::dispose();
}

VCL_BUILDER_DECL_FACTORY(FadeEffectLB)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    rRet = VclPtr<FadeEffectLB>::Create(pParent, nBits);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
