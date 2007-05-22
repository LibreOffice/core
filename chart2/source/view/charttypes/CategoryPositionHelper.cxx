/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CategoryPositionHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:15:43 $
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

#ifndef _CHART2_CATEGORYPOSITIONHELPER_HXX
#include "CategoryPositionHelper.hxx"
#endif

//.............................................................................
namespace chart
{
//.............................................................................
//using namespace ::com::sun::star;
//using namespace ::com::sun::star::chart2;

CategoryPositionHelper::CategoryPositionHelper( double fSeriesCount, double fCategoryWidth )
    : m_fSeriesCount(fSeriesCount)
    , m_fCategoryWidth(fCategoryWidth)
    , m_fInnerDistance(0.0)
    , m_fOuterDistance(1.0)
{
}

CategoryPositionHelper::CategoryPositionHelper( const CategoryPositionHelper& rSource )
    : m_fSeriesCount( rSource.m_fSeriesCount )
    , m_fCategoryWidth( rSource.m_fCategoryWidth )
    , m_fInnerDistance( rSource.m_fInnerDistance )
    , m_fOuterDistance( rSource.m_fOuterDistance )
{
}

CategoryPositionHelper::CategoryPositionHelper()
    : m_fSeriesCount(1.0)
    , m_fCategoryWidth(1.0)
    , m_fInnerDistance(0.0)
    , m_fOuterDistance(0.0)
{
}

CategoryPositionHelper::~CategoryPositionHelper()
{
}

double CategoryPositionHelper::getSlotWidth() const
{
    double fWidth = m_fCategoryWidth /
                (  m_fSeriesCount
                 + m_fOuterDistance
                 + m_fInnerDistance*( m_fSeriesCount - 1.0) );
    return fWidth;
}

double CategoryPositionHelper::getSlotPos( double fCategoryX, double fSeriesNumber ) const
{
    //the returned position is in the middle of the rect
    //fSeriesNumber 0...n-1
    double fPos = fCategoryX - (m_fCategoryWidth/2.0)
           + (m_fOuterDistance/2.0 + fSeriesNumber*(1.0+m_fInnerDistance)) * getSlotWidth()
           + getSlotWidth()/2.0;

    return fPos;
}

void CategoryPositionHelper::setInnerDistance( double fInnerDistance )
{
    if( fInnerDistance < -1.0 )
        fInnerDistance = -1.0;
    if( fInnerDistance > 1.0 )
        fInnerDistance = 1.0;
    m_fInnerDistance = fInnerDistance;
}

void CategoryPositionHelper::setOuterDistance( double fOuterDistance )
{
    if( fOuterDistance < 0.0 )
        fOuterDistance = 0.0;
    if( fOuterDistance > 6.0 )
        fOuterDistance = 6.0;
    m_fOuterDistance = fOuterDistance;
}

//.............................................................................
} //namespace chart
//.............................................................................
