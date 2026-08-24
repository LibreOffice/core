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

#include <svx/sdr/table/TableStylePreviewPaint.hxx>

#include <comphelper/base64.hxx>
#include <comphelper/propertyvalue.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <o3tl/enumrange.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/virdev.hxx>

namespace sdr::table
{
namespace
{
const sal_Int32 nCellWidth = 12; // one pixel is shared with the next cell!
const sal_Int32 nCellHeight = 7; // one pixel is shared with the next cell!
const sal_Int32 nBitmapWidth
    = (nCellWidth * nTableStylePreviewColumns) - (nTableStylePreviewColumns - 1);
const sal_Int32 nBitmapHeight
    = (nCellHeight * nTableStylePreviewRows) - (nTableStylePreviewRows - 1);
}

Bitmap PaintTableStylePreview(
    const std::function<TableStylePreviewCell(sal_Int32 nRow, sal_Int32 nCol)>& rResolveCell,
    bool bIsPageDark)
{
    // bbbbbbbbbbbb w = 12 pixel
    // bccccccccccb h = 7 pixel
    // bccccccccccb b = border color
    // bcttttttttcb c = cell color
    // bccccccccccb t = text color
    // bccccccccccb
    // bbbbbbbbbbbb

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
    const Size aBmpSize(nBitmapWidth, nBitmapHeight);
    pVirDev->SetOutputSizePixel(aBmpSize);

    pVirDev->SetBackground(bIsPageDark ? COL_BLACK : COL_WHITE);
    pVirDev->Erase();

    // first draw cell background and text line previews
    sal_Int32 nY = 0;
    for (sal_Int32 nRow = 0; nRow < nTableStylePreviewRows; ++nRow, nY += nCellHeight - 1)
    {
        sal_Int32 nX = 0;
        for (sal_Int32 nCol = 0; nCol < nTableStylePreviewColumns; ++nCol, nX += nCellWidth - 1)
        {
            const TableStylePreviewCell aCell = rResolveCell(nRow, nCol);

            const ::tools::Rectangle aRect(nX, nY, nX + nCellWidth - 1, nY + nCellHeight - 1);
            if (aCell.aBackColor != COL_TRANSPARENT)
            {
                pVirDev->SetFillColor(aCell.aBackColor);
                pVirDev->DrawRect(aRect);
            }

            Color aTextColor(aCell.aTextColor);
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
    for (sal_Int32 nRow = 0; nRow < nTableStylePreviewRows; ++nRow, nY += nCellHeight - 1)
    {
        sal_Int32 nX = 0;
        for (sal_Int32 nCol = 0; nCol < nTableStylePreviewColumns; ++nCol, nX += nCellWidth - 1)
        {
            const SvxBoxItem* pBorder = rResolveCell(nRow, nCol).pBorder;
            if (!pBorder)
                continue;

            const Point aPntTL(nX, nY);
            const Point aPntTR(nX + nCellWidth - 1, nY);
            const Point aPntBL(nX, nY + nCellHeight - 1);
            const Point aPntBR(nX + nCellWidth - 1, nY + nCellHeight - 1);

            sal_Int32 aBorderDiffs[8] = { 0, -1, 0, 1, -1, 0, 1, 0 };
            sal_Int32* pDiff = &aBorderDiffs[0];

            for (SvxBoxItemLine nLine : o3tl::enumrange<SvxBoxItemLine>())
            {
                const ::editeng::SvxBorderLine* pBorderLine = pBorder->GetLine(nLine);
                if (!pBorderLine
                    || ((pBorderLine->GetOutWidth() == 0) && (pBorderLine->GetInWidth() == 0)))
                    continue;

                const sal_Int32 nBorderCol = nCol + *pDiff++;
                const sal_Int32 nBorderRow = nRow + *pDiff++;
                if ((nBorderCol >= 0) && (nBorderCol < nTableStylePreviewColumns)
                    && (nBorderRow >= 0) && (nBorderRow < nTableStylePreviewRows))
                {
                    // check whether the neighbouring cell's border on the shared edge wins
                    const SvxBoxItem* pNeighbourBorder = rResolveCell(nBorderRow, nBorderCol).pBorder;
                    const ::editeng::SvxBorderLine* pBorderLine2
                        = pNeighbourBorder
                              ? pNeighbourBorder->GetLine(
                                    static_cast<SvxBoxItemLine>(static_cast<int>(nLine) ^ 1))
                              : nullptr;
                    if (pBorderLine2 && pBorderLine2->HasPriority(*pBorderLine))
                        continue; // other border line wins
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

    return pVirDev->GetBitmap(Point(0, 0), aBmpSize);
}

OString EncodeTableStylePreviewDataUri(const Bitmap& rBitmap)
{
    SvMemoryStream aOStm(65535, 65535);
    // Use fastest compression - these previews are small and regenerated often.
    cpo::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Compression"_ustr, sal_Int32(1)),
    };
    vcl::PngImageWriter aPNGWriter(aOStm);
    aPNGWriter.setParameters(aFilterData);
    if (!aPNGWriter.write(rBitmap))
        return ""_ostr;

    cpo::uno::Sequence<sal_Int8> aSeq(static_cast<sal_Int8 const*>(aOStm.GetData()), aOStm.Tell());
    OStringBuffer aBuffer("data:image/png;base64,");
    ::comphelper::Base64::encode(aBuffer, aSeq);
    return aBuffer.makeStringAndClear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
