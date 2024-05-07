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

#include <fstream>
#include <treeopt.hxx>

#include <treeopt.hrc>
#include <helpids.h>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>

namespace
{
struct PageIdToFileNameMap_Impl
{
    sal_uInt16 m_nPageId;
    OUString m_sContent;
};
}

static PageIdToFileNameMap_Impl FileMap_Impl[] = {

    // { PAGE-ID, CONTENT },

    // ProductName, SID_GENERAL_OPTIONS
    { RID_SFXPAGE_GENERAL, u""_ustr }, // UserData

    { OFA_TP_MISC, u""_ustr }, // General

    { OFA_TP_VIEW, u""_ustr }, // View

    { RID_SFXPAGE_PRINTOPTIONS, u""_ustr }, // Print

    { RID_SFXPAGE_PATH, u""_ustr }, // Paths

    { RID_SVX_FONT_SUBSTITUTION, u""_ustr }, // Fonts

    { RID_SVXPAGE_INET_SECURITY, u""_ustr }, // Security

    { RID_SVXPAGE_PERSONALIZATION, u""_ustr }, // Personalization
    { RID_SVXPAGE_COLORCONFIG, u""_ustr }, // ApplicationColors
    { RID_SVXPAGE_ACCESSIBILITYCONFIG, u""_ustr }, // Accessibility
    { RID_SVXPAGE_OPTIONS_JAVA, u""_ustr }, // Java
    { RID_SVXPAGE_BASICIDE_OPTIONS, u""_ustr }, // BasicIDEOptions
    { RID_SVXPAGE_ONLINEUPDATE, u""_ustr }, // OnlineUpdate
    { RID_SVXPAGE_OPENCL, u""_ustr }, // OpenCL

    // LanguageSettings, SID_LANGUAGE_OPTIONS
    { OFA_TP_LANGUAGES, u""_ustr }, // Languages
    { RID_SFXPAGE_LINGU, u""_ustr }, // WritingAids
    { RID_SVXPAGE_JSEARCH_OPTIONS, u""_ustr }, // SearchingInJapanese
    { RID_SVXPAGE_ASIAN_LAYOUT, u""_ustr }, // AsianLayout
    { RID_SVXPAGE_OPTIONS_CTL, u""_ustr }, // ComplexTextLayout
    { RID_SVXPAGE_LANGTOOL_OPTIONS, u""_ustr }, // LanguageTool Server
    { RID_SVXPAGE_DEEPL_OPTIONS, u""_ustr }, // DeepL Server

    // Internet, SID_INET_DLG
    { RID_SVXPAGE_INET_PROXY, u""_ustr }, // Proxy
    { RID_SVXPAGE_INET_MAIL, u""_ustr }, // Email

    // Load/Save, SID_FILTER_DLG
    { RID_SFXPAGE_SAVE, u""_ustr }, // General
    { SID_OPTFILTER_MSOFFICE, u""_ustr }, // VBA Properties
    { RID_OFAPAGE_MSFILTEROPT2, u""_ustr }, // Microsoft Office
    { RID_OFAPAGE_HTMLOPT, u""_ustr }, // HTML Compatibility

    // Writer, SID_SW_EDITOPTIONS
    { RID_SW_TP_OPTLOAD_PAGE, u""_ustr }, // General
    { RID_SW_TP_CONTENT_OPT, u""_ustr }, // View
    { RID_SW_TP_OPTSHDWCRSR, u""_ustr }, // FormattingAids
    { RID_SVXPAGE_GRID, u""_ustr }, // Grid
    { RID_SW_TP_STD_FONT, u""_ustr }, // BasicFontsWestern
    { RID_SW_TP_STD_FONT_CJK, u""_ustr }, // BasicFontsAsian
    { RID_SW_TP_STD_FONT_CTL, u""_ustr }, // BasicFontsCTL
    { RID_SW_TP_OPTPRINT_PAGE, u""_ustr }, // Print
    { RID_SW_TP_OPTTABLE_PAGE, u""_ustr }, // Table
    { RID_SW_TP_REDLINE_OPT, u""_ustr }, // Changes
    { RID_SW_TP_COMPARISON_OPT, u""_ustr }, // Comparison
    { RID_SW_TP_OPTCOMPATIBILITY_PAGE, u""_ustr }, // Compatibility
    { RID_SW_TP_OPTCAPTION_PAGE, u""_ustr }, // AutoCaption
    { RID_SW_TP_MAILCONFIG, u""_ustr }, // MailMerge
    { RID_SW_TP_OPTTEST_PAGE, u""_ustr }, // Internal Test (only in dev build)

    // WriterWeb, SID_SW_ONLINEOPTIONS
    { RID_SW_TP_HTML_CONTENT_OPT, u""_ustr }, // View
    { RID_SW_TP_HTML_OPTSHDWCRSR, u""_ustr }, // FormattingAids
    { RID_SW_TP_HTML_OPTGRID_PAGE, u""_ustr }, // Grid
    { RID_SW_TP_HTML_OPTPRINT_PAGE, u""_ustr }, // Print
    { RID_SW_TP_HTML_OPTTABLE_PAGE, u""_ustr }, // Table
    { RID_SW_TP_BACKGROUND, u""_ustr }, // Background
    { RID_SW_TP_OPTTEST_PAGE, u""_ustr }, // Internal Test (only in dev build)

    // Math, SID_SM_EDITOPTIONS
    { SID_SM_TP_PRINTOPTIONS, u""_ustr }, // Settings

    // Calc, SID_SC_EDITOPTIONS
    { SID_SC_TP_LAYOUT, u""_ustr }, // General
    { RID_SC_TP_DEFAULTS, u""_ustr }, // Defaults
    { SID_SC_TP_CONTENT, u""_ustr }, // View
    { SID_SC_TP_CALC, u""_ustr }, // Calculate
    { SID_SC_TP_FORMULA, u""_ustr }, // Formula
    { SID_SC_TP_USERLISTS, u""_ustr }, // SortLists
    { SID_SC_TP_CHANGES, u""_ustr }, // Changes
    { SID_SC_TP_COMPATIBILITY, u""_ustr }, // Compatibility
    { SID_SC_TP_GRID, u""_ustr }, // Grid
    { RID_SC_TP_PRINT, u""_ustr }, // Print

    // Impress, SID_SD_EDITOPTIONS
    { SID_SI_TP_MISC, u""_ustr }, // General
    { SID_SI_TP_CONTENTS, u""_ustr }, // View
    { SID_SI_TP_SNAP, u""_ustr }, // Grid
    { SID_SI_TP_PRINT, u""_ustr }, // Print

    // Draw, SID_SD_GRAPHIC_OPTIONS
    { SID_SD_TP_MISC, u""_ustr }, // General
    { SID_SD_TP_CONTENTS, u""_ustr }, // View
    { SID_SD_TP_SNAP, u""_ustr }, // Grid
    { SID_SD_TP_PRINT, u""_ustr }, // Print

    // Charts, SID_SCH_EDITOPTIONS
    { RID_OPTPAGE_CHART_DEFCOLORS, u""_ustr }, // DefaultColor

    // Base, SID_SB_STARBASEOPTIONS
    { SID_SB_CONNECTIONPOOLING, u""_ustr }, // Connections
    { SID_SB_DBREGISTEROPTIONS, u""_ustr } // Databases
};

void TreeOptHelper::storeStringsOfDialog(sal_uInt16 nPageId, const OUString& sPageStrings)
{
    for (PageIdToFileNameMap_Impl& rEntry : FileMap_Impl)
    {
        if (rEntry.m_nPageId == nPageId)
        {
            rEntry.m_sContent = sPageStrings.replaceAll("~", "");
            break;
        }
    }
}

/*
return the strings of:
- labels,
- check buttons,
- radio buttons,
- toggle buttons,
- link buttons,
- buttons

TODO:
- accessible-names
- accessible-descriptions
- tooltip-texts
*/
OUString TreeOptHelper::getStringsFromDialog(sal_uInt16 nPageId)
{
    if (nPageId == 0)
        return OUString();

    for (PageIdToFileNameMap_Impl& rEntry : FileMap_Impl)
    {
        if (rEntry.m_nPageId == nPageId)
            return rEntry.m_sContent;
    }

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
