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



#ifndef _CHART2_CATEGORYPOSITIONHELPER_HXX
#define _CHART2_CATEGORYPOSITIONHELPER_HXX

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class CategoryPositionHelper
{
public:
    CategoryPositionHelper( double fSeriesCount, double CategoryWidth = 1.0);
    CategoryPositionHelper( const CategoryPositionHelper& rSource );
    virtual ~CategoryPositionHelper();

    double getScaledSlotWidth() const;
    virtual double getScaledSlotPos( double fCategoryX, double fSeriesNumber ) const;
    virtual void setCategoryWidth( double fCategoryWidth );

    //Distance between two neighboring bars in same category, seen relative to width of the bar
    void setInnerDistance( double fInnerDistance );

    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    void setOuterDistance( double fOuterDistance );

protected:
    double m_fSeriesCount;
    double m_fCategoryWidth;
    //Distance between two neighboring bars in same category, seen relative to width of the bar:
    double m_fInnerDistance; //[-1,1] m_fInnerDistance=1 --> distance == width; m_fInnerDistance=-1-->all rects are painted on the same position
    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    double m_fOuterDistance; //>=0 m_fOuterDistance=1 --> distance == width
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
