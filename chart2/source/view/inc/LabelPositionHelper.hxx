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

#ifndef _CHART2_VIEW_LABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_LABELPOSITIONHELPER_HXX

#include "LabelAlignment.hxx"
#include "PropertyMapper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PlottingPositionHelper;
class ShapeFactory;

class LabelPositionHelper
{
public:
    LabelPositionHelper(
        PlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~LabelPositionHelper();

    ::com::sun::star::awt::Point transformSceneToScreenPosition(
            const ::com::sun::star::drawing::Position3D& rScenePosition3D ) const;

    static void changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment);
    static void doDynamicFontResize(  tAnySequence& rPropValues, const tNameSequence& rPropNames
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisModelProps
                    , const ::com::sun::star::awt::Size& rNewReferenceSize );

    static void correctPositionForRotation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape2DText
                    , LabelAlignment eLabelAlignment, const double fRotationAngle, bool bRotateAroundCenter );

private:
    LabelPositionHelper();

protected:
    PlottingPositionHelper*  m_pPosHelper;
    sal_Int32                m_nDimensionCount;

private:
    //these members are only necessary for transformation from 3D to 2D
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >    m_xLogicTarget;
    ShapeFactory*                                           m_pShapeFactory;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
