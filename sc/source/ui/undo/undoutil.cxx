/*************************************************************************
 *
 *  $RCSfile: undoutil.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

// INCLUDE ---------------------------------------------------------------

#include "undoutil.hxx"

#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "global.hxx"

// STATIC DATA -----------------------------------------------------------

SEG_EOFGLOBALS()


#pragma SEG_FUNCDEF(undoutil_01)

void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                USHORT nEndX, USHORT nEndY, USHORT nEndZ )
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        USHORT nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab < nStartZ || nViewTab > nEndZ )
            pViewShell->SetTabNo( nStartZ );

        pViewShell->DoneBlockMode();
        pViewShell->MoveCursorAbs( nStartX, nStartY, SC_FOLLOW_JUMP, FALSE, FALSE );
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData().
                SetMarkArea( ScRange( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ) );
    }
}

#pragma SEG_FUNCDEF(undoutil_02)

void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScTripel& rBlockStart,
                                const ScTripel& rBlockEnd )
{
    MarkSimpleBlock( pDocShell, rBlockStart.GetCol(), rBlockStart.GetRow(), rBlockStart.GetTab(),
                                rBlockEnd.GetCol(), rBlockEnd.GetRow(), rBlockEnd.GetTab() );
}

#pragma SEG_FUNCDEF(undoutil_05)

void ScUndoUtil::MarkSimpleBlock( ScDocShell* pDocShell,
                                const ScRange& rRange )
{
    MarkSimpleBlock( pDocShell, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                                rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab()   );
}


#pragma SEG_FUNCDEF(undoutil_03)

ScDBData* ScUndoUtil::GetOldDBData( ScDBData* pUndoData, ScDocument* pDoc, USHORT nTab,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
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

#pragma SEG_FUNCDEF(undoutil_04)

void ScUndoUtil::PaintMore( ScDocShell* pDocShell,
                                const ScRange& rRange )
{
    USHORT nCol1 = rRange.aStart.Col();
    USHORT nRow1 = rRange.aStart.Row();
    USHORT nCol2 = rRange.aEnd.Col();
    USHORT nRow2 = rRange.aEnd.Row();
    if (nCol1) --nCol1;
    if (nRow1) --nRow1;
    if (nCol2<MAXCOL) ++nCol2;
    if (nRow2<MAXROW) ++nRow2;

    pDocShell->PostPaint( nCol1,nRow1,rRange.aStart.Tab(),
                          nCol2,nRow2,rRange.aEnd.Tab(), PAINT_GRID );
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.11  2000/09/17 14:09:28  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/08/31 16:38:40  willem.vandorp
    Header and footer replaced

    Revision 1.9  1996/09/04 17:48:18  RJ
    MarkSimpleBlock mit ScRange-Parameter


      Rev 1.8   04 Sep 1996 19:48:18   RJ
   MarkSimpleBlock mit ScRange-Parameter

      Rev 1.7   19 Aug 1996 21:29:56   NN
   Markierungen werden nicht mehr am Dokument gehalten

      Rev 1.6   16 Aug 1996 13:52:14   RJ
   kleine Umstellung in ::MarkSimpleBlock

      Rev 1.5   10 Apr 1996 11:14:54   NN
   MarkSimpleBlock: Tabelle umstellen

      Rev 1.4   10 Oct 1995 12:21:26   NN
   PaintMore

      Rev 1.3   03 Feb 1995 09:45:34   GT
   Umstellung auf Resource-Strings

      Rev 1.2   19 Jan 1995 18:51:20   NN
   GetOldDBArea

      Rev 1.1   18 Jan 1995 15:06:08   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.0   12 Jan 1995 12:19:10   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


