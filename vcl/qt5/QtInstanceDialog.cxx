/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceDialog.hxx>

QtInstanceDialog::QtInstanceDialog(QDialog* pDialog)
    : QtInstanceWindow(pDialog)
    , m_pDialog(pDialog)
{
}

QtInstanceDialog::~QtInstanceDialog()
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    pQtInstance->RunInMainThread([&] { m_pDialog.reset(); });
}

bool QtInstanceDialog::runAsync(std::shared_ptr<Dialog> const&,
                                const std::function<void(sal_Int32)>&)
{
    return true;
}

bool QtInstanceDialog::runAsync(const std::shared_ptr<weld::DialogController>&,
                                const std::function<void(sal_Int32)>&)
{
    return true;
}

void QtInstanceDialog::collapse(weld::Widget*, weld::Widget*) {}

void QtInstanceDialog::undo_collapse() {}

void QtInstanceDialog::SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&)
{
}

int QtInstanceDialog::run()
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        int nResult = 0;
        pQtInstance->RunInMainThread([&] { nResult = run(); });
        return nResult;
    }

    return qtResponseTypeToVclResponseType(m_pDialog->exec());
}

void QtInstanceDialog::response(int) {}

void QtInstanceDialog::add_button(const OUString&, int, const OUString&) {}

void QtInstanceDialog::set_modal(bool bModal)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { set_modal(bModal); });
        return;
    }

    m_pDialog->setModal(bModal);
}

bool QtInstanceDialog::get_modal() const
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        bool bModal = false;
        pQtInstance->RunInMainThread([&] { bModal = get_modal(); });
        return bModal;
    }

    return m_pDialog->isModal();
}

weld::Button* QtInstanceDialog::weld_widget_for_response(int) { return nullptr; }

void QtInstanceDialog::set_default_response(int) {}

weld::Container* QtInstanceDialog::weld_content_area() { return nullptr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
