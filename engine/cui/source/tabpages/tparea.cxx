/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <colorfillpage.hxx>
#include <cuitabarea.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
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

bool lclIsEffect(FillType eFillType)
{
    return eFillType == FillType::GRADIENT_FILL || eFillType == FillType::HATCH_FILL
           || eFillType == FillType::BITMAP_FILL || eFillType == FillType::PATTERN_FILL;
}

// The preview caption of a fill. A fill without a name is named by its kind only.
OUString lclCaption(TranslateId aId, std::u16string_view aName)
{
    OUString sCaption = CuiResId(aId);
    if (!aName.empty())
        return sCaption.replaceFirst("%1", aName);
    sCaption = comphelper::string::stripEnd(sCaption.replaceFirst("%1", u""), ' ');
    return comphelper::string::stripEnd(comphelper::string::stripEnd(sCaption, ':'), ' ');
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
    , m_aFillAttributeSet(rInAttrs.getPool(), WhichRangesContainer(XATTR_FILL_FIRST, XATTR_FILL_LAST))
    , m_aEffects({ FillType::GRADIENT_FILL, FillType::HATCH_FILL, FillType::BITMAP_FILL })
    , m_xNotebook(m_xBuilder->weld_notebook(u"nbFillType"_ustr))
{

    if (!bSlideBackground)
    {
        if (m_xNotebook->get_page_index(TABID_USE_BACKGROUND) != -1)
            m_xNotebook->remove_page(TABID_USE_BACKGROUND);
    }
    // The tabs are shown only when there is more than one fill type to select.
    m_xNotebook->set_show_tabs(m_xNotebook->get_n_pages() > 1);

    maFillTypeMap = {
        {TABID_COLOR, FillType::SOLID_FILL},
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
    // Pattern Style sits in the Color page, so it goes first.
    m_xPatternStyle.reset();
    m_aFillTabPages.clear();
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
        m_aFillAttributeSet.Put( aFillStyleItem );
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
            m_aFillAttributeSet.Put( rSet.Get( GetWhich( XATTR_FILLCOLOR ) ) );
            eTargetFillType = FillType::SOLID_FILL;
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            m_aFillAttributeSet.Put( rSet.Get( GetWhich( XATTR_FILLGRADIENT ) ) );
            m_aFillAttributeSet.Put(rSet.Get(GetWhich(XATTR_GRADIENTSTEPCOUNT)));
            eTargetFillType = FillType::GRADIENT_FILL;
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            m_aFillAttributeSet.Put( rSet.Get(XATTR_FILLHATCH) );
            m_aFillAttributeSet.Put( rSet.Get(XATTR_FILLUSESLIDEBACKGROUND) );
            m_aFillAttributeSet.Put( rSet.Get(XATTR_FILLCOLOR) );
            eTargetFillType = FillType::HATCH_FILL;
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            const bool bPattern = rSet.Get(GetWhich(XATTR_FILLBITMAP)).isPattern();
            // pass full item set here, bitmap fill has many attributes (tiling, size, offset etc.)
            m_aFillAttributeSet.Put( rSet );
            eTargetFillType = bPattern ? FillType::PATTERN_FILL : FillType::BITMAP_FILL;
            break;
        }
    }

    SelectFillType(eTargetFillType);
}

template< typename TTabPage >
DeactivateRC SvxAreaTabPage::DeactivatePage_Impl( SfxItemSet* _pSet )
{
    return static_cast<TTabPage&>(*getCurrentFillTabPage()).DeactivatePage(_pSet);
}

DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if (m_oEffect)
    {
        if (m_oEffectSet && _pSet)
            _pSet->Put(*m_oEffectSet);
        return DeactivateRC::LeavePage;
    }

    switch (getCurrentFillType())
    {
        case FillType::TRANSPARENT_FILL:
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
        case FillType::SOLID_FILL:
            return DeactivatePage_Impl<SvxColorFillTabPage>(_pSet);
        case FillType::PATTERN_FILL:
        {
            if (_pSet)
                FillPatternItemSet(*_pSet);
            break;
        }
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
    return static_cast<TTabPage&>( *getCurrentFillTabPage() ).FillItemSet( rAttrs );
}

bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    // An effect the document already had is left as it is.
    if (m_oEffect)
    {
        if (!m_oEffectSet)
            return false;
        rAttrs->Put(*m_oEffectSet);
        return true;
    }

    switch (getCurrentFillType())
    {
        case FillType::TRANSPARENT_FILL:
        {
            rAttrs->Put( XFillStyleItem( drawing::FillStyle_NONE ) );
            rAttrs->Put( XFillUseSlideBackgroundItem( false ) );
            return true;
        }
        case FillType::SOLID_FILL:
        {
            return FillItemSet_Impl<SvxColorFillTabPage>( rAttrs );
        }
        case FillType::PATTERN_FILL:
        {
            FillPatternItemSet(*rAttrs);
            return true;
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

FillType SvxAreaTabPage::getCurrentFillType()
{
    auto aFoundType = maFillTypeMap.find(m_xNotebook->get_current_page_ident());
    if (aFoundType == maFillTypeMap.end())
        return FillType::TRANSPARENT_FILL;
    // The Color tab holds the None and the pattern fills too.
    if (aFoundType->second == FillType::SOLID_FILL && m_bNoFill)
        return FillType::TRANSPARENT_FILL;
    if (aFoundType->second == FillType::SOLID_FILL && m_xPatternStyle
        && m_xPatternStyle->HasPattern())
        return FillType::PATTERN_FILL;
    return aFoundType->second;
}

void SvxAreaTabPage::FillPatternItemSet(SfxItemSet& rSet)
{
    if (m_xPatternStyle)
        m_xPatternStyle->FillItemSet(rSet);
}

template< typename TTabPage >
void SvxAreaTabPage::Reset_Impl( const SfxItemSet* rAttrs )
{
    static_cast<TTabPage&>( *getCurrentFillTabPage() ).Reset( rAttrs );
}

void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
    m_bBtnClicked = false;
    m_oEffectSet.reset();

    // Show the fill that the item set holds, before the page is first shown.
    if (rAttrs)
        ActivatePage(*rAttrs);

    OUString sCurrentPage = m_xNotebook->get_current_page_ident();
    auto it = maFillTypeMap.find(sCurrentPage);
    if (it == maFillTypeMap.end())
        return;

    FillType eFillType = it->second;
    switch(eFillType)
    {
        case FillType::SOLID_FILL:
        {
            Reset_Impl<SvxColorFillTabPage>( rAttrs );
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

void SvxAreaTabPage::createFillTabPages()
{
    for (const auto& rEntry : maFillTypeMap)
    {
        std::unique_ptr<SfxTabPage> xPage = CreateFillStyleTabPage(rEntry.second);
        if (!xPage)
            continue;
        xPage->SetDialogController(GetDialogController());
        CreatePage(rEntry.second, *xPage);
        m_aFillTabPages[rEntry.second] = std::move(xPage);
    }
}

SfxTabPage* SvxAreaTabPage::getCurrentFillTabPage()
{
    auto aFoundType = maFillTypeMap.find(m_xNotebook->get_current_page_ident());
    if (aFoundType == maFillTypeMap.end())
        return nullptr;
    auto aFoundPage = m_aFillTabPages.find(aFoundType->second);
    return aFoundPage == m_aFillTabPages.end() ? nullptr : aFoundPage->second.get();
}

IMPL_LINK_NOARG(SvxAreaTabPage, SwitchPageHdl_Impl, const OUString&, void)
{
    m_bBtnClicked = true;
    ClearEffect();
    // every fill page is built already, so the notebook only shows another one
    if (m_aFillTabPages.empty())
        createFillTabPages();
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
        case FillType::TRANSPARENT_FILL:
            fnCreate = nullptr;
            break;
        case FillType::SOLID_FILL:
            fnCreate = &SvxColorFillTabPage::Create;
            break;
        default:
            fnCreate = nullptr;
            break;
    }

    if (!fnCreate)
        return nullptr;

    std::unique_ptr<SfxTabPage> pTabPage = (*fnCreate)(pPage, GetDialogController(), &m_aFillAttributeSet);
    return pTabPage;
}

void SvxAreaTabPage::SelectFillType(FillType eFillType, const SfxItemSet* _pSet)
{
    if (_pSet)
        m_aFillAttributeSet.Set(*_pSet);

    // Build all fill pages once; later calls only load the new attributes into them.
    const bool bCreatePages = m_aFillTabPages.empty();
    if (bCreatePages)
        createFillTabPages();

    // A pattern is selected in Pattern Style, so it is no effect.
    bool bPattern = false;
    if (m_xPatternStyle)
    {
        if (eFillType == FillType::PATTERN_FILL)
            bPattern = m_xPatternStyle->Load(m_aFillAttributeSet);
        else
            m_xPatternStyle->Clear();
    }

    // The Color tab stands for None, the pattern and the effects too.
    const bool bColorTab = lclIsEffect(eFillType) || eFillType == FillType::TRANSPARENT_FILL;
    const OUString pageId = getPageId(bColorTab ? FillType::SOLID_FILL : eFillType);
    if(!pageId.isEmpty())
        m_xNotebook->set_current_page(pageId);
    m_oEffectSet.reset();
    if (lclIsEffect(eFillType) && !bPattern)
        m_oEffect = eFillType;
    else
        m_oEffect.reset();
    m_bNoFill = eFillType == FillType::TRANSPARENT_FILL;

    if (!bCreatePages)
    {
        for (auto& rEntry : m_aFillTabPages)
        {
            rEntry.second->ActivatePage(m_aFillAttributeSet);
            rEntry.second->Reset(&m_aFillAttributeSet);
        }
    }
    if (SvxColorFillTabPage* pColorPage = getColorTabPage())
    {
        pColorPage->SetNoFill(m_bNoFill);
        pColorPage->UpdatePreview();
    }
}

void SvxAreaTabPage::CreatePatternStyle(SvxColorFillTabPage& rColorPage)
{
    // Pattern Style sits in the panel right of the palette.
    m_xPatternStyle = std::make_unique<SvxPatternStyleBox>(&rColorPage.GetPatternBox(), *this);
    m_xPatternStyle->SetChangedHdl(LINK(this, SvxAreaTabPage, PatternChangedHdl_Impl));
    m_xPatternStyle->SetLists(m_pColorList, m_pPatternList, m_pnColorListState);
}

void SvxAreaTabPage::HidePatternStyle() { m_bOfferPattern = false; }

SvxColorFillTabPage* SvxAreaTabPage::getColorTabPage()
{
    auto aFoundPage = m_aFillTabPages.find(FillType::SOLID_FILL);
    if (aFoundPage == m_aFillTabPages.end())
        return nullptr;
    return static_cast<SvxColorFillTabPage*>(aFoundPage->second.get());
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
    if (m_xPatternStyle)
        m_xPatternStyle->SetLists(m_pColorList, m_pPatternList, m_pnColorListState);
}

void SvxAreaTabPage::CreatePage(FillType nId, SfxTabPage& rTab)
{
    if(nId == FillType::SOLID_FILL )
    {
        auto& rColorTab = static_cast<SvxColorFillTabPage&>(rTab);
        rColorTab.SetColorList(m_pColorList);
        rColorTab.SetColorChgd(m_pnColorListState);
        rColorTab.SetColorChangedHdl(LINK(this, SvxAreaTabPage, ColorChangedHdl_Impl));
        rColorTab.SetNoneSelectedHdl(LINK(this, SvxAreaTabPage, NoneSelectedHdl_Impl));
        rColorTab.SetUpdatePreviewHdl(LINK(this, SvxAreaTabPage, UpdatePreviewHdl_Impl));
        weld::Button& rMoreEffects = rColorTab.GetMoreEffectsButton();
        rMoreEffects.connect_clicked(LINK(this, SvxAreaTabPage, MoreEffectsHdl_Impl));
        rMoreEffects.set_visible(!m_aEffects.empty());
        weld::Button& rCustomPattern = rColorTab.GetCustomPatternButton();
        if (m_bOfferPattern)
        {
            CreatePatternStyle(rColorTab);
            rCustomPattern.connect_clicked(LINK(this, SvxAreaTabPage, CustomPatternHdl_Impl));
        }
        else
            rColorTab.HidePatternControls();
        rColorTab.Construct();
        rColorTab.ActivatePage(m_aFillAttributeSet);
        rColorTab.Reset(&m_aFillAttributeSet);
        rColorTab.SetNoFill(m_bNoFill);
        rColorTab.set_visible(true);
    }
}

void SvxAreaTabPage::SetEffects(std::vector<FillType>&& rEffects)
{
    m_aEffects = std::move(rEffects);
    if (SvxColorFillTabPage* pColorPage = getColorTabPage())
        pColorPage->GetMoreEffectsButton().set_visible(!m_aEffects.empty());
}

void SvxAreaTabPage::UpdatePreview()
{
    if (SvxColorFillTabPage* pColorPage = getColorTabPage())
        pColorPage->UpdatePreview();
}

IMPL_LINK(SvxAreaTabPage, UpdatePreviewHdl_Impl, SvxColorFillTabPage&, rColorPage, void)
{
    // The preview shows the whole fill: None, the color, the pattern or the effect.
    SfxItemSet aFill(rColorPage.GetColorFill());
    OUString sCaption;
    if (m_oEffect)
    {
        aFill.Put(m_oEffectSet ? *m_oEffectSet : m_aFillAttributeSet);
        switch (*m_oEffect)
        {
            case FillType::GRADIENT_FILL:
                sCaption = lclCaption(RID_CUISTR_PREVIEW_GRADIENT, aFill.Get(XATTR_FILLGRADIENT).GetName());
                break;
            case FillType::HATCH_FILL:
                sCaption = lclCaption(RID_CUISTR_PREVIEW_HATCH, aFill.Get(XATTR_FILLHATCH).GetName());
                break;
            case FillType::PATTERN_FILL:
                sCaption = lclCaption(RID_CUISTR_PREVIEW_PATTERN, aFill.Get(XATTR_FILLBITMAP).GetName());
                break;
            default:
                sCaption = lclCaption(RID_CUISTR_PREVIEW_IMAGE, aFill.Get(XATTR_FILLBITMAP).GetName());
                break;
        }
    }
    else if (m_bNoFill)
    {
        aFill.Put(XFillStyleItem(drawing::FillStyle_NONE));
        sCaption = CuiResId(RID_CUISTR_FILL_NONE);
    }
    else if (m_xPatternStyle && m_xPatternStyle->HasPattern())
    {
        m_xPatternStyle->FillItemSet(aFill);
        sCaption = lclCaption(RID_CUISTR_PREVIEW_PATTERN, m_xPatternStyle->GetName());
    }
    else
        sCaption = rColorPage.GetColorCaption();
    rColorPage.ShowPreview(aFill, sCaption);
}

void SvxAreaTabPage::ClearEffect()
{
    m_oEffect.reset();
    m_oEffectSet.reset();
}

IMPL_LINK(SvxAreaTabPage, ColorChangedHdl_Impl, SvxColorFillTabPage&, rColorPage, void)
{
    // A color chosen after None, a pattern or an effect replaces it.
    m_bBtnClicked = true;
    ClearEffect();
    m_bNoFill = false;
    rColorPage.SetNoFill(false);
    if (m_xPatternStyle)
        m_xPatternStyle->Clear();
}

IMPL_LINK_NOARG(SvxAreaTabPage, NoneSelectedHdl_Impl, SvxColorFillTabPage&, void)
{
    m_bBtnClicked = true;
    ClearEffect();
    m_bNoFill = true;
    if (m_xPatternStyle)
        m_xPatternStyle->Clear();
}

IMPL_LINK(SvxAreaTabPage, PatternChangedHdl_Impl, SvxPatternStyleBox&, rPatternStyle, void)
{
    m_bBtnClicked = true;
    ClearEffect();
    // A pattern replaces None, and None in Pattern Style goes back to the color.
    if (rPatternStyle.HasPattern())
    {
        m_bNoFill = false;
        if (SvxColorFillTabPage* pColorPage = getColorTabPage())
            pColorPage->SetNoFill(false);
    }
    UpdatePreview();
}

IMPL_LINK_NOARG(SvxAreaTabPage, CustomPatternHdl_Impl, weld::Button&, void)
{
    if (m_xPatternStyle)
        m_xPatternStyle->SelectCustom();
}

IMPL_LINK_NOARG(SvxAreaTabPage, MoreEffectsHdl_Impl, weld::Button&, void)
{
    auto xDialog = std::make_shared<SvxFillEffectsDialog>(
        GetFrameWeld(), m_aFillAttributeSet, m_aEffects, m_oEffect, m_pColorList, m_pGradientList,
        m_pHatchingList, m_pBitmapList, m_pnColorListState);
    SfxTabDialogController::runAsync(xDialog, [this, xDialog](sal_Int32 nResult) {
        if (nResult != RET_OK || !xDialog->GetResult())
            return;
        // The Color tab stands for the effect, so it is shown before the effect is set.
        m_xNotebook->set_current_page(TABID_COLOR);
        m_oEffect = xDialog->GetEffect();
        m_oEffectSet.emplace(*xDialog->GetResult());
        m_bNoFill = false;
        if (SvxColorFillTabPage* pColorPage = getColorTabPage())
            pColorPage->SetNoFill(false);
        if (m_xPatternStyle)
            m_xPatternStyle->Clear();
        m_aFillAttributeSet.Put(*xDialog->GetResult());
        m_bBtnClicked = true;
        UpdatePreview();
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
