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

#include <memory>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/htmlmode.hxx>
#include <sal/macros.h>
#include <osl/diagnose.h>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <page.hxx>
#include <svx/pageitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/paperinf.hxx>
#include <svl/stritem.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <svx/svxids.hrc>
#include <svtools/optionsdrawinglayer.hxx>
#include <svl/slstitm.hxx>
#include <svx/xdef.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include <sal/log.hxx>
#include <svl/grabbagitem.hxx>

// static ----------------------------------------------------------------

// #i19922# - tdf#126051 see svx/source/dialog/hdft.cxx and sw/source/uibase/sidebar/PageMarginControl.hxx
const tools::Long MINBODY = 56;  // 1mm in twips rounded

const sal_uInt16 SvxPageDescPage::pRanges[] =
{
    SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,
    SID_ATTR_PAGE_SHARED,
    SID_SWREGISTER_COLLECTION,
    SID_SWREGISTER_MODE,
    0
};
// ------- Mapping page layout ------------------------------------------

const SvxPageUsage aArr[] =
{
    SvxPageUsage::All,
    SvxPageUsage::Mirror,
    SvxPageUsage::Right,
    SvxPageUsage::Left
};


static sal_uInt16 PageUsageToPos_Impl( SvxPageUsage nUsage )
{
    for ( size_t i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] ==  nUsage )
            return i;
    return 3;
}


static SvxPageUsage PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return SvxPageUsage::NONE;
    return aArr[nPos];
}


static Size GetMinBorderSpace_Impl( const SvxShadowItem& rShadow, const SvxBoxItem& rBox )
{
    Size aSz;
    aSz.setHeight( rShadow.CalcShadowSpace( SvxShadowItemSide::BOTTOM ) + rBox.CalcLineSpace( SvxBoxItemLine::BOTTOM ) );
    aSz.AdjustHeight(rShadow.CalcShadowSpace( SvxShadowItemSide::TOP ) + rBox.CalcLineSpace( SvxBoxItemLine::TOP ) );
    aSz.setWidth( rShadow.CalcShadowSpace( SvxShadowItemSide::LEFT ) + rBox.CalcLineSpace( SvxBoxItemLine::LEFT ) );
    aSz.AdjustWidth(rShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT ) + rBox.CalcLineSpace( SvxBoxItemLine::RIGHT ) );
    return aSz;
}


static tools::Long ConvertLong_Impl( const tools::Long nIn, MapUnit eUnit )
{
    return OutputDevice::LogicToLogic( nIn, eUnit, MapUnit::MapTwip );
}

static bool IsEqualSize_Impl( const SvxSizeItem* pSize, const Size& rSize )
{
    if ( pSize )
    {
        Size aSize = pSize->GetSize();
        tools::Long nDiffW = std::abs( rSize.Width () - aSize.Width () );
        tools::Long nDiffH = std::abs( rSize.Height() - aSize.Height() );
        return ( nDiffW < 10 && nDiffH < 10 );
    }
    else
        return false;
}


#define MARGIN_LEFT     ( MarginPosition(0x0001) )
#define MARGIN_RIGHT    ( MarginPosition(0x0002) )
#define MARGIN_TOP      ( MarginPosition(0x0004) )
#define MARGIN_BOTTOM   ( MarginPosition(0x0008) )

// class SvxPageDescPage --------------------------------------------------

std::unique_ptr<SfxTabPage> SvxPageDescPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxPageDescPage>(pPage, pController, *rSet);
}

SvxPageDescPage::SvxPageDescPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, "cui/ui/pageformatpage.ui", "PageFormatPage", &rAttr)
    , bLandscape(false)
    , eMode(SVX_PAGE_MODE_STANDARD)
    , ePaperStart(PAPER_A3)
    , m_nPos(0)
    , mpDefPrinter(nullptr)
    , mbDelPrinter(false)
    , mbEnableDrawingLayerFillStyles(false)
    , m_xPaperSizeBox(new SvxPaperSizeListBox(m_xBuilder->weld_combo_box("comboPageFormat")))
    , m_xPaperWidthEdit(m_xBuilder->weld_metric_spin_button("spinWidth", FieldUnit::CM))
    , m_xPaperHeightEdit(m_xBuilder->weld_metric_spin_button("spinHeight", FieldUnit::CM))
    , m_xOrientationFT(m_xBuilder->weld_label("labelOrientation"))
    , m_xPortraitBtn(m_xBuilder->weld_radio_button("radiobuttonPortrait"))
    , m_xLandscapeBtn(m_xBuilder->weld_radio_button("radiobuttonLandscape"))
    , m_xTextFlowLbl(m_xBuilder->weld_label("labelTextFlow"))
    , m_xTextFlowBox(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box("comboTextFlowBox")))
    , m_xPaperTrayBox(m_xBuilder->weld_combo_box("comboPaperTray"))
    , m_xLeftMarginLbl(m_xBuilder->weld_label("labelLeftMargin"))
    , m_xLeftMarginEdit(m_xBuilder->weld_metric_spin_button("spinMargLeft", FieldUnit::CM))
    , m_xRightMarginLbl(m_xBuilder->weld_label("labelRightMargin"))
    , m_xRightMarginEdit(m_xBuilder->weld_metric_spin_button("spinMargRight", FieldUnit::CM))
    , m_xTopMarginEdit(m_xBuilder->weld_metric_spin_button("spinMargTop", FieldUnit::CM))
    , m_xBottomMarginEdit(m_xBuilder->weld_metric_spin_button("spinMargBot", FieldUnit::CM))
    , m_xGutterMarginLbl(m_xBuilder->weld_label("labelGutterMargin"))
    , m_xGutterMarginEdit(m_xBuilder->weld_metric_spin_button("spinMargGut", FieldUnit::CM))
    , m_xPageText(m_xBuilder->weld_label("labelPageLayout"))
    , m_xLayoutBox(m_xBuilder->weld_combo_box("comboPageLayout"))
    , m_xNumberFormatText(m_xBuilder->weld_label("labelPageNumbers"))
    , m_xNumberFormatBox(new SvxPageNumberListBox(m_xBuilder->weld_combo_box("comboLayoutFormat")))
    , m_xTblAlignFT(m_xBuilder->weld_label("labelTblAlign"))
    , m_xHorzBox(m_xBuilder->weld_check_button("checkbuttonHorz"))
    , m_xVertBox(m_xBuilder->weld_check_button("checkbuttonVert"))
    , m_xAdaptBox(m_xBuilder->weld_check_button("checkAdaptBox"))
    , m_xRegisterCB(m_xBuilder->weld_check_button("checkRegisterTrue"))
    , m_xRegisterFT(m_xBuilder->weld_label("labelRegisterStyle"))
    , m_xRegisterLB(m_xBuilder->weld_combo_box("comboRegisterStyle"))
    , m_xGutterPositionFT(m_xBuilder->weld_label("labelGutterPosition"))
    , m_xGutterPositionLB(m_xBuilder->weld_combo_box("comboGutterPosition"))
    , m_xRtlGutterCB(m_xBuilder->weld_check_button("checkRtlGutter"))
    , m_xBackgroundFullSizeCB(m_xBuilder->weld_check_button("checkBackgroundFullSize"))
    // Strings stored in UI
    , m_xInsideLbl(m_xBuilder->weld_label("labelInner"))
    , m_xOutsideLbl(m_xBuilder->weld_label("labelOuter"))
    , m_xPrintRangeQueryText(m_xBuilder->weld_label("labelMsg"))
    , m_xBspWin(new weld::CustomWeld(*m_xBuilder, "drawingareaPageDirection", m_aBspWin))
{
    m_xRegisterLB->set_size_request(m_xRegisterLB->get_approximate_digit_width() * 20, -1);

    bBorderModified = false;
    m_aBspWin.EnableRTL(false);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    SvtLanguageOptions aLangOptions;
    bool bCJK = aLangOptions.IsAsianTypographyEnabled();
    bool bCTL = aLangOptions.IsCTLFontEnabled();
    bool bWeb = false;
    const SfxPoolItem* pItem;

    SfxObjectShell* pShell;
    if(SfxItemState::SET == rAttr.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
        bWeb = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);

    //  fill text flow listbox with valid entries

    m_xTextFlowBox->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_PAGEDIR_LTR_HORI));

    if (bCTL)
        m_xTextFlowBox->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_PAGEDIR_RTL_HORI));


    // #109989# do not show vertical directions in Writer/Web
    if( !bWeb && bCJK )
    {
        m_xTextFlowBox->append(SvxFrameDirection::Vertical_RL_TB, SvxResId(RID_SVXSTR_PAGEDIR_RTL_VERT));
        m_xTextFlowBox->append(SvxFrameDirection::Vertical_LR_TB, SvxResId(RID_SVXSTR_PAGEDIR_LTR_VERT));
    }

    // #109989# show the text direction box in Writer/Web too
    if( (bCJK || bCTL) &&
        SfxItemState::UNKNOWN < rAttr.GetItemState(GetWhich( SID_ATTR_FRAMEDIRECTION )))
    {
        m_xTextFlowLbl->show();
        m_xTextFlowBox->show();
        m_xTextFlowBox->connect_changed(LINK(this, SvxPageDescPage, FrameDirectionModify_Impl));

        m_aBspWin.EnableFrameDirection(true);
    }
    Init_Impl();

    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( *m_xLeftMarginEdit, eFUnit );
    SetFieldUnit( *m_xRightMarginEdit, eFUnit );
    SetFieldUnit( *m_xTopMarginEdit, eFUnit );
    SetFieldUnit( *m_xBottomMarginEdit, eFUnit );
    SetFieldUnit(*m_xGutterMarginEdit, eFUnit);
    SetFieldUnit( *m_xPaperWidthEdit, eFUnit );
    SetFieldUnit( *m_xPaperHeightEdit, eFUnit );

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
    {
        mpDefPrinter = SfxViewShell::Current()->GetPrinter();
    }
    else
    {
        mpDefPrinter = VclPtr<Printer>::Create();
        mbDelPrinter = true;
    }

    MapMode aOldMode = mpDefPrinter->GetMapMode();
    mpDefPrinter->SetMapMode(MapMode(MapUnit::MapTwip));

    // set first- and last-values for the margins
    Size aPaperSize = mpDefPrinter->GetPaperSize();
    Size aPrintSize = mpDefPrinter->GetOutputSize();

    /*
     * To convert a point ( 0,0 ) into logic coordinates
     * looks like nonsense; but it makes sense when the
     * coordinate system's origin has been moved.
     */
    Point aPrintOffset = mpDefPrinter->GetPageOffset() - mpDefPrinter->PixelToLogic( Point() );
    mpDefPrinter->SetMapMode( aOldMode );

    nFirstLeftMargin = m_xLeftMarginEdit->convert_value_from(m_xLeftMarginEdit->normalize(aPrintOffset.X()), FieldUnit::TWIP);
    nFirstRightMargin = m_xRightMarginEdit->convert_value_from(m_xRightMarginEdit->normalize(aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X()), FieldUnit::TWIP);
    nFirstTopMargin = m_xTopMarginEdit->convert_value_from(m_xTopMarginEdit->normalize(aPrintOffset.Y() ), FieldUnit::TWIP);
    nFirstBottomMargin = m_xBottomMarginEdit->convert_value_from(m_xBottomMarginEdit->normalize(aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y()), FieldUnit::TWIP );
    nLastLeftMargin = m_xLeftMarginEdit->convert_value_from(m_xLeftMarginEdit->normalize(aPrintOffset.X() + aPrintSize.Width()), FieldUnit::TWIP);
    nLastRightMargin = m_xRightMarginEdit->convert_value_from(m_xRightMarginEdit->normalize(aPrintOffset.X() + aPrintSize.Width()), FieldUnit::TWIP);
    nLastTopMargin = m_xTopMarginEdit->convert_value_from(m_xTopMarginEdit->normalize(aPrintOffset.Y() + aPrintSize.Height()), FieldUnit::TWIP);
    nLastBottomMargin = m_xBottomMarginEdit->convert_value_from(m_xBottomMarginEdit->normalize(aPrintOffset.Y() + aPrintSize.Height()), FieldUnit::TWIP);

    // #i4219# get DrawingLayer options
    const SvtOptionsDrawinglayer aDrawinglayerOpt;

    // #i4219# take Maximum now from configuration (1/100th cm)
    // was: 11900 -> 119 cm ;new value 3 meters -> 300 cm -> 30000
    m_xPaperWidthEdit->set_max(m_xPaperWidthEdit->normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FieldUnit::CM);
    m_xPaperHeightEdit->set_max(m_xPaperHeightEdit->normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FieldUnit::CM);

    // #i4219# also for margins (1/100th cm). Was: 9999, keeping.
    m_xLeftMarginEdit->set_max(m_xLeftMarginEdit->normalize(aDrawinglayerOpt.GetMaximumPaperLeftMargin()), FieldUnit::MM);
    m_xRightMarginEdit->set_max(m_xRightMarginEdit->normalize(aDrawinglayerOpt.GetMaximumPaperRightMargin()), FieldUnit::MM);
    m_xTopMarginEdit->set_max(m_xTopMarginEdit->normalize(aDrawinglayerOpt.GetMaximumPaperTopMargin()), FieldUnit::MM);
    m_xBottomMarginEdit->set_max(m_xBottomMarginEdit->normalize(aDrawinglayerOpt.GetMaximumPaperBottomMargin()), FieldUnit::MM);
    m_xGutterMarginEdit->set_max(
        m_xGutterMarginEdit->normalize(aDrawinglayerOpt.GetMaximumPaperLeftMargin()),
        FieldUnit::MM);

    // Get the i18n framework numberings and add them to the listbox.
    SvxNumOptionsTabPageHelper::GetI18nNumbering(m_xNumberFormatBox->get_widget(), std::numeric_limits<sal_uInt16>::max());
}

SvxPageDescPage::~SvxPageDescPage()
{
    if(mbDelPrinter)
    {
        mpDefPrinter.disposeAndClear();
        mbDelPrinter = false;
    }
}

void SvxPageDescPage::Init_Impl()
{
    // adjust the handler
    m_xLayoutBox->connect_changed(LINK(this, SvxPageDescPage, LayoutHdl_Impl));
    m_xGutterPositionLB->connect_changed(LINK(this, SvxPageDescPage, GutterPositionHdl_Impl));

    m_xPaperSizeBox->connect_changed(LINK(this, SvxPageDescPage, PaperSizeSelect_Impl));
    m_xPaperWidthEdit->connect_value_changed( LINK(this, SvxPageDescPage, PaperSizeModify_Impl));
    m_xPaperHeightEdit->connect_value_changed(LINK(this, SvxPageDescPage, PaperSizeModify_Impl));
    m_xLandscapeBtn->connect_clicked(LINK(this, SvxPageDescPage, SwapOrientation_Impl));
    m_xPortraitBtn->connect_clicked(LINK(this, SvxPageDescPage, SwapOrientation_Impl));

    Link<weld::MetricSpinButton&, void> aLink = LINK(this, SvxPageDescPage, BorderModify_Impl);
    m_xLeftMarginEdit->connect_value_changed(aLink);
    m_xRightMarginEdit->connect_value_changed(aLink);
    m_xTopMarginEdit->connect_value_changed(aLink);
    m_xBottomMarginEdit->connect_value_changed(aLink);
    m_xGutterMarginEdit->connect_value_changed(aLink);

    m_xHorzBox->connect_toggled(LINK(this, SvxPageDescPage, CenterHdl_Impl));
    m_xVertBox->connect_toggled(LINK(this, SvxPageDescPage, CenterHdl_Impl));
}

void SvxPageDescPage::Reset( const SfxItemSet* rSet )
{
    SfxItemPool* pPool = rSet->GetPool();
    SAL_WARN_IF(!pPool, "cui.tabpages", "Where is the pool?");
    MapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) );

    // adjust margins (right/left)
    const SfxPoolItem* pItem = GetItem( *rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(*pItem);
        SetMetricValue( *m_xLeftMarginEdit, rLRSpace.GetLeft(), eUnit );
        SetMetricValue(*m_xGutterMarginEdit, rLRSpace.GetGutterMargin(), eUnit);
        m_aBspWin.SetLeft(
            static_cast<sal_uInt16>(ConvertLong_Impl( rLRSpace.GetLeft(), eUnit )) );
        SetMetricValue( *m_xRightMarginEdit, rLRSpace.GetRight(), eUnit );
        m_aBspWin.SetRight(
            static_cast<sal_uInt16>(ConvertLong_Impl( rLRSpace.GetRight(), eUnit )) );
    }

    // adjust margins (top/bottom)
    pItem = GetItem( *rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(*pItem);
        SetMetricValue( *m_xTopMarginEdit, rULSpace.GetUpper(), eUnit );
        m_aBspWin.SetTop(
            static_cast<sal_uInt16>(ConvertLong_Impl( static_cast<tools::Long>(rULSpace.GetUpper()), eUnit )) );
        SetMetricValue( *m_xBottomMarginEdit, rULSpace.GetLower(), eUnit );
        m_aBspWin.SetBottom(
            static_cast<sal_uInt16>(ConvertLong_Impl( static_cast<tools::Long>(rULSpace.GetLower()), eUnit )) );
    }

    if (rSet->HasItem(SID_ATTR_CHAR_GRABBAG, &pItem))
    {
        const auto& rGrabBagItem = static_cast<const SfxGrabBagItem&>(*pItem);
        bool bGutterAtTop{};
        auto it = rGrabBagItem.GetGrabBag().find("GutterAtTop");
        if (it != rGrabBagItem.GetGrabBag().end())
        {
            it->second >>= bGutterAtTop;
        }

        if (bGutterAtTop)
        {
            m_xGutterPositionLB->set_active(1);
        }
        else
        {
            // Left.
            m_xGutterPositionLB->set_active(0);
        }
        it = rGrabBagItem.GetGrabBag().find("RtlGutter");
        bool bRtlGutter{};
        if (it != rGrabBagItem.GetGrabBag().end())
        {
            it->second >>= bRtlGutter;
            m_xRtlGutterCB->set_active(bRtlGutter);
            m_xRtlGutterCB->show();
        }
        it = rGrabBagItem.GetGrabBag().find("BackgroundFullSize");
        bool isBackgroundFullSize{};
        if (it != rGrabBagItem.GetGrabBag().end())
        {
            it->second >>= isBackgroundFullSize;
            m_xBackgroundFullSizeCB->set_active(isBackgroundFullSize);
            m_xBackgroundFullSizeCB->show();
        }
    }

    // general page data
    SvxNumType eNumType = SVX_NUM_ARABIC;
    bLandscape = ( mpDefPrinter->GetOrientation() == Orientation::Landscape );
    SvxPageUsage nUse = SvxPageUsage::All;
    pItem = GetItem( *rSet, SID_ATTR_PAGE );

    if ( pItem )
    {
        const SvxPageItem& rItem = static_cast<const SvxPageItem&>(*pItem);
        eNumType = rItem.GetNumType();
        nUse = rItem.GetPageUsage();
        bLandscape = rItem.IsLandscape();
    }

    // alignment
    m_xLayoutBox->set_active(::PageUsageToPos_Impl(nUse));
    m_aBspWin.SetUsage( nUse );
    LayoutHdl_Impl( *m_xLayoutBox );

    //adjust numeration type of the page style
    m_xNumberFormatBox->set_active_id(eNumType);

    m_xPaperTrayBox->clear();
    sal_uInt8 nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    pItem = GetItem( *rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( pItem )
    {
        nPaperBin = static_cast<const SvxPaperBinItem*>(pItem)->GetValue();

        if ( nPaperBin >= mpDefPrinter->GetPaperBinCount() )
            nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    }

    OUString aBinName;

    if ( PAPERBIN_PRINTER_SETTINGS  == nPaperBin )
        aBinName = EditResId(RID_SVXSTR_PAPERBIN_SETTINGS);
    else
        aBinName = mpDefPrinter->GetPaperBinName( static_cast<sal_uInt16>(nPaperBin) );

    m_xPaperTrayBox->append(OUString::number(nPaperBin), aBinName);
    m_xPaperTrayBox->set_active_text(aBinName);
    // reset focus handler to default first so know none already connected
    m_xPaperTrayBox->connect_focus_in(Link<weld::Widget&, void>());
    // update the list when widget gets focus
    m_xPaperTrayBox->connect_focus_in(LINK(this, SvxPageDescPage, PaperBinHdl_Impl));

    Size aPaperSize = SvxPaperInfo::GetPaperSize( mpDefPrinter );
    pItem = GetItem( *rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aPaperSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

    bool bOrientationSupport =
        mpDefPrinter->HasSupport( PrinterSupport::SetOrientation );

    if ( !bOrientationSupport &&
         aPaperSize.Width() > aPaperSize.Height() )
        bLandscape = true;

    // tdf#130548 disable callbacks on the other of a pair of the radiogroup
    // when toggling its partner
    m_xLandscapeBtn->connect_clicked(Link<weld::Button&, void>());
    m_xPortraitBtn->connect_clicked(Link<weld::Button&, void>());

    m_xLandscapeBtn->set_active(bLandscape);
    m_xPortraitBtn->set_active(!bLandscape);

    m_xLandscapeBtn->connect_clicked(LINK(this, SvxPageDescPage, SwapOrientation_Impl));
    m_xPortraitBtn->connect_clicked(LINK(this, SvxPageDescPage, SwapOrientation_Impl));

    m_aBspWin.SetSize( Size( ConvertLong_Impl( aPaperSize.Width(), eUnit ),
                           ConvertLong_Impl( aPaperSize.Height(), eUnit ) ) );

    aPaperSize = OutputDevice::LogicToLogic(aPaperSize, MapMode(eUnit), MapMode(MapUnit::Map100thMM));
    if ( bLandscape )
        Swap( aPaperSize );

    // Actual Paper Format
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aPaperSize, MapUnit::Map100thMM );

    if ( PAPER_USER != ePaper )
        aPaperSize = SvxPaperInfo::GetPaperSize( ePaper, MapUnit::Map100thMM );

    if ( bLandscape )
        Swap( aPaperSize );

    // write values into the edits
    SetMetricValue( *m_xPaperHeightEdit, aPaperSize.Height(), MapUnit::Map100thMM );
    SetMetricValue( *m_xPaperWidthEdit, aPaperSize.Width(), MapUnit::Map100thMM );
    m_xPaperSizeBox->clear();

    m_xPaperSizeBox->FillPaperSizeEntries( ( ePaperStart == PAPER_A3 ) ? PaperSizeApp::Std : PaperSizeApp::Draw );
    m_xPaperSizeBox->set_active_id( ePaper );

    // application specific

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            m_xTblAlignFT->show();
            m_xHorzBox->show();
            m_xVertBox->show();
            DisableVerticalPageDir();

            // horizontal alignment
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT1 );
            m_xHorzBox->set_active(pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue());

            // vertical alignment
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT2 );
            m_xVertBox->set_active(pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue());

            // set example window on the table
            m_aBspWin.SetTable( true );
            m_aBspWin.SetHorz(m_xHorzBox->get_active());
            m_aBspWin.SetVert(m_xVertBox->get_active());

            m_xGutterMarginLbl->hide();
            m_xGutterMarginEdit->hide();
            m_xGutterPositionFT->hide();
            m_xGutterPositionLB->hide();

            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            DisableVerticalPageDir();
            m_xAdaptBox->show();
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT1 );
            m_xAdaptBox->set_active( pItem &&
                static_cast<const SfxBoolItem*>(pItem)->GetValue() );

            //!!! hidden, because not implemented by StarDraw
            m_xLayoutBox->hide();
            m_xPageText->hide();

            m_xGutterMarginLbl->hide();
            m_xGutterMarginEdit->hide();
            m_xGutterPositionFT->hide();
            m_xGutterPositionLB->hide();

            break;
        }
        default: ;//prevent warning
    }


    // display background and border in the example
    ResetBackground_Impl( *rSet );
//! UpdateExample_Impl();
    RangeHdl_Impl();

    InitHeadFoot_Impl( *rSet );

    bBorderModified = false;
    SwapFirstValues_Impl( false );
    UpdateExample_Impl();

    m_xLeftMarginEdit->save_value();
    m_xRightMarginEdit->save_value();
    m_xTopMarginEdit->save_value();
    m_xBottomMarginEdit->save_value();
    m_xGutterMarginEdit->save_value();
    m_xLayoutBox->save_value();
    m_xNumberFormatBox->save_value();
    m_xPaperSizeBox->save_value();
    m_xPaperWidthEdit->save_value();
    m_xPaperHeightEdit->save_value();
    m_xPortraitBtn->save_state();
    m_xLandscapeBtn->save_state();
    m_xPaperTrayBox->save_value();
    m_xVertBox->save_state();
    m_xHorzBox->save_state();
    m_xAdaptBox->save_state();
    m_xGutterPositionLB->save_value();
    m_xRtlGutterCB->save_state();
    m_xBackgroundFullSizeCB->save_state();

    CheckMarginEdits( true );


    if(SfxItemState::SET == rSet->GetItemState(SID_SWREGISTER_MODE))
    {
        m_xRegisterCB->set_active(static_cast<const SfxBoolItem&>(rSet->Get(
                                  SID_SWREGISTER_MODE)).GetValue());
        m_xRegisterCB->save_state();
        RegisterModify(*m_xRegisterCB);
    }
    if(SfxItemState::SET == rSet->GetItemState(SID_SWREGISTER_COLLECTION))
    {
        m_xRegisterLB->set_active_text(
                static_cast<const SfxStringItem&>(rSet->Get(SID_SWREGISTER_COLLECTION)).GetValue());
        m_xRegisterLB->save_value();
    }

    SfxItemState eState = rSet->GetItemState( GetWhich( SID_ATTR_FRAMEDIRECTION ),
                                                true, &pItem );
    if( SfxItemState::UNKNOWN != eState )
    {
        SvxFrameDirection nVal  = SfxItemState::SET == eState
                                ? static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue()
                                : SvxFrameDirection::Horizontal_LR_TB;
        m_xTextFlowBox->set_active_id(nVal);

        m_xTextFlowBox->save_value();
        m_aBspWin.SetFrameDirection(nVal);
    }
}

void SvxPageDescPage::FillUserData()
{
    if (SVX_PAGE_MODE_PRESENTATION == eMode)
        SetUserData(m_xAdaptBox->get_active() ? OUString("1") : OUString("0")) ;

}

bool SvxPageDescPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    MapUnit eUnit = pPool->GetMetric( nWhich );
    const SfxPoolItem* pOld = nullptr;

    // copy old left and right margins
    SvxLRSpaceItem aMargin( static_cast<const SvxLRSpaceItem&>(rOldSet.Get( nWhich )) );

    // copy old top and bottom margins
    nWhich = GetWhich( SID_ATTR_ULSPACE );
    SvxULSpaceItem aTopMargin( static_cast<const SvxULSpaceItem&>(rOldSet.Get( nWhich )) );

    if (m_xLeftMarginEdit->get_value_changed_from_saved())
    {
        aMargin.SetLeft( static_cast<sal_uInt16>(GetCoreValue( *m_xLeftMarginEdit, eUnit )) );
        bModified = true;
    }

    if (m_xRightMarginEdit->get_value_changed_from_saved())
    {
        aMargin.SetRight( static_cast<sal_uInt16>(GetCoreValue( *m_xRightMarginEdit, eUnit )) );
        bModified = true;
    }

    if (m_xGutterMarginEdit->get_value_changed_from_saved())
    {
        aMargin.SetGutterMargin(static_cast<sal_uInt16>(GetCoreValue(*m_xGutterMarginEdit, eUnit)));
        bModified = true;
    }

    // set left and right margins
    if (bModified)
    {
        pOld = GetOldItem( *rSet, SID_ATTR_LRSPACE );

        if ( !pOld || *static_cast<const SvxLRSpaceItem*>(pOld) != aMargin )
            rSet->Put( aMargin );
        else
            bModified = false;
    }

    if (rOldSet.HasItem(SID_ATTR_CHAR_GRABBAG))
    {
        // Set gutter position.
        SfxGrabBagItem aGrabBagItem(
                static_cast<const SfxGrabBagItem&>(rOldSet.Get(SID_ATTR_CHAR_GRABBAG)));
        if (m_xGutterPositionLB->get_value_changed_from_saved())
        {
            bool bGutterAtTop = m_xGutterPositionLB->get_active() == 1;
            aGrabBagItem.GetGrabBag()["GutterAtTop"] <<= bGutterAtTop;
            bModified = true;
        }
        if (m_xRtlGutterCB->get_state_changed_from_saved())
        {
            bool const bRtlGutter(m_xRtlGutterCB->get_active());
            aGrabBagItem.GetGrabBag()["RtlGutter"] <<= bRtlGutter;
            bModified = true;
        }
        if (m_xBackgroundFullSizeCB->get_state_changed_from_saved())
        {
            bool const isBackgroundFullSize(m_xBackgroundFullSizeCB->get_active());
            aGrabBagItem.GetGrabBag()["BackgroundFullSize"] <<= isBackgroundFullSize;
            bModified = true;
        }

        if (bModified)
        {
            pOld = rOldSet.GetItem(SID_ATTR_CHAR_GRABBAG);

            if (!pOld || static_cast<const SfxGrabBagItem&>(*pOld) != aGrabBagItem)
                rSet->Put(aGrabBagItem);
            else
                bModified = false;
        }
    }

    bool bMod = false;

    if (m_xTopMarginEdit->get_value_changed_from_saved())
    {
        aTopMargin.SetUpper( static_cast<sal_uInt16>(GetCoreValue( *m_xTopMarginEdit, eUnit )) );
        bMod = true;
    }

    if (m_xBottomMarginEdit->get_value_changed_from_saved())
    {
        aTopMargin.SetLower( static_cast<sal_uInt16>(GetCoreValue( *m_xBottomMarginEdit, eUnit )) );
        bMod = true;
    }

    // set top and bottom margins

    if ( bMod )
    {
        pOld = GetOldItem( *rSet, SID_ATTR_ULSPACE );

        if ( !pOld || *static_cast<const SvxULSpaceItem*>(pOld) != aTopMargin )
        {
            bModified = true;
            rSet->Put( aTopMargin );
        }
    }

    // paper tray
    nWhich = GetWhich( SID_ATTR_PAGE_PAPERBIN );
    sal_Int32 nPos = m_xPaperTrayBox->get_active();
    sal_uInt16 nBin = m_xPaperTrayBox->get_id(nPos).toInt32();
    pOld = GetOldItem( *rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( !pOld || static_cast<const SvxPaperBinItem*>(pOld)->GetValue() != nBin )
    {
        rSet->Put( SvxPaperBinItem( nWhich, static_cast<sal_uInt8>(nBin) ) );
        bModified = true;
    }

    Paper ePaper = m_xPaperSizeBox->get_active_id();
    bool bChecked = m_xLandscapeBtn->get_active();

    if ( PAPER_USER == ePaper )
    {
        if ( m_xPaperSizeBox->get_value_changed_from_saved()    ||
             m_xPaperWidthEdit->get_value_changed_from_saved()  ||
             m_xPaperHeightEdit->get_value_changed_from_saved() ||
             m_xLandscapeBtn->get_state_changed_from_saved() )
        {
            Size aSize( GetCoreValue( *m_xPaperWidthEdit, eUnit ),
                        GetCoreValue( *m_xPaperHeightEdit, eUnit ) );
            pOld = GetOldItem( *rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || static_cast<const SvxSizeItem*>(pOld)->GetSize() != aSize )
            {
                rSet->Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified = true;
            }
        }
    }
    else
    {
        if (m_xPaperSizeBox->get_value_changed_from_saved() || m_xLandscapeBtn->get_state_changed_from_saved())
        {
            Size aSize( SvxPaperInfo::GetPaperSize( ePaper, eUnit ) );

            if ( bChecked )
                Swap( aSize );

            pOld = GetOldItem( *rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || static_cast<const SvxSizeItem*>(pOld)->GetSize() != aSize )
            {
                rSet->Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified = true;
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE );
    SvxPageItem aPage( static_cast<const SvxPageItem&>(rOldSet.Get( nWhich )) );
    bMod = m_xLayoutBox->get_value_changed_from_saved();

    if ( bMod )
        aPage.SetPageUsage(::PosToPageUsage_Impl(m_xLayoutBox->get_active()));

    if (m_xLandscapeBtn->get_state_changed_from_saved())
    {
        aPage.SetLandscape(bChecked);
        bMod = true;
    }

    //Get the NumType value
    if (m_xNumberFormatBox->get_value_changed_from_saved())
    {
        SvxNumType nEntryData = m_xNumberFormatBox->get_active_id();
        aPage.SetNumType( nEntryData );
        bMod = true;
    }

    if ( bMod )
    {
        pOld = GetOldItem( *rSet, SID_ATTR_PAGE );

        if ( !pOld || *static_cast<const SvxPageItem*>(pOld) != aPage )
        {
            rSet->Put( aPage );
            bModified = true;
        }
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich ) )
        rSet->ClearItem( nWhich );
    else
        rSet->Put( rOldSet.Get( nWhich ) );

    // evaluate mode specific controls

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            if (m_xHorzBox->get_state_changed_from_saved())
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT1 ),
                                   m_xHorzBox->get_active() );
                rSet->Put( aHorz );
                bModified = true;
            }

            if (m_xVertBox->get_state_changed_from_saved())
            {
                SfxBoolItem aVert( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   m_xVertBox->get_active() );
                rSet->Put( aVert );
                bModified = true;
            }
            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            // always put so that draw can evaluate this
            rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_PAGE_EXT1 ),
                      m_xAdaptBox->get_active() ) );
            bModified = true;
            break;
        }
        default: ;//prevent warning

    }

    if (m_xRegisterCB->get_visible() &&
       (m_xRegisterCB->get_active() || m_xRegisterCB->get_state_changed_from_saved()))
    {
        const SfxBoolItem& rRegItem = static_cast<const SfxBoolItem&>(rOldSet.Get(SID_SWREGISTER_MODE));
        std::unique_ptr<SfxBoolItem> pRegItem(rRegItem.Clone());
        bool bCheck = m_xRegisterCB->get_active();
        pRegItem->SetValue(bCheck);
        rSet->Put(std::move(pRegItem));
        bModified = true;
        if(bCheck)
        {
            bModified = true;
            rSet->Put(SfxStringItem(SID_SWREGISTER_COLLECTION,
                            m_xRegisterLB->get_active_text()));
        }
    }

    if (m_xTextFlowBox->get_visible() && m_xTextFlowBox->get_value_changed_from_saved())
    {
        SvxFrameDirection eDirection = m_xTextFlowBox->get_active_id();
        rSet->Put( SvxFrameDirectionItem( eDirection, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
        bModified = true;
    }

    return bModified;
}

IMPL_LINK_NOARG(SvxPageDescPage, LayoutHdl_Impl, weld::ComboBox&, void)
{
    // switch inside outside
    const SvxPageUsage nUsage = PosToPageUsage_Impl(m_xLayoutBox->get_active());

    if (nUsage == SvxPageUsage::Mirror)
    {
        m_xLeftMarginLbl->hide();
        m_xRightMarginLbl->hide();
        m_xInsideLbl->show();
        m_xOutsideLbl->show();
    }
    else
    {
        m_xLeftMarginLbl->show();
        m_xRightMarginLbl->show();
        m_xInsideLbl->hide();
        m_xOutsideLbl->hide();
    }
    UpdateExample_Impl( true );
}

IMPL_LINK_NOARG(SvxPageDescPage, GutterPositionHdl_Impl, weld::ComboBox&, void)
{
    UpdateExample_Impl(true);
}

IMPL_LINK_NOARG(SvxPageDescPage, PaperBinHdl_Impl, weld::Widget&, void)
{
    // tdf#124226 disconnect so not called again, unless Reset occurs
    m_xPaperTrayBox->connect_focus_in(Link<weld::Widget&, void>());

    OUString aOldName = m_xPaperTrayBox->get_active_text();
    m_xPaperTrayBox->freeze();
    m_xPaperTrayBox->clear();
    m_xPaperTrayBox->append(OUString::number(PAPERBIN_PRINTER_SETTINGS), EditResId(RID_SVXSTR_PAPERBIN_SETTINGS));
    OUString aPaperBin(EditResId(RID_SVXSTR_PAPERBIN));
    const sal_uInt16 nBinCount = mpDefPrinter->GetPaperBinCount();

    for (sal_uInt16 i = 0; i < nBinCount; ++i)
    {
        OUString aName = mpDefPrinter->GetPaperBinName(i);
        if (aName.isEmpty())
        {
            aName = aPaperBin + " " + OUString::number( i+1 );
        }
        m_xPaperTrayBox->append(OUString::number(i), aName);
    }
    m_xPaperTrayBox->set_active_text(aOldName);
    m_xPaperTrayBox->thaw();

    // tdf#123650 explicitly grab-focus after the modification otherwise gtk loses track
    // of there the focus should be
    m_xPaperTrayBox->grab_focus();
}

IMPL_LINK_NOARG(SvxPageDescPage, PaperSizeSelect_Impl, weld::ComboBox&, void)
{
    Paper ePaper = m_xPaperSizeBox->get_active_id();

    if ( ePaper == PAPER_USER )
        return;

    Size aSize( SvxPaperInfo::GetPaperSize( ePaper, MapUnit::Map100thMM ) );

    if (m_xLandscapeBtn->get_active())
        Swap( aSize );

    if ( aSize.Height() < m_xPaperHeightEdit->get_min( FieldUnit::MM_100TH ) )
        m_xPaperHeightEdit->set_min(
            m_xPaperHeightEdit->normalize( aSize.Height() ), FieldUnit::MM_100TH );
    if ( aSize.Width() < m_xPaperWidthEdit->get_min( FieldUnit::MM_100TH ) )
        m_xPaperWidthEdit->set_min(
            m_xPaperWidthEdit->normalize( aSize.Width() ), FieldUnit::MM_100TH );
    SetMetricValue( *m_xPaperHeightEdit, aSize.Height(), MapUnit::Map100thMM );
    SetMetricValue( *m_xPaperWidthEdit, aSize.Width(), MapUnit::Map100thMM );

    CalcMargin_Impl();

    RangeHdl_Impl();
    UpdateExample_Impl( true );

    if ( eMode != SVX_PAGE_MODE_PRESENTATION )
        return;

    // Draw: if paper format the margin shall be 1 cm
    tools::Long nTmp = 0;
    bool bScreen = (( PAPER_SCREEN_4_3 == ePaper )||( PAPER_SCREEN_16_9 == ePaper)||( PAPER_SCREEN_16_10 == ePaper));

    if ( !bScreen )
        // no margin if screen
        nTmp = 1; // accordingly 1 cm

    if ( bScreen || m_xRightMarginEdit->get_value(FieldUnit::NONE) == 0 )
        SetMetricValue( *m_xRightMarginEdit, nTmp, MapUnit::MapCM );
    if ( bScreen || m_xLeftMarginEdit->get_value(FieldUnit::NONE) == 0 )
        SetMetricValue( *m_xLeftMarginEdit, nTmp, MapUnit::MapCM );
    if ( bScreen || m_xBottomMarginEdit->get_value(FieldUnit::NONE) == 0 )
        SetMetricValue( *m_xBottomMarginEdit, nTmp, MapUnit::MapCM );
    if ( bScreen || m_xTopMarginEdit->get_value(FieldUnit::NONE) == 0 )
        SetMetricValue( *m_xTopMarginEdit, nTmp, MapUnit::MapCM );
    UpdateExample_Impl( true );
}

IMPL_LINK_NOARG(SvxPageDescPage, PaperSizeModify_Impl, weld::MetricSpinButton&, void)
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    MapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    Size aSize( GetCoreValue( *m_xPaperWidthEdit, eUnit ),
                GetCoreValue( *m_xPaperHeightEdit, eUnit ) );

    if ( aSize.Width() > aSize.Height() )
    {
        m_xLandscapeBtn->set_active(true);
        bLandscape = true;
    }
    else
    {
        m_xPortraitBtn->set_active(true);
        bLandscape = false;
    }

    Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, eUnit );
    m_xPaperSizeBox->set_active_id( ePaper );
    UpdateExample_Impl( true );

    RangeHdl_Impl();
}

IMPL_LINK(SvxPageDescPage, SwapOrientation_Impl, weld::Button&, rBtn, void)
{
    if (
        !((!bLandscape && &rBtn == m_xLandscapeBtn.get()) ||
        (bLandscape  && &rBtn == m_xPortraitBtn.get()))
       )
        return;

    bLandscape = m_xLandscapeBtn->get_active();

    const tools::Long lWidth = GetCoreValue( *m_xPaperWidthEdit, MapUnit::Map100thMM );
    const tools::Long lHeight = GetCoreValue( *m_xPaperHeightEdit, MapUnit::Map100thMM );

    // swap width and height
    SetMetricValue(*m_xPaperWidthEdit, lHeight, MapUnit::Map100thMM);
    SetMetricValue(*m_xPaperHeightEdit, lWidth, MapUnit::Map100thMM);

    // recalculate margins if necessary
    CalcMargin_Impl();

    PaperSizeSelect_Impl(m_xPaperSizeBox->get_widget());
    RangeHdl_Impl();
    SwapFirstValues_Impl(bBorderModified);
    UpdateExample_Impl(true);
}

void SvxPageDescPage::SwapFirstValues_Impl( bool bSet )
{
    MapMode aOldMode = mpDefPrinter->GetMapMode();
    Orientation eOri = Orientation::Portrait;

    if ( bLandscape )
        eOri = Orientation::Landscape;
    Orientation eOldOri = mpDefPrinter->GetOrientation();
    mpDefPrinter->SetOrientation( eOri );
    mpDefPrinter->SetMapMode(MapMode(MapUnit::MapTwip));

    // set first- and last-values for margins
    Size aPaperSize = mpDefPrinter->GetPaperSize();
    Size aPrintSize = mpDefPrinter->GetOutputSize();
    /*
     * To convert a point ( 0,0 ) into logic coordinates
     * looks like nonsense; but it makes sense if the
     * coordinate system's origin has been moved.
     */
    Point aPrintOffset = mpDefPrinter->GetPageOffset() - mpDefPrinter->PixelToLogic( Point() );
    mpDefPrinter->SetMapMode( aOldMode );
    mpDefPrinter->SetOrientation( eOldOri );

    sal_Int64 nSetL = m_xLeftMarginEdit->denormalize(
                    m_xLeftMarginEdit->get_value( FieldUnit::TWIP ) );
    sal_Int64 nSetR = m_xRightMarginEdit->denormalize(
                    m_xRightMarginEdit->get_value( FieldUnit::TWIP ) );
    sal_Int64 nSetT = m_xTopMarginEdit->denormalize(
                    m_xTopMarginEdit->get_value( FieldUnit::TWIP ) );
    sal_Int64 nSetB = m_xBottomMarginEdit->denormalize(
                    m_xBottomMarginEdit->get_value( FieldUnit::TWIP ) );

    tools::Long nNewL = aPrintOffset.X();
    tools::Long nNewR = aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X();
    tools::Long nNewT = aPrintOffset.Y();
    tools::Long nNewB = aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y();

    nFirstLeftMargin = m_xLeftMarginEdit->convert_value_from(m_xLeftMarginEdit->normalize(nNewL), FieldUnit::TWIP);
    nFirstRightMargin = m_xRightMarginEdit->convert_value_from(m_xRightMarginEdit->normalize(nNewR), FieldUnit::TWIP);
    nFirstTopMargin = m_xTopMarginEdit->convert_value_from(m_xTopMarginEdit->normalize(nNewT), FieldUnit::TWIP);
    nFirstBottomMargin = m_xBottomMarginEdit->convert_value_from(m_xBottomMarginEdit->normalize(nNewB), FieldUnit::TWIP);

    if ( !bSet )
        return;

    if ( nSetL < nNewL )
        m_xLeftMarginEdit->set_value( m_xLeftMarginEdit->normalize( nNewL ),
                                  FieldUnit::TWIP );
    if ( nSetR < nNewR )
        m_xRightMarginEdit->set_value( m_xRightMarginEdit->normalize( nNewR ),
                                   FieldUnit::TWIP );
    if ( nSetT < nNewT )
        m_xTopMarginEdit->set_value( m_xTopMarginEdit->normalize( nNewT ),
                                 FieldUnit::TWIP );
    if ( nSetB < nNewB )
        m_xBottomMarginEdit->set_value( m_xBottomMarginEdit->normalize( nNewB ),
                                    FieldUnit::TWIP );
}

IMPL_LINK_NOARG(SvxPageDescPage, BorderModify_Impl, weld::MetricSpinButton&, void)
{
    if ( !bBorderModified )
        bBorderModified = true;
    UpdateExample_Impl();

    RangeHdl_Impl();
}

void SvxPageDescPage::UpdateExample_Impl( bool bResetbackground )
{
    // Size
    Size aSize( GetCoreValue( *m_xPaperWidthEdit, MapUnit::MapTwip ),
                GetCoreValue( *m_xPaperHeightEdit, MapUnit::MapTwip ) );

    m_aBspWin.SetSize( aSize );

    // Margins
    bool bGutterAtTop = m_xGutterPositionLB->get_active() == 1;
    tools::Long nTop = GetCoreValue(*m_xTopMarginEdit, MapUnit::MapTwip);
    if (bGutterAtTop)
    {
        nTop += GetCoreValue(*m_xGutterMarginEdit, MapUnit::MapTwip);
    }
    m_aBspWin.SetTop(nTop);
    m_aBspWin.SetBottom( GetCoreValue( *m_xBottomMarginEdit, MapUnit::MapTwip ) );
    tools::Long nLeft = GetCoreValue(*m_xLeftMarginEdit, MapUnit::MapTwip);
    if (!bGutterAtTop)
    {
        nLeft += GetCoreValue(*m_xGutterMarginEdit, MapUnit::MapTwip);
    }
    m_aBspWin.SetLeft(nLeft);
    m_aBspWin.SetRight( GetCoreValue( *m_xRightMarginEdit, MapUnit::MapTwip ) );

    // Layout
    m_aBspWin.SetUsage(PosToPageUsage_Impl(m_xLayoutBox->get_active()));
    if ( bResetbackground )
        m_aBspWin.ResetBackground();
    m_aBspWin.Invalidate();
}


void SvxPageDescPage::ResetBackground_Impl(const SfxItemSet& rSet)
{
    sal_uInt16 nWhich(GetWhich(SID_ATTR_PAGE_HEADERSET));

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich, false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = static_cast< const SfxBoolItem& >(rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON)));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aHeaderFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                // create FillAttributes directly from DrawingLayer FillStyle entries
                aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rTmpSet);
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    // create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
                }
            }

            m_aBspWin.setHeaderFillAttributes(aHeaderFillAttributes);
        }
    }

    nWhich = GetWhich(SID_ATTR_PAGE_FOOTERSET);

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich,false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = static_cast< const SfxBoolItem& >(rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON)));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFooterFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                // create FillAttributes directly from DrawingLayer FillStyle entries
                aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rTmpSet);
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    // create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
                }
            }

            m_aBspWin.setFooterFillAttributes(aFooterFillAttributes);
        }
    }

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aPageFillAttributes;

    if(mbEnableDrawingLayerFillStyles)
    {
        // create FillAttributes directly from DrawingLayer FillStyle entries
        aPageFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rSet);
    }
    else
    {
        const SfxPoolItem* pItem = GetItem(rSet, SID_ATTR_BRUSH);

        if(pItem)
        {
            // create FillAttributes from SvxBrushItem
            const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(*pItem);
            SfxItemSet aTempSet(*rSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

            setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
            aPageFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
        }
    }

    m_aBspWin.setPageFillAttributes(aPageFillAttributes);
}

void SvxPageDescPage::InitHeadFoot_Impl( const SfxItemSet& rSet )
{
    bLandscape = m_xLandscapeBtn->get_active();
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        m_aBspWin.SetSize( static_cast<const SvxSizeItem*>(pItem)->GetSize() );

    const SvxSetItem* pSetItem = nullptr;

    // evaluate header attributes

    if ( SfxItemState::SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                            false, reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            static_cast<const SfxBoolItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) ));

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) ));
            tools::Long nDist = rUL.GetLower();
            m_aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            m_aBspWin.SetHdDist( nDist );
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            m_aBspWin.SetHdLeft( rLR.GetLeft() );
            m_aBspWin.SetHdRight( rLR.GetRight() );
            m_aBspWin.SetHeader( true );
        }
        else
            m_aBspWin.SetHeader( false );

        // show background and border in the example
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aHeaderFillAttributes;

        if(mbEnableDrawingLayerFillStyles)
        {
            // create FillAttributes directly from DrawingLayer FillStyle entries
            aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rHeaderSet);
        }
        else
        {
            const sal_uInt16 nWhich(GetWhich(SID_ATTR_BRUSH));

            if(rHeaderSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
            {
                // aBspWin.SetHdColor(rItem.GetColor());
                const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rHeaderSet.Get(nWhich));
                SfxItemSet aTempSet(*rHeaderSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

                setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
            }
        }

        m_aBspWin.setHeaderFillAttributes(aHeaderFillAttributes);
    }

    // evaluate footer attributes

    if ( SfxItemState::SET !=
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                            false, reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
        return;

    const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
    const SfxBoolItem& rFooterOn =
        static_cast<const SfxBoolItem&>(rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) ));

    if ( rFooterOn.GetValue() )
    {
        const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(
            rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
        const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(
            rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) ));
        tools::Long nDist = rUL.GetUpper();
        m_aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
        m_aBspWin.SetFtDist( nDist );
        const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
            rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
        m_aBspWin.SetFtLeft( rLR.GetLeft() );
        m_aBspWin.SetFtRight( rLR.GetRight() );
        m_aBspWin.SetFooter( true );
    }
    else
        m_aBspWin.SetFooter( false );

    // show background and border in the example
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFooterFillAttributes;

    if(mbEnableDrawingLayerFillStyles)
    {
        // create FillAttributes directly from DrawingLayer FillStyle entries
        aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rFooterSet);
    }
    else
    {
        const sal_uInt16 nWhich(GetWhich(SID_ATTR_BRUSH));

        if(rFooterSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
        {
            // aBspWin.SetFtColor(rItem.GetColor());
            const SvxBrushItem& rItem = static_cast<const SvxBrushItem&>(rFooterSet.Get(nWhich));
            SfxItemSet aTempSet(*rFooterSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

            setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
            aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
        }
    }

    m_aBspWin.setFooterFillAttributes(aFooterFillAttributes);
}

void SvxPageDescPage::ActivatePage( const SfxItemSet& rSet )
{
    InitHeadFoot_Impl( rSet );
    UpdateExample_Impl();
    ResetBackground_Impl( rSet );
    RangeHdl_Impl();
}

DeactivateRC SvxPageDescPage::DeactivatePage( SfxItemSet* _pSet )
{
    // Inquiry whether the page margins are beyond the printing area.
    // If not, ask user whether they shall be taken.
    // If not, stay on the TabPage.
    Paper ePaper = m_xPaperSizeBox->get_active_id();

    if ( ePaper != PAPER_SCREEN_4_3 && ePaper != PAPER_SCREEN_16_9 && ePaper != PAPER_SCREEN_16_10 && IsMarginOutOfRange() )
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       m_xPrintRangeQueryText->get_label()));
        xQueryBox->set_default_response(RET_NO);
        if (xQueryBox->run() == RET_NO)
        {
            weld::MetricSpinButton* pField = nullptr;
            if ( IsPrinterRangeOverflow( *m_xLeftMarginEdit, nFirstLeftMargin, nLastLeftMargin, MARGIN_LEFT ) )
                pField = m_xLeftMarginEdit.get();
            if (    IsPrinterRangeOverflow( *m_xRightMarginEdit, nFirstRightMargin, nLastRightMargin, MARGIN_RIGHT )
                 && !pField )
                pField = m_xRightMarginEdit.get();
            if (    IsPrinterRangeOverflow( *m_xTopMarginEdit, nFirstTopMargin, nLastTopMargin, MARGIN_TOP )
                 && !pField )
                pField = m_xTopMarginEdit.get();
            if (    IsPrinterRangeOverflow( *m_xBottomMarginEdit, nFirstBottomMargin, nLastBottomMargin, MARGIN_BOTTOM )
                 && !pField )
                pField = m_xBottomMarginEdit.get();
            if ( pField )
                pField->grab_focus();
            UpdateExample_Impl();
            return DeactivateRC::KeepPage;
        }
        else
            CheckMarginEdits( false );
    }

    if ( _pSet )
    {
        FillItemSet( _pSet );

        // put portray/landscape if applicable
        sal_uInt16 nWh = GetWhich( SID_ATTR_PAGE_SIZE );
        MapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWh );
        Size aSize( GetCoreValue( *m_xPaperWidthEdit, eUnit ),
                    GetCoreValue( *m_xPaperHeightEdit, eUnit ) );

        // put, if current size is different to the value in _pSet
        const SvxSizeItem* pSize = GetItem( *_pSet, SID_ATTR_PAGE_SIZE );
        if ( aSize.Width() && ( !pSize || !IsEqualSize_Impl( pSize, aSize ) ) )
            _pSet->Put( SvxSizeItem( nWh, aSize ) );
    }

    return DeactivateRC::LeavePage;
}

void SvxPageDescPage::RangeHdl_Impl()
{
    // example window
    tools::Long nHHeight = m_aBspWin.GetHdHeight();
    tools::Long nHDist = m_aBspWin.GetHdDist();

    tools::Long nFHeight = m_aBspWin.GetFtHeight();
    tools::Long nFDist = m_aBspWin.GetFtDist();

    tools::Long nHFLeft = std::max(m_aBspWin.GetHdLeft(), m_aBspWin.GetFtLeft());
    tools::Long nHFRight = std::max(m_aBspWin.GetHdRight(), m_aBspWin.GetFtRight());

    // current values for page margins
    tools::Long nBT = static_cast<tools::Long>(m_xTopMarginEdit->denormalize(m_xTopMarginEdit->get_value(FieldUnit::TWIP)));
    tools::Long nBB = static_cast<tools::Long>(m_xBottomMarginEdit->denormalize(m_xBottomMarginEdit->get_value(FieldUnit::TWIP)));
    tools::Long nBL = static_cast<tools::Long>(m_xLeftMarginEdit->denormalize(m_xLeftMarginEdit->get_value(FieldUnit::TWIP)));
    tools::Long nBR = static_cast<tools::Long>(m_xRightMarginEdit->denormalize(m_xRightMarginEdit->get_value(FieldUnit::TWIP)));

    // calculate width of page border
    const SfxItemSet* _pSet = &GetItemSet();
    Size aBorder;

    if ( _pSet->GetItemState( GetWhich(SID_ATTR_BORDER_SHADOW) ) >=
            SfxItemState::DEFAULT &&
         _pSet->GetItemState( GetWhich(SID_ATTR_BORDER_OUTER)  ) >=
            SfxItemState::DEFAULT )
    {
        aBorder = GetMinBorderSpace_Impl(
            static_cast<const SvxShadowItem&>(_pSet->Get(GetWhich(SID_ATTR_BORDER_SHADOW))),
            static_cast<const SvxBoxItem&>(_pSet->Get(GetWhich(SID_ATTR_BORDER_OUTER))));
    }

    // limits paper
    // maximum is 54 cm

    tools::Long nMin = nHHeight + nHDist + nFDist + nFHeight + nBT + nBB +
                MINBODY + aBorder.Height();
    m_xPaperHeightEdit->set_min(m_xPaperHeightEdit->normalize(nMin), FieldUnit::TWIP);

    nMin = MINBODY + nBL + nBR + aBorder.Width();
    m_xPaperWidthEdit->set_min(m_xPaperWidthEdit->normalize(nMin), FieldUnit::TWIP);

    tools::Long nH = static_cast<tools::Long>(m_xPaperHeightEdit->denormalize(m_xPaperHeightEdit->get_value(FieldUnit::TWIP)));
    tools::Long nW = static_cast<tools::Long>(m_xPaperWidthEdit->denormalize(m_xPaperWidthEdit->get_value(FieldUnit::TWIP)));

    // Top
    tools::Long nMax = nH - nBB - aBorder.Height() - MINBODY -
                nFDist - nFHeight - nHDist - nHHeight;

    m_xTopMarginEdit->set_max(m_xTopMarginEdit->normalize(nMax), FieldUnit::TWIP);

    // Bottom
    nMax = nH - nBT - aBorder.Height() - MINBODY -
           nFDist - nFHeight - nHDist - nHHeight;

    m_xBottomMarginEdit->set_max(m_xTopMarginEdit->normalize(nMax), FieldUnit::TWIP);

    // Left
    nMax = nW - nBR - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    m_xLeftMarginEdit->set_max(m_xLeftMarginEdit->normalize(nMax), FieldUnit::TWIP);

    // Right
    nMax = nW - nBL - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    m_xRightMarginEdit->set_max(m_xRightMarginEdit->normalize(nMax), FieldUnit::TWIP);
}

void SvxPageDescPage::CalcMargin_Impl()
{
    // current values for page margins
    tools::Long nBT = GetCoreValue( *m_xTopMarginEdit, MapUnit::MapTwip );
    tools::Long nBB = GetCoreValue( *m_xBottomMarginEdit, MapUnit::MapTwip );

    tools::Long nBL = GetCoreValue( *m_xLeftMarginEdit, MapUnit::MapTwip );
    tools::Long nBR = GetCoreValue( *m_xRightMarginEdit, MapUnit::MapTwip );

    tools::Long nH  = GetCoreValue( *m_xPaperHeightEdit, MapUnit::MapTwip );
    tools::Long nW  = GetCoreValue( *m_xPaperWidthEdit, MapUnit::MapTwip );

    tools::Long nWidth = nBL + nBR + MINBODY;
    tools::Long nHeight = nBT + nBB + MINBODY;

    if ( nWidth <= nW && nHeight <= nH )
        return;

    if ( nWidth > nW )
    {
        tools::Long nTmp = nBL <= nBR ? nBR : nBL;
        nTmp -= nWidth - nW;

        if ( nBL <= nBR )
            SetMetricValue( *m_xRightMarginEdit, nTmp, MapUnit::MapTwip );
        else
            SetMetricValue( *m_xLeftMarginEdit, nTmp, MapUnit::MapTwip );
    }

    if ( nHeight > nH )
    {
        tools::Long nTmp = nBT <= nBB ? nBB : nBT;
        nTmp -= nHeight - nH;

        if ( nBT <= nBB )
            SetMetricValue( *m_xBottomMarginEdit, nTmp, MapUnit::MapTwip );
        else
            SetMetricValue( *m_xTopMarginEdit, nTmp, MapUnit::MapTwip );
    }
}

IMPL_LINK_NOARG(SvxPageDescPage, CenterHdl_Impl, weld::ToggleButton&, void)
{
    m_aBspWin.SetHorz(m_xHorzBox->get_active());
    m_aBspWin.SetVert(m_xVertBox->get_active());
    UpdateExample_Impl();
}

void SvxPageDescPage::SetCollectionList(const std::vector<OUString> &aList)
{
    OSL_ENSURE(!aList.empty(), "Empty string list");

    sStandardRegister = aList[0];
    m_xRegisterLB->freeze();
    for (size_t i = 1; i < aList.size(); ++i)
        m_xRegisterLB->append_text(aList[i]);
    m_xRegisterLB->thaw();

    m_xRegisterCB->show();
    m_xRegisterFT->show();
    m_xRegisterLB->show();
    m_xRegisterCB->connect_toggled(LINK(this, SvxPageDescPage, RegisterModify));
}

IMPL_LINK(SvxPageDescPage, RegisterModify, weld::ToggleButton&, rBox, void)
{
    bool bEnable = false;
    if (rBox.get_active())
    {
        bEnable = true;
        if (m_xRegisterLB->get_active() == -1)
            m_xRegisterLB->set_active_text(sStandardRegister);
    }
    m_xRegisterFT->set_sensitive(bEnable);
    m_xRegisterLB->set_sensitive(bEnable);
}

void SvxPageDescPage::DisableVerticalPageDir()
{
    m_xTextFlowBox->remove_id(SvxFrameDirection::Vertical_RL_TB);
    m_xTextFlowBox->remove_id(SvxFrameDirection::Vertical_LR_TB);
    if (m_xTextFlowBox->get_count() < 2)
    {
        m_xTextFlowLbl->hide();
        m_xTextFlowBox->hide();
        m_aBspWin.EnableFrameDirection( false );
    }
}

IMPL_LINK_NOARG(SvxPageDescPage, FrameDirectionModify_Impl, weld::ComboBox&, void)
{
    m_aBspWin.SetFrameDirection(m_xTextFlowBox->get_active_id());
    m_aBspWin.Invalidate();
}

bool SvxPageDescPage::IsPrinterRangeOverflow(
    weld::MetricSpinButton& rField, tools::Long nFirstMargin, tools::Long nLastMargin, MarginPosition nPos )
{
    bool bRet = false;
    bool bCheck = ( ( m_nPos & nPos ) == 0 );
    tools::Long nValue = rField.get_value(FieldUnit::NONE);
    if ( bCheck &&
         (  nValue < nFirstMargin || nValue > nLastMargin ) &&
         rField.get_value_changed_from_saved() )
    {
        rField.set_value(nValue < nFirstMargin ? nFirstMargin : nLastMargin, FieldUnit::NONE);
        bRet = true;
    }

    return bRet;
}

/** Check if a value of a margin edit is outside the printer paper margins
    and save this information.
*/
void SvxPageDescPage::CheckMarginEdits( bool _bClear )
{
    if ( _bClear )
        m_nPos = 0;

    sal_Int64 nValue = m_xLeftMarginEdit->get_value(FieldUnit::NONE);
    if (  nValue < nFirstLeftMargin || nValue > nLastLeftMargin )
        m_nPos |= MARGIN_LEFT;
    nValue = m_xRightMarginEdit->get_value(FieldUnit::NONE);
    if (  nValue < nFirstRightMargin || nValue > nLastRightMargin )
        m_nPos |= MARGIN_RIGHT;
    nValue = m_xTopMarginEdit->get_value(FieldUnit::NONE);
    if (  nValue < nFirstTopMargin || nValue > nLastTopMargin )
        m_nPos |= MARGIN_TOP;
    nValue = m_xBottomMarginEdit->get_value(FieldUnit::NONE);
    if (  nValue < nFirstBottomMargin || nValue > nLastBottomMargin )
        m_nPos |= MARGIN_BOTTOM;
}

bool SvxPageDescPage::IsMarginOutOfRange() const
{
    bool bRet = ( ( ( !( m_nPos & MARGIN_LEFT ) &&
                      m_xLeftMarginEdit->get_value_changed_from_saved() ) &&
                    ( m_xLeftMarginEdit->get_value(FieldUnit::NONE) < nFirstLeftMargin ||
                      m_xLeftMarginEdit->get_value(FieldUnit::NONE) > nLastLeftMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_RIGHT ) &&
                      m_xRightMarginEdit->get_value_changed_from_saved() ) &&
                    ( m_xRightMarginEdit->get_value(FieldUnit::NONE) < nFirstRightMargin ||
                      m_xRightMarginEdit->get_value(FieldUnit::NONE) > nLastRightMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_TOP ) &&
                      m_xTopMarginEdit->get_value_changed_from_saved() ) &&
                    ( m_xTopMarginEdit->get_value(FieldUnit::NONE) < nFirstTopMargin ||
                      m_xTopMarginEdit->get_value(FieldUnit::NONE) > nLastTopMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_BOTTOM ) &&
                      m_xBottomMarginEdit->get_value_changed_from_saved() ) &&
                    ( m_xBottomMarginEdit->get_value(FieldUnit::NONE) < nFirstBottomMargin ||
                      m_xBottomMarginEdit->get_value(FieldUnit::NONE) > nLastBottomMargin ) ) );
    return bRet;
}

void SvxPageDescPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pModeItem = aSet.GetItem(SID_ENUM_PAGE_MODE, false);
    const SfxUInt16Item* pPaperStartItem = aSet.GetItem(SID_PAPER_START, false);
    const SfxUInt16Item* pPaperEndItem = aSet.GetItem(SID_PAPER_END, false);
    const SfxStringListItem* pCollectListItem = aSet.GetItem<SfxStringListItem>(SID_COLLECT_LIST, false);
    const SfxBoolItem* pSupportDrawingLayerFillStyleItem = aSet.GetItem<SfxBoolItem>(SID_DRAWINGLAYER_FILLSTYLES, false);
    const SfxBoolItem* pIsImpressDoc = aSet.GetItem<SfxBoolItem>(SID_IMPRESS_DOC, false);

    if (pModeItem)
    {
        eMode = static_cast<SvxModeType>(pModeItem->GetValue());
    }

    if(pPaperStartItem && pPaperEndItem)
    {
        SetPaperFormatRanges(static_cast<Paper>(pPaperStartItem->GetValue()));
    }

    if(pCollectListItem)
    {
        SetCollectionList(pCollectListItem->GetList());
    }

    if(pSupportDrawingLayerFillStyleItem)
    {
        const bool bNew(pSupportDrawingLayerFillStyleItem->GetValue());

        mbEnableDrawingLayerFillStyles = bNew;
    }

    if (pIsImpressDoc)
        m_xNumberFormatText->set_label(SvxResId(STR_SLIDE_NUMBERS));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
