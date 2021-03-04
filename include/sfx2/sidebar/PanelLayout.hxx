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

class DataChangedEvent;
class VclSimpleEvent;

/// This class is the base for the Widget Layout-based sidebar panels.
class SFX2_DLLPUBLIC PanelLayout
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    weld::Widget* m_pInitialFocusWidget;

    virtual void DataChanged(const DataChangedEvent& rEvent);

private:
    css::uno::Reference<css::frame::XFrame> mxFrame;

    DECL_LINK(DataChangedEventListener, VclSimpleEvent&, void);

public:
    PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame> &rFrame);
    virtual ~PanelLayout();

    Size get_preferred_size() const
    {
        return m_xContainer->get_preferred_size();
    }

    void queue_resize();

#if 0
    virtual Size GetOptimalSize() const override;
    virtual void GetFocus() override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
