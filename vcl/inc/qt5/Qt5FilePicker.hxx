/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <vclpluginapi.h>

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtWidgets/QFileDialog>

#include <memory>

class QGridLayout;
class QLabel;
class QWidget;

typedef ::cppu::WeakComponentImplHelper<
    css::ui::dialogs::XFilePicker3, css::ui::dialogs::XFilePickerControlAccess,
    css::ui::dialogs::XFolderPicker2, css::lang::XInitialization, css::lang::XServiceInfo>
    Qt5FilePicker_Base;

class VCLPLUG_QT5_PUBLIC Qt5FilePicker : public QObject, public Qt5FilePicker_Base
{
    Q_OBJECT

protected:
    css::uno::Reference<css::ui::dialogs::XFilePickerListener> m_xListener;

    std::unique_ptr<QFileDialog> m_pFileDialog; ///< the non-native file picker dialog

    osl::Mutex m_aHelperMutex; ///< mutex used by the WeakComponentImplHelper

    QStringList m_aNamedFilterList; ///< to keep the original sequence
    QHash<QString, QString> m_aTitleToFilterMap;
    // to retrieve the filename extension for a given filter
    QHash<QString, QString> m_aNamedFilterToExtensionMap;
    QString m_aCurrentFilter;

    QWidget* m_pExtraControls; ///< widget to contain extra custom controls
    QGridLayout* m_pLayout; ///< layout for extra custom controls
    QLabel* m_pFilenameLabel; ///< label to display the filename
    QLabel* m_pFilterLabel; ///< label to display the filter
    QHash<sal_Int16, QWidget*> m_aCustomWidgetsMap; ///< map of SAL control ID's to widget

    bool m_bIsFolderPicker;

public:
    explicit Qt5FilePicker(QFileDialog::FileMode);
    virtual ~Qt5FilePicker() override;

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;
    virtual void SAL_CALL removeFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;

    // XFilterManager functions
    virtual void SAL_CALL appendFilter(const OUString& rTitle, const OUString& rFilter) override;
    virtual void SAL_CALL setCurrentFilter(const OUString& rTitle) override;
    virtual OUString SAL_CALL getCurrentFilter() override;

    // XFilterGroupManager functions
    virtual void SAL_CALL
    appendFilterGroup(const OUString& rGroupTitle,
                      const css::uno::Sequence<css::beans::StringPair>& rFilters) override;

    // XCancellable
    virtual void SAL_CALL cancel() override;

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle(const OUString& rTitle) override;
    virtual sal_Int16 SAL_CALL execute() override;

    // XFilePicker functions
    virtual void SAL_CALL setMultiSelectionMode(sal_Bool bMode) override;
    virtual void SAL_CALL setDefaultName(const OUString& rName) override;
    virtual void SAL_CALL setDisplayDirectory(const OUString& rDirectory) override;
    virtual OUString SAL_CALL getDisplayDirectory() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getFiles() override;

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue(sal_Int16 nControlId, sal_Int16 nControlAction,
                                   const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getValue(sal_Int16 nControlId,
                                            sal_Int16 nControlAction) override;
    virtual void SAL_CALL enableControl(sal_Int16 nControlId, sal_Bool bEnable) override;
    virtual void SAL_CALL setLabel(sal_Int16 nControlId, const OUString& rLabel) override;
    virtual OUString SAL_CALL getLabel(sal_Int16 nControlId) override;

    // XFilePicker2 functions
    virtual css::uno::Sequence<OUString> SAL_CALL getSelectedFiles() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XEventListener
    /// @throws css::uno::RuntimeException
    virtual void disposing(const css::lang::EventObject& rEvent);
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XFolderPicker functions
    virtual OUString SAL_CALL getDirectory() override;
    virtual void SAL_CALL setDescription(const OUString& rDescription) override;

private Q_SLOTS:
    // XExecutableDialog functions
    /// @throws css::uno::RuntimeException
    void setTitleSlot(const OUString& rTitle) { return setTitle(rTitle); }
    /// @throws css::uno::RuntimeException
    sal_Int16 executeSlot() { return execute(); }

    // XFilePicker functions
    /// @throws css::uno::RuntimeException
    void setMultiSelectionModeSlot(bool bMode) { return setMultiSelectionMode(bMode); }
    /// @throws css::uno::RuntimeException
    void setDefaultNameSlot(const OUString& rName) { return setDefaultName(rName); }
    /// @throws css::uno::RuntimeException
    void setDisplayDirectorySlot(const OUString& rDirectory)
    {
        return setDisplayDirectory(rDirectory);
    }
    /// @throws css::uno::RuntimeException
    OUString getDisplayDirectorySlot() { return getDisplayDirectory(); }
    /// @throws css::uno::RuntimeException
    css::uno::Sequence<OUString> getFilesSlot() { return getFiles(); }

    // XFilterManager functions
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void appendFilterSlot(const OUString& rTitle, const OUString& rFilter)
    {
        return appendFilter(rTitle, rFilter);
    }
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void setCurrentFilterSlot(const OUString& rTitle) { return setCurrentFilter(rTitle); }
    /// @throws css::uno::RuntimeException
    OUString getCurrentFilterSlot() { return getCurrentFilter(); }

    // XFilterGroupManager functions
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void appendFilterGroupSlot(const OUString& rGroupTitle,
                               const css::uno::Sequence<css::beans::StringPair>& rFilters)
    {
        return appendFilterGroup(rGroupTitle, rFilters);
    }

    // XFilePickerControlAccess functions
    /// @throws css::uno::RuntimeException
    void setValueSlot(sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any& rValue)
    {
        return setValue(nControlId, nControlAction, rValue);
    }
    /// @throws css::uno::RuntimeException
    css::uno::Any getValueSlot(sal_Int16 nControlId, sal_Int16 nControlAction)
    {
        return getValue(nControlId, nControlAction);
    }
    /// @throws css::uno::RuntimeException
    void enableControlSlot(sal_Int16 nControlId, bool bEnable)
    {
        return enableControl(nControlId, bEnable);
    }
    /// @throws css::uno::RuntimeException
    void setLabelSlot(sal_Int16 nControlId, const OUString& rLabel)
    {
        return setLabel(nControlId, rLabel);
    }
    /// @throws css::uno::RuntimeException
    OUString getLabelSlot(sal_Int16 nControlId) { return getLabel(nControlId); }

    // XFilePicker2 functions
    /// @throws css::uno::RuntimeException
    css::uno::Sequence<OUString> getSelectedFilesSlot() { return getSelectedFiles(); }

    // XInitialization
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    void initializeSlot(const css::uno::Sequence<css::uno::Any>& rArguments)
    {
        return initialize(rArguments);
    }

Q_SIGNALS:
    // XExecutableDialog functions
    void setTitleSignal(const OUString& rTitle);
    sal_Int16 executeSignal();

    // XFilePicker functions
    void setMultiSelectionModeSignal(bool bMode);
    void setDefaultNameSignal(const OUString& rName);
    void setDisplayDirectorySignal(const OUString& rDirectory);
    OUString getDisplayDirectorySignal();
    css::uno::Sequence<OUString> getFilesSignal();

    // XFilterManager functions
    void appendFilterSignal(const OUString& rTitle, const OUString& rFilter);
    void setCurrentFilterSignal(const OUString& rTitle);
    OUString getCurrentFilterSignal();

    // XFilterGroupManager functions
    void appendFilterGroupSignal(const OUString& rGroupTitle,
                                 const css::uno::Sequence<css::beans::StringPair>& rFilters);

    // XFilePickerControlAccess functions
    void setValueSignal(sal_Int16 nControlId, sal_Int16 nControlAction,
                        const css::uno::Any& rValue);
    css::uno::Any getValueSignal(sal_Int16 nControlId, sal_Int16 nControlAction);
    void enableControlSignal(sal_Int16 nControlId, bool bEnable);
    void setLabelSignal(sal_Int16 nControlId, const OUString& rLabel);
    OUString getLabelSignal(sal_Int16 nControlId);

    // XFilePicker2 functions
    css::uno::Sequence<OUString> getSelectedFilesSignal();

    // XInitialization
    void initializeSignal(const css::uno::Sequence<css::uno::Any>& rArguments);

private:
    Qt5FilePicker(const Qt5FilePicker&) = delete;
    Qt5FilePicker& operator=(const Qt5FilePicker&) = delete;

    void addCustomControl(sal_Int16 controlId);

    static QString getResString(const char* pRedId);

private Q_SLOTS:
    // emit XFilePickerListener controlStateChanged event
    void filterSelected(const QString&);
    // emit XFilePickerListener fileSelectionChanged event
    void currentChanged(const QString&);
    // (un)set automatic file extension
    void updateAutomaticFileExtension();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
