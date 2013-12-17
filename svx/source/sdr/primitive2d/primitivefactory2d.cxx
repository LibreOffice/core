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

#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace com::sun::star;

namespace {

typedef cppu::WeakComponentImplHelper1< graphic::XPrimitiveFactory2D > PrimitiveFactory2DImplBase;

// base class for C++ implementation of com::sun::star::graphic::XPrimitiveFactory2D
class PrimitiveFactory2D: protected comphelper::OBaseMutex, public PrimitiveFactory2DImplBase
{
public:
    PrimitiveFactory2D(): PrimitiveFactory2DImplBase(m_aMutex) {}

    // Methods from XPrimitiveFactory2D
    virtual uno::Sequence< uno::Reference< graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXShape( const uno::Reference< drawing::XShape >& xShape, const uno::Sequence< beans::PropertyValue >& aParms ) throw (uno::RuntimeException);
    virtual uno::Sequence< uno::Reference< graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXDrawPage( const uno::Reference< drawing::XDrawPage >& xDrawPage, const uno::Sequence< beans::PropertyValue >& aParms ) throw (uno::RuntimeException);
};

        Primitive2DSequence SAL_CALL PrimitiveFactory2D::createPrimitivesFromXShape(
            const uno::Reference< drawing::XShape >& xShape,
            const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException)
        {
            Primitive2DSequence aRetval;

            if(xShape.is())
            {
                SdrObject* pSource = GetSdrObjectFromXShape(xShape);

                if(pSource)
                {
                    const sdr::contact::ViewContact& rSource(pSource->GetViewContact());
                    aRetval = rSource.getViewIndependentPrimitive2DSequence();
                }
            }

            return aRetval;
        }

        Primitive2DSequence SAL_CALL PrimitiveFactory2D::createPrimitivesFromXDrawPage(
            const uno::Reference< drawing::XDrawPage >& xDrawPage,
            const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException)
        {
            Primitive2DSequence aRetval;

            if(xDrawPage.is())
            {
                SdrPage* pSource = GetSdrPageFromXDrawPage(xDrawPage);

                if(pSource)
                {
                    const sdr::contact::ViewContact& rSource(pSource->GetViewContact());

                    aRetval = rSource.getViewIndependentPrimitive2DSequence();
                }
            }

            return aRetval;
        }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_graphic_PrimitiveFactory2D_implementation_getFactory(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    css::uno::Reference<css::uno::XInterface> x(
        static_cast<cppu::OWeakObject *>(new PrimitiveFactory2D));
    x->acquire();
    return x.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
