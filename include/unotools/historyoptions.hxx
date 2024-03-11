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

#include <config_options.h>
#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <optional>

/// You can use these enum values to specify right history if you call our interface methods.
enum class EHistoryType
{
    PickList,
    HelpBookmarks
};

/** Collect information about history features.

    Interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."

    key "PickList": The last used documents displayed in the file menu.
    key "History":  The last opened documents general.
*/
namespace SvtHistoryOptions
{

    /** Clear complete specified list.

        @param      eHistory select right history.
    */
    UNLESS_MERGELIBS_MORE(UNOTOOLS_DLLPUBLIC) void Clear(EHistoryType eHistory, const bool bClearPinnedItems = true);

    /** Return the complete specified history list.

        @param  eHistory select right history.
        @return A list of history items is returned.
    */
    struct HistoryItem
    {
        OUString sURL;
        OUString sFilter;
        OUString sTitle;
        OUString sThumbnail;
        bool isReadOnly = false;
        bool isPinned = false;
    };
    UNLESS_MERGELIBS_MORE(UNOTOOLS_DLLPUBLIC) std::vector< HistoryItem > GetList(EHistoryType eHistory);

    /** Append a new item to the specified list.

        The oldest entry is deleted automatically when the size reaches the maximum.

        @param eHistory    select right history.
        @param sURL        URL to save in history
        @param sFilter     filter name to save in history
        @param sTitle      document title to save in history
        @param sThumbnail  base64 encoded thumbnail of the item
        @param oIsReadOnly item was opened editable or read-only
    */
    UNLESS_MERGELIBS_MORE(UNOTOOLS_DLLPUBLIC) void AppendItem(EHistoryType eHistory, const OUString& sURL,
                                       const OUString& sFilter, const OUString& sTitle,
                                       const std::optional<OUString>& sThumbnail,
                                       std::optional<bool> oIsReadOnly);

    /** Delete item from the specified list.
    */
    UNLESS_MERGELIBS_MORE(UNOTOOLS_DLLPUBLIC) void DeleteItem(EHistoryType eHistory, const OUString& sURL,
                                       const bool bClearPinned = true);

    // tdf#38742 - toggle pinned state of an item
    UNLESS_MERGELIBS_MORE(UNOTOOLS_DLLPUBLIC) void TogglePinItem(EHistoryType eHistory, const OUString& sURL);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
