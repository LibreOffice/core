/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dpresfilter.hxx"
#include "global.hxx"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

using namespace com::sun::star;
using namespace std;

ScDPResultFilter::ScDPResultFilter(const OUString& rDimName, bool bDataLayout) :
    maDimName(rDimName), mbHasValue(false), mbDataLayout(bDataLayout) {}

ScDPResultFilterContext::ScDPResultFilterContext() :
    mnCol(0), mnRow(0) {}

ScDPResultTree::DimensionNode::DimensionNode(const MemberNode* pParent) :
    mpParent(pParent) {}

ScDPResultTree::DimensionNode::~DimensionNode()
{
    MembersType::iterator it = maChildMembers.begin(), itEnd = maChildMembers.end();
    for (; it != itEnd; ++it)
        delete it->second;
}

#if DEBUG_PIVOT_TABLE
void ScDPResultTree::DimensionNode::dump(int nLevel) const
{
    string aIndent(nLevel*2, ' ');
    MembersType::const_iterator it = maChildMembers.begin(), itEnd = maChildMembers.end();
    for (; it != itEnd; ++it)
    {
        cout << aIndent << "member: ";
        const ScDPItemData& rVal = it->first;
        if (rVal.IsValue())
            cout << rVal.GetValue();
        else
            cout << rVal.GetString();
        cout << endl;

        it->second->dump(nLevel+1);
    }
}
#endif

ScDPResultTree::MemberNode::MemberNode(const DimensionNode* pParent) :
    mpParent(pParent) {}

ScDPResultTree::MemberNode::~MemberNode()
{
    DimensionsType::iterator it = maChildDimensions.begin(), itEnd = maChildDimensions.end();
    for (; it != itEnd; ++it)
        delete it->second;
}

#if DEBUG_PIVOT_TABLE
void ScDPResultTree::MemberNode::dump(int nLevel) const
{
    string aIndent(nLevel*2, ' ');
    ValuesType::const_iterator itVal = maValues.begin(), itValEnd = maValues.end();
    for (; itVal != itValEnd; ++itVal)
        cout << aIndent << "value: " << *itVal << endl;

    DimensionsType::const_iterator it = maChildDimensions.begin(), itEnd = maChildDimensions.end();
    for (; it != itEnd; ++it)
    {
        cout << aIndent << "dimension: " << it->first << endl;
        it->second->dump(nLevel+1);
    }
}
#endif

ScDPResultTree::ScDPResultTree() : mpRoot(new MemberNode(NULL)) {}
ScDPResultTree::~ScDPResultTree()
{
    delete mpRoot;
}

void ScDPResultTree::add(
    const std::vector<ScDPResultFilter>& rFilters, long /*nCol*/, long /*nRow*/, double fVal)
{
    // TODO: I'll work on the col / row to value node mapping later.

    MemberNode* pMemNode = mpRoot;

    std::vector<ScDPResultFilter>::const_iterator itFilter = rFilters.begin(), itFilterEnd = rFilters.end();
    for (; itFilter != itFilterEnd; ++itFilter)
    {
        const ScDPResultFilter& filter = *itFilter;
        if (filter.mbDataLayout)
            continue;

        if (maPrimaryDimName.isEmpty())
            maPrimaryDimName = filter.maDimName;

        // See if this dimension exists.
        DimensionsType& rDims = pMemNode->maChildDimensions;
        DimensionsType::iterator itDim = rDims.find(filter.maDimName);
        if (itDim == rDims.end())
        {
            // New dimenison.  Insert it.
            std::pair<DimensionsType::iterator, bool> r =
                rDims.insert(DimensionsType::value_type(filter.maDimName, new DimensionNode(pMemNode)));

            if (!r.second)
                // Insertion failed!
                return;

            itDim = r.first;
        }

        // Now, see if this dimension member exists.
        DimensionNode* pDim = itDim->second;
        MembersType& rMembers = pDim->maChildMembers;
        MembersType::iterator itMem = rMembers.find(filter.maValue);
        if (itMem == rMembers.end())
        {
            // New member.  Insert it.
            std::pair<MembersType::iterator, bool> r =
                rMembers.insert(
                    MembersType::value_type(filter.maValue, new MemberNode(pDim)));

            if (!r.second)
                // Insertion failed!
                return;

            itMem = r.first;
        }

        pMemNode = itMem->second;
    }

    pMemNode->maValues.push_back(fVal);
}

void ScDPResultTree::swap(ScDPResultTree& rOther)
{
    std::swap(maPrimaryDimName, rOther.maPrimaryDimName);
    std::swap(mpRoot, rOther.mpRoot);
}

bool ScDPResultTree::empty() const
{
    return mpRoot->maChildDimensions.empty();
}

void ScDPResultTree::clear()
{
    maPrimaryDimName = EMPTY_OUSTRING;
    delete mpRoot;
    mpRoot = new MemberNode(NULL);
}

const ScDPResultTree::ValuesType* ScDPResultTree::getResults(
    const uno::Sequence<sheet::DataPilotFieldFilter>& rFilters) const
{
    const sheet::DataPilotFieldFilter* p = rFilters.getConstArray();
    const sheet::DataPilotFieldFilter* pEnd = p + static_cast<size_t>(rFilters.getLength());
    const MemberNode* pMember = mpRoot;
    for (; p != pEnd; ++p)
    {
        DimensionsType::const_iterator itDim = pMember->maChildDimensions.find(p->FieldName);
        if (itDim == pMember->maChildDimensions.end())
            // Specified dimension not found.
            return NULL;

        const DimensionNode* pDim = itDim->second;
        MembersType::const_iterator itMem = pDim->maChildMembers.find(p->MatchValue);
        if (itMem == pDim->maChildMembers.end())
            // Specified member not found.
            return NULL;

        pMember = itMem->second;
    }

    return &pMember->maValues;
}

#if DEBUG_PIVOT_TABLE
void ScDPResultTree::dump() const
{
    cout << "primary dimension name: " << maPrimaryDimName << endl;
    mpRoot->dump(0);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
