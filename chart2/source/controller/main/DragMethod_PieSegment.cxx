/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragMethod_PieSegment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:07:37 $
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

#include "DragMethod_PieSegment.hxx"

#include "Strings.hrc"
#include "ResId.hxx"
#include "macros.hxx"
#include "ObjectIdentifier.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
//header for class SdrPageView
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

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
void DragMethod_PieSegment::TakeComment(String& rStr) const
{
    rStr = String( SchResId( STR_STATUS_PIE_SEGMENT_EXPLODED ) );
    rStr.SearchAndReplaceAscii( "%PERCENTVALUE", String::CreateFromInt32( static_cast<sal_Int32>((m_fAdditionalOffset+m_fInitialOffset)*100.0) ));
}
void DragMethod_PieSegment::MovPoint(Point& rPnt)
{
    rPnt.X()+=DragStat().GetDX();
    rPnt.Y()+=DragStat().GetDY();
}
FASTBOOL DragMethod_PieSegment::Beg()
{
    Point aStart( DragStat().GetStart() );
    m_aStartVector = B2DVector( aStart.X(), aStart.Y() );

    SdrObject* pObj = m_rDrawViewWrapper.getSelectedObject();
    SdrPageView* pPV = m_rDrawViewWrapper.GetPageView();
    if( pObj && pPV )
    {
        pPV->setDragPoly0(pObj->TakeXorPoly(true));
        pPV->setDragPoly(pPV->getDragPoly0());
    }
    Show();
    return true;
}
void DragMethod_PieSegment::Mov(const Point& rPnt)
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
            MovAllPoints();
            Show();
        }
    }
}
FASTBOOL DragMethod_PieSegment::End(FASTBOOL bCopy)
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
//.............................................................................
} //namespace chart
//.............................................................................
