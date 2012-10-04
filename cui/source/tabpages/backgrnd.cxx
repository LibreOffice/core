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

#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/wallitem.hxx>
#include <svl/cntwall.hxx>
#include <sfx2/cntids.hrc>
#include <svx/dialogs.hrc>

#define _SVX_BACKGRND_CXX

#include <cuires.hrc>
#include "backgrnd.hrc"
#include <svx/dialmgr.hxx>
#include <editeng/memberids.hrc>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

// table background
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

#include <editeng/brshitem.hxx>
#include "backgrnd.hxx"

#include <svx/xtable.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svxerr.hxx>
#include <svx/drawitem.hxx>
#include <dialmgr.hxx>
#include <svx/htmlmode.hxx>
#include <svtools/controldims.hrc>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
using namespace ::com::sun::star;
// static ----------------------------------------------------------------

static sal_uInt16 pRanges[] =
{
    SID_VIEW_FLD_PIC, SID_VIEW_FLD_PIC,
    SID_ATTR_BRUSH, SID_ATTR_BRUSH,
    SID_ATTR_BRUSH_CHAR, SID_ATTR_BRUSH_CHAR,
    0
};

struct SvxBackgroundTable_Impl
{
    SvxBrushItem*   pCellBrush;
    SvxBrushItem*   pRowBrush;
    SvxBrushItem*   pTableBrush;
    sal_uInt16          nCellWhich;
    sal_uInt16          nRowWhich;
    sal_uInt16          nTableWhich;
    sal_uInt16          nActPos;

    SvxBackgroundTable_Impl() :
        pCellBrush(NULL), pRowBrush(NULL), pTableBrush(NULL),
        nCellWhich(0), nRowWhich(0), nTableWhich(0) {}
};

struct SvxBackgroundPara_Impl
{
    SvxBrushItem*   pParaBrush;
    SvxBrushItem*   pCharBrush;

    sal_uInt16          nActPos;

    SvxBackgroundPara_Impl() :
        pParaBrush(NULL), pCharBrush(NULL) {}
};

struct SvxBackgroundPage_Impl
{
    Timer*          pLoadTimer;
    sal_Bool            bIsImportDlgInExecute;

    SvxBackgroundPage_Impl() :
        pLoadTimer(NULL), bIsImportDlgInExecute(sal_False) {}
};

inline sal_uInt8 lcl_PercentToTransparency(long nPercent)
{
    //0xff must not be returned!
    return sal_uInt8(nPercent ? (50 + 0xfe * nPercent) / 100 : 0);
}
inline sal_uInt8 lcl_TransparencyToPercent(sal_uInt8 nTrans)
{
    return (nTrans * 100 + 127) / 254;
}
void lcl_SetTransparency(SvxBrushItem& rBrush, long nTransparency)
{
    uno::Any aTransparency;
    aTransparency <<= (sal_Int8)nTransparency;
    rBrush.PutValue(aTransparency, MID_GRAPHIC_TRANSPARENCY);
}
//-------------------------------------------------------------------------

sal_uInt16 GetItemId_Impl( ValueSet& rValueSet, const Color& rCol )
{
    sal_Bool    bFound = sal_False;
    sal_uInt16  nCount = rValueSet.GetItemCount();
    sal_uInt16  n      = 1;

    while ( !bFound && n <= nCount )
    {
        Color aValCol = rValueSet.GetItemColor(n);

        bFound = (   aValCol.GetRed()   == rCol.GetRed()
                  && aValCol.GetGreen() == rCol.GetGreen()
                  && aValCol.GetBlue()  == rCol.GetBlue() );

        if ( !bFound )
            n++;
    }
    return bFound ? n : 0;
}

// class BackgroundPreview -----------------------------------------------

/*  [Description]

    preview window for brush or bitmap
*/

class BackgroundPreviewImpl : public Window
{
public:
    BackgroundPreviewImpl( Window* pParent,
                           const ResId& rResId, sal_Bool bIsBmpPreview );
    ~BackgroundPreviewImpl();

    void            NotifyChange( const Color&  rColor );
    void            NotifyChange( const Bitmap* pBitmap );

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

private:
    const sal_Bool      bIsBmp;
    Bitmap*         pBitmap;
    Point           aDrawPos;
    Size            aDrawSize;
    Rectangle       aDrawRect;
    sal_uInt8            nTransparency;
};

//-----------------------------------------------------------------------

BackgroundPreviewImpl::BackgroundPreviewImpl
(
    Window* pParent,
    const ResId& rResId,
    sal_Bool bIsBmpPreview
) :

    Window( pParent, rResId ),

    bIsBmp   ( bIsBmpPreview ),
    pBitmap  ( NULL ),
    aDrawRect( Point(0,0), GetOutputSizePixel() ),
    nTransparency(0)

{
    SetBorderStyle(WINDOW_BORDER_MONO);
    Paint( aDrawRect );
}

//-----------------------------------------------------------------------

BackgroundPreviewImpl::~BackgroundPreviewImpl()
{
    delete pBitmap;
}

//-----------------------------------------------------------------------
void BackgroundPreviewImpl::NotifyChange( const Color& rColor )
{
    if ( !bIsBmp )
    {
        const static Color aTranspCol( COL_TRANSPARENT );

        nTransparency = lcl_TransparencyToPercent( rColor.GetTransparency() );

        SetFillColor( rColor == aTranspCol ? GetSettings().GetStyleSettings().GetFieldColor() : (Color) rColor.GetRGBColor() );
        Paint( aDrawRect );
    }
}

//-----------------------------------------------------------------------

void BackgroundPreviewImpl::NotifyChange( const Bitmap* pNewBitmap )
{
    if ( bIsBmp && (pNewBitmap || pBitmap) )
    {
        if ( pNewBitmap && pBitmap )
            *pBitmap = *pNewBitmap;
        else if ( pNewBitmap && !pBitmap )
            pBitmap = new Bitmap( *pNewBitmap );
        else if ( !pNewBitmap )
            DELETEZ( pBitmap );

        if ( pBitmap )
        {
            Size aSize = GetOutputSizePixel();
            // InnerSize == Size without one pixel border
            Size aInnerSize = aSize;
            aInnerSize.Width() -= 2;
            aInnerSize.Height() -= 2;
            aDrawSize = pBitmap->GetSizePixel();

            // bitmap bigger than preview window?
            if ( aDrawSize.Width() > aInnerSize.Width() )
            {
                aDrawSize.Height() = aDrawSize.Height() * aInnerSize.Width() / aDrawSize.Width();
                if ( aDrawSize.Height() > aInnerSize.Height() )
                {
                    aDrawSize.Width() = aDrawSize.Height();
                    aDrawSize.Height() = aInnerSize.Height();
                }
                else
                    aDrawSize.Width() = aInnerSize.Width();
            }
            else if ( aDrawSize.Height() > aInnerSize.Height() )
            {
                aDrawSize.Width() = aDrawSize.Width() * aInnerSize.Height() / aDrawSize.Height();
                if ( aDrawSize.Width() > aInnerSize.Width() )
                {
                    aDrawSize.Height() = aDrawSize.Width();
                    aDrawSize.Width() = aInnerSize.Width();
                }
                else
                    aDrawSize.Height() = aInnerSize.Height();
            }

            aDrawPos.X() = (aSize.Width()  - aDrawSize.Width())  / 2;
            aDrawPos.Y() = (aSize.Height() - aDrawSize.Height()) / 2;
        }
        Invalidate( aDrawRect );
        Update();
    }
}

//-----------------------------------------------------------------------

void BackgroundPreviewImpl::Paint( const Rectangle& )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
    SetLineColor();
    if(bIsBmp)
        SetFillColor( Color(COL_TRANSPARENT) );
    DrawRect( aDrawRect );
    if ( bIsBmp )
    {
        if ( pBitmap )
            DrawBitmap( aDrawPos, aDrawSize, *pBitmap );
        else
        {
            Size aSize = GetOutputSizePixel();
            DrawLine( Point(0,0),               Point(aSize.Width(),aSize.Height()) );
            DrawLine( Point(0,aSize.Height()),  Point(aSize.Width(),0) );
        }
    }
}

void BackgroundPreviewImpl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Invalidate();
    }
    Window::DataChanged( rDCEvt );
}

// class SvxBackgroundTabPage --------------------------------------------

#define HDL(hdl) LINK(this,SvxBackgroundTabPage,hdl)

SvxBackgroundTabPage::SvxBackgroundTabPage( Window* pParent,
                                            const SfxItemSet& rCoreSet ) :

    SvxTabPage( pParent, CUI_RES( RID_SVXPAGE_BACKGROUND ), rCoreSet ),

    aSelectTxt          ( this, CUI_RES( FT_SELECTOR ) ),
    aLbSelect           ( this, CUI_RES( LB_SELECTOR ) ),
    aStrBrowse          ( CUI_RES( STR_BROWSE ) ),
    aStrUnlinked        ( CUI_RES( STR_UNLINKED ) ),
    aTblDesc            ( this, CUI_RES( FT_TBL_DESC ) ),
    aTblLBox            ( this, CUI_RES( LB_TBL_BOX ) ),
    aParaLBox           ( this, CUI_RES( LB_PARA_BOX ) ),

    aBorderWin          ( this, CUI_RES(CT_BORDER) ),
    aBackgroundColorSet ( &aBorderWin, CUI_RES( SET_BGDCOLOR ) ),
    aBackgroundColorBox ( this, CUI_RES( GB_BGDCOLOR ) ),
    pPreviewWin1        ( new BackgroundPreviewImpl( this, CUI_RES( WIN_PREVIEW1 ), sal_False ) ),

    aColTransFT         ( this, CUI_RES( FT_COL_TRANS ) ),
    aColTransMF         ( this, CUI_RES( MF_COL_TRANS ) ),
    aBtnPreview         ( this, CUI_RES( BTN_PREVIEW ) ),
    aGbFile             ( this, CUI_RES( GB_FILE ) ),
    aBtnBrowse          ( this, CUI_RES( BTN_BROWSE ) ),
    aBtnLink            ( this, CUI_RES( BTN_LINK ) ),
    aGbPosition         ( this, CUI_RES( GB_POSITION ) ),
    aBtnPosition        ( this, CUI_RES( BTN_POSITION ) ),
    aBtnArea            ( this, CUI_RES( BTN_AREA ) ),
    aBtnTile            ( this, CUI_RES( BTN_TILE ) ),
    aWndPosition        ( this, CUI_RES( WND_POSITION ), RP_MM ),
    aFtFile             ( this, CUI_RES( FT_FILE ) ),
    aGraphTransFL       ( this, CUI_RES( FL_GRAPH_TRANS ) ),
    aGraphTransMF       ( this, CUI_RES( MF_GRAPH_TRANS ) ),
    pPreviewWin2        ( new BackgroundPreviewImpl(
                            this, CUI_RES( WIN_PREVIEW2 ), sal_True ) ),

    nHtmlMode           ( 0 ),
    bAllowShowSelector  ( sal_True ),
    bIsGraphicValid     ( sal_False ),
    bLinkOnly           ( sal_False ),
    bResized            ( sal_False ),
    bColTransparency    ( sal_False ),
    bGraphTransparency  ( sal_False ),

    pPageImpl           ( new SvxBackgroundPage_Impl ),
    pImportDlg          ( NULL ),
    pTableBck_Impl      ( NULL ),
    pParaBck_Impl       ( NULL )

{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;

    if ( SFX_ITEM_SET == rCoreSet.GetItemState( SID_HTML_MODE, sal_False, &pItem )
         || ( 0 != ( pShell = SfxObjectShell::Current()) &&
              0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
    }

    FillColorValueSets_Impl();

    aBackgroundColorSet.SetSelectHdl( HDL(BackgroundColorHdl_Impl) );
    FreeResource();

    aBtnBrowse.SetAccessibleRelationMemberOf(&aGbFile);
    aWndPosition.SetAccessibleRelationMemberOf(&aGbPosition);
    aWndPosition.SetAccessibleRelationLabeledBy(&aBtnPosition);
    aBackgroundColorSet.SetAccessibleRelationLabeledBy(&aBackgroundColorBox);
}

//------------------------------------------------------------------------

SvxBackgroundTabPage::~SvxBackgroundTabPage()
{
    delete pPreviewWin1;
    delete pPreviewWin2;
    delete pPageImpl->pLoadTimer;
    delete pPageImpl;
    delete pImportDlg;

    if( pTableBck_Impl)
    {
        delete pTableBck_Impl->pCellBrush;
        delete pTableBck_Impl->pRowBrush;
        delete pTableBck_Impl->pTableBrush;
        delete pTableBck_Impl;
    }

    if(pParaBck_Impl)
    {
        delete pParaBck_Impl->pParaBrush;
        delete pParaBck_Impl->pCharBrush;
        delete pParaBck_Impl;
    }
}

//------------------------------------------------------------------------

sal_uInt16* SvxBackgroundTabPage::GetRanges()

/*  [Description]

    returns the area of the which-values
*/

{
    return pRanges;
}

//------------------------------------------------------------------------

SfxTabPage* SvxBackgroundTabPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet )

/*  [Description]

    create method for the TabDialog
*/

{
    return ( new SvxBackgroundTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

void SvxBackgroundTabPage::Reset( const SfxItemSet& rSet )
{
// os: Such a nonsense! One will always find such an item somewhere,
//     but it must be existing in the rSet!
//  const SfxPoolItem* pX = GetOldItem( rSet, SID_VIEW_FLD_PIC );
//  if( pX && pX->ISA(SfxWallpaperItem))
    if(SFX_ITEM_AVAILABLE <= rSet.GetItemState(GetWhich(SID_VIEW_FLD_PIC), sal_False))
    {
        ResetFromWallpaperItem( rSet );
        return;
    }

    // condition of the preview button is persistent due to UserData
    String aUserData = GetUserData();
    aBtnPreview.Check( aUserData.Len() && sal_Unicode('1') == aUserData.GetChar( 0 ) );

    // don't be allowed to call ShowSelector() after reset anymore
    bAllowShowSelector = sal_False;


    // get and evaluate Input-BrushItem
    const SvxBrushItem* pBgdAttr = NULL;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    const SfxPoolItem* pItem;
    sal_uInt16 nDestValue = USHRT_MAX;

    if ( SFX_ITEM_SET == rSet.GetItemState( SID_BACKGRND_DESTINATION,
                                            sal_False, &pItem ) )
    {
        nDestValue = ((const SfxUInt16Item*)pItem)->GetValue();
        aTblLBox.SelectEntryPos(nDestValue);

        switch ( nDestValue )
        {
            case TBL_DEST_CELL:
                nSlot = SID_ATTR_BRUSH;
            break;
            case TBL_DEST_ROW:
                nSlot = SID_ATTR_BRUSH_ROW;
            break;
            case TBL_DEST_TBL:
                nSlot = SID_ATTR_BRUSH_TABLE;
            break;
        }
    }
    else if( SFX_ITEM_SET == rSet.GetItemState(
                SID_PARA_BACKGRND_DESTINATION, sal_False, &pItem ) )
    {
        nDestValue = ((const SfxUInt16Item*)pItem)->GetValue();
        // character activated?
        sal_uInt16 nParaSel  = aParaLBox.GetSelectEntryPos();
        if(1 == nParaSel)
        {
            // then it was a "standard"-call
            nDestValue = nParaSel;
        }
        aParaLBox.SelectEntryPos(nDestValue);

        switch ( nDestValue )
        {
            case PARA_DEST_PARA:
                nSlot = SID_ATTR_BRUSH;
            break;
            case PARA_DEST_CHAR:
                nSlot = SID_ATTR_BRUSH_CHAR;
            break;
        }
    }
    //#111173# the destination item is missing when the parent style has been changed
    if(USHRT_MAX == nDestValue && (aParaLBox.IsVisible()||aTblLBox.IsVisible()))
        nDestValue = 0;
    sal_uInt16 nWhich = GetWhich( nSlot );

    if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
        pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );

    aBtnTile.Check();

    if ( pBgdAttr )
    {
        FillControls_Impl(*pBgdAttr, aUserData);
        aBgdColor = ( (SvxBrushItem*)pBgdAttr )->GetColor();
    }
    else
    {
        aSelectTxt.Hide();
        aLbSelect.Hide();
        aLbSelect.SelectEntryPos( 0 );
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_BRUSH );

        if ( pOld )
            aBgdColor = ( (SvxBrushItem*)pOld )->GetColor();
    }

    if ( nDestValue != USHRT_MAX )
    {
        if(aTblLBox.IsVisible())
        {
            sal_uInt16 nValue = aTblLBox.GetSelectEntryPos();

            if ( pTableBck_Impl )
            {
                DELETEZ( pTableBck_Impl->pCellBrush);
                DELETEZ( pTableBck_Impl->pRowBrush);
                DELETEZ( pTableBck_Impl->pTableBrush);
            }
            else
                pTableBck_Impl = new SvxBackgroundTable_Impl();

            pTableBck_Impl->nActPos = nValue;

            nWhich = GetWhich( SID_ATTR_BRUSH );
            if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pTableBck_Impl->pCellBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nCellWhich = nWhich;

            if ( rSet.GetItemState( SID_ATTR_BRUSH_ROW, sal_False ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( SID_ATTR_BRUSH_ROW ) );
                pTableBck_Impl->pRowBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nRowWhich = SID_ATTR_BRUSH_ROW;

            if ( rSet.GetItemState( SID_ATTR_BRUSH_TABLE, sal_False ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( SID_ATTR_BRUSH_TABLE ) );
                pTableBck_Impl->pTableBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nTableWhich = SID_ATTR_BRUSH_TABLE;

            TblDestinationHdl_Impl(&aTblLBox);
            aTblLBox.SaveValue();
        }
        else
        {
            sal_uInt16 nValue = aParaLBox.GetSelectEntryPos();

            if ( pParaBck_Impl )
            {
                delete pParaBck_Impl->pParaBrush;
                delete pParaBck_Impl->pCharBrush;
            }
            else
                pParaBck_Impl = new SvxBackgroundPara_Impl();

            pParaBck_Impl->nActPos = nValue;

            nWhich = GetWhich( SID_ATTR_BRUSH );
            if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pParaBck_Impl->pParaBrush = new SvxBrushItem(*pBgdAttr);
            }

            nWhich = GetWhich( SID_ATTR_BRUSH_CHAR );
            rSet.GetItemState( nWhich, sal_True );
            rSet.GetItemState( nWhich, sal_False );
            if ( rSet.GetItemState( nWhich, sal_True ) > SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pParaBck_Impl->pCharBrush = new SvxBrushItem(*pBgdAttr);
            }
            else
                pParaBck_Impl->pCharBrush = new SvxBrushItem(SID_ATTR_BRUSH_CHAR);

            ParaDestinationHdl_Impl(&aParaLBox);
            aParaLBox.SaveValue();
        }
    }
    if(!bResized)
    {
        if(!aLbSelect.IsVisible() && !aTblLBox.IsVisible() && !aParaLBox.IsVisible())
        {
            long nY(LogicToPixel(Point(11,14), MAP_APPFONT).X());
            long nX(LogicToPixel(Point(11,14), MAP_APPFONT).Y());
            Point aPos(aBorderWin.GetPosPixel());
            aPos.X() = nX;
            aPos.Y() = nY;
            aBorderWin.SetPosPixel(aPos);
            aPos = pPreviewWin1->GetPosPixel();
            aPos.Y()  = nY;
            pPreviewWin1->SetPosPixel(aPos);
            aBackgroundColorBox.Hide();
            aBackgroundColorSet.SetAccessibleRelationLabeledBy(&aBackgroundColorSet);
        }
    }
}

void SvxBackgroundTabPage::ResetFromWallpaperItem( const SfxItemSet& rSet )
{
    ShowSelector();

    // condition of the preview button is persistent due to UserData
    String aUserData = GetUserData();
    aBtnPreview.Check( aUserData.Len() && sal_Unicode('1') == aUserData.GetChar( 0 ) );

    // get and evaluate Input-BrushItem
    const SvxBrushItem* pBgdAttr = NULL;
    sal_uInt16 nSlot = SID_VIEW_FLD_PIC;
    sal_uInt16 nWhich = GetWhich( nSlot );
    SvxBrushItem* pTemp = 0;
    const CntWallpaperItem* pItem = 0;

    if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
    {
        pItem = (const CntWallpaperItem*)&rSet.Get( nWhich );
        pTemp = new SvxBrushItem( *pItem, nWhich );
        pBgdAttr = pTemp;
    }

    aBtnTile.Check();

    if ( pBgdAttr )
    {
        FillControls_Impl(*pBgdAttr, aUserData);
        // brush shall be kept when showing the graphic, too
        if( aBgdColor != pBgdAttr->GetColor() )
        {
            aBgdColor = pBgdAttr->GetColor();
            sal_uInt16 nCol = GetItemId_Impl( aBackgroundColorSet, aBgdColor );
            aBackgroundColorSet.SelectItem( nCol );
            pPreviewWin1->NotifyChange( aBgdColor );
        }
    }
    else
    {
        aLbSelect.SelectEntryPos( 0 );
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( rSet, SID_VIEW_FLD_PIC );
        if ( pOld )
            aBgdColor = Color( ((CntWallpaperItem*)pOld)->GetColor() );
    }

    // We now have always a link to the background
    bLinkOnly = sal_True;
    aBtnLink.Check( sal_True );
    aBtnLink.Show( sal_False );
//  if( !pItem || !pItem->GetWallpaper(sal_False).IsBitmap() )
//      aBtnLink.Check();

    delete pTemp;
}



//------------------------------------------------------------------------

void SvxBackgroundTabPage::FillUserData()

/*  [Description]

    When destroying a SfxTabPage this virtual method is called,
    so that the TabPage can save internal information.

    In this case the condition of the preview button is saved.
*/

{
    SetUserData( aBtnPreview.IsChecked() ? rtl::OUString('1') : rtl::OUString('0') );
}

//------------------------------------------------------------------------

sal_Bool SvxBackgroundTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    if ( pPageImpl->pLoadTimer && pPageImpl->pLoadTimer->IsActive() )
    {
        pPageImpl->pLoadTimer->Stop();
        LoadTimerHdl_Impl( pPageImpl->pLoadTimer );
    }
// os: Such a nonsense! One will always find such an item somewhere,
//     but it must be existing in the rSet!

//  const SfxPoolItem* pX = GetOldItem( rCoreSet, SID_VIEW_FLD_PIC );
//  if( pX && pX->ISA(SfxWallpaperItem))
    if(SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(GetWhich(SID_VIEW_FLD_PIC), sal_False))
        return FillItemSetWithWallpaperItem( rCoreSet, SID_VIEW_FLD_PIC );

    sal_Bool bModified = sal_False;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;

    if ( aTblLBox.IsVisible() )
    {
        switch( aTblLBox.GetSelectEntryPos() )
        {
            case TBL_DEST_CELL:
                nSlot = SID_ATTR_BRUSH;
            break;
            case TBL_DEST_ROW:
                nSlot = SID_ATTR_BRUSH_ROW;
            break;
            case TBL_DEST_TBL:
                nSlot = SID_ATTR_BRUSH_TABLE;
            break;
        }
    }
    else if(aParaLBox.GetData() == &aParaLBox)
    {
        switch(aParaLBox.GetSelectEntryPos())
        {
            case PARA_DEST_PARA:
                nSlot = SID_ATTR_BRUSH;
            break;
            case PARA_DEST_CHAR:
                nSlot = SID_ATTR_BRUSH_CHAR;
            break;
        }
    }
    sal_uInt16 nWhich = GetWhich( nSlot );

    const SfxPoolItem* pOld = GetOldItem( rCoreSet, nSlot );
    SfxItemState eOldItemState = rCoreSet.GetItemState(nSlot, sal_False);
    const SfxItemSet& rOldSet = GetItemSet();

    sal_Bool bGraphTransparencyChanged = bGraphTransparency && (aGraphTransMF.GetText() != aGraphTransMF.GetSavedValue());
    if ( pOld )
    {
        const SvxBrushItem& rOldItem    = (const SvxBrushItem&)*pOld;
        SvxGraphicPosition  eOldPos     = rOldItem.GetGraphicPos();
        const sal_Bool          bIsBrush    = ( 0 == aLbSelect.GetSelectEntryPos() );

        // transparency has to be set if enabled, the color not already set to "No fill" and
        if( bColTransparency &&
            aBgdColor.GetTransparency() < 0xff)
        {
            aBgdColor.SetTransparency(lcl_PercentToTransparency(static_cast<long>(aColTransMF.GetValue())));
        }
        if (   ( (GPOS_NONE == eOldPos) && bIsBrush  )
            || ( (GPOS_NONE != eOldPos) && !bIsBrush ) ) // Brush <-> Bitmap changed?
        {
            // background art hasn't been changed:

            if ( (GPOS_NONE == eOldPos) || !aLbSelect.IsVisible() )
            {
                // Brush-treatment:
                if ( rOldItem.GetColor() != aBgdColor ||
                        (SFX_ITEM_AVAILABLE >= eOldItemState && !aBackgroundColorSet.IsNoSelection()))
                {
                    bModified = sal_True;
                    rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
                }
                else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
                    rCoreSet.ClearItem( nWhich );
            }
            else
            {
                // Bitmap-treatment:

                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const sal_Bool          bIsLink  = aBtnLink.IsChecked();
                const sal_Bool          bWasLink = (NULL != rOldItem.GetGraphicLink() );


                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                if (    bGraphTransparencyChanged ||
                        eNewPos != eOldPos
                    || bIsLink != bWasLink
                    || ( bWasLink  &&    *rOldItem.GetGraphicLink()
                                      != aBgdGraphicPath )
                    || ( !bWasLink &&    rOldItem.GetGraphic()->GetBitmap()
                                      != aBgdGraphic.GetBitmap() )
                   )
                {
                    bModified = sal_True;

                    SvxBrushItem aTmpBrush(nWhich);
                    if ( bIsLink )
                    {
                        aTmpBrush = SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                eNewPos,
                                                nWhich );
                    }
                    else
                        aTmpBrush = SvxBrushItem( aBgdGraphic,
                                        eNewPos,
                                        nWhich );
                    lcl_SetTransparency(aTmpBrush, static_cast<long>(aGraphTransMF.GetValue()));

                    rCoreSet.Put(aTmpBrush);
                }
                else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
                    rCoreSet.ClearItem( nWhich );
            }
        }
        else // Brush <-> Bitmap changed!
        {
            if ( bIsBrush )
                rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
            else
            {
                SvxBrushItem* pTmpBrush = 0;
                if ( aBtnLink.IsChecked() )
                {
                    pTmpBrush = new SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                GetGraphicPosition_Impl(),
                                                nWhich );
                }
                else
                {
                    if ( !bIsGraphicValid )
                        bIsGraphicValid = LoadLinkedGraphic_Impl();

                    if ( bIsGraphicValid )
                        pTmpBrush = new SvxBrushItem( aBgdGraphic,
                                                    GetGraphicPosition_Impl(),
                                                    nWhich );
                }
                if(pTmpBrush)
                {
                    lcl_SetTransparency(*pTmpBrush, static_cast<long>(aGraphTransMF.GetValue()));
                    rCoreSet.Put(*pTmpBrush);
                    delete pTmpBrush;
                }
            }
            bModified = ( bIsBrush || aBtnLink.IsChecked() || bIsGraphicValid );
        }
    }
    else if ( SID_ATTR_BRUSH_CHAR == nSlot && aBgdColor != Color( COL_WHITE ) )
    {
        rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
        bModified = sal_True;
    }

    if( aTblLBox.IsVisible() )
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && pTableBck_Impl->pCellBrush)
        {
            const SfxPoolItem* pOldCell =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH );

            if ( *pTableBck_Impl->pCellBrush != *pOldCell )
            {
                rCoreSet.Put( *pTableBck_Impl->pCellBrush );
                bModified |= sal_True;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_ROW && pTableBck_Impl->pRowBrush)
        {
            const SfxPoolItem* pOldRow =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH_ROW );

            if ( *pTableBck_Impl->pRowBrush != *pOldRow )
            {
                rCoreSet.Put( *pTableBck_Impl->pRowBrush );
                bModified |= sal_True;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_TABLE && pTableBck_Impl->pTableBrush)
        {
            const SfxPoolItem* pOldTable =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH_TABLE );

            if ( *pTableBck_Impl->pTableBrush != *pOldTable )
            {
                rCoreSet.Put( *pTableBck_Impl->pTableBrush );
                bModified |= sal_True;
            }
        }

        if( aTblLBox.GetSavedValue() != aTblLBox.GetSelectEntryPos() )
        {
            rCoreSet.Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                         aTblLBox.GetSelectEntryPos() ) );
            bModified |= sal_True;
        }
    }
    else if(aParaLBox.GetData() == &aParaLBox)
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && aParaLBox.IsVisible()) // not in search format dialog
        {
            const SfxPoolItem* pOldPara =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH );

            if ( *pParaBck_Impl->pParaBrush != *pOldPara )
            {
                rCoreSet.Put( *pParaBck_Impl->pParaBrush );
                bModified |= sal_True;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_CHAR )
        {
            const SfxPoolItem* pOldChar =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH_CHAR );
            DBG_ASSERT(pParaBck_Impl, "pParaBck_Impl == NULL ?");
            if ( pOldChar &&
                    //#111173#  crash report shows that pParaBck_Impl can be NULL, the cause is unknown
                    pParaBck_Impl &&
                        (*pParaBck_Impl->pCharBrush != *pOldChar ||
                *pParaBck_Impl->pCharBrush != SvxBrushItem(SID_ATTR_BRUSH_CHAR)))
            {
                rCoreSet.Put( *pParaBck_Impl->pCharBrush );
                bModified |= sal_True;
            }
        }

        if( aParaLBox.GetSavedValue() != aParaLBox.GetSelectEntryPos() )
        {
            rCoreSet.Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                         aParaLBox.GetSelectEntryPos() ) );
            bModified |= sal_True;
        }
    }
    return bModified;
}

sal_Bool SvxBackgroundTabPage::FillItemSetWithWallpaperItem( SfxItemSet& rCoreSet, sal_uInt16 nSlot)
{
    sal_uInt16 nWhich = GetWhich( nSlot );
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, nSlot );
    const SfxItemSet& rOldSet = GetItemSet();
    DBG_ASSERT(pOld,"FillItemSetWithWallpaperItem: Item not found");

    SvxBrushItem        rOldItem( (const CntWallpaperItem&)*pOld, nWhich );
    SvxGraphicPosition  eOldPos     = rOldItem.GetGraphicPos();
    const sal_Bool          bIsBrush    = ( 0 == aLbSelect.GetSelectEntryPos() );
    sal_Bool                bModified = sal_False;

    if (   ( (GPOS_NONE == eOldPos) && bIsBrush  )
        || ( (GPOS_NONE != eOldPos) && !bIsBrush ) ) // Brush <-> Bitmap changed?
    {
        // background art hasn't been changed

        if ( (GPOS_NONE == eOldPos) || !aLbSelect.IsVisible() )
        {
            // Brush-treatment:
            if ( rOldItem.GetColor() != aBgdColor )
            {
                bModified = sal_True;
                CntWallpaperItem aItem( nWhich );
                aItem.SetColor( aBgdColor );
                rCoreSet.Put( aItem );
            }
            else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
                rCoreSet.ClearItem( nWhich );
        }
        else
        {
            // Bitmap-treatment:
            SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();

            int bBitmapChanged = ( ( eNewPos != eOldPos ) ||
                                   ( *rOldItem.GetGraphicLink() != aBgdGraphicPath ) );
            int bBrushChanged = ( rOldItem.GetColor() != aBgdColor );
            if( bBitmapChanged || bBrushChanged )
            {
                bModified = sal_True;

                CntWallpaperItem aItem( nWhich );
                WallpaperStyle eWallStyle = SvxBrushItem::GraphicPos2WallpaperStyle(eNewPos);
                aItem.SetStyle( sal::static_int_cast< sal_uInt16 >( eWallStyle ) );
                aItem.SetColor( aBgdColor );
                aItem.SetBitmapURL( aBgdGraphicPath );
                rCoreSet.Put( aItem );
            }
            else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
                rCoreSet.ClearItem( nWhich );
        }
    }
    else // Brush <-> Bitmap changed!
    {
        CntWallpaperItem aItem( nWhich );
        if ( bIsBrush )
        {
            aItem.SetColor( aBgdColor );
            rCoreSet.Put( aItem );
        }
        else
        {
            WallpaperStyle eWallStyle =
                SvxBrushItem::GraphicPos2WallpaperStyle( GetGraphicPosition_Impl() );
            aItem.SetStyle( sal::static_int_cast< sal_uInt16 >( eWallStyle ) );
            aItem.SetColor( aBgdColor );
            aItem.SetBitmapURL( aBgdGraphicPath );
            rCoreSet.Put( aItem );
        }

        bModified = sal_True;
    }
    return bModified;
}

//-----------------------------------------------------------------------

int SvxBackgroundTabPage::DeactivatePage( SfxItemSet* _pSet )

/*  [Description]

    virtual method; is called on deactivation
*/

{
    if ( pPageImpl->bIsImportDlgInExecute )
        return KEEP_PAGE;

    if ( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

//-----------------------------------------------------------------------

void SvxBackgroundTabPage::PointChanged( Window* , RECT_POINT  )
{
    // has to be implemented so that position control can work
}

//-----------------------------------------------------------------------

void SvxBackgroundTabPage::ShowSelector()
{
    if( bAllowShowSelector)
    {
        aSelectTxt.Show();
        aLbSelect.Show();
        aLbSelect.SetSelectHdl( HDL(SelectHdl_Impl) );
        aBtnLink.SetClickHdl( HDL(FileClickHdl_Impl) );
        aBtnPreview.SetClickHdl( HDL(FileClickHdl_Impl) );
        aBtnBrowse.SetClickHdl( HDL(BrowseHdl_Impl) );
        aBtnArea.SetClickHdl( HDL(RadioClickHdl_Impl) );
        aBtnTile.SetClickHdl( HDL(RadioClickHdl_Impl) );
        aBtnPosition.SetClickHdl( HDL(RadioClickHdl_Impl) );

        // delayed loading via timer (because of UI-Update)
        pPageImpl->pLoadTimer = new Timer;
        pPageImpl->pLoadTimer->SetTimeout( 500 );
        pPageImpl->pLoadTimer->SetTimeoutHdl(
            LINK( this, SvxBackgroundTabPage, LoadTimerHdl_Impl ) );

        bAllowShowSelector = sal_False;

        if(nHtmlMode & HTMLMODE_ON)
        {
            aBtnArea.Enable(sal_False);
        }
    }
}

//------------------------------------------------------------------------


void SvxBackgroundTabPage::RaiseLoadError_Impl()
{
    SfxErrorContext aContext( ERRCTX_SVX_BACKGROUND,
                              String(),
                              this,
                              RID_SVXERRCTX,
                              &CUI_MGR() );

    ErrorHandler::HandleError(
        *new StringErrorInfo( ERRCODE_SVX_GRAPHIC_NOTREADABLE,
                              aBgdGraphicPath ) );
}

//------------------------------------------------------------------------

sal_Bool SvxBackgroundTabPage::LoadLinkedGraphic_Impl()
{
    sal_Bool bResult = ( aBgdGraphicPath.Len() > 0 ) &&
                   ( GRFILTER_OK == GraphicFilter::LoadGraphic( aBgdGraphicPath,
                                                 aBgdGraphicFilter,
                                                 aBgdGraphic ) );
    return bResult;
}

//------------------------------------------------------------------------


void SvxBackgroundTabPage::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorListRef pColorTable = NULL;
    const Size aSize15x15 = Size( 15, 15 );

    if ( pDocSh && ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
        pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    if ( pColorTable.is() )
    {
        short i = 0;
        long nCount = pColorTable->Count();
        XColorEntry* pEntry = NULL;
        Color aColWhite( COL_WHITE );
        String aStrWhite( EditResId( RID_SVXITEMS_COLOR_WHITE ) );
        WinBits nBits = ( aBackgroundColorSet.GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD );
        aBackgroundColorSet.SetText( SVX_RESSTR( RID_SVXSTR_TRANSPARENT ) );
        aBackgroundColorSet.SetStyle( nBits );
        aBackgroundColorSet.SetAccessibleName(aBackgroundColorBox.GetText());
        for ( i = 0; i < nCount; i++ )
        {
            pEntry = pColorTable->GetColor(i);
            aBackgroundColorSet.InsertItem( i + 1, pEntry->GetColor(), pEntry->GetName() );
        }

        while ( i < 104 )
        {
            aBackgroundColorSet.InsertItem( i + 1, aColWhite, aStrWhite );
            i++;
        }

        if ( nCount > 104 )
        {
            aBackgroundColorSet.SetStyle( nBits | WB_VSCROLL );
        }
    }

    aBackgroundColorSet.SetColCount( 8 );
    aBackgroundColorSet.SetLineCount( 13 );
    aBackgroundColorSet.CalcWindowSizePixel( aSize15x15 );

}

//------------------------------------------------------------------------



//------------------------------------------------------------------------

void SvxBackgroundTabPage::ShowColorUI_Impl()

/*  [Description]

    Hide the controls for editing the bitmap
    and show the controls for color settings instead.
*/

{
    if( !aBackgroundColorSet.IsVisible() )
    {
        aBackgroundColorSet.Show();
        aBackgroundColorBox.Show();
        aBorderWin.Show();
        pPreviewWin1->Show();
        aBtnBrowse.Hide();
        aFtFile.Hide();
        aBtnLink.Hide();
        aBtnPreview.Hide();
        aGbFile.Hide();
        aBtnPosition.Hide();
        aBtnArea.Hide();
        aBtnTile.Hide();
        aWndPosition.Hide();
        aGbPosition.Hide();
        pPreviewWin2->Hide();
        aGraphTransFL.Show(sal_False);
        aGraphTransMF.Show(sal_False);
        if(bColTransparency)
        {
            aColTransFT.Show();
            aColTransMF.Show();
        }
    }
}

//------------------------------------------------------------------------

void SvxBackgroundTabPage::ShowBitmapUI_Impl()

/*  [Description]

    Hide the the controls for color settings
    and show controls for editing the bitmap instead.
*/

{
    if ( aLbSelect.IsVisible() &&
         (
        aBackgroundColorSet.IsVisible()
         || !aBtnBrowse.IsVisible() ) )
    {
        aBackgroundColorSet.Hide();
        aBackgroundColorBox.Hide();
        aBorderWin.Hide();
        pPreviewWin1->Hide();
        aBtnBrowse.Show();
        aFtFile.Show();

        if ( !bLinkOnly && ! nHtmlMode & HTMLMODE_ON )
            aBtnLink.Show();
        aBtnPreview.Show();
        aGbFile.Show();
        aBtnPosition.Show();
        aBtnArea.Show();
        aBtnTile.Show();
        aWndPosition.Show();
        aGbPosition.Show();
        pPreviewWin2->Show();
        if(bGraphTransparency)
        {
            aGraphTransFL.Show();
            aGraphTransMF.Show();
        }
        aColTransFT.Show(sal_False);
        aColTransMF.Show(sal_False);
    }
}

//------------------------------------------------------------------------

void SvxBackgroundTabPage::SetGraphicPosition_Impl( SvxGraphicPosition ePos )
{
    switch ( ePos )
    {
        case GPOS_AREA:
        {
            aBtnArea.Check();
            aWndPosition.Disable();
        }
        break;

        case GPOS_TILED:
        {
            aBtnTile.Check();
            aWndPosition.Disable();
        }
        break;

        default:
        {
            aBtnPosition.Check();
            aWndPosition.Enable();
            RECT_POINT eNewPos = RP_MM;

            switch ( ePos )
            {
                case GPOS_MM:   break;
                case GPOS_LT:   eNewPos = RP_LT; break;
                case GPOS_MT:   eNewPos = RP_MT; break;
                case GPOS_RT:   eNewPos = RP_RT; break;
                case GPOS_LM:   eNewPos = RP_LM; break;
                case GPOS_RM:   eNewPos = RP_RM; break;
                case GPOS_LB:   eNewPos = RP_LB; break;
                case GPOS_MB:   eNewPos = RP_MB; break;
                case GPOS_RB:   eNewPos = RP_RB; break;
                default: ;//prevent warning
            }
            aWndPosition.SetActualRP( eNewPos );
        }
        break;
    }
    aWndPosition.Invalidate();
}

//------------------------------------------------------------------------

SvxGraphicPosition SvxBackgroundTabPage::GetGraphicPosition_Impl()
{
    if ( aBtnTile.IsChecked() )
        return GPOS_TILED;
    else if ( aBtnArea.IsChecked() )
        return GPOS_AREA;
    else
    {
        switch ( aWndPosition.GetActualRP() )
        {
            case RP_LT: return GPOS_LT;
            case RP_MT: return GPOS_MT;
            case RP_RT: return GPOS_RT;
            case RP_LM: return GPOS_LM;
            case RP_MM: return GPOS_MM;
            case RP_RM: return GPOS_RM;
            case RP_LB: return GPOS_LB;
            case RP_MB: return GPOS_MB;
            case RP_RB: return GPOS_RB;
        }
    }
    return GPOS_MM;
}

//-----------------------------------------------------------------------
// Handler
//-----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBackgroundTabPage, BackgroundColorHdl_Impl)
/*
    Handler, called when color selection is changed
*/
{
    sal_uInt16 nItemId = aBackgroundColorSet.GetSelectItemId();
    Color aColor = nItemId ? ( aBackgroundColorSet.GetItemColor( nItemId ) ) : Color( COL_TRANSPARENT );
    aBgdColor = aColor;
    pPreviewWin1->NotifyChange( aBgdColor );
    sal_Bool bEnableTransp = aBgdColor.GetTransparency() < 0xff;
    aColTransFT.Enable(bEnableTransp);
    aColTransMF.Enable(bEnableTransp);
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBackgroundTabPage, SelectHdl_Impl)
{
    if ( 0 == aLbSelect.GetSelectEntryPos() )
    {
        ShowColorUI_Impl();
        aParaLBox.Enable(); // drawing background can't be a bitmap
    }
    else
    {
        ShowBitmapUI_Impl();
        aParaLBox.Enable(sal_False); // drawing background can't be a bitmap
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBackgroundTabPage, FileClickHdl_Impl, CheckBox*, pBox )
{
    if ( &aBtnLink == pBox )
    {
        if ( aBtnLink.IsChecked() )
        {
            INetURLObject aObj( aBgdGraphicPath );
            String aFilePath;
            if ( aObj.GetProtocol() == INET_PROT_FILE )
                aFilePath = aObj.getFSysPath( INetURLObject::FSYS_DETECT );
            else
                aFilePath = aBgdGraphicPath;
            aFtFile.SetText( aFilePath );
        }
        else
            aFtFile.SetText( aStrUnlinked );
    }
    else if ( &aBtnPreview == pBox )
    {
        if ( aBtnPreview.IsChecked() )
        {
            if ( !bIsGraphicValid )
                bIsGraphicValid = LoadLinkedGraphic_Impl();

            if ( bIsGraphicValid )
            {
                Bitmap aBmp = aBgdGraphic.GetBitmap();
                pPreviewWin2->NotifyChange( &aBmp );
            }
            else
            {
                if ( aBgdGraphicPath.Len() > 0 ) // only for linked bitmap
                    RaiseLoadError_Impl();
                pPreviewWin2->NotifyChange( NULL );
            }
        }
        else
            pPreviewWin2->NotifyChange( NULL );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBackgroundTabPage, RadioClickHdl_Impl, RadioButton*, pBtn )
{
    if ( pBtn == &aBtnPosition )
    {
        if ( !aWndPosition.IsEnabled() )
        {
            aWndPosition.Enable();
            aWndPosition.Invalidate();
        }
    }
    else if ( aWndPosition.IsEnabled() )
    {
        aWndPosition.Disable();
        aWndPosition.Invalidate();
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBackgroundTabPage, BrowseHdl_Impl)

/*  [Description]

    Handler, called by pressing the browse button.
    Create graphic/insert dialog, set path and start.
*/

{
    if ( pPageImpl->pLoadTimer->IsActive() )
        return 0;
    sal_Bool bHtml = 0 != ( nHtmlMode & HTMLMODE_ON );

    pImportDlg = new SvxOpenGraphicDialog( aStrBrowse );
    if ( bHtml || bLinkOnly )
        pImportDlg->EnableLink(sal_False);
    pImportDlg->SetPath( aBgdGraphicPath, aBtnLink.IsChecked() );

    pPageImpl->bIsImportDlgInExecute = sal_True;
    short nErr = pImportDlg->Execute();
    pPageImpl->bIsImportDlgInExecute = sal_False;

    if( !nErr )
    {
        if ( bHtml )
            aBtnLink.Check();
        // if link isn't checked and preview isn't, either,
        // activate preview, so that the user sees which
        // graphic he has chosen
        if ( !aBtnLink.IsChecked() && !aBtnPreview.IsChecked() )
            aBtnPreview.Check( sal_True );
        // timer-delayed loading of the graphic
        pPageImpl->pLoadTimer->Start();
    }
    else
        DELETEZ( pImportDlg );
    return 0;
}

//-----------------------------------------------------------------------

IMPL_LINK( SvxBackgroundTabPage, LoadTimerHdl_Impl, Timer* , pTimer )

/*  [Description]

    Delayed loading of the graphic.
    Graphic is only loaded, if it's
    different to the current graphic.
*/

{
    if ( pTimer == pPageImpl->pLoadTimer )
    {
        pPageImpl->pLoadTimer->Stop();

        if ( pImportDlg )
        {
            INetURLObject aOld( aBgdGraphicPath );
            INetURLObject aNew( pImportDlg->GetPath() );
            if ( !aBgdGraphicPath.Len() || aNew != aOld )
            {
                // new file chosen
                aBgdGraphicPath   = pImportDlg->GetPath();
                aBgdGraphicFilter = pImportDlg->GetCurrentFilter();
                sal_Bool bLink = ( nHtmlMode & HTMLMODE_ON ) || bLinkOnly ? sal_True : pImportDlg->IsAsLink();
                aBtnLink.Check( bLink );
                aBtnLink.Enable();

                if ( aBtnPreview.IsChecked() )
                {
                    if( !pImportDlg->GetGraphic(aBgdGraphic) )
                    {
                        bIsGraphicValid = sal_True;
                    }
                    else
                    {
                        aBgdGraphicFilter.Erase();
                        aBgdGraphicPath.Erase();
                        bIsGraphicValid = sal_False;
                    }
                }
                else
                    bIsGraphicValid = sal_False; // load graphic not until preview click

                if ( aBtnPreview.IsChecked() && bIsGraphicValid )
                {
                    Bitmap aBmp = aBgdGraphic.GetBitmap();
                    pPreviewWin2->NotifyChange( &aBmp );
                }
                else
                    pPreviewWin2->NotifyChange( NULL );
            }

            FileClickHdl_Impl( &aBtnLink );
            DELETEZ( pImportDlg );
        }
    }
    return 0;
}

//-----------------------------------------------------------------------

void SvxBackgroundTabPage::ShowTblControl()
{
    aTblLBox            .SetSelectHdl( HDL(TblDestinationHdl_Impl) );
    aTblLBox            .SelectEntryPos(0);
    aTblDesc.Show();
    aTblLBox.Show();
}

//-----------------------------------------------------------------------

void SvxBackgroundTabPage::ShowParaControl(sal_Bool bCharOnly)
{
    aParaLBox.SetSelectHdl(HDL(ParaDestinationHdl_Impl));
    aParaLBox.SelectEntryPos(0);
    if(!bCharOnly)
    {
        aTblDesc.Show();
        aParaLBox.Show();
    }
    aParaLBox.SetData(&aParaLBox); // here it can be recognized that this mode is turned on
}
//-----------------------------------------------------------------------

IMPL_LINK( SvxBackgroundTabPage, TblDestinationHdl_Impl, ListBox*, pBox )
{
    sal_uInt16 nSelPos = pBox->GetSelectEntryPos();
    if( pTableBck_Impl && pTableBck_Impl->nActPos != nSelPos)
    {
        SvxBrushItem** pActItem = new (SvxBrushItem*);
        sal_uInt16 nWhich = 0;
        switch(pTableBck_Impl->nActPos)
        {
            case TBL_DEST_CELL:
                *pActItem = pTableBck_Impl->pCellBrush;
                nWhich = pTableBck_Impl->nCellWhich;
            break;
            case TBL_DEST_ROW:
                *pActItem = pTableBck_Impl->pRowBrush;
                nWhich = pTableBck_Impl->nRowWhich;
            break;
            case TBL_DEST_TBL:
                *pActItem = pTableBck_Impl->pTableBrush;
                nWhich = pTableBck_Impl->nTableWhich;
            break;
        }
        pTableBck_Impl->nActPos = nSelPos;
        if(!*pActItem)
            *pActItem = new SvxBrushItem(nWhich);
        if(0 == aLbSelect.GetSelectEntryPos())  // brush selected
        {
            **pActItem = SvxBrushItem( aBgdColor, nWhich );
        }
        else
        {
                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const sal_Bool          bIsLink  = aBtnLink.IsChecked();

                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                if ( bIsLink )
                    **pActItem = SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                eNewPos,
                                                (*pActItem)->Which() );
                else
                    **pActItem = SvxBrushItem( aBgdGraphic,
                                                eNewPos,
                                                (*pActItem)->Which() );
        }
        switch(nSelPos)
        {
            case TBL_DEST_CELL:
                *pActItem = pTableBck_Impl->pCellBrush;
                aLbSelect.Enable();
                nWhich = pTableBck_Impl->nCellWhich;
            break;
            case TBL_DEST_ROW:
            {
                if((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & HTMLMODE_SOME_STYLES))
                    aLbSelect.Disable();
                *pActItem = pTableBck_Impl->pRowBrush;
                nWhich = pTableBck_Impl->nRowWhich;
            }
            break;
            case TBL_DEST_TBL:
                *pActItem = pTableBck_Impl->pTableBrush;
                aLbSelect.Enable();
                nWhich = pTableBck_Impl->nTableWhich;
            break;
        }
        String aUserData = GetUserData();
        if(!*pActItem)
            *pActItem = new SvxBrushItem(nWhich);
        FillControls_Impl(**pActItem, aUserData);
        delete pActItem;
    }
    return 0;
}

//-----------------------------------------------------------------------

IMPL_LINK( SvxBackgroundTabPage, ParaDestinationHdl_Impl, ListBox*, pBox )
{
    sal_uInt16 nSelPos = pBox->GetSelectEntryPos();
    if( pParaBck_Impl && pParaBck_Impl->nActPos != nSelPos)
    {
        SvxBrushItem** pActItem = new (SvxBrushItem*);
        switch(pParaBck_Impl->nActPos)
        {
            case PARA_DEST_PARA:
                *pActItem = pParaBck_Impl->pParaBrush;
            break;
            case PARA_DEST_CHAR:
                *pActItem = pParaBck_Impl->pCharBrush;
            break;
        }
        pParaBck_Impl->nActPos = nSelPos;
        if(0 == aLbSelect.GetSelectEntryPos())  // brush selected
        {
            sal_uInt16 nWhich = (*pActItem)->Which();
            **pActItem = SvxBrushItem( aBgdColor, nWhich );
        }
        else
        {
                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const sal_Bool          bIsLink  = aBtnLink.IsChecked();

                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                if ( bIsLink )
                    **pActItem = SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                eNewPos,
                                                (*pActItem)->Which() );
                else
                    **pActItem = SvxBrushItem( aBgdGraphic,
                                                eNewPos,
                                                (*pActItem)->Which() );
        }
        switch(nSelPos)
        {
            case PARA_DEST_PARA:
                *pActItem = pParaBck_Impl->pParaBrush;
                aLbSelect.Enable();
            break;
            case PARA_DEST_CHAR:
            {
                *pActItem = pParaBck_Impl->pCharBrush;
                aLbSelect.Enable(sal_False);
            }
            break;
        }
        String aUserData = GetUserData();
        FillControls_Impl(**pActItem, aUserData);
        delete pActItem;
    }
    return 0;
}

//-----------------------------------------------------------------------

void SvxBackgroundTabPage::FillControls_Impl( const SvxBrushItem& rBgdAttr,
                                              const String& rUserData )
{
    SvxGraphicPosition  ePos = rBgdAttr.GetGraphicPos();
    const Color& rColor = rBgdAttr.GetColor();
    if(bColTransparency)
    {
        aColTransMF.SetValue(lcl_TransparencyToPercent(rColor.GetTransparency()));
        aColTransMF.SaveValue();
        sal_Bool bEnableTransp = rColor.GetTransparency() < 0xff;
        aColTransFT.Enable(bEnableTransp);
        aColTransMF.Enable(bEnableTransp);
        //the default setting should be "no transparency"
        if(!bEnableTransp)
            aColTransMF.SetValue(0);
    }

    if ( GPOS_NONE == ePos || !aLbSelect.IsVisible() )
    {
        aLbSelect.SelectEntryPos( 0 );
        ShowColorUI_Impl();
        Color aTrColor( COL_TRANSPARENT );
        aBgdColor = rColor;

        sal_uInt16 nCol = ( aTrColor != aBgdColor ) ?
            GetItemId_Impl( aBackgroundColorSet, aBgdColor ) : 0;

        if( aTrColor != aBgdColor && nCol == 0)
        {
            aBackgroundColorSet.SetNoSelection();
        }
        else
        {
            aBackgroundColorSet.SelectItem( nCol );
        }

        pPreviewWin1->NotifyChange( aBgdColor );
        if ( aLbSelect.IsVisible() ) // initialize graphic part
        {
            aBgdGraphicFilter.Erase();
            aBgdGraphicPath.Erase();

            if ( !rUserData.Len() )
                aBtnPreview.Check( sal_False );
            aBtnLink.Check( sal_False );
            aBtnLink.Disable();
            pPreviewWin2->NotifyChange( NULL );
            SetGraphicPosition_Impl( GPOS_TILED );  // tiles as default
        }
    }
    else
    {
        const String*   pStrLink   = rBgdAttr.GetGraphicLink();
        const String*   pStrFilter = rBgdAttr.GetGraphicFilter();

        aLbSelect.SelectEntryPos( 1 );
        ShowBitmapUI_Impl();

        if ( pStrLink )
        {
#ifdef DBG_UTIL
            INetURLObject aObj( *pStrLink );
            DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
#endif
            aBgdGraphicPath = *pStrLink;
            aBtnLink.Check( sal_True );
            aBtnLink.Enable();
        }
        else
        {
            aBgdGraphicPath.Erase();
            aBtnLink.Check( sal_False );
            aBtnLink.Disable();
        }

        if(bGraphTransparency)
        {
            const GraphicObject* pObject = rBgdAttr.GetGraphicObject();
            if(pObject)
                aGraphTransMF.SetValue(lcl_TransparencyToPercent(pObject->GetAttr().GetTransparency()));
            else
                aGraphTransMF.SetValue(0);
            aGraphTransMF.SaveValue();
        }

        FileClickHdl_Impl( &aBtnLink );

        if ( pStrFilter )
            aBgdGraphicFilter = *pStrFilter;
        else
            aBgdGraphicFilter.Erase();

        if ( !pStrLink || aBtnPreview.IsChecked() )
        {
            // Graphic exists in the item and doesn't have
            // to be loaded:

            const Graphic* pGraphic = rBgdAttr.GetGraphic();

            if ( !pGraphic && aBtnPreview.IsChecked() )
                bIsGraphicValid = LoadLinkedGraphic_Impl();
            else if ( pGraphic )
            {
                aBgdGraphic = *pGraphic;
                bIsGraphicValid = sal_True;

                if ( !rUserData.Len() )
                    aBtnPreview.Check();
            }
            else
            {
                RaiseLoadError_Impl();
                bIsGraphicValid = sal_False;

                if ( !rUserData.Len() )
                    aBtnPreview.Check( sal_False );
            }
        }

        if ( aBtnPreview.IsChecked() && bIsGraphicValid )
        {
            Bitmap aBmp = aBgdGraphic.GetBitmap();
            pPreviewWin2->NotifyChange( &aBmp );
        }
        else
            pPreviewWin2->NotifyChange( NULL );

        SetGraphicPosition_Impl( ePos );
    }
}

void SvxBackgroundTabPage::EnableTransparency(sal_Bool bColor, sal_Bool bGraphic)
{
    bColTransparency  = bColor;
    bGraphTransparency = bGraphic;
    if(bColor)
    {
        aColTransFT.Show();
        aColTransMF.Show();
    }
    if(bGraphic)
    {
        Size aRectSize(aWndPosition.GetSizePixel());
        Point aRectPos(aWndPosition.GetPosPixel());
        Point aFLPos(aGraphTransFL.GetPosPixel());
        Size aTmp(LogicToPixel(Size(RSC_SP_FLGR_SPACE_Y, RSC_SP_FLGR_SPACE_Y), MAP_APPFONT));
        long nRectHeight = aFLPos.Y() - aRectPos.Y() - aTmp.Height();
        aRectSize.Height() = nRectHeight;
        aWndPosition.SetSizePixel(aRectSize);
        aWndPosition.Invalidate();
    }
}

void SvxBackgroundTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_SHOW_TBLCTL ) == SVX_SHOW_TBLCTL )
            ShowTblControl();
        if ( ( nFlags & SVX_SHOW_PARACTL ) == SVX_SHOW_PARACTL )
            ShowParaControl();
        if ( ( nFlags & SVX_SHOW_SELECTOR ) == SVX_SHOW_SELECTOR )
            ShowSelector();
        if ( ( nFlags & SVX_ENABLE_TRANSPARENCY ) == SVX_ENABLE_TRANSPARENCY )
            EnableTransparency(sal_True, sal_True);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
