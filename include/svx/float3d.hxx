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
    PushButton*         m_pBtnGeo;
    PushButton*         m_pBtnRepresentation;
    PushButton*         m_pBtnLight;
    PushButton*         m_pBtnTexture;
    PushButton*         m_pBtnMaterial;
    PushButton*         m_pBtnUpdate;
    PushButton*         m_pBtnAssign;

// geometry
    VclContainer*       m_pFLGeometrie;
    FixedText*          m_pFtPercentDiagonal;
    MetricField*        m_pMtrPercentDiagonal;
    FixedText*          m_pFtBackscale;
    MetricField*        m_pMtrBackscale;
    FixedText*          m_pFtEndAngle;
    MetricField*        m_pMtrEndAngle;
    FixedText*          m_pFtDepth;
    MetricField*        m_pMtrDepth;

    VclContainer*       m_pFLSegments;
    NumericField*       m_pNumHorizontal;
    NumericField*       m_pNumVertical;

    VclContainer*       m_pFLNormals;
    PushButton*         m_pBtnNormalsObj;
    PushButton*         m_pBtnNormalsFlat;
    PushButton*         m_pBtnNormalsSphere;
    PushButton*         m_pBtnNormalsInvert;
    PushButton*         m_pBtnTwoSidedLighting;
    PushButton*         m_pBtnDoubleSided;

// presentation
    VclContainer*       m_pFLRepresentation;
    ListBox*            m_pLbShademode;

    VclContainer*       m_pFLShadow;
    PushButton*         m_pBtnShadow3d;
    FixedText*          m_pFtSlant;
    MetricField*        m_pMtrSlant;

    VclContainer*       m_pFLCamera;
    MetricField*        m_pMtrDistance;
    MetricField*        m_pMtrFocalLength;

// lighting
    VclContainer*       m_pFLLight;
    PushButton*         m_pBtnLight1;
    PushButton*         m_pBtnLight2;
    PushButton*         m_pBtnLight3;
    PushButton*         m_pBtnLight4;
    PushButton*         m_pBtnLight5;
    PushButton*         m_pBtnLight6;
    PushButton*         m_pBtnLight7;
    PushButton*         m_pBtnLight8;
    ColorLB*            m_pLbLight1;
    ColorLB*            m_pLbLight2;
    ColorLB*            m_pLbLight3;
    ColorLB*            m_pLbLight4;
    ColorLB*            m_pLbLight5;
    ColorLB*            m_pLbLight6;
    ColorLB*            m_pLbLight7;
    ColorLB*            m_pLbLight8;
    PushButton*         m_pBtnLightColor;
    ColorLB*            m_pLbAmbientlight;    // ListBox
    PushButton*         m_pBtnAmbientColor;   // color button

// Textures
    VclContainer*       m_pFLTexture;
    PushButton*         m_pBtnTexLuminance;
    PushButton*         m_pBtnTexColor;
    PushButton*         m_pBtnTexReplace;
    PushButton*         m_pBtnTexModulate;
    PushButton*         m_pBtnTexBlend;
    PushButton*         m_pBtnTexObjectX;
    PushButton*         m_pBtnTexParallelX;
    PushButton*         m_pBtnTexCircleX;
    PushButton*         m_pBtnTexObjectY;
    PushButton*         m_pBtnTexParallelY;
    PushButton*         m_pBtnTexCircleY;
    PushButton*         m_pBtnTexFilter;

// material
// material editor
    VclContainer*       m_pFLMaterial;
    ListBox*            m_pLbMatFavorites;
    ColorLB*            m_pLbMatColor;
    PushButton*         m_pBtnMatColor;
    ColorLB*            m_pLbMatEmission;
    PushButton*         m_pBtnEmissionColor;

    VclContainer*       m_pFLMatSpecular;
    ColorLB*            m_pLbMatSpecular;
    PushButton*         m_pBtnSpecularColor;
    MetricField*        m_pMtrMatSpecularIntensity;

    Svx3DPreviewControl* m_pCtlPreview;
    SvxLightCtl3D*      m_pCtlLightPreview;

// bottom part
    PushButton*         m_pBtnConvertTo3D;
    PushButton*         m_pBtnLatheObject;
    PushButton*         m_pBtnPerspective;

// the rest ...
    Image               aImgLightOn;
    Image               aImgLightOff;
    bool                bUpdate;
    ViewType3D          eViewType;

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

    bool                bOnly3DChanged;



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

    void    InitColorLB( const SdrModel* pDoc );
    bool    IsUpdateMode() const { return bUpdate; }

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
