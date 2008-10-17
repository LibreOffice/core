/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hittestprimitive3d.cxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: aw $ $Date: 2008/09/25 17:12:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/hittestprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence HitTestPrimitive3D::createLocalDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // return empty sequence
            return Primitive3DSequence();
        }

        HitTestPrimitive3D::HitTestPrimitive3D(
            const Primitive3DSequence& rChildren)
        :   GroupPrimitive3D(rChildren)
        {
        }

        basegfx::B3DRange HitTestPrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            return getB3DRangeFromPrimitive3DSequence(getChildren(), rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(HitTestPrimitive3D, PRIMITIVE3D_ID_HITTESTPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
