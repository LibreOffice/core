/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/ctrl.hxx>
#include <vcl/weld.hxx>
#include <swrect.hxx>

class SwEditWin;
class SwContentControl;

/// This button is shown when the cursor is inside a content control with drop-down capability.
class SwContentControlButton : public Control
{
public:
    SwContentControlButton(SwEditWin* pEditWin, const SwContentControl& rContentControl);
    virtual ~SwContentControlButton() override;
    virtual void dispose() override;

    void CalcPosAndSize(const SwRect& rPortionPaintArea);

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    DECL_LINK(PopupModeEndHdl, weld::Popover&, void);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual WindowHitTest ImplHitTest(const Point& rFramePos) override;

    virtual void LaunchPopup();
    virtual void DestroyPopup();

private:
    tools::Rectangle m_aFramePixel;

protected:
    const SwContentControl& m_rContentControl;
    std::unique_ptr<weld::Builder> m_xPopupBuilder;
    std::unique_ptr<weld::Popover> m_xPopup;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
