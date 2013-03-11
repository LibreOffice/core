/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_DPRESFILTER_HXX__
#define __SC_DPRESFILTER_HXX__

#include "dpitemdata.hxx"

#include <vector>
#include <boost/noncopyable.hpp>

#if DEBUG_PIVOT_TABLE
#include <map>
#else
#include <boost/unordered_map.hpp>
#endif

struct ScDPResultFilter
{
    OUString maDimName;
    ScDPItemData maValue;

    bool mbHasValue:1;
    bool mbDataLayout:1;

    ScDPResultFilter(const OUString& rDimName, bool bDataLayout);
};

class ScDPResultFilterSet : boost::noncopyable
{
    struct MemberNode;
    struct DimensionNode;
#if DEBUG_PIVOT_TABLE
    // To keep the entries sorted in the tree dump.
    typedef std::map<ScDPItemData, MemberNode*> MembersType;
    typedef std::map<OUString, DimensionNode*> DimensionsType;
#else
    typedef boost::unordered_map<ScDPItemData, MemberNode*, ScDPItemData::Hash> MembersType;
    typedef boost::unordered_map<OUString, DimensionNode*, OUStringHash> DimensionsType;
#endif
    typedef std::vector<double> ValuesType;

    struct DimensionNode : boost::noncopyable
    {
        const MemberNode* mpParent;
        MembersType maChildMembers;

        DimensionNode(const MemberNode* pParent);
        ~DimensionNode();

#if DEBUG_PIVOT_TABLE
        void dump(int nLevel) const;
#endif
    };

    struct MemberNode : boost::noncopyable
    {
        const DimensionNode* mpParent;
        double mfValue;
        ValuesType maValues;
        DimensionsType maChildDimensions;

        MemberNode(const DimensionNode* pParent);
        ~MemberNode();

#if DEBUG_PIVOT_TABLE
        void dump(int nLevel) const;
#endif
    };

    MemberNode* mpRoot;

public:
    ScDPResultFilterSet();
    ~ScDPResultFilterSet();

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

    void swap(ScDPResultFilterSet& rOther);

#if DEBUG_PIVOT_TABLE
    void dump() const;
#endif
};

struct ScDPResultFilterContext
{
    ScDPResultFilterSet maFilterSet;
    std::vector<ScDPResultFilter> maFilters;
    long mnCol;
    long mnRow;

    ScDPResultFilterContext();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
