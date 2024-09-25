/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtBuilder.hxx>

#include <QtTools.hxx>

#include <rtl/ustrbuf.hxx>

#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>

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
    QWidget* pParentWidget = qobject_cast<QWidget*>(pParent);

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
                pObject = new QVBoxLayout();
            else
                pObject = new QHBoxLayout();
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

    m_aChildren.emplace_back(sID, pObject);

    return pObject;
}

void QtBuilder::tweakInsertedChild(QObject*, QObject*, std::string_view, std::string_view) {}

void QtBuilder::setPriority(QObject*, int) { SAL_WARN("vcl.qt", "Ignoring priority"); }

void QtBuilder::setContext(QObject*, std::vector<vcl::EnumContext::Context>&&)
{
    SAL_WARN("vcl.qt", "Ignoring context");
}

void QtBuilder::applyAtkProperties(QObject*, const stringmap&, bool)
{
    SAL_WARN("vcl.qt", "QtBuilder::applyAtkProperties not implemented yet");
}

void QtBuilder::applyPackingProperties(QObject*, QObject*, const stringmap&)
{
    SAL_WARN("vcl.qt", "QtBuilder::applyPackingProperties not implemented yet");
}

void QtBuilder::set_response(std::u16string_view, short)
{
    SAL_WARN("vcl.qt", "QtBuilder::set_response not implemented yet");
}

void QtBuilder::setProperties(QObject* obj, stringmap& rProps)
{
    if (QMessageBox* pMessageBox = qobject_cast<QMessageBox*>(obj))
    {
        for (auto const & [ Key, rValue ] : rProps)
        {
            OUString rKey = Key;

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
    else if (QPushButton* pButton = qobject_cast<QPushButton*>(obj))
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

QDialogButtonBox* QtBuilder::findButtonBox(QMessageBox* pMessageBox)
{
    assert(pMessageBox);
    QLayout* pLayout = pMessageBox->layout();
    assert(pLayout && "QMessageBox has no layout");
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
