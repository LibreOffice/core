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

#include <vcl/weld.hxx>

#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SFX2_DLLPUBLIC PanelLayout
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    weld::Widget* m_pInitialFocusWidget;

private:
    css::uno::Reference<css::frame::XFrame> mxFrame;

public:
    PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame> &rFrame);
    virtual ~PanelLayout();

    Size get_preferred_size() const
    {
        return m_xContainer->get_preferred_size();
    }
#if 0
    virtual void dispose() override;

    virtual Size GetOptimalSize() const override;
    virtual void GetFocus() override;
    virtual void setPosSizePixel(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
