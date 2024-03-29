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

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <framework/addonsoptions.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace framework
{
class FixedImageControl final : public InterimItemWindow
{
public:
    FixedImageControl(vcl::Window* pParent, const OUString& rCommand);
    virtual ~FixedImageControl() override;
    virtual void dispose() override;
    DECL_LINK(KeyInputHdl, const ::KeyEvent&, bool);

private:
    std::unique_ptr<weld::Image> m_xWidget;
};

FixedImageControl::FixedImageControl(vcl::Window* pParent, const OUString& rCommand)
    : InterimItemWindow(pParent, "svt/ui/fixedimagecontrol.ui", "FixedImageControl")
    , m_xWidget(m_xBuilder->weld_image("image"))
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_key_press(LINK(this, FixedImageControl, KeyInputHdl));

    bool bBigImages(SvtMiscOptions::AreCurrentSymbolsLarge());
    auto xImage
        = Graphic(AddonsOptions().GetImageFromURL(rCommand, bBigImages, true)).GetXGraphic();
    m_xWidget->set_image(xImage);

    SetSizePixel(get_preferred_size());
}

IMPL_LINK(FixedImageControl, KeyInputHdl, const ::KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

FixedImageControl::~FixedImageControl() { disposeOnce(); }

void FixedImageControl::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

FixedImageToolbarController::FixedImageToolbarController(
    const Reference<XComponentContext>& rxContext, const Reference<XFrame>& rFrame,
    ToolBox* pToolbar, ToolBoxItemId nID, const OUString& rCommand)
    : ComplexToolbarController(rxContext, rFrame, pToolbar, nID, rCommand)
    , m_eSymbolSize(SvtMiscOptions::GetCurrentSymbolsSize())
{
    m_pFixedImageControl = VclPtr<FixedImageControl>::Create(m_xToolbar, rCommand);
    m_xToolbar->SetItemWindow(m_nID, m_pFixedImageControl);

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

    const sal_Int16 eNewSymbolSize = SvtMiscOptions::GetCurrentSymbolsSize();

    if (m_eSymbolSize == eNewSymbolSize)
        return;

    m_eSymbolSize = eNewSymbolSize;

    // Refresh images if requested
    ::Size aSize(16, 16);
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
    m_pFixedImageControl->SetSizePixel(aSize);
}

IMPL_LINK_NOARG(FixedImageToolbarController, MiscOptionsChanged, LinkParamNone*, void)
{
    CheckAndUpdateImages();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
