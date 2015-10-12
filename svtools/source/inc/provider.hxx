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

#ifndef INCLUDED_SVTOOLS_SOURCE_INC_PROVIDER_HXX
#define INCLUDED_SVTOOLS_SOURCE_INC_PROVIDER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

namespace {

class GraphicProvider : public ::cppu::WeakImplHelper< ::com::sun::star::graphic::XGraphicProvider,
                                                        ::com::sun::star::lang::XServiceInfo >
{
public:

    GraphicProvider();
    virtual ~GraphicProvider();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XGraphicProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL queryGraphicDescriptor( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL queryGraphic( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL storeGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& MediaProperties ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

private:

    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadMemory( const OUString& rResourceURL );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadGraphicObject( const OUString& rResourceURL );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadResource( const OUString& rResourceURL );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadRepositoryImage( const OUString& rResourceURL );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& rBitmap );
    static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > implLoadStandardImage( const OUString& rResourceURL );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
