/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <recursionhelper.hxx>
#include <formulacell.hxx>

void ScRecursionHelper::Init()
{
    nRecursionCount    = 0;
    nDependencyComputationLevel = 0;
    bInRecursionReturn = bDoingRecursion = bInIterationReturn = false;
    aInsertPos = GetIterationEnd();
    ResetIteration();
    // Must not force clear aFGList ever.
}

void ScRecursionHelper::ResetIteration()
{
    aLastIterationStart = GetIterationEnd();
    nIteration = 0;
    bConverging = false;
}

ScRecursionHelper::ScRecursionHelper()
{
    Init();
}

void ScRecursionHelper::SetInRecursionReturn( bool b )
{
    // Do not use IsInRecursionReturn() here, it decouples iteration.
    if (b && !bInRecursionReturn)
        aInsertPos = aRecursionFormulas.begin();
    bInRecursionReturn = b;
}

void ScRecursionHelper::Insert(
    ScFormulaCell* p, bool bOldRunning, const ScFormulaResult & rRes )
{
    aRecursionFormulas.insert( aInsertPos, ScFormulaRecursionEntry( p,
                bOldRunning, rRes));
}

void ScRecursionHelper::SetInIterationReturn( bool b )
{
    // An iteration return is always coupled to a recursion return.
    SetInRecursionReturn( b);
    bInIterationReturn = b;
}

void ScRecursionHelper::StartIteration()
{
    SetInIterationReturn( false);
    nIteration = 1;
    bConverging = false;
    aLastIterationStart = GetIterationStart();
}

void ScRecursionHelper::ResumeIteration()
{
    SetInIterationReturn( false);
    aLastIterationStart = GetIterationStart();
}

void ScRecursionHelper::IncIteration()
{
    ++nIteration;
}

void ScRecursionHelper::EndIteration()
{
    aRecursionFormulas.erase( GetIterationStart(), GetIterationEnd());
    ResetIteration();
}

ScFormulaRecursionList::iterator ScRecursionHelper::GetIterationStart()
{
    return aRecursionFormulas.begin();
}

ScFormulaRecursionList::iterator ScRecursionHelper::GetIterationEnd()
{
    return aRecursionFormulas.end();
}

void ScRecursionHelper::Clear()
{
    aRecursionFormulas.clear();
    while (!aRecursionInIterationStack.empty())
        aRecursionInIterationStack.pop();
    Init();
}

static ScFormulaCell* lcl_GetTopCell(ScFormulaCell* pCell)
{
    if (!pCell)
        return nullptr;

    const ScFormulaCellGroupRef& mxGroup = pCell->GetCellGroup();
    if (!mxGroup)
        return pCell;
    return mxGroup->mpTopCell;
}

bool ScRecursionHelper::PushFormulaGroup(ScFormulaCell* pCell)
{
    assert(pCell);

    if (pCell->GetSeenInPath())
    {
        // Found a simple cycle of formula-groups.
        // Disable group calc for all elements of this cycle.
        sal_Int32 nIdx = aFGList.size();
        assert(nIdx > 0);
        do
        {
            --nIdx;
            assert(nIdx >= 0);
            const ScFormulaCellGroupRef& mxGroup = aFGList[nIdx]->GetCellGroup();
            if (mxGroup)
                mxGroup->mbPartOfCycle = true;
        } while (aFGList[nIdx] != pCell);

        return false;
    }

    pCell->SetSeenInPath(true);
    aFGList.push_back(pCell);
    return true;
}

void ScRecursionHelper::PopFormulaGroup()
{
    if (aFGList.empty())
        return;
    ScFormulaCell* pCell = aFGList.back();
    pCell->SetSeenInPath(false);
    aFGList.pop_back();
}

bool ScRecursionHelper::AnyParentFGInCycle()
{
    sal_Int32 nIdx = aFGList.size() - 1;
    while (nIdx >= 0)
    {
        const ScFormulaCellGroupRef& mxGroup = aFGList[nIdx]->GetCellGroup();
        if (mxGroup)
            return mxGroup->mbPartOfCycle;
        --nIdx;
    };
    return false;
}

void ScRecursionHelper::AddTemporaryGroupCell(ScFormulaCell* cell)
{
    aTemporaryGroupCells.push_back( cell );
}

void ScRecursionHelper::CleanTemporaryGroupCells()
{
    if( GetRecursionCount() == 0 )
    {
        for( ScFormulaCell* cell : aTemporaryGroupCells )
            cell->SetCellGroup( nullptr );
        aTemporaryGroupCells.clear();
    }
}

ScFormulaGroupCycleCheckGuard::ScFormulaGroupCycleCheckGuard(ScRecursionHelper& rRecursionHelper, ScFormulaCell* pCell) :
    mrRecHelper(rRecursionHelper)
{
    if (pCell)
    {
        pCell = lcl_GetTopCell(pCell);
        mbShouldPop = mrRecHelper.PushFormulaGroup(pCell);
    }
    else
        mbShouldPop = false;
}

ScFormulaGroupCycleCheckGuard::~ScFormulaGroupCycleCheckGuard()
{
    if (mbShouldPop)
        mrRecHelper.PopFormulaGroup();
}

ScFormulaGroupDependencyComputeGuard::ScFormulaGroupDependencyComputeGuard(ScRecursionHelper& rRecursionHelper) :
    mrRecHelper(rRecursionHelper)
{
    mrRecHelper.IncDepComputeLevel();
}

ScFormulaGroupDependencyComputeGuard::~ScFormulaGroupDependencyComputeGuard()
{
    mrRecHelper.DecDepComputeLevel();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
