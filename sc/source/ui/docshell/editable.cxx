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

#include <editable.hxx>
#include <document.hxx>
#include <viewfunc.hxx>
#include <globstr.hrc>

ScEditableTester::ScEditableTester() :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
}

ScEditableTester::ScEditableTester( const ScDocument& rDoc, SCTAB nTab,
        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, bool bNoMatrixAtAll ) :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
    TestBlock( rDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow, bNoMatrixAtAll );
}

ScEditableTester::ScEditableTester( const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark ) :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
    TestSelectedBlock( rDoc, nStartCol, nStartRow, nEndCol, nEndRow, rMark );
}

ScEditableTester::ScEditableTester( const ScDocument& rDoc, const ScRange& rRange ) :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
    TestRange( rDoc, rRange );
}

ScEditableTester::ScEditableTester( const ScDocument& rDoc, const ScMarkData& rMark ) :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
    TestSelection( rDoc, rMark );
}

ScEditableTester::ScEditableTester( ScViewFunc* pView ) :
    mbIsEditable(true),
    mbOnlyMatrix(true)
{
    bool bThisMatrix;
    if ( !pView->SelectionEditable( &bThisMatrix ) )
    {
        mbIsEditable = false;
        if ( !bThisMatrix )
            mbOnlyMatrix = false;
    }
}

ScEditableTester::ScEditableTester(
    const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd, const ScMarkData& rMark ) :
    ScEditableTester()
{
    TestBlockForAction(rDoc, eAction, nStart, nEnd, rMark);
}

void ScEditableTester::TestBlock( const ScDocument& rDoc, SCTAB nTab,
        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, bool bNoMatrixAtAll )
{
    if (mbIsEditable || mbOnlyMatrix)
    {
        bool bThisMatrix;
        if (!rDoc.IsBlockEditable( nTab, nStartCol, nStartRow, nEndCol, nEndRow, &bThisMatrix, bNoMatrixAtAll))
        {
            mbIsEditable = false;
            if ( !bThisMatrix )
                mbOnlyMatrix = false;
        }
    }
}

void ScEditableTester::TestSelectedBlock( const ScDocument& rDoc,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark )
{
    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : rMark)
    {
        if (rTab >= nTabCount)
            break;

        TestBlock( rDoc, rTab, nStartCol, nStartRow, nEndCol, nEndRow, false );
    }
}

void ScEditableTester::TestRange(  const ScDocument& rDoc, const ScRange& rRange )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
        TestBlock( rDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow, false );
}

void ScEditableTester::TestSelection( const ScDocument& rDoc, const ScMarkData& rMark )
{
    if (mbIsEditable || mbOnlyMatrix)
    {
        bool bThisMatrix;
        if ( !rDoc.IsSelectionEditable( rMark, &bThisMatrix ) )
        {
            mbIsEditable = false;
            if ( !bThisMatrix )
                mbOnlyMatrix = false;
        }
    }
}

void ScEditableTester::TestBlockForAction(
    const ScDocument& rDoc, sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd,
    const ScMarkData& rMark )
{
    mbOnlyMatrix = false;

    for (const auto& rTab : rMark)
    {
        if (!mbIsEditable)
            return;

        mbIsEditable = rDoc.IsEditActionAllowed(eAction, rTab, nStart, nEnd);
    }
}

const char* ScEditableTester::GetMessageId() const
{
    if (mbIsEditable)
        return nullptr;
    else if (mbOnlyMatrix)
        return STR_MATRIXFRAGMENTERR;
    else
        return STR_PROTECTIONERR;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
