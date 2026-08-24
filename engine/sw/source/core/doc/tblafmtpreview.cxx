/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tblafmtpreview.hxx>

#include <tblafmt.hxx>
#include <swtable.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/sdr/table/TableStylePreviewPaint.hxx>
#include <tools/color.hxx>

namespace sw
{
namespace
{
/// Which box format position a cell in the preview grid resolves to, given the settings.
sal_uInt8 GetPreviewCellPos(sal_Int32 nRow, sal_Int32 nCol, const SwTableStyleSettings& rSettings)
{
    const sal_uInt8 nRowRole = SwTableAutoFormat::GetTableStyleRowRole(
            nRow, sdr::table::nTableStylePreviewRows, rSettings);
    const sal_uInt8 nColRole = SwTableAutoFormat::GetTableStyleColRole(
            nCol, sdr::table::nTableStylePreviewColumns, rSettings);
    return static_cast<sal_uInt8>(nRowRole * SwTableAutoFormat::nRoleCount + nColRole);
}
}

OString CreateTableStylePreviewDataUri(const SwTableAutoFormat& rStyle,
                                       const SwTableStyleSettings& rSettings, bool bIsPageDark)
{
    const Bitmap aBitmap = sdr::table::PaintTableStylePreview(
        [&rStyle, &rSettings](sal_Int32 nRow, sal_Int32 nCol) -> sdr::table::TableStylePreviewCell
        {
            const SwAutoFormatProps& rProps
                = rStyle.GetBoxFormat(GetPreviewCellPos(nRow, nCol, rSettings)).GetProps();
            return { rProps.GetBackground().GetColor(), rProps.GetColor().GetValue(),
                     &rProps.GetBox() };
        },
        bIsPageDark);

    return sdr::table::EncodeTableStylePreviewDataUri(aBitmap);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
