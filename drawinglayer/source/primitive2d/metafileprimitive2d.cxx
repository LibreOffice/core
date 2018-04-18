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

#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <wmfemfhelper.hxx>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

using namespace com::sun::star;

namespace drawinglayer
{
    namespace primitive2d
    {
        void MetafilePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
        {
            // Interpret the Metafile and get the content. There should be only one target, as in the start condition,
            // but iterating will be the right thing to do when some push/pop is not closed
            Primitive2DContainer xRetval(wmfemfhelper::interpretMetafile(getMetaFile(), rViewInformation));

            if(!xRetval.empty())
            {
                // get target size
                const ::tools::Rectangle aMtfTarget(getMetaFile().GetPrefMapMode().GetOrigin(), getMetaFile().GetPrefSize());
                const basegfx::B2DRange aMtfRange(aMtfTarget.Left(), aMtfTarget.Top(), aMtfTarget.Right(), aMtfTarget.Bottom());

                // tdf#113197 get content range and check if we have an overlap with
                // defined target range (aMtfRange)
                if (!aMtfRange.isEmpty())
                {
                    const basegfx::B2DRange aContentRange(xRetval.getB2DRange(rViewInformation));

                    // also test equal since isInside gives also true for equal
                    if (!aMtfRange.equal(aContentRange) && !aMtfRange.isInside(aContentRange))
                    {
                        // contentRange is partly larger than aMtfRange (stuff sticks
                        // outside), clipping is needed
                        const drawinglayer::primitive2d::Primitive2DReference xMask(
                            new drawinglayer::primitive2d::MaskPrimitive2D(
                                basegfx::B2DPolyPolygon(
                                    basegfx::utils::createPolygonFromRect(
                                        aMtfRange)),
                                xRetval));

                        xRetval = drawinglayer::primitive2d::Primitive2DContainer{ xMask };
                    }
                }

                // create transformation
                basegfx::B2DHomMatrix aAdaptedTransform;

                aAdaptedTransform.translate(-aMtfTarget.Left(), -aMtfTarget.Top());
                aAdaptedTransform.scale(
                    aMtfTarget.getWidth() ? 1.0 / aMtfTarget.getWidth() : 1.0,
                    aMtfTarget.getHeight() ? 1.0 / aMtfTarget.getHeight() : 1.0);
                aAdaptedTransform = getTransform() * aAdaptedTransform;

                // embed to target transformation
                const Primitive2DReference aEmbeddedTransform(
                    new TransformPrimitive2D(
                        aAdaptedTransform,
                        xRetval));

                xRetval = Primitive2DContainer { aEmbeddedTransform };
            }

            rContainer.insert(rContainer.end(), xRetval.begin(), xRetval.end());
        }

        MetafilePrimitive2D::MetafilePrimitive2D(
            const basegfx::B2DHomMatrix& rMetaFileTransform,
            const GDIMetaFile& rMetaFile)
        :   BufferedDecompositionPrimitive2D(),
            maMetaFileTransform(rMetaFileTransform),
            maMetaFile(rMetaFile)
        {
        }

        bool MetafilePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MetafilePrimitive2D& rCompare = static_cast<const MetafilePrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getMetaFile() == rCompare.getMetaFile());
            }

            return false;
        }

        basegfx::B2DRange MetafilePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use own implementation to quickly answer the getB2DRange question. The
            // MetafilePrimitive2D assumes that all geometry is inside of the shape. If
            // this is not the case (i have already seen some wrong Metafiles) it should
            // be embedded to a MaskPrimitive2D
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());

            return aRetval;
        }

        // from MetafileAccessor
        bool MetafilePrimitive2D::accessMetafile(GDIMetaFile& rTargetMetafile) const
        {
            rTargetMetafile = maMetaFile;
            return true;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(MetafilePrimitive2D, PRIMITIVE2D_ID_METAFILEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
