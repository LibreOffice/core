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



#if ! defined INCLUDED_SLIDESHOW_SPIRALWIPE_HXX
#define INCLUDED_SLIDESHOW_SPIRALWIPE_HXX

#include "parametricpolypolygon.hxx"
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>


namespace slideshow {
namespace internal {

/// Generates a topLeftClockWise or
/// bottomLeftCounterClockWise (flipOnYAxis=true) spiral wipe:
class SpiralWipe : public ParametricPolyPolygon
{
public:
    SpiralWipe( sal_Int32 nElements, bool flipOnYAxis = false );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
protected:
    ::basegfx::B2DPolyPolygon calcNegSpiral( double t ) const;

    const sal_Int32 m_elements;
    const sal_Int32 m_sqrtElements;
    const bool m_flipOnYAxis;
};

/// Generates a twoBoxLeft or fourBoxHorizontal wipe:
class BoxSnakesWipe : public SpiralWipe
{
public:
    BoxSnakesWipe( sal_Int32 nElements, bool fourBox = false )
        : SpiralWipe(nElements), m_fourBox(fourBox) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    const bool m_fourBox;
};

}
}

#endif /* INCLUDED_SLIDESHOW_SPIRALWIPE_HXX */
