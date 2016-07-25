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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <tools/resary.hxx>
#include <vcl/graph.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/itemiter.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/configitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <sal/macros.h>

#include <cuires.hrc>
#include "helpid.hrc"
#include "page.hxx"
#include <svx/pageitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/paperinf.hxx>
#include <dialmgr.hxx>
#include <sfx2/module.hxx>
#include <svl/stritem.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <svx/svxids.hrc>
#include <svtools/optionsdrawinglayer.hxx>
#include <svl/slstitm.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/request.hxx>

//UUUU
#include <svx/xdef.hxx>
#include <svx/unobrushitemhelper.hxx>

#include <svx/SvxNumOptionsTabPageHelper.hxx>

// static ----------------------------------------------------------------

static const long MINBODY       = 284;  // 0,5 cm rounded up in twips
//static const long PRINT_OFFSET    = 17;   // 0,03 cm rounded down in twips
static const long PRINT_OFFSET  = 0;    // why was this ever set to 17 ? it led to wrong right and bottom margins.

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

const sal_uInt16 aArr[] =
{
    SVX_PAGE_ALL,
    SVX_PAGE_MIRROR,
    SVX_PAGE_RIGHT,
    SVX_PAGE_LEFT
};


sal_uInt16 PageUsageToPos_Impl( sal_uInt16 nUsage )
{
    for ( sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] == ( nUsage & 0x000f ) )
            return i;
    return SVX_PAGE_ALL;
}


sal_uInt16 PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return 0;
    return aArr[nPos];
}


Size GetMinBorderSpace_Impl( const SvxShadowItem& rShadow, const SvxBoxItem& rBox )
{
    Size aSz;
    aSz.Height() = rShadow.CalcShadowSpace( SvxShadowItemSide::BOTTOM ) + rBox.CalcLineSpace( SvxBoxItemLine::BOTTOM );
    aSz.Height() += rShadow.CalcShadowSpace( SvxShadowItemSide::TOP ) + rBox.CalcLineSpace( SvxBoxItemLine::TOP );
    aSz.Width() = rShadow.CalcShadowSpace( SvxShadowItemSide::LEFT ) + rBox.CalcLineSpace( SvxBoxItemLine::LEFT );
    aSz.Width() += rShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT ) + rBox.CalcLineSpace( SvxBoxItemLine::RIGHT );
    return aSz;
}


long ConvertLong_Impl( const long nIn, SfxMapUnit eUnit )
{
    return OutputDevice::LogicToLogic( nIn, (MapUnit)eUnit, MAP_TWIP );
}

bool IsEqualSize_Impl( const SvxSizeItem* pSize, const Size& rSize )
{
    if ( pSize )
    {
        Size aSize = pSize->GetSize();
        long nDiffW = std::abs( rSize.Width () - aSize.Width () );
        long nDiffH = std::abs( rSize.Height() - aSize.Height() );
        return ( nDiffW < 10 && nDiffH < 10 );
    }
    else
        return false;
}


#define MARGIN_LEFT     ( (MarginPosition)0x0001 )
#define MARGIN_RIGHT    ( (MarginPosition)0x0002 )
#define MARGIN_TOP      ( (MarginPosition)0x0004 )
#define MARGIN_BOTTOM   ( (MarginPosition)0x0008 )

//UUUU
//struct SvxPage_Impl
//{
//  MarginPosition  m_nPos;
//    Printer*        mpDefPrinter;
//    bool            mbDelPrinter;
//
//    SvxPage_Impl() :
//        m_nPos( 0 ),
//        mpDefPrinter( 0 ),
//        mbDelPrinter( false ) {}
//
//    ~SvxPage_Impl() { if ( mbDelPrinter ) delete mpDefPrinter; }
//};

// class SvxPageDescPage --------------------------------------------------

VclPtr<SfxTabPage> SvxPageDescPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxPageDescPage>::Create( pParent, *rSet );
}

SvxPageDescPage::SvxPageDescPage( vcl::Window* pParent, const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, "PageFormatPage", "cui/ui/pageformatpage.ui", &rAttr ),

    bLandscape          ( false ),
    eMode               ( SVX_PAGE_MODE_STANDARD ),
    ePaperStart         ( PAPER_A3 ),
    ePaperEnd           ( PAPER_ENV_DL ),

    //UUUU
    // pImpl                ( new SvxPage_Impl ),

    //UUUU
    m_nPos( 0 ),
    mpDefPrinter( nullptr ),
    mbDelPrinter( false ),

    //UUUU
    mbEnableDrawingLayerFillStyles(false)
{
    get(m_pPaperSizeBox,"comboPageFormat");
    get(m_pPaperWidthEdit,"spinWidth");
    get(m_pPaperHeightEdit,"spinHeight");

    get(m_pOrientationFT,"labelOrientation");
    get(m_pPortraitBtn,"radiobuttonPortrait");
    get(m_pLandscapeBtn,"radiobuttonLandscape");

    get(m_pBspWin,"drawingareaPageDirection");
    get(m_pTextFlowLbl,"labelTextFlow");
    get(m_pTextFlowBox,"comboTextFlowBox");
    get(m_pPaperTrayBox,"comboPaperTray");

    get(m_pLeftMarginLbl,"labelLeftMargin");
    get(m_pLeftMarginEdit,"spinMargLeft");
    get(m_pRightMarginLbl,"labelRightMargin");
    get(m_pRightMarginEdit,"spinMargRight");

    get(m_pTopMarginEdit,"spinMargTop");

    get(m_pBottomMarginEdit,"spinMargBot");

    get(m_pPageText,"labelPageLayout");
    get(m_pLayoutBox,"comboPageLayout");
    get(m_pNumberFormatBox,"comboLayoutFormat");

    get(m_pRegisterCB,"checkRegisterTrue");
    get(m_pRegisterFT,"labelRegisterStyle");
    get(m_pRegisterLB,"comboRegisterStyle");

    get(m_pTblAlignFT,"labelTblAlign");
    get(m_pHorzBox,"checkbuttonHorz");
    get(m_pVertBox,"checkbuttonVert");

    get(m_pAdaptBox,"checkAdaptBox");
    // Strings stored in UI
    get(m_pInsideLbl,"labelInner");
    get(m_pOutsideLbl,"labelOuter");
    get(m_pPrintRangeQueryText,"labelMsg");

    bBorderModified = false;
    m_pBspWin->EnableRTL( false );

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

    m_pTextFlowBox->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_LTR_HORI ), FRMDIR_HORI_LEFT_TOP );


    if( bCTL )
        m_pTextFlowBox->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_RTL_HORI ), FRMDIR_HORI_RIGHT_TOP );


    // #109989# do not show vertical directions in Writer/Web
    if( !bWeb )
    {
        if( bCJK )
        {
            m_pTextFlowBox->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_RTL_VERT ), FRMDIR_VERT_TOP_RIGHT );
//            m_pTextFlowBox->InsertEntryValue( CUI_RESSTR( RID_SVXSTR_PAGEDIR_LTR_VERT ), FRMDIR_VERT_TOP_LEFT );
        }
    }

    // #109989# show the text direction box in Writer/Web too
    if( (bCJK || bCTL) &&
        SfxItemState::UNKNOWN < rAttr.GetItemState(GetWhich( SID_ATTR_FRAMEDIRECTION )))
    {
        m_pTextFlowLbl->Show();
        m_pTextFlowBox->Show();
        m_pTextFlowBox->SetSelectHdl(LINK(this, SvxPageDescPage, FrameDirectionModify_Impl ));

        m_pBspWin->EnableFrameDirection(true);
    }
    Init_Impl();

    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( *m_pLeftMarginEdit, eFUnit );
    SetFieldUnit( *m_pRightMarginEdit, eFUnit );
    SetFieldUnit( *m_pTopMarginEdit, eFUnit );
    SetFieldUnit( *m_pBottomMarginEdit, eFUnit );
    SetFieldUnit( *m_pPaperWidthEdit, eFUnit );
    SetFieldUnit( *m_pPaperHeightEdit, eFUnit );

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
    mpDefPrinter->SetMapMode( MAP_TWIP );

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

    long nOffset = !aPrintOffset.X() && !aPrintOffset.Y() ? 0 : PRINT_OFFSET;
    m_pLeftMarginEdit->SetFirst( m_pLeftMarginEdit->Normalize( aPrintOffset.X() ), FUNIT_TWIP );
    nFirstLeftMargin = static_cast<long>(m_pLeftMarginEdit->GetFirst());

    m_pRightMarginEdit->SetFirst( m_pRightMarginEdit->Normalize( aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset ), FUNIT_TWIP);
    nFirstRightMargin = static_cast<long>(m_pRightMarginEdit->GetFirst());

    m_pTopMarginEdit->SetFirst( m_pTopMarginEdit->Normalize( aPrintOffset.Y() ), FUNIT_TWIP );
    nFirstTopMargin = static_cast<long>(m_pTopMarginEdit->GetFirst());

    m_pBottomMarginEdit->SetFirst( m_pBottomMarginEdit->Normalize( aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset ), FUNIT_TWIP );
    nFirstBottomMargin = static_cast<long>(m_pBottomMarginEdit->GetFirst());

    m_pLeftMarginEdit->SetLast( m_pLeftMarginEdit->Normalize( aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastLeftMargin = static_cast<long>(m_pLeftMarginEdit->GetLast());

    m_pRightMarginEdit->SetLast( m_pRightMarginEdit->Normalize( aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastRightMargin = static_cast<long>(m_pRightMarginEdit->GetLast());

    m_pTopMarginEdit->SetLast( m_pTopMarginEdit->Normalize( aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastTopMargin = static_cast<long>(m_pTopMarginEdit->GetLast());

    m_pBottomMarginEdit->SetLast( m_pBottomMarginEdit->Normalize( aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastBottomMargin = static_cast<long>(m_pBottomMarginEdit->GetLast());

    // #i4219# get DrawingLayer options
    const SvtOptionsDrawinglayer aDrawinglayerOpt;

    // #i4219# take Maximum now from configuration (1/100th cm)
    // was: 11900 -> 119 cm ;new value 3 meters -> 300 cm -> 30000
    m_pPaperWidthEdit->SetMax(m_pPaperWidthEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    m_pPaperWidthEdit->SetLast(m_pPaperWidthEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    m_pPaperHeightEdit->SetMax(m_pPaperHeightEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);
    m_pPaperHeightEdit->SetLast(m_pPaperHeightEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);

    // #i4219# also for margins (1/100th cm). Was: 9999, keeping.
    m_pLeftMarginEdit->SetMax(m_pLeftMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperLeftMargin()), FUNIT_MM);
    m_pLeftMarginEdit->SetLast(m_pLeftMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperLeftMargin()), FUNIT_MM);
    m_pRightMarginEdit->SetMax(m_pRightMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperRightMargin()), FUNIT_MM);
    m_pRightMarginEdit->SetLast(m_pRightMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperRightMargin()), FUNIT_MM);
    m_pTopMarginEdit->SetMax(m_pTopMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperTopMargin()), FUNIT_MM);
    m_pTopMarginEdit->SetLast(m_pTopMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperTopMargin()), FUNIT_MM);
    m_pBottomMarginEdit->SetMax(m_pBottomMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperBottomMargin()), FUNIT_MM);
    m_pBottomMarginEdit->SetLast(m_pBottomMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperBottomMargin()), FUNIT_MM);

    // Get the i18n framework numberings and add them to the listbox.
    SvxNumOptionsTabPageHelper::GetI18nNumbering( *m_pNumberFormatBox, ::std::numeric_limits<sal_uInt16>::max());
}


SvxPageDescPage::~SvxPageDescPage()
{
    disposeOnce();
}

void SvxPageDescPage::dispose()
{
    if(mbDelPrinter)
    {
        mpDefPrinter.disposeAndClear();
        mbDelPrinter = false;
    }
    m_pPaperSizeBox.clear();
    m_pPaperWidthEdit.clear();
    m_pPaperHeightEdit.clear();
    m_pOrientationFT.clear();
    m_pPortraitBtn.clear();
    m_pLandscapeBtn.clear();
    m_pBspWin.clear();
    m_pTextFlowLbl.clear();
    m_pTextFlowBox.clear();
    m_pPaperTrayBox.clear();
    m_pLeftMarginLbl.clear();
    m_pLeftMarginEdit.clear();
    m_pRightMarginLbl.clear();
    m_pRightMarginEdit.clear();
    m_pTopMarginEdit.clear();
    m_pBottomMarginEdit.clear();
    m_pPageText.clear();
    m_pLayoutBox.clear();
    m_pNumberFormatBox.clear();
    m_pTblAlignFT.clear();
    m_pHorzBox.clear();
    m_pVertBox.clear();
    m_pAdaptBox.clear();
    m_pRegisterCB.clear();
    m_pRegisterFT.clear();
    m_pRegisterLB.clear();
    m_pInsideLbl.clear();
    m_pOutsideLbl.clear();
    m_pPrintRangeQueryText.clear();
    SfxTabPage::dispose();
}


void SvxPageDescPage::Init_Impl()
{
        // adjust the handler
    m_pLayoutBox->SetSelectHdl( LINK( this, SvxPageDescPage, LayoutHdl_Impl ) );
    m_pPaperSizeBox->SetDropDownLineCount(10);

    m_pPaperTrayBox->SetGetFocusHdl(  LINK( this, SvxPageDescPage, PaperBinHdl_Impl ) );
    m_pPaperSizeBox->SetSelectHdl(    LINK( this, SvxPageDescPage, PaperSizeSelect_Impl ) );
    m_pPaperWidthEdit->SetModifyHdl(  LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    m_pPaperHeightEdit->SetModifyHdl( LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    m_pLandscapeBtn->SetClickHdl(     LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );
    m_pPortraitBtn->SetClickHdl(      LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxPageDescPage, BorderModify_Impl );
    m_pLeftMarginEdit->SetModifyHdl( aLink );
    m_pRightMarginEdit->SetModifyHdl( aLink );
    m_pTopMarginEdit->SetModifyHdl( aLink );
    m_pBottomMarginEdit->SetModifyHdl( aLink );

    Link<Control&,void> aLink2 = LINK( this, SvxPageDescPage, RangeHdl_Impl );
    m_pPaperWidthEdit->SetLoseFocusHdl( aLink2 );
    m_pPaperHeightEdit->SetLoseFocusHdl( aLink2 );
    m_pLeftMarginEdit->SetLoseFocusHdl( aLink2 );
    m_pRightMarginEdit->SetLoseFocusHdl( aLink2 );
    m_pTopMarginEdit->SetLoseFocusHdl( aLink2 );
    m_pBottomMarginEdit->SetLoseFocusHdl( aLink2 );

    m_pHorzBox->SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );
    m_pVertBox->SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );

}


void SvxPageDescPage::Reset( const SfxItemSet* rSet )
{
    SfxItemPool* pPool = rSet->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) );

    // adjust margins (right/left)
    const SfxPoolItem* pItem = GetItem( *rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(*pItem);
        SetMetricValue( *m_pLeftMarginEdit, rLRSpace.GetLeft(), eUnit );
        m_pBspWin->SetLeft(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetLeft(), eUnit ) );
        SetMetricValue( *m_pRightMarginEdit, rLRSpace.GetRight(), eUnit );
        m_pBspWin->SetRight(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetRight(), eUnit ) );
    }

    // adjust margins (top/bottom)
    pItem = GetItem( *rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(*pItem);
        SetMetricValue( *m_pTopMarginEdit, rULSpace.GetUpper(), eUnit );
        m_pBspWin->SetTop(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetUpper(), eUnit ) );
        SetMetricValue( *m_pBottomMarginEdit, rULSpace.GetLower(), eUnit );
        m_pBspWin->SetBottom(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetLower(), eUnit ) );
    }

    // general page data
    SvxNumType eNumType = SVX_ARABIC;
    bLandscape = ( mpDefPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
    sal_uInt16 nUse = (sal_uInt16)SVX_PAGE_ALL;
    pItem = GetItem( *rSet, SID_ATTR_PAGE );

    if ( pItem )
    {
        const SvxPageItem& rItem = static_cast<const SvxPageItem&>(*pItem);
        eNumType = rItem.GetNumType();
        nUse = rItem.GetPageUsage();
        bLandscape = rItem.IsLandscape();
    }

    // alignment
    m_pLayoutBox->SelectEntryPos( ::PageUsageToPos_Impl( nUse ) );
    m_pBspWin->SetUsage( nUse );
    LayoutHdl_Impl( *m_pLayoutBox );

    //adjust numeration type of the page style
    //Get the Position of the saved NumType
    for(int i=0; i<m_pNumberFormatBox->GetEntryCount(); i++)
        if(eNumType == (sal_uInt16)reinterpret_cast<sal_uLong>(m_pNumberFormatBox->GetEntryData(i)))
        {
            m_pNumberFormatBox->SelectEntryPos( i );
            break;
        }

    m_pPaperTrayBox->Clear();
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
        aBinName = EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS );
    else
        aBinName = mpDefPrinter->GetPaperBinName( (sal_uInt16)nPaperBin );

    const sal_Int32 nEntryPos = m_pPaperTrayBox->InsertEntry( aBinName );
    m_pPaperTrayBox->SetEntryData( nEntryPos, reinterpret_cast<void*>((sal_uLong)nPaperBin) );
    m_pPaperTrayBox->SelectEntry( aBinName );

    Size aPaperSize = SvxPaperInfo::GetPaperSize( mpDefPrinter );
    pItem = GetItem( *rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aPaperSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

    bool bOrientationSupport =
        mpDefPrinter->HasSupport( PrinterSupport::SetOrientation );

    if ( !bOrientationSupport &&
         aPaperSize.Width() > aPaperSize.Height() )
        bLandscape = true;

    m_pLandscapeBtn->Check( bLandscape );
    m_pPortraitBtn->Check( !bLandscape );

    m_pBspWin->SetSize( Size( ConvertLong_Impl( aPaperSize.Width(), eUnit ),
                           ConvertLong_Impl( aPaperSize.Height(), eUnit ) ) );

    aPaperSize = OutputDevice::LogicToLogic(aPaperSize, (MapUnit)eUnit, MAP_100TH_MM);
    if ( bLandscape )
        Swap( aPaperSize );

    // Actual Paper Format
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aPaperSize, MAP_100TH_MM, true );

    if ( PAPER_USER != ePaper )
        aPaperSize = SvxPaperInfo::GetPaperSize( ePaper, MAP_100TH_MM );

    if ( bLandscape )
        Swap( aPaperSize );

    // write values into the edits
    SetMetricValue( *m_pPaperHeightEdit, aPaperSize.Height(), SFX_MAPUNIT_100TH_MM );
    SetMetricValue( *m_pPaperWidthEdit, aPaperSize.Width(), SFX_MAPUNIT_100TH_MM );
    m_pPaperSizeBox->Clear();

    m_pPaperSizeBox->FillPaperSizeEntries( ( ePaperStart == PAPER_A3 ) ? PaperSizeStd : PaperSizeDraw );
    m_pPaperSizeBox->SetSelection( ePaper );

    // application specific

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            m_pTblAlignFT->Show();
            m_pHorzBox->Show();
            m_pVertBox->Show();
            DisableVerticalPageDir();

            // horizontal alignment
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT1 );
            m_pHorzBox->Check( pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue() );

            // vertical alignment
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT2 );
            m_pVertBox->Check( pItem && static_cast<const SfxBoolItem*>(pItem)->GetValue() );

            // set example window on the table
            m_pBspWin->SetTable( true );
            m_pBspWin->SetHorz( m_pHorzBox->IsChecked() );
            m_pBspWin->SetVert( m_pVertBox->IsChecked() );

            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            DisableVerticalPageDir();
            m_pAdaptBox->Show();
            pItem = GetItem( *rSet, SID_ATTR_PAGE_EXT1 );
            m_pAdaptBox->Check( pItem &&
                static_cast<const SfxBoolItem*>(pItem)->GetValue() );

            //!!! hidden, because not implemented by StarDraw
            m_pLayoutBox->Hide();
            m_pPageText->Hide();

            break;
        }
        default: ;//prevent warning
    }


    // display background and border in the example
    ResetBackground_Impl( *rSet );
//! UpdateExample_Impl();
    RangeHdl_Impl( *m_pPaperWidthEdit );

    InitHeadFoot_Impl( *rSet );

    bBorderModified = false;
    SwapFirstValues_Impl( false );
    UpdateExample_Impl();

    m_pLeftMarginEdit->SaveValue();
    m_pRightMarginEdit->SaveValue();
    m_pTopMarginEdit->SaveValue();
    m_pBottomMarginEdit->SaveValue();
    m_pLayoutBox->SaveValue();
    m_pNumberFormatBox->SaveValue();
    m_pPaperSizeBox->SaveValue();
    m_pPaperWidthEdit->SaveValue();
    m_pPaperHeightEdit->SaveValue();
    m_pPortraitBtn->SaveValue();
    m_pLandscapeBtn->SaveValue();
    m_pPaperTrayBox->SaveValue();
    m_pVertBox->SaveValue();
    m_pHorzBox->SaveValue();
    m_pAdaptBox->SaveValue();

    CheckMarginEdits( true );


    if(SfxItemState::SET == rSet->GetItemState(SID_SWREGISTER_MODE))
    {
        m_pRegisterCB->Check(static_cast<const SfxBoolItem&>(rSet->Get(
                                SID_SWREGISTER_MODE)).GetValue());
        m_pRegisterCB->SaveValue();
        RegisterModify(m_pRegisterCB);
    }
    if(SfxItemState::SET == rSet->GetItemState(SID_SWREGISTER_COLLECTION))
    {
        m_pRegisterLB->SelectEntry(
                static_cast<const SfxStringItem&>(rSet->Get(SID_SWREGISTER_COLLECTION)).GetValue());
        m_pRegisterLB->SaveValue();
    }

    SfxItemState eState = rSet->GetItemState( GetWhich( SID_ATTR_FRAMEDIRECTION ),
                                                true, &pItem );
    if( SfxItemState::UNKNOWN != eState )
    {
        sal_uInt32 nVal  = SfxItemState::SET == eState
                                ? static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue()
                                : 0;
        m_pTextFlowBox->SelectEntryValue( static_cast< SvxFrameDirection >( nVal ) );

        m_pTextFlowBox->SaveValue();
        m_pBspWin->SetFrameDirection(nVal);
    }
}


void SvxPageDescPage::FillUserData()
{
    if ( SVX_PAGE_MODE_PRESENTATION == eMode )
        SetUserData( m_pAdaptBox->IsChecked() ? OUString("1") : OUString("0") ) ;

}


bool SvxPageDescPage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = false;
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = pPool->GetMetric( nWhich );
    const SfxPoolItem* pOld = nullptr;

    // copy old left and right margins
    SvxLRSpaceItem aMargin( static_cast<const SvxLRSpaceItem&>(rOldSet.Get( nWhich )) );

    // copy old top and bottom margins
    nWhich = GetWhich( SID_ATTR_ULSPACE );
    SvxULSpaceItem aTopMargin( static_cast<const SvxULSpaceItem&>(rOldSet.Get( nWhich )) );

    if ( m_pLeftMarginEdit->IsValueChangedFromSaved() )
    {
        aMargin.SetLeft( (sal_uInt16)GetCoreValue( *m_pLeftMarginEdit, eUnit ) );
        bModified = true;
    }

    if ( m_pRightMarginEdit->IsValueChangedFromSaved() )
    {
        aMargin.SetRight( (sal_uInt16)GetCoreValue( *m_pRightMarginEdit, eUnit ) );
        bModified = true;
    }

    // set left and right margins
    if ( bModified )
    {
        pOld = GetOldItem( *rSet, SID_ATTR_LRSPACE );

        if ( !pOld || !( *static_cast<const SvxLRSpaceItem*>(pOld) == aMargin ) )
            rSet->Put( aMargin );
        else
            bModified = false;
    }

    bool bMod = false;

    if ( m_pTopMarginEdit->IsValueChangedFromSaved() )
    {
        aTopMargin.SetUpper( (sal_uInt16)GetCoreValue( *m_pTopMarginEdit, eUnit ) );
        bMod = true;
    }

    if ( m_pBottomMarginEdit->IsValueChangedFromSaved() )
    {
        aTopMargin.SetLower( (sal_uInt16)GetCoreValue( *m_pBottomMarginEdit, eUnit ) );
        bMod = true;
    }

    // set top and bottom margins

    if ( bMod )
    {
        pOld = GetOldItem( *rSet, SID_ATTR_ULSPACE );

        if ( !pOld || !( *static_cast<const SvxULSpaceItem*>(pOld) == aTopMargin ) )
        {
            bModified = true;
            rSet->Put( aTopMargin );
        }
    }

    // paper tray
    nWhich = GetWhich( SID_ATTR_PAGE_PAPERBIN );
    sal_Int32 nPos = m_pPaperTrayBox->GetSelectEntryPos();
    sal_uInt16 nBin = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pPaperTrayBox->GetEntryData( nPos ));
    pOld = GetOldItem( *rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( !pOld || static_cast<const SvxPaperBinItem*>(pOld)->GetValue() != nBin )
    {
        rSet->Put( SvxPaperBinItem( nWhich, (sal_uInt8)nBin ) );
        bModified = true;
    }

    nPos = m_pPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)reinterpret_cast<sal_uLong>(m_pPaperSizeBox->GetEntryData( nPos ));
    const sal_Int32 nOld = m_pPaperSizeBox->GetSavedValue();
    bool bChecked = m_pLandscapeBtn->IsChecked();

    if ( PAPER_USER == ePaper )
    {
        if ( nOld != nPos                       ||
             m_pPaperWidthEdit->IsValueModified()  ||
             m_pPaperHeightEdit->IsValueModified() ||
             m_pLandscapeBtn->IsValueChangedFromSaved() )
        {
            Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                        GetCoreValue( *m_pPaperHeightEdit, eUnit ) );
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
        if ( nOld != nPos || m_pLandscapeBtn->IsValueChangedFromSaved() )
        {
            Size aSize( SvxPaperInfo::GetPaperSize( ePaper, (MapUnit)eUnit ) );

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
    bMod = m_pLayoutBox->IsValueChangedFromSaved();

    if ( bMod )
        aPage.SetPageUsage(
            ::PosToPageUsage_Impl( m_pLayoutBox->GetSelectEntryPos() ) );

    if ( m_pLandscapeBtn->IsValueChangedFromSaved() )
    {
        aPage.SetLandscape(bChecked);
        bMod = true;
    }

    //Get the NumType value
    nPos = m_pNumberFormatBox->GetSelectEntryPos();
    sal_uInt16 nEntryData = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pNumberFormatBox->GetEntryData(nPos));
    if ( m_pNumberFormatBox->IsValueChangedFromSaved() )
    {
        aPage.SetNumType( (SvxNumType)nEntryData );
        bMod = true;
    }

    if ( bMod )
    {
        pOld = GetOldItem( *rSet, SID_ATTR_PAGE );

        if ( !pOld || !( *static_cast<const SvxPageItem*>(pOld) == aPage ) )
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
            if ( m_pHorzBox->IsValueChangedFromSaved() )
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT1 ),
                                   m_pHorzBox->IsChecked() );
                rSet->Put( aHorz );
                bModified = true;
            }

            if ( m_pVertBox->IsValueChangedFromSaved() )
            {
                SfxBoolItem aVert( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   m_pVertBox->IsChecked() );
                rSet->Put( aVert );
                bModified = true;
            }
            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            // always put so that draw can evaluate this
            rSet->Put( SfxBoolItem( GetWhich( SID_ATTR_PAGE_EXT1 ),
                      m_pAdaptBox->IsChecked() ) );
            bModified = true;
            break;
        }
        default: ;//prevent warning

    }

    if(m_pRegisterCB->IsVisible() &&
       (m_pRegisterCB->IsChecked() || m_pRegisterCB->IsValueChangedFromSaved()))
    {
        const SfxBoolItem& rRegItem = static_cast<const SfxBoolItem&>(rOldSet.Get(SID_SWREGISTER_MODE));
        std::unique_ptr<SfxBoolItem> pRegItem(static_cast<SfxBoolItem*>(rRegItem.Clone()));
        bool bCheck = m_pRegisterCB->IsChecked();
        pRegItem->SetValue(bCheck);
        rSet->Put(*pRegItem);
        bModified = true;
        if(bCheck)
        {
            bModified = true;
            rSet->Put(SfxStringItem(SID_SWREGISTER_COLLECTION,
                            m_pRegisterLB->GetSelectEntry()));
        }
    }

    SvxFrameDirection eDirection = m_pTextFlowBox->GetSelectEntryValue();
    if( m_pTextFlowBox->IsVisible() && m_pTextFlowBox->IsValueChangedFromSaved() )
    {
        rSet->Put( SvxFrameDirectionItem( eDirection, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
        bModified = true;
    }

    return bModified;
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, LayoutHdl_Impl, ListBox&, void)
{
    // switch inside outside
    const sal_uInt16 nPos = PosToPageUsage_Impl( m_pLayoutBox->GetSelectEntryPos() );

    if ( nPos == SVX_PAGE_MIRROR )
    {
        m_pLeftMarginLbl->Hide();
        m_pRightMarginLbl->Hide();
        m_pInsideLbl->Show();
        m_pOutsideLbl->Show();
    }
    else
    {
        m_pLeftMarginLbl->Show();
        m_pRightMarginLbl->Show();
        m_pInsideLbl->Hide();
        m_pOutsideLbl->Hide();
    }
    UpdateExample_Impl( true );
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, PaperBinHdl_Impl, Control&, void)
{
    if ( m_pPaperTrayBox->GetEntryCount() > 1 )
        // already filled
        return;

    OUString aOldName = m_pPaperTrayBox->GetSelectEntry();
    m_pPaperTrayBox->SetUpdateMode( false );
    m_pPaperTrayBox->Clear();
    sal_Int32 nEntryPos = m_pPaperTrayBox->InsertEntry(
        EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS ) );
    m_pPaperTrayBox->SetEntryData( nEntryPos,
        reinterpret_cast<void*>((sal_uLong)PAPERBIN_PRINTER_SETTINGS) );
    OUString aPaperBin( EditResId( RID_SVXSTR_PAPERBIN ) );
    sal_uInt16 nBinCount = mpDefPrinter->GetPaperBinCount();

    for ( sal_uInt16 i = 0; i < nBinCount; ++i )
    {
        OUString aName = mpDefPrinter->GetPaperBinName(i);

        if ( aName.isEmpty() )
        {
            aName = aPaperBin + " " + OUString::number( i+1 );
        }
        nEntryPos = m_pPaperTrayBox->InsertEntry( aName );
        m_pPaperTrayBox->SetEntryData( nEntryPos, reinterpret_cast<void*>((sal_uLong)i) );
    }
    m_pPaperTrayBox->SelectEntry( aOldName );
    m_pPaperTrayBox->SetUpdateMode( true );
}


IMPL_LINK_TYPED( SvxPageDescPage, PaperSizeSelect_Impl, ListBox&, rBox, void )
{
    PaperSizeListBox& rListBox = static_cast<PaperSizeListBox&>( rBox );
    Paper ePaper = rListBox.GetSelection();

    if ( ePaper != PAPER_USER )
    {
        Size aSize( SvxPaperInfo::GetPaperSize( ePaper, MAP_100TH_MM ) );

        if ( m_pLandscapeBtn->IsChecked() )
            Swap( aSize );

        if ( aSize.Height() < m_pPaperHeightEdit->GetMin( FUNIT_100TH_MM ) )
            m_pPaperHeightEdit->SetMin(
                m_pPaperHeightEdit->Normalize( aSize.Height() ), FUNIT_100TH_MM );
        if ( aSize.Width() < m_pPaperWidthEdit->GetMin( FUNIT_100TH_MM ) )
            m_pPaperWidthEdit->SetMin(
                m_pPaperWidthEdit->Normalize( aSize.Width() ), FUNIT_100TH_MM );
        SetMetricValue( *m_pPaperHeightEdit, aSize.Height(), SFX_MAPUNIT_100TH_MM );
        SetMetricValue( *m_pPaperWidthEdit, aSize.Width(), SFX_MAPUNIT_100TH_MM );

        CalcMargin_Impl();

        RangeHdl_Impl( *m_pPaperWidthEdit );
        UpdateExample_Impl( true );

        if ( eMode == SVX_PAGE_MODE_PRESENTATION )
        {
            // Draw: if paper format the margin shall be 1 cm
            long nTmp = 0;
            bool bScreen = (( PAPER_SCREEN_4_3 == ePaper )||( PAPER_SCREEN_16_9 == ePaper)||( PAPER_SCREEN_16_10 == ePaper));

            if ( !bScreen )
                // no margin if screen
                nTmp = 1; // accordingly 1 cm

            if ( bScreen || m_pRightMarginEdit->GetValue() == 0 )
            {
                SetMetricValue( *m_pRightMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     m_pRightMarginEdit->GetFirst() > m_pRightMarginEdit->GetValue() )
                    m_pRightMarginEdit->SetValue( m_pRightMarginEdit->GetFirst() );
            }
            if ( bScreen || m_pLeftMarginEdit->GetValue() == 0 )
            {
                SetMetricValue( *m_pLeftMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     m_pLeftMarginEdit->GetFirst() > m_pLeftMarginEdit->GetValue() )
                    m_pLeftMarginEdit->SetValue( m_pLeftMarginEdit->GetFirst() );
            }
            if ( bScreen || m_pBottomMarginEdit->GetValue() == 0 )
            {
                SetMetricValue( *m_pBottomMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     m_pBottomMarginEdit->GetFirst() > m_pBottomMarginEdit->GetValue() )
                    m_pBottomMarginEdit->SetValue( m_pBottomMarginEdit->GetFirst() );
            }
            if ( bScreen || m_pTopMarginEdit->GetValue() == 0 )
            {
                SetMetricValue( *m_pTopMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     m_pTopMarginEdit->GetFirst() > m_pTopMarginEdit->GetValue() )
                    m_pTopMarginEdit->SetValue( m_pTopMarginEdit->GetFirst() );
            }
            UpdateExample_Impl( true );
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, PaperSizeModify_Impl, Edit&, void)
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                GetCoreValue( *m_pPaperHeightEdit, eUnit ) );
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, (MapUnit)eUnit, true );

    m_pPaperSizeBox->SetSelection( ePaper );
    UpdateExample_Impl( true );
}


IMPL_LINK_TYPED( SvxPageDescPage, SwapOrientation_Impl, Button *, pBtn, void )
{
    if (
        (!bLandscape && pBtn == m_pLandscapeBtn) ||
        (bLandscape  && pBtn == m_pPortraitBtn)
       )
    {
        bLandscape = m_pLandscapeBtn->IsChecked();

        const long lWidth = GetCoreValue( *m_pPaperWidthEdit, SFX_MAPUNIT_100TH_MM );
        const long lHeight = GetCoreValue( *m_pPaperHeightEdit, SFX_MAPUNIT_100TH_MM );

        // swap width and height
        SetMetricValue( *m_pPaperWidthEdit, lHeight, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( *m_pPaperHeightEdit, lWidth, SFX_MAPUNIT_100TH_MM );

        // recalculate margins if necessary
        CalcMargin_Impl();

        PaperSizeSelect_Impl( *m_pPaperSizeBox );
        RangeHdl_Impl( *m_pPaperWidthEdit );
        SwapFirstValues_Impl( bBorderModified );
        UpdateExample_Impl( true );
    }
}


void SvxPageDescPage::SwapFirstValues_Impl( bool bSet )
{
    MapMode aOldMode = mpDefPrinter->GetMapMode();
    Orientation eOri = ORIENTATION_PORTRAIT;

    if ( bLandscape )
        eOri = ORIENTATION_LANDSCAPE;
    Orientation eOldOri = mpDefPrinter->GetOrientation();
    mpDefPrinter->SetOrientation( eOri );
    mpDefPrinter->SetMapMode( MAP_TWIP );

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

    sal_Int64 nSetL = m_pLeftMarginEdit->Denormalize(
                    m_pLeftMarginEdit->GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetR = m_pRightMarginEdit->Denormalize(
                    m_pRightMarginEdit->GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetT = m_pTopMarginEdit->Denormalize(
                    m_pTopMarginEdit->GetValue( FUNIT_TWIP ) );
    sal_Int64 nSetB = m_pBottomMarginEdit->Denormalize(
                    m_pBottomMarginEdit->GetValue( FUNIT_TWIP ) );

    long nOffset = !aPrintOffset.X() && !aPrintOffset.Y() ? 0 : PRINT_OFFSET;
    long nNewL = aPrintOffset.X();
    long nNewR =
        aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset;
    long nNewT = aPrintOffset.Y();
    long nNewB =
        aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset;

    m_pLeftMarginEdit->SetFirst( m_pLeftMarginEdit->Normalize( nNewL ), FUNIT_TWIP );
    nFirstLeftMargin = static_cast<long>(m_pLeftMarginEdit->GetFirst());
    m_pRightMarginEdit->SetFirst( m_pRightMarginEdit->Normalize( nNewR ), FUNIT_TWIP );
    nFirstRightMargin = static_cast<long>(m_pRightMarginEdit->GetFirst());
    m_pTopMarginEdit->SetFirst( m_pTopMarginEdit->Normalize( nNewT ), FUNIT_TWIP );
    nFirstTopMargin = static_cast<long>(m_pTopMarginEdit->GetFirst());
    m_pBottomMarginEdit->SetFirst( m_pBottomMarginEdit->Normalize( nNewB ), FUNIT_TWIP );
    nFirstBottomMargin = static_cast<long>(m_pBottomMarginEdit->GetFirst());

    if ( bSet )
    {
        if ( nSetL < nNewL )
            m_pLeftMarginEdit->SetValue( m_pLeftMarginEdit->Normalize( nNewL ),
                                      FUNIT_TWIP );
        if ( nSetR < nNewR )
            m_pRightMarginEdit->SetValue( m_pRightMarginEdit->Normalize( nNewR ),
                                       FUNIT_TWIP );
        if ( nSetT < nNewT )
            m_pTopMarginEdit->SetValue( m_pTopMarginEdit->Normalize( nNewT ),
                                     FUNIT_TWIP );
        if ( nSetB < nNewB )
            m_pBottomMarginEdit->SetValue( m_pBottomMarginEdit->Normalize( nNewB ),
                                        FUNIT_TWIP );
    }
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, BorderModify_Impl, Edit&, void)
{
    if ( !bBorderModified )
        bBorderModified = true;
    UpdateExample_Impl();
}

void SvxPageDescPage::UpdateExample_Impl( bool bResetbackground )
{
    // Size
    Size aSize( GetCoreValue( *m_pPaperWidthEdit, SFX_MAPUNIT_TWIP ),
                GetCoreValue( *m_pPaperHeightEdit, SFX_MAPUNIT_TWIP ) );

    m_pBspWin->SetSize( aSize );

    // Margins
    m_pBspWin->SetTop( GetCoreValue( *m_pTopMarginEdit, SFX_MAPUNIT_TWIP ) );
    m_pBspWin->SetBottom( GetCoreValue( *m_pBottomMarginEdit, SFX_MAPUNIT_TWIP ) );
    m_pBspWin->SetLeft( GetCoreValue( *m_pLeftMarginEdit, SFX_MAPUNIT_TWIP ) );
    m_pBspWin->SetRight( GetCoreValue( *m_pRightMarginEdit, SFX_MAPUNIT_TWIP ) );

    // Layout
    m_pBspWin->SetUsage( PosToPageUsage_Impl( m_pLayoutBox->GetSelectEntryPos() ) );
    if ( bResetbackground )
        m_pBspWin->ResetBackground();
    m_pBspWin->Invalidate();
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
                //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
                aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rTmpSet));
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    //UUUU create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
                }
            }

            m_pBspWin->setHeaderFillAttributes(aHeaderFillAttributes);
            nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

            if(rTmpSet.GetItemState(nWhich) == SfxItemState::SET)
            {
                const SvxBoxItem& rItem =
                    static_cast<const SvxBoxItem&>(rTmpSet.Get( nWhich ));
                m_pBspWin->SetHdBorder( rItem );
            }
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
                //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
                aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rTmpSet));
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    //UUUU create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
                }
            }

            m_pBspWin->setFooterFillAttributes(aFooterFillAttributes);
            nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

            if(rTmpSet.GetItemState(nWhich) == SfxItemState::SET)
            {
                const SvxBoxItem& rItem = static_cast< const SvxBoxItem& >(rTmpSet.Get(nWhich));
                m_pBspWin->SetFtBorder(rItem);
            }
        }
    }

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aPageFillAttributes;
    const SfxPoolItem* pItem = nullptr;

    if(mbEnableDrawingLayerFillStyles)
    {
        //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
        aPageFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rSet));
    }
    else
    {
        pItem = GetItem(rSet, SID_ATTR_BRUSH);

        if(pItem)
        {
            //UUUU create FillAttributes from SvxBrushItem
            const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(*pItem);
            SfxItemSet aTempSet(*rSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

            setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
            aPageFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
        }
    }

    m_pBspWin->setPageFillAttributes(aPageFillAttributes);
    pItem = GetItem(rSet, SID_ATTR_BORDER_OUTER);

    if(pItem)
    {
        m_pBspWin->SetBorder(static_cast< const SvxBoxItem& >(*pItem));
    }
}

void SvxPageDescPage::InitHeadFoot_Impl( const SfxItemSet& rSet )
{
    bLandscape = m_pLandscapeBtn->IsChecked();
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        m_pBspWin->SetSize( static_cast<const SvxSizeItem*>(pItem)->GetSize() );

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
            long nDist = rUL.GetLower();
            m_pBspWin->SetHdHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetHdDist( nDist );
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            m_pBspWin->SetHdLeft( rLR.GetLeft() );
            m_pBspWin->SetHdRight( rLR.GetRight() );
            m_pBspWin->SetHeader( true );
        }
        else
            m_pBspWin->SetHeader( false );

        // show background and border in the example
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aHeaderFillAttributes;

        if(mbEnableDrawingLayerFillStyles)
        {
            //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
            aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rHeaderSet));
        }
        else
        {
            const sal_uInt16 nWhich(GetWhich(SID_ATTR_BRUSH));

            if(rHeaderSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
            {
                //UUUU aBspWin.SetHdColor(rItem.GetColor());
                const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rHeaderSet.Get(nWhich));
                SfxItemSet aTempSet(*rHeaderSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
            }
        }

        m_pBspWin->setHeaderFillAttributes(aHeaderFillAttributes);
        const sal_uInt16 nWhich(GetWhich(SID_ATTR_BORDER_OUTER));

        if ( rHeaderSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
        {
            const SvxBoxItem& rItem =
                static_cast<const SvxBoxItem&>(rHeaderSet.Get( nWhich ));
            m_pBspWin->SetHdBorder( rItem );
        }
    }

    // evaluate footer attributes

    if ( SfxItemState::SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                            false, reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            static_cast<const SfxBoolItem&>(rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) ));

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) ));
            long nDist = rUL.GetUpper();
            m_pBspWin->SetFtHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetFtDist( nDist );
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            m_pBspWin->SetFtLeft( rLR.GetLeft() );
            m_pBspWin->SetFtRight( rLR.GetRight() );
            m_pBspWin->SetFooter( true );
        }
        else
            m_pBspWin->SetFooter( false );

        // show background and border in the example
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFooterFillAttributes;

        if(mbEnableDrawingLayerFillStyles)
        {
            //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
            aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rFooterSet));
        }
        else
        {
            const sal_uInt16 nWhich(GetWhich(SID_ATTR_BRUSH));

            if(rFooterSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
            {
                //UUUU aBspWin.SetFtColor(rItem.GetColor());
                const SvxBrushItem& rItem = static_cast<const SvxBrushItem&>(rFooterSet.Get(nWhich));
                SfxItemSet aTempSet(*rFooterSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
            }
        }

        m_pBspWin->setFooterFillAttributes(aFooterFillAttributes);
        const sal_uInt16 nWhich(GetWhich(SID_ATTR_BORDER_OUTER));

        if ( rFooterSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
        {
            const SvxBoxItem& rItem =
                static_cast<const SvxBoxItem&>(rFooterSet.Get( nWhich ));
            m_pBspWin->SetFtBorder( rItem );
        }
    }
}


void SvxPageDescPage::ActivatePage( const SfxItemSet& rSet )
{
    InitHeadFoot_Impl( rSet );
    UpdateExample_Impl();
    ResetBackground_Impl( rSet );
    RangeHdl_Impl( *m_pPaperWidthEdit );
}


DeactivateRC SvxPageDescPage::DeactivatePage( SfxItemSet* _pSet )
{
    // Inquiry whether the page margins are beyond the printing area.
    // If not, ask user whether they shall be taken.
    // If not, stay on the TabPage.
    Paper ePaper = m_pPaperSizeBox->GetSelection();

    if ( ePaper != PAPER_SCREEN_4_3 && ePaper != PAPER_SCREEN_16_9 && ePaper != PAPER_SCREEN_16_10 && IsMarginOutOfRange() )
    {
        if ( ScopedVclPtr<QueryBox>::Create( this, WB_YES_NO | WB_DEF_NO, m_pPrintRangeQueryText->GetText() )->Execute() == RET_NO )
        {
            MetricField* pField = nullptr;
            if ( IsPrinterRangeOverflow( *m_pLeftMarginEdit, nFirstLeftMargin, nLastLeftMargin, MARGIN_LEFT ) )
                pField = m_pLeftMarginEdit;
            if (    IsPrinterRangeOverflow( *m_pRightMarginEdit, nFirstRightMargin, nLastRightMargin, MARGIN_RIGHT )
                 && !pField )
                pField = m_pRightMarginEdit;
            if (    IsPrinterRangeOverflow( *m_pTopMarginEdit, nFirstTopMargin, nLastTopMargin, MARGIN_TOP )
                 && !pField )
                pField = m_pTopMarginEdit;
            if (    IsPrinterRangeOverflow( *m_pBottomMarginEdit, nFirstBottomMargin, nLastBottomMargin, MARGIN_BOTTOM )
                 && !pField )
                pField = m_pBottomMarginEdit;
            if ( pField )
                pField->GrabFocus();
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
        SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWh );
        Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                    GetCoreValue( *m_pPaperHeightEdit, eUnit ) );

        // put, if current size is different to the value in _pSet
        const SvxSizeItem* pSize = static_cast<const SvxSizeItem*>(GetItem( *_pSet, SID_ATTR_PAGE_SIZE ));
        if ( aSize.Width() && ( !pSize || !IsEqualSize_Impl( pSize, aSize ) ) )
            _pSet->Put( SvxSizeItem( nWh, aSize ) );
    }

    return DeactivateRC::LeavePage;
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, RangeHdl_Impl, Control&, void)
{
    // example window
    long nHHeight = m_pBspWin->GetHdHeight();
    long nHDist = m_pBspWin->GetHdDist();

    long nFHeight = m_pBspWin->GetFtHeight();
    long nFDist = m_pBspWin->GetFtDist();

    long nHFLeft = std::max( m_pBspWin->GetHdLeft(), m_pBspWin->GetFtLeft() );
    long nHFRight = std::max( m_pBspWin->GetHdRight(), m_pBspWin->GetFtRight() );

    // current values for page margins
    long nBT = static_cast<long>(m_pTopMarginEdit->Denormalize(m_pTopMarginEdit->GetValue(FUNIT_TWIP)));
    long nBB = static_cast<long>(m_pBottomMarginEdit->Denormalize(m_pBottomMarginEdit->GetValue(FUNIT_TWIP)));
    long nBL = static_cast<long>(m_pLeftMarginEdit->Denormalize(m_pLeftMarginEdit->GetValue(FUNIT_TWIP)));
    long nBR = static_cast<long>(m_pRightMarginEdit->Denormalize(m_pRightMarginEdit->GetValue(FUNIT_TWIP)));

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

    long nMin = nHHeight + nHDist + nFDist + nFHeight + nBT + nBB +
                MINBODY + aBorder.Height();
    m_pPaperHeightEdit->SetMin(m_pPaperHeightEdit->Normalize(nMin), FUNIT_TWIP);

    nMin = MINBODY + nBL + nBR + aBorder.Width();
    m_pPaperWidthEdit->SetMin(m_pPaperWidthEdit->Normalize(nMin), FUNIT_TWIP);

    long nH = static_cast<long>(m_pPaperHeightEdit->Denormalize(m_pPaperHeightEdit->GetValue(FUNIT_TWIP)));
    long nW = static_cast<long>(m_pPaperWidthEdit->Denormalize(m_pPaperWidthEdit->GetValue(FUNIT_TWIP)));

    // Top
    long nMax = nH - nBB - aBorder.Height() - MINBODY -
                nFDist - nFHeight - nHDist - nHHeight;

    m_pTopMarginEdit->SetMax(m_pTopMarginEdit->Normalize(nMax), FUNIT_TWIP);

    // Bottom
    nMax = nH - nBT - aBorder.Height() - MINBODY -
           nFDist - nFHeight - nHDist - nHHeight;

    m_pBottomMarginEdit->SetMax(m_pTopMarginEdit->Normalize(nMax), FUNIT_TWIP);

    // Left
    nMax = nW - nBR - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    m_pLeftMarginEdit->SetMax(m_pLeftMarginEdit->Normalize(nMax), FUNIT_TWIP);

    // Right
    nMax = nW - nBL - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    m_pRightMarginEdit->SetMax(m_pRightMarginEdit->Normalize(nMax), FUNIT_TWIP);
}


void SvxPageDescPage::CalcMargin_Impl()
{
    // current values for page margins
    long nBT = GetCoreValue( *m_pTopMarginEdit, SFX_MAPUNIT_TWIP );
    long nBB = GetCoreValue( *m_pBottomMarginEdit, SFX_MAPUNIT_TWIP );

    long nBL = GetCoreValue( *m_pLeftMarginEdit, SFX_MAPUNIT_TWIP );
    long nBR = GetCoreValue( *m_pRightMarginEdit, SFX_MAPUNIT_TWIP );

    long nH  = GetCoreValue( *m_pPaperHeightEdit, SFX_MAPUNIT_TWIP );
    long nW  = GetCoreValue( *m_pPaperWidthEdit, SFX_MAPUNIT_TWIP );

    long nWidth = nBL + nBR + MINBODY;
    long nHeight = nBT + nBB + MINBODY;

    if ( nWidth > nW || nHeight > nH )
    {
        if ( nWidth > nW )
        {
            long nTmp = nBL <= nBR ? nBR : nBL;
            nTmp -= nWidth - nW;

            if ( nBL <= nBR )
                SetMetricValue( *m_pRightMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( *m_pLeftMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }

        if ( nHeight > nH )
        {
            long nTmp = nBT <= nBB ? nBB : nBT;
            nTmp -= nHeight - nH;

            if ( nBT <= nBB )
                SetMetricValue( *m_pBottomMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( *m_pTopMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxPageDescPage, CenterHdl_Impl, Button*, void)
{
    m_pBspWin->SetHorz( m_pHorzBox->IsChecked() );
    m_pBspWin->SetVert( m_pVertBox->IsChecked() );
    UpdateExample_Impl();
}

void SvxPageDescPage::SetCollectionList(const std::vector<OUString> &aList)
{
    OSL_ENSURE(!aList.empty(), "Empty string list");

    sStandardRegister = aList[0];
    for( size_t i = 1; i < aList.size(); i++   )
        m_pRegisterLB->InsertEntry(aList[i]);

    m_pRegisterCB->Show();
    m_pRegisterFT->Show();
    m_pRegisterLB->Show();
    m_pRegisterCB->SetClickHdl(LINK(this, SvxPageDescPage, RegisterModify));
}


IMPL_LINK_TYPED( SvxPageDescPage, RegisterModify, Button*, pBox, void )
{
    bool bEnable = false;
    if(static_cast<CheckBox*>(pBox)->IsChecked())
    {
        bEnable = true;
        if(USHRT_MAX == m_pRegisterLB->GetSelectEntryPos())
            m_pRegisterLB->SelectEntry(sStandardRegister);
    }
    m_pRegisterFT->Enable( bEnable );
    m_pRegisterLB->Enable( bEnable );
}


void SvxPageDescPage::DisableVerticalPageDir()
{
    m_pTextFlowBox->RemoveEntryValue( FRMDIR_VERT_TOP_RIGHT );
    m_pTextFlowBox->RemoveEntryValue( FRMDIR_VERT_TOP_LEFT );
    if( m_pTextFlowBox->GetEntryCount() < 2 )
    {
        m_pTextFlowLbl->Hide();
        m_pTextFlowBox->Hide();
        m_pBspWin->EnableFrameDirection( false );
    }
}

IMPL_LINK_NOARG_TYPED( SvxPageDescPage, FrameDirectionModify_Impl, ListBox&, void)
{
    m_pBspWin->SetFrameDirection( (sal_uInt32) m_pTextFlowBox->GetSelectEntryValue() );
    m_pBspWin->Invalidate();
}

bool SvxPageDescPage::IsPrinterRangeOverflow(
    MetricField& rField, long nFirstMargin, long nLastMargin, MarginPosition nPos )
{
    bool bRet = false;
    bool bCheck = ( ( m_nPos & nPos ) == 0 );
    long nValue = static_cast<long>(rField.GetValue());
    if ( bCheck &&
         (  nValue < nFirstMargin || nValue > nLastMargin ) &&
         rField.IsValueChangedFromSaved() )
    {
        rField.SetValue( nValue < nFirstMargin ? nFirstMargin : nLastMargin );
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

    sal_Int64 nValue = m_pLeftMarginEdit->GetValue();
    if (  nValue < nFirstLeftMargin || nValue > nLastLeftMargin )
        m_nPos |= MARGIN_LEFT;
    nValue = m_pRightMarginEdit->GetValue();
    if (  nValue < nFirstRightMargin || nValue > nLastRightMargin )
        m_nPos |= MARGIN_RIGHT;
    nValue = m_pTopMarginEdit->GetValue();
    if (  nValue < nFirstTopMargin || nValue > nLastTopMargin )
        m_nPos |= MARGIN_TOP;
    nValue = m_pBottomMarginEdit->GetValue();
    if (  nValue < nFirstBottomMargin || nValue > nLastBottomMargin )
        m_nPos |= MARGIN_BOTTOM;
}

bool SvxPageDescPage::IsMarginOutOfRange()
{
    bool bRet = ( ( ( !( m_nPos & MARGIN_LEFT ) &&
                      m_pLeftMarginEdit->IsValueChangedFromSaved() ) &&
                    ( m_pLeftMarginEdit->GetValue() < nFirstLeftMargin ||
                      m_pLeftMarginEdit->GetValue() > nLastLeftMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_RIGHT ) &&
                      m_pRightMarginEdit->IsValueChangedFromSaved() ) &&
                    ( m_pRightMarginEdit->GetValue() < nFirstRightMargin ||
                      m_pRightMarginEdit->GetValue() > nLastRightMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_TOP ) &&
                      m_pTopMarginEdit->IsValueChangedFromSaved() ) &&
                    ( m_pTopMarginEdit->GetValue() < nFirstTopMargin ||
                      m_pTopMarginEdit->GetValue() > nLastTopMargin ) ) ||
                  ( ( !( m_nPos & MARGIN_BOTTOM ) &&
                      m_pBottomMarginEdit->IsValueChangedFromSaved() ) &&
                    ( m_pBottomMarginEdit->GetValue() < nFirstBottomMargin ||
                      m_pBottomMarginEdit->GetValue() > nLastBottomMargin ) ) );
    return bRet;
}

void SvxPageDescPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxAllEnumItem* pModeItem = aSet.GetItem<SfxAllEnumItem>(SID_ENUM_PAGE_MODE, false);
    const SfxAllEnumItem* pPaperStartItem = aSet.GetItem<SfxAllEnumItem>(SID_PAPER_START, false);
    const SfxAllEnumItem* pPaperEndItem = aSet.GetItem<SfxAllEnumItem>(SID_PAPER_END, false);
    const SfxStringListItem* pCollectListItem = aSet.GetItem<SfxStringListItem>(SID_COLLECT_LIST, false);

    //UUUU
    const SfxBoolItem* pSupportDrawingLayerFillStyleItem = aSet.GetItem<SfxBoolItem>(SID_DRAWINGLAYER_FILLSTYLES, false);

    if (pModeItem)
    {
        SetMode((SvxModeType)pModeItem->GetEnumValue());
    }

    if(pPaperStartItem && pPaperEndItem)
    {
        SetPaperFormatRanges((Paper)pPaperStartItem->GetEnumValue(),(Paper)pPaperEndItem->GetEnumValue());
    }

    if(pCollectListItem)
    {
        SetCollectionList(pCollectListItem->GetList());
    }

    if(pSupportDrawingLayerFillStyleItem)
    {
        const bool bNew(pSupportDrawingLayerFillStyleItem->GetValue());

        EnableDrawingLayerFillStyles(bNew);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
