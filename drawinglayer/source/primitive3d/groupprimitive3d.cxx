/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupprimitive3d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-07 15:49:10 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /// default: just return children, so all renderers not supporting group will use it's content
        Primitive3DSequence GroupPrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            return getChildren();
        }

        GroupPrimitive3D::GroupPrimitive3D( const Primitive3DSequence& rChildren )
        :   BasePrimitive3D(),
            maChildren(rChildren)
        {
        }

        /** The compare opertator uses the Sequence::==operator, so only checking if
            the rererences are equal. All non-equal references are interpreted as
            non-equal.
         */
        bool GroupPrimitive3D::operator==( const BasePrimitive3D& rPrimitive ) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
            {
                const GroupPrimitive3D& rCompare = static_cast< const GroupPrimitive3D& >(rPrimitive);

                return (arePrimitive3DSequencesEqual(getChildren(), rCompare.getChildren()));
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(GroupPrimitive3D, '3','G','r','o')

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
