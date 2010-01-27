/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrfillbitmapattribute.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class Bitmap;

namespace basegfx {
    class B2DRange;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class FillBitmapAttribute;
    class ImpSdrFillBitmapAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFillBitmapAttribute
        {
        private:
            ImpSdrFillBitmapAttribute*          mpSdrFillBitmapAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFillBitmapAttribute(
                const Bitmap& rBitmap,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize);
            SdrFillBitmapAttribute();
            SdrFillBitmapAttribute(const SdrFillBitmapAttribute& rCandidate);
            SdrFillBitmapAttribute& operator=(const SdrFillBitmapAttribute& rCandidate);
            ~SdrFillBitmapAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFillBitmapAttribute& rCandidate) const;

            // data read access
            const Bitmap& getBitmap() const;
            const basegfx::B2DVector& getSize() const;
            const basegfx::B2DVector& getOffset() const;
            const basegfx::B2DVector& getOffsetPosition() const;
            const basegfx::B2DVector& getRectPoint() const;
            bool getTiling() const;
            bool getStretch() const;
            bool getLogSize() const;

            // FillBitmapAttribute generator
            FillBitmapAttribute getFillBitmapAttribute(const basegfx::B2DRange& rRange) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX

// eof
