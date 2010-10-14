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

#ifndef _GOODIES_PROVIDER_HXX
#define _GOODIES_PROVIDER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

using namespace com::sun::star;

namespace unographic {

// -------------------
// - GraphicProvider -
// -------------------

class GraphicProvider : public ::cppu::WeakImplHelper1< ::com::sun::star::graphic::XGraphicProvider >
{
public:

    GraphicProvider();
    ~GraphicProvider();

    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XGraphicProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL queryGraphicDescriptor( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL queryGraphic( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadMemory( const ::rtl::OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadGraphicObject( const ::rtl::OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadResource( const ::rtl::OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadRepositoryImage( const ::rtl::OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& rBitmap ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadStandardImage( const ::rtl::OUString& rResourceURL ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
