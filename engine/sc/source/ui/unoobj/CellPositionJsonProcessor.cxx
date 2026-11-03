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

#include <CellPositionJsonProcessor.hxx>

#include <document.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <rtl/math.hxx>
#include <rtl/strbuf.hxx>
#include <tools/json_writer.hxx>

#include <algorithm>

namespace sc
{
CellAxis::CellAxis(const ScDocument& rDocument, SCTAB nTab, bool bRows, SCCOLROW nOrigin)
    : mrDocument(rDocument)
    , mnTab(nTab)
    , mbRows(bRows)
    , mnOrigin(nOrigin)
    , mnLast(bRows ? rDocument.MaxRow() : rDocument.MaxCol())
    , maForward{ 0.0 }
{
    if (mbRows)
    {
        mfEnd = mrDocument.GetRowHeight(mnOrigin, mnLast, mnTab, /*bHiddenAsZero*/ true);
        mfStart = mnOrigin > 0
                      ? -double(mrDocument.GetRowHeight(0, mnOrigin - 1, mnTab, true))
                      : 0.0;
    }
    else
    {
        mfEnd = mrDocument.GetColWidth(SCCOL(mnOrigin), SCCOL(mnLast), mnTab);
        mfStart = mnOrigin > 0
                      ? -double(mrDocument.GetColWidth(SCCOL(0), SCCOL(mnOrigin - 1), mnTab))
                      : 0.0;
    }
}

double CellAxis::sizeOf(SCCOLROW nIndex) const
{
    if (mbRows)
        return mrDocument.GetRowHeight(nIndex, mnTab, /*bHiddenAsZero*/ true);
    return mrDocument.GetColWidth(SCCOL(nIndex), mnTab, /*bHiddenAsZero*/ true);
}

std::pair<SCCOLROW, double> CellAxis::map(double fPosition)
{
    if (fPosition < mfStart)
        return { 0, fPosition - mfStart };
    if (fPosition >= mfEnd)
        return { mnLast, fPosition - (mfEnd - sizeOf(mnLast)) };

    if (fPosition >= 0)
    {
        // The starts run on until one lies past the position, so the cell before it holds it.
        // Cells of size 0 share their start with the next cell, which takes the position.
        while (maForward.back() <= fPosition)
        {
            const SCCOLROW nIndex = mnOrigin + SCCOLROW(maForward.size()) - 1;
            if (nIndex > mnLast)
                break;
            maForward.push_back(maForward.back() + sizeOf(nIndex));
        }
        const auto aNext = std::upper_bound(maForward.begin(), maForward.end(), fPosition);
        const size_t nCell = size_t(aNext - maForward.begin()) - 1;
        return std::make_pair(mnOrigin + SCCOLROW(nCell), fPosition - maForward[nCell]);
    }

    // Before the origin the starts go back from it, so they fall.
    while (maBackward.empty() || maBackward.back() > fPosition)
    {
        const SCCOLROW nIndex = mnOrigin - 1 - SCCOLROW(maBackward.size());
        if (nIndex < 0)
            break;
        const double fEnd = maBackward.empty() ? 0.0 : maBackward.back();
        maBackward.push_back(fEnd - sizeOf(nIndex));
    }
    const auto aCell = std::find_if(maBackward.begin(), maBackward.end(),
                                    [fPosition](double fStart) { return fStart <= fPosition; });
    if (aCell == maBackward.end())
        return { 0, fPosition - mfStart };
    const size_t nCell = size_t(aCell - maBackward.begin());
    return std::make_pair(mnOrigin - 1 - SCCOLROW(nCell), fPosition - *aCell);
}

namespace
{
OString offset(double fOffset) { return OString::number(rtl::math::round(fOffset, 2)); }
}

CellPositionJsonProcessor::CellPositionJsonProcessor(tools::JsonWriter& rWriter,
                                                     const ScDocument& rDocument, SCTAB nTab,
                                                     SCCOL nOriginColumn, SCROW nOriginRow)
    : Primitive2dJsonProcessor(rWriter)
    , maColumns(rDocument, nTab, false, nOriginColumn)
    , maRows(rDocument, nTab, true, nOriginRow)
    , maAxes{ Axes() }
{
}

OString CellPositionJsonProcessor::position(double fValue, bool bVertical)
{
    const Axes& rAxes = maAxes.back();
    if (rAxes.bMapping)
    {
        const double fContent = bVertical ? rAxes.d * fValue + rAxes.f : rAxes.a * fValue + rAxes.e;
        const auto [nCell, fOffset] = (bVertical ? maRows : maColumns).map(fContent);
        return "[" + OString::number(nCell) + "," + offset(fOffset) + "]";
    }
    return OString::number(fValue);
}

OString CellPositionJsonProcessor::pathPosition(double fValue, bool bVertical)
{
    const Axes& rAxes = maAxes.back();
    if (rAxes.bMapping)
    {
        const double fContent = bVertical ? rAxes.d * fValue + rAxes.f : rAxes.a * fValue + rAxes.e;
        const auto [nCell, fOffset] = (bVertical ? maRows : maColumns).map(fContent);
        return OString::number(nCell) + ":" + offset(fOffset);
    }
    return OString::number(fValue);
}

void CellPositionJsonProcessor::writePositionValue(tools::JsonWriter& rWriter, double fValue,
                                                   bool bVertical)
{
    rWriter.putRaw(position(fValue, bVertical));
}

void CellPositionJsonProcessor::putPosition(std::string_view sName, double fValue, bool bVertical)
{
    tools::JsonWriter& rWriter = getWriter();
    const OString aValue = position(fValue, bVertical);
    if (!aValue.startsWith("["))
    {
        rWriter.put(sName, fValue);
        return;
    }
    auto aArray = rWriter.startArray(sName);
    rWriter.putRaw(aValue.subView(1, aValue.getLength() - 2));
}

void CellPositionJsonProcessor::putTextOrigin(double fX, double fY)
{
    if (!maAxes.back().bMapping)
    {
        Primitive2dJsonProcessor::putTextOrigin(fX, fY);
        return;
    }
    // The origin rounds to whole units, as the plain one does, before it is placed in its cell.
    putPosition("x", double(std::llround(fX)), false);
    putPosition("y", double(std::llround(fY)), true);
}

void CellPositionJsonProcessor::putRange(std::string_view sName, const basegfx::B2DRange& rRange,
                                         double fScale)
{
    if (!maAxes.back().bMapping)
    {
        Primitive2dJsonProcessor::putRange(sName, rRange, fScale);
        return;
    }
    tools::JsonWriter& rWriter = getWriter();
    auto aArray = rWriter.startArray(sName);
    writePositionValue(rWriter, rRange.getMinX() * fScale, false);
    writePositionValue(rWriter, rRange.getMinY() * fScale, true);
    writePositionValue(rWriter, rRange.getMaxX() * fScale, false);
    writePositionValue(rWriter, rRange.getMaxY() * fScale, true);
}

void CellPositionJsonProcessor::putPath(std::string_view sName,
                                        const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    if (!maAxes.back().bMapping)
    {
        Primitive2dJsonProcessor::putPath(sName, rPolyPolygon);
        return;
    }
    OStringBuffer aPath;
    auto appendPoint = [this, &aPath](char cCommand, const basegfx::B2DPoint& rPoint) {
        aPath.append(cCommand);
        aPath.append(pathPosition(rPoint.getX(), false) + "," + pathPosition(rPoint.getY(), true));
    };

    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        const sal_uInt32 nCount = rPolygon.count();
        if (!nCount)
            continue;
        appendPoint('M', rPolygon.getB2DPoint(0));
        const sal_uInt32 nEdges = rPolygon.isClosed() ? nCount : nCount - 1;
        for (sal_uInt32 nIndex = 0; nIndex < nEdges; ++nIndex)
        {
            const sal_uInt32 nNext = (nIndex + 1) % nCount;
            if (rPolygon.isNextControlPointUsed(nIndex) || rPolygon.isPrevControlPointUsed(nNext))
            {
                appendPoint('C', rPolygon.getNextControlPoint(nIndex));
                appendPoint(' ', rPolygon.getPrevControlPoint(nNext));
                appendPoint(' ', rPolygon.getB2DPoint(nNext));
            }
            else if (nNext != 0)
                appendPoint('L', rPolygon.getB2DPoint(nNext));
        }
        if (rPolygon.isClosed())
            aPath.append('Z');
    }
    getWriter().put(sName, aPath.makeStringAndClear());
}

void CellPositionJsonProcessor::putMatrix(std::string_view sName,
                                          const basegfx::B2DHomMatrix& rMatrix, MatrixKind eKind)
{
    const double a = rMatrix.get(0, 0);
    const double b = rMatrix.get(1, 0);
    const double c = rMatrix.get(0, 1);
    const double d = rMatrix.get(1, 1);
    const bool bUpright = b == 0.0 && c == 0.0 && a != 0.0 && d != 0.0;
    // A transformation that only scales and moves passes its children on as positions, so its
    // own matrix stays as it is.
    if (!maAxes.back().bMapping || (eKind == MatrixKind::Children && bUpright))
    {
        Primitive2dJsonProcessor::putMatrix(sName, rMatrix, eKind);
        return;
    }

    const double fScale = getScaleFactor();
    const double e = rMatrix.get(0, 2) * fScale;
    const double f = rMatrix.get(1, 2) * fScale;
    // The linear part of a transformation of children is not scaled, the others are.
    const double fLinear = eKind == MatrixKind::Children ? 1.0 : fScale;
    tools::JsonWriter& rWriter = getWriter();

    if (eKind == MatrixKind::Image && bUpright)
    {
        // An upright image stretches with its cells: its far corner is a position too, written in
        // the places of the scale.
        const OString aRight = position(e + a * fScale, false);
        const OString aBottom = position(f + d * fScale, true);
        if (aRight.startsWith("[") && aBottom.startsWith("["))
        {
            auto aArray = rWriter.startArray(sName);
            rWriter.putRaw(aRight);
            rWriter.putSimpleValue(0.0);
            rWriter.putSimpleValue(0.0);
            rWriter.putRaw(aBottom);
            writePositionValue(rWriter, e, false);
            writePositionValue(rWriter, f, true);
            return;
        }
    }

    auto aArray = rWriter.startArray(sName);
    rWriter.putSimpleValue(a * fLinear);
    rWriter.putSimpleValue(b * fLinear);
    rWriter.putSimpleValue(c * fLinear);
    rWriter.putSimpleValue(d * fLinear);
    writePositionValue(rWriter, e, false);
    writePositionValue(rWriter, f, true);
}

void CellPositionJsonProcessor::beginChildren(const basegfx::B2DHomMatrix& rTransform)
{
    Axes aInner = maAxes.back();
    const double a = rTransform.get(0, 0);
    const double b = rTransform.get(1, 0);
    const double c = rTransform.get(0, 1);
    const double d = rTransform.get(1, 1);
    const double fScale = getScaleFactor();
    if (aInner.bMapping && b == 0.0 && c == 0.0 && a != 0.0 && d != 0.0)
    {
        aInner.e = aInner.a * rTransform.get(0, 2) * fScale + aInner.e;
        aInner.f = aInner.d * rTransform.get(1, 2) * fScale + aInner.f;
        aInner.a *= a;
        aInner.d *= d;
    }
    else
        aInner.bMapping = false;
    maAxes.push_back(aInner);
}

void CellPositionJsonProcessor::endChildren() { maAxes.pop_back(); }

void CellPositionJsonProcessor::writeCellRange(tools::JsonWriter& rWriter, std::string_view sName,
                                               const basegfx::B2DRange& rRange)
{
    auto aNode = rWriter.startNode(sName);
    writeCellRangeFields(rWriter, rRange);
}

void CellPositionJsonProcessor::writeCellRangeFields(tools::JsonWriter& rWriter,
                                                     const basegfx::B2DRange& rRange)
{
    const std::pair<std::string_view, std::pair<double, bool>> aEdges[]
        = { { "x", { rRange.getMinX(), false } },
            { "y", { rRange.getMinY(), true } },
            { "right", { rRange.getMaxX(), false } },
            { "bottom", { rRange.getMaxY(), true } } };
    for (const auto& [sName, rEdge] : aEdges)
    {
        const OString aValue = position(rEdge.first, rEdge.second);
        if (!aValue.startsWith("["))
        {
            rWriter.put(sName, rEdge.first);
            continue;
        }
        auto aArray = rWriter.startArray(sName);
        rWriter.putRaw(aValue.subView(1, aValue.getLength() - 2));
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
