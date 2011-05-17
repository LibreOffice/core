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
#ifndef INCLUDED_SVT_FILEPICKER_HXX
#define INCLUDED_SVT_FILEPICKER_HXX

#include <cppuhelper/implbase7.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


#include <tools/wintypes.hxx>
#include "commonpicker.hxx"
#include "pickercallbacks.hxx"

#include <list>

class Dialog;

struct FilterEntry;
struct ElementEntry_Impl;

typedef ::std::list< FilterEntry >                          FilterList;     // can be maintained more effectively
typedef ::std::list < ElementEntry_Impl >                   ElementList;

typedef ::com::sun::star::beans::StringPair                 UnoFilterEntry;
typedef ::com::sun::star::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively
typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >  OUStringList;   // can be transported more effectively

// class SvtFilePicker ---------------------------------------------------

typedef ::cppu::ImplHelper7 <   ::com::sun::star::ui::dialogs::XFilePickerControlAccess
                            ,   ::com::sun::star::ui::dialogs::XFilePickerNotifier
                            ,   ::com::sun::star::ui::dialogs::XFilePreview
                            ,   ::com::sun::star::ui::dialogs::XFilterManager
                            ,   ::com::sun::star::ui::dialogs::XFilterGroupManager
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::ui::dialogs::XAsynchronousExecutableDialog
                            >   SvtFilePicker_Base;

class SvtFilePicker :public SvtFilePicker_Base
                    ,public ::svt::OCommonPicker
                    ,public ::svt::IFilePickerListener
{
private:
    FilterList*         m_pFilterList;
    ElementList*        m_pElemList;

    sal_Bool            m_bMultiSelection;
    sal_Int16           m_nServiceType;
    ::rtl::OUString     m_aDefaultName;
    ::rtl::OUString     m_aCurrentFilter;

    // #97148# --------------
    ::rtl::OUString     m_aOldDisplayDirectory;
    ::rtl::OUString     m_aOldHideDirectory;

    ::rtl::OUString     m_aStandardDir;
    OUStringList        m_aBlackList;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >
                        m_xListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >
                        m_xDlgClosedListener;

public:
                       SvtFilePicker( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual           ~SvtFilePicker();

    //------------------------------------------------------------------------------------
    // disambiguate XInterface
    //------------------------------------------------------------------------------------
    DECLARE_XINTERFACE( )

    //------------------------------------------------------------------------------------
    // disambiguate XTypeProvider
    //------------------------------------------------------------------------------------
    DECLARE_XTYPEPROVIDER( )

    //------------------------------------------------------------------------------------
    // XExecutableDialog functions
    //------------------------------------------------------------------------------------
    virtual void SAL_CALL setTitle( const ::rtl::OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // XAsynchronousExecutableDialog functions
    //------------------------------------------------------------------------------------
    virtual void SAL_CALL setDialogTitle( const ::rtl::OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startExecuteModal( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // XFilePicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL           setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setDefaultName( const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setDisplayDirectory( const ::rtl::OUString& aDirectory ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL    getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles() throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL           setValue( sal_Int16 ElementID, sal_Int16 ControlAction, const com::sun::star::uno::Any& value ) throw( ::com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL           getValue( sal_Int16 ElementID, sal_Int16 ControlAction ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setLabel( sal_Int16 ElementID, const ::rtl::OUString& aValue ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL    getLabel( sal_Int16 ElementID ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           enableControl( sal_Int16 ElementID, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePickerNotifier functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL           addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw ( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePreview functions
    //------------------------------------------------------------------------------------

    virtual com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats() throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL      getTargetColorDepth() throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL      getAvailableWidth() throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL      getAvailableHeight() throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setImage( sal_Int16 aImageFormat, const com::sun::star::uno::Any& aImage ) throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL       setShowState( sal_Bool bShowState ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL       getShowState() throw ( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL           appendFilter( const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setCurrentFilter( const ::rtl::OUString& aTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL    getCurrentFilter() throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterGroupManager functions
    //------------------------------------------------------------------------------------
    virtual void SAL_CALL           appendFilterGroup( const ::rtl::OUString& sGroupTitle, const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& aFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // XInitialization functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL           initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments ) throw ( com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XServiceInfo functions
    //------------------------------------------------------------------------------------

    /* XServiceInfo */
    virtual ::rtl::OUString SAL_CALL    getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL       supportsService( const ::rtl::OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                    getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    /* Helper for XServiceInfo */
    static com::sun::star::uno::Sequence< ::rtl::OUString >
                                    impl_getStaticSupportedServiceNames();
    static ::rtl::OUString          impl_getStaticImplementationName();

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL impl_createInstance (
        const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( com::sun::star::uno::Exception );

protected:
    //------------------------------------------------------------------------------------
    // OCommonPicker overridables
    //------------------------------------------------------------------------------------
    virtual SvtFileDialog*  implCreateDialog( Window* _pParent );
    virtual sal_Int16       implExecutePicker( );
    virtual sal_Bool        implHandleInitializationArgument(
                                const ::rtl::OUString& _rName,
                                const ::com::sun::star::uno::Any& _rValue
                            )
                            SAL_THROW( ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) );

private:
    WinBits             getWinBits( WinBits& rExtraBits );
    virtual void        notify( sal_Int16 _nEventId, sal_Int16 _nControlId );

    sal_Bool            FilterNameExists( const ::rtl::OUString& rTitle );
    sal_Bool            FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void                ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter );

    void                prepareExecute( );

    DECL_LINK(          DialogClosedHdl, Dialog* );
};

#endif // INCLUDED_SVT_FILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
