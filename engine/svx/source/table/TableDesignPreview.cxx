/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <com/sun/star/style/XStyle.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <svx/svddef.hxx>
#include <svx/svdetc.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <tools/debug.hxx>

#include <svx/sdr/table/TableDesignPreview.hxx>
#include <svx/sdr/table/TableStylePreviewPaint.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

namespace sdr::table
{
namespace
{
struct CellInfo
{
    Color maCellColor;
    Color maTextColor;
    std::shared_ptr<SvxBoxItem> maBorder;

    explicit CellInfo(const Reference<XStyle>& xStyle);
};

CellInfo::CellInfo(const Reference<XStyle>& xStyle)
    : maBorder(std::make_shared<SvxBoxItem>(SDRATTR_TABLE_BORDER))
{
    SfxStyleSheet* pStyleSheet = SfxUnoStyleSheet::getUnoStyleSheet(xStyle);
    if (!pStyleSheet)
        return;

    SfxItemSet& rSet = pStyleSheet->GetItemSet();

    // get style fill color
    maCellColor = GetDraftFillColor(rSet).value_or(COL_TRANSPARENT);

    // get style text color
    const SvxColorItem* pTextColor = rSet.GetItem(EE_CHAR_COLOR);
    if (pTextColor)
        maTextColor = pTextColor->GetValue();
    else
        maTextColor = COL_TRANSPARENT;

    // get border
    const SvxBoxItem* pBoxItem = rSet.GetItem(SDRATTR_TABLE_BORDER);
    if (pBoxItem)
        maBorder.reset(pBoxItem->Clone());
}

typedef std::vector<std::shared_ptr<CellInfo>> CellInfoVector;
typedef std::shared_ptr<CellInfo>
    CellInfoMatrix[nTableStylePreviewColumns * nTableStylePreviewRows];

void FillCellInfoVector(const Reference<XIndexAccess>& xTableStyle, CellInfoVector& rVector)
{
    DBG_ASSERT(xTableStyle.is() && (xTableStyle->getCount() == style_count),
               "sdr::table::FillCellInfoVector(), invalid table style!");
    if (!xTableStyle.is())
        return;

    try
    {
        rVector.resize(style_count);

        for (sal_Int32 nStyle = 0; nStyle < style_count; ++nStyle)
        {
            Reference<XStyle> xStyle(xTableStyle->getByIndex(nStyle), UNO_QUERY);
            if (xStyle.is())
                rVector[nStyle] = std::make_shared<CellInfo>(xStyle);
        }
    }
    catch (cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx.table", "sdr::table::FillCellInfoVector()");
    }
}

void FillCellInfoMatrix(const CellInfoVector& rStyle, const TableDesignPreviewSettings& rSettings,
                        CellInfoMatrix& rMatrix)
{
    for (sal_Int32 nRow = 0; nRow < nTableStylePreviewRows; ++nRow)
    {
        const bool bFirstRow = rSettings.mbUseFirstRow && (nRow == 0);
        const bool bLastRow = rSettings.mbUseLastRow && (nRow == nTableStylePreviewRows - 1);

        for (sal_Int32 nCol = 0; nCol < nTableStylePreviewColumns; ++nCol)
        {
            std::shared_ptr<CellInfo> xCellInfo;

            // first and last row win first, if used and available
            if (bFirstRow)
            {
                xCellInfo = rStyle[first_row_style];
            }
            else if (bLastRow)
            {
                xCellInfo = rStyle[last_row_style];
            }

            if (!xCellInfo)
            {
                // next come first and last column, if used and available
                if (rSettings.mbUseFirstColumn && (nCol == 0))
                {
                    xCellInfo = rStyle[first_column_style];
                }
                else if (rSettings.mbUseLastColumn && (nCol == nTableStylePreviewColumns - 1))
                {
                    xCellInfo = rStyle[last_column_style];
                }
            }

            if (!xCellInfo)
            {
                if (rSettings.mbUseRowBanding)
                {
                    if ((nRow & 1) == 0)
                    {
                        xCellInfo = rStyle[even_rows_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[odd_rows_style];
                    }
                }
            }

            if (!xCellInfo)
            {
                if (rSettings.mbUseColumnBanding)
                {
                    if ((nCol & 1) == 0)
                    {
                        xCellInfo = rStyle[even_columns_style];
                    }
                    else
                    {
                        xCellInfo = rStyle[odd_columns_style];
                    }
                }
            }

            if (!xCellInfo)
            {
                // use default cell style if non found yet
                xCellInfo = rStyle[body_style];
            }

            rMatrix[(nCol * nTableStylePreviewColumns) + nRow] = std::move(xCellInfo);
        }
    }
}
}

Bitmap CreateTableDesignPreview(const Reference<XIndexAccess>& xTableStyle,
                                const TableDesignPreviewSettings& rSettings, bool bIsPageDark)
{
    CellInfoVector aCellInfoVector(style_count);
    FillCellInfoVector(xTableStyle, aCellInfoVector);

    CellInfoMatrix aMatrix;
    FillCellInfoMatrix(aCellInfoVector, rSettings, aMatrix);

    return PaintTableStylePreview(
        [&aMatrix](sal_Int32 nRow, sal_Int32 nCol) -> TableStylePreviewCell
        {
            const std::shared_ptr<CellInfo>& xCellInfo
                = aMatrix[(nCol * nTableStylePreviewColumns) + nRow];
            if (!xCellInfo)
                return {};
            return { xCellInfo->maCellColor, xCellInfo->maTextColor, xCellInfo->maBorder.get() };
        },
        bIsPageDark);
}

OString CreateTableDesignPreviewDataUri(const Reference<XIndexAccess>& xTableStyle,
                                        const TableDesignPreviewSettings& rSettings,
                                        bool bIsPageDark)
{
    return EncodeTableStylePreviewDataUri(
        CreateTableDesignPreview(xTableStyle, rSettings, bIsPageDark));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
