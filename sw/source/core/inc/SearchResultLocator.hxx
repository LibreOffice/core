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
    Undefined = 0,
    WriterNode = 1,
    CommonNode = 2 // node in a SdrObject
};

struct SearchIndexData
{
    NodeType meType = NodeType::Undefined;
    SwNodeOffset mnNodeIndex{ 0 };
    OUString maObjectName;

    SearchIndexData() {}

    SearchIndexData(NodeType eType, SwNodeOffset nNodeIndex,
                    OUString const& aObjectName = OUString())
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
    static bool tryParseJSON(const char* pPayload,
                             std::vector<sw::search::SearchIndexData>& rDataVector);
    static bool tryParseXML(const char* pPayload,
                            std::vector<sw::search::SearchIndexData>& rDataVector);

public:
    SearchResultLocator(SwDoc* pDoc)
        : mpDocument(pDoc)
    {
    }

    LocationResult find(std::vector<SearchIndexData> const& rSearchIndexDataVector);
    LocationResult findForPayload(const char* pPayload);
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
