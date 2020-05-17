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

class SearchContext;

class VCL_DLLPUBLIC VectorGraphicSearch final
{
private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;
    Graphic maGraphic;
    std::unique_ptr<SearchContext> mpSearchContext;

    bool searchPDF(std::shared_ptr<VectorGraphicData> const& rData, OUString const& rSearchString);

public:
    VectorGraphicSearch(Graphic const& rGraphic);
    ~VectorGraphicSearch();
    bool search(OUString const& rSearchString);
    basegfx::B2DSize pageSize();
    bool next();
    int index();
    std::vector<basegfx::B2DRectangle> getTextRectangles();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
