/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DPRESFILTER_HXX
#define INCLUDED_SC_INC_DPRESFILTER_HXX

#include "dpitemdata.hxx"

#include <map>
#include <vector>
#include <unordered_map>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

struct ScDPResultFilter
{
    OUString maDimName;
    OUString maValue;

    bool mbHasValue:1;
    bool mbDataLayout:1;

    ScDPResultFilter(const OUString& rDimName, bool bDataLayout);
};

/**
 * This class maintains pivot table calculation result in a tree structure
 * which represents the logical structure of pivot table result layout as
 * presented in the sheet.
 *
 * <p>The root node has two child nodes if the pivot table consists of both
 * column and row dimensions. The first child stores the result tree that is
 * first filtered by row dimensions then by column dimensions. The second
 * child stores the result tree that is filtered by column dimensions only
 * (for column grand totals).</p>
 *
 * <p>If the pivot table layout only consists of either column or row
 * dimensions, the root node only has one child node.</p>
 */
class ScDPResultTree
{
public:
    typedef std::vector<double> ValuesType;

private:

    struct MemberNode;
    struct DimensionNode;
    typedef std::map<OUString, MemberNode*> MembersType;
    typedef std::map<OUString, DimensionNode*> DimensionsType;

    struct DimensionNode
    {
        MembersType maChildMembers;

        DimensionNode();
        DimensionNode(const DimensionNode&) = delete;
        const DimensionNode& operator=(const DimensionNode&) = delete;
        ~DimensionNode();

#if DEBUG_PIVOT_TABLE
        void dump(int nLevel) const;
#endif
    };

    struct MemberNode
    {
        ValuesType maValues;
        DimensionsType maChildDimensions;

        MemberNode();
        MemberNode(const MemberNode&) = delete;
        const MemberNode& operator=(const MemberNode&) = delete;
        ~MemberNode();

#if DEBUG_PIVOT_TABLE
        void dump(int nLevel) const;
#endif
    };

    typedef std::pair<OUString, OUString> NamePairType;

    struct NamePairHash
    {
        size_t operator() (const NamePairType& rPair) const;
    };
    typedef std::unordered_map<NamePairType, double, NamePairHash> LeafValuesType;
    LeafValuesType maLeafValues;

    OUString maPrimaryDimName;
    MemberNode* mpRoot;

public:

    ScDPResultTree();
    ScDPResultTree(const ScDPResultTree&) = delete;
    const ScDPResultTree& operator=(const ScDPResultTree&) = delete;
    ~ScDPResultTree();
    /**
     * Add a single value filter path.  The filters are expected to be sorted
     * by row dimension order then by column dimension order.
     *
     * @param rFilter set of filters.
     * @param nCol column position relative to the top-left cell within the
     *             data field range.
     * @param nRow row position relative to the top-left cell within the data
     *             field range.
     * @param fVal result value, as displayed in the table output.
     */
    void add(const std::vector<ScDPResultFilter>& rFilter, long nCol, long nRow, double fVal);

    void swap(ScDPResultTree& rOther);

    bool empty() const;
    void clear();

    const ValuesType* getResults(
        const css::uno::Sequence< css::sheet::DataPilotFieldFilter>& rFilters) const;

    double getLeafResult(const css::sheet::DataPilotFieldFilter& rFilter) const;

#if DEBUG_PIVOT_TABLE
    void dump() const;
#endif
};

struct ScDPResultFilterContext
{
    ScDPResultTree maFilterSet;
    std::vector<ScDPResultFilter> maFilters;
    long mnCol;
    long mnRow;

    ScDPResultFilterContext();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
