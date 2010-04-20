/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrcustomshapeprimitive2d.hxx,v $
 *
 * $Revision: 1.2.18.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrCustomShapePrimitive2D : public BasePrimitive2D
        {
        private:
            attribute::SdrShadowTextAttribute           maSdrSTAttribute;
            Primitive2DSequence                         maSubPrimitives;
            basegfx::B2DHomMatrix                       maTextBox;

            // bitfield
            // defines if SdrTextWordWrapItem was set at SdrObjCustomShape which means
            // that the text needs to be block formatted
            unsigned                                    mbWordWrap : 1;
            // #SJ# Allow text clipping against TextBox in special cases (used for SC)
            unsigned                                    mbForceTextClipToTextRange : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrCustomShapePrimitive2D(
                const attribute::SdrShadowTextAttribute& rSdrSTAttribute,
                const Primitive2DSequence& rSubPrimitives,
                const basegfx::B2DHomMatrix& rTextBox,
                bool bWordWrap,
                bool bForceTextClipToTextRange);

            // data access
            const attribute::SdrShadowTextAttribute& getSdrSTAttribute() const { return maSdrSTAttribute; }
            const Primitive2DSequence& getSubPrimitives() const { return maSubPrimitives; }
            const basegfx::B2DHomMatrix& getTextBox() const { return maTextBox; }
            bool getWordWrap() const { return mbWordWrap; }
            bool isForceTextClipToTextRange() const { return mbForceTextClipToTextRange; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX

// eof
