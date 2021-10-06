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

#include <unotools/fontdefs.hxx>

#include <font/DirectFontSubstitution.hxx>

#include <svdata.hxx>

#include <string>
#include <vector>

namespace vcl::font
{
void DirectFontSubstitution::AddFontSubstitute(const OUString& rFontName,
                                               const OUString& rSubstFontName,
                                               AddFontSubstituteFlags nFlags)
{
    maFontSubstList.emplace_back(rFontName, rSubstFontName, nFlags);
}

void DirectFontSubstitution::RemoveFontsSubstitute() { maFontSubstList.clear(); }

bool DirectFontSubstitution::FindFontSubstitute(OUString& rSubstName,
                                                std::u16string_view rSearchName) const
{
    // TODO: get rid of O(N) searches
    std::vector<FontSubstEntry>::const_iterator it = std::find_if(
        maFontSubstList.begin(), maFontSubstList.end(), [&](const FontSubstEntry& s) {
            return (s.mnFlags & AddFontSubstituteFlags::ALWAYS) && (s.maSearchName == rSearchName);
        });
    if (it != maFontSubstList.end())
    {
        rSubstName = it->maSearchReplaceName;
        return true;
    }
    return false;
}

void ImplFontSubstitute(OUString& rFontName)
{
    // must be canonicalised
    assert(GetEnglishSearchFontName(rFontName) == rFontName);
    OUString aSubstFontName;
    // apply user-configurable font replacement (eg, from the list in Tools->Options)
    const DirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if (pSubst && pSubst->FindFontSubstitute(aSubstFontName, rFontName))
    {
        rFontName = aSubstFontName;
        return;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
