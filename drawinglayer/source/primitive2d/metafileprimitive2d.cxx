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

//#include <basegfx/utils/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
//#include <basegfx/color/bcolor.hxx>
//#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
//#include <vcl/lineinfo.hxx>
//#include <drawinglayer/attribute/lineattribute.hxx>
//#include <drawinglayer/attribute/strokeattribute.hxx>
//#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
//#include <vcl/metaact.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
//#include <basegfx/matrix/b2dhommatrixtools.hxx>
//#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
//#include <basegfx/polygon/b2dpolygontools.hxx>
//#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
//#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
//#include <vcl/salbtype.hxx>
//#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
//#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
//#include <vcl/svapp.hxx>
//#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
//#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
//#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
//#include <basegfx/polygon/b2dpolygonclipper.hxx>
//#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
//#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
//#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
//#include <drawinglayer/primitive2d/wallpaperprimitive2d.hxx>
//#include <drawinglayer/primitive2d/textprimitive2d.hxx>
//#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
//#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
//#include <i18nlangtag/languagetag.hxx>
//#include <drawinglayer/primitive2d/textlineprimitive2d.hxx>
//#include <drawinglayer/primitive2d/textstrikeoutprimitive2d.hxx>
//#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
//#include <tools/fract.hxx>
//#include <numeric>
//#include <emfplushelper.hxx>

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
