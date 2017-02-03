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
#include "ResId.hxx"
#include "Bitmaps.hrc"
#include "CommonConverters.hxx"

#include <svx/colorbox.hxx>
#include "svx/dialogs.hrc"
#include <svx/dialmgr.hxx>
#include <rtl/math.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svddef.hxx>
#include <vcl/builderfactory.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LightButton::LightButton( vcl::Window* pParent)
            : ImageButton( pParent)
            , m_bLightOn(false)
{
    SetModeImage(Image(BitmapEx(SVX_RES(RID_SVXBMP_LAMP_OFF))));
}

VCL_BUILDER_FACTORY(LightButton)

void LightButton::switchLightOn(bool bOn)
{
    if( m_bLightOn==bOn )
        return;
    m_bLightOn = bOn;
    if(m_bLightOn)
    {
        SetModeImage(Image(BitmapEx(SVX_RES(RID_SVXBMP_LAMP_ON))));
    }
    else
    {
        SetModeImage(Image(BitmapEx(SVX_RES(RID_SVXBMP_LAMP_OFF))));
    }
}

struct LightSource
{
    long nDiffuseColor;
    css::drawing::Direction3D aDirection;
    bool bIsEnabled;

    LightSource() :
            nDiffuseColor( 0xcccccc ),
            aDirection( 1.0, 1.0, -1.0 ),
            bIsEnabled( false )
    {}
};

struct LightSourceInfo
{
    VclPtr<LightButton> pButton;
    LightSource  aLightSource;

    LightSourceInfo();
    void initButtonFromSource();
};

LightSourceInfo::LightSourceInfo()
    : pButton(nullptr)
    , aLightSource()
{
    aLightSource.nDiffuseColor = 0xffffff; // white
    aLightSource.aDirection = drawing::Direction3D(1,1,1);
    aLightSource.bIsEnabled = false;
}

void LightSourceInfo::initButtonFromSource()
{
    if(!pButton)
        return;
    pButton->SetModeImage(Image(BitmapEx(SVX_RES(
        aLightSource.bIsEnabled ? RID_SVXBMP_LAMP_ON : RID_SVXBMP_LAMP_OFF
    ) ) ));
}

namespace
{
    OUString lcl_makeColorName(const Color& rColor)
    {
        OUString aStr = SVX_RESSTR(RID_SVXFLOAT3D_FIX_R) +
                        OUString::number(rColor.GetRed()) +
                        " " +
                        SVX_RESSTR(RID_SVXFLOAT3D_FIX_G) +
                        OUString::number(rColor.GetGreen()) +
                        " " +
                        SVX_RESSTR(RID_SVXFLOAT3D_FIX_B) +
                        OUString::number(rColor.GetBlue());
        return aStr;
    }

    void lcl_selectColor(SvxColorListBox& rListBox, const Color& rColor)
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
            catch( const uno::Exception & ex )
            {
                (void)(ex); // no warning in non-debug builds
                OSL_FAIL( OUStringToOString("Property Exception caught. Message: " +
                                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
            }
        }
        return aResult;
    }

    void lcl_setLightSource(
        const uno::Reference< beans::XPropertySet > & xSceneProperties,
        const ::chart::LightSource & rLightSource,
        sal_Int32 nIndex )
    {
        if( 0 <= nIndex && nIndex < 8 )
        {
            OUString aIndex( OUString::number( nIndex + 1 ));

            try
            {
                xSceneProperties->setPropertyValue( "D3DSceneLightColor" + aIndex,
                                                    uno::Any( rLightSource.nDiffuseColor ));
                xSceneProperties->setPropertyValue( "D3DSceneLightDirection" + aIndex,
                                                    uno::Any( rLightSource.aDirection ));
                xSceneProperties->setPropertyValue( "D3DSceneLightOn" + aIndex,
                                                    uno::Any( rLightSource.bIsEnabled ));
            }
            catch( const uno::Exception & ex )
            {
                (void)(ex); // no warning in non-debug builds
                OSL_FAIL( OUStringToOString("Property Exception caught. Message: " +
                                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
            }
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
        catch( const uno::Exception & ex )
        {
            (void)(ex); // no warning in non-debug builds
            OSL_FAIL( OUStringToOString("Property Exception caught. Message: " +
                                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
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
                                               uno::Any( rColor.GetColor()));
        }
        catch( const uno::Exception & ex )
        {
            (void)(ex); // no warning in non-debug builds
            OSL_FAIL( OUStringToOString( "Property Exception caught. Message: " +
                                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        }
    }
}

ThreeD_SceneIllumination_TabPage::ThreeD_SceneIllumination_TabPage( vcl::Window* pWindow
                , const uno::Reference< beans::XPropertySet > & xSceneProperties
                , const uno::Reference< frame::XModel >& xChartModel )
                : TabPage ( pWindow
                          ,"tp_3D_SceneIllumination"
                          ,"modules/schart/ui/tp_3D_SceneIllumination.ui")
                , m_pLightSourceInfoList(nullptr)
                , m_xSceneProperties( xSceneProperties )
                , m_aTimerTriggeredControllerLock( xChartModel )
                , m_bInCommitToModel( false )
                , m_xChartModel( xChartModel )
{
    get(m_pBtn_Light1, "BTN_LIGHT_1");
    get(m_pBtn_Light2, "BTN_LIGHT_2");
    get(m_pBtn_Light3, "BTN_LIGHT_3");
    get(m_pBtn_Light4, "BTN_LIGHT_4");
    get(m_pBtn_Light5, "BTN_LIGHT_5");
    get(m_pBtn_Light6, "BTN_LIGHT_6");
    get(m_pBtn_Light7, "BTN_LIGHT_7");
    get(m_pBtn_Light8, "BTN_LIGHT_8");

    get(m_pLB_LightSource, "LB_LIGHTSOURCE");
    get(m_pLB_AmbientLight, "LB_AMBIENTLIGHT");
    get(m_pBtn_LightSource_Color, "BTN_LIGHTSOURCE_COLOR");
    get(m_pBtn_AmbientLight_Color, "BTN_AMBIENT_COLOR");

    get(m_pCtl_Preview, "CTL_LIGHT_PREVIEW");

    m_pLightSourceInfoList = new LightSourceInfo[8];
    m_pLightSourceInfoList[0].pButton = m_pBtn_Light1;
    m_pLightSourceInfoList[1].pButton = m_pBtn_Light2;
    m_pLightSourceInfoList[2].pButton = m_pBtn_Light3;
    m_pLightSourceInfoList[3].pButton = m_pBtn_Light4;
    m_pLightSourceInfoList[4].pButton = m_pBtn_Light5;
    m_pLightSourceInfoList[5].pButton = m_pBtn_Light6;
    m_pLightSourceInfoList[6].pButton = m_pBtn_Light7;
    m_pLightSourceInfoList[7].pButton = m_pBtn_Light8;

    fillControlsFromModel(nullptr);

    m_pBtn_Light1->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light2->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light3->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light4->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light5->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light6->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light7->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_pBtn_Light8->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );

    m_pLB_AmbientLight->SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );
    m_pLB_LightSource->SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );

    m_pBtn_AmbientLight_Color->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );
    m_pBtn_LightSource_Color->SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );

    m_pCtl_Preview->SetUserInteractiveChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewChangeHdl ) );
    m_pCtl_Preview->SetUserSelectionChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewSelectHdl ) );

    ClickLightSourceButtonHdl(m_pBtn_Light2);
}

ThreeD_SceneIllumination_TabPage::~ThreeD_SceneIllumination_TabPage()
{
    disposeOnce();
}

void ThreeD_SceneIllumination_TabPage::dispose()
{
    delete[] m_pLightSourceInfoList;
    m_pLightSourceInfoList = nullptr;
    m_pBtn_Light1.clear();
    m_pBtn_Light2.clear();
    m_pBtn_Light3.clear();
    m_pBtn_Light4.clear();
    m_pBtn_Light5.clear();
    m_pBtn_Light6.clear();
    m_pBtn_Light7.clear();
    m_pBtn_Light8.clear();
    m_pLB_LightSource.clear();
    m_pBtn_LightSource_Color.clear();
    m_pLB_AmbientLight.clear();
    m_pBtn_AmbientLight_Color.clear();
    m_pCtl_Preview.clear();
    TabPage::dispose();
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

    lcl_selectColor( *m_pLB_AmbientLight, lcl_getAmbientColor( m_xSceneProperties ));

    this->updatePreview();
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
    const SfxItemSet a3DLightAttributes(m_pCtl_Preview->GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue());

    pInfo = &m_pLightSourceInfoList[1];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue());

    pInfo = &m_pLightSourceInfoList[2];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue());

    pInfo = &m_pLightSourceInfoList[3];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue());

    pInfo = &m_pLightSourceInfoList[4];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue());

    pInfo = &m_pLightSourceInfoList[5];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue());

    pInfo = &m_pLightSourceInfoList[6];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue());

    pInfo = &m_pLightSourceInfoList[7];
    pInfo->aLightSource.nDiffuseColor = static_cast<const SvxColorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = static_cast<const SfxBoolItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(static_cast<const SvxB3DVectorItem&>(a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue());

    applyLightSourcesToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneIllumination_TabPage, PreviewSelectHdl, SvxLightCtl3D*, void)
{
    sal_uInt32 nLightNumber = m_pCtl_Preview->GetSvx3DLightControl().GetSelectedLight();
    if(nLightNumber<8)
    {
        LightButton* pButton = m_pLightSourceInfoList[nLightNumber].pButton;
        if(!pButton->IsChecked())
            ClickLightSourceButtonHdl(pButton);

        applyLightSourcesToModel();
    }
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, ColorDialogHdl, Button*, pButton, void )
{
    bool bIsAmbientLight = (pButton==m_pBtn_AmbientLight_Color);
    SvxColorListBox* pListBox = bIsAmbientLight ? m_pLB_AmbientLight.get() : m_pLB_LightSource.get();

    SvColorDialog aColorDlg( this );
    aColorDlg.SetColor( pListBox->GetSelectEntryColor() );
    if( aColorDlg.Execute() == RET_OK )
    {
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
                if(pInfo->pButton->IsChecked())
                    break;
                pInfo = nullptr;
            }
            if(pInfo)
                applyLightSourceToModel( nL );
        }
        SelectColorHdl( *pListBox );
    }
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, SelectColorHdl, SvxColorListBox&, rBox, void )
{
    SvxColorListBox* pListBox = &rBox;
    if(pListBox==m_pLB_AmbientLight)
    {
        m_bInCommitToModel = true;
        lcl_setAmbientColor( m_xSceneProperties, pListBox->GetSelectEntryColor().GetColor());
        m_bInCommitToModel = false;
    }
    else if(pListBox==m_pLB_LightSource)
    {
        //get active lightsource:
        LightSourceInfo* pInfo = nullptr;
        sal_Int32 nL=0;
        for( nL=0; nL<8; nL++)
        {
            pInfo = &m_pLightSourceInfoList[nL];
            if(pInfo->pButton->IsChecked())
                break;
            pInfo = nullptr;
        }
        if(pInfo)
        {
            pInfo->aLightSource.nDiffuseColor = pListBox->GetSelectEntryColor().GetColor();
            applyLightSourceToModel( nL );
        }
    }
    this->updatePreview();
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl, Button*, pBtn, void )
{
    LightButton* pButton = static_cast<LightButton*>(pBtn);
    if( !pButton )
        return;

    LightSourceInfo* pInfo = nullptr;
    sal_Int32 nL=0;
    for( nL=0; nL<8; nL++)
    {
        if( m_pLightSourceInfoList[nL].pButton == pButton )
        {
            pInfo = &m_pLightSourceInfoList[nL];
            break;
        }
    }

    //update light button
    bool bIsChecked = pButton->IsChecked();
    if(bIsChecked)
    {
        pButton->switchLightOn(!pButton->isLightOn());
        if(pInfo)
        {
            pInfo->aLightSource.bIsEnabled=pButton->isLightOn();
            applyLightSourceToModel( nL );
        }
    }
    else
    {
        ControllerLockGuardUNO aGuard( m_xChartModel );
        for( nL=0; nL<8; nL++)
        {
            LightButton* pLightButton = m_pLightSourceInfoList[nL].pButton;
            pLightButton->Check( pLightButton == pButton );
        }
    }

    //update color list box
    if(pInfo)
    {
        lcl_selectColor( *m_pLB_LightSource, pInfo->aLightSource.nDiffuseColor );
    }
    this->updatePreview();
}

void ThreeD_SceneIllumination_TabPage::updatePreview()
{
    SfxItemSet aItemSet(m_pCtl_Preview->GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    // AmbientColor
    aItemSet.Put(makeSvx3DAmbientcolorItem(m_pLB_AmbientLight->GetSelectEntryColor()));

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
    m_pCtl_Preview->GetSvx3DLightControl().Set3DAttributes(aItemSet);

    // select light
    for(sal_uInt32 a(0); a < 8; a++)
    {
        if(m_pLightSourceInfoList[a].pButton->IsChecked())
        {
            m_pCtl_Preview->GetSvx3DLightControl().SelectLight(a);
            m_pCtl_Preview->CheckSelection();
            break;
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
