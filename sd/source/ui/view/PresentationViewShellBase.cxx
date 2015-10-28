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

#include "PresentationViewShellBase.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "framework/FrameworkHelper.hxx"
#include "framework/PresentationModule.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

class DrawDocShell;

TYPEINIT1(PresentationViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new PresentationViewShellBase object has been constructed.

SfxViewFactory* PresentationViewShellBase::pFactory;
SfxViewShell* PresentationViewShellBase::CreateInstance (
    SfxViewFrame *_pFrame, SfxViewShell *pOldView)
{
    PresentationViewShellBase* pBase =
        new PresentationViewShellBase(_pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msPresentationViewURL);
    return pBase;
}
void PresentationViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,nPrio,"FullScreenPresentation");
    InitFactory();
}
void PresentationViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}

PresentationViewShellBase::PresentationViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_pFrame, pOldShell)
{
    // Hide the automatic (non-context sensitive) tool bars.
    Reference<beans::XPropertySet> xFrameSet (
        _pFrame->GetFrame().GetFrameInterface(),
        UNO_QUERY);
    if (xFrameSet.is())
    {
        Reference<beans::XPropertySet> xLayouterSet(xFrameSet->getPropertyValue("LayoutManager"), UNO_QUERY);
        if (xLayouterSet.is())
        {
            xLayouterSet->setPropertyValue("AutomaticToolbars", makeAny(sal_False));
        }
    }
}

PresentationViewShellBase::~PresentationViewShellBase()
{
}

void PresentationViewShellBase::InitializeFramework()
{
    css::uno::Reference<css::frame::XController>
        xController (GetController());
    sd::framework::PresentationModule::Initialize(xController);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
