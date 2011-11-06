/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_RECURSIONHELPER_HXX
#define INCLUDED_RECURSIONHELPER_HXX

#include "formularesult.hxx"

#include <list>
#include <stack>
#include <tools/solar.h>

class ScFormulaCell;

struct ScFormulaRecursionEntry
{
    ScFormulaCell*  pCell;
    sal_Bool            bOldRunning;
    ScFormulaResult aPreviousResult;
    ScFormulaRecursionEntry( ScFormulaCell* p, sal_Bool bR,
            const ScFormulaResult & rRes ) :
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
    sal_uInt16                              nRecursionCount;
    sal_uInt16                              nIteration;
    bool                                bInRecursionReturn;
    bool                                bDoingRecursion;
    bool                                bInIterationReturn;
    bool                                bConverging;

    void    Init()
    {
        nRecursionCount    = 0;
        bInRecursionReturn = bDoingRecursion = bInIterationReturn = false;
        aInsertPos = GetEnd();
        ResetIteration();
    }
    void    ResetIteration()
    {
        aLastIterationStart = GetEnd();
        nIteration = 0;
        bConverging = false;
    }

    public:

    ScRecursionHelper() { Init(); }
    sal_uInt16  GetRecursionCount() const       { return nRecursionCount; }
    void    IncRecursionCount()             { ++nRecursionCount; }
    void    DecRecursionCount()             { --nRecursionCount; }
    /// A pure recursion return, no iteration.
    bool    IsInRecursionReturn() const     { return bInRecursionReturn &&
        !bInIterationReturn; }
    void    SetInRecursionReturn( bool b )
    {
        // Do not use IsInRecursionReturn() here, it decouples iteration.
        if (b && !bInRecursionReturn)
            aInsertPos = aRecursionFormulas.begin();
        bInRecursionReturn = b;
    }
    bool    IsDoingRecursion() const        { return bDoingRecursion; }
    void    SetDoingRecursion( bool b )     { bDoingRecursion = b; }
    void    Insert( ScFormulaCell* p, sal_Bool bOldRunning,
                    const ScFormulaResult & rRes )
    {
        aRecursionFormulas.insert( aInsertPos, ScFormulaRecursionEntry( p,
                    bOldRunning, rRes));
    }
    ScFormulaRecursionList::iterator    GetStart()
    {
        return aRecursionFormulas.begin();
    }
    ScFormulaRecursionList::iterator    GetEnd()
    {
        return aRecursionFormulas.end();
    }
    bool    IsInIterationReturn() const     { return bInIterationReturn; }
    void    SetInIterationReturn( bool b )
    {
        // An iteration return is always coupled to a recursion return.
        SetInRecursionReturn( b);
        bInIterationReturn = b;
    }
    bool    IsDoingIteration() const        { return nIteration > 0; }
    sal_uInt16  GetIteration() const            { return nIteration; }
    bool &  GetConvergingReference()        { return bConverging; }
    void    StartIteration()
    {
        SetInIterationReturn( false);
        nIteration = 1;
        bConverging = false;
        aLastIterationStart = GetIterationStart();
    }
    void    ResumeIteration()
    {
        SetInIterationReturn( false);
        aLastIterationStart = GetIterationStart();
    }
    void    IncIteration()                  { ++nIteration; }
    void    EndIteration()
    {
        aRecursionFormulas.erase( GetIterationStart(), GetIterationEnd());
        ResetIteration();
    }
    ScFormulaRecursionList::iterator GetLastIterationStart() { return aLastIterationStart; }
    ScFormulaRecursionList::iterator GetIterationStart() { return GetStart(); }
    ScFormulaRecursionList::iterator GetIterationEnd() { return GetEnd(); }
    /** Any return, recursion or iteration, iteration is always coupled with
        recursion. */
    bool    IsInReturn() const              { return bInRecursionReturn; }
    const ScFormulaRecursionList&   GetList() const { return aRecursionFormulas; }
    ScFormulaRecursionList&         GetList()       { return aRecursionFormulas; }
    ScRecursionInIterationStack&    GetRecursionInIterationStack()  { return aRecursionInIterationStack; }
    void    Clear()
    {
        aRecursionFormulas.clear();
        while (!aRecursionInIterationStack.empty())
            aRecursionInIterationStack.pop();
        Init();
    }
};

#endif // INCLUDED_RECURSIONHELPER_HXX
