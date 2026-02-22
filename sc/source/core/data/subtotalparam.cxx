/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <dputil.hxx>
#include <subtotalparam.hxx>

#include <osl/diagnose.h>

#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/uno/Sequence.hxx>

ScSubTotalParam::SubtotalGroup::SubtotalGroup(const SubtotalGroup& r)
    : bActive(r.bActive)
    , nField(r.nField)
{
    if (r.nSubTotals > 0)
    {
        assert(r.pSubTotals);
        AllocSubTotals(r.nSubTotals);
        std::copy_n(r.pSubTotals.get(), r.nSubTotals, pSubTotals.get());
    }

    if (r.nCustFuncs > 0)
    {
        assert(r.pCustFuncs);
        AllocCustFuncs(r.nCustFuncs);
        for (SCCOL i = 0; i < r.nCustFuncs; ++i)
        {
            if (r.pCustFuncs[i].second)
                pCustFuncs[i] = std::make_pair(r.pCustFuncs[i].first, r.pCustFuncs[i].second->Clone());
            else
                pCustFuncs[i] = std::make_pair(r.pCustFuncs[i].first, nullptr);
        }
    }

    if (r.nSubLabels > 0)
    {
        assert(r.pSubLabels);
        AllocSubLabels(r.nSubLabels);
        std::copy_n(r.pSubLabels.get(), r.nSubLabels, pSubLabels.get());
    }
}

ScSubTotalParam::SubtotalGroup& ScSubTotalParam::SubtotalGroup::operator=(const SubtotalGroup& r)
{
    bActive = r.bActive;
    nField = r.nField;

    AllocSubTotals(r.nSubTotals);
    if (r.nSubTotals > 0)
    {
        assert(r.pSubTotals);
        std::copy_n(r.pSubTotals.get(), r.nSubTotals, pSubTotals.get());
    }

    AllocCustFuncs(r.nCustFuncs);
    if (r.nCustFuncs > 0)
    {
        assert(r.pCustFuncs);
        for (SCCOL i = 0; i < r.nCustFuncs; ++i)
        {
            if (r.pCustFuncs[i].second)
                pCustFuncs[i] = std::make_pair(r.pCustFuncs[i].first, r.pCustFuncs[i].second->Clone());
            else
                pCustFuncs[i] = std::make_pair(r.pCustFuncs[i].first, nullptr);
        }
    }

    AllocSubLabels(r.nSubLabels);
    if (r.nSubLabels > 0)
    {
        assert(r.pSubLabels);
        std::copy_n(r.pSubLabels.get(), r.nSubLabels, pSubLabels.get());
    }

    return *this;
}

bool ScSubTotalParam::SubtotalGroup::operator==(const SubtotalGroup& r) const
{
    return bActive == r.bActive && nField == r.nField && nSubTotals == r.nSubTotals && nSubLabels == r.nSubLabels
           && (!nSubTotals
               || std::equal(pSubTotals.get(), pSubTotals.get() + nSubTotals, r.pSubTotals.get()))
           && (!nCustFuncs
               || std::equal(pCustFuncs.get(), pCustFuncs.get() + nCustFuncs, r.pCustFuncs.get()))
           && (!nSubLabels
               || std::equal(pSubLabels.get(), pSubLabels.get() + nSubLabels, r.pSubLabels.get()));
}

void ScSubTotalParam::SubtotalGroup::AllocSubTotals(SCCOL n)
{
    if (nSubTotals != n)
    {
        nSubTotals = std::max(n, SCCOL(0));
        pSubTotals.reset(nSubTotals ? new std::pair<SCCOL, ScSubTotalFunc>[nSubTotals] : nullptr);
    }
}

void ScSubTotalParam::SubtotalGroup::AllocCustFuncs(SCCOL n)
{
    if (nCustFuncs != n)
    {
        nCustFuncs = std::max(n, SCCOL(0));
        pCustFuncs.reset(nCustFuncs ? new std::pair<SCCOL, std::unique_ptr<ScTokenArray>>[nCustFuncs] : nullptr);
    }
}

void ScSubTotalParam::SubtotalGroup::AllocSubLabels(SCCOL n)
{
    if (nSubLabels != n)
    {
        nSubLabels = std::max(n, SCCOL(0));
        pSubLabels.reset(nSubLabels ? new std::pair<SCCOL, rtl::OUString>[nSubLabels] : nullptr);
    }
}

void ScSubTotalParam::SubtotalGroup::SetSubtotals(const css::uno::Sequence<css::sheet::SubTotalColumn>& seq)
{
    AllocSubTotals(seq.getLength());
    for (SCCOL i = 0; i < nSubTotals; ++i)
        pSubTotals[i] = { seq[i].Column,
                          ScDPUtil::toSubTotalFunc(static_cast<ScGeneralFunction>(seq[i].Function)) };
}

//void ScSubTotalParam::SubtotalGroup::SetCustFuncs(const css::uno::Sequence<css::sheet::SubTotalColumn>& /*seq*/)
//{
// TODO UNO::API: SubTotalColumn has no token array member
//}

//void ScSubTotalParam::SubtotalGroup::SetSublabels(const css::uno::Sequence<css::sheet::SubTotalColumn>& /*seq*/)
//{
// TODO UNO::API: SubTotalColumn has no LabelName member
//}

void ScSubTotalParam::SetSubTotals( sal_uInt16 nGroup,
                                    const SCCOL* ptrSubTotals,
                                    const ScSubTotalFunc* ptrFunctions,
                                    sal_uInt16 nCount )
{
    OSL_ENSURE( (nGroup <= MAXSUBTOTAL),
                "ScSubTotalParam::SetSubTotals(): nGroup > MAXSUBTOTAL!" );
    OSL_ENSURE( ptrSubTotals,
                "ScSubTotalParam::SetSubTotals(): ptrSubTotals == NULL!" );
    OSL_ENSURE( ptrFunctions,
                "ScSubTotalParam::SetSubTotals(): ptrFunctions == NULL!" );
    OSL_ENSURE( (nCount > 0),
                "ScSubTotalParam::SetSubTotals(): nCount == 0!" );

    if ( !(ptrSubTotals && ptrFunctions && (nCount > 0) && (nGroup <= MAXSUBTOTAL)) )
        return;

    // 0 is interpreted as 1, otherwise decrementing the array index
    if (nGroup != 0)
        nGroup--;

    aGroups[nGroup].AllocSubTotals(nCount);
    for ( sal_uInt16 i=0; i<nCount; i++ )
        aGroups[nGroup].pSubTotals[i] = { ptrSubTotals[i], ptrFunctions[i] };
}

void ScSubTotalParam::SetCustFuncs(sal_uInt16 nGroupIdx,
                       std::vector<std::pair<SCCOL, std::unique_ptr<ScTokenArray>>>& rColFuncs,
                       sal_uInt16 nCount )
{
    OSL_ENSURE((nGroupIdx < MAXSUBTOTAL), "ScSubTotalParam::SetCustFuncs(): nGroupIdx >= MAXSUBTOTAL!");
    if (nGroupIdx >= MAXSUBTOTAL)
        return;

    aGroups[nGroupIdx].AllocCustFuncs(nCount);
    for (sal_uInt16 i = 0; i < nCount; i++)
        aGroups[nGroupIdx].pCustFuncs[i] = std::make_pair(rColFuncs[i].first, std::move(rColFuncs[i].second));
}

void ScSubTotalParam::SetSubLabels(sal_uInt16 nGroupIdx,
                       std::vector<std::pair<SCCOL, rtl::OUString>>& rColLabels,
                       sal_uInt16 nCount )
{
    OSL_ENSURE((nGroupIdx < MAXSUBTOTAL), "ScSubTotalParam::SetSubLabels(): nGroupIdx > MAXSUBTOTAL!");
    if (nGroupIdx >= MAXSUBTOTAL)
        return;

    aGroups[nGroupIdx].AllocSubLabels(nCount);
    for (sal_uInt16 i = 0; i < nCount; i++)
        aGroups[nGroupIdx].pSubLabels[i] = std::make_pair(rColLabels[i].first, std::move(rColLabels[i].second));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
