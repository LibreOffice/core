/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Novell, Inc.
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

#ifndef INCLUDED_ODMA_FILEPICKER_HXX
#define INCLUDED_ODMA_FILEPICKER_HXX

#include <cppuhelper/compbase9.hxx>

#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

// class ODMAFilePicker ---------------------------------------------------

class ODMAFilePicker :
    public cppu::WeakComponentImplHelper9<
      ::com::sun::star::ui::dialogs::XFilterManager,
      ::com::sun::star::ui::dialogs::XFilterGroupManager,
      ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
      ::com::sun::star::ui::dialogs::XFilePickerNotifier,
      ::com::sun::star::ui::dialogs::XFilePreview,
      ::com::sun::star::lang::XInitialization,
      ::com::sun::star::util::XCancellable,
      ::com::sun::star::lang::XEventListener,
      ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    sal_Bool m_bUseDMS;
    sal_Bool m_bMultiSelectionMode;
    rtl::OUString m_aDefaultName;
    rtl::OUString m_aDisplayDirectory;
    ::com::sun::star::uno::Sequence< rtl::OUString > m_aFiles;
    enum { OPEN, SAVE } m_nDialogKind;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xSystemFilePicker;

protected:
    ::osl::Mutex m_rbHelperMtx;

public:

    ODMAFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    // XFilterManager functions

    virtual void SAL_CALL appendFilter( const ::rtl::OUString& aTitle,
                                        const ::rtl::OUString& aFilter )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getCurrentFilter( )
        throw( ::com::sun::star::uno::RuntimeException );

    // XFilterGroupManager functions

    virtual void SAL_CALL appendFilterGroup( const ::rtl::OUString& sGroupTitle,
                                             const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );

    // XFilePickerControlAccess functions

    virtual void SAL_CALL setValue( sal_Int16 nControlID,
                                    sal_Int16 nControlAction,
                                    const ::com::sun::star::uno::Any& aValue )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 nControlID,
                                             sal_Int16 nControlAction )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 nControlID,
                                    const ::rtl::OUString& aValue )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 nControlID )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL enableControl( sal_Int16 nControlID,
                                         sal_Bool bEnable )
        throw( ::com::sun::star::uno::RuntimeException );

    // XFilePicker functions

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDefaultName( const ::rtl::OUString& aName )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& aDirectory )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getDisplayDirectory( )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( )
        throw( ::com::sun::star::uno::RuntimeException );

    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL execute(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XFilePickerNotifier functions

    virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XFilePreview functions

    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats( )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getTargetColorDepth( )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getAvailableWidth( )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getAvailableHeight( )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat,
                                    const ::com::sun::star::uno::Any& aImage )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL getShowState( )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XInitialization functions

    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    // XCancellable functions

    virtual void SAL_CALL cancel( )
        throw( ::com::sun::star::uno::RuntimeException );

    // XEventListener functions

    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo functions

    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw( ::com::sun::star::uno::RuntimeException );

    /* Helper for XServiceInfo */
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames( );
    static ::rtl::OUString impl_getStaticImplementationName( );

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
        throw( ::com::sun::star::uno::Exception );
};

#endif // INCLUDED_ODMA_FILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
