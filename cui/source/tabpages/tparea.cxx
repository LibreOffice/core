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
#include <svx/drawitem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xgrscit.hxx>
#include <cuitabarea.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/tabdlg.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/weld/Builder.hxx>

using namespace com::sun::star;

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

OUString AreaTabHelper::GetPalettePath()
{
    const OUString aPalettePath = SvtPathOptions().GetPalettePath();
    OUString aPath;
    sal_Int32 nIndex = 0;
    do
    {
        aPath = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    return aPath;
}

/*************************************************************************
|*
|*  Dialog to modify fill-attributes
|*
\************************************************************************/

SvxAreaTabPage::SvxAreaTabPage(weld::Container* pPage, weld::DialogController* pController,
                               const SfxItemSet& rInAttrs, bool bSlideBackground)
    : SfxTabPage(pPage, pController, u"cui/ui/areatabpage.ui"_ustr, u"AreaTabPage"_ustr, &rInAttrs)
    // local fixed not o be changed values for local pointers
    , maFixed_ChangeType(ChangeType::NONE)
    // init with pointers to fixed ChangeType
    , m_pnColorListState(&maFixed_ChangeType)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_xFillTab(m_xBuilder->weld_container(u"fillstylebox"_ustr))
    , m_xBtnNone(m_xBuilder->weld_toggle_button(u"btnnone"_ustr))
    , m_xBtnColor(m_xBuilder->weld_toggle_button(u"btncolor"_ustr))
    , m_xBtnGradient(m_xBuilder->weld_toggle_button(u"btngradient"_ustr))
    , m_xBtnHatch(m_xBuilder->weld_toggle_button(u"btnhatch"_ustr))
    , m_xBtnBitmap(m_xBuilder->weld_toggle_button(u"btnbitmap"_ustr))
    , m_xBtnPattern(m_xBuilder->weld_toggle_button(u"btnpattern"_ustr))
    , m_xBtnUseBackground(m_xBuilder->weld_toggle_button(u"btnusebackground"_ustr))
{
    maBox.AddButton(m_xBtnNone.get(), FillType::TRANSPARENT);
    maBox.AddButton(m_xBtnColor.get(), FillType::SOLID);
    maBox.AddButton(m_xBtnGradient.get(), FillType::GRADIENT);
    maBox.AddButton(m_xBtnHatch.get(), FillType::HATCH);
    maBox.AddButton(m_xBtnBitmap.get(), FillType::BITMAP);
    maBox.AddButton(m_xBtnPattern.get(), FillType::PATTERN);

    Link<weld::Toggleable&, void> aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_xBtnNone->connect_toggled(aLink);
    m_xBtnColor->connect_toggled(aLink);
    m_xBtnGradient->connect_toggled(aLink);
    m_xBtnHatch->connect_toggled(aLink);
    m_xBtnBitmap->connect_toggled(aLink);
    m_xBtnPattern->connect_toggled(aLink);
    if (bSlideBackground)
    {
        maBox.AddButton(m_xBtnUseBackground.get(), FillType::USE_BACKGROUND_FILL);
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
        const XFillStyleItem& aFillStyleItem( rSet.Get( GetWhich( XATTR_FILLSTYLE ) ) );
        eXFS = aFillStyleItem.GetValue();
        m_rXFSet.Put( aFillStyleItem );
    }

    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            const XFillUseSlideBackgroundItem& aBckItem( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND));
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
    const FillType eFillType = maBox.GetCurrentFillType();
    switch( eFillType )
    {
        case FillType::TRANSPARENT:
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
        case FillType::SOLID:
            return DeactivatePage_Impl<SvxColorTabPage>(_pSet);
        case FillType::GRADIENT:
            return DeactivatePage_Impl<SvxGradientTabPage>(_pSet);
        case FillType::HATCH:
            return DeactivatePage_Impl<SvxHatchTabPage>(_pSet);
        case FillType::BITMAP:
            return DeactivatePage_Impl<SvxBitmapTabPage&>(_pSet);
        case FillType::PATTERN:
            return DeactivatePage_Impl<SvxPatternTabPage>(_pSet);
        case FillType::USE_BACKGROUND_FILL:
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
    OUStringBuffer sAllStrings;
    OUString toggleButton[] = { u"btnnone"_ustr,    u"btncolor"_ustr, u"btngradient"_ustr,     u"btnbitmap"_ustr,
                                u"btnpattern"_ustr, u"btnhatch"_ustr, u"btnusebackground"_ustr };

    for (const auto& toggle : toggleButton)
    {
        if (const auto pString = m_xBuilder->weld_toggle_button(toggle))
            sAllStrings.append(pString->get_label() + " ");
    }

    return sAllStrings.toString().replaceAll("_", "");
}

bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    const FillType eFillType = maBox.GetCurrentFillType();
    switch( eFillType )
    {
        case FillType::TRANSPARENT:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            rAttrs->Put( XFillUseSlideBackgroundItem( false ) );
            return true;
        }
        case FillType::SOLID:
        {
            return FillItemSet_Impl<SvxColorTabPage>( rAttrs );
        }
        case FillType::GRADIENT:
        {
            return FillItemSet_Impl<SvxGradientTabPage>( rAttrs );
        }
        case FillType::HATCH:
        {
            return FillItemSet_Impl<SvxHatchTabPage>( rAttrs );
        }
        case FillType::BITMAP:
        {
            return FillItemSet_Impl<SvxBitmapTabPage>( rAttrs );
        }
        case FillType::PATTERN:
        {
            return FillItemSet_Impl<SvxPatternTabPage>( rAttrs );
        }
        case FillType::USE_BACKGROUND_FILL:
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
    const FillType eFillType = maBox.GetCurrentFillType();
    switch(eFillType)
    {
        case FillType::SOLID:
        {
            Reset_Impl<SvxColorTabPage>( rAttrs );
            break;
        }
        case FillType::GRADIENT:
        {
            Reset_Impl<SvxGradientTabPage>( rAttrs );
            break;
        }
        case FillType::HATCH:
        {
            Reset_Impl<SvxHatchTabPage>( rAttrs );
            break;
        }
        case FillType::BITMAP:
        {
            Reset_Impl<SvxBitmapTabPage>( rAttrs );
            break;
        }
        case FillType::PATTERN:
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

std::unique_ptr<SfxTabPage> SvxAreaTabPage::CreateFillStyleTabPage(FillType eFillType)
{
    SfxOkDialogController* pController = GetDialogController();
    CreateTabPage fnCreate = nullptr;
    switch (eFillType)
    {
        case FillType::TRANSPARENT: fnCreate = nullptr; break;
        case FillType::SOLID: fnCreate = &SvxColorTabPage::Create; break;
        case FillType::GRADIENT: fnCreate = &SvxGradientTabPage::Create; break;
        case FillType::HATCH: fnCreate = &SvxHatchTabPage::Create; break;
        case FillType::BITMAP: fnCreate = &SvxBitmapTabPage::Create; break;
        case FillType::PATTERN: fnCreate = &SvxPatternTabPage::Create; break;
        case FillType::USE_BACKGROUND_FILL: fnCreate = nullptr; break;
        default: break;
    }

    if (!fnCreate)
        return nullptr;

    std::unique_ptr<SfxTabPage> pTabPage = (*fnCreate)(m_xFillTab.get(), pController, &m_rXFSet);
    pTabPage->SetDialogController(pController);
    return pTabPage;
}

IMPL_LINK(SvxAreaTabPage, SelectFillTypeHdl_Impl, weld::Toggleable&, rButton, void)
{
    if (rButton.get_active())
    {
        SelectFillType(rButton);
        m_bBtnClicked = true;
    }
    else if (maBox.GetCurrentFillType() == maBox.GetFillType(rButton))
    {
        // tdf#124549 - If the button is already active do not toggle it back.
        rButton.set_active(true);
    }
}

void SvxAreaTabPage::SelectFillType(weld::Toggleable& rButton, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_rXFSet.Set(*_pSet);

    if (_pSet || maBox.GetFillType(rButton) != maBox.GetCurrentFillType())
    {
        maBox.SelectButton(rButton);
        FillType eFillType = maBox.GetCurrentFillType();
        m_xFillTabPage = CreatePage(eFillType);
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

std::unique_ptr<SfxTabPage> SvxAreaTabPage::CreatePage(FillType eFillType)
{
    std::unique_ptr<SfxTabPage> pTabPage = CreateFillStyleTabPage(eFillType);
    if (!pTabPage)
        return nullptr;

    if (eFillType == FillType::SOLID)
    {
        auto& rColorTab = static_cast<SvxColorTabPage&>(*pTabPage);
        rColorTab.SetColorList(m_pColorList);
        rColorTab.SetColorChgd(m_pnColorListState);
        rColorTab.Construct();
        rColorTab.ActivatePage(m_rXFSet);
        rColorTab.Reset(&m_rXFSet);
        rColorTab.set_visible(true);
    }
    else if (eFillType == FillType::GRADIENT)
    {
        auto& rGradientTab = static_cast<SvxGradientTabPage&>(*pTabPage);
        rGradientTab.SetColorList(m_pColorList);
        rGradientTab.SetGradientList(m_pGradientList);
        rGradientTab.SetColorChgd(m_pnColorListState);
        rGradientTab.Construct();
        rGradientTab.ActivatePage(m_rXFSet);
        rGradientTab.Reset(&m_rXFSet);
        rGradientTab.set_visible(true);
    }
    else if (eFillType == FillType::HATCH)
    {
        auto& rHatchTab = static_cast<SvxHatchTabPage&>(*pTabPage);
        rHatchTab.SetColorList(m_pColorList);
        rHatchTab.SetHatchingList(m_pHatchingList);
        rHatchTab.SetColorChgd(m_pnColorListState);
        rHatchTab.Construct();
        rHatchTab.ActivatePage(m_rXFSet);
        rHatchTab.Reset(&m_rXFSet);
        rHatchTab.set_visible(true);
    }
    else if (eFillType == FillType::BITMAP)
    {
        auto& rBitmapTab = static_cast<SvxBitmapTabPage&>(*pTabPage);
        rBitmapTab.SetBitmapList(m_pBitmapList);
        rBitmapTab.Construct();
        rBitmapTab.ActivatePage(m_rXFSet);
        rBitmapTab.Reset(&m_rXFSet);
        rBitmapTab.set_visible(true);
    }
    else if (eFillType == FillType::PATTERN)
    {
        auto& rPatternTab = static_cast<SvxPatternTabPage&>(*pTabPage);
        rPatternTab.SetColorList(m_pColorList);
        rPatternTab.SetPatternList(m_pPatternList);
        rPatternTab.SetColorChgd(m_pnColorListState);
        rPatternTab.Construct();
        rPatternTab.ActivatePage(m_rXFSet);
        rPatternTab.Reset(&m_rXFSet);
        rPatternTab.set_visible(true);
    }

    return pTabPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
