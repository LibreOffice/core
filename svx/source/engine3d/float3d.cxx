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

#include <sal/config.h>

#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svtools/colrdlg.hxx>
#include <svx/colorbox.hxx>
#include <svx/f3dchild.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/fmmodel.hxx>
#include <svx/dlgutil.hxx>
#include <svx/sdshitm.hxx>
#include <svx/svx3ditems.hxx>

#include <svx/dialmgr.hxx>
#include <svx/viewpt3d.hxx>

#include <svx/svxids.hrc>
#include <svx/strings.hrc>

#include <editeng/colritem.hxx>
#include <osl/diagnose.h>
#include <svx/e3ditem.hxx>
#include <svl/whiter.hxx>
#include <svtools/unitconv.hxx>

#include <svx/float3d.hxx>
#include <vcl/floatwin.hxx>

#include <bitmaps.hlst>

using namespace com::sun::star;

SFX_IMPL_DOCKINGWINDOW_WITHID( Svx3DChildWindow, SID_3D_WIN )

struct Svx3DWinImpl
{
    SfxItemPool*        pPool;
};

namespace {
    /** Get the dispatcher from the current view frame, or, if that is not
        available, from the given bindings.
        @param pBindings
            May be NULL.
        @returns NULL when both the current view frame is NULL and the given
            bindings are NULL.
    */
    SfxDispatcher* LocalGetDispatcher (const SfxBindings* pBindings)
    {
        SfxDispatcher* pDispatcher = nullptr;

        if (SfxViewFrame::Current() != nullptr)
            pDispatcher = SfxViewFrame::Current()->GetDispatcher();
        else if (pBindings != nullptr)
            pDispatcher = pBindings->GetDispatcher();

        return pDispatcher;
    }
}

Svx3DWin::Svx3DWin(SfxBindings* pInBindings, SfxChildWindow *pCW, vcl::Window* pParent)
    : SfxDockingWindow(pInBindings, pCW, pParent,
        "Docking3DEffects", "svx/ui/docking3deffects.ui")

    , m_xBtnGeo(m_xBuilder->weld_toggle_button("geometry"))
    , m_xBtnRepresentation(m_xBuilder->weld_toggle_button("representation"))
    , m_xBtnLight(m_xBuilder->weld_toggle_button("light"))
    , m_xBtnTexture(m_xBuilder->weld_toggle_button("texture"))
    , m_xBtnMaterial(m_xBuilder->weld_toggle_button("material"))
    , m_xBtnUpdate(m_xBuilder->weld_toggle_button("update"))
    , m_xBtnAssign(m_xBuilder->weld_button("assign"))

    , m_xFLGeometrie(m_xBuilder->weld_container("geoframe"))
    , m_xFtPercentDiagonal(m_xBuilder->weld_label("diagonalft"))
    , m_xMtrPercentDiagonal(m_xBuilder->weld_metric_spin_button("diagonal", FieldUnit::PERCENT))
    , m_xFtBackscale(m_xBuilder->weld_label("scaleddepthft"))
    , m_xMtrBackscale(m_xBuilder->weld_metric_spin_button("scaleddepth", FieldUnit::PERCENT))
    , m_xFtEndAngle(m_xBuilder->weld_label("angleft"))
    , m_xMtrEndAngle(m_xBuilder->weld_metric_spin_button("angle", FieldUnit::DEGREE))
    , m_xFtDepth(m_xBuilder->weld_label("depthft"))
    , m_xMtrDepth(m_xBuilder->weld_metric_spin_button("depth", FieldUnit::CM))

    , m_xFLSegments(m_xBuilder->weld_container("segmentsframe"))
    , m_xNumHorizontal(m_xBuilder->weld_spin_button("hori"))
    , m_xNumVertical(m_xBuilder->weld_spin_button("veri"))

    , m_xFLNormals(m_xBuilder->weld_container("normals"))
    , m_xBtnNormalsObj(m_xBuilder->weld_toggle_button("objspecific"))
    , m_xBtnNormalsFlat(m_xBuilder->weld_toggle_button("flat"))
    , m_xBtnNormalsSphere(m_xBuilder->weld_toggle_button("spherical"))
    , m_xBtnNormalsInvert(m_xBuilder->weld_toggle_button("invertnormals"))
    , m_xBtnTwoSidedLighting(m_xBuilder->weld_toggle_button("doublesidedillum"))
    , m_xBtnDoubleSided(m_xBuilder->weld_toggle_button("doublesided"))

    , m_xFLRepresentation(m_xBuilder->weld_container("shadingframe"))
    , m_xLbShademode(m_xBuilder->weld_combo_box("mode"))

    , m_xFLShadow(m_xBuilder->weld_container("shadowframe"))
    , m_xBtnShadow3d(m_xBuilder->weld_toggle_button("shadow"))
    , m_xFtSlant(m_xBuilder->weld_label("slantft"))
    , m_xMtrSlant(m_xBuilder->weld_metric_spin_button("slant", FieldUnit::DEGREE))

    , m_xFLCamera(m_xBuilder->weld_container("cameraframe"))
    , m_xMtrDistance(m_xBuilder->weld_metric_spin_button("distance", FieldUnit::CM))
    , m_xMtrFocalLength(m_xBuilder->weld_metric_spin_button("focal", FieldUnit::CM))

    , m_xFLLight(m_xBuilder->weld_container("illumframe"))
    , m_xBtnLight1(new LightButton(m_xBuilder->weld_toggle_button("light1")))
    , m_xBtnLight2(new LightButton(m_xBuilder->weld_toggle_button("light2")))
    , m_xBtnLight3(new LightButton(m_xBuilder->weld_toggle_button("light3")))
    , m_xBtnLight4(new LightButton(m_xBuilder->weld_toggle_button("light4")))
    , m_xBtnLight5(new LightButton(m_xBuilder->weld_toggle_button("light5")))
    , m_xBtnLight6(new LightButton(m_xBuilder->weld_toggle_button("light6")))
    , m_xBtnLight7(new LightButton(m_xBuilder->weld_toggle_button("light7")))
    , m_xBtnLight8(new LightButton(m_xBuilder->weld_toggle_button("light8")))
    , m_xLbLight1(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor1"), GetFrameWeld()))
    , m_xLbLight2(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor2"), GetFrameWeld()))
    , m_xLbLight3(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor3"), GetFrameWeld()))
    , m_xLbLight4(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor4"), GetFrameWeld()))
    , m_xLbLight5(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor5"), GetFrameWeld()))
    , m_xLbLight6(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor6"), GetFrameWeld()))
    , m_xLbLight7(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor7"), GetFrameWeld()))
    , m_xLbLight8(new ColorListBox(m_xBuilder->weld_menu_button("lightcolor8"), GetFrameWeld()))
    , m_xBtnLightColor(m_xBuilder->weld_button("colorbutton1"))
    , m_xLbAmbientlight(new ColorListBox(m_xBuilder->weld_menu_button("ambientcolor"), GetFrameWeld()))
    , m_xBtnAmbientColor(m_xBuilder->weld_button("colorbutton2"))

    , m_xFLTexture(m_xBuilder->weld_container("textureframe"))
    , m_xBtnTexLuminance(m_xBuilder->weld_toggle_button("textype"))
    , m_xBtnTexColor(m_xBuilder->weld_toggle_button("texcolor"))
    , m_xBtnTexReplace(m_xBuilder->weld_toggle_button("texreplace"))
    , m_xBtnTexModulate(m_xBuilder->weld_toggle_button("texmodulate"))
    , m_xBtnTexObjectX(m_xBuilder->weld_toggle_button("texobjx"))
    , m_xBtnTexParallelX(m_xBuilder->weld_toggle_button("texparallelx"))
    , m_xBtnTexCircleX(m_xBuilder->weld_toggle_button("texcirclex"))
    , m_xBtnTexObjectY(m_xBuilder->weld_toggle_button("texobjy"))
    , m_xBtnTexParallelY(m_xBuilder->weld_toggle_button("texparallely"))
    , m_xBtnTexCircleY(m_xBuilder->weld_toggle_button("texcircley"))
    , m_xBtnTexFilter(m_xBuilder->weld_toggle_button("texfilter"))

    , m_xFLMaterial(m_xBuilder->weld_container("materialframe"))
    , m_xLbMatFavorites(m_xBuilder->weld_combo_box("favorites"))
    , m_xLbMatColor(new ColorListBox(m_xBuilder->weld_menu_button("objcolor"), GetFrameWeld()))
    , m_xBtnMatColor(m_xBuilder->weld_button("colorbutton3"))
    , m_xLbMatEmission(new ColorListBox(m_xBuilder->weld_menu_button("illumcolor"), GetFrameWeld()))
    , m_xBtnEmissionColor(m_xBuilder->weld_button("colorbutton4"))

    , m_xFLMatSpecular(m_xBuilder->weld_container("specframe"))
    , m_xLbMatSpecular(new ColorListBox(m_xBuilder->weld_menu_button("speccolor"), GetFrameWeld()))
    , m_xBtnSpecularColor(m_xBuilder->weld_button("colorbutton5"))
    , m_xMtrMatSpecularIntensity(m_xBuilder->weld_metric_spin_button("intensity", FieldUnit::PERCENT))

    , m_xCtlPreview(new Svx3DPreviewControl)
    , m_xCtlPreviewWin(new weld::CustomWeld(*m_xBuilder, "preview", *m_xCtlPreview))

    , m_xLightPreviewGrid(m_xBuilder->weld_container("lightpreviewgrid"))
    , m_xHoriScale(m_xBuilder->weld_scale("horiscale"))
    , m_xVertScale(m_xBuilder->weld_scale("vertscale"))
    , m_xBtn_Corner(m_xBuilder->weld_button("corner"))
    , m_xLightPreview(new Svx3DLightControl)
    , m_xCtlLightPreviewWin(new weld::CustomWeld(*m_xBuilder, "lightpreview", *m_xLightPreview))
    , m_xCtlLightPreview(new SvxLightCtl3D(*m_xLightPreview, *m_xHoriScale, *m_xVertScale, *m_xBtn_Corner)) // TODO might be other body widget as arg 1

    , m_xBtnConvertTo3D(m_xBuilder->weld_button("to3d"))
    , m_xBtnLatheObject(m_xBuilder->weld_button("tolathe"))
    , m_xBtnPerspective(m_xBuilder->weld_toggle_button("perspective"))

    , bUpdate(false)
    , eViewType(ViewType3D::Geo)
    , pBindings(pInBindings)
    , mpImpl(new Svx3DWinImpl)
    , ePoolUnit(MapUnit::MapMM)
{
    SetText(SvxResId(RID_SVXDLG_FLOAT3D_STR_TITLE));

    weld::DrawingArea* pDrawingArea = m_xCtlPreview->GetDrawingArea();
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(83, 76), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    m_xCtlPreview->SetOutputSizePixel(aSize);

    m_xLightPreviewGrid->set_size_request(aSize.Width(), aSize.Height());
    pDrawingArea = m_xLightPreview->GetDrawingArea();
    pDrawingArea->set_size_request(42, 42); // small to fit to m_xLightPreviewGrid

    mpImpl->pPool = nullptr;

    // Set Metric
    eFUnit = pInBindings->GetDispatcher()->GetModule()->GetFieldUnit();

    m_xMtrDepth->set_unit( eFUnit );
    m_xMtrDistance->set_unit( eFUnit );
    m_xMtrFocalLength->set_unit( eFUnit );

    pControllerItem.reset( new Svx3DCtrlItem(SID_3D_STATE, pBindings) );
    pConvertTo3DItem.reset( new SvxConvertTo3DItem(SID_CONVERT_TO_3D, pBindings) );
    pConvertTo3DLatheItem.reset( new SvxConvertTo3DItem(SID_CONVERT_TO_3D_LATHE_FAST, pBindings) );

    m_xBtnAssign->connect_clicked( LINK( this, Svx3DWin, ClickAssignHdl ) );
    m_xBtnUpdate->connect_toggled( LINK( this, Svx3DWin, ClickUpdateHdl ) );

    Link<weld::Button&,void> aLink( LINK( this, Svx3DWin, ClickViewTypeHdl ) );
    m_xBtnGeo->connect_clicked( aLink );
    m_xBtnRepresentation->connect_clicked( aLink );
    m_xBtnLight->connect_clicked( aLink );
    m_xBtnTexture->connect_clicked( aLink );
    m_xBtnMaterial->connect_clicked( aLink );

    aLink = LINK( this, Svx3DWin, ClickHdl );
    m_xBtnPerspective->connect_clicked( aLink );
    m_xBtnConvertTo3D->connect_clicked( aLink );
    m_xBtnLatheObject->connect_clicked( aLink );

    // Geometry
    m_xBtnNormalsObj->connect_clicked( aLink );
    m_xBtnNormalsFlat->connect_clicked( aLink );
    m_xBtnNormalsSphere->connect_clicked( aLink );
    m_xBtnTwoSidedLighting->connect_clicked( aLink );
    m_xBtnNormalsInvert->connect_clicked( aLink );
    m_xBtnDoubleSided->connect_clicked( aLink );

    // Representation
    m_xBtnShadow3d->connect_clicked( aLink );

    // Lighting
    m_xBtnLight1->connect_clicked( aLink );
    m_xBtnLight2->connect_clicked( aLink );
    m_xBtnLight3->connect_clicked( aLink );
    m_xBtnLight4->connect_clicked( aLink );
    m_xBtnLight5->connect_clicked( aLink );
    m_xBtnLight6->connect_clicked( aLink );
    m_xBtnLight7->connect_clicked( aLink );
    m_xBtnLight8->connect_clicked( aLink );

    // Textures
    m_xBtnTexLuminance->connect_clicked( aLink );
    m_xBtnTexColor->connect_clicked( aLink );
    m_xBtnTexReplace->connect_clicked( aLink );
    m_xBtnTexModulate->connect_clicked( aLink );
    m_xBtnTexParallelX->connect_clicked( aLink );
    m_xBtnTexCircleX->connect_clicked( aLink );
    m_xBtnTexObjectX->connect_clicked( aLink );
    m_xBtnTexParallelY->connect_clicked( aLink );
    m_xBtnTexCircleY->connect_clicked( aLink );
    m_xBtnTexObjectY->connect_clicked( aLink );
    m_xBtnTexFilter->connect_clicked( aLink );

    // Material
    aLink = LINK( this, Svx3DWin, ClickColorHdl );
    m_xBtnLightColor->connect_clicked( aLink );
    m_xBtnAmbientColor->connect_clicked( aLink );
    m_xBtnMatColor->connect_clicked( aLink );
    m_xBtnEmissionColor->connect_clicked( aLink );
    m_xBtnSpecularColor->connect_clicked( aLink );


    Link<weld::ComboBox&,void> aLink2 = LINK( this, Svx3DWin, SelectHdl );
    Link<ColorListBox&,void> aLink4 = LINK( this, Svx3DWin, SelectColorHdl );
    m_xLbMatFavorites->connect_changed( aLink2 );
    m_xLbMatColor->SetSelectHdl( aLink4 );
    m_xLbMatEmission->SetSelectHdl( aLink4 );
    m_xLbMatSpecular->SetSelectHdl( aLink4 );
    m_xLbLight1->SetSelectHdl( aLink4 );
    m_xLbLight2->SetSelectHdl( aLink4 );
    m_xLbLight3->SetSelectHdl( aLink4 );
    m_xLbLight4->SetSelectHdl( aLink4 );
    m_xLbLight5->SetSelectHdl( aLink4 );
    m_xLbLight6->SetSelectHdl( aLink4 );
    m_xLbLight7->SetSelectHdl( aLink4 );
    m_xLbLight8->SetSelectHdl( aLink4 );
    m_xLbAmbientlight->SetSelectHdl( aLink4 );
    m_xLbShademode->connect_changed( aLink2 );

    Link<weld::MetricSpinButton&,void> aLink3 = LINK( this, Svx3DWin, ModifyMetricHdl );
    Link<weld::SpinButton&,void> aLink5 = LINK( this, Svx3DWin, ModifySpinHdl );
    m_xMtrMatSpecularIntensity->connect_value_changed( aLink3 );
    m_xNumHorizontal->connect_value_changed( aLink5 );
    m_xNumVertical->connect_value_changed( aLink5 );
    m_xMtrSlant->connect_value_changed( aLink3 );

    // Preview callback
    m_xCtlLightPreview->SetUserSelectionChangeCallback(LINK( this, Svx3DWin, ChangeSelectionCallbackHdl ));

    aSize = GetOutputSizePixel();
    SetMinOutputSizePixel( aSize );

    Construct();

    // Initiation of the initialization of the ColorLBs
    SfxDispatcher* pDispatcher = LocalGetDispatcher(pBindings);
    if (pDispatcher != nullptr)
    {
        SfxBoolItem aItem( SID_3D_INIT, true );
        pDispatcher->ExecuteList(SID_3D_INIT,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
    }

    Reset();

    //lock down the size of the initial largest default mode as the permanent size
    aSize = get_preferred_size();
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());
}

Svx3DWin::~Svx3DWin()
{
    disposeOnce();
}

void Svx3DWin::dispose()
{
    pModel.reset();

    pControllerItem.reset();
    pConvertTo3DItem.reset();
    pConvertTo3DLatheItem.reset();

    mpImpl.reset();

    m_xBtnGeo.reset();
    m_xBtnRepresentation.reset();
    m_xBtnLight.reset();
    m_xBtnTexture.reset();
    m_xBtnMaterial.reset();
    m_xBtnUpdate.reset();
    m_xBtnAssign.reset();
    m_xFLGeometrie.reset();
    m_xFtPercentDiagonal.reset();
    m_xMtrPercentDiagonal.reset();
    m_xFtBackscale.reset();
    m_xMtrBackscale.reset();
    m_xFtEndAngle.reset();
    m_xMtrEndAngle.reset();
    m_xFtDepth.reset();
    m_xMtrDepth.reset();
    m_xFLSegments.reset();
    m_xNumHorizontal.reset();
    m_xNumVertical.reset();
    m_xFLNormals.reset();
    m_xBtnNormalsObj.reset();
    m_xBtnNormalsFlat.reset();
    m_xBtnNormalsSphere.reset();
    m_xBtnNormalsInvert.reset();
    m_xBtnTwoSidedLighting.reset();
    m_xBtnDoubleSided.reset();
    m_xFLRepresentation.reset();
    m_xLbShademode.reset();
    m_xFLShadow.reset();
    m_xBtnShadow3d.reset();
    m_xFtSlant.reset();
    m_xMtrSlant.reset();
    m_xFLCamera.reset();
    m_xMtrDistance.reset();
    m_xMtrFocalLength.reset();
    m_xFLLight.reset();
    m_xBtnLight1.reset();
    m_xBtnLight2.reset();
    m_xBtnLight3.reset();
    m_xBtnLight4.reset();
    m_xBtnLight5.reset();
    m_xBtnLight6.reset();
    m_xBtnLight7.reset();
    m_xBtnLight8.reset();
    m_xLbLight1.reset();
    m_xLbLight2.reset();
    m_xLbLight3.reset();
    m_xLbLight4.reset();
    m_xLbLight5.reset();
    m_xLbLight6.reset();
    m_xLbLight7.reset();
    m_xLbLight8.reset();
    m_xBtnLightColor.reset();
    m_xLbAmbientlight.reset();
    m_xBtnAmbientColor.reset();
    m_xFLTexture.reset();
    m_xBtnTexLuminance.reset();
    m_xBtnTexColor.reset();
    m_xBtnTexReplace.reset();
    m_xBtnTexModulate.reset();
    m_xBtnTexObjectX.reset();
    m_xBtnTexParallelX.reset();
    m_xBtnTexCircleX.reset();
    m_xBtnTexObjectY.reset();
    m_xBtnTexParallelY.reset();
    m_xBtnTexCircleY.reset();
    m_xBtnTexFilter.reset();
    m_xFLMaterial.reset();
    m_xLbMatFavorites.reset();
    m_xLbMatColor.reset();
    m_xBtnMatColor.reset();
    m_xLbMatEmission.reset();
    m_xBtnEmissionColor.reset();
    m_xFLMatSpecular.reset();
    m_xLbMatSpecular.reset();
    m_xBtnSpecularColor.reset();
    m_xMtrMatSpecularIntensity.reset();
    m_xCtlPreviewWin.reset();
    m_xCtlPreview.reset();

    m_xCtlLightPreview.reset();
    m_xCtlLightPreviewWin.reset();
    m_xLightPreview.reset();
    m_xBtn_Corner.reset();
    m_xVertScale.reset();
    m_xHoriScale.reset();
    m_xLightPreviewGrid.reset();

    m_xBtnConvertTo3D.reset();
    m_xBtnLatheObject.reset();
    m_xBtnPerspective.reset();

    SfxDockingWindow::dispose();
}

void Svx3DWin::Construct()
{
    m_xBtnGeo->set_active(true);
    ClickViewTypeHdl(*m_xBtnGeo);
    m_xLightPreviewGrid->hide();
}

void Svx3DWin::Reset()
{
    // Various initializations, default is AllAttributes
    m_xLbShademode->set_active( 0 );
    m_xMtrMatSpecularIntensity->set_value( 50, FieldUnit::PERCENT );

    m_xBtnLight1->set_active(true);
    m_xBtnUpdate->set_active(true);
    ClickUpdateHdl(*m_xBtnUpdate);

    // Select nothing, to avoid errors when selecting the first
    m_xCtlLightPreview->GetSvx3DLightControl().SelectLight(0);
    m_xCtlLightPreview->CheckSelection();
}

void Svx3DWin::Update( SfxItemSet const & rAttrs )
{
    // remember 2d attributes
    if(mpRemember2DAttributes)
        mpRemember2DAttributes->ClearItem();
    else
        mpRemember2DAttributes = std::make_unique<SfxItemSet>(*rAttrs.GetPool(),
            svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
            SDRATTR_3D_FIRST, SDRATTR_3D_LAST>{});

    SfxWhichIter aIter(*mpRemember2DAttributes);
    sal_uInt16 nWhich(aIter.FirstWhich());

    while(nWhich)
    {
        SfxItemState eState = rAttrs.GetItemState(nWhich, false);
        if(SfxItemState::DONTCARE == eState)
            mpRemember2DAttributes->InvalidateItem(nWhich);
        else if(SfxItemState::SET == eState)
            mpRemember2DAttributes->Put(rAttrs.Get(nWhich, false));

        nWhich = aIter.NextWhich();
    }

    // construct field values
    const SfxPoolItem* pItem;

    // Possible determine PoolUnit
    if( !mpImpl->pPool )
    {
        mpImpl->pPool = rAttrs.GetPool();
        DBG_ASSERT( mpImpl->pPool, "Where is the Pool? ");
        ePoolUnit = mpImpl->pPool->GetMetric( SID_ATTR_LINE_WIDTH );
    }
    eFUnit = GetModuleFieldUnit( rAttrs );


    // Segment Number Can be changed? and other states
    SfxItemState eState = rAttrs.GetItemState( SID_ATTR_3D_INTERN, false, &pItem );
    if( SfxItemState::SET == eState )
    {
        sal_uInt32 nState = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
        bool bExtrude = ( nState & 2 );
        bool bSphere  = ( nState & 4 );
        bool bCube    = ( nState & 8 );

        bool bChart = ( nState & 32 ); // Chart

        if( !bChart )
        {
            // For cube objects are no segments set
            m_xFLSegments->set_sensitive(!bCube);

            m_xFtPercentDiagonal->set_sensitive( !bCube && !bSphere );
            m_xMtrPercentDiagonal->set_sensitive( !bCube && !bSphere );
            m_xFtBackscale->set_sensitive( !bCube && !bSphere );
            m_xMtrBackscale->set_sensitive( !bCube && !bSphere );
            m_xFtDepth->set_sensitive( !bCube && !bSphere );
            m_xMtrDepth->set_sensitive( !bCube && !bSphere );
            if( bCube )
            {
                m_xNumHorizontal->set_text("");
                m_xNumVertical->set_text("");
            }
            if( bCube || bSphere )
            {
                m_xMtrPercentDiagonal->set_text("");
                m_xMtrBackscale->set_text("");
                m_xMtrDepth->set_text("");
            }

            // There is a final angle only for Lathe objects.
            m_xFtEndAngle->set_sensitive( !bExtrude && !bCube && !bSphere );
            m_xMtrEndAngle->set_sensitive( !bExtrude && !bCube && !bSphere );
            if( bExtrude || bCube || bSphere )
                m_xMtrEndAngle->set_text("");
        }
        else
        {
            // Geometry
            m_xNumHorizontal->set_text("");
            m_xNumVertical->set_text("");
            m_xFLSegments->set_sensitive( false );
            m_xFtEndAngle->set_sensitive( false );
            m_xMtrEndAngle->set_sensitive( false );
            m_xMtrEndAngle->set_text("");
            m_xFtDepth->set_sensitive( false );
            m_xMtrDepth->set_sensitive( false );
            m_xMtrDepth->set_text("");

            // Representation
            m_xFLShadow->set_sensitive(false);

            m_xMtrDistance->set_text("");
            m_xMtrFocalLength->set_text("");
            m_xFLCamera->set_sensitive( false );

            //Lower Range
            m_xBtnConvertTo3D->set_sensitive( false );
            m_xBtnLatheObject->set_sensitive( false );
        }
    }
    // Bitmap fill ? -> Status
    bool bBitmap(false);
    eState = rAttrs.GetItemState(XATTR_FILLSTYLE);
    if(eState != SfxItemState::DONTCARE)
    {
        drawing::FillStyle eXFS = rAttrs.Get(XATTR_FILLSTYLE).GetValue();
        bBitmap = (eXFS == drawing::FillStyle_BITMAP || eXFS == drawing::FillStyle_GRADIENT || eXFS == drawing::FillStyle_HATCH);
    }

    m_xFLTexture->set_sensitive(bBitmap);

    // Geometry
    // Number of segments (horizontal)
    if( m_xNumHorizontal->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_HORZ_SEGS);
        if(eState != SfxItemState::DONTCARE)
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_HORZ_SEGS).GetValue();
            if (nValue != static_cast<sal_uInt32>(m_xNumHorizontal->get_value()))
            {
                m_xNumHorizontal->set_value( nValue );
                bUpdate = true;
            }
            else if( m_xNumHorizontal->get_text().isEmpty() )
                m_xNumHorizontal->set_value( nValue );
        }
        else
        {
            if( !m_xNumHorizontal->get_text().isEmpty() )
            {
                m_xNumHorizontal->set_text("");
                bUpdate = true;
            }
        }
    }

    //Number of segments (vertical)
    if( m_xNumVertical->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_VERT_SEGS);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_VERT_SEGS).GetValue();
            if( nValue != static_cast<sal_uInt32>(m_xNumVertical->get_value()) )
            {
                m_xNumVertical->set_value( nValue );
                bUpdate = true;
            }
            else if( m_xNumVertical->get_text().isEmpty() )
                m_xNumVertical->set_value( nValue );
        }
        else
        {
            if( !m_xNumVertical->get_text().isEmpty() )
            {
                m_xNumVertical->set_text("");
                bUpdate = true;
            }
        }
    }

    // Depth
    if( m_xMtrDepth->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_DEPTH);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_DEPTH).GetValue();
            sal_uInt32 nValue2 = GetCoreValue(*m_xMtrDepth, ePoolUnit);
            if( nValue != nValue2 )
            {
                if( eFUnit != m_xMtrDepth->get_unit() )
                    SetFieldUnit(*m_xMtrDepth, eFUnit);

                SetMetricValue(*m_xMtrDepth, nValue, ePoolUnit);
                bUpdate = true;
            }
            else if( m_xMtrDepth->get_text().isEmpty() )
                m_xMtrDepth->set_value(m_xMtrDepth->get_value(FieldUnit::NONE), FieldUnit::NONE);
        }
        else
        {
            if( !m_xMtrDepth->get_text().isEmpty() )
            {
                m_xMtrDepth->set_text("");
                bUpdate = true;
            }
        }
    }

    // Double walled / Double sided
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_DOUBLE_SIDED);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_DOUBLE_SIDED).GetValue();
        if( bValue != m_xBtnDoubleSided->get_active() )
        {
            m_xBtnDoubleSided->set_active( bValue );
            bUpdate = true;
        }
        else if( m_xBtnDoubleSided->get_state() == TRISTATE_INDET )
            m_xBtnDoubleSided->set_active( bValue );
    }
    else
    {
        if( m_xBtnDoubleSided->get_state() != TRISTATE_INDET )
        {
            m_xBtnDoubleSided->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Edge rounding
    if( m_xMtrPercentDiagonal->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_PERCENT_DIAGONAL);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL).GetValue();
            if( nValue != m_xMtrPercentDiagonal->get_value(FieldUnit::PERCENT) )
            {
                m_xMtrPercentDiagonal->set_value(nValue, FieldUnit::PERCENT);
                bUpdate = true;
            }
            else if( m_xMtrPercentDiagonal->get_text().isEmpty() )
                m_xMtrPercentDiagonal->set_value(nValue, FieldUnit::PERCENT);
        }
        else
        {
            if( !m_xMtrPercentDiagonal->get_text().isEmpty() )
            {
                m_xMtrPercentDiagonal->set_text("");
                bUpdate = true;
            }
        }
    }

    // Depth scaling
    if( m_xMtrBackscale->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_BACKSCALE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_BACKSCALE).GetValue();
            if( nValue != m_xMtrBackscale->get_value(FieldUnit::PERCENT) )
            {
                m_xMtrBackscale->set_value(nValue, FieldUnit::PERCENT);
                bUpdate = true;
            }
            else if( m_xMtrBackscale->get_text().isEmpty() )
                m_xMtrBackscale->set_value(nValue, FieldUnit::PERCENT);
        }
        else
        {
            if( !m_xMtrBackscale->get_text().isEmpty() )
            {
                m_xMtrBackscale->set_text("");
                bUpdate = true;
            }
        }
    }

    // End angle
    if( m_xMtrEndAngle->get_sensitive() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_END_ANGLE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_Int32 nValue = rAttrs.Get(SDRATTR_3DOBJ_END_ANGLE).GetValue();
            if( nValue != m_xMtrEndAngle->get_value(FieldUnit::DEGREE) )
            {
                m_xMtrEndAngle->set_value(nValue, FieldUnit::DEGREE);
                bUpdate = true;
            }
        }
        else
        {
            if( !m_xMtrEndAngle->get_text().isEmpty() )
            {
                m_xMtrEndAngle->set_text("");
                bUpdate = true;
            }
        }
    }

    // Normal type
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_NORMALS_KIND);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_NORMALS_KIND).GetValue();

        if( ( !m_xBtnNormalsObj->get_active() && nValue == 0 ) ||
            ( !m_xBtnNormalsFlat->get_active() && nValue == 1 ) ||
            ( !m_xBtnNormalsSphere->get_active() && nValue == 2 ) )
        {
            m_xBtnNormalsObj->set_active( nValue == 0 );
            m_xBtnNormalsFlat->set_active( nValue == 1 );
            m_xBtnNormalsSphere->set_active( nValue == 2 );
            bUpdate = true;
        }
    }
    else
    {
        if( m_xBtnNormalsObj->get_active() ||
            m_xBtnNormalsFlat->get_active() ||
            m_xBtnNormalsSphere->get_active() )
        {
            m_xBtnNormalsObj->set_active( false );
            m_xBtnNormalsFlat->set_active( false );
            m_xBtnNormalsSphere->set_active( false );
            bUpdate = true;
        }
    }

    // Normal inverted
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_NORMALS_INVERT);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_NORMALS_INVERT).GetValue();
        if( bValue != m_xBtnNormalsInvert->get_active() )
        {
            m_xBtnNormalsInvert->set_active( bValue );
            bUpdate = true;
        }
        else if( m_xBtnNormalsInvert->get_state() == TRISTATE_INDET )
            m_xBtnNormalsInvert->set_active( bValue );
    }
    else
    {
        if( m_xBtnNormalsInvert->get_state() != TRISTATE_INDET )
        {
            m_xBtnNormalsInvert->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // 2-sided lighting
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING).GetValue();
        if( bValue != m_xBtnTwoSidedLighting->get_active() )
        {
            m_xBtnTwoSidedLighting->set_active( bValue );
            bUpdate = true;
        }
        else if( m_xBtnTwoSidedLighting->get_state() == TRISTATE_INDET )
            m_xBtnTwoSidedLighting->set_active( bValue );
    }
    else
    {
        if( m_xBtnTwoSidedLighting->get_state() != TRISTATE_INDET )
        {
            m_xBtnTwoSidedLighting->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Representation
    // Shademode
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_SHADE_MODE);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DSCENE_SHADE_MODE).GetValue();
        if( nValue != m_xLbShademode->get_active() )
        {
            m_xLbShademode->set_active( nValue );
            bUpdate = true;
        }
    }
    else
    {
        if( m_xLbShademode->get_active() != 0 )
        {
            m_xLbShademode->set_active(-1);
            bUpdate = true;
        }
    }

    // 3D-Shadow
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_SHADOW_3D);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_SHADOW_3D).GetValue();
        if( bValue != m_xBtnShadow3d->get_active() )
        {
            m_xBtnShadow3d->set_active( bValue );
            m_xFtSlant->set_sensitive( bValue );
            m_xMtrSlant->set_sensitive( bValue );
            bUpdate = true;
        }
        else if( m_xBtnShadow3d->get_state() == TRISTATE_INDET )
            m_xBtnShadow3d->set_active( bValue );
    }
    else
    {
        if( m_xBtnShadow3d->get_state() != TRISTATE_INDET )
        {
            m_xBtnShadow3d->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Inclination (Shadow)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_SHADOW_SLANT);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DSCENE_SHADOW_SLANT).GetValue();
        if( nValue != m_xMtrSlant->get_value(FieldUnit::DEGREE) )
        {
            m_xMtrSlant->set_value(nValue, FieldUnit::DEGREE);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_xMtrSlant->get_text().isEmpty() )
        {
            m_xMtrSlant->set_text("");
            bUpdate = true;
        }
    }

    // Distance
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_DISTANCE);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DSCENE_DISTANCE).GetValue();
        sal_uInt32 nValue2 = GetCoreValue(*m_xMtrDistance, ePoolUnit);
        if( nValue != nValue2 )
        {
            if( eFUnit != m_xMtrDistance->get_unit() )
                SetFieldUnit(*m_xMtrDistance, eFUnit);

            SetMetricValue(*m_xMtrDistance, nValue, ePoolUnit);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_xMtrDepth->get_text().isEmpty() )
        {
            m_xMtrDepth->set_text("");
            bUpdate = true;
        }
    }

    // Focal length
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_FOCAL_LENGTH);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DSCENE_FOCAL_LENGTH).GetValue();
        sal_uInt32 nValue2 = GetCoreValue(*m_xMtrFocalLength, ePoolUnit);
        if( nValue != nValue2 )
        {
            if( eFUnit != m_xMtrFocalLength->get_unit() )
                SetFieldUnit(*m_xMtrFocalLength, eFUnit);

            SetMetricValue(*m_xMtrFocalLength, nValue, ePoolUnit);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_xMtrFocalLength->get_text().isEmpty() )
        {
            m_xMtrFocalLength->set_text("");
            bUpdate = true;
        }
    }

// Lighting
    Color aColor;
    // Light 1 (Color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_1);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1).GetValue();
        ColorListBox* pLb = m_xLbLight1.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight1->IsNoSelection())
        {
            m_xLbLight1->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 1 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_1);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_1).GetValue();
        if (bOn != m_xBtnLight1->isLightOn())
        {
            m_xBtnLight1->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight1->get_state() == TRISTATE_INDET )
            m_xBtnLight1->set_active( m_xBtnLight1->get_active() );
    }
    else
    {
        if( m_xBtnLight1->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight1->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 1 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_1);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    //Light 2 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_2);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2).GetValue();
        ColorListBox* pLb = m_xLbLight2.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight2->IsNoSelection())
        {
            m_xLbLight2->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 2 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_2);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_2).GetValue();
        if (bOn != m_xBtnLight2->isLightOn())
        {
            m_xBtnLight2->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight2->get_state() == TRISTATE_INDET )
            m_xBtnLight2->set_active( m_xBtnLight2->get_active() );
    }
    else
    {
        if( m_xBtnLight2->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight2->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    //Light 2 (Direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_2);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    //Light 3 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_3);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3).GetValue();
        ColorListBox* pLb = m_xLbLight3.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight3->IsNoSelection())
        {
            m_xLbLight3->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 3 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_3);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_3).GetValue();
        if (bOn != m_xBtnLight3->isLightOn())
        {
            m_xBtnLight3->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight3->get_state() == TRISTATE_INDET )
            m_xBtnLight3->set_active( m_xBtnLight3->get_active() );
    }
    else
    {
        if( m_xBtnLight3->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight3->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 3 (Direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_3);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Light 4 (Color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_4);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4).GetValue();
        ColorListBox* pLb = m_xLbLight4.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight4->IsNoSelection())
        {
            m_xLbLight4->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 4 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_4);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_4).GetValue();
        if (bOn != m_xBtnLight4->isLightOn())
        {
            m_xBtnLight4->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight4->get_state() == TRISTATE_INDET )
            m_xBtnLight4->set_active( m_xBtnLight4->get_active() );
    }
    else
    {
        if( m_xBtnLight4->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight4->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 4 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_4);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Light 5 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_5);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5).GetValue();
        ColorListBox* pLb = m_xLbLight5.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight5->IsNoSelection())
        {
            m_xLbLight5->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 5 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_5);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_5).GetValue();
        if (bOn != m_xBtnLight5->isLightOn())
        {
            m_xBtnLight5->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight5->get_state() == TRISTATE_INDET )
            m_xBtnLight5->set_active( m_xBtnLight5->get_active() );
    }
    else
    {
        if( m_xBtnLight5->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight5->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 5 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_5);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Light 6 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_6);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6).GetValue();
        ColorListBox* pLb = m_xLbLight6.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight6->IsNoSelection())
        {
            m_xLbLight6->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 6 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_6);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_6).GetValue();
        if (bOn != m_xBtnLight6->isLightOn())
        {
            m_xBtnLight6->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight6->get_state() == TRISTATE_INDET )
            m_xBtnLight6->set_active( m_xBtnLight6->get_active() );
    }
    else
    {
        if( m_xBtnLight6->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight6->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 6 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_6);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Light 7 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_7);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7).GetValue();
        ColorListBox* pLb = m_xLbLight7.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight7->IsNoSelection())
        {
            m_xLbLight7->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 7 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_7);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_7).GetValue();
        if (bOn != m_xBtnLight7->isLightOn())
        {
            m_xBtnLight7->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight7->get_state() == TRISTATE_INDET )
            m_xBtnLight7->set_active( m_xBtnLight7->get_active() );
    }
    else
    {
        if( m_xBtnLight7->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight7->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 7 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_7);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Light 8 (color)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTCOLOR_8);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8).GetValue();
        ColorListBox* pLb = m_xLbLight8.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbLight8->IsNoSelection())
        {
            m_xLbLight8->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 8 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_8);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_8).GetValue();
        if (bOn != m_xBtnLight8->isLightOn())
        {
            m_xBtnLight8->switchLightOn(bOn);
            bUpdate = true;
        }
        if( m_xBtnLight8->get_state() == TRISTATE_INDET )
            m_xBtnLight8->set_active( m_xBtnLight8->get_active() );
    }
    else
    {
        if( m_xBtnLight8->get_state() != TRISTATE_INDET )
        {
            m_xBtnLight8->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }
    // Light 8 (direction)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTDIRECTION_8);
    if( eState != SfxItemState::DONTCARE )
    {
        bUpdate = true;
    }

    // Ambient light
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_AMBIENTCOLOR);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DSCENE_AMBIENTCOLOR).GetValue();
        ColorListBox* pLb = m_xLbAmbientlight.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbAmbientlight->IsNoSelection())
        {
            m_xLbAmbientlight->SetNoSelection();
            bUpdate = true;
        }
    }


// Textures
    // Art
    if( bBitmap )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_KIND);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_KIND).GetValue();

            if( ( !m_xBtnTexLuminance->get_active() && nValue == 1 ) ||
                ( !m_xBtnTexColor->get_active() && nValue == 3 ) )
            {
                m_xBtnTexLuminance->set_active( nValue == 1 );
                m_xBtnTexColor->set_active( nValue == 3 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_xBtnTexLuminance->get_active() ||
                m_xBtnTexColor->get_active() )
            {
                m_xBtnTexLuminance->set_active( false );
                m_xBtnTexColor->set_active( false );
                bUpdate = true;
            }
        }

        // Mode
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_MODE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_MODE).GetValue();

            if( ( !m_xBtnTexReplace->get_active() && nValue == 1 ) ||
                ( !m_xBtnTexModulate->get_active() && nValue == 2 ) )
            {
                m_xBtnTexReplace->set_active( nValue == 1 );
                m_xBtnTexModulate->set_active( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_xBtnTexReplace->get_active() ||
                m_xBtnTexModulate->get_active() )
            {
                m_xBtnTexReplace->set_active( false );
                m_xBtnTexModulate->set_active( false );
                bUpdate = true;
            }
        }

        // Projection X
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_PROJ_X);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X).GetValue();

            if( ( !m_xBtnTexObjectX->get_active() && nValue == 0 ) ||
                ( !m_xBtnTexParallelX->get_active() && nValue == 1 ) ||
                ( !m_xBtnTexCircleX->get_active() && nValue == 2 ) )
            {
                m_xBtnTexObjectX->set_active( nValue == 0 );
                m_xBtnTexParallelX->set_active( nValue == 1 );
                m_xBtnTexCircleX->set_active( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_xBtnTexObjectX->get_active() ||
                m_xBtnTexParallelX->get_active() ||
                m_xBtnTexCircleX->get_active() )
            {
                m_xBtnTexObjectX->set_active( false );
                m_xBtnTexParallelX->set_active( false );
                m_xBtnTexCircleX->set_active( false );
                bUpdate = true;
            }
        }

        // Projection Y
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_PROJ_Y);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y).GetValue();

            if( ( !m_xBtnTexObjectY->get_active() && nValue == 0 ) ||
                ( !m_xBtnTexParallelY->get_active() && nValue == 1 ) ||
                ( !m_xBtnTexCircleY->get_active() && nValue == 2 ) )
            {
                m_xBtnTexObjectY->set_active( nValue == 0 );
                m_xBtnTexParallelY->set_active( nValue == 1 );
                m_xBtnTexCircleY->set_active( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_xBtnTexObjectY->get_active() ||
                m_xBtnTexParallelY->get_active() ||
                m_xBtnTexCircleY->get_active() )
            {
                m_xBtnTexObjectY->set_active( false );
                m_xBtnTexParallelY->set_active( false );
                m_xBtnTexCircleY->set_active( false );
                bUpdate = true;
            }
        }

        // Filter
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_FILTER);
        if( eState != SfxItemState::DONTCARE )
        {
            bool bValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_FILTER).GetValue();
            if( bValue != m_xBtnTexFilter->get_active() )
            {
                m_xBtnTexFilter->set_active( bValue );
                bUpdate = true;
            }
            if( m_xBtnTexFilter->get_state() == TRISTATE_INDET )
                m_xBtnTexFilter->set_active( bValue );
        }
        else
        {
            if( m_xBtnTexFilter->get_state() != TRISTATE_INDET )
            {
                m_xBtnTexFilter->set_state( TRISTATE_INDET );
                bUpdate = true;
            }
        }
    }


    // Material Favorites
    m_xLbMatFavorites->set_active( 0 );

    // Object color
    eState = rAttrs.GetItemState(XATTR_FILLCOLOR);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(XATTR_FILLCOLOR).GetColorValue();
        ColorListBox* pLb = m_xLbMatColor.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbMatColor->IsNoSelection())
        {
            m_xLbMatColor->SetNoSelection();
            bUpdate = true;
        }
    }

    // Self-luminous color
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_EMISSION);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DOBJ_MAT_EMISSION).GetValue();
        ColorListBox* pLb = m_xLbMatEmission.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbMatEmission->IsNoSelection())
        {
            m_xLbMatEmission->SetNoSelection();
            bUpdate = true;
        }
    }

    // Specular
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_SPECULAR);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DOBJ_MAT_SPECULAR).GetValue();
        ColorListBox* pLb = m_xLbMatSpecular.get();
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_xLbMatSpecular->IsNoSelection())
        {
            m_xLbMatSpecular->SetNoSelection();
            bUpdate = true;
        }
    }

    // Specular Intensity
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY).GetValue();
        if( nValue != m_xMtrMatSpecularIntensity->get_value(FieldUnit::PERCENT) )
        {
            m_xMtrMatSpecularIntensity->set_value(nValue, FieldUnit::PERCENT);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_xMtrMatSpecularIntensity->get_text().isEmpty() )
        {
            m_xMtrMatSpecularIntensity->set_text("");
            bUpdate = true;
        }
    }


// Other
    // Perspective
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_PERSPECTIVE);
    if( eState != SfxItemState::DONTCARE )
    {
        ProjectionType ePT = static_cast<ProjectionType>(rAttrs.Get(SDRATTR_3DSCENE_PERSPECTIVE).GetValue());
        if( ( !m_xBtnPerspective->get_active() && ePT == ProjectionType::Perspective ) ||
            ( m_xBtnPerspective->get_active() && ePT == ProjectionType::Parallel ) )
        {
            m_xBtnPerspective->set_active( ePT == ProjectionType::Perspective );
            bUpdate = true;
        }
        if( m_xBtnPerspective->get_state() == TRISTATE_INDET )
            m_xBtnPerspective->set_active( ePT == ProjectionType::Perspective );
    }
    else
    {
        if( m_xBtnPerspective->get_state() != TRISTATE_INDET )
        {
            m_xBtnPerspective->set_state( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    if( !bUpdate )
    {
        // however the 2D attributes may be different. Compare these and decide

        bUpdate = true;
    }

    // Update preview
    SfxItemSet aSet(rAttrs);

    // set LineStyle hard to drawing::LineStyle_NONE when it's not set so that
    // the default (drawing::LineStyle_SOLID) is not used for 3d preview
    if(SfxItemState::SET != aSet.GetItemState(XATTR_LINESTYLE, false))
        aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));

    // set FillColor hard to WHITE when it's SfxItemState::DONTCARE so that
    // the default (Blue7) is not used for 3d preview
    if(SfxItemState::DONTCARE == aSet.GetItemState(XATTR_FILLCOLOR, false))
        aSet.Put(XFillColorItem(OUString(), COL_WHITE));

    m_xCtlPreview->Set3DAttributes(aSet);
    m_xCtlLightPreview->GetSvx3DLightControl().Set3DAttributes(aSet);

    // try to select light corresponding to active button
    sal_uInt32 nNumber(0xffffffff);

    if(m_xBtnLight1->get_active())
        nNumber = 0;
    else if(m_xBtnLight2->get_active())
        nNumber = 1;
    else if(m_xBtnLight3->get_active())
        nNumber = 2;
    else if(m_xBtnLight4->get_active())
        nNumber = 3;
    else if(m_xBtnLight5->get_active())
        nNumber = 4;
    else if(m_xBtnLight6->get_active())
        nNumber = 5;
    else if(m_xBtnLight7->get_active())
        nNumber = 6;
    else if(m_xBtnLight8->get_active())
        nNumber = 7;

    if(nNumber != 0xffffffff)
    {
        m_xCtlLightPreview->GetSvx3DLightControl().SelectLight(nNumber);
    }

    // handle state of converts possible
    m_xBtnConvertTo3D->set_sensitive(pConvertTo3DItem->GetState());
    m_xBtnLatheObject->set_sensitive(pConvertTo3DLatheItem->GetState());
}


void Svx3DWin::GetAttr( SfxItemSet& rAttrs )
{
    // get remembered 2d attributes from the dialog
    if(mpRemember2DAttributes)
    {
        SfxWhichIter aIter(*mpRemember2DAttributes);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            SfxItemState eState = mpRemember2DAttributes->GetItemState(nWhich, false);
            if(SfxItemState::DONTCARE == eState)
                rAttrs.InvalidateItem(nWhich);
            else if(SfxItemState::SET == eState)
                rAttrs.Put(mpRemember2DAttributes->Get(nWhich, false));

            nWhich = aIter.NextWhich();
        }
    }

//Others must stand as the front on all sides
    // Perspective
    if( m_xBtnPerspective->get_state() != TRISTATE_INDET )
    {
        ProjectionType nValue;
        if( m_xBtnPerspective->get_active() )
            nValue = ProjectionType::Perspective;
        else
            nValue = ProjectionType::Parallel;
        rAttrs.Put(Svx3DPerspectiveItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_PERSPECTIVE);

// Geometry
    // Possible determine PoolUnit (in this case this has not happened in Update() )
    if( !mpImpl->pPool )
    {
        OSL_FAIL( "No Pool in GetAttr()! May be incompatible to drviewsi.cxx ?" );
        mpImpl->pPool = rAttrs.GetPool();
        DBG_ASSERT( mpImpl->pPool, "Where is the Pool?" );
        ePoolUnit = mpImpl->pPool->GetMetric( SID_ATTR_LINE_WIDTH );

        eFUnit = GetModuleFieldUnit( rAttrs );
    }

    // Number of segments (horizontal)
    if( !m_xNumHorizontal->get_text().isEmpty() )
    {
        sal_uInt32 nValue = static_cast<sal_uInt32>(m_xNumHorizontal->get_value());
        rAttrs.Put(makeSvx3DHorizontalSegmentsItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_HORZ_SEGS);

    //  Number of segments (vertical)
    if( !m_xNumVertical->get_text().isEmpty() )
    {
        sal_uInt32 nValue = static_cast<sal_uInt32>(m_xNumVertical->get_value());
        rAttrs.Put(makeSvx3DVerticalSegmentsItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_VERT_SEGS);

    // Depth
    if( !m_xMtrDepth->get_text().isEmpty() )
    {
        sal_uInt32 nValue = GetCoreValue(*m_xMtrDepth, ePoolUnit);
        rAttrs.Put(makeSvx3DDepthItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_DEPTH);

    // Double-sided
    TriState eState = m_xBtnDoubleSided->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DDoubleSidedItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_DOUBLE_SIDED);

    // Edge rounding
    if( !m_xMtrPercentDiagonal->get_text().isEmpty() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_xMtrPercentDiagonal->get_value(FieldUnit::PERCENT));
        rAttrs.Put(makeSvx3DPercentDiagonalItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_PERCENT_DIAGONAL);

    // Depth scale
    if( !m_xMtrBackscale->get_text().isEmpty() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_xMtrBackscale->get_value(FieldUnit::PERCENT));
        rAttrs.Put(makeSvx3DBackscaleItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_BACKSCALE);

    // End angle
    if( !m_xMtrEndAngle->get_text().isEmpty() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_xMtrEndAngle->get_value(FieldUnit::DEGREE));
        rAttrs.Put(makeSvx3DEndAngleItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_END_ANGLE);

    // Normal type
    sal_uInt16 nValue = 99;
    if( m_xBtnNormalsObj->get_active() )
        nValue = 0;
    else if( m_xBtnNormalsFlat->get_active() )
        nValue = 1;
    else if( m_xBtnNormalsSphere->get_active() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DNormalsKindItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_NORMALS_KIND);

    // Normal inverted
    eState = m_xBtnNormalsInvert->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DNormalsInvertItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_NORMALS_INVERT);

    // 2-sided lighting
    eState = m_xBtnTwoSidedLighting->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DTwoSidedLightingItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING);

// Representation
    // Shade mode
    if( m_xLbShademode->get_active() != -1 )
    {
        nValue = m_xLbShademode->get_active();
        rAttrs.Put(Svx3DShadeModeItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_SHADE_MODE);

    // 3D-Shadow
    eState = m_xBtnShadow3d->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DShadow3DItem(bValue));
        rAttrs.Put(makeSdrShadowItem(bValue));
    }
    else
    {
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_SHADOW_3D);
        rAttrs.InvalidateItem(SDRATTR_SHADOW);
    }

    // Slant (Shadow)
    if( !m_xMtrSlant->get_text().isEmpty() )
    {
        sal_uInt16 nValue2 = static_cast<sal_uInt16>(m_xMtrSlant->get_value(FieldUnit::DEGREE));
        rAttrs.Put(makeSvx3DShadowSlantItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_SHADOW_SLANT);

    // Distance
    if( !m_xMtrDistance->get_text().isEmpty() )
    {
        sal_uInt32 nValue2 = GetCoreValue(*m_xMtrDistance, ePoolUnit);
        rAttrs.Put(makeSvx3DDistanceItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_DISTANCE);

    // Focal length
    if( !m_xMtrFocalLength->get_text().isEmpty() )
    {
        sal_uInt32 nValue2 = GetCoreValue(*m_xMtrFocalLength, ePoolUnit);
        rAttrs.Put(makeSvx3DFocalLengthItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_FOCAL_LENGTH);

    // Lighting
    Color aColor;
    const SfxItemSet aLightItemSet(m_xCtlLightPreview->GetSvx3DLightControl().Get3DAttributes());

    // Light 1 color
    if (!m_xLbLight1->IsNoSelection())
    {
        aColor = m_xLbLight1->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor1Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_1);
    // Light 1 (on/off)
    eState = m_xBtnLight1->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight1->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff1Item(bValue));

        // Light 1 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_1);


    // Light 2 color
    if (!m_xLbLight2->IsNoSelection())
    {
        aColor = m_xLbLight2->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor2Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_2);
    // Light 2 (on/off)
    eState = m_xBtnLight2->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight2->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff2Item(bValue));

        // Light 2 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_2);

    // Light 3 color
    if (!m_xLbLight3->IsNoSelection())
    {
        aColor = m_xLbLight3->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor3Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_3);
    // Light 3 (on/off)
    eState = m_xBtnLight3->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight3->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff3Item(bValue));

        // Light 3 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_3);

    // Light 4 color
    if (!m_xLbLight4->IsNoSelection())
    {
        aColor = m_xLbLight4->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor4Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_4);
    // Light 4 (on/off)
    eState = m_xBtnLight4->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight4->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff4Item(bValue));

        // Light 4 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_4);

    // Light 5 color
    if (!m_xLbLight5->IsNoSelection())
    {
        aColor = m_xLbLight5->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor5Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_5);
    // Light 5 (on/off)
    eState = m_xBtnLight5->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight5->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff5Item(bValue));

        // Light 5 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_5);

    // Light 6 color
    if (!m_xLbLight6->IsNoSelection())
    {
        aColor = m_xLbLight6->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor6Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_6);
    // Light 6 (on/off)
    eState = m_xBtnLight6->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight6->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff6Item(bValue));

        // Light 6 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_6);

    // Light 7 color
    if (!m_xLbLight7->IsNoSelection())
    {
        aColor = m_xLbLight7->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor7Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_7);
    // Light 7 (on/off)
    eState = m_xBtnLight7->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight7->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff7Item(bValue));

        // Light 7 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_7);

    // Light 8 color
    if (!m_xLbLight8->IsNoSelection())
    {
        aColor = m_xLbLight8->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor8Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_8);
    // Light 8 (on/off)
    eState = m_xBtnLight8->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = m_xBtnLight8->isLightOn();
        rAttrs.Put(makeSvx3DLightOnOff8Item(bValue));

        // Light 8 (direction)
        if( bValue )
        {
            rAttrs.Put(aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8));
        }
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTON_8);

    // Ambient light
    if (!m_xLbAmbientlight->IsNoSelection())
    {
        aColor = m_xLbAmbientlight->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DAmbientcolorItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_AMBIENTCOLOR);

// Textures
    // Art
    nValue = 99;
    if( m_xBtnTexLuminance->get_active() )
        nValue = 1;
    else if( m_xBtnTexColor->get_active() )
        nValue = 3;

    if( nValue == 1 || nValue == 3 )
        rAttrs.Put(Svx3DTextureKindItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_KIND);


    // Mode
    nValue = 99;
    if( m_xBtnTexReplace->get_active() )
        nValue = 1;
    else if( m_xBtnTexModulate->get_active() )
        nValue = 2;

    if( nValue == 1 || nValue == 2 )
        rAttrs.Put(Svx3DTextureModeItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_MODE);

    // X projection
    nValue = 99;
    if( m_xBtnTexObjectX->get_active() )
        nValue = 0;
    else if( m_xBtnTexParallelX->get_active() )
        nValue = 1;
    else if( m_xBtnTexCircleX->get_active() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DTextureProjectionXItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_PROJ_X);

    // Y projection
    nValue = 99;
    if( m_xBtnTexObjectY->get_active() )
        nValue = 0;
    else if( m_xBtnTexParallelY->get_active() )
        nValue = 1;
    else if( m_xBtnTexCircleY->get_active() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DTextureProjectionYItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_PROJ_Y);


    // Filter
    eState = m_xBtnTexFilter->get_state();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DTextureFilterItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_FILTER);


// Material
    // Object color
    if (!m_xLbMatColor->IsNoSelection())
    {
        aColor = m_xLbMatColor->GetSelectEntryColor();
        rAttrs.Put( XFillColorItem( "", aColor) );
    }
    else
    {
        rAttrs.InvalidateItem( XATTR_FILLCOLOR );
    }

    // luminous color
    if (!m_xLbMatEmission->IsNoSelection())
    {
        aColor = m_xLbMatEmission->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DMaterialEmissionItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_EMISSION);

    // Specular
    if (!m_xLbMatSpecular->IsNoSelection())
    {
        aColor = m_xLbMatSpecular->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DMaterialSpecularItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_SPECULAR);

    // Specular intensity
    if( !m_xMtrMatSpecularIntensity->get_text().isEmpty() )
    {
        sal_uInt16 nValue2 = static_cast<sal_uInt16>(m_xMtrMatSpecularIntensity->get_value(FieldUnit::PERCENT));
        rAttrs.Put(makeSvx3DMaterialSpecularIntensityItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY);
}

void Svx3DWin::Resize()
{
    Size aWinSize( GetOutputSizePixel() ); // why rSize in Resizing()?

    if( aWinSize.Height() >= GetMinOutputSizePixel().Height() &&
        aWinSize.Width() >= GetMinOutputSizePixel().Width() )
    {
        // Hide
        m_xBtnUpdate->hide();
        m_xBtnAssign->hide();

        m_xBtnConvertTo3D->hide();
        m_xBtnLatheObject->hide();
        m_xBtnPerspective->hide();

        m_xCtlPreview->Hide();
        m_xLightPreviewGrid->hide();

        m_xFLGeometrie->hide();
        m_xFLRepresentation->hide();
        m_xFLLight->hide();
        m_xFLTexture->hide();
        m_xFLMaterial->hide();

        // Show
        m_xBtnUpdate->show();
        m_xBtnAssign->show();

        m_xBtnConvertTo3D->show();
        m_xBtnLatheObject->show();
        m_xBtnPerspective->show();

        if( m_xBtnGeo->get_active() )
            ClickViewTypeHdl(*m_xBtnGeo);
        if( m_xBtnRepresentation->get_active() )
            ClickViewTypeHdl(*m_xBtnRepresentation);
        if( m_xBtnLight->get_active() )
            ClickViewTypeHdl(*m_xBtnLight);
        if( m_xBtnTexture->get_active() )
            ClickViewTypeHdl(*m_xBtnTexture);
        if( m_xBtnMaterial->get_active() )
            ClickViewTypeHdl(*m_xBtnMaterial);
    }

    SfxDockingWindow::Resize();
}

IMPL_LINK_NOARG(Svx3DWin, ClickUpdateHdl, weld::ToggleButton&, void)
{
    bUpdate = m_xBtnUpdate->get_active();

    if( bUpdate )
    {
        SfxDispatcher* pDispatcher = LocalGetDispatcher(pBindings);
        if (pDispatcher != nullptr)
        {
            SfxBoolItem aItem( SID_3D_STATE, true );
            pDispatcher->ExecuteList(SID_3D_STATE,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
        }
    }
    else
    {
        // Controls can be disabled during certain circumstances
    }
}

IMPL_LINK_NOARG(Svx3DWin, ClickAssignHdl, weld::Button&, void)
{
    SfxDispatcher* pDispatcher = LocalGetDispatcher(pBindings);
    if (pDispatcher != nullptr)
    {
        SfxBoolItem aItem( SID_3D_ASSIGN, true );
        pDispatcher->ExecuteList(SID_3D_ASSIGN,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
    }
}

IMPL_LINK( Svx3DWin, ClickViewTypeHdl, weld::Button&, rBtn, void )
{
    // Since the permanent updating of the preview would be too expensive
    bool bUpdatePreview = m_xBtnLight->get_active();

    m_xBtnGeo->set_active(m_xBtnGeo.get() == &rBtn);
    m_xBtnRepresentation->set_active(m_xBtnRepresentation.get() == &rBtn);
    m_xBtnLight->set_active(m_xBtnLight.get() == &rBtn);
    m_xBtnTexture->set_active(m_xBtnTexture.get() == &rBtn);
    m_xBtnMaterial->set_active(m_xBtnMaterial.get() == &rBtn);

    if( m_xBtnGeo->get_active() )
        eViewType = ViewType3D::Geo;
    if( m_xBtnRepresentation->get_active() )
        eViewType = ViewType3D::Representation;
    if( m_xBtnLight->get_active() )
        eViewType = ViewType3D::Light;
    if( m_xBtnTexture->get_active() )
        eViewType = ViewType3D::Texture;
    if( m_xBtnMaterial->get_active() )
        eViewType = ViewType3D::Material;

    // Geometry
    if( eViewType == ViewType3D::Geo )
    {
        m_xFLSegments->show();
        m_xFLGeometrie->show();
        m_xFLNormals->show();
    }
    else
    {
        m_xFLSegments->hide();
        m_xFLGeometrie->hide();
        m_xFLNormals->hide();
    }

    // Representation
    if( eViewType == ViewType3D::Representation )
    {
        m_xFLShadow->show();
        m_xFLCamera->show();
        m_xFLRepresentation->show();
    }
    else
    {
        m_xFLShadow->hide();
        m_xFLCamera->hide();
        m_xFLRepresentation->hide();
    }

    // Lighting
    if( eViewType == ViewType3D::Light )
    {
        m_xFLLight->show();

        ColorListBox* pLb = GetCLbByButton();
        if( pLb )
            pLb->show();

        m_xLightPreviewGrid->show();
        m_xCtlPreview->Hide();
    }
    else
    {
        m_xFLLight->hide();

        if( !m_xCtlPreview->IsVisible() )
        {
            m_xCtlPreview->Show();
            m_xLightPreviewGrid->hide();
        }
    }

    // Textures
    if (eViewType == ViewType3D::Texture)
        m_xFLTexture->show();
    else
        m_xFLTexture->hide();

    // Material
    if( eViewType == ViewType3D::Material )
    {
        m_xFLMatSpecular->show();
        m_xFLMaterial->show();
    }
    else
    {
        m_xFLMatSpecular->hide();
        m_xFLMaterial->hide();
    }
    if( bUpdatePreview && !m_xBtnLight->get_active() )
        UpdatePreview();
}

IMPL_LINK( Svx3DWin, ClickHdl, weld::Button&, rBtn, void )
{
    bool bUpdatePreview = false;
    sal_uInt16 nSId = 0;

    if( &rBtn == m_xBtnConvertTo3D.get() )
    {
        nSId = SID_CONVERT_TO_3D;
    }
    else if( &rBtn == m_xBtnLatheObject.get() )
    {
        nSId = SID_CONVERT_TO_3D_LATHE_FAST;
    }
    // Geometry
    else if( &rBtn == m_xBtnNormalsObj.get() ||
             &rBtn == m_xBtnNormalsFlat.get() ||
             &rBtn == m_xBtnNormalsSphere.get() )
    {
        m_xBtnNormalsObj->set_active( &rBtn == m_xBtnNormalsObj.get() );
        m_xBtnNormalsFlat->set_active( &rBtn == m_xBtnNormalsFlat.get() );
        m_xBtnNormalsSphere->set_active( &rBtn == m_xBtnNormalsSphere.get() );
        bUpdatePreview = true;
    }
    else if( &rBtn == m_xBtnLight1->get_widget() ||
             &rBtn == m_xBtnLight2->get_widget() ||
             &rBtn == m_xBtnLight3->get_widget() ||
             &rBtn == m_xBtnLight4->get_widget() ||
             &rBtn == m_xBtnLight5->get_widget() ||
             &rBtn == m_xBtnLight6->get_widget() ||
             &rBtn == m_xBtnLight7->get_widget() ||
             &rBtn == m_xBtnLight8->get_widget() )
    {
        // Lighting
        LightButton* pToggleBtn = GetLbByButton(&rBtn);

        ColorListBox* pLb = GetCLbByButton(pToggleBtn);
        pLb->show();

        bool bIsChecked = pToggleBtn->get_prev_active();

        if (pToggleBtn != m_xBtnLight1.get() && m_xBtnLight1->get_active())
        {
            m_xBtnLight1->set_active( false );
            m_xBtnLight1->set_prev_active(false);
            m_xLbLight1->hide();
        }
        if (pToggleBtn != m_xBtnLight2.get() && m_xBtnLight2->get_active())
        {
            m_xBtnLight2->set_active( false );
            m_xBtnLight2->set_prev_active(false);
            m_xLbLight2->hide();
        }
        if( pToggleBtn != m_xBtnLight3.get() && m_xBtnLight3->get_active() )
        {
            m_xBtnLight3->set_active( false );
            m_xBtnLight3->set_prev_active(false);
            m_xLbLight3->hide();
        }
        if( pToggleBtn != m_xBtnLight4.get() && m_xBtnLight4->get_active() )
        {
            m_xBtnLight4->set_active( false );
            m_xBtnLight4->set_prev_active(false);
            m_xLbLight4->hide();
        }
        if( pToggleBtn != m_xBtnLight5.get() && m_xBtnLight5->get_active() )
        {
            m_xBtnLight5->set_active( false );
            m_xBtnLight5->set_prev_active(false);
            m_xLbLight5->hide();
        }
        if( pToggleBtn != m_xBtnLight6.get() && m_xBtnLight6->get_active() )
        {
            m_xBtnLight6->set_active( false );
            m_xBtnLight6->set_prev_active(false);
            m_xLbLight6->hide();
        }
        if( pToggleBtn != m_xBtnLight7.get() && m_xBtnLight7->get_active() )
        {
            m_xBtnLight7->set_active( false );
            m_xBtnLight7->set_prev_active(false);
            m_xLbLight7->hide();
        }
        if( pToggleBtn != m_xBtnLight8.get() && m_xBtnLight8->get_active() )
        {
            m_xBtnLight8->set_active( false );
            m_xBtnLight8->set_prev_active(false);
            m_xLbLight8->hide();
        }

        //update light button
        pToggleBtn->set_active(true);
        pToggleBtn->set_prev_active(true);
        if (bIsChecked)
            pToggleBtn->switchLightOn(!pToggleBtn->isLightOn());

        bool bEnable = pToggleBtn->isLightOn();
        m_xBtnLightColor->set_sensitive( bEnable );
        pLb->set_sensitive( bEnable );

        ClickLight(*pToggleBtn);
        bUpdatePreview = true;
    }
    // Textures
    else if( &rBtn == m_xBtnTexLuminance.get() ||
             &rBtn == m_xBtnTexColor.get() )
    {
        m_xBtnTexLuminance->set_active( &rBtn == m_xBtnTexLuminance.get() );
        m_xBtnTexColor->set_active( &rBtn == m_xBtnTexColor.get() );
        bUpdatePreview = true;
    }
    else if( &rBtn == m_xBtnTexReplace.get() ||
             &rBtn == m_xBtnTexModulate.get() )
    {
        m_xBtnTexReplace->set_active( &rBtn == m_xBtnTexReplace.get() );
        m_xBtnTexModulate->set_active( &rBtn == m_xBtnTexModulate.get() );
        bUpdatePreview = true;
    }
    else if( &rBtn == m_xBtnTexParallelX.get() ||
             &rBtn == m_xBtnTexCircleX.get() ||
             &rBtn == m_xBtnTexObjectX.get() )
    {
        m_xBtnTexParallelX->set_active( &rBtn == m_xBtnTexParallelX.get() );
        m_xBtnTexCircleX->set_active( &rBtn == m_xBtnTexCircleX.get() );
        m_xBtnTexObjectX->set_active( &rBtn == m_xBtnTexObjectX.get() );
        bUpdatePreview = true;
    }
    else if( &rBtn == m_xBtnTexParallelY.get() ||
             &rBtn == m_xBtnTexCircleY.get() ||
             &rBtn == m_xBtnTexObjectY.get() )
    {
        m_xBtnTexParallelY->set_active( &rBtn == m_xBtnTexParallelY.get() );
        m_xBtnTexCircleY->set_active( &rBtn == m_xBtnTexCircleY.get() );
        m_xBtnTexObjectY->set_active( &rBtn == m_xBtnTexObjectY.get() );
        bUpdatePreview = true;
    }
    else if (&rBtn == m_xBtnShadow3d.get())
    {
        m_xFtSlant->set_sensitive( m_xBtnShadow3d->get_active() );
        m_xMtrSlant->set_sensitive( m_xBtnShadow3d->get_active() );
        bUpdatePreview = true;
    }
    // Other (no groups)
    else
    {
        bUpdatePreview = true;
    }

    if( nSId > 0 )
    {
        SfxDispatcher* pDispatcher = LocalGetDispatcher(pBindings);
        if (pDispatcher != nullptr)
        {
            SfxBoolItem aItem( nSId, true );
            pDispatcher->ExecuteList(nSId,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
        }
    }
    else if( bUpdatePreview )
        UpdatePreview();
}

IMPL_LINK( Svx3DWin, ClickColorHdl, weld::Button&, rBtn, void)
{
    SvColorDialog aColorDlg;
    ColorListBox* pLb;

    if( &rBtn == m_xBtnLightColor.get() )
        pLb = GetCLbByButton();
    else if( &rBtn == m_xBtnAmbientColor.get() )
        pLb = m_xLbAmbientlight.get();
    else if( &rBtn == m_xBtnMatColor.get() )
        pLb = m_xLbMatColor.get();
    else if( &rBtn == m_xBtnEmissionColor.get() )
        pLb = m_xLbMatEmission.get();
    else // if( &rBtn == m_xBtnSpecularColor.get() )
        pLb = m_xLbMatSpecular.get();

    Color aColor = pLb->GetSelectEntryColor();

    aColorDlg.SetColor( aColor );
    if( aColorDlg.Execute(GetFrameWeld()) == RET_OK )
    {
        aColor = aColorDlg.GetColor();
        LBSelectColor(pLb, aColor);
        SelectColorHdl(*pLb);
    }
}

IMPL_LINK( Svx3DWin, SelectHdl, weld::ComboBox&, rListBox, void )
{
    bool bUpdatePreview = false;

    // Material
    if (&rListBox == m_xLbMatFavorites.get())
    {
        Color aColObj( COL_WHITE );
        Color aColEmis( COL_BLACK );
        Color aColSpec( COL_WHITE );
        sal_uInt16 nSpecIntens = 20;

        switch( m_xLbMatFavorites->get_active() )
        {
            case 1: // Metall
            {
                aColObj = Color(230,230,255);
                aColEmis = Color(10,10,30);
                aColSpec = Color(200,200,200);
                nSpecIntens = 20;
            }
            break;

            case 2: // Gold
            {
                aColObj = Color(230,255,0);
                aColEmis = Color(51,0,0);
                aColSpec = Color(255,255,240);
                nSpecIntens = 20;
            }
            break;

            case 3: // Chrome
            {
                aColObj = Color(36,117,153);
                aColEmis = Color(18,30,51);
                aColSpec = Color(230,230,255);
                nSpecIntens = 2;
            }
            break;

            case 4: // Plastic
            {
                aColObj = Color(255,48,57);
                aColEmis = Color(35,0,0);
                aColSpec = Color(179,202,204);
                nSpecIntens = 60;
            }
            break;

            case 5: // Wood
            {
                aColObj = Color(153,71,1);
                aColEmis = Color(21,22,0);
                aColSpec = Color(255,255,153);
                nSpecIntens = 75;
            }
            break;
        }
        LBSelectColor( m_xLbMatColor.get(), aColObj );
        LBSelectColor( m_xLbMatEmission.get(), aColEmis );
        LBSelectColor( m_xLbMatSpecular.get(), aColSpec );
        m_xMtrMatSpecularIntensity->set_value(nSpecIntens, FieldUnit::PERCENT);

        bUpdatePreview = true;
    }
    else if (&rListBox == m_xLbShademode.get())
        bUpdatePreview = true;

    if( bUpdatePreview )
        UpdatePreview();
}

IMPL_LINK( Svx3DWin, SelectColorHdl, ColorListBox&, rListBox, void )
{
    bool bUpdatePreview = false;

    if( &rListBox == m_xLbMatColor.get() ||
        &rListBox == m_xLbMatEmission.get() ||
        &rListBox == m_xLbMatSpecular.get() )
    {
        m_xLbMatFavorites->set_active( 0 );
        bUpdatePreview = true;
    }
    // Lighting
    else if( &rListBox == m_xLbAmbientlight.get() )
    {
        bUpdatePreview = true;
    }
    else if( &rListBox == m_xLbLight1.get() ||
             &rListBox == m_xLbLight2.get() ||
             &rListBox == m_xLbLight3.get() ||
             &rListBox == m_xLbLight4.get() ||
             &rListBox == m_xLbLight5.get() ||
             &rListBox == m_xLbLight6.get() ||
             &rListBox == m_xLbLight7.get() ||
             &rListBox == m_xLbLight8.get() )
    {
        bUpdatePreview = true;
    }

    if( bUpdatePreview )
        UpdatePreview();
}

IMPL_LINK_NOARG( Svx3DWin, ModifyMetricHdl, weld::MetricSpinButton&, void )
{
    UpdatePreview();
}

IMPL_LINK_NOARG( Svx3DWin, ModifySpinHdl, weld::SpinButton&, void )
{
    UpdatePreview();
}

void Svx3DWin::ClickLight(const LightButton& rBtn)
{
    sal_uInt16 nLightSource = GetLightSource( &rBtn );
    ColorListBox* pLb = GetCLbByButton( &rBtn );
    Color aColor( pLb->GetSelectEntryColor() );
    SfxItemSet aLightItemSet(m_xCtlLightPreview->GetSvx3DLightControl().Get3DAttributes());
    const bool bOnOff(rBtn.isLightOn());

    switch(nLightSource)
    {
        case 0: aLightItemSet.Put(makeSvx3DLightcolor1Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff1Item(bOnOff)); break;
        case 1: aLightItemSet.Put(makeSvx3DLightcolor2Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff2Item(bOnOff)); break;
        case 2: aLightItemSet.Put(makeSvx3DLightcolor3Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff3Item(bOnOff)); break;
        case 3: aLightItemSet.Put(makeSvx3DLightcolor4Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff4Item(bOnOff)); break;
        case 4: aLightItemSet.Put(makeSvx3DLightcolor5Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff5Item(bOnOff)); break;
        case 5: aLightItemSet.Put(makeSvx3DLightcolor6Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff6Item(bOnOff)); break;
        case 6: aLightItemSet.Put(makeSvx3DLightcolor7Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff7Item(bOnOff)); break;
        default:
        case 7: aLightItemSet.Put(makeSvx3DLightcolor8Item(aColor)); aLightItemSet.Put(makeSvx3DLightOnOff8Item(bOnOff)); break;
    }

    m_xCtlLightPreview->GetSvx3DLightControl().Set3DAttributes(aLightItemSet);
    m_xCtlLightPreview->GetSvx3DLightControl().SelectLight(nLightSource);
    m_xCtlLightPreview->CheckSelection();
}

IMPL_LINK_NOARG(Svx3DWin, ChangeSelectionCallbackHdl, SvxLightCtl3D*, void)
{
    const sal_uInt32 nLight(m_xCtlLightPreview->GetSvx3DLightControl().GetSelectedLight());
    weld::Button* pBtn = nullptr;

    switch( nLight )
    {
        case 0: pBtn = m_xBtnLight1->get_widget(); break;
        case 1: pBtn = m_xBtnLight2->get_widget(); break;
        case 2: pBtn = m_xBtnLight3->get_widget(); break;
        case 3: pBtn = m_xBtnLight4->get_widget(); break;
        case 4: pBtn = m_xBtnLight5->get_widget(); break;
        case 5: pBtn = m_xBtnLight6->get_widget(); break;
        case 6: pBtn = m_xBtnLight7->get_widget(); break;
        case 7: pBtn = m_xBtnLight8->get_widget(); break;
        default: break;
    }

    if (pBtn)
        ClickHdl(*pBtn);
    else
    {
        // Status: No lamp selected
        if( m_xBtnLight1->get_active() )
        {
            m_xBtnLight1->set_active( false );
            m_xLbLight1->set_sensitive( false );
        }
        else if( m_xBtnLight2->get_active() )
        {
            m_xBtnLight2->set_active( false );
            m_xLbLight2->set_sensitive( false );
        }
        else if( m_xBtnLight3->get_active() )
        {
            m_xBtnLight3->set_active( false );
            m_xLbLight3->set_sensitive( false );
        }
        else if( m_xBtnLight4->get_active() )
        {
            m_xBtnLight4->set_active( false );
            m_xLbLight4->set_sensitive( false );
        }
        else if( m_xBtnLight5->get_active() )
        {
            m_xBtnLight5->set_active( false );
            m_xLbLight5->set_sensitive( false );
        }
        else if( m_xBtnLight6->get_active() )
        {
            m_xBtnLight6->set_active( false );
            m_xLbLight6->set_sensitive( false );
        }
        else if( m_xBtnLight7->get_active() )
        {
            m_xBtnLight7->set_active( false );
            m_xLbLight7->set_sensitive( false );
        }
        else if( m_xBtnLight8->get_active() )
        {
            m_xBtnLight8->set_active( false );
            m_xLbLight8->set_sensitive( false );
        }
        m_xBtnLightColor->set_sensitive( false );
    }
}

namespace
{
    OUString lcl_makeColorName(const Color& rColor)
    {
        OUString aStr = SvxResId(RID_SVXFLOAT3D_FIX_R) +
                        OUString::number(rColor.GetRed()) +
                        " " +
                        SvxResId(RID_SVXFLOAT3D_FIX_G) +
                        OUString::number(rColor.GetGreen()) +
                        " " +
                        SvxResId(RID_SVXFLOAT3D_FIX_B) +
                        OUString::number(rColor.GetBlue());
        return aStr;
    }
}

// Method to ensure that the LB is also associated with a color
void Svx3DWin::LBSelectColor( ColorListBox* pLb, const Color& rColor )
{
    pLb->SetNoSelection();
    pLb->SelectEntry(std::make_pair(rColor, lcl_makeColorName(rColor)));
}

void Svx3DWin::UpdatePreview()
{
    if(!pModel)
    {
        pModel.reset(new FmFormModel());
    }

    // Get Itemset
    SfxItemSet aSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{});

    // Get Attributes and set the preview
    GetAttr( aSet );
    m_xCtlPreview->Set3DAttributes( aSet );
    m_xCtlLightPreview->GetSvx3DLightControl().Set3DAttributes( aSet );
}


// document is to be reloaded, destroy remembered ItemSet
void Svx3DWin::DocumentReload()
{
    mpRemember2DAttributes.reset();
}

void Svx3DWin::InitColorLB()
{
    // First...
    Color aColWhite( COL_WHITE );
    Color aColBlack( COL_BLACK );
    m_xLbLight1->SelectEntry( aColWhite );
    m_xLbLight2->SelectEntry( aColWhite );
    m_xLbLight3->SelectEntry( aColWhite );
    m_xLbLight4->SelectEntry( aColWhite );
    m_xLbLight5->SelectEntry( aColWhite );
    m_xLbLight6->SelectEntry( aColWhite );
    m_xLbLight7->SelectEntry( aColWhite );
    m_xLbLight8->SelectEntry( aColWhite );
    m_xLbAmbientlight->SelectEntry( aColBlack );
    m_xLbMatColor->SelectEntry( aColWhite );
    m_xLbMatEmission->SelectEntry( aColBlack );
    m_xLbMatSpecular->SelectEntry( aColWhite );
}

sal_uInt16 Svx3DWin::GetLightSource( const LightButton* pBtn )
{
    sal_uInt16 nLight = 8;

    if (pBtn == m_xBtnLight1.get())
        nLight = 0;
    else if (pBtn == m_xBtnLight2.get())
        nLight = 1;
    else if( pBtn == m_xBtnLight3.get() )
        nLight = 2;
    else if( pBtn == m_xBtnLight4.get() )
        nLight = 3;
    else if( pBtn == m_xBtnLight5.get() )
        nLight = 4;
    else if( pBtn == m_xBtnLight6.get() )
        nLight = 5;
    else if( pBtn == m_xBtnLight7.get() )
        nLight = 6;
    else if( pBtn == m_xBtnLight8.get() )
        nLight = 7;

    return nLight;
};

ColorListBox* Svx3DWin::GetCLbByButton( const LightButton* pBtn )
{
    ColorListBox* pLb = nullptr;

    if( pBtn == nullptr )
    {
        if( m_xBtnLight1->get_active() )
            pLb = m_xLbLight1.get();
        else if( m_xBtnLight2->get_active() )
            pLb = m_xLbLight2.get();
        else if( m_xBtnLight3->get_active() )
            pLb = m_xLbLight3.get();
        else if( m_xBtnLight4->get_active() )
            pLb = m_xLbLight4.get();
        else if( m_xBtnLight5->get_active() )
            pLb = m_xLbLight5.get();
        else if( m_xBtnLight6->get_active() )
            pLb = m_xLbLight6.get();
        else if( m_xBtnLight7->get_active() )
            pLb = m_xLbLight7.get();
        else if( m_xBtnLight8->get_active() )
            pLb = m_xLbLight8.get();
    }
    else
    {
        if( pBtn == m_xBtnLight1.get() )
            pLb = m_xLbLight1.get();
        else if (pBtn == m_xBtnLight2.get())
            pLb = m_xLbLight2.get();
        else if( pBtn == m_xBtnLight3.get() )
            pLb = m_xLbLight3.get();
        else if( pBtn == m_xBtnLight4.get() )
            pLb = m_xLbLight4.get();
        else if( pBtn == m_xBtnLight5.get() )
            pLb = m_xLbLight5.get();
        else if( pBtn == m_xBtnLight6.get() )
            pLb = m_xLbLight6.get();
        else if( pBtn == m_xBtnLight7.get() )
            pLb = m_xLbLight7.get();
        else if( pBtn == m_xBtnLight8.get() )
            pLb = m_xLbLight8.get();
    }
    return pLb;
};

LightButton* Svx3DWin::GetLbByButton( const weld::Button* pBtn )
{
    LightButton* pLb = nullptr;

    if( pBtn == m_xBtnLight1->get_widget() )
        pLb = m_xBtnLight1.get();
    else if (pBtn == m_xBtnLight2->get_widget() )
        pLb = m_xBtnLight2.get();
    else if( pBtn == m_xBtnLight3->get_widget() )
        pLb = m_xBtnLight3.get();
    else if( pBtn == m_xBtnLight4->get_widget() )
        pLb = m_xBtnLight4.get();
    else if( pBtn == m_xBtnLight5->get_widget() )
        pLb = m_xBtnLight5.get();
    else if( pBtn == m_xBtnLight6->get_widget() )
        pLb = m_xBtnLight6.get();
    else if( pBtn == m_xBtnLight7->get_widget() )
        pLb = m_xBtnLight7.get();
    else if( pBtn == m_xBtnLight8->get_widget() )
        pLb = m_xBtnLight8.get();

    return pLb;
};

// Derivation from SfxChildWindow as "containers" for effects
Svx3DChildWindow::Svx3DChildWindow( vcl::Window* _pParent,
                                                         sal_uInt16 nId,
                                                         SfxBindings* pBindings,
                                                         SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    VclPtr<Svx3DWin> pWin = VclPtr<Svx3DWin>::Create( pBindings, this, _pParent );
    SetWindow(pWin);

    pWin->Initialize( pInfo );
}

Svx3DCtrlItem::Svx3DCtrlItem( sal_uInt16 _nId,
                                SfxBindings* _pBindings) :
    SfxControllerItem( _nId, *_pBindings )
{
}


void Svx3DCtrlItem::StateChanged( sal_uInt16 /*nSId*/,
                        SfxItemState /*eState*/, const SfxPoolItem* /*pItem*/ )
{
}

// ControllerItem for Status Slot SID_CONVERT_TO_3D

SvxConvertTo3DItem::SvxConvertTo3DItem(sal_uInt16 _nId, SfxBindings* _pBindings)
:   SfxControllerItem(_nId, *_pBindings),
    bState(false)
{
}

void SvxConvertTo3DItem::StateChanged(sal_uInt16 /*_nId*/, SfxItemState eState, const SfxPoolItem* /*pState*/)
{
    bool bNewState = (eState != SfxItemState::DISABLED);
    if(bNewState != bState)
    {
        bState = bNewState;
        SfxDispatcher* pDispatcher = LocalGetDispatcher(&GetBindings());
        if (pDispatcher != nullptr)
        {
            SfxBoolItem aItem( SID_3D_STATE, true );
            pDispatcher->ExecuteList(SID_3D_STATE,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD, { &aItem });
        }
    }
}

LightButton::LightButton(std::unique_ptr<weld::ToggleButton> xButton)
    : m_xButton(std::move(xButton))
    , m_bLightOn(false)
    , m_bButtonPrevActive(false)
{
    m_xButton->set_from_icon_name(RID_SVXBMP_LAMP_OFF);
}

void LightButton::switchLightOn(bool bOn)
{
    if (m_bLightOn == bOn)
        return;
    m_bLightOn = bOn;
    if (m_bLightOn)
        m_xButton->set_from_icon_name(RID_SVXBMP_LAMP_ON);
    else
        m_xButton->set_from_icon_name(RID_SVXBMP_LAMP_OFF);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
