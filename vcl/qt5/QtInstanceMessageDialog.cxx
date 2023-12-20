/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMessageDialog.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
