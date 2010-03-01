/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrallattribute3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:19 $
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

#include <drawinglayer/attribute/sdrallattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowAttribute3D::SdrLineFillShadowAttribute3D(
            const SdrLineAttribute& rLine,
            const SdrFillAttribute& rFill,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const FillGradientAttribute& rFillFloatTransGradient)
        :   maLine(rLine),
            maFill(rFill),
            maLineStartEnd(rLineStartEnd),
            maShadow(rShadow),
            maFillFloatTransGradient(rFillFloatTransGradient)
        {
        }

        SdrLineFillShadowAttribute3D::SdrLineFillShadowAttribute3D()
        :   maLine(),
            maFill(),
            maLineStartEnd(),
            maShadow(),
            maFillFloatTransGradient()
        {
        }

        bool SdrLineFillShadowAttribute3D::isDefault() const
        {
            return(getLine().isDefault()
                && getFill().isDefault()
                && getLineStartEnd().isDefault()
                && getShadow().isDefault()
                && getFillFloatTransGradient().isDefault());
        }

        bool SdrLineFillShadowAttribute3D::operator==(const SdrLineFillShadowAttribute3D& rCandidate) const
        {
            return(getLine() == rCandidate.getLine()
                && getFill() == rCandidate.getFill()
                && getLineStartEnd() == rCandidate.getLineStartEnd()
                && getShadow() == rCandidate.getShadow()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient());
        }
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
