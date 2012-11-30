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

#include "editable.hxx"
#include "document.hxx"
#include "viewfunc.hxx"
#include "globstr.hrc"
#include "markdata.hxx"

//------------------------------------------------------------------

ScEditableTester::ScEditableTester() :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
    TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark ) :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
    TestSelectedBlock( pDoc, nStartCol, nStartRow, nEndCol, nEndRow, rMark );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScRange& rRange ) :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
    TestRange( pDoc, rRange );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScMarkData& rMark ) :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
    TestSelection( pDoc, rMark );
}

ScEditableTester::ScEditableTester( ScViewFunc* pView ) :
    bIsEditable( sal_True ),
    bOnlyMatrix( sal_True )
{
    TestView( pView );
}

//------------------------------------------------------------------

void ScEditableTester::TestBlock( ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        bool bThisMatrix;
        if ( !pDoc->IsBlockEditable( nTab, nStartCol, nStartRow, nEndCol, nEndRow, &bThisMatrix ) )
        {
            bIsEditable = false;
            if ( !bThisMatrix )
                bOnlyMatrix = false;
        }
    }
}

void ScEditableTester::TestSelectedBlock( ScDocument* pDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark )
{
    SCTAB nTabCount = pDoc->GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
        TestBlock( pDoc, *itr, nStartCol, nStartRow, nEndCol, nEndRow );
}

void ScEditableTester::TestRange( ScDocument* pDoc, const ScRange& rRange )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
        TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

void ScEditableTester::TestSelection( ScDocument* pDoc, const ScMarkData& rMark )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        bool bThisMatrix;
        if ( !pDoc->IsSelectionEditable( rMark, &bThisMatrix ) )
        {
            bIsEditable = false;
            if ( !bThisMatrix )
                bOnlyMatrix = false;
        }
    }
}

void ScEditableTester::TestView( ScViewFunc* pView )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        bool bThisMatrix;
        if ( !pView->SelectionEditable( &bThisMatrix ) )
        {
            bIsEditable = false;
            if ( !bThisMatrix )
                bOnlyMatrix = false;
        }
    }
}

//------------------------------------------------------------------

sal_uInt16 ScEditableTester::GetMessageId() const
{
    if (bIsEditable)
        return 0;
    else if (bOnlyMatrix)
        return STR_MATRIXFRAGMENTERR;
    else
        return STR_PROTECTIONERR;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
