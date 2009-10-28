/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrongspellprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// WrongSpellPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** WrongSpellPrimitive2D class

            This is a helper primitive to hold evtl. WrongSpell visualisations
            in the sequence of primitives. The primitive holds this information
            separated form the TextPortions to where it belongs to tot expand the
            TextSimplePortionPrimitive2D more as needed.

            A renderer who does not want to visualize this (if contained at all)
            can detect and ignore this primitive. If it's decomposition is used,
            it will be visualized as red wavelines.

            The geometric definition defines a line on the X-Axis (no Y-coordinates)
            which will when transformed by Transformation, create the coordinate data.
         */
        class WrongSpellPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// geometry definition
            basegfx::B2DHomMatrix                           maTransformation;
            double                                          mfStart;
            double                                          mfStop;

            /// color (usually red)
            basegfx::BColor                                 maColor;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            WrongSpellPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                double fStart,
                double fStop,
                const basegfx::BColor& rColor);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            double getStart() const { return mfStart; }
            double getStop() const { return mfStop; }
            const basegfx::BColor& getColor() const { return maColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
