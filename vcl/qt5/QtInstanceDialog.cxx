/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceDialog.hxx>
#include <QtInstanceDialog.moc>

#include <QtInstanceButton.hxx>

#include <vcl/help.hxx>

const char* const QtInstanceDialog::PROPERTY_VCL_RESPONSE_CODE = "response-code";

QtInstanceDialog::QtInstanceDialog(QDialog* pDialog)
    : QtInstanceWindow(pDialog)
    , m_pDialog(pDialog)
    , m_pContentArea(nullptr)
    , m_aRunAsyncFunc(nullptr)
{
    assert(m_pDialog);
}

QtInstanceDialog::~QtInstanceDialog()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pDialog->hide();
        m_pDialog->deleteLater();
    });
}

bool QtInstanceDialog::runAsync(const std::shared_ptr<weld::DialogController>& rxOwner,
                                const std::function<void(sal_Int32)>& func)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bRet = false;
        rQtInstance.RunInMainThread([&] { bRet = runAsync(rxOwner, func); });
        return bRet;
    }

    assert(m_pDialog);

    m_xRunAsyncDialogController = rxOwner;
    m_aRunAsyncFunc = func;
    connect(m_pDialog, &QDialog::finished, this, &QtInstanceDialog::dialogFinished);
    m_pDialog->open();

    return true;
}

bool QtInstanceDialog::runAsync(std::shared_ptr<Dialog> const& rxSelf,
                                const std::function<void(sal_Int32)>& func)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bRet;
        rQtInstance.RunInMainThread([&] { bRet = runAsync(rxSelf, func); });
        return bRet;
    }

    assert(m_pDialog);
    assert(rxSelf.get() == this);

    m_xRunAsyncDialog = rxSelf;
    m_aRunAsyncFunc = func;
    connect(m_pDialog, &QDialog::finished, this, &QtInstanceDialog::dialogFinished);
    m_pDialog->open();

    return true;
}

void QtInstanceDialog::collapse(weld::Widget*, weld::Widget*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceDialog::undo_collapse() { assert(false && "Not implemented yet"); }

void QtInstanceDialog::SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>&)
{
}

int QtInstanceDialog::run()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        int nResult = 0;
        rQtInstance.RunInMainThread([&] { nResult = run(); });
        return nResult;
    }

    return m_pDialog->exec();
}

void QtInstanceDialog::response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    rQtInstance.RunInMainThread([&] { m_pDialog->done(nResponse); });
}

void QtInstanceDialog::add_button(const OUString&, int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceDialog::set_modal(bool bModal)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_modal(bModal); });
        return;
    }

    m_pDialog->setModal(bModal);
}

bool QtInstanceDialog::get_modal() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        bool bModal = false;
        rQtInstance.RunInMainThread([&] { bModal = get_modal(); });
        return bModal;
    }

    return m_pDialog->isModal();
}

void QtInstanceDialog::set_centered_on_parent(bool)
{
    // QDialog is centered on parent toplevel by default
}

std::unique_ptr<weld::Button> QtInstanceDialog::weld_button_for_response(int nResponse)
{
    SolarMutexGuard g;

    QPushButton* pButton = nullptr;
    GetQtInstance().RunInMainThread([&] {
        if (QDialogButtonBox* pButtonBox = findButtonBox(m_pDialog))
        {
            const QList<QAbstractButton*> aButtons = pButtonBox->buttons();
            pButton = buttonForResponseCode(aButtons, nResponse);
        }
    });

    if (pButton)
        return std::make_unique<QtInstanceButton>(pButton);

    return nullptr;
}

void QtInstanceDialog::set_default_response(int) { assert(false && "Not implemented yet"); }

std::unique_ptr<weld::Container> QtInstanceDialog::weld_content_area()
{
    if (!m_pContentArea)
    {
        if (QBoxLayout* pBoxLayout = qobject_cast<QBoxLayout*>(m_pDialog->layout()))
        {
            // insert an extra widget and layout at beginning of the dialog's layout
            m_pContentArea = new QWidget;
            m_pContentArea->setLayout(new QVBoxLayout);
            pBoxLayout->insertWidget(0, m_pContentArea);
        }
        else
        {
            assert(false && "Dialog has layout that's not supported (yet)");
        }
    }

    return std::make_unique<QtInstanceContainer>(m_pContentArea);
}

void QtInstanceDialog::dialogFinished(int nResult)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { dialogFinished(nResult); });
        return;
    }

    disconnect(m_pDialog, &QDialog::finished, this, &QtInstanceDialog::dialogFinished);

    // use local variables for these, as members might have got de-allocated by the time they're reset
    std::shared_ptr<weld::Dialog> xRunAsyncDialog = m_xRunAsyncDialog;
    std::shared_ptr<weld::DialogController> xRunAsyncDialogController = m_xRunAsyncDialogController;
    std::function<void(sal_Int32)> aFunc = m_aRunAsyncFunc;
    m_aRunAsyncFunc = nullptr;
    m_xRunAsyncDialogController.reset();
    m_xRunAsyncDialog.reset();

    if (aFunc)
        aFunc(nResult);

    xRunAsyncDialogController.reset();
    xRunAsyncDialog.reset();
}

QDialogButtonBox* QtInstanceDialog::findButtonBox(QDialog* pDialog)
{
    assert(pDialog);
    QLayout* pLayout = pDialog->layout();
    if (!pLayout)
        return nullptr;

    for (int i = 0; i < pLayout->count(); i++)
    {
        QLayoutItem* pItem = pLayout->itemAt(i);
        if (QWidget* pItemWidget = pItem->widget())
        {
            if (QDialogButtonBox* pButtonBox = qobject_cast<QDialogButtonBox*>(pItemWidget))
                return pButtonBox;
        }
    }
    return nullptr;
}

QPushButton* QtInstanceDialog::buttonForResponseCode(const QList<QAbstractButton*>& rButtons,
                                                     int nResponse)
{
    for (QAbstractButton* pAbstractButton : rButtons)
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

void QtInstanceDialog::handleButtonClick(QDialog& rDialog, QAbstractButton& rButton)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { handleButtonClick(rDialog, rButton); });
        return;
    }

    // skip default handling if a custom click handler is set
    if (QtInstanceButton::hasCustomClickHandler(rButton))
        return;

    QVariant aResponseProperty = rButton.property(QtInstanceDialog::PROPERTY_VCL_RESPONSE_CODE);
    if (!aResponseProperty.isValid())
        return;

    assert(aResponseProperty.canConvert<int>());
    const int nResponseCode = aResponseProperty.toInt();

    // close dialog with button's response code unless it's the "Help" button
    if (nResponseCode != RET_HELP)
    {
        rDialog.done(nResponseCode);
        return;
    }

    // handle "Help" button
    Help* pHelp = Application::GetHelp();
    if (!pHelp)
        return;

    QtInstanceWidget aButtonWidget(&rButton);
    pHelp->Start(aButtonWidget.get_help_id(), &aButtonWidget);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
