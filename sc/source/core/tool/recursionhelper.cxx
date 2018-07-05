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

bool ScRecursionHelper::PushFormulaGroup(ScFormulaCellGroup* pGrp)
{
    if (!pGrp)
        return false;

    if (pGrp->mbSeenInPath)
    {
        // Found a simple cycle of formula-groups.
        // Disable group calc for all elements of this cycle.
        sal_Int32 nIdx = aFGList.size();
        assert(nIdx > 0);
        do
        {
            --nIdx;
            assert(nIdx >= 0);
            aFGList[nIdx]->mbPartOfCycle = true;
        } while (aFGList[nIdx] != pGrp);

        return false;
    }

    pGrp->mbSeenInPath = true;
    aFGList.push_back(pGrp);
    return true;
}

void ScRecursionHelper::PopFormulaGroup()
{
    if (aFGList.empty())
        return;
    ScFormulaCellGroup* pGrp = aFGList.back();
    pGrp->mbSeenInPath = false;
    aFGList.pop_back();
}

ScFormulaGroupCycleCheckGuard::ScFormulaGroupCycleCheckGuard(ScRecursionHelper& rRecursionHelper, ScFormulaCellGroup* pGrp) :
    mrRecHelper(rRecursionHelper)
{
    mbShouldPop = mrRecHelper.PushFormulaGroup(pGrp);
}

ScFormulaGroupCycleCheckGuard::~ScFormulaGroupCycleCheckGuard()
{
    if (mbShouldPop)
        mrRecHelper.PopFormulaGroup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
