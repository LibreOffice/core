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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <rtl/ref.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace com::sun::star;

namespace {

typedef cppu::WeakComponentImplHelper< ::com::sun::star::graphic::XPrimitiveFactory2D, css::lang::XServiceInfo > PrimitiveFactory2DImplBase;

// base class for C++ implementation of com::sun::star::graphic::XPrimitiveFactory2D
class PrimitiveFactory2D
    :   protected comphelper::OBaseMutex,
        public PrimitiveFactory2DImplBase
{
public:
    PrimitiveFactory2D(): PrimitiveFactory2DImplBase(m_aMutex) {}

            // Methods from XPrimitiveFactory2D
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXDrawPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.comp.graphic.PrimitiveFactory2D"); }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return css::uno::Sequence<OUString>{
            "com.sun.star.graphic.PrimitiveFactory2D"};
    }
};

css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > SAL_CALL PrimitiveFactory2D::createPrimitivesFromXShape(
    const uno::Reference< drawing::XShape >& xShape,
    const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException, std::exception)
{
    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aRetval;

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

css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > SAL_CALL PrimitiveFactory2D::createPrimitivesFromXDrawPage(
    const uno::Reference< drawing::XDrawPage >& xDrawPage,
    const uno::Sequence< beans::PropertyValue >& /*aParms*/ ) throw (uno::RuntimeException, std::exception)
{
    css::uno::Sequence< css::uno::Reference< css::graphic::XPrimitive2D > > aRetval;

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
com_sun_star_comp_graphic_PrimitiveFactory2D_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PrimitiveFactory2D);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
