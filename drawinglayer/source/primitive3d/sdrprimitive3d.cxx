/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrprimitive3d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        basegfx::B3DRange SdrPrimitive3D::getStandard3DRange() const
        {
            basegfx::B3DRange aUnitRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
            aUnitRange.transform(getTransform());

            if(getSdrLFSAttribute().getLine())
            {
                const attribute::SdrLineAttribute& rLine = *getSdrLFSAttribute().getLine();

                if(rLine.isVisible() && !basegfx::fTools::equalZero(rLine.getWidth()))
                {
                    // expand by hald LineWidth as tube radius
                    aUnitRange.grow(rLine.getWidth() / 2.0);
                }
            }

            return aUnitRange;
        }

        basegfx::B3DRange SdrPrimitive3D::get3DRangeFromSlices(const Slice3DVector& rSlices) const
        {
            basegfx::B3DRange aRetval;

            if(rSlices.size())
            {
                for(sal_uInt32 a(0L); a < rSlices.size(); a++)
                {
                    aRetval.expand(basegfx::tools::getRange(rSlices[a].getB3DPolyPolygon()));
                }

                aRetval.transform(getTransform());

                if(getSdrLFSAttribute().getLine())
                {
                    const attribute::SdrLineAttribute& rLine = *getSdrLFSAttribute().getLine();

                    if(rLine.isVisible() && !basegfx::fTools::equalZero(rLine.getWidth()))
                    {
                        // expand by hald LineWidth as tube radius
                        aRetval.grow(rLine.getWidth() / 2.0);
                    }
                }
            }

            return aRetval;
        }

        SdrPrimitive3D::SdrPrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   BasePrimitive3D(),
            maTransform(rTransform),
            maTextureSize(rTextureSize),
            maSdrLFSAttribute(rSdrLFSAttribute),
            maSdr3DObjectAttribute(rSdr3DObjectAttribute)
        {
        }

        bool SdrPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
            {
                const SdrPrimitive3D& rCompare = static_cast< const SdrPrimitive3D& >(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getTextureSize() == rCompare.getTextureSize()
                    && getSdrLFSAttribute() == rCompare.getSdrLFSAttribute()
                    && getSdr3DObjectAttribute() == rCompare.getSdr3DObjectAttribute());
            }

            return false;
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
