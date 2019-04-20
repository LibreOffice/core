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

#include "LabelPositionHelper.hxx"
#include <com/sun/star/awt/Point.hpp>

namespace chart
{

class PolarPlottingPositionHelper;

class PolarLabelPositionHelper : public LabelPositionHelper
{
public:
    PolarLabelPositionHelper(
        PolarPlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const css::uno::Reference< css::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~PolarLabelPositionHelper() override;

    css::awt::Point getLabelScreenPositionAndAlignmentForLogicValues(
                        LabelAlignment& rAlignment
                        , double fLogicValueOnAngleAxis
                        , double fLogicValueOnRadiusAxis
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const;

    /** Calculate the anchor point position for a text label.
     *  When the requested label placement is of `INSIDE` or `OUTSIDE` type the
     *  returned anchor point for the text label is the middle point of the
     *  outer arc for the given slice; when the requested label placement is of
     *  `CENTER` type the returned anchor point for the text label is the
     *  middle point of the line segment bisecting the slice.
     *  The text alignment is always centered when the requested label
     *  placement is of `CENTER` type else it is dependent on the value of the
     *  angle defined by the horizontal axis and the ray bisecting the slice.
     *
     */
    css::awt::Point getLabelScreenPositionAndAlignmentForUnitCircleValues(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement /*see css::chart::DataLabelPlacement*/
                        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection ) const;

private:
    PolarPlottingPositionHelper*    m_pPosHelper;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
