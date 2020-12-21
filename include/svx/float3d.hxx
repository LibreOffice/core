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

#include <memory>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdllapi.h>

#include <svx/dlgctl3d.hxx>


enum class ViewType3D
{
    Geo = 1,
    Representation,
    Light,
    Texture,
    Material
};

class FmFormModel;
class Svx3DCtrlItem;
class SvxConvertTo3DItem;
class ColorListBox;

struct Svx3DWinImpl;

class SVX_DLLPUBLIC LightButton final
{
public:
    explicit LightButton(std::unique_ptr<weld::ToggleButton> xButton);

    void switchLightOn(bool bOn);
    bool isLightOn() const { return m_bLightOn;}

    bool get_active() const { return m_xButton->get_active(); }
    void set_active(bool bActive) { m_xButton->set_active(bActive); }

    TriState get_state() const { return m_xButton->get_state(); }
    void set_state(TriState eState) { m_xButton->set_state(eState); }

    weld::ToggleButton* get_widget() const { return m_xButton.get(); }

    void connect_clicked(const Link<weld::Button&, void>& rLink)
    {
        m_xButton->connect_clicked(rLink);
    }

    bool get_prev_active() const { return m_bButtonPrevActive; }
    void set_prev_active(bool bPrevActive) { m_bButtonPrevActive = bPrevActive; }

private:
    std::unique_ptr<weld::ToggleButton> m_xButton;
    bool m_bLightOn;
    bool m_bButtonPrevActive;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC Svx3DWin final : public SfxDockingWindow
{
    friend class        Svx3DChildWindow;
    friend class        Svx3DCtrlItem;

private:
    std::unique_ptr<weld::ToggleButton> m_xBtnGeo;
    std::unique_ptr<weld::ToggleButton> m_xBtnRepresentation;
    std::unique_ptr<weld::ToggleButton> m_xBtnLight;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexture;
    std::unique_ptr<weld::ToggleButton> m_xBtnMaterial;
    std::unique_ptr<weld::ToggleButton> m_xBtnUpdate;
    std::unique_ptr<weld::Button> m_xBtnAssign;

// geometry
    std::unique_ptr<weld::Container> m_xFLGeometrie;
    std::unique_ptr<weld::Label> m_xFtPercentDiagonal;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrPercentDiagonal;
    std::unique_ptr<weld::Label> m_xFtBackscale;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrBackscale;
    std::unique_ptr<weld::Label> m_xFtEndAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrEndAngle;
    std::unique_ptr<weld::Label> m_xFtDepth;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrDepth;

    std::unique_ptr<weld::Container> m_xFLSegments;
    std::unique_ptr<weld::SpinButton> m_xNumHorizontal;
    std::unique_ptr<weld::SpinButton> m_xNumVertical;

    std::unique_ptr<weld::Container> m_xFLNormals;
    std::unique_ptr<weld::ToggleButton> m_xBtnNormalsObj;
    std::unique_ptr<weld::ToggleButton> m_xBtnNormalsFlat;
    std::unique_ptr<weld::ToggleButton> m_xBtnNormalsSphere;
    std::unique_ptr<weld::ToggleButton> m_xBtnNormalsInvert;
    std::unique_ptr<weld::ToggleButton> m_xBtnTwoSidedLighting;
    std::unique_ptr<weld::ToggleButton> m_xBtnDoubleSided;

// presentation
    std::unique_ptr<weld::Container> m_xFLRepresentation;
    std::unique_ptr<weld::ComboBox> m_xLbShademode;

    std::unique_ptr<weld::Container> m_xFLShadow;
    std::unique_ptr<weld::ToggleButton> m_xBtnShadow3d;
    std::unique_ptr<weld::Label> m_xFtSlant;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrSlant;

    std::unique_ptr<weld::Container> m_xFLCamera;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrDistance;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFocalLength;

// lighting
    std::unique_ptr<weld::Container> m_xFLLight;
    std::unique_ptr<LightButton> m_xBtnLight1;
    std::unique_ptr<LightButton> m_xBtnLight2;
    std::unique_ptr<LightButton> m_xBtnLight3;
    std::unique_ptr<LightButton> m_xBtnLight4;
    std::unique_ptr<LightButton> m_xBtnLight5;
    std::unique_ptr<LightButton> m_xBtnLight6;
    std::unique_ptr<LightButton> m_xBtnLight7;
    std::unique_ptr<LightButton> m_xBtnLight8;
    std::unique_ptr<ColorListBox> m_xLbLight1;
    std::unique_ptr<ColorListBox> m_xLbLight2;
    std::unique_ptr<ColorListBox> m_xLbLight3;
    std::unique_ptr<ColorListBox> m_xLbLight4;
    std::unique_ptr<ColorListBox> m_xLbLight5;
    std::unique_ptr<ColorListBox> m_xLbLight6;
    std::unique_ptr<ColorListBox> m_xLbLight7;
    std::unique_ptr<ColorListBox> m_xLbLight8;
    std::unique_ptr<weld::Button> m_xBtnLightColor;
    std::unique_ptr<ColorListBox> m_xLbAmbientlight;    // ListBox
    std::unique_ptr<weld::Button> m_xBtnAmbientColor;   // color button

// Textures
    std::unique_ptr<weld::Container> m_xFLTexture;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexLuminance;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexColor;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexReplace;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexModulate;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexObjectX;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexParallelX;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexCircleX;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexObjectY;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexParallelY;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexCircleY;
    std::unique_ptr<weld::ToggleButton> m_xBtnTexFilter;

// material
// material editor
    std::unique_ptr<weld::Container> m_xFLMaterial;
    std::unique_ptr<weld::ComboBox> m_xLbMatFavorites;
    std::unique_ptr<ColorListBox> m_xLbMatColor;
    std::unique_ptr<weld::Button> m_xBtnMatColor;
    std::unique_ptr<ColorListBox> m_xLbMatEmission;
    std::unique_ptr<weld::Button> m_xBtnEmissionColor;

    std::unique_ptr<weld::Container> m_xFLMatSpecular;
    std::unique_ptr<ColorListBox> m_xLbMatSpecular;
    std::unique_ptr<weld::Button> m_xBtnSpecularColor;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrMatSpecularIntensity;

    std::unique_ptr<Svx3DPreviewControl> m_xCtlPreview;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreviewWin;

    std::unique_ptr<weld::Widget> m_xLightPreviewGrid;
    std::unique_ptr<weld::Scale> m_xHoriScale;
    std::unique_ptr<weld::Scale> m_xVertScale;
    std::unique_ptr<weld::Button> m_xBtn_Corner;
    std::unique_ptr<Svx3DLightControl> m_xLightPreview;
    std::unique_ptr<weld::CustomWeld> m_xCtlLightPreviewWin;
    std::unique_ptr<SvxLightCtl3D> m_xCtlLightPreview;

// bottom part
    std::unique_ptr<weld::Button> m_xBtnConvertTo3D;
    std::unique_ptr<weld::Button> m_xBtnLatheObject;
    std::unique_ptr<weld::ToggleButton> m_xBtnPerspective;

// the rest ...
    bool                bUpdate;
    ViewType3D          eViewType;

    // Model, Page, View etc. for favourites
    std::unique_ptr<FmFormModel>         pModel;

    SfxBindings*                         pBindings;
    std::unique_ptr<Svx3DCtrlItem>       pControllerItem;

    std::unique_ptr<SvxConvertTo3DItem>  pConvertTo3DItem;
    std::unique_ptr<SvxConvertTo3DItem>  pConvertTo3DLatheItem;

    std::unique_ptr<Svx3DWinImpl>        mpImpl;
    MapUnit             ePoolUnit;
    FieldUnit           eFUnit;

    // ItemSet used to remember set 2d attributes
    std::unique_ptr<SfxItemSet> mpRemember2DAttributes;

    DECL_LINK( ClickViewTypeHdl, weld::Button&, void );
    DECL_LINK( ClickUpdateHdl, weld::ToggleButton&, void );
    DECL_LINK( ClickAssignHdl, weld::Button&, void );
    DECL_LINK( ClickHdl, weld::Button&, void );
    DECL_LINK( ClickColorHdl, weld::Button&, void );
    DECL_LINK( SelectHdl, weld::ComboBox&, void );
    DECL_LINK( SelectColorHdl, ColorListBox&, void );
    DECL_LINK( ModifyMetricHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifySpinHdl, weld::SpinButton&, void );
    void ClickLight(const LightButton& rBtn);

    DECL_LINK( ChangeSelectionCallbackHdl, SvxLightCtl3D*, void );

    SVX_DLLPRIVATE void         Construct();
    SVX_DLLPRIVATE void         Reset();

    SVX_DLLPRIVATE static void  LBSelectColor( ColorListBox* pLb, const Color& rColor );
    SVX_DLLPRIVATE sal_uInt16   GetLightSource( const LightButton* pBtn );
    SVX_DLLPRIVATE ColorListBox* GetCLbByButton( const LightButton* pBtn = nullptr );
    SVX_DLLPRIVATE LightButton* GetLbByButton( const weld::Button* pBtn );

    virtual void    Resize() override;

public:
            Svx3DWin( SfxBindings* pBindings, SfxChildWindow *pCW,
                        vcl::Window* pParent );
            virtual ~Svx3DWin() override;
    virtual void dispose() override;

    void    InitColorLB();
    bool    IsUpdateMode() const { return bUpdate; }

    void    Update( SfxItemSet const & rSet );
    void    GetAttr( SfxItemSet& rSet );

    void UpdatePreview(); // upward (private)
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
                                const SfxPoolItem* pState ) override;

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
    virtual void StateChanged(sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState) override;

public:
    SvxConvertTo3DItem(sal_uInt16 nId, SfxBindings* pBindings);
    bool GetState() const { return bState; }
};

#endif // INCLUDED_SVX_FLOAT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
