/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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
