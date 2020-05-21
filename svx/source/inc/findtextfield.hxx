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

#include <vcl/InterimItemWindow.hxx>

namespace svt
{
class AcceleratorExecute;
}

class FindTextFieldControl final : public InterimItemWindow
{
public:
    FindTextFieldControl(vcl::Window* pParent,
                         css::uno::Reference<css::frame::XFrame> const& xFrame,
                         const css::uno::Reference<css::uno::XComponentContext>& xContext);

    virtual void dispose() override;

    virtual ~FindTextFieldControl() override;

    virtual void GetFocus() override;

    void Remember_Impl(const OUString& rStr);
    void SetTextToSelected_Impl();

    void connect_changed(const Link<weld::ComboBox&, void>& rLink);

    int get_count() const;
    OUString get_text(int nIndex) const;
    OUString get_active_text() const;
    void append_text(const OUString& rText);
    void set_entry_message_type(weld::EntryMessageType eType);

private:
    ImplSVEvent* m_nAsyncGetFocusId;
    std::unique_ptr<weld::ComboBox> m_xWidget;
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    std::unique_ptr<svt::AcceleratorExecute> m_pAcc;
    Link<weld::ComboBox&, void> m_aChangeHdl;

    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(OnAsyncGetFocus, void*, void);

    void ActivateFind(bool bShift);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
