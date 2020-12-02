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

#ifndef INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX
#define INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

#include <optional>

namespace com::sun::star::beans { struct PropertyValue; }

// The method GetList() returns a list of property values.
// Use follow defines to separate values by names.

#define HISTORY_PROPERTYNAME_URL            "URL"
#define HISTORY_PROPERTYNAME_FILTER         "Filter"
#define HISTORY_PROPERTYNAME_TITLE          "Title"
#define HISTORY_PROPERTYNAME_PASSWORD       "Password"
#define HISTORY_PROPERTYNAME_THUMBNAIL      "Thumbnail"

/// You can use these enum values to specify right history if you call our interface methods.
enum class EHistoryType
{
    PickList,
    HelpBookmarks
};

class SvtHistoryOptions_Impl;

/** Collect information about history features.

    Interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."

    key "PickList": The last used documents displayed in the file menu.
    key "History":  The last opened documents general.
*/
class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtHistoryOptions final : public utl::detail::Options
{
public:
    SvtHistoryOptions();
    virtual ~SvtHistoryOptions() override;

    /** Clear complete specified list.

        @param      eHistory select right history.
    */
    void Clear(EHistoryType eHistory);

    /** Return the complete specified history list.

        @param  eHistory select right history.
        @return A list of history items is returned.
    */
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > GetList(EHistoryType eHistory) const;

    /** Append a new item to the specified list.

        The oldest entry is deleted automatically when the size reaches the maximum.

        @param eHistory  select right history.
        @param sURL      URL to save in history
        @param sFilter   filter name to save in history
        @param sTitle    document title to save in history
    */
    void AppendItem(EHistoryType eHistory,
            const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
            const std::optional<OUString>& sThumbnail);

    /** Delete item from the specified list.
    */
    void DeleteItem(EHistoryType eHistory, const OUString& sURL);

private:
    std::shared_ptr<SvtHistoryOptions_Impl> m_pImpl;
};

#endif // INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
