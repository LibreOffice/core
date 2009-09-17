/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupprimitive2d.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-07-21 17:41:18 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// GroupPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** Baseclass for all grouping 2D primitives

            The grouping primitive in it's basic form is capable of holding
            a child primitive content and returns it on decomposition on default.
            It is used for two main purposes, but more may apply:

            - to transport extended information, e.g. for text classification,
              see e.g. TextHierarchy*Primitive2D implementations. Since they
              decompose to their child content, renderers not aware/interested
              in that extra information will just ignore these primitives

            - to encapsulate common geometry, e.g. the ShadowPrimitive2D implements
              applying a generic shadow to a child sequence by adding the needed
              offset and color stuff in the decomposition

            In most cases the decomposition is straightforward, so by default
            this primitive will not buffer the result and is not derived from
            BufferedDecompositionPrimitive2D, but from BasePrimitive2D.

            A renderer has to take GroupPrimitive2D derivations into account which
            are used to hold a state.

            Current Basic 2D StatePrimitives are:

            - AlphaPrimitive2D (objects with freely defined transparence)
            - InvertPrimitive2D (for XOR)
            - MaskPrimitive2D (for masking)
            - ModifiedColorPrimitive2D (for a stack of color modifications)
            - TransformPrimitive2D (for a transformation stack)
         */
        class GroupPrimitive2D : public BasePrimitive2D
        {
        private:
            // the children. Declared private since this shall never be changed at all after construction
            Primitive2DSequence                             maChildren;

        public:
            // constructor
            GroupPrimitive2D(const Primitive2DSequence& rChildren);

            // data access
            const Primitive2DSequence& getChildren() const { return maChildren; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            // local decomposition. Implementation will just return children
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
