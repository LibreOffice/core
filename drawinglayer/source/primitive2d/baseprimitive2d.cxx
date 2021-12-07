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

#include <sal/config.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

BasePrimitive2DImplBase::~BasePrimitive2DImplBase() {}

css::uno::Any BasePrimitive2DImplBase::queryInterface(css::uno::Type const& rType)
{
    css::uno::Any aReturn = ::cppu::queryInterface(
        rType, static_cast<uno::XWeak*>(this), static_cast<lang::XComponent*>(this),
        static_cast<lang::XTypeProvider*>(this), static_cast<graphic::XPrimitive2D*>(this),
        static_cast<util::XAccounting*>(this));
    if (aReturn.hasValue())
        return aReturn;
    return OWeakObject::queryInterface(rType);
}

void BasePrimitive2DImplBase::acquire() noexcept { OWeakObject::acquire(); }

void BasePrimitive2DImplBase::release() noexcept
{
    if (osl_atomic_decrement(&m_refCount) != 0)
        return;

    // ensure no other references are created, via the weak connection point, from now on
    disposeWeakConnectionPoint();
    // restore reference count:
    osl_atomic_increment(&m_refCount);
    //    if (! rBHelper.bDisposed) {
    //        try {
    //            dispose();
    //        }
    //        catch (RuntimeException const& exc) { // don't break throw ()
    //            SAL_WARN( "cppuhelper", exc );
    //        }
    //        OSL_ASSERT( rBHelper.bDisposed );
    //    }
    OWeakObject::release();
}

void BasePrimitive2DImplBase::dispose() {}

void BasePrimitive2DImplBase::addEventListener(
    css::uno::Reference<css::lang::XEventListener> const&)
{
    assert(false);
}
void BasePrimitive2DImplBase::removeEventListener(
    css::uno::Reference<css::lang::XEventListener> const&)
{
    assert(false);
}

css::uno::Sequence<css::uno::Type> BasePrimitive2DImplBase::getTypes()
{
    static const css::uno::Sequence<uno::Type> aTypeList{
        cppu::UnoType<uno::XWeak>::get(), cppu::UnoType<lang::XComponent>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(), cppu::UnoType<graphic::XPrimitive2D>::get(),
        cppu::UnoType<util::XAccounting>::get()
    };

    return aTypeList;
}

namespace drawinglayer::primitive2d
{
BasePrimitive2D::BasePrimitive2D() {}

BasePrimitive2D::~BasePrimitive2D() {}

bool BasePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    return (getPrimitive2DID() == rPrimitive.getPrimitive2DID());
}

namespace
{
// Visitor class to get the B2D range from a tree of Primitive2DReference's
//
class B2DRangeVisitor : public Primitive2DDecompositionVisitor
{
public:
    const geometry::ViewInformation2D& mrViewInformation;
    basegfx::B2DRange maRetval;
    B2DRangeVisitor(const geometry::ViewInformation2D& rViewInformation)
        : mrViewInformation(rViewInformation)
    {
    }
    virtual void visit(const Primitive2DReference& r) override
    {
        maRetval.expand(getB2DRangeFromPrimitive2DReference(r, mrViewInformation));
    }
    virtual void visit(const Primitive2DContainer& r) override
    {
        maRetval.expand(r.getB2DRange(mrViewInformation));
    }
    virtual void visit(Primitive2DContainer&& r) override
    {
        maRetval.expand(r.getB2DRange(mrViewInformation));
    }
};
}

basegfx::B2DRange
BasePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    B2DRangeVisitor aVisitor(rViewInformation);
    get2DDecomposition(aVisitor, rViewInformation);
    return aVisitor.maRetval;
}

void BasePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& /*rVisitor*/,
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
}

Primitive2DContainer
BasePrimitive2D::getDecomposition(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    const auto aViewInformation = geometry::createViewInformation2D(rViewParameters);
    Primitive2DContainer aContainer;
    get2DDecomposition(aContainer, aViewInformation);
    return aContainer;
}

css::geometry::RealRectangle2D
BasePrimitive2D::getRange(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    const auto aViewInformation = geometry::createViewInformation2D(rViewParameters);
    return basegfx::unotools::rectangle2DFromB2DRectangle(getB2DRange(aViewInformation));
}

sal_Int64 BasePrimitive2D::estimateUsage()
{
    return 0; // for now ignore the objects themselves
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
