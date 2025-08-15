/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/itempool.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/uno/Any.h>

//#include <vector>

namespace chart
{
// if you change the size of the chart thumbnails, update the UI files that display them.
// dlg_Theme.ui , chartthemepopup.ui
constexpr int ChartThemeThumbSizeX = 192;
constexpr int ChartThemeThumbSizeY = 140;

constexpr OUString sUnoChartTheme = u".uno:ChartTheme"_ustr;

// Properties of a chart element, like Title.
// At first it contained properties only from Font, and Font Effect dialog tab panel.
// but now, it store other properties. For more info about what properties it can store,
// check lcl_GetPropertyNameVec in ChartController.cxx
class ChartElementThemeType
{
public:
    std::vector<std::pair<OUString, css::uno::Any>> m_aProperties;
    void convertPoolItemsToProperties(std::vector<SfxPoolItem*> aPoolItems);
};

// A chart theme (or style) conains several chart elements properties
enum class ChartThemeElementID : sal_uInt8
{
    Title,
    Legend,
    Axis,
    Label,
    SubTitle,
    AxisY,
    AxisZ,
    Nothing
};
class ChartThemeType
{
public:
    static constexpr size_t ElementCount = 7;
    std::array<ChartElementThemeType, ElementCount> m_aElements;
    ChartThemeType() {}

    OUString m_aName;
    sal_Int32 m_nID;
};

// It is a list of themes for charts.
// All of them contains separete properties for different chart elements.
// TODO: it is a singleton .. probably we shoul find a new place to store it.
class ChartThemesType
{
public:
    std::vector<ChartThemeType> m_aThemes;

    static ChartThemesType& getInstance();
    ChartThemesType();
    ChartThemesType(ChartThemesType const&) = delete;
    void operator=(ChartThemesType const&) = delete;

    static ChartThemesType* m_aInstance;

    // the first FixedCount Themes are not customiyable, they are pre-defined
    static constexpr sal_Int32 FixedCount = 4;
    sal_uInt32 getThemesCount() { return m_aThemes.size(); }
    static sal_uInt32 getThemesPreDefCount() { return FixedCount; }
};

} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
