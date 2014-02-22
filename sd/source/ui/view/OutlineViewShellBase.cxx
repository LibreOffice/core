/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "OutlineViewShellBase.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "framework/FrameworkHelper.hxx"

namespace sd {

class DrawDocShell;

TYPEINIT1(OutlineViewShellBase, ViewShellBase);




SfxViewFactory* OutlineViewShellBase::pFactory;
SfxViewShell* OutlineViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    OutlineViewShellBase* pBase = new OutlineViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msOutlineViewURL);
    return pBase;
}
void OutlineViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"Outline");
    InitFactory();
}
void OutlineViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}




OutlineViewShellBase::OutlineViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ImpressViewShellBase (_pFrame, pOldShell)
{
}




OutlineViewShellBase::~OutlineViewShellBase (void)
{
}




} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
