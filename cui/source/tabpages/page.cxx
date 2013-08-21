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
#include <svx/dialogs.hrc>  // for RID_SVXPAGE_PAGE
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc> // for RID_SVXSTR_PAPERBIN...,
#include <svx/svxids.hrc>

// #i4219#
#include <svtools/optionsdrawinglayer.hxx>
#include <svl/slstitm.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/request.hxx>

#include <numpages.hxx>     // for GetI18nNumbering()

// static ----------------------------------------------------------------

static const long MINBODY       = 284;  // 0,5 cm rounded up in twips
//static const long PRINT_OFFSET    = 17;   // 0,03 cm rounded down in twips
static const long PRINT_OFFSET  = 0;    // why was this ever set to 17 ? it led to wrong right and bottom margins.

static sal_uInt16 pRanges[] =
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

sal_uInt16 aArr[] =
{
    SVX_PAGE_ALL,
    SVX_PAGE_MIRROR,
    SVX_PAGE_RIGHT,
    SVX_PAGE_LEFT
};

// -----------------------------------------------------------------------

sal_uInt16 PageUsageToPos_Impl( sal_uInt16 nUsage )
{
    for ( sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] == ( nUsage & 0x000f ) )
            return i;
    return SVX_PAGE_ALL;
}

// -----------------------------------------------------------------------

sal_uInt16 PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return 0;
    return aArr[nPos];
}

// -----------------------------------------------------------------------

Size GetMinBorderSpace_Impl( const SvxShadowItem& rShadow, const SvxBoxItem& rBox )
{
    Size aSz;
    aSz.Height() = rShadow.CalcShadowSpace( SHADOW_BOTTOM ) + rBox.CalcLineSpace( BOX_LINE_BOTTOM );
    aSz.Height() += rShadow.CalcShadowSpace( SHADOW_TOP ) + rBox.CalcLineSpace( BOX_LINE_TOP );
    aSz.Width() = rShadow.CalcShadowSpace( SHADOW_LEFT ) + rBox.CalcLineSpace( BOX_LINE_LEFT );
    aSz.Width() += rShadow.CalcShadowSpace( SHADOW_RIGHT ) + rBox.CalcLineSpace( BOX_LINE_RIGHT );
    return aSz;
}

// -----------------------------------------------------------------------

long ConvertLong_Impl( const long nIn, SfxMapUnit eUnit )
{
    return OutputDevice::LogicToLogic( nIn, (MapUnit)eUnit, MAP_TWIP );
}

sal_Bool IsEqualSize_Impl( const SvxSizeItem* pSize, const Size& rSize )
{
    if ( pSize )
    {
        Size aSize = pSize->GetSize();
        long nDiffW = std::abs( rSize.Width () - aSize.Width () );
        long nDiffH = std::abs( rSize.Height() - aSize.Height() );
        return ( nDiffW < 10 && nDiffH < 10 );
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

#define MARGIN_LEFT     ( (MarginPosition)0x0001 )
#define MARGIN_RIGHT    ( (MarginPosition)0x0002 )
#define MARGIN_TOP      ( (MarginPosition)0x0004 )
#define MARGIN_BOTTOM   ( (MarginPosition)0x0008 )

struct SvxPage_Impl
{
    MarginPosition  m_nPos;
    Printer*        mpDefPrinter;
    bool            mbDelPrinter;

    SvxPage_Impl() :
        m_nPos( 0 ),
        mpDefPrinter( 0 ),
        mbDelPrinter( false ) {}

    ~SvxPage_Impl() { if ( mbDelPrinter ) delete mpDefPrinter; }
};

// class SvxPageDescPage --------------------------------------------------

// returns the range of the Which values
sal_uInt16* SvxPageDescPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPageDescPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxPageDescPage( pParent, rSet );
}

// -----------------------------------------------------------------------

SvxPageDescPage::SvxPageDescPage( Window* pParent, const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, "PageFormatPage", "cui/ui/pageformatpage.ui", rAttr ),

    bLandscape          ( sal_False ),
    eMode               ( SVX_PAGE_MODE_STANDARD ),
    ePaperStart         ( PAPER_A3 ),
    ePaperEnd           ( PAPER_ENV_DL ),
    pImpl               ( new SvxPage_Impl )

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

    bBorderModified = sal_False;
    m_pBspWin->EnableRTL( sal_False );

    // this page needs ExchangeSupport
    SetExchangeSupport();

    SvtLanguageOptions aLangOptions;
    sal_Bool bCJK = aLangOptions.IsAsianTypographyEnabled();
    sal_Bool bCTL = aLangOptions.IsCTLFontEnabled();
    sal_Bool bWeb = sal_False;
    const SfxPoolItem* pItem;

    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
        bWeb = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);

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
        SFX_ITEM_UNKNOWN < rAttr.GetItemState(GetWhich( SID_ATTR_FRAMEDIRECTION )))
    {
        m_pTextFlowLbl->Show();
        m_pTextFlowBox->Show();
        m_pTextFlowBox->SetSelectHdl(LINK(this, SvxPageDescPage, FrameDirectionModify_Impl ));

        m_pBspWin->EnableFrameDirection(sal_True);
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
        pImpl->mpDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pImpl->mpDefPrinter = new Printer;
        pImpl->mbDelPrinter = true;
    }

    MapMode aOldMode = pImpl->mpDefPrinter->GetMapMode();
    pImpl->mpDefPrinter->SetMapMode( MAP_TWIP );

    // set first- and last-values for the margins
    Size aPaperSize = pImpl->mpDefPrinter->GetPaperSize();
    Size aPrintSize = pImpl->mpDefPrinter->GetOutputSize();
    /*
     * To convert a point ( 0,0 ) into logic coordinates
     * looks like nonsense; but it makes sense when the
     * coordinate system's origin has been moved.
     */
    Point aPrintOffset = pImpl->mpDefPrinter->GetPageOffset() -
                         pImpl->mpDefPrinter->PixelToLogic( Point() );
    pImpl->mpDefPrinter->SetMapMode( aOldMode );

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
    m_pLeftMarginEdit->SetMax(aDrawinglayerOpt.GetMaximumPaperLeftMargin());
    m_pLeftMarginEdit->SetLast(aDrawinglayerOpt.GetMaximumPaperLeftMargin());
    m_pRightMarginEdit->SetMax(aDrawinglayerOpt.GetMaximumPaperRightMargin());
    m_pRightMarginEdit->SetLast(aDrawinglayerOpt.GetMaximumPaperRightMargin());
    m_pTopMarginEdit->SetMax(aDrawinglayerOpt.GetMaximumPaperTopMargin());
    m_pTopMarginEdit->SetLast(aDrawinglayerOpt.GetMaximumPaperTopMargin());
    m_pBottomMarginEdit->SetMax(aDrawinglayerOpt.GetMaximumPaperBottomMargin());
    m_pBottomMarginEdit->SetLast(aDrawinglayerOpt.GetMaximumPaperBottomMargin());

    m_pPortraitBtn->SetAccessibleRelationMemberOf(m_pOrientationFT);
    m_pLandscapeBtn->SetAccessibleRelationMemberOf(m_pOrientationFT);

    // Get the i18n framework numberings and add them to the listbox.
    SvxNumOptionsTabPage::GetI18nNumbering( *m_pNumberFormatBox, ::std::numeric_limits<sal_uInt16>::max());
}

// -----------------------------------------------------------------------

SvxPageDescPage::~SvxPageDescPage()
{
    delete pImpl;
}

// -----------------------------------------------------------------------

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

    Link aLink = LINK( this, SvxPageDescPage, BorderModify_Impl );
    m_pLeftMarginEdit->SetModifyHdl( aLink );
    m_pRightMarginEdit->SetModifyHdl( aLink );
    m_pTopMarginEdit->SetModifyHdl( aLink );
    m_pBottomMarginEdit->SetModifyHdl( aLink );

    aLink = LINK( this, SvxPageDescPage, RangeHdl_Impl );
    m_pPaperWidthEdit->SetLoseFocusHdl( aLink );
    m_pPaperHeightEdit->SetLoseFocusHdl( aLink );
    m_pLeftMarginEdit->SetLoseFocusHdl( aLink );
    m_pRightMarginEdit->SetLoseFocusHdl( aLink );
    m_pTopMarginEdit->SetLoseFocusHdl( aLink );
    m_pBottomMarginEdit->SetLoseFocusHdl( aLink );

    m_pHorzBox->SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );
    m_pVertBox->SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );

}

// -----------------------------------------------------------------------

void SvxPageDescPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) );

    // adjust margins (right/left)
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;
        SetMetricValue( *m_pLeftMarginEdit, rLRSpace.GetLeft(), eUnit );
        m_pBspWin->SetLeft(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetLeft(), eUnit ) );
        SetMetricValue( *m_pRightMarginEdit, rLRSpace.GetRight(), eUnit );
        m_pBspWin->SetRight(
            (sal_uInt16)ConvertLong_Impl( (long)rLRSpace.GetRight(), eUnit ) );
    }

    // adjust margins (top/bottom)
    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;
        SetMetricValue( *m_pTopMarginEdit, rULSpace.GetUpper(), eUnit );
        m_pBspWin->SetTop(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetUpper(), eUnit ) );
        SetMetricValue( *m_pBottomMarginEdit, rULSpace.GetLower(), eUnit );
        m_pBspWin->SetBottom(
            (sal_uInt16)ConvertLong_Impl( (long)rULSpace.GetLower(), eUnit ) );
    }

    // general page data
    SvxNumType eNumType = SVX_ARABIC;
    bLandscape = ( pImpl->mpDefPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
    sal_uInt16 nUse = (sal_uInt16)SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
    {
        const SvxPageItem& rItem = (const SvxPageItem&)*pItem;
        eNumType = rItem.GetNumType();
        nUse = rItem.GetPageUsage();
        bLandscape = rItem.IsLandscape();
    }

    // alignment
    m_pLayoutBox->SelectEntryPos( ::PageUsageToPos_Impl( nUse ) );
    m_pBspWin->SetUsage( nUse );
    LayoutHdl_Impl( 0 );

    //adjust numeration type of the page style
    //Get the Position of the saved NumType
    for(int i=0; i<m_pNumberFormatBox->GetEntryCount(); i++)
        if(eNumType == (sal_uInt16)(sal_uLong)m_pNumberFormatBox->GetEntryData(i)){
            m_pNumberFormatBox->SelectEntryPos( i );
            break;
        }

    m_pPaperTrayBox->Clear();
    sal_uInt8 nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    pItem = GetItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( pItem )
    {
        nPaperBin = ( (const SvxPaperBinItem*)pItem )->GetValue();

        if ( nPaperBin >= pImpl->mpDefPrinter->GetPaperBinCount() )
            nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    }

    String aBinName;

    if ( PAPERBIN_PRINTER_SETTINGS  == nPaperBin )
        aBinName = EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS );
    else
        aBinName = pImpl->mpDefPrinter->GetPaperBinName( (sal_uInt16)nPaperBin );

    sal_uInt16 nEntryPos = m_pPaperTrayBox->InsertEntry( aBinName );
    m_pPaperTrayBox->SetEntryData( nEntryPos, (void*)(sal_uLong)nPaperBin );
    m_pPaperTrayBox->SelectEntry( aBinName );

    Size aPaperSize = SvxPaperInfo::GetPaperSize( pImpl->mpDefPrinter );
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aPaperSize = ( (const SvxSizeItem*)pItem )->GetSize();

    bool bOrientationSupport =
        pImpl->mpDefPrinter->HasSupport( SUPPORT_SET_ORIENTATION );

    if ( !bOrientationSupport &&
         aPaperSize.Width() > aPaperSize.Height() )
        bLandscape = sal_True;

    m_pLandscapeBtn->Check( bLandscape );
    m_pPortraitBtn->Check( !bLandscape );

    m_pBspWin->SetSize( Size( ConvertLong_Impl( aPaperSize.Width(), eUnit ),
                           ConvertLong_Impl( aPaperSize.Height(), eUnit ) ) );

    aPaperSize = OutputDevice::LogicToLogic(aPaperSize, (MapUnit)eUnit, MAP_100TH_MM);
    if ( bLandscape )
        Swap( aPaperSize );

    // Actual Paper Format
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aPaperSize, MAP_100TH_MM, sal_True );

    if ( PAPER_USER != ePaper )
        aPaperSize = SvxPaperInfo::GetPaperSize( ePaper, MAP_100TH_MM );

    if ( bLandscape )
        Swap( aPaperSize );

    // write values into the edits
    SetMetricValue( *m_pPaperHeightEdit, aPaperSize.Height(), SFX_MAPUNIT_100TH_MM );
    SetMetricValue( *m_pPaperWidthEdit, aPaperSize.Width(), SFX_MAPUNIT_100TH_MM );
    m_pPaperSizeBox->Clear();

    sal_uInt16 nActPos = LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16 nAryId = RID_SVXSTRARY_PAPERSIZE_STD;

    if ( ePaperStart != PAPER_A3 )
        nAryId = RID_SVXSTRARY_PAPERSIZE_DRAW;
    ResStringArray aPaperAry( CUI_RES( nAryId ) );
    sal_uInt32 nCnt = aPaperAry.Count();

    sal_uInt16 nUserPos = LISTBOX_ENTRY_NOTFOUND;
    for ( sal_uInt32 i = 0; i < nCnt; ++i )
    {
        String aStr = aPaperAry.GetString(i);
        Paper eSize = (Paper)aPaperAry.GetValue(i);
        sal_uInt16 nPos = m_pPaperSizeBox->InsertEntry( aStr );
        m_pPaperSizeBox->SetEntryData( nPos, (void*)(sal_uLong)eSize );

        if ( eSize == ePaper )
            nActPos = nPos;
        if( eSize == PAPER_USER )
            nUserPos = nPos;
    }
    // preselect current paper format - #115915#: ePaper might not be in aPaperSizeBox so use PAPER_USER instead
    m_pPaperSizeBox->SelectEntryPos( nActPos != LISTBOX_ENTRY_NOTFOUND ? nActPos : nUserPos );

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
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
            m_pHorzBox->Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : sal_False );

            // vertical alignment
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );
            m_pVertBox->Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : sal_False );

            // set example window on the table
            m_pBspWin->SetTable( sal_True );
            m_pBspWin->SetHorz( m_pHorzBox->IsChecked() );
            m_pBspWin->SetVert( m_pVertBox->IsChecked() );

            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            DisableVerticalPageDir();
            m_pAdaptBox->Show();
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
            m_pAdaptBox->Check( pItem ?
                ( (const SfxBoolItem*)pItem )->GetValue() : sal_False );

            //!!! hidden, because not implemented by StarDraw
            m_pLayoutBox->Hide();
            m_pPageText->Hide();

            break;
        }
        default: ;//prevent warning
    }


    // display background and border in the example
    ResetBackground_Impl( rSet );
//! UpdateExample_Impl();
    RangeHdl_Impl( 0 );

    InitHeadFoot_Impl( rSet );

    bBorderModified = sal_False;
    SwapFirstValues_Impl( sal_False );
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


    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_MODE))
    {
        m_pRegisterCB->Check(((const SfxBoolItem&)rSet.Get(
                                SID_SWREGISTER_MODE)).GetValue());
        m_pRegisterCB->SaveValue();
        RegisterModify(m_pRegisterCB);
    }
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_COLLECTION))
    {
        m_pRegisterLB->SelectEntry(
                ((const SfxStringItem&)rSet.Get(SID_SWREGISTER_COLLECTION)).GetValue());
        m_pRegisterLB->SaveValue();
    }

    SfxItemState eState = rSet.GetItemState( GetWhich( SID_ATTR_FRAMEDIRECTION ),
                                                sal_True, &pItem );
    if( SFX_ITEM_UNKNOWN != eState )
    {
        sal_uInt32 nVal  = SFX_ITEM_SET == eState
                                ? ((SvxFrameDirectionItem*)pItem)->GetValue()
                                : 0;
        m_pTextFlowBox->SelectEntryValue( static_cast< SvxFrameDirection >( nVal ) );

        m_pTextFlowBox->SaveValue();
        m_pBspWin->SetFrameDirection(nVal);
    }
}

// -----------------------------------------------------------------------

void SvxPageDescPage::FillUserData()
{
    if ( SVX_PAGE_MODE_PRESENTATION == eMode )
        SetUserData( m_pAdaptBox->IsChecked() ? OUString("1") : OUString("0") ) ;

}

// -----------------------------------------------------------------------

sal_Bool SvxPageDescPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = pPool->GetMetric( nWhich );
    const SfxPoolItem* pOld = 0;

    // copy old left and right margins
    SvxLRSpaceItem aMargin( (const SvxLRSpaceItem&)rOldSet.Get( nWhich ) );

    // copy old top and bottom margins
    nWhich = GetWhich( SID_ATTR_ULSPACE );
    SvxULSpaceItem aTopMargin( (const SvxULSpaceItem&)rOldSet.Get( nWhich ) );

    if ( m_pLeftMarginEdit->GetText() != m_pLeftMarginEdit->GetSavedValue() )
    {
        aMargin.SetLeft( (sal_uInt16)GetCoreValue( *m_pLeftMarginEdit, eUnit ) );
        bModified |= sal_True;
    }

    if ( m_pRightMarginEdit->GetText() != m_pRightMarginEdit->GetSavedValue() )
    {
        aMargin.SetRight( (sal_uInt16)GetCoreValue( *m_pRightMarginEdit, eUnit ) );
        bModified |= sal_True;
    }

    // set left and right margins
    if ( bModified )
    {
        pOld = GetOldItem( rSet, SID_ATTR_LRSPACE );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) )
            rSet.Put( aMargin );
        else
            bModified = sal_False;
    }

    sal_Bool bMod = sal_False;

    if ( m_pTopMarginEdit->GetText() != m_pTopMarginEdit->GetSavedValue() )
    {
        aTopMargin.SetUpper( (sal_uInt16)GetCoreValue( *m_pTopMarginEdit, eUnit ) );
        bMod |= sal_True;
    }

    if ( m_pBottomMarginEdit->GetText() != m_pBottomMarginEdit->GetSavedValue() )
    {
        aTopMargin.SetLower( (sal_uInt16)GetCoreValue( *m_pBottomMarginEdit, eUnit ) );
        bMod |= sal_True;
    }

    // set top and bottom margins
    //
    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_ULSPACE );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aTopMargin ) )
        {
            bModified |= sal_True;
            rSet.Put( aTopMargin );
        }
    }

    // paper tray
    nWhich = GetWhich( SID_ATTR_PAGE_PAPERBIN );
    sal_uInt16 nPos = m_pPaperTrayBox->GetSelectEntryPos();
    sal_uInt16 nBin = (sal_uInt16)(sal_uLong)m_pPaperTrayBox->GetEntryData( nPos );
    pOld = GetOldItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( !pOld || ( (const SvxPaperBinItem*)pOld )->GetValue() != nBin )
    {
        rSet.Put( SvxPaperBinItem( nWhich, (sal_uInt8)nBin ) );
        bModified |= sal_True;
    }

    nPos = m_pPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)m_pPaperSizeBox->GetEntryData( nPos );
    const sal_uInt16 nOld = m_pPaperSizeBox->GetSavedValue();
    sal_Bool bChecked = m_pLandscapeBtn->IsChecked();

    if ( PAPER_USER == ePaper )
    {
        if ( nOld != nPos                       ||
             m_pPaperWidthEdit->IsValueModified()  ||
             m_pPaperHeightEdit->IsValueModified() ||
             bChecked != m_pLandscapeBtn->GetSavedValue() )
        {
            Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                        GetCoreValue( *m_pPaperHeightEdit, eUnit ) );
            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= sal_True;
            }
        }
    }
    else
    {
        if ( nOld != nPos || bChecked != m_pLandscapeBtn->GetSavedValue() )
        {
            Size aSize( SvxPaperInfo::GetPaperSize( ePaper, (MapUnit)eUnit ) );

            if ( bChecked )
                Swap( aSize );

            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= sal_True;
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE );
    SvxPageItem aPage( (const SvxPageItem&)rOldSet.Get( nWhich ) );
    bMod =  m_pLayoutBox->GetSelectEntryPos()  != m_pLayoutBox->GetSavedValue();

    if ( bMod )
        aPage.SetPageUsage(
            ::PosToPageUsage_Impl( m_pLayoutBox->GetSelectEntryPos() ) );

    if ( bChecked != m_pLandscapeBtn->GetSavedValue() )
    {
        aPage.SetLandscape(bChecked);
        bMod |= sal_True;
    }

    //Get the NumType value
    nPos = m_pNumberFormatBox->GetSelectEntryPos();
    sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)m_pNumberFormatBox->GetEntryData(nPos);
    if ( nPos != m_pNumberFormatBox->GetSavedValue() )
    {
        aPage.SetNumType( (SvxNumType)nEntryData );
        bMod |= sal_True;
    }

    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_PAGE );

        if ( !pOld || !( *(const SvxPageItem*)pOld == aPage ) )
        {
            rSet.Put( aPage );
            bModified |= sal_True;
        }
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich ) )
        rSet.ClearItem( nWhich );
    else
        rSet.Put( rOldSet.Get( nWhich ) );

    // evaluate mode specific controls

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            if ( m_pHorzBox->IsChecked() != m_pHorzBox->GetSavedValue() )
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT1 ),
                                   m_pHorzBox->IsChecked() );
                rSet.Put( aHorz );
                bModified |= sal_True;
            }

            if ( m_pVertBox->IsChecked() != m_pVertBox->GetSavedValue() )
            {
                SfxBoolItem aVert( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   m_pVertBox->IsChecked() );
                rSet.Put( aVert );
                bModified |= sal_True;
            }
            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            // always put so that draw can evaluate this
            rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PAGE_EXT1 ),
                      m_pAdaptBox->IsChecked() ) );
            bModified |= sal_True;
            break;
        }
        default: ;//prevent warning

    }

    if(m_pRegisterCB->IsVisible() &&
        (m_pRegisterCB->IsChecked() || m_pRegisterCB->GetSavedValue() != m_pRegisterCB->IsChecked()))
    {
        const SfxBoolItem& rRegItem = (const SfxBoolItem&)rOldSet.Get(SID_SWREGISTER_MODE);
        SfxBoolItem* pRegItem = (SfxBoolItem*)rRegItem.Clone();
        sal_Bool bCheck = m_pRegisterCB->IsChecked();
        pRegItem->SetValue(bCheck);
        rSet.Put(*pRegItem);
        bModified |= sal_True;
        if(bCheck)
        {
            bModified |= sal_True;
            rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION,
                            m_pRegisterLB->GetSelectEntry()));
        }
        delete pRegItem;
    }

    SvxFrameDirection eDirection = m_pTextFlowBox->GetSelectEntryValue();
    if( m_pTextFlowBox->IsVisible() && (eDirection != m_pTextFlowBox->GetSavedValue()) )
    {
        rSet.Put( SvxFrameDirectionItem( eDirection, GetWhich( SID_ATTR_FRAMEDIRECTION ) ) );
        bModified = sal_True;
    }

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPageDescPage, LayoutHdl_Impl)
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
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPageDescPage, PaperBinHdl_Impl)
{
    if ( m_pPaperTrayBox->GetEntryCount() > 1 )
        // already filled
        return 0;

    String aOldName = m_pPaperTrayBox->GetSelectEntry();
    m_pPaperTrayBox->SetUpdateMode( sal_False );
    m_pPaperTrayBox->Clear();
    sal_uInt16 nEntryPos = m_pPaperTrayBox->InsertEntry(
        EE_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS ) );
    m_pPaperTrayBox->SetEntryData( nEntryPos,
        (void*)(sal_uLong)PAPERBIN_PRINTER_SETTINGS );
    String aPaperBin( EditResId( RID_SVXSTR_PAPERBIN ) );
    sal_uInt16 nBinCount = pImpl->mpDefPrinter->GetPaperBinCount();

    for ( sal_uInt16 i = 0; i < nBinCount; ++i )
    {
        String aName = pImpl->mpDefPrinter->GetPaperBinName(i);

        if ( !aName.Len() )
        {
            aName = aPaperBin;
            aName.Append( sal_Unicode(' ') );
            aName.Append( OUString::number( i+1 ) );
        }
        nEntryPos = m_pPaperTrayBox->InsertEntry( aName );
        m_pPaperTrayBox->SetEntryData( nEntryPos, (void*)(sal_uLong)i );
    }
    m_pPaperTrayBox->SelectEntry( aOldName );
    m_pPaperTrayBox->SetUpdateMode( sal_True );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperSizeSelect_Impl, ListBox *, pBox )
{
    const sal_uInt16 nPos = pBox->GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)m_pPaperSizeBox->GetEntryData( nPos );

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

        RangeHdl_Impl( 0 );
        UpdateExample_Impl( true );

        if ( eMode == SVX_PAGE_MODE_PRESENTATION )
        {
            // Draw: if paper format the margin shall be 1 cm
            long nTmp = 0;
            sal_Bool bScreen = (( PAPER_SCREEN_4_3 == ePaper )||( PAPER_SCREEN_16_9 == ePaper)||( PAPER_SCREEN_16_10 == ePaper));

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
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPageDescPage, PaperSizeModify_Impl)
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                GetCoreValue( *m_pPaperHeightEdit, eUnit ) );
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, (MapUnit)eUnit, sal_True );
    sal_uInt16 nEntryCount = m_pPaperSizeBox->GetEntryCount();

    for ( sal_uInt16 i = 0; i < nEntryCount; ++i )
    {
        Paper eTmp = (Paper)(sal_uLong)m_pPaperSizeBox->GetEntryData(i);

        if ( eTmp == ePaper )
        {
            m_pPaperSizeBox->SelectEntryPos(i);
            break;
        }
    }
    UpdateExample_Impl( true );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, SwapOrientation_Impl, RadioButton *, pBtn )
{
    if (
        (!bLandscape && pBtn == m_pLandscapeBtn) ||
        (bLandscape  && pBtn == m_pPortraitBtn)
       )
    {
        bLandscape = m_pLandscapeBtn->IsChecked();

        const long lWidth = GetCoreValue( *m_pPaperWidthEdit, SFX_MAPUNIT_100TH_MM );
        const long lHeight = GetCoreValue( *m_pPaperHeightEdit, SFX_MAPUNIT_100TH_MM );

        // swap with and height
        SetMetricValue( *m_pPaperWidthEdit, lHeight, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( *m_pPaperHeightEdit, lWidth, SFX_MAPUNIT_100TH_MM );

        // recalculate margins if necessary
        CalcMargin_Impl();

        PaperSizeSelect_Impl( m_pPaperSizeBox );
        RangeHdl_Impl( 0 );
        SwapFirstValues_Impl( bBorderModified );
        UpdateExample_Impl( true );
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::SwapFirstValues_Impl( bool bSet )
{
    MapMode aOldMode = pImpl->mpDefPrinter->GetMapMode();
    Orientation eOri = ORIENTATION_PORTRAIT;

    if ( bLandscape )
        eOri = ORIENTATION_LANDSCAPE;
    Orientation eOldOri = pImpl->mpDefPrinter->GetOrientation();
    pImpl->mpDefPrinter->SetOrientation( eOri );
    pImpl->mpDefPrinter->SetMapMode( MAP_TWIP );

    // set first- and last-values for margins
    Size aPaperSize = pImpl->mpDefPrinter->GetPaperSize();
    Size aPrintSize = pImpl->mpDefPrinter->GetOutputSize();
    /*
     * To convert a point ( 0,0 ) into logic coordinates
     * looks like nonsense; but it makes sense if the
     * coordinate system's origin has been moved.
     */
    Point aPrintOffset = pImpl->mpDefPrinter->GetPageOffset() -
                         pImpl->mpDefPrinter->PixelToLogic( Point() );
    pImpl->mpDefPrinter->SetMapMode( aOldMode );
    pImpl->mpDefPrinter->SetOrientation( eOldOri );

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

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxPageDescPage, BorderModify_Impl)
{
    if ( !bBorderModified )
        bBorderModified = sal_True;
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxPageDescPage, BorderModify_Impl)

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SvxPageDescPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetFtBorder( rItem );
            }
        }
    }

    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_BRUSH );

    if ( pItem )
    {
        m_pBspWin->SetColor( ( (const SvxBrushItem*)pItem )->GetColor() );
        const Graphic* pGrf = ( (const SvxBrushItem*)pItem )->GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            m_pBspWin->SetBitmap( &aBitmap );
        }
        else
            m_pBspWin->SetBitmap( NULL );
    }

    pItem = GetItem( rSet, SID_ATTR_BORDER_OUTER );

    if ( pItem )
        m_pBspWin->SetBorder( (SvxBoxItem&)*pItem );
}

// -----------------------------------------------------------------------

void SvxPageDescPage::InitHeadFoot_Impl( const SfxItemSet& rSet )
{
    bLandscape = m_pLandscapeBtn->IsChecked();
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        m_pBspWin->SetSize( ( (const SvxSizeItem*)pItem )->GetSize() );

    const SvxSetItem* pSetItem = 0;

    // evaluate header attributes

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                            sal_False, (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            long nDist = rUL.GetLower();
            m_pBspWin->SetHdHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetHdDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            m_pBspWin->SetHdLeft( rLR.GetLeft() );
            m_pBspWin->SetHdRight( rLR.GetRight() );
            m_pBspWin->SetHeader( sal_True );
        }
        else
            m_pBspWin->SetHeader( sal_False );

        // show background and border in the example
        sal_uInt16 nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rHeaderSet.Get( nWhich );
            m_pBspWin->SetHdColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rHeaderSet.Get( nWhich );
            m_pBspWin->SetHdBorder( rItem );
        }
    }

    // evaluate footer attributes

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                            sal_False, (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            long nDist = rUL.GetUpper();
            m_pBspWin->SetFtHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetFtDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            m_pBspWin->SetFtLeft( rLR.GetLeft() );
            m_pBspWin->SetFtRight( rLR.GetRight() );
            m_pBspWin->SetFooter( sal_True );
        }
        else
            m_pBspWin->SetFooter( sal_False );

        // show background and border in the example
        sal_uInt16 nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rFooterSet.Get( nWhich );
            m_pBspWin->SetFtColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rFooterSet.Get( nWhich );
            m_pBspWin->SetFtBorder( rItem );
        }
    }
}

// -----------------------------------------------------------------------

void SvxPageDescPage::ActivatePage( const SfxItemSet& rSet )
{
    InitHeadFoot_Impl( rSet );
    ResetBackground_Impl( rSet );
    RangeHdl_Impl( 0 );
}

// -----------------------------------------------------------------------

int SvxPageDescPage::DeactivatePage( SfxItemSet* _pSet )
{
    // Inquiry whether the page margins are beyond the printing area.
    // If not, ask user whether they shall be taken.
    // If not, stay on the TabPage.
    sal_uInt16 nPos = m_pPaperSizeBox->GetSelectEntryPos();
    Paper ePaper = (Paper)(sal_uLong)m_pPaperSizeBox->GetEntryData( nPos );

    if ( ePaper != PAPER_SCREEN_4_3 && ePaper != PAPER_SCREEN_16_9 && ePaper != PAPER_SCREEN_16_10 && IsMarginOutOfRange() )
    {
        if ( QueryBox( this, WB_YES_NO | WB_DEF_NO, m_pPrintRangeQueryText->GetText() ).Execute() == RET_NO )
        {
            MetricField* pField = NULL;
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
            return KEEP_PAGE;
        }
        else
            CheckMarginEdits( false );
    }

    if ( _pSet )
    {
        FillItemSet( *_pSet );

        // put portray/landscape if applicable
        sal_uInt16 nWh = GetWhich( SID_ATTR_PAGE_SIZE );
        SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWh );
        Size aSize( GetCoreValue( *m_pPaperWidthEdit, eUnit ),
                    GetCoreValue( *m_pPaperHeightEdit, eUnit ) );

        // put, if current size is different to the value in _pSet
        const SvxSizeItem* pSize = (const SvxSizeItem*)GetItem( *_pSet, SID_ATTR_PAGE_SIZE );
        if ( aSize.Width() && ( !pSize || !IsEqualSize_Impl( pSize, aSize ) ) )
            _pSet->Put( SvxSizeItem( nWh, aSize ) );
    }

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPageDescPage, RangeHdl_Impl)
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
            SFX_ITEM_AVAILABLE &&
         _pSet->GetItemState( GetWhich(SID_ATTR_BORDER_OUTER)  ) >=
            SFX_ITEM_AVAILABLE )
    {
        aBorder = ( GetMinBorderSpace_Impl(
            (const SvxShadowItem&)_pSet->Get(GetWhich(SID_ATTR_BORDER_SHADOW)),
            (const SvxBoxItem&)_pSet->Get(GetWhich(SID_ATTR_BORDER_OUTER))));
    }

    long nH  = static_cast<long>(m_pPaperHeightEdit->Denormalize(m_pPaperHeightEdit->GetValue(FUNIT_TWIP)));
    long nW  = static_cast<long>(m_pPaperWidthEdit->Denormalize(m_pPaperWidthEdit->GetValue(FUNIT_TWIP)));

    // limits paper
    // maximum is 54 cm
    //
    long nMin = nHHeight + nHDist + nFDist + nFHeight + nBT + nBB +
                MINBODY + aBorder.Height();
    m_pPaperHeightEdit->SetMin(m_pPaperHeightEdit->Normalize(nMin), FUNIT_TWIP);

    nMin = MINBODY + nBL + nBR + aBorder.Width();
    m_pPaperWidthEdit->SetMin(m_pPaperWidthEdit->Normalize(nMin), FUNIT_TWIP);

    // if the paper size has been changed
    nH = static_cast<long>(m_pPaperHeightEdit->Denormalize(m_pPaperHeightEdit->GetValue(FUNIT_TWIP)));
    nW = static_cast<long>(m_pPaperWidthEdit->Denormalize(m_pPaperWidthEdit->GetValue(FUNIT_TWIP)));

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
    return 0;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxPageDescPage, CenterHdl_Impl)
{
    m_pBspWin->SetHorz( m_pHorzBox->IsChecked() );
    m_pBspWin->SetVert( m_pVertBox->IsChecked() );
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxPageDescPage, CenterHdl_Impl)

// -----------------------------------------------------------------------

void SvxPageDescPage::SetCollectionList(const std::vector<OUString> &aList)
{
    OSL_ENSURE(!aList.empty(), "Empty string list");

    sStandardRegister = aList[0];
    for( sal_uInt16 i = 1; i < aList.size(); i++   )
        m_pRegisterLB->InsertEntry(aList[i]);

    m_pRegisterCB->Show();
    m_pRegisterFT->Show();
    m_pRegisterLB->Show();
    m_pRegisterCB->SetClickHdl(LINK(this, SvxPageDescPage, RegisterModify));
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, RegisterModify, CheckBox*, pBox )
{
    sal_Bool bEnable = sal_False;
    if(pBox->IsChecked())
    {
        bEnable = sal_True;
        if(USHRT_MAX == m_pRegisterLB->GetSelectEntryPos())
            m_pRegisterLB->SelectEntry(sStandardRegister);
    }
    m_pRegisterFT->Enable( bEnable );
    m_pRegisterLB->Enable( bEnable );
    return 0;
}

// ----------------------------------------------------------------------------

void SvxPageDescPage::DisableVerticalPageDir()
{
    m_pTextFlowBox->RemoveEntryValue( FRMDIR_VERT_TOP_RIGHT );
    m_pTextFlowBox->RemoveEntryValue( FRMDIR_VERT_TOP_LEFT );
    if( m_pTextFlowBox->GetEntryCount() < 2 )
    {
        m_pTextFlowLbl->Hide();
        m_pTextFlowBox->Hide();
        m_pBspWin->EnableFrameDirection( sal_False );
    }
}

IMPL_LINK( SvxPageDescPage, FrameDirectionModify_Impl, ListBox*,  EMPTYARG)
{
    m_pBspWin->SetFrameDirection( (sal_uInt32) m_pTextFlowBox->GetSelectEntryValue() );
    m_pBspWin->Invalidate();
    return 0;
}

bool SvxPageDescPage::IsPrinterRangeOverflow(
    MetricField& rField, long nFirstMargin, long nLastMargin, MarginPosition nPos )
{
    bool bRet = false;
    bool bCheck = ( ( pImpl->m_nPos & nPos ) == 0 );
    long nValue = static_cast<long>(rField.GetValue());
    if ( bCheck &&
         (  nValue < nFirstMargin || nValue > nLastMargin ) &&
         rField.GetText() != rField.GetSavedValue() )
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
        pImpl->m_nPos = 0;

    sal_Int64 nValue = m_pLeftMarginEdit->GetValue();
    if (  nValue < nFirstLeftMargin || nValue > nLastLeftMargin )
        pImpl->m_nPos |= MARGIN_LEFT;
    nValue = m_pRightMarginEdit->GetValue();
    if (  nValue < nFirstRightMargin || nValue > nLastRightMargin )
        pImpl->m_nPos |= MARGIN_RIGHT;
    nValue = m_pTopMarginEdit->GetValue();
    if (  nValue < nFirstTopMargin || nValue > nLastTopMargin )
        pImpl->m_nPos |= MARGIN_TOP;
    nValue = m_pBottomMarginEdit->GetValue();
    if (  nValue < nFirstBottomMargin || nValue > nLastBottomMargin )
        pImpl->m_nPos |= MARGIN_BOTTOM;
}

bool SvxPageDescPage::IsMarginOutOfRange()
{
    bool bRet = ( ( ( !( pImpl->m_nPos & MARGIN_LEFT ) &&
                      ( m_pLeftMarginEdit->GetText() != m_pLeftMarginEdit->GetSavedValue() ) ) &&
                    ( m_pLeftMarginEdit->GetValue() < nFirstLeftMargin ||
                      m_pLeftMarginEdit->GetValue() > nLastLeftMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_RIGHT ) &&
                      ( m_pRightMarginEdit->GetText() != m_pRightMarginEdit->GetSavedValue() ) ) &&
                    ( m_pRightMarginEdit->GetValue() < nFirstRightMargin ||
                      m_pRightMarginEdit->GetValue() > nLastRightMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_TOP ) &&
                      ( m_pTopMarginEdit->GetText() != m_pTopMarginEdit->GetSavedValue() ) ) &&
                    ( m_pTopMarginEdit->GetValue() < nFirstTopMargin ||
                      m_pTopMarginEdit->GetValue() > nLastTopMargin ) ) ||
                  ( ( !( pImpl->m_nPos & MARGIN_BOTTOM ) &&
                      ( m_pBottomMarginEdit->GetText() != m_pBottomMarginEdit->GetSavedValue() ) ) &&
                    ( m_pBottomMarginEdit->GetValue() < nFirstBottomMargin ||
                      m_pBottomMarginEdit->GetValue() > nLastBottomMargin ) ) );
    return bRet;
}

void SvxPageDescPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pModeItem,SfxAllEnumItem,SID_ENUM_PAGE_MODE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPaperStartItem,SfxAllEnumItem,SID_PAPER_START,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPaperEndItem,SfxAllEnumItem,SID_PAPER_END,sal_False);
    SFX_ITEMSET_ARG (&aSet,pCollectListItem,SfxStringListItem,SID_COLLECT_LIST,sal_False);
    if (pModeItem)
        SetMode((SvxModeType)pModeItem->GetEnumValue());
    if (pPaperStartItem && pPaperEndItem)
        SetPaperFormatRanges( (Paper)pPaperStartItem->GetEnumValue(), (Paper)pPaperEndItem->GetEnumValue() );
    if (pCollectListItem)
        SetCollectionList(pCollectListItem->GetList());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
