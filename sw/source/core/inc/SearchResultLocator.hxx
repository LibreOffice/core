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

namespace sw::search
{
enum class NodeType
{
    Undefined,
    WriterNode,
    SdrObject
};

struct SearchIndexData
{
    NodeType meType = NodeType::Undefined;
    sal_uInt32 mnNodeIndex = 0;
    OUString maObjectName;

    SearchIndexData() {}

    SearchIndexData(NodeType eType, sal_uInt32 nNodeIndex, OUString const& aObjectName = OUString())
        : meType(eType)
        , mnNodeIndex(nNodeIndex)
        , maObjectName(aObjectName)
    {
    }
};

struct LocationResult
{
    bool mbFound = false;
    std::vector<basegfx::B2DRange> maRectangles;
};

class SW_DLLPUBLIC SearchResultLocator
{
    SwDoc* mpDocument;

    void findOne(LocationResult& rResult, SearchIndexData const& rSearchIndexData);

public:
    SearchResultLocator(SwDoc* pDoc)
        : mpDocument(pDoc)
    {
    }

    LocationResult find(std::vector<SearchIndexData> const& rSearchIndexDataVector);
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
