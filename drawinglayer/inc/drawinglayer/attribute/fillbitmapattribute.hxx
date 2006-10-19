/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillbitmapattribute.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:30:35 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DRange;
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class FillBitmapAttribute
        {
            Bitmap                                      maBitmap;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;

            // bitfield
            unsigned                                    mbTiling : 1;

        public:
            FillBitmapAttribute(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize, bool bTiling);
            bool operator==(const FillBitmapAttribute& rCandidate) const;

            // data access
            const Bitmap& getBitmap() const { return maBitmap; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            bool getTiling() const { return mbTiling; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX

// eof
