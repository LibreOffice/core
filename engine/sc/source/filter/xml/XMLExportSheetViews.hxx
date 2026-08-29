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

#include <types.hxx>
#include <rtl/ustring.hxx>

class ScDocument;
class ScXMLExport;
namespace sc
{
class SheetView;
}

/** Writes the sheet views of a document as the coext:sheet-views element.
 *
 *  A sheet view lives in a hidden holder table that is not written. What goes into the file is
 *  the view's identity, the columns it hides, and the filter and sort of its auto-filter range,
 *  all against the sheet the view belongs to.
 */
class ScXMLExportSheetViews
{
    ScXMLExport& mrExport;

    void writeSheetView(ScDocument& rDoc, SCTAB nTab, const OUString& rTableName,
                        const sc::SheetView& rView);

public:
    explicit ScXMLExportSheetViews(ScXMLExport& rExport);

    /** Writes one coext:sheet-view per sheet view, or nothing when the document has none. */
    void WriteSheetViews(ScDocument& rDoc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
