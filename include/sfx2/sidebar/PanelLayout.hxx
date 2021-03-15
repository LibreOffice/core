/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/dllapi.h>

#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SFX2_DLLPUBLIC PanelLayout : public Control
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    VclPtr<vcl::Window> m_xVclContentArea;
    std::unique_ptr<weld::Container> m_xContainer;
    weld::Widget* m_pInitialFocusWidget;

private:
    Idle m_aPanelLayoutIdle;
    bool m_bInClose;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    DECL_DLLPRIVATE_LINK(ImplHandlePanelLayoutTimerHdl, Timer*, void);

public:
    PanelLayout(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame> &rFrame);
    virtual ~PanelLayout() override;
    virtual void dispose() override;

    virtual Size GetOptimalSize() const override;
    virtual void GetFocus() override;
    virtual void setPosSizePixel(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
