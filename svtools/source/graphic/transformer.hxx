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

#ifndef INCLUDED_SVTOOLS_SOURCE_GRAPHIC_TRANSFORMER_HXX
#define INCLUDED_SVTOOLS_SOURCE_GRAPHIC_TRANSFORMER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

namespace unographic {

typedef ::cppu::WeakAggImplHelper< css::graphic::XGraphicTransformer > GraphicTransformer_UnoImplHelper;
class GraphicTransformer : public GraphicTransformer_UnoImplHelper
{
    public:

    GraphicTransformer();
    virtual ~GraphicTransformer() override;

    // XGraphicTransformer
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL colorChange(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL applyDuotone(
        const css::uno::Reference< css::graphic::XGraphic >& rGraphic,
        sal_Int32 nColorOne, sal_Int32 nColorTwo )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL applyBrightnessContrast(
        const css::uno::Reference< css::graphic::XGraphic >& rxGraphic,
        sal_Int32 nBrightness, sal_Int32 nContrast, sal_Bool mso )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
