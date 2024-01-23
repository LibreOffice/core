/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMessageDialog.hxx>

namespace
{
QMessageBox::ButtonRole lcl_vclResponseTypeToQtMessageBoxButtonRole(int nResponseType)
{
    // RET_CANCEL, RET_HELP, RET_YES, RET_NO and RET_OK have a matching equivalent
    // in Qt, the others are a bit more arbitrary; what really matters about these
    // is only that the mapping here and the other way around
    // (in lcl_qtMessageBoxButtonRoleToVclResponseType) is consistent
    switch (nResponseType)
    {
        case RET_CANCEL:
            return QMessageBox::ButtonRole::RejectRole;
        case RET_HELP:
            return QMessageBox::ButtonRole::HelpRole;
        case RET_YES:
            return QMessageBox::ButtonRole::YesRole;
        case RET_NO:
            return QMessageBox::ButtonRole::NoRole;
        case RET_OK:
            return QMessageBox::ButtonRole::AcceptRole;
        case RET_RETRY:
            return QMessageBox::ButtonRole::ResetRole;
        case RET_IGNORE:
            return QMessageBox::ButtonRole::ActionRole;
        case RET_CLOSE:
            return QMessageBox::ButtonRole::DestructiveRole;
        default:
            assert(false && "Unhandled vcl response type");
            return QMessageBox::InvalidRole;
    }
}

VclResponseType lcl_qtMessageBoxButtonRoleToVclResponseType(int nRet)
{
    // AcceptRole, HelpRole, NoRole, RejectRole and YesRole have a matching equivalent
    // in VCL, the others are a bit more arbitrary; what really matters about these
    // is only that the mapping here and the other way around
    // (in lcl_vclResponseTypeToQtMessageBoxButtonRole) is consistent
    switch (nRet)
    {
        case QMessageBox::ButtonRole::AcceptRole:
            return RET_OK;
        case QMessageBox::ButtonRole::HelpRole:
            return RET_HELP;
        case QMessageBox::ButtonRole::NoRole:
            return RET_NO;
        case QMessageBox::ButtonRole::RejectRole:
            return RET_CANCEL;
        case QMessageBox::ButtonRole::YesRole:
            return RET_YES;
        case QMessageBox::ButtonRole::ResetRole:
            return RET_RETRY;
        case QMessageBox::ButtonRole::ActionRole:
            return RET_IGNORE;
        case QMessageBox::ButtonRole::DestructiveRole:
            return RET_CLOSE;
        default:
            assert(false && "Unhandled QMessageBox::ButtonRole");
            return RET_CANCEL;
    }
}
}

QtInstanceMessageDialog::QtInstanceMessageDialog(QMessageBox* pMessageDialog)
    : QtInstanceDialog(pMessageDialog)
    , m_pMessageDialog(pMessageDialog)
{
}

void QtInstanceMessageDialog::set_primary_text(const rtl::OUString& rText)
{
    m_pMessageDialog->setText(toQString(rText));
}

void QtInstanceMessageDialog::set_secondary_text(const rtl::OUString& rText)
{
    m_pMessageDialog->setInformativeText(toQString(rText));
}

weld::Container* QtInstanceMessageDialog::weld_message_area() { return nullptr; }

OUString QtInstanceMessageDialog::get_primary_text() const
{
    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->text());
}

OUString QtInstanceMessageDialog::get_secondary_text() const
{
    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->informativeText());
}

void QtInstanceMessageDialog::add_button(const OUString& rText, int nResponse, const OUString&)
{
    assert(m_pMessageDialog);
    m_pMessageDialog->addButton(vclToQtStringWithAccelerator(rText),
                                lcl_vclResponseTypeToQtMessageBoxButtonRole(nResponse));
}

int QtInstanceMessageDialog::run()
{
    // cannot use the QMessageBox::exec() return value right away, because it returns the
    // QMessageBox::StandardButton value or an opaque value, so use the clicked
    // button to retrieve its role instead and map that to the VCL response
    m_pMessageDialog->exec();
    QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton();
    if (!pClickedButton)
        return RET_CLOSE;
    return lcl_qtMessageBoxButtonRoleToVclResponseType(
        m_pMessageDialog->buttonRole(pClickedButton));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
