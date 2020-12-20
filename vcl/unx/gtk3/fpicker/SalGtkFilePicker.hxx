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

#ifndef INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFILEPICKER_HXX
#define INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFILEPICKER_HXX

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
        css::ui::dialogs::XFilePickerControlAccess,
        css::ui::dialogs::XFilePreview,
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

        // XFilePickerControlAccess functions

        virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const css::uno::Any& aValue ) override;

        virtual css::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) override;

        virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) override;

        virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel ) override;

        virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) override;

        // XFilePreview

        virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) override;

        virtual sal_Int32 SAL_CALL getTargetColorDepth(  ) override;

        virtual sal_Int32 SAL_CALL getAvailableWidth(  ) override;

        virtual sal_Int32 SAL_CALL getAvailableHeight(  ) override;

        virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage ) override;

        virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState ) override;

        virtual sal_Bool SAL_CALL getShowState(  ) override;

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

    private:
        css::uno::Reference< css::ui::dialogs::XFilePickerListener >
            m_xListener;
        OUString msPlayLabel;
        std::unique_ptr<std::vector<FilterEntry>> m_pFilterVector;
        GtkWidget  *m_pParentWidget;
        GtkWidget  *m_pVBox;
        GtkWidget  *m_pFilterExpander;
        GtkWidget  *m_pFilterView;
        GtkListStore *m_pFilterStore;

        enum {
            AUTOEXTENSION,
            PASSWORD,
            FILTEROPTIONS,
            READONLY,
            LINK,
            PREVIEW,
            SELECTION,
            GPGENCRYPTION,
            TOGGLE_LAST
              };

        GtkWidget  *m_pToggles[ TOGGLE_LAST ];

        bool mbToggleVisibility[TOGGLE_LAST];

        enum {
            PLAY,
            BUTTON_LAST };

        GtkWidget  *m_pButtons[ BUTTON_LAST ];

        enum {
            VERSION,
            TEMPLATE,
            IMAGE_TEMPLATE,
            IMAGE_ANCHOR,
            LIST_LAST
              };

        GtkWidget *m_pHBoxs[ LIST_LAST ];
        GtkWidget *m_pLists[ LIST_LAST ];
        GtkWidget *m_pListLabels[ LIST_LAST ];
        bool mbListVisibility[ LIST_LAST ];
        bool mbButtonVisibility[ BUTTON_LAST ];
        gulong mnHID_FolderChange;
        gulong mnHID_SelectionChange;

        OUString m_aCurrentFilter;
        OUString m_aInitialFilter;

        bool bVersionWidthUnset;
        bool mbPreviewState;
        gulong mHID_Preview;
        GtkWidget* m_pPreview;
        GtkFileFilter* m_pPseudoFilter;
        static constexpr sal_Int32 g_PreviewImageWidth = 256;
        static constexpr sal_Int32 g_PreviewImageHeight = 256;

        GtkWidget  *getWidget( sal_Int16 nControlId, GType *pType = nullptr);

        void SetCurFilter( const OUString& rFilter );
        void SetFilters();
        void UpdateFilterfromUI();

        void implChangeType( GtkTreeSelection *selection );
        GtkFileFilter * implAddFilter( const OUString& rFilter, const OUString& rType );
        void implAddFilterGroup(
                     const css::uno::Sequence< css::beans::StringPair>& _rFilters );
        void updateCurrentFilterFromName(const gchar* filtername);
        void unselect_type();
        void InitialMapping();

        void HandleSetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction,
            const css::uno::Any& rValue);
        static css::uno::Any HandleGetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction);

        static void expander_changed_cb( GtkExpander *expander, SalGtkFilePicker *pobjFP );
        static void preview_toggled_cb( GObject *cb, SalGtkFilePicker *pobjFP );
        static void filter_changed_cb( GtkFileChooser *file_chooser, GParamSpec *pspec, SalGtkFilePicker *pobjFP );
        static void type_changed_cb( GtkTreeSelection *selection, SalGtkFilePicker *pobjFP );
        static void folder_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void selection_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void update_preview_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void dialog_mapped_cb(GtkWidget *widget, SalGtkFilePicker *pobjFP);
    public:
         virtual ~SalGtkFilePicker() override;

};
#endif // INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
