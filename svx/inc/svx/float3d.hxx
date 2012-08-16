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

#ifndef _SVX_FLOAT3D_HXX
#define _SVX_FLOAT3D_HXX

#include <sfx2/ctrlitem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include "svx/svxdllapi.h"

#include <svx/f3dchild.hxx>
#include <svx/dlgctl3d.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/svdmodel.hxx>

enum ViewType3D
{
    VIEWTYPE_GEO = 1,
    VIEWTYPE_REPRESENTATION,
    VIEWTYPE_LIGHT,
    VIEWTYPE_TEXTURE,
    VIEWTYPE_MATERIAL
};

class SdrModel;
class FmFormModel;
class VirtualDevice;
class E3dView;
class SdrPageView;
class Svx3DCtrlItem;
class SvxConvertTo3DItem;

//------------------------------------------------------------------------
struct Svx3DWinImpl;

class SVX_DLLPUBLIC Svx3DWin : public SfxDockingWindow
{
    friend class        Svx3DChildWindow;
    friend class        Svx3DCtrlItem;
    using Window::Update;

private:
    ImageButton         aBtnGeo;
    ImageButton         aBtnRepresentation;
    ImageButton         aBtnLight;
    ImageButton         aBtnTexture;
    ImageButton         aBtnMaterial;
    ImageButton         aBtnUpdate;
    ImageButton         aBtnAssign;

// geometry
    FixedLine           aFLGeometrie;
    FixedText           aFtPercentDiagonal;
    MetricField         aMtrPercentDiagonal;
    FixedText           aFtBackscale;
    MetricField         aMtrBackscale;
    FixedText           aFtEndAngle;
    MetricField         aMtrEndAngle;
    FixedText           aFtDepth;
    MetricField         aMtrDepth;
      FixedLine           aFLSegments;

    FixedText           aFtHorizontal;
    NumericField        aNumHorizontal;
    FixedText           aFtVertical;
    NumericField        aNumVertical;
       FixedLine           aFLNormals;

    ImageButton         aBtnNormalsObj;
    ImageButton         aBtnNormalsFlat;
    ImageButton         aBtnNormalsSphere;
    ImageButton         aBtnNormalsInvert;
    ImageButton         aBtnTwoSidedLighting;

    ImageButton         aBtnDoubleSided;
    FixedLine           aFLRepresentation;
// presentation
    FixedText           aFtShademode;
    ListBox             aLbShademode;
    FixedLine           aFLShadow;
    ImageButton         aBtnShadow3d;
    FixedText           aFtSlant;
    MetricField         aMtrSlant;
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    FixedText           aFtFocalLeng;
    MetricField         aMtrFocalLength;
    FixedLine           aFLCamera;
    FixedLine           aFLLight;

// lighting
    ImageButton         aBtnLight1;
    ImageButton         aBtnLight2;
    ImageButton         aBtnLight3;
    ImageButton         aBtnLight4;
    ImageButton         aBtnLight5;
    ImageButton         aBtnLight6;
    ImageButton         aBtnLight7;
    ImageButton         aBtnLight8;
    FixedText           aFTLightsource;
    ColorLB             aLbLight1;
    ColorLB             aLbLight2;
    ColorLB             aLbLight3;
    ColorLB             aLbLight4;
    ColorLB             aLbLight5;
    ColorLB             aLbLight6;
    ColorLB             aLbLight7;
    ColorLB             aLbLight8;

    ImageButton         aBtnLightColor;

    // #99694# Keyboard shortcuts activate the next control, so the
    // order needed to be changed here
    FixedText           aFTAmbientlight;    // Text label
    ColorLB             aLbAmbientlight;    // ListBox
    ImageButton         aBtnAmbientColor;   // color button

       FixedLine           aFLTexture;

// Textures
    FixedText           aFtTexKind;
    ImageButton         aBtnTexLuminance;
    ImageButton         aBtnTexColor;
    FixedText           aFtTexMode;
    ImageButton         aBtnTexReplace;
    ImageButton         aBtnTexModulate;
    ImageButton         aBtnTexBlend;
    FixedText           aFtTexProjectionX;
    ImageButton         aBtnTexObjectX;
    ImageButton         aBtnTexParallelX;
    ImageButton         aBtnTexCircleX;
    FixedText           aFtTexProjectionY;
    ImageButton         aBtnTexObjectY;
    ImageButton         aBtnTexParallelY;
    ImageButton         aBtnTexCircleY;
    FixedText           aFtTexFilter;
    ImageButton         aBtnTexFilter;

// material
// material editor
       FixedLine           aFLMaterial;
    FixedText           aFtMatFavorites;
    ListBox             aLbMatFavorites;
    FixedText           aFtMatColor;
    ColorLB             aLbMatColor;
    ImageButton         aBtnMatColor;
    FixedText           aFtMatEmission;
    ColorLB             aLbMatEmission;
    ImageButton         aBtnEmissionColor;
       FixedLine           aFLMatSpecular;
    FixedText           aFtMatSpecular;
    ColorLB             aLbMatSpecular;
    ImageButton         aBtnSpecularColor;
    FixedText           aFtMatSpecularIntensity;
    MetricField         aMtrMatSpecularIntensity;

    Svx3DPreviewControl aCtlPreview;
    SvxLightCtl3D       aCtlLightPreview;

// bottom part
    ImageButton         aBtnConvertTo3D;
    ImageButton         aBtnLatheObject;
    ImageButton         aBtnPerspective;

// the rest ...
    Image               aImgLightOn;
    Image               aImgLightOff;
    sal_Bool                bUpdate;
    ViewType3D          eViewType;
    Size                aSize;

    // Model, Page, View etc. for favourites
    FmFormModel*        pModel;
    VirtualDevice*      pVDev;
    E3dView*            p3DView;

    SfxBindings*                pBindings;
    Svx3DCtrlItem*              pControllerItem;

    SvxConvertTo3DItem*         pConvertTo3DItem;
    SvxConvertTo3DItem*         pConvertTo3DLatheItem;

    Svx3DWinImpl*       mpImpl;
    SfxMapUnit          ePoolUnit;
    FieldUnit           eFUnit;

    // ItemSet used to remember set 2d attributes
    SfxItemSet*         mpRemember2DAttributes;

    sal_Bool                bOnly3DChanged;

    //------------------------------------

    DECL_LINK( ClickViewTypeHdl, void * );
    DECL_LINK( ClickUpdateHdl, void * );
    DECL_LINK( ClickAssignHdl, void * );
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( ClickColorHdl, PushButton * );
    DECL_LINK( SelectHdl, void * );
    DECL_LINK( ModifyHdl, void * );
    void ClickLight(PushButton &rBtn);

    DECL_LINK( ChangeLightCallbackHdl, void * );
    DECL_LINK( ChangeSelectionCallbackHdl, void * );

    SVX_DLLPRIVATE void         Construct();
    SVX_DLLPRIVATE void         Reset();

    SVX_DLLPRIVATE sal_Bool         LBSelectColor( ColorLB* pLb, const Color& rColor );
    SVX_DLLPRIVATE sal_uInt16           GetLightSource( const PushButton* pBtn = NULL );
    SVX_DLLPRIVATE ColorLB*     GetLbByButton( const PushButton* pBtn = NULL );

    SVX_DLLPRIVATE bool         GetUILightState( const ImageButton& rBtn ) const;
    SVX_DLLPRIVATE void         SetUILightState( ImageButton& aBtn, bool bState );

protected:
    virtual void    Resize();

public:
            Svx3DWin( SfxBindings* pBindings, SfxChildWindow *pCW,
                        Window* pParent );
            ~Svx3DWin();

    void    InitColorLB( const SdrModel* pDoc );
    sal_Bool    IsUpdateMode() const { return bUpdate; }

    void    Update( SfxItemSet& rSet );
    void    GetAttr( SfxItemSet& rSet );

    void UpdatePreview(); // nach oben (private)
    void DocumentReload(); // #83951#
};

/*************************************************************************
|*
|* Controller item for 3D Window (Floating/Docking)
|*
\************************************************************************/

class Svx3DCtrlItem : public SfxControllerItem
{
 protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    Svx3DCtrlItem( sal_uInt16, SfxBindings* );
};

/*************************************************************************
|*
|* ControllerItem for State of a Slot
|* (SID_CONVERT_TO_3D, SID_CONVERT_TO_3D_LATHE_FAST)
|*
\************************************************************************/

class SvxConvertTo3DItem : public SfxControllerItem
{
    sal_Bool                        bState;

protected:
    virtual void StateChanged(sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState);

public:
    SvxConvertTo3DItem(sal_uInt16 nId, SfxBindings* pBindings);
    sal_Bool GetState() const { return bState; }
};

#endif      // _SVX_FLOAT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
