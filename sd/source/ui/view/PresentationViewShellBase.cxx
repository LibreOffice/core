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

#include <PresentationViewShellBase.hxx>
#include <DrawDocShell.hxx>
#include <DrawController.hxx>
#include <framework/FrameworkHelper.hxx>
#include <framework/PresentationModule.hxx>

#include <sfx2/viewfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

class DrawDocShell;


// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new PresentationViewShellBase object has been constructed.

SfxViewFactory* PresentationViewShellBase::s_pFactory;
SfxViewShell* PresentationViewShellBase::CreateInstance (
    SfxViewFrame& _rFrame, SfxViewShell *pOldView)
{
    PresentationViewShellBase* pBase =
        new PresentationViewShellBase(_rFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msPresentationViewURL);
    return pBase;
}
void PresentationViewShellBase::RegisterFactory( SfxInterfaceId nPrio )
{
    s_pFactory = new SfxViewFactory(
        &CreateInstance,nPrio,"FullScreenPresentation");
    InitFactory();
}
void PresentationViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}

PresentationViewShellBase::PresentationViewShellBase (
    SfxViewFrame& _rFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_rFrame, pOldShell)
{
    // Hide the automatic (non-context sensitive) tool bars.
    Reference<beans::XPropertySet> xFrameSet (
        _rFrame.GetFrame().GetFrameInterface(),
        UNO_QUERY);
    if (xFrameSet.is())
    {
        Reference<beans::XPropertySet> xLayouterSet(xFrameSet->getPropertyValue(u"LayoutManager"_ustr), UNO_QUERY);
        if (xLayouterSet.is())
        {
            xLayouterSet->setPropertyValue(u"AutomaticToolbars"_ustr, Any(false));
        }
    }
}

PresentationViewShellBase::~PresentationViewShellBase()
{
}

void PresentationViewShellBase::InitializeFramework()
{
    rtl::Reference<sd::DrawController> xController (GetDrawController());
    sd::framework::PresentationModule::Initialize(xController);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
