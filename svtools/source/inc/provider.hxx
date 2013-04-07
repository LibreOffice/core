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

    static OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XGraphicProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL queryGraphicDescriptor( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL queryGraphic( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadMemory( const OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadGraphicObject( const OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadResource( const OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadRepositoryImage( const OUString& rResourceURL ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& rBitmap ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadStandardImage( const OUString& rResourceURL ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
