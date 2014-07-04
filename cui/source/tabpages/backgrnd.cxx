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
#include <vcl/builder.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/cntwall.hxx>
#include <sfx2/cntids.hrc>
#include <svx/dialogs.hrc>

#include <cuires.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/memberids.hrc>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>

// table background
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

#include <editeng/brushitem.hxx>
#include "backgrnd.hxx"

#include <svx/xtable.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svxerr.hxx>
#include <svx/drawitem.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <svtools/controldims.hrc>
#include <svx/flagsdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <svtools/grfmgr.hxx>

using namespace ::com::sun::star;
// static ----------------------------------------------------------------

static const sal_uInt16 pRanges[] =
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
    sal_uInt16      nCellWhich;
    sal_uInt16      nRowWhich;
    sal_uInt16      nTableWhich;
    sal_Int32       nActPos;

    SvxBackgroundTable_Impl()
        : pCellBrush(NULL)
        , pRowBrush(NULL)
        , pTableBrush(NULL)
        , nCellWhich(0)
        , nRowWhich(0)
        , nTableWhich(0)
        , nActPos(0)
    {}
};

struct SvxBackgroundPara_Impl
{
    SvxBrushItem*   pParaBrush;
    SvxBrushItem*   pCharBrush;

    sal_Int32           nActPos;

    SvxBackgroundPara_Impl()
        : pParaBrush(NULL)
        , pCharBrush(NULL)
        , nActPos(LISTBOX_ENTRY_NOTFOUND)
    {}

};

struct SvxBackgroundPage_Impl
{
    Timer*          pLoadTimer;
    bool        bIsImportDlgInExecute;

    SvxBackgroundPage_Impl()
        : pLoadTimer(NULL)
        , bIsImportDlgInExecute(false)
    {}
};

static inline sal_uInt8 lcl_PercentToTransparency(long nPercent)
{
    //0xff must not be returned!
    return sal_uInt8(nPercent ? (50 + 0xfe * nPercent) / 100 : 0);
}
static inline sal_uInt8 lcl_TransparencyToPercent(sal_uInt8 nTrans)
{
    return (nTrans * 100 + 127) / 254;
}
static void lcl_SetTransparency(SvxBrushItem& rBrush, long nTransparency)
{
    uno::Any aTransparency;
    aTransparency <<= (sal_Int8)nTransparency;
    rBrush.PutValue(aTransparency, MID_GRAPHIC_TRANSPARENCY);
}

/// Returns the fill style of the currently selected entry.
static XFillStyle lcl_getFillStyle(ListBox* pLbSelect)
{
    return (XFillStyle)(sal_uLong)pLbSelect->GetEntryData(pLbSelect->GetSelectEntryPos());
}

// Selects the entry matching the specified fill style.
static void lcl_setFillStyle(ListBox* pLbSelect, XFillStyle eStyle)
{
    for (int i = 0; i < pLbSelect->GetEntryCount(); ++i)
        if ((XFillStyle)(sal_uLong)pLbSelect->GetEntryData(i) == eStyle)
        {
            pLbSelect->SelectEntryPos(i);
            return;
        }
}


sal_uInt16 GetItemId_Impl( ValueSet& rValueSet, const Color& rCol )
{
    bool    bFound = false;
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
    BackgroundPreviewImpl(Window* pParent);
    void setBmp(bool bBmp);
    virtual ~BackgroundPreviewImpl();

    void            NotifyChange( const Color&  rColor );
    void            NotifyChange( const Bitmap* pBitmap );

protected:
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;

private:

    void recalcDrawPos();

    bool            bIsBmp;
    Bitmap*         pBitmap;
    Point           aDrawPos;
    Size            aDrawSize;
    Rectangle       aDrawRect;
    sal_uInt8            nTransparency;
};

BackgroundPreviewImpl::BackgroundPreviewImpl(Window* pParent)
    : Window(pParent, WB_BORDER)
    , bIsBmp(false)
    , pBitmap(NULL)
    , aDrawRect(Point(0,0), GetOutputSizePixel())
    , nTransparency(0)
{
    SetBorderStyle(WINDOW_BORDER_MONO);
    Paint(aDrawRect);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeBackgroundPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new BackgroundPreviewImpl(pParent);
}

void BackgroundPreviewImpl::setBmp(bool bBmp)
{
    bIsBmp = bBmp;
    Invalidate();
}



BackgroundPreviewImpl::~BackgroundPreviewImpl()
{
    delete pBitmap;
}


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

        recalcDrawPos();

        Invalidate( aDrawRect );
        Update();
    }
}

void BackgroundPreviewImpl::recalcDrawPos()
{
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
}

void BackgroundPreviewImpl::Resize()
{
    Window::Resize();
    aDrawRect = Rectangle(Point(0,0), GetOutputSizePixel());
    recalcDrawPos();
}



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

SvxBackgroundTabPage::SvxBackgroundTabPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SvxTabPage(pParent, "BackgroundPage", "cui/ui/backgroundpage.ui", rCoreSet)
    , m_pXPool(rCoreSet.GetPool())
    , m_aXFillAttr(m_pXPool)
    , m_rXFillSet(m_aXFillAttr.GetItemSet())
    , nHtmlMode(0)
    , bAllowShowSelector(true)
    , bIsGraphicValid(false)
    , bLinkOnly(false)
    , bColTransparency(false)
    , bGraphTransparency(false)
    , pPageImpl(new SvxBackgroundPage_Impl)
    , pImportDlg(NULL)
    , pTableBck_Impl(NULL)
    , pParaBck_Impl(NULL)
{
    get(m_pAsGrid, "asgrid");
    get(m_pSelectTxt, "asft");
    get(m_pLbSelect, "selectlb");
    get(m_pTblDesc, "forft");
    get(m_pTblLBox, "tablelb");
    get(m_pParaLBox, "paralb");

    get(m_pBackGroundColorFrame, "backgroundcolorframe");
    get(m_pBackgroundColorSet, "backgroundcolorset");
    get(m_pPreviewWin1, "preview1");

    get(m_pColTransFT, "transparencyft");
    get(m_pColTransMF, "transparencymf");
    get(m_pBtnPreview, "showpreview");

    // Initialize gradient controls
    get(m_pBackGroundGradientFrame, "backgroundgradientframe");
    get(m_pLbGradients, "gradientslb");
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbGradients->set_width_request(aSize.Width());
    m_pLbGradients->set_height_request(aSize.Height());
    get(m_pCtlPreview, "previewctl");
    aSize = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());

    get(m_pBitmapContainer, "graphicgrid");
    get(m_pFileFrame, "fileframe");
    get(m_pBtnBrowse, "browse");
    get(m_pBtnLink, "link");
    get(m_pFtUnlinked, "unlinkedft");
    get(m_pFtFile, "fileft");

    get(m_pTypeFrame, "typeframe");
    get(m_pBtnPosition, "positionrb");
    get(m_pBtnArea, "arearb");
    get(m_pBtnTile, "tilerb");
    get(m_pWndPosition, "windowpos");

    get(m_pGraphTransFrame, "graphtransframe");
    get(m_pGraphTransMF, "graphtransmf");

    get(m_pPreviewWin2, "preview2");
    m_pPreviewWin2->setBmp(true);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;

    if ( SFX_ITEM_SET == rCoreSet.GetItemState( SID_HTML_MODE, false, &pItem )
         || ( 0 != ( pShell = SfxObjectShell::Current()) &&
              0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
    }

    FillColorValueSets_Impl();

    m_pBackgroundColorSet->SetSelectHdl( HDL(BackgroundColorHdl_Impl) );
    m_pBackgroundColorSet->SetStyle(m_pBackgroundColorSet->GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD);
    m_pBackgroundColorSet->SetText(SVX_RESSTR(RID_SVXSTR_TRANSPARENT));
}



SvxBackgroundTabPage::~SvxBackgroundTabPage()
{
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



const sal_uInt16* SvxBackgroundTabPage::GetRanges()

/*  [Description]

    returns the area of the which-values
*/

{
    return pRanges;
}



SfxTabPage* SvxBackgroundTabPage::Create( Window* pParent,
                                          const SfxItemSet& rAttrSet )

/*  [Description]

    create method for the TabDialog
*/

{
    return ( new SvxBackgroundTabPage( pParent, rAttrSet ) );
}



void SvxBackgroundTabPage::Reset( const SfxItemSet& rSet )
{
// os: Such a nonsense! One will always find such an item somewhere,
//     but it must be existing in the rSet!
//  const SfxPoolItem* pX = GetOldItem( rSet, SID_VIEW_FLD_PIC );
//  if( pX && pX->ISA(SfxWallpaperItem))
    if(SFX_ITEM_AVAILABLE <= rSet.GetItemState(GetWhich(SID_VIEW_FLD_PIC), false))
    {
        ResetFromWallpaperItem( rSet );
        return;
    }

    // condition of the preview button is persistent due to UserData
    OUString aUserData = GetUserData();
    m_pBtnPreview->Check( !aUserData.isEmpty() && '1' == aUserData[0] );

    // don't be allowed to call ShowSelector() after reset anymore
    bAllowShowSelector = false;


    // get and evaluate Input-BrushItem
    const SvxBrushItem* pBgdAttr = NULL;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    const SfxPoolItem* pItem;
    sal_uInt16 nDestValue = USHRT_MAX;

    if ( SFX_ITEM_SET == rSet.GetItemState( SID_BACKGRND_DESTINATION,
                                            false, &pItem ) )
    {
        nDestValue = ((const SfxUInt16Item*)pItem)->GetValue();
        m_pTblLBox->SelectEntryPos(nDestValue);

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
                SID_PARA_BACKGRND_DESTINATION, false, &pItem ) )
    {
        nDestValue = ((const SfxUInt16Item*)pItem)->GetValue();
        // character activated?
        sal_Int32 nParaSel  = m_pParaLBox->GetSelectEntryPos();
        if(1 == nParaSel)
        {
            // then it was a "standard"-call
            nDestValue = nParaSel;
        }
        m_pParaLBox->SelectEntryPos(nDestValue);

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
    if(USHRT_MAX == nDestValue && (m_pParaLBox->IsVisible()||m_pTblLBox->IsVisible()))
        nDestValue = 0;
    sal_uInt16 nWhich = GetWhich( nSlot );

    if ( rSet.GetItemState( nWhich, false ) >= SFX_ITEM_AVAILABLE )
        pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );

    m_pBtnTile->Check();

    if ( pBgdAttr )
    {
        FillControls_Impl(*pBgdAttr, aUserData);
        aBgdColor = ( (SvxBrushItem*)pBgdAttr )->GetColor();
    }
    else
    {
        m_pSelectTxt->Hide();
        m_pLbSelect->Hide();
        lcl_setFillStyle(m_pLbSelect, XFILL_SOLID);
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_BRUSH );

        if ( pOld )
            aBgdColor = ( (SvxBrushItem*)pOld )->GetColor();
    }

    if ( nDestValue != USHRT_MAX )
    {
        if(m_pTblLBox->IsVisible())
        {
            sal_Int32 nValue = m_pTblLBox->GetSelectEntryPos();

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
            if ( rSet.GetItemState( nWhich, false ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pTableBck_Impl->pCellBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nCellWhich = nWhich;

            if ( rSet.GetItemState( SID_ATTR_BRUSH_ROW, false ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( SID_ATTR_BRUSH_ROW ) );
                pTableBck_Impl->pRowBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nRowWhich = SID_ATTR_BRUSH_ROW;

            if ( rSet.GetItemState( SID_ATTR_BRUSH_TABLE, false ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( SID_ATTR_BRUSH_TABLE ) );
                pTableBck_Impl->pTableBrush = new SvxBrushItem(*pBgdAttr);
            }
            pTableBck_Impl->nTableWhich = SID_ATTR_BRUSH_TABLE;

            TblDestinationHdl_Impl(m_pTblLBox);
            m_pTblLBox->SaveValue();
        }
        else
        {
            sal_Int32 nValue = m_pParaLBox->GetSelectEntryPos();

            if ( pParaBck_Impl )
            {
                delete pParaBck_Impl->pParaBrush;
                delete pParaBck_Impl->pCharBrush;
            }
            else
                pParaBck_Impl = new SvxBackgroundPara_Impl();

            pParaBck_Impl->nActPos = nValue;

            nWhich = GetWhich( SID_ATTR_BRUSH );
            if ( rSet.GetItemState( nWhich, false ) >= SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pParaBck_Impl->pParaBrush = new SvxBrushItem(*pBgdAttr);
            }

            nWhich = GetWhich( SID_ATTR_BRUSH_CHAR );
            rSet.GetItemState( nWhich, true );
            rSet.GetItemState( nWhich, false );
            if ( rSet.GetItemState( nWhich, true ) > SFX_ITEM_AVAILABLE )
            {
                pBgdAttr = (const SvxBrushItem*)&( rSet.Get( nWhich ) );
                pParaBck_Impl->pCharBrush = new SvxBrushItem(*pBgdAttr);
            }
            else
                pParaBck_Impl->pCharBrush = new SvxBrushItem(SID_ATTR_BRUSH_CHAR);

            ParaDestinationHdl_Impl(m_pParaLBox);
            m_pParaLBox->SaveValue();
        }
    }
}

void SvxBackgroundTabPage::ResetFromWallpaperItem( const SfxItemSet& rSet )
{
    ShowSelector();

    // condition of the preview button is persistent due to UserData
    OUString aUserData = GetUserData();
    m_pBtnPreview->Check( !aUserData.isEmpty() && '1' == aUserData[0] );

    // get and evaluate Input-BrushItem
    const SvxBrushItem* pBgdAttr = NULL;
    sal_uInt16 nSlot = SID_VIEW_FLD_PIC;
    sal_uInt16 nWhich = GetWhich( nSlot );
    SvxBrushItem* pTemp = 0;

    if ( rSet.GetItemState( nWhich, false ) >= SFX_ITEM_AVAILABLE )
    {
        const CntWallpaperItem* pItem = (const CntWallpaperItem*)&rSet.Get( nWhich );
        pTemp = new SvxBrushItem( *pItem, nWhich );
        pBgdAttr = pTemp;
    }

    m_pBtnTile->Check();

    if ( pBgdAttr )
    {
        FillControls_Impl(*pBgdAttr, aUserData);
        // brush shall be kept when showing the graphic, too
        if( aBgdColor != pBgdAttr->GetColor() )
        {
            aBgdColor = pBgdAttr->GetColor();
            sal_uInt16 nCol = GetItemId_Impl(*m_pBackgroundColorSet, aBgdColor);
            m_pBackgroundColorSet->SelectItem( nCol );
            m_pPreviewWin1->NotifyChange( aBgdColor );
        }
    }
    else
    {
        lcl_setFillStyle(m_pLbSelect, XFILL_SOLID);
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( rSet, SID_VIEW_FLD_PIC );
        if ( pOld )
            aBgdColor = Color( ((CntWallpaperItem*)pOld)->GetColor() );
    }

    // We now have always a link to the background
    bLinkOnly = true;
    m_pBtnLink->Check( true );
    m_pBtnLink->Show( false );

    delete pTemp;
}





void SvxBackgroundTabPage::FillUserData()

/*  [Description]

    When destroying a SfxTabPage this virtual method is called,
    so that the TabPage can save internal information.

    In this case the condition of the preview button is saved.
*/

{
    SetUserData( m_pBtnPreview->IsChecked() ? OUString('1') : OUString('0') );
}



bool SvxBackgroundTabPage::FillItemSet( SfxItemSet& rCoreSet )
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
    if(SFX_ITEM_AVAILABLE <= rCoreSet.GetItemState(GetWhich(SID_VIEW_FLD_PIC), false))
        return FillItemSetWithWallpaperItem( rCoreSet, SID_VIEW_FLD_PIC );

    bool bModified = false;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;

    if ( m_pTblLBox->IsVisible() )
    {
        switch( m_pTblLBox->GetSelectEntryPos() )
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
    else if (m_pParaLBox->GetData() == m_pParaLBox)
    {
        switch(m_pParaLBox->GetSelectEntryPos())
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
    SfxItemState eOldItemState = rCoreSet.GetItemState(nSlot, false);
    const SfxItemSet& rOldSet = GetItemSet();

    bool bGraphTransparencyChanged = bGraphTransparency && m_pGraphTransMF->IsValueChangedFromSaved();
    if ( pOld )
    {
        const SvxBrushItem& rOldItem    = (const SvxBrushItem&)*pOld;
        SvxGraphicPosition  eOldPos     = rOldItem.GetGraphicPos();
        const bool          bIsBrush    = ( XFILL_SOLID == lcl_getFillStyle(m_pLbSelect) );
        const bool bIsGradient = ( XFILL_GRADIENT == lcl_getFillStyle(m_pLbSelect) );

        // transparency has to be set if enabled, the color not already set to "No fill" and
        if( bColTransparency &&
            aBgdColor.GetTransparency() < 0xff)
        {
            aBgdColor.SetTransparency(lcl_PercentToTransparency(static_cast<long>(m_pColTransMF->GetValue())));
        }
        if (   ( (GPOS_NONE == eOldPos) && (bIsBrush || bIsGradient)  )
            || ( (GPOS_NONE != eOldPos) && !(bIsBrush || bIsGradient) ) ) // Brush <-> Bitmap changed?
        {
            // background art hasn't been changed:

            if ( (GPOS_NONE == eOldPos) || !m_pLbSelect->IsVisible() )
            {
                if (bIsBrush)
                {
                    // Brush-treatment:
                    if ( rOldItem.GetColor() != aBgdColor ||
                            (SFX_ITEM_AVAILABLE >= eOldItemState && !m_pBackgroundColorSet->IsNoSelection()))
                    {
                        bModified = true;
                        rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
                    }
                    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, false ) )
                        rCoreSet.ClearItem( nWhich );
                    // Handle XFILL_GRADIENT -> XFILL_SOLID
                    XFillStyleItem aFillStyleItem(XFILL_SOLID, GetWhich(SID_SW_ATTR_FILL_STYLE));
                    rCoreSet.Put(aFillStyleItem);
                }
                else
                {
                    XFillStyleItem aFillStyleItem(((const XFillStyleItem&)m_rXFillSet.Get(XATTR_FILLSTYLE)).GetValue(), GetWhich(SID_SW_ATTR_FILL_STYLE));
                    rCoreSet.Put(aFillStyleItem);

                    const XFillGradientItem& rFillGradientItem = (const XFillGradientItem&)m_rXFillSet.Get(XATTR_FILLGRADIENT);
                    XFillGradientItem aFillGradientItem(rFillGradientItem.GetName(), rFillGradientItem.GetGradientValue(), GetWhich(SID_SW_ATTR_FILL_GRADIENT));
                    rCoreSet.Put(aFillGradientItem);
                }
            }
            else
            {
                // Bitmap-treatment:

                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const bool          bIsLink  = m_pBtnLink->IsChecked();
                const bool          bWasLink = (NULL != rOldItem.GetGraphicLink() );


                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                if (   bGraphTransparencyChanged ||
                       eNewPos != eOldPos
                    || bIsLink != bWasLink
                    || ( bWasLink  && rOldItem.GetGraphicLink()
                                       != aBgdGraphicPath )
                    || ( !bWasLink && rOldItem.GetGraphic()->GetBitmap()
                                       != aBgdGraphic.GetBitmap() )
                   )
                {
                    bModified = true;

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
                    lcl_SetTransparency(aTmpBrush, static_cast<long>(m_pGraphTransMF->GetValue()));

                    rCoreSet.Put(aTmpBrush);
                }
                else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, false ) )
                    rCoreSet.ClearItem( nWhich );
            }
        }
        else // Brush <-> Bitmap changed!
        {
            if (bIsBrush || bIsGradient)
            {
                rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
                if (bIsGradient)
                {
                    // Handle XFILL_BITMAP -> XFILL_GRADIENT
                    XFillStyleItem aFillStyleItem(((const XFillStyleItem&)m_rXFillSet.Get(XATTR_FILLSTYLE)).GetValue(), GetWhich(SID_SW_ATTR_FILL_STYLE));
                    rCoreSet.Put(aFillStyleItem);

                    const XFillGradientItem& rFillGradientItem = (const XFillGradientItem&)m_rXFillSet.Get(XATTR_FILLGRADIENT);
                    XFillGradientItem aFillGradientItem(rFillGradientItem.GetName(), rFillGradientItem.GetGradientValue(), GetWhich(SID_SW_ATTR_FILL_GRADIENT));
                    rCoreSet.Put(aFillGradientItem);
                }
            }
            else
            {
                SvxBrushItem* pTmpBrush = 0;
                if ( m_pBtnLink->IsChecked() )
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
                    lcl_SetTransparency(*pTmpBrush, static_cast<long>(m_pGraphTransMF->GetValue()));
                    rCoreSet.Put(*pTmpBrush);
                    delete pTmpBrush;
                }
            }
            bModified = ( bIsBrush || bIsGradient || m_pBtnLink->IsChecked() || bIsGraphicValid );
        }
    }
    else if ( SID_ATTR_BRUSH_CHAR == nSlot && aBgdColor != Color( COL_WHITE ) )
    {
        rCoreSet.Put( SvxBrushItem( aBgdColor, nWhich ) );
        bModified = true;
    }

    if( m_pTblLBox->IsVisible() )
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && pTableBck_Impl->pCellBrush)
        {
            const SfxPoolItem* pOldCell =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH );

            if ( *pTableBck_Impl->pCellBrush != *pOldCell )
            {
                rCoreSet.Put( *pTableBck_Impl->pCellBrush );
                bModified |= true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_ROW && pTableBck_Impl->pRowBrush)
        {
            const SfxPoolItem* pOldRow =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH_ROW );

            if ( *pTableBck_Impl->pRowBrush != *pOldRow )
            {
                rCoreSet.Put( *pTableBck_Impl->pRowBrush );
                bModified |= true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_TABLE && pTableBck_Impl->pTableBrush)
        {
            const SfxPoolItem* pOldTable =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH_TABLE );

            if ( *pTableBck_Impl->pTableBrush != *pOldTable )
            {
                rCoreSet.Put( *pTableBck_Impl->pTableBrush );
                bModified |= true;
            }
        }

        if( m_pTblLBox->IsValueChangedFromSaved() )
        {
            rCoreSet.Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                         m_pTblLBox->GetSelectEntryPos() ) );
            bModified |= true;
        }
    }
    else if (m_pParaLBox->GetData() == m_pParaLBox)
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && m_pParaLBox->IsVisible()) // not in search format dialog
        {
            const SfxPoolItem* pOldPara =
                GetOldItem( rCoreSet, SID_ATTR_BRUSH );

            if ( *pParaBck_Impl->pParaBrush != *pOldPara )
            {
                rCoreSet.Put( *pParaBck_Impl->pParaBrush );
                bModified |= true;
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
                bModified |= true;
            }
        }

        if( m_pParaLBox->IsValueChangedFromSaved() )
        {
            rCoreSet.Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                         m_pParaLBox->GetSelectEntryPos() ) );
            bModified |= true;
        }
    }
    return bModified;
}

bool SvxBackgroundTabPage::FillItemSetWithWallpaperItem( SfxItemSet& rCoreSet, sal_uInt16 nSlot)
{
    sal_uInt16 nWhich = GetWhich( nSlot );
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, nSlot );
    DBG_ASSERT(pOld,"FillItemSetWithWallpaperItem: Item not found");
    if (!pOld)
        return false;
    const SfxItemSet& rOldSet = GetItemSet();

    SvxBrushItem        rOldItem( (const CntWallpaperItem&)*pOld, nWhich );
    SvxGraphicPosition  eOldPos     = rOldItem.GetGraphicPos();
    const bool          bIsBrush    = ( XFILL_SOLID == lcl_getFillStyle(m_pLbSelect) );
    bool                bModified = false;

    if (   ( (GPOS_NONE == eOldPos) && bIsBrush  )
        || ( (GPOS_NONE != eOldPos) && !bIsBrush ) ) // Brush <-> Bitmap changed?
    {
        // background art hasn't been changed

        if ( (GPOS_NONE == eOldPos) || !m_pLbSelect->IsVisible() )
        {
            // Brush-treatment:
            if ( rOldItem.GetColor() != aBgdColor )
            {
                bModified = true;
                CntWallpaperItem aItem( nWhich );
                aItem.SetColor( aBgdColor );
                rCoreSet.Put( aItem );
            }
            else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, false ) )
                rCoreSet.ClearItem( nWhich );
        }
        else
        {
            // Bitmap-treatment:
            SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();

            bool bBitmapChanged = ( ( eNewPos != eOldPos ) ||
                                   ( rOldItem.GetGraphicLink() != aBgdGraphicPath ) );
            bool bBrushChanged = ( rOldItem.GetColor() != aBgdColor );
            if( bBitmapChanged || bBrushChanged )
            {
                bModified = true;

                CntWallpaperItem aItem( nWhich );
                WallpaperStyle eWallStyle = SvxBrushItem::GraphicPos2WallpaperStyle(eNewPos);
                aItem.SetStyle( sal::static_int_cast< sal_uInt16 >( eWallStyle ) );
                aItem.SetColor( aBgdColor );
                aItem.SetBitmapURL( aBgdGraphicPath );
                rCoreSet.Put( aItem );
            }
            else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, false ) )
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

        bModified = true;
    }
    return bModified;
}



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



void SvxBackgroundTabPage::PointChanged( Window* , RECT_POINT  )
{
    // has to be implemented so that position control can work
}



void SvxBackgroundTabPage::ShowSelector()
{
    if( bAllowShowSelector)
    {
        m_pAsGrid->Show();
        m_pSelectTxt->Show();
        m_pLbSelect->Show();
        m_pLbSelect->SetSelectHdl( HDL(SelectHdl_Impl) );
        m_pBtnLink->SetClickHdl( HDL(FileClickHdl_Impl) );
        m_pBtnPreview->SetClickHdl( HDL(FileClickHdl_Impl) );
        m_pBtnBrowse->SetClickHdl( HDL(BrowseHdl_Impl) );
        m_pBtnArea->SetClickHdl( HDL(RadioClickHdl_Impl) );
        m_pBtnTile->SetClickHdl( HDL(RadioClickHdl_Impl) );
        m_pBtnPosition->SetClickHdl( HDL(RadioClickHdl_Impl) );
        m_pLbGradients->SetSelectHdl(HDL(ModifyGradientHdl_Impl));

        // delayed loading via timer (because of UI-Update)
        pPageImpl->pLoadTimer = new Timer;
        pPageImpl->pLoadTimer->SetTimeout( 500 );
        pPageImpl->pLoadTimer->SetTimeoutHdl(
            LINK( this, SvxBackgroundTabPage, LoadTimerHdl_Impl ) );

        bAllowShowSelector = false;

        if(nHtmlMode & HTMLMODE_ON)
        {
            m_pBtnArea->Enable(false);
        }
    }
}




void SvxBackgroundTabPage::RaiseLoadError_Impl()
{
    SfxErrorContext aContext( ERRCTX_SVX_BACKGROUND,
                              OUString(),
                              this,
                              RID_SVXERRCTX,
                              &CUI_MGR() );

    ErrorHandler::HandleError(
        *new StringErrorInfo( ERRCODE_SVX_GRAPHIC_NOTREADABLE,
                              aBgdGraphicPath ) );
}



bool SvxBackgroundTabPage::LoadLinkedGraphic_Impl()
{
    bool bResult = ( !aBgdGraphicPath.isEmpty() ) &&
                   ( GRFILTER_OK == GraphicFilter::LoadGraphic( aBgdGraphicPath,
                                                 aBgdGraphicFilter,
                                                 aBgdGraphic ) );
    return bResult;
}




void SvxBackgroundTabPage::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorListRef pColorTable = NULL;
    if ( pDocSh && ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
    {
        pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    if ( pColorTable.is() )
    {
        m_pBackgroundColorSet->Clear();
        m_pBackgroundColorSet->addEntriesForXColorList(*pColorTable);
    }

    const WinBits nBits(m_pBackgroundColorSet->GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD);
    m_pBackgroundColorSet->SetStyle(nBits);
    m_pBackgroundColorSet->SetColCount(m_pBackgroundColorSet->getColumnCount());
}







void SvxBackgroundTabPage::ShowColorUI_Impl()

/*  [Description]

    Hide the controls for editing the bitmap
    and show the controls for color settings instead.
*/

{
    if (!m_pBackGroundColorFrame->IsVisible())
    {
        HideBitmapUI_Impl();
        HideGradientUI_Impl();
        m_pBackGroundColorFrame->Show();

        if(bColTransparency)
        {
            m_pColTransFT->Show();
            m_pColTransMF->Show();
        }
    }
}

void SvxBackgroundTabPage::HideColorUI_Impl()
{
        m_pBackGroundColorFrame->Hide();
}



void SvxBackgroundTabPage::ShowBitmapUI_Impl()

/*  [Description]

    Hide the controls for color settings
    and show controls for editing the bitmap instead.
*/

{
    if (m_pLbSelect->IsVisible() &&
         (m_pBackGroundColorFrame->IsVisible() || !m_pFileFrame->IsVisible()))
    {
        HideColorUI_Impl();
        HideGradientUI_Impl();


        m_pBitmapContainer->Show();

        m_pFileFrame->Show();
        m_pBtnLink->Show(!bLinkOnly && !(nHtmlMode & HTMLMODE_ON));

        m_pTypeFrame->Show();

        m_pPreviewWin2->Show();
        m_pBtnPreview->Show();

        m_pGraphTransFrame->Show(bGraphTransparency);
        m_pColTransFT->Show(false);
        m_pColTransMF->Show(false);
    }
}

void SvxBackgroundTabPage::HideBitmapUI_Impl()
{
    m_pBitmapContainer->Hide();
    m_pFileFrame->Hide();
    m_pTypeFrame->Hide();
    m_pPreviewWin2->Hide();
    m_pBtnPreview->Hide();
    m_pGraphTransFrame->Hide();
}

void SvxBackgroundTabPage::ShowGradientUI_Impl()
{
    if (!m_pBackGroundGradientFrame->IsVisible())
    {
        HideColorUI_Impl();
        HideBitmapUI_Impl();

        m_pBackGroundGradientFrame->Show();
        if (!m_rXFillSet.HasItem(XATTR_FILLSTYLE) || ((const XFillStyleItem&)m_rXFillSet.Get(XATTR_FILLSTYLE)).GetValue() != XFILL_GRADIENT)
        {
            // Frame has no gradient? Then select the first one, just to be able to show something in the preview control.
            m_pLbGradients->SelectEntryPos(0);
            ModifyGradientHdl_Impl(this);
        }
        else
        {
            // It has one, try to select the matching entry in the gradient list box.
            const XFillGradientItem& rFillGradientItem = (const XFillGradientItem&)m_rXFillSet.Get(XATTR_FILLGRADIENT);
            m_pLbGradients->SelectEntryByList(m_pGradientList, rFillGradientItem.GetName(), rFillGradientItem.GetGradientValue());
        }
    }
}

void SvxBackgroundTabPage::HideGradientUI_Impl()
{
    m_pBackGroundGradientFrame->Hide();
}



void SvxBackgroundTabPage::SetGraphicPosition_Impl( SvxGraphicPosition ePos )
{
    switch ( ePos )
    {
        case GPOS_AREA:
        {
            m_pBtnArea->Check();
            m_pWndPosition->Disable();
        }
        break;

        case GPOS_TILED:
        {
            m_pBtnTile->Check();
            m_pWndPosition->Disable();
        }
        break;

        default:
        {
            m_pBtnPosition->Check();
            m_pWndPosition->Enable();
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
            m_pWndPosition->SetActualRP( eNewPos );
        }
        break;
    }
    m_pWndPosition->Invalidate();
}



SvxGraphicPosition SvxBackgroundTabPage::GetGraphicPosition_Impl()
{
    if ( m_pBtnTile->IsChecked() )
        return GPOS_TILED;
    else if ( m_pBtnArea->IsChecked() )
        return GPOS_AREA;
    else
    {
        switch ( m_pWndPosition->GetActualRP() )
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


// Handler


IMPL_LINK_NOARG(SvxBackgroundTabPage, BackgroundColorHdl_Impl)
/*
    Handler, called when color selection is changed
*/
{
    sal_uInt16 nItemId = m_pBackgroundColorSet->GetSelectItemId();
    Color aColor = nItemId ? ( m_pBackgroundColorSet->GetItemColor( nItemId ) ) : Color( COL_TRANSPARENT );
    aBgdColor = aColor;
    m_pPreviewWin1->NotifyChange( aBgdColor );
    bool bEnableTransp = aBgdColor.GetTransparency() < 0xff;
    m_pColTransFT->Enable(bEnableTransp);
    m_pColTransMF->Enable(bEnableTransp);
    return 0;
}



IMPL_LINK_NOARG(SvxBackgroundTabPage, SelectHdl_Impl)
{
    if ( XFILL_SOLID == lcl_getFillStyle(m_pLbSelect) )
    {
        ShowColorUI_Impl();
        m_pParaLBox->Enable(); // drawing background can't be a bitmap
    }
    else if ( XFILL_BITMAP == lcl_getFillStyle(m_pLbSelect) )
    {
        ShowBitmapUI_Impl();
        m_pParaLBox->Enable(false); // drawing background can't be a bitmap
    }
    else
    {
        ShowGradientUI_Impl();
    }
    return 0;
}



IMPL_LINK( SvxBackgroundTabPage, FileClickHdl_Impl, CheckBox*, pBox )
{
    if (m_pBtnLink == pBox)
    {
        if ( m_pBtnLink->IsChecked() )
        {
            m_pFtUnlinked->Hide();
            INetURLObject aObj( aBgdGraphicPath );
            OUString aFilePath;
            if ( aObj.GetProtocol() == INET_PROT_FILE )
                aFilePath = aObj.getFSysPath( INetURLObject::FSYS_DETECT );
            else
                aFilePath = aBgdGraphicPath;
            m_pFtFile->SetText( aFilePath );
            m_pFtFile->Show();
        }
        else
        {
            m_pFtUnlinked->Show();
            m_pFtFile->Hide();
        }
    }
    else if (m_pBtnPreview == pBox)
    {
        if ( m_pBtnPreview->IsChecked() )
        {
            if ( !bIsGraphicValid )
                bIsGraphicValid = LoadLinkedGraphic_Impl();

            if ( bIsGraphicValid )
            {
                Bitmap aBmp = aBgdGraphic.GetBitmap();
                m_pPreviewWin2->NotifyChange( &aBmp );
            }
            else
            {
                if ( !aBgdGraphicPath.isEmpty() ) // only for linked bitmap
                    RaiseLoadError_Impl();
                m_pPreviewWin2->NotifyChange( NULL );
            }
        }
        else
            m_pPreviewWin2->NotifyChange( NULL );
    }
    return 0;
}



IMPL_LINK( SvxBackgroundTabPage, RadioClickHdl_Impl, RadioButton*, pBtn )
{
    if (pBtn == m_pBtnPosition)
    {
        if ( !m_pWndPosition->IsEnabled() )
        {
            m_pWndPosition->Enable();
            m_pWndPosition->Invalidate();
        }
    }
    else if ( m_pWndPosition->IsEnabled() )
    {
        m_pWndPosition->Disable();
        m_pWndPosition->Invalidate();
    }
    return 0;
}

IMPL_LINK_NOARG(SvxBackgroundTabPage, ModifyGradientHdl_Impl)
{
    sal_Int32 nPos = m_pLbGradients->GetSelectEntryPos();

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        XGradientEntry* pEntry = m_pGradientList->GetGradient(nPos);
        m_rXFillSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        m_rXFillSet.Put( XFillGradientItem( pEntry->GetName(), pEntry->GetGradient() ) );
    }
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
    return 0;
}



IMPL_LINK_NOARG(SvxBackgroundTabPage, BrowseHdl_Impl)

/*  [Description]

    Handler, called by pressing the browse button.
    Create graphic/insert dialog, set path and start.
*/

{
    if ( pPageImpl->pLoadTimer->IsActive() )
        return 0;
    bool bHtml = 0 != ( nHtmlMode & HTMLMODE_ON );

    OUString aStrBrowse(get<Window>("findgraphicsft")->GetText());
    pImportDlg = new SvxOpenGraphicDialog( aStrBrowse );
    if ( bHtml || bLinkOnly )
        pImportDlg->EnableLink(false);
    pImportDlg->SetPath( aBgdGraphicPath, m_pBtnLink->IsChecked() );

    pPageImpl->bIsImportDlgInExecute = true;
    short nErr = pImportDlg->Execute();
    pPageImpl->bIsImportDlgInExecute = false;

    if( !nErr )
    {
        if ( bHtml )
            m_pBtnLink->Check();
        // if link isn't checked and preview isn't, either,
        // activate preview, so that the user sees which
        // graphic he has chosen
        if ( !m_pBtnLink->IsChecked() && !m_pBtnPreview->IsChecked() )
            m_pBtnPreview->Check( true );
        // timer-delayed loading of the graphic
        pPageImpl->pLoadTimer->Start();
    }
    else
        DELETEZ( pImportDlg );
    return 0;
}



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
            if ( aBgdGraphicPath.isEmpty() || aNew != aOld )
            {
                // new file chosen
                aBgdGraphicPath   = pImportDlg->GetPath();
                aBgdGraphicFilter = pImportDlg->GetCurrentFilter();
                bool bLink = ( nHtmlMode & HTMLMODE_ON ) || bLinkOnly ? sal_True : pImportDlg->IsAsLink();
                m_pBtnLink->Check( bLink );
                m_pBtnLink->Enable();

                if ( m_pBtnPreview->IsChecked() )
                {
                    if( !pImportDlg->GetGraphic(aBgdGraphic) )
                    {
                        bIsGraphicValid = true;
                    }
                    else
                    {
                        aBgdGraphicFilter = "";
                        aBgdGraphicPath = "";
                        bIsGraphicValid = false;
                    }
                }
                else
                    bIsGraphicValid = false; // load graphic not until preview click

                if ( m_pBtnPreview->IsChecked() && bIsGraphicValid )
                {
                    Bitmap aBmp = aBgdGraphic.GetBitmap();
                    m_pPreviewWin2->NotifyChange( &aBmp );
                }
                else
                    m_pPreviewWin2->NotifyChange( NULL );
            }

            FileClickHdl_Impl(m_pBtnLink);
            DELETEZ( pImportDlg );
        }
    }
    return 0;
}



void SvxBackgroundTabPage::ShowTblControl()
{
    m_pTblLBox->SetSelectHdl( HDL(TblDestinationHdl_Impl) );
    m_pTblLBox->SelectEntryPos(0);
    m_pTblDesc->Show();
    m_pTblLBox->Show();
    m_pAsGrid->Show();
}



void SvxBackgroundTabPage::ShowParaControl(bool bCharOnly)
{
    m_pParaLBox->SetSelectHdl(HDL(ParaDestinationHdl_Impl));
    m_pParaLBox->SelectEntryPos(0);
    if (!bCharOnly)
    {
        m_pTblDesc->Show();
        m_pParaLBox->Show();
        m_pAsGrid->Show();
    }
    m_pParaLBox->SetData(m_pParaLBox); // here it can be recognized that this mode is turned on
}


IMPL_LINK( SvxBackgroundTabPage, TblDestinationHdl_Impl, ListBox*, pBox )
{
    sal_Int32 nSelPos = pBox->GetSelectEntryPos();
    if( pTableBck_Impl && pTableBck_Impl->nActPos != nSelPos)
    {
        boost::scoped_ptr<SvxBrushItem> xItemHolder;
        SvxBrushItem* pActItem = NULL;
        sal_uInt16 nWhich = 0;
        switch(pTableBck_Impl->nActPos)
        {
        case TBL_DEST_CELL:
            pActItem = pTableBck_Impl->pCellBrush;
            nWhich = pTableBck_Impl->nCellWhich;
            break;
        case TBL_DEST_ROW:
            pActItem = pTableBck_Impl->pRowBrush;
            nWhich = pTableBck_Impl->nRowWhich;
            break;
        case TBL_DEST_TBL:
            pActItem = pTableBck_Impl->pTableBrush;
            nWhich = pTableBck_Impl->nTableWhich;
            break;
        default:
            pActItem = NULL;
            break;
        }
        pTableBck_Impl->nActPos = nSelPos;
        if(!pActItem)
        {
            xItemHolder.reset(new SvxBrushItem(nWhich));
            pActItem = xItemHolder.get();
        }
        if(XFILL_SOLID == lcl_getFillStyle(m_pLbSelect))  // brush selected
        {
            *pActItem = SvxBrushItem( aBgdColor, nWhich );
        }
        else
        {
            SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
            const bool          bIsLink  = m_pBtnLink->IsChecked();

            if ( !bIsLink && !bIsGraphicValid )
                bIsGraphicValid = LoadLinkedGraphic_Impl();

            if ( bIsLink )
                *pActItem = SvxBrushItem( aBgdGraphicPath,
                                            aBgdGraphicFilter,
                                            eNewPos,
                                            pActItem->Which() );
            else
                *pActItem = SvxBrushItem( aBgdGraphic,
                                            eNewPos,
                                            pActItem->Which() );
        }
        switch(nSelPos)
        {
        case TBL_DEST_CELL:
            pActItem = pTableBck_Impl->pCellBrush;
            m_pLbSelect->Enable();
            nWhich = pTableBck_Impl->nCellWhich;
            break;
        case TBL_DEST_ROW:
            if((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & HTMLMODE_SOME_STYLES))
                m_pLbSelect->Disable();
            pActItem = pTableBck_Impl->pRowBrush;
            nWhich = pTableBck_Impl->nRowWhich;
            break;
        case TBL_DEST_TBL:
            pActItem = pTableBck_Impl->pTableBrush;
            m_pLbSelect->Enable();
            nWhich = pTableBck_Impl->nTableWhich;
            break;
        default:
            // The item will be new'ed again below, but that will be the
            // default item then, not an existing modified one.
            xItemHolder.reset();
            pActItem = NULL;
            break;
        }
        OUString aUserData = GetUserData();
        if (!pActItem)
        {
            xItemHolder.reset(new SvxBrushItem(nWhich));
            pActItem = xItemHolder.get();
        }
        FillControls_Impl(*pActItem, aUserData);
    }
    return 0;
}



IMPL_LINK( SvxBackgroundTabPage, ParaDestinationHdl_Impl, ListBox*, pBox )
{
    sal_Int32 nSelPos = pBox->GetSelectEntryPos();
    if( pParaBck_Impl && pParaBck_Impl->nActPos != nSelPos)
    {
        SvxBrushItem* pActItem = NULL;
        switch(pParaBck_Impl->nActPos)
        {
        case PARA_DEST_PARA:
            pActItem = pParaBck_Impl->pParaBrush;
            break;
        case PARA_DEST_CHAR:
            pActItem = pParaBck_Impl->pCharBrush;
            break;
        default:
            /* we assert here because the rest of the code expect pActItem to be non NULL */
            assert(false);
            return 0;
        }
        pParaBck_Impl->nActPos = nSelPos;
        if(XFILL_SOLID == lcl_getFillStyle(m_pLbSelect))  // brush selected
        {
            sal_uInt16 nWhich = pActItem->Which();
            *pActItem = SvxBrushItem( aBgdColor, nWhich );
        }
        else
        {
                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const bool          bIsLink  = m_pBtnLink->IsChecked();

                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                if ( bIsLink )
                    *pActItem = SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                eNewPos,
                                                pActItem->Which() );
                else
                    *pActItem = SvxBrushItem( aBgdGraphic,
                                                eNewPos,
                                                pActItem->Which() );
        }
        switch(nSelPos)
        {
            case PARA_DEST_PARA:
                pActItem = pParaBck_Impl->pParaBrush;
                m_pLbSelect->Enable();
            break;
            case PARA_DEST_CHAR:
            {
                pActItem = pParaBck_Impl->pCharBrush;
                m_pLbSelect->Enable(false);
            }
            break;
        }
        OUString aUserData = GetUserData();
        FillControls_Impl(*pActItem, aUserData);
    }
    return 0;
}



void SvxBackgroundTabPage::FillControls_Impl( const SvxBrushItem& rBgdAttr,
                                              const OUString& rUserData )
{
    SvxGraphicPosition  ePos = rBgdAttr.GetGraphicPos();
    const Color& rColor = rBgdAttr.GetColor();
    if(bColTransparency)
    {
        m_pColTransMF->SetValue(lcl_TransparencyToPercent(rColor.GetTransparency()));
        m_pColTransMF->SaveValue();
        bool bEnableTransp = rColor.GetTransparency() < 0xff;
        m_pColTransFT->Enable(bEnableTransp);
        m_pColTransMF->Enable(bEnableTransp);
        //the default setting should be "no transparency"
        if(!bEnableTransp)
            m_pColTransMF->SetValue(0);
    }

    if ( GPOS_NONE == ePos || !m_pLbSelect->IsVisible() )
    {
        // We don't have a graphic, do we have gradient fill style?
        if (!m_rXFillSet.HasItem(XATTR_FILLSTYLE) || ((const XFillStyleItem&)m_rXFillSet.Get(XATTR_FILLSTYLE)).GetValue() != XFILL_GRADIENT)
        {
            lcl_setFillStyle(m_pLbSelect, XFILL_SOLID);
            ShowColorUI_Impl();
            Color aTrColor( COL_TRANSPARENT );
            aBgdColor = rColor;

            sal_uInt16 nCol = ( aTrColor != aBgdColor ) ?
                GetItemId_Impl(*m_pBackgroundColorSet, aBgdColor) : 0;

            if( aTrColor != aBgdColor && nCol == 0)
            {
                m_pBackgroundColorSet->SetNoSelection();
            }
            else
            {
                m_pBackgroundColorSet->SelectItem( nCol );
            }

            m_pPreviewWin1->NotifyChange( aBgdColor );
        }
        else
        {
            // Gradient fill style, then initialize preview with data from Writer.
            lcl_setFillStyle(m_pLbSelect, XFILL_GRADIENT);
            ShowGradientUI_Impl();
            m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
            m_pCtlPreview->Invalidate();
        }
        if ( m_pLbSelect->IsVisible() ) // initialize graphic part
        {
            aBgdGraphicFilter = "";
            aBgdGraphicPath = "";

            if ( rUserData.isEmpty() )
                m_pBtnPreview->Check( false );
            m_pBtnLink->Check( false );
            m_pBtnLink->Disable();
            m_pPreviewWin2->NotifyChange( NULL );
            SetGraphicPosition_Impl( GPOS_TILED );  // tiles as default
        }
    }
    else
    {
        const OUString  aStrLink   = rBgdAttr.GetGraphicLink();
        const OUString  aStrFilter = rBgdAttr.GetGraphicFilter();

        lcl_setFillStyle(m_pLbSelect, XFILL_BITMAP);
        ShowBitmapUI_Impl();

        if ( !aStrLink.isEmpty() )
        {
#ifdef DBG_UTIL
            INetURLObject aObj( aStrLink );
            DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
#endif
            aBgdGraphicPath = aStrLink;
            m_pBtnLink->Check( true );
            m_pBtnLink->Enable();
        }
        else
        {
            aBgdGraphicPath = "";
            m_pBtnLink->Check( false );
            m_pBtnLink->Disable();
        }

        if(bGraphTransparency)
        {
            const GraphicObject* pObject = rBgdAttr.GetGraphicObject();
            if(pObject)
                m_pGraphTransMF->SetValue(lcl_TransparencyToPercent(pObject->GetAttr().GetTransparency()));
            else
                m_pGraphTransMF->SetValue(0);
            m_pGraphTransMF->SaveValue();
        }

        FileClickHdl_Impl(m_pBtnLink);

        aBgdGraphicFilter = aStrFilter;

        if ( aStrLink.isEmpty() || m_pBtnPreview->IsChecked() )
        {
            // Graphic exists in the item and doesn't have
            // to be loaded:

            const Graphic* pGraphic = rBgdAttr.GetGraphic();

            if ( !pGraphic && m_pBtnPreview->IsChecked() )
                bIsGraphicValid = LoadLinkedGraphic_Impl();
            else if ( pGraphic )
            {
                aBgdGraphic = *pGraphic;
                bIsGraphicValid = true;

                if ( rUserData.isEmpty() )
                    m_pBtnPreview->Check();
            }
            else
            {
                RaiseLoadError_Impl();
                bIsGraphicValid = false;

                if ( rUserData.isEmpty() )
                    m_pBtnPreview->Check( false );
            }
        }

        if ( m_pBtnPreview->IsChecked() && bIsGraphicValid )
        {
            Bitmap aBmp = aBgdGraphic.GetBitmap();
            m_pPreviewWin2->NotifyChange( &aBmp );
        }
        else
            m_pPreviewWin2->NotifyChange( NULL );

        SetGraphicPosition_Impl( ePos );
    }
}

void SvxBackgroundTabPage::EnableTransparency(bool bColor, bool bGraphic)
{
    bColTransparency  = bColor;
    bGraphTransparency = bGraphic;
    m_pColTransFT->Show(bColor);
    m_pColTransMF->Show(bColor);
}

void SvxBackgroundTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,false);
    SFX_ITEMSET_ARG (&aSet,pGradientListItem,SvxGradientListItem,SID_GRADIENT_LIST,false);

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
            EnableTransparency(true, true);
    }

    if (pGradientListItem)
    {
        // If we get a gradient list, also read fill and gradient style.
        m_pGradientList = pGradientListItem->GetGradientList();
        m_pLbGradients->Fill(m_pGradientList);
        const XFillStyleItem& rFillStyleItem = (const XFillStyleItem&)aSet.Get(SID_SW_ATTR_FILL_STYLE);
        m_rXFillSet.Put(XFillStyleItem(rFillStyleItem.GetValue()));
        const XFillGradientItem& rFillGradientItem = (const XFillGradientItem&)aSet.Get(SID_SW_ATTR_FILL_GRADIENT);
        m_rXFillSet.Put(XFillGradientItem(rFillGradientItem.GetName(), rFillGradientItem.GetGradientValue()));
    }
    else
        // Otherwise hide the gradient UI.
        for (int i = 0; i < m_pLbSelect->GetEntryCount(); ++i)
            if ((XFillStyle)(sal_uLong)m_pLbSelect->GetEntryData(i) == XFILL_GRADIENT)
            {
                m_pLbSelect->RemoveEntry(i);
                break;
            }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
