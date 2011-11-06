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
