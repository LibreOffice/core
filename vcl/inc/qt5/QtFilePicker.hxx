/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <unotools/resmgr.hxx>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtWidgets/QFileDialog>

#include <memory>

class QComboBox;
class QGridLayout;
class QLabel;
class QWidget;

typedef ::cppu::WeakComponentImplHelper<
    css::frame::XTerminateListener, css::lang::XInitialization, css::lang::XServiceInfo,
    css::ui::dialogs::XFilePicker3, css::ui::dialogs::XFilePickerControlAccess,
    css::ui::dialogs::XAsynchronousExecutableDialog, css::ui::dialogs::XFolderPicker2>
    QtFilePicker_Base;

class VCLPLUG_QT_PUBLIC QtFilePicker : public QObject, public QtFilePicker_Base
{
    Q_OBJECT

private:
    css::uno::Reference<css::uno::XComponentContext> m_context;

    css::uno::Reference<css::ui::dialogs::XFilePickerListener> m_xListener;
    css::uno::Reference<css::ui::dialogs::XDialogClosedListener> m_xClosedListener;

    osl::Mutex m_aHelperMutex; ///< mutex used by the WeakComponentImplHelper

    QStringList m_aNamedFilterList; ///< to keep the original sequence
    QHash<QString, QString> m_aTitleToFilterMap;
    // to retrieve the filename extension for a given filter
    QHash<QString, QString> m_aNamedFilterToExtensionMap;
    QString m_aCurrentFilter;

    QGridLayout* m_pLayout; ///< layout for extra custom controls
    QHash<sal_Int16, QWidget*> m_aCustomWidgetsMap; ///< map of SAL control ID's to widget

    const bool m_bIsFolderPicker;

    QWidget* m_pParentWidget;

protected:
    std::unique_ptr<QFileDialog> m_pFileDialog; ///< the file picker dialog
    QWidget* m_pExtraControls; ///< widget to contain extra custom controls

public:
    // use non-native file dialog by default; there's no easy way to add custom widgets
    // in a generic way in the native one
    explicit QtFilePicker(css::uno::Reference<css::uno::XComponentContext> context,
                          QFileDialog::FileMode, bool bUseNative = false);
    virtual ~QtFilePicker() override;

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

    // XAsynchronousExecutableDialog functions
    virtual void SAL_CALL setDialogTitle(const OUString&) override;
    virtual void SAL_CALL
    startExecuteModal(const css::uno::Reference<css::ui::dialogs::XDialogClosedListener>&) override;

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
    void SAL_CALL disposing(const css::lang::EventObject& rEvent) override;
    using cppu::WeakComponentImplHelperBase::disposing;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XFolderPicker functions
    virtual OUString SAL_CALL getDirectory() override;
    virtual void SAL_CALL setDescription(const OUString& rDescription) override;

    // XTerminateListener
    void SAL_CALL queryTermination(const css::lang::EventObject& aEvent) override;
    void SAL_CALL notifyTermination(const css::lang::EventObject& aEvent) override;

protected:
    virtual void addCustomControl(sal_Int16 controlId);
    void setCustomControlWidgetLayout(QGridLayout* pLayout) { m_pLayout = pLayout; }

private:
    QtFilePicker(const QtFilePicker&) = delete;
    QtFilePicker& operator=(const QtFilePicker&) = delete;

    static QString getResString(TranslateId pRedId);
    void applyTemplate(sal_Int16 nTemplateId);
    static css::uno::Any handleGetListValue(const QComboBox* pWidget, sal_Int16 nControlAction);
    static void handleSetListValue(QComboBox* pQComboBox, sal_Int16 nAction,
                                   const css::uno::Any& rValue);

    void prepareExecute();

private Q_SLOTS:
    // emit XFilePickerListener controlStateChanged event
    void filterSelected(const QString&);
    // emit XFilePickerListener fileSelectionChanged event
    void currentChanged(const QString&);
    // (un)set automatic file extension
    virtual void updateAutomaticFileExtension();
    void finished(int);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
