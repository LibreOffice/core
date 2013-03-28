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

#include "formulaiter.hxx"

#include "formulacell.hxx"
#include "formula/token.hxx"

using namespace formula;

ScDetectiveRefIter::ScDetectiveRefIter( ScFormulaCell* pCell )
{
    pCode = pCell->GetCode();
    pCode->Reset();
    aPos = pCell->aPos;
}

static bool lcl_ScDetectiveRefIter_SkipRef( ScToken* p )
{
    ScSingleRefData& rRef1 = p->GetSingleRef();
    if ( rRef1.IsColDeleted() || rRef1.IsRowDeleted() || rRef1.IsTabDeleted()
            || !rRef1.Valid() )
        return true;
    if ( p->GetType() == svDoubleRef || p->GetType() == svExternalDoubleRef )
    {
        ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
        if ( rRef2.IsColDeleted() || rRef2.IsRowDeleted() || rRef2.IsTabDeleted()
                || !rRef2.Valid() )
            return true;
    }
    return false;
}

bool ScDetectiveRefIter::GetNextRef( ScRange& rRange )
{
    bool bRet = false;
    ScToken* p = GetNextRefToken();
    if( p )
    {
        SingleDoubleRefProvider aProv( *p );
        rRange.aStart.Set( aProv.Ref1.nCol, aProv.Ref1.nRow, aProv.Ref1.nTab );
        rRange.aEnd.Set( aProv.Ref2.nCol, aProv.Ref2.nRow, aProv.Ref2.nTab );
        bRet = true;
    }

    return bRet;
}

ScToken* ScDetectiveRefIter::GetNextRefToken()
{
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    if (p)
        p->CalcAbsIfRel( aPos );

    while ( p && lcl_ScDetectiveRefIter_SkipRef( p ) )
    {
        p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        if (p)
            p->CalcAbsIfRel( aPos );
    }
    return p;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
