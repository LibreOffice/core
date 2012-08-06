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

#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence EpsPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const GDIMetaFile& rSubstituteContent = getMetaFile();

            if( rSubstituteContent.GetActionSize() )
            {
                // the default decomposition will use the Metafile replacement visualisation.
                // To really use the Eps data, a renderer has to know and interpret this primitive
                // directly.
                xRetval.realloc(1);

                xRetval[0] = Primitive2DReference(
                    new MetafilePrimitive2D(
                        getEpsTransform(),
                        rSubstituteContent));
            }

            return xRetval;
        }

        EpsPrimitive2D::EpsPrimitive2D(
            const basegfx::B2DHomMatrix& rEpsTransform,
            const GfxLink& rGfxLink,
            const GDIMetaFile& rMetaFile)
        :   BufferedDecompositionPrimitive2D(),
            maEpsTransform(rEpsTransform),
            maGfxLink(rGfxLink),
            maMetaFile(rMetaFile)
        {
        }

        bool EpsPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const EpsPrimitive2D& rCompare = (EpsPrimitive2D&)rPrimitive;

                return (getEpsTransform() == rCompare.getEpsTransform()
                    && getGfxLink().IsEqual(rCompare.getGfxLink())
                    && getMetaFile() == rCompare.getMetaFile());
            }

            return false;
        }

        basegfx::B2DRange EpsPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use own implementation to quickly answer the getB2DRange question.
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getEpsTransform());

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(EpsPrimitive2D, PRIMITIVE2D_ID_EPSPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
