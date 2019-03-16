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

SvxAreaTabPage::SvxAreaTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "cui/ui/areatabpage.ui", "AreaTabPage", &rInAttrs)
    // local fixed not o be changed values for local pointers
    , maFixed_ChangeType(ChangeType::NONE)
    // init with pointers to fixed ChangeType
    , m_pnColorListState(&maFixed_ChangeType)
    , m_pnBitmapListState(&maFixed_ChangeType)
    , m_pnPatternListState(&maFixed_ChangeType)
    , m_pnGradientListState(&maFixed_ChangeType)
    , m_pnHatchingListState(&maFixed_ChangeType)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_xFillTab(m_xBuilder->weld_container("fillstylebox"))
    , m_xBtnNone(m_xBuilder->weld_toggle_button("btnnone"))
    , m_xBtnColor(m_xBuilder->weld_toggle_button("btncolor"))
    , m_xBtnGradient(m_xBuilder->weld_toggle_button("btngradient"))
    , m_xBtnHatch(m_xBuilder->weld_toggle_button("btnhatch"))
    , m_xBtnBitmap(m_xBuilder->weld_toggle_button("btnbitmap"))
    , m_xBtnPattern(m_xBuilder->weld_toggle_button("btnpattern"))
{
    maBox.AddButton(m_xBtnNone.get());
    maBox.AddButton(m_xBtnColor.get());
    maBox.AddButton(m_xBtnGradient.get());
    maBox.AddButton(m_xBtnHatch.get());
    maBox.AddButton(m_xBtnBitmap.get());
    maBox.AddButton(m_xBtnPattern.get());
    Link<weld::ToggleButton&, void> aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_xBtnNone->connect_toggled(aLink);
    m_xBtnColor->connect_toggled(aLink);
    m_xBtnGradient->connect_toggled(aLink);
    m_xBtnHatch->connect_toggled(aLink);
    m_xBtnBitmap->connect_toggled(aLink);
    m_xBtnPattern->connect_toggled(aLink);

    SetExchangeSupport();
}

void SvxAreaTabPage::SetOptimalSize(weld::DialogController* pController)
{
    m_xFillTab->set_size_request(-1, -1);

    TabPageParent aFillTab(m_xFillTab.get(), pController);
    // TEMP
    if (!aFillTab.pController)
        aFillTab.pParent = GetParentDialog();

    // Calculate optimal size of all pages..
    m_pFillTabPage.disposeAndReset(SvxColorTabPage::Create(aFillTab, &m_rXFSet));
    Size aSize(m_xFillTab->get_preferred_size());

    if (m_xBtnGradient->get_visible())
    {
        m_pFillTabPage.disposeAndReset(SvxGradientTabPage::Create(aFillTab, &m_rXFSet));
        Size aGradientSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aGradientSize);
    }
    if (m_xBtnBitmap->get_visible())
    {
        m_pFillTabPage.disposeAndReset(SvxBitmapTabPage::Create(aFillTab, &m_rXFSet));
        Size aBitmapSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aBitmapSize);
    }
    if (m_xBtnHatch->get_visible())
    {
        m_pFillTabPage.disposeAndReset(SvxHatchTabPage::Create(aFillTab, &m_rXFSet));
        Size aHatchSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aHatchSize);
    }
    if (m_xBtnPattern->get_visible())
    {
        m_pFillTabPage.disposeAndReset(SvxPatternTabPage::Create(aFillTab, &m_rXFSet));
        Size aPatternSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aPatternSize);
    }
    m_pFillTabPage.disposeAndClear();

    m_xFillTab->set_size_request(aSize.Width(), aSize.Height());
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    disposeOnce();
}

void SvxAreaTabPage::dispose()
{
    m_pFillTabPage.disposeAndClear();
    SfxTabPage::dispose();
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
            SelectFillTypeHdl_Impl(*m_xBtnNone);
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            m_rXFSet.Put( static_cast<const XFillColorItem&>( rSet.Get( GetWhich( XATTR_FILLCOLOR ) ) ) );
            SelectFillTypeHdl_Impl(*m_xBtnColor);
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            m_rXFSet.Put( static_cast<const XFillGradientItem&>( rSet.Get( GetWhich( XATTR_FILLGRADIENT ) ) ) );
            SelectFillTypeHdl_Impl(*m_xBtnGradient);
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            m_rXFSet.Put( rSet.Get(XATTR_FILLHATCH) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLBACKGROUND) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLCOLOR) );
            SelectFillTypeHdl_Impl(*m_xBtnHatch);
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            XFillBitmapItem aItem(static_cast<const XFillBitmapItem&>( rSet.Get( GetWhich( XATTR_FILLBITMAP ) ) ));
            // pass full item set here, bitmap fill has many attributes (tiling, size, offset etc.)
            m_rXFSet.Put( rSet );
            if(!aItem.isPattern())
                SelectFillTypeHdl_Impl(*m_xBtnBitmap);
            else
                SelectFillTypeHdl_Impl(*m_xBtnPattern);
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

VclPtr<SfxTabPage> SvxAreaTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrs)
{
    auto xRet = VclPtr<SvxAreaTabPage>::Create(pParent, *rAttrs);
    xRet->SetOptimalSize(pParent.pController);
    return xRet;
}

namespace {

VclPtr<SfxTabPage> lcl_CreateFillStyleTabPage(sal_uInt16 nId, TabPageParent pParent, const SfxItemSet& rSet)
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

IMPL_LINK(SvxAreaTabPage, SelectFillTypeHdl_Impl, weld::ToggleButton&, rButton, void)
{
    SelectFillType(rButton);
}

void SvxAreaTabPage::SelectFillType(weld::ToggleButton& rButton, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_rXFSet.Set(*_pSet);

    sal_Int32 nPos = maBox.GetButtonPos(&rButton);
    if (nPos != -1 && (_pSet || nPos != maBox.GetCurrentButtonPos()))
    {
        maBox.SelectButton(&rButton);
        FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
        TabPageParent aFillTab(m_xFillTab.get(), GetDialogController());
        // TEMP
        if (!aFillTab.pController)
            aFillTab.pParent = GetParentDialog();
        m_pFillTabPage.disposeAndReset(lcl_CreateFillStyleTabPage(eFillType, aFillTab, m_rXFSet));
        if (m_pFillTabPage)
        {
            m_pFillTabPage->SetTabDialog(GetTabDialog());
            m_pFillTabPage->SetDialogController(GetDialogController());
        }
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
