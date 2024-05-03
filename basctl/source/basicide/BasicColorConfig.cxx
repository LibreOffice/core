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

#include <tools/solar.h>
#include <BasicColorConfig.hxx>
#include <officecfg/Office/BasicIDE.hxx>

#include <sal/log.hxx>

namespace basctl
{
BasicColorConfig::BasicColorConfig()
    : ConfigItem(u"Office.BasicIDE/IDEColorSchemes"_ustr)
{
    // Initially the active color scheme is the one defined in the registry
    m_sCurrentColorScheme = officecfg::Office::BasicIDE::EditorSettings::ColorScheme::get();

    // Initialize all available scheme names
    m_aSchemeNames = GetNodeNames(u""_ustr);
}

BasicColorConfig::~BasicColorConfig() {}

void BasicColorConfig::Notify(const css::uno::Sequence<OUString>&) {}

void BasicColorConfig::ImplCommit() {}

ColorScheme BasicColorConfig::GetColorScheme(const OUString& rScheme)
{
    // If the default scheme is being requested or the scheme does not exist, return the Application Colors scheme
    if (rScheme == DEFAULT_SCHEME || comphelper::findValue(m_aSchemeNames, rScheme) == -1)
    {
        return GetAutomaticColorScheme();
    }

    std::vector<OUString> aVecPropNames = { OUString(rScheme + "/GenericColor/Color"),
                                            OUString(rScheme + "/IdentifierColor/Color"),
                                            OUString(rScheme + "/NumberColor/Color"),
                                            OUString(rScheme + "/StringColor/Color"),
                                            OUString(rScheme + "/CommentColor/Color"),
                                            OUString(rScheme + "/ErrorColor/Color"),
                                            OUString(rScheme + "/OperatorColor/Color"),
                                            OUString(rScheme + "/KeywordColor/Color"),
                                            OUString(rScheme + "/BackgroundColor/Color"),
                                            OUString(rScheme + "/LineHighlightColor/Color") };

    css::uno::Sequence<OUString> aPropNames(aVecPropNames.size());
    OUString* pPropNames = aPropNames.getArray();
    for (size_t i = 0; i < aVecPropNames.size(); i++)
    {
        pPropNames[i] = aVecPropNames[i];
    }
    css::uno::Sequence<css::uno::Any> aColors = GetProperties(aPropNames);

    ColorScheme aColorScheme;
    aColorScheme.m_sSchemeName = rScheme;
    aColorScheme.m_bIsDefault = comphelper::findValue(m_aDefaultSchemes, rScheme) != -1;
    aColors[0] >>= aColorScheme.m_aGenericFontColor;
    aColors[1] >>= aColorScheme.m_aIdentifierColor;
    aColors[2] >>= aColorScheme.m_aNumberColor;
    aColors[3] >>= aColorScheme.m_aStringColor;
    aColors[4] >>= aColorScheme.m_aCommentColor;
    aColors[5] >>= aColorScheme.m_aErrorColor;
    aColors[6] >>= aColorScheme.m_aOperatorColor;
    aColors[7] >>= aColorScheme.m_aKeywordColor;
    aColors[8] >>= aColorScheme.m_aBackgroundColor;
    aColors[9] >>= aColorScheme.m_aLineHighlightColor;

    return aColorScheme;
}

ColorScheme BasicColorConfig::GetAutomaticColorScheme()
{
    // Application Colors do not define a line highlight color, so here we adjust
    // the background color to get a highlight color
    Color aBackgroundColor = aColorConfig.GetColorValue(svtools::BASICEDITOR).nColor;
    Color aHighlightColor(aBackgroundColor);
    if (aBackgroundColor.IsDark())
        aHighlightColor.ApplyTintOrShade(1000);
    else
        aHighlightColor.ApplyTintOrShade(-1000);

    ColorScheme aScheme = { DEFAULT_SCHEME,
                            false,
                            aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor,
                            aColorConfig.GetColorValue(svtools::BASICIDENTIFIER).nColor,
                            aColorConfig.GetColorValue(svtools::BASICNUMBER).nColor,
                            aColorConfig.GetColorValue(svtools::BASICSTRING).nColor,
                            aColorConfig.GetColorValue(svtools::BASICCOMMENT).nColor,
                            aColorConfig.GetColorValue(svtools::BASICERROR).nColor,
                            aColorConfig.GetColorValue(svtools::BASICOPERATOR).nColor,
                            aColorConfig.GetColorValue(svtools::BASICKEYWORD).nColor,
                            aBackgroundColor,
                            aHighlightColor };

    return aScheme;
}

OUString& BasicColorConfig::GetCurrentColorSchemeName()
{
    // Always return from the registry to get the most up-to-date value
    m_sCurrentColorScheme = officecfg::Office::BasicIDE::EditorSettings::ColorScheme::get();
    return m_sCurrentColorScheme;
}

TranslateId BasicColorConfig::GetSchemeTranslateId(const OUString& rScheme)
{
    return m_aTranslateIdsMap.find(rScheme)->second;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
