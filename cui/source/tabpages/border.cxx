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

#include <sfx2/objsh.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <border.hxx>
#include <svx/dlgutil.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/flagsdef.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/int64item.hxx>
#include <sal/macros.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/lok.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>

using namespace ::editeng;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::UNO_QUERY;


/*
 * [Description:]
 * TabPage for setting the border attributes.
 * Needs
 *      a SvxShadowItem: shadow
 *      a SvxBoxItem:    lines left, right, top, bottom,
 *      a SvxBoxInfo:    lines vertical, horizontal, distance, flags
 *
 * Lines can have three conditions:
 *      1. Show     ( -> valid values )
 *      2. Hide     ( -> NULL-Pointer )
 *      3. DontCare ( -> special Valid-Flags in the InfoItem )
 */

// static ----------------------------------------------------------------

const WhichRangesContainer SvxBorderTabPage::pRanges(
    svl::Items<
        SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_SHADOW,
        SID_ATTR_ALIGN_MARGIN,      SID_ATTR_ALIGN_MARGIN,
        SID_ATTR_BORDER_CONNECT,    SID_ATTR_BORDER_CONNECT,
        SID_SW_COLLAPSING_BORDERS,  SID_SW_COLLAPSING_BORDERS,
        SID_ATTR_BORDER_DIAG_TLBR,  SID_ATTR_BORDER_DIAG_BLTR>);

namespace
{
constexpr int twipsToPt100(sal_Int64 nTwips)
{
    return o3tl::convert(nTwips * 100, o3tl::Length::twip, o3tl::Length::pt);
}
constexpr int s_LineWidths[] = { twipsToPt100(SvxBorderLineWidth::Hairline),
                                 twipsToPt100(SvxBorderLineWidth::VeryThin),
                                 twipsToPt100(SvxBorderLineWidth::Thin),
                                 twipsToPt100(SvxBorderLineWidth::Medium),
                                 twipsToPt100(SvxBorderLineWidth::Thick),
                                 twipsToPt100(SvxBorderLineWidth::ExtraThick),
                                 -1 };
}

static void lcl_SetDecimalDigitsTo1(weld::MetricSpinButton& rField)
{
    auto nMin = rField.denormalize(rField.get_min(FieldUnit::TWIP));
    rField.set_digits(1);
    rField.set_min(rField.normalize(nMin), FieldUnit::TWIP);
}

// returns in pt
static sal_Int64 lcl_GetMinLineWidth(SvxBorderLineStyle aStyle)
{
    switch (aStyle)
    {
    case SvxBorderLineStyle::NONE:
        return 0;

    case SvxBorderLineStyle::SOLID:
    case SvxBorderLineStyle::DOTTED:
    case SvxBorderLineStyle::DASHED:
    case SvxBorderLineStyle::FINE_DASHED:
    case SvxBorderLineStyle::DASH_DOT:
    case SvxBorderLineStyle::DASH_DOT_DOT:
        return 15;

        // Double lines
    case SvxBorderLineStyle::DOUBLE: return 15;
    case SvxBorderLineStyle::DOUBLE_THIN: return 15;
    case SvxBorderLineStyle::THINTHICK_SMALLGAP: return 20;
    case SvxBorderLineStyle::THINTHICK_MEDIUMGAP: return 15;
    case SvxBorderLineStyle::THINTHICK_LARGEGAP: return 15;
    case SvxBorderLineStyle::THICKTHIN_SMALLGAP: return 20;
    case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP: return 15;
    case SvxBorderLineStyle::THICKTHIN_LARGEGAP: return 15;

    case SvxBorderLineStyle::EMBOSSED: return 15;
    case SvxBorderLineStyle::ENGRAVED: return 15;

    case SvxBorderLineStyle::OUTSET: return 10;
    case SvxBorderLineStyle::INSET: return 10;

    default:
        return 15;
    }
}

// number of preset images to show
const sal_uInt16 BORDER_PRESET_COUNT = 5;

// number of shadow images to show
const sal_uInt16 BORDER_SHADOW_COUNT = 5;

ShadowControlsWrapper::ShadowControlsWrapper(weld::IconView& rIvPos, weld::MetricSpinButton& rMfSize, ColorListBox& rLbColor)
    : mrIvPos(rIvPos)
    , mrMfSize(rMfSize)
    , mrLbColor(rLbColor)
{
}

SvxShadowItem ShadowControlsWrapper::GetControlValue(const SvxShadowItem& rItem) const
{
    SvxShadowItem aItem(rItem);
    OUString sSelectedId = mrIvPos.get_selected_id();
    if (!sSelectedId.isEmpty())
    {
        sal_Int32 nSelectedId = sSelectedId.toInt32();
        switch (nSelectedId)
        {
            case 1:
                aItem.SetLocation(SvxShadowLocation::NONE);
                break;
            case 2:
                aItem.SetLocation(SvxShadowLocation::BottomRight);
                break;
            case 3:
                aItem.SetLocation(SvxShadowLocation::TopRight);
                break;
            case 4:
                aItem.SetLocation(SvxShadowLocation::BottomLeft);
                break;
            case 5:
                aItem.SetLocation(SvxShadowLocation::TopLeft);
                break;
            default:
                aItem.SetLocation(SvxShadowLocation::NONE);
                break;
        }
    }
    // Default value was saved; so don't change the aItem's width if the control
    // has not changed its value, to avoid round-trip errors (like twip->cm->twip)
    // E.g., initial 100 twip will become 0.18 cm, which will return as 102 twip
    if (mrMfSize.get_value_changed_from_saved())
        aItem.SetWidth(mrMfSize.denormalize(mrMfSize.get_value(FieldUnit::TWIP)));
    if (!mrLbColor.IsNoSelection())
        aItem.SetColor(mrLbColor.GetSelectEntryColor());
    return aItem;
}

void ShadowControlsWrapper::SetControlValue(const SvxShadowItem& rItem)
{
    switch (rItem.GetLocation())
    {
        case SvxShadowLocation::NONE:
            mrIvPos.select(0);
            break;
        case SvxShadowLocation::BottomRight:
            mrIvPos.select(1);
            break;
        case SvxShadowLocation::TopRight:
            mrIvPos.select(2);
            break;
        case SvxShadowLocation::BottomLeft:
            mrIvPos.select(3);
            break;
        case SvxShadowLocation::TopLeft:
            mrIvPos.select(4);
            break;
        default:
            mrIvPos.unselect_all();
            break;
    }
    msSavedShadowItemId = mrIvPos.get_selected_id();
    mrMfSize.set_value(mrMfSize.normalize(rItem.GetWidth()), FieldUnit::TWIP);
    mrMfSize.save_value();
    mrLbColor.SelectEntry(rItem.GetColor());
    mrLbColor.SaveValue();
}

bool ShadowControlsWrapper::get_value_changed_from_saved() const
{
    return (mrIvPos.get_selected_id() != msSavedShadowItemId) ||
           mrMfSize.get_value_changed_from_saved() ||
           mrLbColor.IsValueChangedFromSaved();
}

void ShadowControlsWrapper::SetControlDontKnow()
{
    mrIvPos.unselect_all();
    mrMfSize.set_text(u""_ustr);
    mrLbColor.SetNoSelection();
}

MarginControlsWrapper::MarginControlsWrapper(weld::MetricSpinButton& rMfLeft, weld::MetricSpinButton& rMfRight,
                                             weld::MetricSpinButton& rMfTop, weld::MetricSpinButton& rMfBottom)
    : mrLeftWrp(rMfLeft)
    , mrRightWrp(rMfRight)
    , mrTopWrp(rMfTop)
    , mrBottomWrp(rMfBottom)
{
}

SvxMarginItem MarginControlsWrapper::GetControlValue(const SvxMarginItem &rItem) const
{
    SvxMarginItem aItem(rItem);
    if (mrLeftWrp.get_sensitive())
        aItem.SetLeftMargin(mrLeftWrp.denormalize(mrLeftWrp.get_value(FieldUnit::TWIP)));
    if (mrRightWrp.get_sensitive())
        aItem.SetRightMargin(mrRightWrp.denormalize(mrRightWrp.get_value(FieldUnit::TWIP)));
    if (mrTopWrp.get_sensitive())
        aItem.SetTopMargin(mrTopWrp.denormalize(mrTopWrp.get_value(FieldUnit::TWIP)));
    if (mrBottomWrp.get_sensitive())
        aItem.SetBottomMargin(mrBottomWrp.denormalize(mrBottomWrp.get_value(FieldUnit::TWIP)));
    return aItem;
}

bool MarginControlsWrapper::get_value_changed_from_saved() const
{
    return mrLeftWrp.get_value_changed_from_saved() ||
           mrRightWrp.get_value_changed_from_saved() ||
           mrTopWrp.get_value_changed_from_saved() ||
           mrBottomWrp.get_value_changed_from_saved();
}

void MarginControlsWrapper::SetControlValue(const SvxMarginItem& rItem)
{
    mrLeftWrp.set_value(mrLeftWrp.normalize(rItem.GetLeftMargin()), FieldUnit::TWIP);
    mrRightWrp.set_value(mrRightWrp.normalize(rItem.GetRightMargin()), FieldUnit::TWIP);
    mrTopWrp.set_value(mrTopWrp.normalize(rItem.GetTopMargin()), FieldUnit::TWIP);
    mrBottomWrp.set_value(mrBottomWrp.normalize(rItem.GetBottomMargin()), FieldUnit::TWIP);
    mrLeftWrp.save_value();
    mrRightWrp.save_value();
    mrTopWrp.save_value();
    mrBottomWrp.save_value();
}

void MarginControlsWrapper::SetControlDontKnow()
{
    const OUString sEmpty;
    mrLeftWrp.set_text(sEmpty);
    mrRightWrp.set_text(sEmpty);
    mrTopWrp.set_text(sEmpty);
    mrBottomWrp.set_text(sEmpty);
}

SvxBorderTabPage::SvxBorderTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/borderpage.ui"_ustr, u"BorderPage"_ustr, &rCoreAttrs)
    , nMinValue(0)
    , nSWMode(SwBorderModes::NONE)
    , mnBoxSlot(SID_ATTR_BORDER_OUTER)
    , mnShadowSlot(SID_ATTR_BORDER_SHADOW)
    , mbHorEnabled(false)
    , mbVerEnabled(false)
    , mbTLBREnabled(false)
    , mbBLTREnabled(false)
    , mbUseMarginItem(false)
    , mbLeftModified(false)
    , mbRightModified(false)
    , mbTopModified(false)
    , mbBottomModified(false)
    , mbSync(true)
    , mbRemoveAdjacentCellBorders(false)
    , bIsCalcDoc(false)
    , m_xWndPresets(m_xBuilder->weld_icon_view(u"presets"_ustr))
    , m_xUserDefFT(m_xBuilder->weld_label(u"userdefft"_ustr))
    , m_xFrameSelWin(new weld::CustomWeld(*m_xBuilder, u"framesel"_ustr, m_aFrameSel))
    , m_xLbLineStyle(new SvtLineListBox(m_xBuilder->weld_menu_button(u"linestylelb"_ustr)))
    , m_xLbLineColor(new ColorListBox(m_xBuilder->weld_menu_button(u"linecolorlb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xLineWidthLB(m_xBuilder->weld_combo_box(u"linewidthlb"_ustr))
    , m_xLineWidthMF(m_xBuilder->weld_metric_spin_button(u"linewidthmf"_ustr, FieldUnit::POINT))
    , m_xSpacingFrame(m_xBuilder->weld_container(u"spacing"_ustr))
    , m_xLeftFT(m_xBuilder->weld_label(u"leftft"_ustr))
    , m_xLeftMF(m_xBuilder->weld_metric_spin_button(u"leftmf"_ustr, FieldUnit::MM))
    , m_xRightFT(m_xBuilder->weld_label(u"rightft"_ustr))
    , m_xRightMF(m_xBuilder->weld_metric_spin_button(u"rightmf"_ustr, FieldUnit::MM))
    , m_xTopFT(m_xBuilder->weld_label(u"topft"_ustr))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button(u"topmf"_ustr, FieldUnit::MM))
    , m_xBottomFT(m_xBuilder->weld_label(u"bottomft"_ustr))
    , m_xBottomMF(m_xBuilder->weld_metric_spin_button(u"bottommf"_ustr, FieldUnit::MM))
    , m_xSynchronizeCB(m_xBuilder->weld_check_button(u"sync"_ustr))
    , m_xShadowFrame(m_xBuilder->weld_container(u"shadow"_ustr))
    , m_xWndShadows(m_xBuilder->weld_icon_view(u"shadows"_ustr))
    , m_xFtShadowSize(m_xBuilder->weld_label(u"distanceft"_ustr))
    , m_xEdShadowSize(m_xBuilder->weld_metric_spin_button(u"distancemf"_ustr, FieldUnit::MM))
    , m_xFtShadowColor(m_xBuilder->weld_label(u"shadowcolorft"_ustr))
    , m_xLbShadowColor(new ColorListBox(m_xBuilder->weld_menu_button(u"shadowcolorlb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xPropertiesFrame(m_xBuilder->weld_container(u"properties"_ustr))
    , m_xMergeWithNextCB(m_xBuilder->weld_check_button(u"mergewithnext"_ustr))
    , m_xMergeAdjacentBordersCB(m_xBuilder->weld_check_button(u"mergeadjacent"_ustr))
    , m_xRemoveAdjacentCellBordersCB(m_xBuilder->weld_check_button(u"rmadjcellborders"_ustr))
    , m_xRemoveAdjacentCellBordersFT(m_xBuilder->weld_label(u"rmadjcellbordersft"_ustr))
{
    static std::vector<OUString> aBorderImageIds;

    if (aBorderImageIds.empty())
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            aBorderImageIds.insert(aBorderImageIds.end(), {
                RID_SVXBMP_CELL_NONE_32,
                RID_SVXBMP_CELL_ALL_32,
                RID_SVXBMP_CELL_LR_32,
                RID_SVXBMP_CELL_TB_32,
                RID_SVXBMP_CELL_L_32,
                RID_SVXBMP_CELL_DIAG_32
            });
        }
        else
        {
            aBorderImageIds.insert(aBorderImageIds.end(), {
                RID_SVXBMP_CELL_NONE,
                RID_SVXBMP_CELL_ALL,
                RID_SVXBMP_CELL_LR,
                RID_SVXBMP_CELL_TB,
                RID_SVXBMP_CELL_L,
                RID_SVXBMP_CELL_DIAG
            });
        }
        aBorderImageIds.insert(aBorderImageIds.end(), {
            RID_SVXBMP_HOR_NONE,
            RID_SVXBMP_HOR_OUTER,
            RID_SVXBMP_HOR_HOR,
            RID_SVXBMP_HOR_ALL,
            RID_SVXBMP_HOR_OUTER2,
            RID_SVXBMP_VER_NONE,
            RID_SVXBMP_VER_OUTER,
            RID_SVXBMP_VER_VER,
            RID_SVXBMP_VER_ALL,
            RID_SVXBMP_VER_OUTER2,
            RID_SVXBMP_TABLE_NONE,
            RID_SVXBMP_TABLE_OUTER,
            RID_SVXBMP_TABLE_OUTERH,
            RID_SVXBMP_TABLE_ALL,
            RID_SVXBMP_TABLE_OUTER2
        });
    }

    for (auto const & rImageId : aBorderImageIds)
        m_aBorderImgVec.emplace_back(StockImage::Yes, rImageId);

    static std::vector<OUString> aShadowImageIds;
    if (aShadowImageIds.empty())
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            aShadowImageIds.insert(aShadowImageIds.end(), {
                RID_SVXBMP_SHADOWNONE_32,
                RID_SVXBMP_SHADOW_BOT_RIGHT_32,
                RID_SVXBMP_SHADOW_TOP_RIGHT_32,
                RID_SVXBMP_SHADOW_BOT_LEFT_32,
                RID_SVXBMP_SHADOW_TOP_LEFT_32
            });
        }
        else
        {
            aShadowImageIds.insert(aShadowImageIds.end(), {
                RID_SVXBMP_SHADOWNONE,
                RID_SVXBMP_SHADOW_BOT_RIGHT,
                RID_SVXBMP_SHADOW_TOP_RIGHT,
                RID_SVXBMP_SHADOW_BOT_LEFT,
                RID_SVXBMP_SHADOW_TOP_LEFT
            });
        }
    }

    for (auto const & rImageId : aShadowImageIds)
        m_aShadowImgVec.emplace_back(StockImage::Yes, rImageId);

    assert(m_aShadowImgVec.size() == BORDER_SHADOW_COUNT);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    /*  Use SvxMarginItem instead of margins from SvxBoxItem, if present.
        ->  Remember this state in mbUseMarginItem, because other special handling
            is needed across various functions... */
    mbUseMarginItem = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_MARGIN)) != SfxItemState::UNKNOWN;

    if (const SfxIntegerListItem* p = rCoreAttrs.GetItemIfSet(SID_ATTR_BORDER_STYLES))
    {
        std::vector<sal_Int32> aUsedStyles = p->GetList();
        for (int aUsedStyle : aUsedStyles)
            maUsedBorderStyles.insert(static_cast<SvxBorderLineStyle>(aUsedStyle));
    }

    if (const SfxInt64Item* p = rCoreAttrs.GetItemIfSet(SID_ATTR_BORDER_DEFAULT_WIDTH))
    {
        // The caller specifies default line width.  Honor it.
        SetLineWidth(p->GetValue());
    }

    // set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreAttrs );

    if( mbUseMarginItem )
    {
        // copied from SvxAlignmentTabPage
        switch ( eFUnit )
        {
            //  #103396# the default value (1pt) can't be accurately represented in
            //  inches or pica with two decimals, so point is used instead.
            case FieldUnit::PICA:
            case FieldUnit::INCH:
            case FieldUnit::FOOT:
            case FieldUnit::MILE:
                eFUnit = FieldUnit::POINT;
                break;

            case FieldUnit::CM:
            case FieldUnit::M:
            case FieldUnit::KM:
                eFUnit = FieldUnit::MM;
                break;
            default: ;//prevent warning
        }
    }
    else
    {
        switch ( eFUnit )
        {
            case FieldUnit::M:
            case FieldUnit::KM:
                eFUnit = FieldUnit::MM;
                break;
            default: ; //prevent warning
        }
    }

    SetFieldUnit(*m_xEdShadowSize, eFUnit);

    sal_uInt16 nWhich = GetWhich( SID_ATTR_BORDER_INNER, false );
    bool bIsDontCare = true;

    if ( rCoreAttrs.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        // paragraph or table
        const SvxBoxInfoItem* pBoxInfo =
            static_cast<const SvxBoxInfoItem*>(&( rCoreAttrs.Get( nWhich ) ));

        mbHorEnabled = pBoxInfo->IsHorEnabled();
        mbVerEnabled = pBoxInfo->IsVerEnabled();
        mbTLBREnabled = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_BORDER_DIAG_TLBR)) != SfxItemState::UNKNOWN;
        mbBLTREnabled = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_BORDER_DIAG_BLTR)) != SfxItemState::UNKNOWN;

        if(pBoxInfo->IsDist())
        {
            SetFieldUnit(*m_xLeftMF, eFUnit);
            SetFieldUnit(*m_xRightMF, eFUnit);
            SetFieldUnit(*m_xTopMF, eFUnit);
            SetFieldUnit(*m_xBottomMF, eFUnit);
            m_xSynchronizeCB->connect_toggled(LINK(this, SvxBorderTabPage, SyncHdl_Impl));
            m_xLeftMF->connect_value_changed(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_xRightMF->connect_value_changed(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_xTopMF->connect_value_changed(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_xBottomMF->connect_value_changed(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
        }
        else
        {
            m_xSpacingFrame->hide();
        }
        bIsDontCare = !pBoxInfo->IsValid( SvxBoxInfoItemValidFlags::DISABLE );
    }
    if(!mbUseMarginItem && eFUnit == FieldUnit::MM && MapUnit::MapTwip == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        //#i91548# changing the number of decimal digits changes the minimum values, too
        lcl_SetDecimalDigitsTo1(*m_xLeftMF);
        lcl_SetDecimalDigitsTo1(*m_xRightMF);
        lcl_SetDecimalDigitsTo1(*m_xTopMF);
        lcl_SetDecimalDigitsTo1(*m_xBottomMF);
        lcl_SetDecimalDigitsTo1(*m_xEdShadowSize);
    }

    FrameSelFlags nFlags = FrameSelFlags::Outer;
    if( mbHorEnabled )
        nFlags |= FrameSelFlags::InnerHorizontal;
    if( mbVerEnabled )
        nFlags |= FrameSelFlags::InnerVertical;
    if( mbTLBREnabled )
        nFlags |= FrameSelFlags::DiagonalTLBR;
    if( mbBLTREnabled )
        nFlags |= FrameSelFlags::DiagonalBLTR;
    if( bIsDontCare )
        nFlags |= FrameSelFlags::DontCare;
    m_aFrameSel.Initialize( nFlags );

    m_aFrameSel.SetSelectHdl(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    m_xLbLineStyle->SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    m_xLbLineColor->SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    m_xLineWidthLB->connect_changed(LINK(this, SvxBorderTabPage, ModifyWidthLBHdl_Impl));
    m_xLineWidthMF->connect_value_changed(LINK(this, SvxBorderTabPage, ModifyWidthMFHdl_Impl));
    m_xWndPresets->connect_selection_changed( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    m_xWndShadows->connect_selection_changed( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );
    m_xWndPresets->connect_query_tooltip( LINK( this, SvxBorderTabPage, QueryTooltipPreHdl ) );
    m_xWndShadows->connect_query_tooltip( LINK( this, SvxBorderTabPage, QueryTooltipSdwHdl ) );

    FillIconViews();
    FillLineListBox_Impl();

    // Reapply line width: probably one of predefined values should be selected
    SetLineWidth(m_xLineWidthMF->get_value(FieldUnit::NONE));

    // connections
    const SfxPoolItem* pItem = nullptr;
    if (rCoreAttrs.HasItem(GetWhich(SID_ATTR_PARA_GRABBAG), &pItem))
    {
        const SfxGrabBagItem* pGrabBag = static_cast<const SfxGrabBagItem*>(pItem);
        auto it = pGrabBag->GetGrabBag().find(u"DialogUseCharAttr"_ustr);
        if (it != pGrabBag->GetGrabBag().end())
        {
            bool bDialogUseCharAttr = false;
            it->second >>= bDialogUseCharAttr;
            if (bDialogUseCharAttr)
            {
                mnShadowSlot = SID_ATTR_CHAR_SHADOW;
                mnBoxSlot = SID_ATTR_CHAR_BOX;
            }
        }
    }

    bool bSupportsShadow = !SfxItemPool::IsSlot(GetWhich(mnShadowSlot));
    if( bSupportsShadow )
        m_xShadowControls.reset(new ShadowControlsWrapper(*m_xWndShadows, *m_xEdShadowSize, *m_xLbShadowColor));
    else
        HideShadowControls();

    if (mbUseMarginItem)
        m_xMarginControls.reset(new MarginControlsWrapper(*m_xLeftMF, *m_xRightMF, *m_xTopMF, *m_xBottomMF));

    // checkbox "Merge with next paragraph" only visible for Writer dialog format.paragraph
    m_xMergeWithNextCB->hide();
    // checkbox "Merge adjacent line styles" only visible for Writer dialog format.table
    m_xMergeAdjacentBordersCB->hide();

    if (SfxObjectShell* pDocSh = SfxObjectShell::Current())
    {
        Reference< XServiceInfo > xSI( pDocSh->GetModel(), UNO_QUERY );
        if ( xSI.is() )
            bIsCalcDoc = xSI->supportsService(u"com.sun.star.sheet.SpreadsheetDocument"_ustr);
    }
    if( bIsCalcDoc )
    {
        m_xRemoveAdjacentCellBordersCB->connect_toggled(LINK(this, SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl));
        m_xRemoveAdjacentCellBordersCB->show();
        m_xRemoveAdjacentCellBordersCB->set_sensitive(false);
    }
    else
    {
        m_xRemoveAdjacentCellBordersCB->hide();
        m_xRemoveAdjacentCellBordersFT->hide();
    }
}

SvxBorderTabPage::~SvxBorderTabPage()
{
    m_xLbShadowColor.reset();
    m_xLbLineColor.reset();
    m_xLbLineStyle.reset();
    m_xFrameSelWin.reset();
}

std::unique_ptr<SfxTabPage> SvxBorderTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxBorderTabPage>(pPage, pController, *rAttrSet);
}

void SvxBorderTabPage::ResetFrameLine_Impl( svx::FrameBorderType eBorder, const SvxBorderLine* pCoreLine, bool bValid )
{
    if( m_aFrameSel.IsBorderEnabled( eBorder ) )
    {
        if( bValid )
            m_aFrameSel.ShowBorder( eBorder, pCoreLine );
        else
            m_aFrameSel.SetBorderDontCare( eBorder );
    }
}

bool SvxBorderTabPage::IsBorderLineStyleAllowed( SvxBorderLineStyle nStyle ) const
{
    if (maUsedBorderStyles.empty())
        // All border styles are allowed.
        return true;

    return maUsedBorderStyles.count(nStyle) > 0;
}

void SvxBorderTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemPool* pPool = rSet->GetPool();

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::TLBR))
    {
        sal_uInt16 nBorderDiagId = pPool->GetWhichIDFromSlotID(SID_ATTR_BORDER_DIAG_TLBR);
        if (const SvxLineItem* pLineItem = static_cast<const SvxLineItem*>(rSet->GetItem(nBorderDiagId)))
            m_aFrameSel.ShowBorder(svx::FrameBorderType::TLBR, pLineItem->GetLine());
        else
            m_aFrameSel.SetBorderDontCare(svx::FrameBorderType::TLBR);
    }

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::BLTR))
    {
        sal_uInt16 nBorderDiagId = pPool->GetWhichIDFromSlotID(SID_ATTR_BORDER_DIAG_BLTR);
        if (const SvxLineItem* pLineItem = static_cast<const SvxLineItem*>(rSet->GetItem(nBorderDiagId)))
            m_aFrameSel.ShowBorder(svx::FrameBorderType::BLTR, pLineItem->GetLine());
        else
            m_aFrameSel.SetBorderDontCare(svx::FrameBorderType::BLTR);
    }

    if (m_xShadowControls)
    {
        sal_uInt16 nShadowId = pPool->GetWhichIDFromSlotID(mnShadowSlot);
        const SfxPoolItem* pItem = rSet->GetItem(nShadowId);
        if (pItem)
            m_xShadowControls->SetControlValue(*static_cast<const SvxShadowItem*>(pItem));
        else
            m_xShadowControls->SetControlDontKnow();
    }

    if (m_xMarginControls)
    {
        sal_uInt16 nAlignMarginId = pPool->GetWhichIDFromSlotID(SID_ATTR_ALIGN_MARGIN);
        const SfxPoolItem* pItem = rSet->GetItem(nAlignMarginId);
        if (pItem)
            m_xMarginControls->SetControlValue(*static_cast<const SvxMarginItem*>(pItem));
        else
            m_xMarginControls->SetControlDontKnow();
    }

    sal_uInt16 nMergeAdjacentBordersId = pPool->GetWhichIDFromSlotID(SID_SW_COLLAPSING_BORDERS);
    const SfxBoolItem *pMergeAdjacentBorders = static_cast<const SfxBoolItem*>(rSet->GetItem(nMergeAdjacentBordersId));
    if (!pMergeAdjacentBorders)
        m_xMergeAdjacentBordersCB->set_state(TRISTATE_INDET);
    else
        m_xMergeAdjacentBordersCB->set_active(pMergeAdjacentBorders->GetValue());
    m_xMergeAdjacentBordersCB->save_state();

    sal_uInt16 nMergeWithNextId = pPool->GetWhichIDFromSlotID(SID_ATTR_BORDER_CONNECT);
    const SfxBoolItem *pMergeWithNext = static_cast<const SfxBoolItem*>(rSet->GetItem(nMergeWithNextId));
    if (!pMergeWithNext)
        m_xMergeWithNextCB->set_state(TRISTATE_INDET);
    else
        m_xMergeWithNextCB->set_active(pMergeWithNext->GetValue());
    m_xMergeWithNextCB->save_state();

    const SvxBoxItem*       pBoxItem;
    const SvxBoxInfoItem*   pBoxInfoItem;
    sal_uInt16              nWhichBox       = GetWhich(mnBoxSlot);
    MapUnit                 eCoreUnit;

    pBoxItem  = static_cast<const SvxBoxItem*>(GetItem( *rSet, mnBoxSlot ));

    pBoxInfoItem = GetItem( *rSet, SID_ATTR_BORDER_INNER, false );

    eCoreUnit = pPool->GetMetric( nWhichBox );

    if ( pBoxItem && pBoxInfoItem ) // -> Don't Care
    {
        ResetFrameLine_Impl( svx::FrameBorderType::Left,   pBoxItem->GetLeft(),     pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::LEFT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Right,  pBoxItem->GetRight(),    pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::RIGHT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Top,    pBoxItem->GetTop(),      pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::TOP ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Bottom, pBoxItem->GetBottom(),   pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Vertical,    pBoxInfoItem->GetVert(), pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::VERT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Horizontal,    pBoxInfoItem->GetHori(), pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::HORI ) );


        // distance inside

        if( !mbUseMarginItem )
        {
            if (m_xLeftMF->get_visible())
            {
                SetMetricValue(*m_xLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_xRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_xTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_xBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit);

                nMinValue = m_xLeftMF->get_value(FieldUnit::NONE);

                if ( pBoxInfoItem->IsDist() )
                {
                    if( rSet->GetItemState( nWhichBox ) >= SfxItemState::DEFAULT )
                    {
                        bool bIsAnyBorderVisible = m_aFrameSel.IsAnyBorderVisible();
                        if( !bIsAnyBorderVisible || !pBoxInfoItem->IsMinDist() )
                        {
                            m_xLeftMF->set_min(0, FieldUnit::NONE);
                            m_xRightMF->set_min(0, FieldUnit::NONE);
                            m_xTopMF->set_min(0, FieldUnit::NONE);
                            m_xBottomMF->set_min(0, FieldUnit::NONE);
                        }
                        tools::Long nLeftDist = pBoxItem->GetDistance( SvxBoxItemLine::LEFT);
                        SetMetricValue(*m_xLeftMF, nLeftDist, eCoreUnit);
                        tools::Long nRightDist = pBoxItem->GetDistance( SvxBoxItemLine::RIGHT);
                        SetMetricValue(*m_xRightMF, nRightDist, eCoreUnit);
                        tools::Long nTopDist = pBoxItem->GetDistance( SvxBoxItemLine::TOP);
                        SetMetricValue( *m_xTopMF, nTopDist, eCoreUnit );
                        tools::Long nBottomDist = pBoxItem->GetDistance( SvxBoxItemLine::BOTTOM);
                        SetMetricValue( *m_xBottomMF, nBottomDist, eCoreUnit );

                        // if the distance is set with no active border line
                        // or it is null with an active border line
                        // no automatic changes should be made
                        const tools::Long nDefDist = bIsAnyBorderVisible ? pBoxInfoItem->GetDefDist() : 0;
                        bool bDiffDist = (nDefDist != nLeftDist ||
                                    nDefDist != nRightDist ||
                                    nDefDist != nTopDist   ||
                                    nDefDist != nBottomDist);
                        if ((pBoxItem->GetSmallestDistance() || bIsAnyBorderVisible) && bDiffDist )
                        {
                            mbLeftModified = true;
                            mbRightModified = true;
                            mbTopModified = true;
                            mbBottomModified = true;
                        }
                    }
                    else
                    {
                        // #106224# different margins -> do not fill the edits
                        m_xLeftMF->set_text( OUString() );
                        m_xRightMF->set_text( OUString() );
                        m_xTopMF->set_text( OUString() );
                        m_xBottomMF->set_text( OUString() );
                    }
                }
                m_xLeftMF->save_value();
                m_xRightMF->save_value();
                m_xTopMF->save_value();
                m_xBottomMF->save_value();
            }
        }
    }
    else
    {
        // avoid ResetFrameLine-calls:
        m_aFrameSel.HideAllBorders();
    }

    if( !m_aFrameSel.IsAnyBorderVisible() )
        m_aFrameSel.DeselectAllBorders();

    // depict line (color) in controllers if unambiguous:

    {
        // Do all visible lines show the same line widths?
        tools::Long nWidth;
        SvxBorderLineStyle nStyle;
        bool bWidthEq = m_aFrameSel.GetVisibleWidth( nWidth, nStyle );
        if( bWidthEq )
        {
            // Determine the width first as some styles can be missing depending on it
            sal_Int64 nWidthPt =  static_cast<sal_Int64>(vcl::ConvertDoubleValue(
                        sal_Int64( nWidth ), m_xLineWidthMF->get_digits(),
                        MapUnit::MapTwip, FieldUnit::POINT ));
            SetLineWidth(nWidthPt);
            m_xLbLineStyle->SetWidth(nWidth);

            // then set the style
            m_xLbLineStyle->SelectEntry( nStyle );
        }
        else
            m_xLbLineStyle->SelectEntry(SvxBorderLineStyle::SOLID);

        // Do all visible lines show the same line color?
        Color aColor;
        bool bColorEq = m_aFrameSel.GetVisibleColor( aColor );
        if( !bColorEq )
            aColor = COL_BLACK;

        m_xLbLineColor->SelectEntry(aColor);
        auto nTextColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
        m_xLbLineStyle->SetColor(nTextColor);

        // Select all visible lines, if they are all equal.
        if( bWidthEq && bColorEq )
            m_aFrameSel.SelectAllVisibleBorders();

        // set the current style and color (caches style in control even if nothing is selected)
        SelStyleHdl_Impl(*m_xLbLineStyle);
        SelColHdl_Impl(*m_xLbLineColor);
    }

    OUString sShadowSelectedId = m_xWndShadows->get_selected_id();
    bool bEnable = !sShadowSelectedId.isEmpty() && sShadowSelectedId.toInt32() > 1 ;
    m_xFtShadowSize->set_sensitive(bEnable);
    m_xEdShadowSize->set_sensitive(bEnable);
    m_xFtShadowColor->set_sensitive(bEnable);
    m_xLbShadowColor->set_sensitive(bEnable);

    m_xWndPresets->unselect_all();

    // - no line - should not be selected

    if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::NONE)
    {
        m_xLbLineStyle->SelectEntry(SvxBorderLineStyle::SOLID);
        SelStyleHdl_Impl(*m_xLbLineStyle);
    }

    const SfxUInt16Item* pHtmlModeItem = rSet->GetItemIfSet(SID_HTML_MODE, false);
    if(!pHtmlModeItem)
    {
        if (SfxObjectShell* pShell = SfxObjectShell::Current())
            pHtmlModeItem = pShell->GetItem(SID_HTML_MODE);
    }
    if(pHtmlModeItem)
    {
        sal_uInt16 nHtmlMode = pHtmlModeItem->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            // there are no shadows in Html-mode and only complete borders
            m_xShadowFrame->set_sensitive(false);

            if( !(nSWMode & SwBorderModes::TABLE) )
            {
                m_xUserDefFT->set_sensitive(false);
                m_xFrameSelWin->set_sensitive(false);

                if( m_xWndPresets->n_children() > 4 )
                {
                    m_xWndPresets->remove(4);
                    m_xWndPresets->remove(3);
                    m_xWndPresets->remove(2);
                }
            }
        }
    }

    LinesChanged_Impl( nullptr );
    if (m_xLeftMF->get_value(FieldUnit::NONE) == m_xRightMF->get_value(FieldUnit::NONE) &&
        m_xTopMF->get_value(FieldUnit::NONE) == m_xBottomMF->get_value(FieldUnit::NONE) &&
        m_xTopMF->get_value(FieldUnit::NONE) == m_xLeftMF->get_value(FieldUnit::NONE))
    {
        mbSync = true;
    }
    else
        mbSync = false;
    m_xSynchronizeCB->set_active(mbSync);

    mbRemoveAdjacentCellBorders = false;
    m_xRemoveAdjacentCellBordersCB->set_active(false);
    m_xRemoveAdjacentCellBordersCB->set_sensitive(false);
}

void SvxBorderTabPage::ChangesApplied()
{
    m_xLeftMF->save_value();
    m_xRightMF->save_value();
    m_xTopMF->save_value();
    m_xBottomMF->save_value();
    m_xMergeWithNextCB->save_state();
    m_xMergeAdjacentBordersCB->save_state();
}

DeactivateRC SvxBorderTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

bool SvxBorderTabPage::FillItemSet( SfxItemSet* rCoreAttrs )
{
    bool bAttrsChanged = false;

    SfxItemPool* pPool = rCoreAttrs->GetPool();

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::TLBR) &&
        m_aFrameSel.GetFrameBorderState(svx::FrameBorderType::TLBR) != svx::FrameBorderState::DontCare)
    {
        if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, SID_ATTR_BORDER_DIAG_TLBR))
        {
            SvxLineItem aLineItem(*static_cast<const SvxLineItem*>(pOldItem));
            aLineItem.SetLine(m_aFrameSel.GetFrameBorderStyle(svx::FrameBorderType::TLBR));
            rCoreAttrs->Put(aLineItem);
            bAttrsChanged = true;
        }
    }

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::BLTR) &&
        m_aFrameSel.GetFrameBorderState(svx::FrameBorderType::BLTR) != svx::FrameBorderState::DontCare)
    {
        if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, SID_ATTR_BORDER_DIAG_BLTR))
        {
            SvxLineItem aLineItem(*static_cast<const SvxLineItem*>(pOldItem));
            aLineItem.SetLine(m_aFrameSel.GetFrameBorderStyle(svx::FrameBorderType::BLTR));
            rCoreAttrs->Put(aLineItem);
            bAttrsChanged = true;
        }
    }

    if (m_xShadowControls && m_xShadowControls->get_value_changed_from_saved())
    {
        if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, mnShadowSlot))
        {
            const SvxShadowItem& rOldShadowItem = *static_cast<const SvxShadowItem*>(pOldItem);
            rCoreAttrs->Put(m_xShadowControls->GetControlValue(rOldShadowItem));
            bAttrsChanged = true;
        }
    }

    if (m_xMarginControls && m_xMarginControls->get_value_changed_from_saved())
    {
        if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, SID_ATTR_ALIGN_MARGIN))
        {
            const SvxMarginItem& rOldMarginItem = *static_cast<const SvxMarginItem*>(pOldItem);
            rCoreAttrs->Put(m_xMarginControls->GetControlValue(rOldMarginItem));
            bAttrsChanged = true;
        }
    }

    if (m_xMergeAdjacentBordersCB->get_state_changed_from_saved())
    {
        auto nState = m_xMergeAdjacentBordersCB->get_state();
        if (nState == TRISTATE_INDET)
        {
            sal_uInt16 nMergeAdjacentBordersId = pPool->GetWhichIDFromSlotID(SID_SW_COLLAPSING_BORDERS);
            rCoreAttrs->ClearItem(nMergeAdjacentBordersId);
        }
        else
        {
            if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, SID_SW_COLLAPSING_BORDERS))
            {
                std::unique_ptr<SfxBoolItem> xNewItem(static_cast<SfxBoolItem*>(pOldItem->Clone()));
                xNewItem->SetValue(static_cast<bool>(nState));
                rCoreAttrs->Put(std::move(xNewItem));
            }
        }
        bAttrsChanged = true;
    }

    if (m_xMergeWithNextCB->get_state_changed_from_saved())
    {
        auto nState = m_xMergeWithNextCB->get_state();
        if (nState == TRISTATE_INDET)
        {
            sal_uInt16 nMergeWithNextId = pPool->GetWhichIDFromSlotID(SID_ATTR_BORDER_CONNECT);
            rCoreAttrs->ClearItem(nMergeWithNextId);
        }
        else
        {
            if (const SfxPoolItem* pOldItem = GetOldItem(*rCoreAttrs, SID_ATTR_BORDER_CONNECT))
            {
                std::unique_ptr<SfxBoolItem> xNewItem(static_cast<SfxBoolItem*>(pOldItem->Clone()));
                xNewItem->SetValue(static_cast<bool>(nState));
                rCoreAttrs->Put(std::move(xNewItem));
            }
        }
        bAttrsChanged = true;
    }

    bool                  bPut          = true;
    sal_uInt16            nBoxWhich     = GetWhich( mnBoxSlot );
    sal_uInt16            nBoxInfoWhich = pPool->GetWhichIDFromSlotID( SID_ATTR_BORDER_INNER, false );
    const SfxItemSet&     rOldSet       = GetItemSet();
    SvxBoxItem            aBoxItem      ( nBoxWhich );
    SvxBoxInfoItem        aBoxInfoItem  ( nBoxInfoWhich );
    const SvxBoxItem*     pOldBoxItem = static_cast<const SvxBoxItem*>(GetOldItem( *rCoreAttrs, mnBoxSlot ));

    MapUnit eCoreUnit = rOldSet.GetPool()->GetMetric( nBoxWhich );


    // outer border:

    std::pair<svx::FrameBorderType,SvxBoxItemLine> eTypes1[] = {
                                { svx::FrameBorderType::Top,SvxBoxItemLine::TOP },
                                { svx::FrameBorderType::Bottom,SvxBoxItemLine::BOTTOM },
                                { svx::FrameBorderType::Left,SvxBoxItemLine::LEFT },
                                { svx::FrameBorderType::Right,SvxBoxItemLine::RIGHT },
                            };

    for (std::pair<svx::FrameBorderType,SvxBoxItemLine> const & i : eTypes1)
        aBoxItem.SetLine( m_aFrameSel.GetFrameBorderStyle( i.first ), i.second );


    aBoxItem.SetRemoveAdjacentCellBorder( mbRemoveAdjacentCellBorders );
    // border hor/ver and TableFlag

    std::pair<svx::FrameBorderType,SvxBoxInfoItemLine> eTypes2[] = {
                                { svx::FrameBorderType::Horizontal,SvxBoxInfoItemLine::HORI },
                                { svx::FrameBorderType::Vertical,SvxBoxInfoItemLine::VERT }
                            };
    for (std::pair<svx::FrameBorderType,SvxBoxInfoItemLine> const & j : eTypes2)
        aBoxInfoItem.SetLine( m_aFrameSel.GetFrameBorderStyle( j.first ), j.second );

    aBoxInfoItem.EnableHor( mbHorEnabled );
    aBoxInfoItem.EnableVer( mbVerEnabled );


    // inner distance

    if (m_xLeftMF->get_visible())
    {
        // #i40405# enable distance controls for next dialog call
        aBoxInfoItem.SetDist( true );

        if( !mbUseMarginItem )
        {
            // #106224# all edits empty: do nothing
            if( !m_xLeftMF->get_text().isEmpty() || !m_xRightMF->get_text().isEmpty() ||
                !m_xTopMF->get_text().isEmpty() || !m_xBottomMF->get_text().isEmpty() )
            {
                const SvxBoxInfoItem* pOldBoxInfoItem = GetOldItem( *rCoreAttrs, SID_ATTR_BORDER_INNER );
                if (
                    !pOldBoxItem ||
                    m_xLeftMF->get_value_changed_from_saved() ||
                    m_xRightMF->get_value_changed_from_saved() ||
                    m_xTopMF->get_value_changed_from_saved() ||
                    m_xBottomMF->get_value_changed_from_saved() ||
                    nMinValue == m_xLeftMF->get_value(FieldUnit::NONE) ||
                    nMinValue == m_xRightMF->get_value(FieldUnit::NONE) ||
                    nMinValue == m_xTopMF->get_value(FieldUnit::NONE) ||
                    nMinValue == m_xBottomMF->get_value(FieldUnit::NONE) ||
                    (pOldBoxInfoItem && !pOldBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::DISTANCE))
                   )
                {
                    aBoxItem.SetDistance( static_cast<sal_uInt16>(GetCoreValue(*m_xLeftMF, eCoreUnit )), SvxBoxItemLine::LEFT  );
                    aBoxItem.SetDistance( static_cast<sal_uInt16>(GetCoreValue(*m_xRightMF, eCoreUnit )), SvxBoxItemLine::RIGHT );
                    aBoxItem.SetDistance( static_cast<sal_uInt16>(GetCoreValue(*m_xTopMF, eCoreUnit )), SvxBoxItemLine::TOP   );
                    aBoxItem.SetDistance( static_cast<sal_uInt16>(GetCoreValue(*m_xBottomMF, eCoreUnit )), SvxBoxItemLine::BOTTOM);
                }
                else
                {
                    aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::LEFT ), SvxBoxItemLine::LEFT);
                    aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::RIGHT),  SvxBoxItemLine::RIGHT);
                    aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::TOP  ), SvxBoxItemLine::TOP);
                    aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::BOTTOM), SvxBoxItemLine::BOTTOM);
                }
                aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
            }
        }
    }


    // note Don't Care Status in the Info-Item:

    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::TOP,    m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Top )    != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::BOTTOM, m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Bottom ) != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::LEFT,   m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Left )   != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::RIGHT,  m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Right )  != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::HORI,   m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Horizontal )    != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::VERT,   m_aFrameSel.GetFrameBorderState( svx::FrameBorderType::Vertical )    != svx::FrameBorderState::DontCare );


    // Put or Clear of the border?

    bPut = true;

    if (   SfxItemState::DEFAULT == rOldSet.GetItemState( nBoxWhich,     false ))
    {
        bPut = aBoxItem != static_cast<const SvxBoxItem&>(rOldSet.Get(nBoxWhich));
    }
    if(  SfxItemState::DEFAULT == rOldSet.GetItemState( nBoxInfoWhich, false ) )
    {
        const SvxBoxInfoItem& rOldBoxInfo = static_cast<const SvxBoxInfoItem&>(
                                rOldSet.Get(nBoxInfoWhich));

        aBoxInfoItem.SetMinDist( rOldBoxInfo.IsMinDist() );
        aBoxInfoItem.SetDefDist( rOldBoxInfo.GetDefDist() );
        bPut |= (aBoxInfoItem != rOldBoxInfo );
    }

    if ( bPut )
    {
        if ( !pOldBoxItem || *pOldBoxItem != aBoxItem )
        {
            rCoreAttrs->Put( aBoxItem );
            bAttrsChanged = true;
        }
        const SfxPoolItem* pOld = GetOldItem( *rCoreAttrs, SID_ATTR_BORDER_INNER, false );

        if ( !pOld || *static_cast<const SvxBoxInfoItem*>(pOld) != aBoxInfoItem )
        {
            rCoreAttrs->Put( aBoxInfoItem );
            bAttrsChanged = true;
        }
    }
    else
    {
        rCoreAttrs->ClearItem( nBoxWhich );
        rCoreAttrs->ClearItem( nBoxInfoWhich );
    }

    return bAttrsChanged;
}

void SvxBorderTabPage::HideShadowControls()
{
    m_xShadowFrame->hide();
}

#define IID_PRE_CELL_NONE       1
#define IID_PRE_CELL_ALL        2
#define IID_PRE_CELL_LR         3
#define IID_PRE_CELL_TB         4
#define IID_PRE_CELL_L          5
#define IID_PRE_CELL_DIAG       6
#define IID_PRE_HOR_NONE        7
#define IID_PRE_HOR_OUTER       8
#define IID_PRE_HOR_HOR         9
#define IID_PRE_HOR_ALL         10
#define IID_PRE_HOR_OUTER2      11
#define IID_PRE_VER_NONE        12
#define IID_PRE_VER_OUTER       13
#define IID_PRE_VER_VER         14
#define IID_PRE_VER_ALL         15
#define IID_PRE_VER_OUTER2      16
#define IID_PRE_TABLE_NONE      17
#define IID_PRE_TABLE_OUTER     18
#define IID_PRE_TABLE_OUTERH    19
#define IID_PRE_TABLE_ALL       20
#define IID_PRE_TABLE_OUTER2    21

IMPL_LINK_NOARG(SvxBorderTabPage, SelPreHdl_Impl, weld::IconView&, void)
{
    const svx::FrameBorderState SHOW = svx::FrameBorderState::Show;
    const svx::FrameBorderState HIDE = svx::FrameBorderState::Hide;
    const svx::FrameBorderState DONT = svx::FrameBorderState::DontCare;

    static const svx::FrameBorderState ppeStates[][ svx::FRAMEBORDERTYPE_COUNT ] =
    {                   /*    Left  Right Top   Bot   Hor   Ver   TLBR  BLTR */
/* ---------------------+--------------------------------------------------- */
/* IID_PRE_CELL_NONE    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_ALL     */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_LR      */  { SHOW, SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_TB      */  { HIDE, HIDE, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_L       */  { SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_DIAG    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, SHOW, SHOW },
/* IID_PRE_HOR_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_HOR      */  { HIDE, HIDE, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_ALL      */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, DONT, HIDE, HIDE, HIDE },
/* IID_PRE_VER_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_VER      */  { SHOW, SHOW, HIDE, HIDE, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_ALL      */  { SHOW, SHOW, SHOW, SHOW, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, HIDE, DONT, HIDE, HIDE },
/* IID_PRE_TABLE_NONE   */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER  */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTERH */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_ALL    */  { SHOW, SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER2 */  { SHOW, SHOW, SHOW, SHOW, DONT, DONT, HIDE, HIDE }
    };

    // first hide and deselect all frame borders
    m_aFrameSel.HideAllBorders();
    m_aFrameSel.DeselectAllBorders();

    // Using image ID to find correct line in table above.
    sal_uInt16 nSelectedId = m_xWndPresets->get_selected_id().toUInt32();
    sal_uInt16 nLine = GetPresetImageId(nSelectedId) - 1;

    // Apply all styles from the table
    for( int nBorder = 0; nBorder < svx::FRAMEBORDERTYPE_COUNT; ++nBorder )
    {
        svx::FrameBorderType eBorder = svx::GetFrameBorderTypeFromIndex( nBorder );
        switch( ppeStates[ nLine ][ nBorder ] )
        {
            case SHOW:  m_aFrameSel.SelectBorder( eBorder );      break;
            case HIDE:  /* nothing to do */                     break;
            case DONT:  m_aFrameSel.SetBorderDontCare( eBorder ); break;
        }
    }

    // Show all lines that have been selected above
    if( m_aFrameSel.IsAnyBorderSelected() )
    {
        // any visible style, but "no-line" in line list box? -> use hair-line
        if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::NONE)
            m_xLbLineStyle->SelectEntry(SvxBorderLineStyle::SOLID);

        // set current style to all previously selected lines
        SelStyleHdl_Impl(*m_xLbLineStyle);
        SelColHdl_Impl(*m_xLbLineColor);
    }

    // Presets IconView does not show a selection (used as push buttons).
    m_xWndPresets->unselect_all();

    LinesChanged_Impl( nullptr );
    UpdateRemoveAdjCellBorderCB( nLine + 1 );
}

IMPL_LINK_NOARG(SvxBorderTabPage, SelSdwHdl_Impl, weld::IconView&, void)
{
    OUString sSelectedId = m_xWndShadows->get_selected_id();
    bool bEnable = !sSelectedId.isEmpty() && sSelectedId.toInt32() > 1;
    m_xFtShadowSize->set_sensitive(bEnable);
    m_xEdShadowSize->set_sensitive(bEnable);
    m_xFtShadowColor->set_sensitive(bEnable);
    m_xLbShadowColor->set_sensitive(bEnable);
}

IMPL_LINK(SvxBorderTabPage, QueryTooltipPreHdl, const weld::TreeIter&, iter, OUString)
{
    const OUString sId = m_xWndPresets->get_id(iter);
    if (!sId.isEmpty())
        return SvxResId( GetPresetStringId( sId.toInt32() ) );

    return OUString();
}

IMPL_LINK(SvxBorderTabPage, QueryTooltipSdwHdl, const weld::TreeIter&, iter, OUString)
{
    const OUString sId = m_xWndShadows->get_id(iter);
    if (!sId.isEmpty())
        return CuiResId( GetShadowStringId( sId.toInt32() ) );

    return OUString();
}

IMPL_LINK(SvxBorderTabPage, SelColHdl_Impl, ColorListBox&, rColorBox, void)
{
    const NamedColor& aNamedColor = rColorBox.GetSelectedEntry();
    m_aFrameSel.SetColorToSelection(aNamedColor.m_aColor, aNamedColor.getComplexColor());
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthLBHdl_Impl, weld::ComboBox&, void)
{
    sal_Int32 nPos = m_xLineWidthLB->get_active();
    sal_Int32 nRemovedType = 0;
    if (m_xLineWidthLB->get_values_changed_from_saved()) {
        nRemovedType = std::size(s_LineWidths) - m_xLineWidthLB->get_count();
    }

    SetLineWidth(s_LineWidths[nPos + nRemovedType], nRemovedType);

    // Call the spinner handler to trigger all related modifications
    ModifyWidthMFHdl_Impl(*m_xLineWidthMF);
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthMFHdl_Impl, weld::MetricSpinButton&, void)
{
    sal_Int64 nVal = m_xLineWidthMF->get_value(FieldUnit::NONE);

    // for DOUBLE_THIN line style we cannot allow thinner line width then 1.10pt
    if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::DOUBLE_THIN)
        m_xLineWidthMF->set_min(110, FieldUnit::NONE);
    else
        m_xLineWidthMF->set_min(5, FieldUnit::NONE);

    nVal = static_cast<sal_Int64>(vcl::ConvertDoubleValue(
                nVal,
                m_xLineWidthMF->get_digits(),
                FieldUnit::POINT, MapUnit::MapTwip ));
    m_xLbLineStyle->SetWidth( nVal );

    m_aFrameSel.SetStyleToSelection( nVal,
        m_xLbLineStyle->GetSelectEntryStyle() );
}

IMPL_LINK_NOARG(SvxBorderTabPage, SelStyleHdl_Impl, SvtLineListBox&, void)
{
    sal_Int64 nOldWidth = m_xLineWidthMF->get_value(FieldUnit::NONE);

    // for DOUBLE_THIN line style we cannot allow thinner line width then 1.10pt
    if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::DOUBLE_THIN)
        m_xLineWidthMF->set_min(110, FieldUnit::NONE);
    else
        m_xLineWidthMF->set_min(5, FieldUnit::NONE);

    nOldWidth = static_cast<sal_Int64>(vcl::ConvertDoubleValue(
        nOldWidth,
        m_xLineWidthMF->get_digits(),
        FieldUnit::POINT,
        MapUnit::MapTwip));

    const sal_Int64 nOldMinWidth = lcl_GetMinLineWidth(m_aFrameSel.getCurrentStyleLineStyle());
    const sal_Int64 nNewMinWidth = lcl_GetMinLineWidth(m_xLbLineStyle->GetSelectEntryStyle());

    // auto change line-width if it doesn't correspond to minimal value
    // let's change only in case when user has not changed the line-width into some custom value
    sal_Int64 nNewWidth = (nOldMinWidth == nOldWidth) ? nNewMinWidth : nOldWidth;

    // if we had selected a predefined border width under SvxBorderLineWidth::Medium set the Medium as default
    // otherwise if we had a custom border width under 1.10pt then set the spinner to the maximum allowed value for double border styles
    bool bNewDoubleHairline = m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::DOUBLE_THIN && !m_xLineWidthMF->get_visible() &&
        (nOldWidth == SvxBorderLineWidth::Hairline || nOldWidth == SvxBorderLineWidth::VeryThin || nOldWidth == SvxBorderLineWidth::Thin);
    if (bNewDoubleHairline && nNewWidth < SvxBorderLineWidth::Medium)
        nNewWidth = SvxBorderLineWidth::Medium;

    // set value inside edit box
    if (nOldWidth != nNewWidth)
    {
        const sal_Int64 nNewWidthPt = static_cast<sal_Int64>(vcl::ConvertDoubleValue(
            nNewWidth,
            m_xLineWidthMF->get_digits(),
            MapUnit::MapTwip,
            FieldUnit::POINT));
        SetLineWidth(nNewWidthPt);
    }

    if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::DOUBLE_THIN)
    {
        for (size_t i = 0; i < 3; i++)
        {
            m_xLineWidthLB->save_values_by_id(OUString::number(i));
            m_xLineWidthLB->remove_id(OUString::number(i));
        }
        if (m_xLineWidthLB->get_active_id().isEmpty())
            m_xLineWidthLB->set_active_id(u"3"_ustr);
    }
    else
    {
        if (m_xLineWidthLB->get_values_changed_from_saved())
        {
            for (size_t i = 0; i < 3; i++)
                m_xLineWidthLB->append(i, OUString::number(i), m_xLineWidthLB->get_saved_values(i));
            m_xLineWidthLB->removeSavedValues();
        }
    }

    // set value inside style box
    m_aFrameSel.SetStyleToSelection( nNewWidth,
        m_xLbLineStyle->GetSelectEntryStyle() );
}


// IconView handling
sal_uInt16 SvxBorderTabPage::GetPresetImageId( sal_uInt16 nIconViewIdx ) const
{
    // table with all sets of predefined border styles
    static const sal_uInt16 ppnImgIds[][ BORDER_PRESET_COUNT ] =
    {
        // simple cell without diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_L          },
        // simple cell with diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_DIAG       },
        // with horizontal inner frame border
        {   IID_PRE_HOR_NONE,   IID_PRE_HOR_OUTER,      IID_PRE_HOR_HOR,        IID_PRE_HOR_ALL,    IID_PRE_HOR_OUTER2      },
        // with vertical inner frame border
        {   IID_PRE_VER_NONE,   IID_PRE_VER_OUTER,      IID_PRE_VER_VER,        IID_PRE_VER_ALL,    IID_PRE_VER_OUTER2      },
        // with horizontal and vertical inner frame borders
        {   IID_PRE_TABLE_NONE, IID_PRE_TABLE_OUTER,    IID_PRE_TABLE_OUTERH,   IID_PRE_TABLE_ALL,  IID_PRE_TABLE_OUTER2    }
    };

    // find correct set of presets
    int nLine = 0;
    if( !mbHorEnabled && !mbVerEnabled )
        nLine = (mbTLBREnabled || mbBLTREnabled) ? 1 : 0;
    else if( mbHorEnabled && !mbVerEnabled )
        nLine = 2;
    else if( !mbHorEnabled && mbVerEnabled )
        nLine = 3;
    else
        nLine = 4;

    DBG_ASSERT( (1 <= nIconViewIdx) && (nIconViewIdx <= BORDER_PRESET_COUNT),
        "SvxBorderTabPage::GetPresetImageId - wrong index" );
    return ppnImgIds[ nLine ][ nIconViewIdx - 1 ];
}

TranslateId SvxBorderTabPage::GetShadowStringId( sal_uInt16 nIconViewIdx )
{
    static const TranslateId pnStrIds[ BORDER_SHADOW_COUNT ] =
    {
        RID_CUISTR_SHADOW_STYLE_NONE,
        RID_CUISTR_SHADOW_STYLE_BOTTOMRIGHT,
        RID_CUISTR_SHADOW_STYLE_TOPRIGHT,
        RID_CUISTR_SHADOW_STYLE_BOTTOMLEFT,
        RID_CUISTR_SHADOW_STYLE_TOPLEFT
    };

    return pnStrIds[ nIconViewIdx - 1 ];
}

TranslateId SvxBorderTabPage::GetPresetStringId( sal_uInt16 nIconViewIdx ) const
{
    // string resource IDs for each image (in order of the IID_PRE_* image IDs)
    static const TranslateId pnStrIds[] =
    {
        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_PARA_PRESET_ALL,
        RID_SVXSTR_PARA_PRESET_ONLYLEFTRIGHT,
        RID_SVXSTR_PARA_PRESET_ONLYTOPBOTTOM,
        RID_SVXSTR_PARA_PRESET_ONLYLEFT,
        RID_SVXSTR_PARA_PRESET_DIAGONAL,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_HOR_PRESET_ONLYHOR,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_VER_PRESET_ONLYVER,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_TABLE_PRESET_OUTERHORI,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER
    };
    return pnStrIds[ GetPresetImageId( nIconViewIdx ) - 1 ];
}

void SvxBorderTabPage::FillPresetIV()
{
    m_xWndPresets->clear();

    for( sal_uInt16 nIdx = 1; nIdx <= BORDER_PRESET_COUNT; ++nIdx )
    {
        OUString sId = OUString::number(nIdx);
        VclPtr<VirtualDevice> pVDev = GetVirtualDevice(m_aBorderImgVec[GetPresetImageId(nIdx) - 1]);
        m_xWndPresets->insert(-1, nullptr, &sId, pVDev, nullptr);
    }

    // show the control
    m_xWndPresets->unselect_all();
}

void SvxBorderTabPage::FillShadowIV()
{
    // Clear any existing items
    m_xWndShadows->clear();

    // insert images and help texts
    for( sal_uInt16 nIdx = 1; nIdx <= BORDER_SHADOW_COUNT; ++nIdx )
    {
        OUString sId = OUString::number(nIdx);
        VclPtr<VirtualDevice> pVDev = GetVirtualDevice(m_aShadowImgVec[nIdx-1]);
        m_xWndShadows->insert(-1, nullptr, &sId, pVDev, nullptr);
    }

    // show the control
    m_xWndShadows->select(0);
}

VclPtr<VirtualDevice> SvxBorderTabPage::GetVirtualDevice( Image pImage )
{
    BitmapEx aPreviewBitmap = pImage.GetBitmapEx();
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();

    if (pVDev->GetDPIScaleFactor() > 1)
        aPreviewBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());

    const Size aOriginalSize = aPreviewBitmap.GetSizePixel();
    const sal_Int32 nPadding = 3;
    const Size aPaddedSize(aOriginalSize.Width() + (nPadding * 2),
                           aOriginalSize.Height() + (nPadding * 2));

    pVDev->SetOutputSizePixel(aPaddedSize);

    const Point aCenteredPos(nPadding, nPadding);
    pVDev->DrawBitmapEx(aCenteredPos, aPreviewBitmap);
    return pVDev;
}

void SvxBorderTabPage::FillIconViews()
{
    FillPresetIV();
    FillShadowIV();
}

void SvxBorderTabPage::SetLineWidth( sal_Int64 nWidth, sal_Int32 nRemovedType )
{
    if ( nWidth >= 0 )
        m_xLineWidthMF->set_value( nWidth, FieldUnit::POINT );

    auto it = std::find( std::begin(s_LineWidths), std::end(s_LineWidths), nWidth );

    if ( it != std::end(s_LineWidths) && *it >= 0 )
    {
        // Select predefined value in combobox
        m_xLineWidthMF->hide();
        m_xLineWidthLB->set_active(std::distance(std::begin(s_LineWidths), it) - nRemovedType);
    }
    else
    {
        // This is not one of predefined values. Show spinner
        m_xLineWidthLB->set_active(std::size(s_LineWidths) - nRemovedType -1);
        m_xLineWidthMF->show();
    }
}

static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

void SvxBorderTabPage::FillLineListBox_Impl()
{
    using namespace ::com::sun::star::table::BorderLineStyle;

    static struct {
        SvxBorderLineStyle mnStyle;
        SvtLineListBox::ColorFunc mpColor1Fn;
        SvtLineListBox::ColorFunc mpColor2Fn;
        SvtLineListBox::ColorDistFunc mpColorDistFn;
    } const aLines[] = {
        // Simple lines
        { SvxBorderLineStyle::SOLID,        &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DOTTED,       &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASHED,       &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::FINE_DASHED,  &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASH_DOT,     &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASH_DOT_DOT, &sameColor, &sameColor, &sameDistColor },

        // Double lines
        { SvxBorderLineStyle::DOUBLE,              &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DOUBLE_THIN,         &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_SMALLGAP,  &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_MEDIUMGAP, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_LARGEGAP,  &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_SMALLGAP,  &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_MEDIUMGAP, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_LARGEGAP,  &sameColor, &sameColor, &sameDistColor },

        { SvxBorderLineStyle::EMBOSSED, &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor, &lcl_mediumColor },
        { SvxBorderLineStyle::ENGRAVED, &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor, &lcl_mediumColor },

        { SvxBorderLineStyle::OUTSET, &SvxBorderLine::lightColor, &SvxBorderLine::darkColor, &sameDistColor },
        { SvxBorderLineStyle::INSET,  &SvxBorderLine::darkColor, &SvxBorderLine::lightColor, &sameDistColor }
    };

    m_xLbLineStyle->SetSourceUnit( FieldUnit::TWIP );

    for (size_t i = 0; i < std::size(aLines); ++i)
    {
        if (!IsBorderLineStyleAllowed(aLines[i].mnStyle))
            continue;

        m_xLbLineStyle->InsertEntry(
            SvxBorderLine::getWidthImpl(aLines[i].mnStyle),
            aLines[i].mnStyle,
            lcl_GetMinLineWidth(aLines[i].mnStyle),
            aLines[i].mpColor1Fn,
            aLines[i].mpColor2Fn,
            aLines[i].mpColorDistFn);
    }

    sal_Int64 nVal = m_xLineWidthMF->get_value(FieldUnit::NONE);
    nVal = static_cast<sal_Int64>(vcl::ConvertDoubleValue(nVal, m_xLineWidthMF->get_digits(),
                                                                  m_xLineWidthMF->get_unit(), MapUnit::MapTwip));
    m_xLbLineStyle->SetWidth( nVal );
}


IMPL_LINK_NOARG(SvxBorderTabPage, LinesChanged_Impl, LinkParamNone*, void)
{
    if (!mbUseMarginItem && m_xLeftMF->get_visible())
    {
        bool bLineSet = m_aFrameSel.IsAnyBorderVisible();
        bool bSpaceModified =   mbLeftModified ||
                                mbRightModified ||
                                mbTopModified ||
                                mbBottomModified;

        if(bLineSet)
        {
            if(!bSpaceModified)
            {
                m_xLeftMF->set_value(nMinValue, FieldUnit::NONE);
                m_xRightMF->set_value(nMinValue, FieldUnit::NONE);
                m_xTopMF->set_value(nMinValue, FieldUnit::NONE);
                m_xBottomMF->set_value(nMinValue, FieldUnit::NONE);
            }
        }
        else
        {
            m_xLeftMF->set_min(0, FieldUnit::NONE);
            m_xRightMF->set_min(0, FieldUnit::NONE);
            m_xTopMF->set_min(0, FieldUnit::NONE);
            m_xBottomMF->set_min(0, FieldUnit::NONE);
        }
        // for tables everything is allowed
        SvxBoxInfoItemValidFlags nValid = SvxBoxInfoItemValidFlags::TOP|SvxBoxInfoItemValidFlags::BOTTOM|SvxBoxInfoItemValidFlags::LEFT|SvxBoxInfoItemValidFlags::RIGHT;

        m_xLeftFT->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::LEFT) );
        m_xRightFT->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::RIGHT) );
        m_xTopFT->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::TOP) );
        m_xBottomFT->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::BOTTOM) );
        m_xLeftMF->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::LEFT) );
        m_xRightMF->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::RIGHT) );
        m_xTopMF->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::TOP) );
        m_xBottomMF->set_sensitive( bool(nValid & SvxBoxInfoItemValidFlags::BOTTOM) );
        m_xSynchronizeCB->set_sensitive(m_xRightMF->get_sensitive() || m_xTopMF->get_sensitive() ||
                                        m_xBottomMF->get_sensitive() || m_xLeftMF->get_sensitive());
    }
    UpdateRemoveAdjCellBorderCB( SAL_MAX_UINT16 );
}


IMPL_LINK( SvxBorderTabPage, ModifyDistanceHdl_Impl, weld::MetricSpinButton&, rField, void)
{
    if (&rField == m_xLeftMF.get())
        mbLeftModified = true;
    else if (&rField == m_xRightMF.get())
        mbRightModified = true;
    else if (&rField == m_xTopMF.get())
        mbTopModified = true;
    else if (&rField == m_xBottomMF.get())
        mbBottomModified = true;

    if (mbSync)
    {
        const auto nVal = rField.get_value(FieldUnit::NONE);
        if (&rField != m_xLeftMF.get())
            m_xLeftMF->set_value(nVal, FieldUnit::NONE);
        if (&rField != m_xRightMF.get())
            m_xRightMF->set_value(nVal, FieldUnit::NONE);
        if (&rField != m_xTopMF.get())
            m_xTopMF->set_value(nVal, FieldUnit::NONE);
        if (&rField != m_xBottomMF.get())
            m_xBottomMF->set_value(nVal, FieldUnit::NONE);
    }
}

IMPL_LINK( SvxBorderTabPage, SyncHdl_Impl, weld::Toggleable&, rBox, void)
{
    mbSync = rBox.get_active();
}

IMPL_LINK( SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl, weld::Toggleable&, rBox, void)
{
    mbRemoveAdjacentCellBorders = rBox.get_active();
}

void SvxBorderTabPage::UpdateRemoveAdjCellBorderCB( sal_uInt16 nPreset )
{
    if( !bIsCalcDoc )
        return;
    const SfxItemSet&     rOldSet         = GetItemSet();
    const SvxBoxInfoItem* pOldBoxInfoItem = GetOldItem( rOldSet, SID_ATTR_BORDER_INNER );
    const SvxBoxItem*     pOldBoxItem     = static_cast<const SvxBoxItem*>(GetOldItem( rOldSet, mnBoxSlot ));
    if( !pOldBoxInfoItem || !pOldBoxItem )
        return;
    std::pair<svx::FrameBorderType, SvxBoxInfoItemValidFlags> eTypes1[] = {
        { svx::FrameBorderType::Top,SvxBoxInfoItemValidFlags::TOP },
        { svx::FrameBorderType::Bottom,SvxBoxInfoItemValidFlags::BOTTOM },
        { svx::FrameBorderType::Left,SvxBoxInfoItemValidFlags::LEFT },
        { svx::FrameBorderType::Right,SvxBoxInfoItemValidFlags::RIGHT },
    };
    SvxBoxItemLine const eTypes2[] = {
        SvxBoxItemLine::TOP,
        SvxBoxItemLine::BOTTOM,
        SvxBoxItemLine::LEFT,
        SvxBoxItemLine::RIGHT,
    };

    // Check if current selection involves deletion of at least one border
    bool bBorderDeletionReq = false;
    for ( size_t i=0; i < std::size( eTypes1 ); ++i )
    {
        if( pOldBoxItem->GetLine( eTypes2[i] ) || !( pOldBoxInfoItem->IsValid( eTypes1[i].second ) ) )
        {
            if( m_aFrameSel.GetFrameBorderState( eTypes1[i].first ) == svx::FrameBorderState::Hide )
            {
                bBorderDeletionReq = true;
                break;
            }
        }
    }

    if( !bBorderDeletionReq && ( nPreset == IID_PRE_CELL_NONE || nPreset == IID_PRE_TABLE_NONE ) )
        bBorderDeletionReq = true;

    m_xRemoveAdjacentCellBordersCB->set_sensitive(bBorderDeletionReq);

    if( !bBorderDeletionReq )
    {
        mbRemoveAdjacentCellBorders = false;
        m_xRemoveAdjacentCellBordersCB->set_active(false);
    }
}

void SvxBorderTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pSWModeItem = aSet.GetItem<SfxUInt16Item>(SID_SWMODE_TYPE, false);
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pSWModeItem)
    {
        nSWMode = static_cast<SwBorderModes>(pSWModeItem->GetValue());
        // #i43593#
        // show checkbox <m_xMergeWithNextCB> for format.paragraph
        if ( nSWMode == SwBorderModes::PARA )
        {
            m_xMergeWithNextCB->show();
            m_xPropertiesFrame->show();
        }
        // show checkbox <m_xMergeAdjacentBordersCB> for format.paragraph
        else if ( nSWMode == SwBorderModes::TABLE )
        {
            m_xMergeAdjacentBordersCB->show();
            m_xPropertiesFrame->show();
        }
    }
    if (pFlagItem)
        if ( ( pFlagItem->GetValue() & SVX_HIDESHADOWCTL ) == SVX_HIDESHADOWCTL )
            HideShadowControls();
}

void SvxBorderTabPage::SetTableMode()
{
    nSWMode = SwBorderModes::TABLE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
