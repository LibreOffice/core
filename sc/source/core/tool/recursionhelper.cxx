/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "recursionhelper.hxx"

void ScRecursionHelper::Init()
{
    nRecursionCount    = 0;
    bInRecursionReturn = bDoingRecursion = bInIterationReturn = false;
    aInsertPos = GetIterationEnd();
    ResetIteration();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */





