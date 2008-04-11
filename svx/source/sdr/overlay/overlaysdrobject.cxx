/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlaysdrobject.cxx,v $
 * $Revision: 1.6 $
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
#include <svx/sdr/overlay/overlaysdrobject.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdobj.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlaySdrObject::drawGeometry(OutputDevice& rOutputDevice)
        {
            XOutputDevice aExtOut(&rOutputDevice);
            SdrPaintInfoRec aInfoRec;
            mrSdrObject.SingleObjectPainter(aExtOut, aInfoRec);
        }

        void OverlaySdrObject::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            // reset range and expand it
            maBaseRange.reset();

            // get BoundRect
            Rectangle aBoundRect(mrSdrObject.GetCurrentBoundRect());
            const basegfx::B2DPoint aTopLeft(aBoundRect.Left(), aBoundRect.Top());
            const basegfx::B2DPoint aBottomRight(aBoundRect.Right(), aBoundRect.Bottom());

            maBaseRange.expand(aTopLeft);
            maBaseRange.expand(aBottomRight);
        }

        OverlaySdrObject::OverlaySdrObject(
            const basegfx::B2DPoint& rBasePos,
            const SdrObject& rObject)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_BLACK)),
            mrSdrObject(rObject)
        {
        }

        OverlaySdrObject::~OverlaySdrObject()
        {
        }

        sal_Bool OverlaySdrObject::isHit(const basegfx::B2DPoint& rPos, double fTol) const
        {
            if(isHittable())
            {
                Point aPnt( (long)rPos.getX(), (long)rPos.getY() );
                return mrSdrObject.CheckHit(aPnt, (USHORT)fTol, 0) != 0 ? sal_True : sal_False;
            }

            return sal_False;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
