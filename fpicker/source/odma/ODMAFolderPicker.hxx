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

#ifndef INCLUDED_ODMA_FOLDERPICKER_HXX
#define INCLUDED_ODMA_FOLDERPICKER_HXX

#include <cppuhelper/compbase2.hxx>

#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

// class ODMAFolderPicker ---------------------------------------------------

class ODMAFolderPicker :
    public cppu::WeakComponentImplHelper2<
      ::com::sun::star::ui::dialogs::XFolderPicker,
      ::com::sun::star::lang::XServiceInfo >
{
private:
    sal_Bool m_bUseDMS;

protected:
    ::osl::Mutex m_rbHelperMtx;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xInterface;

public:
    ODMAFolderPicker( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    // XFolderPicker functions

    virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& aDirectory )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getDisplayDirectory( )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getDirectory( )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDescription( const ::rtl::OUString& aDescription )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const ::rtl::OUString& _rTitle )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL execute(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo functions

    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw( ::com::sun::star::uno::RuntimeException );

    /* Helper for XServiceInfo */
    static com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames( );
    static ::rtl::OUString impl_getStaticImplementationName( );

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL impl_createInstance ( const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( com::sun::star::uno::Exception );
};

#endif // INCLUDED_ODMA_FOLDERPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
