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
#include <sfx2/tabdlg.hxx>
#include "sfx2/opengrf.hxx"
#include <vcl/layout.hxx>

using namespace com::sun::star;

// static ----------------------------------------------------------------

enum eFillStyle
{
    TRANSPARENT,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

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
    m_pbAreaTP(false),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() ),
    m_pFillTabPage (nullptr)

{
    get(m_pBtnNone, "btnnone");
    get(m_pBtnColor, "btncolor");
    get(m_pBtnGradient, "btngradient");
    get(m_pBtnHatch, "btnhatch");
    get(m_pBtnBitmap, "btnbitmap");
    get(m_pBtnPattern, "btnpattern");
    get(m_pFillTab, "fillstylebox");

    Link< Button*, void > aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_pBtnNone->SetClickHdl(aLink);
    m_pBtnColor->SetClickHdl(aLink);
    m_pBtnGradient->SetClickHdl(aLink);
    m_pBtnHatch->SetClickHdl(aLink);
    m_pBtnBitmap->SetClickHdl(aLink);
    m_pBtnPattern->SetClickHdl(aLink);

    m_pFillTab->set_width_request(600);
    m_pFillTab->set_height_request(400);
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    disposeOnce();
}

void SvxAreaTabPage::dispose()
{
    m_pFillTab.clear();
    m_pFillTabPage.disposeAndClear();
    SvxTabPage::dispose();
}


void SvxAreaTabPage::Construct()
{
}


void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rSet.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        eXFS = (drawing::FillStyle) ( static_cast<const XFillStyleItem&>( rSet.Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
    }
    switch(eXFS)
    {
        case drawing::FillStyle_NONE: SelectFillTypeHdl_Impl( m_pBtnNone ); break;
        case drawing::FillStyle_SOLID: SelectFillTypeHdl_Impl( m_pBtnColor ); break;
        case drawing::FillStyle_GRADIENT: SelectFillTypeHdl_Impl( m_pBtnGradient ); break;
        case drawing::FillStyle_HATCH: SelectFillTypeHdl_Impl( m_pBtnHatch ); break;
    }
}


DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(m_pBtnNone->IsPressed())
        return DeactivateRC::LeavePage;
//    if(m_pBtnColor->IsPressed())

}


bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    if(m_pBtnNone->IsPressed())
    {
        rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
    }
    if(m_pBtnColor->IsPressed())
    {
        static_cast<SvxColorTabPage&>(*m_pFillTabPage).FillItemSet(rAttrs);
    }
    if(m_pBtnGradient->IsPressed())
    {
        static_cast<SvxGradientTabPage&>(*m_pFillTabPage).FillItemSet(rAttrs);
    }
    if(m_pBtnHatch->IsPressed())
    {
        static_cast<SvxHatchTabPage&>(*m_pFillTabPage).FillItemSet(rAttrs);
    }
    if(m_pBtnBitmap->IsPressed())
    {
        static_cast<SvxBitmapTabPage&>(*m_pFillTabPage).FillItemSet(rAttrs);
    }
    if(m_pBtnPattern->IsPressed())
    {
        static_cast<SvxPatternTabPage&>(*m_pFillTabPage).FillItemSet(rAttrs);
    }
    return true;
}


void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rAttrs->GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        eXFS = (drawing::FillStyle) ( static_cast<const XFillStyleItem&>( rAttrs->Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
    }
    switch(eXFS)
    {
        case drawing::FillStyle_NONE: SelectFillTypeHdl_Impl( m_pBtnNone ); break;
        case drawing::FillStyle_SOLID: SelectFillTypeHdl_Impl( m_pBtnColor ); break;
        case drawing::FillStyle_GRADIENT: SelectFillTypeHdl_Impl( m_pBtnGradient ); break;
        case drawing::FillStyle_HATCH: SelectFillTypeHdl_Impl( m_pBtnHatch ); break;
    }
}

VclPtr<SfxTabPage> SvxAreaTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxAreaTabPage>::Create( pWindow, *rAttrs );
}

VclPtr<SfxTabPage> CreateFillStyleTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
{
    CreateTabPage fnCreate = nullptr;
    switch(nId)
    {
        case TRANSPARENT: fnCreate = nullptr; break;
        case SOLID: fnCreate = &SvxColorTabPage::Create; break;
        case GRADIENT: fnCreate = &SvxGradientTabPage::Create; break;
        case HATCH: fnCreate = &SvxHatchTabPage::Create; break;
        case BITMAP: fnCreate = &SvxBitmapTabPage::Create; break;
        case PATTERN: fnCreate = &SvxPatternTabPage::Create; break;
    }
    VclPtr<SfxTabPage> pRet = fnCreate ? (*fnCreate)( pParent, &rSet ) : nullptr;
    return pRet;
}

IMPL_LINK_TYPED(SvxAreaTabPage, SelectFillTypeHdl_Impl, Button*, pButton, void)
{
    sal_Int32 nPos = -1;
    if(pButton == m_pBtnNone)
    {
        m_pBtnNone->SetPressed(true);
        m_pBtnColor->SetPressed(false);
        m_pBtnGradient->SetPressed(false);
        m_pBtnHatch->SetPressed(false);
        m_pBtnBitmap->SetPressed(false);
        m_pBtnPattern->SetPressed(false);
        nPos = 0;
    }
    else if(pButton == m_pBtnColor)
    {
        m_pBtnNone->SetPressed(false);
        m_pBtnColor->SetPressed(true);
        m_pBtnGradient->SetPressed(false);
        nPos = 1;
    }
    else if(pButton == m_pBtnGradient)
    {
        m_pBtnNone->SetPressed(false);
        m_pBtnColor->SetPressed(false);
        m_pBtnGradient->SetPressed(true);
        m_pBtnHatch->SetPressed(false);
        m_pBtnBitmap->SetPressed(false);
        m_pBtnPattern->SetPressed(false);
        nPos = 2;
    }
    else if(pButton == m_pBtnHatch)
    {
        m_pBtnNone->SetPressed(false);
        m_pBtnColor->SetPressed(false);
        m_pBtnGradient->SetPressed(false);
        m_pBtnHatch->SetPressed(true);
        m_pBtnBitmap->SetPressed(false);
        m_pBtnPattern->SetPressed(false);
        nPos = 3;
    }
    else if(pButton == m_pBtnBitmap)
    {
        m_pBtnNone->SetPressed(false);
        m_pBtnColor->SetPressed(false);
        m_pBtnGradient->SetPressed(false);
        m_pBtnHatch->SetPressed(false);
        m_pBtnBitmap->SetPressed(true);
        m_pBtnPattern->SetPressed(false);
        nPos = 4;
    }
    else if(pButton == m_pBtnPattern)
    {
        m_pBtnNone->SetPressed(false);
        m_pBtnColor->SetPressed(false);
        m_pBtnGradient->SetPressed(false);
        m_pBtnHatch->SetPressed(false);
        m_pBtnBitmap->SetPressed(false);
        m_pBtnPattern->SetPressed(true);
        nPos = 5;
    }
    m_pFillTabPage.disposeAndReset( CreateFillStyleTabPage(nPos, m_pFillTab, m_rXFSet) );
    CreatePage( nPos , *m_pFillTabPage);
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

void SvxAreaTabPage::CreatePage( sal_Int32 nId, SfxTabPage& pTab )
{
    if(nId == 0)
    {

    }
    else if(nId == 1 )
    {
        m_nPageType = PT_COLOR;
        static_cast<SvxColorTabPage&>(pTab).SetColorList( m_pColorList );
        static_cast<SvxColorTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxColorTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxColorTabPage&>(pTab).SetPos( &m_nPos );
        static_cast<SvxColorTabPage&>(pTab).SetAreaTP( m_pbAreaTP );
        static_cast<SvxColorTabPage&>(pTab).SetColorChgd( m_pnColorListState );
        static_cast<SvxColorTabPage&>(pTab).Construct();
        static_cast<SvxColorTabPage&>(pTab).ActivatePage( m_rXFSet );
        static_cast<SvxColorTabPage&>(pTab).Reset(&m_rXFSet);
        static_cast<SvxColorTabPage&>(pTab).Show();
    }
    else if(nId == 2)
    {
        m_nPageType = PT_GRADIENT;
        static_cast<SvxGradientTabPage&>(pTab).SetColorList( m_pColorList );
        static_cast<SvxGradientTabPage&>(pTab).SetGradientList( m_pGradientList );
        static_cast<SvxGradientTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxGradientTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxGradientTabPage&>(pTab).SetPos( &m_nPos );
        static_cast<SvxGradientTabPage&>(pTab).SetAreaTP( m_pbAreaTP );
        static_cast<SvxGradientTabPage&>(pTab).SetGrdChgd( m_pnGradientListState );
        static_cast<SvxGradientTabPage&>(pTab).SetColorChgd( m_pnColorListState );
        static_cast<SvxGradientTabPage&>(pTab).Construct();
        static_cast<SvxGradientTabPage&>(pTab).ActivatePage( m_rXFSet );
        static_cast<SvxGradientTabPage&>(pTab).Show();
    }
    else if(nId == 3)
    {
        m_nPageType = PT_HATCH;
        static_cast<SvxHatchTabPage&>(pTab).SetColorList( m_pColorList );
        static_cast<SvxHatchTabPage&>(pTab).SetHatchingList( m_pHatchingList );
        static_cast<SvxHatchTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxHatchTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxHatchTabPage&>(pTab).SetPos(&m_nPos);
        static_cast<SvxHatchTabPage&>(pTab).SetAreaTP( m_pbAreaTP );
        static_cast<SvxHatchTabPage&>(pTab).SetHtchChgd( m_pnHatchingListState );
        static_cast<SvxHatchTabPage&>(pTab).SetColorChgd( m_pnColorListState );
        static_cast<SvxHatchTabPage&>(pTab).Construct();
        static_cast<SvxHatchTabPage&>(pTab).ActivatePage( m_rXFSet );
        static_cast<SvxHatchTabPage&>(pTab).Show();
    }
    else if(nId == 4)
    {
        m_nPageType = PT_BITMAP;
        static_cast<SvxBitmapTabPage&>(pTab).SetBitmapList( m_pBitmapList );
        static_cast<SvxBitmapTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxBitmapTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxBitmapTabPage&>(pTab).SetPos( &m_nPos );
        static_cast<SvxBitmapTabPage&>(pTab).SetAreaTP( m_pbAreaTP );
        static_cast<SvxBitmapTabPage&>(pTab).SetBmpChgd( m_pnBitmapListState );
        static_cast<SvxBitmapTabPage&>(pTab).Construct();
        static_cast<SvxBitmapTabPage&>(pTab).ActivatePage( m_rXFSet );
        static_cast<SvxBitmapTabPage&>(pTab).Show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
