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

#include <prnsave.hxx>
#include <address.hxx>

#include <osl/diagnose.h>

//      Data per table

ScPrintSaverTab::ScPrintSaverTab() :
    mbEntireSheet(false)
{
}

ScPrintSaverTab::~ScPrintSaverTab()
{
    mpRepeatCol.reset();
    mpRepeatRow.reset();
}

void ScPrintSaverTab::SetAreas( const ScRangeVec& rRanges, bool bEntireSheet )
{
    maPrintRanges = rRanges;
    mbEntireSheet = bEntireSheet;
}

void ScPrintSaverTab::SetRepeat( const ScRange* pCol, const ScRange* pRow )
{
    mpRepeatCol.reset(pCol ? new ScRange(*pCol) : nullptr);
    mpRepeatRow.reset(pRow ? new ScRange(*pRow) : nullptr);
}

static bool PtrEqual( const ScRange* p1, const ScRange* p2 )
{
    return ( !p1 && !p2 ) || ( p1 && p2 && *p1 == *p2 );
}

bool ScPrintSaverTab::operator==( const ScPrintSaverTab& rCmp ) const
{
    return
        PtrEqual( mpRepeatCol.get(), rCmp.mpRepeatCol.get() ) &&
        PtrEqual( mpRepeatRow.get(), rCmp.mpRepeatRow.get() ) &&
        (mbEntireSheet == rCmp.mbEntireSheet) &&
        (maPrintRanges == rCmp.maPrintRanges);
}

//      Data for the whole document

ScPrintRangeSaver::ScPrintRangeSaver( SCTAB nCount ) :
    nTabCount( nCount )
{
    if (nCount > 0)
        pData.reset( new ScPrintSaverTab[nCount] );
}

ScPrintRangeSaver::~ScPrintRangeSaver()
{
}

ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab)
{
    OSL_ENSURE(nTab<nTabCount,"ScPrintRangeSaver Tab too big");
    return pData[nTab];
}

const ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab) const
{
    OSL_ENSURE(nTab<nTabCount,"ScPrintRangeSaver Tab too big");
    return pData[nTab];
}

bool ScPrintRangeSaver::operator==( const ScPrintRangeSaver& rCmp ) const
{
    bool bEqual = ( nTabCount == rCmp.nTabCount );
    if (bEqual)
        for (SCTAB i=0; i<nTabCount; i++)
            if (!(pData[i]==rCmp.pData[i]))
            {
                bEqual = false;
                break;
            }
    return bEqual;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
