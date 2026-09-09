/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    Graphic(const ::Graphic& rGraphic);
    virtual ~Graphic() noexcept override;

    const ::Graphic& GetGraphic() const { return maGraphic; }

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;
private:
    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

    // XGraphic
    virtual ::sal_Int8 getType(  ) override;

    // XBitmap
    virtual css::awt::Size getSize(  ) override;
    virtual cpo::uno::Sequence< ::sal_Int8 > getDIB(  ) override;
    virtual cpo::uno::Sequence< ::sal_Int8 > getMaskDIB(  ) override;

    // XGraphicTransformer
    virtual css::uno::Reference< css::graphic::XGraphic > colorChange(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo ) override;

    virtual css::uno::Reference< css::graphic::XGraphic > applyDuotone(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorOne, sal_Int32 nColorTwo ) override;

    virtual css::uno::Reference< css::graphic::XGraphic > applyBrightnessContrast(
        const css::uno::Reference< css::graphic::XGraphic >& rxGraphic,
        sal_Int32 nBrightness, sal_Int32 nContrast, bool mso ) override;

    ::Graphic maGraphic;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
