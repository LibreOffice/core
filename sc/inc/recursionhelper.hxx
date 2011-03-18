/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
