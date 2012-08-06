/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SdrOleContentPrimitive2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDROLECONTENTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefinitions

class SdrOle2Obj;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrOleContentPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            SdrObjectWeakRef                            mpSdrOle2Obj;
            basegfx::B2DHomMatrix                       maObjectTransform;

            // #i104867# The GraphicVersion number to identify in operator== if
            // the graphic has changed, but without fetching it (which may
            // be expensive, e.g. triggering chart creation)
            sal_uInt32                                  mnGraphicVersion;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrOleContentPrimitive2D(
                const SdrOle2Obj& rSdrOle2Obj,
                const basegfx::B2DHomMatrix& rObjectTransform,
                sal_uInt32 nGraphicVersion
            );

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // The default implementation will use getDecomposition results to create the range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // data access
            const basegfx::B2DHomMatrix& getObjectTransform() const { return maObjectTransform; }
            sal_uInt32 getGraphicVersion() const { return mnGraphicVersion; }

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDROLECONTENTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
