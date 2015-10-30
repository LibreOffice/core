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

#include <sdr/primitive3d/sdrattributecreator3d.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <svx/xflclit.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        attribute::Sdr3DObjectAttribute* createNewSdr3DObjectAttribute(const SfxItemSet& rSet)
        {
            // get NormalsKind
            css::drawing::NormalsKind aNormalsKind(css::drawing::NormalsKind_SPECIFIC);
            const sal_uInt16 nNormalsValue(static_cast<const Svx3DNormalsKindItem&>(rSet.Get(SDRATTR_3DOBJ_NORMALS_KIND)).GetValue());

            if(1L == nNormalsValue)
            {
                aNormalsKind = css::drawing::NormalsKind_FLAT;
            }
            else if(2L == nNormalsValue)
            {
                aNormalsKind = css::drawing::NormalsKind_SPHERE;
            }

            // get NoermalsInvert flag
            const bool bInvertNormals(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DOBJ_NORMALS_INVERT)).GetValue());

            // get TextureProjectionX
            css::drawing::TextureProjectionMode aTextureProjectionX(css::drawing::TextureProjectionMode_OBJECTSPECIFIC);
            const sal_uInt16 nTextureValueX(static_cast<const Svx3DTextureProjectionXItem&>(rSet.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X)).GetValue());

            if(1L == nTextureValueX)
            {
                aTextureProjectionX = css::drawing::TextureProjectionMode_PARALLEL;
            }
            else if(2L == nTextureValueX)
            {
                aTextureProjectionX = css::drawing::TextureProjectionMode_SPHERE;
            }

            // get TextureProjectionY
            css::drawing::TextureProjectionMode aTextureProjectionY(css::drawing::TextureProjectionMode_OBJECTSPECIFIC);
            const sal_uInt16 nTextureValueY(static_cast<const Svx3DTextureProjectionYItem&>(rSet.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y)).GetValue());

            if(1L == nTextureValueY)
            {
                aTextureProjectionY = css::drawing::TextureProjectionMode_PARALLEL;
            }
            else if(2L == nTextureValueY)
            {
                aTextureProjectionY = css::drawing::TextureProjectionMode_SPHERE;
            }

            // get DoubleSided flag
            const bool bDoubleSided(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DOBJ_DOUBLE_SIDED)).GetValue());

            // get Shadow3D flag
            const bool bShadow3D(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DOBJ_SHADOW_3D)).GetValue());

            // get TextureFilter flag
            const bool bTextureFilter(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DOBJ_TEXTURE_FILTER)).GetValue());

            // get texture kind
            // TextureKind: 1 == Base3DTextureLuminance, 2 == Base3DTextureIntensity, 3 == Base3DTextureColor
            css::drawing::TextureKind2 aTextureKind(css::drawing::TextureKind2_LUMINANCE);
            const sal_uInt16 nTextureKind(static_cast<const Svx3DTextureKindItem&>(rSet.Get(SDRATTR_3DOBJ_TEXTURE_KIND)).GetValue());

            if(2 == nTextureKind)
            {
                aTextureKind = css::drawing::TextureKind2_INTENSITY;
            }
            else if(3 == nTextureKind)
            {
                aTextureKind = css::drawing::TextureKind2_COLOR;
            }

            // get texture mode
            // TextureMode: 1 == Base3DTextureReplace, 2 == Base3DTextureModulate, 3 == Base3DTextureBlend
            css::drawing::TextureMode aTextureMode(css::drawing::TextureMode_REPLACE);
            const sal_uInt16 nTextureMode(static_cast<const Svx3DTextureModeItem&>(rSet.Get(SDRATTR_3DOBJ_TEXTURE_MODE)).GetValue());

            if(2 == nTextureMode)
            {
                aTextureMode = css::drawing::TextureMode_MODULATE;
            }
            else if(3 == nTextureMode)
            {
                aTextureMode = css::drawing::TextureMode_BLEND;
            }

            // get object color
            const ::basegfx::BColor aObjectColor(static_cast<const XFillColorItem&>(rSet.Get(XATTR_FILLCOLOR)).GetColorValue().getBColor());

            // get specular color
            const ::basegfx::BColor aSpecular(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DOBJ_MAT_SPECULAR)).GetValue().getBColor());

            // get emissive color
            const ::basegfx::BColor aEmission(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DOBJ_MAT_EMISSION)).GetValue().getBColor());

            // get specular intensity
            sal_uInt16 nSpecularIntensity(static_cast<const SfxUInt16Item&>(rSet.Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY)).GetValue());

            if(nSpecularIntensity > 128)
            {
                nSpecularIntensity = 128;
            }

            // get reduced line geometry
            const bool bReducedLineGeometry(static_cast<const Svx3DReducedLineGeometryItem&>(rSet.Get(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)).GetValue());

            // prepare material
            attribute::MaterialAttribute3D aMaterial(aObjectColor, aSpecular, aEmission, nSpecularIntensity);

            return new attribute::Sdr3DObjectAttribute(
                aNormalsKind, aTextureProjectionX, aTextureProjectionY,
                aTextureKind, aTextureMode, aMaterial,
                bInvertNormals, bDoubleSided, bShadow3D, bTextureFilter, bReducedLineGeometry);
        }
    } // end of namespace primitive2d
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
