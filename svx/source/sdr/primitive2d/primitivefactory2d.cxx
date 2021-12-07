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

#include <sdr/primitive2d/primitivefactory2d.hxx>

using namespace com::sun::star;

css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > SAL_CALL PrimitiveFactory2D::createPrimitivesFromXShape(
    const uno::Reference< drawing::XShape >& xShape,
    const uno::Sequence< beans::PropertyValue >& /*aParms*/ )
{
    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aRetval;

    if(xShape.is())
    {
        SdrObject* pSource = SdrObject::getSdrObjectFromXShape(xShape);

        if(pSource)
        {
            const sdr::contact::ViewContact& rSource(pSource->GetViewContact());
            drawinglayer::primitive2d::Primitive2DContainer aSourceVal;
            rSource.getViewIndependentPrimitive2DContainer(aSourceVal);
            aRetval = aSourceVal.toSequence();
        }
    }

    return aRetval;
}

void PrimitiveFactory2D::createPrimitivesFromXShape(
    const uno::Reference< drawing::XShape >& xShape,
    const uno::Sequence< beans::PropertyValue >& /*aParms*/,
    drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor)
{
    if(xShape.is())
    {
        SdrObject* pSource = SdrObject::getSdrObjectFromXShape(xShape);

        if(pSource)
        {
            const sdr::contact::ViewContact& rSource(pSource->GetViewContact());
            rSource.getViewIndependentPrimitive2DContainer(rVisitor);
        }
    }
}

css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > SAL_CALL PrimitiveFactory2D::createPrimitivesFromXDrawPage(
    const uno::Reference< drawing::XDrawPage >& xDrawPage,
    const uno::Sequence< beans::PropertyValue >& /*aParms*/ )
{
    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aRetval;

    if(xDrawPage.is())
    {
        SdrPage* pSource = GetSdrPageFromXDrawPage(xDrawPage);

        if(pSource)
        {
            const sdr::contact::ViewContact& rSource(pSource->GetViewContact());
            drawinglayer::primitive2d::Primitive2DContainer aSourceRetval;
            rSource.getViewIndependentPrimitive2DContainer(aSourceRetval);
            aRetval = aSourceRetval.toSequence();
        }
    }

    return aRetval;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PrimitiveFactory2D);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
