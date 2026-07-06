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

#include <comphelper/base64.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/propertyvalue.hxx>
#include <o3tl/enumrange.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <svx/svddef.hxx>
#include <svx/svdetc.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/virdev.hxx>

#include <svx/sdr/table/TableDesignPreview.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

namespace sdr::table
{
namespace
{
const sal_Int32 nPreviewColumns = 5;
const sal_Int32 nPreviewRows = 5;
const sal_Int32 nCellWidth = 12; // one pixel is shared with the next cell!
const sal_Int32 nCellHeight = 7; // one pixel is shared with the next cell!
const sal_Int32 nBitmapWidth = (nCellWidth * nPreviewColumns) - (nPreviewColumns - 1);
const sal_Int32 nBitmapHeight = (nCellHeight * nPreviewRows) - (nPreviewRows - 1);

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
typedef std::shared_ptr<CellInfo> CellInfoMatrix[nPreviewColumns * nPreviewRows];

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
    for (sal_Int32 nRow = 0; nRow < nPreviewRows; ++nRow)
    {
        const bool bFirstRow = rSettings.mbUseFirstRow && (nRow == 0);
        const bool bLastRow = rSettings.mbUseLastRow && (nRow == nPreviewRows - 1);

        for (sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol)
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
                else if (rSettings.mbUseLastColumn && (nCol == nPreviewColumns - 1))
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

            rMatrix[(nCol * nPreviewColumns) + nRow] = std::move(xCellInfo);
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

    // bbbbbbbbbbbb w = 12 pixel
    // bccccccccccb h = 7 pixel
    // bccccccccccb b = border color
    // bcttttttttcb c = cell color
    // bccccccccccb t = text color
    // bccccccccccb
    // bbbbbbbbbbbb

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
    Size aBmpSize(nBitmapWidth, nBitmapHeight);
    pVirDev->SetOutputSizePixel(aBmpSize);

    pVirDev->SetBackground(bIsPageDark ? COL_BLACK : COL_WHITE);
    pVirDev->Erase();

    // first draw cell background and text line previews
    sal_Int32 nY = 0;
    sal_Int32 nRow;
    for (nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight - 1)
    {
        sal_Int32 nX = 0;
        for (sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth - 1)
        {
            std::shared_ptr<CellInfo> xCellInfo(aMatrix[(nCol * nPreviewColumns) + nRow]);

            Color aTextColor(COL_AUTO);
            if (xCellInfo)
            {
                // fill cell background
                const ::tools::Rectangle aRect(nX, nY, nX + nCellWidth - 1, nY + nCellHeight - 1);

                if (xCellInfo->maCellColor != COL_TRANSPARENT)
                {
                    pVirDev->SetFillColor(xCellInfo->maCellColor);
                    pVirDev->DrawRect(aRect);
                }

                aTextColor = xCellInfo->maTextColor;
            }

            // draw text preview line
            if (aTextColor == COL_AUTO)
                aTextColor = bIsPageDark ? COL_WHITE : COL_BLACK;
            pVirDev->SetLineColor(aTextColor);
            const Point aPnt1(nX + 2, nY + ((nCellHeight - 1) >> 1));
            const Point aPnt2(nX + nCellWidth - 3, aPnt1.Y());
            pVirDev->DrawLine(aPnt1, aPnt2);
        }
    }

    // second draw border lines
    nY = 0;
    for (nRow = 0; nRow < nPreviewRows; ++nRow, nY += nCellHeight - 1)
    {
        sal_Int32 nX = 0;
        for (sal_Int32 nCol = 0; nCol < nPreviewColumns; ++nCol, nX += nCellWidth - 1)
        {
            std::shared_ptr<CellInfo> xCellInfo(aMatrix[(nCol * nPreviewColumns) + nRow]);

            if (xCellInfo)
            {
                const Point aPntTL(nX, nY);
                const Point aPntTR(nX + nCellWidth - 1, nY);
                const Point aPntBL(nX, nY + nCellHeight - 1);
                const Point aPntBR(nX + nCellWidth - 1, nY + nCellHeight - 1);

                sal_Int32 border_diffs[8] = { 0, -1, 0, 1, -1, 0, 1, 0 };
                sal_Int32* pDiff = &border_diffs[0];

                // draw top border
                for (SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>())
                {
                    const ::editeng::SvxBorderLine* pBorderLine
                        = xCellInfo->maBorder->GetLine(nLine);
                    if (!pBorderLine
                        || ((pBorderLine->GetOutWidth() == 0) && (pBorderLine->GetInWidth() == 0)))
                        continue;

                    sal_Int32 nBorderCol = nCol + *pDiff++;
                    sal_Int32 nBorderRow = nRow + *pDiff++;
                    if ((nBorderCol >= 0) && (nBorderCol < nPreviewColumns) && (nBorderRow >= 0)
                        && (nBorderRow < nPreviewRows))
                    {
                        // check border
                        std::shared_ptr<CellInfo> xBorderInfo(
                            aMatrix[(nBorderCol * nPreviewColumns) + nBorderRow]);
                        if (xBorderInfo)
                        {
                            const ::editeng::SvxBorderLine* pBorderLine2
                                = xBorderInfo->maBorder->GetLine(
                                    static_cast<SvxBoxItemLine>(static_cast<int>(nLine) ^ 1));
                            if (pBorderLine2 && pBorderLine2->HasPriority(*pBorderLine))
                                continue; // other border line wins
                        }
                    }

                    pVirDev->SetLineColor(pBorderLine->GetColor());
                    switch (nLine)
                    {
                        case SvxBoxItemLine::TOP:
                            pVirDev->DrawLine(aPntTL, aPntTR);
                            break;
                        case SvxBoxItemLine::BOTTOM:
                            pVirDev->DrawLine(aPntBL, aPntBR);
                            break;
                        case SvxBoxItemLine::LEFT:
                            pVirDev->DrawLine(aPntTL, aPntBL);
                            break;
                        case SvxBoxItemLine::RIGHT:
                            pVirDev->DrawLine(aPntTR, aPntBR);
                            break;
                    }
                }
            }
        }
    }

    return pVirDev->GetBitmap(Point(0, 0), aBmpSize);
}

OString CreateTableDesignPreviewDataUri(const Reference<XIndexAccess>& xTableStyle,
                                        const TableDesignPreviewSettings& rSettings,
                                        bool bIsPageDark)
{
    Bitmap aBitmap(CreateTableDesignPreview(xTableStyle, rSettings, bIsPageDark));

    SvMemoryStream aOStm(65535, 65535);
    // Use fastest compression - these previews are small and regenerated often.
    cpo::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Compression"_ustr, sal_Int32(1)),
    };
    vcl::PngImageWriter aPNGWriter(aOStm);
    aPNGWriter.setParameters(aFilterData);
    if (!aPNGWriter.write(aBitmap))
        return ""_ostr;

    cpo::uno::Sequence<sal_Int8> aSeq(static_cast<sal_Int8 const*>(aOStm.GetData()), aOStm.Tell());
    OStringBuffer aBuffer("data:image/png;base64,");
    ::comphelper::Base64::encode(aBuffer, aSeq);
    return aBuffer.makeStringAndClear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
