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

#include <svx/sdr/primitive3d/sdrattributecreator3d.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <svx/xflclit.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        attribute::Sdr3DObjectAttribute* createNewSdr3DObjectAttribute(const SfxItemSet& rSet)
        {
            // get NormalsKind
            ::com::sun::star::drawing::NormalsKind aNormalsKind(::com::sun::star::drawing::NormalsKind_SPECIFIC);
            const sal_uInt16 nNormalsValue(((const Svx3DNormalsKindItem&)rSet.Get(SDRATTR_3DOBJ_NORMALS_KIND)).GetValue());

            if(1L == nNormalsValue)
            {
                aNormalsKind = ::com::sun::star::drawing::NormalsKind_FLAT;
            }
            else if(2L == nNormalsValue)
            {
                aNormalsKind = ::com::sun::star::drawing::NormalsKind_SPHERE;
            }

            // get NoermalsInvert flag
            const bool bInvertNormals(((const Svx3DNormalsInvertItem&)rSet.Get(SDRATTR_3DOBJ_NORMALS_INVERT)).GetValue());

            // get TextureProjectionX
            ::com::sun::star::drawing::TextureProjectionMode aTextureProjectionX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC);
            const sal_uInt16 nTextureValueX(((const Svx3DTextureProjectionXItem&)rSet.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X)).GetValue());

            if(1L == nTextureValueX)
            {
                aTextureProjectionX = ::com::sun::star::drawing::TextureProjectionMode_PARALLEL;
            }
            else if(2L == nTextureValueX)
            {
                aTextureProjectionX = ::com::sun::star::drawing::TextureProjectionMode_SPHERE;
            }

            // get TextureProjectionY
            ::com::sun::star::drawing::TextureProjectionMode aTextureProjectionY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC);
            const sal_uInt16 nTextureValueY(((const Svx3DTextureProjectionYItem&)rSet.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y)).GetValue());

            if(1L == nTextureValueY)
            {
                aTextureProjectionY = ::com::sun::star::drawing::TextureProjectionMode_PARALLEL;
            }
            else if(2L == nTextureValueY)
            {
                aTextureProjectionY = ::com::sun::star::drawing::TextureProjectionMode_SPHERE;
            }

            // get DoubleSided flag
            const bool bDoubleSided(((const Svx3DDoubleSidedItem&)rSet.Get(SDRATTR_3DOBJ_DOUBLE_SIDED)).GetValue());

            // get Shadow3D flag
            const bool bShadow3D(((const Svx3DShadow3DItem&)rSet.Get(SDRATTR_3DOBJ_SHADOW_3D)).GetValue());

            // get TextureFilter flag
            const bool bTextureFilter(((const Svx3DTextureFilterItem&)rSet.Get(SDRATTR_3DOBJ_TEXTURE_FILTER)).GetValue());

            // get texture kind
            // TextureKind: 1 == Base3DTextureLuminance, 2 == Base3DTextureIntensity, 3 == Base3DTextureColor
            ::com::sun::star::drawing::TextureKind2 aTextureKind(::com::sun::star::drawing::TextureKind2_LUMINANCE);
            const sal_uInt16 nTextureKind(((const Svx3DTextureKindItem&)rSet.Get(SDRATTR_3DOBJ_TEXTURE_KIND)).GetValue());

            if(2 == nTextureKind)
            {
                aTextureKind = ::com::sun::star::drawing::TextureKind2_INTENSITY;
            }
            else if(3 == nTextureKind)
            {
                aTextureKind = ::com::sun::star::drawing::TextureKind2_COLOR;
            }

            // get texture mode
            // TextureMode: 1 == Base3DTextureReplace, 2 == Base3DTextureModulate, 3 == Base3DTextureBlend
            ::com::sun::star::drawing::TextureMode aTextureMode(::com::sun::star::drawing::TextureMode_REPLACE);
            const sal_uInt16 nTextureMode(((const Svx3DTextureModeItem&)rSet.Get(SDRATTR_3DOBJ_TEXTURE_MODE)).GetValue());

            if(2 == nTextureMode)
            {
                aTextureMode = ::com::sun::star::drawing::TextureMode_MODULATE;
            }
            else if(3 == nTextureMode)
            {
                aTextureMode = ::com::sun::star::drawing::TextureMode_BLEND;
            }

            // get object color
            const ::basegfx::BColor aObjectColor(((const XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetColorValue().getBColor());

            // get specular color
            const ::basegfx::BColor aSpecular(((const Svx3DMaterialSpecularItem&)rSet.Get(SDRATTR_3DOBJ_MAT_SPECULAR)).GetValue().getBColor());

            // get emissive color
            const ::basegfx::BColor aEmission(((const Svx3DMaterialEmissionItem&)rSet.Get(SDRATTR_3DOBJ_MAT_EMISSION)).GetValue().getBColor());

            // get specular intensity
            sal_uInt16 nSpecularIntensity(((const Svx3DMaterialSpecularIntensityItem&)rSet.Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY)).GetValue());

            if(nSpecularIntensity > 128)
            {
                nSpecularIntensity = 128;
            }

            // get reduced line geometry
            const bool bReducedLineGeometry(((const Svx3DReducedLineGeometryItem&)rSet.Get(SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)).GetValue());

            // prepare material
            attribute::MaterialAttribute3D aMaterial(aObjectColor, aSpecular, aEmission, nSpecularIntensity);

            return new attribute::Sdr3DObjectAttribute(
                aNormalsKind, aTextureProjectionX, aTextureProjectionY,
                aTextureKind, aTextureMode, aMaterial,
                bInvertNormals, bDoubleSided, bShadow3D, bTextureFilter, bReducedLineGeometry);
        }
    } // end of namespace primitive2d
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
