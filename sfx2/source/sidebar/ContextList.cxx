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
#include <sfx2/sidebar/ContextList.hxx>
#include <sfx2/sidebar/Context.hxx>

namespace sfx2 { namespace sidebar {

ContextList::ContextList()
    : maEntries()
{
}

const ContextList::Entry* ContextList::GetMatch (const Context& rContext) const
{
    const ::std::vector<Entry>::const_iterator iEntry = FindBestMatch(rContext);
    if (iEntry != maEntries.end())
        return &*iEntry;
    else
        return nullptr;
}

ContextList::Entry* ContextList::GetMatch (const Context& rContext)
{
    const ::std::vector<Entry>::const_iterator iEntry = FindBestMatch(rContext);
    if (iEntry != maEntries.end())
        return const_cast<Entry*>(&*iEntry);
    else
        return nullptr;
}

::std::vector<ContextList::Entry>::const_iterator ContextList::FindBestMatch (const Context& rContext) const
{
    sal_Int32 nBestMatch (Context::NoMatch);
    ::std::vector<Entry>::const_iterator iBestMatch (maEntries.end());

    for (::std::vector<Entry>::const_iterator
             iEntry(maEntries.begin()),
             iEnd(maEntries.end());
         iEntry!=iEnd;
         ++iEntry)
    {
        const sal_Int32 nMatch (rContext.EvaluateMatch(iEntry->maContext));
        if (nMatch < nBestMatch)
        {
            nBestMatch = nMatch;
            iBestMatch = iEntry;
        }
        if (nBestMatch == Context::OptimalMatch)
            return iEntry;
    }

    return iBestMatch;
}

void ContextList::AddContextDescription (
    const Context& rContext,
    const bool bIsInitiallyVisible,
    const OUString& rsMenuCommand)
{
    maEntries.emplace_back();
    maEntries.back().maContext = rContext;
    maEntries.back().mbIsInitiallyVisible = bIsInitiallyVisible;
    maEntries.back().msMenuCommand = rsMenuCommand;
}

void ContextList::ToggleVisibilityForContext( const Context &rContext, const bool bVisible)
{
    ContextList::Entry *pEntry = GetMatch( rContext );

    if ( !pEntry )
        return;

    const sal_Int32 nMatch( rContext.EvaluateMatch( pEntry->maContext ) );

    if ( nMatch & Context::ApplicationWildcardMatch )
    {
        // Create a separate context list entry for this app if 'any'
        // is the only context that matches. Toggling the visibility
        // for 'any' would change it for all apps, not just this one
        AddContextDescription( rContext, bVisible, OUString() );
    }
    else if ( nMatch == Context::OptimalMatch || nMatch == Context::ContextWildcardMatch )
        pEntry->mbIsInitiallyVisible = bVisible;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
