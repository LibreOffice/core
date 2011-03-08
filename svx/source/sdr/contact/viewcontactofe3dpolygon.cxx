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
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofe3dpolygon.hxx>
#include <svx/polygn3d.hxx>
#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewContactOfE3dPolygon::ViewContactOfE3dPolygon(E3dPolygonObj& rPolygon)
        :   ViewContactOfE3d(rPolygon)
        {
        }

        ViewContactOfE3dPolygon::~ViewContactOfE3dPolygon()
        {
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewContactOfE3dPolygon::createViewIndependentPrimitive3DSequence() const
        {
            drawinglayer::primitive3d::Primitive3DSequence xRetval;
            const SfxItemSet& rItemSet = GetE3dPolygonObj().GetMergedItemSet();
            const bool bSuppressFill(GetE3dPolygonObj().GetLineOnly());
            const drawinglayer::attribute::SdrLineFillShadowAttribute3D aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowAttribute(rItemSet, bSuppressFill));

            // get extrude geometry
            basegfx::B3DPolyPolygon aPolyPolygon3D(GetE3dPolygonObj().GetPolyPolygon3D());
            const basegfx::B3DPolyPolygon aPolyNormals3D(GetE3dPolygonObj().GetPolyNormals3D());
            const basegfx::B2DPolyPolygon aPolyTexture2D(GetE3dPolygonObj().GetPolyTexture2D());
            const bool bNormals(aPolyNormals3D.count() && aPolyNormals3D.count() == aPolyPolygon3D.count());
            const bool bTexture(aPolyTexture2D.count() && aPolyTexture2D.count() == aPolyPolygon3D.count());

            if(bNormals || bTexture)
            {
                for(sal_uInt32 a(0L); a < aPolyPolygon3D.count(); a++)
                {
                    basegfx::B3DPolygon aCandidate3D(aPolyPolygon3D.getB3DPolygon(a));
                    basegfx::B3DPolygon aNormals3D;
                    basegfx::B2DPolygon aTexture2D;

                    if(bNormals)
                    {
                        aNormals3D = aPolyNormals3D.getB3DPolygon(a);
                    }

                    if(bTexture)
                    {
                        aTexture2D = aPolyTexture2D.getB2DPolygon(a);
                    }

                    for(sal_uInt32 b(0L); b < aCandidate3D.count(); b++)
                    {
                        if(bNormals)
                        {
                            sal_uInt32 nNormalCount = aNormals3D.count();
                            if( b < nNormalCount )
                                aCandidate3D.setNormal(b, aNormals3D.getB3DPoint(b));
                            else if( nNormalCount > 0 )
                                aCandidate3D.setNormal(b, aNormals3D.getB3DPoint(0));
                        }
                        if(bTexture)
                        {
                            sal_uInt32 nTextureCount = aTexture2D.count();
                            if( b < nTextureCount )
                                aCandidate3D.setTextureCoordinate(b, aTexture2D.getB2DPoint(b));
                            else if( nTextureCount > 0 )
                                aCandidate3D.setTextureCoordinate(b, aTexture2D.getB2DPoint(0));
                        }
                    }

                    aPolyPolygon3D.setB3DPolygon(a, aCandidate3D);
                }
            }

            // get 3D Object Attributes
            drawinglayer::attribute::Sdr3DObjectAttribute* pSdr3DObjectAttribute = drawinglayer::primitive2d::createNewSdr3DObjectAttribute(rItemSet);

            // calculate texture size
            basegfx::B2DVector aTextureSize(1.0, 1.0);

            if(bTexture)
            {
                // #i98314#
                // create texture size from object's size
                const basegfx::B3DRange aObjectRange(basegfx::tools::getRange(aPolyPolygon3D));

                double fWidth(0.0);
                double fHeight(0.0);

                // this is a polygon object, so Width/Height and/or Depth may be zero (e.g. left
                // wall of chart). Take this into account
                if(basegfx::fTools::equalZero(aObjectRange.getWidth()))
                {
                    // width is zero, use height and depth
                    fWidth = aObjectRange.getHeight();
                    fHeight = aObjectRange.getDepth();
                }
                else if(basegfx::fTools::equalZero(aObjectRange.getHeight()))
                {
                    // height is zero, use width and depth
                    fWidth = aObjectRange.getWidth();
                    fHeight = aObjectRange.getDepth();
                }
                else
                {
                    // use width and height
                    fWidth = aObjectRange.getWidth();
                    fHeight = aObjectRange.getHeight();
                }

                if(basegfx::fTools::lessOrEqual(fWidth, 0.0) ||basegfx::fTools::lessOrEqual(fHeight, 0.0))
                {
                    // no texture; fallback to very small size
                    aTextureSize.setX(0.01);
                    aTextureSize.setY(0.01);
                }
                else
                {
                    aTextureSize.setX(fWidth);
                    aTextureSize.setY(fHeight);
                }
            }

            // #i98295#
            // unfortunately, this SdrObject type which allows a free 3d geometry definition was defined
            // wrong topologically in relation to it's plane normal and 3D visibility when it was invented
            // a long time ago. Since the API allows creation of this SDrObject type, it is not possible to
            // simply change this definition. Only the chart should use it, and at least this object type
            // only exists at Runtime (is not saved and/or loaded in any FileFormat). Still someone external
            // may have used it in it's API. To not risk wrong 3D lightings, i have to switch the orientation
            // of the polygon here
            aPolyPolygon3D.flip();

            // create primitive and add
            const basegfx::B3DHomMatrix aWorldTransform;
            const drawinglayer::primitive3d::Primitive3DReference xReference(
                new drawinglayer::primitive3d::SdrPolyPolygonPrimitive3D(
                    aPolyPolygon3D, aWorldTransform, aTextureSize, aAttribute, *pSdr3DObjectAttribute));
            xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);

            // delete 3D Object Attributes
            delete pSdr3DObjectAttribute;

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
