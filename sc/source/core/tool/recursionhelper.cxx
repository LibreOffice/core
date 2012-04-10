/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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





