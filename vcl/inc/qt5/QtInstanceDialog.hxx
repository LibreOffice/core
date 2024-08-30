/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWindow.hxx"

class QtInstanceDialog : public QtInstanceWindow, public virtual weld::Dialog
{
    std::unique_ptr<QDialog> m_pDialog;

public:
    QtInstanceDialog(QDialog* pDialog);
    ~QtInstanceDialog();

    virtual bool runAsync(std::shared_ptr<Dialog> const&,
                          const std::function<void(sal_Int32)>&) override;

    virtual bool runAsync(const std::shared_ptr<weld::DialogController>&,
                          const std::function<void(sal_Int32)>&) override;

    virtual void collapse(weld::Widget*, weld::Widget*) override;

    virtual void undo_collapse() override;

    virtual void
    SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&) override;

    virtual int run() override;

    virtual void response(int) override;

    virtual void add_button(const OUString&, int, const OUString& rHelpId = {}) override;

    virtual void set_modal(bool bModal) override;

    virtual bool get_modal() const override;

    virtual weld::Button* weld_widget_for_response(int) override;

    virtual void set_default_response(int) override;

    virtual weld::Container* weld_content_area() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
