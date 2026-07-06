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

#include <svx/svxdllapi.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <rtl/string.hxx>
#include <vcl/bitmap.hxx>

namespace sdr::table
{
/// Which named style element applies to each cell position when rendering
/// a table design preview: the first/last row or column, or an alternating
/// row/column band, falling back to the style's body element.
struct TableDesignPreviewSettings
{
    bool mbUseFirstRow;
    bool mbUseLastRow;
    bool mbUseFirstColumn;
    bool mbUseLastColumn;
    bool mbUseRowBanding;
    bool mbUseColumnBanding;

    TableDesignPreviewSettings()
        : mbUseFirstRow(true)
        , mbUseLastRow(false)
        , mbUseFirstColumn(false)
        , mbUseLastColumn(false)
        , mbUseRowBanding(true)
        , mbUseColumnBanding(false)
    {
    }
};

/// Renders a small grid bitmap previewing one named table design style,
/// used both by the sidebar's Table Design panel and by the notebookbar's
/// Table Design gallery toolbar item.
SVXCORE_DLLPUBLIC Bitmap
CreateTableDesignPreview(const css::uno::Reference<css::container::XIndexAccess>& xTableStyle,
                         const TableDesignPreviewSettings& rSettings, bool bIsPageDark);

/// Same preview, encoded as a "data:image/png;base64,..." string, for
/// embedding directly in JSON sent to the Online browser.
SVXCORE_DLLPUBLIC OString CreateTableDesignPreviewDataUri(
    const css::uno::Reference<css::container::XIndexAccess>& xTableStyle,
    const TableDesignPreviewSettings& rSettings, bool bIsPageDark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
