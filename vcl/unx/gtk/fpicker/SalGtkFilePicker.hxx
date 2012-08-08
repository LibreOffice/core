/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SALGTKFILEPICKER_HXX_
#define _SALGTKFILEPICKER_HXX_

#include <cppuhelper/compbase9.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <list>
#include <memory>
#include <rtl/ustring.hxx>

#include "gtk/fpicker/SalGtkPicker.hxx"

//----------------------------------------------------------
// Implementation class for the XFilePicker Interface
//----------------------------------------------------------

using ::rtl::OUString;
using ::rtl::OString;
struct FilterEntry;
struct ElementEntry_Impl;

typedef ::std::list < FilterEntry >     FilterList;
typedef ::std::list < ElementEntry_Impl >   ElementList;
typedef ::com::sun::star::beans::StringPair UnoFilterEntry;
typedef ::com::sun::star::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively


//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalGtkFilePicker :
        public SalGtkPicker,
    public cppu::WeakComponentImplHelper9<
        ::com::sun::star::ui::dialogs::XFilterManager,
        ::com::sun::star::ui::dialogs::XFilterGroupManager,
        ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
        ::com::sun::star::ui::dialogs::XFilePickerNotifier,
        ::com::sun::star::ui::dialogs::XFilePreview,
        ::com::sun::star::ui::dialogs::XFilePicker2,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::util::XCancellable,
        ::com::sun::star::lang::XEventListener >
{
    public:

        // constructor
        SalGtkFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceMgr );

        //------------------------------------------------------------------------------------
        // XFilePickerNotifier
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XExecutableDialog functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int16 SAL_CALL execute()
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XFilePicker functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDefaultName( const ::rtl::OUString& aName )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& aDirectory )
            throw( com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException );

        virtual ::rtl::OUString SAL_CALL getDisplayDirectory(  )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles(  )
            throw( ::com::sun::star::uno::RuntimeException );

         //------------------------------------------------------------------------------------
        // XFilePicker2 functions
        //------------------------------------------------------------------------------------

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSelectedFiles()
                throw (::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // XFilterManager functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL appendFilter( const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilter )
            throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setCurrentFilter( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

        virtual ::rtl::OUString SAL_CALL getCurrentFilter(  )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XFilterGroupManager functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL appendFilterGroup( const ::rtl::OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // XFilePickerControlAccess functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any& aValue )
                throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
                throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable )
            throw(::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setLabel( sal_Int16 nControlId, const ::rtl::OUString& aLabel )
            throw (::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 nControlId )
            throw (::com::sun::star::uno::RuntimeException);

        //------------------------------------------------
        // XFilePreview
        //------------------------------------------------

        virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL getTargetColorDepth(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL getAvailableWidth(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL getAvailableHeight(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL getShowState(  )
            throw (::com::sun::star::uno::RuntimeException);

        //------------------------------------------------
        // XInitialization
        //------------------------------------------------

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        //------------------------------------------------
        // XCancellable
        //------------------------------------------------

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------
        // XEventListener
        //------------------------------------------------

        using cppu::WeakComponentImplHelperBase::disposing;
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // FilePicker Event functions
        //------------------------------------------------------------------------------------

    private:
        // prevent copy and assignment
        SalGtkFilePicker( const SalGtkFilePicker& );
        SalGtkFilePicker& operator=( const SalGtkFilePicker& );

        sal_Bool FilterNameExists( const ::rtl::OUString& rTitle );
        sal_Bool FilterNameExists( const UnoFilterList& _rGroupedFilters );

        void ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter );

        void impl_fileSelectionChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
        void impl_directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
        void impl_controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >
            m_xListener;
        ::rtl::OUString msPlayLabel;
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
        bool mbToggleChecked[TOGGLE_LAST];

        static const rtl::OString m_ToggleLabels[TOGGLE_LAST];

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

        ::rtl::OUString m_aCurrentFilter;
        ::rtl::OUString m_aInitialFilter;

        bool bVersionWidthUnset;
        sal_Bool mbPreviewState;
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
        ::com::sun::star::uno::Any HandleGetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction) const;

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
#endif // _SALGTKFILEPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
