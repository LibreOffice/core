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

#include <rtl/ustring.hxx>
#include <strings.hrc>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/configitem.hxx>
#include <svtools/colorcfg.hxx>
#include "colorscheme.hxx"
#include "iderid.hxx"
#include <map>

namespace basctl
{
// Name used to refer to the application color scheme (the one defined in Application Colors)
inline constexpr OUString DEFAULT_SCHEME = u"COLORSCHEME_DEFAULT"_ustr;

typedef std::map<OUString, TranslateId> SchemeTranslateIdMap;

class BasicColorConfig : public utl::ConfigItem
{
private:
    // Name of the color scheme that is currently active
    OUString m_sCurrentColorScheme;

    // Names of all available scheme names
    css::uno::Sequence<OUString> m_aSchemeNames;

    // Names of default color schemes shipped with LibreOffice
    css::uno::Sequence<OUString> m_aDefaultSchemes
        = { u"COLORSCHEME_LIBREOFFICE_LIGHT"_ustr, u"COLORSCHEME_LIBREOFFICE_DARK"_ustr,
            u"COLORSCHEME_BREEZE_LIGHT"_ustr,      u"COLORSCHEME_BREEZE_DARK"_ustr,
            u"COLORSCHEME_SOLARIZED_LIGHT"_ustr,   u"COLORSCHEME_SOLARIZED_DARK"_ustr };

    // Maps the scheme names to their TranslateId
    SchemeTranslateIdMap m_aTranslateIdsMap = {
        { "COLORSCHEME_LIBREOFFICE_LIGHT", RID_STR_COLORSCHEME_LIGHT },
        { "COLORSCHEME_LIBREOFFICE_DARK", RID_STR_COLORSCHEME_DARK },
        { "COLORSCHEME_BREEZE_LIGHT", RID_STR_COLORSCHEME_BREEZE_LIGHT },
        { "COLORSCHEME_BREEZE_DARK", RID_STR_COLORSCHEME_BREEZE_DARK },
        { "COLORSCHEME_SOLARIZED_LIGHT", RID_STR_COLORSCHEME_SOLARIZED_LIGHT },
        { "COLORSCHEME_SOLARIZED_DARK", RID_STR_COLORSCHEME_SOLARIZED_DARK },
    };

    // Used to get colors defined in the Application Colors dialog
    const svtools::ColorConfig aColorConfig;

    virtual void ImplCommit() override;

public:
    BasicColorConfig();
    virtual ~BasicColorConfig() override;

    virtual void Notify(const css::uno::Sequence<OUString>& aPropertyNames) override;

    ColorScheme GetColorScheme(const OUString& rScheme);
    css::uno::Sequence<OUString> GetColorSchemeNames() { return m_aSchemeNames; }

    // Returns the color scheme defined by the current Application Colors
    ColorScheme GetAutomaticColorScheme();

    // Returns the name of the currently active color scheme
    OUString& GetCurrentColorSchemeName();

    // Returns the current color scheme
    ColorScheme GetCurrentColorScheme() { return GetColorScheme(GetCurrentColorSchemeName()); }

    // Returns true if the scheme is a scheme preinstalled with LO
    bool IsDefaultScheme(const OUString& rScheme)
    {
        return comphelper::findValue(m_aDefaultSchemes, rScheme) != -1;
    }

    // Returns the TranslateId of the scheme name
    TranslateId GetSchemeTranslateId(const OUString& rScheme);
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
