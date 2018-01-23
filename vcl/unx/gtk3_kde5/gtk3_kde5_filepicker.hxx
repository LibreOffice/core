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

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <boost/process/child.hpp>
#include <boost/process/pipe.hpp>

#include "gtk3_kde5_filepicker_ipc.hxx"

#include <functional>

typedef ::cppu::WeakComponentImplHelper<css::ui::dialogs::XFilePicker3,
                                        css::ui::dialogs::XFilePickerControlAccess
                                        // TODO css::ui::dialogs::XFilePreview
                                        ,
                                        css::lang::XInitialization, css::lang::XServiceInfo>
    Gtk3KDE5FilePicker_Base;

class Gtk3KDE5FilePicker : public Gtk3KDE5FilePicker_Base
{
protected:
    css::uno::Reference<css::ui::dialogs::XFilePickerListener> m_xListener;

    osl::Mutex _helperMutex;
    Gtk3KDE5FilePickerIpc m_ipc;

public:
    explicit Gtk3KDE5FilePicker(const css::uno::Reference<css::uno::XComponentContext>&);
    virtual ~Gtk3KDE5FilePicker() override;

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;
    virtual void SAL_CALL removeFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override;

    // XExecutableDialog functions
    virtual void SAL_CALL setTitle(const OUString& rTitle) override;
    virtual sal_Int16 SAL_CALL execute() override;

    // XFilePicker functions
    virtual void SAL_CALL setMultiSelectionMode(sal_Bool bMode) override;
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
    Gtk3KDE5FilePicker(const Gtk3KDE5FilePicker&) = delete;
    Gtk3KDE5FilePicker& operator=(const Gtk3KDE5FilePicker&) = delete;

    //add a custom control widget to the file dialog
    void addCustomControl(sal_Int16 controlId);

    // emit XFilePickerListener controlStateChanged event
    void filterChanged();
    // emit XFilePickerListener fileSelectionChanged event
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
