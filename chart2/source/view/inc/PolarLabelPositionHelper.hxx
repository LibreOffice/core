/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX

#include "LabelPositionHelper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class PolarLabelPositionHelper : public LabelPositionHelper
{
public:
    PolarLabelPositionHelper(
        PolarPlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~PolarLabelPositionHelper();

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForLogicValues(
                        LabelAlignment& rAlignment
                        , double fLogicValueOnAngleAxis
                        , double fLogicValueOnRadiusAxis
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForUnitCircleValues(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement /*see ::com::sun::star::chart::DataLabelPlacement*/
                        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

private:
    PolarPlottingPositionHelper*    m_pPosHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
