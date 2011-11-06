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

#include "AccessibleViewForwarder.hxx"
#include "AccessibleChartView.hxx"

#include <vcl/window.hxx>

using namespace ::com::sun::star;


namespace chart
{

AccessibleViewForwarder::AccessibleViewForwarder( AccessibleChartView* pAccChartView, Window* pWindow )
    :m_pAccChartView( pAccChartView )
    ,m_pWindow( pWindow )
    ,m_aMapMode( MAP_100TH_MM )
{
}

AccessibleViewForwarder::~AccessibleViewForwarder()
{
}

// ________ IAccessibleViewforwarder ________

sal_Bool AccessibleViewForwarder::IsValid() const
{
    return sal_True;
}

Rectangle AccessibleViewForwarder::GetVisibleArea() const
{
    Rectangle aVisibleArea;
    if ( m_pWindow )
    {
        aVisibleArea.SetPos( Point( 0, 0 ) );
        aVisibleArea.SetSize( m_pWindow->GetOutputSizePixel() );
        aVisibleArea = m_pWindow->PixelToLogic( aVisibleArea, m_aMapMode );
    }
    return aVisibleArea;
}

Point AccessibleViewForwarder::LogicToPixel( const Point& rPoint ) const
{
    Point aPoint;
    if ( m_pAccChartView && m_pWindow )
    {
        awt::Point aLocation = m_pAccChartView->getLocationOnScreen();
        Point aTopLeft( aLocation.X, aLocation.Y );
        aPoint = m_pWindow->LogicToPixel( rPoint, m_aMapMode ) + aTopLeft;
    }
    return aPoint;
}

Size AccessibleViewForwarder::LogicToPixel( const Size& rSize ) const
{
    Size aSize;
    if ( m_pWindow )
    {
        aSize = m_pWindow->LogicToPixel( rSize, m_aMapMode );
    }
    return aSize;
}

Point AccessibleViewForwarder::PixelToLogic( const Point& rPoint ) const
{
    Point aPoint;
    if ( m_pAccChartView && m_pWindow )
    {
        awt::Point aLocation = m_pAccChartView->getLocationOnScreen();
        Point aTopLeft( aLocation.X, aLocation.Y );
        aPoint = m_pWindow->PixelToLogic( rPoint - aTopLeft, m_aMapMode );
    }
    return aPoint;
}

Size AccessibleViewForwarder::PixelToLogic( const Size& rSize ) const
{
    Size aSize;
    if ( m_pWindow )
    {
        aSize = m_pWindow->PixelToLogic( rSize, m_aMapMode );
    }
    return aSize;
}

} // namespace chart
