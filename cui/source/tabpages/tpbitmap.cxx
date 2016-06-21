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
    m_rXFSet              ( m_aXFillAttr.GetItemSet() ),

    m_ePoolUnit(SFX_MAPUNIT_100TH_MM),
    m_eFUnit(FUNIT_NONE)
{
    get(m_pLbBitmap,"LB_BITMAP");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");

    get(m_pBxBitmap,"boxBITMAP");

    get(m_pFlSize,"FL_SIZE");
    get(m_pGridX_Y,"gridX_Y");
    get(m_pTsbOriginal,"TSB_ORIGINAL");
    get(m_pTsbScale,"TSB_SCALE");
    get(m_pFtXSize,"FT_X_SIZE");
    get(m_pMtrFldXSize,"MTR_FLD_X_SIZE");
    get(m_pFtYSize,"FT_Y_SIZE");
    get(m_pMtrFldYSize,"MTR_FLD_Y_SIZE");

    get(m_pFlPosition,"framePOSITION");
    get(m_pCtlPosition,"CTL_POSITION");
    get(m_pGridOffset,"gridOFFSET");
    get(m_pMtrFldXOffset,"MTR_FLD_X_OFFSET");
    get(m_pMtrFldYOffset,"MTR_FLD_Y_OFFSET");
    get(m_pBxTile,"boxTILE");
    get(m_pTsbTile,"TSB_TILE");
    get(m_pTsbStretch,"TSB_STRETCH");

    get(m_pFlOffset,"FL_OFFSET");
    get(m_pRbtRow,"RBT_ROW");
    get(m_pRbtColumn,"RBT_COLUMN");
    get(m_pMtrFldOffset,"MTR_FLD_OFFSET");

    VclContainer *pMainFrame = get<VclContainer>("mainframe");
    Size aBitmapSize(m_pBxBitmap->get_preferred_size());
    Size aMainFrame(aBitmapSize.Width(),aBitmapSize.Height());
    pMainFrame->set_width_request(aMainFrame.Width());
    pMainFrame->set_height_request(aMainFrame.Height());

    m_pBxBitmap->Hide();
    m_pTsbOriginal->EnableTriState( false );

    // set Metrics
    m_eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( m_eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            m_eFUnit = FUNIT_MM;
        break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_pMtrFldXSize, m_eFUnit, true );
    SetFieldUnit( *m_pMtrFldYSize, m_eFUnit, true );

    // get PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    assert( pPool && "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXBitmapItem );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pLbBitmap->SetSelectHdl(   LINK( this, SvxBitmapTabPage, ModifyBitmapHdl_Impl ) );

    Link<Edit&,void> aLink( LINK( this, SvxBitmapTabPage, ModifyTileHdl_Impl ) );
    Link<Button*,void> aClickLink( LINK( this, SvxBitmapTabPage, ModifyTileClickHdl_Impl ) );
    m_pTsbTile->SetClickHdl( aClickLink );
    m_pTsbStretch->SetClickHdl( aClickLink );
    m_pTsbOriginal->SetClickHdl( aClickLink );
    m_pMtrFldXSize->SetModifyHdl( aLink );
    m_pMtrFldYSize->SetModifyHdl( aLink );
    m_pRbtRow->SetClickHdl( aClickLink );
    m_pRbtColumn->SetClickHdl( aClickLink );
    m_pMtrFldOffset->SetModifyHdl( aLink );
    m_pMtrFldXOffset->SetModifyHdl( aLink );
    m_pMtrFldYOffset->SetModifyHdl( aLink );
    m_pTsbScale->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickScaleHdl_Impl ) );

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
    m_pBxBitmap.clear();
    m_pFlSize.clear();
    m_pTsbOriginal.clear();
    m_pTsbScale.clear();
    m_pGridX_Y.clear();
    m_pFtXSize.clear();
    m_pMtrFldXSize.clear();
    m_pFtYSize.clear();
    m_pMtrFldYSize.clear();
    m_pFlPosition.clear();
    m_pCtlPosition.clear();
    m_pGridOffset.clear();
    m_pMtrFldXOffset.clear();
    m_pMtrFldYOffset.clear();
    m_pBxTile.clear();
    m_pTsbTile.clear();
    m_pTsbStretch.clear();
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
                ModifyBitmapHdl_Impl( *m_pLbBitmap );
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
        if( m_pTsbTile->IsEnabled() )
        {
            TriState eState = m_pTsbTile->GetState();
            if( m_pTsbTile->IsValueChangedFromSaved() )
            {
                XFillBmpTileItem aFillBmpTileItem( eState != TRISTATE_FALSE );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_TILE );
                if ( !pOld || !( *static_cast<const XFillBmpTileItem*>(pOld) == aFillBmpTileItem ) )
                {
                    rAttrs->Put( aFillBmpTileItem );
                    bModified = true;
                }
            }
        }

        if( m_pTsbStretch->IsEnabled() )
        {
            TriState eState = m_pTsbStretch->GetState();
            if( m_pTsbStretch->IsValueChangedFromSaved() )
            {
                XFillBmpStretchItem aFillBmpStretchItem(
                    eState != TRISTATE_FALSE );
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_STRETCH );
                if ( !pOld || !( *static_cast<const XFillBmpStretchItem*>(pOld) == aFillBmpStretchItem ) )
                {
                    rAttrs->Put( aFillBmpStretchItem );
                    bModified = true;
                }
            }
        }

        // Original size (in the UI) is used as follows:
        // Controls are disabled, but have to be set.
        // SizeX = 0; SizeY = 0; Log = sal_True

        //aTsbScale
        TriState eState = m_pTsbScale->GetState();
        if( m_pTsbScale->IsValueChangedFromSaved() ||
            ( !m_pTsbScale->IsEnabled() &&
              m_pTsbOriginal->IsEnabled() &&
              m_pTsbScale->GetSavedValue() != TRISTATE_TRUE ) )
        {
            std::unique_ptr<XFillBmpSizeLogItem> pItem;
            if( m_pTsbScale->IsEnabled() )
                pItem.reset(new XFillBmpSizeLogItem( eState == TRISTATE_FALSE ));
            else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == TRISTATE_TRUE )
                pItem.reset(new XFillBmpSizeLogItem( true ));

            if( pItem )
            {
                pOld = GetOldItem( *rAttrs, XATTR_FILLBMP_SIZELOG );
                if ( !pOld || !( *static_cast<const XFillBmpSizeLogItem*>(pOld) == *pItem ) )
                {
                    rAttrs->Put( *pItem );
                    bModified = true;
                }
            }
        }

        //aMtrFldXSize
        OUString aStr = m_pMtrFldXSize->GetText();
        {
            std::unique_ptr<XFillBmpSizeXItem> pItem;
            TriState eScaleState = m_pTsbScale->GetState();

            if( m_pMtrFldXSize->IsEnabled() &&
                !aStr.isEmpty()  &&
                aStr != m_pMtrFldXSize->GetSavedValue() )
            {
                if( eScaleState == TRISTATE_FALSE )
                    pItem.reset(new XFillBmpSizeXItem( GetCoreValue( *m_pMtrFldXSize, m_ePoolUnit ) ));
                else
                {
                    // Percentage values are set negatively, so that
                    // they aren't scaled; this is considered in the item.
                    pItem.reset(new XFillBmpSizeXItem( -labs( static_cast<long>(m_pMtrFldXSize->GetValue()) ) ));
                }
            }
            else if( m_pTsbOriginal->IsEnabled() &&
                     m_pTsbOriginal->GetState() == TRISTATE_TRUE &&
                     !m_pMtrFldXSize->GetSavedValue().isEmpty() )
                pItem.reset(new XFillBmpSizeXItem( 0 ));

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
            TriState eScaleState = m_pTsbScale->GetState();

            if( m_pMtrFldYSize->IsEnabled() &&
                !aStr.isEmpty()  &&
                aStr != m_pMtrFldYSize->GetSavedValue() )
            {
                if( eScaleState == TRISTATE_FALSE )
                    pItem.reset(new XFillBmpSizeYItem( GetCoreValue( *m_pMtrFldYSize, m_ePoolUnit ) ));
                else
                {
                    // Percentage values are set negatively, so that
                    // they aren't scaled by the MetricItem;
                    // this is considered in the item.
                    pItem.reset(new XFillBmpSizeYItem( -labs( static_cast<long>(m_pMtrFldYSize->GetValue()) ) ));
                }
            }
            else if( m_pTsbOriginal->IsEnabled() &&
                     m_pTsbOriginal->GetState() == TRISTATE_TRUE &&
                     !m_pMtrFldYSize->GetSavedValue().isEmpty() )
                pItem.reset(new XFillBmpSizeYItem( 0 ));

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
        if( m_pCtlPosition->IsEnabled() )
        {
            bool bPut = false;
            RECT_POINT _eRP = m_pCtlPosition->GetActualRP();

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

    if( rAttrs->GetItemState( XATTR_FILLBMP_TILE ) != SfxItemState::DONTCARE )
    {
        m_pTsbTile->EnableTriState( false );

        if( static_cast<const XFillBmpTileItem&>( rAttrs->Get( XATTR_FILLBMP_TILE ) ).GetValue() )
            m_pTsbTile->SetState( TRISTATE_TRUE );
        else
            m_pTsbTile->SetState( TRISTATE_FALSE );
    }
    else
        m_pTsbTile->SetState( TRISTATE_INDET );

    if( rAttrs->GetItemState( XATTR_FILLBMP_STRETCH ) != SfxItemState::DONTCARE )
    {
        m_pTsbStretch->EnableTriState( false );

        if( static_cast<const XFillBmpStretchItem&>( rAttrs->Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
            m_pTsbStretch->SetState( TRISTATE_TRUE );
        else
            m_pTsbStretch->SetState( TRISTATE_FALSE );
    }
    else
        m_pTsbStretch->SetState( TRISTATE_INDET );


    //aTsbScale
    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZELOG ) != SfxItemState::DONTCARE )
    {
        m_pTsbScale->EnableTriState( false );

        if( static_cast<const XFillBmpSizeLogItem&>( rAttrs->Get( XATTR_FILLBMP_SIZELOG ) ).GetValue() )
            m_pTsbScale->SetState( TRISTATE_FALSE );
        else
            m_pTsbScale->SetState( TRISTATE_TRUE );

        ClickScaleHdl_Impl( nullptr );
    }
    else
        m_pTsbScale->SetState( TRISTATE_INDET );


    // determine status for the original size
    TriState eOriginal = TRISTATE_FALSE;

    //aMtrFldXSize
    if( rAttrs->GetItemState( XATTR_FILLBMP_SIZEX ) != SfxItemState::DONTCARE )
    {
        sal_Int32 nValue = static_cast<const XFillBmpSizeXItem&>( rAttrs->Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if( m_pTsbScale->GetState() == TRISTATE_TRUE )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
            m_pMtrFldXSize->SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( *m_pMtrFldXSize, nValue, m_ePoolUnit );
        m_pMtrFldXSize->SaveValue();

        if( nValue == 0 )
        {
            eOriginal = TRISTATE_TRUE;
            // value would be too small otherwise when turning off the original size
            // (performance problem)
            m_pMtrFldXSize->SetValue( 100 );
        }
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
        if( m_pTsbScale->GetState() == TRISTATE_TRUE )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
            m_pMtrFldYSize->SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( *m_pMtrFldYSize, nValue, m_ePoolUnit );
        m_pMtrFldYSize->SaveValue();

        if( nValue == 0 )
            m_pMtrFldYSize->SetValue( 100 ); //s.o.
        else
            eOriginal = TRISTATE_FALSE;
    }
    else
    {
        m_pMtrFldYSize->SetText( "" );
        m_pMtrFldYSize->SaveValue();
        eOriginal = TRISTATE_FALSE;
    }

    // aTsbOriginal
    m_pTsbOriginal->SetState( eOriginal );

    // #93372# Setting proper state after changing button
    ModifyTileHdl_Impl( *m_pMtrFldXSize );

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
        m_pCtlPosition->SetActualRP( eValue );
    }
    else
        m_pCtlPosition->Reset();

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

    m_pTsbTile->SaveValue();
    m_pTsbStretch->SaveValue();
    m_pTsbScale->SaveValue();
    m_pRbtRow->SaveValue();
    m_pRbtColumn->SaveValue();
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

    m_pBxTile->Enable();

    m_pFlSize->Enable();
    m_pFlPosition->Enable();
    m_pFlOffset->Enable();

    if (!m_pRbtRow->IsChecked() && !m_pRbtColumn->IsChecked())
        m_pRbtRow->Check();

    m_pBxBitmap->Show();

    ModifyBitmapHdl_Impl( *m_pLbBitmap );
    ModifyTileHdl_Impl( *m_pMtrFldXSize );
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyBitmapHdl_Impl, ListBox&, void)
{
    //UUUU

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

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyTileClickHdl_Impl, Button*, void)
{
    ModifyTileHdl_Impl(*m_pMtrFldXSize);
}
IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ModifyTileHdl_Impl, Edit&, void)
{
    TriState eState = m_pTsbTile->GetState();
    if( eState == TRISTATE_TRUE )
    {
        // tiled
        // disable stretched for tiled graphic
        m_pTsbStretch->Disable();
        m_pFlOffset->Enable();

        // allow positioning
        m_pCtlPosition->Invalidate();
        // allow 'Position" title
        m_pFlPosition->Enable();

        // allow size definitions
        m_pFlSize->Enable();
    }
    else if( eState == TRISTATE_FALSE )
    {
        // non-tiled
        // enable stretch selection
        m_pTsbStretch->Enable();
        // no need for offsets, only position is supported in non-tiled
        m_pFlOffset->Disable();

        if( m_pTsbStretch->GetState() != TRISTATE_FALSE )
        {
            // non-tiled, stretched
            // no need for positioning
            m_pCtlPosition->Invalidate();
            // no need for 'Position" title, all deactivated
            m_pFlPosition->Disable();

            // no need for size definitions
            m_pFlSize->Disable();
        }
        else
        {
            // non-tiled, non-stretched
            // allow positioning
            m_pCtlPosition->Enable();
            m_pCtlPosition->Invalidate();
            // allow 'Position" title, positioning is active
            m_pFlPosition->Enable();
            m_pFlSize->Enable();
        }
    }
    else
    {
        // disable all when tiling is undefined
        m_pTsbStretch->Disable();
        m_pFlOffset->Disable();

        m_pCtlPosition->Invalidate();
        m_pFlPosition->Disable();

        m_pFlSize->Disable();
    }

    if( m_pTsbOriginal->GetState() == TRISTATE_TRUE )
    {
        m_pMtrFldXSize->SetText( "" );
        m_pMtrFldYSize->SetText( "" );
        m_pGridX_Y->Disable();
        m_pTsbScale->Disable();
    }
    else
    {
        m_pMtrFldXSize->SetValue( m_pMtrFldXSize->GetValue() );
        m_pMtrFldYSize->SetValue( m_pMtrFldYSize->GetValue() );
    }

    m_rXFSet.Put( XFillBmpTileItem( eState != TRISTATE_FALSE ) );

    if( m_pTsbStretch->IsEnabled() )
        m_rXFSet.Put(
            XFillBmpStretchItem(
                m_pTsbStretch->GetState() != TRISTATE_FALSE ) );

    if( m_pTsbScale->IsEnabled() )
        m_rXFSet.Put( XFillBmpSizeLogItem( m_pTsbScale->GetState() == TRISTATE_FALSE ) );

    if( m_pMtrFldXSize->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeXItem> pItem;
        TriState eScaleState = m_pTsbScale->GetState();

        if( eScaleState == TRISTATE_FALSE )
            pItem.reset(new XFillBmpSizeXItem( GetCoreValue( *m_pMtrFldXSize, m_ePoolUnit ) ));
        else
            pItem.reset(new XFillBmpSizeXItem( -labs( static_cast<long>(m_pMtrFldXSize->GetValue()) ) ));

        m_rXFSet.Put( *pItem );
    }
    else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == TRISTATE_TRUE )
    {
        // original size -> size == 0
        m_rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        m_rXFSet.Put( XFillBmpSizeLogItem( true ) );
    }

    if( m_pMtrFldYSize->IsEnabled() )
    {
        std::unique_ptr<XFillBmpSizeYItem> pItem;
        TriState eScaleState = m_pTsbScale->GetState();

        if( eScaleState == TRISTATE_FALSE )
            pItem.reset(new XFillBmpSizeYItem( GetCoreValue( *m_pMtrFldYSize, m_ePoolUnit ) ));
        else
            pItem.reset(new XFillBmpSizeYItem( -labs( static_cast<long>(m_pMtrFldYSize->GetValue()) ) ));

        m_rXFSet.Put( *pItem );
    }
    else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == TRISTATE_TRUE )
    {
        // original size -> size == 0
        m_rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        m_rXFSet.Put( XFillBmpSizeLogItem( true ) );
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

    if( m_pCtlPosition->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosItem( m_pCtlPosition->GetActualRP() ) );

    if( m_pMtrFldXOffset->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosOffsetXItem( (sal_uInt16) m_pMtrFldXOffset->GetValue() ) );

    if( m_pMtrFldYOffset->IsEnabled() )
        m_rXFSet.Put( XFillBmpPosOffsetYItem( (sal_uInt16) m_pMtrFldYOffset->GetValue() ) );


    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxBitmapTabPage, ClickScaleHdl_Impl, Button*, void)
{
    if( m_pTsbScale->GetState() == TRISTATE_TRUE )
    {
        m_pMtrFldXSize->SetDecimalDigits( 0 );
        m_pMtrFldXSize->SetUnit(FUNIT_PERCENT);
        m_pMtrFldXSize->SetValue( 100 );
        m_pMtrFldXSize->SetMax( 100 );
        m_pMtrFldXSize->SetLast( 100 );

        m_pMtrFldYSize->SetDecimalDigits( 0 );
        m_pMtrFldYSize->SetUnit(FUNIT_PERCENT);
        m_pMtrFldYSize->SetValue( 100 );
        m_pMtrFldYSize->SetMax( 100 );
        m_pMtrFldYSize->SetLast( 100 );
    }
    else
    {
        m_pMtrFldXSize->SetDecimalDigits( 2 );
        m_pMtrFldXSize->SetUnit( m_eFUnit );
        m_pMtrFldXSize->SetValue( 100 );
        m_pMtrFldXSize->SetMax( 999900 );
        m_pMtrFldXSize->SetLast( 100000 );

        m_pMtrFldYSize->SetDecimalDigits( 2 );
        m_pMtrFldYSize->SetUnit( m_eFUnit );
        m_pMtrFldYSize->SetValue( 100 );
        m_pMtrFldYSize->SetMax( 999900 );
        m_pMtrFldYSize->SetLast( 100000 );
    }

    ModifyTileHdl_Impl( *m_pMtrFldXSize );
}


void SvxBitmapTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
    // alignment of the bitmap fill
    ModifyTileHdl_Impl( *m_pMtrFldXSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
