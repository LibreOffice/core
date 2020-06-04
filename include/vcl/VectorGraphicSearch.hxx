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

#include <vcl/graph.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/dllapi.h>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include <memory>

enum class SearchStartPosition
{
    Begin,
    End
};

struct VCL_DLLPUBLIC VectorGraphicSearchOptions final
{
    SearchStartPosition meStartPosition;
    bool mbMatchCase;
    bool mbMatchWholeWord;

    VectorGraphicSearchOptions()
        : meStartPosition(SearchStartPosition::Begin)
        , mbMatchCase(false)
        , mbMatchWholeWord(false)
    {
    }

    VectorGraphicSearchOptions(SearchStartPosition eStartPosition, bool bMatchCase,
                               bool bMatchWholeWord)
        : meStartPosition(eStartPosition)
        , mbMatchCase(bMatchCase)
        , mbMatchWholeWord(bMatchWholeWord)
    {
    }
};

class VCL_DLLPUBLIC VectorGraphicSearch final
{
private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;
    Graphic maGraphic;

    bool searchPDF(std::shared_ptr<VectorGraphicData> const& rData);

public:
    VectorGraphicSearch(Graphic const& rGraphic);
    ~VectorGraphicSearch();
    bool search(OUString const& rSearchString,
                VectorGraphicSearchOptions const& rOptions = VectorGraphicSearchOptions());
    basegfx::B2DSize pageSize();
    bool next();
    bool previous();
    int index();
    std::vector<basegfx::B2DRectangle> getTextRectangles();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
