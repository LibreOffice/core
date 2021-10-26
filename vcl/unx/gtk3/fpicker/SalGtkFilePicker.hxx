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
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <vector>
#include <memory>
#include <rtl/ustring.hxx>

#include "SalGtkPicker.hxx"

// Implementation class for the XFilePicker Interface

struct FilterEntry;
struct ElementEntry_Impl;


typedef cppu::WeakComponentImplHelper<
        css::ui::dialogs::XFilePicker3,
        css::lang::XInitialization
        > SalGtkFilePicker_Base;

class SalGtkFilePicker : public SalGtkPicker, public SalGtkFilePicker_Base
{
    public:

        // constructor
        SalGtkFilePicker( const css::uno::Reference< css::uno::XComponentContext >& xServiceMgr );

        // XFilePickerNotifier

        virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;
        virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;

        // XExecutableDialog functions

        virtual void SAL_CALL setTitle( const OUString& aTitle ) override;

        virtual sal_Int16 SAL_CALL execute() override;

        // XFilePicker functions

        virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) override;

        virtual void SAL_CALL setDefaultName( const OUString& aName ) override;

        virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory ) override;

        virtual OUString SAL_CALL getDisplayDirectory(  ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getFiles(  ) override;

        // XFilePicker2 functions

        virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles() override;

        // XFilterManager functions

        virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter ) override;

        virtual void SAL_CALL setCurrentFilter( const OUString& aTitle ) override;

        virtual OUString SAL_CALL getCurrentFilter(  ) override;

        // XFilterGroupManager functions

        virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters ) override;

        // XInitialization

        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XCancellable

        virtual void SAL_CALL cancel( ) override;

        // FilePicker Event functions

    private:
        SalGtkFilePicker( const SalGtkFilePicker& ) = delete;
        SalGtkFilePicker& operator=( const SalGtkFilePicker& ) = delete;

        bool FilterNameExists( const OUString& rTitle );
        bool FilterNameExists( const css::uno::Sequence< css::beans::StringPair >& _rGroupedFilters );

        void ensureFilterVector( const OUString& _rInitialCurrentFilter );

        void impl_fileSelectionChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
        void impl_directoryChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
        void impl_controlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
        void impl_initialize(GtkWidget* pParentWidget, sal_Int16 templateId);

    private:
        css::uno::Reference< css::ui::dialogs::XFilePickerListener >
            m_xListener;
        std::unique_ptr<std::vector<FilterEntry>> m_pFilterVector;

        GtkListStore *m_pFilterStore;

        gulong mnHID_FolderChange;
        gulong mnHID_SelectionChange;

        OUString m_aCurrentFilter;
        OUString m_aInitialFilter;

        bool bVersionWidthUnset;
        bool mbInitialized;
        gulong mHID_Preview;
        GtkFileFilter* m_pPseudoFilter;

        GtkWidget  *getWidget( sal_Int16 nControlId, GType *pType = nullptr);

        void SetCurFilter( const OUString& rFilter );
        void SetFilters();
        void UpdateFilterfromUI();

        GtkFileFilter * implAddFilter( const OUString& rFilter, const OUString& rType );
        void implAddFilterGroup(
                     const css::uno::Sequence< css::beans::StringPair>& _rFilters );
        void updateCurrentFilterFromName(const gchar* filtername);
        void unselect_type();

        static void filter_changed_cb( GtkFileChooser *file_chooser, GParamSpec *pspec, SalGtkFilePicker *pobjFP );
        static void type_changed_cb( GtkTreeSelection *selection, SalGtkFilePicker *pobjFP );
        static void folder_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void selection_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
    public:
         virtual ~SalGtkFilePicker() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
