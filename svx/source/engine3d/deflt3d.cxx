/*************************************************************************
 *
 *  $RCSfile: deflt3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR

#ifndef _E3D_DEFLT3D_HXX
#include "deflt3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif

#ifndef _SVXE3DITEM_HXX
#include "e3ditem.hxx"
#endif

/*************************************************************************
|*
|* Klasse zum verwalten der 3D-Default Attribute
|*
\************************************************************************/

// Konstruktor
E3dDefaultAttributes::E3dDefaultAttributes()
{
    Reset();
}

void E3dDefaultAttributes::Reset()
{
    // Compound-Objekt
//  aDefaultFrontMaterial;
//  aDefaultBackMaterial;
    eDefaultTextureKind = Base3DTextureColor;
    eDefaultTextureMode = Base3DTextureModulate;
    bDefaultDoubleSided = FALSE;
    bDefaultCreateNormals = TRUE;
    bDefaultCreateTexture = TRUE;
    bDefaultUseStdNormals = FALSE;
    bDefaultUseStdNormalsUseSphere = FALSE;
    bDefaultInvertNormals = FALSE;
    bDefaultUseStdTextureX = FALSE;
    bDefaultUseStdTextureXUseSphere = FALSE;
    bDefaultUseStdTextureY = FALSE;
    bDefaultUseStdTextureYUseSphere = FALSE;
    bDefaultShadow3D = FALSE;
    bDefaultFilterTexture = FALSE;
    bDefaultUseDifferentBackMaterial = FALSE;

    // Cube-Objekt
    aDefaultCubePos = Vector3D(-500.0, -500.0, -500.0);
    aDefaultCubeSize = Vector3D(1000.0, 1000.0, 1000.0);
    nDefaultCubeSideFlags = CUBE_FULL;
    bDefaultCubePosIsCenter = FALSE;

    // Sphere-Objekt
    nDefaultHSegments = 24;
    nDefaultVSegments = 24;
    aDefaultSphereCenter = Vector3D(0.0, 0.0, 0.0);
    aDefaultSphereSize = Vector3D(1000.0, 1000.0, 1000.0);

    // Lathe-Objekt
    nDefaultLatheEndAngle = 3600;
    fDefaultLatheScale = 1.0;
    fDefaultBackScale = 1.0;
    fDefaultPercentDiag = 0.05;
    bDefaultLatheSmoothed = TRUE;
    bDefaultLatheSmoothFrontBack = FALSE;
    bDefaultLatheCharacterMode = FALSE;
    bDefaultLatheCloseFront = TRUE;
    bDefaultLatheCloseBack = TRUE;

    // Extrude-Objekt
    fDefaultExtrudeScale = 1.0;
    fDefaultExtrudeDepth = 1000.0;
    bDefaultExtrudeSmoothed = TRUE;
    bDefaultExtrudeSmoothFrontBack = FALSE;
    bDefaultExtrudeCharacterMode = FALSE;
    bDefaultExtrudeCloseFront = TRUE;
    bDefaultExtrudeCloseBack = TRUE;

    // Scene-Objekt
//  aDefaultLightGroup;
    aDefaultShadowPlaneDirection = Vector3D(0.0, 0.0, 1.0);
    eDefaultShadeModel = Base3DSmooth;
    bDefaultDither = TRUE;
    bDefaultForceDraftShadeModel = FALSE;
}

/*************************************************************************
|*
|* Default-Attribute setzen/lesen
|*
\************************************************************************/

void E3dDefaultAttributes::SetDefaultValues(const SfxItemSet& rAttr)
{
    const SfxPoolItem* pPoolItem = NULL;
    B3dMaterial aNewMat = GetDefaultFrontMaterial();
    BOOL bNewMatUsed = FALSE;

    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_DOUBLE_SIDED, TRUE, &pPoolItem ) )
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        SetDefaultDoubleSided(bNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_KIND, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        if(nNew == 0)
        {
            SetDefaultUseStdNormals(FALSE);
            SetDefaultUseStdNormalsUseSphere(FALSE);
        }
        else if(nNew == 1)
        {
            SetDefaultUseStdNormals(TRUE);
            SetDefaultUseStdNormalsUseSphere(FALSE);
        }
        else
        {
            SetDefaultUseStdNormals(TRUE);
            SetDefaultUseStdNormalsUseSphere(TRUE);
        }
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_INVERT, TRUE, &pPoolItem ) )
    {
        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
        SetDefaultInvertNormals(bNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_X, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        if(nNew == 0)
        {
            SetDefaultUseStdTextureX(FALSE);
            SetDefaultUseStdTextureXUseSphere(FALSE);
        }
        else if(nNew == 1)
        {
            SetDefaultUseStdTextureX(TRUE);
            SetDefaultUseStdTextureXUseSphere(FALSE);
        }
        else
        {
            SetDefaultUseStdTextureX(TRUE);
            SetDefaultUseStdTextureXUseSphere(TRUE);
        }
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_Y, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        if(nNew == 0)
        {
            SetDefaultUseStdTextureY(FALSE);
            SetDefaultUseStdTextureYUseSphere(FALSE);
        }
        else if(nNew == 1)
        {
            SetDefaultUseStdTextureY(TRUE);
            SetDefaultUseStdTextureYUseSphere(FALSE);
        }
        else
        {
            SetDefaultUseStdTextureY(TRUE);
            SetDefaultUseStdTextureYUseSphere(TRUE);
        }
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_SHADOW_3D, TRUE, &pPoolItem ) )
    {
        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
        SetDefaultShadow3D(bNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_COLOR, TRUE, &pPoolItem ) )
    {
        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
        aNewMat.SetMaterial(aNew, Base3DMaterialDiffuse);
        bNewMatUsed = TRUE;
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_EMISSION, TRUE, &pPoolItem ) )
    {
        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
        aNewMat.SetMaterial(aNew, Base3DMaterialEmission);
        bNewMatUsed = TRUE;
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR, TRUE, &pPoolItem ) )
    {
        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
        aNewMat.SetMaterial(aNew, Base3DMaterialSpecular);
        bNewMatUsed = TRUE;
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR_INTENSITY, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        aNewMat.SetShininess(nNew);
        bNewMatUsed = TRUE;
    }
    if(bNewMatUsed)
        SetDefaultFrontMaterial(aNewMat);

    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_KIND, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        if(nNew == 0)
        {
            SetDefaultTextureKind(Base3DTextureLuminance);
        }
        else
        {
            SetDefaultTextureKind(Base3DTextureColor);
        }
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_MODE, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
        if(nNew == 0)
        {
            SetDefaultTextureMode(Base3DTextureReplace);
        }
        else if(nNew == 1)
        {
            SetDefaultTextureMode(Base3DTextureModulate);
        }
        else
        {
            SetDefaultTextureMode(Base3DTextureBlend);
        }
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_FILTER, TRUE, &pPoolItem ) )
    {
        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
        SetDefaultFilterTexture(bNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_HORZ_SEGS, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ( ( const SfxUInt32Item* ) pPoolItem )->GetValue();
        SetDefaultHSegments(nNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_VERT_SEGS, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ( ( const SfxUInt32Item* ) pPoolItem )->GetValue();
        SetDefaultVSegments(nNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_PERCENT_DIAGONAL, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        SetDefaultPercentDiag(((double)(nNew)) / 200.0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_BACKSCALE, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        SetDefaultBackScale(((double)(nNew)) / 100.0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_END_ANGLE, TRUE, &pPoolItem ) )
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        SetDefaultLatheEndAngle((long)nNew);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_DEPTH, TRUE, &pPoolItem ) )
    {
        UINT32 nNew = ((const SfxUInt32Item*)pPoolItem)->GetValue();
        SetDefaultExtrudeDepth((double)nNew);
    }

    // Szene
    B3dLightGroup aLightGroup = GetDefaultLightGroup();

    // TwoSidedLighting
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_TWO_SIDED_LIGHTING, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.SetModelTwoSide( bNew );
    }

    // LightColors
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_1, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_2, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight1);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_3, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight2);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_4, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight3);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_5, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight4);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_6, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight5);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_7, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight6);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_8, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight7);
    }

    // AmbientColor
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_AMBIENTCOLOR, FALSE, &pPoolItem))
    {
        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
        aLightGroup.SetGlobalAmbientLight( aNew );
    }

    // LightOn
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_1, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_2, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight1);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_3, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight2);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_4, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight3);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_5, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight4);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_6, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight5);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_7, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight6);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_8, FALSE, &pPoolItem))
    {
        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
        aLightGroup.Enable( bNew, Base3DLight7);
    }

    // LightDirection
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_1, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight0);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_2, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight1);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_3, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight2);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_4, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight3);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_5, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight4);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_6, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight5);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_7, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight6);
    }
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_8, FALSE, &pPoolItem))
    {
        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
        aLightGroup.SetDirection( aNew, Base3DLight7);
    }

    // ShadowSlant
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADOW_SLANT, FALSE, &pPoolItem))
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        double fWink = (double)nNew * F_PI180;
        Vector3D aVec(0.0, sin(fWink), cos(fWink));
        aVec.Normalize();
        SetDefaultShadowPlaneDirection(aVec);
    }

    // ShadeMode
    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADE_MODE, FALSE, &pPoolItem))
    {
        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
        if(nNew == 3)
        {
            SetDefaultForceDraftShadeModel(TRUE);
            SetDefaultShadeModel(Base3DSmooth);
        }
        else
        {
            SetDefaultForceDraftShadeModel(FALSE);
            if(nNew == 0)
            {
                SetDefaultShadeModel(Base3DFlat);
            }
            else if(nNew == 1)
            {
                SetDefaultShadeModel(Base3DPhong);
            }
            else
            {
                // Gouraud
                SetDefaultShadeModel(Base3DSmooth);
            }
        }
    }

    // Nachbehandlung
    SetDefaultLightGroup(aLightGroup);
}

void E3dDefaultAttributes::TakeDefaultValues(SfxItemSet& rAttr)
{
    // DoubleSided
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED, GetDefaultDoubleSided()));

    // NormalsKind
    UINT16 nObjNormalsKind;
    if(!GetDefaultUseStdNormals())
    {
        nObjNormalsKind = 0;
    }
    else
    {
        if(GetDefaultUseStdNormalsUseSphere())
        {
            nObjNormalsKind = 2;
        }
        else
        {
            nObjNormalsKind = 1;
        }
    }
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_NORMALS_KIND, nObjNormalsKind));

    // NormalsInvert
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_NORMALS_INVERT, GetDefaultInvertNormals()));

    // TextureProjectionX
    UINT16 nObjTextureProjX;
    if(!GetDefaultUseStdTextureX())
    {
        nObjTextureProjX = 0;
    }
    else
    {
        if(GetDefaultUseStdTextureXUseSphere())
        {
            nObjTextureProjX = 2;
        }
        else
        {
            nObjTextureProjX = 1;
        }
    }
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_PROJ_X, nObjTextureProjX));

    // TextureProjectionY
    UINT16 nObjTextureProjY;
    if(!GetDefaultUseStdTextureY())
    {
        nObjTextureProjY = 0;
    }
    else
    {
        if(GetDefaultUseStdTextureYUseSphere())
        {
            nObjTextureProjY = 2;
        }
        else
        {
            nObjTextureProjY = 1;
        }
    }
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_PROJ_Y, nObjTextureProjY));

    // Shadow3D UND Shadow2D
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_SHADOW_3D, GetDefaultShadow3D()));
    rAttr.Put(SdrShadowItem( GetDefaultShadow3D() ));

    // Material
    const B3dMaterial& rMat = GetDefaultFrontMaterial();
    Color aObjMaterialColor = rMat.GetMaterial(Base3DMaterialDiffuse);
    Color aObjMaterialEmission = rMat.GetMaterial(Base3DMaterialEmission);
    Color aObjMaterialSpecular = rMat.GetMaterial(Base3DMaterialSpecular);
    UINT16 nObjMaterialIntensity = rMat.GetShininess();
    rAttr.Put(SvxColorItem(aObjMaterialColor, SID_ATTR_3D_MAT_COLOR));
    rAttr.Put(SvxColorItem(aObjMaterialEmission, SID_ATTR_3D_MAT_EMISSION));
    rAttr.Put(SvxColorItem(aObjMaterialSpecular, SID_ATTR_3D_MAT_SPECULAR));
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_MAT_SPECULAR_INTENSITY, nObjMaterialIntensity));

    // TextureKind
    UINT16 nObjTextureKind;
    if(GetDefaultTextureKind() == Base3DTextureColor)
    {
        nObjTextureKind = 1;
    }
    else
    {
        nObjTextureKind = 0;
    }
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_KIND, nObjTextureKind));

    // TextureMode
    UINT16 nObjTextureMode;
    if(GetDefaultTextureMode() == Base3DTextureReplace)
    {
        nObjTextureMode = 0;
    }
    else if(GetDefaultTextureMode() == Base3DTextureModulate)
    {
        nObjTextureMode = 1;
    }
    else
    {
        nObjTextureMode = 2;
    }
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_MODE, nObjTextureMode));

    // TextureFilter
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_TEXTURE_FILTER, GetDefaultFilterTexture()));

    // HorizSegs
    rAttr.Put(SfxUInt32Item(SID_ATTR_3D_HORZ_SEGS, (UINT32)GetDefaultHSegments()));

    // VertSegs
    rAttr.Put(SfxUInt32Item(SID_ATTR_3D_VERT_SEGS, (UINT32)GetDefaultVSegments()));

    // PercentDiagonal
    UINT16 nObjPercentDiagonal = (UINT16)((GetDefaultPercentDiag() * 200.0) + 0.5);
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_PERCENT_DIAGONAL, nObjPercentDiagonal));

    // BackScale
    UINT16 nObjBackScale = (UINT16)((GetDefaultBackScale() * 100.0) + 0.5);
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_BACKSCALE, nObjBackScale));

    // EndAngle
    UINT16 nObjEndAngle = (UINT16)(GetDefaultLatheEndAngle() + 0.5);
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_END_ANGLE, nObjEndAngle));

    // ExtrudeDepth
    UINT32 nObjDeepth = (UINT32)(GetDefaultExtrudeDepth() + 0.5);
    rAttr.Put(SfxUInt32Item(SID_ATTR_3D_DEPTH, nObjDeepth));

    // Szenenparameter
    B3dLightGroup& rLightGroup = (B3dLightGroup&)GetDefaultLightGroup();
    BOOL     bSceneTwoSidedLighting = rLightGroup.GetModelTwoSide();
    Color    aSceneLightColor1 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight0);
    Color    aSceneLightColor2 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight1);
    Color    aSceneLightColor3 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight2);
    Color    aSceneLightColor4 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight3);
    Color    aSceneLightColor5 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight4);
    Color    aSceneLightColor6 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight5);
    Color    aSceneLightColor7 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight6);
    Color    aSceneLightColor8 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight7);
    Color    aSceneAmbientColor = rLightGroup.GetGlobalAmbientLight();
    BOOL     bSceneLightOn1 = rLightGroup.IsEnabled(Base3DLight0);
    BOOL     bSceneLightOn2 = rLightGroup.IsEnabled(Base3DLight1);
    BOOL     bSceneLightOn3 = rLightGroup.IsEnabled(Base3DLight2);
    BOOL     bSceneLightOn4 = rLightGroup.IsEnabled(Base3DLight3);
    BOOL     bSceneLightOn5 = rLightGroup.IsEnabled(Base3DLight4);
    BOOL     bSceneLightOn6 = rLightGroup.IsEnabled(Base3DLight5);
    BOOL     bSceneLightOn7 = rLightGroup.IsEnabled(Base3DLight6);
    BOOL     bSceneLightOn8 = rLightGroup.IsEnabled(Base3DLight7);
    Vector3D aSceneLightDirection1 = rLightGroup.GetDirection( Base3DLight0 );
    Vector3D aSceneLightDirection2 = rLightGroup.GetDirection( Base3DLight1 );
    Vector3D aSceneLightDirection3 = rLightGroup.GetDirection( Base3DLight2 );
    Vector3D aSceneLightDirection4 = rLightGroup.GetDirection( Base3DLight3 );
    Vector3D aSceneLightDirection5 = rLightGroup.GetDirection( Base3DLight4 );
    Vector3D aSceneLightDirection6 = rLightGroup.GetDirection( Base3DLight5 );
    Vector3D aSceneLightDirection7 = rLightGroup.GetDirection( Base3DLight6 );
    Vector3D aSceneLightDirection8 = rLightGroup.GetDirection( Base3DLight7 );
    UINT16   nSceneShadeMode;
    const Vector3D& rShadowVec = GetDefaultShadowPlaneDirection();
    UINT16 nSceneShadowSlant = (UINT16)((atan2(rShadowVec.Y(), rShadowVec.Z()) / F_PI180) + 0.5);

    if(GetDefaultForceDraftShadeModel())
    {
        nSceneShadeMode = 3; // Draft-Modus
    }
    else
    {
        if(GetDefaultShadeModel() == Base3DSmooth)
        {
            nSceneShadeMode = 2; // Gouraud
        }
        else if(GetDefaultShadeModel() == Base3DFlat)
        {
            nSceneShadeMode = 0; // Flat
        }
        else // Base3DPhong
        {
            nSceneShadeMode = 1; // Phong
        }
    }

    // TwoSidedLighting
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_TWO_SIDED_LIGHTING, bSceneTwoSidedLighting));

    // LightColors
    rAttr.Put(SvxColorItem(aSceneLightColor1, SID_ATTR_3D_LIGHTCOLOR_1));
    rAttr.Put(SvxColorItem(aSceneLightColor2, SID_ATTR_3D_LIGHTCOLOR_2));
    rAttr.Put(SvxColorItem(aSceneLightColor3, SID_ATTR_3D_LIGHTCOLOR_3));
    rAttr.Put(SvxColorItem(aSceneLightColor4, SID_ATTR_3D_LIGHTCOLOR_4));
    rAttr.Put(SvxColorItem(aSceneLightColor5, SID_ATTR_3D_LIGHTCOLOR_5));
    rAttr.Put(SvxColorItem(aSceneLightColor6, SID_ATTR_3D_LIGHTCOLOR_6));
    rAttr.Put(SvxColorItem(aSceneLightColor7, SID_ATTR_3D_LIGHTCOLOR_7));
    rAttr.Put(SvxColorItem(aSceneLightColor8, SID_ATTR_3D_LIGHTCOLOR_8));

    // AmbientColor
    rAttr.Put(SvxColorItem(aSceneAmbientColor, SID_ATTR_3D_AMBIENTCOLOR));

    // LightOn
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_1, bSceneLightOn1));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_2, bSceneLightOn2));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_3, bSceneLightOn3));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_4, bSceneLightOn4));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_5, bSceneLightOn5));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_6, bSceneLightOn6));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_7, bSceneLightOn7));
    rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_8, bSceneLightOn8));

    // LightDirection
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_1, aSceneLightDirection1));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_2, aSceneLightDirection2));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_3, aSceneLightDirection3));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_4, aSceneLightDirection4));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_5, aSceneLightDirection5));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_6, aSceneLightDirection6));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_7, aSceneLightDirection7));
    rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_8, aSceneLightDirection8));

    // ShadowSlant
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_SHADOW_SLANT, nSceneShadowSlant));

    // ShadeMode
    rAttr.Put(SfxUInt16Item(SID_ATTR_3D_SHADE_MODE, nSceneShadeMode));
}


