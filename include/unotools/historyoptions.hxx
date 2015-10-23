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

#include <boost/optional.hpp>
#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

// The method GetList() returns a list of property values.
// Use follow defines to separate values by names.

#define HISTORY_PROPERTYNAME_URL            "URL"
#define HISTORY_PROPERTYNAME_FILTER         "Filter"
#define HISTORY_PROPERTYNAME_TITLE          "Title"
#define HISTORY_PROPERTYNAME_PASSWORD       "Password"
#define HISTORY_PROPERTYNAME_THUMBNAIL      "Thumbnail"

/// You can use these enum values to specify right history if you call our interface methods.
enum EHistoryType
{
    ePICKLIST       = 0,
    eHELPBOOKMARKS  = 1
};

class SvtHistoryOptions_Impl;

/** Collect information about history features.

    Interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."

    key "PickList": The last used documents displayed in the file menu.
    key "History":  The last opened documents general.
*/
class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtHistoryOptions : public utl::detail::Options
{
public:
    SvtHistoryOptions();
    virtual ~SvtHistoryOptions();

    /** Get max size of specified history.

        Call this methods to get information about max. size of specified list.
        If a new one is add to it the oldest one is deleted automatically.

        @param  eHistory select right history.
        @return Current max size of specified list.
    */
    sal_uInt32 GetSize(EHistoryType eHistory) const;

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
        @param sPassword password to save in history
    */
    void AppendItem(EHistoryType eHistory,
            const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
            const OUString& sPassword, const boost::optional<OUString>& sThumbnail);

    /** Delete item from the specified list.
    */
    void DeleteItem(EHistoryType eHistory, const OUString& sURL);

private:

    /* Attention

        Don't initialize these static members in these headers!
        a) Double defined symbols will be detected ...
        b) and unresolved externals exist at linking time.
        Do it in your source only.
     */

    static SvtHistoryOptions_Impl* m_pDataContainer;
    static sal_Int32               m_nRefCount;
};

#endif // INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
