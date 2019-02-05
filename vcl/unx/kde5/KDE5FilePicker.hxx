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

#include <Qt5FilePicker.hxx>

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

#include <functional>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtWidgets/QFileDialog>

class QFileDialog;
class QGridLayout;
class QWidget;
class QComboBox;

class KDE5FilePicker : public Qt5FilePicker
{
    Q_OBJECT
protected:
    css::uno::Reference<css::ui::dialogs::XFilePickerListener> m_xListener;
    osl::Mutex _helperMutex;

    //running filter string to add to dialog
    QStringList _filters;
    // map of filter titles to full filter for selection
    QHash<QString, QString> _titleToFilters;
    // string to set the current filter
    QString _currentFilter;

    //mapping of SAL control ID's to created custom controls
    QHash<sal_Int16, QWidget*> _customWidgets;
    QHash<sal_Int16, QWidget*> _customListboxes;

    //widget to contain extra custom controls
    QWidget* _extraControls;

    //layout for extra custom controls
    QGridLayout* _layout;

    bool allowRemoteUrls;

public:
    explicit KDE5FilePicker(QFileDialog::FileMode);
    virtual ~KDE5FilePicker() override;

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;
    virtual void SAL_CALL removeFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle(const OUString& rTitle) override;
    virtual sal_Int16 SAL_CALL execute() override;

    // XFilePicker functions
    virtual void SAL_CALL setDefaultName(const OUString& rName) override;
    virtual void SAL_CALL setDisplayDirectory(const OUString& rDirectory) override;
    virtual OUString SAL_CALL getDisplayDirectory() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getFiles() override;

    // XFilterManager functions
    virtual void SAL_CALL appendFilter(const OUString& rTitle, const OUString& rFilter) override;
    virtual void SAL_CALL setCurrentFilter(const OUString& rTitle) override;
    virtual OUString SAL_CALL getCurrentFilter() override;

    // XFilterGroupManager functions
    virtual void SAL_CALL
    appendFilterGroup(const OUString& rGroupTitle,
                      const css::uno::Sequence<css::beans::StringPair>& rFilters) override;

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue(sal_Int16 nControlId, sal_Int16 nControlAction,
                                   const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getValue(sal_Int16 nControlId,
                                            sal_Int16 nControlAction) override;
    virtual void SAL_CALL enableControl(sal_Int16 nControlId, sal_Bool bEnable) override;
    virtual void SAL_CALL setLabel(sal_Int16 nControlId, const OUString& rLabel) override;
    virtual OUString SAL_CALL getLabel(sal_Int16 nControlId) override;

    // XFolderPicker stuff
    virtual OUString SAL_CALL getDirectory() override;
    virtual void SAL_CALL setDescription(const OUString& rDescription) override;

    /* TODO XFilePreview

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  );
    virtual sal_Int32 SAL_CALL  getTargetColorDepth(  );
    virtual sal_Int32 SAL_CALL  getAvailableWidth(  );
    virtual sal_Int32 SAL_CALL  getAvailableHeight(  );
    virtual void SAL_CALL       setImage( sal_Int16 aImageFormat, const css::uno::Any &rImage );
    virtual sal_Bool SAL_CALL   setShowState( sal_Bool bShowState );
    virtual sal_Bool SAL_CALL   getShowState(  );
    */

    // XFilePicker2 functions
    virtual css::uno::Sequence<OUString> SAL_CALL getSelectedFiles() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XCancellable
    virtual void SAL_CALL cancel() override;

    // XEventListener
    virtual void disposing(const css::lang::EventObject& rEvent);
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    //add a custom control widget to the file dialog
    void addCustomControl(sal_Int16 controlId);
    static void handleSetListValue(QComboBox* pQComboBox, sal_Int16 nAction,
                                   const css::uno::Any& rValue);
    static css::uno::Any handleGetListValue(QComboBox* pQComboBox, sal_Int16 nAction);
    OUString implGetDirectory();

    // emit XFilePickerListener controlStateChanged event
    void filterChanged();
    // emit XFilePickerListener fileSelectionChanged event
    void selectionChanged();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

Q_SIGNALS:
    sal_Int16 executeSignal();
    void setTitleSignal(const OUString& rTitle);
    void setDefaultNameSignal(const OUString& rName);
    void setDisplayDirectorySignal(const OUString& rDir);
    OUString getDisplayDirectorySignal();
    OUString getDirectorySignal();
    void setValueSignal(sal_Int16 nControlId, sal_Int16 nControlAction,
                        const css::uno::Any& rValue);
    css::uno::Any getValueSignal(sal_Int16 nControlId, sal_Int16 nControlAction);
    void enableControlSignal(sal_Int16 nControlId, bool bEnable);
    void setLabelSignal(sal_Int16 nControlId, const OUString& rLabel);
    OUString getLabelSignal(sal_Int16 nControlId);
    void appendFilterSignal(const OUString& rTitle, const OUString& rFilter);
    void appendFilterGroupSignal(const OUString& rTitle,
                                 const css::uno::Sequence<css::beans::StringPair>& rFilters);
    void setCurrentFilterSignal(const OUString& rFilter);
    OUString getCurrentFilterSignal();
    css::uno::Sequence<OUString> getFilesSignal();
    css::uno::Sequence<OUString> getSelectedFilesSignal();

private Q_SLOTS:
    void setTitleSlot(const OUString& rTitle) { return setTitle(rTitle); }
    void setDefaultNameSlot(const OUString& rName) { return setDefaultName(rName); }
    void setDisplayDirectorySlot(const OUString& rDir) { return setDisplayDirectory(rDir); }
    OUString getDisplayDirectorySlot() { return implGetDirectory(); }
    OUString getDirectorySlot() { return implGetDirectory(); }
    void setValueSlot(sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any& rValue)
    {
        return setValue(nControlId, nControlAction, rValue);
    }

    css::uno::Any getValueSlot(sal_Int16 nControlId, sal_Int16 nControlAction)
    {
        return getValue(nControlId, nControlAction);
    }

    void enableControlSlot(sal_Int16 nControlId, bool bEnable)
    {
        return enableControl(nControlId, bEnable);
    }

    void setLabelSlot(sal_Int16 nControId, const OUString& rLabel)
    {
        return setLabel(nControId, rLabel);
    }

    OUString getLabelSlot(sal_Int16 nControlId) { return getLabel(nControlId); }

    void appendFilterSlot(const OUString& rTitle, const OUString& rFilter)
    {
        return appendFilter(rTitle, rFilter);
    }

    void appendFilterGroupSlot(const OUString& rTitle,
                               const css::uno::Sequence<css::beans::StringPair>& rFilters)
    {
        return appendFilterGroup(rTitle, rFilters);
    }

    void setCurrentFilterSlot(const OUString& rFilter) { return setCurrentFilter(rFilter); }
    OUString getCurrentFilterSlot() { return getCurrentFilter(); }
    css::uno::Sequence<OUString> getSelectedFilesSlot() { return getFiles(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
