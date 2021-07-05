/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <swdllapi.h>
#include <doc.hxx>
#include <basegfx/range/b2drange.hxx>

namespace sw
{
struct SearchIndexData
{
    sal_uInt32 nNodeIndex;
};

struct LocationResult
{
    bool mbFound = false;
    std::vector<basegfx::B2DRange> maRectangles;
};

class SW_DLLPUBLIC SearchResultLocator
{
    SwDoc* mpDocument;

public:
    SearchResultLocator(SwDoc* pDoc)
        : mpDocument(pDoc)
    {
    }

    LocationResult find(SearchIndexData const& rSearchIndexData);
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
