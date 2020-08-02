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

#include <undoutil.hxx>

#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <globalnames.hxx>
#include <global.hxx>
#include <markdata.hxx>

void ScUndoUtil::MarkSimpleBlock( const ScDocShell* pDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ )
{
    if ( pDocShell->IsPaintLocked() )
        return;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (!pViewShell)
        return;

    SCTAB nViewTab = pViewShell->GetViewData().GetTabNo();
    if ( nViewTab < nStartZ || nViewTab > nEndZ )
        pViewShell->SetTabNo( nStartZ );

    pViewShell->DoneBlockMode();
    pViewShell->MoveCursorAbs( nStartX, nStartY, SC_FOLLOW_JUMP, false, false );
    pViewShell->InitOwnBlockMode();
    pViewShell->GetViewData().GetMarkData().
            SetMarkArea( ScRange( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ) );
    pViewShell->MarkDataChanged();
}

void ScUndoUtil::MarkSimpleBlock( const ScDocShell* pDocShell,
                                const ScAddress& rBlockStart,
                                const ScAddress& rBlockEnd )
{
    MarkSimpleBlock( pDocShell, rBlockStart.Col(), rBlockStart.Row(), rBlockStart.Tab(),
                                rBlockEnd.Col(), rBlockEnd.Row(), rBlockEnd.Tab() );
}

void ScUndoUtil::MarkSimpleBlock( const ScDocShell* pDocShell,
                                const ScRange& rRange )
{
    MarkSimpleBlock( pDocShell, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                                rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab()   );
}

ScDBData* ScUndoUtil::GetOldDBData( const ScDBData* pUndoData, ScDocument* pDoc, SCTAB nTab,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    ScDBData* pRet = pDoc->GetDBAtArea( nTab, nCol1, nRow1, nCol2, nRow2 );

    if (!pRet)
    {
        bool bWasTemp = false;
        if ( pUndoData )
        {
            const OUString& aName = pUndoData->GetName();
            if ( aName == STR_DB_LOCAL_NONAME )
                bWasTemp = true;
        }
        OSL_ENSURE(bWasTemp, "Undo: didn't find database range");
        pRet = pDoc->GetAnonymousDBData(nTab);
        if (!pRet)
        {
            pRet = new ScDBData( STR_DB_LOCAL_NONAME, nTab,
                                nCol1,nRow1, nCol2,nRow2, true,
                                pDoc->HasColHeader( nCol1,nRow1,nCol2,nRow2,nTab ) );
            pDoc->SetAnonymousDBData(nTab, std::unique_ptr<ScDBData>(pRet));
        }
    }

    return pRet;
}

void ScUndoUtil::PaintMore( ScDocShell* pDocShell,
                                const ScRange& rRange )
{
    SCCOL nCol1 = rRange.aStart.Col();
    SCROW nRow1 = rRange.aStart.Row();
    SCCOL nCol2 = rRange.aEnd.Col();
    SCROW nRow2 = rRange.aEnd.Row();
    ScDocument& rDoc = pDocShell->GetDocument();
    if (nCol1 > 0) --nCol1;
    if (nRow1 > 0) --nRow1;
    if (nCol2<rDoc.MaxCol()) ++nCol2;
    if (nRow2<rDoc.MaxRow()) ++nRow2;

    pDocShell->PostPaint( nCol1,nRow1,rRange.aStart.Tab(),
                          nCol2,nRow2,rRange.aEnd.Tab(), PaintPartFlags::Grid );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
