/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVX_FLOAT3D_HXX
#define _SVX_FLOAT3D_HXX

#include <sfx2/ctrlitem.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <sfx2/dockwin.hxx>
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svtools/valueset.hxx>
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
class FmFormPage;
class VirtualDevice;
class E3dView;
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

    FixedLine           aFLGeometrie;
// Geometrie
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
// Darstellung
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

// Beleuchtung
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

// Texturen
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

// Material
       FixedLine           aFLMaterial;
// Materialeditor
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

// Unterer Teil
    ImageButton         aBtnConvertTo3D;
    ImageButton         aBtnLatheObject;
    ImageButton         aBtnPerspective;

// der Rest ...
    Image               aImgLightOn;
    Image               aImgLightOff;
    sal_Bool                bUpdate;
    ViewType3D          eViewType;
    Size                aSize;

    // Model, Page, View etc. fuer Favoriten
    FmFormModel*        pModel;
    FmFormPage*         pFmPage;
    VirtualDevice*      pVDev;
    E3dView*            p3DView;
    List*               pFavorSetList;
    List*               pMatFavSetList;

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
    DECL_LINK( ClickLightHdl, PushButton * );

    DECL_LINK( DoubleClickHdl, void * );

    DECL_LINK( ChangeLightCallbackHdl, void * );
    DECL_LINK( ChangeSelectionCallbackHdl, void * );

    SVX_DLLPRIVATE void         Construct();
    SVX_DLLPRIVATE void         Reset();

    SVX_DLLPRIVATE sal_Bool         LBSelectColor( ColorLB* pLb, const Color& rColor );
    SVX_DLLPRIVATE sal_uInt16           GetLightSource( const PushButton* pBtn = NULL );
    SVX_DLLPRIVATE ColorLB*     GetLbByButton( const PushButton* pBtn = NULL );

    SVX_DLLPRIVATE bool         GetUILightState( ImageButton& aBtn ) const;
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
|* ControllerItem fuer 3D-Window (Floating/Docking)
|*
\************************************************************************/

class Svx3DCtrlItem : public SfxControllerItem
{
    Svx3DWin* p3DWin;

 protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    Svx3DCtrlItem( sal_uInt16, Svx3DWin*, SfxBindings* );
};

/*************************************************************************
|*
|* ControllerItem fuer Status eines Slots
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

