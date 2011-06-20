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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "DragMethod_PieSegment.hxx"

#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"
#include "ObjectIdentifier.hxx"
#include <rtl/math.hxx>
#include <svx/svdpagv.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::basegfx::B2DVector;

DragMethod_PieSegment::DragMethod_PieSegment( DrawViewWrapper& rDrawViewWrapper
                                             , const rtl::OUString& rObjectCID
                                             , const Reference< frame::XModel >& xChartModel )
    : DragMethod_Base( rDrawViewWrapper, rObjectCID, xChartModel )
    , m_aStartVector(100.0,100.0)
    , m_fInitialOffset(0.0)
    , m_fAdditionalOffset(0.0)
    , m_aDragDirection(1000.0,1000.0)
    , m_fDragRange( 1.0 )
{
    rtl::OUString aParameter( ObjectIdentifier::getDragParameterString( m_aObjectCID ) );

    sal_Int32 nOffsetPercent(0);
    awt::Point aMinimumPosition(0,0);
    awt::Point aMaximumPosition(0,0);

    ObjectIdentifier::parsePieSegmentDragParameterString(
          aParameter, nOffsetPercent, aMinimumPosition, aMaximumPosition );

    m_fInitialOffset = nOffsetPercent / 100.0;
    if( m_fInitialOffset < 0.0 )
        m_fInitialOffset = 0.0;
    if( m_fInitialOffset > 1.0 )
        m_fInitialOffset = 1.0;
    B2DVector aMinVector( aMinimumPosition.X, aMinimumPosition.Y );
    B2DVector aMaxVector( aMaximumPosition.X, aMaximumPosition.Y );
    m_aDragDirection = aMaxVector - aMinVector;
    m_fDragRange = m_aDragDirection.scalar( m_aDragDirection );
    if( ::rtl::math::approxEqual( m_fDragRange, 0.0 ) )
        m_fDragRange = 1.0;
}
DragMethod_PieSegment::~DragMethod_PieSegment()
{
}
void DragMethod_PieSegment::TakeSdrDragComment(String& rStr) const
{
    rStr = String( SchResId( STR_STATUS_PIE_SEGMENT_EXPLODED ) );
    rStr.SearchAndReplaceAscii( "%PERCENTVALUE", String::CreateFromInt32( static_cast<sal_Int32>((m_fAdditionalOffset+m_fInitialOffset)*100.0) ));
}
bool DragMethod_PieSegment::BeginSdrDrag()
{
    Point aStart( DragStat().GetStart() );
    m_aStartVector = B2DVector( aStart.X(), aStart.Y() );
    Show();
    return true;
}
void DragMethod_PieSegment::MoveSdrDrag(const Point& rPnt)
{
    if( DragStat().CheckMinMoved(rPnt) )
    {
        //calculate new offset
        B2DVector aShiftVector(( B2DVector( rPnt.X(), rPnt.Y() ) - m_aStartVector ));
        m_fAdditionalOffset = m_aDragDirection.scalar( aShiftVector )/m_fDragRange; // projection

        if( m_fAdditionalOffset < -m_fInitialOffset )
            m_fAdditionalOffset = -m_fInitialOffset;
        else if( m_fAdditionalOffset > (1.0-m_fInitialOffset) )
            m_fAdditionalOffset = 1.0 - m_fInitialOffset;

        B2DVector aNewPosVector = m_aStartVector + (m_aDragDirection * m_fAdditionalOffset);
        Point aNewPos = Point( (long)(aNewPosVector.getX()), (long)(aNewPosVector.getY()) );
        if( aNewPos != DragStat().GetNow() )
        {
            Hide();
            DragStat().NextMove( aNewPos );
            Show();
        }
    }
}
bool DragMethod_PieSegment::EndSdrDrag(bool /*bCopy*/)
{
    Hide();

    try
    {
        Reference< frame::XModel > xChartModel( this->getChartModel() );
        if( xChartModel.is() )
        {
            Reference< beans::XPropertySet > xPointProperties(
                ObjectIdentifier::getObjectPropertySet( m_aObjectCID, xChartModel ) );
            if( xPointProperties.is() )
                xPointProperties->setPropertyValue( C2U( "Offset" ), uno::makeAny( m_fAdditionalOffset+m_fInitialOffset ));
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return true;
}
basegfx::B2DHomMatrix DragMethod_PieSegment::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval;

    aRetval.translate(DragStat().GetDX(), DragStat().GetDY());

    return aRetval;
}
void DragMethod_PieSegment::createSdrDragEntries()
{
    SdrObject* pObj = m_rDrawViewWrapper.getSelectedObject();
    SdrPageView* pPV = m_rDrawViewWrapper.GetPageView();

    if( pObj && pPV )
    {
        const basegfx::B2DPolyPolygon aNewPolyPolygon(pObj->TakeXorPoly());
        addSdrDragEntry(new SdrDragEntryPolyPolygon(aNewPolyPolygon));
    }
}
//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
