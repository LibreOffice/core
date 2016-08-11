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

#include <stdlib.h>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
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
#include "paragrph.hrc"
#include "sfx2/opengrf.hxx"
#include <vcl/layout.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/viewsh.hxx>

using namespace com::sun::star;

enum eBitmapStyle
{
    ORIGINAL,
    FILLED,
    STRETCHED,
    ZOOMED,
    CUSTOM,
    TILED
};

enum eTileOff
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
    m_nObjectWidth(0.0),
    m_nObjectHeight(0.0),
    m_nPageType(nullptr),
    m_nDlgType(0),
    m_nPos(nullptr),

    m_pbAreaTP( nullptr ),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() ),
    mpView(nullptr),
    meDlgUnit(FUNIT_NONE)
{
    get(m_pBitmapLB,"BITMAP");
    get(m_pBitmapStyleLB, "bitmapstyle");
    get(m_pSizeBox, "sizebox");
    get(m_pBitmapWidth, "width");
    get(m_pBitmapHeight, "height");
    get(m_pPositionBox, "posbox");
    get(m_pPositionLB, "positionlb");
    get(m_pPositionOffBox, "posoffbox");
    get(m_pPositionOffX, "posoffx");
    get(m_pPositionOffY, "posoffy");
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
    m_pPositionLB->SetSelectHdl( LINK( this, SvxBitmapTabPage, ModifyBitmapPositionHdl ) );
    Link<Edit&, void> aLink( LINK( this, SvxBitmapTabPage, ModifyTileOffsetHdl ) );
    m_pPositionOffX->SetModifyHdl(aLink);
    m_pPositionOffY->SetModifyHdl(aLink);
    m_pBtnImport->SetClickHdl( LINK(this, SvxBitmapTabPage, ClickImportHdl) );

    // Calculate size of display boxes
    Size aSize = LogicToPixel(Size(90, 42), MAP_APPFONT);
    m_pCtlBitmapPreview->set_width_request(aSize.Width());
    m_pCtlBitmapPreview->set_height_request(aSize.Height());

    SfxItemPool* pPool = m_rXFSet.GetPool();
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_WIDTH );
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell )
        mpView = pViewShell->GetDrawView();
    else
        mpView = nullptr;
    DBG_ASSERT( mpView, "no valid view (!)" );
    meDlgUnit = GetModuleFieldUnit( GetItemSet() );
    SetFieldUnit( *m_pBitmapWidth, FUNIT_PERCENT, true );
    SetFieldUnit( *m_pBitmapHeight, FUNIT_PERCENT, true );
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
    m_pPositionBox.clear();
    m_pPositionLB.clear();
    m_pPositionOffBox.clear();
    m_pPositionOffX.clear();
    m_pPositionOffY.clear();
    m_pCtlBitmapPreview.clear();
    m_pBtnImport.clear();
    SvxTabPage::dispose();
}


void SvxBitmapTabPage::Construct()
{
    m_pBitmapLB->FillPresetListBox( *m_pBitmapList );
}


void SvxBitmapTabPage::ActivatePage( const SfxItemSet& )
{
    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = false;

        if( *m_nPageType == PT_BITMAP && *m_nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nId = m_pBitmapLB->GetItemId( static_cast<size_t>( *m_nPos ) );
            m_pBitmapLB->SelectItem(nId);
        }
        *m_nPageType = PT_BITMAP;
        *m_nPos = LISTBOX_ENTRY_NOTFOUND;
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
    if( m_nDlgType == 0 && !*m_pbAreaTP )
    {
        if(PT_BITMAP == *m_nPageType)
        {
            rAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
            size_t nPos = m_pBitmapLB->GetSelectItemPos();
            if(VALUESET_ITEM_NOTFOUND != nPos)
            {
                const XBitmapEntry* pXBitmapEntry = m_pBitmapList->GetBitmap( static_cast<sal_uInt16>(nPos) );
                const OUString aString(m_pBitmapLB->GetItemText( m_pBitmapLB->GetSelectItemId() ));
                rAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
            }
            eBitmapStyle nStylePos = (eBitmapStyle)m_pBitmapStyleLB->GetSelectEntryPos();
            switch(nStylePos)
            {
                case ORIGINAL:
                {
                    rAttrs->Put( XFillBmpTileItem(false) );
                    rAttrs->Put(XFillBmpStretchItem(false));
                    std::unique_ptr<XFillBmpSizeXItem> pXItem;
                    pXItem.reset(new XFillBmpSizeXItem( rBitmapSize.Width() ));
                    std::unique_ptr<XFillBmpSizeYItem> pYItem;
                    pYItem.reset(new XFillBmpSizeYItem( rBitmapSize.Height() ));
                    rAttrs->Put( *pXItem );
                    rAttrs->Put( *pYItem );
                }
                break;
                case FILLED:
                {
                    rAttrs->Put( XFillBmpTileItem(false) );
                    rAttrs->Put(XFillBmpStretchItem(false));
                    std::unique_ptr<XFillBmpSizeXItem> pXItem;
                    std::unique_ptr<XFillBmpSizeYItem> pYItem;
                    if(rBitmapSize.Width() > rBitmapSize.Height())
                    {
                        pXItem.reset(new XFillBmpSizeXItem(m_nObjectWidth));
                        pYItem.reset(new XFillBmpSizeYItem(rBitmapSize.Height()*m_nObjectWidth/rBitmapSize.Width()));
                    }
                    else
                    {
                        pXItem.reset(new XFillBmpSizeXItem(rBitmapSize.Width()*m_nObjectHeight/rBitmapSize.Height()));
                        pYItem.reset(new XFillBmpSizeYItem(m_nObjectHeight));
                    }
                    rAttrs->Put( *pXItem );
                    rAttrs->Put( *pYItem );
                }
                break;
                case STRETCHED:
                {
                    rAttrs->Put( XFillBmpTileItem(false) );
                    rAttrs->Put(XFillBmpStretchItem(true));
                }
                break;
                case ZOOMED:
                {
                    rAttrs->Put( XFillBmpTileItem(false) );
                    rAttrs->Put(XFillBmpStretchItem(false));
                    std::unique_ptr<XFillBmpSizeXItem> pXItem;
                    std::unique_ptr<XFillBmpSizeYItem> pYItem;
                    if(rBitmapSize.Width() > rBitmapSize.Height())
                    {
                        pXItem.reset(new XFillBmpSizeXItem(rBitmapSize.Width()*m_nObjectHeight/rBitmapSize.Height()));
                        pYItem.reset(new XFillBmpSizeYItem(m_nObjectHeight));
                    }
                    else
                    {
                        pXItem.reset(new XFillBmpSizeXItem(m_nObjectWidth));
                        pYItem.reset(new XFillBmpSizeYItem(rBitmapSize.Height()*m_nObjectWidth/rBitmapSize.Width()));
                    }
                    rAttrs->Put( *pXItem );
                    rAttrs->Put( *pYItem );
                }
                break;
                case CUSTOM:
                {
                    rAttrs->Put( XFillBmpTileItem(false) );
                    rAttrs->Put(XFillBmpStretchItem(false));
                }
                break;
                case TILED:
                {
                    rAttrs->Put( XFillBmpTileItem(true) );
                    rAttrs->Put(XFillBmpStretchItem(false));
                }
                break;
            }
            if(m_pPositionLB->IsEnabled())
                rAttrs->Put( XFillBmpPosItem( static_cast<RECT_POINT>( m_pPositionLB->GetSelectEntryPos() ) ) );
            if(m_pPositionOffX->IsEnabled())
                rAttrs->Put( XFillBmpPosOffsetXItem( m_pPositionOffX->GetValue() ) );
            if(m_pPositionOffY->IsEnabled())
                rAttrs->Put( XFillBmpPosOffsetYItem( m_pPositionOffY->GetValue() ) );
        }
    }
    return true;
}


void SvxBitmapTabPage::Reset( const SfxItemSet* rAttrs )
{
    SfxItemSet rGeoAttr(mpView->GetGeoAttrFromMarked());
    const SfxPoolItem* pItem = nullptr;
    const double fUIScale(double(mpView->GetModel()->GetUIScale()));
    pItem = GetItem( rGeoAttr, SID_ATTR_TRANSFORM_WIDTH );
    m_nObjectWidth = std::max( pItem ? (double)static_cast<const SfxUInt32Item*>(pItem)->GetValue() : 0.0, 1.0 );
    double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(m_nObjectWidth),(MapUnit)mePoolUnit, MAP_100TH_MM )) / fUIScale);
    m_nObjectWidth = fTmpWidth;
    pItem = GetItem( rGeoAttr, SID_ATTR_TRANSFORM_HEIGHT );
    m_nObjectHeight = std::max( pItem ? (double)static_cast<const SfxUInt32Item*>(pItem)->GetValue() : 0.0, 1.0 );
    double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(m_nObjectHeight),(MapUnit)mePoolUnit, MAP_100TH_MM )) / fUIScale);
    m_nObjectHeight = fTmpHeight;

    XFillBitmapItem aItem( static_cast<const XFillBitmapItem&>(rAttrs->Get(XATTR_FILLBITMAP)) );
    if(!aItem.isPattern())
    {
        m_rXFSet.Put( aItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
        m_pCtlBitmapPreview->Disable();

    if( rAttrs->GetItemState( XATTR_FILLBMP_POS ) != SfxItemState::DONTCARE )
    {
        RECT_POINT eValue = static_cast<const XFillBmpPosItem&>( rAttrs->Get( XATTR_FILLBMP_POS ) ).GetValue();
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

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyBitmapHdl, ValueSet*, void)
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
    BitmapEx aBmpEx(pGraphicObject->GetGraphic().GetBitmapEx());
    Size aTempBitmapSize = aBmpEx.GetSizePixel();
    const double fUIScale(double(mpView->GetModel()->GetUIScale()));
    rBitmapSize.Width() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Width()),MAP_PIXEL, MAP_100TH_MM )) / fUIScale);
    rBitmapSize.Height() = ((OutputDevice::LogicToLogic(static_cast<sal_Int32>(aTempBitmapSize.Height()),MAP_PIXEL, MAP_100TH_MM )) / fUIScale);
    ModifyBitmapStyleHdl( *m_pBitmapStyleLB );
    ModifyBitmapPositionHdl( *m_pPositionLB );
    //m_pBitmapWidth->SetValue(rBitmapSize.Width, FUNIT_PERCENT);
    m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
    m_rXFSet.Put(XFillBitmapItem(OUString(), *pGraphicObject));

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ClickRenameHdl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pBitmapLB->GetSelectItemId();
    size_t nPos = m_pBitmapLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_BITMAP ) );
        OUString aName( m_pBitmapList->GetBitmap( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
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
                XBitmapEntry* pEntry = m_pBitmapList->GetBitmap( static_cast<sal_uInt16>(nPos) );
                pEntry->SetName( aName );

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

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ClickDeleteHdl, SvxPresetListBox*, void)
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

IMPL_LINK_NOARG_TYPED( SvxBitmapTabPage, ModifyBitmapStyleHdl, ListBox&, void )
{
    eBitmapStyle nStylePos = (eBitmapStyle)m_pBitmapStyleLB->GetSelectEntryPos();
    switch(nStylePos)
    {
        case ORIGINAL:
            {
                m_pSizeBox->Enable();
                m_pPositionBox->Enable();
                m_pPositionOffBox->Disable();
                m_rXFSet.Put( XFillBmpTileItem(false) );
                m_rXFSet.Put(XFillBmpStretchItem(false));
            }
            break;
        case FILLED:
            {
                m_pSizeBox->Enable();
                m_pPositionBox->Enable();
                m_pPositionOffBox->Disable();
                m_rXFSet.Put( XFillBmpTileItem(false) );
                m_rXFSet.Put(XFillBmpStretchItem(false));
            }
            break;
        case STRETCHED:
            {
                m_pSizeBox->Disable();
                m_pPositionBox->Disable();
                m_pPositionOffBox->Disable();
                m_rXFSet.Put( XFillBmpTileItem(false) );
                m_rXFSet.Put(XFillBmpStretchItem(true));
            }
            break;
        case ZOOMED:
            {
                m_pSizeBox->Enable();
                m_pPositionBox->Enable();
                m_pPositionOffBox->Disable();
                m_rXFSet.Put( XFillBmpTileItem(false) );
                m_rXFSet.Put(XFillBmpStretchItem(false));
            }
            break;
        case CUSTOM:
            {
                m_pSizeBox->Enable();
                m_pPositionBox->Enable();
                m_pPositionOffBox->Disable();
                m_rXFSet.Put( XFillBmpTileItem(false) );
                m_rXFSet.Put(XFillBmpStretchItem(false));
            }
            break;
        case TILED:
            {
                m_pSizeBox->Enable();
                m_pPositionBox->Enable();
                m_pPositionOffBox->Enable();
                m_rXFSet.Put( XFillBmpTileItem(true) );
                m_rXFSet.Put(XFillBmpStretchItem(false));
            }
            break;
    }
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyBitmapPositionHdl, ListBox&, void)
{
    if(m_pPositionLB->IsEnabled())
        m_rXFSet.Put( XFillBmpPosItem( static_cast< RECT_POINT >( m_pPositionLB->GetSelectEntryPos() ) ) );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyTileOffsetHdl, Edit&, void)
{
    if(m_pPositionOffX->IsEnabled())
        m_rXFSet.Put( XFillBmpPosOffsetXItem( m_pPositionOffX->GetValue() ) );
    if(m_pPositionOffY->IsEnabled())
        m_rXFSet.Put( XFillBmpPosOffsetYItem( m_pPositionOffY->GetValue() ) );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ClickImportHdl, Button*, void)
{
    ResMgr& rMgr = CUI_MGR();
    SvxOpenGraphicDialog aDlg( OUString("Import") );
    aDlg.EnableLink(false);
    long nCount = m_pBitmapList->Count();

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;

        EnterWait();
        int nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            OUString aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
            ScopedVclPtr<MessageDialog> pWarnBox;

            // convert file URL to UI name
            OUString        aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialog creation failed!");
            std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aURL.GetName().getToken( 0, '.' ), aDesc ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
            nError = 1;

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                bool bDifferent = true;

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == m_pBitmapList->GetBitmap( i )->GetName() )
                        bDifferent = false;

                if( bDifferent ) {
                    nError = 0;
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

            pDlg.reset();
            pWarnBox.disposeAndClear();

            if( !nError )
            {
                XBitmapEntry* pEntry = new XBitmapEntry( aGraphic, aName );
                m_pBitmapList->Insert( pEntry, nCount );

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
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
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

void SvxBitmapTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
