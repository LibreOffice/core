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

#ifndef INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRAPHIC_HXX
#define INCLUDED_SVTOOLS_SOURCE_GRAPHIC_GRAPHIC_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

#include "descriptor.hxx"
#include "transformer.hxx"

namespace unographic {

class Graphic : public ::com::sun::star::graphic::XGraphic,
                public ::com::sun::star::awt::XBitmap,
                public ::com::sun::star::lang::XUnoTunnel,
                public ::unographic::GraphicDescriptor,
                public ::unographic::GraphicTransformer
{
public:

    Graphic();
    virtual ~Graphic() throw();

    using ::unographic::GraphicDescriptor::init;
    void init( const ::Graphic& rGraphic ) throw();

    static const ::Graphic* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    static OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XGraphic
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XBitmap
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getDIB(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getMaskDIB(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:

    ::Graphic* mpGraphic;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
