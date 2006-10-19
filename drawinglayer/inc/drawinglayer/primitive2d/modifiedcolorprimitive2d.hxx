/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modifiedcolorprimitive2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:28:02 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLORMODIFIER_HXX
#include <basegfx/color/bcolormodifier.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class ModifiedColorPrimitive2D : public GroupPrimitive2D
        {
        private:
            basegfx::BColorModifier                 maColorModifier;

        public:
            ModifiedColorPrimitive2D(
                const Primitive2DSequence& rChildren,
                const basegfx::BColorModifier& rColorModifier);

            // get data
            const basegfx::BColorModifier& getColorModifier() const { return maColorModifier; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            virtual sal_uInt32 getPrimitiveID() const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
