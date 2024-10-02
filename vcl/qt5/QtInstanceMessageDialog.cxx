/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMessageDialog.hxx>
#include <QtInstanceMessageDialog.moc>

#include <QtWidgets/QPushButton>

QtInstanceMessageDialog::QtInstanceMessageDialog(QMessageBox* pMessageDialog)
    : QtInstanceDialog(pMessageDialog)
    , m_pMessageDialog(pMessageDialog)
{
    assert(m_pMessageDialog);
}

void QtInstanceMessageDialog::set_primary_text(const rtl::OUString& rText)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_primary_text(rText); });
        return;
    }

    m_pMessageDialog->setText(toQString(rText));
}

void QtInstanceMessageDialog::set_secondary_text(const rtl::OUString& rText)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_secondary_text(rText); });
        return;
    }

    m_pMessageDialog->setInformativeText(toQString(rText));
}

weld::Container* QtInstanceMessageDialog::weld_message_area() { return nullptr; }

OUString QtInstanceMessageDialog::get_primary_text() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    OUString sText;
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { sText = get_primary_text(); });
        return sText;
    }

    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->text());
}

OUString QtInstanceMessageDialog::get_secondary_text() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    OUString sText;
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { sText = get_secondary_text(); });
        return sText;
    }

    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->informativeText());
}

void QtInstanceMessageDialog::add_button(const OUString& rText, int nResponse, const OUString&)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { add_button(rText, nResponse); });
        return;
    }

    assert(m_pMessageDialog);
    QPushButton* pButton = m_pMessageDialog->addButton(vclToQtStringWithAccelerator(rText),
                                                       QMessageBox::ButtonRole::ActionRole);
    pButton->setProperty(PROPERTY_VCL_RESPONSE_CODE, QVariant::fromValue(nResponse));
}

void QtInstanceMessageDialog::set_default_response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_default_response(nResponse); });
        return;
    }

    assert(m_pMessageDialog);

    QPushButton* pButton = buttonForResponseCode(nResponse);
    if (pButton)
        m_pMessageDialog->setDefaultButton(pButton);
}

QtInstanceButton* QtInstanceMessageDialog::weld_widget_for_response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        QtInstanceButton* pButton;
        rQtInstance.RunInMainThread([&] { pButton = weld_widget_for_response(nResponse); });
        return pButton;
    }

    if (QPushButton* pButton = buttonForResponseCode(nResponse))
        return new QtInstanceButton(pButton);

    return nullptr;
}

int QtInstanceMessageDialog::run()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        int nRet = 0;
        rQtInstance.RunInMainThread([&] { nRet = run(); });
        return nRet;
    }

    m_pMessageDialog->exec();
    QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton();
    if (!pClickedButton)
        return RET_CLOSE;
    return pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();
}

void QtInstanceMessageDialog::response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { response(nResponse); });
        return;
    }

    assert(m_pMessageDialog);
    m_pMessageDialog->done(nResponse);
}

void QtInstanceMessageDialog::dialogFinished(int nResult)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { dialogFinished(nResult); });
        return;
    }

    // if a button was clicked, use its response code, otherwise the passed one
    int nResponseCode = nResult;
    if (QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton())
        nResponseCode = pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();

    QtInstanceDialog::dialogFinished(nResponseCode);
}

QPushButton* QtInstanceMessageDialog::buttonForResponseCode(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        QPushButton* pButton;
        rQtInstance.RunInMainThread([&] { pButton = buttonForResponseCode(nResponse); });
        return pButton;
    }

    assert(m_pMessageDialog);

    const QList<QAbstractButton*> aButtons = m_pMessageDialog->buttons();
    for (QAbstractButton* pAbstractButton : aButtons)
    {
        if (pAbstractButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt() == nResponse)
        {
            QPushButton* pButton = qobject_cast<QPushButton*>(pAbstractButton);
            assert(pButton);
            return pButton;
        }
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
