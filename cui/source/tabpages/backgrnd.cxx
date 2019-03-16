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

/// Returns the fill style of the currently selected entry.
static drawing::FillStyle lcl_getFillStyle(const weld::ComboBox& rLbSelect)
{
    return static_cast<drawing::FillStyle>(rLbSelect.get_active_id().toUInt32());
}

// Selects the entry matching the specified fill style.
static void lcl_setFillStyle(weld::ComboBox& rLbSelect, drawing::FillStyle eStyle)
{
    OUString sStyle = OUString::number(static_cast<sal_uInt32>(eStyle));
    for (int i = 0; i < rLbSelect.get_count(); ++i)
    {
        if (rLbSelect.get_id(i) == sStyle)
        {
            rLbSelect.set_active(i);
            return;
        }
    }
}

static sal_uInt16 GetItemId_Impl(const SvtValueSet& rValueSet, const Color& rCol)
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
class BackgroundPreviewImpl : public weld::CustomWidgetController
{
public:
    explicit BackgroundPreviewImpl();
    void setBmp(bool bBmp);

    void            NotifyChange(const Color&  rColor);
    void            NotifyChange(const Bitmap* pBitmap);
    void            SetFillColor(const Color& rColor) { aColor = rColor; }

protected:
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect ) override;
    virtual void    SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void    Resize() override;

private:

    void recalcDrawPos();

    bool            bIsBmp;
    std::unique_ptr<Bitmap> pBitmap;
    Point           aDrawPos;
    Size            aDrawSize;
    ::tools::Rectangle aDrawRect;
    Color           aColor;
};

BackgroundPreviewImpl::BackgroundPreviewImpl()
    : bIsBmp(false)
    , aColor(Application::GetSettings().GetStyleSettings().GetFieldColor())
{
}

void BackgroundPreviewImpl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(300, 77)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    aDrawRect = tools::Rectangle(Point(0,0), aSize);
    Invalidate(aDrawRect);
}

void BackgroundPreviewImpl::setBmp(bool bBmp)
{
    bIsBmp = bBmp;
    Invalidate();
}

void BackgroundPreviewImpl::NotifyChange( const Color& rColor )
{
    if ( !bIsBmp )
    {
        const static Color aTranspCol(COL_TRANSPARENT);
        SetFillColor(rColor == aTranspCol ? Application::GetSettings().GetStyleSettings().GetFieldColor() : rColor.GetRGBColor());
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
    CustomWidgetController::Resize();
    aDrawRect = tools::Rectangle(Point(0,0), GetOutputSizePixel());
    recalcDrawPos();
}

void BackgroundPreviewImpl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
    rRenderContext.SetLineColor();

    if (bIsBmp)
        rRenderContext.SetFillColor(COL_TRANSPARENT);
    else
        rRenderContext.SetFillColor(aColor);

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

#define HDL(hdl) LINK(this,SvxBackgroundTabPage,hdl)

SvxBackgroundTabPage::SvxBackgroundTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SvxTabPage(pParent, "cui/ui/backgroundpage.ui", "BackgroundPage", rCoreSet)
    , nHtmlMode(0)
    , bAllowShowSelector(true)
    , bIsGraphicValid(false)
    , bHighlighting(false)
    , bCharBackColor(false)
    , m_bColorSelected(false)
    , m_xWndPosition(new SvxRectCtl(this))
    , m_xBackgroundColorSet(new ColorValueSet(m_xBuilder->weld_scrolled_window("backgroundcolorsetwin")))
    , m_xPreview1(new BackgroundPreviewImpl)
    , m_xPreview2(new BackgroundPreviewImpl)
    , m_xFindGraphicsFt(m_xBuilder->weld_label("findgraphicsft"))
    , m_xAsGrid(m_xBuilder->weld_widget("asgrid"))
    , m_xSelectTxt(m_xBuilder->weld_label("asft"))
    , m_xLbSelect(m_xBuilder->weld_combo_box("selectlb"))
    , m_xTblDesc(m_xBuilder->weld_label("forft"))
    , m_xTblLBox(m_xBuilder->weld_combo_box("tablelb"))
    , m_xBackGroundColorLabelFT(m_xBuilder->weld_label("background_label"))
    , m_xBackGroundColorFrame(m_xBuilder->weld_widget("backgroundcolorframe"))
    , m_xBtnPreview(m_xBuilder->weld_check_button("showpreview"))
    , m_xBitmapContainer(m_xBuilder->weld_widget("graphicgrid"))
    , m_xFileFrame(m_xBuilder->weld_widget("fileframe"))
    , m_xBtnBrowse(m_xBuilder->weld_button("browse"))
    , m_xBtnLink(m_xBuilder->weld_check_button("link"))
    , m_xFtUnlinked(m_xBuilder->weld_label("unlinkedft"))
    , m_xFtFile(m_xBuilder->weld_label("fileft"))
    , m_xTypeFrame(m_xBuilder->weld_widget("typeframe"))
    , m_xBtnPosition(m_xBuilder->weld_radio_button("positionrb"))
    , m_xBtnArea(m_xBuilder->weld_radio_button("arearb"))
    , m_xBtnTile(m_xBuilder->weld_radio_button("tilerb"))
    , m_xWndPositionWin(new weld::CustomWeld(*m_xBuilder, "windowpos", *m_xWndPosition))
    , m_xBackgroundColorSetWin(new weld::CustomWeld(*m_xBuilder, "backgroundcolorset", *m_xBackgroundColorSet))
    , m_xPreviewWin1(new weld::CustomWeld(*m_xBuilder, "preview1", *m_xPreview1))
    , m_xPreviewWin2(new weld::CustomWeld(*m_xBuilder, "preview2", *m_xPreview2))

{
    m_xPreview2->setBmp(true);

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

    m_xBackgroundColorSet->SetSelectHdl(HDL(BackgroundColorHdl_Impl));
    m_xBackgroundColorSet->SetStyle(m_xBackgroundColorSet->GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD);
    m_xBackgroundColorSet->SetText(SvxResId(RID_SVXSTR_NOFILL));
}

SvxBackgroundTabPage::~SvxBackgroundTabPage()
{
    disposeOnce();
}

void SvxBackgroundTabPage::dispose()
{
    m_pLoadIdle.reset();
    pImportDlg.reset();
    m_pCellBrush.reset();
    m_pRowBrush.reset();
    m_pTableBrush.reset();
    m_xPreviewWin2.reset();
    m_xPreviewWin1.reset();
    m_xBackgroundColorSetWin.reset();
    m_xWndPositionWin.reset();
    m_xPreview2.reset();
    m_xPreview1.reset();
    m_xBackgroundColorSet.reset();
    m_xWndPosition.reset();

    SvxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxBackgroundTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SvxBackgroundTabPage>::Create(pParent, *rAttrSet);
}

void SvxBackgroundTabPage::Reset( const SfxItemSet* rSet )
{
    m_bColorSelected = false;

    // condition of the preview button is persistent due to UserData
    OUString aUserData = GetUserData();
    m_xBtnPreview->set_active(!aUserData.isEmpty() && '1' == aUserData[0]);

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
        m_xTblLBox->set_active(nDestValue);

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
    if (USHRT_MAX == nDestValue && m_xTblLBox->get_visible())
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

    m_xBtnTile->set_active(true);

    if (bBrushItemSet)
    {
        FillControls_Impl(aBgdAttr, aUserData);
        aBgdColor = aBgdAttr.GetColor();
    }
    else
    {
        m_xSelectTxt->hide();
        m_xLbSelect->hide();
        lcl_setFillStyle(*m_xLbSelect, drawing::FillStyle_SOLID);
        ShowColorUI_Impl();

        const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_BRUSH );

        if ( pOld )
            aBgdColor = static_cast<const SvxBrushItem*>(pOld)->GetColor();
    }

    if ( nDestValue != USHRT_MAX )
    {
        if (m_xTblLBox->get_visible())
        {
            m_pCellBrush.reset();
            m_pRowBrush.reset();
            m_pTableBrush.reset();
            m_nActPos = m_xTblLBox->get_active();

            nWhich = GetWhich( SID_ATTR_BRUSH );
            if ( rSet->GetItemState( nWhich, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(nWhich));
                m_pCellBrush.reset(new SvxBrushItem(aBgdAttr));
            }
            m_nCellWhich = nWhich;

            if ( rSet->GetItemState( SID_ATTR_BRUSH_ROW, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(SID_ATTR_BRUSH_ROW));
                m_pRowBrush.reset(new SvxBrushItem(aBgdAttr));
            }
            m_nRowWhich = SID_ATTR_BRUSH_ROW;

            if ( rSet->GetItemState( SID_ATTR_BRUSH_TABLE, false ) >= SfxItemState::DEFAULT )
            {
                aBgdAttr = static_cast<const SvxBrushItem&>(rSet->Get(SID_ATTR_BRUSH_TABLE));
                m_pTableBrush.reset(new SvxBrushItem(aBgdAttr));
            }
            m_nTableWhich = SID_ATTR_BRUSH_TABLE;

            TblDestinationHdl_Impl(*m_xTblLBox);
            m_xTblLBox->save_value();
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
    SetUserData(m_xBtnPreview->get_active() ? OUString('1') : OUString('0'));
}

bool SvxBackgroundTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( m_pLoadIdle && m_pLoadIdle->IsActive() )
    {
        m_pLoadIdle->Stop();
        LoadIdleHdl_Impl( m_pLoadIdle.get() );
    }

    bool bModified = false;
    bool bCompareOldBrush = true;
    sal_uInt16 nSlot = SID_ATTR_BRUSH;

    if (m_xTblLBox->get_visible())
    {
        switch (m_xTblLBox->get_active())
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
        const bool          bIsBrush    = ( drawing::FillStyle_SOLID == lcl_getFillStyle(*m_xLbSelect) );

        if (   ( (GPOS_NONE == eOldPos) && bIsBrush  )
            || ( (GPOS_NONE != eOldPos) && !bIsBrush ) ) // Brush <-> Bitmap changed?
        {
            // background art hasn't been changed:

            if (GPOS_NONE == eOldPos || !m_xLbSelect->get_visible())
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
                const bool          bIsLink  = m_xBtnLink->get_active();
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
                if ( m_xBtnLink->get_active() )
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
            bModified = ( bIsBrush || m_xBtnLink->get_active() || bIsGraphicValid );
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

    if (m_xTblLBox->get_visible())
    {
        // the current condition has already been put
        if( nSlot != SID_ATTR_BRUSH && m_pCellBrush)
        {
            const SfxPoolItem* pOldCell =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH );

            if ( *m_pCellBrush != *pOldCell )
            {
                rCoreSet->Put( *m_pCellBrush );
                bModified = true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_ROW && m_pRowBrush)
        {
            const SfxPoolItem* pOldRow =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH_ROW );

            if ( *m_pRowBrush != *pOldRow )
            {
                rCoreSet->Put( *m_pRowBrush );
                bModified = true;
            }
        }

        if( nSlot != SID_ATTR_BRUSH_TABLE && m_pTableBrush)
        {
            const SfxPoolItem* pOldTable =
                GetOldItem( *rCoreSet, SID_ATTR_BRUSH_TABLE );

            if ( *m_pTableBrush != *pOldTable )
            {
                rCoreSet->Put( *m_pTableBrush );
                bModified = true;
            }
        }

        if (m_xTblLBox->get_value_changed_from_saved())
        {
            rCoreSet->Put(SfxUInt16Item(SID_BACKGRND_DESTINATION,
                                        m_xTblLBox->get_active()));
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
    if ( m_bIsImportDlgInExecute )
        return DeactivateRC::KeepPage;

    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

void SvxBackgroundTabPage::PointChanged( weld::DrawingArea*, RectPoint )
{
    // has to be implemented so that position control can work
}

void SvxBackgroundTabPage::ShowSelector()
{
    if( bAllowShowSelector)
    {
        m_xAsGrid->show();
        m_xSelectTxt->show();
        m_xLbSelect->show();
        m_xLbSelect->connect_changed(HDL(SelectHdl_Impl));
        m_xBtnLink->connect_toggled(HDL(FileClickHdl_Impl));
        m_xBtnPreview->connect_toggled(HDL(FileClickHdl_Impl));
        m_xBtnBrowse->connect_clicked(HDL(BrowseHdl_Impl));
        m_xBtnArea->connect_toggled(HDL(RadioClickHdl_Impl));
        m_xBtnTile->connect_toggled(HDL(RadioClickHdl_Impl));
        m_xBtnPosition->connect_toggled(HDL(RadioClickHdl_Impl));

        // delayed loading via timer (because of UI-Update)
        m_pLoadIdle.reset( new Idle("DelayedLoad") );
        m_pLoadIdle->SetPriority( TaskPriority::LOWEST );
        m_pLoadIdle->SetInvokeHandler(
            LINK( this, SvxBackgroundTabPage, LoadIdleHdl_Impl ) );

        bAllowShowSelector = false;

        if (nHtmlMode & HTMLMODE_ON)
        {
            m_xBtnArea->set_sensitive(false);
        }
    }
}

void SvxBackgroundTabPage::RaiseLoadError_Impl()
{
    SfxErrorContext aContext( ERRCTX_SVX_BACKGROUND,
                              OUString(),
                              GetDialogFrameWeld(),
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
    XColorListRef pColorTable;
    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
    {
        pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    if ( pColorTable.is() )
    {
        m_xBackgroundColorSet->Clear();
        m_xBackgroundColorSet->addEntriesForXColorList(*pColorTable);
    }

    const WinBits nBits(m_xBackgroundColorSet->GetStyle() | WB_ITEMBORDER | WB_NAMEFIELD | WB_NONEFIELD);
    m_xBackgroundColorSet->SetStyle(nBits);
    m_xBackgroundColorSet->SetColCount(SvxColorValueSet::getColumnCount());
}

/** Hide the controls for editing the bitmap
    and show the controls for color settings instead.
*/
void SvxBackgroundTabPage::ShowColorUI_Impl()
{
    if (!m_xBackGroundColorFrame->get_visible())
    {
        HideBitmapUI_Impl();
        m_xBackGroundColorFrame->show();
    }
}

void SvxBackgroundTabPage::HideColorUI_Impl()
{
    m_xBackGroundColorFrame->hide();
}

/** Hide the controls for color settings
    and show controls for editing the bitmap instead.
*/
void SvxBackgroundTabPage::ShowBitmapUI_Impl()
{
    if (m_xLbSelect->get_visible() &&
       (m_xBackGroundColorFrame->get_visible() || !m_xFileFrame->get_visible()))
    {
        HideColorUI_Impl();

        m_xBitmapContainer->show();
        m_xFileFrame->show();
        m_xBtnLink->set_visible(!(nHtmlMode & HTMLMODE_ON));
        m_xTypeFrame->show();
        m_xPreviewWin2->show();
        m_xBtnPreview->show();
    }
}

void SvxBackgroundTabPage::HideBitmapUI_Impl()
{
    m_xBitmapContainer->hide();
    m_xFileFrame->hide();
    m_xTypeFrame->hide();
    m_xPreviewWin2->hide();
    m_xBtnPreview->hide();
}

void SvxBackgroundTabPage::SetGraphicPosition_Impl( SvxGraphicPosition ePos )
{
    switch ( ePos )
    {
        case GPOS_AREA:
        {
            m_xBtnArea->set_active(true);
            m_xWndPositionWin->set_sensitive(false);
        }
        break;

        case GPOS_TILED:
        {
            m_xBtnTile->set_active(true);
            m_xWndPositionWin->set_sensitive(false);
        }
        break;

        default:
        {
            m_xBtnPosition->set_active(true);
            m_xWndPositionWin->set_sensitive(true);
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
            m_xWndPosition->SetActualRP( eNewPos );
        }
        break;
    }
    m_xWndPosition->Invalidate();
}

SvxGraphicPosition SvxBackgroundTabPage::GetGraphicPosition_Impl()
{
    if (m_xBtnTile->get_active())
        return GPOS_TILED;
    else if (m_xBtnArea->get_active())
        return GPOS_AREA;
    else
    {
        switch (m_xWndPosition->GetActualRP())
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
IMPL_LINK_NOARG(SvxBackgroundTabPage, BackgroundColorHdl_Impl, SvtValueSet*, void)
{
    sal_uInt16 nItemId = m_xBackgroundColorSet->GetSelectedItemId();
    Color aColor = nItemId ? ( m_xBackgroundColorSet->GetItemColor( nItemId ) ) : COL_TRANSPARENT;
    aBgdColor = aColor;
    m_bColorSelected = true;
    m_xPreview1->NotifyChange( aBgdColor );
}

IMPL_LINK_NOARG(SvxBackgroundTabPage, SelectHdl_Impl, weld::ComboBox&, void)
{
    if ( drawing::FillStyle_SOLID == lcl_getFillStyle(*m_xLbSelect) )
    {
        ShowColorUI_Impl();
    }
    else
    {
        ShowBitmapUI_Impl();
    }
}

IMPL_LINK(SvxBackgroundTabPage, FileClickHdl_Impl, weld::ToggleButton&, rBox, void)
{
    if (m_xBtnLink.get() == &rBox)
    {
        if (m_xBtnLink->get_active())
        {
            m_xFtUnlinked->hide();
            INetURLObject aObj( aBgdGraphicPath );
            OUString aFilePath;
            if ( aObj.GetProtocol() == INetProtocol::File )
                aFilePath = aObj.getFSysPath( FSysStyle::Detect );
            else
                aFilePath = aBgdGraphicPath;
            m_xFtFile->set_label(aFilePath);
            m_xFtFile->show();
        }
        else
        {
            m_xFtUnlinked->show();
            m_xFtFile->hide();
        }
    }
    else if (m_xBtnPreview.get() == &rBox)
    {
        if (m_xBtnPreview->get_active())
        {
            if ( !bIsGraphicValid )
                bIsGraphicValid = LoadLinkedGraphic_Impl();

            if ( bIsGraphicValid )
            {
                Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
                m_xPreview2->NotifyChange( &aBmp );
            }
            else
            {
                if ( !aBgdGraphicPath.isEmpty() ) // only for linked bitmap
                    RaiseLoadError_Impl();
                m_xPreview2->NotifyChange( nullptr );
            }
        }
        else
            m_xPreview2->NotifyChange( nullptr );
    }
}

IMPL_LINK(SvxBackgroundTabPage, RadioClickHdl_Impl, weld::ToggleButton&, rBtn, void)
{
    if (&rBtn == m_xBtnPosition.get())
    {
        if (!m_xWndPositionWin->get_sensitive())
        {
            m_xWndPositionWin->set_sensitive(true);
            m_xWndPositionWin->queue_draw();
        }
    }
    else if (m_xWndPositionWin->get_sensitive())
    {
        m_xWndPositionWin->set_sensitive(false);
        m_xWndPositionWin->queue_draw();
    }
}

/** Handler, called by pressing the browse button.
    Create graphic/insert dialog, set path and start.
*/
IMPL_LINK_NOARG(SvxBackgroundTabPage, BrowseHdl_Impl, weld::Button&, void)
{
    if ( m_pLoadIdle->IsActive() )
        return;
    bool bHtml = 0 != ( nHtmlMode & HTMLMODE_ON );

    OUString aStrBrowse(m_xFindGraphicsFt->get_label());
    pImportDlg.reset(new SvxOpenGraphicDialog(aStrBrowse, GetDialogFrameWeld()));
    if ( bHtml )
        pImportDlg->EnableLink(false);
    pImportDlg->SetPath(aBgdGraphicPath, m_xBtnLink->get_active());

    m_bIsImportDlgInExecute = true;
    ErrCode nErr = pImportDlg->Execute();
    m_bIsImportDlgInExecute = false;

    if( !nErr )
    {
        if (bHtml)
            m_xBtnLink->set_active(true);
        // if link isn't checked and preview isn't, either,
        // activate preview, so that the user sees which
        // graphic he has chosen
        if (!m_xBtnLink->get_active() && !m_xBtnPreview->get_active())
            m_xBtnPreview->set_active(true);
        // timer-delayed loading of the graphic
        m_pLoadIdle->Start();
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
    if ( pIdle == m_pLoadIdle.get() )
    {
        m_pLoadIdle->Stop();

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
                m_xBtnLink->set_active(bLink);
                m_xBtnLink->set_sensitive(true);

                if (m_xBtnPreview->get_active())
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

                if (m_xBtnPreview->get_active() && bIsGraphicValid)
                {
                    Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
                    m_xPreview2->NotifyChange( &aBmp );
                }
                else
                    m_xPreview2->NotifyChange( nullptr );
            }

            FileClickHdl_Impl(*m_xBtnLink);
            pImportDlg.reset();
        }
    }
}

void SvxBackgroundTabPage::ShowTblControl()
{
    m_xTblLBox->connect_changed(HDL(TblDestinationHdl_Impl));
    m_xTblLBox->set_active(0);
    m_xTblDesc->show();
    m_xTblLBox->show();
    m_xAsGrid->show();
}

IMPL_LINK(SvxBackgroundTabPage, TblDestinationHdl_Impl, weld::ComboBox&, rBox, void)
{
    int nSelPos = rBox.get_active();
    if( m_nActPos != nSelPos)
    {
        std::unique_ptr<SvxBrushItem> xItemHolder;
        SvxBrushItem* pActItem = nullptr;
        sal_uInt16 nWhich = 0;
        switch(m_nActPos)
        {
        case TBL_DEST_CELL:
            pActItem = m_pCellBrush.get();
            nWhich = m_nCellWhich;
            break;
        case TBL_DEST_ROW:
            pActItem = m_pRowBrush.get();
            nWhich = m_nRowWhich;
            break;
        case TBL_DEST_TBL:
            pActItem = m_pTableBrush.get();
            nWhich = m_nTableWhich;
            break;
        default:
            pActItem = nullptr;
            break;
        }
        m_nActPos = nSelPos;
        if(!pActItem)
        {
            xItemHolder.reset(new SvxBrushItem(nWhich));
            pActItem = xItemHolder.get();
        }
        if(drawing::FillStyle_SOLID == lcl_getFillStyle(*m_xLbSelect))  // brush selected
        {
            *pActItem = SvxBrushItem( aBgdColor, nWhich );
        }
        else
        {
            SvxGraphicPosition  eNewPos  = GetGraphicPosition_Impl();
            const bool bIsLink  = m_xBtnLink->get_active();

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
            pActItem = m_pCellBrush.get();
            m_xLbSelect->set_sensitive(true);
            nWhich = m_nCellWhich;
            break;
        case TBL_DEST_ROW:
            pActItem = m_pRowBrush.get();
            nWhich = m_nRowWhich;
            break;
        case TBL_DEST_TBL:
            pActItem = m_pTableBrush.get();
            m_xLbSelect->set_sensitive(true);
            nWhich = m_nTableWhich;
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

    if (GPOS_NONE == ePos || !m_xLbSelect->get_visible())
    {
        lcl_setFillStyle(*m_xLbSelect, drawing::FillStyle_SOLID);
        ShowColorUI_Impl();
        Color aTrColor( COL_TRANSPARENT );
        aBgdColor = rColor;

        sal_uInt16 nCol = ( aTrColor != aBgdColor ) ?
            GetItemId_Impl(*m_xBackgroundColorSet, aBgdColor) : 0;

        if( aTrColor != aBgdColor && nCol == 0)
        {
            m_xBackgroundColorSet->SetNoSelection();
        }
        else
        {
            m_xBackgroundColorSet->SelectItem( nCol );
        }

        m_xPreview1->NotifyChange( aBgdColor );

        if (m_xLbSelect->get_visible()) // initialize graphic part
        {
            aBgdGraphicFilter.clear();
            aBgdGraphicPath.clear();

            if ( rUserData.isEmpty() )
                m_xBtnPreview->set_active(false);
            m_xBtnLink->set_active(false);
            m_xBtnLink->set_sensitive(false);
            m_xPreview2->NotifyChange(nullptr);
            SetGraphicPosition_Impl( GPOS_TILED );  // tiles as default
        }
    }
    else
    {
        const OUString&  aStrLink   = rBgdAttr.GetGraphicLink();
        const OUString&  aStrFilter = rBgdAttr.GetGraphicFilter();

        lcl_setFillStyle(*m_xLbSelect, drawing::FillStyle_BITMAP);
        ShowBitmapUI_Impl();

        if ( !aStrLink.isEmpty() )
        {
#ifdef DBG_UTIL
            INetURLObject aObj( aStrLink );
            DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );
#endif
            aBgdGraphicPath = aStrLink;
            m_xBtnLink->set_active(true);
            m_xBtnLink->set_sensitive(true);
        }
        else
        {
            aBgdGraphicPath.clear();
            m_xBtnLink->set_active(false);
            m_xBtnLink->set_sensitive(false);
        }

        FileClickHdl_Impl(*m_xBtnLink);

        aBgdGraphicFilter = aStrFilter;

        if (aStrLink.isEmpty() || m_xBtnPreview->get_active())
        {
            // Graphic exists in the item and doesn't have
            // to be loaded:

            const Graphic* pGraphic = rBgdAttr.GetGraphic();

            if (!pGraphic && m_xBtnPreview->get_active())
                bIsGraphicValid = LoadLinkedGraphic_Impl();
            else if ( pGraphic )
            {
                aBgdGraphic = *pGraphic;
                bIsGraphicValid = true;

                if (rUserData.isEmpty())
                    m_xBtnPreview->set_active(true);
            }
            else
            {
                RaiseLoadError_Impl();
                bIsGraphicValid = false;

                if ( rUserData.isEmpty() )
                    m_xBtnPreview->set_active(false);
            }
        }

        if (m_xBtnPreview->get_active() && bIsGraphicValid)
        {
            Bitmap aBmp = aBgdGraphic.GetBitmapEx().GetBitmap();
            m_xPreview2->NotifyChange( &aBmp );
        }
        else
            m_xPreview2->NotifyChange( nullptr );

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
            m_xBackGroundColorLabelFT->set_label(CuiResId(RID_SVXSTR_CHARNAME_HIGHLIGHTING));
            bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
    }
}

static sal_uInt16 lcl_GetTableDestSlot(sal_Int32 nTblDest)
{
    switch (nTblDest)
    {
        default:
        case TBL_DEST_CELL:
        {
            return SID_ATTR_BRUSH;
        }
        case TBL_DEST_ROW:
        {
            return SID_ATTR_BRUSH_ROW;
        }
        case TBL_DEST_TBL:
        {
            return SID_ATTR_BRUSH_TABLE;
        }
    }
}

SvxBkgTabPage::SvxBkgTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxAreaTabPage(pParent, rInAttrs),
    bHighlighting(false),
    bCharBackColor(false),
    maSet(rInAttrs)
{
    m_xBtnGradient->hide();
    m_xBtnHatch->hide();
    m_xBtnBitmap->hide();
    m_xBtnPattern->hide();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = nullptr;

    XColorListRef pColorTable;
    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) ) )
    {
        pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    XBitmapListRef pBitmapList;
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
    m_xTblLBox.reset();
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

void SvxBkgTabPage::Reset( const SfxItemSet* )
{
    maSet.Set( *m_pResetSet.get() );
    if ( m_xTblLBox && m_xTblLBox->get_visible() )
    {
        m_nActPos = -1;
        const SfxPoolItem* pItem;
        if ( SfxItemState::SET == m_pResetSet->GetItemState( SID_BACKGRND_DESTINATION, false, &pItem ) )
        {
            sal_uInt16 nDestValue = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            m_xTblLBox->set_active( nDestValue );
            TblDestinationHdl_Impl( *m_xTblLBox );
        }
        m_xTblLBox->save_value();
    }
    SvxAreaTabPage::Reset( &maSet );
}

bool SvxBkgTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    if (m_xTblLBox && m_xTblLBox->get_visible())
        nSlot = lcl_GetTableDestSlot(m_xTblLBox->get_active());
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
            if ( GraphicType::NONE != aBrushItem.GetGraphicObject()->GetType() )
                rCoreSet->Put( aBrushItem );
            break;
        }
        default:
            break;
    }

    if (m_xTblLBox && m_xTblLBox->get_visible())
    {
        if (nSlot != SID_ATTR_BRUSH)
        {
            nWhich = maSet.GetPool()->GetWhich(SID_ATTR_BRUSH);
            if (SfxItemState::SET == maSet.GetItemState(nWhich))
            {
                SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
                rCoreSet->Put(aBrushItem);
            }
        }
        if (nSlot != SID_ATTR_BRUSH_ROW)
        {
            nWhich = maSet.GetPool()->GetWhich(SID_ATTR_BRUSH_ROW);
            if (SfxItemState::SET == maSet.GetItemState(nWhich))
            {
                SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
                rCoreSet->Put(aBrushItem);
            }
        }
        if (nSlot != SID_ATTR_BRUSH_TABLE)
        {
            nWhich = maSet.GetPool()->GetWhich(SID_ATTR_BRUSH_TABLE);
            if (SfxItemState::SET == maSet.GetItemState(nWhich))
            {
                SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
                rCoreSet->Put(aBrushItem);
            }
        }

        if (m_xTblLBox->get_value_changed_from_saved())
        {
            rCoreSet->Put(SfxUInt16Item(SID_BACKGRND_DESTINATION, m_xTblLBox->get_active()));
        }
    }

    return true;
}

VclPtr<SfxTabPage> SvxBkgTabPage::Create(TabPageParent pWindow, const SfxItemSet* rAttrs)
{
    auto xRet = VclPtr<SvxBkgTabPage>::Create(pWindow, *rAttrs);
    xRet->SetOptimalSize(pWindow.pController);
    return xRet;
}

void SvxBkgTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        SvxBackgroundTabFlags nFlags = static_cast<SvxBackgroundTabFlags>(pFlagItem->GetValue());
        if ( nFlags & SvxBackgroundTabFlags::SHOW_TBLCTL )
        {
            m_xBtnBitmap->show();
            m_xTblLBox = m_xBuilder->weld_combo_box("tablelb");
            m_xTblLBox->connect_changed(LINK(this, SvxBkgTabPage, TblDestinationHdl_Impl));
            m_xTblLBox->show();
        }
        if ((nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING) ||
            (nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR))
        {
            bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
        if (nFlags & SvxBackgroundTabFlags::SHOW_SELECTOR)
            m_xBtnBitmap->show();
        SetOptimalSize(GetDialogController());
    }

    if ( bCharBackColor )
    {
        sal_uInt16 nWhich(maSet.GetPool()->GetWhich(SID_ATTR_CHAR_BACK_COLOR));
        Color aBackColor(static_cast<const SvxBackgroundColorItem&>(maSet.Get(nWhich)).GetValue());
        SvxBrushItem aBrushItem(SvxBrushItem(aBackColor, SID_ATTR_BRUSH_CHAR));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
    }
    else
    {
        sal_uInt16 nWhich(maSet.GetPool()->GetWhich(bHighlighting ? SID_ATTR_BRUSH_CHAR : SID_ATTR_BRUSH));
        SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
    }

    m_pResetSet = maSet.Clone();

    SvxAreaTabPage::PageCreated(aSet);
}

IMPL_LINK(SvxBkgTabPage, TblDestinationHdl_Impl, weld::ComboBox&, rBox, void)
{
    if (m_nActPos > -1)
    {
        // fill local item set with XATTR_FILL settings gathered from tab page
        // and convert to SvxBrushItem and store in table destination slot Which
        SvxAreaTabPage::FillItemSet(&maSet);
        maSet.Put(getSvxBrushItemFromSourceSet(maSet, maSet.GetPool()->GetWhich(lcl_GetTableDestSlot(m_nActPos))));
    }

    sal_Int32 nSelPos = rBox.get_active();
    if (m_nActPos != nSelPos)
    {
        m_nActPos = nSelPos;

        // fill local item set with XATTR_FILL created from SvxBushItem for table destination slot Which
        sal_uInt16 nWhich = maSet.GetPool()->GetWhich(lcl_GetTableDestSlot(nSelPos));
        if (SfxItemState::SET == maSet.GetItemState(nWhich))
        {
            SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
            setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
        }
        else
        {
            SelectFillType(*m_xBtnNone, &maSet);
            return;
        }

        // show tab page
        drawing::FillStyle eXFS = drawing::FillStyle_NONE;
        if (maSet.GetItemState(XATTR_FILLSTYLE) != SfxItemState::DONTCARE)
        {
            XFillStyleItem aFillStyleItem(static_cast<const XFillStyleItem&>(maSet.Get(GetWhich( XATTR_FILLSTYLE))));
            eXFS = aFillStyleItem.GetValue();
        }
        switch(eXFS)
        {
            default:
            case drawing::FillStyle_NONE:
            {
                SelectFillType(*m_xBtnNone, &maSet);
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                SelectFillType(*m_xBtnColor, &maSet);
                // color tab page Active and New preview controls are same after SelectFillType
                // hack to restore color tab page Active preview
                setSvxBrushItemAsFillAttributesToTargetSet(static_cast<const SvxBrushItem&>(m_pResetSet->Get(nWhich)), *m_pResetSet);
                static_cast<SvxColorTabPage*>(GetFillTabPage())->SetCtlPreviewOld(*m_pResetSet);
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                SelectFillType(*m_xBtnBitmap, &maSet);
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
