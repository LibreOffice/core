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
#include "precompiled_svx.hxx"

#include <svx/sdrobjecttools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdlegacy.hxx>

//////////////////////////////////////////////////////////////////////////////

void initializeDefaultSdrPathObjByObjectType(SdrPathObj& rObj, DefaultSdrPathObjType eType, const basegfx::B2DRange& rRange, bool bClose)
{
    basegfx::B2DPolyPolygon aNewPolyPolygon;

    switch(eType)
    {
        case DefaultSdrPathObjType_Line:
        {
            basegfx::B2DPolygon aInnerPoly;

            aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getCenterY()));
            aInnerPoly.append(basegfx::B2DPoint(rRange.getMaxX(), rRange.getCenterY()));
            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
        case DefaultSdrPathObjType_BezierFill:
        {
            const basegfx::B2DPolygon aInnerPoly(
                basegfx::tools::createPolygonFromEllipse(
                    rRange.getCenter(),
                    rRange.getWidth() * 0.5,
                    rRange.getHeight() * 0.5));

            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
        case DefaultSdrPathObjType_Bezier:
        {
            basegfx::B2DPolygon aInnerPoly;
            const basegfx::B2DPoint aTopLeft(rRange.getMinimum());
            const basegfx::B2DVector aScale(rRange.getRange());

            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.0, 1.0)));
            aInnerPoly.appendBezierSegment(
                aTopLeft + (aScale * basegfx::B2DTuple(0.25, 1.0)),
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 1.0)),
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.5)));
            aInnerPoly.appendBezierSegment(
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.0)),
                aTopLeft + (aScale * basegfx::B2DTuple(0.75, 0.0)),
                aTopLeft + (aScale * basegfx::B2DTuple(1.0, 0.0)));
            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
        case DefaultSdrPathObjType_Freeline:
        {
            basegfx::B2DPolygon aInnerPoly;
            const basegfx::B2DPoint aTopLeft(rRange.getMinimum());
            const basegfx::B2DVector aScale(rRange.getRange());

            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.0, 1.0)));
            aInnerPoly.appendBezierSegment(
                aTopLeft + (aScale * basegfx::B2DTuple(0.0, 0.5)),
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.25)),
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.5)));
            aInnerPoly.appendBezierSegment(
                aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.75)),
                aTopLeft + (aScale * basegfx::B2DTuple(1.0, 0.5)),
                aTopLeft + (aScale * basegfx::B2DTuple(1.0, 0.0)));

            if(bClose)
            {
                aInnerPoly.append(aTopLeft + aScale);
            }

            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
        case DefaultSdrPathObjType_Polygon:
        {
            basegfx::B2DPolygon aInnerPoly;
            const basegfx::B2DPoint aTopLeft(rRange.getMinimum());
            const basegfx::B2DVector aScale(rRange.getRange());

            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.1, 1.0)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.3, 0.6)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.0, 0.3)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.2, 0.0)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.6, 0.2)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.8, 0.0)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.7, 0.8)));
            aInnerPoly.append(aTopLeft + aScale);

            if(!bClose)
            {
                aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.6, 1.0)));
            }

            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
        case DefaultSdrPathObjType_XPolygon:
        {
            basegfx::B2DPolygon aInnerPoly;
            const basegfx::B2DPoint aTopLeft(rRange.getMinimum());
            const basegfx::B2DVector aScale(rRange.getRange());

            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.0, 1.0)));
            aInnerPoly.append(aTopLeft);
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.0)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.5, 0.5)));
            aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(1.0, 0.5)));
            aInnerPoly.append(aTopLeft + aScale);

            if(!bClose)
            {
                aInnerPoly.append(aTopLeft + (aScale * basegfx::B2DTuple(0.7, 1.0)));
            }

            aNewPolyPolygon.append(aInnerPoly);
            break;
        }
    }

    if(bClose)
    {
        aNewPolyPolygon.setClosed(true);
    }

    rObj.setB2DPolyPolygonInObjectCoordinates(aNewPolyPolygon);
}

//////////////////////////////////////////////////////////////////////////////
// eof
