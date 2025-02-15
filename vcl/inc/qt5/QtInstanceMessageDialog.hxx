/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceDialog.hxx"
#include <QtWidgets/QMessageBox>

class QtInstanceMessageDialog : public QtInstanceDialog, public virtual weld::MessageDialog
{
    Q_OBJECT

private:
    QMessageBox* m_pMessageDialog;
    // widget containing a layout to add additional items
    QWidget* m_pExtraControlsContainer;

public:
    QtInstanceMessageDialog(QMessageBox* pMessageDialog);

    virtual void set_primary_text(const rtl::OUString& rText) override;

    virtual void set_secondary_text(const rtl::OUString& rText) override;

    virtual std::unique_ptr<weld::Container> weld_message_area() override;

    virtual OUString get_primary_text() const override;

    virtual OUString get_secondary_text() const override;

    // weld::Dialog overrides
    virtual void add_button(const OUString& rText, int nResponse,
                            const OUString& rHelpId = {}) override;
    virtual void set_default_response(int nResponse) override;
    std::unique_ptr<weld::Button> weld_button_for_response(int nResponse) override;
    virtual int run() override;

    void addStandardButtons(VclButtonsType eButtonType);

private:
    void positionExtraControlsContainer();
    QPushButton* buttonForResponseCode(int nResponse);

protected slots:
    virtual void dialogFinished(int nResult) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
