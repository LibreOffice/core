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

#include "SlideSorterViewShellBase.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "framework/FrameworkHelper.hxx"

namespace sd {

class DrawDocShell;

TYPEINIT1(SlideSorterViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new SlideSorterViewShellBase object has been constructed.

SfxViewFactory* SlideSorterViewShellBase::pFactory;

SfxViewShell* SlideSorterViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    SlideSorterViewShellBase* pBase = new SlideSorterViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msSlideSorterURL);
    return pBase;
}

void SlideSorterViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"SlideSorter");
    InitFactory();
}

void SlideSorterViewShellBase::InitFactory() { SFX_VIEW_REGISTRATION(DrawDocShell); }

TYPEINIT1(SlideJockeyViewShellBase, ViewShellBase);

SfxViewFactory* SlideJockeyViewShellBase::pFactory;

SfxViewShell* SlideJockeyViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    SlideJockeyViewShellBase* pBase = new SlideJockeyViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msJockeySorterURL);
    return pBase;
}

void SlideJockeyViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"SlideJockey");
    InitFactory();
}

void SlideJockeyViewShellBase::InitFactory() { SFX_VIEW_REGISTRATION(DrawDocShell); }

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
