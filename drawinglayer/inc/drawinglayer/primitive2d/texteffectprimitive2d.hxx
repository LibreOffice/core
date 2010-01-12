/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texteffectprimitive2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTEFFECTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTEFFECTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextEffectStyle2D definition */
        enum TextEffectStyle2D
        {
            TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT,
            TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT,
            TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED,
            TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED,
            TEXTEFFECTSTYLE2D_OUTLINE
        };

        /** TextEffectPrimitive2D class

            This primitive embeds text primitives (normally, as can be seen can
            also be used for any other primitives) which have some TextEffect applied
            and create the needed geometry and embedding on decomposition.
        */
        class TextEffectPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the text (or other) content
            Primitive2DSequence                             maTextContent;

            /// the style to apply, the direction and the rotation center
            const basegfx::B2DPoint                         maRotationCenter;
            double                                          mfDirection;
            TextEffectStyle2D                               meTextEffectStyle2D;

            /** the last used object to view transformtion used from getDecomposition
                for decide buffering
             */
            basegfx::B2DHomMatrix                           maLastObjectToViewTransformation;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// construcor
            TextEffectPrimitive2D(
                const Primitive2DSequence& rTextContent,
                const basegfx::B2DPoint& rRotationCenter,
                double fDirection,
                TextEffectStyle2D eTextEffectStyle2D);

            /// data read access
            const Primitive2DSequence& getTextContent() const { return maTextContent; }
            const basegfx::B2DPoint& getRotationCenter() const { return maRotationCenter; }
            double getDirection() const { return mfDirection; }
            TextEffectStyle2D getTextEffectStyle2D() const { return meTextEffectStyle2D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /** own get range implementation to solve more effective. Content is by definition displaced
                by a fixed discrete unit, thus the contained geometry needs only once be asked for it's
                own basegfx::B2DRange
             */
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTEFFECTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
