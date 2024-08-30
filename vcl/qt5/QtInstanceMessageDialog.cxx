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

/**
 * Name of the property to set on a QPushButton that holds the
 * VCL response code of that button.
 */
const char* const PROPERTY_VCL_RESPONSE_CODE = "response-code";

QtInstanceMessageDialog::QtInstanceMessageDialog(QMessageBox* pMessageDialog)
    : QtInstanceDialog(pMessageDialog)
    , m_pMessageDialog(pMessageDialog)
    , m_aRunAsyncFunc(nullptr)
{
    assert(m_pMessageDialog);
}

void QtInstanceMessageDialog::set_primary_text(const rtl::OUString& rText)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { set_primary_text(rText); });
        return;
    }

    m_pMessageDialog->setText(toQString(rText));
}

void QtInstanceMessageDialog::set_secondary_text(const rtl::OUString& rText)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { set_secondary_text(rText); });
        return;
    }

    m_pMessageDialog->setInformativeText(toQString(rText));
}

weld::Container* QtInstanceMessageDialog::weld_message_area() { return nullptr; }

OUString QtInstanceMessageDialog::get_primary_text() const
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    OUString sText;
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { sText = get_primary_text(); });
        return sText;
    }

    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->text());
}

OUString QtInstanceMessageDialog::get_secondary_text() const
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    OUString sText;
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { sText = get_secondary_text(); });
        return sText;
    }

    assert(m_pMessageDialog);
    return toOUString(m_pMessageDialog->informativeText());
}

void QtInstanceMessageDialog::add_button(const OUString& rText, int nResponse, const OUString&)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { add_button(rText, nResponse); });
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
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { set_default_response(nResponse); });
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
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        QtInstanceButton* pButton;
        pQtInstance->RunInMainThread([&] { pButton = weld_widget_for_response(nResponse); });
        return pButton;
    }

    if (QPushButton* pButton = buttonForResponseCode(nResponse))
        return new QtInstanceButton(pButton);

    return nullptr;
}

int QtInstanceMessageDialog::run()
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        int nRet = 0;
        pQtInstance->RunInMainThread([&] { nRet = run(); });
        return nRet;
    }

    m_pMessageDialog->exec();
    QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton();
    if (!pClickedButton)
        return RET_CLOSE;
    return pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();
}

bool QtInstanceMessageDialog::runAsync(const std::shared_ptr<weld::DialogController>& rxOwner,
                                       const std::function<void(sal_Int32)>& func)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        bool bRet = false;
        pQtInstance->RunInMainThread([&] { bRet = runAsync(rxOwner, func); });
        return bRet;
    }

    assert(m_pMessageDialog);

    m_xRunAsyncDialogController = rxOwner;
    m_aRunAsyncFunc = func;
    connect(m_pMessageDialog, &QDialog::finished, this, &QtInstanceMessageDialog::dialogFinished);
    m_pMessageDialog->open();

    return true;
}

bool QtInstanceMessageDialog::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                                       const std::function<void(sal_Int32)>& func)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        bool bRet;
        pQtInstance->RunInMainThread([&] { bRet = runAsync(rxSelf, func); });
        return bRet;
    }

    assert(m_pMessageDialog);
    assert(rxSelf.get() == this);

    m_xRunAsyncDialog = rxSelf;
    m_aRunAsyncFunc = func;
    connect(m_pMessageDialog, &QDialog::finished, this, &QtInstanceMessageDialog::dialogFinished);
    m_pMessageDialog->open();

    return true;
}

void QtInstanceMessageDialog::response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { response(nResponse); });
        return;
    }

    assert(m_pMessageDialog);
    m_pMessageDialog->done(nResponse);
}

void QtInstanceMessageDialog::dialogFinished(int nResult)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { dialogFinished(nResult); });
        return;
    }

    assert(m_aRunAsyncFunc);

    disconnect(m_pMessageDialog, &QDialog::finished, this,
               &QtInstanceMessageDialog::dialogFinished);

    // use local variables for these, as members might have got de-allocated by the time they're reset
    std::shared_ptr<weld::Dialog> xRunAsyncDialog = m_xRunAsyncDialog;
    std::shared_ptr<weld::DialogController> xRunAsyncDialogController = m_xRunAsyncDialogController;
    std::function<void(sal_Int32)> aFunc = m_aRunAsyncFunc;
    m_aRunAsyncFunc = nullptr;
    m_xRunAsyncDialogController.reset();
    m_xRunAsyncDialog.reset();

    // if a button was clicked, use its response code, otherwise the passed one
    int nRet = nResult;
    if (QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton())
        nRet = pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();

    aFunc(nRet);

    xRunAsyncDialogController.reset();
    xRunAsyncDialog.reset();
}

QPushButton* QtInstanceMessageDialog::buttonForResponseCode(int nResponse)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        QPushButton* pButton;
        pQtInstance->RunInMainThread([&] { pButton = buttonForResponseCode(nResponse); });
        return pButton;
    }

    assert(m_pMessageDialog);

    const QList<QAbstractButton*> aButtons = m_pMessageDialog->buttons();
    for (QAbstractButton* pAbstractButton : aButtons)
    {
        if (pAbstractButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt() == nResponse)
        {
            QPushButton* pButton = dynamic_cast<QPushButton*>(pAbstractButton);
            assert(pButton);
            return pButton;
        }
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
