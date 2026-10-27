/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <svx/msdffdef.hxx>
#include <svx/svxdllapi.h>
#include <svx/EnhancedCustomShapeFunctionParser.hxx>
#include <tools/gen.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <cpo/uno/Sequence.h>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>

#include <memory>
#include <vector>

class SdrCustomShapeGeometryItem;

/// geometry of a custom shape from its CustomShapeGeometry for a given logic rectangle:
/// view box, path size, adjustments, equations, coordinates, glue points, handles and
/// text frames, no SdrObject needed
class SVXCORE_DLLPUBLIC EnhancedCustomShapeFormulas
{
protected:
    MSO_SPT m_eSpType;

    sal_Int32 m_nCoordLeft;
    sal_Int32 m_nCoordTop;
    sal_Int32 m_nCoordWidthG;
    sal_Int32 m_nCoordHeightG;
    sal_Int32 m_nCoordWidth;
    sal_Int32 m_nCoordHeight;
    tools::Rectangle m_aLogicRect;

    double m_fXScale;
    double m_fYScale;
    double m_fXRatio;
    double m_fYRatio;

    bool m_bOOXMLShape;

    sal_Int32 m_nXRef;
    sal_Int32 m_nYRef;

    struct SAL_DLLPRIVATE EquationResult
    {
        mutable bool bParsed = false;
        bool bReady = false;
        double fValue = 0;
        mutable std::shared_ptr<EnhancedCustomShape::ExpressionNode> xNode;
    };
    cpo::uno::Sequence<OUString> m_seqEquations;
    std::vector<EquationResult> m_vEquationResults;

    cpo::uno::Sequence<css::drawing::EnhancedCustomShapeSegment> m_seqSegments;
    cpo::uno::Sequence<css::drawing::EnhancedCustomShapeParameterPair> m_seqCoordinates;
    cpo::uno::Sequence<css::drawing::EnhancedCustomShapeTextFrame> m_seqTextFrames;
    cpo::uno::Sequence<css::drawing::EnhancedCustomShapeParameterPair> m_seqGluePoints;
    cpo::uno::Sequence<css::drawing::EnhancedCustomShapeAdjustmentValue> m_seqAdjustmentValues;
    cpo::uno::Sequence<css::beans::PropertyValues> m_seqHandles;
    cpo::uno::Sequence<css::awt::Size> m_seqSubViewSize;
    cpo::uno::Sequence<double> m_seqGluePointLeavingDirections;

    bool m_bFilled;
    bool m_bStroked;

    bool m_bFlipH;
    bool m_bFlipV;

    SAL_DLLPRIVATE bool SetAdjustValueAsDouble(const double& rValue, const sal_Int32 nIndex);
    SAL_DLLPRIVATE Point GetPoint(const css::drawing::EnhancedCustomShapeParameterPair&,
                                  const bool bScale = true,
                                  const bool bReplaceGeoSize = false) const;
    SAL_DLLPRIVATE basegfx::B2DPoint
    GetPointAsB2DPoint(const css::drawing::EnhancedCustomShapeParameterPair&,
                       const bool bScale = true, const bool bReplaceGeoSize = false) const;
    SAL_DLLPRIVATE void ApplyShapeAttributes(const SdrCustomShapeGeometryItem& rItem);
    SAL_DLLPRIVATE void SetPathSize(sal_Int32 nIndex = 0);

public:
    /// bFilled and bStroked are what the equations ask for as hasfill and hasstroke
    EnhancedCustomShapeFormulas(const SdrCustomShapeGeometryItem& rGeometryItem,
                                const tools::Rectangle& rLogicRect, bool bFilled, bool bStroked);

    /// the text frame of the geometry rGeometry in a shape of rSize, in the unit of rSize and
    /// from the shape's top left
    static tools::Rectangle
    GetTextRectFor(const cpo::uno::Sequence<css::beans::PropertyValue>& rGeometry,
                   const Size& rSize);

    SAL_DLLPRIVATE bool IsFlipVert() const { return m_bFlipV; };
    SAL_DLLPRIVATE bool IsFlipHorz() const { return m_bFlipH; };
    const tools::Rectangle& GetLogicRect() const { return m_aLogicRect; }
    tools::Rectangle GetTextRect() const;

    /// Drops what the formulas were worked out to, so that they are worked out again from
    /// whatever the values now say.
    SAL_DLLPRIVATE void ForgetTheFormulaResults();

    SAL_DLLPRIVATE double GetEnumFunc(const EnhancedCustomShape::ExpressionFunct eVal) const;

    void GetParameter(double& rParameterReturnValue,
                      const css::drawing::EnhancedCustomShapeParameter&,
                      const bool bReplaceGeoWidth, const bool bReplaceGeoHeight) const;
    SAL_DLLPRIVATE double GetAdjustValueAsDouble(const sal_Int32 nIndex) const;
    SAL_DLLPRIVATE double GetEquationValueAsDouble(const sal_Int32 nIndex) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
