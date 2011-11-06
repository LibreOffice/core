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



#if ! defined INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX
#define INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX

#include "parametricpolypolygon.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>


namespace slideshow {
namespace internal {

/// Generates a left to right zigZag wipe:
class ZigZagWipe : public ParametricPolyPolygon
{
public:
    ZigZagWipe( sal_Int32 nZigs );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
protected:
    const double m_zigEdge;
    ::basegfx::B2DPolygon m_stdZigZag;
};

/// Generates a vertical barnZigZag wipe:
class BarnZigZagWipe : public ZigZagWipe
{
public:
    BarnZigZagWipe( sal_Int32 nZigs ) : ZigZagWipe(nZigs) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
};

}
}

#endif /* INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX */
