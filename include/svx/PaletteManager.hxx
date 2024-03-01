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
#pragma once

#include <svx/Palette.hxx>
#include <rtl/ustring.hxx>
#include <svx/xtable.hxx>
#include <svtools/colrdlg.hxx>
#include <svx/theme/ThemeColorPaletteManager.hxx>

#include <deque>
#include <vector>
#include <memory>
#include <set>

namespace com::sun::star::uno { class XComponentContext; }
namespace svx { class ToolboxButtonColorUpdaterBase; }
namespace weld { class Window; }
namespace model { class ColorSet; }
namespace tools { class JsonWriter; }

class SVXCORE_DLLPUBLIC PaletteManager : public std::enable_shared_from_this<PaletteManager>
{
    const sal_uInt16        mnMaxRecentColors;

    sal_uInt16              mnNumOfPalettes;
    sal_uInt16              mnCurrentPalette;

    tools::Long                    mnColorCount;
    svx::ToolboxButtonColorUpdaterBase* mpBtnUpdater;

    XColorListRef           pColorList;
    std::deque<NamedColor>  maRecentColors;
    std::vector<std::unique_ptr<Palette>> m_Palettes;

    ColorSelectFunction maColorSelectFunction;

    std::unique_ptr<SvColorDialog> m_pColorDlg;
    std::optional<svx::ThemePaletteCollection> moThemePaletteCollection;

    PaletteManager(const PaletteManager* pClone);
public:
    PaletteManager();
    ~PaletteManager();
    PaletteManager(const PaletteManager&) = delete;
    PaletteManager& operator=(const PaletteManager&) = delete;
    void        LoadPalettes();
    void        ReloadColorSet(SvxColorValueSet& rColorSet);
    void        ReloadRecentColorSet(SvxColorValueSet& rColorSet);
    std::vector<OUString> GetPaletteList();
    void        SetPalette( sal_Int32 nPos );
    sal_Int32   GetPalette() const;
    sal_Int32   GetPaletteCount() const { return mnNumOfPalettes; }
    OUString    GetPaletteName();
    OUString    GetSelectedPalettePath();

    tools::Long        GetColorCount() const;
    tools::Long        GetRecentColorCount() const;
    void        AddRecentColor(const Color& rRecentColor, const OUString& rColorName, bool bFront = true);
    void        SetSplitButtonColor(const NamedColor& rColor);

    void        SetBtnUpdater(svx::ToolboxButtonColorUpdaterBase* pBtnUpdater);
    void        PopupColorPicker(weld::Window* pParent, const OUString& aCommand, const Color& rInitialColor);

    void        SetColorSelectFunction(const ColorSelectFunction& aColorSelectFunction);

    bool IsThemePaletteSelected() const;

    PaletteManager* Clone() const;

    static bool GetThemeAndEffectIndex(sal_uInt16 nItemId, sal_uInt16& rThemeIndex, sal_uInt16& rEffectIndex);
    bool GetLumModOff(sal_uInt16 nThemeIndex, sal_uInt16 nEffect, sal_Int16& rLumMod, sal_Int16& rLumOff);

    static void DispatchColorCommand(const OUString& aCommand, const NamedColor& rColor);

    /// Appends node for Document Colors into the ptree
    static void generateJSON(tools::JsonWriter& aTree, const std::set<Color>& rColors);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
