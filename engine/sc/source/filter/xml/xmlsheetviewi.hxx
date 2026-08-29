/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "importcontext.hxx"
#include "xmlsorti.hxx"

#include <address.hxx>
#include <queryparam.hxx>

#include <utility>
#include <vector>

namespace sax_fastparser
{
class FastAttributeList;
}

/** One coext:sheet-view element, kept until the whole document is read. */
struct ScXMLSheetViewData
{
    OUString maName;
    /// The sheet the view belongs to.
    OUString maTableName;
    OString maGUID;
    OString maFilterGUID;
    /// Runs of columns the view hides, as inclusive 0-based column indices.
    std::vector<std::pair<SCCOL, SCCOL>> maHiddenColumns;
    /// True when the view carries an auto-filter range, held in maQueryParam with its filter.
    bool mbHasFilterRange = false;
    /// The auto-filter range on the sheet the view belongs to, and the filter on it.
    ScQueryParam maQueryParam;
    /// True when a table:sort element was read into maSortSequence.
    bool mbHasSort = false;
    cpo::uno::Sequence<css::beans::PropertyValue> maSortSequence;
};

/** Reads coext:sheet-views. */
class ScXMLSheetViewsContext : public ScXMLImportContext
{
public:
    explicit ScXMLSheetViewsContext(ScXMLImport& rImport);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

/** Reads one coext:sheet-view and hands it to the import once the element is complete. */
class ScXMLSheetViewContext : public ScXMLImportContext, public ScXMLSortSequenceReceiver
{
    ScXMLSheetViewData maData;

public:
    ScXMLSheetViewContext(ScXMLImport& rImport,
                          const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void
    SetSortSequence(const cpo::uno::Sequence<css::beans::PropertyValue>& rSortSequence) override;

    void AddHiddenColumns(SCCOL nStartColumn, SCCOL nEndColumn);
};

/** Reads one coext:hidden-columns element into the sheet view it belongs to. */
class ScXMLSheetViewHiddenColumnsContext : public ScXMLImportContext
{
public:
    ScXMLSheetViewHiddenColumnsContext(
        ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScXMLSheetViewContext& rSheetView);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
