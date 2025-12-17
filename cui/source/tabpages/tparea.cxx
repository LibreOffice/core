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
#include <unotools/pathoptions.hxx>

using namespace com::sun::star;

// static ----------------------------------------------------------------
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
    , m_xNotebook(m_xBuilder->weld_notebook(u"nbFillType"_ustr))
{
    if (!bSlideBackground)
    {
        if (m_xNotebook->get_page_index(TABID_USE_BACKGROUND) != -1)
            m_xNotebook->remove_page(TABID_USE_BACKGROUND);
    }

    maFillTypeMap = {
        {TABID_NONE, FillType::TRANSPARENT_FILL},
        {TABID_COLOR, FillType::SOLID_FILL},
        {TABID_GRADIENT, FillType::GRADIENT_FILL},
        {TABID_HATCH, FillType::HATCH_FILL},
        {TABID_BITMAP, FillType::BITMAP_FILL},
        {TABID_PATTERN, FillType::PATTERN_FILL},
        {TABID_USE_BACKGROUND, FillType::USE_BACKGROUND_FILL}
    };

    m_xNotebook->connect_enter_page(LINK(this, SvxAreaTabPage, SwitchPageHdl_Impl));

    SetExchangeSupport();
}

OUString SvxAreaTabPage::getPageId(FillType eFillType)
{
    for (const auto& entry : maFillTypeMap)
    {
        if (entry.second == eFillType)
            return entry.first;
    }
    return OUString();
}

void SvxAreaTabPage::SetOptimalSize()
{
    Size aSize;

    auto getPageSize = [&](const OUString& rPageId) -> Size {
        weld::Container* pPage = m_xNotebook->get_page(rPageId);
        if (!pPage)
            return Size();

        std::unique_ptr<SfxTabPage> xTempPage;
        Size aPageSize;

        auto it = maFillTypeMap.find(rPageId);
        if (it != maFillTypeMap.end())
        {
            xTempPage = CreateFillStyleTabPage(it->second);
            if (xTempPage)
            {
                aPageSize = pPage->get_preferred_size();
                xTempPage.reset();
            }
        }

        return aPageSize;
    };

    Size aColorSize = getPageSize(TABID_COLOR);
    if (aColorSize.Width() > 0)
        aSize = aColorSize;

    Size aGradientSize = getPageSize(TABID_GRADIENT);
    if (aGradientSize.Width() > 0)
        lclExtendSize(aSize, aGradientSize);

    Size aBitmapSize = getPageSize(TABID_BITMAP);
    if (aBitmapSize.Width() > 0)
        lclExtendSize(aSize, aBitmapSize);

    Size aHatchSize = getPageSize(TABID_HATCH);
    if (aHatchSize.Width() > 0)
        lclExtendSize(aSize, aHatchSize);

    Size aPatternSize = getPageSize(TABID_PATTERN);
    if (aPatternSize.Width() > 0)
        lclExtendSize(aSize, aPatternSize);

    if (aSize.Width() == 0 || aSize.Height() == 0)
    {
        aSize = Size(m_xNotebook->get_approximate_digit_width() * 50,
                     m_xNotebook->get_text_height() * 20);
    }

    aSize.extendBy(10, 10); // apply a bit of margin

    m_xNotebook->set_size_request(aSize.Width(), aSize.Height());
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    m_xFillTabPage.reset();
}

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if (m_bBtnClicked)
        return;

    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if( rSet.GetItemState( XATTR_FILLSTYLE ) != SfxItemState::INVALID )
    {
        const XFillStyleItem& aFillStyleItem( rSet.Get( GetWhich( XATTR_FILLSTYLE ) ) );
        eXFS = aFillStyleItem.GetValue();
        m_rXFSet.Put( aFillStyleItem );
    }

    FillType eTargetFillType;
    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            const XFillUseSlideBackgroundItem& aBckItem( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND));
            eTargetFillType = aBckItem.GetValue() ? FillType::USE_BACKGROUND_FILL : FillType::TRANSPARENT_FILL;
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            m_rXFSet.Put( rSet.Get( GetWhich( XATTR_FILLCOLOR ) ) );
            eTargetFillType = FillType::SOLID_FILL;
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            m_rXFSet.Put( rSet.Get( GetWhich( XATTR_FILLGRADIENT ) ) );
            m_rXFSet.Put(rSet.Get(GetWhich(XATTR_GRADIENTSTEPCOUNT)));
            eTargetFillType = FillType::GRADIENT_FILL;
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            m_rXFSet.Put( rSet.Get(XATTR_FILLHATCH) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND) );
            m_rXFSet.Put( rSet.Get(XATTR_FILLCOLOR) );
            eTargetFillType = FillType::HATCH_FILL;
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            const bool bPattern = rSet.Get(GetWhich(XATTR_FILLBITMAP)).isPattern();
            // pass full item set here, bitmap fill has many attributes (tiling, size, offset etc.)
            m_rXFSet.Put( rSet );
            eTargetFillType = bPattern ? FillType::PATTERN_FILL : FillType::BITMAP_FILL;
            break;
        }
    }

    SelectFillType(eTargetFillType);
}

template< typename TTabPage >
DeactivateRC SvxAreaTabPage::DeactivatePage_Impl( SfxItemSet* _pSet )
{
    return static_cast<TTabPage&>(*m_xFillTabPage).DeactivatePage(_pSet);
}

DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    OUString sCurrentPage = m_xNotebook->get_current_page_ident();
    auto it = maFillTypeMap.find(sCurrentPage);
    if (it == maFillTypeMap.end())
        return DeactivateRC::LeavePage;

    FillType eFillType = it->second;
    switch( eFillType )
    {
        case TRANSPARENT_FILL:
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
        case SOLID_FILL:
            return DeactivatePage_Impl<SvxColorTabPage>(_pSet);
        case GRADIENT_FILL:
            return DeactivatePage_Impl<SvxGradientTabPage>(_pSet);
        case HATCH_FILL:
            return DeactivatePage_Impl<SvxHatchTabPage>(_pSet);
        case BITMAP_FILL:
            return DeactivatePage_Impl<SvxBitmapTabPage>(_pSet);
        case PATTERN_FILL:
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
    OUString tabLabels[] = { u"None"_ustr,  u"Color"_ustr,   u"Gradient"_ustr, u"Hatch"_ustr,
                             u"Image"_ustr, u"Pattern"_ustr, u"Use Background"_ustr };

    for (const auto& label : tabLabels)
    {
        sAllStrings += label + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    OUString sCurrentPage = m_xNotebook->get_current_page_ident();
    auto it = maFillTypeMap.find(sCurrentPage);
    if (it == maFillTypeMap.end())
        return false;

    FillType eFillType = it->second;
    switch( eFillType )
    {
        case TRANSPARENT_FILL:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            rAttrs->Put( XFillUseSlideBackgroundItem( false ) );
            return true;
        }
        case SOLID_FILL:
        {
            return FillItemSet_Impl<SvxColorTabPage>( rAttrs );
        }
        case GRADIENT_FILL:
        {
            return FillItemSet_Impl<SvxGradientTabPage>( rAttrs );
        }
        case HATCH_FILL:
        {
            return FillItemSet_Impl<SvxHatchTabPage>( rAttrs );
        }
        case BITMAP_FILL:
        {
            return FillItemSet_Impl<SvxBitmapTabPage>( rAttrs );
        }
        case PATTERN_FILL:
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
    OUString sCurrentPage = m_xNotebook->get_current_page_ident();
    auto it = maFillTypeMap.find(sCurrentPage);
    if (it == maFillTypeMap.end())
        return;

    FillType eFillType = it->second;
    switch(eFillType)
    {
        case SOLID_FILL:
        {
            Reset_Impl<SvxColorTabPage>( rAttrs );
            break;
        }
        case GRADIENT_FILL:
        {
            Reset_Impl<SvxGradientTabPage>( rAttrs );
            break;
        }
        case HATCH_FILL:
        {
            Reset_Impl<SvxHatchTabPage>( rAttrs );
            break;
        }
        case BITMAP_FILL:
        {
            Reset_Impl<SvxBitmapTabPage>( rAttrs );
            break;
        }
        case PATTERN_FILL:
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
    xRet->SetOptimalSize();
    return xRet;
}

std::unique_ptr<SfxTabPage> SvxAreaTabPage::CreateWithSlideBackground(
    weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    auto xRet = std::make_unique<SvxAreaTabPage>(pPage, pController, *rAttrs, true);
    xRet->SetOptimalSize();
    return xRet;
}

void SvxAreaTabPage::SelectFillTypeByPage(FillType eFillType, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_rXFSet.Set(*_pSet);

    OUString sPageId = getPageId(eFillType);
    if (sPageId.isEmpty())
        return;

    weld::Container* pContainer = m_xNotebook->get_page(sPageId);
    if (!pContainer)
        return;

    m_xFillTabPage = CreateFillStyleTabPage(eFillType);
    if (m_xFillTabPage)
    {
        m_xFillTabPage->SetDialogController(GetDialogController());
        CreatePage(eFillType, *m_xFillTabPage);
    }
}

IMPL_LINK(SvxAreaTabPage, SwitchPageHdl_Impl, const OUString&, rPageIdent, void)
{
    m_bBtnClicked = true;
    auto it = maFillTypeMap.find(rPageIdent);
    if (it != maFillTypeMap.end())
    {
        FillType eFillType = it->second;
        SelectFillTypeByPage(eFillType);
        m_bBtnClicked = true;
    }
}

std::unique_ptr<SfxTabPage> SvxAreaTabPage::CreateFillStyleTabPage(FillType eFillType)
{
    OUString sPageId = getPageId(eFillType);

    weld::Container* pPage = m_xNotebook->get_page(sPageId);
    if (!pPage)
        return nullptr;

    CreateTabPage fnCreate = nullptr;
    switch(eFillType)
    {
        case TRANSPARENT_FILL:
            fnCreate = nullptr;
            break;
        case SOLID_FILL:
            fnCreate = &SvxColorTabPage::Create;
            break;
        case GRADIENT_FILL:
            fnCreate = &SvxGradientTabPage::Create;
            break;
        case HATCH_FILL:
            fnCreate = &SvxHatchTabPage::Create;
            break;
        case BITMAP_FILL:
            fnCreate = &SvxBitmapTabPage::Create;
            break;
        case PATTERN_FILL:
            fnCreate = &SvxPatternTabPage::Create;
            break;
        case USE_BACKGROUND_FILL:
            fnCreate = nullptr;
            break;
    }

    if (!fnCreate)
        return nullptr;

    std::unique_ptr<SfxTabPage> pTabPage = (*fnCreate)(pPage, GetDialogController(), &m_rXFSet);
    return pTabPage;
}

void SvxAreaTabPage::SelectFillType(FillType eFillType, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_rXFSet.Set(*_pSet);

    const OUString pageId = getPageId(eFillType);
    if(!pageId.isEmpty())
        m_xNotebook->set_current_page(pageId);

    m_xFillTabPage = CreateFillStyleTabPage(eFillType);
    if (m_xFillTabPage)
    {
        m_xFillTabPage->SetDialogController(GetDialogController());
        CreatePage(eFillType, *m_xFillTabPage);
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
    if(nId == SOLID_FILL )
    {
        auto& rColorTab = static_cast<SvxColorTabPage&>(rTab);
        rColorTab.SetColorList(m_pColorList);
        rColorTab.SetColorChgd(m_pnColorListState);
        rColorTab.Construct();
        rColorTab.ActivatePage(m_rXFSet);
        rColorTab.Reset(&m_rXFSet);
        rColorTab.set_visible(true);
    }
    else if(nId == GRADIENT_FILL)
    {
        auto& rGradientTab = static_cast<SvxGradientTabPage&>(rTab);
        rGradientTab.SetColorList(m_pColorList);
        rGradientTab.SetGradientList(m_pGradientList);
        rGradientTab.SetColorChgd(m_pnColorListState);
        rGradientTab.Construct();
        rGradientTab.ActivatePage(m_rXFSet);
        rGradientTab.Reset(&m_rXFSet);
        rGradientTab.set_visible(true);
    }
    else if(nId == HATCH_FILL)
    {
        auto& rHatchTab = static_cast<SvxHatchTabPage&>(rTab);
        rHatchTab.SetColorList(m_pColorList);
        rHatchTab.SetHatchingList(m_pHatchingList);
        rHatchTab.SetColorChgd(m_pnColorListState);
        rHatchTab.Construct();
        rHatchTab.ActivatePage(m_rXFSet);
        rHatchTab.Reset(&m_rXFSet);
        rHatchTab.set_visible(true);
    }
    else if(nId == BITMAP_FILL)
    {
        auto& rBitmapTab = static_cast<SvxBitmapTabPage&>(rTab);
        rBitmapTab.SetBitmapList(m_pBitmapList);
        rBitmapTab.Construct();
        rBitmapTab.ActivatePage(m_rXFSet);
        rBitmapTab.Reset(&m_rXFSet);
        rBitmapTab.set_visible(true);
    }
    else if(nId == PATTERN_FILL)
    {
        auto& rPatternTab = static_cast<SvxPatternTabPage&>(rTab);
        rPatternTab.SetColorList(m_pColorList);
        rPatternTab.SetPatternList(m_pPatternList);
        rPatternTab.SetColorChgd(m_pnColorListState);
        rPatternTab.Construct();
        rPatternTab.ActivatePage(m_rXFSet);
        rPatternTab.Reset(&m_rXFSet);
        rPatternTab.set_visible(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
