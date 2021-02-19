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

#ifndef INCLUDED_VCL_SOURCE_GRAPHIC_GRAPHIC_HXX
#define INCLUDED_VCL_SOURCE_GRAPHIC_GRAPHIC_HXX

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

#include <graphic/UnoGraphicDescriptor.hxx>
#include <graphic/UnoGraphicTransformer.hxx>

#include <vcl/graph.hxx>

namespace unographic {

class Graphic final : public css::graphic::XGraphic,
                public css::awt::XBitmap,
                public css::lang::XUnoTunnel,
                public ::unographic::GraphicDescriptor,
                public ::unographic::GraphicTransformer
{
public:
    Graphic();
    virtual ~Graphic() throw() override;

    using ::unographic::GraphicDescriptor::init;
    void init(const ::Graphic& rGraphic);

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;
private:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XGraphic
    virtual ::sal_Int8 SAL_CALL getType(  ) override;

    // XBitmap
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getDIB(  ) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getMaskDIB(  ) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;

    ::Graphic maGraphic;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
