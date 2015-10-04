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

#ifndef INCLUDED_SVX_FLOAT3D_HXX
#define INCLUDED_SVX_FLOAT3D_HXX

#include <sfx2/ctrlitem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/svxdllapi.h>

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


struct Svx3DWinImpl;

class SVX_DLLPUBLIC SAL_WARN_UNUSED Svx3DWin : public SfxDockingWindow
{
    friend class        Svx3DChildWindow;
    friend class        Svx3DCtrlItem;
    using Window::Update;

private:
    VclPtr<PushButton>         m_pBtnGeo;
    VclPtr<PushButton>         m_pBtnRepresentation;
    VclPtr<PushButton>         m_pBtnLight;
    VclPtr<PushButton>         m_pBtnTexture;
    VclPtr<PushButton>         m_pBtnMaterial;
    VclPtr<PushButton>         m_pBtnUpdate;
    VclPtr<PushButton>         m_pBtnAssign;

// geometry
    VclPtr<VclContainer>       m_pFLGeometrie;
    VclPtr<FixedText>          m_pFtPercentDiagonal;
    VclPtr<MetricField>        m_pMtrPercentDiagonal;
    VclPtr<FixedText>          m_pFtBackscale;
    VclPtr<MetricField>        m_pMtrBackscale;
    VclPtr<FixedText>          m_pFtEndAngle;
    VclPtr<MetricField>        m_pMtrEndAngle;
    VclPtr<FixedText>          m_pFtDepth;
    VclPtr<MetricField>        m_pMtrDepth;

    VclPtr<VclContainer>       m_pFLSegments;
    VclPtr<NumericField>       m_pNumHorizontal;
    VclPtr<NumericField>       m_pNumVertical;

    VclPtr<VclContainer>       m_pFLNormals;
    VclPtr<PushButton>         m_pBtnNormalsObj;
    VclPtr<PushButton>         m_pBtnNormalsFlat;
    VclPtr<PushButton>         m_pBtnNormalsSphere;
    VclPtr<PushButton>         m_pBtnNormalsInvert;
    VclPtr<PushButton>         m_pBtnTwoSidedLighting;
    VclPtr<PushButton>         m_pBtnDoubleSided;

// presentation
    VclPtr<VclContainer>       m_pFLRepresentation;
    VclPtr<ListBox>            m_pLbShademode;

    VclPtr<VclContainer>       m_pFLShadow;
    VclPtr<PushButton>         m_pBtnShadow3d;
    VclPtr<FixedText>          m_pFtSlant;
    VclPtr<MetricField>        m_pMtrSlant;

    VclPtr<VclContainer>       m_pFLCamera;
    VclPtr<MetricField>        m_pMtrDistance;
    VclPtr<MetricField>        m_pMtrFocalLength;

// lighting
    VclPtr<VclContainer>       m_pFLLight;
    VclPtr<PushButton>         m_pBtnLight1;
    VclPtr<PushButton>         m_pBtnLight2;
    VclPtr<PushButton>         m_pBtnLight3;
    VclPtr<PushButton>         m_pBtnLight4;
    VclPtr<PushButton>         m_pBtnLight5;
    VclPtr<PushButton>         m_pBtnLight6;
    VclPtr<PushButton>         m_pBtnLight7;
    VclPtr<PushButton>         m_pBtnLight8;
    VclPtr<ColorLB>            m_pLbLight1;
    VclPtr<ColorLB>            m_pLbLight2;
    VclPtr<ColorLB>            m_pLbLight3;
    VclPtr<ColorLB>            m_pLbLight4;
    VclPtr<ColorLB>            m_pLbLight5;
    VclPtr<ColorLB>            m_pLbLight6;
    VclPtr<ColorLB>            m_pLbLight7;
    VclPtr<ColorLB>            m_pLbLight8;
    VclPtr<PushButton>         m_pBtnLightColor;
    VclPtr<ColorLB>            m_pLbAmbientlight;    // ListBox
    VclPtr<PushButton>         m_pBtnAmbientColor;   // color button

// Textures
    VclPtr<VclContainer>       m_pFLTexture;
    VclPtr<PushButton>         m_pBtnTexLuminance;
    VclPtr<PushButton>         m_pBtnTexColor;
    VclPtr<PushButton>         m_pBtnTexReplace;
    VclPtr<PushButton>         m_pBtnTexModulate;
    VclPtr<PushButton>         m_pBtnTexBlend;
    VclPtr<PushButton>         m_pBtnTexObjectX;
    VclPtr<PushButton>         m_pBtnTexParallelX;
    VclPtr<PushButton>         m_pBtnTexCircleX;
    VclPtr<PushButton>         m_pBtnTexObjectY;
    VclPtr<PushButton>         m_pBtnTexParallelY;
    VclPtr<PushButton>         m_pBtnTexCircleY;
    VclPtr<PushButton>         m_pBtnTexFilter;

// material
// material editor
    VclPtr<VclContainer>       m_pFLMaterial;
    VclPtr<ListBox>            m_pLbMatFavorites;
    VclPtr<ColorLB>            m_pLbMatColor;
    VclPtr<PushButton>         m_pBtnMatColor;
    VclPtr<ColorLB>            m_pLbMatEmission;
    VclPtr<PushButton>         m_pBtnEmissionColor;

    VclPtr<VclContainer>       m_pFLMatSpecular;
    VclPtr<ColorLB>            m_pLbMatSpecular;
    VclPtr<PushButton>         m_pBtnSpecularColor;
    VclPtr<MetricField>        m_pMtrMatSpecularIntensity;

    VclPtr<Svx3DPreviewControl> m_pCtlPreview;
    VclPtr<SvxLightCtl3D>      m_pCtlLightPreview;

// bottom part
    VclPtr<PushButton>         m_pBtnConvertTo3D;
    VclPtr<PushButton>         m_pBtnLatheObject;
    VclPtr<PushButton>         m_pBtnPerspective;

// the rest ...
    Image               aImgLightOn;
    Image               aImgLightOff;
    bool                bUpdate;
    ViewType3D          eViewType;

    // Model, Page, View etc. for favourites
    FmFormModel*        pModel;
    VclPtr<VirtualDevice> pVDev;
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

    bool                bOnly3DChanged;

    DECL_LINK_TYPED( ClickViewTypeHdl, Button*, void );
    DECL_LINK_TYPED( ClickUpdateHdl, Button*, void );
    DECL_LINK_TYPED( ClickAssignHdl, Button*, void );
    DECL_LINK_TYPED( ClickHdl, Button*, void );
    DECL_LINK_TYPED( ClickColorHdl, Button*, void );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
    DECL_LINK( ModifyHdl, void * );
    void ClickLight(PushButton &rBtn);

    DECL_LINK_TYPED( ChangeSelectionCallbackHdl, SvxLightCtl3D*, void );

    SVX_DLLPRIVATE void         Construct();
    SVX_DLLPRIVATE void         Reset();

    SVX_DLLPRIVATE bool         LBSelectColor( ColorLB* pLb, const Color& rColor );
    SVX_DLLPRIVATE sal_uInt16   GetLightSource( const PushButton* pBtn = NULL );
    SVX_DLLPRIVATE ColorLB*     GetLbByButton( const PushButton* pBtn = NULL );

    SVX_DLLPRIVATE bool         GetUILightState( const PushButton& rBtn ) const;
    SVX_DLLPRIVATE void         SetUILightState( PushButton& aBtn, bool bState );

protected:
    virtual void    Resize() SAL_OVERRIDE;

public:
            Svx3DWin( SfxBindings* pBindings, SfxChildWindow *pCW,
                        vcl::Window* pParent );
            virtual ~Svx3DWin();
    virtual void dispose() SAL_OVERRIDE;

    void    InitColorLB( const SdrModel* pDoc );
    bool    IsUpdateMode() const { return bUpdate; }

    void    Update( SfxItemSet& rSet );
    void    GetAttr( SfxItemSet& rSet );

    void UpdatePreview(); // nach oben (private)
    void DocumentReload();
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
                                const SfxPoolItem* pState ) SAL_OVERRIDE;

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
    bool                        bState;

protected:
    virtual void StateChanged(sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState) SAL_OVERRIDE;

public:
    SvxConvertTo3DItem(sal_uInt16 nId, SfxBindings* pBindings);
    bool GetState() const { return bState; }
};

#endif // INCLUDED_SVX_FLOAT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
