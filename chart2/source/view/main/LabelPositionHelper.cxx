/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LabelPositionHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:37:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif

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

awt::Point LabelPositionHelper::transformScaledLogicToScreenPosition( const drawing::Position3D& rLogicPosition3D ) const
{
    drawing::Position3D aScenePosition3D( SequenceToPosition3D(
        m_pPosHelper->getTransformationScaledLogicToScene()->transform(
            Position3DToSequence(rLogicPosition3D) ) ) );
    /*
    if(3==m_nDimensionCount)
    {
        drawing::Position3D aScenePosition3D_rotated( aScenePosition3D.PositionX, -aScenePosition3D.PositionZ, aScenePosition3D.PositionY );
        aScenePosition3D = aScenePosition3D_rotated;
    }
    */
    awt::Point aScreenPosition2D( this->transformSceneToScreenPosition( aScenePosition3D ) );
    return aScreenPosition2D;
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
    if( xAxisModelProps->getPropertyValue( C2U("ReferenceDiagramSize")) >>= aOldReferenceSize )
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
