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

#pragma once

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

#include <graphic/UnoGraphicDescriptor.hxx>

#include <vcl/graph.hxx>

namespace unographic {

class Graphic final : public css::graphic::XGraphic,
                public css::awt::XBitmap,
                public css::graphic::XGraphicTransformer,
                public ::unographic::GraphicDescriptor
{
public:
    Graphic();
    virtual ~Graphic() noexcept override;

    using ::unographic::GraphicDescriptor::init;
    void init(const ::Graphic& rGraphic);

    const ::Graphic& GetGraphic() const { return maGraphic; }

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;
private:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // XGraphic
    virtual ::sal_Int8 SAL_CALL getType(  ) override;

    // XBitmap
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getDIB(  ) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getMaskDIB(  ) override;

    // XGraphicTransformer
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL colorChange(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo ) override;

    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL applyDuotone(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorOne, sal_Int32 nColorTwo ) override;

    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL applyBrightnessContrast(
        const css::uno::Reference< css::graphic::XGraphic >& rxGraphic,
        sal_Int32 nBrightness, sal_Int32 nContrast, sal_Bool mso ) override;

    ::Graphic maGraphic;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
