/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LabelPositionHelper.cxx,v $
 * $Revision: 1.7.44.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "LabelPositionHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
#include "PropertyMapper.hxx"
#include "ShapeFactory.hxx"
#include "macros.hxx"
#include "RelativeSizeHelper.hxx"
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LabelPositionHelper::LabelPositionHelper(
                    PlottingPositionHelper* pPosHelper
                    , sal_Int32 nDimensionCount
                    , const uno::Reference< drawing::XShapes >& xLogicTarget
                    , ShapeFactory* pShapeFactory )
                    : m_pPosHelper(pPosHelper)
                    , m_nDimensionCount(nDimensionCount)
                    , m_xLogicTarget(xLogicTarget)
                    , m_pShapeFactory(pShapeFactory)
{
}

LabelPositionHelper::~LabelPositionHelper()
{
}

awt::Point LabelPositionHelper::transformSceneToScreenPosition( const drawing::Position3D& rScenePosition3D ) const
{
    return PlottingPositionHelper::transformSceneToScreenPosition(
                  rScenePosition3D, m_xLogicTarget, m_pShapeFactory, m_nDimensionCount );
}

//static
void LabelPositionHelper::changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment)
{
    //HorizontalAdjustment
    {
        drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
        if( LABEL_ALIGN_RIGHT==eAlignment || LABEL_ALIGN_RIGHT_TOP==eAlignment || LABEL_ALIGN_RIGHT_BOTTOM==eAlignment )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_LEFT;
        else if( LABEL_ALIGN_LEFT==eAlignment || LABEL_ALIGN_LEFT_TOP==eAlignment || LABEL_ALIGN_LEFT_BOTTOM==eAlignment )
            eHorizontalAdjust = drawing::TextHorizontalAdjust_RIGHT;
        uno::Any* pHorizontalAdjustAny = PropertyMapper::getValuePointer(rPropValues,rPropNames,C2U("TextHorizontalAdjust"));
        if(pHorizontalAdjustAny)
            *pHorizontalAdjustAny = uno::makeAny(eHorizontalAdjust);
    }

    //VerticalAdjustment
    {
        drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
        if( LABEL_ALIGN_TOP==eAlignment || LABEL_ALIGN_RIGHT_TOP==eAlignment || LABEL_ALIGN_LEFT_TOP==eAlignment )
            eVerticalAdjust = drawing::TextVerticalAdjust_BOTTOM;
        else if( LABEL_ALIGN_BOTTOM==eAlignment || LABEL_ALIGN_RIGHT_BOTTOM==eAlignment || LABEL_ALIGN_LEFT_BOTTOM==eAlignment )
            eVerticalAdjust = drawing::TextVerticalAdjust_TOP;
        uno::Any* pVerticalAdjustAny = PropertyMapper::getValuePointer(rPropValues,rPropNames,C2U("TextVerticalAdjust"));
        if(pVerticalAdjustAny)
            *pVerticalAdjustAny = uno::makeAny(eVerticalAdjust);
    }
}

void lcl_doDynamicFontResize( uno::Any* pAOldAndNewFontHeightAny
                          , const awt::Size& rOldReferenceSize
                          , const awt::Size& rNewReferenceSize  )
{
    double fOldFontHeight = 0, fNewFontHeight;
    if( pAOldAndNewFontHeightAny && ( *pAOldAndNewFontHeightAny >>= fOldFontHeight ) )
    {
        fNewFontHeight = RelativeSizeHelper::calculate( fOldFontHeight, rOldReferenceSize, rNewReferenceSize );
        *pAOldAndNewFontHeightAny = uno::makeAny(fNewFontHeight);
    }
}

//static
void LabelPositionHelper::doDynamicFontResize( tAnySequence& rPropValues
                    , const tNameSequence& rPropNames
                    , const uno::Reference< beans::XPropertySet >& xAxisModelProps
                    , const awt::Size& rNewReferenceSize
                    )
{
    //-------------------------
    //handle dynamic font resize:
    awt::Size aOldReferenceSize;
    if( xAxisModelProps->getPropertyValue( C2U("ReferencePageSize")) >>= aOldReferenceSize )
    {
        uno::Any* pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, C2U("CharHeight") );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, C2U("CharHeightAsian") );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
        pAOldAndNewFontHeightAny = PropertyMapper::getValuePointer( rPropValues, rPropNames, C2U("CharHeightComplex") );
        lcl_doDynamicFontResize( pAOldAndNewFontHeightAny, aOldReferenceSize, rNewReferenceSize );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
