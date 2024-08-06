/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

class QtInstanceMessageDialog : public QObject,
                                public QtInstanceDialog,
                                public virtual weld::MessageDialog
{
    Q_OBJECT;

private:
    QMessageBox* m_pMessageDialog;

    // the DialogController/Dialog/function passed to the runAsync variants
    std::shared_ptr<weld::DialogController> m_xRunAsyncDialogController;
    std::shared_ptr<weld::Dialog> m_xRunAsyncDialog;
    std::function<void(sal_Int32)> m_aRunAsyncFunc;

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
    virtual int run() override;
    virtual bool runAsync(std::shared_ptr<weld::DialogController> const& rxOwner,
                          const std::function<void(sal_Int32)>& func) override;
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func) override;
    virtual void response(int nResponse) override;

private:
    virtual QPushButton* buttonForResponseCode(int nResponse);

private slots:
    void dialogFinished(int nResult);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
