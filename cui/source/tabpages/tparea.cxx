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

//UUUU
#include "sfx2/opengrf.hxx"
#include <vcl/layout.hxx>

#define DEFAULT_GRADIENTSTEP 64

using namespace com::sun::star;

// static ----------------------------------------------------------------

const sal_uInt16 SvxAreaTabPage::pAreaRanges[] =
{
    XATTR_GRADIENTSTEPCOUNT,
    XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE,
    SID_ATTR_FILL_BITMAP,
    0
};

/*************************************************************************
|*
|*  Dialog to modify fill-attributes
|*
\************************************************************************/

SvxAreaTabPage::SvxAreaTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage( pParent,
                "AreaTabPage",
                "cui/ui/areatabpage.ui",
               rInAttrs ),
    m_rOutAttrs (rInAttrs ),
    m_eRP( RP_LT ),

    m_pColorList( nullptr ),
    m_pGradientList( nullptr ),
    m_pHatchingList( nullptr ),
    m_pBitmapList( nullptr ),

    // local fixed not o be changed values for local pointers
    maFixed_ChangeType(ChangeType::NONE),
    maFixed_sal_Bool(false),

    // init with pointers to fixed ChangeType
    m_pnColorListState(&maFixed_ChangeType),
    m_pnBitmapListState(&maFixed_ChangeType),
    m_pnGradientListState(&maFixed_ChangeType),
    m_pnHatchingListState(&maFixed_ChangeType),

    m_nPageType(0),
    m_nDlgType(0),
    m_nPos(0),

    // init with pointer to fixed bool
    m_pbAreaTP(&maFixed_sal_Bool),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pTypeLB,"LB_AREA_TYPE");
    get(m_pFillLB,"boxLB_FILL");
    get(m_pLbColor,"LB_COLOR");
    get(m_pLbGradient,"LB_GRADIENT");
    get(m_pLbHatching,"LB_HATCHING");
    get(m_pLbBitmap,"LB_BITMAP");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");
    get(m_pCtlXRectPreview,"CTL_COLOR_PREVIEW");


    // groups that overlay each other
    m_pLbBitmap->Hide();
    m_pCtlBitmapPreview->Hide();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // get PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    m_ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // setting the output device
    m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    m_rXFSet.Put( XFillColorItem( OUString(), COL_BLACK ) );
    m_pCtlXRectPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pLbColor->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyColorHdl_Impl ) );
    m_pLbGradient->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyGradientHdl_Impl ) );
    m_pLbHatching->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyHatchingHdl_Impl ) );
    m_pLbBitmap->SetSelectHdl(   LINK( this, SvxAreaTabPage, ModifyBitmapHdl_Impl ) );

    m_pTypeLB->SetSelectHdl( LINK( this, SvxAreaTabPage, SelectDialogTypeHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlXRectPreview->EnableRTL(false);

    // Calculate size of dropdown listboxes
    Size aSize = LogicToPixel(Size(108, 103), MAP_APPFONT);

    m_pLbColor->set_width_request(aSize.Width());
    m_pLbColor->set_height_request(aSize.Height());
    //m_pLbColor->

    m_pLbGradient->set_width_request(aSize.Width());
    m_pLbGradient->set_height_request(aSize.Height());
    m_pLbHatching->set_width_request(aSize.Width());
    m_pLbHatching->set_height_request(aSize.Height());
    m_pLbBitmap->set_width_request(aSize.Width());
    m_pLbBitmap->set_height_request(aSize.Height());

    // Calculate size of display boxes
    Size aSize2 = LogicToPixel(Size(110, 42), MAP_APPFONT);
    m_pCtlBitmapPreview->set_width_request(aSize2.Width());
    m_pCtlBitmapPreview->set_height_request(aSize2.Height());
    m_pCtlXRectPreview->set_width_request(aSize2.Width());
    m_pCtlXRectPreview->set_height_request(aSize2.Height());
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    disposeOnce();
}

void SvxAreaTabPage::dispose()
{
    m_pTypeLB.clear();
    m_pFillLB.clear();
    m_pLbColor.clear();
    m_pLbGradient.clear();
    m_pLbHatching.clear();
    m_pLbBitmap.clear();
    m_pCtlBitmapPreview.clear();
    m_pCtlXRectPreview.clear();
    SvxTabPage::dispose();
}


void SvxAreaTabPage::Construct()
{
    // fill colortables / lists
    m_pLbColor->Fill( m_pColorList );
    m_pLbGradient->Fill( m_pGradientList );
    m_pLbHatching->Fill( m_pHatchingList );
    m_pLbBitmap->Fill( m_pBitmapList );
}


void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_Int32 nCount(0);
    const SfxUInt16Item* pPageTypeItem = rSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pPosItem = rSet.GetItem<SfxUInt16Item>(SID_TABPAGE_POS, false);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());
    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = true;

        if( m_pColorList.is() )
        {
            //UUUU use evtl. previously selected entry to avoid changing values just by
            // switching TabPages in dialogs using this TabPage
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
            }

            if( *m_pnHatchingListState != ChangeType::NONE )
            {
                if( *m_pnHatchingListState & ChangeType::CHANGED )
                    m_pHatchingList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewHatchingList();

                _nPos = m_pLbHatching->GetSelectEntryPos();

                m_pLbHatching->Clear();
                m_pLbHatching->Fill( m_pHatchingList );
                nCount = m_pLbHatching->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbHatching->SelectEntryPos( 0 );
                else
                    m_pLbHatching->SelectEntryPos( _nPos );
                ModifyHatchingHdl_Impl( *m_pLbHatching );
            }

            if( *m_pnGradientListState != ChangeType::NONE )
            {
                if( *m_pnGradientListState & ChangeType::CHANGED )
                    m_pGradientList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewGradientList();

                _nPos = m_pLbGradient->GetSelectEntryPos();

                m_pLbGradient->Clear();
                m_pLbGradient->Fill( m_pGradientList );
                nCount = m_pLbGradient->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbGradient->SelectEntryPos( 0 );
                else
                    m_pLbGradient->SelectEntryPos( _nPos );
                ModifyGradientHdl_Impl( *m_pLbGradient );
            }

            if( *m_pnColorListState != ChangeType::NONE )
            {
                if( *m_pnColorListState & ChangeType::CHANGED )
                    m_pColorList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewColorList();
                // aLbColor
                _nPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( m_pColorList );
                nCount = m_pLbColor->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbColor->SelectEntryPos( 0 );
                else
                    m_pLbColor->SelectEntryPos( _nPos );

                ModifyColorHdl_Impl( *m_pLbColor );
            }

            // evaluate if any other Tabpage set another filltype
            if( m_pTypeLB->GetSelectEntryPos() > drawing::FillStyle_NONE)
            {
                switch( m_nPageType )
                {
                    case PT_GRADIENT:
                        m_pTypeLB->SelectEntryPos( drawing::FillStyle_GRADIENT );
                        m_pLbGradient->SelectEntryPos( _nPos );
                        ClickGradientHdl_Impl();
                    break;

                    case PT_HATCH:
                        m_pTypeLB->SelectEntryPos( drawing::FillStyle_HATCH );
                        m_pLbHatching->SelectEntryPos( _nPos );
                        ClickHatchingHdl_Impl();
                    break;

                    case PT_BITMAP:
                        m_pTypeLB->SelectEntryPos( drawing::FillStyle_BITMAP );
                        m_pLbBitmap->SelectEntryPos( _nPos );
                        ClickBitmapHdl_Impl();
                    break;

                    case PT_COLOR:
                        m_pTypeLB->SelectEntryPos( drawing::FillStyle_SOLID );
                        m_pLbColor->SelectEntryPos( _nPos );
                        ClickColorHdl_Impl();
                    break;
                }
            }
            m_nPageType = PT_AREA;
        }
    }
}


DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( m_nDlgType == 0 ) // area dialog
    {
        sal_Int32 nPosOrig = m_nPos;
        drawing::FillStyle eStyle = (drawing::FillStyle) m_pTypeLB->GetSelectEntryPos();
        switch( eStyle )
        {
            case drawing::FillStyle_GRADIENT:
            {
                        m_nPageType = PT_GRADIENT;
                        m_nPos = m_pLbGradient->GetSelectEntryPos();
                        if( nPosOrig != m_nPos )
                            *m_pnGradientListState |= ChangeType::MODIFIED;
            }
            break;
            case drawing::FillStyle_HATCH:
            {
                m_nPageType = PT_HATCH;
                m_nPos = m_pLbHatching->GetSelectEntryPos();
                if( nPosOrig != m_nPos )
                    *m_pnHatchingListState |= ChangeType::MODIFIED;
            }
            break;
            case drawing::FillStyle_BITMAP:
            {
                m_nPageType = PT_BITMAP;
                m_nPos = m_pLbBitmap->GetSelectEntryPos();
                if( nPosOrig != m_nPos )
                    *m_pnBitmapListState |= ChangeType::MODIFIED;
            }
            break;
            case drawing::FillStyle_SOLID:
            {
                m_nPageType = PT_COLOR;
                m_nPos = m_pLbColor->GetSelectEntryPos();
                if( nPosOrig != m_nPos )
                    *m_pnColorListState |= ChangeType::MODIFIED;
            }
            break;
            default: ;//prevent warning
        }
    }

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    sal_Int32  _nPos;
    bool    bModified = false;

    if( m_nDlgType != 0 || *m_pbAreaTP )
    {
        const SfxPoolItem* pOld = nullptr;
        drawing::FillStyle eStyle = (drawing::FillStyle) m_pTypeLB->GetSelectEntryPos();
        drawing::FillStyle eSavedStyle = (drawing::FillStyle) m_pTypeLB->GetSavedValue();
        switch( eStyle )
        {
            default:
            case drawing::FillStyle_NONE:
            {
                if(  eSavedStyle != eStyle )
                {
                    XFillStyleItem aStyleItem( drawing::FillStyle_NONE );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *static_cast<const XFillStyleItem*>(pOld) == aStyleItem ) )
                    {
                        rAttrs->Put( aStyleItem );
                        bModified = true;
                    }
                }
            }
            break;
            case drawing::FillStyle_SOLID:
            {
                _nPos = m_pLbColor->GetSelectEntryPos();
                 if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                     m_pLbColor->IsValueChangedFromSaved() )
                 {
                     XFillColorItem aItem( m_pLbColor->GetSelectEntry(),
                                           m_pLbColor->GetSelectEntryColor() );
                     pOld = GetOldItem( *rAttrs, XATTR_FILLCOLOR );
                     if ( !pOld || !( *static_cast<const XFillColorItem*>(pOld) == aItem ) )
                     {
                         rAttrs->Put( aItem );
                         bModified = true;
                     }
                 }
                 // NEW
                 if( (eSavedStyle != eStyle) &&
                     ( bModified ||
                       SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ) ) ) )
                 {
                     XFillStyleItem aStyleItem( drawing::FillStyle_SOLID );
                     pOld = GetOldItem( *rAttrs, XATTR_FILLSTYLE );
                     if ( !pOld || !( *static_cast<const XFillStyleItem*>(pOld) == aStyleItem ) )
                     {
                         rAttrs->Put( aStyleItem );
                         bModified = true;
                     }
                 }
            }
            break;
            case drawing::FillStyle_GRADIENT:
            {
                _nPos = m_pLbGradient->GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                    m_pLbGradient->IsValueChangedFromSaved() )
                {
                    XGradient aGradient = m_pGradientList->GetGradient( _nPos )->GetGradient();
                    OUString aString = m_pLbGradient->GetSelectEntry();
                    XFillGradientItem aItem( aString, aGradient );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLGRADIENT );
                    if ( !pOld || !( *static_cast<const XFillGradientItem*>(pOld) == aItem ) )
                    {
                        rAttrs->Put( aItem );
                        bModified = true;
                    }
                }
                // NEW
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ) ) ) )
                {
                    XFillStyleItem aStyleItem( drawing::FillStyle_GRADIENT );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *static_cast<const XFillStyleItem*>(pOld) == aStyleItem ) )
                    {
                        rAttrs->Put( aStyleItem );
                        bModified = true;
                    }
                }
            }
            break;
            case drawing::FillStyle_HATCH:
            {
                _nPos = m_pLbHatching->GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                   m_pLbHatching->IsValueChangedFromSaved() )
                {
                    XHatch aHatching = m_pHatchingList->GetHatch( _nPos )->GetHatch();
                    OUString aString = m_pLbHatching->GetSelectEntry();
                    XFillHatchItem aItem( aString, aHatching );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLHATCH );
                    if ( !pOld || !( *static_cast<const XFillHatchItem*>(pOld) == aItem ) )
                    {
                        rAttrs->Put( aItem );
                        bModified = true;
                    }
                }
                // NEW
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ) ) ) )
                {
                    XFillStyleItem aStyleItem( drawing::FillStyle_HATCH );
                    pOld = GetOldItem( *rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *static_cast<const XFillStyleItem*>(pOld) == aStyleItem ) )
                    {
                        rAttrs->Put( aStyleItem );
                        bModified = true;
                    }
                }
            }
            break;
            case drawing::FillStyle_BITMAP:
            {
                //UUUU
                if(mbDirectGraphicSet && GraphicType::NONE != maDirectGraphic.GetType())
                {
                    const XFillBitmapItem aXBmpItem(maDirectName, maDirectGraphic);
                    rAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
                    rAttrs->Put(aXBmpItem);
                    bModified = true;
                }
                else
                {
                    m_nPos = m_pLbBitmap->GetSelectEntryPos();
                    if( m_nPos != LISTBOX_ENTRY_NOTFOUND &&
                        m_pLbBitmap->IsValueChangedFromSaved() )
                    {
                        const XBitmapEntry* pXBitmapEntry = m_pBitmapList->GetBitmap(m_nPos);
                        const OUString aString(m_pLbBitmap->GetSelectEntry());
                        const XFillBitmapItem aFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject());
                        pOld = GetOldItem( *rAttrs, XATTR_FILLBITMAP );
                        if ( !pOld || !( *static_cast<const XFillBitmapItem*>(pOld) == aFillBitmapItem ) )
                        {
                            rAttrs->Put( aFillBitmapItem );
                            bModified = true;
                        }
                    }
                    // NEW
                    if( (eSavedStyle != eStyle) &&
                        ( bModified ||
                          SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ) ) ) )
                    {
                            XFillStyleItem aStyleItem( drawing::FillStyle_BITMAP );
                            pOld = GetOldItem( *rAttrs, XATTR_FILLSTYLE );
                            if ( !pOld || !( *static_cast<const XFillStyleItem*>(pOld) == aStyleItem ) )
                            {
                                rAttrs->Put( aStyleItem );
                                bModified = true;
                            }
                    }
                }
           }
           break;
        }
        rAttrs->Put (SfxUInt16Item(SID_PAGE_TYPE,m_nPageType));
        rAttrs->Put (SfxUInt16Item(SID_TABPAGE_POS,m_nPos));
    }

    return bModified;
}


void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
    bool isMissingHatching(false);
    bool isMissingGradient(false);
    bool isMissingBitmap(false);
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rAttrs->GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        eXFS = (drawing::FillStyle) ( static_cast<const XFillStyleItem&>( rAttrs->
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        m_pTypeLB->SelectEntryPos( sal::static_int_cast< sal_Int32 >( eXFS ) );

        if (SfxItemState::DONTCARE != rAttrs->GetItemState(XATTR_FILLCOLOR))
        {
            XFillColorItem const& rColorItem(static_cast<const XFillColorItem&>(
                                rAttrs->Get(XATTR_FILLCOLOR)) );
            m_pLbColor->SelectEntry( rColorItem.GetColorValue() );
        }

        SfxItemState const eGradState(rAttrs->GetItemState(XATTR_FILLGRADIENT));
        XFillGradientItem const* pGradientItem(nullptr);
        if (SfxItemState::DONTCARE != eGradState)
        {
            pGradientItem = &static_cast<const XFillGradientItem&>(
                                    rAttrs->Get(XATTR_FILLGRADIENT));
            OUString  const aString( pGradientItem->GetName() );
            XGradient const aGradient( pGradientItem->GetGradientValue() );
            m_pLbGradient->SelectEntryByList(m_pGradientList, aString, aGradient);
        }
        if (!m_pLbGradient->GetSelectEntryCount()
            && (SfxItemState::DEFAULT == eGradState
                || (pGradientItem && pGradientItem->GetName().isEmpty())))
        {   // avoid relying on pool default - cannot export that
            m_pLbGradient->SelectEntryPos(0); // anything better than nothing
            isMissingGradient = true;
        }

        SfxItemState const eHatchState(rAttrs->GetItemState(XATTR_FILLHATCH));
        XFillHatchItem const* pHatch(nullptr);
        if (SfxItemState::DONTCARE != eHatchState)
        {
            pHatch = &static_cast<const XFillHatchItem&>(
                                rAttrs->Get(XATTR_FILLHATCH));
            m_pLbHatching->SelectEntry(pHatch->GetName());
        }
        if (!m_pLbHatching->GetSelectEntryCount()
            && (SfxItemState::DEFAULT == eHatchState
                || (pHatch && pHatch->GetName().isEmpty())))
        {   // avoid relying on pool default - cannot export that
            m_pLbHatching->SelectEntryPos(0); // anything better than nothing
            isMissingHatching = true;
        }

        SfxItemState const eBitmapState(rAttrs->GetItemState(XATTR_FILLBITMAP));
        XFillBitmapItem const* pBitmapItem(nullptr);
        if (SfxItemState::DONTCARE != eBitmapState)
        {
            pBitmapItem = &static_cast<const XFillBitmapItem&>(
                            rAttrs->Get(XATTR_FILLBITMAP));
            m_pLbBitmap->SelectEntry(pBitmapItem->GetName());
        }
        if (!m_pLbBitmap->GetSelectEntryCount()
            && (SfxItemState::DEFAULT == eBitmapState
                || (pBitmapItem && pBitmapItem->GetName().isEmpty())))
        {   // avoid relying on pool default - cannot export that
            m_pLbBitmap->SelectEntryPos(0); // anything better than nothing
            isMissingBitmap = true;
        }

        switch( eXFS )
        {
            case drawing::FillStyle_NONE:
                ClickInvisibleHdl_Impl();
            break;

            case drawing::FillStyle_SOLID:
                ClickColorHdl_Impl();
            break;

            case drawing::FillStyle_GRADIENT:
                ClickGradientHdl_Impl();
            break;

            case drawing::FillStyle_HATCH:
                ClickHatchingHdl_Impl();
            break;

            case drawing::FillStyle_BITMAP:
            {
                ClickBitmapHdl_Impl();
            }
            break;

            default:
                assert(false);
            break;
        }
    }
    else
    {
        // make all LBs not accessible
        m_pFillLB->Hide();

        m_pCtlBitmapPreview->Hide();
        m_pLbColor->Disable();
        m_pLbColor->Show();

        // so that Reset() also works correctly with Back
        m_pTypeLB->SetNoSelection();
    }

    if( m_pTypeLB->GetSelectEntryPos() == drawing::FillStyle_BITMAP )
        ClickBitmapHdl_Impl();

    m_pTypeLB->SaveValue();
    if(eXFS == drawing::FillStyle_SOLID)
        m_pLbColor->SaveValue();
    if (!isMissingGradient)
        m_pLbGradient->SaveValue();
    if (!isMissingHatching)
        m_pLbHatching->SaveValue();
    if (!isMissingBitmap)
        m_pLbBitmap->SaveValue();
}

void SvxAreaTabPage::ChangesApplied()
{
    m_pTypeLB->SaveValue();
    m_pLbColor->SaveValue();
    m_pLbGradient->SaveValue();
    m_pLbHatching->SaveValue();
    m_pLbBitmap->SaveValue();
}


VclPtr<SfxTabPage> SvxAreaTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxAreaTabPage>::Create( pWindow, *rAttrs );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabPage, SelectDialogTypeHdl_Impl, ListBox&, void)
{
    switch( (drawing::FillStyle)m_pTypeLB->GetSelectEntryPos() )
    {
        default:
        case drawing::FillStyle_NONE: ClickInvisibleHdl_Impl(); break;
        case drawing::FillStyle_SOLID: ClickColorHdl_Impl(); break;
        case drawing::FillStyle_GRADIENT: ClickGradientHdl_Impl(); break;
        case drawing::FillStyle_HATCH: ClickHatchingHdl_Impl(); break;
        case drawing::FillStyle_BITMAP: ClickBitmapHdl_Impl(); break;
    }
}

void SvxAreaTabPage::ClickInvisibleHdl_Impl()
{
    m_pFillLB->Hide();

    m_pCtlXRectPreview->Hide();
    m_pCtlBitmapPreview->Hide();

    m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
    m_pCtlXRectPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pCtlXRectPreview->Invalidate();
    m_pCtlBitmapPreview->Invalidate();
}


void SvxAreaTabPage::ClickColorHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Enable();
    m_pLbColor->Show();
    m_pLbGradient->Hide();
    m_pLbHatching->Hide();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    ModifyColorHdl_Impl( *m_pLbColor );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabPage, ModifyColorHdl_Impl, ListBox&, void)
{
    const SfxPoolItem* pPoolItem = nullptr;
    sal_Int32 _nPos = m_pLbColor->GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
        m_rXFSet.Put( XFillColorItem( OUString(), m_pLbColor->GetSelectEntryColor() ) );
    }
    // NEW
    else if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), true, &pPoolItem ) )
    {
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
        Color aColor( static_cast<const XFillColorItem*>( pPoolItem )->GetColorValue() );
        m_rXFSet.Put( XFillColorItem( OUString(), aColor ) );
    }
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    m_pCtlXRectPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();
}


void SvxAreaTabPage::ClickGradientHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Enable();
    m_pLbGradient->Show();
    m_pLbHatching->Hide();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    ModifyGradientHdl_Impl( *m_pLbGradient );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabPage, ModifyGradientHdl_Impl, ListBox&, void)
{
    const SfxPoolItem* pPoolItem = nullptr;
    sal_Int32 _nPos = m_pLbGradient->GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        XGradientEntry* pEntry = m_pGradientList->GetGradient( _nPos );

        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
        m_rXFSet.Put( XFillGradientItem( OUString(), pEntry->GetGradient() ) );
    }
    else if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), true, &pPoolItem ) )
    {
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
        m_rXFSet.Put( XFillGradientItem( OUString(), static_cast<const XFillGradientItem*>( pPoolItem )->GetGradientValue() ) );
    }
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    sal_uInt16 nValue = static_cast<const XGradientStepCountItem&>( m_rOutAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
    if( nValue == 0 )
        nValue = DEFAULT_GRADIENTSTEP;
    m_rXFSet.Put( XGradientStepCountItem( nValue ) );

    m_pCtlXRectPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();
}


void SvxAreaTabPage::ClickHatchingHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Hide();
    m_pLbHatching->Enable();
    m_pLbHatching->Show();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    ModifyHatchingHdl_Impl( *m_pLbHatching );
}

IMPL_LINK_NOARG_TYPED(SvxAreaTabPage, ModifyHatchingHdl_Impl, ListBox&, void)
{
    // fill Hatch ItemSet
    const SfxPoolItem* pPoolItem = nullptr;
    sal_Int32 _nPos = m_pLbHatching->GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        XHatchEntry* pEntry = m_pHatchingList->GetHatch( _nPos );

        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
        m_rXFSet.Put( XFillHatchItem( OUString(), pEntry->GetHatch() ) );
    }
    else if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), true, &pPoolItem ) )
    {
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
        m_rXFSet.Put( XFillHatchItem( OUString(), static_cast<const XFillHatchItem*>( pPoolItem )->GetHatchValue() ) );
    }
    else
        m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    // fill Hatch background ItemSet
    XFillBackgroundItem aItem(static_cast<const XFillBackgroundItem&>(m_rOutAttrs.Get( XATTR_FILLBACKGROUND )));
    aItem.SetWhich( XATTR_FILLBACKGROUND );
    m_rXFSet.Put ( aItem );
    if(aItem.GetValue())
    {
        if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), true, &pPoolItem ) )
        {
            Color aColor( static_cast<const XFillColorItem*>( pPoolItem )->GetColorValue() );
            m_rXFSet.Put( XFillColorItem( OUString(), aColor ) );
        }
        else
            m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
    }

    m_pCtlXRectPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();
}

void SvxAreaTabPage::ClickBitmapHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Hide();
    m_pLbHatching->Hide();
    m_pLbBitmap->Show();
    m_pCtlBitmapPreview->Enable();
    m_pCtlBitmapPreview->Show();
    m_pCtlXRectPreview->Hide();

    ModifyBitmapHdl_Impl( *m_pLbBitmap );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabPage, ModifyBitmapHdl_Impl, ListBox&, void)
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

    m_pCtlBitmapPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();
}

void SvxAreaTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
}

void SvxAreaTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxColorListItem* pColorListItem = aSet.GetItem<SvxColorListItem>(SID_COLOR_TABLE, false);
    const SvxGradientListItem* pGradientListItem = aSet.GetItem<SvxGradientListItem>(SID_GRADIENT_LIST, false);
    const SvxHatchListItem* pHatchingListItem = aSet.GetItem<SvxHatchListItem>(SID_HATCH_LIST, false);
    const SvxBitmapListItem* pBitmapListItem = aSet.GetItem<SvxBitmapListItem>(SID_BITMAP_LIST, false);
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);
    const SfxUInt16Item* pPosItem = aSet.GetItem<SfxUInt16Item>(SID_TABPAGE_POS, false);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pGradientListItem)
        SetGradientList(pGradientListItem->GetGradientList());
    if (pHatchingListItem)
        SetHatchingList(pHatchingListItem->GetHatchList());
    if (pBitmapListItem)
        SetBitmapList(pBitmapListItem->GetBitmapList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
