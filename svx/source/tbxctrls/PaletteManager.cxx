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

#include <svx/PaletteManager.hxx>

#include <comphelper/propertyvalue.hxx>
#include <tools/urlobj.hxx>
#include <tools/json_writer.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/objsh.hxx>
#include <svx/drawitem.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>

#include <tbxcolorupdate.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/sequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/color/ComplexColorJSON.hxx>

#include <palettes.hxx>

#include <memory>
#include <array>
#include <stack>

PaletteManager::PaletteManager() :
    mnMaxRecentColors(Application::GetSettings().GetStyleSettings().GetColorValueSetColumnCount()),
    mnNumOfPalettes(3),
    mnCurrentPalette(0),
    mnColorCount(0),
    mpBtnUpdater(nullptr),
    maColorSelectFunction(PaletteManager::DispatchColorCommand)

{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if(pDocSh)
    {
        const SfxPoolItem* pItem = nullptr;
        if( nullptr != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) )
            pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }
    if(!pColorList.is())
        pColorList = XColorList::CreateStdColorList();
    LoadPalettes();
    mnNumOfPalettes += m_Palettes.size();
}

PaletteManager::PaletteManager(const PaletteManager* pClone)
    : mnMaxRecentColors(pClone->mnMaxRecentColors)
    , mnNumOfPalettes(pClone->mnNumOfPalettes)
    , mnCurrentPalette(pClone->mnCurrentPalette)
    , mnColorCount(pClone->mnColorCount)
    , mpBtnUpdater(nullptr)
    , pColorList(pClone->pColorList)
    , maRecentColors(pClone->maRecentColors)
    , maColorSelectFunction(PaletteManager::DispatchColorCommand)
{
    for (const auto& a : pClone->m_Palettes)
        m_Palettes.emplace_back(a->Clone());
}

PaletteManager* PaletteManager::Clone() const
{
    return new PaletteManager(this);
}

PaletteManager::~PaletteManager()
{
}

void PaletteManager::LoadPalettes()
{
    m_Palettes.clear();
    OUString aPalPaths = SvtPathOptions().GetPalettePath();

    std::stack<OUString> aDirs;
    sal_Int32 nIndex = 0;
    do
    {
        aDirs.push(aPalPaths.getToken(0, ';', nIndex));
    }
    while (nIndex >= 0);

    std::set<OUString> aNames;
    //try all entries palette path list user first, then
    //system, ignoring duplicate file names
    while (!aDirs.empty())
    {
        OUString aPalPath = aDirs.top();
        aDirs.pop();

        osl::Directory aDir(aPalPath);
        osl::DirectoryItem aDirItem;
        osl::FileStatus aFileStat( osl_FileStatus_Mask_FileName |
                                   osl_FileStatus_Mask_FileURL  |
                                   osl_FileStatus_Mask_Type     );
        if( aDir.open() == osl::FileBase::E_None )
        {
            while( aDir.getNextItem(aDirItem) == osl::FileBase::E_None )
            {
                aDirItem.getFileStatus(aFileStat);
                if(aFileStat.isRegular() || aFileStat.isLink())
                {
                    OUString aFName = aFileStat.getFileName();
                    INetURLObject aURLObj( aFileStat.getFileURL() );
                    OUString aFNameWithoutExt = aURLObj.GetBase();
                    if (aNames.find(aFName) == aNames.end())
                    {
                        std::unique_ptr<Palette> pPalette;
                        if( aFName.endsWithIgnoreAsciiCase(".gpl") )
                            pPalette.reset(new PaletteGPL(aFileStat.getFileURL(), aFNameWithoutExt));
                        else if( aFName.endsWithIgnoreAsciiCase(".soc") )
                        {
                            if (aFNameWithoutExt == "standard")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_STANDARD);
                            else if (aFNameWithoutExt == "tonal")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_TONAL);
                            else if (aFNameWithoutExt == "html")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_HTML);
                            else if (aFNameWithoutExt == "chart-palettes")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_CHARTPALETTES);
                            else if (aFNameWithoutExt == "compatibility")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_COMPATIBILITY);
                            else if (aFNameWithoutExt == "material")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_MATERIAL);
                            else if (aFNameWithoutExt == "libreoffice")
                                aFNameWithoutExt = "LibreOffice";
                            else if (aFNameWithoutExt == "freecolour-hlc")
                                aFNameWithoutExt = SvxResId(RID_SVXSTR_COLOR_PALETTE_FREECOLOURHLC);
                            pPalette.reset(new PaletteSOC(aFileStat.getFileURL(), aFNameWithoutExt));
                        }
                        else if ( aFName.endsWithIgnoreAsciiCase(".ase") )
                            pPalette.reset(new PaletteASE(aFileStat.getFileURL(), aFNameWithoutExt));

                        if( pPalette && pPalette->IsValid() )
                            m_Palettes.push_back( std::move(pPalette) );

                        aNames.insert(aFNameWithoutExt);
                    }
                }
            }
        }
    }
}

bool PaletteManager::IsThemePaletteSelected() const
{
    return mnCurrentPalette == mnNumOfPalettes - 2;
}

bool PaletteManager::GetThemeAndEffectIndex(sal_uInt16 nItemId, sal_uInt16& rThemeIndex, sal_uInt16& rEffectIndex)
{
    // Each column is the same color with different effects.
    rThemeIndex = nItemId % 12;

    rEffectIndex = nItemId / 12;
    if (rEffectIndex > 5)
        return false;
    return true;
}

bool PaletteManager::GetLumModOff(sal_uInt16 nThemeIndex, sal_uInt16 nEffect, sal_Int16& rLumMod, sal_Int16& rLumOff)
{
    if (!moThemePaletteCollection)
        return false;

    auto const& aThemeColorData = moThemePaletteCollection->maColors[nThemeIndex];

    rLumMod = aThemeColorData.getLumMod(nEffect);
    rLumOff = aThemeColorData.getLumOff(nEffect);

    return true;
}

void PaletteManager::ReloadColorSet(SvxColorValueSet &rColorSet)
{
    moThemePaletteCollection.reset();
    if( mnCurrentPalette == 0)
    {
        rColorSet.Clear();
        css::uno::Sequence< sal_Int32 > CustomColorList( officecfg::Office::Common::UserColors::CustomColor::get() );
        css::uno::Sequence< OUString > CustomColorNameList( officecfg::Office::Common::UserColors::CustomColorName::get() );
        int nIx = 1;
        for (int i = 0; i < CustomColorList.getLength(); ++i)
        {
            Color aColor(ColorTransparency, CustomColorList[i]);
            rColorSet.InsertItem(nIx, aColor, CustomColorNameList[i]);
            ++nIx;
        }
    }
    else if (IsThemePaletteSelected())
    {
        SfxObjectShell* pObjectShell = SfxObjectShell::Current();
        if (pObjectShell)
        {
            auto pColorSet = pObjectShell->GetThemeColors();
            mnColorCount = 12;
            rColorSet.Clear();
            sal_uInt16 nItemId = 1;

            if (!pColorSet)
                return;

            svx::ThemeColorPaletteManager aThemeColorManager(pColorSet);
            moThemePaletteCollection = aThemeColorManager.generate();

            // Each row is one effect type (no effect + each type).
            for (size_t nEffect : {0, 1, 2, 3, 4, 5})
            {
                // Each column is one color type.
                for (auto const& rColorData : moThemePaletteCollection->maColors)
                {
                    auto const& rEffect = rColorData.maEffects[nEffect];
                    rColorSet.InsertItem(nItemId++, rEffect.maColor, rEffect.maColorName);
                }
            }
        }
    }
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
    {
        // Add doc colors to palette
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if (pDocSh)
        {
            std::set<Color> aColors = pDocSh->GetDocColors();
            mnColorCount = aColors.size();
            rColorSet.Clear();
            rColorSet.addEntriesForColorSet(aColors, Concat2View(SvxResId( RID_SVXSTR_DOC_COLOR_PREFIX ) + " ") );
        }
    }
    else
    {
        m_Palettes[mnCurrentPalette - 1]->LoadColorSet( rColorSet );
        mnColorCount = rColorSet.GetItemCount();
    }
}

void PaletteManager::ReloadColorSet(weld::IconView &pIconView)
{
    moThemePaletteCollection.reset();
    if( mnCurrentPalette == 0)
    {
        pIconView.clear();
        css::uno::Sequence< sal_Int32 > CustomColorList( officecfg::Office::Common::UserColors::CustomColor::get() );
        css::uno::Sequence< OUString > CustomColorNameList( officecfg::Office::Common::UserColors::CustomColorName::get() );
        for (int i = 0; i < CustomColorList.getLength(); ++i)
        {
            Color aColor(ColorTransparency, CustomColorList[i]);
            VclPtr<VirtualDevice> pColorVDev = SvxColorIconView::createColorVirtualDevice(aColor);
            OUString sId = OUString::number(i);
            OUString sColorName = CustomColorNameList[i];
            pIconView.insert(i, &sColorName, &sId, pColorVDev, nullptr);
        }
    }
    else if (IsThemePaletteSelected())
    {
        SfxObjectShell* pObjectShell = SfxObjectShell::Current();
        if (pObjectShell)
        {
            auto pColorSet = pObjectShell->GetThemeColors();
            mnColorCount = 12;
            pIconView.clear();
            sal_uInt16 nItemId = 0;

            if (!pColorSet)
                return;

            svx::ThemeColorPaletteManager aThemeColorManager(pColorSet);
            moThemePaletteCollection = aThemeColorManager.generate();

            // Each row is one effect type (no effect + each type).
            for (size_t nEffect : {0, 1, 2, 3, 4, 5})
            {
                // Each column is one color type.
                for (auto const& rColorData : moThemePaletteCollection->maColors)
                {
                    auto const& rEffect = rColorData.maEffects[nEffect];
                    Color aColor = rEffect.maColor;
                    VclPtr<VirtualDevice> pColorVDev = SvxColorIconView::createColorVirtualDevice(aColor);
                    OUString sColorName = rEffect.maColorName;
                    OUString sId = OUString::number(nItemId);
                    pIconView.insert(nItemId, &sColorName, &sId, pColorVDev, nullptr);
                    nItemId++;
                }
            }
        }
    }
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
    {
        // Add doc colors to palette
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if (pDocSh)
        {
            std::set<Color> aColors = pDocSh->GetDocColors();
            mnColorCount = aColors.size();
            pIconView.clear();
            SvxColorIconView::addEntriesForColorSet(pIconView, aColors, Concat2View(SvxResId( RID_SVXSTR_DOC_COLOR_PREFIX ) + " ") );
        }
    }
    else
    {
        m_Palettes[mnCurrentPalette - 1]->LoadColorSet( pIconView );
        mnColorCount = pIconView.n_children();
    }
}

void PaletteManager::ReloadRecentColorSet(weld::IconView& pIconView)
{
    maRecentColors.clear();
    pIconView.clear();
    css::uno::Sequence< sal_Int32 > Colorlist(officecfg::Office::Common::UserColors::RecentColor::get());
    css::uno::Sequence< OUString > ColorNamelist(officecfg::Office::Common::UserColors::RecentColorName::get());
    int nIx = 0;
    const bool bHasColorNames = Colorlist.getLength() == ColorNamelist.getLength();
    for (int i = 0; i < Colorlist.getLength(); ++i)
    {
        Color aColor(ColorTransparency, Colorlist[i]);
        VclPtr<VirtualDevice> pColorVDev = SvxColorIconView::createColorVirtualDevice(aColor);
        OUString sColorName = bHasColorNames ? ColorNamelist[i] : ("#" + aColor.AsRGBHexString().toAsciiUpperCase());
        maRecentColors.emplace_back(aColor, sColorName);
        OUString sId = OUString::number(nIx);
        pIconView.insert(nIx, &sColorName, &sId, pColorVDev, nullptr);
        ++nIx;
    }
}

std::vector<OUString> PaletteManager::GetPaletteList()
{
    std::vector<OUString> aPaletteNames
    {
        SvxResId( RID_SVXSTR_CUSTOM_PAL )
    };
    for (auto const& it : m_Palettes)
    {
        aPaletteNames.push_back( (*it).GetName() );
    }
    aPaletteNames.push_back(SvxResId(RID_SVXSTR_THEME_COLORS));
    aPaletteNames.push_back( SvxResId ( RID_SVXSTR_DOC_COLORS ) );

    return aPaletteNames;
}

void PaletteManager::SetPalette( sal_Int32 nPos )
{
    mnCurrentPalette = nPos;
    if( nPos != mnNumOfPalettes - 1 && nPos != 0)
    {
        pColorList = XPropertyList::AsColorList(
                            XPropertyList::CreatePropertyListFromURL(
                            XPropertyListType::Color, GetSelectedPalettePath()));
        auto name = GetPaletteName(); // may change pColorList
        pColorList->SetName(name);
        if(pColorList->Load())
        {
            SfxObjectShell* pShell = SfxObjectShell::Current();
            if (pShell != nullptr)
            {
                SvxColorListItem aColorItem(pColorList, SID_COLOR_TABLE);
                pShell->PutItem( aColorItem );
            }
        }
    }
    OUString aPaletteName(officecfg::Office::Common::UserColors::PaletteName::get());
    if (aPaletteName != GetPaletteName())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::UserColors::PaletteName::set(GetPaletteName(), batch);
        batch->commit();
    }
}

sal_Int32 PaletteManager::GetPalette() const
{
    return mnCurrentPalette;
}

OUString PaletteManager::GetPaletteName()
{
    std::vector<OUString> aNames(GetPaletteList());
    if(mnCurrentPalette != mnNumOfPalettes - 1 && mnCurrentPalette != 0)
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if(pDocSh)
        {
            const SfxPoolItem* pItem = nullptr;
            if( nullptr != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) )
                pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
        }
    }
    return aNames[mnCurrentPalette];
}

const OUString & PaletteManager::GetSelectedPalettePath()
{
    if (mnCurrentPalette < m_Palettes.size() && mnCurrentPalette != 0)
        return m_Palettes[mnCurrentPalette - 1]->GetPath();
    else
        return EMPTY_OUSTRING;
}

std::vector<NamedColor> PaletteManager::GetColors() const
{
    std::vector<NamedColor> colors;

    if (mnCurrentPalette == 0) {
        css::uno::Sequence<sal_Int32> CustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
        css::uno::Sequence<OUString> CustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());

        for (int i = 0; i < CustomColorList.getLength(); ++i) {
            Color aColor(ColorTransparency, CustomColorList[i]);
            OUString colorName = (i < CustomColorNameList.getLength()) ? CustomColorNameList[i] : OUString();
            colors.emplace_back(aColor, colorName);
        }
    }
    else if (IsThemePaletteSelected()) {
        if (moThemePaletteCollection) {
            for (size_t nEffect : {0, 1, 2, 3, 4, 5}) {
                for (auto const& rColorData : moThemePaletteCollection->maColors) {
                    auto const& rEffect = rColorData.maEffects[nEffect];
                    colors.emplace_back(rEffect.maColor, rEffect.maColorName);
                }
            }
        }
    }
    else if (mnCurrentPalette == mnNumOfPalettes - 1) {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        if (pDocSh) {
            std::set<Color> aColors = pDocSh->GetDocColors();
            OUString namePrefix = SvxResId(RID_SVXSTR_DOC_COLOR_PREFIX) + " ";
            int index = 1;
            for (const Color& color : aColors) {
                colors.emplace_back(color, namePrefix + OUString::number(index++));
            }
        }
    }
    else {
        return m_Palettes[mnCurrentPalette - 1]->GetColorList();
    }

    return colors;
}

std::vector< NamedColor > PaletteManager::GetRecentColors() const
{
    return std::vector< NamedColor >(maRecentColors.begin(), maRecentColors.end());
}

tools::Long PaletteManager::GetColorCount() const
{
    return mnColorCount;
}

tools::Long PaletteManager::GetRecentColorCount() const
{
    return maRecentColors.size();
}

void PaletteManager::AddRecentColor(const Color& rRecentColor, const OUString& rName, bool bFront)
{
    auto itColor = std::find_if(maRecentColors.begin(),
                                maRecentColors.end(),
                                [rRecentColor] (const NamedColor &aColor) { return aColor.m_aColor == rRecentColor; });
    // if recent color to be added is already in list, remove it
    if( itColor != maRecentColors.end() )
        maRecentColors.erase( itColor );

    if (maRecentColors.size() == mnMaxRecentColors)
        maRecentColors.pop_back();
    if (bFront)
        maRecentColors.emplace_front(rRecentColor, rName);
    else
        maRecentColors.emplace_back(rRecentColor, rName);
    css::uno::Sequence< sal_Int32 > aColorList(maRecentColors.size());
    auto aColorListRange = asNonConstRange(aColorList);
    css::uno::Sequence< OUString > aColorNameList(maRecentColors.size());
    auto aColorNameListRange = asNonConstRange(aColorNameList);
    for (size_t i = 0; i < maRecentColors.size(); ++i)
    {
        aColorListRange[i] = static_cast<sal_Int32>(maRecentColors[i].m_aColor);
        aColorNameListRange[i] = maRecentColors[i].m_aName;
    }
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::UserColors::RecentColor::set(aColorList, batch);
    officecfg::Office::Common::UserColors::RecentColorName::set(aColorNameList, batch);
    batch->commit();
}

void PaletteManager::SetSplitButtonColor(const NamedColor& rColor)
{
    if (mpBtnUpdater)
        mpBtnUpdater->SetRecentColor(rColor);
}

void PaletteManager::SetBtnUpdater(svx::ToolboxButtonColorUpdaterBase* pBtnUpdater)
{
    mpBtnUpdater = pBtnUpdater;
}

void PaletteManager::SetColorSelectFunction(const ColorSelectFunction& aColorSelectFunction)
{
    maColorSelectFunction = aColorSelectFunction;
}

void PaletteManager::PopupColorPicker(weld::Window* pParent, const OUString& aCommand, const Color& rInitialColor)
{
    // The calling object goes away during aColorDlg.Execute(), so we must copy this
    OUString aCommandCopy = aCommand;
    m_pColorDlg = std::make_unique<SvColorDialog>();
    m_pColorDlg->SetColor(rInitialColor);
    m_pColorDlg->SetMode(svtools::ColorPickerMode::Modify);
    std::shared_ptr<PaletteManager> xSelf(shared_from_this());
    m_pColorDlg->ExecuteAsync(pParent, [xSelf=std::move(xSelf),
                                        aCommandCopy=std::move(aCommandCopy)] (sal_Int32 nResult) {
        if (nResult == RET_OK)
        {
            Color aLastColor = xSelf->m_pColorDlg->GetColor();
            OUString sColorName = "#" + aLastColor.AsRGBHexString().toAsciiUpperCase();
            NamedColor aNamedColor(aLastColor, sColorName);
            xSelf->SetSplitButtonColor(aNamedColor);
            xSelf->AddRecentColor(aLastColor, sColorName);
            xSelf->maColorSelectFunction(aCommandCopy, aNamedColor);
        }
    });
}

void PaletteManager::DispatchColorCommand(const OUString& aCommand, const NamedColor& rColor)
{
    using namespace css;
    using namespace css::uno;
    using namespace css::frame;
    using namespace css::beans;
    using namespace css::util;

    const Reference<XComponentContext>& xContext(comphelper::getProcessComponentContext());
    Reference<XDesktop2> xDesktop = Desktop::create(xContext);
    Reference<XFrame> xFrame(xDesktop->getCurrentFrame());
    Reference<XDispatchProvider> xDispatchProvider(xFrame, UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    INetURLObject aObj( aCommand );

    std::vector<PropertyValue> aArgs{
        comphelper::makePropertyValue(aObj.GetURLPath()+ ".Color", sal_Int32(rColor.m_aColor)),
    };

    if (rColor.m_nThemeIndex != -1)
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setThemeColor(model::convertToThemeColorType(rColor.m_nThemeIndex));
        if (rColor.m_nLumMod != 10000)
            aComplexColor.addTransformation({model::TransformationType::LumMod, rColor.m_nLumMod});
        if (rColor.m_nLumMod != 0)
            aComplexColor.addTransformation({model::TransformationType::LumOff, rColor.m_nLumOff});

        uno::Any aAny;
        aAny <<= OStringToOUString(model::color::convertToJSON(aComplexColor), RTL_TEXTENCODING_UTF8);

        aArgs.push_back(comphelper::makePropertyValue(aObj.GetURLPath() + ".ComplexColorJSON", aAny));
    }

    URL aTargetURL;
    aTargetURL.Complete = aCommand;
    Reference<XURLTransformer> xURLTransformer(URLTransformer::create(comphelper::getProcessComponentContext()));
    xURLTransformer->parseStrict(aTargetURL);

    Reference<XDispatch> xDispatch = xDispatchProvider->queryDispatch(aTargetURL, OUString(), 0);
    if (xDispatch.is())
    {
        xDispatch->dispatch(aTargetURL, comphelper::containerToSequence(aArgs));
        if (xFrame->getContainerWindow().is())
            xFrame->getContainerWindow()->setFocus();
    }
}

void PaletteManager::generateColorNamesJSON(tools::JsonWriter& aTree)
{
    XColorListRef xUserColorList;
    OUString aPaletteStandard = SvxResId(RID_SVXSTR_COLOR_PALETTE_STANDARD);
    PaletteManager aPaletteManager;
    std::vector<OUString> aPaletteNames = aPaletteManager.GetPaletteList();
    for (size_t i = 0, nLen = aPaletteNames.size(); i < nLen; ++i)
    {
        if (aPaletteStandard == aPaletteNames[i])
        {
            aPaletteManager.SetPalette(i);
            xUserColorList
                = XPropertyList::AsColorList(XPropertyList::CreatePropertyListFromURL(
                    XPropertyListType::Color, aPaletteManager.GetSelectedPalettePath()));
            if (!xUserColorList->Load())
                xUserColorList = nullptr;
            break;
        }
    }
    if (xUserColorList)
    {
        auto colorNames = aTree.startArray("ColorNames");
        int nCount = xUserColorList->Count();

        for (int i = 0; i < nCount; i++)
        {
            XColorEntry* pColorEntry = xUserColorList->GetColor(i);
            if (pColorEntry)
            {
                auto aColorTree = aTree.startStruct();
                aTree.put("hexCode", pColorEntry->GetColor().AsRGBHEXString());
                aTree.put("name", pColorEntry->GetName());
            }
        }
    }
}

// TODO: make it generic, send any palette
void PaletteManager::generateJSON(tools::JsonWriter& aTree, const std::set<Color>& rColors)
{
    auto aColorListTree = aTree.startArray("DocumentColors");
    sal_uInt32 nStartIndex = 1;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    sal_uInt32 nColumnCount = rStyleSettings.GetColorValueSetColumnCount();
    const OUString aNamePrefix(Concat2View(SvxResId(RID_SVXSTR_DOC_COLOR_PREFIX) + " "));

    auto aColorIt = rColors.begin();
    while (aColorIt != rColors.end())
    {
        auto aColorRowTree = aTree.startAnonArray();

        for (sal_uInt32 nColumn = 0; nColumn < nColumnCount; nColumn++)
        {
            auto aColorTree = aTree.startStruct();
            OUString sName = aNamePrefix + OUString::number(nStartIndex++);
            aTree.put("Value", aColorIt->AsRGBHexString().toUtf8());
            aTree.put("Name", sName);

            aColorIt++;
            if (aColorIt == rColors.end())
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
