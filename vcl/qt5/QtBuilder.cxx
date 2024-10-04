/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtBuilder.hxx>

#include <QtInstanceMessageDialog.hxx>
#include <QtTools.hxx>

#include <rtl/ustrbuf.hxx>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>

namespace
{
QString convertAccelerator(const OUString& rText)
{
    // preserve literal '&'s and use '&' instead of '_' for the accelerator
    return toQString(rText.replaceAll("&", "&&").replace('_', '&'));
}
}

QtBuilder::QtBuilder(QObject* pParent, std::u16string_view sUIRoot, const OUString& rUIFile)
    : WidgetBuilder(sUIRoot, rUIFile, false)
{
    processUIFile(pParent);
}

QtBuilder::~QtBuilder() {}

QObject* QtBuilder::get_by_name(std::u16string_view sID)
{
    for (auto const& child : m_aChildren)
    {
        if (child.m_sID == sID)
            return child.m_pWindow;
    }

    return nullptr;
}

void QtBuilder::insertComboBoxOrListBoxItems(QObject*, stringmap&,
                                             const std::vector<ComboBoxTextItem>&)
{
    assert(false && "comboboxes and list boxes are not supported yet");
}

QObject* QtBuilder::insertObject(QObject* pParent, const OUString& rClass, const OUString& rID,
                                 stringmap& rProps, stringmap&, stringmap&)
{
    QObject* pCurrentChild = nullptr;

    pCurrentChild = makeObject(pParent, rClass, rID, rProps);

    setProperties(pCurrentChild, rProps);

    rProps.clear();

    return pCurrentChild;
}

QObject* QtBuilder::makeObject(QObject* pParent, std::u16string_view sName, const OUString& sID,
                               stringmap& rMap)
{
    // ignore placeholders
    if (sName.empty())
        return nullptr;

    QWidget* pParentWidget = qobject_cast<QWidget*>(pParent);
    QLayout* pParentLayout = qobject_cast<QLayout*>(pParent);

    QObject* pObject = nullptr;

    if (sName == u"GtkMessageDialog")
    {
        pObject = new QMessageBox(pParentWidget);
    }
    else if (sName == u"GtkBox")
    {
        // for a QMessageBox, return the existing layout instead of creating a new one
        if (QMessageBox* pMessageBox = qobject_cast<QMessageBox*>(pParent))
        {
            pObject = pMessageBox->layout();
            assert(pObject && "QMessageBox has no layout");
        }
        else
        {
            const bool bVertical = hasOrientationVertical(rMap);
            if (bVertical)
                pObject = new QVBoxLayout(pParentWidget);
            else
                pObject = new QHBoxLayout(pParentWidget);
        }
    }
    else if (sName == u"GtkButtonBox")
    {
        QWidget* pTopLevel = windowForObject(pParent);
        if (QMessageBox* pMessageBox = qobject_cast<QMessageBox*>(pTopLevel))
        {
            // for a QMessageBox, return the existing button box instead of creating a new one
            QDialogButtonBox* pButtonBox = findButtonBox(pMessageBox);
            assert(pButtonBox && "Could not find QMessageBox's button box");
            pObject = pButtonBox;

            // skip adding to layout below, button box is already contained in dialog
            pParentLayout = nullptr;
        }
        else
        {
            pObject = new QDialogButtonBox(pParentWidget);
        }
    }
    else if (sName == u"GtkButton")
    {
        if (QDialogButtonBox* pButtonBox = qobject_cast<QDialogButtonBox*>(pParentWidget))
        {
            pObject = pButtonBox->addButton("", QDialogButtonBox::NoRole);

            // for message boxes, avoid implicit standard buttons in addition to those explicitly added
            if (QMessageBox* pMessageBox = qobject_cast<QMessageBox*>(pParentWidget->window()))
                pMessageBox->setStandardButtons(QMessageBox::NoButton);
        }
        else
        {
            pObject = new QPushButton(pParentWidget);
        }
    }
    else if (sName == u"GtkCheckButton")
    {
        pObject = new QCheckBox(pParentWidget);
    }
    else if (sName == u"GtkDialog")
    {
        pObject = new QDialog(pParentWidget);
    }
    else if (sName == u"GtkEntry")
    {
        pObject = new QLineEdit(pParentWidget);
    }
    else if (sName == u"GtkFrame")
    {
        pObject = new QGroupBox(pParentWidget);
    }
    else if (sName == u"GtkLabel")
    {
        extractMnemonicWidget(sID, rMap);
        pObject = new QLabel(pParentWidget);
    }
    else if (sName == u"GtkScrolledWindow")
    {
        pObject = new QScrollArea(pParentWidget);
    }
    else if (sName == u"GtkTextView")
    {
        pObject = new QPlainTextEdit(pParentWidget);
    }
    else
    {
        SAL_WARN("vcl.qt", "Widget type not supported yet: "
                               << OUStringToOString(sName, RTL_TEXTENCODING_UTF8));
        assert(false && "Widget type not supported yet");
    }

    if (QWidget* pWidget = qobject_cast<QWidget*>(pObject))
    {
        // add widget to parent layout
        if (pParentLayout)
            pParentLayout->addWidget(pWidget);

        QtInstanceWidget::setHelpId(*pWidget, getHelpRoot() + sID);

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        // Set GtkBuilder ID as accessible ID
        pWidget->setAccessibleIdentifier(toQString(sID));
#endif
    }
    else if (QLayout* pLayout = qobject_cast<QLayout*>(pObject))
    {
        // add layout to parent layout
        if (QBoxLayout* pParentBoxLayout = qobject_cast<QBoxLayout*>(pParentLayout))
            pParentBoxLayout->addLayout(pLayout);
    }

    m_aChildren.emplace_back(sID, pObject);

    return pObject;
}

void QtBuilder::tweakInsertedChild(QObject* pParent, QObject* pCurrentChild, std::string_view sType,
                                   std::string_view)
{
    if (sType == "label")
    {
        if (QLabel* pLabel = qobject_cast<QLabel*>(pCurrentChild))
        {
            if (QGroupBox* pGroupBox = qobject_cast<QGroupBox*>(pParent))
            {
                // GtkFrame has a `child-type="label"` child for the GtkFrame label
                // in the GtkBuilder .ui file, s. https://docs.gtk.org/gtk3/class.Frame.html
                // For QGroupBox, the title can be set directly. Therefore, take over the
                // title from the label and delete the separate label widget again
                pGroupBox->setTitle(pLabel->text());
                pLabel->setParent(nullptr);
                pLabel->deleteLater();
            }
        }
    }

    if (QDialog* pDialog = qobject_cast<QDialog*>(pCurrentChild))
    {
        // no action needed for QMessageBox, where the default button box is used
        // and button click is handled in QtInstanceMessageDialog
        if (!qobject_cast<QMessageBox*>(pDialog))
        {
            if (QDialogButtonBox* pButtonBox = findButtonBox(pDialog))
            {
                // ensure that button box is the last item in QDialog's layout
                // (that seems to be implicitly the case for GtkDialog in GTK)
                QLayout* pLayout = pDialog->layout();
                assert(pLayout && "dialog has no layout");
                pLayout->removeWidget(pButtonBox);
                pLayout->addWidget(pButtonBox);

                // connect button click handler
                const QList<QAbstractButton*> aButtons = pButtonBox->buttons();
                for (QAbstractButton* pButton : aButtons)
                {
                    assert(pButton);
                    QObject::connect(pButton, &QAbstractButton::clicked, pDialog,
                                     [pDialog, pButton] {
                                         QtInstanceDialog::handleButtonClick(*pDialog, *pButton);
                                     });
                }
            }
        }
    }
}

void QtBuilder::setMnemonicWidget(const OUString& rLabelId, const OUString& rMnemonicWidgetId)
{
    QLabel* pLabel = get<QLabel>(rLabelId);
    QObject* pBuddy = get_by_name(rMnemonicWidgetId);

    if (!pLabel || !pBuddy || !pBuddy->isWidgetType())
        return;

    pLabel->setBuddy(static_cast<QWidget*>(pBuddy));
}

void QtBuilder::setPriority(QObject*, int) { SAL_WARN("vcl.qt", "Ignoring priority"); }

void QtBuilder::setContext(QObject*, std::vector<vcl::EnumContext::Context>&&)
{
    SAL_WARN("vcl.qt", "Ignoring context");
}

void QtBuilder::applyAtkProperties(QObject* pObject, const stringmap& rProperties, bool)
{
    if (!pObject || !pObject->isWidgetType())
        return;

    QWidget* pWidget = static_cast<QWidget*>(pObject);

    for (auto const & [ rKey, rValue ] : rProperties)
    {
        if (rKey == "AtkObject::accessible-description")
            pWidget->setAccessibleDescription(toQString(rValue));
        else if (rKey == "AtkObject::accessible-name")
            pWidget->setAccessibleName(toQString(rValue));
    }
}

void QtBuilder::applyPackingProperties(QObject*, QObject*, const stringmap&)
{
    SAL_WARN("vcl.qt", "QtBuilder::applyPackingProperties not implemented yet");
}

void QtBuilder::set_response(std::u16string_view sID, short nResponse)
{
    QPushButton* pPushButton = get<QPushButton>(sID);
    assert(pPushButton);
    pPushButton->setProperty(QtInstanceMessageDialog::PROPERTY_VCL_RESPONSE_CODE, int(nResponse));
}

void QtBuilder::setProperties(QObject* pObject, stringmap& rProps)
{
    if (QMessageBox* pMessageBox = qobject_cast<QMessageBox*>(pObject))
    {
        for (auto const & [ rKey, rValue ] : rProps)
        {
            if (rKey == u"text")
            {
                pMessageBox->setText(toQString(rValue));
            }
            else if (rKey == u"title")
            {
                pMessageBox->setWindowTitle(toQString(rValue));
            }
            else if (rKey == u"secondary-text")
            {
                pMessageBox->setInformativeText(toQString(rValue));
            }
            else if (rKey == u"message-type")
            {
                if (rValue == u"error")
                    pMessageBox->setIcon(QMessageBox::Critical);
                else if (rValue == u"info")
                    pMessageBox->setIcon(QMessageBox::Information);
                else if (rValue == u"question")
                    pMessageBox->setIcon(QMessageBox::Question);
                else if (rValue == u"warning")
                    pMessageBox->setIcon(QMessageBox::Warning);
                else
                    assert(false && "Unhandled message-type");
            }
        }
    }
    else if (QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pObject))
    {
        for (auto const & [ rKey, rValue ] : rProps)
        {
            if (rKey == u"active")
                pCheckBox->setChecked(toBool(rValue));
            else if (rKey == u"label")
                pCheckBox->setText(convertAccelerator(rValue));
        }
    }
    else if (QDialog* pDialog = qobject_cast<QDialog*>(pObject))
    {
        for (auto const & [ rKey, rValue ] : rProps)
        {
            if (rKey == u"modal")
                pDialog->setModal(toBool(rValue));
            else if (rKey == u"title")
                pDialog->setWindowTitle(toQString(rValue));
        }
    }
    else if (QLabel* pLabel = qobject_cast<QLabel*>(pObject))
    {
        for (auto const & [ rKey, rValue ] : rProps)
        {
            if (rKey == u"label")
                pLabel->setText(convertAccelerator(rValue));
            else if (rKey == u"wrap")
                pLabel->setWordWrap(toBool(rValue));
        }
    }
    else if (QPushButton* pButton = qobject_cast<QPushButton*>(pObject))
    {
        for (auto const & [ rKey, rValue ] : rProps)
        {
            if (rKey == u"label")
                pButton->setText(convertAccelerator(rValue));
        }
    }
}

QWidget* QtBuilder::windowForObject(QObject* pObject)
{
    if (QWidget* pWidget = qobject_cast<QWidget*>(pObject))
        return pWidget->window();

    if (QLayout* pLayout = qobject_cast<QLayout*>(pObject))
    {
        if (QWidget* pParentWidget = pLayout->parentWidget())
            return pParentWidget->window();
    }

    return nullptr;
}

QDialogButtonBox* QtBuilder::findButtonBox(QDialog* pDialog)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
