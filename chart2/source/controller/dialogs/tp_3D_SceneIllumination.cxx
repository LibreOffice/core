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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "tp_3D_SceneIllumination.hxx"
#include "tp_3D_SceneIllumination.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "CommonConverters.hxx"
#include "NoWarningThisInCTOR.hxx"

#include "svx/dialogs.hrc"
// header for define SVX_RES
#include <svx/dialmgr.hxx>
#include <rtl/math.hxx>

// header for class SvColorDialog
#include <svtools/colrdlg.hxx>

// header for define RET_OK
#include <vcl/msgbox.hxx>

#include <svx/svx3ditems.hxx>
#include <svx/svddef.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LightButton::LightButton( Window* pParent, const ResId& rResId, sal_Int32 nLightNumber )
            : ImageButton( pParent, rResId )
            , m_bLightOn(false)
{
    SetModeImage( Image( SVX_RES(RID_SVXIMAGE_LIGHT_OFF)   ) );

    String aTipHelpStr( SchResId(STR_TIP_LIGHTSOURCE_X) );
    rtl::OUString aTipHelp( aTipHelpStr  );
    const rtl::OUString aReplacementStr( RTL_CONSTASCII_USTRINGPARAM( "%LIGHTNUMBER" ));
    sal_Int32 nIndex = aTipHelp.indexOf( aReplacementStr );
    if( nIndex != -1 )
    {
        aTipHelp = aTipHelp.replaceAt(nIndex, aReplacementStr.getLength(),
            rtl::OUString::valueOf( nLightNumber ) );
    }
    this->SetQuickHelpText( String( aTipHelp ) );
}

LightButton::~LightButton()
{
}

void LightButton::switchLightOn(bool bOn)
{
    if( m_bLightOn==bOn )
        return;
    m_bLightOn = bOn;
    if(m_bLightOn)
    {
        SetModeImage( Image( SVX_RES(RID_SVXIMAGE_LIGHT_ON) ) );
    }
    else
    {
        SetModeImage( Image( SVX_RES(RID_SVXIMAGE_LIGHT_OFF) ) );
    }
}

bool LightButton::isLightOn() const
{
    return m_bLightOn;
}

//-----------------------------------------------------------------------------

ColorButton::ColorButton( Window* pParent, const ResId& rResId )
            : ImageButton( pParent, rResId )
{
    SetModeImage( Image( SVX_RES(RID_SVXIMAGE_COLORDLG) ) );
    this->SetQuickHelpText( String( SchResId(STR_TIP_CHOOSECOLOR) ) );
}

ColorButton::~ColorButton()
{
}

//-----------------------------------------------------------------------------

struct LightSource
{
    long nDiffuseColor;
    ::com::sun::star::drawing::Direction3D aDirection;
    bool bIsEnabled;

    LightSource() :
            nDiffuseColor( 0xcccccc ),
            aDirection( 1.0, 1.0, -1.0 ),
            bIsEnabled( false )
    {}
};

struct LightSourceInfo
{
    LightButton* pButton;
    LightSource  aLightSource;

    LightSourceInfo();
    void initButtonFromSource();
};

LightSourceInfo::LightSourceInfo()
    : pButton(0)
    , aLightSource()
{
    aLightSource.nDiffuseColor = 0xffffff; // white
    aLightSource.aDirection = drawing::Direction3D(1,1,1);
    aLightSource.bIsEnabled = sal_False;
}

void LightSourceInfo::initButtonFromSource()
{
    if(!pButton)
        return;
    pButton->SetModeImage( Image( SVX_RES(
        aLightSource.bIsEnabled ? RID_SVXIMAGE_LIGHT_ON : RID_SVXIMAGE_LIGHT_OFF
    ) ) );
}

//-----------------------------------------------------------------------------

namespace
{
    rtl::OUString lcl_makeColorName( Color rColor )
    {
        String aStr(SVX_RES(RID_SVXFLOAT3D_FIX_R));
        aStr += String::CreateFromInt32((sal_Int32)rColor.GetRed());
        aStr += sal_Unicode(' ');
        aStr += String(SVX_RES(RID_SVXFLOAT3D_FIX_G));
        aStr += String::CreateFromInt32((sal_Int32)rColor.GetGreen());
        aStr += sal_Unicode(' ');
        aStr += String(SVX_RES(RID_SVXFLOAT3D_FIX_B));
        aStr += String::CreateFromInt32((sal_Int32)rColor.GetBlue());
        return aStr;
    }
    void lcl_selectColor( ColorListBox& rListBox, const Color& rColor )
    {
        rListBox.SetNoSelection();
        rListBox.SelectEntry( rColor );
        if( rListBox.GetSelectEntryCount() == 0 )
        {
            sal_uInt16 nPos = rListBox.InsertEntry( rColor, lcl_makeColorName( rColor ) );
            rListBox.SelectEntryPos( nPos );
        }
    }

    ::chart::LightSource lcl_getLightSourceFromProperties(
        const uno::Reference< beans::XPropertySet > & xSceneProperties,
        sal_Int32 nIndex )
    {
        ::chart::LightSource aResult;
        if( 0 <= nIndex && nIndex < 8 )
        {
            ::rtl::OUString aColorPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor"));
            ::rtl::OUString aDirectionPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection"));
            ::rtl::OUString aEnabledPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn"));
            ::rtl::OUString aIndex( ::rtl::OUString::valueOf( nIndex + 1 ));

            try
            {
                xSceneProperties->getPropertyValue( aColorPropertyPrefix + aIndex ) >>= aResult.nDiffuseColor;
                xSceneProperties->getPropertyValue( aDirectionPropertyPrefix + aIndex ) >>= aResult.aDirection;
                xSceneProperties->getPropertyValue( aEnabledPropertyPrefix + aIndex ) >>= aResult.bIsEnabled;
            }
            catch( const uno::Exception & ex )
            {
                (void)(ex); // no warning in non-debug builds
                OSL_ENSURE( false, ::rtl::OUStringToOString(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property Exception caught. Message: " )) +
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
            ::rtl::OUString aColorPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor"));
            ::rtl::OUString aDirectionPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection"));
            ::rtl::OUString aEnabledPropertyPrefix( RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn"));
            ::rtl::OUString aIndex( ::rtl::OUString::valueOf( nIndex + 1 ));

            try
            {
                xSceneProperties->setPropertyValue( aColorPropertyPrefix + aIndex,
                                                    uno::makeAny( rLightSource.nDiffuseColor ));
                xSceneProperties->setPropertyValue( aDirectionPropertyPrefix + aIndex,
                                                    uno::makeAny( rLightSource.aDirection ));
                xSceneProperties->setPropertyValue( aEnabledPropertyPrefix + aIndex,
                                                    uno::makeAny( rLightSource.bIsEnabled ));
            }
            catch( const uno::Exception & ex )
            {
                (void)(ex); // no warning in non-debug builds
                OSL_ENSURE( false, ::rtl::OUStringToOString(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property Exception caught. Message: " )) +
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
            xSceneProperties->getPropertyValue(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor"))) >>= nResult;
        }
        catch( const uno::Exception & ex )
        {
            (void)(ex); // no warning in non-debug builds
            OSL_ENSURE( false, ::rtl::OUStringToOString(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property Exception caught. Message: " )) +
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
            xSceneProperties->setPropertyValue(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor")),
                uno::makeAny( rColor.GetColor()));
        }
        catch( const uno::Exception & ex )
        {
            (void)(ex); // no warning in non-debug builds
            OSL_ENSURE( false, ::rtl::OUStringToOString(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property Exception caught. Message: " )) +
                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        }
    }
}

//-----------------------------------------------------------------------------

ThreeD_SceneIllumination_TabPage::ThreeD_SceneIllumination_TabPage( Window* pWindow
                , const uno::Reference< beans::XPropertySet > & xSceneProperties
                , const uno::Reference< frame::XModel >& xChartModel
                , XColorTable* pColorTable )
                : TabPage           ( pWindow, SchResId( TP_3D_SCENEILLUMINATION ) )
                , m_aFT_LightSource( this, SchResId( FT_LIGHTSOURCE ) )
                , m_aBtn_Light1( this, SchResId( BTN_LIGHT_1 ), 1 )
                , m_aBtn_Light2( this, SchResId( BTN_LIGHT_2 ), 2 )
                , m_aBtn_Light3( this, SchResId( BTN_LIGHT_3 ), 3 )
                , m_aBtn_Light4( this, SchResId( BTN_LIGHT_4 ), 4 )
                , m_aBtn_Light5( this, SchResId( BTN_LIGHT_5 ), 5 )
                , m_aBtn_Light6( this, SchResId( BTN_LIGHT_6 ), 6 )
                , m_aBtn_Light7( this, SchResId( BTN_LIGHT_7 ), 7 )
                , m_aBtn_Light8( this, SchResId( BTN_LIGHT_8 ), 8 )
                , m_aLB_LightSource( this, SchResId( LB_LIGHTSOURCE ) )
                , m_aBtn_LightSource_Color( this, SchResId( BTN_LIGHTSOURCE_COLOR ) )
                , m_aFT_AmbientLight( this, SchResId( FT_AMBIENTLIGHT ) )
                , m_aLB_AmbientLight( this, SchResId( LB_AMBIENTLIGHT ) )
                , m_aBtn_AmbientLight_Color( this, SchResId( BTN_AMBIENT_COLOR ) )
                , m_aCtl_Preview( this, SchResId( CTL_LIGHT_PREVIEW ) )
                , m_pLightSourceInfoList(0)
                , m_xSceneProperties( xSceneProperties )
                , m_aTimerTriggeredControllerLock( xChartModel )
                , m_bInCommitToModel( false )
                , m_aModelChangeListener( LINK( this, ThreeD_SceneIllumination_TabPage, fillControlsFromModel ) )
                , m_xChartModel( xChartModel )
{
    FreeResource();

    if(pColorTable)
    {
        m_aLB_AmbientLight.Fill( pColorTable );
        m_aLB_LightSource.Fill( pColorTable );
    }
    m_aLB_AmbientLight.SetDropDownLineCount(10);
    m_aLB_LightSource.SetDropDownLineCount(10);

    m_pLightSourceInfoList = new LightSourceInfo[8];
    m_pLightSourceInfoList[0].pButton = &m_aBtn_Light1;
    m_pLightSourceInfoList[1].pButton = &m_aBtn_Light2;
    m_pLightSourceInfoList[2].pButton = &m_aBtn_Light3;
    m_pLightSourceInfoList[3].pButton = &m_aBtn_Light4;
    m_pLightSourceInfoList[4].pButton = &m_aBtn_Light5;
    m_pLightSourceInfoList[5].pButton = &m_aBtn_Light6;
    m_pLightSourceInfoList[6].pButton = &m_aBtn_Light7;
    m_pLightSourceInfoList[7].pButton = &m_aBtn_Light8;

    fillControlsFromModel(0);

    m_aBtn_Light1.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light2.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light3.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light4.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light5.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light6.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light7.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );
    m_aBtn_Light8.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl ) );

    m_aLB_AmbientLight.SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );
    m_aLB_LightSource.SetSelectHdl( LINK( this, ThreeD_SceneIllumination_TabPage, SelectColorHdl ) );

    m_aBtn_AmbientLight_Color.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );
    m_aBtn_LightSource_Color.SetClickHdl( LINK( this, ThreeD_SceneIllumination_TabPage, ColorDialogHdl ) );

    m_aCtl_Preview.SetUserInteractiveChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewChangeHdl ) );
    m_aCtl_Preview.SetUserSelectionChangeCallback( LINK( this, ThreeD_SceneIllumination_TabPage, PreviewSelectHdl ) );

    ClickLightSourceButtonHdl(&m_aBtn_Light2);

    m_aModelChangeListener.startListening( uno::Reference< util::XModifyBroadcaster >(m_xSceneProperties, uno::UNO_QUERY) );
    m_aBtn_Light1.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light2.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light3.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light4.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light5.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light6.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light7.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aBtn_Light8.SetAccessibleRelationLabeledBy(&m_aFT_LightSource);
    m_aCtl_Preview.SetAccessibleName(String(SchResId( STR_LIGHT_PREVIEW )));
}

ThreeD_SceneIllumination_TabPage::~ThreeD_SceneIllumination_TabPage()
{
    delete[] m_pLightSourceInfoList;
}

void ThreeD_SceneIllumination_TabPage::commitPendingChanges()
{
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, fillControlsFromModel, void *, EMPTYARG )
{
    if( m_bInCommitToModel )//don't read own changes
        return 0;

    sal_Int32 nL=0;
    for( nL=0; nL<8; nL++)
        m_pLightSourceInfoList[nL].aLightSource = lcl_getLightSourceFromProperties( m_xSceneProperties, nL );
    for( nL=0; nL<8; nL++)
        m_pLightSourceInfoList[nL].initButtonFromSource();

    lcl_selectColor( m_aLB_AmbientLight, lcl_getAmbientColor( m_xSceneProperties ));

    this->updatePreview();

    return 0;
}

void ThreeD_SceneIllumination_TabPage::applyLightSourceToModel( sal_uInt32 nLightNumber )
{
    ControllerLockGuard aGuard( m_xChartModel );
    m_bInCommitToModel = true;
    sal_Int32 nIndex( nLightNumber );
    lcl_setLightSource( m_xSceneProperties, m_pLightSourceInfoList[nIndex].aLightSource, nIndex );
    m_bInCommitToModel = false;
}

void ThreeD_SceneIllumination_TabPage::applyLightSourcesToModel()
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuard aGuard( m_xChartModel );
    for( sal_Int32 nL=0; nL<8; nL++)
        applyLightSourceToModel( nL );
    m_aTimerTriggeredControllerLock.startTimer();
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, PreviewChangeHdl, void*, EMPTYARG )
{
    m_aTimerTriggeredControllerLock.startTimer();

    //update m_pLightSourceInfoList from preview
    const SfxItemSet a3DLightAttributes(m_aCtl_Preview.GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor1Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff1Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection1Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue());

    pInfo = &m_pLightSourceInfoList[1];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor2Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff2Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection2Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue());

    pInfo = &m_pLightSourceInfoList[2];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor3Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff3Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection3Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue());

    pInfo = &m_pLightSourceInfoList[3];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor4Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff4Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection4Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue());

    pInfo = &m_pLightSourceInfoList[4];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor5Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff5Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection5Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue());

    pInfo = &m_pLightSourceInfoList[5];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor6Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff6Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection6Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue());

    pInfo = &m_pLightSourceInfoList[6];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor7Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff7Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection7Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue());

    pInfo = &m_pLightSourceInfoList[7];
    pInfo->aLightSource.nDiffuseColor = ((const Svx3DLightcolor8Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue().GetColor();
    pInfo->aLightSource.bIsEnabled = ((const Svx3DLightOnOff8Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue();
    pInfo->aLightSource.aDirection = B3DVectorToDirection3D(((const Svx3DLightDirection8Item&)a3DLightAttributes.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue());

    applyLightSourcesToModel();

    return 0;
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, PreviewSelectHdl, void*, EMPTYARG )
{
    sal_uInt32 nLightNumber = m_aCtl_Preview.GetSvx3DLightControl().GetSelectedLight();
    if(nLightNumber<8)
    {
        LightButton* pButton = m_pLightSourceInfoList[nLightNumber].pButton;
        if(!pButton->IsChecked())
            ClickLightSourceButtonHdl(pButton);

        applyLightSourcesToModel();
    }
    return 0;
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, ColorDialogHdl, Button*, pButton )
{
    bool bIsAmbientLight = (pButton==&m_aBtn_AmbientLight_Color);
    ColorLB* pListBox = ( bIsAmbientLight ? &m_aLB_AmbientLight : &m_aLB_LightSource);

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
            LightSourceInfo* pInfo = 0;
            sal_Int32 nL=0;
            for( nL=0; nL<8; nL++)
            {
                pInfo = &m_pLightSourceInfoList[nL];
                if(pInfo->pButton->IsChecked())
                    break;
                pInfo = 0;
            }
            if(pInfo)
                applyLightSourceToModel( nL );
        }
        SelectColorHdl( pListBox );
    }
    return 0;
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, SelectColorHdl, ColorLB*, pListBox )
{
    if(pListBox==&m_aLB_AmbientLight)
    {
        m_bInCommitToModel = true;
        lcl_setAmbientColor( m_xSceneProperties, pListBox->GetSelectEntryColor().GetColor());
        m_bInCommitToModel = false;
    }
    else if(pListBox==&m_aLB_LightSource)
    {
        //get active lightsource:
        LightSourceInfo* pInfo = 0;
        sal_Int32 nL=0;
        for( nL=0; nL<8; nL++)
        {
            pInfo = &m_pLightSourceInfoList[nL];
            if(pInfo->pButton->IsChecked())
                break;
            pInfo = 0;
        }
        if(pInfo)
        {
            pInfo->aLightSource.nDiffuseColor = pListBox->GetSelectEntryColor().GetColor();
            applyLightSourceToModel( nL );
        }
    }
    this->updatePreview();
    return 0;
}

IMPL_LINK( ThreeD_SceneIllumination_TabPage, ClickLightSourceButtonHdl, LightButton*, pButton )
{
    if( !pButton )
        return 0;

    LightSourceInfo* pInfo = 0;
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
        ControllerLockGuard aGuard( m_xChartModel );
        for( nL=0; nL<8; nL++)
        {
            LightButton* pLightButton = m_pLightSourceInfoList[nL].pButton;
            pLightButton->Check( pLightButton == pButton );
        }
    }

    //update color list box
    if(pInfo)
    {
        lcl_selectColor( m_aLB_LightSource, pInfo->aLightSource.nDiffuseColor );
    }
    this->updatePreview();
    return 0;
}

void ThreeD_SceneIllumination_TabPage::updatePreview()
{
    SfxItemSet aItemSet(m_aCtl_Preview.GetSvx3DLightControl().Get3DAttributes());
    LightSourceInfo* pInfo = &m_pLightSourceInfoList[0];

    // AmbientColor
    aItemSet.Put(Svx3DAmbientcolorItem(m_aLB_AmbientLight.GetSelectEntryColor()));

    aItemSet.Put(Svx3DLightcolor1Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff1Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection1Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[1];
    aItemSet.Put(Svx3DLightcolor2Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff2Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection2Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[2];
    aItemSet.Put(Svx3DLightcolor3Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff3Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection3Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[3];
    aItemSet.Put(Svx3DLightcolor4Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff4Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection4Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[4];
    aItemSet.Put(Svx3DLightcolor5Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff5Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection5Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[5];
    aItemSet.Put(Svx3DLightcolor6Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff6Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection6Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[6];
    aItemSet.Put(Svx3DLightcolor7Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff7Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection7Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    pInfo = &m_pLightSourceInfoList[7];
    aItemSet.Put(Svx3DLightcolor8Item(pInfo->aLightSource.nDiffuseColor));
    aItemSet.Put(Svx3DLightOnOff8Item(pInfo->aLightSource.bIsEnabled));
    aItemSet.Put(Svx3DLightDirection8Item(Direction3DToB3DVector(pInfo->aLightSource.aDirection)));

    // set lights and ambient light
    m_aCtl_Preview.GetSvx3DLightControl().Set3DAttributes(aItemSet);

    // select light
    for(sal_uInt32 a(0); a < 8; a++)
    {
        if(m_pLightSourceInfoList[a].pButton->IsChecked())
        {
            m_aCtl_Preview.GetSvx3DLightControl().SelectLight(a);
            m_aCtl_Preview.CheckSelection();
            break;
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
