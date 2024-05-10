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

#include <svx/colorbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xftadit.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftshxy.hxx>
#include <svx/xtextit0.hxx>

#include <svtools/unitconv.hxx>
#include <svx/svxids.hrc>
#include <bitmaps.hlst>
#include <svx/fontwork.hxx>
#include <svl/itemset.hxx>

#define WIDTH_CHARS 10

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

void SvxFontWorkControllerItem::StateChangedAtToolBoxControl( sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
                                              const SfxPoolItem* pItem )
{
    switch ( GetId() )
    {
        case SID_FORMTEXT_STYLE:
        {
            const XFormTextStyleItem* pStateItem =
                                dynamic_cast<const XFormTextStyleItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextStyleItem expected");
            rFontWorkDlg.SetStyle_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_ADJUST:
        {
            const XFormTextAdjustItem* pStateItem =
                                dynamic_cast<const XFormTextAdjustItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextAdjustItem expected");
            rFontWorkDlg.SetAdjust_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_DISTANCE:
        {
            const XFormTextDistanceItem* pStateItem =
                                dynamic_cast<const XFormTextDistanceItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextDistanceItem expected");
            rFontWorkDlg.SetDistance_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_START:
        {
            const XFormTextStartItem* pStateItem =
                                dynamic_cast<const XFormTextStartItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextStartItem expected");
            rFontWorkDlg.SetStart_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_MIRROR:
        {
            const XFormTextMirrorItem* pStateItem =
                                dynamic_cast<const XFormTextMirrorItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextMirrorItem expected");
            rFontWorkDlg.SetMirror_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_HIDEFORM:
        {
            const XFormTextHideFormItem* pStateItem =
                                dynamic_cast<const XFormTextHideFormItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextHideFormItem expected");
            rFontWorkDlg.SetShowForm_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_OUTLINE:
        {
            const XFormTextOutlineItem* pStateItem =
                                dynamic_cast<const XFormTextOutlineItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextOutlineItem expected");
            rFontWorkDlg.SetOutline_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHADOW:
        {
            const XFormTextShadowItem* pStateItem =
                                dynamic_cast<const XFormTextShadowItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextShadowItem expected");
            rFontWorkDlg.SetShadow_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWCOLOR:
        {
            const XFormTextShadowColorItem* pStateItem =
                                dynamic_cast<const XFormTextShadowColorItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextShadowColorItem expected");
            rFontWorkDlg.SetShadowColor_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWXVAL:
        {
            const XFormTextShadowXValItem* pStateItem =
                                dynamic_cast<const XFormTextShadowXValItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextShadowXValItem expected");
            rFontWorkDlg.SetShadowXVal_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWYVAL:
        {
            const XFormTextShadowYValItem* pStateItem =
                                dynamic_cast<const XFormTextShadowYValItem*>( pItem );
            DBG_ASSERT(pStateItem || pItem == nullptr, "XFormTextShadowYValItem expected");
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
    : SfxDockingWindow(pBindinx, pCW, _pParent, u"DockingFontwork"_ustr, u"svx/ui/dockingfontwork.ui"_ustr)
    , rBindings(*pBindinx)
    , aInputIdle("SvxFontWorkDialog Input")
    , nSaveShadowX(0)
    , nSaveShadowY(0)
    , nSaveShadowAngle(450)
    , nSaveShadowSize (100)
    , m_xTbxStyle(m_xBuilder->weld_toolbar(u"style"_ustr))
    , m_xTbxAdjust(m_xBuilder->weld_toolbar(u"adjust"_ustr))
    , m_xMtrFldDistance(m_xBuilder->weld_metric_spin_button(u"distance"_ustr, FieldUnit::CM))
    , m_xMtrFldTextStart(m_xBuilder->weld_metric_spin_button(u"indent"_ustr, FieldUnit::CM))
    , m_xTbxShadow(m_xBuilder->weld_toolbar(u"shadow"_ustr))
    , m_xFbShadowX(m_xBuilder->weld_image(u"shadowx"_ustr))
    , m_xMtrFldShadowX(m_xBuilder->weld_metric_spin_button(u"distancex"_ustr, FieldUnit::CM))
    , m_xFbShadowY(m_xBuilder->weld_image(u"shadowy"_ustr))
    , m_xMtrFldShadowY(m_xBuilder->weld_metric_spin_button(u"distancey"_ustr, FieldUnit::CM))
    , m_xShadowColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"color"_ustr), [this]{ return GetFrameWeld(); } ))
{
    SetText(SvxResId(RID_SVXSTR_FONTWORK));

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

    m_xTbxStyle->connect_clicked(LINK(this, SvxFontWorkDialog, SelectStyleHdl_Impl));
    m_xTbxAdjust->connect_clicked(LINK(this, SvxFontWorkDialog, SelectAdjustHdl_Impl));
    m_xTbxShadow->connect_clicked(LINK(this, SvxFontWorkDialog, SelectShadowHdl_Impl));

    Link<weld::MetricSpinButton&,void> aLink = LINK(this, SvxFontWorkDialog, ModifyInputHdl_Impl);
    m_xMtrFldDistance->connect_value_changed( aLink );
    m_xMtrFldTextStart->connect_value_changed( aLink );
    m_xMtrFldShadowX->connect_value_changed( aLink );
    m_xMtrFldShadowY->connect_value_changed( aLink );

    // Set System metric
    const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    SetFieldUnit(*m_xMtrFldDistance, eDlgUnit, true);
    SetFieldUnit(*m_xMtrFldTextStart, eDlgUnit, true);
    SetFieldUnit(*m_xMtrFldShadowX, eDlgUnit, true);
    SetFieldUnit(*m_xMtrFldShadowY, eDlgUnit, true);
    if( eDlgUnit == FieldUnit::MM )
    {
        m_xMtrFldDistance->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldTextStart->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldShadowX->set_increments(50, 500, FieldUnit::NONE);
        m_xMtrFldShadowY->set_increments(50, 500, FieldUnit::NONE);
    }
    else
    {
        m_xMtrFldDistance->set_increments(10, 100, FieldUnit::NONE);
        m_xMtrFldTextStart->set_increments(10, 100, FieldUnit::NONE);
        m_xMtrFldShadowX->set_increments(10, 100, FieldUnit::NONE);
        m_xMtrFldShadowY->set_increments(10, 100, FieldUnit::NONE);
    }
    m_xMtrFldShadowX->set_width_chars(WIDTH_CHARS);
    m_xMtrFldShadowY->set_width_chars(WIDTH_CHARS);

    m_xShadowColorLB->SetSelectHdl( LINK(this, SvxFontWorkDialog, ColorSelectHdl_Impl) );

    aInputIdle.SetPriority(TaskPriority::LOWEST);
    aInputIdle.SetInvokeHandler(LINK(this, SvxFontWorkDialog, InputTimeoutHdl_Impl));
}

SvxFontWorkDialog::~SvxFontWorkDialog()
{
    disposeOnce();
}

void SvxFontWorkDialog::dispose()
{
    for (SvxFontWorkControllerItem* pCtrlItem : pCtrlItems)
        pCtrlItem->dispose();
    m_xTbxStyle.reset();
    m_xTbxAdjust.reset();
    m_xMtrFldDistance.reset();
    m_xMtrFldTextStart.reset();
    m_xTbxShadow.reset();
    m_xFbShadowX.reset();
    m_xMtrFldShadowX.reset();
    m_xFbShadowY.reset();
    m_xMtrFldShadowY.reset();
    m_xShadowColorLB.reset();
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
        OUString sId = u"off"_ustr;

        switch ( pItem->GetValue() )
        {
            case XFormTextStyle::Rotate : sId = "rotate";   break;
            case XFormTextStyle::Upright: sId = "upright";  break;
            case XFormTextStyle::SlantX : sId = "hori";   break;
            case XFormTextStyle::SlantY : sId = "vert";   break;
            default: ;//prevent warning
        }
        m_xTbxStyle->set_sensitive(true);

        // Make sure that there is always exactly one checked toolbox item.
        if ( pItem->GetValue() == XFormTextStyle::NONE )
        {
            m_xTbxStyle->set_item_active(u"rotate"_ustr, false);
            m_xTbxStyle->set_item_active(u"upright"_ustr, false);
            m_xTbxStyle->set_item_active(u"hori"_ustr, false);
            m_xTbxStyle->set_item_active(u"vert"_ustr, false);

            m_xTbxStyle->set_item_active(u"off"_ustr, true);
        }
        else
        {
            m_xTbxStyle->set_item_active(u"off"_ustr, false);
            m_xTbxStyle->set_item_active(sId, true);
        }

        m_sLastStyleTbxId = sId;
    }
    else
        m_xTbxStyle->set_sensitive(false);
}

// Set adjust buttons

void SvxFontWorkDialog::SetAdjust_Impl(const XFormTextAdjustItem* pItem)
{
    if ( pItem )
    {
        OUString sId;

        m_xTbxAdjust->set_sensitive(true);
        m_xMtrFldDistance->set_sensitive(true);

        if ( pItem->GetValue() == XFormTextAdjust::Left || pItem->GetValue() == XFormTextAdjust::Right )
        {
            if (pItem->GetValue() == XFormTextAdjust::Left)
                sId = "left";
            else
                sId = "right";
            m_xMtrFldTextStart->set_sensitive(true);
        }
        else
        {
            if (pItem->GetValue() == XFormTextAdjust::Center)
                sId = "center";
            else
                sId = "autosize";
            m_xMtrFldTextStart->set_sensitive(false);
        }

        if (!m_xTbxAdjust->get_item_active(sId))
            m_xTbxAdjust->set_item_active(sId, true);

        m_sLastAdjustTbxId = sId;
    }
    else
    {
        m_xTbxAdjust->set_sensitive(false);
        m_xMtrFldTextStart->set_sensitive(false);
        m_xMtrFldDistance->set_sensitive(false);
    }
}

// Enter Distance value in the edit field

void SvxFontWorkDialog::SetDistance_Impl(const XFormTextDistanceItem* pItem)
{
    if (pItem && !m_xMtrFldDistance->has_focus())
    {
        SetMetricValue(*m_xMtrFldDistance, pItem->GetValue(), MapUnit::Map100thMM);
    }
}

// Enter indent value in the edit field

void SvxFontWorkDialog::SetStart_Impl(const XFormTextStartItem* pItem)
{
    if (pItem && !m_xMtrFldTextStart->has_focus())
    {
        SetMetricValue(*m_xMtrFldTextStart, pItem->GetValue(), MapUnit::Map100thMM);
    }
}

// Set button for reversing the direction of text

void SvxFontWorkDialog::SetMirror_Impl(const XFormTextMirrorItem* pItem)
{
    if ( pItem )
        m_xTbxAdjust->set_item_active(u"orientation"_ustr, pItem->GetValue());
}

// Set button for contour display

void SvxFontWorkDialog::SetShowForm_Impl(const XFormTextHideFormItem* pItem)
{
    if ( pItem )
        m_xTbxShadow->set_item_active(u"contour"_ustr, !pItem->GetValue());
}

// Set button for text border

void SvxFontWorkDialog::SetOutline_Impl(const XFormTextOutlineItem* pItem)
{
    if ( pItem )
        m_xTbxShadow->set_item_active(u"textcontour"_ustr, pItem->GetValue());
}

// Set shadow buttons

void SvxFontWorkDialog::SetShadow_Impl(const XFormTextShadowItem* pItem,
                                        bool bRestoreValues)
{
    if ( pItem )
    {
        OUString sId;

        m_xTbxShadow->set_sensitive(true);

        if ( pItem->GetValue() == XFormTextShadow::NONE )
        {
            sId = "noshadow";
            m_xFbShadowX->hide();
            m_xFbShadowY->hide();
            m_xMtrFldShadowX->set_sensitive(false);
            m_xMtrFldShadowY->set_sensitive(false);
            m_xShadowColorLB->set_sensitive(false);
        }
        else
        {
            m_xFbShadowX->show();
            m_xFbShadowY->show();
            m_xMtrFldShadowX->set_sensitive(true);
            m_xMtrFldShadowY->set_sensitive(true);
            m_xShadowColorLB->set_sensitive(true);

            if ( pItem->GetValue() == XFormTextShadow::Normal )
            {
                sId = "vertical";
                const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();

                m_xMtrFldShadowX->set_unit( eDlgUnit );
                m_xMtrFldShadowX->set_digits(2);
                m_xMtrFldShadowX->set_range(INT_MIN, INT_MAX, FieldUnit::NONE);
                if( eDlgUnit == FieldUnit::MM )
                    m_xMtrFldShadowX->set_increments(50, 500, FieldUnit::NONE);
                else
                    m_xMtrFldShadowX->set_increments(10, 100, FieldUnit::NONE);
                m_xMtrFldShadowX->set_width_chars(WIDTH_CHARS);

                m_xMtrFldShadowY->set_unit( eDlgUnit );
                m_xMtrFldShadowY->set_digits(2);
                m_xMtrFldShadowY->set_range(INT_MIN, INT_MAX, FieldUnit::NONE);
                if( eDlgUnit == FieldUnit::MM )
                    m_xMtrFldShadowY->set_increments(50, 500, FieldUnit::NONE);
                else
                    m_xMtrFldShadowY->set_increments(10, 100, FieldUnit::NONE);
                m_xMtrFldShadowY->set_width_chars(WIDTH_CHARS);

                if ( bRestoreValues )
                {
                    SetMetricValue(*m_xMtrFldShadowX, nSaveShadowX, MapUnit::Map100thMM);
                    SetMetricValue(*m_xMtrFldShadowY, nSaveShadowY, MapUnit::Map100thMM);

                    XFormTextShadowXValItem aXItem( nSaveShadowX );
                    XFormTextShadowYValItem aYItem( nSaveShadowY );

                    GetBindings().GetDispatcher()->ExecuteList(
                        SID_FORMTEXT_SHDWXVAL, SfxCallMode::RECORD,
                        { &aXItem, &aYItem });
                }
            }
            else
            {
                sId = "slant";

                m_xMtrFldShadowX->set_unit(FieldUnit::DEGREE);
                m_xMtrFldShadowX->set_digits(1);
                m_xMtrFldShadowX->set_range(-1800, 1800, FieldUnit::NONE);
                m_xMtrFldShadowX->set_increments(10, 100, FieldUnit::NONE);
                m_xMtrFldShadowX->set_width_chars(WIDTH_CHARS);

                m_xMtrFldShadowY->set_unit(FieldUnit::PERCENT);
                m_xMtrFldShadowY->set_digits(0);
                m_xMtrFldShadowY->set_range(-999, 999, FieldUnit::NONE);
                m_xMtrFldShadowY->set_increments(10, 100, FieldUnit::NONE);
                m_xMtrFldShadowY->set_width_chars(WIDTH_CHARS);

                if ( bRestoreValues )
                {
                    m_xMtrFldShadowX->set_value(nSaveShadowAngle, FieldUnit::NONE);
                    m_xMtrFldShadowY->set_value(nSaveShadowSize, FieldUnit::NONE);
                    XFormTextShadowXValItem aXItem(nSaveShadowAngle);
                    XFormTextShadowYValItem aYItem(nSaveShadowSize);
                    GetBindings().GetDispatcher()->ExecuteList(
                        SID_FORMTEXT_SHDWXVAL, SfxCallMode::RECORD,
                        { &aXItem, &aYItem });
                }
            }
        }

        if (!m_xTbxShadow->get_item_active(sId))
            m_xTbxShadow->set_item_active(sId, true);
        m_sLastShadowTbxId = sId;

        ApplyImageList();
    }
    else
    {
        m_xTbxShadow->set_sensitive(false);
        m_xMtrFldShadowX->set_sensitive(false);
        m_xMtrFldShadowY->set_sensitive(false);
        m_xShadowColorLB->set_sensitive(false);
    }
}

// Insert shadow color in listbox

void SvxFontWorkDialog::SetShadowColor_Impl(const XFormTextShadowColorItem* pItem)
{
    if ( pItem )
        m_xShadowColorLB->SelectEntry(pItem->GetColorValue());
}

// Enter X-value for shadow in edit field
void SvxFontWorkDialog::SetShadowXVal_Impl(const XFormTextShadowXValItem* pItem)
{
    if (!pItem || m_xMtrFldShadowX->has_focus())
        return;

    // #i19251#
    // sal_Int32 nValue = pItem->GetValue();

    // #i19251#
    // The two involved fields/items are used double and contain/give different
    // values regarding to the access method. Thus, here we need to separate the access
    // methods regarding to the kind of value accessed.
    if (m_xTbxShadow->get_item_active(u"slant"_ustr))
    {
        // #i19251#
        // There is no value correction necessary at all, i think this
        // was only tried to be done without understanding that the two
        // involved fields/items are used double and contain/give different
        // values regarding to the access method.
        // nValue = nValue - ( int( float( nValue ) / 360.0 ) * 360 );
        m_xMtrFldShadowX->set_value(pItem->GetValue(), FieldUnit::NONE);
    }
    else
    {
        SetMetricValue(*m_xMtrFldShadowX, pItem->GetValue(), MapUnit::Map100thMM);
    }
}

// Enter Y-value for shadow in edit field
void SvxFontWorkDialog::SetShadowYVal_Impl(const XFormTextShadowYValItem* pItem)
{
    if (!pItem || m_xMtrFldShadowY->has_focus())
        return;

    // #i19251#
    // The two involved fields/items are used double and contain/give different
    // values regarding to the access method. Thus, here we need to separate the access
    // methods regarding to the kind of value accessed.
    if (m_xTbxShadow->get_item_active(u"slant"_ustr))
    {
        m_xMtrFldShadowY->set_value(pItem->GetValue(), FieldUnit::NONE);
    }
    else
    {
        SetMetricValue(*m_xMtrFldShadowY, pItem->GetValue(), MapUnit::Map100thMM);
    }
}

IMPL_LINK(SvxFontWorkDialog, SelectStyleHdl_Impl, const OUString&, rId, void)
{
    // Execute this block when a different toolbox item has been clicked or
    // when the off item has been clicked.  The later is necessary to
    // override the toolbox behaviour of unchecking the item after second
    // click on it: One of the items has to be checked at all times (when
    // enabled that is.)
    if (rId != "off" && rId == m_sLastStyleTbxId)
        return;

    XFormTextStyle eStyle = XFormTextStyle::NONE;

    if (rId == "rotate")
        eStyle = XFormTextStyle::Rotate;
    else if (rId == "upright")
        eStyle = XFormTextStyle::Upright;
    else if (rId == "hori")
        eStyle = XFormTextStyle::SlantX;
    else if (rId == "vert")
        eStyle = XFormTextStyle::SlantY;

    XFormTextStyleItem aItem( eStyle );
    GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_STYLE,
            SfxCallMode::RECORD, { &aItem });
    SetStyle_Impl( &aItem );
    m_sLastStyleTbxId = rId;
}

IMPL_LINK(SvxFontWorkDialog, SelectAdjustHdl_Impl, const OUString&, rId, void)
{
    if (rId == "orientation")
    {
        XFormTextMirrorItem aItem(m_xTbxAdjust->get_item_active(rId));
        GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_MIRROR,
                SfxCallMode::SLOT, { &aItem });
    }
    else if (rId != m_sLastAdjustTbxId)
    {
        XFormTextAdjust eAdjust = XFormTextAdjust::AutoSize;

        if (rId == "left")
            eAdjust = XFormTextAdjust::Left;
        else if (rId == "center")
            eAdjust = XFormTextAdjust::Center;
        else if (rId == "right")
            eAdjust = XFormTextAdjust::Right;

        XFormTextAdjustItem aItem(eAdjust);
        GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_ADJUST,
                SfxCallMode::RECORD, { &aItem });
        SetAdjust_Impl(&aItem);
        m_sLastAdjustTbxId = rId;
    }
}

IMPL_LINK(SvxFontWorkDialog, SelectShadowHdl_Impl, const OUString&, rId, void)
{
    if (rId == "contour")
    {
        XFormTextHideFormItem aItem(!m_xTbxShadow->get_item_active(rId));
        GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_HIDEFORM,
                SfxCallMode::RECORD, { &aItem });
    }
    else if (rId == "textcontour")
    {
        XFormTextOutlineItem aItem(m_xTbxShadow->get_item_active(rId));
        GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_OUTLINE,
                SfxCallMode::RECORD, { &aItem });
    }
    else if (rId != m_sLastShadowTbxId)
    {
        XFormTextShadow eShadow = XFormTextShadow::NONE;

        if (m_sLastShadowTbxId == "vertical")
        {
            nSaveShadowX = GetCoreValue(*m_xMtrFldShadowX, MapUnit::Map100thMM);
            nSaveShadowY = GetCoreValue(*m_xMtrFldShadowY, MapUnit::Map100thMM);
        }
        else if (m_sLastShadowTbxId == "slant")
        {
            nSaveShadowAngle = m_xMtrFldShadowX->get_value(FieldUnit::NONE);
            nSaveShadowSize  = m_xMtrFldShadowY->get_value(FieldUnit::NONE);
        }
        m_sLastShadowTbxId = rId;

        if ( rId == "vertical")  eShadow = XFormTextShadow::Normal;
        else if (rId == "slant") eShadow = XFormTextShadow::Slant;

        XFormTextShadowItem aItem(eShadow);
        GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_SHADOW,
                SfxCallMode::RECORD, { &aItem });
        SetShadow_Impl(&aItem, true);
    }
}

IMPL_LINK_NOARG(SvxFontWorkDialog, ModifyInputHdl_Impl, weld::MetricSpinButton&, void)
{
    aInputIdle.Start();
}

IMPL_LINK_NOARG(SvxFontWorkDialog, InputTimeoutHdl_Impl, Timer*, void)
{
    // Possibly set the Metric system again. This should be done with a
    // listen, this is however not possible at the moment due to compatibility
    // issues.
    const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    if( eDlgUnit != m_xMtrFldDistance->get_unit() )
    {
        SetFieldUnit(*m_xMtrFldDistance, eDlgUnit, true);
        SetFieldUnit(*m_xMtrFldTextStart, eDlgUnit, true);
        if (eDlgUnit == FieldUnit::MM)
        {
            m_xMtrFldDistance->set_increments(50, 500, FieldUnit::NONE);
            m_xMtrFldTextStart->set_increments(50, 500, FieldUnit::NONE);
        }
        else
        {
            m_xMtrFldDistance->set_increments(10, 100, FieldUnit::NONE);
            m_xMtrFldTextStart->set_increments(10, 100, FieldUnit::NONE);
        }
    }
    if( eDlgUnit != m_xMtrFldShadowX->get_unit() &&
        m_xTbxShadow->get_item_active(u"vertical"_ustr) )
    {
        SetFieldUnit(*m_xMtrFldShadowX, eDlgUnit, true);
        SetFieldUnit(*m_xMtrFldShadowY, eDlgUnit, true);

        if (eDlgUnit == FieldUnit::MM)
        {
            m_xMtrFldShadowX->set_increments(50, 500, FieldUnit::NONE);
            m_xMtrFldShadowY->set_increments(50, 500, FieldUnit::NONE);
        }
        else
        {
            m_xMtrFldShadowX->set_increments(10, 100, FieldUnit::NONE);
            m_xMtrFldShadowY->set_increments(10, 100, FieldUnit::NONE);
        }
    }

    tools::Long nValue = GetCoreValue(*m_xMtrFldDistance, MapUnit::Map100thMM);
    XFormTextDistanceItem aDistItem( nValue );
    nValue = GetCoreValue(*m_xMtrFldTextStart, MapUnit::Map100thMM);
    XFormTextStartItem aStartItem( nValue );

    sal_Int32 nValueX(0);
    sal_Int32 nValueY(0);

    // #i19251#
    // The two involved fields/items are used double and contain/give different
    // values regarding to the access method. Thus, here we need to separate the access
    // method regarding to the kind of value accessed.
    if (m_sLastShadowTbxId == "vertical")
    {
        nValueX = GetCoreValue(*m_xMtrFldShadowX, MapUnit::Map100thMM);
        nValueY = GetCoreValue(*m_xMtrFldShadowY, MapUnit::Map100thMM);
    }
    else if (m_sLastShadowTbxId == "slant")
    {
        nValueX = m_xMtrFldShadowX->get_value(FieldUnit::NONE);
        nValueY = m_xMtrFldShadowY->get_value(FieldUnit::NONE);
    }

    XFormTextShadowXValItem aShadowXItem( nValueX );
    XFormTextShadowYValItem aShadowYItem( nValueY );

    // Slot-ID does not matter, the Exec method evaluates the entire item set
    GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_DISTANCE,
            SfxCallMode::RECORD,
            { &aDistItem, &aStartItem, &aShadowXItem, &aShadowYItem });
}

IMPL_LINK_NOARG(SvxFontWorkDialog, ColorSelectHdl_Impl, ColorListBox&, void)
{
    XFormTextShadowColorItem aItem( u""_ustr, m_xShadowColorLB->GetSelectEntryColor() );
    GetBindings().GetDispatcher()->ExecuteList(SID_FORMTEXT_SHDWCOLOR,
            SfxCallMode::RECORD, { &aItem });
}

void SvxFontWorkDialog::ApplyImageList()
{
    if (m_sLastShadowTbxId == "slant")
    {
        m_xFbShadowX->set_from_icon_name(RID_SVXBMP_SHADOW_ANGLE);
        m_xFbShadowY->set_from_icon_name(RID_SVXBMP_SHADOW_SIZE);
    }
    else
    {
        m_xFbShadowX->set_from_icon_name(RID_SVXBMP_SHADOW_XDIST);
        m_xFbShadowY->set_from_icon_name(RID_SVXBMP_SHADOW_YDIST);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
