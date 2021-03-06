/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include "calcmacros.hxx"

#include <memory>
#include <map>
#include <vector>
#include <unordered_map>

namespace com::sun::star::uno
{
template <typename> class Sequence;
}
namespace com::sun::star::sheet
{
struct DataPilotFieldFilter;
}

struct ScDPResultFilter
{
    OUString maDimName;
    OUString maValueName;
    OUString maValue;

    bool mbHasValue : 1;
    bool mbDataLayout : 1;

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
    typedef std::map<OUString, std::shared_ptr<MemberNode>> MembersType;

    struct DimensionNode
    {
        MembersType maChildMembersValueNames;
        MembersType maChildMembersValues;

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
        std::map<OUString, std::unique_ptr<DimensionNode>> maChildDimensions;

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
        size_t operator()(const NamePairType& rPair) const;
    };
    typedef std::unordered_map<NamePairType, double, NamePairHash> LeafValuesType;
    LeafValuesType maLeafValues;

    OUString maPrimaryDimName;
    std::unique_ptr<MemberNode> mpRoot;

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
     * @param fVal result value, as displayed in the table output.
     */
    void add(const std::vector<ScDPResultFilter>& rFilter, double fVal);

    void swap(ScDPResultTree& rOther);

    bool empty() const;
    void clear();

    const ValuesType*
    getResults(const css::uno::Sequence<css::sheet::DataPilotFieldFilter>& rFilters) const;

    double getLeafResult(const css::sheet::DataPilotFieldFilter& rFilter) const;

#if DEBUG_PIVOT_TABLE
    void dump() const;
#endif
};

struct ScDPResultFilterContext
{
    ScDPResultTree maFilterSet;
    std::vector<ScDPResultFilter> maFilters;
    sal_Int32 mnCol;
    sal_Int32 mnRow;

    ScDPResultFilterContext();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
