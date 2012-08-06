/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // helpers to create HitTestPrimitives Line
        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DHomMatrix& rMatrix);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolygon);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange,
            const basegfx::B2DHomMatrix& rMatrix);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolygon,
            const basegfx::B2DHomMatrix& rMatrix);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
