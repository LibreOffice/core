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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <unotools/pathoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <vcl/idle.hxx>
#include <vcl/window.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <strings.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/memberids.h>

#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <backgrnd.hxx>

#include <svx/xtable.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svxerr.hxx>
#include <svx/drawitem.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <svtools/controldims.hxx>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <vcl/GraphicObject.hxx>

#include <svx/unobrushitemhelper.hxx>

using namespace css;

// table background
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

const sal_uInt16 SvxBackgroundTabPage::pPageRanges[] =
{
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
        : pCellBrush(nullptr)
        , pRowBrush(nullptr)
        , pTableBrush(nullptr)
        , nCellWhich(0)
        , nRowWhich(0)
        , nTableWhich(0)
        , nActPos(0)
    {}
};

struct SvxBackgroundPage_Impl
{
    Idle*          pLoadIdle;
    bool        bIsImportDlgInExecute;

    SvxBackgroundPage_Impl()
        : pLoadIdle(nullptr)
        , bIsImportDlgInExecute(false)
    {}
};

/// Returns the fill style of the currently selected entry.
static drawing::FillStyle lcl_getFillStyle(ListBox const * pLbSelect)
{
    return static_cast<drawing::FillStyle>(reinterpret_cast<sal_uLong>(pLbSelect->GetSelectedEntryData()));
}

// Selects the entry matching the specified fill style.
static void lcl_setFillStyle(ListBox* pLbSelect, drawing::FillStyle eStyle)
{
    for (int i = 0; i < pLbSelect->GetEntryCount(); ++i)
    {
        if (static_cast<drawing::FillStyle>(reinterpret_cast<sal_uLong>(pLbSelect->GetEntryData(i))) == eStyle)
        {
            pLbSelect->SelectEntryPos(i);
            return;
        }
    }
}

sal_uInt16 GetItemId_Impl( ValueSet const & rValueSet, const Color& rCol )
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


/** Preview window for brush or bitmap */
class BackgroundPreviewImpl : public vcl::Window
{
public:
    explicit BackgroundPreviewImpl(vcl::Window* pParent);
    void setBmp(bool bBmp);
    virtual ~BackgroundPreviewImpl() override;
    virtual void    dispose() override;

    void            NotifyChange( const Color&  rColor );
    void            NotifyChange( const Bitmap* pBitmap );

protected:
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    Resize() override;

private:

    void recalcDrawPos();

    bool            bIsBmp;
    std::unique_ptr<Bitmap> pBitmap;
    Point           aDrawPos;
    Size            aDrawSize;
    ::tools::Rectangle       aDrawRect;
};

BackgroundPreviewImpl::BackgroundPreviewImpl(vcl::Window* pParent)
    : Window(pParent, WB_BORDER)
    , bIsBmp(false)
    , pBitmap(nullptr)
    , aDrawRect(Point(0,0), GetOutputSizePixel())
{
    SetBorderStyle(WindowBorderStyle::MONO);
    Invalidate(aDrawRect);
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeBackgroundPreview(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<BackgroundPreviewImpl>::Create(pParent);
}

void BackgroundPreviewImpl::setBmp(bool bBmp)
{
    bIsBmp = bBmp;
    Invalidate();
}

BackgroundPreviewImpl::~BackgroundPreviewImpl()
{
    disposeOnce();
}

void BackgroundPreviewImpl::dispose()
{
    pBitmap.reset();
    vcl::Window::dispose();
}

void BackgroundPreviewImpl::NotifyChange( const Color& rColor )
{
    if ( !bIsBmp )
    {
        const static Color aTranspCol( COL_TRANSPARENT );

        SetFillColor( rColor == aTranspCol ? GetSettings().GetStyleSettings().GetFieldColor() : rColor.GetRGBColor() );
        Invalidate(aDrawRect);
    }
}

void BackgroundPreviewImpl::NotifyChange( const Bitmap* pNewBitmap )
{
    if (bIsBmp && (pNewBitmap || pBitmap))
    {
        if (pNewBitmap && pBitmap)
            *pBitmap = *pNewBitmap;
        else if (pNewBitmap && !pBitmap)
            pBitmap.reset( new Bitmap(*pNewBitmap) );
        else if (!pNewBitmap)
            pBitmap.reset();

        recalcDrawPos();

        Invalidate(aDrawRect);
        Update();
    }
}

void BackgroundPreviewImpl::recalcDrawPos()
{
    if (pBitmap)
    {
        Size aSize = GetOutputSizePixel();
        // InnerSize == Size without one pixel border
        Size aInnerSize = aSize;
        aInnerSize.AdjustWidth( -2 );
        aInnerSize.AdjustHeight( -2 );
        aDrawSize = pBitmap->GetSizePixel();

        // bitmap bigger than preview window?
        if (aDrawSize.Width() > aInnerSize.Width())
        {
            aDrawSize.setHeight( aDrawSize.Height() * aInnerSize.Width() / aDrawSize.Width() );
            if (aDrawSize.Height() > aInnerSize.Height())
            {
                aDrawSize.setWidth( aDrawSize.Height() );
                aDrawSize.setHeight( aInnerSize.Height() );
            }
            else
                aDrawSize.setWidth( aInnerSize.Width() );
        }
        else if (aDrawSize.Height() > aInnerSize.Height())
        {
            aDrawSize.setWidth( aDrawSize.Width() * aInnerSize.Height() / aDrawSize.Height() );
            if (aDrawSize.Width() > aInnerSize.Width())
            {
                aDrawSize.setHeight( aDrawSize.Width() );
                aDrawSize.setWidth( aInnerSize.Width() );
            }
            else
                aDrawSize.setHeight( aInnerSize.Height() );
        }

        aDrawPos.setX( (aSize.Width()  - aDrawSize.Width())  / 2 );
        aDrawPos.setY( (aSize.Height() - aDrawSize.Height()) / 2 );
    }
}

void BackgroundPreviewImpl::Resize()
{
    Window::Resize();
    aDrawRect = ::tools::Rectangle(Point(0,0), GetOutputSizePixel());
    recalcDrawPos();
}

void BackgroundPreviewImpl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
    rRenderContext.SetLineColor();

    if (bIsBmp)
    {
        rRenderContext.SetFillColor(COL_TRANSPARENT);
    }
    rRenderContext.DrawRect(aDrawRect);

    if (bIsBmp)
    {
        if (pBitmap)
            rRenderContext.DrawBitmap(aDrawPos, aDrawSize, *pBitmap);
        else
        {
            Size aSize(GetOutputSizePixel());
            rRenderContext.DrawLine(Point(0, 0), Point(aSize.Width(), aSize.Height()));
            rRenderContext.DrawLine(Point(0, aSize.Height()), Point(aSize.Width(), 0));
        }
    }
}

void BackgroundPreviewImpl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        Invalidate();
    }
    Window::DataChanged( rDCEvt );
}

#define HDL(hdl) LINK(this,SvxBackgroundTabPage,hdl)

SvxBackgroundTabPage::SvxBackgroundTabPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SvxTabPage(pParent, "BackgroundPage", "cui/ui/backgroundpage.ui", rCoreSet)
    , nHtmlMode(0)
    , bAllowShowSelector(true)
    , bIsGraphicValid(false)
    , bHighlighting(false)
    , bCharBackColor(false)
    , m_bColorSelected(false)
    , pPageImpl(new SvxBackgroundPage_Impl)
    , pImportDlg(nullptr)
    , pTableBck_Impl(nullptr)
    , pHighlighting(nullptr)
{
    get(m_pAsGrid, "asgrid");
    get(m_pSelectTxt, "asft");
    get(m_pLbSelect, "selectlb");
    get(m_pTblDesc, "forft");
    get(m_pTblLBox, "tablelb");

    get(m_pBackGroundColorLabelFT, "background_label");
    get(m_pBackGroundColorFrame, "backgroundcolorframe");
    get(m_pBackgroundColorSet, "backgroundcolorset");
    get(m_pPreviewWin1, "preview1");

    get(m_pBtnPreview, "showpreview");

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

    get(m_pPreviewWin2, "preview2");
    m_pPreviewWin2->setBmp(true);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;

    if ( SfxItemState::SET == rCoreSet.GetItemState( SID_HTML_MODE, false, &pItem )
         || ( nullptr != ( pShell = SfxObjectShell::Current()) &&
              nullptr != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    }

    FillColorValueSets_Impl();

    m_pBackgroundColorSet->SetSelectHdl( HDL(BackgroundColorHdl_Impl) );
    m_pBackgroundColorSet->SetStyle(m_pBackgroundColorSet->GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD);
    m_pBackgroundColorSet->SetText(SvxResId(RID_SVXSTR_NOFILL));
}

SvxBackgroundTabPage::~SvxBackgroundTabPage()
{
    disposeOnce();
}

void SvxBackgroundTabPage::dispose()
{
    if (pPageImpl)
    {
        delete pPageImpl->pLoadIdle;
        pPageImpl.reset();
    }

    pImportDlg.reset();

    if( pTableBck_Impl)
    {
        delete pTableBck_Impl->pCellBrush;
        delete pTableBck_Impl->pRowBrush;
        delete pTableBck_Impl->pTableBrush;
        pTableBck_Impl.reset();
    }

    m_pAsGrid.clear();
    m_pSelectTxt.clear();
    m_pLbSelect.clear();
    m_pTblDesc.clear();
    m_pTblLBox.clear();
    m_pBackGroundColorFrame.clear();
    m_pBackgroundColorSet.clear();
    m_pBackGroundColorLabelFT.clear();
    m_pPreviewWin1.clear();
    m_pBtnPreview.clear();
    m_pBitmapContainer.clear();
    m_pFileFrame.clear();
    m_pBtnBrowse.clear();
    m_pBtnLink.clear();
    m_pFtUnlinked.clear();
    m_pFtFile.clear();
    m_pTypeFrame.clear();
    m_pBtnPosition.clear();
    m_pBtnArea.clear();
    m_pBtnTile.clear();
    m_pWndPosition.clear();
    m_pPreviewWin2.clear();
    SvxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxBackgroundTabPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxBackgroundTabPage>::Create( pParent.pParent, *rAttrSet );
}

void SvxBackgroundTabPage::Reset( const SfxItemSet* rSet )
{
    m_bColorSelected = false;

    // condition of the preview button is persistent due to UserData
    OUString aUserData = GetUserData();
    m_pBtnPreview->Check( !aUserData.isEmpty() && '1' == aUserData[0] );

    // don't be allowed to call ShowSelector() after reset anymore
    bAllowShowSelector = false;


    // get and evaluate Input-BrushItem
    bool bBrushItemSet = false;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    const SfxPoolItem* pItem;
    sal_uInt16 nDestValue = USHRT_MAX;

    if ( SfxItemState::SET == rSet->GetItemState( SID_BACKGRND_DESTINATION,
                                            false, &pItem ) )
    {
        nDestValue = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
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
    else if( bHighlighting )
    {
        nSlot = SID_ATTR_BRUSH_CHAR;
    }
    else if( bCharBackColor )
    {
        nSlot = SID_ATTR_CHAR_BACK_COLOR;
    }

    //#111173# the destination item is missing when the parent style has been changed
    if(USHRT_MAX == nDestValue && m_pTblLBox->IsVisible())
        nDestValue = 0;
    sal_uInt16 nWhich = GetWhich(nSlot);
    SvxBrushItem aBgdAttr(nWhich);

    if (rSet->GetItemState( nWhich, false ) >= SfxItemState::DEFAULT)
    {
        if (!bCharBackColor)
            aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(nWhich));
        else
        {
            // EE_CHAR_BKGCOLOR is SvxBackgroundColorItem, but char background tabpage
            // can only work with SvxBrushItems
            // extract Color out of SvxBackColorItem
            Color aBackColor = static_cast<const SvxBackgroundColorItem&>(rSet->Get(nWhich)).GetValue();
            // make new SvxBrushItem with this Color
            aBgdAttr = SvxBrushItem(aBackColor, SID_ATTR_BRUSH_CHAR);
        }
        bBrushItemSet = true;
    }

    m_pBtnTile->Check();

    if (bBrushItemSet)
    {
        FillControls_Impl(aBgdAttr, aUserData);
        aBgdColor = aBgdAttr.GetColor();
    }
    else
    {
        m_pSelectTxt->Hide();
        m_pLbSelect->Hide();
        lcl_setFillStyle(m_pLbSelect, drawing::FillStyle_SOLID);
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_BRUSH );

        if ( pOld )
            aBgdColor = static_cast<const SvxBrushItem*>(pOld)->GetColor();
    }

    if ( nDestValue != USHRT_MAX )
    {
        if(m_pTblLBox->IsVisible())
        {
            sal_Int32 nValue = m_pTblLBox->GetSelectedEntryPos();

            if ( pTableBck_Impl )
            {
                DELETEZ( pTableBck_Impl->pCellBrush);
                DELETEZ( pTableBck_Impl->pRowBrush);
                DELETEZ( pTableBck_Impl->pTableBrush);
            }
            else
                pTableBck_Impl.reset( new SvxBackgroundTable_Impl() );

            pTableBck_Impl->nActPos = nValue;

            nWhich = GetWhich( SID_ATTR_BRUSH );
            if ( rSet->GetItemState( nWhich, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(nWhich));
                pTableBck_Impl->pCellBrush = new SvxBrushItem(aBgdAttr);
            }
            pTableBck_Impl->nCellWhich = nWhich;

            if ( rSet->GetItemState( SID_ATTR_BRUSH_ROW, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(SID_ATTR_BRUSH_ROW));
                pTableBck_Impl->pRowBrush = new SvxBrushItem(aBgdAttr);
            }
            pTableBck_Impl->nRowWhich = SID_ATTR_BRUSH_ROW;

            if ( rSet->GetItemState( SID_ATTR_BRUSH_TABLE, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(SID_ATTR_BRUSH_TABLE));
                pTableBck_Impl->pTableBrush = new SvxBrushItem(aBgdAttr);
            }
            pTableBck_Impl->nTableWhich = SID_ATTR_BRUSH_TABLE;

            TblDestinationHdl_Impl(*m_pTblLBox);
            m_pTblLBox->SaveValue();
        }
        else if( bHighlighting )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH_CHAR );
            if ( rSet->GetItemState( nWhich, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(nWhich));
                pHighlighting.reset(new SvxBrushItem(aBgdAttr));
            }
        }
        else if( bCharBackColor )
        {
            nWhich = GetWhich(SID_ATTR_CHAR_BACK_COLOR);
            if ( rSet->GetItemState( nWhich, false ) >= SfxItemState::DEFAULT )
            {
                // EE_CHAR_BKGCOLOR is SvxBackgroundColorItem, but char background tabpage
                // can only work with SvxBrushItems
                // extract Color out of SvxBackColorItem
                Color aBackColor = static_cast<const SvxBackgroundColorItem&>(rSet->Get(nWhich)).GetValue();
                // make new SvxBrushItem with this Color
                aBgdAttr = SvxBrushItem(aBackColor, SID_ATTR_BRUSH_CHAR);
                pHighlighting.reset(new SvxBrushItem(aBgdAttr));
            }
        }
    }
}

/** When destroying a SfxTabPage this virtual method is called,
    so that the TabPage can save internal information.

    In this case the condition of the preview button is saved.
*/
void SvxBackgroundTabPage::FillUserData()
{
    SetUserData( m_pBtnPreview->IsChecked() ? OUString('1') : OUString('0') );
}

bool SvxBackgroundTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( pPageImpl->pLoadIdle && pPageImpl->pLoadIdle->IsActive() )
    {
        pPageImpl->pLoadIdle->Stop();
        LoadIdleHdl_Impl( pPageImpl->pLoadIdle );
    }

    bool bModified = false;
    bool bCompareOldBrush = true;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;

    if ( m_pTblLBox->IsVisible() )
    {
        switch( m_pTblLBox->GetSelectedEntryPos() )
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
    else if( bHighlighting )
    {
        nSlot = SID_ATTR_BRUSH_CHAR;
    }
    else if( bCharBackColor )
    {
        nSlot = SID_ATTR_CHAR_BACK_COLOR;
        bCompareOldBrush = false;
    }

    sal_uInt16 nWhich = GetWhich( nSlot );

    const SfxPoolItem* pOld = GetOldItem(*rCoreSet, nSlot);
    if (pOld && bCompareOldBrush)
    {
        SfxItemState eOldItemState = rCoreSet->GetItemState(nSlot, false);
        const SfxItemSet& rOldSet = GetItemSet();
        const SvxBrushItem& rOldItem    = static_cast<const SvxBrushItem&>(*pOld);

        SvxGraphicPosition  eOldPos     = rOldItem.GetGraphicPos();
        const bool          bIsBrush    = ( drawing::FillStyle_SOLID == lcl_getFillStyle(m_pLbSelect) );

        if (   ( (GPOS_NONE == eOldPos) && bIsBrush  )
            || ( (GPOS_NONE != eOldPos) && !bIsBrush ) ) // Brush <-> Bitmap changed?
        {
            // background art hasn't been changed:

            if ( (GPOS_NONE == eOldPos) || !m_pLbSelect->IsVisible() )
            {
                // Brush-treatment:
                if ( rOldItem.GetColor() != aBgdColor ||
                     (SfxItemState::DEFAULT >= eOldItemState && m_bColorSelected))
                {
                    bModified = true;
                    rCoreSet->Put( SvxBrushItem( aBgdColor, nWhich ) );
                }
                else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
                    rCoreSet->ClearItem( nWhich );
            }
            else
            {
                // Bitmap-treatment:

                SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
                const bool          bIsLink  = m_pBtnLink->IsChecked();
                const bool          bWasLink = !rOldItem.GetGraphicLink().isEmpty();


                if ( !bIsLink && !bIsGraphicValid )
                    bIsGraphicValid = LoadLinkedGraphic_Impl();

                bool bModifyBrush = false;
                if (eNewPos != eOldPos || bIsLink != bWasLink)
                    bModifyBrush = true;
                else if (bWasLink && rOldItem.GetGraphicLink() != aBgdGraphicPath)
                    bModifyBrush = true;
                else if (!bWasLink)
                {
                    const Graphic* pGraphic = rOldItem.GetGraphic();
                    if (pGraphic)
                        bModifyBrush = pGraphic->GetBitmapEx() != aBgdGraphic.GetBitmapEx();
                }
                if (bModifyBrush)
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

                    rCoreSet->Put(aTmpBrush);
                }
                else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
                    rCoreSet->ClearItem( nWhich );
            }
        }
        else // Brush <-> Bitmap changed!
        {
            if ( bIsBrush )
            {
                rCoreSet->Put( SvxBrushItem( aBgdColor, nWhich ) );
            }
            else
            {
                std::unique_ptr<SvxBrushItem> pTmpBrush;
                if ( m_pBtnLink->IsChecked() )
                {
                    pTmpBrush.reset(new SvxBrushItem( aBgdGraphicPath,
                                                aBgdGraphicFilter,
                                                GetGraphicPosition_Impl(),
                                                nWhich ));
                }
                else
                {
                    if ( !bIsGraphicValid )
                        bIsGraphicValid = LoadLinkedGraphic_Impl();

                    if ( bIsGraphicValid )
                        pTmpBrush.reset(new SvxBrushItem( aBgdGraphic,
                                                    GetGraphicPosition_Impl(),
                                                    nWhich ));
                }
                if(pTmpBrush)
                {
                    rCoreSet->Put(*pTmpBrush);
                }
            }
            bModified = ( bIsBrush || m_pBtnLink->IsChecked() || bIsGraphicValid );
        }
    }
    else if (pOld && SID_ATTR_CHAR_BACK_COLOR == nSlot)
    {
        SfxItemState eOldItemState = rCoreSet->GetItemState(nSlot, false);
        const SfxItemSet& rOldSet = GetItemSet();
        const SvxBackgroundColorItem& rOldItem = static_cast<const SvxBackgroundColorItem&>(*pOld);

        // Brush-treatment:
        if ( rOldItem.GetValue() != aBgdColor ||
             (SfxItemState::DEFAULT >= eOldItemState && m_bColorSelected))
        {
            bModified = true;
            rCoreSet->Put(SvxBackgroundColorItem(aBgdColor, nWhich));
        }
        else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
            rCoreSet->ClearItem( nWhich );
    }
    else if ( SID_ATTR_BRUSH_CHAR == nSlot && aBgdColor != COL_WHITE )
    {
        rCoreSet->Put( SvxBrushItem( aBgdColor, nWhich ) );
        bModified = true;
    }

    if( m_pTblLBox->IsVisible() )
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && pTableBck_Impl->pCellBrush)
        {
            const SfxPoolItem* pOldCell =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH );

            if ( *pTableBck_Impl->pCellBrush != *pOldCell )
            {
                rCoreSet->Put( *pTableBck_Impl->pCellBrush );
                bModified = true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_ROW && pTableBck_Impl->pRowBrush)
        {
            const SfxPoolItem* pOldRow =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH_ROW );

            if ( *pTableBck_Impl->pRowBrush != *pOldRow )
            {
                rCoreSet->Put( *pTableBck_Impl->pRowBrush );
                bModified = true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_TABLE && pTableBck_Impl->pTableBrush)
        {
            const SfxPoolItem* pOldTable =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH_TABLE );

            if ( *pTableBck_Impl->pTableBrush != *pOldTable )
            {
                rCoreSet->Put( *pTableBck_Impl->pTableBrush );
                bModified = true;
            }
        }

        if( m_pTblLBox->IsValueChangedFromSaved() )
        {
            rCoreSet->Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                         m_pTblLBox->GetSelectedEntryPos() ) );
            bModified = true;
        }
    }
    else if( bHighlighting )
    {
        if( nSlot != SID_ATTR_BRUSH_CHAR )
        {
            const SfxPoolItem* pOldChar =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH_CHAR );
            if ( pOldChar && pHighlighting &&
                (*pHighlighting != *pOldChar || *pHighlighting != SvxBrushItem(SID_ATTR_BRUSH_CHAR)))
            {
                rCoreSet->Put( *pHighlighting );
                bModified = true;
            }
        }
    }
    return bModified;
}

/** virtual method; is called on deactivation */
DeactivateRC SvxBackgroundTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( pPageImpl->bIsImportDlgInExecute )
        return DeactivateRC::KeepPage;

    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

void SvxBackgroundTabPage::PointChanged( vcl::Window*, RectPoint )
{
    // has to be implemented so that position control can work
}

void SvxBackgroundTabPage::PointChanged( weld::DrawingArea*, RectPoint )
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

        // delayed loading via timer (because of UI-Update)
        pPageImpl->pLoadIdle = new Idle("DelayedLoad");
        pPageImpl->pLoadIdle->SetPriority( TaskPriority::LOWEST );
        pPageImpl->pLoadIdle->SetInvokeHandler(
            LINK( this, SvxBackgroundTabPage, LoadIdleHdl_Impl ) );

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
                              GetFrameWeld(),
                              RID_SVXERRCTX,
                              SvxResLocale() );

    ErrorHandler::HandleError(
        *new StringErrorInfo( ERRCODE_SVX_GRAPHIC_NOTREADABLE,
                              aBgdGraphicPath ) );
}

bool SvxBackgroundTabPage::LoadLinkedGraphic_Impl()
{
    bool bResult = ( !aBgdGraphicPath.isEmpty() ) &&
                   ( ERRCODE_NONE == GraphicFilter::LoadGraphic( aBgdGraphicPath,
                                                 aBgdGraphicFilter,
                                                 aBgdGraphic ) );
    return bResult;
}

void SvxBackgroundTabPage::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = nullptr;
    XColorListRef pColorTable = nullptr;
    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
    {
        pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
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
    m_pBackgroundColorSet->SetColCount(SvxColorValueSet::getColumnCount());
}

/** Hide the controls for editing the bitmap
    and show the controls for color settings instead.
*/
void SvxBackgroundTabPage::ShowColorUI_Impl()
{
    if (!m_pBackGroundColorFrame->IsVisible())
    {
        HideBitmapUI_Impl();
        m_pBackGroundColorFrame->Show();
    }
}

void SvxBackgroundTabPage::HideColorUI_Impl()
{
        m_pBackGroundColorFrame->Hide();
}

/** Hide the controls for color settings
    and show controls for editing the bitmap instead.
*/
void SvxBackgroundTabPage::ShowBitmapUI_Impl()
{
    if (m_pLbSelect->IsVisible() &&
         (m_pBackGroundColorFrame->IsVisible() || !m_pFileFrame->IsVisible()))
    {
        HideColorUI_Impl();


        m_pBitmapContainer->Show();

        m_pFileFrame->Show();
        m_pBtnLink->Show(!(nHtmlMode & HTMLMODE_ON));

        m_pTypeFrame->Show();

        m_pPreviewWin2->Show();
        m_pBtnPreview->Show();
    }
}

void SvxBackgroundTabPage::HideBitmapUI_Impl()
{
    m_pBitmapContainer->Hide();
    m_pFileFrame->Hide();
    m_pTypeFrame->Hide();
    m_pPreviewWin2->Hide();
    m_pBtnPreview->Hide();
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
            RectPoint eNewPos = RectPoint::MM;

            switch ( ePos )
            {
                case GPOS_MM:   break;
                case GPOS_LT:   eNewPos = RectPoint::LT; break;
                case GPOS_MT:   eNewPos = RectPoint::MT; break;
                case GPOS_RT:   eNewPos = RectPoint::RT; break;
                case GPOS_LM:   eNewPos = RectPoint::LM; break;
                case GPOS_RM:   eNewPos = RectPoint::RM; break;
                case GPOS_LB:   eNewPos = RectPoint::LB; break;
                case GPOS_MB:   eNewPos = RectPoint::MB; break;
                case GPOS_RB:   eNewPos = RectPoint::RB; break;
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
            case RectPoint::LT: return GPOS_LT;
            case RectPoint::MT: return GPOS_MT;
            case RectPoint::RT: return GPOS_RT;
            case RectPoint::LM: return GPOS_LM;
            case RectPoint::MM: return GPOS_MM;
            case RectPoint::RM: return GPOS_RM;
            case RectPoint::LB: return GPOS_LB;
            case RectPoint::MB: return GPOS_MB;
            case RectPoint::RB: return GPOS_RB;
        }
    }
    return GPOS_MM;
}


// Handler

/** Handler, called when color selection is changed */
IMPL_LINK_NOARG(SvxBackgroundTabPage, BackgroundColorHdl_Impl, ValueSet*, void)
{
    sal_uInt16 nItemId = m_pBackgroundColorSet->GetSelectedItemId();
    Color aColor = nItemId ? ( m_pBackgroundColorSet->GetItemColor( nItemId ) ) : COL_TRANSPARENT;
    aBgdColor = aColor;
    m_bColorSelected = true;
    m_pPreviewWin1->NotifyChange( aBgdColor );
}

IMPL_LINK_NOARG(SvxBackgroundTabPage, SelectHdl_Impl, ListBox&, void)
{
    if ( drawing::FillStyle_SOLID == lcl_getFillStyle(m_pLbSelect) )
    {
        ShowColorUI_Impl();
    }
    else
    {
        ShowBitmapUI_Impl();
    }
}

IMPL_LINK( SvxBackgroundTabPage, FileClickHdl_Impl, Button*, pBox, void )
{
    if (m_pBtnLink == pBox)
    {
        if ( m_pBtnLink->IsChecked() )
        {
            m_pFtUnlinked->Hide();
            INetURLObject aObj( aBgdGraphicPath );
            OUString aFilePath;
            if ( aObj.GetProtocol() == INetProtocol::File )
                aFilePath = aObj.getFSysPath( FSysStyle::Detect );
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
                Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
                m_pPreviewWin2->NotifyChange( &aBmp );
            }
            else
            {
                if ( !aBgdGraphicPath.isEmpty() ) // only for linked bitmap
                    RaiseLoadError_Impl();
                m_pPreviewWin2->NotifyChange( nullptr );
            }
        }
        else
            m_pPreviewWin2->NotifyChange( nullptr );
    }
}

IMPL_LINK( SvxBackgroundTabPage, RadioClickHdl_Impl, Button*, pBtn, void )
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
}

/** Handler, called by pressing the browse button.
    Create graphic/insert dialog, set path and start.
*/
IMPL_LINK_NOARG(SvxBackgroundTabPage, BrowseHdl_Impl, Button*, void)
{
    if ( pPageImpl->pLoadIdle->IsActive() )
        return;
    bool bHtml = 0 != ( nHtmlMode & HTMLMODE_ON );

    OUString aStrBrowse(get<vcl::Window>("findgraphicsft")->GetText());
    pImportDlg.reset( new SvxOpenGraphicDialog(aStrBrowse, GetFrameWeld()) );
    if ( bHtml )
        pImportDlg->EnableLink(false);
    pImportDlg->SetPath( aBgdGraphicPath, m_pBtnLink->IsChecked() );

    pPageImpl->bIsImportDlgInExecute = true;
    ErrCode nErr = pImportDlg->Execute();
    pPageImpl->bIsImportDlgInExecute = false;

    if( !nErr )
    {
        if ( bHtml )
            m_pBtnLink->Check();
        // if link isn't checked and preview isn't, either,
        // activate preview, so that the user sees which
        // graphic he has chosen
        if ( !m_pBtnLink->IsChecked() && !m_pBtnPreview->IsChecked() )
            m_pBtnPreview->Check();
        // timer-delayed loading of the graphic
        pPageImpl->pLoadIdle->Start();
    }
    else
        pImportDlg.reset();
}

/** Delayed loading of the graphic.
    Graphic is only loaded, if it's
    different to the current graphic.
*/
IMPL_LINK( SvxBackgroundTabPage, LoadIdleHdl_Impl, Timer*, pIdle, void )
{
    if ( pIdle == pPageImpl->pLoadIdle )
    {
        pPageImpl->pLoadIdle->Stop();

        if ( pImportDlg )
        {
            INetURLObject aOld( aBgdGraphicPath );
            INetURLObject aNew( pImportDlg->GetPath() );
            if ( aBgdGraphicPath.isEmpty() || aNew != aOld )
            {
                // new file chosen
                aBgdGraphicPath   = pImportDlg->GetPath();
                aBgdGraphicFilter = pImportDlg->GetDetectedFilter();
                bool bLink = ( nHtmlMode & HTMLMODE_ON ) || pImportDlg->IsAsLink();
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
                        aBgdGraphicFilter.clear();
                        aBgdGraphicPath.clear();
                        bIsGraphicValid = false;
                    }
                }
                else
                    bIsGraphicValid = false; // load graphic not until preview click

                if ( m_pBtnPreview->IsChecked() && bIsGraphicValid )
                {
                    Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
                    m_pPreviewWin2->NotifyChange( &aBmp );
                }
                else
                    m_pPreviewWin2->NotifyChange( nullptr );
            }

            FileClickHdl_Impl(m_pBtnLink);
            pImportDlg.reset();
        }
    }
}

void SvxBackgroundTabPage::ShowTblControl()
{
    m_pTblLBox->SetSelectHdl( HDL(TblDestinationHdl_Impl) );
    m_pTblLBox->SelectEntryPos(0);
    m_pTblDesc->Show();
    m_pTblLBox->Show();
    m_pAsGrid->Show();
}

IMPL_LINK( SvxBackgroundTabPage, TblDestinationHdl_Impl, ListBox&, rBox, void )
{
    sal_Int32 nSelPos = rBox.GetSelectedEntryPos();
    if( pTableBck_Impl && pTableBck_Impl->nActPos != nSelPos)
    {
        std::unique_ptr<SvxBrushItem> xItemHolder;
        SvxBrushItem* pActItem = nullptr;
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
            pActItem = nullptr;
            break;
        }
        pTableBck_Impl->nActPos = nSelPos;
        if(!pActItem)
        {
            xItemHolder.reset(new SvxBrushItem(nWhich));
            pActItem = xItemHolder.get();
        }
        if(drawing::FillStyle_SOLID == lcl_getFillStyle(m_pLbSelect))  // brush selected
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
            pActItem = nullptr;
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
}

void SvxBackgroundTabPage::FillControls_Impl( const SvxBrushItem& rBgdAttr,
                                              const OUString& rUserData )
{
    SvxGraphicPosition  ePos = rBgdAttr.GetGraphicPos();
    const Color& rColor = rBgdAttr.GetColor();

    if ( GPOS_NONE == ePos || !m_pLbSelect->IsVisible() )
    {
        lcl_setFillStyle(m_pLbSelect, drawing::FillStyle_SOLID);
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

        if ( m_pLbSelect->IsVisible() ) // initialize graphic part
        {
            aBgdGraphicFilter.clear();
            aBgdGraphicPath.clear();

            if ( rUserData.isEmpty() )
                m_pBtnPreview->Check( false );
            m_pBtnLink->Check( false );
            m_pBtnLink->Disable();
            m_pPreviewWin2->NotifyChange( nullptr );
            SetGraphicPosition_Impl( GPOS_TILED );  // tiles as default
        }
    }
    else
    {
        const OUString&  aStrLink   = rBgdAttr.GetGraphicLink();
        const OUString&  aStrFilter = rBgdAttr.GetGraphicFilter();

        lcl_setFillStyle(m_pLbSelect, drawing::FillStyle_BITMAP);
        ShowBitmapUI_Impl();

        if ( !aStrLink.isEmpty() )
        {
#ifdef DBG_UTIL
            INetURLObject aObj( aStrLink );
            DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );
#endif
            aBgdGraphicPath = aStrLink;
            m_pBtnLink->Check();
            m_pBtnLink->Enable();
        }
        else
        {
            aBgdGraphicPath.clear();
            m_pBtnLink->Check( false );
            m_pBtnLink->Disable();
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
            Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
            m_pPreviewWin2->NotifyChange( &aBmp );
        }
        else
            m_pPreviewWin2->NotifyChange( nullptr );

        SetGraphicPosition_Impl( ePos );
    }
}

void SvxBackgroundTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);

    if (pFlagItem)
    {
        SvxBackgroundTabFlags nFlags = static_cast<SvxBackgroundTabFlags>(pFlagItem->GetValue());
        if (nFlags & SvxBackgroundTabFlags::SHOW_TBLCTL )
            ShowTblControl();
        if ( nFlags & SvxBackgroundTabFlags::SHOW_SELECTOR )
        {
            ShowSelector();
        }
        if ((nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING) ||
            (nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR))
        {
            m_pBackGroundColorLabelFT->SetText(CuiResId(RID_SVXSTR_CHARNAME_HIGHLIGHTING));
            bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
    }
}

SvxBkgTabPage::SvxBkgTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :
    SvxAreaTabPage( pParent, rInAttrs ),
    m_pTblLBox(nullptr),
    bHighlighting(false),
    bCharBackColor(false),
    maSet(rInAttrs)
{
    VclPtr<vcl::Window> pBtn;
    get(pBtn, "btngradient"); pBtn->Hide();
    get(pBtn, "btnhatch");    pBtn->Hide();
    get(pBtn, "btnbitmap");   pBtn->Hide();
    get(pBtn, "btnpattern");  pBtn->Hide();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = nullptr;

    XColorListRef pColorTable = nullptr;
    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
    {
        pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    XBitmapListRef pBitmapList = nullptr;
    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem( SID_BITMAP_LIST ) ) ) )
    {
        pBitmapList = static_cast<const SvxBitmapListItem*>(pItem)->GetBitmapList();
    }

    SetColorList(pColorTable);
    SetBitmapList(pBitmapList);
}

SvxBkgTabPage::~SvxBkgTabPage()
{
    disposeOnce();
}

void SvxBkgTabPage::dispose()
{
    m_pTblLBox.clear();
    SvxAreaTabPage::dispose();
}

void SvxBkgTabPage::ActivatePage( const SfxItemSet& )
{
    SvxAreaTabPage::ActivatePage( maSet );
}

DeactivateRC SvxBkgTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( DeactivateRC::KeepPage == SvxAreaTabPage::DeactivatePage( &maSet ) )
        return DeactivateRC::KeepPage;

    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

bool SvxBkgTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    if ( m_pTblLBox && m_pTblLBox->IsVisible() )
    {
        switch( m_pTblLBox->GetSelectedEntryPos() )
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
    else if ( bHighlighting )
        nSlot = SID_ATTR_BRUSH_CHAR;
    else if( bCharBackColor )
        nSlot = SID_ATTR_CHAR_BACK_COLOR;

    sal_uInt16 nWhich = GetWhich(nSlot);

    drawing::FillStyle eFillType = maSet.Get( XATTR_FILLSTYLE ).GetValue();
    switch( eFillType )
    {
        case drawing::FillStyle_NONE:
        {
            if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
            {
                maSet.Put( SvxBackgroundColorItem( COL_TRANSPARENT, nWhich ) );
                rCoreSet->Put( SvxBackgroundColorItem( COL_TRANSPARENT, nWhich ) );
            }
            else
            {
                maSet.Put( SvxBrushItem( COL_TRANSPARENT, nWhich ) );
                rCoreSet->Put( SvxBrushItem( COL_TRANSPARENT, nWhich ) );
            }
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aColorItem( maSet.Get( XATTR_FILLCOLOR ) );
            if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
            {
                maSet.Put( SvxBackgroundColorItem( aColorItem.GetColorValue(), nWhich ) );
                rCoreSet->Put( SvxBackgroundColorItem( aColorItem.GetColorValue(), nWhich ) );
            }
            else
            {
                maSet.Put( SvxBrushItem( aColorItem.GetColorValue(), nWhich ) );
                rCoreSet->Put( SvxBrushItem( aColorItem.GetColorValue(), nWhich ) );
            }
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            SvxBrushItem aBrushItem( getSvxBrushItemFromSourceSet( maSet, nWhich ) );
            if ( GraphicType::NONE != aBrushItem.GetGraphicObject()->GetType() ) // no selection so use current
                rCoreSet->Put( aBrushItem );
            break;
        }
        default:
            break;
    }

    return true;
}

VclPtr<SfxTabPage> SvxBkgTabPage::Create( TabPageParent pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxBkgTabPage>::Create( pWindow.pParent, *rAttrs );
}

void SvxBkgTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        SvxBackgroundTabFlags nFlags = static_cast<SvxBackgroundTabFlags>(pFlagItem->GetValue());
        if ( nFlags & SvxBackgroundTabFlags::SHOW_TBLCTL )
        {
            VclPtr<vcl::Window> pBtn;
            get(pBtn, "btnbitmap");
            pBtn->Show();
            get(m_pTblLBox, "tablelb");
            m_pTblLBox->SelectEntryPos(0);
            m_pTblLBox->Show();
        }
        if ((nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING) ||
            (nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR))
        {
            bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
        if (nFlags & SvxBackgroundTabFlags::SHOW_SELECTOR)
        {
            VclPtr<vcl::Window> pBtn;
            get(pBtn, "btnbitmap");
            pBtn->Show();
        }
    }

    if ( bCharBackColor )
    {
        sal_uInt16 nWhich( maSet.GetPool()->GetWhich( SID_ATTR_CHAR_BACK_COLOR ) );
        Color aBackColor( static_cast<const SvxBackgroundColorItem&>( maSet.Get( nWhich ) ).GetValue() );
        SvxBrushItem aBrushItem( SvxBrushItem( aBackColor, SID_ATTR_BRUSH_CHAR ) );
        setSvxBrushItemAsFillAttributesToTargetSet( aBrushItem, maSet );
    }
    else
    {
        sal_uInt16 nWhich( maSet.GetPool()->GetWhich( bHighlighting ? SID_ATTR_BRUSH_CHAR : SID_ATTR_BRUSH ) );
        SvxBrushItem aBrushItem( static_cast<const SvxBrushItem&>( maSet.Get( nWhich ) ) );
        setSvxBrushItemAsFillAttributesToTargetSet( aBrushItem, maSet );
    }

    SvxAreaTabPage::PageCreated( aSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
