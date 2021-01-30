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

using namespace css;

namespace drawinglayer::primitive2d
{
BasePrimitive2D::BasePrimitive2D()
    : BasePrimitive2DImplBase(m_aMutex)
{
}

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
    virtual void append(const Primitive2DReference& r) override
    {
        maRetval.expand(getB2DRangeFromPrimitive2DReference(r, mrViewInformation));
    }
    virtual void append(const Primitive2DContainer& r) override
    {
        maRetval.expand(r.getB2DRange(mrViewInformation));
    }
    virtual void append(Primitive2DContainer&& r) override
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

css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> SAL_CALL
BasePrimitive2D::getDecomposition(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    const geometry::ViewInformation2D aViewInformation(rViewParameters);
    Primitive2DContainer aContainer;
    get2DDecomposition(aContainer, aViewInformation);
    return comphelper::containerToSequence(aContainer);
}

css::geometry::RealRectangle2D SAL_CALL
BasePrimitive2D::getRange(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    const geometry::ViewInformation2D aViewInformation(rViewParameters);
    return basegfx::unotools::rectangle2DFromB2DRectangle(getB2DRange(aViewInformation));
}

sal_Int64 SAL_CALL BasePrimitive2D::estimateUsage()
{
    return 0; // for now ignore the objects themselves
}

BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D()
    : BasePrimitive2D()
    , maBuffered2DDecomposition()
{
}

void BufferedDecompositionPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (getBuffered2DDecomposition().empty())
    {
        Primitive2DContainer aNewSequence;
        create2DDecomposition(aNewSequence, rViewInformation);
        const_cast<BufferedDecompositionPrimitive2D*>(this)->setBuffered2DDecomposition(
            aNewSequence);
    }

    rVisitor.append(getBuffered2DDecomposition());
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
