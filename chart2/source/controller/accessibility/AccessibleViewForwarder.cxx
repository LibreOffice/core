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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
