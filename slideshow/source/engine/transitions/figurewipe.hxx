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



#if ! defined INCLUDED_SLIDESHOW_FIGUREWIPE_HXX
#define INCLUDED_SLIDESHOW_FIGUREWIPE_HXX

#include "parametricpolypolygon.hxx"


namespace slideshow {
namespace internal {

class FigureWipe : public ParametricPolyPolygon
{
public:
    static FigureWipe * createTriangleWipe();
    static FigureWipe * createArrowHeadWipe();
    static FigureWipe * createStarWipe( sal_Int32 nPoints );
    static FigureWipe * createPentagonWipe();
    static FigureWipe * createHexagonWipe();

    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    FigureWipe( ::basegfx::B2DPolygon const & figure ) : m_figure(figure) {}
    const ::basegfx::B2DPolygon m_figure;
};

}
}

#endif /* INCLUDED_SLIDESHOW_FIGUREWIPE_HXX */
