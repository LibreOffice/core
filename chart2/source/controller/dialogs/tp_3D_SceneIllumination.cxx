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

#include "tp_3D_SceneIllumination.hxx"
#include <bitmaps.hlst>
#include <CommonConverters.hxx>
#include <ControllerLockGuard.hxx>

#include <svx/colorbox.hxx>
#include <svx/float3d.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svtools/colrdlg.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svddef.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/beans/XPropertySet.hpp>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace {

struct LightSource
{
    Color nDiffuseColor;
    css::drawing::Direction3D aDirection;
    bool bIsEnabled;

    LightSource() :
            nDiffuseColor( 0xcccccc ),
            aDirection( 1.0, 1.0, -1.0 ),
            bIsEnabled( false )
    {}
};

}

struct LightSourceInfo
{
    LightButton* pButton;
    LightSource aLightSource;

    LightSourceInfo();
    void initButtonFromSource();
};

LightSourceInfo::LightSourceInfo()
    : pButton(nullptr)
    , aLightSource()
{
    aLightSource.nDiffuseColor = Color(0xffffff); // white
    aLightSource.aDirection = drawing::Direction3D(1,1,1);
    aLightSource.bIsEnabled = false;
}

void LightSourceInfo::initButtonFromSource()
{
    if (!pButton)
        return;
    pButton->switchLightOn(aLightSource.bIsEnabled);
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

    void lcl_selectColor(ColorListBox& rListBox, const Color& rColor)
    {
        rListBox.SetNoSelection();
        rListBox.SelectEntry(std::make_pair(rColor, lcl_makeColorName(rColor)));
    }

    ::chart::LightSource lcl_getLightSourceFromProperties(
        const uno::Reference< beans::XPropertySet > & xSceneProperties,
        sal_Int32 nIndex )
    {
        ::chart::LightSource aResult;
        if( 0 <= nIndex && nIndex < 8 )
        {
            OUString aIndex( OUString::number( nIndex + 1 ));

            try
            {
                xSceneProperties->getPropertyValue( "D3DSceneLightColor" + aIndex ) >>= aResult.nDiffuseColor;
                xSceneProperties->getPropertyValue( "D3DSceneLightDirection" + aIndex ) >>= aResult.aDirection;
                xSceneProperties->getPropertyValue( "D3DSceneLightOn" + aIndex ) >>= aResult.bIsEnabled;
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
        return aResult;
    }

    void lcl_setLightSource(
        const uno::Reference< beans::XPropertySet > & xSceneProperties,
        const ::chart::LightSource & rLightSource,
        sal_Int32 nIndex )
    {
        if( 0 > nIndex || nIndex >= 8 )
            return;

        OUString aIndex( OUString::number( nIndex + 1 ));

        try
        {
            xSceneProperties->setPropertyValue( "D3DSceneLightColor" + aIndex,
                                                uno::makeAny( rLightSource.nDiffuseColor ));
            xSceneProperties->setPropertyValue( "D3DSceneLightDirection" + aIndex,
                                                uno::Any( rLightSource.aDirection ));
            xSceneProperties->setPropertyValue( "D3DSceneLightOn" + aIndex,
                                                uno::Any( rLightSource.bIsEnabled ));
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    Color lcl_getAmbientColor(
        const uno::Reference< beans::XPropertySet > & xSceneProperties )
    {
        sal_Int32 nResult = 0x000000;
        try
        {
            xSceneProperties->getPropertyValue("D3DSceneAmbientColor") >>= nResult;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
        return Color( nResult );
    }

    void lcl_setAmbientColor(
        const uno::Reference< beans::XPropertySet > & xSceneProperties,
        const Color & rColor )
    {
        try
        {
            xSceneProperties->setPropertyValue("D3DSceneAmbientColor",
                                               uno::makeAny( rColor ));
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

ThreeD_SceneIllumination_TabPage::ThreeD_SceneIllumination_TabPage(weld::Container* pParent,
    weld::Window* pTopLevel,
    const uno::Reference< beans::XPropertySet > & xSceneProperties,
    const uno::Reference< frame::XModel >& xChartModel)
    : m_xSceneProperties( xSceneProperties )
    , m_aTimerTriggeredControllerLock( xChartModel )
    , m_bInCommitToModel( false )
    , m_xChartModel( xChartModel )
    , m_pTopLevel(pTopLevel)
    , m_xBuilder(Application::CreateBuilder(pParent, "modules/schart/ui/tp_3D_SceneIllumination.ui"))
    , m_xContainer(m_xBuilder->weld_container("tp_3D_SceneIllumination"))
    , m_xBtn_Light1(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_1")))
    , m_xBtn_Light2(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_2")))
    , m_xBtn_Light3(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_3")))
    , m_xBtn_Light4(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_4")))
    , m_xBtn_Light5(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_5")))
    , m_xBtn_Light6(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_6")))
    , m_xBtn_Light7(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_7")))
    , m_xBtn_Light8(new LightButton(m_xBuilder->weld_toggle_button("BTN_LIGHT_8")))
    , m_xLB_LightSource(new ColorListBox(m_xBuilder->weld_menu_button("LB_LIGHTSOURCE"), pTopLevel))
    , m_xBtn_LightSource_Color(m_xBuilder->weld_button("BTN_LIGHTSOURCE_COLOR"))
    , m_xLB_AmbientLight(new ColorListBox(m_xBuilder->weld_menu_button("LB_AMBIENTLIGHT"), pTopLevel))
    , m_xBtn_AmbientLight_Color(m_xBuilder->weld_button("BTN_AMBIENT_COLOR"))
    , m_xHoriScale(m_xBuilder->weld_scale("hori"))
    , m_xVertScale(m_xBuilder->weld_scale("vert"))
    , m_xBtn_Corner(m_xBuilder->weld_button("corner"))
    , m_xPreview(new Svx3DLightControl)
    , m_xPreviewWnd(new weld::CustomWeld(*m_xBuilder, "CTL_LIGHT_PREVIEW", *m_xPreview))
    , m_xCtl_Preview(new SvxLightCtl3D(*m_xPreview, *m_xHoriScale, *m_xVertScale, *m_xBtn_Corner))
{
    m_pLightSourceInfoList.reset(new LightSourceInfo[8]);
    m_pLightSourceInfoList[0].pButton = m_xBtn_Light1.get();
    m_pLightSourceInfoList[1].pButton = m_xBtn_Light2.get();
    m_pLightSourceInfoList[2].pButton = m_xBtn_Light3.get();
    m_pLightSourceInfoList[3].pButton = m_xBtn_Light4.get();
    m_pLightSourceInfoList[4].pButton = m_xBtn_Light5.get();
    m_pLightSourceInfoList[5].pButton = m_xBtn_Light6.get();
    m_pLightSourceInfoList[6].pButton = m_xBtn_Light7.get();
    m_pLightSourceInfoList[7].pButton = m_xBtn_Light8.get();

    fillControlsFromModel(nullptr);

    m_xBtn_Light1->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light2->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light3->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light4->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light5->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light6->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light7->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_xBtn_Light8->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );

    m_xLB_AmbientLight->SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );
    m_xLB_LightSource->SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );

    m_xBtn_AmbientLight_Color->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );
    m_xBtn_LightSource_Color->connect_clicked( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );

    m_xCtl_Preview->SetUserInteractiveChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewChangeHdl ) );
    m_xCtl_Preview->SetUserSelectionChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewSelectHdl ) );

    ClickLightSourceButtonHdl(*m_xBtn_Light2->get_widget());
}

ThreeD_SceneIllumination_TabPage::~ThreeD_SceneIllumination_TabPage()
{
}

IMPL_LINK_NOARG(ThreeD_SceneIllumination_TabPage, fillControlsFromModel, void*, void)
{
    if( m_bInCommitToModel )//don't read own changes
        return;

    sal_Int32 nL=0;
    for( nL=0; nL<8; nL++)
        m_pLightSourceInfoList[nL].aLightSource = lcl_getLightSourceFromProperties( m_xSceneProperties, nL );
    for( nL=0; nL<8; nL++)
        m_pLightSourceInfoList[nL].initButtonFromSource();

    lcl_selectColor( *m_xLB_AmbientLight, lcl_getAmbientColor( m_xSceneProperties ));

    updatePreview();
}

void ThreeD_SceneIllumination_TabPage::applyLightSourceToModel( sal_uInt32 nLightNumber )
{
    ControllerLockGuardUNO aGuard( m_xChartModel );
    m_bInCommitToModel = true;
    sal_Int32 nIndex( nLightNumber );
    lcl_setLightSource( m_xSceneProperties, m_pLightSourceInfoList[nIndex].aLightSource, nIndex );
    m_bInCommitToModel = false;
}

void ThreeD_SceneIllumination_TabPage::applyLightSourcesToModel()
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aGuard( m_xChartModel );
    for( sal_Int32 nL=0; nL<8; nL++)
        applyLightSourceToModel( nL );
    m_aTimerTriggeredControllerLock.startTimer();
}

IMPL_LINK_NOARG(ThreeD_SceneIllumination_TabPage, PreviewChangeHdl, SvxLightCtl3D*, void)
{
    m_aTimerTriggeredControllerLock.startTimer();

    //update m_pLightSourceInfoList from preview
    const SfxItemSet a3DLightAttributes(m_xCtl_Preview->GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_1).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1).GetValue());

    pInfo = &m_pLightSourceInfoList[1];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_2).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2).GetValue());

    pInfo = &m_pLightSourceInfoList[2];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_3).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3).GetValue());

    pInfo = &m_pLightSourceInfoList[3];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_4).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4).GetValue());

    pInfo = &m_pLightSourceInfoList[4];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_5).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5).GetValue());

    pInfo = &m_pLightSourceInfoList[5];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_6).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6).GetValue());

    pInfo = &m_pLightSourceInfoList[6];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_7).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7).GetValue());

    pInfo = &m_pLightSourceInfoList[7];
    pInfo->aLightSource.nDiffuseColor = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8).GetValue();
    pInfo->aLightSource.bIsEnabled = a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_8).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8).GetValue());

    applyLightSourcesToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneIllumination_TabPage, PreviewSelectHdl, SvxLightCtl3D*, void)
{
    sal_uInt32 nLightNumber = m_xCtl_Preview->GetSvx3DLightControl().GetSelectedLight();
    if(nLightNumber<8)
    {
        LightButton* pButton = m_pLightSourceInfoList[nLightNumber].pButton;
        if(!pButton->get_active())
            ClickLightSourceButtonHdl(*pButton->get_widget());

        applyLightSourcesToModel();
    }
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, ColorDialogHdl, weld::Button&, rButton, void )
{
    bool bIsAmbientLight = (&rButton == m_xBtn_AmbientLight_Color.get());
    ColorListBox* pListBox = bIsAmbientLight ? m_xLB_AmbientLight.get() : m_xLB_LightSource.get();

    SvColorDialog aColorDlg;
    aColorDlg.SetColor( pListBox->GetSelectEntryColor() );
    if( aColorDlg.Execute(m_pTopLevel) != RET_OK )
        return;

    Color aColor( aColorDlg.GetColor());
    lcl_selectColor( *pListBox, aColor );
    if( bIsAmbientLight )
    {
        m_bInCommitToModel = true;
        lcl_setAmbientColor( m_xSceneProperties, aColor );
        m_bInCommitToModel = false;
    }
    else
    {
    //get active lightsource:
        LightSourceInfo* pInfo = nullptr;
        sal_Int32 nL=0;
        for( nL=0; nL<8; nL++)
        {
            pInfo = &m_pLightSourceInfoList[nL];
            if(pInfo->pButton->get_active())
                break;
            pInfo = nullptr;
        }
        if(pInfo)
            applyLightSourceToModel( nL );
    }
    SelectColorHdl( *pListBox );
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, SelectColorHdl, ColorListBox&, rBox, void )
{
    ColorListBox* pListBox = &rBox;
    if (pListBox == m_xLB_AmbientLight.get())
    {
        m_bInCommitToModel = true;
        lcl_setAmbientColor( m_xSceneProperties, pListBox->GetSelectEntryColor());
        m_bInCommitToModel = false;
    }
    else if (pListBox == m_xLB_LightSource.get())
    {
        //get active lightsource:
        LightSourceInfo* pInfo = nullptr;
        sal_Int32 nL=0;
        for( nL=0; nL<8; nL++)
        {
            pInfo = &m_pLightSourceInfoList[nL];
            if (pInfo->pButton->get_active())
                break;
            pInfo = nullptr;
        }
        if(pInfo)
        {
            pInfo->aLightSource.nDiffuseColor = pListBox->GetSelectEntryColor();
            applyLightSourceToModel( nL );
        }
    }
    updatePreview();
}

IMPL_LINK(ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl, weld::Button&, rBtn, void)
{
    LightButton* pButton = nullptr;
    LightSourceInfo* pInfo = nullptr;
    sal_Int32 nL=0;
    for( nL=0; nL<8; nL++)
    {
        if (m_pLightSourceInfoList[nL].pButton->get_widget() == &rBtn)
        {
            pButton = m_pLightSourceInfoList[nL].pButton;
            pInfo = &m_pLightSourceInfoList[nL];
            break;
        }
    }

    assert(pInfo);

    bool bIsChecked = pInfo->pButton->get_prev_active();

    ControllerLockGuardUNO aGuard( m_xChartModel );
    for( nL=0; nL<8; nL++)
    {
        LightButton* pLightButton = m_pLightSourceInfoList[nL].pButton;
        if (pLightButton == pButton)
        {
            pLightButton->set_active(true);
            if (!pLightButton->get_widget()->has_focus())
                pLightButton->get_widget()->grab_focus();
            m_pLightSourceInfoList[nL].pButton->set_prev_active(true);
        }
        else
        {
            pLightButton->set_active(false);
            m_pLightSourceInfoList[nL].pButton->set_prev_active(false);
        }
    }

    //update light button
    if (bIsChecked)
    {
        pButton->switchLightOn(!pButton->isLightOn());
        pInfo->aLightSource.bIsEnabled=pButton->isLightOn();
        applyLightSourceToModel( nL );
    }

    //update color list box
    lcl_selectColor( *m_xLB_LightSource, pInfo->aLightSource.nDiffuseColor );
    updatePreview();
}

void ThreeD_SceneIllumination_TabPage::updatePreview()
{
    SfxItemSet aItemSet(m_xCtl_Preview->GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    // AmbientColor
    aItemSet.Put(makeSvx3DAmbientcolorItem(m_xLB_AmbientLight->GetSelectEntryColor()));

    aItemSet.Put(makeSvx3DLightcolor1Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff1Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection1Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[1];
    aItemSet.Put(makeSvx3DLightcolor2Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff2Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection2Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[2];
    aItemSet.Put(makeSvx3DLightcolor3Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff3Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection3Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[3];
    aItemSet.Put(makeSvx3DLightcolor4Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff4Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection4Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[4];
    aItemSet.Put(makeSvx3DLightcolor5Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff5Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection5Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[5];
    aItemSet.Put(makeSvx3DLightcolor6Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff6Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection6Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[6];
    aItemSet.Put(makeSvx3DLightcolor7Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff7Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection7Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[7];
    aItemSet.Put(makeSvx3DLightcolor8Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(makeSvx3DLightOnOff8Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(makeSvx3DLightDirection8Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    // set lights and ambient light
    m_xCtl_Preview->GetSvx3DLightControl().Set3DAttributes(aItemSet);

    // select light
    for(sal_uInt32 a(0); a < 8; a++)
    {
        if (m_pLightSourceInfoList[a].pButton->get_active())
        {
            m_xCtl_Preview->GetSvx3DLightControl().SelectLight(a);
            m_xCtl_Preview->CheckSelection();
            break;
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
