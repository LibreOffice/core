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

#include <sfx2/objsh.hxx>
#include <svx/svxids.hrc>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <border.hxx>
#include <svx/dlgutil.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svx/flagsdef.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/int64item.hxx>
#include <sal/macros.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/lok.hxx>
#include <svtools/unitconv.hxx>

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

const sal_uInt16 SvxBorderTabPage::pRanges[] =
{
    SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_SHADOW,
    SID_ATTR_ALIGN_MARGIN,      SID_ATTR_ALIGN_MARGIN,
    SID_ATTR_BORDER_CONNECT,    SID_ATTR_BORDER_CONNECT,
    SID_SW_COLLAPSING_BORDERS,  SID_SW_COLLAPSING_BORDERS,
    SID_ATTR_BORDER_DIAG_TLBR,  SID_ATTR_BORDER_DIAG_BLTR,
    0
};

namespace
{
int lcl_twipsToPt(sal_Int64 nTwips)
{
    return MetricField::ConvertDoubleValue(nTwips, 0, FieldUnit::TWIP, MapUnit::MapPoint) * 100;
}
}

const std::vector<int> SvxBorderTabPage::m_aLineWidths = {
    lcl_twipsToPt(SvxBorderLineWidth::Hairline),
    lcl_twipsToPt(SvxBorderLineWidth::VeryThin),
    lcl_twipsToPt(SvxBorderLineWidth::Thin),
    lcl_twipsToPt(SvxBorderLineWidth::Medium),
    lcl_twipsToPt(SvxBorderLineWidth::Thick),
    lcl_twipsToPt(SvxBorderLineWidth::ExtraThick),
    -1
};

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
const sal_uInt16 SVX_BORDER_PRESET_COUNT = 5;

// number of shadow images to show
const sal_uInt16 SVX_BORDER_SHADOW_COUNT = 5;

ShadowControlsWrapper::ShadowControlsWrapper(SvtValueSet& rVsPos, weld::MetricSpinButton& rMfSize, ColorListBox& rLbColor)
    : mrVsPos(rVsPos)
    , mrMfSize(rMfSize)
    , mrLbColor(rLbColor)
{
}

SvxShadowItem ShadowControlsWrapper::GetControlValue(const SvxShadowItem& rItem) const
{
    SvxShadowItem aItem(rItem);
    if (!mrVsPos.IsNoSelection())
    {
        switch (mrVsPos.GetSelectedItemId())
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
            mrVsPos.SelectItem(1);
            break;
        case SvxShadowLocation::BottomRight:
            mrVsPos.SelectItem(2);
            break;
        case SvxShadowLocation::TopRight:
            mrVsPos.SelectItem(3);
            break;
        case SvxShadowLocation::BottomLeft:
            mrVsPos.SelectItem(4);
            break;
        case SvxShadowLocation::TopLeft:
            mrVsPos.SelectItem(5);
            break;
        default:
            mrVsPos.SetNoSelection();
            break;
    }
    mrVsPos.SaveValue();
    mrMfSize.set_value(mrMfSize.normalize(rItem.GetWidth()), FieldUnit::TWIP);
    mrMfSize.save_value();
    mrLbColor.SelectEntry(rItem.GetColor());
    mrLbColor.SaveValue();
}

bool ShadowControlsWrapper::get_value_changed_from_saved() const
{
    return mrVsPos.IsValueChangedFromSaved() ||
           mrMfSize.get_value_changed_from_saved() ||
           mrLbColor.IsValueChangedFromSaved();
}

void ShadowControlsWrapper::SetControlDontKnow()
{
    mrVsPos.SetNoSelection();
    mrMfSize.set_text("");
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
    : SfxTabPage(pPage, pController, "cui/ui/borderpage.ui", "BorderPage", &rCoreAttrs)
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
    , m_xWndPresets(new SvtValueSet(nullptr))
    , m_xWndPresetsWin(new weld::CustomWeld(*m_xBuilder, "presets", *m_xWndPresets))
    , m_xUserDefFT(m_xBuilder->weld_label("userdefft"))
    , m_xFrameSelWin(new weld::CustomWeld(*m_xBuilder, "framesel", m_aFrameSel))
    , m_xLbLineStyle(new SvtLineListBox(m_xBuilder->weld_menu_button("linestylelb")))
    , m_xLbLineColor(new ColorListBox(m_xBuilder->weld_menu_button("linecolorlb"), pController->getDialog()))
    , m_xLineWidthLB(m_xBuilder->weld_combo_box("linewidthlb"))
    , m_xLineWidthMF(m_xBuilder->weld_metric_spin_button("linewidthmf", FieldUnit::POINT))
    , m_xSpacingFrame(m_xBuilder->weld_container("spacing"))
    , m_xLeftFT(m_xBuilder->weld_label("leftft"))
    , m_xLeftMF(m_xBuilder->weld_metric_spin_button("leftmf", FieldUnit::MM))
    , m_xRightFT(m_xBuilder->weld_label("rightft"))
    , m_xRightMF(m_xBuilder->weld_metric_spin_button("rightmf", FieldUnit::MM))
    , m_xTopFT(m_xBuilder->weld_label("topft"))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button("topmf", FieldUnit::MM))
    , m_xBottomFT(m_xBuilder->weld_label("bottomft"))
    , m_xBottomMF(m_xBuilder->weld_metric_spin_button("bottommf", FieldUnit::MM))
    , m_xSynchronizeCB(m_xBuilder->weld_check_button("sync"))
    , m_xShadowFrame(m_xBuilder->weld_container("shadow"))
    , m_xWndShadows(new SvtValueSet(nullptr))
    , m_xWndShadowsWin(new weld::CustomWeld(*m_xBuilder, "shadows", *m_xWndShadows))
    , m_xFtShadowSize(m_xBuilder->weld_label("distanceft"))
    , m_xEdShadowSize(m_xBuilder->weld_metric_spin_button("distancemf", FieldUnit::MM))
    , m_xFtShadowColor(m_xBuilder->weld_label("shadowcolorft"))
    , m_xLbShadowColor(new ColorListBox(m_xBuilder->weld_menu_button("shadowcolorlb"), pController->getDialog()))
    , m_xPropertiesFrame(m_xBuilder->weld_container("properties"))
    , m_xMergeWithNextCB(m_xBuilder->weld_check_button("mergewithnext"))
    , m_xMergeAdjacentBordersCB(m_xBuilder->weld_check_button("mergeadjacent"))
    , m_xRemoveAdjcentCellBordersCB(m_xBuilder->weld_check_button("rmadjcellborders"))
    , m_xRemoveAdjcentCellBordersFT(m_xBuilder->weld_label("rmadjcellbordersft"))
{
    static std::vector<OUStringLiteral> aBorderImageIds;

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

    static std::vector<OUStringLiteral> aShadowImageIds;
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

    assert(m_aShadowImgVec.size() == SVX_BORDER_SHADOW_COUNT);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    /*  Use SvxMarginItem instead of margins from SvxBoxItem, if present.
        ->  Remember this state in mbUseMarginItem, because other special handling
            is needed across various functions... */
    mbUseMarginItem = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_MARGIN)) != SfxItemState::UNKNOWN;

    const SfxPoolItem* pItem = nullptr;
    if (rCoreAttrs.HasItem(SID_ATTR_BORDER_STYLES, &pItem))
    {
        const SfxIntegerListItem* p = static_cast<const SfxIntegerListItem*>(pItem);
        std::vector<sal_Int32> aUsedStyles = p->GetList();
        for (int aUsedStyle : aUsedStyles)
            maUsedBorderStyles.insert(static_cast<SvxBorderLineStyle>(aUsedStyle));
    }

    if (rCoreAttrs.HasItem(SID_ATTR_BORDER_DEFAULT_WIDTH, &pItem))
    {
        // The caller specifies default line width.  Honor it.
        const SfxInt64Item* p = static_cast<const SfxInt64Item*>(pItem);
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
    m_xWndPresets->SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    m_xWndShadows->SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets();
    FillLineListBox_Impl();

    // Reapply line width: probably one of prefefined values should be selected
    SetLineWidth(m_xLineWidthMF->get_value(FieldUnit::NONE));

    // connections
    if (rCoreAttrs.HasItem(GetWhich(SID_ATTR_PARA_GRABBAG), &pItem))
    {
        const SfxGrabBagItem* pGrabBag = static_cast<const SfxGrabBagItem*>(pItem);
        auto it = pGrabBag->GetGrabBag().find("DialogUseCharAttr");
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

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if (pDocSh)
    {
        Reference< XServiceInfo > xSI( pDocSh->GetModel(), UNO_QUERY );
        if ( xSI.is() )
            bIsCalcDoc = xSI->supportsService("com.sun.star.sheet.SpreadsheetDocument");
    }
    if( bIsCalcDoc )
    {
        m_xRemoveAdjcentCellBordersCB->connect_toggled(LINK(this, SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl));
        m_xRemoveAdjcentCellBordersCB->show();
        m_xRemoveAdjcentCellBordersCB->set_sensitive(false);
    }
    else
    {
        m_xRemoveAdjcentCellBordersCB->hide();
        m_xRemoveAdjcentCellBordersFT->hide();
    }
}

SvxBorderTabPage::~SvxBorderTabPage()
{
    m_xLbShadowColor.reset();
    m_xWndShadowsWin.reset();
    m_xWndShadows.reset();
    m_xLbLineColor.reset();
    m_xLbLineStyle.reset();
    m_xFrameSelWin.reset();
    m_xWndPresetsWin.reset();
    m_xWndPresets.reset();
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
        sal_uInt16 nBorderDiagId = pPool->GetWhich(SID_ATTR_BORDER_DIAG_TLBR);
        if (const SvxLineItem* pLineItem = static_cast<const SvxLineItem*>(rSet->GetItem(nBorderDiagId)))
            m_aFrameSel.ShowBorder(svx::FrameBorderType::TLBR, pLineItem->GetLine());
        else
            m_aFrameSel.SetBorderDontCare(svx::FrameBorderType::TLBR);
    }

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::BLTR))
    {
        sal_uInt16 nBorderDiagId = pPool->GetWhich(SID_ATTR_BORDER_DIAG_BLTR);
        if (const SvxLineItem* pLineItem = static_cast<const SvxLineItem*>(rSet->GetItem(nBorderDiagId)))
            m_aFrameSel.ShowBorder(svx::FrameBorderType::BLTR, pLineItem->GetLine());
        else
            m_aFrameSel.SetBorderDontCare(svx::FrameBorderType::BLTR);
    }

    if (m_xShadowControls)
    {
        sal_uInt16 nShadowId = pPool->GetWhich(mnShadowSlot);
        const SfxPoolItem* pItem = rSet->GetItem(nShadowId);
        if (pItem)
            m_xShadowControls->SetControlValue(*static_cast<const SvxShadowItem*>(pItem));
        else
            m_xShadowControls->SetControlDontKnow();
    }

    if (m_xMarginControls)
    {
        sal_uInt16 nAlignMarginId = pPool->GetWhich(SID_ATTR_ALIGN_MARGIN);
        const SfxPoolItem* pItem = rSet->GetItem(nAlignMarginId);
        if (pItem)
            m_xMarginControls->SetControlValue(*static_cast<const SvxMarginItem*>(pItem));
        else
            m_xMarginControls->SetControlDontKnow();
    }

    sal_uInt16 nMergeAdjacentBordersId = pPool->GetWhich(SID_SW_COLLAPSING_BORDERS);
    const SfxBoolItem *pMergeAdjacentBorders = static_cast<const SfxBoolItem*>(rSet->GetItem(nMergeAdjacentBordersId));
    if (!pMergeAdjacentBorders)
        m_xMergeAdjacentBordersCB->set_state(TRISTATE_INDET);
    else
        m_xMergeAdjacentBordersCB->set_active(pMergeAdjacentBorders->GetValue());
    m_xMergeAdjacentBordersCB->save_state();

    sal_uInt16 nMergeWithNextId = pPool->GetWhich(SID_ATTR_BORDER_CONNECT);
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
                        long nLeftDist = pBoxItem->GetDistance( SvxBoxItemLine::LEFT);
                        SetMetricValue(*m_xLeftMF, nLeftDist, eCoreUnit);
                        long nRightDist = pBoxItem->GetDistance( SvxBoxItemLine::RIGHT);
                        SetMetricValue(*m_xRightMF, nRightDist, eCoreUnit);
                        long nTopDist = pBoxItem->GetDistance( SvxBoxItemLine::TOP);
                        SetMetricValue( *m_xTopMF, nTopDist, eCoreUnit );
                        long nBottomDist = pBoxItem->GetDistance( SvxBoxItemLine::BOTTOM);
                        SetMetricValue( *m_xBottomMF, nBottomDist, eCoreUnit );

                        // if the distance is set with no active border line
                        // or it is null with an active border line
                        // no automatic changes should be made
                        const long nDefDist = bIsAnyBorderVisible ? pBoxInfoItem->GetDefDist() : 0;
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
        long nWidth;
        SvxBorderLineStyle nStyle;
        bool bWidthEq = m_aFrameSel.GetVisibleWidth( nWidth, nStyle );
        if( bWidthEq )
        {
            // Determine the width first as some styles can be missing depending on it
            sal_Int64 nWidthPt =  static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
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

    bool bEnable = m_xWndShadows->GetSelectedItemId() > 1 ;
    m_xFtShadowSize->set_sensitive(bEnable);
    m_xEdShadowSize->set_sensitive(bEnable);
    m_xFtShadowColor->set_sensitive(bEnable);
    m_xLbShadowColor->set_sensitive(bEnable);

    m_xWndPresets->SetNoSelection();

    // - no line - should not be selected

    if (m_xLbLineStyle->GetSelectEntryStyle() == SvxBorderLineStyle::NONE)
    {
        m_xLbLineStyle->SelectEntry(SvxBorderLineStyle::SOLID);
        SelStyleHdl_Impl(*m_xLbLineStyle);
    }

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            // there are no shadows in Html-mode and only complete borders
            m_xShadowFrame->set_sensitive(false);

            if( !(nSWMode & SwBorderModes::TABLE) )
            {
                m_xUserDefFT->set_sensitive(false);
                m_xFrameSelWin->set_sensitive(false);
                m_xWndPresets->RemoveItem(3);
                m_xWndPresets->RemoveItem(4);
                m_xWndPresets->RemoveItem(5);
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
    m_xRemoveAdjcentCellBordersCB->set_active(false);
    m_xRemoveAdjcentCellBordersCB->set_sensitive(false);
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

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::TLBR))
    {
        sal_uInt16 nBorderDiagId = pPool->GetWhich(SID_ATTR_BORDER_DIAG_TLBR);
        SvxLineItem aLineItem(*static_cast<const SvxLineItem*>(rCoreAttrs->GetItem(nBorderDiagId)));
        aLineItem.SetLine(m_aFrameSel.GetFrameBorderStyle(svx::FrameBorderType::TLBR));
        rCoreAttrs->Put(aLineItem);
        bAttrsChanged = true;
    }

    if (m_aFrameSel.IsBorderEnabled(svx::FrameBorderType::BLTR))
    {
        sal_uInt16 nBorderDiagId = pPool->GetWhich(SID_ATTR_BORDER_DIAG_BLTR);
        SvxLineItem aLineItem(*static_cast<const SvxLineItem*>(rCoreAttrs->GetItem(nBorderDiagId)));
        aLineItem.SetLine(m_aFrameSel.GetFrameBorderStyle(svx::FrameBorderType::BLTR));
        rCoreAttrs->Put(aLineItem);
        bAttrsChanged = true;
    }

    if (m_xShadowControls && m_xShadowControls->get_value_changed_from_saved())
    {
        sal_uInt16 nShadowId = pPool->GetWhich(mnShadowSlot);
        const SvxShadowItem& rOldShadowItem = *static_cast<const SvxShadowItem*>(rCoreAttrs->GetItem(nShadowId));
        rCoreAttrs->Put(m_xShadowControls->GetControlValue(rOldShadowItem));
        bAttrsChanged = true;
    }

    if (m_xMarginControls && m_xMarginControls->get_value_changed_from_saved())
    {
        sal_uInt16 nAlignMarginId = pPool->GetWhich(SID_ATTR_ALIGN_MARGIN);
        const SvxMarginItem& rOldMarginItem = *static_cast<const SvxMarginItem*>(rCoreAttrs->GetItem(nAlignMarginId));
        rCoreAttrs->Put(m_xMarginControls->GetControlValue(rOldMarginItem));
        bAttrsChanged = true;
    }

    if (m_xMergeAdjacentBordersCB->get_state_changed_from_saved())
    {
        sal_uInt16 nMergeAdjacentBordersId = pPool->GetWhich(SID_SW_COLLAPSING_BORDERS);
        auto nState = m_xMergeAdjacentBordersCB->get_state();
        if (nState == TRISTATE_INDET)
            rCoreAttrs->ClearItem(nMergeAdjacentBordersId);
        else
        {
            std::unique_ptr<SfxBoolItem> xNewItem(static_cast<SfxBoolItem*>(rCoreAttrs->Get(nMergeAdjacentBordersId).Clone()));
            xNewItem->SetValue(static_cast<bool>(nState));
            rCoreAttrs->Put(std::move(xNewItem));
        }
        bAttrsChanged = true;
    }

    if (m_xMergeWithNextCB->get_state_changed_from_saved())
    {
        sal_uInt16 nMergeWithNextId = pPool->GetWhich(SID_ATTR_BORDER_CONNECT);
        auto nState = m_xMergeWithNextCB->get_state();
        if (nState == TRISTATE_INDET)
            rCoreAttrs->ClearItem(nMergeWithNextId);
        else
        {
            std::unique_ptr<SfxBoolItem> xNewItem(static_cast<SfxBoolItem*>(rCoreAttrs->Get(nMergeWithNextId).Clone()));
            xNewItem->SetValue(static_cast<bool>(nState));
            rCoreAttrs->Put(std::move(xNewItem));
        }
        bAttrsChanged = true;
    }

    bool                  bPut          = true;
    sal_uInt16            nBoxWhich     = GetWhich( mnBoxSlot );
    sal_uInt16            nBoxInfoWhich = pPool->GetWhich( SID_ATTR_BORDER_INNER, false );
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

IMPL_LINK_NOARG(SvxBorderTabPage, SelPreHdl_Impl, SvtValueSet*, void)
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
    sal_uInt16 nLine = GetPresetImageId( m_xWndPresets->GetSelectedItemId() ) - 1;

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

    // Presets ValueSet does not show a selection (used as push buttons).
    m_xWndPresets->SetNoSelection();

    LinesChanged_Impl( nullptr );
    UpdateRemoveAdjCellBorderCB( nLine + 1 );
}

IMPL_LINK_NOARG(SvxBorderTabPage, SelSdwHdl_Impl, SvtValueSet*, void)
{
    bool bEnable = m_xWndShadows->GetSelectedItemId() > 1;
    m_xFtShadowSize->set_sensitive(bEnable);
    m_xEdShadowSize->set_sensitive(bEnable);
    m_xFtShadowColor->set_sensitive(bEnable);
    m_xLbShadowColor->set_sensitive(bEnable);
}

IMPL_LINK(SvxBorderTabPage, SelColHdl_Impl, ColorListBox&, rColorBox, void)
{
    Color aColor = rColorBox.GetSelectEntryColor();
    m_aFrameSel.SetColorToSelection(aColor);
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthLBHdl_Impl, weld::ComboBox&, void)
{
    sal_Int32 nPos = m_xLineWidthLB->get_active();

    SetLineWidth(m_aLineWidths[nPos]);

    // Call the spinner handler to trigger all related modifications
    ModifyWidthMFHdl_Impl(*m_xLineWidthMF);
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthMFHdl_Impl, weld::MetricSpinButton&, void)
{
    sal_Int64 nVal = m_xLineWidthMF->get_value(FieldUnit::NONE);
    nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
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
    nOldWidth = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
        nOldWidth,
        m_xLineWidthMF->get_digits(),
        FieldUnit::POINT,
        MapUnit::MapTwip));

    const sal_Int64 nOldMinWidth = lcl_GetMinLineWidth(m_aFrameSel.getCurrentStyleLineStyle());
    const sal_Int64 nNewMinWidth = lcl_GetMinLineWidth(m_xLbLineStyle->GetSelectEntryStyle());

    // auto change line-width if it doesn't correspond to minimal value
    // let's change only in case when user has not changed the line-width into some custom value
    const sal_Int64 nNewWidth = (nOldMinWidth == nOldWidth)? nNewMinWidth : nOldWidth;

    // set value inside edit box
    if (nOldWidth != nNewWidth)
    {
        const sal_Int64 nNewWidthPt = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
            nNewWidth,
            m_xLineWidthMF->get_digits(),
            MapUnit::MapTwip,
            FieldUnit::POINT));
        SetLineWidth(nNewWidthPt);
    }

    // set value inside style box
    m_aFrameSel.SetStyleToSelection( nNewWidth,
        m_xLbLineStyle->GetSelectEntryStyle() );
}


// ValueSet handling
sal_uInt16 SvxBorderTabPage::GetPresetImageId( sal_uInt16 nValueSetIdx ) const
{
    // table with all sets of predefined border styles
    static const sal_uInt16 ppnImgIds[][ SVX_BORDER_PRESET_COUNT ] =
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

    DBG_ASSERT( (1 <= nValueSetIdx) && (nValueSetIdx <= SVX_BORDER_PRESET_COUNT),
        "SvxBorderTabPage::GetPresetImageId - wrong index" );
    return ppnImgIds[ nLine ][ nValueSetIdx - 1 ];
}

const char* SvxBorderTabPage::GetPresetStringId( sal_uInt16 nValueSetIdx ) const
{
    // string resource IDs for each image (in order of the IID_PRE_* image IDs)
    static const char* pnStrIds[] =
    {
        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_PARA_PRESET_ALL,
        RID_SVXSTR_PARA_PRESET_LEFTRIGHT,
        RID_SVXSTR_PARA_PRESET_TOPBOTTOM,
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
    return pnStrIds[ GetPresetImageId( nValueSetIdx ) - 1 ];
}

void SvxBorderTabPage::FillPresetVS()
{
    // basic initialization of the ValueSet
    m_xWndPresets->SetStyle( m_xWndPresets->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_xWndPresets->SetColCount( SVX_BORDER_PRESET_COUNT );

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_PRESET_COUNT; ++nVSIdx )
    {
        m_xWndPresets->InsertItem( nVSIdx );
        m_xWndPresets->SetItemImage(nVSIdx, m_aBorderImgVec[GetPresetImageId(nVSIdx) - 1]);
        m_xWndPresets->SetItemText( nVSIdx, CuiResId( GetPresetStringId( nVSIdx ) ) );
    }

    // show the control
    m_xWndPresets->SetNoSelection();
    m_xWndPresets->SetOptimalSize();
    m_xWndPresets->Show();
}

void SvxBorderTabPage::FillShadowVS()
{
    // basic initialization of the ValueSet
    m_xWndShadows->SetStyle( m_xWndShadows->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_xWndShadows->SetColCount( SVX_BORDER_SHADOW_COUNT );

    // string resource IDs for each image
    static const char* pnStrIds[ SVX_BORDER_SHADOW_COUNT ] =
        { RID_SVXSTR_SHADOW_STYLE_NONE, RID_SVXSTR_SHADOW_STYLE_BOTTOMRIGHT, RID_SVXSTR_SHADOW_STYLE_TOPRIGHT, RID_SVXSTR_SHADOW_STYLE_BOTTOMLEFT, RID_SVXSTR_SHADOW_STYLE_TOPLEFT };

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_SHADOW_COUNT; ++nVSIdx )
    {
        m_xWndShadows->InsertItem( nVSIdx );
        m_xWndShadows->SetItemImage(nVSIdx, m_aShadowImgVec[nVSIdx-1]);
        m_xWndShadows->SetItemText( nVSIdx, CuiResId( pnStrIds[ nVSIdx - 1 ] ) );
    }

    // show the control
    m_xWndShadows->SelectItem( 1 );
    m_xWndShadows->SetOptimalSize();
    m_xWndShadows->Show();
}


void SvxBorderTabPage::FillValueSets()
{
    FillPresetVS();
    FillShadowVS();
}

void SvxBorderTabPage::SetLineWidth( sal_Int64 nWidth )
{
    if ( nWidth >= 0 )
        m_xLineWidthMF->set_value( nWidth, FieldUnit::POINT );

    auto it = std::find_if( m_aLineWidths.begin(), m_aLineWidths.end(),
                            [nWidth](const int val) -> bool { return val == nWidth; } );

    if ( it != m_aLineWidths.end() && *it >= 0 )
    {
        // Select predefined value in combobox
        m_xLineWidthMF->hide();
        m_xLineWidthLB->set_active(std::distance(m_aLineWidths.begin(), it));
    }
    else
    {
        // This is not one of predefined values. Show spinner
        m_xLineWidthLB->set_active(m_aLineWidths.size()-1);
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

    for (size_t i = 0; i < SAL_N_ELEMENTS(aLines); ++i)
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
    nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(nVal, m_xLineWidthMF->get_digits(),
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

IMPL_LINK( SvxBorderTabPage, SyncHdl_Impl, weld::ToggleButton&, rBox, void)
{
    mbSync = rBox.get_active();
}

IMPL_LINK( SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl, weld::ToggleButton&, rBox, void)
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
    for ( size_t i=0; i < SAL_N_ELEMENTS( eTypes1 ); ++i )
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

    m_xRemoveAdjcentCellBordersCB->set_sensitive(bBorderDeletionReq);

    if( !bBorderDeletionReq )
    {
        mbRemoveAdjacentCellBorders = false;
        m_xRemoveAdjcentCellBordersCB->set_active(false);
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
