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
#include <com/sun/star/beans/StringPair.hpp>

#include <list>
#include <memory>
#include <rtl/ustring.hxx>

#include "gtk/fpicker/SalGtkPicker.hxx"

// Implementation class for the XFilePicker Interface

struct FilterEntry;
struct ElementEntry_Impl;

typedef ::std::list < FilterEntry >     FilterList;
typedef ::std::list < ElementEntry_Impl >   ElementList;
typedef ::com::sun::star::beans::StringPair UnoFilterEntry;
typedef ::com::sun::star::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively

// class declaration

typedef cppu::WeakComponentImplHelper<
        ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
        ::com::sun::star::ui::dialogs::XFilePreview,
        ::com::sun::star::ui::dialogs::XFilePicker3,
        ::com::sun::star::lang::XInitialization
        > SalGtkFilePicker_Base;

class SalGtkFilePicker : public SalGtkPicker, public SalGtkFilePicker_Base
{
    public:

        // constructor
        SalGtkFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceMgr );

        // XFilePickerNotifier

        virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XExecutableDialog functions

        virtual void SAL_CALL setTitle( const OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual sal_Int16 SAL_CALL execute()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XFilePicker functions

        virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setDefaultName( const OUString& aName )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
            throw( com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual OUString SAL_CALL getDisplayDirectory(  )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles(  )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XFilePicker2 functions

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedFiles()
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XFilterManager functions

        virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
            throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setCurrentFilter( const OUString& aTitle )
            throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual OUString SAL_CALL getCurrentFilter(  )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XFilterGroupManager functions

        virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XFilePickerControlAccess functions

        virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any& aValue )
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable )
            throw(::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual OUString SAL_CALL getLabel( sal_Int16 nControlId )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XFilePreview

        virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Int32 SAL_CALL getTargetColorDepth(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Int32 SAL_CALL getAvailableWidth(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Int32 SAL_CALL getAvailableHeight(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL getShowState(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XCancellable

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // FilePicker Event functions

    private:
        SalGtkFilePicker( const SalGtkFilePicker& ) = delete;
        SalGtkFilePicker& operator=( const SalGtkFilePicker& ) = delete;

        bool FilterNameExists( const OUString& rTitle );
        bool FilterNameExists( const UnoFilterList& _rGroupedFilters );

        void ensureFilterList( const OUString& _rInitialCurrentFilter );

        void impl_fileSelectionChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
        void impl_directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
        void impl_controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >
            m_xListener;
        OUString msPlayLabel;
        FilterList *m_pFilterList;
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
            LIST_LAST
              };

        GtkWidget *m_pHBoxs[ LIST_LAST ];
        GtkWidget *m_pAligns[ LIST_LAST ];
        GtkWidget *m_pLists[ LIST_LAST ];
        GtkListStore *m_pListStores[ LIST_LAST ];
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
        sal_Int32 m_PreviewImageWidth;
        sal_Int32 m_PreviewImageHeight;

        GtkWidget  *getWidget( sal_Int16 nControlId, GType *pType = NULL);

        void SetCurFilter( const OUString& rFilter );
        void SetFilters();
        void UpdateFilterfromUI();

        void implChangeType( GtkTreeSelection *selection );
        GtkFileFilter * implAddFilter( const OUString& rFilter, const OUString& rType );
        void implAddFilterGroup( const OUString& rFilter,
                     const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair>& _rFilters );
        void updateCurrentFilterFromName(const gchar* filtername);
        void unselect_type();
        void InitialMapping();

        void HandleSetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction,
            const ::com::sun::star::uno::Any& rValue);
        static ::com::sun::star::uno::Any HandleGetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction);

        static void expander_changed_cb( GtkExpander *expander, SalGtkFilePicker *pobjFP );
        static void preview_toggled_cb( GObject *cb, SalGtkFilePicker *pobjFP );
        static void filter_changed_cb( GtkFileChooser *file_chooser, GParamSpec *pspec, SalGtkFilePicker *pobjFP );
        static void type_changed_cb( GtkTreeSelection *selection, SalGtkFilePicker *pobjFP );
        static void folder_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void selection_changed_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void update_preview_cb (GtkFileChooser *file_chooser, SalGtkFilePicker *pobjFP);
        static void dialog_mapped_cb(GtkWidget *widget, SalGtkFilePicker *pobjFP);
    public:
         virtual ~SalGtkFilePicker();

};
#endif // INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
