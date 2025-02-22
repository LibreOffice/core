/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWindow.hxx"

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>

class QtInstanceDialog : public QtInstanceWindow, public virtual weld::Dialog
{
    Q_OBJECT

    QDialog* m_pDialog;
    QWidget* m_pContentArea;

    // the DialogController/Dialog/function passed to the runAsync variants
    std::shared_ptr<weld::DialogController> m_xRunAsyncDialogController;
    std::shared_ptr<weld::Dialog> m_xRunAsyncDialog;
    std::function<void(sal_Int32)> m_aRunAsyncFunc;

public:
    QtInstanceDialog(QDialog* pDialog);
    ~QtInstanceDialog();

    virtual bool runAsync(const std::shared_ptr<weld::DialogController>& rxOwner,
                          const std::function<void(sal_Int32)>& func) override;

    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func) override;

    virtual void collapse(weld::Widget*, weld::Widget*) override;

    virtual void undo_collapse() override;

    virtual void
    SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&) override;

    virtual int run() override;

    virtual void response(int nResponse) override;

    virtual void add_button(const OUString&, int, const OUString& rHelpId = {}) override;

    virtual void set_modal(bool bModal) override;

    virtual bool get_modal() const override;

    virtual void set_centered_on_parent(bool bTrackGeometryRequests) override;

    virtual std::unique_ptr<weld::Button> weld_button_for_response(int) override;

    virtual void set_default_response(int) override;

    virtual std::unique_ptr<weld::Container> weld_content_area() override;

    static QDialogButtonBox* findButtonBox(QDialog* pDialog);
    static void handleButtonClick(QDialog& rDialog, QAbstractButton& rButton);

    /**
    * Name of the property to set on a QPushButton that holds the
    * int VCL response code of that button.
    */
    static const char* const PROPERTY_VCL_RESPONSE_CODE;

protected:
    static QPushButton* buttonForResponseCode(const QList<QAbstractButton*>& rButtons,
                                              int nResponse);

protected slots:
    virtual void dialogFinished(int nResult);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
