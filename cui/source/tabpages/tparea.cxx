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

#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <svx/xflbckit.hxx>
#include <svx/drawitem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xgrscit.hxx>
#include <cuitabarea.hxx>
#include <sfx2/tabdlg.hxx>

using namespace com::sun::star;

// static ----------------------------------------------------------------

namespace {

enum FillType
{
    TRANSPARENT,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN,
    USE_BACKGROUND_FILL
};

}

const WhichRangesContainer SvxAreaTabPage::pAreaRanges(
    svl::Items<
    XATTR_GRADIENTSTEPCOUNT, XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE, SID_ATTR_FILL_BITMAP>);

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

SvxAreaTabPage::SvxAreaTabPage(weld::Container* pPage, weld::DialogController* pController,
                               const SfxItemSet& rInAttrs, bool bSlideBackground)
    : SfxTabPage(pPage, pController, "cui/ui/areatabpage.ui", "AreaTabPage", &rInAttrs)
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
    , m_xBtnUseBackground(m_xBuilder->weld_toggle_button("btnusebackground"))
{
    maBox.AddButton(m_xBtnNone.get());
    maBox.AddButton(m_xBtnColor.get());
    maBox.AddButton(m_xBtnGradient.get());
    maBox.AddButton(m_xBtnHatch.get());
    maBox.AddButton(m_xBtnBitmap.get());
    maBox.AddButton(m_xBtnPattern.get());

    Link<weld::Toggleable&, void> aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_xBtnNone->connect_toggled(aLink);
    m_xBtnColor->connect_toggled(aLink);
    m_xBtnGradient->connect_toggled(aLink);
    m_xBtnHatch->connect_toggled(aLink);
    m_xBtnBitmap->connect_toggled(aLink);
    m_xBtnPattern->connect_toggled(aLink);
    if (bSlideBackground)
    {
        maBox.AddButton(m_xBtnUseBackground.get());
        m_xBtnUseBackground->connect_toggled(aLink);
    }
    else
        m_xBtnUseBackground->hide();

    SetExchangeSupport();
}

void SvxAreaTabPage::SetOptimalSize(weld::DialogController* pController)
{
    m_xFillTab->set_size_request(-1, -1);

    // Calculate optimal size of all pages...
    m_xFillTabPage = SvxColorTabPage::Create(m_xFillTab.get(), pController, &m_rXFSet);
    Size aSize(m_xFillTab->get_preferred_size());

    if (m_xBtnGradient->get_visible())
    {
        m_xFillTabPage = SvxGradientTabPage::Create(m_xFillTab.get(), pController, &m_rXFSet);
        Size aGradientSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aGradientSize);
    }
    if (m_xBtnBitmap->get_visible())
    {
        m_xFillTabPage = SvxBitmapTabPage::Create(m_xFillTab.get(), pController, &m_rXFSet);
        Size aBitmapSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aBitmapSize);
    }
    if (m_xBtnHatch->get_visible())
    {
        m_xFillTabPage = SvxHatchTabPage::Create(m_xFillTab.get(), pController, &m_rXFSet);
        Size aHatchSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aHatchSize);
    }
    if (m_xBtnPattern->get_visible())
    {
        m_xFillTabPage = SvxPatternTabPage::Create(m_xFillTab.get(), pController, &m_rXFSet);
        Size aPatternSize = m_xFillTab->get_preferred_size();
        lclExtendSize(aSize, aPatternSize);
    }
    m_xFillTabPage.reset();

    aSize.extendBy(10, 10); // apply a bit of margin

    m_xFillTab->set_size_request(aSize.Width(), aSize.Height());
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    m_xFillTabPage.reset();
}

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rSet.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::INVALID )
    {
        XFillStyleItem aFillStyleItem( rSet.Get( GetWhich( XATTR_FILLSTYLE ) ) );
        eXFS = aFillStyleItem.GetValue();
        m_rXFSet.Put( aFillStyleItem );
    }

    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            XFillUseSlideBackgroundItem aBckItem( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND));
            if (aBckItem.GetValue())
                SelectFillType(*m_xBtnUseBackground);
            else
                SelectFillType(*m_xBtnNone);
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            m_rXFSet.Put( rSet.Get( GetWhich( XATTR_FILLCOLOR ) ) );
            SelectFillType(*m_xBtnColor);
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            m_rXFSet.Put( rSet.Get( GetWhich( XATTR_FILLGRADIENT ) ) );
            m_rXFSet.Put(rSet.Get(GetWhich(XATTR_GRADIENTSTEPCOUNT)));
            SelectFillType(*m_xBtnGradient);
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            m_rXFSet.Put( rSet.Get(XATTR_FILLHATCH) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLCOLOR) );
            SelectFillType(*m_xBtnHatch);
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            const bool bPattern = rSet.Get(GetWhich(XATTR_FILLBITMAP)).isPattern();
            // pass full item set here, bitmap fill has many attributes (tiling, size, offset etc.)
            m_rXFSet.Put( rSet );
            if (!bPattern)
                SelectFillType(*m_xBtnBitmap);
            else
                SelectFillType(*m_xBtnPattern);
            break;
        }
    }
}

template< typename TTabPage >
DeactivateRC SvxAreaTabPage::DeactivatePage_Impl( SfxItemSet* _pSet )
{
    return static_cast<TTabPage&>(*m_xFillTabPage).DeactivatePage(_pSet);
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
            if ( m_bBtnClicked )
            {
                XFillStyleItem aStyleItem( drawing::FillStyle_NONE );
                _pSet->Put( aStyleItem );
                XFillUseSlideBackgroundItem aFillBgItem( false );
                _pSet->Put( aFillBgItem );
            }
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
        case USE_BACKGROUND_FILL:
        {
            if ( m_bBtnClicked )
            {
                XFillStyleItem aStyleItem( drawing::FillStyle_NONE );
                _pSet->Put( aStyleItem );
                XFillUseSlideBackgroundItem aFillBgItem( true );
                _pSet->Put( aFillBgItem );
            }
            break;
        }
        default:
            break;
    }
    return DeactivateRC::LeavePage;
}

template< typename TTabPage >
bool SvxAreaTabPage::FillItemSet_Impl( SfxItemSet* rAttrs)
{
    return static_cast<TTabPage&>( *m_xFillTabPage ).FillItemSet( rAttrs );
}

OUString SvxAreaTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString toggleButton[] = { "btnnone",    "btncolor", "btngradient",     "btnbitmap",
                                "btnpattern", "btnhatch", "btnusebackground" };

    for (const auto& toggle : toggleButton)
    {
        if (const auto& pString = m_xBuilder->weld_toggle_button(toggle))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
    switch( eFillType )
    {
        case TRANSPARENT:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            rAttrs->Put( XFillUseSlideBackgroundItem( false ) );
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
        case USE_BACKGROUND_FILL:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            rAttrs->Put( XFillUseSlideBackgroundItem( true ) );
            return true;
        }
        default:
            return false;
    }
}

template< typename TTabPage >
void SvxAreaTabPage::Reset_Impl( const SfxItemSet* rAttrs )
{
    static_cast<TTabPage&>( *m_xFillTabPage ).Reset( rAttrs );
}

void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
    m_bBtnClicked = false;
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

std::unique_ptr<SfxTabPage> SvxAreaTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    auto xRet = std::make_unique<SvxAreaTabPage>(pPage, pController, *rAttrs);
    xRet->SetOptimalSize(pController);
    return xRet;
}

std::unique_ptr<SfxTabPage> SvxAreaTabPage::CreateWithSlideBackground(
    weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    auto xRet = std::make_unique<SvxAreaTabPage>(pPage, pController, *rAttrs, true);
    xRet->SetOptimalSize(pController);
    return xRet;
}

namespace {

std::unique_ptr<SfxTabPage> lcl_CreateFillStyleTabPage(sal_uInt16 nId, weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
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
        case USE_BACKGROUND_FILL: fnCreate = nullptr; break;
    }
    return fnCreate ? (*fnCreate)( pPage, pController, &rSet ) : nullptr;
}

}

IMPL_LINK(SvxAreaTabPage, SelectFillTypeHdl_Impl, weld::Toggleable&, rButton, void)
{
    //tdf#124549 - If the button is already active do not toggle it back.
    if(!rButton.get_active())
        rButton.set_active(true);

    SelectFillType(rButton);
    m_bBtnClicked = true;
}

void SvxAreaTabPage::SelectFillType(weld::Toggleable& rButton, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_rXFSet.Set(*_pSet);

    sal_Int32 nPos = maBox.GetButtonPos(&rButton);
    if (nPos != -1 && (_pSet || nPos != maBox.GetCurrentButtonPos()))
    {
        maBox.SelectButton(&rButton);
        FillType eFillType = static_cast<FillType>(maBox.GetCurrentButtonPos());
        m_xFillTabPage = lcl_CreateFillStyleTabPage(eFillType, m_xFillTab.get(), GetDialogController(), m_rXFSet);
        if (m_xFillTabPage)
        {
            m_xFillTabPage->SetDialogController(GetDialogController());
            CreatePage(eFillType, *m_xFillTabPage);
        }
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

void SvxAreaTabPage::CreatePage(sal_Int32 nId, SfxTabPage& rTab)
{
    if(nId == SOLID )
    {
        auto& rColorTab = static_cast<SvxColorTabPage&>(rTab);
        rColorTab.SetColorList(m_pColorList);
        rColorTab.SetColorChgd(m_pnColorListState);
        rColorTab.Construct();
        rColorTab.ActivatePage(m_rXFSet);
        rColorTab.Reset(&m_rXFSet);
        rColorTab.set_visible(true);
    }
    else if(nId == GRADIENT)
    {
        auto& rGradientTab = static_cast<SvxGradientTabPage&>(rTab);
        rGradientTab.SetColorList(m_pColorList);
        rGradientTab.SetGradientList(m_pGradientList);
        rGradientTab.SetGrdChgd(m_pnGradientListState);
        rGradientTab.SetColorChgd(m_pnColorListState);
        rGradientTab.Construct();
        rGradientTab.ActivatePage(m_rXFSet);
        rGradientTab.Reset(&m_rXFSet);
        rGradientTab.set_visible(true);
    }
    else if(nId == HATCH)
    {
        auto& rHatchTab = static_cast<SvxHatchTabPage&>(rTab);
        rHatchTab.SetColorList(m_pColorList);
        rHatchTab.SetHatchingList(m_pHatchingList);
        rHatchTab.SetHtchChgd(m_pnHatchingListState);
        rHatchTab.SetColorChgd(m_pnColorListState);
        rHatchTab.Construct();
        rHatchTab.ActivatePage(m_rXFSet);
        rHatchTab.Reset(&m_rXFSet);
        rHatchTab.set_visible(true);
    }
    else if(nId == BITMAP)
    {
        auto& rBitmapTab = static_cast<SvxBitmapTabPage&>(rTab);
        rBitmapTab.SetBitmapList(m_pBitmapList);
        rBitmapTab.SetBmpChgd(m_pnBitmapListState);
        rBitmapTab.Construct();
        rBitmapTab.ActivatePage(m_rXFSet);
        rBitmapTab.Reset(&m_rXFSet);
        rBitmapTab.set_visible(true);
    }
    else if(nId == PATTERN)
    {
        auto& rPatternTab = static_cast<SvxPatternTabPage&>(rTab);
        rPatternTab.SetColorList(m_pColorList);
        rPatternTab.SetPatternList(m_pPatternList);
        rPatternTab.SetPtrnChgd(m_pnPatternListState);
        rPatternTab.SetColorChgd(m_pnColorListState);
        rPatternTab.Construct();
        rPatternTab.ActivatePage(m_rXFSet);
        rPatternTab.Reset(&m_rXFSet);
        rPatternTab.set_visible(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
