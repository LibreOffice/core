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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_ACTCTRL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_ACTCTRL_HXX

#include <sfx2/InterimItemWindow.hxx>
#include <vcl/weld.hxx>
#include <swdllapi.h>

// numerical input
class NumEditAction final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::SpinButton> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    NumEditAction(vcl::Window* pParent);

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual ~NumEditAction() override
    {
        disposeOnce();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void connect_value_changed(const Link<weld::SpinButton&, void>& rLink)
    {
        m_xWidget->connect_value_changed(rLink);
    }

    int get_value() const
    {
        return m_xWidget->get_value();
    }

    void set_value(int nValue)
    {
        m_xWidget->set_value(nValue);
    }

    void set_accessible_name(const OUString& rName)
    {
        m_xWidget->set_accessible_name(rName);
    }

    void set_max(int nMax);

    void limitWidth();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
