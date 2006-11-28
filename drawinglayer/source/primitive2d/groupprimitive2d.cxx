/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupprimitive2d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-28 11:03:57 $
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
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /// default: just return children, so all renderers not supporting group will use it's content
        Primitive2DSequence GroupPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return getChildren();
        }

        GroupPrimitive2D::GroupPrimitive2D( const Primitive2DSequence& rChildren )
        :   BasePrimitive2D(),
            maChildren(rChildren)
        {
        }

        /** The compare opertator uses the Sequence::==operator, so only checking if
            the rererences are equal. All non-equal references are interpreted as
            non-equal.
         */
        bool GroupPrimitive2D::operator==( const BasePrimitive2D& rPrimitive ) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const GroupPrimitive2D& rCompare = static_cast< const GroupPrimitive2D& >(rPrimitive);

                return (arePrimitive2DSequencesEqual(getChildren(), rCompare.getChildren()));
            }

            return false;
        }

        bool GroupPrimitive2D::mayDecompositionBeAnimated() const
        {
            // check children. If one returns true, return true
            if(!maChildren.hasElements())
            {
                return false;
            }

            const sal_Int32 nCount(maChildren.getLength());

            for(sal_Int32 a(0L); a < nCount; a++)
            {
                // get reference
                const primitive2d::Primitive2DReference xReference(maChildren[a]);

                if(xReference.is())
                {
                    // try to cast to BasePrimitive2D implementation
                    const primitive2d::BasePrimitive2D* pBasePrimitive = dynamic_cast< const primitive2d::BasePrimitive2D* >(xReference.get());

                    if(pBasePrimitive)
                    {
                        if(pBasePrimitive->mayDecompositionBeAnimated())
                        {
                            return true;
                        }
                    }
                    else
                    {
                        // unknown implementation -> may contain animation
                        return true;
                    }
                }
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(GroupPrimitive2D, '2','G','r','o')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
