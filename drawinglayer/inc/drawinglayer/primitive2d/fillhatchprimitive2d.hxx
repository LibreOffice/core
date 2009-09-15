/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillhatchprimitive2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// FillbitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class FillHatchPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DRange                       maObjectRange;
            attribute::FillHatchAttribute           maFillHatch;
            basegfx::BColor                         maBColor;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocal2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            FillHatchPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);

            // get data
            const basegfx::B2DRange& getObjectRange() const { return maObjectRange; }
            const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
