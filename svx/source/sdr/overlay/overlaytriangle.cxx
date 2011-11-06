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
#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <tools/poly.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayTriangle::createOverlayObjectPrimitive2DSequence()
        {
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(getBasePosition());
            aPolygon.append(getSecondPosition());
            aPolygon.append(getThirdPosition());
            aPolygon.setClosed(true);

            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aPolygon),
                    getBaseColor().getBColor()));

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayTriangle::OverlayTriangle(
            const basegfx::B2DPoint& rBasePos,
            const basegfx::B2DPoint& rSecondPos,
            const basegfx::B2DPoint& rThirdPos,
            Color aTriangleColor)
        :   OverlayObjectWithBasePosition(rBasePos, aTriangleColor),
            maSecondPosition(rSecondPos),
            maThirdPosition(rThirdPos)
        {
        }

        OverlayTriangle::~OverlayTriangle()
        {
        }

        void OverlayTriangle::setSecondPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maSecondPosition)
            {
                // remember new value
                maSecondPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayTriangle::setThirdPosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maThirdPosition)
            {
                // remember new value
                maThirdPosition = rNew;

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
