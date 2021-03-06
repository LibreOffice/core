/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include "formularesult.hxx"

#include <list>
#include <vector>
#include <stack>
#include <o3tl/sorted_vector.hxx>

class ScFormulaCell;

struct ScFormulaRecursionEntry
{
    ScFormulaCell*  pCell;
    bool            bOldRunning;
    ScFormulaResult aPreviousResult;
    ScFormulaRecursionEntry(
        ScFormulaCell* p, bool bR, const ScFormulaResult & rRes ) :
        pCell(p), bOldRunning(bR), aPreviousResult( rRes)
    {
    }
};

typedef ::std::list< ScFormulaRecursionEntry > ScFormulaRecursionList;

class ScRecursionHelper
{
    typedef ::std::stack< ScFormulaCell* >  ScRecursionInIterationStack;
    ScFormulaRecursionList              aRecursionFormulas;
    ScFormulaRecursionList::iterator    aInsertPos;
    ScFormulaRecursionList::iterator    aLastIterationStart;
    ScRecursionInIterationStack         aRecursionInIterationStack;
    std::vector< ScFormulaCell* >       aFGList;
    // Flag list corresponding to aFGList to indicate whether each formula-group
    // is in a dependency evaluation mode or not.
    std::vector< bool >                 aInDependencyEvalMode;
    sal_uInt16                              nRecursionCount;
    sal_uInt16                              nIteration;
    // Count of ScFormulaCell::CheckComputeDependencies in current call-stack.
    sal_uInt16                              nDependencyComputationLevel;
    bool                                bInRecursionReturn;
    bool                                bDoingRecursion;
    bool                                bInIterationReturn;
    bool                                bConverging;
    bool                                bGroupsIndependent;
    bool                                bAbortingDependencyComputation;
    std::vector< ScFormulaCell* >       aTemporaryGroupCells;
    o3tl::sorted_vector< ScFormulaCellGroup* >* pFGSet;

    void Init();
    void ResetIteration();

public:

    ScRecursionHelper();
    sal_uInt16  GetRecursionCount() const       { return nRecursionCount; }
    void    IncRecursionCount()             { ++nRecursionCount; }
    void    DecRecursionCount()             { --nRecursionCount; }
    sal_uInt16 GetDepComputeLevel() const   { return nDependencyComputationLevel; }
    void    IncDepComputeLevel();
    void    DecDepComputeLevel();
    /// A pure recursion return, no iteration.
    bool    IsInRecursionReturn() const     { return bInRecursionReturn &&
        !bInIterationReturn; }
    void SetInRecursionReturn( bool b );
    bool    IsDoingRecursion() const        { return bDoingRecursion; }
    void    SetDoingRecursion( bool b )     { bDoingRecursion = b; }

    void Insert( ScFormulaCell* p, bool bOldRunning, const ScFormulaResult & rRes );

    bool    IsInIterationReturn() const     { return bInIterationReturn; }
    void SetInIterationReturn( bool b );
    bool    IsDoingIteration() const        { return nIteration > 0; }
    sal_uInt16  GetIteration() const            { return nIteration; }
    bool &  GetConvergingReference()        { return bConverging; }
    void StartIteration();
    void ResumeIteration();
    void IncIteration();
    void EndIteration();

    const ScFormulaRecursionList::iterator& GetLastIterationStart() const { return aLastIterationStart; }
    ScFormulaRecursionList::iterator GetIterationStart();
    ScFormulaRecursionList::iterator GetIterationEnd();
    /** Any return, recursion or iteration, iteration is always coupled with
        recursion. */
    bool    IsInReturn() const              { return bInRecursionReturn; }
    const ScFormulaRecursionList&   GetList() const { return aRecursionFormulas; }
    ScFormulaRecursionList&         GetList()       { return aRecursionFormulas; }
    ScRecursionInIterationStack&    GetRecursionInIterationStack()  { return aRecursionInIterationStack; }

    void Clear();

    /** Detects a simple cycle involving formula-groups and singleton formula-cells. */
    bool PushFormulaGroup(ScFormulaCell* pCell);
    void PopFormulaGroup();
    bool AnyCycleMemberInDependencyEvalMode(const ScFormulaCell* pCell);
    bool AnyParentFGInCycle();
    void SetFormulaGroupDepEvalMode(bool bSet);
    // When dependency computation detects a cycle, it may not compute proper cell values.
    // This sets a flag that ScFormulaCell will use to avoid setting those new values
    // and resetting the dirty flag, until the dependency computation bails out.
    void AbortDependencyComputation();
    bool IsAbortingDependencyComputation() const { return bAbortingDependencyComputation; }

    void AddTemporaryGroupCell(ScFormulaCell* cell);
    void CleanTemporaryGroupCells();

    void SetFormulaGroupSet(o3tl::sorted_vector<ScFormulaCellGroup*>* pSet) { pFGSet = pSet; }
    bool HasFormulaGroupSet() const { return pFGSet != nullptr; }
    bool CheckFGIndependence(ScFormulaCellGroup* pFG);
    void SetGroupsIndependent(bool bSet) { bGroupsIndependent = bSet; }
    bool AreGroupsIndependent() { return bGroupsIndependent; }
};

/** A class to wrap ScRecursionHelper::PushFormulaGroup(),
    ScRecursionHelper::PopFormulaGroup() and make these calls
    exception safe. */
class ScFormulaGroupCycleCheckGuard
{
    ScRecursionHelper& mrRecHelper;
    bool mbShouldPop;
public:
    ScFormulaGroupCycleCheckGuard() = delete;
    ScFormulaGroupCycleCheckGuard(ScRecursionHelper& rRecursionHelper, ScFormulaCell* pCell);
    ~ScFormulaGroupCycleCheckGuard();

};

class ScFormulaGroupDependencyComputeGuard
{
    ScRecursionHelper& mrRecHelper;
public:
    ScFormulaGroupDependencyComputeGuard() = delete;
    ScFormulaGroupDependencyComputeGuard(ScRecursionHelper& rRecursionHelper);
    ~ScFormulaGroupDependencyComputeGuard();
};

class ScCheckIndependentFGGuard
{
    ScRecursionHelper& mrRecHelper;
    bool mbUsedFGSet;
public:
    ScCheckIndependentFGGuard() = delete;
    ScCheckIndependentFGGuard(ScRecursionHelper& rRecursionHelper,
                              o3tl::sorted_vector<ScFormulaCellGroup*>* pSet);
    ~ScCheckIndependentFGGuard();

    bool AreGroupsIndependent();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
