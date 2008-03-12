/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: overlayobjectcell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:34:16 $
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

#ifndef _SDR_OVERLAY_OVERLAYOBJECTCELL_HXX
#define _SDR_OVERLAY_OVERLAYOBJECTCELL_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////

// #114409#
namespace sdr
{
    namespace overlay
    {
        enum CellOverlayType { CELL_OVERLAY_INVERT, CELL_OVERLAY_HATCH, CELL_OVERLAY_TRANSPARENT, CELL_OVERLAY_LIGHT_TRANSPARENT };

        // OverlayObjectCell - used for cell cursor, selection and AutoFill handle

        class SVX_DLLPUBLIC OverlayObjectCell : public OverlayObject
        {
        public:
            typedef ::std::vector< basegfx::B2DRange > RangeVector;

        private:
            CellOverlayType mePaintType;
            RangeVector     maRectangles;

            virtual void drawGeometry(OutputDevice& rOutputDevice);
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayObjectCell( CellOverlayType eType, const Color& rColor, const RangeVector& rRects);
            virtual ~OverlayObjectCell();

            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };

    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYLINE_HXX

// eof
