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
#include <basegfx/text/UnoTextLayouter.hxx>
#include <comphelper/sequence.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <com/sun/star/graphic/XTextLayouter.hpp>

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
    VisitingParameters const& mrParameters;
    basegfx::B2DRange maRetval;

    B2DRangeVisitor(VisitingParameters const& rParameters)
        : mrParameters(rParameters)
    {
    }

    virtual void append(const Primitive2DReference& rReference) override
    {
        maRetval.expand(getB2DRangeFromPrimitive2DReference(rReference, mrParameters));
    }

    virtual void append(const Primitive2DContainer& rReference) override
    {
        maRetval.expand(rReference.getB2DRange(mrParameters));
    }

    virtual void append(Primitive2DContainer&& rReference) override
    {
        maRetval.expand(rReference.getB2DRange(mrParameters));
    }
};
}

basegfx::B2DRange BasePrimitive2D::getB2DRange(VisitingParameters const& rParameters) const
{
    B2DRangeVisitor aVisitor(rParameters);
    get2DDecomposition(aVisitor, rParameters);
    return aVisitor.maRetval;
}

void BasePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& /*rVisitor*/,
                                         VisitingParameters const& /*rParameters*/) const
{
}

namespace
{
std::shared_ptr<gfx::ITextLayouter>
getTextLayouter(const uno::Sequence<beans::PropertyValue>& rProperties)
{
    std::shared_ptr<gfx::ITextLayouter> pTextLayouter;

    if (!rProperties.hasElements())
        return pTextLayouter;

    for (const beans::PropertyValue& rProperty : rProperties)
    {
        if (rProperty.Name == "TextLayouter")
        {
            uno::Reference<graphic::XTextLayouter> xTextLayouter;
            rProperty.Value >>= xTextLayouter;
            if (xTextLayouter.is())
                pTextLayouter = gfx::getTextLayouterFromUno(xTextLayouter);
            return pTextLayouter;
        }
    }
    return pTextLayouter;
}

} // end anonymous namespace

css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> SAL_CALL
BasePrimitive2D::getDecomposition(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    Primitive2DContainer aContainer;
    geometry::ViewInformation2D aViewInformation2D(rViewParameters);
    std::shared_ptr<gfx::ITextLayouter> pTextLayouter = getTextLayouter(rViewParameters);
    VisitingParameters aParameters(aViewInformation2D, pTextLayouter);
    get2DDecomposition(aContainer, aParameters);
    return comphelper::containerToSequence(aContainer);
}

css::geometry::RealRectangle2D SAL_CALL
BasePrimitive2D::getRange(const uno::Sequence<beans::PropertyValue>& rViewParameters)
{
    geometry::ViewInformation2D aViewInformation2D(rViewParameters);
    std::shared_ptr<gfx::ITextLayouter> pTextLayouter = getTextLayouter(rViewParameters);
    VisitingParameters aParameters(aViewInformation2D, pTextLayouter);
    return basegfx::unotools::rectangle2DFromB2DRectangle(getB2DRange(aParameters));
}

sal_Int64 SAL_CALL BasePrimitive2D::estimateUsage()
{
    return 0; // for now ignore the objects themselves
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
