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
    { RID_SFXPAGE_GENERAL, "" }, // UserData

    { OFA_TP_MISC, "" }, // General

    { OFA_TP_VIEW, "" }, // View

    { RID_SFXPAGE_PRINTOPTIONS, "" }, // Print

    { RID_SFXPAGE_PATH, "" }, // Paths

    { RID_SVX_FONT_SUBSTITUTION, "" }, // Fonts

    { RID_SVXPAGE_INET_SECURITY, "" }, // Security

    { RID_SVXPAGE_PERSONALIZATION, "" }, // Personalization
    { RID_SVXPAGE_COLORCONFIG, "" }, // ApplicationColors
    { RID_SVXPAGE_ACCESSIBILITYCONFIG, "" }, // Accessibility
    { RID_SVXPAGE_OPTIONS_JAVA, "" }, // Java
    { RID_SVXPAGE_BASICIDE_OPTIONS, "" }, // BasicIDEOptions
    { RID_SVXPAGE_ONLINEUPDATE, "" }, // OnlineUpdate
    { RID_SVXPAGE_OPENCL, "" }, // OpenCL

    // LanguageSettings, SID_LANGUAGE_OPTIONS
    { OFA_TP_LANGUAGES, "" }, // Languages
    { RID_SFXPAGE_LINGU, "" }, // WritingAids
    { RID_SVXPAGE_JSEARCH_OPTIONS, "" }, // SearchingInJapanese
    { RID_SVXPAGE_ASIAN_LAYOUT, "" }, // AsianLayout
    { RID_SVXPAGE_OPTIONS_CTL, "" }, // ComplexTextLayout
    { RID_SVXPAGE_LANGTOOL_OPTIONS, "" }, // LanguageTool Server
    { RID_SVXPAGE_DEEPL_OPTIONS, "" }, // DeepL Server

    // Internet, SID_INET_DLG
    { RID_SVXPAGE_INET_PROXY, "" }, // Proxy
    { RID_SVXPAGE_INET_MAIL, "" }, // Email

    // Load/Save, SID_FILTER_DLG
    { RID_SFXPAGE_SAVE, "" }, // General
    { SID_OPTFILTER_MSOFFICE, "" }, // VBA Properties
    { RID_OFAPAGE_MSFILTEROPT2, "" }, // Microsoft Office
    { RID_OFAPAGE_HTMLOPT, "" }, // HTML Compatibility

    // Writer, SID_SW_EDITOPTIONS
    { RID_SW_TP_OPTLOAD_PAGE, "" }, // General
    { RID_SW_TP_CONTENT_OPT, "" }, // View
    { RID_SW_TP_OPTSHDWCRSR, "" }, // FormattingAids
    { RID_SVXPAGE_GRID, "" }, // Grid
    { RID_SW_TP_STD_FONT, "" }, // BasicFontsWestern
    { RID_SW_TP_STD_FONT_CJK, "" }, // BasicFontsAsian
    { RID_SW_TP_STD_FONT_CTL, "" }, // BasicFontsCTL
    { RID_SW_TP_OPTPRINT_PAGE, "" }, // Print
    { RID_SW_TP_OPTTABLE_PAGE, "" }, // Table
    { RID_SW_TP_REDLINE_OPT, "" }, // Changes
    { RID_SW_TP_COMPARISON_OPT, "" }, // Comparison
    { RID_SW_TP_OPTCOMPATIBILITY_PAGE, "" }, // Compatibility
    { RID_SW_TP_OPTCAPTION_PAGE, "" }, // AutoCaption
    { RID_SW_TP_MAILCONFIG, "" }, // MailMerge
    { RID_SW_TP_OPTTEST_PAGE, "" }, // Internal Test (only in dev build)

    // WriterWeb, SID_SW_ONLINEOPTIONS
    { RID_SW_TP_HTML_CONTENT_OPT, "" }, // View
    { RID_SW_TP_HTML_OPTSHDWCRSR, "" }, // FormattingAids
    { RID_SW_TP_HTML_OPTGRID_PAGE, "" }, // Grid
    { RID_SW_TP_HTML_OPTPRINT_PAGE, "" }, // Print
    { RID_SW_TP_HTML_OPTTABLE_PAGE, "" }, // Table
    { RID_SW_TP_BACKGROUND, "" }, // Background
    { RID_SW_TP_OPTTEST_PAGE, "" }, // Internal Test (only in dev build)

    // Math, SID_SM_EDITOPTIONS
    { SID_SM_TP_PRINTOPTIONS, "" }, // Settings

    // Calc, SID_SC_EDITOPTIONS
    { SID_SC_TP_LAYOUT, "" }, // General
    { RID_SC_TP_DEFAULTS, "" }, // Defaults
    { SID_SC_TP_CONTENT, "" }, // View
    { SID_SC_TP_CALC, "" }, // Calculate
    { SID_SC_TP_FORMULA, "" }, // Formula
    { SID_SC_TP_USERLISTS, "" }, // SortLists
    { SID_SC_TP_CHANGES, "" }, // Changes
    { SID_SC_TP_COMPATIBILITY, "" }, // Compatibility
    { SID_SC_TP_GRID, "" }, // Grid
    { RID_SC_TP_PRINT, "" }, // Print

    // Impress, SID_SD_EDITOPTIONS
    { SID_SI_TP_MISC, "" }, // General
    { SID_SI_TP_CONTENTS, "" }, // View
    { SID_SI_TP_SNAP, "" }, // Grid
    { SID_SI_TP_PRINT, "" }, // Print

    // Draw, SID_SD_GRAPHIC_OPTIONS
    { SID_SD_TP_MISC, "" }, // General
    { SID_SD_TP_CONTENTS, "" }, // View
    { SID_SD_TP_SNAP, "" }, // Grid
    { SID_SD_TP_PRINT, "" }, // Print

    // Charts, SID_SCH_EDITOPTIONS
    { RID_OPTPAGE_CHART_DEFCOLORS, "" }, // DefaultColor

    // Base, SID_SB_STARBASEOPTIONS
    { SID_SB_CONNECTIONPOOLING, "" }, // Connections
    { SID_SB_DBREGISTEROPTIONS, "" } // Databases
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
