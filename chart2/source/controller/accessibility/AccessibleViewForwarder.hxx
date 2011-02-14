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
#ifndef _CHART2_ACCESSIBLEVIEWFORWARDER_HXX_
#define _CHART2_ACCESSIBLEVIEWFORWARDER_HXX_

#include <vcl/mapmod.hxx>
#include <svx/IAccessibleViewForwarder.hxx>

class Window;

namespace chart
{

class AccessibleChartView;

class AccessibleViewForwarder : public ::accessibility::IAccessibleViewForwarder
{
public:
    AccessibleViewForwarder( AccessibleChartView* pAccChartView, Window* pWindow );
    virtual ~AccessibleViewForwarder();

    // ________ IAccessibleViewforwarder ________
    virtual sal_Bool IsValid() const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel( const Point& rPoint ) const;
    virtual Size LogicToPixel( const Size& rSize ) const;
    virtual Point PixelToLogic( const Point& rPoint ) const;
    virtual Size PixelToLogic( const Size& rSize ) const;

private:
    AccessibleViewForwarder( AccessibleViewForwarder& );
    AccessibleViewForwarder& operator=( AccessibleViewForwarder& );

    AccessibleChartView* m_pAccChartView;
    Window* m_pWindow;
    MapMode m_aMapMode;
};

}  // namespace chart

#endif
