/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDROLECONTENTPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDROLECONTENTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdobj.hxx>


// predefinitions

class SdrOle2Obj;



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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrOleContentPrimitive2D(
                const SdrOle2Obj& rSdrOle2Obj,
                const basegfx::B2DHomMatrix& rObjectTransform,
                sal_uInt32 nGraphicVersion
            );

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // The default implementation will use getDecomposition results to create the range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            // data access
            const basegfx::B2DHomMatrix& getObjectTransform() const { return maObjectTransform; }
            sal_uInt32 getGraphicVersion() const { return mnGraphicVersion; }

            // provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDROLECONTENTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
