/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recursionhelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:18:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_RECURSIONHELPER_HXX
#define INCLUDED_RECURSIONHELPER_HXX

#ifndef SC_FORMULARESULT_HXX
#include "formularesult.hxx"
#endif

#include <list>
#include <stack>
#include <tools/solar.h>

class ScFormulaCell;

struct ScFormulaRecursionEntry
{
    ScFormulaCell*  pCell;
    BOOL            bOldRunning;
    ScFormulaResult aPreviousResult;
    ScFormulaRecursionEntry( ScFormulaCell* p, BOOL bR,
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
    USHORT                              nRecursionCount;
    USHORT                              nIteration;
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
    USHORT  GetRecursionCount() const       { return nRecursionCount; }
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
    void    Insert( ScFormulaCell* p, BOOL bOldRunning,
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
    USHORT  GetIteration() const            { return nIteration; }
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
