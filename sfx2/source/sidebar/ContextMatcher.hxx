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
#ifndef SFX_SIDEBAR_CONTEXT_MATCHER_HXX
#define SFX_SIDEBAR_CONTEXT_MATCHER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>


namespace sfx2 { namespace sidebar {

class Context;


/** Data read from the configuration for matching contexts.
*/
class ContextMatcher
{
public:
    ContextMatcher (void);
    ~ContextMatcher (void);

    sal_Int32 EvaluateMatch (
        const Context& rContext) const;

    void AddMatcher (
        const ::rtl::OUString& rsApplicationName,
        const ::std::vector<rtl::OUString>& rContextNames,
        const bool mbIsContextListNegated);
    void AddMatcher (
        const ::rtl::OUString& rsApplicationName,
        const ::rtl::OUString& rsContextName);

private:
    class Entry
    {
    public:
        ::rtl::OUString msApplicationName;
        ::std::vector<rtl::OUString> maContextNames;
        bool mbIsContextListNegated;
    };
    ::std::vector<Entry> maEntries;

    sal_Int32 EvaluateMatch (
        const Context& rContext,
        const Entry& rEntry) const;

};
static bool IsMatchBetterThan (const sal_Int32 nMatchA, const sal_Int32 nMatchB);


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
