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
#include <stdlib.h>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
#include <strings.hrc>
#include <svx/xflbckit.hxx>
#include <svx/svdattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include "sfx2/opengrf.hxx"
#include <vcl/layout.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dialoghelper.hxx>
#include <o3tl/make_unique.hxx>

using namespace com::sun::star;

enum BitmapStyle
{
    ORIGINAL,
    FILLED,
    STRETCHED,
    ZOOMED,
    CUSTOM,
    TILED
};

enum TileOffset
{
    ROW,
    COLUMN
};

const sal_uInt16 SvxBitmapTabPage::pBitmapRanges[] =
{
    SID_ATTR_TRANSFORM_WIDTH,
    SID_ATTR_TRANSFORM_HEIGHT,
    0
};

SvxBitmapTabPage::SvxBitmapTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage( pParent,
                "BitmapTabPage",
                "cui/ui/bitmaptabpage.ui",
               rInAttrs ),
    m_rOutAttrs (rInAttrs ),

    m_pBitmapList( nullptr ),

    m_pnBitmapListState( nullptr ),
    m_aXFStyleItem( drawing::FillStyle_BITMAP ),
    m_aXBitmapItem( OUString(), Graphic() ),
    m_fObjectWidth(0.0),
    m_fObjectHeight(0.0),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() ),
    mpView(nullptr),
    nFilledWidthPercent(0),
    nFilledHeightPercent(0),
    nZoomedWidthPercent(0),
    nZoomedHeightPercent(0)
{
    get(m_pBitmapLB,"BITMAP");
    get(m_pBitmapStyleLB, "bitmapstyle");
    get(m_pSizeBox, "sizebox");
    get(m_pBitmapWidth, "width");
    get(m_pBitmapHeight, "height");
    get(m_pTsbScale, "scaletsb");
    get(m_pPositionBox, "posbox");
    get(m_pPositionLB, "positionlb");
    get(m_pPositionOffBox, "posoffbox");
    get(m_pPositionOffX, "posoffx");
    get(m_pPositionOffY, "posoffy");
    get(m_pTileOffBox, "tileoffbox");
    get(m_pTileOffLB, "tileofflb");
    get(m_pTileOffset, "tileoffmtr");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");
    get(m_pBtnImport, "BTN_IMPORT");

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXBitmapItem );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pBitmapLB->SetSelectHdl( LINK(this, SvxBitmapTabPage, ModifyBitmapHdl) );
    m_pBitmapLB->SetRenameHdl( LINK(this, SvxBitmapTabPage, ClickRenameHdl) );
    m_pBitmapLB->SetDeleteHdl( LINK(this, SvxBitmapTabPage, ClickDeleteHdl) );
    m_pBitmapStyleLB->SetSelectHdl( LINK(this, SvxBitmapTabPage, ModifyBitmapStyleHdl) );
    Link<Edit&, void> aLink1( LINK(this, SvxBitmapTabPage, ModifyBitmapSizeHdl) );
    m_pBitmapWidth->SetModifyHdl( aLink1 );
    m_pBitmapHeight->SetModifyHdl( aLink1 );
    m_pTsbScale->SetClickHdl( LINK(this, SvxBitmapTabPage, ClickScaleHdl) );
    m_pPositionLB->SetSelectHdl( LINK( this, SvxBitmapTabPage, ModifyBitmapPositionHdl ) );
    Link<Edit&, void> aLink( LINK( this, SvxBitmapTabPage, ModifyPositionOffsetHdl ) );
    m_pPositionOffX->SetModifyHdl(aLink);
    m_pPositionOffY->SetModifyHdl(aLink);
    m_pTileOffset->SetModifyHdl( LINK( this, SvxBitmapTabPage, ModifyTileOffsetHdl ) );
    m_pBtnImport->SetClickHdl( LINK(this, SvxBitmapTabPage, ClickImportHdl) );

    // Calculate size of display boxes
    Size aSize = getDrawPreviewOptimalSize(this);
    m_pBitmapLB->set_width_request(aSize.Width());
    m_pBitmapLB->set_height_request(aSize.Height());
    m_pCtlBitmapPreview->set_width_request(aSize.Width());
    m_pCtlBitmapPreview->set_height_request(aSize.Height());

    SfxItemPool* pPool = m_rXFSet.GetPool();
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_WIDTH );
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell )
        mpView = pViewShell->GetDrawView();
    DBG_ASSERT( mpView, "no valid view (!)" );
}

SvxBitmapTabPage::~SvxBitmapTabPage()
{
    disposeOnce();
}

void SvxBitmapTabPage::dispose()
{
    m_pBitmapLB.clear();
    m_pBitmapStyleLB.clear();
    m_pSizeBox.clear();
    m_pBitmapWidth.clear();
    m_pBitmapHeight.clear();
    m_pTsbScale.clear();
    m_pPositionBox.clear();
    m_pPositionLB.clear();
    m_pPositionOffBox.clear();
    m_pPositionOffX.clear();
    m_pPositionOffY.clear();
    m_pTileOffBox.clear();
    m_pTileOffLB.clear();
    m_pTileOffset.clear();
    m_pCtlBitmapPreview.clear();
    m_pBtnImport.clear();
    SvxTabPage::dispose();
}


void SvxBitmapTabPage::Construct()
{
    m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
}


void SvxBitmapTabPage::ActivatePage( const SfxItemSet& rSet )
{
    XFillBitmapItem aItem( static_cast<const XFillBitmapItem&>(rSet.Get(XATTR_FILLBITMAP)) );

    sal_Int32 nPos = SearchBitmapList( aItem.GetName() );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_uInt16 nId = m_pBitmapLB->GetItemId( static_cast<size_t>( nPos ) );
        m_pBitmapLB->SelectItem( nId );
    }
}


DeactivateRC SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxBitmapTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    rAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
    size_t nPos = m_pBitmapLB->GetSelectItemPos();
    if(VALUESET_ITEM_NOTFOUND != nPos)
    {
        const XBitmapEntry* pXBitmapEntry = m_pBitmapList->GetBitmap(nPos);
        const OUString aString(m_pBitmapLB->GetItemText( m_pBitmapLB->GetSelectItemId() ));
        rAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
    }

    BitmapStyle eStylePos = (BitmapStyle)m_pBitmapStyleLB->GetSelectEntryPos();        bool bIsStretched( eStylePos == STRETCHED );
    bool bIsTiled( eStylePos == TILED );

    rAttrs->Put( XFillBmpTileItem(bIsTiled) );
    rAttrs->Put( XFillBmpStretchItem(bIsStretched) );

    if(!bIsStretched)
    {
        Size aSetBitmapSize;
        switch(eStylePos)
        {
            case ORIGINAL:
                aSetBitmapSize = rBitmapSize;
                break;
            case FILLED:
                aSetBitmapSize = rFilledSize;
                break;
            case ZOOMED:
                aSetBitmapSize = rZoomedSize;
                break;
            case CUSTOM:
            case TILED:
            {
                sal_Int64 nWidthPercent = m_pBitmapWidth->Denormalize(m_pBitmapWidth->GetValue());
                sal_Int64 nHeightPercent = m_pBitmapHeight->Denormalize(m_pBitmapHeight->GetValue());
                if(eStylePos == CUSTOM && m_pTsbScale->IsEnabled() && m_pTsbScale->GetState() == TRISTATE_TRUE)
                {
                    aSetBitmapSize.Width() = -nWidthPercent;
                    aSetBitmapSize.Height() = -nHeightPercent;
                }
                else
                {
                    aSetBitmapSize.Width() = static_cast<long>(nWidthPercent*rBitmapSize.Width()/100);
                    aSetBitmapSize.Height() = static_cast<long>(nHeightPercent*rBitmapSize.Height()/100);
                }
                break;
            }
            default:
                break;
        }
        rAttrs->Put( XFillBmpSizeXItem( aSetBitmapSize.Width() ) );
        rAttrs->Put( XFillBmpSizeYItem( aSetBitmapSize.Height() ) );
    }

    if(m_pPositionLB->IsEnabled())
        rAttrs->Put( XFillBmpPosItem( static_cast<RectPoint>( m_pPositionLB->GetSelectEntryPos() ) ) );
    if(m_pPositionOffX->IsEnabled())
        rAttrs->Put( XFillBmpPosOffsetXItem( m_pPositionOffX->GetValue() ) );
    if(m_pPositionOffY->IsEnabled())
        rAttrs->Put( XFillBmpPosOffsetYItem( m_pPositionOffY->GetValue() ) );
    if(m_pTileOffBox->IsEnabled())
    {
        TileOffset eValue = static_cast<TileOffset>(m_pTileOffLB->GetSelectEntryPos());
        sal_uInt16 nOffsetValue = static_cast<sal_uInt16>(m_pTileOffset->GetValue());
        sal_uInt16 nRowOff = (eValue == ROW) ? nOffsetValue : 0;
        sal_uInt16 nColOff = (eValue == COLUMN) ? nOffsetValue : 0;
        rAttrs->Put( XFillBmpTileOffsetXItem(nRowOff) );
        rAttrs->Put( XFillBmpTileOffsetYItem(nColOff) );
    }
    return true;
}


void SvxBitmapTabPage::Reset( const SfxItemSet* rAttrs )
{
    const SfxPoolItem* pItemTransfWidth = nullptr;
    const SfxPoolItem* pItemTransfHeight = nullptr;
    double fUIScale  = 1.0;
    if (mpView)
    {
        fUIScale  = ( mpView->GetModel() ? double(mpView->GetModel()->GetUIScale()) : 1.0);


        if (mpView->AreObjectsMarked())
        {
            SfxItemSet rGeoAttr(mpView->GetGeoAttrFromMarked());
            pItemTransfWidth = GetItem( rGeoAttr, SID_ATTR_TRANSFORM_WIDTH );
            pItemTransfHeight= GetItem( rGeoAttr, SID_ATTR_TRANSFORM_HEIGHT );
        }
    }
    m_fObjectWidth = std::max( pItemTransfWidth ? (double)static_cast<const SfxUInt32Item*>(pItemTransfWidth)->GetValue() : 0.0, 1.0 );
    m_fObjectHeight = std::max( pItemTransfHeight ? (double)static_cast<const SfxUInt32Item*>(pItemTransfHeight)->GetValue() : 0.0, 1.0 );
    double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(m_fObjectWidth), mePoolUnit, MapUnit::Map100thMM )) / fUIScale);
    m_fObjectWidth = fTmpWidth;

    double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(m_fObjectHeight), mePoolUnit, MapUnit::Map100thMM )) / fUIScale);
    m_fObjectHeight = fTmpHeight;

    XFillBitmapItem aItem( static_cast<const XFillBitmapItem&>(rAttrs->Get(XATTR_FILLBITMAP)) );

    if(!aItem.isPattern())
    {
        m_rXFSet.Put( aItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
        m_pCtlBitmapPreview->Disable();

    std::unique_ptr<GraphicObject> pGraphicObject;
    pGraphicObject.reset( new GraphicObject(aItem.GetGraphicObject()) );

    if(pGraphicObject)
    {
        BitmapEx aBmpEx(pGraphicObject->GetGraphic().GetBitmapEx());
        Size aTempBitmapSize = aBmpEx.GetSizePixel();
        rBitmapSize.Width() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Width()),MapUnit::MapPixel, MapUnit::Map100thMM )) / fUIScale);
        rBitmapSize.Height() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Height()),MapUnit::MapPixel, MapUnit::Map100thMM )) / fUIScale);
        CalculateBitmapPresetSize();
    }

    if(rAttrs->GetItemState( XATTR_FILLBMP_TILE ) != SfxItemState::DONTCARE)
    {
        if( static_cast<const XFillBmpTileItem&>( rAttrs->Get( XATTR_FILLBMP_TILE ) ).GetValue() )
        {
            m_pBitmapStyleLB->SelectEntryPos(static_cast<sal_Int32>(TILED));
        }
    }

    if( m_pBitmapStyleLB->GetSelectEntryPos() == 0 && rAttrs->GetItemState( XATTR_FILLBMP_STRETCH ) != SfxItemState::DONTCARE)
    {
        if( static_cast<const XFillBmpStretchItem&>( rAttrs->Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
        {
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(STRETCHED) );
        }
    }

    long nWidth = 0;
    long nHeight = 0;

    TriState eRelative = TRISTATE_FALSE;
    if(rAttrs->GetItemState(XATTR_FILLBMP_SIZEX) != SfxItemState::DONTCARE)
    {
        nWidth = static_cast<const XFillBmpSizeXItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if(nWidth == 0)
            nWidth = rBitmapSize.Width();
        else if(nWidth < 0)
        {
            eRelative = TRISTATE_TRUE;
            nWidth = std::abs(nWidth);
        }
    }

    if(rAttrs->GetItemState( XATTR_FILLBMP_SIZEY ) != SfxItemState::DONTCARE)
    {
        nHeight = static_cast<const XFillBmpSizeYItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        if(nHeight == 0)
            nHeight = rBitmapSize.Height();
        else if(nHeight < 0)
        {
            eRelative = TRISTATE_TRUE;
            nHeight = std::abs(nHeight);
        }
    }
    m_pTsbScale->SetState(eRelative);

    if(eRelative != TRISTATE_TRUE)
    {
        nWidth = (OutputDevice::LogicToLogic(nWidth, mePoolUnit, MapUnit::Map100thMM )) / fUIScale;
        nHeight = (OutputDevice::LogicToLogic(nHeight, mePoolUnit, MapUnit::Map100thMM )) / fUIScale;
    }

    if(m_pBitmapStyleLB->GetSelectEntryPos() == 0)
    {
        if( nWidth == rBitmapSize.Width() && nHeight == rBitmapSize.Height() )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(ORIGINAL) );
        else if( nWidth == rFilledSize.Width() && nHeight == rFilledSize.Height() )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(FILLED) );
        else if( nWidth == rZoomedSize.Width() && nHeight == rZoomedSize.Height() )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(ZOOMED) );
        else
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(CUSTOM) );
    }

    if(rBitmapSize.Width() > 0 && rBitmapSize.Height() > 0)
    {
        if(eRelative == TRISTATE_TRUE)
        {
            m_pBitmapWidth->SetValue(nWidth);
            m_pBitmapHeight->SetValue(nHeight);
        }
        else
        {
            sal_Int64 nWidthPercent = m_pBitmapWidth->Normalize(nWidth*100/rBitmapSize.Width());
            m_pBitmapWidth->SetValue(nWidthPercent);
            sal_Int64 nHeightPercent = m_pBitmapHeight->Normalize(nHeight*100/rBitmapSize.Height());
            m_pBitmapHeight->SetValue(nHeightPercent);
        }
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_POS ) != SfxItemState::DONTCARE )
    {
        RectPoint eValue = static_cast<const XFillBmpPosItem&>( rAttrs->Get( XATTR_FILLBMP_POS ) ).GetValue();
        m_pPositionLB->SelectEntryPos( static_cast< sal_Int32 >(eValue) );
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_POSOFFSETX ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpPosOffsetXItem&>( rAttrs->Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        m_pPositionOffX->SetValue( nValue );
    }
    else
        m_pPositionOffX->SetText("");

    if( rAttrs->GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpPosOffsetYItem&>( rAttrs->Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        m_pPositionOffY->SetValue( nValue );
    }
    else
        m_pPositionOffY->SetText("");

    if( rAttrs->GetItemState( XATTR_FILLBMP_TILEOFFSETX ) != SfxItemState::DONTCARE)
    {
        sal_Int32 nValue = static_cast<const XFillBmpTileOffsetXItem&>( rAttrs->Get( XATTR_FILLBMP_TILEOFFSETX ) ).GetValue();
        if(nValue > 0)
        {
            m_pTileOffLB->SelectEntryPos(static_cast<sal_Int32>(ROW));
            m_pTileOffset->SetValue( nValue );
        }
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_TILEOFFSETY ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpTileOffsetYItem&>( rAttrs->Get( XATTR_FILLBMP_TILEOFFSETY ) ).GetValue();
        if(nValue > 0)
        {
            m_pTileOffLB->SelectEntryPos(static_cast<sal_Int32>(COLUMN));
            m_pTileOffset->SetValue( nValue );
        }
    }

    ClickBitmapHdl_Impl();
}


VclPtr<SfxTabPage> SvxBitmapTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxBitmapTabPage>::Create( pWindow, *rAttrs );
}


void SvxBitmapTabPage::ClickBitmapHdl_Impl()
{
    m_pBitmapLB->Enable();
    m_pCtlBitmapPreview->Enable();

    ModifyBitmapHdl( m_pBitmapLB );
}

void SvxBitmapTabPage::CalculateBitmapPresetSize()
{
    if(rBitmapSize.Width() > 0 && rBitmapSize.Height() > 0)
    {
        long nObjectWidth = static_cast<long>(m_fObjectWidth);
        long nObjectHeight = static_cast<long>(m_fObjectHeight);

        if(std::abs(rBitmapSize.Width() - nObjectWidth) < std::abs(rBitmapSize.Height() - nObjectHeight))
        {
            rFilledSize.Width() = nObjectWidth;
            rFilledSize.Height() = rBitmapSize.Height()*nObjectWidth/rBitmapSize.Width();
            rZoomedSize.Width() = rBitmapSize.Width()*nObjectHeight/rBitmapSize.Height();
            rZoomedSize.Height() = nObjectHeight;
        }
        else
        {
            rFilledSize.Width() = rBitmapSize.Width()*nObjectHeight/rBitmapSize.Height();
            rFilledSize.Height() = nObjectHeight;
            rZoomedSize.Width() = nObjectWidth;
            rZoomedSize.Height() = rBitmapSize.Height()*nObjectWidth/rBitmapSize.Width();
        }

        nFilledWidthPercent = static_cast<sal_Int64>(rFilledSize.Width()*100/rBitmapSize.Width());
        nFilledHeightPercent = static_cast<sal_Int64>(rFilledSize.Width()*100/rBitmapSize.Height());
        nZoomedWidthPercent = static_cast<sal_Int64>(rZoomedSize.Width()*100/rBitmapSize.Width());
        nZoomedHeightPercent = static_cast<sal_Int64>(rZoomedSize.Height()*100/rBitmapSize.Height());
    }
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ModifyBitmapHdl, ValueSet*, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    size_t nPos = m_pBitmapLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        pGraphicObject.reset(new GraphicObject(m_pBitmapList->GetBitmap( static_cast<sal_uInt16>(nPos) )->GetGraphicObject()));
    }
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;

        if(SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const drawing::FillStyle eXFS((drawing::FillStyle)static_cast<const XFillStyleItem*>(pPoolItem)->GetValue());

            if((drawing::FillStyle_BITMAP == eXFS) && (SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
            }
        }

        if(!pGraphicObject)
        {
            sal_uInt16 nId = m_pBitmapLB->GetItemId(0);
            m_pBitmapLB->SelectItem(nId);

            if(0 != nId)
            {
                pGraphicObject.reset(new GraphicObject(m_pBitmapList->GetBitmap(0)->GetGraphicObject()));
            }
        }
    }

    if(pGraphicObject)
    {
        BitmapEx aBmpEx(pGraphicObject->GetGraphic().GetBitmapEx());
        Size aTempBitmapSize = aBmpEx.GetSizePixel();
        const double fUIScale = ( (mpView && mpView->GetModel()) ? double(mpView->GetModel()->GetUIScale()) : 1.0);

        rBitmapSize.Width() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Width()),MapUnit::MapPixel, MapUnit::Map100thMM )) / fUIScale);
        rBitmapSize.Height() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Height()),MapUnit::MapPixel, MapUnit::Map100thMM )) / fUIScale);
        CalculateBitmapPresetSize();
        ModifyBitmapStyleHdl( *m_pBitmapStyleLB );
        ModifyBitmapPositionHdl( *m_pPositionLB );

        m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        m_rXFSet.Put(XFillBitmapItem(OUString(), *pGraphicObject));

        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
    {
        SAL_WARN("cui.tabpages", "SvxBitmapTabPage::ModifyBitmapHdl(): null pGraphicObject");
    }

}

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickRenameHdl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pBitmapLB->GetSelectItemId();
    size_t nPos = m_pBitmapLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc( CuiResId( RID_SVXSTR_DESC_NEW_BITMAP ) );
        OUString aName( m_pBitmapList->GetBitmap( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        bool bLoop = true;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nBitmapPos = SearchBitmapList( aName );
            bool bValidBitmapName = (nBitmapPos == static_cast<sal_Int32>(nPos) ) || (nBitmapPos == LISTBOX_ENTRY_NOTFOUND);

            if(bValidBitmapName)
            {
                bLoop = false;
                m_pBitmapList->GetBitmap(nPos)->SetName(aName);

                m_pBitmapLB->SetItemText(nId, aName);
                m_pBitmapLB->SelectItem( nId );

                *m_pnBitmapListState |= ChangeType::MODIFIED;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox( GetParentDialog()
                                                            ,"DuplicateNameDialog"
                                                            ,"cui/ui/queryduplicatedialog.ui" );
                aBox->Execute();
            }
        }
    }
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickDeleteHdl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pBitmapLB->GetSelectItemId();
    size_t nPos = m_pBitmapLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelBitmapDialog","cui/ui/querydeletebitmapdialog.ui" );

        if( aQueryBox->Execute() == RET_YES )
        {
            m_pBitmapList->Remove( static_cast<sal_uInt16>(nPos) );
            m_pBitmapLB->RemoveItem( nId );
            nId = m_pBitmapLB->GetItemId(0);
            m_pBitmapLB->SelectItem( nId );

            m_pCtlBitmapPreview->Invalidate();
            ModifyBitmapHdl( m_pBitmapLB );
            *m_pnBitmapListState |= ChangeType::MODIFIED;
        }
    }
}

IMPL_LINK_NOARG( SvxBitmapTabPage, ModifyBitmapSizeHdl, Edit&, void )
{
    if( m_pTsbScale->GetState() != TRISTATE_TRUE && static_cast<BitmapStyle>(m_pBitmapStyleLB->GetSelectEntryPos()) != TILED )
    {
        sal_Int64 nWidthPercent = m_pBitmapWidth->Denormalize( m_pBitmapWidth->GetValue() );
        sal_Int64 nHeightPercent = m_pBitmapHeight->Denormalize( m_pBitmapHeight->GetValue() );
        if( nWidthPercent == 100 && nHeightPercent == 100 )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(ORIGINAL) );
        else if( nWidthPercent == nFilledWidthPercent && nHeightPercent == nFilledHeightPercent )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(FILLED) );
        else if( nWidthPercent == nZoomedWidthPercent && nHeightPercent == nZoomedHeightPercent )
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(ZOOMED) );
        else
            m_pBitmapStyleLB->SelectEntryPos( static_cast<sal_Int32>(CUSTOM) );
    }
    ModifyBitmapStyleHdl(*m_pBitmapStyleLB);

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG( SvxBitmapTabPage, ClickScaleHdl, Button*, void )
{
    ModifyBitmapStyleHdl( *m_pBitmapStyleLB );
}

IMPL_LINK_NOARG( SvxBitmapTabPage, ModifyBitmapStyleHdl, ListBox&, void )
{
    BitmapStyle eStylePos = (BitmapStyle)m_pBitmapStyleLB->GetSelectEntryPos();
    bool bIsStretched( eStylePos == STRETCHED );
    bool bIsTiled( eStylePos == TILED );

    m_pSizeBox->Enable( !bIsStretched );
    m_pTsbScale->Enable( eStylePos == CUSTOM );
    m_pPositionBox->Enable( !bIsStretched );
    m_pPositionOffBox->Enable( bIsTiled );
    m_pTileOffBox->Enable( bIsTiled );

    m_rXFSet.Put( XFillBmpTileItem( bIsTiled ) );
    m_rXFSet.Put( XFillBmpStretchItem( bIsStretched ) );

    if(!bIsStretched)
    {
        Size aSetBitmapSize;
        switch(eStylePos)
        {
            case ORIGINAL:
            {
                m_pBitmapWidth->SetValue(100);
                m_pBitmapHeight->SetValue(100);
                aSetBitmapSize = rBitmapSize;
            }
            break;
            case FILLED:
            {
                sal_Int64 nWidthPercent = m_pBitmapWidth->Normalize(nFilledWidthPercent);
                m_pBitmapWidth->SetValue(nWidthPercent);
                sal_Int64 nHeightPercent = m_pBitmapHeight->Normalize(nFilledHeightPercent);
                m_pBitmapHeight->SetValue(nHeightPercent);
                aSetBitmapSize = rFilledSize;
            }
            break;
            case ZOOMED:
            {
                sal_Int64 nWidthPercent = m_pBitmapWidth->Normalize(nZoomedWidthPercent);
                m_pBitmapWidth->SetValue(nWidthPercent);
                sal_Int64 nHeightPercent = m_pBitmapHeight->Normalize(nZoomedHeightPercent);
                m_pBitmapHeight->SetValue(nHeightPercent);
                aSetBitmapSize = rZoomedSize;
            }
            break;
            case CUSTOM:
            case TILED:
            {
                sal_Int64 nWidthPercent = m_pBitmapWidth->Denormalize( m_pBitmapWidth->GetValue() );
                sal_Int64 nHeightPercent = m_pBitmapWidth->Denormalize( m_pBitmapHeight->GetValue() );
                if(eStylePos == CUSTOM && m_pTsbScale->IsEnabled() &&  m_pTsbScale->GetState() == TRISTATE_TRUE)
                {
                    aSetBitmapSize.Width() = -nWidthPercent;
                    aSetBitmapSize.Height() = -nHeightPercent;
                }
                else
                {
                    aSetBitmapSize.Width() = static_cast<long>(nWidthPercent*rBitmapSize.Width()/100);
                    aSetBitmapSize.Height() = static_cast<long>(nHeightPercent*rBitmapSize.Height()/100);
                }
            }
            break;
            default:
            break;
        }

        m_rXFSet.Put( XFillBmpSizeXItem( aSetBitmapSize.Width() ) );
        m_rXFSet.Put( XFillBmpSizeYItem( aSetBitmapSize.Height() ) );
    }

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ModifyBitmapPositionHdl, ListBox&, void)
{
    if(m_pPositionLB->IsEnabled())
        m_rXFSet.Put( XFillBmpPosItem( static_cast< RectPoint >( m_pPositionLB->GetSelectEntryPos() ) ) );

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ModifyPositionOffsetHdl, Edit&, void)
{
    if(m_pPositionOffX->IsEnabled())
        m_rXFSet.Put( XFillBmpPosOffsetXItem( m_pPositionOffX->GetValue() ) );

    if(m_pPositionOffY->IsEnabled())
        m_rXFSet.Put( XFillBmpPosOffsetYItem( m_pPositionOffY->GetValue() ) );

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ModifyTileOffsetHdl, Edit&, void)
{
    sal_uInt16 nTileXOff = 0;
    sal_uInt16 nTileYOff = 0;

    if(m_pTileOffLB->GetSelectEntryPos() == static_cast<sal_Int32>(ROW))
        nTileXOff = m_pTileOffset->GetValue();

    if(m_pTileOffLB->GetSelectEntryPos() == static_cast<sal_Int32>(COLUMN))
        nTileYOff = m_pTileOffset->GetValue();

    m_rXFSet.Put( XFillBmpTileOffsetXItem(nTileXOff) );
    m_rXFSet.Put( XFillBmpTileOffsetYItem(nTileYOff) );

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickImportHdl, Button*, void)
{
    SvxOpenGraphicDialog aDlg( "Import" );
    aDlg.EnableLink(false);
    long nCount = m_pBitmapList->Count();

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;

        EnterWait();
        ErrCode nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            OUString aDesc(CuiResId(RID_SVXSTR_DESC_EXT_BITMAP));
            ScopedVclPtr<MessageDialog> pWarnBox;

            // convert file URL to UI name
            OUString        aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialog creation failed!");
            ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aURL.GetName().getToken( 0, '.' ), aDesc ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
            nError = ErrCode(1);

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                bool bDifferent = true;

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == m_pBitmapList->GetBitmap( i )->GetName() )
                        bDifferent = false;

                if( bDifferent ) {
                    nError = ERRCODE_NONE;
                    break;
                }

                if( !pWarnBox )
                {
                    pWarnBox.disposeAndReset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                                 ,"DuplicateNameDialog"
                                                 ,"cui/ui/queryduplicatedialog.ui"));
                }

                if( pWarnBox->Execute() != RET_OK )
                    break;
            }

            pDlg.disposeAndClear();
            pWarnBox.disposeAndClear();

            if( !nError )
            {
                m_pBitmapList->Insert(o3tl::make_unique<XBitmapEntry>(aGraphic, aName), nCount);

                sal_Int32 nId = m_pBitmapLB->GetItemId( nCount - 1 );
                Bitmap aBitmap = m_pBitmapList->GetBitmapForPreview( nCount, m_pBitmapLB->GetIconSize() );

                m_pBitmapLB->InsertItem( nId + 1, Image(aBitmap), aName );
                m_pBitmapLB->SelectItem( nId + 1 );
                *m_pnBitmapListState |= ChangeType::MODIFIED;

                ModifyBitmapHdl( m_pBitmapLB );
            }
        }
        else
            // graphic couldn't be loaded
            ScopedVclPtrInstance<MessageDialog>( GetParentDialog()
                          ,"NoLoadedFileDialog"
                          ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
    }
}

sal_Int32 SvxBitmapTabPage::SearchBitmapList(const OUString& rBitmapName)
{
    long nCount = m_pBitmapList->Count();
    bool bValidBitmapName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidBitmapName;i++)
    {
        if(rBitmapName == m_pBitmapList->GetBitmap( i )->GetName())
        {
            nPos = i;
            bValidBitmapName = false;
        }
    }
    return nPos;
}

void SvxBitmapTabPage::PointChanged( vcl::Window* , RectPoint )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
