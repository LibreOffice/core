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

#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/xflbckit.hxx>
#include <svx/svdattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <dlgname.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/opengrf.hxx>
#include <vcl/layout.hxx>

using namespace com::sun::star;

// static ----------------------------------------------------------------

enum FillType
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

namespace
{

void lclExtendSize(Size& rSize, const Size& rInputSize)
{
    if (rSize.Width() < rInputSize.Width())
        rSize.setWidth( rInputSize.Width() );
    if (rSize.Height() < rInputSize.Height())
        rSize.setHeight( rInputSize.Height() );
}

} // end anonymous namespace

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
    m_pFillTabPage( nullptr ),
    m_pColorList( nullptr ),
    m_pGradientList( nullptr ),
    m_pHatchingList( nullptr ),
    m_pBitmapList( nullptr ),
    m_pPatternList( nullptr ),

    // local fixed not o be changed values for local pointers
    maFixed_ChangeType(ChangeType::NONE),

    // init with pointers to fixed ChangeType
    m_pnColorListState(&maFixed_ChangeType),
    m_pnBitmapListState(&maFixed_ChangeType),
    m_pnPatternListState(&maFixed_ChangeType),
    m_pnGradientListState(&maFixed_ChangeType),
    m_pnHatchingListState(&maFixed_ChangeType),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{

    get(m_pBtnNone, "btnnone");
    get(m_pBtnColor, "btncolor");
    get(m_pBtnGradient, "btngradient");
    get(m_pBtnHatch, "btnhatch");
    get(m_pBtnBitmap, "btnbitmap");
    get(m_pBtnPattern, "btnpattern");
    get(m_pFillTab, "fillstylebox");
    maBox.AddButton( m_pBtnNone );
    maBox.AddButton( m_pBtnColor );
    maBox.AddButton( m_pBtnGradient );
    maBox.AddButton( m_pBtnHatch );
    maBox.AddButton( m_pBtnBitmap );
    maBox.AddButton( m_pBtnPattern );
    Link< Button*, void > aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_pBtnNone->SetClickHdl(aLink);
    m_pBtnColor->SetClickHdl(aLink);
    m_pBtnGradient->SetClickHdl(aLink);
    m_pBtnHatch->SetClickHdl(aLink);
    m_pBtnBitmap->SetClickHdl(aLink);
    m_pBtnPattern->SetClickHdl(aLink);

    SetExchangeSupport();

    // Calculate optimal size of all pages..
    m_pFillTabPage.disposeAndReset(SvxColorTabPage::Create(m_pFillTab, &m_rXFSet));
    Size aSize = m_pFillTabPage->GetOptimalSize();
    m_pFillTabPage.disposeAndReset(SvxGradientTabPage::Create(m_pFillTab, &m_rXFSet));
    lclExtendSize(aSize, m_pFillTabPage->GetOptimalSize());
    m_pFillTabPage.disposeAndReset(SvxBitmapTabPage::Create(m_pFillTab, &m_rXFSet));
    lclExtendSize(aSize, m_pFillTabPage->GetOptimalSize());
    m_pFillTabPage.disposeAndReset(SvxHatchTabPage::Create(m_pFillTab, &m_rXFSet));
    lclExtendSize(aSize, m_pFillTabPage->GetOptimalSize());
    m_pFillTabPage.disposeAndReset(SvxPatternTabPage::Create(m_pFillTab, &m_rXFSet));
    lclExtendSize(aSize, m_pFillTabPage->GetOptimalSize());
    m_pFillTabPage.disposeAndClear();

    m_pFillTab->set_width_request(aSize.Width());
    m_pFillTab->set_height_request(aSize.Height());
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    disposeOnce();
}

void SvxAreaTabPage::dispose()
{
    m_pBtnNone.clear();
    m_pBtnColor.clear();
    m_pBtnGradient.clear();
    m_pBtnHatch.clear();
    m_pBtnBitmap.clear();
    m_pBtnPattern.clear();
    m_pFillTab.clear();
    m_pFillTabPage.disposeAndClear();
    SvxTabPage::dispose();
}

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rSet.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::DONTCARE )
    {
        XFillStyleItem aFillStyleItem( static_cast<const XFillStyleItem&>( rSet.Get( GetWhich( XATTR_FILLSTYLE ) ) ) );
        eXFS = aFillStyleItem.GetValue();
        m_rXFSet.Put( aFillStyleItem );
    }

    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            SelectFillTypeHdl_Impl( m_pBtnNone );
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            m_rXFSet.Put( static_cast<const XFillColorItem&>( rSet.Get( GetWhich( XATTR_FILLCOLOR ) ) ) );
            SelectFillTypeHdl_Impl( m_pBtnColor );
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            m_rXFSet.Put( static_cast<const XFillGradientItem&>( rSet.Get( GetWhich( XATTR_FILLGRADIENT ) ) ) );
            SelectFillTypeHdl_Impl( m_pBtnGradient );
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            m_rXFSet.Put( rSet.Get(XATTR_FILLHATCH) );
            SelectFillTypeHdl_Impl( m_pBtnHatch );
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            XFillBitmapItem aItem(static_cast<const XFillBitmapItem&>( rSet.Get( GetWhich( XATTR_FILLBITMAP ) ) ));
            m_rXFSet.Put( aItem );
            if(!aItem.isPattern())
                SelectFillTypeHdl_Impl( m_pBtnBitmap );
            else
                SelectFillTypeHdl_Impl( m_pBtnPattern );
            break;
        }
    }
}

template< typename TTabPage >
DeactivateRC SvxAreaTabPage::DeactivatePage_Impl( SfxItemSet* _pSet )
{
    return static_cast<TTabPage&>(*m_pFillTabPage).DeactivatePage(_pSet);
}

DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
    switch( eFillType )
    {
        case TRANSPARENT:
        {
            // Fill: None doesn't have its own tabpage and thus
            // implementation of FillItemSet, so we supply it here
            XFillStyleItem aStyleItem( drawing::FillStyle_NONE );
            _pSet->Put( aStyleItem );
            break;
        }
        case SOLID:
            return DeactivatePage_Impl<SvxColorTabPage>(_pSet);
        case GRADIENT:
            return DeactivatePage_Impl<SvxGradientTabPage>(_pSet);
        case HATCH:
            return DeactivatePage_Impl<SvxHatchTabPage>(_pSet);
        case BITMAP:
            return DeactivatePage_Impl<SvxBitmapTabPage&>(_pSet);
        case PATTERN:
            return DeactivatePage_Impl<SvxPatternTabPage>(_pSet);
        default:
            break;
    }
    return DeactivateRC::LeavePage;
}

template< typename TTabPage >
bool SvxAreaTabPage::FillItemSet_Impl( SfxItemSet* rAttrs)
{
    return static_cast<TTabPage&>( *m_pFillTabPage ).FillItemSet( rAttrs );
}

bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
    switch( eFillType )
    {
        case TRANSPARENT:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            return true;
        }
        case SOLID:
        {
            return FillItemSet_Impl<SvxColorTabPage>( rAttrs );
        }
        case GRADIENT:
        {
            return FillItemSet_Impl<SvxGradientTabPage>( rAttrs );
        }
        case HATCH:
        {
            return FillItemSet_Impl<SvxHatchTabPage>( rAttrs );
        }
        case BITMAP:
        {
            return FillItemSet_Impl<SvxBitmapTabPage>( rAttrs );
        }
        case PATTERN:
        {
            return FillItemSet_Impl<SvxPatternTabPage>( rAttrs );
        }
        default:
            return false;
    }
}

template< typename TTabPage >
void SvxAreaTabPage::Reset_Impl( const SfxItemSet* rAttrs )
{
    static_cast<TTabPage&>( *m_pFillTabPage ).Reset( rAttrs );
}

void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
    auto eFillType = maBox.GetCurrentButtonPos();
    switch(eFillType)
    {
        case SOLID:
        {
            Reset_Impl<SvxColorTabPage>( rAttrs );
            break;
        }
        case GRADIENT:
        {
            Reset_Impl<SvxGradientTabPage>( rAttrs );
            break;
        }
        case HATCH:
        {
            Reset_Impl<SvxHatchTabPage>( rAttrs );
            break;
        }
        case BITMAP:
        {
            Reset_Impl<SvxBitmapTabPage>( rAttrs );
            break;
        }
        case PATTERN:
        {
            Reset_Impl<SvxPatternTabPage>( rAttrs );
            break;
        }
        default:
            break;
    }
}

VclPtr<SfxTabPage> SvxAreaTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxAreaTabPage>::Create( pWindow, *rAttrs );
}

namespace {

VclPtr<SfxTabPage> lcl_CreateFillStyleTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
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

}

IMPL_LINK(SvxAreaTabPage, SelectFillTypeHdl_Impl, Button*, pButton, void)
{
    sal_Int32 nPos = maBox.GetButtonPos( static_cast<PushButton*>(pButton) );
    if(nPos != -1 && nPos != maBox.GetCurrentButtonPos())
    {
        maBox.SelectButton(static_cast<PushButton*>(pButton));
        FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
        m_pFillTabPage.disposeAndReset( lcl_CreateFillStyleTabPage(eFillType, m_pFillTab, m_rXFSet) );
        CreatePage( eFillType , m_pFillTabPage);
    }
}

void SvxAreaTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxColorListItem* pColorListItem = aSet.GetItem<SvxColorListItem>(SID_COLOR_TABLE, false);
    const SvxGradientListItem* pGradientListItem = aSet.GetItem<SvxGradientListItem>(SID_GRADIENT_LIST, false);
    const SvxHatchListItem* pHatchingListItem = aSet.GetItem<SvxHatchListItem>(SID_HATCH_LIST, false);
    const SvxBitmapListItem* pBitmapListItem = aSet.GetItem<SvxBitmapListItem>(SID_BITMAP_LIST, false);
    const SvxPatternListItem* pPatternListItem = aSet.GetItem<SvxPatternListItem>(SID_PATTERN_LIST, false);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pGradientListItem)
        SetGradientList(pGradientListItem->GetGradientList());
    if (pHatchingListItem)
        SetHatchingList(pHatchingListItem->GetHatchList());
    if (pBitmapListItem)
        SetBitmapList(pBitmapListItem->GetBitmapList());
    if (pPatternListItem)
        SetPatternList(pPatternListItem->GetPatternList());
}

void SvxAreaTabPage::PointChanged( vcl::Window* , RectPoint )
{
}

void SvxAreaTabPage::CreatePage( sal_Int32 nId, SfxTabPage* pTab )
{
    if(nId == SOLID )
    {
        static_cast<SvxColorTabPage*>(pTab)->SetColorList( m_pColorList );
        static_cast<SvxColorTabPage*>(pTab)->SetColorChgd( m_pnColorListState );
        static_cast<SvxColorTabPage*>(pTab)->Construct();
        static_cast<SvxColorTabPage*>(pTab)->ActivatePage( m_rXFSet );
        static_cast<SvxColorTabPage*>(pTab)->Reset(&m_rXFSet);
        static_cast<SvxColorTabPage*>(pTab)->Show();
    }
    else if(nId == GRADIENT)
    {
        static_cast<SvxGradientTabPage*>(pTab)->SetColorList( m_pColorList );
        static_cast<SvxGradientTabPage*>(pTab)->SetGradientList( m_pGradientList );
        static_cast<SvxGradientTabPage*>(pTab)->SetGrdChgd( m_pnGradientListState );
        static_cast<SvxGradientTabPage*>(pTab)->SetColorChgd( m_pnColorListState );
        static_cast<SvxGradientTabPage*>(pTab)->Construct();
        static_cast<SvxGradientTabPage*>(pTab)->ActivatePage( m_rXFSet );
        static_cast<SvxGradientTabPage*>(pTab)->Reset(&m_rXFSet);
        static_cast<SvxGradientTabPage*>(pTab)->Show();
    }
    else if(nId == HATCH)
    {
        static_cast<SvxHatchTabPage*>(pTab)->SetColorList( m_pColorList );
        static_cast<SvxHatchTabPage*>(pTab)->SetHatchingList( m_pHatchingList );
        static_cast<SvxHatchTabPage*>(pTab)->SetHtchChgd( m_pnHatchingListState );
        static_cast<SvxHatchTabPage*>(pTab)->SetColorChgd( m_pnColorListState );
        static_cast<SvxHatchTabPage*>(pTab)->Construct();
        static_cast<SvxHatchTabPage*>(pTab)->ActivatePage( m_rXFSet );
        static_cast<SvxHatchTabPage*>(pTab)->Reset(&m_rXFSet);
        static_cast<SvxHatchTabPage*>(pTab)->Show();
    }
    else if(nId == BITMAP)
    {
        static_cast<SvxBitmapTabPage*>(pTab)->SetBitmapList( m_pBitmapList );
        static_cast<SvxBitmapTabPage*>(pTab)->SetBmpChgd( m_pnBitmapListState );
        static_cast<SvxBitmapTabPage*>(pTab)->Construct();
        static_cast<SvxBitmapTabPage*>(pTab)->ActivatePage( m_rXFSet );
        static_cast<SvxBitmapTabPage*>(pTab)->Reset(&m_rXFSet);
        static_cast<SvxBitmapTabPage*>(pTab)->Show();
    }
    else if(nId == PATTERN)
    {
        static_cast<SvxPatternTabPage*>(pTab)->SetColorList( m_pColorList );
        static_cast<SvxPatternTabPage*>(pTab)->SetPatternList( m_pPatternList );
        static_cast<SvxPatternTabPage*>(pTab)->SetPtrnChgd( m_pnPatternListState );
        static_cast<SvxPatternTabPage*>(pTab)->SetColorChgd( m_pnColorListState );
        static_cast<SvxPatternTabPage*>(pTab)->Construct();
        static_cast<SvxPatternTabPage*>(pTab)->ActivatePage( m_rXFSet );
        static_cast<SvxPatternTabPage*>(pTab)->Reset( &m_rXFSet );
        static_cast<SvxPatternTabPage*>(pTab)->Show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
