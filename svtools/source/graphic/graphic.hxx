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

class Graphic : public css::graphic::XGraphic,
                public css::awt::XBitmap,
                public css::lang::XUnoTunnel,
                public ::unographic::GraphicDescriptor,
                public ::unographic::GraphicTransformer
{
public:

    Graphic();
    virtual ~Graphic() throw();

    using ::unographic::GraphicDescriptor::init;
    void init( const ::Graphic& rGraphic ) throw();

    static const ::Graphic* getImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace ) throw();
    static OUString getImplementationName_Static() throw();
    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XGraphic
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XBitmap
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getDIB(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getMaskDIB(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) override;

private:

    ::Graphic* mpGraphic;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
