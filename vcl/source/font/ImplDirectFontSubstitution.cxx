/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <rtl/ustring.hxx>

#include <font/ImplDirectFontSubstitution.hxx>
#include <font/ImplFontSubstEntry.hxx>

void ImplDirectFontSubstitution::AddFontSubstitute(const OUString& rFontName,
                                                   const OUString& rSubstFontName,
                                                   AddFontSubstituteFlags nFlags)
{
    maFontSubstList.emplace_back(rFontName, rSubstFontName, nFlags);
}

void ImplDirectFontSubstitution::RemoveFontsSubstitute() { maFontSubstList.clear(); }

bool ImplDirectFontSubstitution::FindFontSubstitute(OUString& rSubstName,
                                                    std::u16string_view rSearchName) const
{
    // TODO: get rid of O(N) searches
    std::vector<ImplFontSubstEntry>::const_iterator it = std::find_if(
        maFontSubstList.begin(), maFontSubstList.end(), [&](const ImplFontSubstEntry& s) {
            return (s.mnFlags & AddFontSubstituteFlags::ALWAYS) && (s.maSearchName == rSearchName);
        });
    if (it != maFontSubstList.end())
    {
        rSubstName = it->maSearchReplaceName;
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
