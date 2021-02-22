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

#pragma once

#include <config_options.h>
#include <svtools/svtdllapi.h>

#include <memory>

#include <rtl/ref.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/weld.hxx>

namespace com :: sun :: star :: frame { class XFrame; }
namespace com :: sun :: star :: frame { struct FeatureStateEvent; }
namespace svt { class FrameStatusListener; }

class SVT_DLLPUBLIC WeldToolbarPopup
{
private:
    DECL_LINK(FocusHdl, weld::Widget&, void);

protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xTopLevel;
    std::unique_ptr<weld::Container> m_xContainer;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    rtl::Reference<svt::FrameStatusListener> m_xStatusListener;

public:
    WeldToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame,
                     weld::Widget* pParent, const OUString& rUIFile, const OString& rId);
    virtual ~WeldToolbarPopup();
    weld::Container* getTopLevel() { return m_xTopLevel.get(); }
    weld::Container* getContainer() { return m_xContainer.get(); }
    void AddStatusListener(const OUString& rCommandURL);

    // Forwarded from XStatusListener (subclasses must override this one to get the status updates):
    /// @throws css::uno::RuntimeException
    virtual void statusChanged(const css::frame::FeatureStateEvent& Event);
    virtual void GrabFocus() = 0;
};

// we want to create WeldToolbarPopup on-demand when a toolbar dropdown is
// clicked, but the widget to be shown must exist before the dropdown
// is activated, so ToolbarPopupContainer is that widget and the
// contents of the on-demand created WeldToolbarPopup is placed
// within the ToolbarPopupContainer
class SVT_DLLPUBLIC ToolbarPopupContainer final
{
private:
    DECL_LINK(FocusHdl, weld::Widget&, void);

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xTopLevel;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<WeldToolbarPopup> m_xPopup;
public:
    ToolbarPopupContainer(weld::Widget* pParent);
    ~ToolbarPopupContainer();
    weld::Container* getTopLevel() { return m_xTopLevel.get(); }

    void setPopover(std::unique_ptr<WeldToolbarPopup> xPopup);
    WeldToolbarPopup* getPopover() const { return m_xPopup.get(); }
    void unsetPopover();
};

class SVT_DLLPUBLIC InterimToolbarPopup final : public DropdownDockingWindow
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<WeldToolbarPopup> m_xPopup;
public:
    InterimToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParent,
                        std::unique_ptr<WeldToolbarPopup> xPopup, bool bTearable = false);
    virtual void dispose() override;
    virtual ~InterimToolbarPopup() override;

    virtual void GetFocus() override;

    void EndPopupMode();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
