/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMessageDialog.hxx>

#include <QtWidgets/QPushButton>

/**
 * Name of the property to set on a QPushButton that holds the
 * VCL response code of that button.
 */
const char* const PROPERTY_VCL_RESPONSE_CODE = "response-code";

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
    QPushButton* pButton = m_pMessageDialog->addButton(vclToQtStringWithAccelerator(rText),
                                                       QMessageBox::ButtonRole::ActionRole);
    pButton->setProperty(PROPERTY_VCL_RESPONSE_CODE, QVariant::fromValue(nResponse));
}

void QtInstanceMessageDialog::set_default_response(int nResponse)
{
    assert(m_pMessageDialog);

    const QList<QAbstractButton*> aButtons = m_pMessageDialog->buttons();
    for (QAbstractButton* pAbstractButton : aButtons)
    {
        if (pAbstractButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt() == nResponse)
        {
            QPushButton* pButton = dynamic_cast<QPushButton*>(pAbstractButton);
            assert(pButton);
            m_pMessageDialog->setDefaultButton(pButton);
            return;
        }
    }
}

int QtInstanceMessageDialog::run()
{
    m_pMessageDialog->exec();
    QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton();
    if (!pClickedButton)
        return RET_CLOSE;
    return pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
