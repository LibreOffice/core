/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SIDEBAR_PANELLAYOUT_HXX
#define INCLUDED_SVX_SIDEBAR_PANELLAYOUT_HXX

#include <svx/svxdllapi.h>

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SVX_DLLPUBLIC PanelLayout : public Control, public VclBuilderContainer
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;

private:
    Idle m_aPanelLayoutIdle;
    bool m_bInClose;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    DECL_DLLPRIVATE_LINK(ImplHandlePanelLayoutTimerHdl, Timer*, void);

public:
    PanelLayout(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
            const css::uno::Reference<css::frame::XFrame> &rFrame, bool bInterimBuilder = false);
    virtual ~PanelLayout() override;
    virtual void dispose() override;

    virtual Size GetOptimalSize() const override;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
