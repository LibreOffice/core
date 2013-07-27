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
#include "tokenarray.hxx"
#include "formula/token.hxx"

using namespace formula;

ScDetectiveRefIter::ScDetectiveRefIter( ScFormulaCell* pCell )
{
    pCode = pCell->GetCode();
    pCode->Reset();
    aPos = pCell->aPos;
}

static bool lcl_ScDetectiveRefIter_SkipRef( ScToken* p, const ScAddress& rPos )
{
    ScSingleRefData& rRef1 = p->GetSingleRef();
    ScAddress aAbs1 = rRef1.toAbs(rPos);
    if (!ValidAddress(aAbs1))
        return true;
    if ( p->GetType() == svDoubleRef || p->GetType() == svExternalDoubleRef )
    {
        ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
        ScAddress aAbs2 = rRef2.toAbs(rPos);
        if (!ValidAddress(aAbs2))
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
        rRange.aStart = aProv.Ref1.toAbs(aPos);
        rRange.aEnd = aProv.Ref2.toAbs(aPos);
        bRet = true;
    }

    return bRet;
}

ScToken* ScDetectiveRefIter::GetNextRefToken()
{
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    while (p && lcl_ScDetectiveRefIter_SkipRef(p, aPos))
    {
        p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    }
    return p;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
