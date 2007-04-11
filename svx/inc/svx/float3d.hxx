/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: float3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:49:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_FLOAT3D_HXX
#define _SVX_FLOAT3D_HXX

#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
class B3dLightGroup;

class FmFormModel;
class FmFormPage;
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

// Geometrie
    FixedText           aFtPercentDiagonal;
    MetricField         aMtrPercentDiagonal;
    FixedText           aFtBackscale;
    MetricField         aMtrBackscale;
    FixedText           aFtEndAngle;
    MetricField         aMtrEndAngle;
    FixedText           aFtDepth;
    MetricField         aMtrDepth;
    FixedLine           aFLGeometrie;

    FixedText           aFtHorizontal;
    NumericField        aNumHorizontal;
    FixedText           aFtVertical;
    NumericField        aNumVertical;
    FixedLine           aFLSegments;

    ImageButton         aBtnNormalsObj;
    ImageButton         aBtnNormalsFlat;
    ImageButton         aBtnNormalsSphere;
    ImageButton         aBtnNormalsInvert;
    ImageButton         aBtnTwoSidedLighting;
    FixedLine           aFLNormals;

    ImageButton         aBtnDoubleSided;

// Darstellung
    FixedText           aFtShademode;
    ListBox             aLbShademode;
    ImageButton         aBtnShadow3d;
    FixedText           aFtSlant;
    MetricField         aMtrSlant;
    FixedLine           aFLShadow;
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    FixedText           aFtFocalLeng;
    MetricField         aMtrFocalLength;
    FixedLine           aFLCamera;
    FixedLine           aFLRepresentation;

// Beleuchtung
    ImageButton         aBtnLight1;
    ImageButton         aBtnLight2;
    ImageButton         aBtnLight3;
    ImageButton         aBtnLight4;
    ImageButton         aBtnLight5;
    ImageButton         aBtnLight6;
    ImageButton         aBtnLight7;
    ImageButton         aBtnLight8;
    ColorLB             aLbLight1;
    ColorLB             aLbLight2;
    ColorLB             aLbLight3;
    ColorLB             aLbLight4;
    ColorLB             aLbLight5;
    ColorLB             aLbLight6;
    ColorLB             aLbLight7;
    ColorLB             aLbLight8;

    ImageButton         aBtnLightColor;
    FixedText           aFTLightsource;

    // #99694# Keyboard shortcuts activate the next control, so the
    // order needed to be changed here
    FixedText           aFTAmbientlight;    // Text label
    ColorLB             aLbAmbientlight;    // ListBox
    ImageButton         aBtnAmbientColor;   // color button

    FixedLine           aFLLight;

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
    FixedLine           aFLTexture;

// Material
// Materialeditor
    FixedText           aFtMatFavorites;
    ListBox             aLbMatFavorites;
    FixedText           aFtMatColor;
    ColorLB             aLbMatColor;
    ImageButton         aBtnMatColor;
    FixedText           aFtMatEmission;
    ColorLB             aLbMatEmission;
    ImageButton         aBtnEmissionColor;
    FixedText           aFtMatSpecular;
    ColorLB             aLbMatSpecular;
    ImageButton         aBtnSpecularColor;
    FixedText           aFtMatSpecularIntensity;
    MetricField         aMtrMatSpecularIntensity;
    FixedLine           aFLMatSpecular;
    FixedLine           aFLMaterial;

// Unterer Teil
    ImageButton         aBtnConvertTo3D;
    ImageButton         aBtnLatheObject;
    ImageButton         aBtnPerspective;
    Svx3DPreviewControl aCtlPreview;
    SvxLightCtl3D       aCtlLightPreview; // <-- Probleme mit Select/Change-Hdl, wenn ueberladen

// der Rest ...
    B3dLightGroup*      pLightGroup;
    Image               aImgLightOn;
    Image               aImgLightOff;
    BOOL                bUpdate;
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

    BOOL                bOnly3DChanged;

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

    SVX_DLLPRIVATE BOOL         LBSelectColor( ColorLB* pLb, const Color& rColor );
    SVX_DLLPRIVATE USHORT           GetLightSource( const PushButton* pBtn = NULL );
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
    BOOL    IsUpdateMode() const { return bUpdate; }

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
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    Svx3DCtrlItem( USHORT, Svx3DWin*, SfxBindings* );
};

/*************************************************************************
|*
|* ControllerItem fuer Status eines Slots
|* (SID_CONVERT_TO_3D, SID_CONVERT_TO_3D_LATHE_FAST)
|*
\************************************************************************/

class SvxConvertTo3DItem : public SfxControllerItem
{
    BOOL                        bState;

protected:
    virtual void StateChanged(UINT16 nSId, SfxItemState eState, const SfxPoolItem* pState);

public:
    SvxConvertTo3DItem(UINT16 nId, SfxBindings* pBindings);
    BOOL GetState() const { return bState; }
};

#endif      // _SVX_FLOAT3D_HXX

