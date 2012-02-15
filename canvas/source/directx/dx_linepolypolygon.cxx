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
#include "precompiled_canvas.hxx"

#include <basegfx/tools/canvastools.hxx>
#include "dx_linepolypolygon.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    LinePolyPolygon::LinePolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly ) :
        ::basegfx::unotools::UnoPolyPolygon( rPolyPoly ),
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        mpPath()
    {
    }

    GraphicsPathSharedPtr LinePolyPolygon::getGraphicsPath( bool bNoLineJoin ) const
    {
        // generate GraphicsPath only on demand (gets deleted as soon
        // as any of the modifying methods above touches the
        // B2DPolyPolygon).
        if( !mpPath )
        {
            mpPath = tools::graphicsPathFromB2DPolyPolygon( getPolyPolygonUnsafe(), bNoLineJoin );
            mpPath->SetFillMode( const_cast<LinePolyPolygon*>(this)->getFillRule() == rendering::FillRule_EVEN_ODD ?
                                 Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding );
        }

        return mpPath;
    }

    void LinePolyPolygon::modifying() const
    {
        mpPath.reset();
    }
}
