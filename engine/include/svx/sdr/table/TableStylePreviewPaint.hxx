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
#include <rtl/string.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>

#include <functional>

class SvxBoxItem;

namespace sdr::table
{
/// Fixed sample-grid dimensions every table style preview renders at.
constexpr sal_Int32 nTableStylePreviewColumns = 5;
constexpr sal_Int32 nTableStylePreviewRows = 5;

/// One resolved cell's paint inputs for a table style preview: its background fill, the
/// color its sample text line draws in (COL_AUTO picks black or white for contrast against
/// the page), and the border lines around it. A null border paints no border for that cell.
struct TableStylePreviewCell
{
    Color aBackColor = COL_TRANSPARENT;
    Color aTextColor = COL_AUTO;
    const SvxBoxItem* pBorder = nullptr;
};

/// Paints the nTableStylePreviewRows x nTableStylePreviewColumns sample grid every table
/// style preview shares, asking rResolveCell for each cell's paint inputs.
SVXCORE_DLLPUBLIC Bitmap PaintTableStylePreview(
    const std::function<TableStylePreviewCell(sal_Int32 nRow, sal_Int32 nCol)>& rResolveCell,
    bool bIsPageDark);

/// Encodes a table style preview bitmap as a "data:image/png;base64,..." string, for
/// embedding directly in JSON sent to the Online browser.
SVXCORE_DLLPUBLIC OString EncodeTableStylePreviewDataUri(const Bitmap& rBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
