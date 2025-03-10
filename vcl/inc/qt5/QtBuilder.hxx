/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <QtCore/QObject>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QToolButton>

#include <rtl/ustring.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/builder.hxx>

class QtBuilder : public WidgetBuilder<QObject, QObject*, QMenu, QMenu*>
{
private:
    QWidget* get_by_name(const OUString& rId);
    std::unordered_map<OUString, QWidget*> m_aWidgets;

    // vector of pairs, each containing:
    // * a widget to remove from the widget hierarchy and delete (first item)
    // * the widget to put in its place instead (second item)
    std::vector<std::pair<QWidget*, QWidget*>> m_aWidgetReplacements;

public:
    QtBuilder(QWidget* pParent, std::u16string_view sUIRoot, const OUString& rUIFile);
    virtual ~QtBuilder();

    template <typename T = QWidget> T* get(const OUString& rId);

    QObject* makeObject(QObject* pParent, std::u16string_view sName, std::string_view sType,
                        const OUString& sID, stringmap& rMap);

    virtual void applyAtkProperties(QObject* pObject, const stringmap& rProperties,
                                    bool bToolbarItem) override;
    virtual void applyPackingProperties(QObject* pCurrentChild, QObject* pParent,
                                        const stringmap& rPackingProperties) override;
    virtual void applyTabChildProperties(QObject* pParent, const std::vector<OUString>& rIDs,
                                         std::vector<vcl::EnumContext::Context>& rContext,
                                         stringmap& rProperties,
                                         stringmap& rAtkProperties) override;
    virtual void insertComboBoxOrListBoxItems(QObject* pObject, stringmap& rMap,
                                              const std::vector<ComboBoxTextItem>& rItems) override;

    virtual QObject* insertObject(QObject* pParent, const OUString& rClass, std::string_view sType,
                                  const OUString& rID, stringmap& rProps,
                                  stringmap& rPangoAttributes, stringmap& rAtkProps) override;

    void tweakInsertedChild(QObject* pParent, QObject* pCurrentChild, std::string_view sType,
                            std::string_view sInternalChild) override;

    virtual void setMnemonicWidget(const OUString& rLabelId,
                                   const OUString& rMnemonicWidgetId) override;
    virtual void setRadioButtonGroup(const OUString& rRadioButtonId,
                                     const OUString& rRadioGroupId) override;
    virtual void setPriority(QObject* pObject, int nPriority) override;
    virtual void setContext(QObject* pObject,
                            std::vector<vcl::EnumContext::Context>&& aContext) override;

    virtual bool isHorizontalTabControl(QObject* pObject) override;

    virtual QMenu* createMenu(const OUString& rId) override;
    virtual void insertMenuObject(QMenu* pParent, QMenu* pSubMenu, const OUString& rClass,
                                  const OUString& rID, stringmap& rProps, stringmap& rAtkProps,
                                  accelmap& rAccels) override;

    virtual void set_response(const OUString& rId, int nResponse) override;

private:
    static void deleteObject(QObject* pObject);
    // remove pOldWidget from the widget hierarchy and set (child widget) pNewWidget in its place
    static void replaceWidget(QWidget* pOldWidget, QWidget* pNewWidget);
    void setButtonProperties(QAbstractButton& rButton, stringmap& rProps);
    static void setCheckButtonProperties(QAbstractButton& rButton, stringmap& rProps);
    static void setDialogProperties(QDialog& rDialog, stringmap& rProps);
    static void setEntryProperties(QLineEdit& rLineEdit, stringmap& rProps);
    static void setLabelProperties(QLabel& rLabel, stringmap& rProps);
    static void setMessageDialogProperties(QMessageBox& rMessageBox, stringmap& rProps);
    void setMenuButtonProperties(QToolButton& rButton, stringmap& rProps);
    void setScaleProperties(QSlider& rSlider, stringmap& rProps);
    void setSpinButtonProperties(QDoubleSpinBox& rSpinBox, stringmap& rProps);
    static void setTextViewProperties(QPlainTextEdit& rTextEdit, stringmap& rProps);
    static QWidget* windowForObject(QObject* pObject);

    static void applyGridPackingProperties(QWidget* pCurrentChild, QGridLayout& rGrid,
                                           const stringmap& rPackingProperties);
};

template <typename T> inline T* QtBuilder::get(const OUString& rId)
{
    QWidget* pWidget = get_by_name(rId);
    assert(!pWidget || qobject_cast<T*>(pWidget));
    return static_cast<T*>(pWidget);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
