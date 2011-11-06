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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------



// INCLUDE ---------------------------------------------------------------

#include "undoutil.hxx"

#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "global.hxx"

void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ )
{
    if ( pDocShell->IsPaintLocked() )
        return;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab < nStartZ || nViewTab > nEndZ )
            pViewShell->SetTabNo( nStartZ );

        pViewShell->DoneBlockMode();
        pViewShell->MoveCursorAbs( nStartX, nStartY, SC_FOLLOW_JUMP, sal_False, sal_False );
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData().
                SetMarkArea( ScRange( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ) );
        pViewShell->MarkDataChanged();
    }
}


void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScAddress& rBlockStart,
                                const ScAddress& rBlockEnd )
{
    MarkSimpleBlock( pDocShell, rBlockStart.Col(), rBlockStart.Row(), rBlockStart.Tab(),
                                rBlockEnd.Col(), rBlockEnd.Row(), rBlockEnd.Tab() );
}


void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScRange& rRange )
{
    MarkSimpleBlock( pDocShell, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                                rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab()   );
}



ScDBData* ScUndoUtil::GetOldDBData( ScDBData* pUndoData, ScDocument* pDoc, SCTAB nTab,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    ScDBData* pRet = pDoc->GetDBAtArea( nTab, nCol1, nRow1, nCol2, nRow2 );

    if (!pRet)
    {
        sal_Bool bWasTemp = sal_False;
        if ( pUndoData )
        {
            String aName;
            pUndoData->GetName( aName );
            if ( aName == ScGlobal::GetRscString( STR_DB_NONAME ) )
                bWasTemp = sal_True;
        }
        DBG_ASSERT(bWasTemp, "Undo: didn't find database range");

        sal_uInt16 nIndex;
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if (pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nIndex ))
            pRet = (*pColl)[nIndex];
        else
        {
            pRet = new ScDBData( ScGlobal::GetRscString( STR_DB_NONAME ), nTab,
                                nCol1,nRow1, nCol2,nRow2, sal_True,
                                pDoc->HasColHeader( nCol1,nRow1,nCol2,nRow2,nTab ) );
            pColl->Insert( pRet );
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
    if (nCol1 > 0) --nCol1;
    if (nRow1 > 0) --nRow1;
    if (nCol2<MAXCOL) ++nCol2;
    if (nRow2<MAXROW) ++nRow2;

    pDocShell->PostPaint( nCol1,nRow1,rRange.aStart.Tab(),
                          nCol2,nRow2,rRange.aEnd.Tab(), PAINT_GRID );
}
