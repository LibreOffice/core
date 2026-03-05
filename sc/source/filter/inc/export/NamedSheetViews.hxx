/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <xerecord.hxx>
#include <xeroot.hxx>
#include <sax/fshelper.hxx>
#include <types.hxx>

struct ScQueryParam;
struct ScSortParam;
namespace sc
{
class SheetView;
}

namespace xcl::exp
{
/** Export of named sheet views for a single sheet. */
class NamedSheetViews : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit NamedSheetViews(const XclExpRoot& rRoot, SCTAB nTab);

    virtual void SaveXml(XclExpXmlStream& rStream) override;

private:
    void saveSheetView(const sax_fastparser::FSHelperPtr& pStream, const sc::SheetView& rSheetView);
    void saveColumnFilters(const sax_fastparser::FSHelperPtr& pStream,
                           const ScQueryParam& rQueryParam);
    void saveSortRules(const sax_fastparser::FSHelperPtr& pStream, const ScRange& rRange,
                       SCTAB nViewTab, const ScSortParam& rSortParam);

    SCTAB mnTab;
    bool mbHasSheetViews;
};

} // end namespace xcl::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
