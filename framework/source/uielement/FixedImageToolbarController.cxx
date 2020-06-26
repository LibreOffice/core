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

#include <uielement/FixedImageToolbarController.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <framework/addonsoptions.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{
FixedImageToolbarController::FixedImageToolbarController(
    const Reference<XComponentContext>& rxContext, const Reference<XFrame>& rFrame,
    ToolBox* pToolbar, sal_uInt16 nID, const OUString& aCommand)
    : ComplexToolbarController(rxContext, rFrame, pToolbar, nID, aCommand)
    , m_eSymbolSize(SvtMiscOptions().GetCurrentSymbolsSize())
{
    m_pFixedImageControl = VclPtr<FixedImage>::Create(m_xToolbar, 0);
    m_xToolbar->SetItemWindow(m_nID, m_pFixedImageControl);

    bool bBigImages(SvtMiscOptions().AreCurrentSymbolsLarge());

    Image aImage(AddonsOptions().GetImageFromURL(aCommand, bBigImages, true));
    m_pFixedImageControl->SetImage(aImage);
    m_pFixedImageControl->SetSizePixel(m_pFixedImageControl->GetOptimalSize());

    SvtMiscOptions().AddListenerLink(LINK(this, FixedImageToolbarController, MiscOptionsChanged));
}

void SAL_CALL FixedImageToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    SvtMiscOptions().RemoveListenerLink(
        LINK(this, FixedImageToolbarController, MiscOptionsChanged));
    m_xToolbar->SetItemWindow(m_nID, nullptr);
    m_pFixedImageControl.disposeAndClear();
    ComplexToolbarController::dispose();
}

void FixedImageToolbarController::executeControlCommand(const css::frame::ControlCommand&) {}

void FixedImageToolbarController::CheckAndUpdateImages()
{
    SolarMutexGuard aSolarMutexGuard;

    SvtMiscOptions aMiscOptions;
    const sal_Int16 eNewSymbolSize = aMiscOptions.GetCurrentSymbolsSize();

    if (m_eSymbolSize == eNewSymbolSize)
        return;

    m_eSymbolSize = eNewSymbolSize;

    // Refresh images if requested
    auto aSize(m_pFixedImageControl->GetOptimalSize());
    if (m_eSymbolSize == SFX_SYMBOLS_SIZE_LARGE)
    {
        aSize.setWidth(26);
        aSize.setHeight(26);
    }
    else if (m_eSymbolSize == SFX_SYMBOLS_SIZE_32)
    {
        aSize.setWidth(32);
        aSize.setHeight(32);
    }
    else
    {
        aSize.setWidth(16);
        aSize.setHeight(16);
    }
    m_pFixedImageControl->SetSizePixel(aSize);
}

IMPL_LINK_NOARG(FixedImageToolbarController, MiscOptionsChanged, LinkParamNone*, void)
{
    CheckAndUpdateImages();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
