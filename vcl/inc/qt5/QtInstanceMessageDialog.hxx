/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceButton.hxx"
#include "QtInstanceDialog.hxx"
#include <QtWidgets/QMessageBox>

class QtInstanceMessageDialog : public QtInstanceDialog, public virtual weld::MessageDialog
{
    Q_OBJECT

private:
    QMessageBox* m_pMessageDialog;

public:
    QtInstanceMessageDialog(QMessageBox* pMessageDialog);

    virtual void set_primary_text(const rtl::OUString& rText) override;

    virtual void set_secondary_text(const rtl::OUString& rText) override;

    virtual Container* weld_message_area() override;

    virtual OUString get_primary_text() const override;

    virtual OUString get_secondary_text() const override;

    // weld::Dialog overrides
    virtual void add_button(const OUString& rText, int nResponse,
                            const OUString& rHelpId = {}) override;
    virtual void set_default_response(int nResponse) override;
    QtInstanceButton* weld_widget_for_response(int nResponse) override;
    virtual int run() override;

private:
    virtual QPushButton* buttonForResponseCode(int nResponse);

protected slots:
    virtual void dialogFinished(int nResult) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
