/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: borderlineprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLinePrimitive2D class

            This is the basic primitive to build frames around objects, e.g. tables.
            It defines a single or double line from Start to nd using the LeftWidth,
            Distance and RightWidth definitions.
            The LineStart/End overlap is defined by the Extend(Inner|Outer)(Start|End)
            definitions.
         */
        class BorderLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line definition
            basegfx::B2DPoint                               maStart;
            basegfx::B2DPoint                               maEnd;

            /// the widths of single/double line
            double                                          mfLeftWidth;
            double                                          mfDistance;
            double                                          mfRightWidth;

            /// edge overlap sizes
            double                                          mfExtendInnerStart;
            double                                          mfExtendInnerEnd;
            double                                          mfExtendOuterStart;
            double                                          mfExtendOuterEnd;

            /// the line color
            basegfx::BColor                                 maRGBColor;

            /// bitfield
            /// flags to influence inside/outside creation
            unsigned                                        mbCreateInside : 1;
            unsigned                                        mbCreateOutside : 1;

            /// local helpers
            double getCorrectedLeftWidth() const
            {
                return basegfx::fTools::equal(1.0, mfLeftWidth) ? 0.0 : mfLeftWidth;
            }

            double getCorrectedDistance() const
            {
                return basegfx::fTools::equal(1.0, mfDistance) ? 0.0 : mfDistance;
            }

            double getCorrectedRightWidth() const
            {
                return basegfx::fTools::equal(1.0, mfRightWidth) ? 0.0 : mfRightWidth;
            }

            double getWidth() const
            {
                return getCorrectedLeftWidth() + getCorrectedDistance() + getCorrectedRightWidth();
            }

            bool leftIsHairline() const
            {
                return basegfx::fTools::equal(1.0, mfLeftWidth);
            }

            bool rightIsHairline() const
            {
                return basegfx::fTools::equal(1.0, mfRightWidth);
            }

            bool isInsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfLeftWidth);
            }

            bool isOutsideUsed() const
            {
                return !basegfx::fTools::equalZero(mfRightWidth);
            }

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                double fLeftWidth,
                double fDistance,
                double fRightWidth,
                double fExtendInnerStart,
                double fExtendInnerEnd,
                double fExtendOuterStart,
                double fExtendOuterEnd,
                bool bCreateInside,
                bool bCreateOutside,
                const basegfx::BColor& rRGBColor);

            /// data read access
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            double getLeftWidth() const { return mfLeftWidth; }
            double getDistance() const { return mfDistance; }
            double getRightWidth() const { return mfRightWidth; }
            double getExtendInnerStart() const { return mfExtendInnerStart; }
            double getExtendInnerEnd() const { return mfExtendInnerEnd; }
            double getExtendOuterStart() const { return mfExtendOuterStart; }
            double getExtendOuterEnd() const { return mfExtendOuterEnd; }
            bool getCreateInside() const { return mbCreateInside; }
            bool getCreateOutside() const { return mbCreateOutside; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
