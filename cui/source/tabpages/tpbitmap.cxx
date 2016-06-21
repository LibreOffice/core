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


using namespace com::sun::star;


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

    m_nPageType(0),
    m_nDlgType(0),
    m_nPos(0),

    m_pbAreaTP( nullptr ),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pLbBitmap,"LB_BITMAP");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");

    get(m_pGridX_Y,"gridX_Y");
    get(m_pRbOriginal,"RB_ORIGINAL");
    get(m_pFtXSize,"FT_X_SIZE");
    get(m_pMtrFldXSize,"MTR_FLD_X_SIZE");
    get(m_pFtYSize,"FT_Y_SIZE");
    get(m_pMtrFldYSize,"MTR_FLD_Y_SIZE");

    get(m_pFlPosition,"framePOSITION");
    get(m_pLbPosition,"LB_POSITION");
    get(m_pGridOffset,"gridOFFSET");
    get(m_pMtrFldXOffset,"MTR_FLD_X_OFFSET");
    get(m_pMtrFldYOffset,"MTR_FLD_Y_OFFSET");
    get(m_pRbTile,"RB_TILE");
    get(m_pRbStretch,"RB_STRETCH");
    get(m_pRbFilled,"RB_FILLED");
    get(m_pRbFreeStyle, "RB_FREESTYLE");

    get(m_pFlOffset,"FL_OFFSET");
    get(m_pRbtRow,"RBT_ROW");
    get(m_pRbtColumn,"RBT_COLUMN");
    get(m_pMtrFldOffset,"MTR_FLD_OFFSET");

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXBitmapItem );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pLbBitmap->SetSelectHdl( LINK( this, SvxBitmapTabPage, ModifyBitmapHdl ) );
    m_pLbPosition->SetSelectHdl( LINK( this, SvxBitmapTabPage, ModifyPositionHdl ) );

    Link<Edit&,void> aLink( LINK( this, SvxBitmapTabPage, ModifyTileHdl_Impl ) );
    Link<Button*,void> aClickLink( LINK( this, SvxBitmapTabPage, ModifyStyleHdl_Impl ) );
    m_pRbTile->SetClickHdl( aClickLink );
    m_pRbStretch->SetClickHdl( aClickLink );
    m_pRbOriginal->SetClickHdl( aClickLink );
    m_pRbFilled->SetClickHdl( aClickLink );
    m_pRbFreeStyle->SetClickHdl( aClickLink );
    m_pMtrFldXSize->SetModifyHdl( aLink );
    m_pMtrFldYSize->SetModifyHdl( aLink );
    m_pRbtRow->SetClickHdl( aClickLink );
    m_pRbtColumn->SetClickHdl( aClickLink );
    m_pMtrFldOffset->SetModifyHdl( aLink );
    m_pMtrFldXOffset->SetModifyHdl( aLink );
    m_pMtrFldYOffset->SetModifyHdl( aLink );
    Size aSize = LogicToPixel(Size(108, 103), MAP_APPFONT);

    m_pLbBitmap->set_width_request(aSize.Width());
    m_pLbBitmap->set_height_request(aSize.Height());

    // Calculate size of display boxes
    Size aSize2 = LogicToPixel(Size(110, 42), MAP_APPFONT);
    m_pCtlBitmapPreview->set_width_request(aSize2.Width());
    m_pCtlBitmapPreview->set_height_request(aSize2.Height());
}

SvxBitmapTabPage::~SvxBitmapTabPage()
{
    disposeOnce();
}

void SvxBitmapTabPage::dispose()
{
    m_pLbBitmap.clear();
    m_pCtlBitmapPreview.clear();
    m_pRbOriginal.clear();
    m_pGridX_Y.clear();
    m_pFtXSize.clear();
    m_pMtrFldXSize.clear();
    m_pFtYSize.clear();
    m_pMtrFldYSize.clear();
    m_pFlPosition.clear();
    m_pLbPosition.clear();
    m_pGridOffset.clear();
    m_pMtrFldXOffset.clear();
    m_pMtrFldYOffset.clear();
    m_pRbTile.clear();
    m_pRbStretch.clear();
    m_pRbFilled.clear();
    m_pRbFreeStyle.clear();
    m_pFlOffset.clear();
    m_pRbtRow.clear();
    m_pRbtColumn.clear();
    m_pMtrFldOffset.clear();
    SvxTabPage::dispose();
}


void SvxBitmapTabPage::Construct()
{
    m_pLbBitmap->Fill( m_pBitmapList );
}


void SvxBitmapTabPage::ActivatePage( const SfxItemSet& rSet)
{
    sal_Int32 nCount(0);
    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = false;

        if( m_pBitmapList.is() )
        {
            sal_Int32 _nPos(m_nPos);

            if( *m_pnBitmapListState != ChangeType::NONE )
            {
                if( *m_pnBitmapListState & ChangeType::CHANGED )
                    m_pBitmapList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewBitmapList();

                _nPos = m_pLbBitmap->GetSelectEntryPos();

                m_pLbBitmap->Clear();
                m_pLbBitmap->Fill( m_pBitmapList );
                nCount = m_pLbBitmap->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbBitmap->SelectEntryPos( 0 );
                else
                    m_pLbBitmap->SelectEntryPos( _nPos );
                ModifyBitmapHdl_Impl();
            }
            m_nPageType = PT_BITMAP;
            m_nPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }

    XFillBitmapItem aBitmapItem( static_cast<const XFillBitmapItem&>(rSet.Get(XATTR_FILLBITMAP)) );
    if(!aBitmapItem.isPattern())
    {
        m_rXFSet.Put( aBitmapItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
        m_pCtlBitmapPreview->Disable();
}


DeactivateRC SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxBitmapTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;
    if( m_nDlgType == 0 && !*m_pbAreaTP )
    {
        rAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        sal_Int32 nPos = m_pLbBitmap->GetSelectEntryPos();
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            const XBitmapEntry* pXBitmapEntry = m_pBitmapList->GetBitmap(nPos);
            const OUString aString(m_pLbBitmap->GetSelectEntry());
            rAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
            bModified = true;
        }

        const SfxPoolItem* pOld = nullptr;

        bool bState = m_pRbTile->IsChecked();
        if( m_pRbTile->IsValueChangedFromSaved() )
        {
            XFillBmpTileItem aFillBmpTileItem( bState );
            pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_TILE );
            if ( !pOld || !( *static_cast<const XFillBmpTileItem*>(pOld) == aFillBmpTileItem ) )
            {
                rAttrs->Put( aFillBmpTileItem );
                bModified = true;
            }
        }

        bState = m_pRbStretch->IsChecked();
        if( m_pRbStretch->IsValueChangedFromSaved() )
        {
            XFillBmpStretchItem aFillBmpStretchItem( bState );
            pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_STRETCH );
            if ( !pOld || !( *static_cast<const XFillBmpStretchItem*>(pOld) == aFillBmpStretchItem ) )
            {
                rAttrs->Put( aFillBmpStretchItem );
                bModified = true;
            }
        }

        // Original size (in the UI) is used as follows:
        // Controls are disabled, but have to be set.
        // SizeX = 0; SizeY = 0; Log = sal_True

        //aMtrFldXSize
        bool bOriginal = false;
        OUString aStr = m_pMtrFldXSize->GetText();
        {
            std::unique_ptr<XFillBmpSizeXItem> pItem;
            if( m_pMtrFldXSize->IsEnabled() &&
                !aStr.isEmpty()  &&
                aStr != m_pMtrFldXSize->GetSavedValue() )
            {
                    pItem.reset(new XFillBmpSizeXItem( static_cast<long>(m_pMtrFldXSize->GetValue() ) ) );
            }
            else if( m_pRbOriginal->IsChecked() )
            {
                pItem.reset(new XFillBmpSizeXItem( 0 ));
                bOriginal = true;
            }

            if( pItem )
            {
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZEX );
                if ( !pOld || !( *static_cast<const XFillBmpSizeXItem*>(pOld) == *pItem ) )
                {
                    rAttrs->Put( *pItem );
                    bModified = true;
                }
            }
        }

        //aMtrFldYSize
        aStr = m_pMtrFldYSize->GetText();
        {
            std::unique_ptr<XFillBmpSizeYItem> pItem;

            if( m_pMtrFldYSize->IsEnabled() &&
                !aStr.isEmpty()  &&
                aStr != m_pMtrFldYSize->GetSavedValue() )
            {
                    pItem.reset(new XFillBmpSizeYItem( static_cast<long>(m_pMtrFldYSize->GetValue() ) ) );
            }
            else if( m_pRbOriginal->IsChecked() )
            {
                pItem.reset(new XFillBmpSizeYItem( 0 ));
                bOriginal = true;
            }

            if( pItem )
            {
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZEY );
                if ( !pOld || !( *static_cast<const XFillBmpSizeYItem*>(pOld) == *pItem ) )
                {
                    rAttrs->Put( *pItem );
                    bModified = true;
                }
            }
        }
        std::unique_ptr<XFillBmpSizeLogItem> pItem;
        pItem.reset(new XFillBmpSizeLogItem( bOriginal ));
        if( pItem )
        {
            pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZELOG );
            if ( !pOld || !( *static_cast<const XFillBmpSizeLogItem*>(pOld) == *pItem ) )
            {
                rAttrs->Put( *pItem );
                bModified = true;
            }
        }


        //aRbtRow
        //aRbtColumn
        //aMtrFldOffset
        if( m_pMtrFldOffset->IsEnabled() )
        {
            OUString aMtrString = m_pMtrFldOffset->GetText();
            if( ( !aMtrString.isEmpty()  &&
                  aMtrString != m_pMtrFldOffset->GetSavedValue() ) ||
                  m_pRbtRow->IsValueChangedFromSaved() ||
                  m_pRbtColumn->IsValueChangedFromSaved() )
            {
                if( m_pRbtRow->IsChecked() )
                {
                    XFillBmpTileOffsetXItem aFillBmpTileOffsetXItem( (sal_uInt16) m_pMtrFldOffset->GetValue() );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_TILEOFFSETX );
                    if ( !pOld || !( *static_cast<const XFillBmpTileOffsetXItem*>(pOld) == aFillBmpTileOffsetXItem ) )
                    {
                        rAttrs->Put( aFillBmpTileOffsetXItem );
                        rAttrs->Put( XFillBmpTileOffsetYItem( 0 ) );
                        bModified = true;
                    }
                }
                else if( m_pRbtColumn->IsChecked() )
                {
                    XFillBmpTileOffsetYItem aFillBmpTileOffsetYItem( (sal_uInt16) m_pMtrFldOffset->GetValue() );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_TILEOFFSETY );
                    if ( !pOld || !( *static_cast<const XFillBmpTileOffsetYItem*>(pOld) == aFillBmpTileOffsetYItem ) )
                    {
                        rAttrs->Put( aFillBmpTileOffsetYItem );
                        rAttrs->Put( XFillBmpTileOffsetXItem( 0 ) );
                        bModified = true;
                    }
                }
            }
        }

        //aCtlPosition
        if( m_pLbPosition->IsEnabled() )
        {
            bool bPut = false;
            RECT_POINT _eRP = static_cast<RECT_POINT>(m_pLbPosition->GetSelectEntryPos());

            if( SfxItemState::DONTCARE == m_rOutAttrs.GetItemState( XATTR_FILLBMP_POS ) )
                bPut = true;
            else
            {
                RECT_POINT eValue = static_cast<const XFillBmpPosItem&>( m_rOutAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
                if( eValue != _eRP )
                    bPut = true;
            }
            if( bPut )
            {
                XFillBmpPosItem aFillBmpPosItem( _eRP );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_POS );
                if ( !pOld || !( *static_cast<const XFillBmpPosItem*>(pOld) == aFillBmpPosItem ) )
                {
                    rAttrs->Put( aFillBmpPosItem );
                    bModified = true;
                }
            }
        }

        //aMtrFldXOffset
        if( m_pMtrFldXOffset->IsEnabled() )
        {
            OUString sMtrXOffset = m_pMtrFldXOffset->GetText();
            if( !sMtrXOffset.isEmpty()  &&
                m_pMtrFldXOffset->IsValueChangedFromSaved() )
            {
                XFillBmpPosOffsetXItem aFillBmpPosOffsetXItem( (sal_uInt16) m_pMtrFldXOffset->GetValue() );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_POSOFFSETX );
                if ( !pOld || !( *static_cast<const XFillBmpPosOffsetXItem*>(pOld) == aFillBmpPosOffsetXItem ) )
                {
                    rAttrs->Put( aFillBmpPosOffsetXItem );
                    bModified = true;
                }
            }
        }

        //aMtrFldYOffset
        if( m_pMtrFldYOffset->IsEnabled() )
        {
            OUString sMtrYOffset = m_pMtrFldYOffset->GetText();
            if( !sMtrYOffset.isEmpty() &&
                m_pMtrFldYOffset->IsValueChangedFromSaved() )
            {
                XFillBmpPosOffsetYItem aFillBmpPosOffsetYItem( (sal_uInt16) m_pMtrFldYOffset->GetValue() );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_POSOFFSETY );
                if ( !pOld || !( *static_cast<const XFillBmpPosOffsetYItem*>(pOld) == aFillBmpPosOffsetYItem ) )
                {
                    rAttrs->Put( aFillBmpPosOffsetYItem );
                    bModified = true;
                }
            }
        }
        rAttrs->Put (SfxUInt16Item(SID_PAGE_TYPE,m_nPageType));
        rAttrs->Put (SfxUInt16Item(SID_TABPAGE_POS,m_nPos));
    }

    return bModified;
}


void SvxBitmapTabPage::Reset( const SfxItemSet* rAttrs )
{
    bool bTile = false;
    bool bStretch = false;
    bool bOriginal = false;

    if( rAttrs->GetItemState( XATTR_FILLBMP_TILE ) != SfxItemState::DONTCARE )
    {
        if( static_cast<const XFillBmpTileItem&>( rAttrs->Get( XATTR_FILLBMP_TILE ) ).GetValue() )
        {
            m_pRbTile->SetState( true );
            bTile = true;
        }
        else
            m_pRbTile->SetState( false );
    }

    if( rAttrs->GetItemState( XATTR_FILLBMP_STRETCH ) != SfxItemState::DONTCARE )
    {
        if( static_cast<const XFillBmpStretchItem&>( rAttrs->Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
        {
            m_pRbStretch->SetState( true );
            bStretch = true;
        }
        else
            m_pRbStretch->SetState( false );
    }

    // determine status for the original size
    //aMtrFldXSize
    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZEX ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpSizeXItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        m_pMtrFldXSize->SetValue( nValue );
        m_pMtrFldXSize->SaveValue();

        if( nValue == 0 )
            bOriginal = true;
    }
    else
    {
        m_pMtrFldXSize->SetText( "" );
        m_pMtrFldXSize->SaveValue();
    }

    //aMtrFldYSize
    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZEY ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpSizeYItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        m_pMtrFldYSize->SetValue( nValue );
        m_pMtrFldYSize->SaveValue();

        if( nValue == 0 )
            bOriginal = true;
    }
    else
    {
        m_pMtrFldYSize->SetText( "" );
        m_pMtrFldYSize->SaveValue();
    }

    if(bTile)
        ModifyStyleHdl_Impl(m_pRbTile);
    else if(bStretch)
        ModifyStyleHdl_Impl(m_pRbStretch);
    else if(bOriginal)
        ModifyStyleHdl_Impl(m_pRbOriginal);
    else
        ModifyStyleHdl_Impl(m_pRbFreeStyle);

    // #93372# Setting proper state after changing button
    ModifyBitmapProperties();
    //aRbtRow
    //aRbtColumn
    //aMtrFldOffset
    if( rAttrs->GetItemState( XATTR_FILLBMP_TILEOFFSETX ) != SfxItemState::DONTCARE )
    {
        sal_uInt16 nValue = static_cast<const XFillBmpTileOffsetXItem&>( rAttrs->Get( XATTR_FILLBMP_TILEOFFSETX ) ).GetValue();
        if( nValue > 0 )
        {
            m_pMtrFldOffset->SetValue( nValue );
            m_pRbtRow->Check();
        }
        else if( rAttrs->GetItemState( XATTR_FILLBMP_TILEOFFSETY ) != SfxItemState::DONTCARE )
        {
            nValue = static_cast<const XFillBmpTileOffsetYItem&>( rAttrs->Get( XATTR_FILLBMP_TILEOFFSETY ) ).GetValue();
            if( nValue > 0 )
            {
                m_pMtrFldOffset->SetValue( nValue );
                m_pRbtColumn->Check();
            }
        }
        else
            m_pMtrFldOffset->SetValue( 0 );
    }
    else
        m_pMtrFldOffset->SetText( "" );


    //aCtlPosition
    if( rAttrs->GetItemState( XATTR_FILLBMP_POS ) != SfxItemState::DONTCARE )
    {
        RECT_POINT eValue = static_cast<const XFillBmpPosItem&>( rAttrs->Get( XATTR_FILLBMP_POS ) ).GetValue();
        m_pLbPosition->SelectEntryPos( static_cast< sal_Int32 >(eValue) );
    }

    //aMtrFldXOffset
    if( rAttrs->GetItemState( XATTR_FILLBMP_POSOFFSETX ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpPosOffsetXItem&>( rAttrs->Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        m_pMtrFldXOffset->SetValue( nValue );
    }
    else
        m_pMtrFldXOffset->SetText( "" );

    //aMtrFldYOffset
    if( rAttrs->GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpPosOffsetYItem&>( rAttrs->Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        m_pMtrFldYOffset->SetValue( nValue );
    }
    else
        m_pMtrFldYOffset->SetText( "" );

    XFillBitmapItem aItem( static_cast<const XFillBitmapItem&>(rAttrs->Get(XATTR_FILLBITMAP)) );
    if(!aItem.isPattern())
    {
        m_rXFSet.Put( aItem );
        m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlBitmapPreview->Invalidate();
    }
    else
        m_pCtlBitmapPreview->Disable();
    ClickBitmapHdl_Impl();

    m_pRbTile->SaveValue();
    m_pRbStretch->SaveValue();
    m_pRbOriginal->SaveValue();
    m_pRbFreeStyle->SaveValue();
    m_pRbFilled->SaveValue();
    m_pRbtRow->SaveValue();
    m_pRbtColumn->SaveValue();
    m_pLbPosition->SaveValue();
    m_pMtrFldOffset->SaveValue();
    m_pMtrFldXOffset->SaveValue();
    m_pMtrFldYOffset->SaveValue();
}


VclPtr<SfxTabPage> SvxBitmapTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxBitmapTabPage>::Create( pWindow, *rAttrs );
}


void SvxBitmapTabPage::ClickBitmapHdl_Impl()
{
    m_pLbBitmap->Enable();
    m_pCtlBitmapPreview->Enable();

    if (!m_pRbtRow->IsChecked() && !m_pRbtColumn->IsChecked())
        m_pRbtRow->Check();

    ModifyBitmapHdl_Impl();
    ModifyBitmapProperties();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyBitmapHdl, ListBox&, void)
{
    ModifyBitmapHdl_Impl();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyPositionHdl, ListBox&, void)
{
    ModifyBitmapHdl_Impl();
}

void SvxBitmapTabPage::ModifyBitmapHdl_Impl()
{
    const SfxPoolItem* pPoolItem = nullptr;
    sal_Int32 _nPos = m_pLbBitmap->GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        const XBitmapEntry* pEntry = m_pBitmapList->GetBitmap(_nPos);

        m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        m_rXFSet.Put(XFillBitmapItem(OUString(), pEntry->GetGraphicObject()));
    }
    else if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), true, &pPoolItem ) )
    {
        m_rXFSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
        m_rXFSet.Put(XFillBitmapItem(OUString(), static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
    }
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    if( m_pLbPosition->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosItem( static_cast< RECT_POINT >( m_pLbPosition->GetSelectEntryPos() ) ) );

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}


IMPL_LINK_TYPED(SvxBitmapTabPage, ModifyStyleHdl_Impl, Button*, pButton, void)
{
    if(pButton == m_pRbTile)
    {
        m_pRbOriginal->SetState(false);
        m_pRbStretch->SetState(false);
        m_pRbFilled->SetState(false);
        m_pRbFreeStyle->SetState(false);

        m_pGridX_Y->Enable();
        m_pFlPosition->Enable();
        m_pGridOffset->Enable();
        m_pFlOffset->Enable();
        m_rXFSet.Put( XFillBmpTileItem( true ) );
        m_rXFSet.Put( XFillBmpStretchItem(false) );
    }
    if(pButton == m_pRbStretch)
    {
        m_pRbOriginal->SetState(false);
        m_pRbFilled->SetState(false);
        m_pRbTile->SetState(false);
        m_pRbFreeStyle->SetState(false);

        m_pGridX_Y->Disable();
        m_pFlPosition->Disable();
        m_pGridOffset->Disable();
        m_pFlOffset->Disable();
        m_rXFSet.Put( XFillBmpStretchItem(true) );
        m_rXFSet.Put( XFillBmpTileItem(false) );
        m_rXFSet.Put( XFillBmpSizeLogItem( false ) );
    }
    if(pButton == m_pRbOriginal)
    {
        m_pRbStretch->SetState(false);
        m_pRbTile->SetState(false);
        m_pRbFilled->SetState(false);
        m_pRbFreeStyle->SetState(false);

        m_pGridX_Y->Disable();
        m_pFlPosition->Enable();
        m_pGridOffset->Disable();
        m_pFlOffset->Disable();

        m_rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        m_rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        m_rXFSet.Put( XFillBmpSizeLogItem( true ) );
        m_rXFSet.Put( XFillBmpTileItem(false) );
        m_rXFSet.Put( XFillBmpStretchItem(false) );
    }
    if(pButton == m_pRbFilled)
    {
        m_pRbOriginal->SetState(false);
        m_pRbStretch->SetState(false);
        m_pRbTile->SetState(false);
        m_pRbFreeStyle->SetState(false);

        m_pGridX_Y->Disable();
        m_pFlPosition->Disable();
        m_pGridOffset->Disable();
        m_pFlOffset->Disable();
        m_rXFSet.Put( XFillBmpTileItem(false) );
        m_rXFSet.Put( XFillBmpStretchItem(false) );
        m_rXFSet.Put( XFillBmpSizeLogItem( false ) );
    }
    if(pButton == m_pRbFreeStyle)
    {
        m_pRbOriginal->SetState(false);
        m_pRbTile->SetState(false);
        m_pRbStretch->SetState(false);
        m_pRbFilled->SetState(false);

        m_pGridX_Y->Enable();
        m_pFlPosition->Enable();
        m_pGridOffset->Disable();
        m_pFlOffset->Disable();
        m_rXFSet.Put( XFillBmpTileItem(false) );
        m_rXFSet.Put( XFillBmpStretchItem(false) );
        m_rXFSet.Put( XFillBmpSizeLogItem( false ) );
        m_pMtrFldXSize->SetValue( m_pMtrFldXSize->GetValue() );
        m_pMtrFldYSize->SetValue( m_pMtrFldYSize->GetValue() );
    }
    ModifyBitmapProperties();
}

IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyTileHdl_Impl, Edit&, void)
{
    ModifyBitmapProperties();
}

void SvxBitmapTabPage::ModifyBitmapProperties()
{
    if( m_pMtrFldXSize->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeXItem> pItem;
        pItem.reset(new XFillBmpSizeXItem( ( static_cast<long>(m_pMtrFldXSize->GetValue() )  ) ));

        m_rXFSet.Put( *pItem );
    }

    if( m_pMtrFldYSize->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeYItem> pItem;
        pItem.reset(new XFillBmpSizeYItem( ( static_cast<long>(m_pMtrFldYSize->GetValue() ) ) ));

        m_rXFSet.Put( *pItem );
    }

    if( m_pMtrFldOffset->IsEnabled() )
    {
        if( m_pRbtRow->IsChecked() )
        {
            m_rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) m_pMtrFldOffset->GetValue() ) );
            m_rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) 0 ) );
        }
        else if( m_pRbtColumn->IsChecked() )
        {
            m_rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) 0 ) );
            m_rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) m_pMtrFldOffset->GetValue() ) );
        }
    }

    if( m_pLbPosition->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosItem( static_cast< RECT_POINT >( m_pLbPosition->GetSelectEntryPos() ) ) );

    if( m_pMtrFldXOffset->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosOffsetXItem( (sal_uInt16) m_pMtrFldXOffset->GetValue() ) );

    if( m_pMtrFldYOffset->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosOffsetYItem( (sal_uInt16) m_pMtrFldYOffset->GetValue() ) );


    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

void SvxBitmapTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
