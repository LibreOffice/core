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

#include <b2dpolypolygonsweep.hxx>

#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

// we flatten the polygons and round them to a grid of whole numbers in [0 .. 2^20], so all
// tests on the flat edges are exact in int64_t (largest product about 2^62)
//
// pass 1: split all flat edges at all crossings and touches. A sweep over y holds the edges in a
// balanced tree, a crossing shows up as two neighbours changing their order. A rounded crossing
// may make new tiny ones, so we repeat until nothing gets split
//
// pass 2: merge pieces on top of each other and sweep again, every piece gets the winding left
// and right of it (one per operand) from its left neighbour. A piece is a border when the
// operation has one side inside and the other outside
//
// pass 3: link the borders to loops, inside on the left. Every flat edge knows its input segment
// and its t there, so we cut a run of border pieces on one segment out of the input curve again
// and curves stay curves. At a new crossing we look for the crossing of the true curves, so the
// parts meet there and not on the flat edges

namespace basegfx
{
namespace
{
constexpr int nGridBits = 20;

// the angle bound grows by this factor with each level of subdivision
constexpr double fUnsharpen = 1.6;

// more split points than this in one round of pass 1 take too long to solve, a random polygon of
// some thousand points crossing itself all over gets there
constexpr size_t nDefaultSplitLimit = 1 << 18;

struct GridPoint
{
    int64_t mnX = 0;
    int64_t mnY = 0;
};

bool operator==(const GridPoint& rLeft, const GridPoint& rRight)
{
    return rLeft.mnX == rRight.mnX && rLeft.mnY == rRight.mnY;
}

bool operator!=(const GridPoint& rLeft, const GridPoint& rRight) { return !(rLeft == rRight); }

// the order of the sweep: by y, then by x
bool operator<(const GridPoint& rLeft, const GridPoint& rRight)
{
    return rLeft.mnY < rRight.mnY || (rLeft.mnY == rRight.mnY && rLeft.mnX < rRight.mnX);
}

struct GridPointHash
{
    size_t operator()(const GridPoint& rPoint) const
    {
        return std::hash<uint64_t>()(uint64_t(rPoint.mnX) * 0x9E3779B97F4A7C15ULL
                                     ^ uint64_t(rPoint.mnY));
    }
};

// the input segment a flat edge lies on
struct Origin
{
    sal_Int32 mnPolygon = -1;
    sal_uInt32 mnSegment = 0;
};

bool operator==(const Origin& rLeft, const Origin& rRight)
{
    return rLeft.mnPolygon == rRight.mnPolygon && rLeft.mnSegment == rRight.mnSegment;
}

// a winding number per operand
struct Winding
{
    int mnFirst = 0;
    int mnSecond = 0;

    bool isZero() const { return mnFirst == 0 && mnSecond == 0; }
    Winding operator-() const { return { -mnFirst, -mnSecond }; }
    Winding operator+(const Winding& rOther) const
    {
        return { mnFirst + rOther.mnFirst, mnSecond + rOther.mnSecond };
    }
};

// maFirst < maSecond always. maDelta: +1 per input edge from maFirst to maSecond, -1 per one the
// other way, for the operand of that edge. mfFirstT, mfSecondT: t of both ends on the input segment
struct Edge
{
    GridPoint maFirst;
    GridPoint maSecond;
    Winding maDelta;
    Origin maOrigin;
    double mfFirstT = 0.0;
    double mfSecondT = 0.0;
};

int getSign(int64_t nValue) { return nValue > 0 ? 1 : (nValue < 0 ? -1 : 0); }

bool isHorizontal(const Edge& rEdge) { return rEdge.maFirst.mnY == rEdge.maSecond.mnY; }

int orientation(const GridPoint& rA, const GridPoint& rB, const GridPoint& rC)
{
    const int64_t nCross((rB.mnX - rA.mnX) * (rC.mnY - rA.mnY)
                         - (rB.mnY - rA.mnY) * (rC.mnX - rA.mnX));
    return getSign(nCross);
}

// x of an edge that is not horizontal at height nY, as rNumerator / rDenominator, rDenominator > 0
void getXAt(const Edge& rEdge, int64_t nY, int64_t& rNumerator, int64_t& rDenominator)
{
    rDenominator = rEdge.maSecond.mnY - rEdge.maFirst.mnY;
    rNumerator = rEdge.maFirst.mnX * rDenominator
                 + (nY - rEdge.maFirst.mnY) * (rEdge.maSecond.mnX - rEdge.maFirst.mnX);
}

int compareXAt(const Edge& rEdgeA, const Edge& rEdgeB, int64_t nY)
{
    int64_t nNumeratorA, nDenominatorA, nNumeratorB, nDenominatorB;
    getXAt(rEdgeA, nY, nNumeratorA, nDenominatorA);
    getXAt(rEdgeB, nY, nNumeratorB, nDenominatorB);
    return getSign(nNumeratorA * nDenominatorB - nNumeratorB * nDenominatorA);
}

int64_t roundDivide(int64_t nNumerator, int64_t nDenominator)
{
    if (nDenominator < 0)
    {
        nNumerator = -nNumerator;
        nDenominator = -nDenominator;
    }
    const int64_t nTwice(2 * nNumerator + nDenominator), nTwiceDenominator(2 * nDenominator);
    int64_t nResult(nTwice / nTwiceDenominator);
    if (nTwice % nTwiceDenominator != 0 && nTwice < 0)
        nResult -= 1;
    return nResult;
}

bool isOnInterior(const Edge& rEdge, const GridPoint& rPoint)
{
    if (rPoint == rEdge.maFirst || rPoint == rEdge.maSecond
        || orientation(rEdge.maFirst, rEdge.maSecond, rPoint) != 0)
        return false;
    return rPoint.mnY >= rEdge.maFirst.mnY && rPoint.mnY <= rEdge.maSecond.mnY
           && rPoint.mnX >= std::min(rEdge.maFirst.mnX, rEdge.maSecond.mnX)
           && rPoint.mnX <= std::max(rEdge.maFirst.mnX, rEdge.maSecond.mnX);
}

// an input point as the index of its polygon and of the flat point in it
struct Source
{
    sal_uInt32 mnPolygon;
    sal_uInt32 mnPoint;
};

typedef std::unordered_map<GridPoint, Source, GridPointHash> SourceMap;

// an x value as mnNumerator / mnDenominator with mnDenominator > 0
struct XProbe
{
    int64_t mnNumerator;
    int64_t mnDenominator;
};

// the edges crossing the sweep line, sorted by x at the current y, then by x just below, then by
// index. The tree holds slots and a slot holds an edge, so two neighbours can trade places at a
// crossing without a new search in the tree
class ActiveEdges
{
    const std::vector<Edge>& mrEdges;
    std::vector<int> maSlotEdge;
    std::vector<int> maEdgeSlot;

    struct Less
    {
        const ActiveEdges* mpOwner;
        using is_transparent = void;
        explicit Less(const ActiveEdges* pOwner)
            : mpOwner(pOwner)
        {
        }
        bool operator()(int nLeft, int nRight) const
        {
            return mpOwner->less(mpOwner->maSlotEdge[nLeft], mpOwner->maSlotEdge[nRight]);
        }
        bool operator()(int nSlot, const XProbe& rProbe) const
        {
            return mpOwner->compareProbe(mpOwner->maSlotEdge[nSlot], rProbe) < 0;
        }
        bool operator()(const XProbe& rProbe, int nSlot) const
        {
            return mpOwner->compareProbe(mpOwner->maSlotEdge[nSlot], rProbe) > 0;
        }
    };

public:
    typedef std::set<int, Less>::iterator Iterator;

private:
    std::set<int, Less> maSet;
    std::vector<Iterator> maSlotIterator;

public:
    int64_t mnY = 0;

    explicit ActiveEdges(const std::vector<Edge>& rEdges)
        : mrEdges(rEdges)
        , maEdgeSlot(rEdges.size(), -1)
        , maSet(Less(this))
    {
    }

    bool less(int nEdgeA, int nEdgeB) const
    {
        if (nEdgeA == nEdgeB)
            return false;
        const Edge& rA(mrEdges[nEdgeA]);
        const Edge& rB(mrEdges[nEdgeB]);
        const int nCompare(compareXAt(rA, rB, mnY));
        if (nCompare != 0)
            return nCompare < 0;
        const int64_t nSlope((rA.maSecond.mnX - rA.maFirst.mnX) * (rB.maSecond.mnY - rB.maFirst.mnY)
                             - (rB.maSecond.mnX - rB.maFirst.mnX)
                                   * (rA.maSecond.mnY - rA.maFirst.mnY));
        if (nSlope != 0)
            return nSlope < 0;
        return nEdgeA < nEdgeB;
    }

    int compareProbe(int nEdge, const XProbe& rProbe) const
    {
        int64_t nNumerator, nDenominator;
        getXAt(mrEdges[nEdge], mnY, nNumerator, nDenominator);
        return getSign(nNumerator * rProbe.mnDenominator - rProbe.mnNumerator * nDenominator);
    }

    bool contains(int nEdge) const { return maEdgeSlot[nEdge] >= 0; }
    int edge(Iterator it) const { return maSlotEdge[*it]; }
    Iterator iterator(int nEdge) const { return maSlotIterator[maEdgeSlot[nEdge]]; }
    Iterator begin() const { return maSet.begin(); }
    Iterator end() const { return maSet.end(); }
    Iterator lowerBound(const XProbe& rProbe) const { return maSet.lower_bound(rProbe); }
    Iterator upperBound(const XProbe& rProbe) const { return maSet.upper_bound(rProbe); }

    Iterator insert(int nEdge)
    {
        const int nSlot(maSlotEdge.size());
        maSlotEdge.push_back(nEdge);
        maEdgeSlot[nEdge] = nSlot;
        const Iterator it(maSet.insert(nSlot).first);
        maSlotIterator.push_back(it);
        return it;
    }

    void erase(int nEdge)
    {
        Iterator& rIterator(maSlotIterator[maEdgeSlot[nEdge]]);
        maSet.erase(rIterator);
        // the slot keeps a valid iterator, the vector copies them when it grows
        rIterator = maSet.end();
        maEdgeSlot[nEdge] = -1;
    }

    // the tree stays sorted once the sweep is past the crossing of both
    void swapNeighbours(Iterator itLeft, Iterator itRight)
    {
        const int nLeft(*itLeft), nRight(*itRight);
        std::swap(maSlotEdge[nLeft], maSlotEdge[nRight]);
        maEdgeSlot[maSlotEdge[nLeft]] = nLeft;
        maEdgeSlot[maSlotEdge[nRight]] = nRight;
    }
};

// pass 1: one sweep that collects the points where each edge has to be split
class Splitter
{
    const std::vector<Edge>& mrEdges;
    std::vector<std::vector<GridPoint>>& mrSplits;
    const SourceMap& mrSources;
    ActiveEdges maActive;
    std::vector<int64_t> maYs;
    // per event the neighbours (left, right) that cross in the beam above it
    std::vector<std::vector<std::pair<int, int>>> maSchedule;
    size_t mnSplits = 0;
    size_t mnSplitLimit;

    // a crossing that rounds next to an input point is that input point: both edges went through
    // it in the input, and only the rounding moved them apart
    GridPoint snap(const GridPoint& rCrossing) const
    {
        if (mrSources.count(rCrossing))
            return rCrossing;
        for (int64_t nDeltaY = -1; nDeltaY <= 1; nDeltaY++)
            for (int64_t nDeltaX = -1; nDeltaX <= 1; nDeltaX++)
            {
                const GridPoint aNear{ rCrossing.mnX + nDeltaX, rCrossing.mnY + nDeltaY };
                if (mrSources.count(aNear))
                    return aNear;
            }
        return rCrossing;
    }

    static bool crossProperly(const Edge& rA, const Edge& rB)
    {
        const int nOrientation1(orientation(rA.maFirst, rA.maSecond, rB.maFirst));
        const int nOrientation2(orientation(rA.maFirst, rA.maSecond, rB.maSecond));
        const int nOrientation3(orientation(rB.maFirst, rB.maSecond, rA.maFirst));
        const int nOrientation4(orientation(rB.maFirst, rB.maSecond, rA.maSecond));
        return nOrientation1 * nOrientation2 < 0 && nOrientation3 * nOrientation4 < 0;
    }

    void addSplit(int nEdge, const GridPoint& rPoint)
    {
        mrSplits[nEdge].push_back(rPoint);
        mnSplits++;
    }

    void handlePair(int nEdgeA, int nEdgeB)
    {
        const Edge& rA(mrEdges[nEdgeA]);
        const Edge& rB(mrEdges[nEdgeB]);
        if (isOnInterior(rA, rB.maFirst))
            addSplit(nEdgeA, rB.maFirst);
        if (isOnInterior(rA, rB.maSecond))
            addSplit(nEdgeA, rB.maSecond);
        if (isOnInterior(rB, rA.maFirst))
            addSplit(nEdgeB, rA.maFirst);
        if (isOnInterior(rB, rA.maSecond))
            addSplit(nEdgeB, rA.maSecond);

        if (!crossProperly(rA, rB))
            return;
        const int64_t nAX(rA.maSecond.mnX - rA.maFirst.mnX), nAY(rA.maSecond.mnY - rA.maFirst.mnY);
        const int64_t nBX(rB.maSecond.mnX - rB.maFirst.mnX), nBY(rB.maSecond.mnY - rB.maFirst.mnY);
        const int64_t nDenominator(nAX * nBY - nAY * nBX);
        const int64_t nNumerator((rB.maFirst.mnX - rA.maFirst.mnX) * nBY
                                 - (rB.maFirst.mnY - rA.maFirst.mnY) * nBX);
        const GridPoint aCrossing(
            snap({ rA.maFirst.mnX + roundDivide(nAX * nNumerator, nDenominator),
                   rA.maFirst.mnY + roundDivide(nAY * nNumerator, nDenominator) }));
        if (aCrossing != rA.maFirst && aCrossing != rA.maSecond)
            addSplit(nEdgeA, aCrossing);
        if (aCrossing != rB.maFirst && aCrossing != rB.maSecond)
            addSplit(nEdgeB, aCrossing);
    }

    // the two have to trade places at the first event at or below their crossing
    void schedule(int nLeft, int nRight, size_t nCurrent)
    {
        const Edge& rA(mrEdges[nLeft]);
        const Edge& rB(mrEdges[nRight]);
        if (!crossProperly(rA, rB))
            return;
        const int64_t nAX(rA.maSecond.mnX - rA.maFirst.mnX), nAY(rA.maSecond.mnY - rA.maFirst.mnY);
        const int64_t nBX(rB.maSecond.mnX - rB.maFirst.mnX), nBY(rB.maSecond.mnY - rB.maFirst.mnY);
        int64_t nDenominator(nAX * nBY - nAY * nBX);
        int64_t nNumerator((rB.maFirst.mnX - rA.maFirst.mnX) * nBY
                           - (rB.maFirst.mnY - rA.maFirst.mnY) * nBX);
        if (nDenominator < 0)
        {
            nDenominator = -nDenominator;
            nNumerator = -nNumerator;
        }
        // the crossing is at y = nYNumerator / nDenominator
        const int64_t nYNumerator(rA.maFirst.mnY * nDenominator + nAY * nNumerator);
        const size_t nEvent(std::lower_bound(maYs.begin(), maYs.end(), 0, [&](int64_t nY, int)
                                             { return nY * nDenominator < nYNumerator; })
                            - maYs.begin());
        if (nEvent > nCurrent && nEvent < maYs.size())
            maSchedule[nEvent].emplace_back(nLeft, nRight);
    }

    // nLeft and nRight became neighbours
    void visit(int nLeft, int nRight, size_t nCurrent, std::vector<std::pair<int, int>>* pInverted)
    {
        handlePair(nLeft, nRight);
        if (pInverted && maActive.less(nRight, nLeft))
            pInverted->emplace_back(nLeft, nRight);
        else
            schedule(nLeft, nRight, nCurrent);
    }

    void visitNeighbours(ActiveEdges::Iterator it, size_t nCurrent)
    {
        if (it != maActive.begin())
            visit(maActive.edge(std::prev(it)), maActive.edge(it), nCurrent, nullptr);
        const auto itNext(std::next(it));
        if (itNext != maActive.end())
            visit(maActive.edge(it), maActive.edge(itNext), nCurrent, nullptr);
    }

public:
    Splitter(const std::vector<Edge>& rEdges, std::vector<std::vector<GridPoint>>& rSplits,
             const SourceMap& rSources, size_t nSplitLimit)
        : mrEdges(rEdges)
        , mrSplits(rSplits)
        , mrSources(rSources)
        , maActive(rEdges)
        , mnSplitLimit(nSplitLimit)
    {
    }

    // false when there are too many split points to go on
    bool run()
    {
        std::vector<int> aStarting, aEnding, aHorizontal;
        for (size_t a = 0; a < mrEdges.size(); a++)
        {
            maYs.push_back(mrEdges[a].maFirst.mnY);
            maYs.push_back(mrEdges[a].maSecond.mnY);
            if (isHorizontal(mrEdges[a]))
                aHorizontal.push_back(int(a));
            else
            {
                aStarting.push_back(int(a));
                aEnding.push_back(int(a));
            }
        }
        std::sort(maYs.begin(), maYs.end());
        maYs.erase(std::unique(maYs.begin(), maYs.end()), maYs.end());
        maSchedule.resize(maYs.size());
        std::sort(aStarting.begin(), aStarting.end(), [&](int nLeft, int nRight)
                  { return mrEdges[nLeft].maFirst < mrEdges[nRight].maFirst; });
        std::sort(aEnding.begin(), aEnding.end(), [&](int nLeft, int nRight)
                  { return mrEdges[nLeft].maSecond.mnY < mrEdges[nRight].maSecond.mnY; });
        std::sort(aHorizontal.begin(), aHorizontal.end(), [&](int nLeft, int nRight)
                  { return mrEdges[nLeft].maFirst < mrEdges[nRight].maFirst; });

        size_t nNextStart(0), nNextEnd(0), nNextHorizontal(0);
        std::vector<GridPoint> aVertices;
        std::vector<int> aGroup;
        for (size_t nEvent = 0; nEvent < maYs.size(); nEvent++)
        {
            if (mnSplits > mnSplitLimit)
                return false;
            const int64_t nY(maYs[nEvent]);
            maActive.mnY = nY;

            // neighbours that crossed since the last event trade places until the order at y is
            // right again, every trade is one crossing
            std::vector<std::pair<int, int>> aInverted(std::move(maSchedule[nEvent]));
            while (!aInverted.empty())
            {
                const auto [nLeft, nRight] = aInverted.back();
                aInverted.pop_back();
                if (!maActive.contains(nLeft) || !maActive.contains(nRight))
                    continue;
                const auto itLeft(maActive.iterator(nLeft));
                const auto itRight(std::next(itLeft));
                if (itRight == maActive.end() || maActive.edge(itRight) != nRight
                    || !maActive.less(nRight, nLeft))
                    continue;
                maActive.swapNeighbours(itLeft, itRight);
                if (itLeft != maActive.begin())
                    visit(maActive.edge(std::prev(itLeft)), nRight, nEvent, &aInverted);
                const auto itNext(std::next(itRight));
                if (itNext != maActive.end())
                    visit(nLeft, maActive.edge(itNext), nEvent, &aInverted);
            }

            const size_t nStartBegin(nNextStart);
            while (nNextStart < aStarting.size()
                   && mrEdges[aStarting[nNextStart]].maFirst.mnY == nY)
                nNextStart++;
            const size_t nEndBegin(nNextEnd);
            while (nNextEnd < aEnding.size() && mrEdges[aEnding[nNextEnd]].maSecond.mnY == nY)
                nNextEnd++;
            const size_t nHorizontalBegin(nNextHorizontal);
            while (nNextHorizontal < aHorizontal.size()
                   && mrEdges[aHorizontal[nNextHorizontal]].maFirst.mnY == nY)
                nNextHorizontal++;

            // all edges through a vertex at y touch there
            aVertices.clear();
            for (size_t a = nStartBegin; a < nNextStart; a++)
                aVertices.push_back(mrEdges[aStarting[a]].maFirst);
            for (size_t a = nEndBegin; a < nNextEnd; a++)
                aVertices.push_back(mrEdges[aEnding[a]].maSecond);
            std::sort(aVertices.begin(), aVertices.end());
            aVertices.erase(std::unique(aVertices.begin(), aVertices.end()), aVertices.end());
            size_t nStartAtVertex(nStartBegin);
            for (const GridPoint& rVertex : aVertices)
            {
                aGroup.clear();
                const XProbe aProbe{ rVertex.mnX, 1 };
                for (auto it = maActive.lowerBound(aProbe), itEnd = maActive.upperBound(aProbe);
                     it != itEnd; ++it)
                    aGroup.push_back(maActive.edge(it));
                while (nStartAtVertex < nNextStart
                       && mrEdges[aStarting[nStartAtVertex]].maFirst < rVertex)
                    nStartAtVertex++;
                for (size_t a = nStartAtVertex;
                     a < nNextStart && mrEdges[aStarting[a]].maFirst == rVertex; a++)
                    aGroup.push_back(aStarting[a]);
                for (size_t a = 0; a < aGroup.size(); a++)
                    for (size_t b = a + 1; b < aGroup.size(); b++)
                        handlePair(aGroup[a], aGroup[b]);
            }

            // horizontal edges at y against everything that reaches y
            for (size_t a = nHorizontalBegin; a < nNextHorizontal; a++)
            {
                const Edge& rHorizontal(mrEdges[aHorizontal[a]]);
                for (auto it = maActive.lowerBound({ rHorizontal.maFirst.mnX, 1 }),
                          itEnd = maActive.upperBound({ rHorizontal.maSecond.mnX, 1 });
                     it != itEnd; ++it)
                    handlePair(aHorizontal[a], maActive.edge(it));
                for (size_t b = nStartBegin; b < nNextStart; b++)
                {
                    const GridPoint& rStart(mrEdges[aStarting[b]].maFirst);
                    if (rStart.mnX >= rHorizontal.maFirst.mnX
                        && rStart.mnX <= rHorizontal.maSecond.mnX)
                        handlePair(aHorizontal[a], aStarting[b]);
                }
                for (size_t b = a + 1;
                     b < nNextHorizontal
                     && mrEdges[aHorizontal[b]].maFirst.mnX <= rHorizontal.maSecond.mnX;
                     b++)
                    handlePair(aHorizontal[a], aHorizontal[b]);
            }

            for (size_t a = nEndBegin; a < nNextEnd; a++)
            {
                const int nEdge(aEnding[a]);
                const auto it(maActive.iterator(nEdge));
                const auto itNext(std::next(it));
                const int nLeft(it != maActive.begin() ? maActive.edge(std::prev(it)) : -1);
                const int nRight(itNext != maActive.end() ? maActive.edge(itNext) : -1);
                maActive.erase(nEdge);
                if (nLeft >= 0 && nRight >= 0 && maActive.contains(nLeft)
                    && maActive.contains(nRight) && mrEdges[nLeft].maSecond.mnY > nY
                    && mrEdges[nRight].maSecond.mnY > nY)
                    visit(nLeft, nRight, nEvent, nullptr);
            }

            for (size_t a = nStartBegin; a < nNextStart; a++)
                visitNeighbours(maActive.insert(aStarting[a]), nEvent);
        }
        return mnSplits <= mnSplitLimit;
    }
};

void addPiece(std::vector<Edge>& rTarget, const GridPoint& rFrom, const GridPoint& rTo,
              const Winding& rDelta, const Origin& rOrigin, double fFromT, double fToT)
{
    if (rFrom == rTo)
        return;
    if (rFrom < rTo)
        rTarget.push_back(Edge{ rFrom, rTo, rDelta, rOrigin, fFromT, fToT });
    else
        rTarget.push_back(Edge{ rTo, rFrom, -rDelta, rOrigin, fToT, fFromT });
}

// false when nothing was split
bool applySplits(std::vector<Edge>& rEdges, std::vector<std::vector<GridPoint>>& rSplits)
{
    bool bChanged(false);
    std::vector<Edge> aTarget;
    aTarget.reserve(rEdges.size());
    for (size_t a = 0; a < rEdges.size(); a++)
    {
        const Edge& rEdge(rEdges[a]);
        std::vector<GridPoint>& rPoints(rSplits[a]);
        if (rPoints.empty())
        {
            aTarget.push_back(rEdge);
            continue;
        }
        bChanged = true;
        const int64_t nDeltaX(rEdge.maSecond.mnX - rEdge.maFirst.mnX);
        const int64_t nDeltaY(rEdge.maSecond.mnY - rEdge.maFirst.mnY);
        auto getAlong = [&](const GridPoint& rPoint)
        {
            return (rPoint.mnX - rEdge.maFirst.mnX) * nDeltaX
                   + (rPoint.mnY - rEdge.maFirst.mnY) * nDeltaY;
        };
        std::sort(rPoints.begin(), rPoints.end(),
                  [&](const GridPoint& rLeft, const GridPoint& rRight)
                  { return getAlong(rLeft) < getAlong(rRight); });
        rPoints.erase(std::unique(rPoints.begin(), rPoints.end()), rPoints.end());
        // a split point gets the parameter of its projection onto the edge, a snapped one may
        // project a bit outside
        const double fLength(double(getAlong(rEdge.maSecond)));
        const double fMinT(std::min(rEdge.mfFirstT, rEdge.mfSecondT));
        const double fMaxT(std::max(rEdge.mfFirstT, rEdge.mfSecondT));
        GridPoint aPrevious(rEdge.maFirst);
        double fPreviousT(rEdge.mfFirstT);
        for (const GridPoint& rPoint : rPoints)
        {
            const double fT(std::clamp(rEdge.mfFirstT
                                           + (rEdge.mfSecondT - rEdge.mfFirstT)
                                                 * (double(getAlong(rPoint)) / fLength),
                                       fMinT, fMaxT));
            addPiece(aTarget, aPrevious, rPoint, rEdge.maDelta, rEdge.maOrigin, fPreviousT, fT);
            aPrevious = rPoint;
            fPreviousT = fT;
        }
        addPiece(aTarget, aPrevious, rEdge.maSecond, rEdge.maDelta, rEdge.maOrigin, fPreviousT,
                 rEdge.mfSecondT);
    }
    rEdges.swap(aTarget);
    return bChanged;
}

// turns the same way everywhere and once around in total, so every winding is 0 or 1
bool isConvex(const B2DPolygon& rCandidate)
{
    const B2DPolygon aPolygon(rCandidate.areControlPointsUsed()
                                  ? rCandidate.getDefaultAdaptiveSubdivision()
                                  : rCandidate);
    const sal_uInt32 nCount(aPolygon.count());
    if (nCount < 4)
        return true;
    int nSign(0);
    double fTurn(0.0);
    for (sal_uInt32 a(0); a < nCount; a++)
    {
        const B2DPoint aPrevious(aPolygon.getB2DPoint((a + nCount - 1) % nCount));
        const B2DPoint aCurrent(aPolygon.getB2DPoint(a));
        const B2DPoint aNext(aPolygon.getB2DPoint((a + 1) % nCount));
        const B2DVector aIn(aCurrent - aPrevious), aOut(aNext - aCurrent);
        const double fCross(aIn.cross(aOut));
        if (fCross != 0.0)
        {
            const int nThis(fCross > 0.0 ? 1 : -1);
            if (nSign != 0 && nThis != nSign)
                return false;
            nSign = nThis;
        }
        fTurn += std::atan2(fCross, aIn.scalar(aOut));
    }
    return std::abs(std::abs(fTurn) - 2.0 * M_PI) < 0.5;
}

// the polygon is filled, so an open one has a straight segment from its last point to its first
B2DCubicBezier getSegment(const B2DPolygon& rPolygon, sal_uInt32 nSegment)
{
    if (!rPolygon.isClosed() && nSegment + 1 == rPolygon.count())
    {
        const B2DPoint aStart(rPolygon.getB2DPoint(nSegment)), aEnd(rPolygon.getB2DPoint(0));
        return B2DCubicBezier(aStart, aStart, aEnd, aEnd);
    }
    B2DCubicBezier aRetval;
    rPolygon.getBezierSegment(nSegment, aRetval);
    return aRetval;
}

// on a straight segment t grows evenly from start to end
B2DPoint getPointAt(const B2DCubicBezier& rSegment, double fT)
{
    if (!rSegment.isBezier())
        return rSegment.getStartPoint() + (rSegment.getEndPoint() - rSegment.getStartPoint()) * fT;
    return rSegment.interpolatePoint(fT);
}

B2DVector getDerivativeAt(const B2DCubicBezier& rSegment, double fT)
{
    const B2DPoint& rStart(rSegment.getStartPoint());
    const B2DPoint& rEnd(rSegment.getEndPoint());
    if (!rSegment.isBezier())
        return rEnd - rStart;
    const B2DPoint& rControlA(rSegment.getControlPointA());
    const B2DPoint& rControlB(rSegment.getControlPointB());
    const double fRest(1.0 - fT);
    return (rControlA - rStart) * (3.0 * fRest * fRest)
           + (rControlB - rControlA) * (6.0 * fRest * fT) + (rEnd - rControlB) * (3.0 * fT * fT);
}

double getDistanceToSegment(const B2DPoint& rPoint, const B2DPoint& rStart, const B2DPoint& rEnd)
{
    const B2DVector aEdge(rEnd - rStart);
    const double fSquare(aEdge.scalar(aEdge));
    const double fT(fSquare > 0.0
                        ? std::clamp(B2DVector(rPoint - rStart).scalar(aEdge) / fSquare, 0.0, 1.0)
                        : 0.0);
    return B2DVector(rPoint - (rStart + aEdge * fT)).getLength();
}

// the angle test of B2DCubicBezier::adaptiveSubdivideByAngle, plus the t of every point; appends
// the points after the start up to the end. The angle test only sees the legs at both ends, so we
// also split while a control point is farther than fTolerance from the chord
void subdivideCurve(const B2DCubicBezier& rCurve, double fStartT, double fEndT, double fAngleBound,
                    double fTolerance, int nDepth, std::vector<B2DPoint>& rPoints,
                    std::vector<double>& rParameters)
{
    if (nDepth > 0)
    {
        bool bSplit(getDistanceToSegment(rCurve.getControlPointA(), rCurve.getStartPoint(),
                                         rCurve.getEndPoint())
                        > fTolerance
                    || getDistanceToSegment(rCurve.getControlPointB(), rCurve.getStartPoint(),
                                            rCurve.getEndPoint())
                           > fTolerance);
        if (!bSplit)
        {
            B2DVector aLeft(rCurve.getControlPointA() - rCurve.getStartPoint());
            B2DVector aRight(rCurve.getControlPointB() - rCurve.getEndPoint());
            if (aLeft.equalZero())
                aLeft = rCurve.getControlPointB() - rCurve.getStartPoint();
            if (aRight.equalZero())
                aRight = rCurve.getControlPointA() - rCurve.getEndPoint();
            bSplit = std::abs(aLeft.angle(aRight)) <= M_PI - fAngleBound;
        }
        if (bSplit)
        {
            B2DCubicBezier aFirst, aSecond;
            rCurve.split(0.5, &aFirst, &aSecond);
            const double fMiddleT(0.5 * (fStartT + fEndT));
            subdivideCurve(aFirst, fStartT, fMiddleT, fAngleBound * fUnsharpen, fTolerance,
                           nDepth - 1, rPoints, rParameters);
            subdivideCurve(aSecond, fMiddleT, fEndT, fAngleBound * fUnsharpen, fTolerance,
                           nDepth - 1, rPoints, rParameters);
            return;
        }
    }
    rPoints.push_back(rCurve.getEndPoint());
    rParameters.push_back(fEndT);
}

// Newton steps for rA(rS) == rB(rU) from the given parameters
bool findCrossingByNewton(const B2DCubicBezier& rA, double& rS, const B2DCubicBezier& rB,
                          double& rU, double fEpsilon)
{
    for (int nStep = 0; nStep < 12; nStep++)
    {
        const B2DVector aOffset(getPointAt(rA, rS) - getPointAt(rB, rU));
        if (aOffset.getLength() <= fEpsilon)
            return true;
        const B2DVector aDerivativeA(getDerivativeAt(rA, rS)),
            aDerivativeB(getDerivativeAt(rB, rU));
        // solve aDerivativeA * dS - aDerivativeB * dU = -aOffset
        const double fDeterminant(aDerivativeB.getX() * aDerivativeA.getY()
                                  - aDerivativeA.getX() * aDerivativeB.getY());
        if (fDeterminant == 0.0 || !std::isfinite(fDeterminant))
            return false;
        rS += (aOffset.getX() * aDerivativeB.getY() - aDerivativeB.getX() * aOffset.getY())
              / fDeterminant;
        rU += (aDerivativeA.getY() * aOffset.getX() - aDerivativeA.getX() * aOffset.getY())
              / fDeterminant;
        if (!std::isfinite(rS) || !std::isfinite(rU) || rS < -0.5 || rS > 1.5 || rU < -0.5
            || rU > 1.5)
            return false;
    }
    return B2DVector(getPointAt(rA, rS) - getPointAt(rB, rU)).getLength() <= fEpsilon;
}

double getDistanceToBox(const B2DRange& rBox, const B2DPoint& rPoint)
{
    const double fX(
        std::max({ rBox.getMinX() - rPoint.getX(), 0.0, rPoint.getX() - rBox.getMaxX() }));
    const double fY(
        std::max({ rBox.getMinY() - rPoint.getY(), 0.0, rPoint.getY() - rBox.getMaxY() }));
    return std::hypot(fX, fY);
}

// the curve stays inside this box
B2DRange getControlBox(const B2DCubicBezier& rCurve)
{
    B2DRange aRetval(rCurve.getStartPoint(), rCurve.getEndPoint());
    aRetval.expand(rCurve.getControlPointA());
    aRetval.expand(rCurve.getControlPointB());
    return aRetval;
}

// the t in [fStartT, fEndT] where rCurve comes nearest to rPoint. We halve, a half stays while the
// box of its control points is not farther away than the best point so far
double projectByBisection(const B2DCubicBezier& rCurve, double fStartT, double fEndT,
                          const B2DPoint& rPoint, double fEpsilon)
{
    if (!rCurve.isBezier())
    {
        const B2DVector aEdge(rCurve.getEndPoint() - rCurve.getStartPoint());
        const double fSquare(aEdge.scalar(aEdge));
        return fSquare > 0.0
                   ? std::clamp(B2DVector(rPoint - rCurve.getStartPoint()).scalar(aEdge) / fSquare,
                                fStartT, fEndT)
                   : fStartT;
    }
    double fBestT(0.5 * (fStartT + fEndT));
    double fBest(B2DVector(getPointAt(rCurve, fBestT) - rPoint).getLength());
    struct Item
    {
        B2DCubicBezier maPiece;
        double mfStartT;
        double mfEndT;
    };
    std::vector<Item> aStack{ { rCurve.snippet(fStartT, fEndT), fStartT, fEndT } };
    for (int nStep = 0; !aStack.empty() && nStep < 1000; nStep++)
    {
        const Item aItem(aStack.back());
        aStack.pop_back();
        const B2DRange aBox(getControlBox(aItem.maPiece));
        if (getDistanceToBox(aBox, rPoint) > fBest)
            continue;
        B2DCubicBezier aFirst, aSecond;
        aItem.maPiece.split(0.5, &aFirst, &aSecond);
        const double fMiddleT(0.5 * (aItem.mfStartT + aItem.mfEndT));
        const double fDistance(B2DVector(aFirst.getEndPoint() - rPoint).getLength());
        if (fDistance < fBest)
        {
            fBest = fDistance;
            fBestT = fMiddleT;
        }
        if (std::max(aBox.getWidth(), aBox.getHeight()) > fEpsilon)
        {
            aStack.push_back({ aFirst, aItem.mfStartT, fMiddleT });
            aStack.push_back({ aSecond, fMiddleT, aItem.mfEndT });
        }
    }
    return fBestT;
}

// a crossing of rA in [fStartS, fEndS] with rB in [fStartU, fEndU], the one nearest to rGuess. We
// halve the longer piece, a pair stays while the boxes of their control points overlap, so no half
// that can hold a crossing gets lost
bool crossByBisection(const B2DCubicBezier& rA, double fStartS, double fEndS,
                      const B2DCubicBezier& rB, double fStartU, double fEndU,
                      const B2DPoint& rGuess, double fEpsilon, double& rS, double& rU)
{
    rS = 0.5 * (fStartS + fEndS);
    rU = 0.5 * (fStartU + fEndU);
    if (!rA.isBezier() && !rB.isBezier())
    {
        // the crossing of both lines, parallel ones have none
        const B2DVector aEdgeA(rA.getEndPoint() - rA.getStartPoint());
        const B2DVector aEdgeB(rB.getEndPoint() - rB.getStartPoint());
        const double fDenominator(aEdgeA.cross(aEdgeB));
        if (fDenominator == 0.0)
            return false;
        const B2DVector aOffset(rB.getStartPoint() - rA.getStartPoint());
        rS = aOffset.cross(aEdgeB) / fDenominator;
        rU = aOffset.cross(aEdgeA) / fDenominator;
        return rS >= fStartS && rS <= fEndS && rU >= fStartU && rU <= fEndU;
    }
    // a straight segment as a cubic with even parameter, like getPointAt
    auto getPiece = [](const B2DCubicBezier& rCurve, double fFromT, double fToT)
    {
        if (rCurve.isBezier())
            return rCurve.snippet(fFromT, fToT);
        const B2DPoint aFrom(getPointAt(rCurve, fFromT)), aTo(getPointAt(rCurve, fToT));
        return B2DCubicBezier(aFrom, aFrom + (aTo - aFrom) * (1.0 / 3.0),
                              aFrom + (aTo - aFrom) * (2.0 / 3.0), aTo);
    };
    struct Item
    {
        B2DCubicBezier maPieceA;
        B2DCubicBezier maPieceB;
        double mfStartS;
        double mfEndS;
        double mfStartU;
        double mfEndU;
    };
    std::vector<Item> aStack{ { getPiece(rA, fStartS, fEndS), getPiece(rB, fStartU, fEndU), fStartS,
                                fEndS, fStartU, fEndU } };
    bool bFound(false);
    double fBest(0.0);
    for (int nStep = 0; !aStack.empty() && nStep < 1000; nStep++)
    {
        const Item aItem(aStack.back());
        aStack.pop_back();
        B2DRange aBoxA(getControlBox(aItem.maPieceA));
        const B2DRange aBoxB(getControlBox(aItem.maPieceB));
        const double fSizeA(std::max(aBoxA.getWidth(), aBoxA.getHeight()));
        const double fSizeB(std::max(aBoxB.getWidth(), aBoxB.getHeight()));
        aBoxA.grow(fEpsilon);
        if (!aBoxA.overlaps(aBoxB))
            continue;
        if (std::max(fSizeA, fSizeB) <= fEpsilon)
        {
            const double fMiddleS(0.5 * (aItem.mfStartS + aItem.mfEndS));
            const double fMiddleU(0.5 * (aItem.mfStartU + aItem.mfEndU));
            const double fDistance(B2DVector(getPointAt(rA, fMiddleS) - rGuess).getLength());
            if (!bFound || fDistance < fBest)
            {
                bFound = true;
                fBest = fDistance;
                rS = fMiddleS;
                rU = fMiddleU;
            }
            continue;
        }
        B2DCubicBezier aFirst, aSecond;
        if (fSizeA >= fSizeB)
        {
            const double fMiddleS(0.5 * (aItem.mfStartS + aItem.mfEndS));
            aItem.maPieceA.split(0.5, &aFirst, &aSecond);
            aStack.push_back(
                { aFirst, aItem.maPieceB, aItem.mfStartS, fMiddleS, aItem.mfStartU, aItem.mfEndU });
            aStack.push_back(
                { aSecond, aItem.maPieceB, fMiddleS, aItem.mfEndS, aItem.mfStartU, aItem.mfEndU });
        }
        else
        {
            const double fMiddleU(0.5 * (aItem.mfStartU + aItem.mfEndU));
            aItem.maPieceB.split(0.5, &aFirst, &aSecond);
            aStack.push_back(
                { aItem.maPieceA, aFirst, aItem.mfStartS, aItem.mfEndS, aItem.mfStartU, fMiddleU });
            aStack.push_back(
                { aItem.maPieceA, aSecond, aItem.mfStartS, aItem.mfEndS, fMiddleU, aItem.mfEndU });
        }
    }
    return bFound;
}

// the flat points of one polygon: segment and parameter of each
struct FlatPolygon
{
    std::vector<sal_uInt32> maSegments;
    std::vector<double> maParameters;
};

// a point on an input segment where anything else meets its flat chain
struct Cut
{
    double mfT;
    GridPoint maVertex;
};

// a border runs along the piece mnEdge from maFrom to maTo
struct Border
{
    GridPoint maFrom;
    GridPoint maTo;
    int mnEdge;
};

// the part mnPart of a segment, between two neighbouring cuts, as a border
struct Part
{
    sal_Int32 mnPolygon;
    sal_uInt32 mnSegment;
    int mnPart;
    bool mbForward;
    GridPoint maFrom;
    GridPoint maTo;
};

// a cut, as the index in the cut array of a segment
struct CutReference
{
    sal_Int32 mnPolygon;
    sal_uInt32 mnSegment;
    size_t mnIndex;
};

// the polygons of the first operand come first in rCandidate, nFirstCount of them; an empty
// optional means the result is too expensive
std::optional<B2DPolyPolygon> solve(const B2DPolyPolygon& rCandidate, sal_uInt32 nFirstCount,
                                    utils::SweepFillRule eFirstRule,
                                    utils::SweepFillRule eSecondRule,
                                    utils::SweepOperation eOperation, size_t nSplitLimit)
{
    const bool bFill(eOperation == utils::SweepOperation::Fill);
    auto isInside = [&](const Winding& rWinding)
    {
        const bool bFirst(eFirstRule == utils::SweepFillRule::Nonzero
                              ? rWinding.mnFirst != 0
                              : (rWinding.mnFirst & 1) != 0);
        const bool bSecond(eSecondRule == utils::SweepFillRule::Nonzero
                               ? rWinding.mnSecond != 0
                               : (rWinding.mnSecond & 1) != 0);
        switch (eOperation)
        {
            case utils::SweepOperation::Fill:
                return bFirst;
            case utils::SweepOperation::Or:
                return bFirst || bSecond;
            case utils::SweepOperation::And:
                return bFirst && bSecond;
            case utils::SweepOperation::Xor:
                return bFirst != bSecond;
            case utils::SweepOperation::Diff:
                return bFirst && !bSecond;
        }
        return false;
    };

    const B2DRange aRange(rCandidate.getB2DRange());
    const double fExtent(std::max(aRange.getWidth(), aRange.getHeight()));
    if (rCandidate.count() == 0)
        return bFill ? rCandidate : B2DPolyPolygon();
    // no grid for coordinates that are not finite, that is answered like too many split points
    if (!std::isfinite(fExtent))
        return std::nullopt;
    if (!(fExtent > 0.0))
        return bFill ? rCandidate : B2DPolyPolygon();
    if (bFill && rCandidate.count() == 1 && isConvex(rCandidate.getB2DPolygon(0)))
        return rCandidate;

    const double fScale(double(int64_t(1) << nGridBits) / fExtent);
    const double fMinX(aRange.getMinX()), fMinY(aRange.getMinY());
    // a point that does not fit the grid (not finite, or outside the range) sets bOffGrid
    bool bOffGrid(false);
    auto toGrid = [&](const B2DPoint& rPoint)
    {
        const double fX((rPoint.getX() - fMinX) * fScale);
        const double fY((rPoint.getY() - fMinY) * fScale);
        const double fLimit(double(int64_t(1) << nGridBits) + 1.0);
        if (!(fX >= -1.0 && fX <= fLimit && fY >= -1.0 && fY <= fLimit))
        {
            bOffGrid = true;
            return GridPoint();
        }
        return GridPoint{ std::llround(fX), std::llround(fY) };
    };

    // the input segments, a straight one has its control points on its ends
    const std::vector<B2DPolygon> aOriginals(rCandidate.begin(), rCandidate.end());
    std::vector<std::vector<B2DCubicBezier>> aCurves(aOriginals.size());
    for (size_t a = 0; a < aOriginals.size(); a++)
        for (sal_uInt32 b(0); b < aOriginals[a].count(); b++)
            aCurves[a].push_back(getSegment(aOriginals[a], b));

    // the flat input; grid points from an input point refer back to it, so the result keeps the
    // exact coordinates there
    std::vector<FlatPolygon> aFlats(aOriginals.size());
    std::vector<B2DPolygon> aFlattened;
    aFlattened.reserve(aOriginals.size());
    SourceMap aSources;
    std::vector<Edge> aEdges;
    std::vector<B2DPoint> aPoints;
    std::vector<GridPoint> aGrid;
    const double fAngleBound(2.25 * M_PI / 180.0);
    const double fTolerance(fExtent / double(1 << 14));
    for (size_t a = 0; a < aOriginals.size(); a++)
    {
        const B2DPolygon& rPolygon(aOriginals[a]);
        FlatPolygon& rFlat(aFlats[a]);
        aPoints.clear();
        for (sal_uInt32 nSegment(0); nSegment < rPolygon.count(); nSegment++)
        {
            aPoints.push_back(rPolygon.getB2DPoint(nSegment));
            rFlat.maParameters.push_back(0.0);
            rFlat.maSegments.push_back(nSegment);
            const B2DCubicBezier& rSegment(aCurves[a][nSegment]);
            if (!rSegment.isBezier())
                continue;
            subdivideCurve(rSegment, 0.0, 1.0, fAngleBound, fTolerance, 12, aPoints,
                           rFlat.maParameters);
            // the end point is the start of the next segment
            aPoints.pop_back();
            rFlat.maParameters.pop_back();
            rFlat.maSegments.resize(aPoints.size(), nSegment);
        }
        B2DPolygon aFlat;
        aFlat.reserve(aPoints.size());
        aGrid.clear();
        for (size_t b = 0; b < aPoints.size(); b++)
        {
            aFlat.append(aPoints[b]);
            const GridPoint aGridPoint(toGrid(aPoints[b]));
            aSources.try_emplace(aGridPoint, Source{ sal_uInt32(a), sal_uInt32(b) });
            aGrid.push_back(aGridPoint);
        }
        aFlattened.push_back(aFlat);
        Winding aOne;
        (a < nFirstCount ? aOne.mnFirst : aOne.mnSecond) = 1;
        for (size_t b = 0; b < aGrid.size(); b++)
        {
            const size_t nNext((b + 1) % aGrid.size());
            // the last flat edge of a segment ends at t = 1
            const bool bSameSegment(rFlat.maSegments[nNext] == rFlat.maSegments[b]
                                    && rFlat.maParameters[nNext] > rFlat.maParameters[b]);
            addPiece(aEdges, aGrid[b], aGrid[nNext], aOne, { sal_Int32(a), rFlat.maSegments[b] },
                     rFlat.maParameters[b], bSameSegment ? rFlat.maParameters[nNext] : 1.0);
        }
    }
    if (bOffGrid)
        return std::nullopt;

    // pass 1
    int nRounds(0);
    for (; nRounds < 16; nRounds++)
    {
        std::vector<std::vector<GridPoint>> aSplits(aEdges.size());
        if (!Splitter(aEdges, aSplits, aSources, nSplitLimit).run())
            return std::nullopt;
        if (!applySplits(aEdges, aSplits))
            break;
    }

    // a single polygon without any cut does not cross itself
    if (bFill && nRounds == 0 && rCandidate.count() == 1)
        return rCandidate;

    // the cuts of each segment, both ends included. Between two neighbouring cuts nothing else
    // touches the flat chain, so the windings on both sides stay the same and the part is a border
    // as a whole or not at all
    std::vector<std::vector<std::vector<Cut>>> aCuts(aOriginals.size());
    {
        struct VertexInfo
        {
            int mnDegree = 0;
            Origin maFirst;
            bool mbMixed = false;
        };
        std::unordered_map<GridPoint, VertexInfo, GridPointHash> aVertices;
        for (const Edge& rEdge : aEdges)
            for (const GridPoint* pPoint : { &rEdge.maFirst, &rEdge.maSecond })
            {
                VertexInfo& rInfo(aVertices[*pPoint]);
                if (rInfo.mnDegree == 0)
                    rInfo.maFirst = rEdge.maOrigin;
                else if (!(rInfo.maFirst == rEdge.maOrigin))
                    rInfo.mbMixed = true;
                rInfo.mnDegree++;
            }
        for (size_t a = 0; a < aOriginals.size(); a++)
        {
            const sal_uInt32 nCount(aOriginals[a].count());
            aCuts[a].resize(nCount);
            for (sal_uInt32 b(0); b < nCount; b++)
            {
                aCuts[a][b].push_back(Cut{ 0.0, toGrid(aCurves[a][b].getStartPoint()) });
                aCuts[a][b].push_back(Cut{ 1.0, toGrid(aCurves[a][b].getEndPoint()) });
            }
        }
        for (const Edge& rEdge : aEdges)
            for (int nEnd = 0; nEnd < 2; nEnd++)
            {
                const GridPoint& rPoint(nEnd ? rEdge.maSecond : rEdge.maFirst);
                const VertexInfo& rInfo(aVertices[rPoint]);
                if (rInfo.mnDegree != 2 || rInfo.mbMixed)
                    aCuts[rEdge.maOrigin.mnPolygon][rEdge.maOrigin.mnSegment].push_back(
                        Cut{ nEnd ? rEdge.mfSecondT : rEdge.mfFirstT, rPoint });
            }
        for (auto& rPolygonCuts : aCuts)
            for (auto& rSegmentCuts : rPolygonCuts)
            {
                std::sort(rSegmentCuts.begin(), rSegmentCuts.end(),
                          [](const Cut& rLeft, const Cut& rRight)
                          { return rLeft.mfT < rRight.mfT; });
                rSegmentCuts.erase(std::unique(rSegmentCuts.begin(), rSegmentCuts.end(),
                                               [](const Cut& rLeft, const Cut& rRight)
                                               { return rLeft.mfT == rRight.mfT; }),
                                   rSegmentCuts.end());
                // a cut on the grid vertex of a segment end is that end
                while (rSegmentCuts.size() > 2
                       && rSegmentCuts[1].maVertex == rSegmentCuts[0].maVertex)
                    rSegmentCuts.erase(rSegmentCuts.begin() + 1);
                while (rSegmentCuts.size() > 2
                       && rSegmentCuts[rSegmentCuts.size() - 2].maVertex
                              == rSegmentCuts.back().maVertex)
                    rSegmentCuts.erase(rSegmentCuts.end() - 2);
            }
    }

    // pass 2: merge pieces on top of each other, the merged one keeps the origin of the first
    {
        struct KeyHash
        {
            size_t operator()(const std::pair<GridPoint, GridPoint>& rKey) const
            {
                return GridPointHash()(rKey.first) * 31 + GridPointHash()(rKey.second);
            }
        };
        std::unordered_map<std::pair<GridPoint, GridPoint>, size_t, KeyHash> aIndex;
        std::vector<Edge> aMerged;
        for (const Edge& rEdge : aEdges)
        {
            const auto [it, bNew]
                = aIndex.try_emplace({ rEdge.maFirst, rEdge.maSecond }, aMerged.size());
            if (bNew)
                aMerged.push_back(rEdge);
            else
                aMerged[it->second].maDelta = aMerged[it->second].maDelta + rEdge.maDelta;
        }
        aEdges.clear();
        for (const Edge& rEdge : aMerged)
            if (!rEdge.maDelta.isZero() || isHorizontal(rEdge))
                aEdges.push_back(rEdge);
    }

    std::vector<int64_t> aYs;
    std::vector<int> aSloped, aHorizontal;
    for (size_t a = 0; a < aEdges.size(); a++)
    {
        aYs.push_back(aEdges[a].maFirst.mnY);
        aYs.push_back(aEdges[a].maSecond.mnY);
        (isHorizontal(aEdges[a]) ? aHorizontal : aSloped).push_back(int(a));
    }
    if (aSloped.empty())
        return bFill ? rCandidate : B2DPolyPolygon();
    std::sort(aYs.begin(), aYs.end());
    aYs.erase(std::unique(aYs.begin(), aYs.end()), aYs.end());
    std::sort(aSloped.begin(), aSloped.end(), [&](int nLeft, int nRight)
              { return aEdges[nLeft].maFirst.mnY < aEdges[nRight].maFirst.mnY; });
    std::sort(aHorizontal.begin(), aHorizontal.end(), [&](int nLeft, int nRight)
              { return aEdges[nLeft].maFirst < aEdges[nRight].maFirst; });

    std::vector<Border> aBorders;
    // for a nonzero fill the input is fine already when no winding is even and not zero
    bool bNeedsChange(!bFill || eFirstRule == utils::SweepFillRule::EvenOdd);
    auto isEvenNotZero = [](int nWinding) { return nWinding != 0 && nWinding % 2 == 0; };
    // after pass 1 no two pieces cross, so the winding right of a piece is the same along all of
    // it, and a new piece takes the winding left of it from its left neighbour
    std::vector<Winding> aWindingRight(aEdges.size());
    std::vector<int> aEnding(aSloped);
    std::sort(aEnding.begin(), aEnding.end(), [&](int nLeft, int nRight)
              { return aEdges[nLeft].maSecond.mnY < aEdges[nRight].maSecond.mnY; });
    ActiveEdges aActive(aEdges);
    auto getWindingLeftOf = [&](const Edge& rHorizontal)
    {
        // the middle of a horizontal piece is on no other piece, it would have been split there
        const auto it(
            aActive.lowerBound({ rHorizontal.maFirst.mnX + rHorizontal.maSecond.mnX, 2 }));
        return it == aActive.begin() ? Winding() : aWindingRight[aActive.edge(std::prev(it))];
    };
    size_t nNextStart(0), nNextEnd(0), nNextHorizontal(0);
    std::vector<Winding> aAboveOfHorizontal;
    for (const int64_t nY : aYs)
    {
        aActive.mnY = nY;
        const size_t nHorizontalBegin(nNextHorizontal);
        while (nNextHorizontal < aHorizontal.size()
               && aEdges[aHorizontal[nNextHorizontal]].maFirst.mnY == nY)
            nNextHorizontal++;

        // the winding just above the horizontal pieces, before the pieces at y come and go
        aAboveOfHorizontal.clear();
        for (size_t a = nHorizontalBegin; a < nNextHorizontal; a++)
            aAboveOfHorizontal.push_back(getWindingLeftOf(aEdges[aHorizontal[a]]));

        while (nNextEnd < aEnding.size() && aEdges[aEnding[nNextEnd]].maSecond.mnY == nY)
            aActive.erase(aEnding[nNextEnd++]);

        const size_t nStartBegin(nNextStart);
        while (nNextStart < aSloped.size() && aEdges[aSloped[nNextStart]].maFirst.mnY == nY)
            nNextStart++;
        // left to right, so the left neighbour of each new piece has its winding already
        std::sort(aSloped.begin() + nStartBegin, aSloped.begin() + nNextStart,
                  [&](int nLeft, int nRight) { return aActive.less(nLeft, nRight); });
        for (size_t a = nStartBegin; a < nNextStart; a++)
        {
            const int nEdge(aSloped[a]);
            const Edge& rEdge(aEdges[nEdge]);
            const auto it(aActive.insert(nEdge));
            const Winding aLeft(it == aActive.begin() ? Winding()
                                                      : aWindingRight[aActive.edge(std::prev(it))]);
            const Winding aRight(aLeft + rEdge.maDelta);
            aWindingRight[nEdge] = aRight;
            bNeedsChange |= isEvenNotZero(aLeft.mnFirst) || isEvenNotZero(aRight.mnFirst);
            const bool bLeftInside(isInside(aLeft));
            if (bLeftInside != isInside(aRight))
            {
                // seen from maFirst to maSecond the side with smaller x is on the left
                if (bLeftInside)
                    aBorders.push_back(Border{ rEdge.maFirst, rEdge.maSecond, nEdge });
                else
                    aBorders.push_back(Border{ rEdge.maSecond, rEdge.maFirst, nEdge });
            }
        }

        for (size_t a = nHorizontalBegin; a < nNextHorizontal; a++)
        {
            const Edge& rEdge(aEdges[aHorizontal[a]]);
            const Winding aAbove(aAboveOfHorizontal[a - nHorizontalBegin]);
            const Winding aBelow(getWindingLeftOf(rEdge));
            bNeedsChange |= isEvenNotZero(aBelow.mnFirst) || isEvenNotZero(aAbove.mnFirst);
            const bool bBelowInside(isInside(aBelow));
            if (bBelowInside != isInside(aAbove))
            {
                // seen from maFirst to maSecond (x growing) the side below (y growing) is on the
                // left
                if (bBelowInside)
                    aBorders.push_back(Border{ rEdge.maFirst, rEdge.maSecond, aHorizontal[a] });
                else
                    aBorders.push_back(Border{ rEdge.maSecond, rEdge.maFirst, aHorizontal[a] });
            }
        }
    }

    if (!bNeedsChange)
        return rCandidate;

    // pass 3: the border parts, each once, turned so that the inside is on the left
    std::vector<Part> aParts;
    {
        std::map<std::tuple<sal_Int32, sal_uInt32, int>, size_t> aPartIndex;
        int nDirectionConflicts(0);
        for (const Border& rBorder : aBorders)
        {
            const Edge& rEdge(aEdges[rBorder.mnEdge]);
            const std::vector<Cut>& rCuts(
                aCuts[rEdge.maOrigin.mnPolygon][rEdge.maOrigin.mnSegment]);
            const double fMiddleT(0.5 * (rEdge.mfFirstT + rEdge.mfSecondT));
            const int nPart(std::clamp(
                int(std::upper_bound(rCuts.begin(), rCuts.end(), fMiddleT,
                                     [](double fT, const Cut& rCut) { return fT < rCut.mfT; })
                    - rCuts.begin())
                    - 1,
                0, int(rCuts.size()) - 2));
            const bool bFromFirst(rBorder.maFrom == rEdge.maFirst);
            const bool bForward(bFromFirst ? rEdge.mfSecondT > rEdge.mfFirstT
                                           : rEdge.mfFirstT > rEdge.mfSecondT);
            const auto [it, bNew] = aPartIndex.try_emplace(
                { rEdge.maOrigin.mnPolygon, rEdge.maOrigin.mnSegment, nPart }, aParts.size());
            if (bNew)
            {
                const GridPoint& rLow(rCuts[nPart].maVertex);
                const GridPoint& rHigh(rCuts[nPart + 1].maVertex);
                aParts.push_back(Part{ rEdge.maOrigin.mnPolygon, rEdge.maOrigin.mnSegment, nPart,
                                       bForward, bForward ? rLow : rHigh,
                                       bForward ? rHigh : rLow });
            }
            else if (aParts[it->second].mbForward != bForward)
                nDirectionConflicts++;
        }
        SAL_WARN_IF(nDirectionConflicts, "basegfx",
                    nDirectionConflicts << " border parts run both ways");
    }

    // the point of every vertex where border parts meet. An input point stays exact. For a new
    // crossing we look for the crossing of the true segments near it, other segments through it
    // get their nearest t. That t goes back into the cut arrays, so all parts at a cut use the
    // same point
    std::unordered_map<GridPoint, B2DPoint, GridPointHash> aVertexPoints;
    {
        auto getFlatInterval =
            [&](sal_Int32 nPolygon, sal_uInt32 nSegment, double fT, double& rStartT, double& rEndT)
        {
            const FlatPolygon& rFlat(aFlats[nPolygon]);
            const auto itBegin(
                std::lower_bound(rFlat.maSegments.begin(), rFlat.maSegments.end(), nSegment));
            const auto itEnd(std::upper_bound(itBegin, rFlat.maSegments.end(), nSegment));
            std::vector<double> aParameters(
                rFlat.maParameters.begin() + (itBegin - rFlat.maSegments.begin()),
                rFlat.maParameters.begin() + (itEnd - rFlat.maSegments.begin()));
            aParameters.push_back(1.0);
            const int nLast(int(aParameters.size()) - 1);
            const int nIndex(
                std::clamp(int(std::upper_bound(aParameters.begin(), aParameters.end(), fT)
                               - aParameters.begin())
                               - 1,
                           0, nLast - 1));
            // one flat interval more on each side, for a cut on a flat point
            rStartT = aParameters[std::max(nIndex - 1, 0)];
            rEndT = aParameters[std::min(nIndex + 2, nLast)];
        };
        std::unordered_map<GridPoint, std::vector<CutReference>, GridPointHash> aCutsAt;
        for (const Part& rPart : aParts)
        {
            aCutsAt.try_emplace(rPart.maFrom);
            aCutsAt.try_emplace(rPart.maTo);
        }
        for (size_t a = 0; a < aCuts.size(); a++)
            for (size_t b = 0; b < aCuts[a].size(); b++)
                for (size_t c = 0; c < aCuts[a][b].size(); c++)
                {
                    const auto it(aCutsAt.find(aCuts[a][b][c].maVertex));
                    if (it != aCutsAt.end())
                        it->second.push_back(CutReference{ sal_Int32(a), sal_uInt32(b), c });
                }
        auto getT = [&](const CutReference& rReference) -> double&
        { return aCuts[rReference.mnPolygon][rReference.mnSegment][rReference.mnIndex].mfT; };
        // the box search stops at fEpsilon, Newton steps polish from there
        const double fEpsilon(1e-7 * fExtent);
        // a new parameter stays between the neighbouring cuts
        auto fits = [&](const CutReference& rReference, double fT)
        {
            const std::vector<Cut>& rCuts(aCuts[rReference.mnPolygon][rReference.mnSegment]);
            return rReference.mnIndex != 0 && rReference.mnIndex + 1 != rCuts.size()
                   && fT > rCuts[rReference.mnIndex - 1].mfT
                   && fT < rCuts[rReference.mnIndex + 1].mfT;
        };
        // a segment whose own flat point lies on the vertex has the exact parameter already
        auto isFlatPoint = [&](const CutReference& rReference, const GridPoint& rVertex)
        {
            const FlatPolygon& rFlat(aFlats[rReference.mnPolygon]);
            const double fT(getT(rReference));
            const auto itBegin(std::lower_bound(rFlat.maSegments.begin(), rFlat.maSegments.end(),
                                                rReference.mnSegment));
            const auto itEnd(
                std::upper_bound(itBegin, rFlat.maSegments.end(), rReference.mnSegment));
            const auto itParameterBegin(rFlat.maParameters.begin()
                                        + (itBegin - rFlat.maSegments.begin()));
            const auto itParameterEnd(rFlat.maParameters.begin()
                                      + (itEnd - rFlat.maSegments.begin()));
            const auto it(std::lower_bound(itParameterBegin, itParameterEnd, fT));
            return it != itParameterEnd && *it == fT
                   && toGrid(aFlattened[rReference.mnPolygon].getB2DPoint(
                          it - rFlat.maParameters.begin()))
                          == rVertex;
        };
        for (auto& [rVertex, rReferences] : aCutsAt)
        {
            const auto itSource(aSources.find(rVertex));
            B2DPoint aPoint(
                itSource != aSources.end()
                    ? aFlattened[itSource->second.mnPolygon].getB2DPoint(itSource->second.mnPoint)
                    : B2DPoint(rVertex.mnX / fScale + fMinX, rVertex.mnY / fScale + fMinY));
            const CutReference* pEnd(nullptr);
            for (const CutReference& rReference : rReferences)
                if (getT(rReference) == 0.0 || getT(rReference) == 1.0)
                    pEnd = &rReference;
            std::vector<const CutReference*> aOpen;
            for (const CutReference& rReference : rReferences)
                if (&rReference != pEnd && rReference.mnIndex != 0
                    && rReference.mnIndex + 1
                           != aCuts[rReference.mnPolygon][rReference.mnSegment].size()
                    && !isFlatPoint(rReference, rVertex))
                    aOpen.push_back(&rReference);
            if (pEnd)
            {
                const B2DCubicBezier& rEndCurve(aCurves[pEnd->mnPolygon][pEnd->mnSegment]);
                aPoint = getT(*pEnd) == 0.0 ? rEndCurve.getStartPoint() : rEndCurve.getEndPoint();
            }
            else if (itSource == aSources.end() && aOpen.size() >= 2)
            {
                const CutReference& rA(*aOpen[0]);
                const CutReference& rB(*aOpen[1]);
                const B2DCubicBezier& rCurveA(aCurves[rA.mnPolygon][rA.mnSegment]);
                const B2DCubicBezier& rCurveB(aCurves[rB.mnPolygon][rB.mnSegment]);
                double fStartS, fEndS, fStartU, fEndU, fS, fU;
                getFlatInterval(rA.mnPolygon, rA.mnSegment, getT(rA), fStartS, fEndS);
                getFlatInterval(rB.mnPolygon, rB.mnSegment, getT(rB), fStartU, fEndU);
                const bool bFound(crossByBisection(rCurveA, fStartS, fEndS, rCurveB, fStartU, fEndU,
                                                   aPoint, fEpsilon, fS, fU));
                double fNewtonS(fS), fNewtonU(fU);
                if (findCrossingByNewton(rCurveA, fNewtonS, rCurveB, fNewtonU, 1e-12 * fExtent)
                    && B2DVector(getPointAt(rCurveA, fNewtonS) - getPointAt(rCurveA, fS))
                               .getLength()
                           <= 10.0 * fEpsilon)
                {
                    fS = fNewtonS;
                    fU = fNewtonU;
                }
                const B2DPoint aCrossing(getPointAt(rCurveA, fS));
                // both segments really meet there, near the crossing of the flat edges; a segment
                // that crosses itself keeps the order of both cuts
                if (bFound
                    && B2DVector(aCrossing - getPointAt(rCurveB, fU)).getLength() <= 4.0 * fEpsilon
                    && B2DVector(aCrossing - aPoint).getLength() <= 1e-3 * fExtent && fits(rA, fS)
                    && fits(rB, fU)
                    && !(rA.mnPolygon == rB.mnPolygon && rA.mnSegment == rB.mnSegment
                         && (fS < fU) != (getT(rA) < getT(rB))))
                {
                    getT(rA) = fS;
                    getT(rB) = fU;
                    aPoint = aCrossing;
                    aOpen.erase(aOpen.begin(), aOpen.begin() + 2);
                }
            }
            for (const CutReference* pReference : aOpen)
            {
                double fStartT, fEndT;
                getFlatInterval(pReference->mnPolygon, pReference->mnSegment, getT(*pReference),
                                fStartT, fEndT);
                const double fT(
                    projectByBisection(aCurves[pReference->mnPolygon][pReference->mnSegment],
                                       fStartT, fEndT, aPoint, fEpsilon));
                if (fits(*pReference, fT))
                    getT(*pReference) = fT;
            }
            aVertexPoints[rVertex] = aPoint;
        }
    }

    // link the border parts into loops; parts that follow each other on one segment in one
    // direction become one run, cut out of the input segment as a whole
    std::unordered_map<GridPoint, std::vector<int>, GridPointHash> aOutgoing;
    for (size_t a = 0; a < aParts.size(); a++)
        aOutgoing[aParts[a].maFrom].push_back(int(a));
    std::vector<bool> aUsed(aParts.size(), false);
    auto continues = [&](int nPartA, int nPartB)
    {
        const Part& rA(aParts[nPartA]);
        const Part& rB(aParts[nPartB]);
        return rA.mnPolygon == rB.mnPolygon && rA.mnSegment == rB.mnSegment
               && rA.mbForward == rB.mbForward && rB.mnPart == rA.mnPart + (rA.mbForward ? 1 : -1);
    };
    B2DPolyPolygon aRetval;
    int nDeadEnds(0);
    std::vector<std::pair<B2DPoint, B2DPoint>> aControls;
    std::vector<sal_uInt32> aCurveAt;
    // where a loop starts and its place in the result: the input point with the lowest polygon
    // and point index, so a loop nothing cut starts where its input did; a loop with no input
    // point goes after those, by the place of its first cut on the input
    typedef std::tuple<int, sal_Int32, sal_uInt32, double> LoopKey;
    auto getKey = [&](const Part& rPart)
    {
        const auto it(aSources.find(rPart.maFrom));
        if (it != aSources.end())
            return LoopKey(0, sal_Int32(it->second.mnPolygon), it->second.mnPoint, 0.0);
        return LoopKey(
            1, rPart.mnPolygon, rPart.mnSegment,
            aCuts[rPart.mnPolygon][rPart.mnSegment][rPart.mnPart + (rPart.mbForward ? 0 : 1)].mfT);
    };
    std::vector<std::pair<LoopKey, B2DPolygon>> aLoops;
    // one closed loop of parts becomes one polygon, starting at the beginning of a run
    auto appendLoop = [&](const std::vector<int>& rLoop)
    {
        const size_t nLength(rLoop.size());
        size_t nFirst(nLength);
        LoopKey aKey;
        for (size_t b = 0; b < nLength; b++)
        {
            if (nLength > 1 && continues(rLoop[(b + nLength - 1) % nLength], rLoop[b]))
                continue;
            const LoopKey aCandidate(getKey(aParts[rLoop[b]]));
            if (nFirst == nLength || aCandidate < aKey)
            {
                nFirst = b;
                aKey = aCandidate;
            }
        }
        if (nFirst == nLength)
        {
            nFirst = 0;
            aKey = getKey(aParts[rLoop[0]]);
        }
        B2DPolygon aPolygon;
        // control points get set once the polygon has all its points
        aControls.clear();
        aCurveAt.clear();
        for (size_t b = 0; b < nLength;)
        {
            const Part& rFirst(aParts[rLoop[(nFirst + b) % nLength]]);
            size_t c(b + 1);
            while (c < nLength
                   && continues(rLoop[(nFirst + c - 1) % nLength], rLoop[(nFirst + c) % nLength]))
                c++;
            const Part& rLast(aParts[rLoop[(nFirst + c - 1) % nLength]]);
            const std::vector<Cut>& rCuts(aCuts[rFirst.mnPolygon][rFirst.mnSegment]);
            const double fStartT(rCuts[rFirst.mnPart + (rFirst.mbForward ? 0 : 1)].mfT);
            const double fEndT(rCuts[rLast.mnPart + (rLast.mbForward ? 1 : 0)].mfT);
            const B2DPoint aStartPoint(aVertexPoints[rFirst.maFrom]);
            const B2DPoint aEndPoint(aVertexPoints[rLast.maTo]);
            aPolygon.append(aStartPoint);
            const B2DCubicBezier& rSegment(aCurves[rFirst.mnPolygon][rFirst.mnSegment]);
            if (rSegment.isBezier())
            {
                const bool bReverse(fStartT > fEndT);
                const double fLowT(std::min(fStartT, fEndT)), fHighT(std::max(fStartT, fEndT));
                const B2DCubicBezier aPart(
                    fLowT == 0.0 && fHighT == 1.0 ? rSegment : rSegment.snippet(fLowT, fHighT));
                B2DPoint aControlA(bReverse ? aPart.getControlPointB() : aPart.getControlPointA());
                B2DPoint aControlB(bReverse ? aPart.getControlPointA() : aPart.getControlPointB());
                // the part ends where the next run begins, its control points move along
                aControlA += aStartPoint - (bReverse ? aPart.getEndPoint() : aPart.getStartPoint());
                aControlB += aEndPoint - (bReverse ? aPart.getStartPoint() : aPart.getEndPoint());
                aControls.emplace_back(aControlA, aControlB);
                aCurveAt.push_back(aPolygon.count() - 1);
            }
            b = c;
        }
        const sal_uInt32 nCount(aPolygon.count());
        for (size_t b = 0; b < aCurveAt.size(); b++)
        {
            aPolygon.setNextControlPoint(aCurveAt[b], aControls[b].first);
            aPolygon.setPrevControlPoint((aCurveAt[b] + 1) % nCount, aControls[b].second);
        }
        aPolygon.setClosed(true);
        aLoops.emplace_back(aKey, aPolygon);
    };
    std::vector<int> aLoop, aPending;
    std::unordered_map<GridPoint, size_t, GridPointHash> aIndexOfVertex;
    for (size_t a = 0; a < aParts.size(); a++)
    {
        if (aUsed[a])
            continue;
        aLoop.clear();
        const GridPoint aStart(aParts[a].maFrom);
        int nCurrent(static_cast<int>(a));
        bool bClosed(false);
        while (true)
        {
            aUsed[nCurrent] = true;
            aLoop.push_back(nCurrent);
            const GridPoint aEnd(aParts[nCurrent].maTo);
            if (aEnd == aStart)
            {
                bClosed = true;
                break;
            }
            std::vector<int>& rOutgoing(aOutgoing[aEnd]);
            std::erase_if(rOutgoing, [&aUsed](int nPart) { return aUsed[nPart]; });
            if (rOutgoing.empty())
                break;
            nCurrent = rOutgoing.back();
            rOutgoing.pop_back();
        }
        if (!bClosed)
        {
            nDeadEnds++;
            continue;
        }

        // a loop through one vertex twice is two loops that touch there, every one gets its own
        // polygon; the inside stays on the left, so a touching hole is a hole
        aPending.clear();
        aIndexOfVertex.clear();
        for (const int nPart : aLoop)
        {
            const auto it(aIndexOfVertex.find(aParts[nPart].maFrom));
            if (it != aIndexOfVertex.end())
            {
                const size_t nBegin(it->second);
                for (size_t b = nBegin; b < aPending.size(); b++)
                    aIndexOfVertex.erase(aParts[aPending[b]].maFrom);
                appendLoop(std::vector<int>(aPending.begin() + nBegin, aPending.end()));
                aPending.resize(nBegin);
            }
            aIndexOfVertex[aParts[nPart].maFrom] = aPending.size();
            aPending.push_back(nPart);
        }
        appendLoop(aPending);
    }
    SAL_WARN_IF(nDeadEnds, "basegfx", nDeadEnds << " border loops do not close");
    std::stable_sort(aLoops.begin(), aLoops.end(), [](const auto& rLeft, const auto& rRight)
                     { return rLeft.first < rRight.first; });
    for (const auto& rLoop : aLoops)
        aRetval.append(rLoop.second);
    return aRetval;
}

}
}

namespace basegfx::utils
{
B2DPolyPolygon solvePolygonsBySweep(const B2DPolyPolygon& rFirst, SweepFillRule eFirstRule,
                                    const B2DPolyPolygon& rSecond, SweepFillRule eSecondRule,
                                    SweepOperation eOperation, size_t* pSplitLimit)
{
    B2DPolyPolygon aCandidate(rFirst);
    aCandidate.append(rSecond);
    std::optional<B2DPolyPolygon> oResult(solve(aCandidate, rFirst.count(), eFirstRule, eSecondRule,
                                                eOperation,
                                                pSplitLimit ? *pSplitLimit : nDefaultSplitLimit));
    if (oResult)
        return *oResult;

    if (pSplitLimit)
        *pSplitLimit = 0;
    SAL_WARN("basegfx", "these polygons cross each other too often to be solved in time, or a "
                        "coordinate does not fit, the result ignores the crossings");
    switch (eOperation)
    {
        case SweepOperation::Fill:
            return rFirst;
        case SweepOperation::Or:
        case SweepOperation::Xor:
            return aCandidate;
        case SweepOperation::And:
            return B2DPolyPolygon();
        case SweepOperation::Diff:
            return rFirst;
    }
    return rFirst;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
