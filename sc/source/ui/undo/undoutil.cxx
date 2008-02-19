/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoutil.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 15:34:23 $
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

void ScUndoUtil::MarkSimpleBlock( ScDocShell* /* pDocShell */,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ )
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab < nStartZ || nViewTab > nEndZ )
            pViewShell->SetTabNo( nStartZ );

        pViewShell->DoneBlockMode();
        pViewShell->MoveCursorAbs( nStartX, nStartY, SC_FOLLOW_JUMP, FALSE, FALSE );
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
        BOOL bWasTemp = FALSE;
        if ( pUndoData )
        {
            String aName;
            pUndoData->GetName( aName );
            if ( aName == ScGlobal::GetRscString( STR_DB_NONAME ) )
                bWasTemp = TRUE;
        }
        DBG_ASSERT(bWasTemp, "Undo: didn't find database range");

        USHORT nIndex;
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if (pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nIndex ))
            pRet = (*pColl)[nIndex];
        else
        {
            pRet = new ScDBData( ScGlobal::GetRscString( STR_DB_NONAME ), nTab,
                                nCol1,nRow1, nCol2,nRow2, TRUE,
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
