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
#include <svtools/colrdlg.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/colorbox.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/globl3d.hxx>
#include <svx/view3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <editeng/eeitem.hxx>
#include <svl/style.hxx>
#include <svx/dlgutil.hxx>

#include <svx/dialmgr.hxx>
#include <svx/viewpt3d.hxx>

#include <svx/svxids.hrc>
#include <svx/strings.hrc>

#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/gallery.hxx>
#include <svl/whiter.hxx>

#include <svx/float3d.hxx>
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
    : SfxDockingWindow (pInBindings, pCW, pParent,
        "Docking3DEffects", "svx/ui/docking3deffects.ui")
    , aImgLightOn(StockImage::Yes, RID_SVXBMP_LAMP_ON)
    , aImgLightOff(StockImage::Yes, RID_SVXBMP_LAMP_OFF)
    , bUpdate(false)
    , eViewType(ViewType3D::Geo)
    , pBindings(pInBindings)
    , mpImpl(new Svx3DWinImpl)
    , ePoolUnit(MapUnit::MapMM)
{
    get(m_pBtnGeo, "geometry");
    get(m_pBtnRepresentation, "representation");
    get(m_pBtnLight, "light");
    get(m_pBtnTexture, "texture");
    get(m_pBtnMaterial, "material");
    get(m_pBtnUpdate, "update");
    get(m_pBtnAssign, "assign");

    get(m_pFLGeometrie, "geoframe");
    get(m_pFtPercentDiagonal, "diagonalft");
    get(m_pMtrPercentDiagonal, "diagonal");
    get(m_pFtBackscale, "scaleddepthft");
    get(m_pMtrBackscale, "scaleddepth");
    get(m_pFtEndAngle, "angleft");
    get(m_pMtrEndAngle, "angle");
    get(m_pFtDepth, "depthft");
    get(m_pMtrDepth, "depth");

    get(m_pFLSegments, "segmentsframe");
    get(m_pNumHorizontal, "hori");
    get(m_pNumVertical, "veri");

    get(m_pFLNormals, "normals");
    get(m_pBtnNormalsObj, "objspecific");
    get(m_pBtnNormalsFlat, "flat");
    get(m_pBtnNormalsSphere, "spherical");
    get(m_pBtnNormalsInvert, "invertnormals");
    get(m_pBtnTwoSidedLighting, "doublesidedillum");
    get(m_pBtnDoubleSided, "doublesided");

    get(m_pFLRepresentation, "shadingframe");
    get(m_pLbShademode, "mode");

    get(m_pFLShadow, "shadowframe");
    get(m_pBtnShadow3d, "shadow");
    get(m_pFtSlant, "slantft");
    get(m_pMtrSlant, "slant");

    get(m_pFLCamera, "cameraframe");
    get(m_pMtrDistance, "distance");
    get(m_pMtrFocalLength, "focal");

    get(m_pFLLight, "illumframe");
    get(m_pBtnLight1, "light1");
    get(m_pBtnLight2, "light2");
    get(m_pBtnLight3, "light3");
    get(m_pBtnLight4, "light4");
    get(m_pBtnLight5, "light5");
    get(m_pBtnLight6, "light6");
    get(m_pBtnLight7, "light7");
    get(m_pBtnLight8, "light8");
    get(m_pLbLight1, "lightcolor1");
    get(m_pLbLight2, "lightcolor2");
    get(m_pLbLight3, "lightcolor3");
    get(m_pLbLight4, "lightcolor4");
    get(m_pLbLight5, "lightcolor5");
    get(m_pLbLight6, "lightcolor6");
    get(m_pLbLight7, "lightcolor7");
    get(m_pLbLight8, "lightcolor8");
    get(m_pBtnLightColor, "colorbutton1");
    get(m_pLbAmbientlight, "ambientcolor");
    get(m_pBtnAmbientColor, "colorbutton2");

    get(m_pFLTexture, "textureframe");
    get(m_pBtnTexLuminance, "textype");
    get(m_pBtnTexColor, "texcolor");
    get(m_pBtnTexReplace, "texreplace");
    get(m_pBtnTexModulate, "texmodulate");
    get(m_pBtnTexBlend, "texblend");
    get(m_pBtnTexObjectX, "texobjx");
    get(m_pBtnTexParallelX, "texparallelx");
    get(m_pBtnTexCircleX, "texcirclex");
    get(m_pBtnTexObjectY, "texobjy");
    get(m_pBtnTexParallelY, "texparallely");
    get(m_pBtnTexCircleY, "texcircley");
    get(m_pBtnTexFilter, "texfilter");

    get(m_pFLMaterial, "materialframe");
    get(m_pLbMatFavorites, "favorites");
    get(m_pLbMatColor, "objcolor");
    get(m_pBtnMatColor, "colorbutton3");
    get(m_pLbMatEmission, "illumcolor");
    get(m_pBtnEmissionColor, "colorbutton4");

    get(m_pFLMatSpecular, "specframe");
    get(m_pLbMatSpecular, "speccolor");
    get(m_pBtnSpecularColor, "colorbutton5");
    get(m_pMtrMatSpecularIntensity, "intensity");

    get(m_pCtlPreview, "preview");
    get(m_pCtlLightPreview, "lightpreview");
    Size aSize(LogicToPixel(Size(83, 76), MapMode(MapUnit::MapAppFont)));
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlLightPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    m_pCtlLightPreview->set_height_request(aSize.Height());

    get(m_pBtnConvertTo3D, "to3d");
    get(m_pBtnLatheObject, "tolathe");
    get(m_pBtnPerspective, "perspective");

    mpImpl->pPool = nullptr;

    // Set Metric
    eFUnit = pInBindings->GetDispatcher()->GetModule()->GetFieldUnit();

    m_pMtrDepth->SetUnit( eFUnit );
    m_pMtrDistance->SetUnit( eFUnit );
    m_pMtrFocalLength->SetUnit( eFUnit );

    pControllerItem.reset( new Svx3DCtrlItem(SID_3D_STATE, pBindings) );
    pConvertTo3DItem.reset( new SvxConvertTo3DItem(SID_CONVERT_TO_3D, pBindings) );
    pConvertTo3DLatheItem.reset( new SvxConvertTo3DItem(SID_CONVERT_TO_3D_LATHE_FAST, pBindings) );

    m_pBtnAssign->SetClickHdl( LINK( this, Svx3DWin, ClickAssignHdl ) );
    m_pBtnUpdate->SetClickHdl( LINK( this, Svx3DWin, ClickUpdateHdl ) );

    Link<Button*,void> aLink( LINK( this, Svx3DWin, ClickViewTypeHdl ) );
    m_pBtnGeo->SetClickHdl( aLink );
    m_pBtnRepresentation->SetClickHdl( aLink );
    m_pBtnLight->SetClickHdl( aLink );
    m_pBtnTexture->SetClickHdl( aLink );
    m_pBtnMaterial->SetClickHdl( aLink );

    aLink = LINK( this, Svx3DWin, ClickHdl );
    m_pBtnPerspective->SetClickHdl( aLink );
    m_pBtnConvertTo3D->SetClickHdl( aLink );
    m_pBtnLatheObject->SetClickHdl( aLink );

    // Geometry
    m_pBtnNormalsObj->SetClickHdl( aLink );
    m_pBtnNormalsFlat->SetClickHdl( aLink );
    m_pBtnNormalsSphere->SetClickHdl( aLink );
    m_pBtnTwoSidedLighting->SetClickHdl( aLink );
    m_pBtnNormalsInvert->SetClickHdl( aLink );
    m_pBtnDoubleSided->SetClickHdl( aLink );

    // Representation
    m_pBtnShadow3d->SetClickHdl( aLink );

    // Lighting
    m_pBtnLight1->SetClickHdl( aLink );
    m_pBtnLight2->SetClickHdl( aLink );
    m_pBtnLight3->SetClickHdl( aLink );
    m_pBtnLight4->SetClickHdl( aLink );
    m_pBtnLight5->SetClickHdl( aLink );
    m_pBtnLight6->SetClickHdl( aLink );
    m_pBtnLight7->SetClickHdl( aLink );
    m_pBtnLight8->SetClickHdl( aLink );

    // Textures
    m_pBtnTexLuminance->SetClickHdl( aLink );
    m_pBtnTexColor->SetClickHdl( aLink );
    m_pBtnTexReplace->SetClickHdl( aLink );
    m_pBtnTexModulate->SetClickHdl( aLink );
    m_pBtnTexParallelX->SetClickHdl( aLink );
    m_pBtnTexCircleX->SetClickHdl( aLink );
    m_pBtnTexObjectX->SetClickHdl( aLink );
    m_pBtnTexParallelY->SetClickHdl( aLink );
    m_pBtnTexCircleY->SetClickHdl( aLink );
    m_pBtnTexObjectY->SetClickHdl( aLink );
    m_pBtnTexFilter->SetClickHdl( aLink );

    // Material
    aLink = LINK( this, Svx3DWin, ClickColorHdl );
    m_pBtnLightColor->SetClickHdl( aLink );
    m_pBtnAmbientColor->SetClickHdl( aLink );
    m_pBtnMatColor->SetClickHdl( aLink );
    m_pBtnEmissionColor->SetClickHdl( aLink );
    m_pBtnSpecularColor->SetClickHdl( aLink );


    Link<ListBox&,void> aLink2 = LINK( this, Svx3DWin, SelectHdl );
    Link<SvxColorListBox&,void> aLink4 = LINK( this, Svx3DWin, SelectColorHdl );
    m_pLbMatFavorites->SetSelectHdl( aLink2 );
    m_pLbMatColor->SetSelectHdl( aLink4 );
    m_pLbMatEmission->SetSelectHdl( aLink4 );
    m_pLbMatSpecular->SetSelectHdl( aLink4 );
    m_pLbLight1->SetSelectHdl( aLink4 );
    m_pLbLight2->SetSelectHdl( aLink4 );
    m_pLbLight3->SetSelectHdl( aLink4 );
    m_pLbLight4->SetSelectHdl( aLink4 );
    m_pLbLight5->SetSelectHdl( aLink4 );
    m_pLbLight6->SetSelectHdl( aLink4 );
    m_pLbLight7->SetSelectHdl( aLink4 );
    m_pLbLight8->SetSelectHdl( aLink4 );
    m_pLbAmbientlight->SetSelectHdl( aLink4 );
    m_pLbShademode->SetSelectHdl( aLink2 );

    Link<Edit&,void> aLink3 = LINK( this, Svx3DWin, ModifyHdl );
    m_pMtrMatSpecularIntensity->SetModifyHdl( aLink3 );
    m_pNumHorizontal->SetModifyHdl( aLink3 );
    m_pNumVertical->SetModifyHdl( aLink3 );
    m_pMtrSlant->SetModifyHdl( aLink3 );

    // Preview callback
    m_pCtlLightPreview->SetUserSelectionChangeCallback(LINK( this, Svx3DWin, ChangeSelectionCallbackHdl ));

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

    m_pBtnGeo.clear();
    m_pBtnRepresentation.clear();
    m_pBtnLight.clear();
    m_pBtnTexture.clear();
    m_pBtnMaterial.clear();
    m_pBtnUpdate.clear();
    m_pBtnAssign.clear();
    m_pFLGeometrie.clear();
    m_pFtPercentDiagonal.clear();
    m_pMtrPercentDiagonal.clear();
    m_pFtBackscale.clear();
    m_pMtrBackscale.clear();
    m_pFtEndAngle.clear();
    m_pMtrEndAngle.clear();
    m_pFtDepth.clear();
    m_pMtrDepth.clear();
    m_pFLSegments.clear();
    m_pNumHorizontal.clear();
    m_pNumVertical.clear();
    m_pFLNormals.clear();
    m_pBtnNormalsObj.clear();
    m_pBtnNormalsFlat.clear();
    m_pBtnNormalsSphere.clear();
    m_pBtnNormalsInvert.clear();
    m_pBtnTwoSidedLighting.clear();
    m_pBtnDoubleSided.clear();
    m_pFLRepresentation.clear();
    m_pLbShademode.clear();
    m_pFLShadow.clear();
    m_pBtnShadow3d.clear();
    m_pFtSlant.clear();
    m_pMtrSlant.clear();
    m_pFLCamera.clear();
    m_pMtrDistance.clear();
    m_pMtrFocalLength.clear();
    m_pFLLight.clear();
    m_pBtnLight1.clear();
    m_pBtnLight2.clear();
    m_pBtnLight3.clear();
    m_pBtnLight4.clear();
    m_pBtnLight5.clear();
    m_pBtnLight6.clear();
    m_pBtnLight7.clear();
    m_pBtnLight8.clear();
    m_pLbLight1.clear();
    m_pLbLight2.clear();
    m_pLbLight3.clear();
    m_pLbLight4.clear();
    m_pLbLight5.clear();
    m_pLbLight6.clear();
    m_pLbLight7.clear();
    m_pLbLight8.clear();
    m_pBtnLightColor.clear();
    m_pLbAmbientlight.clear();
    m_pBtnAmbientColor.clear();
    m_pFLTexture.clear();
    m_pBtnTexLuminance.clear();
    m_pBtnTexColor.clear();
    m_pBtnTexReplace.clear();
    m_pBtnTexModulate.clear();
    m_pBtnTexBlend.clear();
    m_pBtnTexObjectX.clear();
    m_pBtnTexParallelX.clear();
    m_pBtnTexCircleX.clear();
    m_pBtnTexObjectY.clear();
    m_pBtnTexParallelY.clear();
    m_pBtnTexCircleY.clear();
    m_pBtnTexFilter.clear();
    m_pFLMaterial.clear();
    m_pLbMatFavorites.clear();
    m_pLbMatColor.clear();
    m_pBtnMatColor.clear();
    m_pLbMatEmission.clear();
    m_pBtnEmissionColor.clear();
    m_pFLMatSpecular.clear();
    m_pLbMatSpecular.clear();
    m_pBtnSpecularColor.clear();
    m_pMtrMatSpecularIntensity.clear();
    m_pCtlPreview.clear();
    m_pCtlLightPreview.clear();
    m_pBtnConvertTo3D.clear();
    m_pBtnLatheObject.clear();
    m_pBtnPerspective.clear();

    SfxDockingWindow::dispose();
}


void Svx3DWin::Construct()
{
    m_pBtnGeo->Check();
    Link<Button*,void> aLink( LINK( this, Svx3DWin, ClickViewTypeHdl ) );
    aLink.Call(m_pBtnGeo);
    m_pCtlLightPreview->Hide();
}


void Svx3DWin::Reset()
{
    // Various initializations, default is AllAttributes
    m_pLbShademode->SelectEntryPos( 0 );
    m_pMtrMatSpecularIntensity->SetValue( 50 );

    m_pBtnLight1->Check();
    ClickUpdateHdl( nullptr );

    // Select nothing, to avoid errors when selecting the first
    m_pCtlLightPreview->GetSvx3DLightControl().SelectLight(0);
    m_pCtlLightPreview->CheckSelection();
}

bool Svx3DWin::GetUILightState(const PushButton& rBtn) const
{
    return (rBtn.GetModeImage() == aImgLightOn);
}

void Svx3DWin::SetUILightState(PushButton& rBtn, bool bState)
{
    rBtn.SetModeImage( bState ? aImgLightOn : aImgLightOff );
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
            m_pFLSegments->Enable(!bCube);

            m_pFtPercentDiagonal->Enable( !bCube && !bSphere );
            m_pMtrPercentDiagonal->Enable( !bCube && !bSphere );
            m_pFtBackscale->Enable( !bCube && !bSphere );
            m_pMtrBackscale->Enable( !bCube && !bSphere );
            m_pFtDepth->Enable( !bCube && !bSphere );
            m_pMtrDepth->Enable( !bCube && !bSphere );
            if( bCube )
            {
                m_pNumHorizontal->SetEmptyFieldValue();
                m_pNumVertical->SetEmptyFieldValue();
            }
            if( bCube || bSphere )
            {
                m_pMtrPercentDiagonal->SetEmptyFieldValue();
                m_pMtrBackscale->SetEmptyFieldValue();
                m_pMtrDepth->SetEmptyFieldValue();
            }

            // There is a final angle only for Lathe objects.
            m_pFtEndAngle->Enable( !bExtrude && !bCube && !bSphere );
            m_pMtrEndAngle->Enable( !bExtrude && !bCube && !bSphere );
            if( bExtrude || bCube || bSphere )
                m_pMtrEndAngle->SetEmptyFieldValue();
        }
        else
        {
            // Geometry
            m_pNumHorizontal->SetEmptyFieldValue();
            m_pNumVertical->SetEmptyFieldValue();
            m_pFLSegments->Enable( false );
            m_pFtEndAngle->Enable( false );
            m_pMtrEndAngle->Enable( false );
            m_pMtrEndAngle->SetEmptyFieldValue();
            m_pFtDepth->Enable( false );
            m_pMtrDepth->Enable( false );
            m_pMtrDepth->SetEmptyFieldValue();

            // Representation
            m_pFLShadow->Enable(false);

            m_pMtrDistance->SetEmptyFieldValue();
            m_pMtrFocalLength->SetEmptyFieldValue();
            m_pFLCamera->Enable( false );

            //Lower Range
            m_pBtnConvertTo3D->Enable( false );
            m_pBtnLatheObject->Enable( false );
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

    m_pFLTexture->Enable(bBitmap);

    // Geometry
    // Number of segments (horizontal)
    if( m_pNumHorizontal->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_HORZ_SEGS);
        if(eState != SfxItemState::DONTCARE)
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_HORZ_SEGS).GetValue();
            if(nValue != static_cast<sal_uInt32>(m_pNumHorizontal->GetValue()))
            {
                m_pNumHorizontal->SetValue( nValue );
                bUpdate = true;
            }
            else if( m_pNumHorizontal->IsEmptyFieldValue() )
                m_pNumHorizontal->SetValue( nValue );
        }
        else
        {
            if( !m_pNumHorizontal->IsEmptyFieldValue() )
            {
                m_pNumHorizontal->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    //Number of segments (vertical)
    if( m_pNumVertical->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_VERT_SEGS);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_VERT_SEGS).GetValue();
            if( nValue != static_cast<sal_uInt32>(m_pNumVertical->GetValue()) )
            {
                m_pNumVertical->SetValue( nValue );
                bUpdate = true;
            }
            else if( m_pNumVertical->IsEmptyFieldValue() )
                m_pNumVertical->SetValue( nValue );
        }
        else
        {
            if( !m_pNumVertical->IsEmptyFieldValue() )
            {
                m_pNumVertical->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    // Depth
    if( m_pMtrDepth->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_DEPTH);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DOBJ_DEPTH).GetValue();
            sal_uInt32 nValue2 = GetCoreValue(*m_pMtrDepth, ePoolUnit);
            if( nValue != nValue2 )
            {
                if( eFUnit != m_pMtrDepth->GetUnit() )
                    SetFieldUnit(*m_pMtrDepth, eFUnit);

                SetMetricValue(*m_pMtrDepth, nValue, ePoolUnit);
                bUpdate = true;
            }
            else if( m_pMtrDepth->IsEmptyFieldValue() )
                m_pMtrDepth->SetValue( m_pMtrDepth->GetValue() );
        }
        else
        {
            if( !m_pMtrDepth->IsEmptyFieldValue() )
            {
                m_pMtrDepth->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    // Double walled / Double sided
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_DOUBLE_SIDED);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_DOUBLE_SIDED).GetValue();
        if( bValue != m_pBtnDoubleSided->IsChecked() )
        {
            m_pBtnDoubleSided->Check( bValue );
            bUpdate = true;
        }
        else if( m_pBtnDoubleSided->GetState() == TRISTATE_INDET )
            m_pBtnDoubleSided->Check( bValue );
    }
    else
    {
        if( m_pBtnDoubleSided->GetState() != TRISTATE_INDET )
        {
            m_pBtnDoubleSided->SetState( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Edge rounding
    if( m_pMtrPercentDiagonal->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_PERCENT_DIAGONAL);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL).GetValue();
            if( nValue != m_pMtrPercentDiagonal->GetValue() )
            {
                m_pMtrPercentDiagonal->SetValue( nValue );
                bUpdate = true;
            }
            else if( m_pMtrPercentDiagonal->IsEmptyFieldValue() )
                m_pMtrPercentDiagonal->SetValue( nValue );
        }
        else
        {
            if( !m_pMtrPercentDiagonal->IsEmptyFieldValue() )
            {
                m_pMtrPercentDiagonal->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    // Depth scaling
    if( m_pMtrBackscale->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_BACKSCALE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_BACKSCALE).GetValue();
            if( nValue != m_pMtrBackscale->GetValue() )
            {
                m_pMtrBackscale->SetValue( nValue );
                bUpdate = true;
            }
            else if( m_pMtrBackscale->IsEmptyFieldValue() )
                m_pMtrBackscale->SetValue( nValue );
        }
        else
        {
            if( !m_pMtrBackscale->IsEmptyFieldValue() )
            {
                m_pMtrBackscale->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    // End angle
    if( m_pMtrEndAngle->IsEnabled() )
    {
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_END_ANGLE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_Int32 nValue = rAttrs.Get(SDRATTR_3DOBJ_END_ANGLE).GetValue();
            if( nValue != m_pMtrEndAngle->GetValue() )
            {
                m_pMtrEndAngle->SetValue( nValue );
                bUpdate = true;
            }
        }
        else
        {
            if( !m_pMtrEndAngle->IsEmptyFieldValue() )
            {
                m_pMtrEndAngle->SetEmptyFieldValue();
                bUpdate = true;
            }
        }
    }

    // Normal type
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_NORMALS_KIND);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_NORMALS_KIND).GetValue();

        if( ( !m_pBtnNormalsObj->IsChecked() && nValue == 0 ) ||
            ( !m_pBtnNormalsFlat->IsChecked() && nValue == 1 ) ||
            ( !m_pBtnNormalsSphere->IsChecked() && nValue == 2 ) )
        {
            m_pBtnNormalsObj->Check( nValue == 0 );
            m_pBtnNormalsFlat->Check( nValue == 1 );
            m_pBtnNormalsSphere->Check( nValue == 2 );
            bUpdate = true;
        }
    }
    else
    {
        if( m_pBtnNormalsObj->IsChecked() ||
            m_pBtnNormalsFlat->IsChecked() ||
            m_pBtnNormalsSphere->IsChecked() )
        {
            m_pBtnNormalsObj->Check( false );
            m_pBtnNormalsFlat->Check( false );
            m_pBtnNormalsSphere->Check( false );
            bUpdate = true;
        }
    }

    // Normal inverted
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_NORMALS_INVERT);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_NORMALS_INVERT).GetValue();
        if( bValue != m_pBtnNormalsInvert->IsChecked() )
        {
            m_pBtnNormalsInvert->Check( bValue );
            bUpdate = true;
        }
        else if( m_pBtnNormalsInvert->GetState() == TRISTATE_INDET )
            m_pBtnNormalsInvert->Check( bValue );
    }
    else
    {
        if( m_pBtnNormalsInvert->GetState() != TRISTATE_INDET )
        {
            m_pBtnNormalsInvert->SetState( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // 2-sided lighting
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING).GetValue();
        if( bValue != m_pBtnTwoSidedLighting->IsChecked() )
        {
            m_pBtnTwoSidedLighting->Check( bValue );
            bUpdate = true;
        }
        else if( m_pBtnTwoSidedLighting->GetState() == TRISTATE_INDET )
            m_pBtnTwoSidedLighting->Check( bValue );
    }
    else
    {
        if( m_pBtnTwoSidedLighting->GetState() != TRISTATE_INDET )
        {
            m_pBtnTwoSidedLighting->SetState( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Representation
    // Shademode
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_SHADE_MODE);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DSCENE_SHADE_MODE).GetValue();
        if( nValue != m_pLbShademode->GetSelectedEntryPos() )
        {
            m_pLbShademode->SelectEntryPos( nValue );
            bUpdate = true;
        }
    }
    else
    {
        if( m_pLbShademode->GetSelectedEntryCount() != 0 )
        {
            m_pLbShademode->SetNoSelection();
            bUpdate = true;
        }
    }

    // 3D-Shadow
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_SHADOW_3D);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bValue = rAttrs.Get(SDRATTR_3DOBJ_SHADOW_3D).GetValue();
        if( bValue != m_pBtnShadow3d->IsChecked() )
        {
            m_pBtnShadow3d->Check( bValue );
            m_pFtSlant->Enable( bValue );
            m_pMtrSlant->Enable( bValue );
            bUpdate = true;
        }
        else if( m_pBtnShadow3d->GetState() == TRISTATE_INDET )
            m_pBtnShadow3d->Check( bValue );
    }
    else
    {
        if( m_pBtnShadow3d->GetState() != TRISTATE_INDET )
        {
            m_pBtnShadow3d->SetState( TRISTATE_INDET );
            bUpdate = true;
        }
    }

    // Inclination (Shadow)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_SHADOW_SLANT);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DSCENE_SHADOW_SLANT).GetValue();
        if( nValue != m_pMtrSlant->GetValue() )
        {
            m_pMtrSlant->SetValue( nValue );
            bUpdate = true;
        }
    }
    else
    {
        if( !m_pMtrSlant->IsEmptyFieldValue() )
        {
            m_pMtrSlant->SetEmptyFieldValue();
            bUpdate = true;
        }
    }

    // Distance
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_DISTANCE);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DSCENE_DISTANCE).GetValue();
        sal_uInt32 nValue2 = GetCoreValue(*m_pMtrDistance, ePoolUnit);
        if( nValue != nValue2 )
        {
            if( eFUnit != m_pMtrDistance->GetUnit() )
                SetFieldUnit(*m_pMtrDistance, eFUnit);

            SetMetricValue(*m_pMtrDistance, nValue, ePoolUnit);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_pMtrDepth->IsEmptyFieldValue() )
        {
            m_pMtrDepth->SetEmptyFieldValue();
            bUpdate = true;
        }
    }

    // Focal length
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_FOCAL_LENGTH);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt32 nValue = rAttrs.Get(SDRATTR_3DSCENE_FOCAL_LENGTH).GetValue();
        sal_uInt32 nValue2 = GetCoreValue(*m_pMtrFocalLength, ePoolUnit);
        if( nValue != nValue2 )
        {
            if( eFUnit != m_pMtrFocalLength->GetUnit() )
                SetFieldUnit(*m_pMtrFocalLength, eFUnit);

            SetMetricValue(*m_pMtrFocalLength, nValue, ePoolUnit);
            bUpdate = true;
        }
    }
    else
    {
        if( !m_pMtrFocalLength->IsEmptyFieldValue() )
        {
            m_pMtrFocalLength->SetEmptyFieldValue();
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
        SvxColorListBox* pLb = m_pLbLight1;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight1->IsNoSelection())
        {
            m_pLbLight1->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 1 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_1);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_1).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight1))
        {
            SetUILightState(*m_pBtnLight1, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight1->GetState() == TRISTATE_INDET )
            m_pBtnLight1->Check( m_pBtnLight1->IsChecked() );
    }
    else
    {
        if( m_pBtnLight1->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight1->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight2;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight2->IsNoSelection())
        {
            m_pLbLight2->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 2 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_2);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_2).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight2))
        {
            SetUILightState(*m_pBtnLight2, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight2->GetState() == TRISTATE_INDET )
            m_pBtnLight2->Check( m_pBtnLight2->IsChecked() );
    }
    else
    {
        if( m_pBtnLight2->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight2->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight3;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight3->IsNoSelection())
        {
            m_pLbLight3->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 3 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_3);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_3).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight3))
        {
            SetUILightState(*m_pBtnLight3, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight3->GetState() == TRISTATE_INDET )
            m_pBtnLight3->Check( m_pBtnLight3->IsChecked() );
    }
    else
    {
        if( m_pBtnLight3->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight3->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight4;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight4->IsNoSelection())
        {
            m_pLbLight4->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 4 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_4);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_4).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight4))
        {
            SetUILightState(*m_pBtnLight4, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight4->GetState() == TRISTATE_INDET )
            m_pBtnLight4->Check( m_pBtnLight4->IsChecked() );
    }
    else
    {
        if( m_pBtnLight4->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight4->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight5;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight5->IsNoSelection())
        {
            m_pLbLight5->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 5 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_5);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_5).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight5))
        {
            SetUILightState(*m_pBtnLight5, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight5->GetState() == TRISTATE_INDET )
            m_pBtnLight5->Check( m_pBtnLight5->IsChecked() );
    }
    else
    {
        if( m_pBtnLight5->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight5->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight6;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight6->IsNoSelection())
        {
            m_pLbLight6->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 6 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_6);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_6).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight6))
        {
            SetUILightState(*m_pBtnLight6, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight6->GetState() == TRISTATE_INDET )
            m_pBtnLight6->Check( m_pBtnLight6->IsChecked() );
    }
    else
    {
        if( m_pBtnLight6->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight6->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight7;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight7->IsNoSelection())
        {
            m_pLbLight7->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 7 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_7);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_7).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight7))
        {
            SetUILightState(*m_pBtnLight7 , bOn);
            bUpdate = true;
        }
        if( m_pBtnLight7->GetState() == TRISTATE_INDET )
            m_pBtnLight7->Check( m_pBtnLight7->IsChecked() );
    }
    else
    {
        if( m_pBtnLight7->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight7->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbLight8;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbLight8->IsNoSelection())
        {
            m_pLbLight8->SetNoSelection();
            bUpdate = true;
        }
    }
    // Light 8 (on/off)
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_LIGHTON_8);
    if( eState != SfxItemState::DONTCARE )
    {
        bool bOn = rAttrs.Get(SDRATTR_3DSCENE_LIGHTON_8).GetValue();
        if (bOn != GetUILightState(*m_pBtnLight8))
        {
            SetUILightState(*m_pBtnLight8, bOn);
            bUpdate = true;
        }
        if( m_pBtnLight8->GetState() == TRISTATE_INDET )
            m_pBtnLight8->Check( m_pBtnLight8->IsChecked() );
    }
    else
    {
        if( m_pBtnLight8->GetState() != TRISTATE_INDET )
        {
            m_pBtnLight8->SetState( TRISTATE_INDET );
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
        SvxColorListBox* pLb = m_pLbAmbientlight;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbAmbientlight->IsNoSelection())
        {
            m_pLbAmbientlight->SetNoSelection();
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

            if( ( !m_pBtnTexLuminance->IsChecked() && nValue == 1 ) ||
                ( !m_pBtnTexColor->IsChecked() && nValue == 3 ) )
            {
                m_pBtnTexLuminance->Check( nValue == 1 );
                m_pBtnTexColor->Check( nValue == 3 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_pBtnTexLuminance->IsChecked() ||
                m_pBtnTexColor->IsChecked() )
            {
                m_pBtnTexLuminance->Check( false );
                m_pBtnTexColor->Check( false );
                bUpdate = true;
            }
        }

        // Mode
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_MODE);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_MODE).GetValue();

            if( ( !m_pBtnTexReplace->IsChecked() && nValue == 1 ) ||
                ( !m_pBtnTexModulate->IsChecked() && nValue == 2 ) )
            {
                m_pBtnTexReplace->Check( nValue == 1 );
                m_pBtnTexModulate->Check( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_pBtnTexReplace->IsChecked() ||
                m_pBtnTexModulate->IsChecked() )
            {
                m_pBtnTexReplace->Check( false );
                m_pBtnTexModulate->Check( false );
                bUpdate = true;
            }
        }

        // Projection X
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_PROJ_X);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_X).GetValue();

            if( ( !m_pBtnTexObjectX->IsChecked() && nValue == 0 ) ||
                ( !m_pBtnTexParallelX->IsChecked() && nValue == 1 ) ||
                ( !m_pBtnTexCircleX->IsChecked() && nValue == 2 ) )
            {
                m_pBtnTexObjectX->Check( nValue == 0 );
                m_pBtnTexParallelX->Check( nValue == 1 );
                m_pBtnTexCircleX->Check( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_pBtnTexObjectX->IsChecked() ||
                m_pBtnTexParallelX->IsChecked() ||
                m_pBtnTexCircleX->IsChecked() )
            {
                m_pBtnTexObjectX->Check( false );
                m_pBtnTexParallelX->Check( false );
                m_pBtnTexCircleX->Check( false );
                bUpdate = true;
            }
        }

        // Projection Y
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_PROJ_Y);
        if( eState != SfxItemState::DONTCARE )
        {
            sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_PROJ_Y).GetValue();

            if( ( !m_pBtnTexObjectY->IsChecked() && nValue == 0 ) ||
                ( !m_pBtnTexParallelY->IsChecked() && nValue == 1 ) ||
                ( !m_pBtnTexCircleY->IsChecked() && nValue == 2 ) )
            {
                m_pBtnTexObjectY->Check( nValue == 0 );
                m_pBtnTexParallelY->Check( nValue == 1 );
                m_pBtnTexCircleY->Check( nValue == 2 );
                bUpdate = true;
            }
        }
        else
        {
            if( m_pBtnTexObjectY->IsChecked() ||
                m_pBtnTexParallelY->IsChecked() ||
                m_pBtnTexCircleY->IsChecked() )
            {
                m_pBtnTexObjectY->Check( false );
                m_pBtnTexParallelY->Check( false );
                m_pBtnTexCircleY->Check( false );
                bUpdate = true;
            }
        }

        // Filter
        eState = rAttrs.GetItemState(SDRATTR_3DOBJ_TEXTURE_FILTER);
        if( eState != SfxItemState::DONTCARE )
        {
            bool bValue = rAttrs.Get(SDRATTR_3DOBJ_TEXTURE_FILTER).GetValue();
            if( bValue != m_pBtnTexFilter->IsChecked() )
            {
                m_pBtnTexFilter->Check( bValue );
                bUpdate = true;
            }
            if( m_pBtnTexFilter->GetState() == TRISTATE_INDET )
                m_pBtnTexFilter->Check( bValue );
        }
        else
        {
            if( m_pBtnTexFilter->GetState() != TRISTATE_INDET )
            {
                m_pBtnTexFilter->SetState( TRISTATE_INDET );
                bUpdate = true;
            }
        }
    }


    // Material Favorites
    m_pLbMatFavorites->SelectEntryPos( 0 );

    // Object color
    eState = rAttrs.GetItemState(XATTR_FILLCOLOR);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(XATTR_FILLCOLOR).GetColorValue();
        SvxColorListBox* pLb = m_pLbMatColor;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbMatColor->IsNoSelection())
        {
            m_pLbMatColor->SetNoSelection();
            bUpdate = true;
        }
    }

    // Self-luminous color
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_EMISSION);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DOBJ_MAT_EMISSION).GetValue();
        SvxColorListBox* pLb = m_pLbMatEmission;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbMatEmission->IsNoSelection())
        {
            m_pLbMatEmission->SetNoSelection();
            bUpdate = true;
        }
    }

    // Specular
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_SPECULAR);
    if( eState != SfxItemState::DONTCARE )
    {
        aColor = rAttrs.Get(SDRATTR_3DOBJ_MAT_SPECULAR).GetValue();
        SvxColorListBox* pLb = m_pLbMatSpecular;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = true;
        }
    }
    else
    {
        if (!m_pLbMatSpecular->IsNoSelection())
        {
            m_pLbMatSpecular->SetNoSelection();
            bUpdate = true;
        }
    }

    // Specular Intensity
    eState = rAttrs.GetItemState(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY);
    if( eState != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = rAttrs.Get(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY).GetValue();
        if( nValue != m_pMtrMatSpecularIntensity->GetValue() )
        {
            m_pMtrMatSpecularIntensity->SetValue( nValue );
            bUpdate = true;
        }
    }
    else
    {
        if( !m_pMtrMatSpecularIntensity->IsEmptyFieldValue() )
        {
            m_pMtrMatSpecularIntensity->SetEmptyFieldValue();
            bUpdate = true;
        }
    }


// Other
    // Perspective
    eState = rAttrs.GetItemState(SDRATTR_3DSCENE_PERSPECTIVE);
    if( eState != SfxItemState::DONTCARE )
    {
        ProjectionType ePT = static_cast<ProjectionType>(rAttrs.Get(SDRATTR_3DSCENE_PERSPECTIVE).GetValue());
        if( ( !m_pBtnPerspective->IsChecked() && ePT == ProjectionType::Perspective ) ||
            ( m_pBtnPerspective->IsChecked() && ePT == ProjectionType::Parallel ) )
        {
            m_pBtnPerspective->Check( ePT == ProjectionType::Perspective );
            bUpdate = true;
        }
        if( m_pBtnPerspective->GetState() == TRISTATE_INDET )
            m_pBtnPerspective->Check( ePT == ProjectionType::Perspective );
    }
    else
    {
        if( m_pBtnPerspective->GetState() != TRISTATE_INDET )
        {
            m_pBtnPerspective->SetState( TRISTATE_INDET );
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

    m_pCtlPreview->Set3DAttributes(aSet);
    m_pCtlLightPreview->GetSvx3DLightControl().Set3DAttributes(aSet);

    // try to select light corresponding to active button
    sal_uInt32 nNumber(0xffffffff);

    if(m_pBtnLight1->IsChecked())
        nNumber = 0;
    else if(m_pBtnLight2->IsChecked())
        nNumber = 1;
    else if(m_pBtnLight3->IsChecked())
        nNumber = 2;
    else if(m_pBtnLight4->IsChecked())
        nNumber = 3;
    else if(m_pBtnLight5->IsChecked())
        nNumber = 4;
    else if(m_pBtnLight6->IsChecked())
        nNumber = 5;
    else if(m_pBtnLight7->IsChecked())
        nNumber = 6;
    else if(m_pBtnLight8->IsChecked())
        nNumber = 7;

    if(nNumber != 0xffffffff)
    {
        m_pCtlLightPreview->GetSvx3DLightControl().SelectLight(nNumber);
    }

    // handle state of converts possible
    m_pBtnConvertTo3D->Enable(pConvertTo3DItem->GetState());
    m_pBtnLatheObject->Enable(pConvertTo3DLatheItem->GetState());
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
    if( m_pBtnPerspective->GetState() != TRISTATE_INDET )
    {
        ProjectionType nValue;
        if( m_pBtnPerspective->IsChecked() )
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
    if( !m_pNumHorizontal->IsEmptyFieldValue() )
    {
        sal_uInt32 nValue = static_cast<sal_uInt32>(m_pNumHorizontal->GetValue());
        rAttrs.Put(makeSvx3DHorizontalSegmentsItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_HORZ_SEGS);

    //  Number of segments (vertical)
    if( !m_pNumVertical->IsEmptyFieldValue() )
    {
        sal_uInt32 nValue = static_cast<sal_uInt32>(m_pNumVertical->GetValue());
        rAttrs.Put(makeSvx3DVerticalSegmentsItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_VERT_SEGS);

    // Depth
    if( !m_pMtrDepth->IsEmptyFieldValue() )
    {
        sal_uInt32 nValue = GetCoreValue(*m_pMtrDepth, ePoolUnit);
        rAttrs.Put(makeSvx3DDepthItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_DEPTH);

    // Double-sided
    TriState eState = m_pBtnDoubleSided->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DDoubleSidedItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_DOUBLE_SIDED);

    // Edge rounding
    if( !m_pMtrPercentDiagonal->IsEmptyFieldValue() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_pMtrPercentDiagonal->GetValue());
        rAttrs.Put(makeSvx3DPercentDiagonalItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_PERCENT_DIAGONAL);

    // Depth scale
    if( !m_pMtrBackscale->IsEmptyFieldValue() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_pMtrBackscale->GetValue());
        rAttrs.Put(makeSvx3DBackscaleItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_BACKSCALE);

    // End angle
    if( !m_pMtrEndAngle->IsEmptyFieldValue() )
    {
        sal_uInt16 nValue = static_cast<sal_uInt16>(m_pMtrEndAngle->GetValue());
        rAttrs.Put(makeSvx3DEndAngleItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_END_ANGLE);

    // Normal type
    sal_uInt16 nValue = 99;
    if( m_pBtnNormalsObj->IsChecked() )
        nValue = 0;
    else if( m_pBtnNormalsFlat->IsChecked() )
        nValue = 1;
    else if( m_pBtnNormalsSphere->IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DNormalsKindItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_NORMALS_KIND);

    // Normal inverted
    eState = m_pBtnNormalsInvert->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DNormalsInvertItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_NORMALS_INVERT);

    // 2-sided lighting
    eState = m_pBtnTwoSidedLighting->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DTwoSidedLightingItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING);

// Representation
    // Shade mode
    if( m_pLbShademode->GetSelectedEntryCount() )
    {
        nValue = m_pLbShademode->GetSelectedEntryPos();
        rAttrs.Put(Svx3DShadeModeItem(nValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_SHADE_MODE);

    // 3D-Shadow
    eState = m_pBtnShadow3d->GetState();
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
    if( !m_pMtrSlant->IsEmptyFieldValue() )
    {
        sal_uInt16 nValue2 = static_cast<sal_uInt16>(m_pMtrSlant->GetValue());
        rAttrs.Put(makeSvx3DShadowSlantItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_SHADOW_SLANT);

    // Distance
    if( !m_pMtrDistance->IsEmptyFieldValue() )
    {
        sal_uInt32 nValue2 = GetCoreValue(*m_pMtrDistance, ePoolUnit);
        rAttrs.Put(makeSvx3DDistanceItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_DISTANCE);

    // Focal length
    if( !m_pMtrFocalLength->IsEmptyFieldValue() )
    {
        sal_uInt32 nValue2 = GetCoreValue(*m_pMtrFocalLength, ePoolUnit);
        rAttrs.Put(makeSvx3DFocalLengthItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_FOCAL_LENGTH);

    // Lighting
    Color aColor;
    const SfxItemSet aLightItemSet(m_pCtlLightPreview->GetSvx3DLightControl().Get3DAttributes());

    // Light 1 color
    if (!m_pLbLight1->IsNoSelection())
    {
        aColor = m_pLbLight1->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor1Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_1);
    // Light 1 (on/off)
    eState = m_pBtnLight1->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight1);
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
    if (!m_pLbLight2->IsNoSelection())
    {
        aColor = m_pLbLight2->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor2Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_2);
    // Light 2 (on/off)
    eState = m_pBtnLight2->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight2);
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
    if (!m_pLbLight3->IsNoSelection())
    {
        aColor = m_pLbLight3->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor3Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_3);
    // Light 3 (on/off)
    eState = m_pBtnLight3->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight3);
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
    if (!m_pLbLight4->IsNoSelection())
    {
        aColor = m_pLbLight4->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor4Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_4);
    // Light 4 (on/off)
    eState = m_pBtnLight4->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight4);
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
    if (!m_pLbLight5->IsNoSelection())
    {
        aColor = m_pLbLight5->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor5Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_5);
    // Light 5 (on/off)
    eState = m_pBtnLight5->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight5);
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
    if (!m_pLbLight6->IsNoSelection())
    {
        aColor = m_pLbLight6->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor6Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_6);
    // Light 6 (on/off)
    eState = m_pBtnLight6->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight6);
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
    if (!m_pLbLight7->IsNoSelection())
    {
        aColor = m_pLbLight7->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor7Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_7);
    // Light 7 (on/off)
    eState = m_pBtnLight7->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight7);
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
    if (!m_pLbLight8->IsNoSelection())
    {
        aColor = m_pLbLight8->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DLightcolor8Item(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_LIGHTCOLOR_8);
    // Light 8 (on/off)
    eState = m_pBtnLight8->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = GetUILightState(*m_pBtnLight8);
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
    if (!m_pLbAmbientlight->IsNoSelection())
    {
        aColor = m_pLbAmbientlight->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DAmbientcolorItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DSCENE_AMBIENTCOLOR);

// Textures
    // Art
    nValue = 99;
    if( m_pBtnTexLuminance->IsChecked() )
        nValue = 1;
    else if( m_pBtnTexColor->IsChecked() )
        nValue = 3;

    if( nValue == 1 || nValue == 3 )
        rAttrs.Put(Svx3DTextureKindItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_KIND);


    // Mode
    nValue = 99;
    if( m_pBtnTexReplace->IsChecked() )
        nValue = 1;
    else if( m_pBtnTexModulate->IsChecked() )
        nValue = 2;

    if( nValue == 1 || nValue == 2 )
        rAttrs.Put(Svx3DTextureModeItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_MODE);

    // X projection
    nValue = 99;
    if( m_pBtnTexObjectX->IsChecked() )
        nValue = 0;
    else if( m_pBtnTexParallelX->IsChecked() )
        nValue = 1;
    else if( m_pBtnTexCircleX->IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DTextureProjectionXItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_PROJ_X);

    // Y projection
    nValue = 99;
    if( m_pBtnTexObjectY->IsChecked() )
        nValue = 0;
    else if( m_pBtnTexParallelY->IsChecked() )
        nValue = 1;
    else if( m_pBtnTexCircleY->IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put(Svx3DTextureProjectionYItem(nValue));
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_PROJ_Y);


    // Filter
    eState = m_pBtnTexFilter->GetState();
    if( eState != TRISTATE_INDET )
    {
        bool bValue = TRISTATE_TRUE == eState;
        rAttrs.Put(makeSvx3DTextureFilterItem(bValue));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_TEXTURE_FILTER);


// Material
    // Object color
    if (!m_pLbMatColor->IsNoSelection())
    {
        aColor = m_pLbMatColor->GetSelectEntryColor();
        rAttrs.Put( XFillColorItem( "", aColor) );
    }
    else
    {
        rAttrs.InvalidateItem( XATTR_FILLCOLOR );
    }

    // luminous color
    if (!m_pLbMatEmission->IsNoSelection())
    {
        aColor = m_pLbMatEmission->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DMaterialEmissionItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_EMISSION);

    // Specular
    if (!m_pLbMatSpecular->IsNoSelection())
    {
        aColor = m_pLbMatSpecular->GetSelectEntryColor();
        rAttrs.Put(makeSvx3DMaterialSpecularItem(aColor));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_SPECULAR);

    // Specular intensity
    if( !m_pMtrMatSpecularIntensity->IsEmptyFieldValue() )
    {
        sal_uInt16 nValue2 = static_cast<sal_uInt16>(m_pMtrMatSpecularIntensity->GetValue());
        rAttrs.Put(makeSvx3DMaterialSpecularIntensityItem(nValue2));
    }
    else
        rAttrs.InvalidateItem(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY);
}

void Svx3DWin::Resize()
{
    if ( !IsFloatingMode() ||
         !GetFloatingWindow()->IsRollUp() )
    {
        Size aWinSize( GetOutputSizePixel() ); // why rSize in Resizing()?

        if( aWinSize.Height() >= GetMinOutputSizePixel().Height() &&
            aWinSize.Width() >= GetMinOutputSizePixel().Width() )
        {
            // Hide
            m_pBtnUpdate->Hide();
            m_pBtnAssign->Hide();

            m_pBtnConvertTo3D->Hide();
            m_pBtnLatheObject->Hide();
            m_pBtnPerspective->Hide();

            m_pCtlPreview->Hide();
            m_pCtlLightPreview->Hide();

            m_pFLGeometrie->Hide();
            m_pFLRepresentation->Hide();
            m_pFLLight->Hide();
            m_pFLTexture->Hide();
            m_pFLMaterial->Hide();

            // Show
            m_pBtnUpdate->Show();
            m_pBtnAssign->Show();

            m_pBtnConvertTo3D->Show();
            m_pBtnLatheObject->Show();
            m_pBtnPerspective->Show();

            if( m_pBtnGeo->IsChecked() )
                ClickViewTypeHdl(m_pBtnGeo);
            if( m_pBtnRepresentation->IsChecked() )
                ClickViewTypeHdl(m_pBtnRepresentation);
            if( m_pBtnLight->IsChecked() )
                ClickViewTypeHdl(m_pBtnLight);
            if( m_pBtnTexture->IsChecked() )
                ClickViewTypeHdl(m_pBtnTexture);
            if( m_pBtnMaterial->IsChecked() )
                ClickViewTypeHdl(m_pBtnMaterial);
        }
    }

    SfxDockingWindow::Resize();
}

IMPL_LINK_NOARG(Svx3DWin, ClickUpdateHdl, Button*, void)
{
    bUpdate = !m_pBtnUpdate->IsChecked();
    m_pBtnUpdate->Check( bUpdate );

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


IMPL_LINK_NOARG(Svx3DWin, ClickAssignHdl, Button*, void)
{
    SfxDispatcher* pDispatcher = LocalGetDispatcher(pBindings);
    if (pDispatcher != nullptr)
    {
        SfxBoolItem aItem( SID_3D_ASSIGN, true );
        pDispatcher->ExecuteList(SID_3D_ASSIGN,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });
    }
}


IMPL_LINK( Svx3DWin, ClickViewTypeHdl, Button*, pBtn, void )
{

    if( pBtn )
    {
        // Since the permanent updating of the preview would be too expensive
        bool bUpdatePreview = m_pBtnLight->IsChecked();

        m_pBtnGeo->Check(m_pBtnGeo == pBtn);
        m_pBtnRepresentation->Check(m_pBtnRepresentation == pBtn);
        m_pBtnLight->Check(m_pBtnLight == pBtn);
        m_pBtnTexture->Check(m_pBtnTexture == pBtn);
        m_pBtnMaterial->Check(m_pBtnMaterial == pBtn);

        if( m_pBtnGeo->IsChecked() )
            eViewType = ViewType3D::Geo;
        if( m_pBtnRepresentation->IsChecked() )
            eViewType = ViewType3D::Representation;
        if( m_pBtnLight->IsChecked() )
            eViewType = ViewType3D::Light;
        if( m_pBtnTexture->IsChecked() )
            eViewType = ViewType3D::Texture;
        if( m_pBtnMaterial->IsChecked() )
            eViewType = ViewType3D::Material;

        // Geometry
        if( eViewType == ViewType3D::Geo )
        {
            m_pFLSegments->Show();
            m_pFLGeometrie->Show();
            m_pFLNormals->Show();
        }
        else
        {
            m_pFLSegments->Hide();
            m_pFLGeometrie->Hide();
            m_pFLNormals->Hide();
        }

        // Representation
        if( eViewType == ViewType3D::Representation )
        {
            m_pFLShadow->Show();
            m_pFLCamera->Show();
            m_pFLRepresentation->Show();
        }
        else
        {
            m_pFLShadow->Hide();
            m_pFLCamera->Hide();
            m_pFLRepresentation->Hide();
        }

        // Lighting
        if( eViewType == ViewType3D::Light )
        {
            m_pFLLight->Show();

            SvxColorListBox* pLb = GetLbByButton();
            if( pLb )
                pLb->Show();

            m_pCtlLightPreview->Show();
            m_pCtlPreview->Hide();
        }
        else
        {
            m_pFLLight->Hide();

            if( !m_pCtlPreview->IsVisible() )
            {
                m_pCtlPreview->Show();
                m_pCtlLightPreview->Hide();
            }
        }

        // Textures
        if (eViewType == ViewType3D::Texture)
            m_pFLTexture->Show();
        else
            m_pFLTexture->Hide();

        // Material
        if( eViewType == ViewType3D::Material )
        {
            m_pFLMatSpecular->Show();
            m_pFLMaterial->Show();
        }
        else
        {
            m_pFLMatSpecular->Hide();
            m_pFLMaterial->Hide();
        }
        if( bUpdatePreview && !m_pBtnLight->IsChecked() )
            UpdatePreview();

    }
    else
    {
        m_pBtnGeo->Check( eViewType == ViewType3D::Geo );
        m_pBtnRepresentation->Check( eViewType == ViewType3D::Representation );
        m_pBtnLight->Check( eViewType == ViewType3D::Light );
        m_pBtnTexture->Check( eViewType == ViewType3D::Texture );
        m_pBtnMaterial->Check( eViewType == ViewType3D::Material );
     }
}


IMPL_LINK( Svx3DWin, ClickHdl, Button *, pButton, void )
{
    PushButton* pBtn = static_cast<PushButton*>(pButton);
    if( pBtn )
    {
        bool bUpdatePreview = false;
        sal_uInt16 nSId = 0;

        if( pBtn == m_pBtnConvertTo3D )
        {
            nSId = SID_CONVERT_TO_3D;
        }
        else if( pBtn == m_pBtnLatheObject )
        {
            nSId = SID_CONVERT_TO_3D_LATHE_FAST;
        }
        // Geometry
        else if( pBtn == m_pBtnNormalsObj ||
                 pBtn == m_pBtnNormalsFlat ||
                 pBtn == m_pBtnNormalsSphere )
        {
            m_pBtnNormalsObj->Check( pBtn == m_pBtnNormalsObj );
            m_pBtnNormalsFlat->Check( pBtn == m_pBtnNormalsFlat );
            m_pBtnNormalsSphere->Check( pBtn == m_pBtnNormalsSphere );
            bUpdatePreview = true;
        }
        else if( pBtn == m_pBtnLight1 ||
                 pBtn == m_pBtnLight2 ||
                 pBtn == m_pBtnLight3 ||
                 pBtn == m_pBtnLight4 ||
                 pBtn == m_pBtnLight5 ||
                 pBtn == m_pBtnLight6 ||
                 pBtn == m_pBtnLight7 ||
                 pBtn == m_pBtnLight8 )
        {
            // Lighting
            SvxColorListBox* pLb = GetLbByButton( pBtn );
            pLb->Show();

            if( pBtn->IsChecked() )
            {
                SetUILightState( *static_cast<ImageButton*>(pBtn), !GetUILightState( *static_cast<ImageButton*>(pBtn) ) );
            }
            else
            {
                pBtn->Check();

                if (pBtn != m_pBtnLight1 && m_pBtnLight1->IsChecked())
                {
                    m_pBtnLight1->Check( false );
                    m_pLbLight1->Hide();
                }
                if (pBtn != m_pBtnLight2 && m_pBtnLight2->IsChecked())
                {
                    m_pBtnLight2->Check( false );
                    m_pLbLight2->Hide();
                }
                if( pBtn != m_pBtnLight3 && m_pBtnLight3->IsChecked() )
                {
                    m_pBtnLight3->Check( false );
                    m_pLbLight3->Hide();
                }
                if( pBtn != m_pBtnLight4 && m_pBtnLight4->IsChecked() )
                {
                    m_pBtnLight4->Check( false );
                    m_pLbLight4->Hide();
                }
                if( pBtn != m_pBtnLight5 && m_pBtnLight5->IsChecked() )
                {
                    m_pBtnLight5->Check( false );
                    m_pLbLight5->Hide();
                }
                if( pBtn != m_pBtnLight6 && m_pBtnLight6->IsChecked() )
                {
                    m_pBtnLight6->Check( false );
                    m_pLbLight6->Hide();
                }
                if( pBtn != m_pBtnLight7 && m_pBtnLight7->IsChecked() )
                {
                    m_pBtnLight7->Check( false );
                    m_pLbLight7->Hide();
                }
                if( pBtn != m_pBtnLight8 && m_pBtnLight8->IsChecked() )
                {
                    m_pBtnLight8->Check( false );
                    m_pLbLight8->Hide();
                }
            }
            bool bEnable = GetUILightState( *static_cast<ImageButton*>(pBtn) );
            m_pBtnLightColor->Enable( bEnable );
            pLb->Enable( bEnable );

            ClickLight(*pBtn);
            bUpdatePreview = true;
        }
        // Textures
        else if( pBtn == m_pBtnTexLuminance ||
                 pBtn == m_pBtnTexColor )
        {
            m_pBtnTexLuminance->Check( pBtn == m_pBtnTexLuminance );
            m_pBtnTexColor->Check( pBtn == m_pBtnTexColor );
            bUpdatePreview = true;
        }
        else if( pBtn == m_pBtnTexReplace ||
                 pBtn == m_pBtnTexModulate )
        {
            m_pBtnTexReplace->Check( pBtn == m_pBtnTexReplace );
            m_pBtnTexModulate->Check( pBtn == m_pBtnTexModulate );
            bUpdatePreview = true;
        }
        else if( pBtn == m_pBtnTexParallelX ||
                 pBtn == m_pBtnTexCircleX ||
                 pBtn == m_pBtnTexObjectX )
        {
            m_pBtnTexParallelX->Check( pBtn == m_pBtnTexParallelX );
            m_pBtnTexCircleX->Check( pBtn == m_pBtnTexCircleX );
            m_pBtnTexObjectX->Check( pBtn == m_pBtnTexObjectX );
            bUpdatePreview = true;
        }
        else if( pBtn == m_pBtnTexParallelY ||
                 pBtn == m_pBtnTexCircleY ||
                 pBtn == m_pBtnTexObjectY )
        {
            m_pBtnTexParallelY->Check( pBtn == m_pBtnTexParallelY );
            m_pBtnTexCircleY->Check( pBtn == m_pBtnTexCircleY );
            m_pBtnTexObjectY->Check( pBtn == m_pBtnTexObjectY );
            bUpdatePreview = true;
        }
        else if (pBtn == m_pBtnShadow3d)
        {
            pBtn->Check( !pBtn->IsChecked() );
            m_pFtSlant->Enable( pBtn->IsChecked() );
            m_pMtrSlant->Enable( pBtn->IsChecked() );
            bUpdatePreview = true;
        }
        // Other (no groups)
        else if( pBtn != nullptr )
        {
            pBtn->Check( !pBtn->IsChecked() );
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
}

IMPL_LINK( Svx3DWin, ClickColorHdl, Button *, pBtn, void)
{
    SvColorDialog aColorDlg;
    SvxColorListBox* pLb;

    if( pBtn == m_pBtnLightColor )
        pLb = GetLbByButton();
    else if( pBtn == m_pBtnAmbientColor )
        pLb = m_pLbAmbientlight;
    else if( pBtn == m_pBtnMatColor )
        pLb = m_pLbMatColor;
    else if( pBtn == m_pBtnEmissionColor )
        pLb = m_pLbMatEmission;
    else // if( pBtn == m_pBtnSpecularColor )
        pLb = m_pLbMatSpecular;

    Color aColor = pLb->GetSelectEntryColor();

    aColorDlg.SetColor( aColor );
    if( aColorDlg.Execute(GetFrameWeld()) == RET_OK )
    {
        aColor = aColorDlg.GetColor();
        LBSelectColor(pLb, aColor);
        SelectColorHdl(*pLb);
    }
}

IMPL_LINK( Svx3DWin, SelectHdl, ListBox&, rListBox, void )
{
    bool bUpdatePreview = false;

    // Material
    if (&rListBox == m_pLbMatFavorites)
    {
        Color aColObj( COL_WHITE );
        Color aColEmis( COL_BLACK );
        Color aColSpec( COL_WHITE );
        sal_uInt16 nSpecIntens = 20;

        switch( m_pLbMatFavorites->GetSelectedEntryPos() )
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
        LBSelectColor( m_pLbMatColor, aColObj );
        LBSelectColor( m_pLbMatEmission, aColEmis );
        LBSelectColor( m_pLbMatSpecular, aColSpec );
        m_pMtrMatSpecularIntensity->SetValue( nSpecIntens );

        bUpdatePreview = true;
    }
    else if (&rListBox == m_pLbShademode)
        bUpdatePreview = true;

    if( bUpdatePreview )
        UpdatePreview();
}

IMPL_LINK( Svx3DWin, SelectColorHdl, SvxColorListBox&, rListBox, void )
{
    bool bUpdatePreview = false;

    if( &rListBox == m_pLbMatColor ||
        &rListBox == m_pLbMatEmission ||
        &rListBox == m_pLbMatSpecular )
    {
        m_pLbMatFavorites->SelectEntryPos( 0 );
        bUpdatePreview = true;
    }
    // Lighting
    else if( &rListBox == m_pLbAmbientlight )
    {
        bUpdatePreview = true;
    }
    else if( &rListBox == m_pLbLight1 ||
             &rListBox == m_pLbLight2 ||
             &rListBox == m_pLbLight3 ||
             &rListBox == m_pLbLight4 ||
             &rListBox == m_pLbLight5 ||
             &rListBox == m_pLbLight6 ||
             &rListBox == m_pLbLight7 ||
             &rListBox == m_pLbLight8 )
    {
        bUpdatePreview = true;
    }

    if( bUpdatePreview )
        UpdatePreview();
}


IMPL_LINK( Svx3DWin, ModifyHdl, Edit&, rField, void )
{
    bool bUpdatePreview = false;

    // Material
    if( &rField == m_pMtrMatSpecularIntensity )
    {
        bUpdatePreview = true;
    }
    else if (&rField == m_pNumHorizontal)
    {
        bUpdatePreview = true;
    }
    else if (&rField == m_pNumVertical)
    {
        bUpdatePreview = true;
    }
    else if (&rField == m_pMtrSlant)
    {
        bUpdatePreview = true;
    }

    if( bUpdatePreview )
        UpdatePreview();
}


void Svx3DWin::ClickLight(PushButton& rBtn)
{
    sal_uInt16 nLightSource = GetLightSource( &rBtn );
    SvxColorListBox* pLb = GetLbByButton( &rBtn );
    Color aColor( pLb->GetSelectEntryColor() );
    SfxItemSet aLightItemSet(m_pCtlLightPreview->GetSvx3DLightControl().Get3DAttributes());
    const bool bOnOff(GetUILightState( static_cast<const ImageButton&>(rBtn) ));

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

    m_pCtlLightPreview->GetSvx3DLightControl().Set3DAttributes(aLightItemSet);
    m_pCtlLightPreview->GetSvx3DLightControl().SelectLight(nLightSource);
    m_pCtlLightPreview->CheckSelection();
}


IMPL_LINK_NOARG(Svx3DWin, ChangeSelectionCallbackHdl, SvxLightCtl3D*, void)
{
    const sal_uInt32 nLight(m_pCtlLightPreview->GetSvx3DLightControl().GetSelectedLight());
    PushButton* pBtn = nullptr;

    switch( nLight )
    {
        case 0: pBtn = m_pBtnLight1; break;
        case 1: pBtn = m_pBtnLight2; break;
        case 2: pBtn = m_pBtnLight3; break;
        case 3: pBtn = m_pBtnLight4; break;
        case 4: pBtn = m_pBtnLight5; break;
        case 5: pBtn = m_pBtnLight6; break;
        case 6: pBtn = m_pBtnLight7; break;
        case 7: pBtn = m_pBtnLight8; break;
        default: break;
    }

    if( pBtn )
        ClickHdl( pBtn );
    else
    {
        // Status: No lamp selected
        if( m_pBtnLight1->IsChecked() )
        {
            m_pBtnLight1->Check( false );
            m_pLbLight1->Enable( false );
        }
        else if( m_pBtnLight2->IsChecked() )
        {
            m_pBtnLight2->Check( false );
            m_pLbLight2->Enable( false );
        }
        else if( m_pBtnLight3->IsChecked() )
        {
            m_pBtnLight3->Check( false );
            m_pLbLight3->Enable( false );
        }
        else if( m_pBtnLight4->IsChecked() )
        {
            m_pBtnLight4->Check( false );
            m_pLbLight4->Enable( false );
        }
        else if( m_pBtnLight5->IsChecked() )
        {
            m_pBtnLight5->Check( false );
            m_pLbLight5->Enable( false );
        }
        else if( m_pBtnLight6->IsChecked() )
        {
            m_pBtnLight6->Check( false );
            m_pLbLight6->Enable( false );
        }
        else if( m_pBtnLight7->IsChecked() )
        {
            m_pBtnLight7->Check( false );
            m_pLbLight7->Enable( false );
        }
        else if( m_pBtnLight8->IsChecked() )
        {
            m_pBtnLight8->Check( false );
            m_pLbLight8->Enable( false );
        }
        m_pBtnLightColor->Enable( false );
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
void Svx3DWin::LBSelectColor( SvxColorListBox* pLb, const Color& rColor )
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
    m_pCtlPreview->Set3DAttributes( aSet );
    m_pCtlLightPreview->GetSvx3DLightControl().Set3DAttributes( aSet );
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
    m_pLbLight1->SelectEntry( aColWhite );
    m_pLbLight2->SelectEntry( aColWhite );
    m_pLbLight3->SelectEntry( aColWhite );
    m_pLbLight4->SelectEntry( aColWhite );
    m_pLbLight5->SelectEntry( aColWhite );
    m_pLbLight6->SelectEntry( aColWhite );
    m_pLbLight7->SelectEntry( aColWhite );
    m_pLbLight8->SelectEntry( aColWhite );
    m_pLbAmbientlight->SelectEntry( aColBlack );
    m_pLbMatColor->SelectEntry( aColWhite );
    m_pLbMatEmission->SelectEntry( aColBlack );
    m_pLbMatSpecular->SelectEntry( aColWhite );
}


sal_uInt16 Svx3DWin::GetLightSource( const PushButton* pBtn )
{
    sal_uInt16 nLight = 8;

    if( pBtn == nullptr )
    {
        if( m_pBtnLight1->IsChecked() )
            nLight = 0;
        else if( m_pBtnLight2->IsChecked() )
            nLight = 1;
        else if( m_pBtnLight3->IsChecked() )
            nLight = 2;
        else if( m_pBtnLight4->IsChecked() )
            nLight = 3;
        else if( m_pBtnLight5->IsChecked() )
            nLight = 4;
        else if( m_pBtnLight6->IsChecked() )
            nLight = 5;
        else if( m_pBtnLight7->IsChecked() )
            nLight = 6;
        else if( m_pBtnLight8->IsChecked() )
            nLight = 7;
    }
    else
    {
        if (pBtn == m_pBtnLight1)
            nLight = 0;
        else if (pBtn == m_pBtnLight2)
            nLight = 1;
        else if( pBtn == m_pBtnLight3 )
            nLight = 2;
        else if( pBtn == m_pBtnLight4 )
            nLight = 3;
        else if( pBtn == m_pBtnLight5 )
            nLight = 4;
        else if( pBtn == m_pBtnLight6 )
            nLight = 5;
        else if( pBtn == m_pBtnLight7 )
            nLight = 6;
        else if( pBtn == m_pBtnLight8 )
            nLight = 7;
    }
    return nLight;
};


SvxColorListBox* Svx3DWin::GetLbByButton( const PushButton* pBtn )
{
    SvxColorListBox* pLb = nullptr;

    if( pBtn == nullptr )
    {
        if( m_pBtnLight1->IsChecked() )
            pLb = m_pLbLight1;
        else if( m_pBtnLight2->IsChecked() )
            pLb = m_pLbLight2;
        else if( m_pBtnLight3->IsChecked() )
            pLb = m_pLbLight3;
        else if( m_pBtnLight4->IsChecked() )
            pLb = m_pLbLight4;
        else if( m_pBtnLight5->IsChecked() )
            pLb = m_pLbLight5;
        else if( m_pBtnLight6->IsChecked() )
            pLb = m_pLbLight6;
        else if( m_pBtnLight7->IsChecked() )
            pLb = m_pLbLight7;
        else if( m_pBtnLight8->IsChecked() )
            pLb = m_pLbLight8;
    }
    else
    {
        if( pBtn == m_pBtnLight1 )
            pLb = m_pLbLight1;
        else if (pBtn == m_pBtnLight2)
            pLb = m_pLbLight2;
        else if( pBtn == m_pBtnLight3 )
            pLb = m_pLbLight3;
        else if( pBtn == m_pBtnLight4 )
            pLb = m_pLbLight4;
        else if( pBtn == m_pBtnLight5 )
            pLb = m_pLbLight5;
        else if( pBtn == m_pBtnLight6 )
            pLb = m_pLbLight6;
        else if( pBtn == m_pBtnLight7 )
            pLb = m_pLbLight7;
        else if( pBtn == m_pBtnLight8 )
            pLb = m_pLbLight8;
    }
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
