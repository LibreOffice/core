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

#include <drawinglayer/primitive2d/Primitive2DVisitor.hxx>
#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

typedef comphelper::WeakComponentImplHelper<css::graphic::XPrimitiveFactory2D,
                                            css::lang::XServiceInfo>
    PrimitiveFactory2DImplBase;

// base class for C++ implementation of css::graphic::XPrimitiveFactory2D
class PrimitiveFactory2D final : public PrimitiveFactory2DImplBase
{
public:
    PrimitiveFactory2D() {}

    // Methods from XPrimitiveFactory2D
    virtual css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>
        SAL_CALL createPrimitivesFromXShape(
            const css::uno::Reference<css::drawing::XShape>& xShape,
            const css::uno::Sequence<css::beans::PropertyValue>& aParms) override;
    virtual css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>
        SAL_CALL createPrimitivesFromXDrawPage(
            const css::uno::Reference<css::drawing::XDrawPage>& xDrawPage,
            const css::uno::Sequence<css::beans::PropertyValue>& aParms) override;

    static void createPrimitivesFromXShape(
        const css::uno::Reference<css::drawing::XShape>& xShape,
        const css::uno::Sequence<css::beans::PropertyValue>& /*aParms*/,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor);

    OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.graphic.PrimitiveFactory2D"_ustr;
    }

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.graphic.PrimitiveFactory2D"_ustr };
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
