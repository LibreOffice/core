/*************************************************************************
 *
 *  $RCSfile: undoutil.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:52:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif


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
        if (!bWasTemp)
            DBG_ERROR("Undo: DB-Bereich nicht gefunden");

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
