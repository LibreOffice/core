/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMessageDialog.hxx>
#include <QtInstanceMessageDialog.moc>

#include <QtInstanceButton.hxx>

#include <vcl/qt/QtUtils.hxx>

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QtInstanceMessageDialog::QtInstanceMessageDialog(QMessageBox* pMessageDialog)
    : QtInstanceDialog(pMessageDialog)
    , m_pMessageDialog(pMessageDialog)
{
    assert(m_pMessageDialog);

    m_pExtraControlsContainer = new QWidget;
    m_pExtraControlsContainer->setLayout(new QVBoxLayout);
    positionExtraControlsContainer();
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

    // update text and ensure that extra controls are contained in the
    // dialog's layout (new layout gets set when setting text)
    m_pMessageDialog->setText(toQString(rText));
    positionExtraControlsContainer();
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

    // update text and ensure that extra controls are contained in the
    // dialog's layout (new layout gets set when setting text)
    m_pMessageDialog->setInformativeText(toQString(rText));
    positionExtraControlsContainer();
}

std::unique_ptr<weld::Container> QtInstanceMessageDialog::weld_message_area()
{
    return std::make_unique<QtInstanceContainer>(m_pExtraControlsContainer);
}

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

std::unique_ptr<weld::Button> QtInstanceMessageDialog::weld_button_for_response(int nResponse)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        std::unique_ptr<weld::Button> xButton;
        rQtInstance.RunInMainThread([&] { xButton = weld_button_for_response(nResponse); });
        return xButton;
    }

    if (QPushButton* pButton = buttonForResponseCode(nResponse))
        return std::make_unique<QtInstanceButton>(pButton);

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

    // if a button was clicked, return its response code
    int nRet = m_pMessageDialog->exec();
    if (QAbstractButton* pClickedButton = m_pMessageDialog->clickedButton())
        return pClickedButton->property(PROPERTY_VCL_RESPONSE_CODE).toInt();

    return nRet;
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
void QtInstanceMessageDialog::positionExtraControlsContainer()
{
    assert(m_pExtraControlsContainer);

    // make use of implementation detail that QMessageBox uses QGridLayout for its layout
    // (logic here will need to be adjusted if that ever changes)
    QGridLayout* pDialogLayout = qobject_cast<QGridLayout*>(m_pMessageDialog->layout());
    assert(pDialogLayout && "QMessageBox has unexpected layout");

    // no need to reposition if layout didn't change
    if (pDialogLayout->indexOf(m_pExtraControlsContainer) >= 0)
        return;

    // find last label
    const int nItemCount = pDialogLayout->count();
    int nLastLabelIndex = -1;
    for (int i = nItemCount - 1; i >= 0; --i)
    {
        if (QLayoutItem* pItem = pDialogLayout->itemAt(i))
        {
            if (qobject_cast<QLabel*>(pItem->widget()))
            {
                nLastLabelIndex = i;
                break;
            }
        }
    }
    assert(nLastLabelIndex >= 0 && "didn't find label in message box");

    // shift everything after the last label down by one row
    for (int i = nLastLabelIndex + 1; i < nItemCount; ++i)
    {
        if (QLayoutItem* pItem = pDialogLayout->itemAt(i))
        {
            int nRow = 0;
            int nCol = 0;
            int nRowSpan = 0;
            int nColSpan = 0;
            pDialogLayout->getItemPosition(i, &nRow, &nCol, &nRowSpan, &nColSpan);
            pDialogLayout->removeItem(pItem);
            pDialogLayout->addItem(pItem, nRow + 1, nCol, nRowSpan, nColSpan);
        }
    }

    // insert an additional layout in the now empty row, underneath the last label
    int nLabelRow = 0;
    int nLabelCol = 0;
    int nLabelRowSpan = 0;
    int nLabelColSpan = 0;
    pDialogLayout->getItemPosition(nLastLabelIndex, &nLabelRow, &nLabelCol, &nLabelRowSpan,
                                   &nLabelColSpan);
    pDialogLayout->addWidget(m_pExtraControlsContainer, nLabelRow + 1, nLabelCol);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
