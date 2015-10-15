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

#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocirc.hxx>
#include <svx/xtextit.hxx>

#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"

#include <svx/dialogs.hrc>
#include <svx/fontwork.hxx>
#include <editeng/outlobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <vcl/settings.hxx>

SFX_IMPL_DOCKINGWINDOW_WITHID( SvxFontWorkChildWindow, SID_FONTWORK );

// ControllerItem for Fontwork

SvxFontWorkControllerItem::SvxFontWorkControllerItem
(
    sal_uInt16 _nId,
    SvxFontWorkDialog& rDlg,
    SfxBindings& rBindings
) :

    SfxControllerItem( _nId, rBindings ),

    rFontWorkDlg( rDlg )
{
}

// StateChanged method for FontWork items

void SvxFontWorkControllerItem::StateChanged( sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
                                              const SfxPoolItem* pItem )
{
    switch ( GetId() )
    {
        case SID_FORMTEXT_STYLE:
        {
            const XFormTextStyleItem* pStateItem =
                                dynamic_cast<const XFormTextStyleItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStyleItem expected");
            rFontWorkDlg.SetStyle_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_ADJUST:
        {
            const XFormTextAdjustItem* pStateItem =
                                dynamic_cast<const XFormTextAdjustItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextAdjustItem expected");
            rFontWorkDlg.SetAdjust_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_DISTANCE:
        {
            const XFormTextDistanceItem* pStateItem =
                                dynamic_cast<const XFormTextDistanceItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextDistanceItem erwartet");
            rFontWorkDlg.SetDistance_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_START:
        {
            const XFormTextStartItem* pStateItem =
                                dynamic_cast<const XFormTextStartItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStartItem expected");
            rFontWorkDlg.SetStart_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_MIRROR:
        {
            const XFormTextMirrorItem* pStateItem =
                                dynamic_cast<const XFormTextMirrorItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextMirrorItem expected");
            rFontWorkDlg.SetMirror_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_HIDEFORM:
        {
            const XFormTextHideFormItem* pStateItem =
                                dynamic_cast<const XFormTextHideFormItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextHideFormItem expected");
            rFontWorkDlg.SetShowForm_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_OUTLINE:
        {
            const XFormTextOutlineItem* pStateItem =
                                dynamic_cast<const XFormTextOutlineItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextOutlineItem expected");
            rFontWorkDlg.SetOutline_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHADOW:
        {
            const XFormTextShadowItem* pStateItem =
                                dynamic_cast<const XFormTextShadowItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowItem expected");
            rFontWorkDlg.SetShadow_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWCOLOR:
        {
            const XFormTextShadowColorItem* pStateItem =
                                dynamic_cast<const XFormTextShadowColorItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowColorItem expected");
            rFontWorkDlg.SetShadowColor_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWXVAL:
        {
            const XFormTextShadowXValItem* pStateItem =
                                dynamic_cast<const XFormTextShadowXValItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowXValItem expected");
            rFontWorkDlg.SetShadowXVal_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWYVAL:
        {
            const XFormTextShadowYValItem* pStateItem =
                                dynamic_cast<const XFormTextShadowYValItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowYValItem expected");
            rFontWorkDlg.SetShadowYVal_Impl(pStateItem);
            break;
        }
    }
}

// Derivation from SfxChildWindow as "containers" for Fontwork dialog

SvxFontWorkChildWindow::SvxFontWorkChildWindow
(
    vcl::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
) :

    SfxChildWindow( _pParent, nId )
{
    VclPtrInstance<SvxFontWorkDialog> pDlg(pBindings, this, _pParent);
    SetWindow(pDlg);

    pDlg->Initialize( pInfo );
}

// Floating Window to the attribution of text effects
SvxFontWorkDialog::SvxFontWorkDialog(SfxBindings *pBindinx,
                                     SfxChildWindow *pCW,
                                     vcl::Window* _pParent)
    : SfxDockingWindow(pBindinx, pCW, _pParent, "DockingFontwork", "svx/ui/dockingfontwork.ui")
    , rBindings(*pBindinx)
    , nLastStyleTbxId(0)
    , nLastAdjustTbxId(0)
    , nLastShadowTbxId(0)
    , nSaveShadowX(0)
    , nSaveShadowY(0)
    , nSaveShadowAngle(450)
    , nSaveShadowSize (100)
{
    get(m_pMtrFldDistance, "distance");
    get(m_pMtrFldTextStart, "indent");
    get(m_pMtrFldShadowX, "distancex");
    get(m_pMtrFldShadowY, "distancey");
    get(m_pShadowColorLB, "color");
    get(m_pFbShadowX, "shadowx");
    get(m_pFbShadowY, "shadowy");

    get(m_pTbxStyle, "style");
    nStyleOffId = m_pTbxStyle->GetItemId(0);
    m_pTbxStyle->SetItemBits(nStyleOffId, ToolBoxItemBits::AUTOCHECK);
    nStyleRotateId = m_pTbxStyle->GetItemId(1);
    m_pTbxStyle->SetItemBits(nStyleRotateId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nStyleUprightId = m_pTbxStyle->GetItemId(2);
    m_pTbxStyle->SetItemBits(nStyleUprightId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nStyleSlantXId = m_pTbxStyle->GetItemId(3);
    m_pTbxStyle->SetItemBits(nStyleSlantXId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nStyleSlantYId = m_pTbxStyle->GetItemId(4);
    m_pTbxStyle->SetItemBits(nStyleSlantYId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);

    get(m_pTbxShadow, "shadow");
    nShowFormId = m_pTbxShadow->GetItemId(0);
    m_pTbxShadow->SetItemBits(nShowFormId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::CHECKABLE);
    nOutlineId = m_pTbxShadow->GetItemId(1);
    m_pTbxShadow->SetItemBits(nOutlineId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nShadowOffId = m_pTbxShadow->GetItemId(2);
    m_pTbxShadow->SetItemBits(nShadowOffId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nShadowNormalId = m_pTbxShadow->GetItemId(3);
    m_pTbxShadow->SetItemBits(nShadowNormalId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nShadowSlantId = m_pTbxShadow->GetItemId(4);
    m_pTbxShadow->SetItemBits(nShadowSlantId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);

    get(m_pTbxAdjust, "adjust");
    nAdjustMirrorId = m_pTbxAdjust->GetItemId(0);
    m_pTbxAdjust->SetItemBits(nAdjustMirrorId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::CHECKABLE);
    nAdjustLeftId = m_pTbxAdjust->GetItemId(1);
    m_pTbxAdjust->SetItemBits(nAdjustLeftId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::CHECKABLE);
    nAdjustCenterId = m_pTbxAdjust->GetItemId(2);
    m_pTbxAdjust->SetItemBits(nAdjustCenterId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nAdjustRightId = m_pTbxAdjust->GetItemId(3);
    m_pTbxAdjust->SetItemBits(nAdjustRightId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);
    nAdjustAutoSizeId = m_pTbxAdjust->GetItemId(4);
    m_pTbxAdjust->SetItemBits(nAdjustAutoSizeId, ToolBoxItemBits::AUTOCHECK|ToolBoxItemBits::RADIOCHECK);

    ApplyImageList();

    pCtrlItems[0] = new SvxFontWorkControllerItem(SID_FORMTEXT_STYLE, *this, rBindings);
    pCtrlItems[1] = new SvxFontWorkControllerItem(SID_FORMTEXT_ADJUST, *this, rBindings);
    pCtrlItems[2] = new SvxFontWorkControllerItem(SID_FORMTEXT_DISTANCE, *this, rBindings);
    pCtrlItems[3] = new SvxFontWorkControllerItem(SID_FORMTEXT_START, *this, rBindings);
    pCtrlItems[4] = new SvxFontWorkControllerItem(SID_FORMTEXT_MIRROR, *this, rBindings);
    pCtrlItems[5] = new SvxFontWorkControllerItem(SID_FORMTEXT_HIDEFORM, *this, rBindings);
    pCtrlItems[6] = new SvxFontWorkControllerItem(SID_FORMTEXT_OUTLINE, *this, rBindings);
    pCtrlItems[7] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHADOW, *this, rBindings);
    pCtrlItems[8] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWCOLOR, *this, rBindings);
    pCtrlItems[9] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWXVAL, *this, rBindings);
    pCtrlItems[10] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWYVAL, *this, rBindings);

    Size aSize = m_pTbxStyle->CalcWindowSizePixel();
    m_pTbxStyle->SetSizePixel(aSize);
    m_pTbxStyle->SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectStyleHdl_Impl) );

    m_pTbxAdjust->SetSizePixel(aSize);
    m_pTbxAdjust->SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectAdjustHdl_Impl) );

    m_pTbxShadow->SetSizePixel(aSize);
    m_pTbxShadow->SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectShadowHdl_Impl) );

    Link<Edit&,void> aLink = LINK(this, SvxFontWorkDialog, ModifyInputHdl_Impl);
    m_pMtrFldDistance->SetModifyHdl( aLink );
    m_pMtrFldTextStart->SetModifyHdl( aLink );
    m_pMtrFldShadowX->SetModifyHdl( aLink );
    m_pMtrFldShadowY->SetModifyHdl( aLink );

    // Set System metric
    const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    SetFieldUnit(*m_pMtrFldDistance, eDlgUnit, true);
    SetFieldUnit(*m_pMtrFldTextStart, eDlgUnit, true);
    SetFieldUnit(*m_pMtrFldShadowX, eDlgUnit, true);
    SetFieldUnit(*m_pMtrFldShadowY, eDlgUnit, true);
    if( eDlgUnit == FUNIT_MM )
    {
        m_pMtrFldDistance->SetSpinSize( 50 );
        m_pMtrFldTextStart->SetSpinSize( 50 );
        m_pMtrFldShadowX->SetSpinSize( 50 );
        m_pMtrFldShadowY->SetSpinSize( 50 );
    }
    else
    {
        m_pMtrFldDistance->SetSpinSize( 10 );
        m_pMtrFldTextStart->SetSpinSize( 10 );
        m_pMtrFldShadowX->SetSpinSize( 10 );
        m_pMtrFldShadowY->SetSpinSize( 10 );
    }

    m_pShadowColorLB->SetSelectHdl( LINK(this, SvxFontWorkDialog, ColorSelectHdl_Impl) );

    aInputIdle.SetPriority(SchedulerPriority::LOWEST);
    aInputIdle.SetIdleHdl(LINK(this, SvxFontWorkDialog, InputTimoutHdl_Impl));
}

SvxFontWorkDialog::~SvxFontWorkDialog()
{
    disposeOnce();
}

void SvxFontWorkDialog::dispose()
{
    for (sal_uInt16 i = 0; i < CONTROLLER_COUNT; i++)
        pCtrlItems[i]->dispose();
    m_pTbxStyle.clear();
    m_pTbxAdjust.clear();
    m_pFbDistance.clear();
    m_pMtrFldDistance.clear();
    m_pFbTextStart.clear();
    m_pMtrFldTextStart.clear();
    m_pTbxShadow.clear();
    m_pFbShadowX.clear();
    m_pMtrFldShadowX.clear();
    m_pFbShadowY.clear();
    m_pMtrFldShadowY.clear();
    m_pShadowColorLB.clear();
    SfxDockingWindow::dispose();
}

SfxChildAlignment SvxFontWorkDialog::CheckAlignment( SfxChildAlignment eActAlign,
                                                     SfxChildAlignment eAlign )
{
    SfxChildAlignment eAlignment;

    switch ( eAlign )
    {
        case SfxChildAlignment::TOP:
        case SfxChildAlignment::HIGHESTTOP:
        case SfxChildAlignment::LOWESTTOP:
        case SfxChildAlignment::BOTTOM:
        case SfxChildAlignment::LOWESTBOTTOM:
        case SfxChildAlignment::HIGHESTBOTTOM:
        {
            eAlignment = eActAlign;
        }
        break;

        case SfxChildAlignment::LEFT:
        case SfxChildAlignment::RIGHT:
        case SfxChildAlignment::FIRSTLEFT:
        case SfxChildAlignment::LASTLEFT:
        case SfxChildAlignment::FIRSTRIGHT:
        case SfxChildAlignment::LASTRIGHT:
        {
            eAlignment = eAlign;
        }
        break;

        default:
        {
            eAlignment = eAlign;
        }
        break;
    }

    return eAlignment;
}

// Set style buttons

void SvxFontWorkDialog::SetStyle_Impl(const XFormTextStyleItem* pItem)
{
    if ( pItem )
    {
        sal_uInt16 nId = nStyleOffId;

        switch ( pItem->GetValue() )
        {
            case XFT_ROTATE : nId = nStyleRotateId;   break;
            case XFT_UPRIGHT: nId = nStyleUprightId;  break;
            case XFT_SLANTX : nId = nStyleSlantXId;   break;
            case XFT_SLANTY : nId = nStyleSlantYId;   break;
            default: ;//prevent warning
        }
        m_pTbxStyle->Enable();

        // Make sure that there is always exactly one checked toolbox item.
        if ( pItem->GetValue() == XFT_NONE )
        {
            m_pTbxStyle->CheckItem(nStyleRotateId, false);
            m_pTbxStyle->CheckItem(nStyleUprightId, false);
            m_pTbxStyle->CheckItem(nStyleSlantXId, false);
            m_pTbxStyle->CheckItem(nStyleSlantYId, false);

            m_pTbxStyle->CheckItem(nStyleOffId);
        }
        else
        {
            m_pTbxStyle->CheckItem(nStyleOffId, false);
            m_pTbxStyle->CheckItem(nId);
        }

        nLastStyleTbxId = nId;
    }
    else
        m_pTbxStyle->Disable();
}

// Set adjust buttons

void SvxFontWorkDialog::SetAdjust_Impl(const XFormTextAdjustItem* pItem)
{
    if ( pItem )
    {
        sal_uInt16 nId;

        m_pTbxAdjust->Enable();
        m_pMtrFldDistance->Enable();

        if ( pItem->GetValue() == XFT_LEFT || pItem->GetValue() == XFT_RIGHT )
        {
            if ( pItem->GetValue() == XFT_LEFT )    nId = nAdjustLeftId;
            else                                    nId = nAdjustRightId;
            m_pMtrFldTextStart->Enable();
        }
        else
        {
            if ( pItem->GetValue() == XFT_CENTER )  nId = nAdjustCenterId;
            else                                    nId = nAdjustAutoSizeId;
            m_pMtrFldTextStart->Disable();
        }

        if ( !m_pTbxAdjust->IsItemChecked(nId) )
        {
            m_pTbxAdjust->CheckItem(nId);
        }
        nLastAdjustTbxId = nId;
    }
    else
    {
        m_pTbxAdjust->Disable();
        m_pMtrFldTextStart->Disable();
        m_pMtrFldDistance->Disable();
    }
}

// Enter Distance value in the edit field

void SvxFontWorkDialog::SetDistance_Impl(const XFormTextDistanceItem* pItem)
{
    // Use HasChildPathFocus() instead of HasFocus() at SpinFields
    if ( pItem && !m_pMtrFldDistance->HasChildPathFocus() )
    {
        SetMetricValue(*m_pMtrFldDistance, pItem->GetValue(), SFX_MAPUNIT_100TH_MM);
    }
}

// Enter indent value in the edit field

void SvxFontWorkDialog::SetStart_Impl(const XFormTextStartItem* pItem)
{
    // Use HasChildPathFocus() instead of HasFocus() at SpinFields
    if ( pItem && !m_pMtrFldTextStart->HasChildPathFocus() )
    {
        SetMetricValue(*m_pMtrFldTextStart, pItem->GetValue(), SFX_MAPUNIT_100TH_MM);
    }
}

// Set button for reversing the direction of text

void SvxFontWorkDialog::SetMirror_Impl(const XFormTextMirrorItem* pItem)
{
    if ( pItem )
        m_pTbxAdjust->CheckItem(nAdjustMirrorId, pItem->GetValue());
}

// Set button for contour display

void SvxFontWorkDialog::SetShowForm_Impl(const XFormTextHideFormItem* pItem)
{
    if ( pItem )
        m_pTbxShadow->CheckItem(nShowFormId, !pItem->GetValue());
}

// Set button for text border

void SvxFontWorkDialog::SetOutline_Impl(const XFormTextOutlineItem* pItem)
{
    if ( pItem )
        m_pTbxShadow->CheckItem(nOutlineId, pItem->GetValue());
}

// Set shadow buttons

void SvxFontWorkDialog::SetShadow_Impl(const XFormTextShadowItem* pItem,
                                        bool bRestoreValues)
{
    if ( pItem )
    {
        sal_uInt16 nId;

        m_pTbxShadow->Enable();

        if ( pItem->GetValue() == XFTSHADOW_NONE )
        {
            nId = nShadowOffId;
            m_pFbShadowX->Hide();
            m_pFbShadowY->Hide();
            m_pMtrFldShadowX->Disable();
            m_pMtrFldShadowY->Disable();
            m_pShadowColorLB->Disable();
        }
        else
        {
            m_pFbShadowX->Show();
            m_pFbShadowY->Show();
            m_pMtrFldShadowX->Enable();
            m_pMtrFldShadowY->Enable();
            m_pShadowColorLB->Enable();

            if ( pItem->GetValue() == XFTSHADOW_NORMAL )
            {
                nId = nShadowNormalId;
                const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();

                m_pMtrFldShadowX->SetUnit( eDlgUnit );
                m_pMtrFldShadowX->SetDecimalDigits(2);
                m_pMtrFldShadowX->SetMin(LONG_MIN);
                m_pMtrFldShadowX->SetMax(LONG_MAX);
                if( eDlgUnit == FUNIT_MM )
                    m_pMtrFldShadowX->SetSpinSize( 50 );
                else
                    m_pMtrFldShadowX->SetSpinSize( 10 );

                m_pMtrFldShadowY->SetUnit( eDlgUnit );
                m_pMtrFldShadowY->SetDecimalDigits(2);
                m_pMtrFldShadowY->SetMin(LONG_MIN);
                m_pMtrFldShadowY->SetMax(LONG_MAX);
                if( eDlgUnit == FUNIT_MM )
                    m_pMtrFldShadowY->SetSpinSize( 50 );
                else
                    m_pMtrFldShadowY->SetSpinSize( 10 );

                if ( bRestoreValues )
                {
                    SetMetricValue(*m_pMtrFldShadowX, nSaveShadowX, SFX_MAPUNIT_100TH_MM);
                    SetMetricValue(*m_pMtrFldShadowY, nSaveShadowY, SFX_MAPUNIT_100TH_MM);

                    XFormTextShadowXValItem aXItem( nSaveShadowX );
                    XFormTextShadowYValItem aYItem( nSaveShadowY );

                    GetBindings().GetDispatcher()->Execute(
                        SID_FORMTEXT_SHDWXVAL, SfxCallMode::RECORD, &aXItem, &aYItem, 0L );
                }
            }
            else
            {
                nId = nShadowSlantId;

                m_pMtrFldShadowX->SetUnit(FUNIT_DEGREE);
                m_pMtrFldShadowX->SetDecimalDigits(1);
                m_pMtrFldShadowX->SetMin(-1800);
                m_pMtrFldShadowX->SetMax( 1800);
                m_pMtrFldShadowX->SetSpinSize(10);

                m_pMtrFldShadowY->SetUnit(FUNIT_PERCENT);
                m_pMtrFldShadowY->SetDecimalDigits(0);
                m_pMtrFldShadowY->SetMin(-999);
                m_pMtrFldShadowY->SetMax( 999);
                m_pMtrFldShadowY->SetSpinSize(10);

                if ( bRestoreValues )
                {
                    m_pMtrFldShadowX->SetValue(nSaveShadowAngle);
                    m_pMtrFldShadowY->SetValue(nSaveShadowSize);
                    XFormTextShadowXValItem aXItem(nSaveShadowAngle);
                    XFormTextShadowYValItem aYItem(nSaveShadowSize);
                    GetBindings().GetDispatcher()->Execute(
                        SID_FORMTEXT_SHDWXVAL, SfxCallMode::RECORD, &aXItem, &aYItem, 0L );
                }
            }
        }

        if ( !m_pTbxShadow->IsItemChecked(nId) )
        {
            m_pTbxShadow->CheckItem(nId);
        }
        nLastShadowTbxId = nId;

        ApplyImageList();
    }
    else
    {
        m_pTbxShadow->Disable();
        m_pMtrFldShadowX->Disable();
        m_pMtrFldShadowY->Disable();
        m_pShadowColorLB->Disable();
    }
}

// Insert shadow color in listbox

void SvxFontWorkDialog::SetShadowColor_Impl(const XFormTextShadowColorItem* pItem)
{
    if ( pItem )
        m_pShadowColorLB->SelectEntry(pItem->GetColorValue());
}

// Enter X-value for shadow in edit field

void SvxFontWorkDialog::SetShadowXVal_Impl(const XFormTextShadowXValItem* pItem)
{
    // Use HasChildPathFocus() instead of HasFocus() at SpinFields
    if ( pItem && !m_pMtrFldShadowX->HasChildPathFocus() )
    {
        // #i19251#
        // sal_Int32 nValue = pItem->GetValue();

        // #i19251#
        // The two involved fields/items are used double and contain/give different
        // values regarding to the access method. Thus, here we need to separate the access
        // methos regarding to the kind of value accessed.
        if (m_pTbxShadow->IsItemChecked(nShadowSlantId))
        {
            // #i19251#
            // There is no value correction necessary at all, i think this
            // was only tried to be done without understanding that the two
            // involved fields/items are used double and contain/give different
            // values regarding to the access method.
            // nValue = nValue - ( int( float( nValue ) / 360.0 ) * 360 );
            m_pMtrFldShadowX->SetValue(pItem->GetValue());
        }
        else
        {
            SetMetricValue(*m_pMtrFldShadowX, pItem->GetValue(), SFX_MAPUNIT_100TH_MM);
        }
    }
}

// Enter Y-value for shadow in edit field

void SvxFontWorkDialog::SetShadowYVal_Impl(const XFormTextShadowYValItem* pItem)
{
    // Use HasChildPathFocus() instead of HasFocus() at SpinFields
    if ( pItem && !m_pMtrFldShadowY->HasChildPathFocus() )
    {
        // #i19251#
        // The two involved fields/items are used double and contain/give different
        // values regarding to the access method. Thus, here we need to separate the access
        // methos regarding to the kind of value accessed.
        if (m_pTbxShadow->IsItemChecked(nShadowSlantId))
        {
            m_pMtrFldShadowY->SetValue(pItem->GetValue());
        }
        else
        {
            SetMetricValue(*m_pMtrFldShadowY, pItem->GetValue(), SFX_MAPUNIT_100TH_MM);
        }
    }
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, SelectStyleHdl_Impl, ToolBox *, void)
{
    sal_uInt16 nId = m_pTbxStyle->GetCurItemId();

    // Execute this block when a different toolbox item has been clicked or
    // when the off item has been clicked.  The later is necessary to
    // override the toolbox behaviour of unchecking the item after second
    // click on it: One of the items has to be checked at all times (when
    // enabled that is.)
    if (nId == nStyleOffId || nId != nLastStyleTbxId )
    {
        XFormTextStyle eStyle = XFT_NONE;

        if (nId == nStyleRotateId)
            eStyle = XFT_ROTATE;
        else if (nId == nStyleUprightId)
            eStyle = XFT_UPRIGHT;
        else if (nId == nStyleSlantXId)
            eStyle = XFT_SLANTX;
        else if (nId == nStyleSlantYId)
            eStyle = XFT_SLANTY;

        XFormTextStyleItem aItem( eStyle );
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_STYLE, SfxCallMode::RECORD, &aItem, 0L );
        SetStyle_Impl( &aItem );
        nLastStyleTbxId = nId;
    }
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, SelectAdjustHdl_Impl, ToolBox *, void)
{
    sal_uInt16 nId = m_pTbxAdjust->GetCurItemId();

    if (nId == nAdjustMirrorId)
    {
        XFormTextMirrorItem aItem(m_pTbxAdjust->IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_MIRROR, SfxCallMode::SLOT, &aItem, 0L );
    }
    else if ( nId != nLastAdjustTbxId )
    {
        XFormTextAdjust eAdjust = XFT_AUTOSIZE;

        if (nId == nAdjustLeftId)
            eAdjust = XFT_LEFT;
        else if (nId == nAdjustCenterId)
            eAdjust = XFT_CENTER;
        else if (nId == nAdjustRightId)
            eAdjust = XFT_RIGHT;

        XFormTextAdjustItem aItem(eAdjust);
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_ADJUST, SfxCallMode::RECORD, &aItem, 0L );
        SetAdjust_Impl(&aItem);
        nLastAdjustTbxId = nId;
    }
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, SelectShadowHdl_Impl, ToolBox *, void)
{
    sal_uInt16 nId = m_pTbxShadow->GetCurItemId();

    if (nId == nShowFormId)
    {
        XFormTextHideFormItem aItem(!m_pTbxShadow->IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_HIDEFORM, SfxCallMode::RECORD, &aItem, 0L );
    }
    else if (nId == nOutlineId)
    {
        XFormTextOutlineItem aItem(m_pTbxShadow->IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_OUTLINE, SfxCallMode::RECORD, &aItem, 0L );
    }
    else if (nId != nLastShadowTbxId)
    {
        XFormTextShadow eShadow = XFTSHADOW_NONE;

        if (nLastShadowTbxId == nShadowNormalId)
        {
            nSaveShadowX = GetCoreValue(*m_pMtrFldShadowX, SFX_MAPUNIT_100TH_MM);
            nSaveShadowY = GetCoreValue(*m_pMtrFldShadowY, SFX_MAPUNIT_100TH_MM);
        }
        else if (nLastShadowTbxId == nShadowSlantId)
        {
            nSaveShadowAngle = static_cast<long>(m_pMtrFldShadowX->GetValue());
            nSaveShadowSize  = static_cast<long>(m_pMtrFldShadowY->GetValue());
        }
        nLastShadowTbxId = nId;

        if ( nId == nShadowNormalId)     eShadow = XFTSHADOW_NORMAL;
        else if (nId == nShadowSlantId) eShadow = XFTSHADOW_SLANT;

        XFormTextShadowItem aItem(eShadow);
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHADOW, SfxCallMode::RECORD, &aItem, 0L );
        SetShadow_Impl(&aItem, true);
    }
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, ModifyInputHdl_Impl, Edit&, void)
{
    aInputIdle.Start();
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, InputTimoutHdl_Impl, Idle *, void)
{
    // Possibly set the Metric system again. This should be done with a
    // listen, this is however not possible at the moment due to compabillity
    // issues.
    const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    if( eDlgUnit != m_pMtrFldDistance->GetUnit() )
    {
        SetFieldUnit(*m_pMtrFldDistance, eDlgUnit, true);
        SetFieldUnit(*m_pMtrFldTextStart, eDlgUnit, true);
        m_pMtrFldDistance->SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
        m_pMtrFldTextStart->SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
    }
    if( eDlgUnit != m_pMtrFldShadowX->GetUnit() &&
        m_pTbxShadow->IsItemChecked(nShadowNormalId) )
    {
        SetFieldUnit(*m_pMtrFldShadowX, eDlgUnit, true);
        SetFieldUnit(*m_pMtrFldShadowY, eDlgUnit, true);
        m_pMtrFldShadowX->SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
        m_pMtrFldShadowY->SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
    }

    long nValue = GetCoreValue(*m_pMtrFldDistance, SFX_MAPUNIT_100TH_MM);
    XFormTextDistanceItem aDistItem( nValue );
    nValue = GetCoreValue(*m_pMtrFldTextStart, SFX_MAPUNIT_100TH_MM);
    XFormTextStartItem aStartItem( nValue );

    sal_Int32 nValueX(0L);
    sal_Int32 nValueY(0L);

    // #i19251#
    // The two involved fields/items are used double and contain/give different
    // values regarding to the access method. Thus, here we need to separate the access
    // methos regarding to the kind of value accessed.
    if (nLastShadowTbxId == nShadowNormalId)
    {
        nValueX = GetCoreValue(*m_pMtrFldShadowX, SFX_MAPUNIT_100TH_MM);
        nValueY = GetCoreValue(*m_pMtrFldShadowY, SFX_MAPUNIT_100TH_MM);
    }
    else if (nLastShadowTbxId == nShadowSlantId)
    {
        nValueX = static_cast<long>(m_pMtrFldShadowX->GetValue());
        nValueY = static_cast<long>(m_pMtrFldShadowY->GetValue());
    }

    XFormTextShadowXValItem aShadowXItem( nValueX );
    XFormTextShadowYValItem aShadowYItem( nValueY );

    // Slot-ID does not matter, the Exec method evaluates the entire item set
    GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_DISTANCE, SfxCallMode::RECORD, &aDistItem,
                                            &aStartItem, &aShadowXItem, &aShadowYItem, 0L );
}

IMPL_LINK_NOARG_TYPED(SvxFontWorkDialog, ColorSelectHdl_Impl, ListBox&, void)
{
    XFormTextShadowColorItem aItem( "", m_pShadowColorLB->GetSelectEntryColor() );
    GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHDWCOLOR, SfxCallMode::RECORD, &aItem, 0L );
}

void SvxFontWorkDialog::SetColorList(const XColorListRef &pList)
{
    if ( pList.is() && pList != pColorList )
    {
        pColorList = pList;
        m_pShadowColorLB->Clear();
        m_pShadowColorLB->Fill(pColorList);
    }
}

void SvxFontWorkDialog::ApplyImageList()
{
    if (nLastShadowTbxId == nShadowSlantId)
    {
        m_pFbShadowX->SetImage(Image(SVX_RES(RID_SVXIMG_SHADOW_ANGLE)));
        m_pFbShadowY->SetImage(Image(SVX_RES(RID_SVXIMG_SHADOW_SIZE)));
    }
    else
    {
        m_pFbShadowX->SetImage(Image(SVX_RES(RID_SVXIMG_SHADOW_XDIST)));
        m_pFbShadowY->SetImage(Image(SVX_RES(RID_SVXIMG_SHADOW_YDIST)));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
