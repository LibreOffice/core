/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/tabctrl.hxx>

#include <com/sun/star/frame/XFrame.hpp>

namespace com::sun::star::uno { class XComponentContext; }
class ChangedUIEventListener;

class NotebookbarTabControl final : public NotebookbarTabControlBase
{
friend class ChangedUIEventListener;

public:
    NotebookbarTabControl( Window* pParent );
    ~NotebookbarTabControl() override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool EventNotify( NotifyEvent& rNEvt ) override;
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual Size calculateRequisition() const override;

private:
    static void FillShortcutsToolBox(css::uno::Reference<css::uno::XComponentContext> const & xContext,
                                          const css::uno::Reference<css::frame::XFrame>& xFrame,
                                          const OUString& aModuleName,
                                          ToolBox* pShortcuts
                             );
    void ArrowStops( sal_uInt16 nCode );

    DECL_LINK(OpenNotebookbarPopupMenu, NotebookBar*, void);

    rtl::Reference<ChangedUIEventListener> m_pListener;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    bool m_bInitialized;
    bool m_bInvalidate;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
