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

#include <NavigatorChildWindow.hxx>
#include <navigatr.hxx>
#include <app.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>

namespace sd {

static void RequestNavigatorUpdate (SfxBindings const * pBindings)
{
    if (pBindings != nullptr
        && pBindings->GetDispatcher() != nullptr)
    {
        SfxBoolItem aItem (SID_NAVIGATOR_INIT, true);
        pBindings->GetDispatcher()->ExecuteList(
            SID_NAVIGATOR_INIT,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aItem });
    }
}

SdNavigatorFloat::SdNavigatorFloat(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                                   vcl::Window* _pParent, SfxChildWinInfo* pInfo)
    : SfxNavigator(_pBindings, _pMgr, _pParent, pInfo)
    , m_xNavWin(std::make_unique<SdNavigatorWin>(m_xContainer.get(), _pBindings, this))
    , m_bSetInitialFocusOnActivate(true)
{
    m_xNavWin->SetUpdateRequestFunctor(
        [_pBindings] () { return RequestNavigatorUpdate(_pBindings); });

    SetMinOutputSizePixel(GetOptimalSize());
}

void SdNavigatorFloat::Activate()
{
    SfxNavigator::Activate();
    // tdf#141708 defer grabbing focus to preferred widget until the float is
    // first activated
    if (m_bSetInitialFocusOnActivate)
    {
        m_xNavWin->FirstFocus();
        m_bSetInitialFocusOnActivate = false;
    }
}

void SdNavigatorFloat::InitTreeLB(const SdDrawDocument* pDoc)
{
    m_xNavWin->InitTreeLB(pDoc);
}

void SdNavigatorFloat::FreshTree(const SdDrawDocument* pDoc)
{
    m_xNavWin->FreshTree(pDoc);
}

void SdNavigatorFloat::dispose()
{
    m_xNavWin.reset();
    SfxNavigator::dispose();
}

SdNavigatorFloat::~SdNavigatorFloat()
{
    disposeOnce();
}

SFX_IMPL_DOCKINGWINDOW(SdNavigatorWrapper, SID_NAVIGATOR);

SdNavigatorWrapper::SdNavigatorWrapper(vcl::Window *_pParent, sal_uInt16 nId,
                                       SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxNavigatorWrapper(_pParent, nId)
{
    SetWindow(VclPtr<SdNavigatorFloat>::Create(pBindings, this, _pParent, pInfo));
    Initialize();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
