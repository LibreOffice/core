/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayobjectcell.cxx,v $
 * $Revision: 1.3 $
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

#include <basegfx/numeric/ftools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/hatch.hxx>
#include <svx/sdr/overlay/overlayobjectcell.hxx>

using namespace ::basegfx;

// #114409#
namespace sdr
{
    namespace overlay
    {
        OverlayObjectCell::OverlayObjectCell( CellOverlayType eType, const Color& rColor, const RangeVector& rRects )
        :   OverlayObject( rColor ),
            mePaintType( eType ),
            maRectangles( rRects )
        {
        }

        OverlayObjectCell::~OverlayObjectCell()
        {
        }

        void OverlayObjectCell::drawGeometry(OutputDevice& rOutputDevice)
        {
            // set colors
            rOutputDevice.SetLineColor();
            rOutputDevice.SetFillColor(getBaseColor());

            if ( mePaintType == CELL_OVERLAY_INVERT )
            {
                rOutputDevice.Push();
                rOutputDevice.SetRasterOp( ROP_XOR );
                rOutputDevice.SetFillColor( COL_WHITE );
            }

            for(sal_uInt32 a(0L);a < maRectangles.size(); a++)
            {
                const basegfx::B2DRange& rRange(maRectangles[a]);
                const Rectangle aRectangle(fround(rRange.getMinX()), fround(rRange.getMinY()), fround(rRange.getMaxX()), fround(rRange.getMaxY()));

                switch(mePaintType)
                {
                    case CELL_OVERLAY_INVERT :
                    {
                        rOutputDevice.DrawRect( aRectangle );

                        // if(OUTDEV_WINDOW == rOutputDevice.GetOutDevType())
                        // {
                        //  ((Window&)rOutputDevice).Invert(aRectangle, INVERT_HIGHLIGHT);
                        // }

                        break;
                    }
                    case CELL_OVERLAY_HATCH :
                    {
                        rOutputDevice.DrawHatch(PolyPolygon(Polygon(aRectangle)), Hatch(HATCH_SINGLE, getBaseColor(), 2, 450));
                        break;
                    }
                    case CELL_OVERLAY_TRANSPARENT :
                    {
                        rOutputDevice.DrawTransparent(PolyPolygon(Polygon(aRectangle)), 50);
                        break;
                    }
                    case CELL_OVERLAY_LIGHT_TRANSPARENT :
                    {
                        rOutputDevice.DrawTransparent(PolyPolygon(Polygon(aRectangle)), 80);
                        break;
                    }
                }
            }

            if ( mePaintType == CELL_OVERLAY_INVERT )
                rOutputDevice.Pop();
        }

        void OverlayObjectCell::createBaseRange(OutputDevice& /*rOutputDevice*/)
        {
            maBaseRange.reset();

            for(sal_uInt32 a(0L); a < maRectangles.size(); a++)
            {
                maBaseRange.expand(maRectangles[a]);
            }
        }

        void OverlayObjectCell::transform(const basegfx::B2DHomMatrix& rMatrix)
        {
            for(sal_uInt32 a(0L); a < maRectangles.size(); a++)
            {
                maRectangles[a].transform(rMatrix);
            }
        }

    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
