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
#include "CategoryPositionHelper.hxx"

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

CategoryPositionHelper::~CategoryPositionHelper()
{
}

double CategoryPositionHelper::getScaledSlotWidth() const
{
    double fWidth = m_fCategoryWidth /
                (  m_fSeriesCount
                 + m_fOuterDistance
                 + m_fInnerDistance*( m_fSeriesCount - 1.0) );
    return fWidth;
}

double CategoryPositionHelper::getScaledSlotPos( double fScaledXPos, double fSeriesNumber ) const
{
    //the returned position is in the middle of the rect
    //fSeriesNumber 0...n-1
    double fPos = fScaledXPos
           - (m_fCategoryWidth/2.0)
           + (m_fOuterDistance/2.0 + fSeriesNumber*(1.0+m_fInnerDistance)) * getScaledSlotWidth()
           + getScaledSlotWidth()/2.0;
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

void CategoryPositionHelper::setCategoryWidth( double fCategoryWidth )
{
    m_fCategoryWidth = fCategoryWidth;
}

//.............................................................................
} //namespace chart
//.............................................................................
