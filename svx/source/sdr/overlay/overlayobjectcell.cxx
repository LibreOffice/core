/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayobjectcell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:47:10 $
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
