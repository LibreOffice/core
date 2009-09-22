/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alphaprimitive2d.hxx,v $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** AlphaPrimitive2D class

            This is the basic primitive for applying freely defined transparence
            to freely defined content. The basic idea is to associate a content
            which is defined as a sequence of primitives and hold as child content
            in the GroupPrimitive2D with an alpha channel also defined as a sequence
            of primitives and hold in the Alpha member.

            The basic definition is to use the Alpha content as Alpha-Mask by
            interpreting the Alpha-content not as RGB, but as Luminance alpha mask
            using the common RGB_to_luminance definition as e.g. used by VCL
         */
        class AlphaPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// The Alpha-Mask who's RGB-Values are interpreted as Luminance
            Primitive2DSequence                     maAlpha;

        public:
            /// constructor
            AlphaPrimitive2D(
                const Primitive2DSequence& rChildren,
                const Primitive2DSequence& rAlpha);

            /// data read access
            const Primitive2DSequence& getAlpha() const { return maAlpha; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
