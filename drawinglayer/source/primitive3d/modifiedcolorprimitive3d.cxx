/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modifiedcolorprimitive3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:15 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        modifiedColorPrimitive3D::modifiedColorPrimitive3D(
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::BColorModifier& rColorModifier)
        :   vectorPrimitive3D(rPrimitiveVector),
            maColorModifier(rColorModifier)
        {
        }

        modifiedColorPrimitive3D::~modifiedColorPrimitive3D()
        {
        }

        bool modifiedColorPrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(vectorPrimitive3D::operator==(rPrimitive))
            {
                const modifiedColorPrimitive3D& rCompare = (modifiedColorPrimitive3D&)rPrimitive;
                return (maColorModifier == rCompare.maColorModifier);
            }

            return false;
        }

        PrimitiveID modifiedColorPrimitive3D::getID() const
        {
            return CreatePrimitiveID('M', 'C', 'L', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
