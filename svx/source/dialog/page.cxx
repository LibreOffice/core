/*************************************************************************
 *
 *  $RCSfile: page.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ma $ $Date: 2001-03-27 12:44:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#define ITEMID_BRUSH        0
#define ITEMID_BOX          0
#define ITEMID_BOXINFO      0
#define ITEMID_SHADOW       0
#define ITEMID_PAPERBIN     0
#define ITEMID_LRSPACE      0
#define ITEMID_ULSPACE      0
#define ITEMID_SIZE         0

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _RESARY_HXX //autogen
#include <vcl/resary.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#define _SVX_PAGE_CXX

#include "dialogs.hrc"
#include "page.hrc"
#include "helpid.hrc"

#define ITEMID_PAGE             0
#define ITEMID_SETITEM          0
#define ITEMID_PTR              0

#include "page.hxx"
#include "pageitem.hxx"
#include "brshitem.hxx"
#include "boxitem.hxx"
#include "shaditem.hxx"
#include "pbinitem.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "sizeitem.hxx"
#include "bbdlg.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "paperinf.hxx"
#include "dialmgr.hxx"

#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif


#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

// static ----------------------------------------------------------------

static const long MINBODY       = 284;  // 0,5cm in twips aufgerundet
static const long PRINT_OFFSET  = 17;   // 0,03cm in twips abgerundet

static USHORT pRanges[] =
{
    SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,
    SID_ATTR_PAGE_SHARED,
    SID_SWREGISTER_COLLECTION,
    SID_SWREGISTER_MODE,
    0
};

// ------- Mapping Seitenlayout ------------------------------------------

USHORT aArr[] =
{
    SVX_PAGE_ALL,
    SVX_PAGE_MIRROR,
    SVX_PAGE_RIGHT,
    SVX_PAGE_LEFT
};

// -----------------------------------------------------------------------

USHORT PageUsageToPos_Impl( USHORT nUsage )
{
    const USHORT nCount = sizeof(aArr) / sizeof(USHORT);

    for ( USHORT i = 0; i < nCount; ++i )
        if ( aArr[i] == ( nUsage & 0x000f ) )
            return i;
    return SVX_PAGE_ALL;
}

// -----------------------------------------------------------------------

USHORT PosToPageUsage_Impl( USHORT nPos )
{
    const USHORT nCount = sizeof(aArr) / sizeof(USHORT);

    if ( nPos >= nCount )
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

BOOL IsEqualSize_Impl( const SvxSizeItem* pSize, const Size& rSize )
{
    if ( pSize )
    {
        Size aSize = pSize->GetSize();
        long nDiffW = Abs( rSize.Width () - aSize.Width () );
        long nDiffH = Abs( rSize.Height() - aSize.Height() );
        return ( nDiffW < 10 && nDiffH < 10 );
    }
    else
        return FALSE;
}

// class SvxPageDescPage --------------------------------------------------

// gibt den Bereich der Which-Werte zur"uck

USHORT* SvxPageDescPage::GetRanges()
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

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_PAGE ), rAttr ),

    aLeftMarginLbl      ( this, ResId( FT_LEFT_MARGIN ) ),
    aLeftMarginEdit     ( this, ResId( ED_LEFT_MARGIN ) ),
    aRightMarginLbl     ( this, ResId( FT_RIGHT_MARGIN ) ),
    aRightMarginEdit    ( this, ResId( ED_RIGHT_MARGIN ) ),
    aTopMarginLbl       ( this, ResId( FT_TOP_MARGIN ) ),
    aTopMarginEdit      ( this, ResId( ED_TOP_MARGIN ) ),
    aBottomMarginLbl    ( this, ResId( FT_BOTTOM_MARGIN ) ),
    aBottomMarginEdit   ( this, ResId( ED_BOTTOM_MARGIN ) ),
    aMarginFl           ( this, ResId( FL_MARGIN ) ),
    aLayoutBox          ( this, ResId( LB_LAYOUT ) ),
    aPageText           ( this, ResId( FT_PAGELAYOUT ) ),
    aNumberFormatBox    ( this, ResId( LB_NUMBER_FORMAT ) ),
    aNumberFormatText   ( this, ResId( FT_NUMBER_FORMAT ) ),
    aNumberFormatFl     ( this, ResId( FL_NUMBER_FORMAT ) ),
    aBspWin             ( this, ResId( WN_BSP ) ),
    aBspFl              ( this, ResId( FL_BSP ) ),
    aPaperFormatText    ( this, ResId( FT_PAPER_FORMAT ) ),
    aPaperSizeBox       ( this, ResId( LB_PAPER_SIZE ) ),
    aPortraitBtn        ( this, ResId( RB_PORTRAIT ) ),
    aLandscapeBtn       ( this, ResId( RB_LANDSCAPE ) ),
    aPaperWidthText     ( this, ResId( FT_PAPER_WIDTH ) ),
    aPaperWidthEdit     ( this, ResId( ED_PAPER_WIDTH ) ),
    aPaperHeightText    ( this, ResId( FT_PAPER_HEIGHT ) ),
    aPaperHeightEdit    ( this, ResId( ED_PAPER_HEIGHT ) ),
    aPaperTrayLbl       ( this, ResId( FT_PAPER_TRAY ) ),
    aPaperTrayBox       ( this, ResId( LB_PAPER_TRAY ) ),
    aPaperSizeFl        ( this, ResId( FL_PAPER_SIZE ) ),
    aHorzBox            ( this, ResId( CB_HORZ ) ),
    aVertBox            ( this, ResId( CB_VERT ) ),
    aAdaptBox           ( this, ResId( CB_ADAPT ) ),
    aPageName           ( this, ResId( FT_PAGE_NAME ) ),
    aExtraFl            ( this, ResId( FL_EXTRA ) ),

    aRegisterCB         ( this, ResId( CB_REGISTER ) ),
    aRegisterFT         ( this, ResId( FT_REGISTER ) ),
    aRegisterLB         ( this, ResId( LB_REGISTER ) ),
    aRegisterFl         ( this, ResId( FL_REGISTER ) ),

    aInsideText         (       ResId( STR_INSIDE ) ),
    aOutsideText        (       ResId( STR_OUTSIDE ) ),
    aPrintRangeQueryText(       ResId( STR_QUERY_PRINTRANGE ) ),

    bLandscape      ( FALSE ),
    eMode           ( SVX_PAGE_MODE_STANDARD ),
    ePaperStart     ( SVX_PAPER_A3 ),
    ePaperEnd       ( SVX_PAPER_DL )

{
    bBorderModified = FALSE;

    FreeResource();
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    Init_Impl();

    FieldUnit eFUnit = GetModuleFieldUnit( &rAttr );
    SetFieldUnit( aLeftMarginEdit, eFUnit );
    SetFieldUnit( aRightMarginEdit, eFUnit );
    SetFieldUnit( aTopMarginEdit, eFUnit );
    SetFieldUnit( aBottomMarginEdit, eFUnit );
    SetFieldUnit( aPaperWidthEdit, eFUnit );
    SetFieldUnit( aPaperHeightEdit, eFUnit );

    Printer* pDefPrinter = 0;
    BOOL bPrinterDel = FALSE;

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
        pDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pDefPrinter = new Printer;
        bPrinterDel = TRUE;
    }

    MapMode aOldMode = pDefPrinter->GetMapMode();
    pDefPrinter->SetMapMode( MAP_TWIP );

    // First- und Last-Werte f"ur die R"ander setzen
    Size aPaperSize = pDefPrinter->GetPaperSize();
    Size aPrintSize = pDefPrinter->GetOutputSize();
    /*
     * einen Punkt ( 0,0 ) in logische Koordinaten zu konvertieren,
     * sieht aus wie Unsinn; ist aber sinnvoll, wenn der Ursprung des
     * Koordinatensystems verschoben ist.
     */
    Point aPrintOffset = pDefPrinter->GetPageOffset() -
                         pDefPrinter->PixelToLogic( Point() );
    pDefPrinter->SetMapMode( aOldMode );

    if ( bPrinterDel )
        delete pDefPrinter;
    long nOffset = !aPrintOffset.X() && !aPrintOffset.X() ? 0 : PRINT_OFFSET;
    aLeftMarginEdit.SetFirst( aLeftMarginEdit.Normalize( aPrintOffset.X() ), FUNIT_TWIP );
    nFirstLeftMargin = aLeftMarginEdit.GetFirst();
    aRightMarginEdit.SetFirst( aRightMarginEdit.Normalize(
        aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset ), FUNIT_TWIP);
    nFirstRightMargin = aRightMarginEdit.GetFirst();
    aTopMarginEdit.SetFirst( aTopMarginEdit.Normalize( aPrintOffset.Y() ), FUNIT_TWIP );
    nFirstTopMargin = aTopMarginEdit.GetFirst();
    aBottomMarginEdit.SetFirst( aBottomMarginEdit.Normalize(
        aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset ), FUNIT_TWIP );
    nFirstBottomMargin = aBottomMarginEdit.GetFirst();
    aLeftMarginEdit.SetLast( aLeftMarginEdit.Normalize(
        aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastLeftMargin = aLeftMarginEdit.GetLast();
    aRightMarginEdit.SetLast( aRightMarginEdit.Normalize(
        aPrintOffset.X() + aPrintSize.Width() ), FUNIT_TWIP );
    nLastRightMargin = aRightMarginEdit.GetLast();
    aTopMarginEdit.SetLast( aTopMarginEdit.Normalize(
        aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastTopMargin = aTopMarginEdit.GetLast();
    aBottomMarginEdit.SetLast( aBottomMarginEdit.Normalize(
        aPrintOffset.Y() + aPrintSize.Height() ), FUNIT_TWIP );
    nLastBottomMargin = aBottomMarginEdit.GetLast();
}

// -----------------------------------------------------------------------

SvxPageDescPage::~SvxPageDescPage()
{
}

// -----------------------------------------------------------------------

void SvxPageDescPage::Init_Impl()
{
    aLeftText = aLeftMarginLbl.GetText();
    aRightText = aRightMarginLbl.GetText();

        // Handler einstellen
    aLayoutBox.SetSelectHdl( LINK( this, SvxPageDescPage, LayoutHdl_Impl ) );

    aPaperTrayBox.SetGetFocusHdl(
        LINK( this, SvxPageDescPage, PaperBinHdl_Impl ) );
    aPaperSizeBox.SetSelectHdl(
        LINK( this, SvxPageDescPage, PaperSizeSelect_Impl ) );
    aPaperWidthEdit.SetModifyHdl(
        LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    aPaperHeightEdit.SetModifyHdl(
        LINK( this, SvxPageDescPage, PaperSizeModify_Impl ) );
    aLandscapeBtn.SetClickHdl(
        LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );
    aPortraitBtn.SetClickHdl(
        LINK( this, SvxPageDescPage, SwapOrientation_Impl ) );

    Link aLink = LINK( this, SvxPageDescPage, BorderModify_Impl );
    aLeftMarginEdit.SetModifyHdl( aLink );
    aRightMarginEdit.SetModifyHdl( aLink );
    aTopMarginEdit.SetModifyHdl( aLink );
    aBottomMarginEdit.SetModifyHdl( aLink );

    aLink = LINK( this, SvxPageDescPage, RangeHdl_Impl );
    aPaperWidthEdit.SetLoseFocusHdl( aLink );
    aPaperHeightEdit.SetLoseFocusHdl( aLink );
    aLeftMarginEdit.SetLoseFocusHdl( aLink );
    aRightMarginEdit.SetLoseFocusHdl( aLink );
    aTopMarginEdit.SetLoseFocusHdl( aLink );
    aBottomMarginEdit.SetLoseFocusHdl( aLink );

    aHorzBox.SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );
    aVertBox.SetClickHdl( LINK( this, SvxPageDescPage, CenterHdl_Impl ) );
}

// -----------------------------------------------------------------------

void SvxPageDescPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) );

    // R"ander (Links/Rechts) einstellen
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;
        SetMetricValue( aLeftMarginEdit, rLRSpace.GetLeft(), eUnit );
        aBspWin.SetLeft(
            (USHORT)ConvertLong_Impl( (long)rLRSpace.GetLeft(), eUnit ) );
        SetMetricValue( aRightMarginEdit, rLRSpace.GetRight(), eUnit );
        aBspWin.SetRight(
            (USHORT)ConvertLong_Impl( (long)rLRSpace.GetRight(), eUnit ) );
    }

    // R"ander (Oben/Unten) einstellen
    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;
        SetMetricValue( aTopMarginEdit, rULSpace.GetUpper(), eUnit );
        aBspWin.SetTop(
            (USHORT)ConvertLong_Impl( (long)rULSpace.GetUpper(), eUnit ) );
        SetMetricValue( aBottomMarginEdit, rULSpace.GetLower(), eUnit );
        aBspWin.SetBottom(
            (USHORT)ConvertLong_Impl( (long)rULSpace.GetLower(), eUnit ) );
    }

    // Printer f"ur die Sch"achte besorgen
    Printer* pDefPrinter = 0;
    BOOL bPrinterDel = FALSE;

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
        pDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pDefPrinter = new Printer;
        bPrinterDel = TRUE;
    }

    // allgemeine Seitendaten
    SvxNumType eNumType = SVX_ARABIC;
    bLandscape = ( pDefPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
    USHORT nUse = (USHORT)SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
    {
        const SvxPageItem& rItem = (const SvxPageItem&)*pItem;
        eNumType = rItem.GetNumType();
        nUse = (SvxPageUsage)rItem.GetPageUsage();
        bLandscape = rItem.IsLandscape();
    }

    // Ausrichtung
    aLayoutBox.SelectEntryPos( ::PageUsageToPos_Impl( nUse ) );
    aBspWin.SetUsage( nUse );
    LayoutHdl_Impl( 0 );

    // Numerierungsart der Seitenvorlage einstellen
    aNumberFormatBox.SelectEntryPos( eNumType );

    // Aktueller Papierschacht
    aPaperTrayBox.Clear();
    BYTE nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    pItem = GetItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( pItem )
    {
        nPaperBin = ( (const SvxPaperBinItem*)pItem )->GetValue();

        if ( nPaperBin >= pDefPrinter->GetPaperBinCount() )
            nPaperBin = PAPERBIN_PRINTER_SETTINGS;
    }

    String aBinName;

    if ( PAPERBIN_PRINTER_SETTINGS  == nPaperBin )
        aBinName = SVX_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS );
    else
        aBinName = pDefPrinter->GetPaperBinName( (USHORT)nPaperBin );

    USHORT nEntryPos = aPaperTrayBox.InsertEntry( aBinName );
    aPaperTrayBox.SetEntryData( nEntryPos, (void*)(ULONG)nPaperBin );
    aPaperTrayBox.SelectEntry( aBinName );

    // Size rausholen
    Size aPaperSize = SvxPaperInfo::GetPaperSize( pDefPrinter );
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aPaperSize = ( (const SvxSizeItem*)pItem )->GetSize();

    FASTBOOL bOrientationSupport =
        pDefPrinter->HasSupport( SUPPORT_SET_ORIENTATION );
#ifdef OS2
    // unter OS/2 wird bei HasSupport() immer TRUE returned
    // aber nur als Dummy, deshalb FALSE
    bOrientationSupport = FALSE;
#endif

    if ( !bOrientationSupport &&
         aPaperSize.Width() > aPaperSize.Height() )
        bLandscape = TRUE;

    aLandscapeBtn.Check( bLandscape );
    aPortraitBtn.Check( !bLandscape );

    aBspWin.SetSize( Size( ConvertLong_Impl( aPaperSize.Width(), eUnit ),
                           ConvertLong_Impl( aPaperSize.Height(), eUnit ) ) );

    // Werte in die Edits eintragen
    SetMetricValue( aPaperHeightEdit, aPaperSize.Height(), eUnit );
    SetMetricValue( aPaperWidthEdit, aPaperSize.Width(), eUnit );
    aPaperSizeBox.Clear();

    // Papierformate
    Size aTmpSize = aPaperSize;

    if ( bLandscape )
        Swap( aTmpSize );
    // aktuelles Format
    SvxPaper ePaper = SvxPaperInfo::GetPaper( aTmpSize, (MapUnit)eUnit, TRUE );
    USHORT nActPos = LISTBOX_ENTRY_NOTFOUND;
    USHORT nAryId = RID_SVXSTRARY_PAPERSIZE_STD;

    if ( ePaperStart != SVX_PAPER_A3 )
        nAryId = RID_SVXSTRARY_PAPERSIZE_DRAW;
    ResStringArray aPaperAry( SVX_RES( nAryId ) );
    USHORT nCnt = aPaperAry.Count();

    for ( USHORT i = 0; i < nCnt; ++i )
    {
        String aStr = aPaperAry.GetString(i);
        SvxPaper eSize = (SvxPaper)aPaperAry.GetValue(i);
        USHORT nPos = aPaperSizeBox.InsertEntry( aStr );
        aPaperSizeBox.SetEntryData( nPos, (void*)(ULONG)eSize );

        if ( eSize == ePaper )
            nActPos = nPos;
    }
    // aktuelles Papierformat selektieren
    aPaperSizeBox.SelectEntryPos( nActPos );

    // ggf. angelegten Printer wieder l"oschen
    if ( bPrinterDel )
        delete pDefPrinter;

    // Applikationsspezifisch

    USHORT nResId = 0;

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            nResId = RID_SVXSTR_CALC_PAGE;
            aHorzBox.Show();
            aVertBox.Show();

            // Horizontale Ausrichtung
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
            aHorzBox.Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : FALSE );

            // Vertikale Ausrichtung
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );
            aVertBox.Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue()
                                  : FALSE );

            // Beispiel-Fenster auf Tabelle setzen
            aBspWin.SetTable( TRUE );
            aBspWin.SetHorz( aHorzBox.IsChecked() );
            aBspWin.SetVert( aVertBox.IsChecked() );

            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            nResId = RID_SVXSTR_DRAW_PAGE;
            aHorzBox.SetText( String( SVX_RESSTR( RID_SVXSTR_FULLSIZE ) ) );
            aHorzBox.Show(); // f"ur Gesamte Seite ausf"ullen (Hintergr.Obj.)
            aHorzBox.SetHelpId( HID_TPPAGE_FULLSIZE );
            aAdaptBox.Show();
            String sUser = GetUserData();

            if ( sUser.Len() )
                aAdaptBox.Check( (BOOL)sUser.ToInt32() );
            else
            {
                pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
                aAdaptBox.Check( pItem ?
                    ( (const SfxBoolItem*)pItem )->GetValue() : FALSE );
            }
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );
            aHorzBox.Check( pItem ? ( (const SfxBoolItem*)pItem )->GetValue() : FALSE );

            //!!! hidden, weil von StarDraw nicht implementiert
            aLayoutBox.Hide();
            aPageText.Hide();

            break;
        }

        default:
        {
            nResId = RID_SVXSTR_WRITER_PAGE;
            if(!aRegisterCB.IsVisible())
                aPageName.Show();
            pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

            if ( pItem )
                aPageName.SetText(((const SfxStringItem*)pItem)->GetValue());
            else
                aPageName.SetText( String() );
        }
    }

    if ( nResId )
        aExtraFl.SetText( SVX_RESSTR( nResId ) );

    // im Beispiel Hintergrund und Umrandung anzeigen
    ResetBackground_Impl( rSet );
//! UpdateExample_Impl();
    RangeHdl_Impl( 0 );

    // Header Footer anzeigen
    InitHeadFoot_Impl( rSet );

    // R"ander auf Hoch/Quer updaten, dann Beispiel updaten
    bBorderModified = FALSE;
    SwapFirstValues_Impl( FALSE );
    UpdateExample_Impl();

    // Alte Werte sichern
    aLeftMarginEdit.SaveValue();
    aRightMarginEdit.SaveValue();
    aTopMarginEdit.SaveValue();
    aBottomMarginEdit.SaveValue();
    aLayoutBox.SaveValue();
    aNumberFormatBox.SaveValue();
    aPaperSizeBox.SaveValue();
    aPaperWidthEdit.SaveValue();
    aPaperHeightEdit.SaveValue();
    aPortraitBtn.SaveValue();
    aLandscapeBtn.SaveValue();
    aPaperTrayBox.SaveValue();
    aVertBox.SaveValue();
    aHorzBox.SaveValue();
    aAdaptBox.SaveValue();

    // Registerhaltigkeit
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_MODE))
    {
        aRegisterCB.Check(((const SfxBoolItem&)rSet.Get(
                                SID_SWREGISTER_MODE)).GetValue());
        aRegisterCB.SaveValue();
        RegisterModify(&aRegisterCB);
    }
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SWREGISTER_COLLECTION))
    {
        aRegisterLB.SelectEntry(
                ((const SfxStringItem&)rSet.Get(SID_SWREGISTER_COLLECTION)).GetValue());
        aRegisterLB.SaveValue();
    }

}

// -----------------------------------------------------------------------

void SvxPageDescPage::FillUserData()
{
    if ( SVX_PAGE_MODE_PRESENTATION == eMode )
        SetUserData( UniString::CreateFromInt32( (sal_Int32)aAdaptBox.IsChecked() ) );
}

// -----------------------------------------------------------------------

BOOL SvxPageDescPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;
    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    USHORT nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = pPool->GetMetric( nWhich );
    const SfxPoolItem* pOld = 0;

    // alten linken und rechten Rand kopieren
    SvxLRSpaceItem aMargin( (const SvxLRSpaceItem&)rOldSet.Get( nWhich ) );

    // alten  oberen und unteren Rand kopieren
    nWhich = GetWhich( SID_ATTR_ULSPACE );
    SvxULSpaceItem aTopMargin( (const SvxULSpaceItem&)rOldSet.Get( nWhich ) );

    if ( aLeftMarginEdit.GetText() != aLeftMarginEdit.GetSavedValue() )
    {
        aMargin.SetLeft( (USHORT)GetCoreValue( aLeftMarginEdit, eUnit ) );
        bModified |= TRUE;
    }

    if ( aRightMarginEdit.GetText() != aRightMarginEdit.GetSavedValue() )
    {
        aMargin.SetRight( (USHORT)GetCoreValue( aRightMarginEdit, eUnit ) );
        bModified |= TRUE;
    }

    // Linken und rechten Rand setzen
    if ( bModified )
    {
        pOld = GetOldItem( rSet, SID_ATTR_LRSPACE );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) )
            rSet.Put( aMargin );
        else
            bModified = FALSE;
    }

    BOOL bMod = FALSE;

    if ( aTopMarginEdit.GetText() != aTopMarginEdit.GetSavedValue() )
    {
        aTopMargin.SetUpper( (USHORT)GetCoreValue( aTopMarginEdit, eUnit ) );
        bMod |= TRUE;
    }

    if ( aBottomMarginEdit.GetText() != aBottomMarginEdit.GetSavedValue() )
    {
        aTopMargin.SetLower( (USHORT)GetCoreValue( aBottomMarginEdit, eUnit ) );
        bMod |= TRUE;
    }

    // unteren oberen Rand setzen
    //
    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_ULSPACE );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aTopMargin ) )
        {
            bModified |= TRUE;
            rSet.Put( aTopMargin );
        }
    }

    // Druckerschacht
    nWhich = GetWhich( SID_ATTR_PAGE_PAPERBIN );
    USHORT nPos = aPaperTrayBox.GetSelectEntryPos();
    USHORT nBin = (USHORT)(ULONG)aPaperTrayBox.GetEntryData( nPos );
    pOld = GetOldItem( rSet, SID_ATTR_PAGE_PAPERBIN );

    if ( !pOld || ( (const SvxPaperBinItem*)pOld )->GetValue() != nBin )
    {
        rSet.Put( SvxPaperBinItem( nWhich, (BYTE)nBin ) );
        bModified |= TRUE;
    }

    nPos = aPaperSizeBox.GetSelectEntryPos();
    SvxPaper ePaper = (SvxPaper)(ULONG)aPaperSizeBox.GetEntryData( nPos );
    const USHORT nOld = aPaperSizeBox.GetSavedValue();
    BOOL bChecked = aLandscapeBtn.IsChecked();

    if ( SVX_PAPER_USER == ePaper )
    {
        if ( nOld != nPos                       ||
             aPaperWidthEdit.IsValueModified()  ||
             aPaperHeightEdit.IsValueModified() ||
             bChecked != aLandscapeBtn.GetSavedValue() )
        {
            Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                        GetCoreValue( aPaperHeightEdit, eUnit ) );
            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= TRUE;
            }
        }
    }
    else
    {
        if ( nOld != nPos || bChecked != aLandscapeBtn.GetSavedValue() )
        {
            Size aSize( SvxPaperInfo::GetPaperSize( ePaper, (MapUnit)eUnit ) );

            if ( bChecked )
                Swap( aSize );

            pOld = GetOldItem( rSet, SID_ATTR_PAGE_SIZE );

            if ( !pOld || ( (const SvxSizeItem*)pOld )->GetSize() != aSize )
            {
                rSet.Put( SvxSizeItem( GetWhich(SID_ATTR_PAGE_SIZE), aSize ) );
                bModified |= TRUE;
            }
        }
    }

    // sonstiges Zeug der Page
    nWhich = GetWhich( SID_ATTR_PAGE );
    SvxPageItem aPage( (const SvxPageItem&)rOldSet.Get( nWhich ) );
    bMod =  aLayoutBox.GetSelectEntryPos()  != aLayoutBox.GetSavedValue();

    if ( bMod )
        aPage.SetPageUsage(
            ::PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() ) );

    if ( bChecked != aLandscapeBtn.GetSavedValue() )
    {
        aPage.SetLandscape(bChecked);
        bMod |= TRUE;
    }

    // Einstellen der Numerierungsart der Seite
    nPos = aNumberFormatBox.GetSelectEntryPos();

    if ( nPos != aNumberFormatBox.GetSavedValue() )
    {
        aPage.SetNumType( (SvxNumType)nPos );
        bMod |= TRUE;
    }

    if ( bMod )
    {
        pOld = GetOldItem( rSet, SID_ATTR_PAGE );

        if ( !pOld || !( *(const SvxPageItem*)pOld == aPage ) )
        {
            rSet.Put( aPage );
            bModified |= TRUE;
        }
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich ) )
        rSet.ClearItem( nWhich );
    else
        rSet.Put( rOldSet.Get( nWhich ) );

    // Modispezifische Controls auswerten

    switch ( eMode )
    {
        case SVX_PAGE_MODE_CENTER:
        {
            if ( aHorzBox.IsChecked() != aHorzBox.GetSavedValue() )
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT1 ),
                                   aHorzBox.IsChecked() );
                rSet.Put( aHorz );
                bModified |= TRUE;
            }

            if ( aVertBox.IsChecked() != aVertBox.GetSavedValue() )
            {
                SfxBoolItem aVert( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   aVertBox.IsChecked() );
                rSet.Put( aVert );
                bModified |= TRUE;
            }
            break;
        }

        case SVX_PAGE_MODE_PRESENTATION:
        {
            // immer putten, damit Draw das auswerten kann
            rSet.Put( SfxBoolItem( GetWhich( SID_ATTR_PAGE_EXT1 ),
                      aAdaptBox.IsChecked() ) );
            bModified |= TRUE;

            if( aHorzBox.IsChecked() != aHorzBox.GetSavedValue() )
            {
                SfxBoolItem aHorz( GetWhich( SID_ATTR_PAGE_EXT2 ),
                                   aHorzBox.IsChecked() );
                rSet.Put( aHorz );
                bModified |= TRUE;
            }

            break;
        }

        // bei Standard tut sich nichts
    }

    if(aRegisterCB.IsVisible() &&
        (aRegisterCB.IsChecked() || aRegisterCB.GetSavedValue() != aRegisterCB.IsChecked()))
    {
        const SfxBoolItem& rRegItem = (const SfxBoolItem&)rOldSet.Get(SID_SWREGISTER_MODE);
        SfxBoolItem* pRegItem = (SfxBoolItem*)rRegItem.Clone();
        BOOL bCheck = aRegisterCB.IsChecked();
        pRegItem->SetValue(bCheck);
        rSet.Put(*pRegItem);
        bModified |= TRUE;
        if(bCheck)
        {
            bModified |= TRUE;
            rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION,
                            aRegisterLB.GetSelectEntry()));
        }
        delete pRegItem;
    }


    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, LayoutHdl_Impl, ListBox *, EMPTYARG )
{
    // innen au\sen umschalten
    const USHORT nPos = PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() );

    if ( nPos == SVX_PAGE_MIRROR )
    {
        if ( aLeftMarginLbl.GetText() != aInsideText )
            aLeftMarginLbl.SetText( aInsideText );

        if ( aRightMarginLbl.GetText() != aOutsideText )
            aRightMarginLbl.SetText( aOutsideText );
    }
    else
    {
        if ( aLeftMarginLbl.GetText() != aLeftText )
            aLeftMarginLbl.SetText( aLeftText );

        if ( aRightMarginLbl.GetText() != aRightText )
            aRightMarginLbl.SetText( aRightText );
    }
    UpdateExample_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperBinHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aPaperTrayBox.GetEntryCount() > 1 )
        // schon gef"ullt
        return 0;
    Printer* pDefPrinter = 0;
    BOOL bPrinterDel = FALSE;

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
        pDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pDefPrinter = new Printer;
        bPrinterDel = TRUE;
    }

    // Schacht-Box initialisieren
    String aOldName = aPaperTrayBox.GetSelectEntry();
    aPaperTrayBox.SetUpdateMode( FALSE );
    aPaperTrayBox.Clear();
    USHORT nEntryPos = aPaperTrayBox.InsertEntry(
        SVX_RESSTR( RID_SVXSTR_PAPERBIN_SETTINGS ) );
    aPaperTrayBox.SetEntryData( nEntryPos,
        (void*)(ULONG)PAPERBIN_PRINTER_SETTINGS );
    String aPaperBin( SVX_RES( RID_SVXSTR_PAPERBIN ) );
    USHORT nBinCount = pDefPrinter->GetPaperBinCount();

    for ( USHORT i = 0; i < nBinCount; ++i )
    {
        String aName = pDefPrinter->GetPaperBinName(i);

        if ( !aName.Len() )
        {
            aName = aPaperBin;
            aName.Append( sal_Unicode(' ') );
            aName.Append( UniString::CreateFromInt32( i+1 ) );
        }
        nEntryPos = aPaperTrayBox.InsertEntry( aName );
        aPaperTrayBox.SetEntryData( nEntryPos, (void*)(ULONG)i );
    }
    aPaperTrayBox.SelectEntry( aOldName );
    aPaperTrayBox.SetUpdateMode( TRUE );

    if ( bPrinterDel )
        delete pDefPrinter;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperSizeSelect_Impl, ListBox *, pBox )
{
    const USHORT nPos = pBox->GetSelectEntryPos();
    SvxPaper ePaper = (SvxPaper)(ULONG)aPaperSizeBox.GetEntryData( nPos );

    if ( ePaper != SVX_PAPER_USER )
    {
        Size aSize( SvxPaperInfo::GetPaperSize( ePaper ) );

        if ( aLandscapeBtn.IsChecked() )
            Swap( aSize );

        if ( aSize.Height() < aPaperHeightEdit.GetMin( FUNIT_TWIP ) )
            aPaperHeightEdit.SetMin(
                aPaperHeightEdit.Normalize( aSize.Height() ), FUNIT_TWIP );
        if ( aSize.Width() < aPaperWidthEdit.GetMin( FUNIT_TWIP ) )
            aPaperWidthEdit.SetMin(
                aPaperWidthEdit.Normalize( aSize.Width() ), FUNIT_TWIP );
        SetMetricValue( aPaperHeightEdit, aSize.Height(), SFX_MAPUNIT_TWIP );
        SetMetricValue( aPaperWidthEdit, aSize.Width(), SFX_MAPUNIT_TWIP );

        // R"ander ggf. neu berechnen
        CalcMargin_Impl();

        RangeHdl_Impl( 0 );
        UpdateExample_Impl();

        if ( eMode == SVX_PAGE_MODE_PRESENTATION )
        {
            // Draw: bei Papierformat soll der Rand 1cm betragen
            long nTmp = 0;
            BOOL bScreen = ( SVX_PAPER_SCREEN == ePaper );

            if ( !bScreen )
                // bei Bildschirm keinen Rand
                nTmp = 1; // entspr. 1cm

            // Abfragen, ob fuer Raender 0 gesetzt ist:
            if ( bScreen || aRightMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aRightMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aRightMarginEdit.GetFirst() > aRightMarginEdit.GetValue() )
                    aRightMarginEdit.SetValue( aRightMarginEdit.GetFirst() );
            }
            if ( bScreen || aLeftMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aLeftMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aLeftMarginEdit.GetFirst() > aLeftMarginEdit.GetValue() )
                    aLeftMarginEdit.SetValue( aLeftMarginEdit.GetFirst() );
            }
            if ( bScreen || aBottomMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aBottomMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aBottomMarginEdit.GetFirst() > aBottomMarginEdit.GetValue() )
                    aBottomMarginEdit.SetValue( aBottomMarginEdit.GetFirst() );
            }
            if ( bScreen || aTopMarginEdit.GetValue() == 0 )
            {
                SetMetricValue( aTopMarginEdit, nTmp, SFX_MAPUNIT_CM );
                if ( !bScreen &&
                     aTopMarginEdit.GetFirst() > aTopMarginEdit.GetValue() )
                    aTopMarginEdit.SetValue( aTopMarginEdit.GetFirst() );
            }
            UpdateExample_Impl();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, PaperSizeModify_Impl, Edit *, EMPTYARG )
{
    USHORT nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                GetCoreValue( aPaperHeightEdit, eUnit ) );
    SvxPaper ePaper = SvxPaperInfo::GetPaper( aSize, (MapUnit)eUnit, TRUE );
    USHORT nEntryCount = aPaperSizeBox.GetEntryCount();

    for ( USHORT i = 0; i < nEntryCount; ++i )
    {
        SvxPaper eTmp = (SvxPaper)(ULONG)aPaperSizeBox.GetEntryData(i);

        if ( eTmp == ePaper )
        {
            aPaperSizeBox.SelectEntryPos(i);
            break;
        }
    }
    UpdateExample_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, SwapOrientation_Impl, RadioButton *, pBtn )
{
    if ( !bLandscape && pBtn == &aLandscapeBtn ||
         bLandscape  && pBtn == &aPortraitBtn )
    {
        bLandscape = aLandscapeBtn.IsChecked();

        const long lWidth = GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_TWIP );
        const long lHeight = GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_TWIP );

        // swappen
        SetMetricValue( aPaperWidthEdit, lHeight, SFX_MAPUNIT_TWIP );
        SetMetricValue( aPaperHeightEdit, lWidth, SFX_MAPUNIT_TWIP );

        // R"ander ggf. neu berechnen
        CalcMargin_Impl();

        PaperSizeSelect_Impl( &aPaperSizeBox );
        RangeHdl_Impl( 0 );
        SwapFirstValues_Impl( bBorderModified );
        UpdateExample_Impl();
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::SwapFirstValues_Impl( FASTBOOL bSet )
{
    Printer* pDefPrinter = 0;
    BOOL bPrinterDel = FALSE;

    if ( SfxViewShell::Current() && SfxViewShell::Current()->GetPrinter() )
        pDefPrinter = (Printer*)SfxViewShell::Current()->GetPrinter();
    else
    {
        pDefPrinter = new Printer;
        bPrinterDel = TRUE;
    }

    MapMode aOldMode = pDefPrinter->GetMapMode();
    Orientation eOri = ORIENTATION_PORTRAIT;

    if ( bLandscape )
        eOri = ORIENTATION_LANDSCAPE;
    Orientation eOldOri = pDefPrinter->GetOrientation();
    pDefPrinter->SetOrientation( eOri );
    pDefPrinter->SetMapMode( MAP_TWIP );

    // First- und Last-Werte f"ur die R"ander setzen
    Size aPaperSize = pDefPrinter->GetPaperSize();
    Size aPrintSize = pDefPrinter->GetOutputSize();
    /*
     * einen Punkt ( 0,0 ) in logische Koordinaten zu konvertieren,
     * sieht aus wie Unsinn; ist aber sinnvoll, wenn der Ursprung des
     * Koordinatensystems verschoben ist.
     */
    Point aPrintOffset = pDefPrinter->GetPageOffset() -
                         pDefPrinter->PixelToLogic( Point() );
    pDefPrinter->SetMapMode( aOldMode );
    pDefPrinter->SetOrientation( eOldOri );

    if ( bPrinterDel )
        delete pDefPrinter;

    long nSetL = aLeftMarginEdit.Denormalize(
                    aLeftMarginEdit.GetValue( FUNIT_TWIP ) );
    long nSetR = aRightMarginEdit.Denormalize(
                    aRightMarginEdit.GetValue( FUNIT_TWIP ) );
    long nSetT = aTopMarginEdit.Denormalize(
                    aTopMarginEdit.GetValue( FUNIT_TWIP ) );
    long nSetB = aBottomMarginEdit.Denormalize(
                    aBottomMarginEdit.GetValue( FUNIT_TWIP ) );

    long nOffset = !aPrintOffset.X() && !aPrintOffset.X() ? 0 : PRINT_OFFSET;
    long nNewL = aPrintOffset.X();
    long nNewR =
        aPaperSize.Width() - aPrintSize.Width() - aPrintOffset.X() + nOffset;
    long nNewT = aPrintOffset.Y();
    long nNewB =
        aPaperSize.Height() - aPrintSize.Height() - aPrintOffset.Y() + nOffset;

    aLeftMarginEdit.SetFirst( aLeftMarginEdit.Normalize( nNewL ), FUNIT_TWIP );
    nFirstLeftMargin = aLeftMarginEdit.GetFirst();
    aRightMarginEdit.SetFirst( aRightMarginEdit.Normalize( nNewR ), FUNIT_TWIP );
    nFirstRightMargin = aRightMarginEdit.GetFirst();
    aTopMarginEdit.SetFirst( aTopMarginEdit.Normalize( nNewT ), FUNIT_TWIP );
    nFirstTopMargin = aTopMarginEdit.GetFirst();
    aBottomMarginEdit.SetFirst( aBottomMarginEdit.Normalize( nNewB ), FUNIT_TWIP );
    nFirstBottomMargin = aBottomMarginEdit.GetFirst();

    if ( bSet )
    {
        // ggf. auch die Werte umsetzen,
        if ( nSetL < nNewL )
            aLeftMarginEdit.SetValue( aLeftMarginEdit.Normalize( nNewL ),
                                      FUNIT_TWIP );
        if ( nSetR < nNewR )
            aRightMarginEdit.SetValue( aRightMarginEdit.Normalize( nNewR ),
                                       FUNIT_TWIP );
        if ( nSetT < nNewT )
            aTopMarginEdit.SetValue( aTopMarginEdit.Normalize( nNewT ),
                                     FUNIT_TWIP );
        if ( nSetB < nNewB )
            aBottomMarginEdit.SetValue( aBottomMarginEdit.Normalize( nNewB ),
                                        FUNIT_TWIP );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPageDescPage, BorderModify_Impl, MetricField *, EMPTYARG )
{
    if ( !bBorderModified )
        bBorderModified = TRUE;
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxPageDescPage, BorderModify_Impl, MetricField *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxPageDescPage::UpdateExample_Impl()
{
    // Size
    Size aSize( GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_TWIP ),
                GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_TWIP ) );

    aBspWin.SetSize( aSize );

    // R"ander
    aBspWin.SetTop( GetCoreValue( aTopMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetBottom( GetCoreValue( aBottomMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetLeft( GetCoreValue( aLeftMarginEdit, SFX_MAPUNIT_TWIP ) );
    aBspWin.SetRight( GetCoreValue( aRightMarginEdit, SFX_MAPUNIT_TWIP ) );

    // Layout
    aBspWin.SetUsage( PosToPageUsage_Impl( aLayoutBox.GetSelectEntryPos() ) );
    aBspWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxPageDescPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, FALSE );
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
                aBspWin.SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, FALSE );
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
                aBspWin.SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtBorder( rItem );
            }
        }
    }

    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_BRUSH );

    if ( pItem )
    {
        aBspWin.SetColor( ( (const SvxBrushItem*)pItem )->GetColor() );
        const Graphic* pGrf = ( (const SvxBrushItem*)pItem )->GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            aBspWin.SetBitmap( &aBitmap );
        }
        else
            aBspWin.SetBitmap( NULL );
    }

    pItem = GetItem( rSet, SID_ATTR_BORDER_OUTER );

    if ( pItem )
        aBspWin.SetBorder( (SvxBoxItem&)*pItem );
}

// -----------------------------------------------------------------------

void SvxPageDescPage::InitHeadFoot_Impl( const SfxItemSet& rSet )
{
    bLandscape = aLandscapeBtn.IsChecked();
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
        aBspWin.SetSize( ( (const SvxSizeItem*)pItem )->GetSize() );

    const SvxSetItem* pSetItem = 0;

    // Kopfzeilen-Attribute auswerten

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                            FALSE, (const SfxPoolItem**)&pSetItem ) )
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
            aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetHdDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            aBspWin.SetHdLeft( rLR.GetLeft() );
            aBspWin.SetHdRight( rLR.GetRight() );
            aBspWin.SetHeader( TRUE );
        }
        else
            aBspWin.SetHeader( FALSE );

        // im Beispiel Hintergrund und Umrandung anzeigen
        USHORT nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rHeaderSet.Get( nWhich );
            aBspWin.SetHdColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rHeaderSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rHeaderSet.Get( nWhich );
            aBspWin.SetHdBorder( rItem );
        }
    }

    // Fusszeilen-Attribute auswerten

    if ( SFX_ITEM_SET ==
         rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                            FALSE, (const SfxPoolItem**)&pSetItem ) )
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
            aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetFtDist( nDist );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            aBspWin.SetFtLeft( rLR.GetLeft() );
            aBspWin.SetFtRight( rLR.GetRight() );
            aBspWin.SetFooter( TRUE );
        }
        else
            aBspWin.SetFooter( FALSE );

        // im Beispiel Hintergrund und Umrandung anzeigen
        USHORT nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBrushItem& rItem =
                (const SvxBrushItem&)rFooterSet.Get( nWhich );
            aBspWin.SetFtColor( rItem.GetColor() );
        }
        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( rFooterSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxBoxItem& rItem =
                (const SvxBoxItem&)rFooterSet.Get( nWhich );
            aBspWin.SetFtBorder( rItem );
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

int SvxPageDescPage::DeactivatePage( SfxItemSet* pSet )
{
    // Abfrage, ob die Seitenr"ander ausserhalb des Druckbereichs liegen
    // Wenn nicht, dann den Anwender fragen, ob sie "ubernommen werden sollen.
    // Wenn nicht, dann auf der TabPage bleiben.
    USHORT nPos = aPaperSizeBox.GetSelectEntryPos();
    SvxPaper ePaper = (SvxPaper)(ULONG)aPaperSizeBox.GetEntryData( nPos );

    if ( ePaper != SVX_PAPER_SCREEN && (
         aLeftMarginEdit.GetValue() < nFirstLeftMargin ||
         aRightMarginEdit.GetValue() < nFirstRightMargin ||
         aTopMarginEdit.GetValue() < nFirstTopMargin ||
         aBottomMarginEdit.GetValue() < nFirstBottomMargin ||
         aLeftMarginEdit.GetValue() > nLastLeftMargin ||
         aRightMarginEdit.GetValue() > nLastRightMargin ||
         aTopMarginEdit.GetValue() > nLastTopMargin ||
         aBottomMarginEdit.GetValue() > nLastBottomMargin ) )
    {
        if ( QueryBox( this, WB_YES_NO | WB_DEF_NO, aPrintRangeQueryText ).Execute() == RET_NO )
        {
            MetricField* pField = NULL;
            if ( aLeftMarginEdit.GetValue() < nFirstLeftMargin )
            {
                aLeftMarginEdit.SetValue( nFirstLeftMargin );
                pField = &aLeftMarginEdit;
            }
            if ( aRightMarginEdit.GetValue() < nFirstRightMargin )
            {
                aRightMarginEdit.SetValue( nFirstRightMargin );
                if ( !pField )
                    pField = &aRightMarginEdit;
            }
            if ( aTopMarginEdit.GetValue() < nFirstTopMargin )
            {
                aTopMarginEdit.SetValue( nFirstTopMargin );
                if ( !pField )
                    pField = &aTopMarginEdit;
            }
            if ( aBottomMarginEdit.GetValue() < nFirstBottomMargin )
            {
                aBottomMarginEdit.SetValue( nFirstBottomMargin );
                if ( !pField )
                    pField = &aBottomMarginEdit;
            }
            if ( aLeftMarginEdit.GetValue() > nLastLeftMargin )
            {
                aLeftMarginEdit.SetValue( nLastLeftMargin );
                if ( !pField )
                    pField = &aLeftMarginEdit;
            }
            if ( aRightMarginEdit.GetValue() > nLastRightMargin )
            {
                aRightMarginEdit.SetValue( nLastRightMargin );
                if ( !pField )
                    pField = &aRightMarginEdit;
            }
            if ( aTopMarginEdit.GetValue() > nLastTopMargin )
            {
                aTopMarginEdit.SetValue( nLastTopMargin );
                if ( !pField )
                    pField = &aTopMarginEdit;
            }
            if ( aBottomMarginEdit.GetValue() > nLastBottomMargin )
            {
                aBottomMarginEdit.SetValue( nLastBottomMargin );
                if ( !pField )
                    pField = &aBottomMarginEdit;
            }
            if ( pField )
                pField->GrabFocus();
            UpdateExample_Impl();
            return KEEP_PAGE;
        }
    }

    if ( pSet )
    {
        FillItemSet( *pSet );

        // ggf. hoch/quer putten
        USHORT nWh = GetWhich( SID_ATTR_PAGE_SIZE );
        SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWh );
        Size aSize( GetCoreValue( aPaperWidthEdit, eUnit ),
                    GetCoreValue( aPaperHeightEdit, eUnit ) );

        // putten, wenn aktuelle Gr"o/se unterschiedlich zum Wert in pSet
        const SvxSizeItem* pSize = (const SvxSizeItem*)GetItem( *pSet, SID_ATTR_PAGE_SIZE );
        if ( aSize.Width() && ( !pSize || !IsEqualSize_Impl( pSize, aSize ) ) )
            pSet->Put( SvxSizeItem( nWh, aSize ) );
    }
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, RangeHdl_Impl, Edit *, EMPTYARG )
{
    // Aktuelle Header-Breite/H"ohe aus dem Bsp
    long nHHeight = aBspWin.GetHdHeight();
    long nHDist = aBspWin.GetHdDist();

    // Aktuelle Footer-Breite/H"ohe aus dem Bsp
    long nFHeight = aBspWin.GetFtHeight();
    long nFDist = aBspWin.GetFtDist();

    // Aktuelle Header/Footer-R"ander aus dem Bsp
    long nHFLeft = Max( aBspWin.GetHdLeft(), aBspWin.GetFtLeft() );
    long nHFRight = Max( aBspWin.GetHdRight(), aBspWin.GetFtRight() );

    // Aktuelle Werte der Seitenr"ander
    long nBT = aTopMarginEdit.Denormalize(aTopMarginEdit.GetValue(FUNIT_TWIP));
    long nBB = aBottomMarginEdit.Denormalize(aBottomMarginEdit.GetValue(FUNIT_TWIP));
    long nBL = aLeftMarginEdit.Denormalize(aLeftMarginEdit.GetValue(FUNIT_TWIP));
    long nBR = aRightMarginEdit.Denormalize(aRightMarginEdit.GetValue(FUNIT_TWIP));

    // Breite Umrandung der Seite berechnen
    const SfxItemSet* pSet = &GetItemSet();
    Size aBorder;

    if ( pSet->GetItemState( GetWhich(SID_ATTR_BORDER_SHADOW) ) >=
            SFX_ITEM_AVAILABLE &&
         pSet->GetItemState( GetWhich(SID_ATTR_BORDER_OUTER)  ) >=
            SFX_ITEM_AVAILABLE )
    {
        aBorder = ( GetMinBorderSpace_Impl(
            (const SvxShadowItem&)pSet->Get(GetWhich(SID_ATTR_BORDER_SHADOW)),
            (const SvxBoxItem&)pSet->Get(GetWhich(SID_ATTR_BORDER_OUTER))));
    }

    long nH  = aPaperHeightEdit.Denormalize(aPaperHeightEdit.GetValue(FUNIT_TWIP));
    long nW  = aPaperWidthEdit.Denormalize(aPaperWidthEdit.GetValue(FUNIT_TWIP));

    // Grenzen Papier
    // Maximum liegt bei 54cm
    //
    long nMin = nHHeight + nHDist + nFDist + nFHeight + nBT + nBB +
                MINBODY + aBorder.Height();
    aPaperHeightEdit.SetMin(aPaperHeightEdit.Normalize(nMin), FUNIT_TWIP);

    nMin = MINBODY + nBL + nBR + aBorder.Width();
    aPaperWidthEdit.SetMin(aPaperWidthEdit.Normalize(nMin), FUNIT_TWIP);

    // Falls sich die Papiergr"o\se ge"adert hat
    nH = aPaperHeightEdit.Denormalize(aPaperHeightEdit.GetValue(FUNIT_TWIP));
    nW = aPaperWidthEdit.Denormalize(aPaperWidthEdit.GetValue(FUNIT_TWIP));

    // Top
    long nMax = nH - nBB - aBorder.Height() - MINBODY -
                nFDist - nFHeight - nHDist - nHHeight;

    aTopMarginEdit.SetMax(aTopMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Bottom
    nMax = nH - nBT - aBorder.Height() - MINBODY -
           nFDist - nFHeight - nHDist - nHHeight;

    aBottomMarginEdit.SetMax(aTopMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Left
    nMax = nW - nBR - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    aLeftMarginEdit.SetMax(aLeftMarginEdit.Normalize(nMax), FUNIT_TWIP);

    // Right
    nMax = nW - nBL - MINBODY - aBorder.Width() - nHFLeft - nHFRight;
    aRightMarginEdit.SetMax(aRightMarginEdit.Normalize(nMax), FUNIT_TWIP);
    return 0;
}

// -----------------------------------------------------------------------

void SvxPageDescPage::CalcMargin_Impl()
{
    // Aktuelle Werte der Seitenr"ander
    long nBT = GetCoreValue( aTopMarginEdit, SFX_MAPUNIT_TWIP );
    long nBB = GetCoreValue( aBottomMarginEdit, SFX_MAPUNIT_TWIP );

    long nBL = GetCoreValue( aLeftMarginEdit, SFX_MAPUNIT_TWIP );
    long nBR = GetCoreValue( aRightMarginEdit, SFX_MAPUNIT_TWIP );

    long nH  = GetCoreValue( aPaperHeightEdit, SFX_MAPUNIT_TWIP );
    long nW  = GetCoreValue( aPaperWidthEdit, SFX_MAPUNIT_TWIP );

    long nWidth = nBL + nBR + MINBODY;
    long nHeight = nBT + nBB + MINBODY;

    if ( nWidth > nW || nHeight > nH )
    {
        if ( nWidth > nW )
        {
            long nTmp = nBL <= nBR ? nBR : nBL;
            nTmp -= nWidth - nW;

            if ( nBL <= nBR )
                SetMetricValue( aRightMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( aLeftMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }

        if ( nHeight > nH )
        {
            long nTmp = nBT <= nBB ? nBB : nBT;
            nTmp -= nHeight - nH;

            if ( nBT <= nBB )
                SetMetricValue( aBottomMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
            else
                SetMetricValue( aTopMarginEdit, nTmp, SFX_MAPUNIT_TWIP );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPageDescPage, CenterHdl_Impl, CheckBox *, EMPTYARG )
{
    // Wird im Draw zweckentfremdet
    if( eMode != SVX_PAGE_MODE_PRESENTATION )
    {
        aBspWin.SetHorz( aHorzBox.IsChecked() );
        aBspWin.SetVert( aVertBox.IsChecked() );
        UpdateExample_Impl();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxPageDescPage, CenterHdl_Impl, CheckBox *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxPageDescPage::SetCollectionList(const List* pList)
{
    sStandardRegister = *(String*)pList->GetObject(0);
    for( USHORT i = 1; i < pList->Count(); i++   )
    {
        aRegisterLB.InsertEntry(*(String*)pList->GetObject(i));
    }

    aPageName .Hide();
    aExtraFl .Hide();
    aRegisterCB  .Show();
    aRegisterFT  .Show();
    aRegisterLB.Show();
    aRegisterFl  .Show();
    aRegisterCB.SetClickHdl(LINK(this, SvxPageDescPage, RegisterModify));
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPageDescPage, RegisterModify, CheckBox*, pBox )
{
    BOOL bEnable = FALSE;
    if(pBox->IsChecked())
    {
        bEnable = TRUE;
        if(USHRT_MAX == aRegisterLB.GetSelectEntryPos())
            aRegisterLB.SelectEntry(sStandardRegister);
    }
    aRegisterFT.Enable( bEnable );
    aRegisterLB.Enable( bEnable );
    return 0;
}


