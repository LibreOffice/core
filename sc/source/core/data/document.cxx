/*************************************************************************
 *
 *  $RCSfile: document.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: er $ $Date: 2000-10-29 16:43:14 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/editeng.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/poolcach.hxx>
#include <svtools/saveopt.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/system.hxx>
#include <unotools/charclass.hxx>

#include "document.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "attarray.hxx"
#include "markarr.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "globstr.hrc"
#include "rechead.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "chartlis.hxx"
#include "rangelst.hxx"
#include "markdata.hxx"
#include "conditio.hxx"
#include "prnsave.hxx"
#include "chgtrack.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "hints.hxx"
#include "detdata.hxx"
#include "cell.hxx"
#include "dpobject.hxx"
#include "indexmap.hxx"
#include "detfunc.hxx"      // for UpdateAllComments


void ScDocument::MakeTable( USHORT nTab )
{
    if ( nTab<=MAXTAB && !pTab[nTab] )
    {
        String aString = ScGlobal::GetRscString(STR_TABLE_DEF); //"Tabelle"
        aString += String::CreateFromInt32(nTab+1);
        CreateValidTabName( aString );  // keine doppelten

        pTab[nTab] = new ScTable(this, nTab, aString);
        ++nMaxTableNumber;
    }
}


BOOL ScDocument::HasTable( USHORT nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return TRUE;

    return FALSE;
}


BOOL ScDocument::GetName( USHORT nTab, String& rName ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            pTab[nTab]->GetName( rName );
            return TRUE;
        }
    rName.Erase();
    return FALSE;
}


BOOL ScDocument::GetTable( const String& rName, USHORT& rTab ) const
{
    String aUpperName = rName;
    ScGlobal::pCharClass->toUpper(aUpperName);
    String aCompName;

    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i])
        {
            pTab[i]->GetName( aCompName );
            ScGlobal::pCharClass->toUpper(aCompName);
            if (aUpperName == aCompName)
            {
                rTab = i;
                return TRUE;
            }
        }
    rTab = 0;
    return FALSE;
}


BOOL ScDocument::ValidTabName( const String& rName ) const
{
    using namespace ::com::sun::star::i18n;
    sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
        KParseTokens::ASC_UNDERSCORE;
    sal_Int32 nContFlags = nStartFlags;
    String aContChars( RTL_CONSTASCII_USTRINGPARAM(" ") );
    ParseResult rRes = ScGlobal::pCharClass->parsePredefinedToken( KParseType::IDENTNAME, rName, 0,
        nStartFlags, EMPTY_STRING, nContFlags, aContChars );
    return (rRes.TokenType & KParseType::IDENTNAME) && rRes.EndPos == rName.Len();
}


BOOL ScDocument::ValidNewTabName( const String& rName ) const
{
    BOOL bValid = ValidTabName(rName);
    for (USHORT i=0; (i<=MAXTAB) && bValid; i++)
        if (pTab[i])
        {
            String aOldName;
            pTab[i]->GetName(aOldName);
            bValid = !ScGlobal::pScInternational->CompareEqual(
                            rName, aOldName, INTN_COMPARE_IGNORECASE );
        }
    return bValid;
}


void ScDocument::CreateValidTabName(String& rName) const
{
    if ( !ValidTabName(rName) )
    {
        // neu erzeugen

        const String aStrTable( ScResId(SCSTR_TABLE) );
        BOOL         bOk   = FALSE;

        //  vorneweg testen, ob der Prefix als gueltig erkannt wird
        //  wenn nicht, nur doppelte vermeiden
        BOOL bPrefix = ValidTabName( aStrTable );
        DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
        USHORT nDummy;

        USHORT nLoops = 0;      // "zur Sicherheit"
        for ( USHORT i = nMaxTableNumber+1; !bOk && nLoops <= MAXTAB; i++ )
        {
            rName  = aStrTable;
            rName += String::CreateFromInt32(i);
            if (bPrefix)
                bOk = ValidNewTabName( rName );
            else
                bOk = !GetTable( rName, nDummy );
            ++nLoops;
        }

        DBG_ASSERT(bOk, "kein gueltiger Tabellenname gefunden");
        if ( !bOk )
            rName = aStrTable;
    }
    else
    {
        // uebergebenen Namen ueberpruefen

        if ( !ValidNewTabName(rName) )
        {
            USHORT i = 1;
            String aName;
            do
            {
                i++;
                aName = rName;
                aName += '_';
                aName += String::CreateFromInt32(i);
            }
            while (!ValidNewTabName(aName) && (i < MAXTAB+1));
            rName = aName;
        }
    }
}


BOOL ScDocument::InsertTab( USHORT nPos, const String& rName,
            BOOL bExternalDocument )
{
    USHORT  nTabCount = GetTableCount();
    BOOL    bValid = (nTabCount <= MAXTAB);
    if ( !bExternalDocument )   // sonst rName == "'Doc'!Tab", vorher pruefen
        bValid = (bValid && ValidNewTabName(rName));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos == nTabCount)
        {
            pTab[nTabCount] = new ScTable(this, nTabCount, rName);
            ++nMaxTableNumber;
            if ( bExternalDocument )
                pTab[nTabCount]->SetVisible( FALSE );
        }
        else
        {
            if (VALIDTAB(nPos) && (nPos < nTabCount))
            {
                ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                pRangeName->UpdateTabRef( nPos, 1 );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,1 );
                UpdateChartRef( URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,1 ) );

                USHORT i;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateInsertTab(nPos);
                for (i = nTabCount; i > nPos; i--)
                    pTab[i] = pTab[i - 1];
                pTab[nPos] = new ScTable(this, nPos, rName);
                ++nMaxTableNumber;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->StartAllListeners();

                //  update conditional formats after table is inserted
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );

                SetDirty();
                bValid = TRUE;
            }
            else
                bValid = FALSE;
        }
    }
    return bValid;
}


BOOL ScDocument::DeleteTab( USHORT nTab, ScDocument* pRefUndoDoc )
{
    BOOL bValid = FALSE;
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            USHORT nTabCount = GetTableCount();
            if (nTabCount > 1)
            {
                BOOL bOldAutoCalc = GetAutoCalc();
                SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
                ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
                DelBroadcastAreasInRange( aRange );

                aRange.aEnd.SetTab( MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                pRangeName->UpdateTabRef( nTab, 2 );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1 );
                UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1 ) );

                USHORT i;
                for (i=0; i<=MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateDeleteTab(nTab,FALSE,
                                    pRefUndoDoc ? pRefUndoDoc->pTab[i] : 0);
                delete pTab[nTab];
                for (i=nTab + 1; i < nTabCount; i++)
                    pTab[i - 1] = pTab[i];
                pTab[nTabCount - 1] = NULL;
                --nMaxTableNumber;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                // Excel-Filter loescht einige Tables waehrend des Ladens,
                // Listener werden erst nach dem Laden aufgesetzt
                if ( !bInsertingFromOtherDoc )
                {
                    for (i = 0; i <= MAXTAB; i++)
                        if (pTab[i])
                            pTab[i]->StartAllListeners();
                    SetDirty();
                }
                SetAutoCalc( bOldAutoCalc );
                bValid = TRUE;
            }
        }
    }
    return bValid;
}


BOOL ScDocument::RenameTab( USHORT nTab, const String& rName, BOOL bUpdateRef,
        BOOL bExternalDocument )
{
    BOOL    bValid = FALSE;
    USHORT  i;
    if VALIDTAB(nTab)
        if (pTab[nTab])
        {
            if ( bExternalDocument )
                bValid = TRUE;      // zusammengesetzter Name
            else
                bValid = ValidTabName(rName);
            for (i=0; (i<=MAXTAB) && bValid; i++)
                if (pTab[i] && (i != nTab))
                {
                    String aOldName;
                    pTab[i]->GetName(aOldName);
                    bValid = !ScGlobal::pScInternational->CompareEqual(
                                    rName, aOldName, INTN_COMPARE_IGNORECASE );
                }
            if (bValid)
            {
                pTab[nTab]->SetName(rName);
/*                                          kann das nicht weg?
                if (bUpdateRef)
                {
                    for (i = 0; i <= MAXTAB; i++)
                        if (pTab[i])
                            pTab[i]->UpdateCompile();
                    SetDirty();
                }
*/
            }
        }
    return bValid;
}


void ScDocument::SetVisible( USHORT nTab, BOOL bVisible )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetVisible(bVisible);
}


BOOL ScDocument::IsVisible( USHORT nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsVisible();

    return FALSE;
}

/* ----------------------------------------------------------------------------
    benutzten Bereich suchen:

    GetCellArea  - nur Daten
    GetTableArea - Daten / Attribute
    GetPrintArea - beruecksichtigt auch Zeichenobjekte,
                    streicht Attribute bis ganz rechts / unten
---------------------------------------------------------------------------- */


BOOL ScDocument::GetCellArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetCellArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}


BOOL ScDocument::GetTableArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetTableArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}


//  zusammenhaengender Bereich

void ScDocument::GetDataArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
                                USHORT& rEndCol, USHORT& rEndRow, BOOL bIncludeOld )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->GetDataArea( rStartCol, rStartRow, rEndCol, rEndRow, bIncludeOld );
}


void ScDocument::LimitChartArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
                                    USHORT& rEndCol, USHORT& rEndRow )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->LimitChartArea( rStartCol, rStartRow, rEndCol, rEndRow );
}


void ScDocument::LimitChartIfAll( ScRangeListRef& rRangeList )
{
    ScRangeListRef aNew = new ScRangeList;
    if (rRangeList.Is())
    {
        ULONG nCount = rRangeList->Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange(*rRangeList->GetObject( i ));
            if ( ( aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL ) ||
                 ( aRange.aStart.Row() == 0 && aRange.aEnd.Row() == MAXROW ) )
            {
                USHORT nStartCol = aRange.aStart.Col();
                USHORT nStartRow = aRange.aStart.Row();
                USHORT nEndCol = aRange.aEnd.Col();
                USHORT nEndRow = aRange.aEnd.Row();
                USHORT nTab = aRange.aStart.Tab();
                if (pTab[nTab])
                    pTab[nTab]->LimitChartArea(nStartCol, nStartRow, nEndCol, nEndRow);
                aRange.aStart.SetCol( nStartCol );
                aRange.aStart.SetRow( nStartRow );
                aRange.aEnd.SetCol( nEndCol );
                aRange.aEnd.SetRow( nEndRow );
            }
            aNew->Append(aRange);
        }
    }
    else
        DBG_ERROR("LimitChartIfAll: Ref==0");
    rRangeList = aNew;
}


BOOL ScDocument::CanInsertRow( const ScRange& rRange ) const
{
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    USHORT nSize = nEndRow - nStartRow + 1;

    BOOL bTest = TRUE;
    for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );

    return bTest;
}


BOOL ScDocument::InsertRow( USHORT nStartCol, USHORT nStartTab,
                            USHORT nEndCol,   USHORT nEndTab,
                            USHORT nStartRow, USHORT nSize )
{
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );

    BOOL bTest = TRUE;
    BOOL bRet = FALSE;
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );
    if (bTest)
    {
        // UpdateBroadcastAreas muss vor UpdateReference gerufen werden, damit nicht
        // Eintraege verschoben werden, die erst bei UpdateReference neu erzeugt werden

        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab )), 0, nSize, 0 );
        UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
                         nEndCol, MAXROW, nEndTab,
                         0, nSize, 0 );
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {   // RelNames wurden in UpdateReference abgehaengt
            for (i=nStartTab; i<=nEndTab; i++)
                if (pTab[i])
                    pTab[i]->StartRelNameListeners();
            // #69592# at least all cells using range names pointing relative to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = TRUE;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


BOOL ScDocument::InsertRow( const ScRange& rRange )
{
    return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Tab(),
                      rRange.aStart.Row(), rRange.aEnd.Row()-rRange.aStart.Row()+1 );
}


void ScDocument::DeleteRow( USHORT nStartCol, USHORT nStartTab,
                            USHORT nEndCol,   USHORT nEndTab,
                            USHORT nStartRow, USHORT nSize,
                            ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden

    if ( nStartRow+nSize <= MAXROW )
    {
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, nStartRow+nSize-1, nEndTab ) ) );
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow+nSize, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab )), 0, -(short) nSize, 0 );
    }
    else
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab ) ) );

    if ( nStartRow+nSize <= MAXROW )
    {
        UpdateReference( URM_INSDEL, nStartCol, nStartRow+nSize, nStartTab,
                         nEndCol, MAXROW, nEndTab,
                         0, -(short) nSize, 0, pRefUndoDoc );
    }

    if (pUndoOutline)
        *pUndoOutline = FALSE;

    for (USHORT i=nStartTab; i<=nEndTab; i++)
        if (pTab[i])
            pTab[i]->DeleteRow( nStartCol, nEndCol, nStartRow, nSize, pUndoOutline );

    if ( nStartRow+nSize <= MAXROW )
    {   // RelNames wurden in UpdateReference abgehaengt
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->StartRelNameListeners();
        // #69592# at least all cells using range names pointing relative to the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteRow( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Tab(),
               rRange.aStart.Row(), rRange.aEnd.Row()-rRange.aStart.Row()+1,
               pRefUndoDoc, pUndoOutline );
}


BOOL ScDocument::CanInsertCol( const ScRange& rRange ) const
{
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    USHORT nSize = nEndCol - nStartCol + 1;

    BOOL bTest = TRUE;
    for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );

    return bTest;
}


BOOL ScDocument::InsertCol( USHORT nStartRow, USHORT nStartTab,
                            USHORT nEndRow,   USHORT nEndTab,
                            USHORT nStartCol, USHORT nSize )
{
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );

    BOOL bTest = TRUE;
    BOOL bRet = FALSE;
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );
    if (bTest)
    {
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab )), nSize, 0, 0 );
        UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
                         MAXCOL, nEndRow, nEndTab,
                         nSize, 0, 0 );
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->InsertCol( nStartCol, nStartRow, nEndRow, nSize );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {   // RelNames wurden in UpdateReference abgehaengt
            for (i=nStartTab; i<=nEndTab; i++)
                if (pTab[i])
                    pTab[i]->StartRelNameListeners();
            // #69592# at least all cells using range names pointing relative to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = TRUE;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


BOOL ScDocument::InsertCol( const ScRange& rRange )
{
    return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      rRange.aStart.Col(), rRange.aEnd.Col()-rRange.aStart.Col()+1 );
}


void ScDocument::DeleteCol(USHORT nStartRow, USHORT nStartTab, USHORT nEndRow, USHORT nEndTab,
                                USHORT nStartCol, USHORT nSize, ScDocument* pRefUndoDoc,
                                BOOL* pUndoOutline )
{
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden

    if ( nStartCol+nSize <= MAXCOL )
    {
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nStartCol+nSize-1, nEndRow, nEndTab ) ) );
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol+nSize, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab )), -(short) nSize, 0, 0 );
    }
    else
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab ) ) );

    if ( nStartCol+nSize <= MAXCOL )
    {
        UpdateReference( URM_INSDEL, nStartCol+nSize, nStartRow, nStartTab,
                         MAXCOL, nEndRow, nEndTab,
                         -(short) nSize, 0, 0, pRefUndoDoc );
    }

    if (pUndoOutline)
        *pUndoOutline = FALSE;

    for (USHORT i=nStartTab; i<=nEndTab; i++)
        if (pTab[i])
            pTab[i]->DeleteCol( nStartCol, nStartRow, nEndRow, nSize, pUndoOutline );

    if ( nStartCol+nSize <= MAXCOL )
    {   // RelNames wurden in UpdateReference abgehaengt
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->StartRelNameListeners();
        // #69592# at least all cells using range names pointing relative to the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteCol( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               rRange.aStart.Col(), rRange.aEnd.Col()-rRange.aStart.Col()+1,
               pRefUndoDoc, pUndoOutline );
}


//  fuer Area-Links: Zellen einuegen/loeschen, wenn sich der Bereich veraendert
//  (ohne Paint)


void lcl_GetInsDelRanges( const ScRange& rOld, const ScRange& rNew,
                            ScRange& rColRange, BOOL& rInsCol, BOOL& rDelCol,
                            ScRange& rRowRange, BOOL& rInsRow, BOOL& rDelRow )
{
    DBG_ASSERT( rOld.aStart == rNew.aStart, "FitBlock: Anfang unterschiedlich" );

    rInsCol = rDelCol = rInsRow = rDelRow = FALSE;

    USHORT nStartX = rOld.aStart.Col();
    USHORT nStartY = rOld.aStart.Row();
    USHORT nOldEndX = rOld.aEnd.Col();
    USHORT nOldEndY = rOld.aEnd.Row();
    USHORT nNewEndX = rNew.aEnd.Col();
    USHORT nNewEndY = rNew.aEnd.Row();
    USHORT nTab = rOld.aStart.Tab();

    //  wenn es mehr Zeilen werden, werden Spalten auf der alten Hoehe eingefuegt/geloescht
    BOOL bGrowY = ( nNewEndY > nOldEndY );
    USHORT nColEndY = bGrowY ? nOldEndY : nNewEndY;
    USHORT nRowEndX = bGrowY ? nNewEndX : nOldEndX;

    //  Spalten

    if ( nNewEndX > nOldEndX )          // Spalten einfuegen
    {
        rColRange = ScRange( nOldEndX+1, nStartY, nTab, nNewEndX, nColEndY, nTab );
        rInsCol = TRUE;
    }
    else if ( nNewEndX < nOldEndX )     // Spalten loeschen
    {
        rColRange = ScRange( nNewEndX+1, nStartY, nTab, nOldEndX, nColEndY, nTab );
        rDelCol = TRUE;
    }

    //  Zeilen

    if ( nNewEndY > nOldEndY )          // Zeilen einfuegen
    {
        rRowRange = ScRange( nStartX, nOldEndY+1, nTab, nRowEndX, nNewEndY, nTab );
        rInsRow = TRUE;
    }
    else if ( nNewEndY < nOldEndY )     // Zeilen loeschen
    {
        rRowRange = ScRange( nStartX, nNewEndY+1, nTab, nRowEndX, nOldEndY, nTab );
        rDelRow = TRUE;
    }
}


BOOL ScDocument::HasPartOfMerged( const ScRange& rRange )
{
    BOOL bPart = FALSE;
    USHORT nTab = rRange.aStart.Tab();

    USHORT nStartX = rRange.aStart.Col();
    USHORT nStartY = rRange.aStart.Row();
    USHORT nEndX = rRange.aEnd.Col();
    USHORT nEndY = rRange.aEnd.Row();

    if (HasAttrib( nStartX, nStartY, nTab, nEndX, nEndY, nTab,
                        HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        ExtendOverlapped( nStartX, nStartY, nEndX, nEndY, nTab );

        bPart = ( nStartX != rRange.aStart.Col() || nEndX != rRange.aEnd.Col() ||
                  nStartY != rRange.aStart.Row() || nEndY != rRange.aEnd.Row() );
    }
    return bPart;
}


BOOL ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
{
    if ( rOld == rNew )
        return TRUE;

    USHORT nTab = rOld.aStart.Tab();
    BOOL bOk = TRUE;
    BOOL bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol && !CanInsertCol( aColRange ) )            // Zellen am Rand ?
        bOk = FALSE;
    if ( bInsRow && !CanInsertRow( aRowRange ) )            // Zellen am Rand ?
        bOk = FALSE;

    if ( bInsCol || bDelCol )
    {
        aColRange.aEnd.SetCol(MAXCOL);
        if ( HasPartOfMerged(aColRange) )
            bOk = FALSE;
    }
    if ( bInsRow || bDelRow )
    {
        aRowRange.aEnd.SetRow(MAXROW);
        if ( HasPartOfMerged(aRowRange) )
            bOk = FALSE;
    }

    return bOk;
}


void ScDocument::FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear )
{
    if (bClear)
        DeleteAreaTab( rOld, IDF_ALL );

    BOOL bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol )
        InsertCol( aColRange );         // Spalten zuerst einfuegen
    if ( bInsRow )
        InsertRow( aRowRange );

    if ( bDelRow )
        DeleteRow( aRowRange );         // Zeilen zuerst loeschen
    if ( bDelCol )
        DeleteCol( aColRange );

    //  Referenzen um eingefuegte Zeilen erweitern

    if ( bInsCol || bInsRow )
    {
        ScRange aGrowSource = rOld;
        aGrowSource.aEnd.SetCol(Min( rOld.aEnd.Col(), rNew.aEnd.Col() ));
        aGrowSource.aEnd.SetRow(Min( rOld.aEnd.Row(), rNew.aEnd.Row() ));
        USHORT nGrowX = bInsCol ? ( rNew.aEnd.Col() - rOld.aEnd.Col() ) : 0;
        USHORT nGrowY = bInsRow ? ( rNew.aEnd.Row() - rOld.aEnd.Row() ) : 0;
        UpdateGrow( aGrowSource, nGrowX, nGrowY );
    }
}


void ScDocument::DeleteArea(USHORT nCol1, USHORT nRow1,
                            USHORT nCol2, USHORT nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for (USHORT i = 0; i <= MAXTAB; i++)
        if (pTab[i])
            if ( rMark.GetTableSelect(i) || bIsUndo )
                pTab[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::DeleteAreaTab(USHORT nCol1, USHORT nRow1,
                                USHORT nCol2, USHORT nRow2,
                                USHORT nTab, USHORT nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    if ( VALIDTAB(nTab) && pTab[nTab] )
    {
        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
        pTab[nTab]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
        SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::DeleteAreaTab( const ScRange& rRange, USHORT nDelFlag )
{
    for ( USHORT nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); nTab++ )
        DeleteAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
                       rRange.aEnd.Col(),   rRange.aEnd.Row(),
                       nTab, nDelFlag );
}


void ScDocument::InitUndo( ScDocument* pSrcDoc, USHORT nTab1, USHORT nTab2,
                                BOOL bColInfo, BOOL bRowInfo )
{
    if (bIsUndo)
    {
        Clear();
        DBG_ASSERT( !bOwner, "Pool gehoert Undo-Doc" );
        bOwner = FALSE;
        pDocPool = pSrcDoc->pDocPool;
        pStylePool = pSrcDoc->pStylePool;
        pFormTable = pSrcDoc->pFormTable;
        pEditPool = pSrcDoc->pEditPool;
        pEnginePool = pSrcDoc->pEnginePool;

        String aString;
        for (USHORT nTab = nTab1; nTab <= nTab2; nTab++)
            pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);

        nMaxTableNumber = nTab2 + 1;
    }
    else
        DBG_ERROR("InitUndo");
}


void ScDocument::AddUndoTab( USHORT nTab1, USHORT nTab2, BOOL bColInfo, BOOL bRowInfo )
{
    if (bIsUndo)
    {
        String aString;
        for (USHORT nTab = nTab1; nTab <= nTab2; nTab++)
            if (!pTab[nTab])
                pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);

        if ( nMaxTableNumber <= nTab2 )
            nMaxTableNumber = nTab2 + 1;
    }
    else
        DBG_ERROR("InitUndo");
}


void ScDocument::SetCutMode( BOOL bVal )
{
    if (bIsClip)
        bCutMode = bVal;
    else
    {
        DBG_ERROR("SetCutMode without bIsClip");
    }
}


BOOL ScDocument::IsCutMode()
{
    if (bIsClip)
        return bCutMode;
    else
    {
        DBG_ERROR("IsCutMode ohne bIsClip");
        return FALSE;
    }
}


void ScDocument::CopyToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, BOOL bColRowFlags )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
        for (USHORT i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->CopyToTable( nCol1, nRow1, nCol2, nRow2, nFlags,
                                      bOnlyMarked, pDestDoc->pTab[i], pMarks,
                                      FALSE, bColRowFlags );
        }
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::UndoToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
        if (nTab1 > 0)
            CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, FALSE, pDestDoc, pMarks );

        for (USHORT i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->UndoToTable(nCol1, nRow1, nCol2, nRow2, nFlags,
                                    bOnlyMarked, pDestDoc->pTab[i], pMarks);
        }

        if (nTab2 < MAXTAB)
            CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, FALSE, pDestDoc, pMarks );
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::CopyToDocument(const ScRange& rRange,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, BOOL bColRowFlags)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();

    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
    for (USHORT i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab(); i++)
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->CopyToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                 aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                 nFlags, bOnlyMarked, pDestDoc->pTab[i],
                                 pMarks, FALSE, bColRowFlags);
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::UndoToDocument(const ScRange& rRange,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();
    USHORT nTab1 = aNewRange.aStart.Tab();
    USHORT nTab2 = aNewRange.aEnd.Tab();

    BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
    if (nTab1 > 0)
        CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, FALSE, pDestDoc, pMarks );

    for (USHORT i = nTab1; i <= nTab2; i++)
    {
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->UndoToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                    aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                    nFlags, bOnlyMarked, pDestDoc->pTab[i], pMarks);
    }

    if (nTab2 < MAXTAB)
        CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, FALSE, pDestDoc, pMarks );
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CopyToClip(USHORT nCol1, USHORT nRow1,
                            USHORT nCol2, USHORT nRow2,
                            BOOL bCut, ScDocument* pClipDoc,
                            BOOL bAllTabs, const ScMarkData* pMarks, BOOL bKeepScenarioFlags)
{
    DBG_ASSERT( bAllTabs || pMarks, "CopyToClip: ScMarkData fehlt" );

    if (!bIsClip)
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if (!pClipDoc)
            pClipDoc = ScGlobal::GetClipDoc();

        pClipDoc->aDocName = aDocName;
        pClipDoc->aClipRange = ScRange( nCol1,nRow1,0, nCol2,nRow2,0 );
        pClipDoc->ResetClip( this, pMarks );
        USHORT i, j;
        pClipDoc->pRangeName->FreeAll();
        for (i = 0; i < pRangeName->GetCount(); i++)        //! DB-Bereiche Pivot-Bereiche auch !!!
        {
            USHORT nIndex = ((ScRangeData*)((*pRangeName)[i]))->GetIndex();
            BOOL bInUse = FALSE;
            for (j = 0; !bInUse && (j <= MAXTAB); j++)
            {
                if (pTab[j])
                    bInUse = pTab[j]->IsRangeNameInUse(nCol1, nRow1, nCol2, nRow2,
                                                       nIndex);
            }
            if (bInUse)
            {
                ScRangeData* pData = new ScRangeData(*((*pRangeName)[i]));
                if (!pClipDoc->pRangeName->Insert(pData))
                    delete pData;
                else
                    pData->SetIndex(nIndex);
            }
        }
        for (i = 0; i <= MAXTAB; i++)
            if (pTab[i] && pClipDoc->pTab[i])
                if ( bAllTabs || !pMarks || pMarks->GetTableSelect(i) )
                    pTab[i]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[i], bKeepScenarioFlags);

        pClipDoc->bCutMode = bCut;
    }
}


void ScDocument::CopyTabToClip(USHORT nCol1, USHORT nRow1,
                                USHORT nCol2, USHORT nRow2,
                                USHORT nTab, ScDocument* pClipDoc)
{
    if (!bIsClip)
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if (!pClipDoc)
            pClipDoc = ScGlobal::GetClipDoc();

        pClipDoc->aDocName = aDocName;
        pClipDoc->aClipRange = ScRange( nCol1,nRow1,0, nCol2,nRow2,0 );
        pClipDoc->ResetClip( this, nTab );

        if (pTab[nTab] && pClipDoc->pTab[nTab])
            pTab[nTab]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[nTab], FALSE);

        pClipDoc->bCutMode = FALSE;
    }
}


void ScDocument::TransposeClip( ScDocument* pTransClip, USHORT nFlags, BOOL bAsLink )
{
    USHORT i;
    DBG_ASSERT( bIsClip && pTransClip && pTransClip->bIsClip,
                    "TransposeClip mit falschem Dokument" );

        //  initialisieren
        //  -> pTransClip muss vor dem Original-Dokument geloescht werden!

    pTransClip->ResetClip(this, (ScMarkData*)NULL);     // alle

        //  Bereiche uebernehmen

    pTransClip->pRangeName->FreeAll();
    for (i = 0; i < pRangeName->GetCount(); i++)        //! DB-Bereiche Pivot-Bereiche auch !!!
    {
        USHORT nIndex = ((ScRangeData*)((*pRangeName)[i]))->GetIndex();
        ScRangeData* pData = new ScRangeData(*((*pRangeName)[i]));
        if (!pTransClip->pRangeName->Insert(pData))
            delete pData;
        else
            pData->SetIndex(nIndex);
    }

        //  Daten

    if ( aClipRange.aEnd.Row()-aClipRange.aStart.Row() <= MAXCOL )
    {
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
            {
                DBG_ASSERT( pTransClip->pTab[i], "TransposeClip: Tabelle nicht da" );
                pTab[i]->TransposeClip( aClipRange.aStart.Col(), aClipRange.aStart.Row(),
                                            aClipRange.aEnd.Col(), aClipRange.aEnd.Row(),
                                            pTransClip->pTab[i], nFlags, bAsLink );
            }

        pTransClip->aClipRange = ScRange( 0, 0, aClipRange.aStart.Tab(),
                                    aClipRange.aEnd.Row() - aClipRange.aStart.Row(),
                                    aClipRange.aEnd.Col() - aClipRange.aStart.Col(),
                                    aClipRange.aEnd.Tab() );
    }
    else
        DBG_ERROR("TransposeClip: zu gross");

        //  Dies passiert erst beim Einfuegen...

    bCutMode = FALSE;
}


BOOL ScDocument::IsClipboardSource() const
{
    ScDocument* pClipDoc = ScGlobal::GetClipDoc();
    return pDocPool == pClipDoc->pDocPool;
}


void ScDocument::StartListeningFromClip( USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark, USHORT nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        for (USHORT i = 0; i <= MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->StartListeningInArea( nCol1, nRow1, nCol2, nRow2 );
    }
}


void ScDocument::BroadcastFromClip( USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        USHORT nClipTab = 0;
        for (USHORT i = 0; i <= MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->BroadcastInArea( nCol1, nRow1, nCol2, nRow2 );
    }
}


void ScDocument::CopyBlockFromClip( USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark,
                                    short nDx, short nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP )
{
    ScTable** ppClipTab = pCBFCP->pClipDoc->pTab;
    USHORT nTabEnd = pCBFCP->nTabEnd;
    USHORT i;
    USHORT nClipTab = 0;
    for (i = pCBFCP->nTabStart; i <= nTabEnd; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
            {
                while (!ppClipTab[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
                pTab[i]->CopyFromClip(nCol1, nRow1, nCol2, nRow2, nDx, nDy,
                                        pCBFCP->nInsFlag, pCBFCP->bAsLink, ppClipTab[nClipTab]);
                nClipTab = (nClipTab+1) % (MAXTAB+1);
            }
    if ( pCBFCP->pClipDoc->bCutMode && (pCBFCP->nInsFlag & IDF_CONTENTS) )
    {
        nClipTab = 0;
        for (i = pCBFCP->nTabStart; i <= nTabEnd; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                {
                    while (!ppClipTab[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
                    short nDz = ((short)i) - nClipTab;
                    UpdateReference(URM_MOVE, nCol1, nRow1, i, nCol2, nRow2, i, nDx, nDy, nDz, pCBFCP->pRefUndoDoc);
                    nClipTab = (nClipTab+1) % (MAXTAB+1);
                }
    }
}


void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                USHORT nInsFlag,
                                ScDocument* pRefUndoDoc, ScDocument* pClipDoc, BOOL bResetCut,
                                BOOL bAsLink )
{
    if (!bIsClip)
    {
        if (!pClipDoc)
            pClipDoc = ScGlobal::GetClipDoc();
        if (pClipDoc->bIsClip && pClipDoc->GetTableCount())
        {
            BOOL bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
            if (pClipDoc->pFormTable && pClipDoc->pFormTable != pFormTable)
            {
                SvULONGTable* pExchangeList =
                         pFormTable->MergeFormatter(*(pClipDoc->pFormTable));
                if (pExchangeList->Count() > 0)
                    pFormatExchangeList = pExchangeList;
            }

            USHORT nClipRangeNames = pClipDoc->pRangeName->GetCount();
            // array containing range names which might need update of indices
            ScRangeData** pClipRangeNames = nClipRangeNames ? new ScRangeData* [nClipRangeNames] : NULL;
            // the index mapping thereof
            ScIndexMap aClipRangeMap( nClipRangeNames );
            BOOL bRangeNameReplace = FALSE;

            USHORT i, k;
            for (i = 0; i < nClipRangeNames; i++)       //! DB-Bereiche Pivot-Bereiche auch
            {
                /*  Copy only if the name doesn't exist in this document.
                    If it exists we use the already existing name instead,
                    another possibility could be to create new names if
                    documents differ.
                    A proper solution would ask the user how to proceed.
                    The adjustment of the indices in the formulas is done later.
                */
                ScRangeData* pClipData = (*pClipDoc->pRangeName)[i];
                if ( pRangeName->SearchName( pClipData->GetName(), k ) )
                {
                    pClipRangeNames[i] = NULL;  // range name not inserted
                    USHORT nOldIndex = pClipData->GetIndex();
                    USHORT nNewIndex = ((*pRangeName)[k])->GetIndex();
                    aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
                    if ( !bRangeNameReplace )
                        bRangeNameReplace = ( nOldIndex != nNewIndex );
                }
                else
                {
                    ScRangeData* pData = new ScRangeData( *pClipData );
                    pData->SetDocument(this);
                    if ( pRangeName->FindIndex( pData->GetIndex() ) )
                        pData->SetIndex(0);     // need new index, done in Insert
                    if ( pRangeName->Insert( pData ) )
                    {
                        pClipRangeNames[i] = pData;
                        USHORT nOldIndex = pClipData->GetIndex();
                        USHORT nNewIndex = pData->GetIndex();
                        aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
                        if ( !bRangeNameReplace )
                            bRangeNameReplace = ( nOldIndex != nNewIndex );
                    }
                    else
                    {   // must be an overflow
                        delete pData;
                        pClipRangeNames[i] = NULL;
                        aClipRangeMap.SetPair( i, pClipData->GetIndex(), 0 );
                        bRangeNameReplace = TRUE;
                    }
                }
            }
            USHORT nCol1 = rDestRange.aStart.Col();
            USHORT nRow1 = rDestRange.aStart.Row();
            USHORT nCol2 = rDestRange.aEnd.Col();
            USHORT nRow2 = rDestRange.aEnd.Row();

            USHORT nXw = pClipDoc->aClipRange.aEnd.Col();
            USHORT nYw = pClipDoc->aClipRange.aEnd.Row();
            pClipDoc->ExtendMerge( pClipDoc->aClipRange.aStart.Col(),
                                    pClipDoc->aClipRange.aStart.Row(),
                                    nXw, nYw, 0 );
            nXw -= pClipDoc->aClipRange.aStart.Col();
            nYw -= pClipDoc->aClipRange.aStart.Row();

            //  Inhalte entweder komplett oder gar nicht loeschen:
            USHORT nDelFlag = IDF_NONE;
            if ( nInsFlag & IDF_CONTENTS )
                nDelFlag |= IDF_CONTENTS;
            if ( nInsFlag & IDF_ATTRIB )
                nDelFlag |= IDF_ATTRIB;
            DeleteArea(nCol1, nRow1, nCol2, nRow2, rMark, nDelFlag);

            bInsertingFromOtherDoc = TRUE;  // kein Broadcast/Listener aufbauen bei Insert
            USHORT nC1 = nCol1;
            USHORT nR1 = nRow1;
            USHORT nC2 = nC1 + nXw;
            USHORT nR2 = nR1 + nYw;
            USHORT nClipStartCol = pClipDoc->aClipRange.aStart.Col();
            USHORT nClipStartRow = pClipDoc->aClipRange.aStart.Row();

            ScCopyBlockFromClipParams aCBFCP;
            aCBFCP.pRefUndoDoc = pRefUndoDoc;
            aCBFCP.pClipDoc = pClipDoc;
            aCBFCP.nInsFlag = nInsFlag;
            aCBFCP.bAsLink  = bAsLink;
            aCBFCP.nTabStart = MAXTAB;      // wird in der Schleife angepasst
            aCBFCP.nTabEnd = 0;             // wird in der Schleife angepasst

            //  Inc/DecRecalcLevel einmal aussen, damit nicht fuer jeden Block
            //  die Draw-Seitengroesse neu berechnet werden muss
            //! nur wenn ganze Zeilen/Spalten kopiert werden?

            for (i = 0; i <= MAXTAB; i++)
                if (pTab[i] && rMark.GetTableSelect(i))
                {
                    if ( i < aCBFCP.nTabStart )
                        aCBFCP.nTabStart = i;
                    aCBFCP.nTabEnd = i;
                    pTab[i]->IncRecalcLevel();
                }

            // bei mindestens 64 Zeilen wird in ScColumn::CopyFromClip voralloziert
            BOOL bDoDouble = ( nYw < 64 && nRow2 - nRow1 > 64);
            BOOL bOldDouble = ScColumn::bDoubleAlloc;
            if (bDoDouble)
                ScColumn::bDoubleAlloc = TRUE;

            do
            {
                do
                {
                    short nDx = ((short)nC1) - nClipStartCol;
                    short nDy = ((short)nR1) - nClipStartRow;
                    CopyBlockFromClip( nC1, nR1, nC2, nR2, rMark, nDx, nDy, &aCBFCP );
                    nC1 = nC2 + 1;
                    nC2 = Min((USHORT)(nC1 + nXw), nCol2);
                }
                while (nC1 <= nCol2);
                nC1 = nCol1;
                nC2 = nC1 + nXw;
                nR1 = nR2 + 1;
                nR2 = Min((USHORT)(nR1 + nYw), nRow2);
            }
            while (nR1 <= nRow2);

            ScColumn::bDoubleAlloc = bOldDouble;

            for (i = 0; i <= MAXTAB; i++)
                if (pTab[i] && rMark.GetTableSelect(i))
                    pTab[i]->DecRecalcLevel();

            bInsertingFromOtherDoc = FALSE;
            pFormatExchangeList = NULL;
            if ( bRangeNameReplace )
            {
                // first update all inserted named formulas if they contain other
                // range names and used indices changed
                for (i = 0; i < nClipRangeNames; i++)       //! DB-Bereiche Pivot-Bereiche auch
                {
                    if ( pClipRangeNames[i] )
                        pClipRangeNames[i]->ReplaceRangeNamesInUse( aClipRangeMap );
                }
                // then update the formulas, they might need the just updated range names
                USHORT nC1 = nCol1;
                USHORT nR1 = nRow1;
                USHORT nC2 = nC1 + nXw;
                USHORT nR2 = nR1 + nYw;
                do
                {
                    do
                    {
                        for (k = 0; k <= MAXTAB; k++)
                        {
                            if ( pTab[k] && rMark.GetTableSelect(k) )
                                pTab[k]->ReplaceRangeNamesInUse(nC1, nR1,
                                    nC2, nR2, aClipRangeMap );
                        }
                        nC1 = nC2 + 1;
                        nC2 = Min((USHORT)(nC1 + nXw), nCol2);
                    } while (nC1 <= nCol2);
                    nC1 = nCol1;
                    nC2 = nC1 + nXw;
                    nR1 = nR2 + 1;
                    nR2 = Min((USHORT)(nR1 + nYw), nRow2);
                } while (nR1 <= nRow2);
            }
            if ( pClipRangeNames )
                delete [] pClipRangeNames;
            // Listener aufbauen nachdem alles inserted wurde
            StartListeningFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
            // nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
            BroadcastFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
            if (bResetCut)
                pClipDoc->bCutMode = FALSE;
            SetAutoCalc( bOldAutoCalc );
        }
    }
}


void ScDocument::SetClipArea( const ScRange& rArea, BOOL bCut )
{
    if (bIsClip)
    {
        aClipRange = rArea;
        bCutMode = bCut;
    }
    else
        DBG_ERROR("SetClipArea: kein Clip");
}


void ScDocument::GetClipArea(USHORT& nClipX, USHORT& nClipY)
{
    if (bIsClip)
    {
        nClipX = aClipRange.aEnd.Col() - aClipRange.aStart.Col();
        nClipY = aClipRange.aEnd.Row() - aClipRange.aStart.Row();
    }
    else
        DBG_ERROR("GetClipArea: kein Clip");
}


void ScDocument::GetClipStart(USHORT& nClipX, USHORT& nClipY)
{
    if (bIsClip)
    {
        nClipX = aClipRange.aStart.Col();
        nClipY = aClipRange.aStart.Row();
    }
    else
        DBG_ERROR("GetClipStart: kein Clip");
}


void ScDocument::MixDocument( const ScRange& rRange, USHORT nFunction, BOOL bSkipEmpty,
                                    ScDocument* pSrcDoc )
{
    USHORT nTab1 = rRange.aStart.Tab();
    USHORT nTab2 = rRange.aEnd.Tab();
    for (USHORT i = nTab1; i <= nTab2; i++)
        if (pTab[i] && pSrcDoc->pTab[i])
            pTab[i]->MixData( rRange.aStart.Col(), rRange.aStart.Row(),
                                rRange.aEnd.Col(), rRange.aEnd.Row(),
                                nFunction, bSkipEmpty, pSrcDoc->pTab[i] );
}


void ScDocument::FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink )
{
    USHORT nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    USHORT nSrcTab = rSrcArea.aStart.Tab();

    if (nSrcTab <= MAXTAB && pTab[nSrcTab])
    {
        USHORT nStartCol = rSrcArea.aStart.Col();
        USHORT nStartRow = rSrcArea.aStart.Row();
        USHORT nEndCol = rSrcArea.aEnd.Col();
        USHORT nEndRow = rSrcArea.aEnd.Row();
        ScDocument* pMixDoc = NULL;
        BOOL bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );                   // Mehrfachberechnungen vermeiden

        USHORT nCount = GetTableCount();
        for (USHORT i=0; i<nCount; i++)
            if ( i!=nSrcTab && pTab[i] && rMark.GetTableSelect(i) )
            {
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc = new ScDocument( SCDOCMODE_UNDO );
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );
                    pTab[i]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, FALSE, pMixDoc->pTab[i] );
                }
                pTab[i]->DeleteArea( nStartCol,nStartRow, nEndCol,nEndRow, nDelFlags);
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                                 nFlags, FALSE, pTab[i], NULL, bAsLink );

                if (bDoMix)
                    pTab[i]->MixData( nStartCol,nStartRow, nEndCol,nEndRow,
                                        nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
        DBG_ERROR("falsche Tabelle");
}


void ScDocument::FillTabMarked( USHORT nSrcTab, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink )
{
    USHORT nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    if (nSrcTab <= MAXTAB && pTab[nSrcTab])
    {
        ScDocument* pMixDoc = NULL;
        BOOL bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );                   // Mehrfachberechnungen vermeiden

        ScRange aArea;
        rMark.GetMultiMarkArea( aArea );
        USHORT nStartCol = aArea.aStart.Col();
        USHORT nStartRow = aArea.aStart.Row();
        USHORT nEndCol = aArea.aEnd.Col();
        USHORT nEndRow = aArea.aEnd.Row();

        USHORT nCount = GetTableCount();
        for (USHORT i=0; i<nCount; i++)
            if ( i!=nSrcTab && pTab[i] && rMark.GetTableSelect(i) )
            {
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc = new ScDocument( SCDOCMODE_UNDO );
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );
                    pTab[i]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, TRUE, pMixDoc->pTab[i], &rMark );
                }

                pTab[i]->DeleteSelection( nDelFlags, rMark );
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                             nFlags, TRUE, pTab[i], &rMark, bAsLink );

                if (bDoMix)
                    pTab[i]->MixMarked( rMark, nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
        DBG_ERROR("falsche Tabelle");
}


void ScDocument::PutCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell, BOOL bForceTab )
{
    if (VALIDTAB(nTab))
    {
        if ( bForceTab && !pTab[nTab] )
        {
            BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

            pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                            bExtras, bExtras);
            ++nMaxTableNumber;
        }

        if (pTab[nTab])
            pTab[nTab]->PutCell( nCol, nRow, pCell );
    }
}


void ScDocument::PutCell( const ScAddress& rPos, ScBaseCell* pCell, BOOL bForceTab )
{
    USHORT nTab = rPos.Tab();
    if ( bForceTab && !pTab[nTab] )
    {
        BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

        pTab[nTab] = new ScTable(this, nTab,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                        bExtras, bExtras);
        ++nMaxTableNumber;
    }

    if (pTab[nTab])
        pTab[nTab]->PutCell( rPos, pCell );
}


BOOL ScDocument::SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->SetString( nCol, nRow, nTab, rString );
    else
        return FALSE;
}


void ScDocument::SetValue( USHORT nCol, USHORT nRow, USHORT nTab, const double& rVal )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetValue( nCol, nRow, rVal );
}


void ScDocument::SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetNote( nCol, nRow, rNote );
}


void ScDocument::GetString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        pTab[nTab]->GetString( nCol, nRow, rString );
    else
        rString.Erase();
}


void ScDocument::GetInputString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        pTab[nTab]->GetInputString( nCol, nRow, rString );
    else
        rString.Erase();
}


void ScDocument::GetValue( USHORT nCol, USHORT nRow, USHORT nTab, double& rValue )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        rValue = pTab[nTab]->GetValue( nCol, nRow );
    else
        rValue = 0.0;
}


double ScDocument::GetValue( const ScAddress& rPos )
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetValue( rPos );
    return 0.0;
}


void ScDocument::GetNumberFormat( USHORT nCol, USHORT nRow, USHORT nTab,
                                  ULONG& rFormat )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            rFormat = pTab[nTab]->GetNumberFormat( nCol, nRow );
            return ;
        }
    rFormat = 0;
}


ULONG ScDocument::GetNumberFormat( const ScAddress& rPos ) const
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetNumberFormat( rPos );
    return 0;
}


void ScDocument::GetNumberFormatInfo( short& nType, ULONG& nIndex,
            const ScAddress& rPos, const ScFormulaCell& rFCell ) const
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
    {
        nIndex = pTab[nTab]->GetNumberFormat( rPos );
        if ( (nIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
            rFCell.GetFormatInfo( nType, nIndex );
        else
            nType = GetFormatTable()->GetType( nIndex );
    }
    else
    {
        nType = NUMBERFORMAT_UNDEFINED;
        nIndex = 0;
    }
}


void ScDocument::GetFormula( USHORT nCol, USHORT nRow, USHORT nTab, String& rFormula,
                             BOOL bAsciiExport ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            pTab[nTab]->GetFormula( nCol, nRow, rFormula, bAsciiExport );
    else
        rFormula.Erase();
}


BOOL ScDocument::GetNote( USHORT nCol, USHORT nRow, USHORT nTab, ScPostIt& rNote )
{
    BOOL bHasNote = FALSE;

    if ( VALIDTAB(nTab) && pTab[nTab] )
        bHasNote = pTab[nTab]->GetNote( nCol, nRow, rNote );
    else
        rNote.Clear();

    return bHasNote;
}


CellType ScDocument::GetCellType( const ScAddress& rPos ) const
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetCellType( rPos );
    return CELLTYPE_NONE;
}


void ScDocument::GetCellType( USHORT nCol, USHORT nRow, USHORT nTab,
        CellType& rCellType ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        rCellType = pTab[nTab]->GetCellType( nCol, nRow );
    else
        rCellType = CELLTYPE_NONE;
}


void ScDocument::GetCell( USHORT nCol, USHORT nRow, USHORT nTab,
        ScBaseCell*& rpCell ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        rpCell = pTab[nTab]->GetCell( nCol, nRow );
    else
    {
        DBG_ERROR("GetCell ohne Tabelle");
        rpCell = NULL;
    }
}


ScBaseCell* ScDocument::GetCell( const ScAddress& rPos ) const
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetCell( rPos );

    DBG_ERROR("GetCell ohne Tabelle");
    return NULL;
}


BOOL ScDocument::HasStringData( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasStringData( nCol, nRow );
    else
        return FALSE;
}


BOOL ScDocument::HasValueData( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasValueData( nCol, nRow );
    else
        return FALSE;
}


BOOL ScDocument::HasStringCells( const ScRange& rRange ) const
{
    //  TRUE, wenn String- oder Editzellen im Bereich

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    for ( USHORT nTab=nStartTab; nTab<=nEndTab; nTab++ )
        if ( pTab[nTab] && pTab[nTab]->HasStringCells( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return TRUE;

    return FALSE;
}


void ScDocument::SetDirtyVar()
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyVar();
}


void ScDocument::SetDirty()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    bAutoCalc = FALSE;      // keine Mehrfachberechnung
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirty();

    //  Charts werden zwar auch ohne AutoCalc im Tracking auf Dirty gesetzt,
    //  wenn alle Formeln dirty sind, werden die Charts aber nicht mehr erwischt
    //  (#45205#) - darum alle Charts nochmal explizit
    if (pChartListenerCollection)
        pChartListenerCollection->SetDirty();

    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetDirty( const ScRange& rRange )
{
    BOOL bOldAutoCalc = GetAutoCalc();
    bAutoCalc = FALSE;      // keine Mehrfachberechnung
    USHORT nTab2 = rRange.aEnd.Tab();
    for (USHORT i=rRange.aStart.Tab(); i<=nTab2; i++)
        if (pTab[i]) pTab[i]->SetDirty( rRange );
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CalcAll()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( TRUE );
    USHORT i;
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyVar();
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CalcAll();
    ClearFormulaTree();
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CompileAll()
{
    if ( pCondFormList )
        pCondFormList->CompileAll();

    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CompileAll();
    SetDirty();
}


void ScDocument::CompileXML()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CompileXML();
    SetDirty();
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CalcAfterLoad()
{
    if (bIsClip)    // Excel-Dateien werden aus dem Clipboard in ein Clip-Doc geladen
        return;     // dann wird erst beim Einfuegen in das richtige Doc berechnet

    bCalcingAfterLoad = TRUE;
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CalcAfterLoad();
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyAfterLoad();
    bCalcingAfterLoad = FALSE;

    SetDetectiveDirty(FALSE);   // noch keine wirklichen Aenderungen
}


void ScDocument::GetErrCode( USHORT nCol, USHORT nRow, USHORT nTab, USHORT& rErrCode )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        rErrCode = pTab[nTab]->GetErrCode( nCol, nRow );
    else
        rErrCode = 0;
}


USHORT ScDocument::GetErrCode( const ScAddress& rPos ) const
{
    USHORT nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetErrCode( rPos );
    return 0;
}


void ScDocument::ResetChanged( const ScRange& rRange )
{
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndTab = rRange.aEnd.Tab();
    for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
        if (pTab[nTab])
            pTab[nTab]->ResetChanged( rRange );
}

//
//  Spaltenbreiten / Zeilenhoehen   --------------------------------------
//


void ScDocument::SetColWidth( USHORT nCol, USHORT nTab, USHORT nNewWidth )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetColWidth( nCol, nNewWidth );
}


void ScDocument::SetRowHeight( USHORT nRow, USHORT nTab, USHORT nNewHeight )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetRowHeight( nRow, nNewHeight );
}


void ScDocument::SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nNewHeight )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetRowHeightRange
            ( nStartRow, nEndRow, nNewHeight, 1.0, 1.0 );
}


void ScDocument::SetManualHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bManual )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetManualHeight( nStartRow, nEndRow, bManual );
}


USHORT ScDocument::GetColWidth( USHORT nCol, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetColWidth( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetOriginalWidth( USHORT nCol, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetOriginalWidth( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetRowHeight( USHORT nRow, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetRowHeight( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetHiddenRowCount( USHORT nRow, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetHiddenRowCount( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


ULONG ScDocument::GetColOffset( USHORT nCol, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetColOffset( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


ULONG ScDocument::GetRowOffset( USHORT nRow, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetRowOffset( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetOptimalColWidth( USHORT nCol, USHORT nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula, const ScMarkData* pMarkData,
                                        BOOL bSimpleTextImport )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetOptimalColWidth( nCol, pDev, nPPTX, nPPTY,
            rZoomX, rZoomY, bFormula, pMarkData, bSimpleTextImport );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


long ScDocument::GetNeededSize( USHORT nCol, USHORT nRow, USHORT nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetNeededSize
                ( nCol, nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, bTotalSize );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


BOOL ScDocument::SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bShrink )
{
//! MarkToMulti();
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->SetOptimalHeight( nStartRow, nEndRow, nExtra,
                                                pDev, nPPTX, nPPTY, rZoomX, rZoomY, bShrink );
    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


//
//  Spalten-/Zeilen-Flags   ----------------------------------------------
//

void ScDocument::ShowCol(USHORT nCol, USHORT nTab, BOOL bShow)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->ShowCol( nCol, bShow );
}


void ScDocument::ShowRow(USHORT nRow, USHORT nTab, BOOL bShow)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->ShowRow( nRow, bShow );
}


void ScDocument::ShowRows(USHORT nRow1, USHORT nRow2, USHORT nTab, BOOL bShow)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->ShowRows( nRow1, nRow2, bShow );
}


void ScDocument::SetColFlags( USHORT nCol, USHORT nTab, BYTE nNewFlags )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetColFlags( nCol, nNewFlags );
}


void ScDocument::SetRowFlags( USHORT nRow, USHORT nTab, BYTE nNewFlags )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->SetRowFlags( nRow, nNewFlags );
}


BYTE ScDocument::GetColFlags( USHORT nCol, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetColFlags( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


BYTE ScDocument::GetRowFlags( USHORT nRow, USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetRowFlags( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetLastFlaggedCol( USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetLastFlaggedCol();
    return 0;
}


USHORT ScDocument::GetLastFlaggedRow( USHORT nTab ) const
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetLastFlaggedRow();
    return 0;
}


void ScDocument::StripHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, USHORT nTab )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->StripHidden( rX1, rY1, rX2, rY2 );
}


void ScDocument::ExtendHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, USHORT nTab )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->ExtendHidden( rX1, rY1, rX2, rY2 );
}

//
//  Attribute   ----------------------------------------------------------
//

const SfxPoolItem* ScDocument::GetAttr( USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
    {
        const SfxPoolItem* pTemp = pTab[nTab]->GetAttr( nCol, nRow, nWhich );
        if (pTemp)
            return pTemp;
        else
            DBG_ERROR( "Attribut Null" );
    }
    return &pDocPool->GetDefaultItem( nWhich );
}


const ScPatternAttr* ScDocument::GetPattern( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetPattern( nCol, nRow );
    return NULL;
}


void ScDocument::ApplyAttr( USHORT nCol, USHORT nRow, USHORT nTab, const SfxPoolItem& rAttr )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->ApplyAttr( nCol, nRow, rAttr );
}


void ScDocument::ApplyPattern( USHORT nCol, USHORT nRow, USHORT nTab, const ScPatternAttr& rAttr )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->ApplyPattern( nCol, nRow, rAttr );
}


void ScDocument::ApplyPatternArea( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark,
                        const ScPatternAttr& rAttr )
{
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}


void ScDocument::ApplyPatternAreaTab( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow, USHORT nTab, const ScPatternAttr& rAttr )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}

void ScDocument::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScMarkData& rMark, const ScPatternAttr& rPattern, short nNewType )
{
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
}


void ScDocument::ApplyStyle( USHORT nCol, USHORT nRow, USHORT nTab, const ScStyleSheet& rStyle)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyStyle( nCol, nRow, rStyle );
}


void ScDocument::ApplyStyleArea( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark,
                        const ScStyleSheet& rStyle)
{
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
}


void ScDocument::ApplyStyleAreaTab( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow, USHORT nTab, const ScStyleSheet& rStyle)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
}


void ScDocument::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    // ApplySelectionStyle needs multi mark
    if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        ScRange aRange;
        rMark.GetMarkArea( aRange );
        ApplyStyleArea( aRange.aStart.Col(), aRange.aStart.Row(),
                          aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rStyle );
    }
    else
    {
        for (USHORT i=0; i<=MAXTAB; i++)
            if ( pTab[i] && rMark.GetTableSelect(i) )
                    pTab[i]->ApplySelectionStyle( rStyle, rMark );
    }
}


void ScDocument::ApplySelectionLineStyle( const ScMarkData& rMark,
                    const SvxBorderLine* pLine, BOOL bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScDocument::GetStyle( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        return pTab[nTab]->GetStyle(nCol, nRow);
    else
        return NULL;
}


const ScStyleSheet* ScDocument::GetSelectionStyle( const ScMarkData& rMark ) const
{
    BOOL    bEqual = TRUE;
    BOOL    bFound;
    USHORT  i;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    if ( rMark.IsMultiMarked() )
        for (i=0; i<=MAXTAB && bEqual; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
            {
                pNewStyle = pTab[i]->GetSelectionStyle( rMark, bFound );
                if (bFound)
                {
                    if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                        bEqual = FALSE;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    if ( rMark.IsMarked() )
    {
        ScRange aRange;
        rMark.GetMarkArea( aRange );
        for (i=aRange.aStart.Tab(); i<=aRange.aEnd.Tab() && bEqual; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
            {
                pNewStyle = pTab[i]->GetAreaStyle( bFound,
                                        aRange.aStart.Col(), aRange.aStart.Row(),
                                        aRange.aEnd.Col(),   aRange.aEnd.Row()   );
                if (bFound)
                {
                    if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                        bEqual = FALSE;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    }

    return bEqual ? pStyle : NULL;
}


void ScDocument::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY )
{
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            pTab[i]->StyleSheetChanged
                ( pStyleSheet, bRemoved, pDev, nPPTX, nPPTY, rZoomX, rZoomY );

    if ( pStyleSheet && pStyleSheet->GetName() == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
    {
        //  update attributes for all note objects

        ScDetectiveFunc aFunc( this, 0 );
        aFunc.UpdateAllComments();
    }
}


BOOL ScDocument::IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const
{
    BOOL bIsUsed = FALSE;

    for ( USHORT i=0; (i<=MAXTAB) && !bIsUsed; i++ )
        if ( pTab[i] )
            bIsUsed = pTab[i]->IsStyleSheetUsed( rStyle );

    return bIsUsed;
}



BOOL ScDocument::ApplyFlags( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark,
                        INT16 nFlags )
{
    BOOL bChanged = FALSE;
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                bChanged |= pTab[i]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );
    return bChanged;
}


BOOL ScDocument::ApplyFlagsTab( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow, USHORT nTab, INT16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    DBG_ERROR("ApplyFlags: falsche Tabelle");
    return FALSE;
}


BOOL ScDocument::RemoveFlags( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark,
                        INT16 nFlags )
{
    BOOL bChanged = FALSE;
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                bChanged |= pTab[i]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );
    return bChanged;
}


BOOL ScDocument::RemoveFlagsTab( USHORT nStartCol, USHORT nStartRow,
                        USHORT nEndCol, USHORT nEndRow, USHORT nTab, INT16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    DBG_ERROR("RemoveFlags: falsche Tabelle");
    return FALSE;
}


void ScDocument::SetPattern( USHORT nCol, USHORT nRow, USHORT nTab, const ScPatternAttr& rAttr,
                                BOOL bPutToPool )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetPattern( nCol, nRow, rAttr, bPutToPool );
}


void ScDocument::SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr,
                                BOOL bPutToPool )
{
    USHORT nTab = rPos.Tab();
    if (pTab[nTab])
        pTab[nTab]->SetPattern( rPos, rAttr, bPutToPool );
}


ScPatternAttr* ScDocument::CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep )
{
    SfxItemSet* pSet = NULL;
    USHORT i;

    if ( rMark.IsMultiMarked() )                                // multi selection
    {
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergeSelectionPattern( &pSet, rMark, bDeep );
    }
    if ( rMark.IsMarked() )                                     // simle selection
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergePatternArea( &pSet,
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), bDeep );
    }

    DBG_ASSERT( pSet, "SelectionPattern Null" );
    if (pSet)
        return new ScPatternAttr( pSet );
    else
        return new ScPatternAttr( GetPool() );      // empty
}


const ScPatternAttr* ScDocument::GetSelectionPattern( const ScMarkData& rMark, BOOL bDeep )
{
    delete pSelectionAttr;
    pSelectionAttr = CreateSelectionPattern( rMark, bDeep );
    return pSelectionAttr;
}


void ScDocument::GetSelectionFrame( const ScMarkData& rMark,
                                    SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    rLineOuter.SetLine(NULL, BOX_LINE_TOP);
    rLineOuter.SetLine(NULL, BOX_LINE_BOTTOM);
    rLineOuter.SetLine(NULL, BOX_LINE_LEFT);
    rLineOuter.SetLine(NULL, BOX_LINE_RIGHT);
    rLineOuter.SetDistance(0);

    rLineInner.SetLine(NULL, BOXINFO_LINE_HORI);
    rLineInner.SetLine(NULL, BOXINFO_LINE_VERT);
    rLineInner.SetTable(TRUE);
    rLineInner.SetDist((BOOL)FALSE);
    rLineInner.SetMinDist(FALSE);

    ScLineFlags aFlags;

    if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        rLineInner.SetTable(aRange.aStart!=aRange.aEnd);
        for (USHORT i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergeBlockFrame( &rLineOuter, &rLineInner, aFlags,
                                          aRange.aStart.Col(), aRange.aStart.Row(),
                                          aRange.aEnd.Col(),   aRange.aEnd.Row() );
    }

        //  Don't care Status auswerten

    rLineInner.SetValid( VALID_LEFT,   ( aFlags.nLeft != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_RIGHT,  ( aFlags.nRight != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_TOP,    ( aFlags.nTop != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_BOTTOM, ( aFlags.nBottom != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_HORI,   ( aFlags.nHori != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_VERT,   ( aFlags.nVert != SC_LINE_DONTCARE ) );
}


BOOL ScDocument::HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2, USHORT nMask )
{
    if ( nMask & HASATTR_ROTATE )
    {
        //  Attribut im Dokument ueberhaupt verwendet?
        //  (wie in fillinfo)

        BOOL bAnyItem = FALSE;
        USHORT nRotCount = pDocPool->GetItemCount( ATTR_ROTATE_VALUE );
        for (USHORT nItem=0; nItem<nRotCount; nItem++)
            if (pDocPool->GetItem( ATTR_ROTATE_VALUE, nItem ))
            {
                bAnyItem = TRUE;
                break;
            }
        if (!bAnyItem)
            nMask &= ~ATTR_ROTATE_VALUE;
    }

    if (!nMask)
        return FALSE;

    BOOL bFound = FALSE;
    for (USHORT i=nTab1; i<=nTab2 && !bFound; i++)
        if (pTab[i])
            bFound |= pTab[i]->HasAttrib( nCol1, nRow1, nCol2, nRow2, nMask );

    return bFound;
}

BOOL ScDocument::HasAttrib( const ScRange& rRange, USHORT nMask )
{
    return HasAttrib( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      nMask );
}

void ScDocument::FindMaxRotCol( USHORT nTab, RowInfo* pRowInfo, USHORT nArrCount,
                                USHORT nX1, USHORT nX2 ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->FindMaxRotCol( pRowInfo, nArrCount, nX1, nX2 );
    else
        DBG_ERRORFILE("FindMaxRotCol: falsche Tabelle");
}

BOOL ScDocument::HasLines( const ScRange& rRange, Rectangle& rSizes ) const
{
    USHORT nTab1 = rRange.aStart.Tab();
    USHORT nTab2 = rRange.aEnd.Tab();
    PutInOrder( nTab1, nTab2 );
    BOOL bFound = FALSE;
    rSizes = Rectangle(0,0,0,0);

    for (USHORT i=nTab1; i<=nTab2; i++)
        if (pTab[i])
            if (pTab[i]->HasLines( rRange, rSizes ))
                bFound = TRUE;

    return bFound;
}

void ScDocument::GetBorderLines( USHORT nCol, USHORT nRow, USHORT nTab,
                        const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                        const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const
{
    //! Seitengrenzen fuer Druck beruecksichtigen !!!!!

    const SvxBoxItem* pThisAttr = (const SvxBoxItem*) GetEffItem( nCol, nRow, nTab, ATTR_BORDER );
    DBG_ASSERT(pThisAttr,"wo ist das Attribut?");

    const SvxBorderLine* pLeftLine   = pThisAttr->GetLeft();
    const SvxBorderLine* pTopLine    = pThisAttr->GetTop();
    const SvxBorderLine* pRightLine  = pThisAttr->GetRight();
    const SvxBorderLine* pBottomLine = pThisAttr->GetBottom();

    if ( nCol > 0 )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol-1, nRow, nTab, ATTR_BORDER ))->GetRight();
        if ( HasPriority( pOther, pLeftLine ) )
            pLeftLine = pOther;
    }
    if ( nRow > 0 )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow-1, nTab, ATTR_BORDER ))->GetBottom();
        if ( HasPriority( pOther, pTopLine ) )
            pTopLine = pOther;
    }
    if ( nCol < MAXCOL )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol+1, nRow, nTab, ATTR_BORDER ))->GetLeft();
        if ( HasPriority( pOther, pRightLine ) )
            pRightLine = pOther;
    }
    if ( nRow < MAXROW )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow+1, nTab, ATTR_BORDER ))->GetTop();
        if ( HasPriority( pOther, pBottomLine ) )
            pBottomLine = pOther;
    }

    if (ppLeft)
        *ppLeft = pLeftLine;
    if (ppTop)
        *ppTop = pTopLine;
    if (ppRight)
        *ppRight = pRightLine;
    if (ppBottom)
        *ppBottom = pBottomLine;
}

BOOL ScDocument::IsBlockEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEmpty( nStartCol, nStartRow, nEndCol, nEndRow );

    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


void ScDocument::LockTable(USHORT nTab)
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->LockTable();
    else
        DBG_ERROR("Falsche Tabellennummer");
}


void ScDocument::UnlockTable(USHORT nTab)
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->UnlockTable();
    else
        DBG_ERROR("Falsche Tabellennummer");
}


BOOL ScDocument::IsBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
        return FALSE;
    }

    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol,
                nEndRow, pOnlyNotBecauseOfMatrix );

    DBG_ERROR("Falsche Tabellennummer");
    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = FALSE;
    return FALSE;
}


BOOL ScDocument::IsSelectedBlockEditable( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    for (USHORT i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                if (!pTab[i]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow ))
                    bOk = FALSE;

    return bOk;
}


BOOL ScDocument::IsSelectionEditable( const ScMarkData& rMark,
            BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
        return FALSE;
    }

    ScRange aRange;
    rMark.GetMarkArea(aRange);

    BOOL bOk = TRUE;
    BOOL bMatrix = ( pOnlyNotBecauseOfMatrix != NULL );
    for ( USHORT i=0; i<=MAXTAB && (bOk || bMatrix); i++ )
    {
        if ( pTab[i] && rMark.GetTableSelect(i) )
        {
            if (rMark.IsMarked())
            {
                if ( !pTab[i]->IsBlockEditable( aRange.aStart.Col(),
                        aRange.aStart.Row(), aRange.aEnd.Col(),
                        aRange.aEnd.Row(), pOnlyNotBecauseOfMatrix ) )
                {
                    bOk = FALSE;
                    if ( pOnlyNotBecauseOfMatrix )
                        bMatrix = *pOnlyNotBecauseOfMatrix;
                }
            }
            if (rMark.IsMultiMarked())
            {
                if ( !pTab[i]->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix ) )
                {
                    bOk = FALSE;
                    if ( pOnlyNotBecauseOfMatrix )
                        bMatrix = *pOnlyNotBecauseOfMatrix;
                }
            }
        }
    }

    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = ( !bOk && bMatrix );

    return bOk;
}


BOOL ScDocument::IsSelectionOrBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            if (rMark.IsMarked())
            {
                ScRange aRange;
                rMark.GetMarkArea(aRange);
                bOk = pTab[nTab]->IsBlockEditable( aRange.aStart.Col(), aRange.aStart.Row(),
                                                   aRange.aEnd.Col(),   aRange.aEnd.Row() );
            }

            if (bOk && rMark.IsMultiMarked())
                bOk = pTab[nTab]->IsSelectionEditable( rMark );
            if ( bOk && !rMark.IsMarked() && !rMark.IsMultiMarked() )
                bOk = pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow );
            return bOk;
        }
    }
    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


BOOL ScDocument::IsSelectedOrBlockEditable( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    for (USHORT i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
            {
                if (rMark.IsMarked())
                {
                    ScRange aRange;
                    rMark.GetMarkArea(aRange);
                    bOk = pTab[i]->IsBlockEditable( aRange.aStart.Col(), aRange.aStart.Row(),
                                                    aRange.aEnd.Col(),   aRange.aEnd.Row() );
                }
                if ( bOk && !rMark.IsMarked() )
                    bOk = pTab[i]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow );
            }

    return bOk;
}


BOOL ScDocument::HasSelectedBlockMatrixFragment( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow,
                                const ScMarkData& rMark ) const
{
    BOOL bOk = TRUE;
    for (USHORT i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                if (pTab[i]->HasBlockMatrixFragment( nStartCol, nStartRow, nEndCol, nEndRow ))
                    bOk = FALSE;

    return !bOk;
}


BOOL ScDocument::ExtendOverlapped( USHORT& rStartCol, USHORT& rStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab )
{
    BOOL bFound = FALSE;
    if ( ValidColRow(rStartCol,rStartRow) && ValidColRow(nEndCol,nEndRow) && nTab<=MAXTAB )
    {
        if (pTab[nTab])
        {
            USHORT nCol;
            USHORT nOldCol = rStartCol;
            USHORT nOldRow = rStartRow;
            for (nCol=nOldCol; nCol<=nEndCol; nCol++)
                while (((ScMergeFlagAttr*)GetAttr(nCol,rStartRow,nTab,ATTR_MERGE_FLAG))->
                            IsVerOverlapped())
                    --rStartRow;

            //!     weiterreichen ?

            ScAttrArray* pAttrArray = pTab[nTab]->aCol[nOldCol].pAttrArray;
            short nIndex;
            pAttrArray->Search( nOldRow, nIndex );
            USHORT nAttrPos = nOldRow;
            while (nAttrPos<=nEndRow)
            {
                DBG_ASSERT( nIndex < (short) pAttrArray->nCount, "Falscher Index im AttrArray" );

                if (((ScMergeFlagAttr&)pAttrArray->pData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped())
                {
                    USHORT nLoopEndRow = Min( nEndRow, pAttrArray->pData[nIndex].nRow );
                    for (USHORT nAttrRow = nAttrPos; nAttrRow <= nLoopEndRow; nAttrRow++)
                    {
                        USHORT nTempCol = nOldCol;
                        do
                            --nTempCol;
                        while (((ScMergeFlagAttr*)GetAttr(nTempCol,nAttrRow,nTab,ATTR_MERGE_FLAG))
                                ->IsHorOverlapped());
                        if (nTempCol < rStartCol)
                            rStartCol = nTempCol;
                    }
                }
                nAttrPos = pAttrArray->pData[nIndex].nRow + 1;
                ++nIndex;
            }
        }
    }
    else
        DBG_ERROR("ExtendOverlapped: falscher Bereich");

    return bFound;
}


BOOL ScDocument::ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                              USHORT& rEndCol,  USHORT& rEndRow,
                              USHORT nTab, BOOL bRefresh, BOOL bAttrs )
{
    BOOL bFound = FALSE;
    if ( ValidColRow(nStartCol,nStartRow) && ValidColRow(rEndCol,rEndRow) && nTab<=MAXTAB )
    {
        if (pTab[nTab])
            bFound = pTab[nTab]->ExtendMerge( nStartCol, nStartRow, rEndCol, rEndRow, bRefresh, bAttrs );

        if (bRefresh)
            RefreshAutoFilter( nStartCol, nStartRow, rEndCol, rEndRow, nTab );
    }
    else
        DBG_ERROR("ExtendMerge: falscher Bereich");

    return bFound;
}


BOOL ScDocument::ExtendMerge( ScRange& rRange, BOOL bRefresh, BOOL bAttrs )
{
    BOOL bFound = FALSE;
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndTab   = rRange.aEnd.Tab();
    USHORT nEndCol   = rRange.aEnd.Col();
    USHORT nEndRow   = rRange.aEnd.Row();

    PutInOrder( nStartTab, nEndTab );
    for (USHORT nTab = nStartTab; nTab <= nEndTab; nTab++ )
    {
        USHORT nExtendCol = rRange.aEnd.Col();
        USHORT nExtendRow = rRange.aEnd.Row();
        if (ExtendMerge( rRange.aStart.Col(), rRange.aStart.Row(),
                         nExtendCol,          nExtendRow,
                         nTab, bRefresh, bAttrs ) )
        {
            bFound = TRUE;
            if (nExtendCol > nEndCol) nEndCol = nExtendCol;
            if (nExtendRow > nEndRow) nEndRow = nExtendRow;
        }
    }

    rRange.aEnd.SetCol(nEndCol);
    rRange.aEnd.SetRow(nEndRow);

    return bFound;
}

BOOL ScDocument::ExtendTotalMerge( ScRange& rRange )
{
    //  Bereich genau dann auf zusammengefasste Zellen erweitern, wenn
    //  dadurch keine neuen nicht-ueberdeckten Zellen getroffen werden

    BOOL bRet = FALSE;
    ScRange aExt = rRange;
    if (ExtendMerge(aExt))
    {
        if ( aExt.aEnd.Row() > rRange.aEnd.Row() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetRow( rRange.aEnd.Row() + 1 );
            if ( HasAttrib( aTest, HASATTR_NOTOVERLAPPED ) )
                aExt.aEnd.SetRow(rRange.aEnd.Row());
        }
        if ( aExt.aEnd.Col() > rRange.aEnd.Col() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetCol( rRange.aEnd.Col() + 1 );
            if ( HasAttrib( aTest, HASATTR_NOTOVERLAPPED ) )
                aExt.aEnd.SetCol(rRange.aEnd.Col());
        }

        bRet = ( aExt.aEnd != rRange.aEnd );
        rRange = aExt;
    }
    return bRet;
}

BOOL ScDocument::ExtendOverlapped( ScRange& rRange )
{
    BOOL bFound = FALSE;
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndTab   = rRange.aEnd.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();

    PutInOrder( nStartTab, nEndTab );
    for (USHORT nTab = nStartTab; nTab <= nEndTab; nTab++ )
    {
        USHORT nExtendCol = rRange.aStart.Col();
        USHORT nExtendRow = rRange.aStart.Row();
        ExtendOverlapped( nExtendCol, nExtendRow,
                                rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
        if (nExtendCol < nStartCol)
        {
            nStartCol = nExtendCol;
            bFound = TRUE;
        }
        if (nExtendRow < nStartRow)
        {
            nStartRow = nExtendRow;
            bFound = TRUE;
        }
    }

    rRange.aStart.SetCol(nStartCol);
    rRange.aStart.SetRow(nStartRow);

    return bFound;
}

BOOL ScDocument::RefreshAutoFilter( USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow, USHORT nTab )
{
    USHORT nCount = pDBCollection->GetCount();
    USHORT i;
    ScDBData* pData;
    USHORT nDBTab;
    USHORT nDBStartCol;
    USHORT nDBStartRow;
    USHORT nDBEndCol;
    USHORT nDBEndRow;

    //      Autofilter loeschen

    BOOL bChange = RemoveFlagsTab( nStartCol,nStartRow, nEndCol,nEndRow, nTab, SC_MF_AUTO );

    //      Autofilter setzen

    for (i=0; i<nCount; i++)
    {
        pData = (*pDBCollection)[i];
        if (pData->HasAutoFilter())
        {
            pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, SC_MF_AUTO ))
                    bChange = TRUE;
            }
        }
    }
    return bChange;
}


void ScDocument::SetAutoFilterFlags()
{
    USHORT nCount = pDBCollection->GetCount();
    for (USHORT i=0; i<nCount; i++)
    {
        ScDBData* pData = (*pDBCollection)[i];
        USHORT nDBTab;
        USHORT nDBStartCol;
        USHORT nDBStartRow;
        USHORT nDBEndCol;
        USHORT nDBEndRow;
        pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
        pData->SetAutoFilter( HasAttrib( nDBStartCol,nDBStartRow,nDBTab,
                                nDBEndCol,nDBStartRow,nDBTab, HASATTR_AUTOFILTER ) );
    }
}


BOOL ScDocument::IsOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


BOOL ScDocument::IsHorOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsHorOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


BOOL ScDocument::IsVerOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsVerOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


void ScDocument::ApplySelectionFrame( const ScMarkData& rMark,
                                      const SvxBoxItem* pLineOuter,
                                      const SvxBoxInfoItem* pLineInner )
{
    if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        for (USHORT i=0; i<=MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->ApplyBlockFrame( pLineOuter, pLineInner,
                                                aRange.aStart.Col(), aRange.aStart.Row(),
                                                aRange.aEnd.Col(),   aRange.aEnd.Row() );
    }
}


void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark )
{
    const SfxItemSet* pSet = &rAttr.GetItemSet();
    BOOL bSet = FALSE;
    USHORT i;
    for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
        if (pSet->GetItemState(i) == SFX_ITEM_SET)
            bSet = TRUE;

    if (bSet)
    {
        // ApplySelectionCache needs multi mark
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ApplyPatternArea( aRange.aStart.Col(), aRange.aStart.Row(),
                              aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rAttr );
        }
        else
        {
            SfxItemPoolCache aCache( pDocPool, pSet );
            for (USHORT i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    if (rMark.GetTableSelect(i))
                        pTab[i]->ApplySelectionCache( &aCache, rMark );
        }
    }
}


void ScDocument::ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ChangeSelectionIndent( bIncrement, rMark );
}


void ScDocument::ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ClearSelectionItems( pWhich, rMark );
}


void ScDocument::DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->DeleteSelection( nDelFlag, rMark );
}


void ScDocument::DeleteSelectionTab( USHORT nTab, USHORT nDelFlag, const ScMarkData& rMark )
{
    if (nTab <= MAXTAB && pTab[nTab])
        pTab[nTab]->DeleteSelection( nDelFlag, rMark );
    else
        DBG_ERROR("Falsche Tabelle");
}


ScPatternAttr* ScDocument::GetDefPattern() const
{
    return (ScPatternAttr*) &pDocPool->GetDefaultItem(ATTR_PATTERN);
}


ScDocumentPool* ScDocument::GetPool()
{
    return pDocPool;
}



ScStyleSheetPool* ScDocument::GetStyleSheetPool() const
{
    return pStylePool;
}


USHORT ScDocument::GetEmptyLinesInBlock( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                            USHORT nEndCol, USHORT nEndRow, USHORT nEndTab, ScDirection eDir )
{
    PutInOrder(nStartCol, nEndCol);
    PutInOrder(nStartRow, nEndRow);
    PutInOrder(nStartTab, nEndTab);
    if (VALIDTAB(nStartTab))
    {
        if (pTab[nStartTab])
            return pTab[nStartTab]->GetEmptyLinesInBlock(nStartCol, nStartRow, nEndCol, nEndRow, eDir);
        else
            return 0;
    }
    else
        return 0;
}


void ScDocument::FindAreaPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->FindAreaPos( rCol, rRow, nMovX, nMovY );
}


void ScDocument::GetNextPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
{
    DBG_ASSERT( !nMovX || !nMovY, "GetNextPos: nur X oder Y" );

    ScMarkData aCopyMark = rMark;
    aCopyMark.SetMarking(FALSE);
    aCopyMark.MarkToMulti();

    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->GetNextPos( rCol, rRow, nMovX, nMovY, bMarked, bUnprotected, aCopyMark );
}

//
//  Datei-Operationen
//


void ScDocument::UpdStlShtPtrsFrmNms()
{
    ScPatternAttr::pDoc = this;

    USHORT nCount = pDocPool->GetItemCount(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (USHORT i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pDocPool->GetItem(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->UpdateStyleSheet();
    }
    ((ScPatternAttr&)pDocPool->GetDefaultItem(ATTR_PATTERN)).UpdateStyleSheet();
}


void ScDocument::StylesToNames()
{
    ScPatternAttr::pDoc = this;

    USHORT nCount = pDocPool->GetItemCount(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (USHORT i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pDocPool->GetItem(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->StyleToName();
    }
    ((ScPatternAttr&)pDocPool->GetDefaultItem(ATTR_PATTERN)).StyleToName();
}


void lcl_RemoveMergeFromStyles( ScStyleSheetPool* pStylePool )
{
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_ALL );

    USHORT nCount = pStylePool->Count();
    for (USHORT i=0; i<nCount; i++)
    {
        //  in alten Versionen wurden statt SFXSTYLEBIT_USERDEF alle Bits gesetzt
        SfxStyleSheetBase* pStyle = (*pStylePool)[i];
        if ( pStyle->GetMask() & SFXSTYLEBIT_READONLY )
            pStyle->SetMask( pStyle->GetMask() & ~SFXSTYLEBIT_READONLY );

        SfxItemSet& rSet = pStyle->GetItemSet();
        rSet.ClearItem( ATTR_MERGE );
        rSet.ClearItem( ATTR_MERGE_FLAG );

        //  Das SvxBoxInfoItem wurde bis zur 358 falsch geladen, so dass
        //  Seitenvorlagen falsche Items mit bDist = FALSE enthalten koennen
        if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PAGE )
        {
            const SvxBoxInfoItem& rPageInfo = (const SvxBoxInfoItem&)rSet.Get(ATTR_BORDER_INNER);
            if ( !rPageInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rPageInfo );
                aNew.SetDist( TRUE );
                rSet.Put( aNew );
            }
            //  Das gilt fuer alle Hdr/Ftr-SetItems, darum kann das SetItem auch
            //  direkt im Pool geaendert werden (const weggecastet):
            SfxItemSet& rHdrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_HEADERSET)).GetItemSet();
            const SvxBoxInfoItem& rHdrInfo = (const SvxBoxInfoItem&)rHdrSet.Get(ATTR_BORDER_INNER);
            if ( !rHdrInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rHdrInfo );
                aNew.SetDist( TRUE );
                rHdrSet.Put( aNew );
            }
            SfxItemSet& rFtrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_FOOTERSET)).GetItemSet();
            const SvxBoxInfoItem& rFtrInfo = (const SvxBoxInfoItem&)rFtrSet.Get(ATTR_BORDER_INNER);
            if ( !rFtrInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rFtrInfo );
                aNew.SetDist( TRUE );
                rFtrSet.Put( aNew );
            }
            const SfxUInt16Item& rScaleItem = (const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALE);
            USHORT nScale = rScaleItem.GetValue();
            //! Extra-Konstanten fuer Seitenformat?
            //  0 ist erlaubt (wird gesetzt bei Scale To Pages)
            if ( nScale != 0 && ( nScale < MINZOOM || nScale > MAXZOOM ) )
            {
                //  konnte anscheinend mal irgendwie kaputtgehen (#34508#)
                DBG_WARNING("kaputter Zoom im Seitenformat muss korrigiert werden");
                rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
            }
        }
    }
}


BOOL ScDocument::LoadPool( SvStream& rStream, BOOL bLoadRefCounts )
{
    //  bLoadingDone wird beim Laden des StylePools (ScStyleSheet::GetItemSet) gebraucht
    bLoadingDone = FALSE;

    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );
    CharSet eOldSet = rStream.GetStreamCharSet();

    SetPrinter( NULL );

    ScPatternAttr::pDoc = this;
//  pStylePool->Clear();
//  pDocPool->Cleanup();

    ScDocument* pClipDoc = ScGlobal::GetClipDoc();
    if (bOwner && !bIsClip && pClipDoc->pDocPool == pDocPool)
    {
        pClipDoc->bOwner = TRUE;
        pStylePool->SetDocument(pClipDoc);      // #67178# don't keep old document pointer
    }
    else
    {
        delete pStylePool;
        delete pDocPool;
        delete pFormTable;
        delete pEditPool;
        delete pEnginePool;
    }
    pFormTable = new SvNumberFormatter( GetServiceManager(), ScGlobal::eLnge );
    pFormTable->SetColorLink(&aColorLink);
    pFormTable->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
    pDocPool = new ScDocumentPool( NULL, bLoadRefCounts );
    pDocPool->FreezeIdRanges();
    pDocPool->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
    pStylePool = new ScStyleSheetPool( *pDocPool, this );
    pEditPool = EditEngine::CreatePool();
    pEditPool->FreezeIdRanges();
    pEnginePool = EditEngine::CreatePool();
    pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEnginePool->FreezeIdRanges();

    BOOL bStylesFound = FALSE;

    BOOL bRet = FALSE;
    USHORT nID;
    rStream >> nID;
    if (nID == SCID_POOLS || nID == SCID_NEWPOOLS)
    {
        ScReadHeader aHdr( rStream );
        while (aHdr.BytesLeft())
        {
            USHORT nSubID;
            rStream >> nSubID;
            ScReadHeader aSubHdr( rStream );
            switch (nSubID)
            {
                case SCID_CHARSET:
                    {
                        BYTE cSet, cGUI;    // cGUI is dummy, old GUIType
                        rStream >> cGUI >> cSet;
                        eSrcSet = (CharSet) cSet;
                        rStream.SetStreamCharSet( eSrcSet );
                    }
                    break;
                case SCID_DOCPOOL:
                    pDocPool->Load( rStream );
                    break;
                case SCID_STYLEPOOL:
                    {
                        //  StylePool konvertiert beim Laden selber
                        CharSet eOld = rStream.GetStreamCharSet();
                        rStream.SetStreamCharSet( gsl_getSystemTextEncoding() );    //! ???
                        pStylePool->Load( rStream );
                        rStream.SetStreamCharSet( eOld );
                        lcl_RemoveMergeFromStyles( pStylePool );    // setzt auch ReadOnly zurueck
                        bStylesFound = TRUE;
                    }
                    break;
                case SCID_EDITPOOL :
                    pEditPool->Load( rStream );
                    break;
                default:
                    DBG_ERROR("unbekannter Sub-Record in ScDocument::LoadPool");
            }
        }

        UpdStlShtPtrsFrmNms();
        bRet = TRUE;
    }
    else
        DBG_ERROR("LoadPool: SCID_POOLS nicht gefunden");

    if (!bStylesFound)
        pStylePool->CreateStandardStyles();

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );

    bLoadingDone = TRUE;

    //  Das Uno-Objekt merkt sich einen Pointer auf den NumberFormatter
    //  -> mitteilen, dass der alte Pointer ungueltig geworden ist
    BroadcastUno( ScPointerChangedHint(SC_POINTERCHANGED_NUMFMT) );

    return bRet;
}


BOOL ScDocument::SavePool( SvStream& rStream ) const
{
    pDocPool->SetFileFormatVersion( (USHORT)rStream.GetVersion() );

    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );
    CharSet eOldSet = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( gsl_getSystemTextEncoding() );

    //  Compress-Mode fuer Grafiken in Brush-Items (Hintergrund im Seitenformat)

    USHORT nComprMode = rStream.GetCompressMode() & ~(COMPRESSMODE_ZBITMAP | COMPRESSMODE_NATIVE);
    SvtSaveOptions aSaveOpt;
    if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_40 &&
            aSaveOpt.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsCompressed )
        nComprMode |= COMPRESSMODE_ZBITMAP;             //  komprimiert ab 4.0
    if ( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 &&
            aSaveOpt.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsOriginal )
        nComprMode |= COMPRESSMODE_NATIVE;              //  Originalformat ab 5.0
    rStream.SetCompressMode( nComprMode );

    {
        rStream << (USHORT) SCID_NEWPOOLS;
        ScWriteHeader aHdr( rStream );

        {
            rStream << (USHORT) SCID_CHARSET;
            ScWriteHeader aSetHdr( rStream, 2 );
            rStream << (BYTE) 0     // dummy, old System::GetGUIType()
                    << (BYTE) ::GetStoreCharSet( gsl_getSystemTextEncoding() );
        }

        {
            rStream << (USHORT) SCID_DOCPOOL;
            ScWriteHeader aDocPoolHdr( rStream );
            pDocPool->Store( rStream );
        }

        {
            rStream << (USHORT) SCID_STYLEPOOL;
            ScWriteHeader aStylePoolHdr( rStream );
            pStylePool->SetSearchMask( SFX_STYLE_FAMILY_ALL );

            //  Um bisherige 5.0 und 5.1 Versionen nicht durcheinander zu bringen,
            //  muss die Standardvorlage unter dem Namen "Standard" in der Datei
            //  stehen, weil bei UpdateStyleSheet sonst die Vorlage fuer das
            //  Default-PatternAttr nicht gefunden wird.

            //! Fuer eine spaetere Dateiversion kann das wegfallen!

            String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
            if ( aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
                pStylePool->SetForceStdName( &aFileStdName );

            pStylePool->Store( rStream, FALSE );

            pStylePool->SetForceStdName( NULL );
        }

        if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_50 )
        {
            rStream << (USHORT) SCID_EDITPOOL;
            ScWriteHeader aEditPoolHdr( rStream );
            pEditPool->Store( rStream );
        }
    }

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );
    return TRUE;
}


long ScDocument::GetCellCount() const
{
    long nCellCount = 0L;

    for ( USHORT nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCellCount += pTab[nTab]->GetCellCount();

    return nCellCount;
}


ULONG ScDocument::GetCodeCount() const
{
    ULONG nCodeCount = 0;

    for ( USHORT nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCodeCount += pTab[nTab]->GetCodeCount();

    return nCodeCount;
}


long ScDocument::GetWeightedCount() const
{
    long nCellCount = 0L;

    for ( USHORT nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCellCount += pTab[nTab]->GetWeightedCount();

    return nCellCount;
}


void ScDocument::PageStyleModified( USHORT nTab, const String& rNewName )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->PageStyleModified( rNewName );
}


void ScDocument::SetPageStyle( USHORT nTab, const String& rName )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->SetPageStyle( rName );
}


const String& ScDocument::GetPageStyle( USHORT nTab ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetPageStyle();

    return EMPTY_STRING;
}


void ScDocument::SetPageSize( USHORT nTab, const Size& rSize )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->SetPageSize( rSize );
}

Size ScDocument::GetPageSize( USHORT nTab ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        return pTab[nTab]->GetPageSize();

    DBG_ERROR("falsche Tab");
    return Size();
}


void ScDocument::SetRepeatArea( USHORT nTab, USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->SetRepeatArea( nStartCol, nEndCol, nStartRow, nEndRow );
}


void ScDocument::UpdatePageBreaks()
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i])
            pTab[i]->UpdatePageBreaks( NULL );
}


void ScDocument::UpdatePageBreaks( USHORT nTab, const ScRange* pUserArea )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->UpdatePageBreaks( pUserArea );
}

void ScDocument::RemoveManualBreaks( USHORT nTab )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->RemoveManualBreaks();
}

BOOL ScDocument::HasManualBreaks( USHORT nTab ) const
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        return pTab[nTab]->HasManualBreaks();

    DBG_ERROR("falsche Tab");
    return FALSE;
}


void ScDocument::GetDocStat( ScDocStat& rDocStat )
{
    rDocStat.nTableCount = GetTableCount();
    rDocStat.aDocName    = aDocName;
    rDocStat.nCellCount  = GetCellCount();
}


BOOL ScDocument::HasPrintRange()
{
    BOOL bResult = FALSE;

    for ( USHORT i=0; !bResult && i<nMaxTableNumber; i++ )
        if ( pTab[i] )
            bResult = ( pTab[i]->GetPrintRangeCount() > 0 );

    return bResult;
}


USHORT ScDocument::GetPrintRangeCount( USHORT nTab )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetPrintRangeCount();

    return 0;
}


const ScRange* ScDocument::GetPrintRange( USHORT nTab, USHORT nPos )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetPrintRange(nPos);

    return NULL;
}


const ScRange* ScDocument::GetRepeatColRange( USHORT nTab )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetRepeatColRange();

    return NULL;
}


const ScRange* ScDocument::GetRepeatRowRange( USHORT nTab )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetRepeatRowRange();

    return NULL;
}


// #42845# zeroptimiert
#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif
void ScDocument::SetPrintRangeCount( USHORT nTab, USHORT nNew )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetPrintRangeCount( nNew );
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif


void ScDocument::SetPrintRange( USHORT nTab, USHORT nPos, const ScRange& rNew )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetPrintRange( nPos, rNew );
}


void ScDocument::SetRepeatColRange( USHORT nTab, const ScRange* pNew )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetRepeatColRange( pNew );
}


void ScDocument::SetRepeatRowRange( USHORT nTab, const ScRange* pNew )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetRepeatRowRange( pNew );
}


ScPrintRangeSaver* ScDocument::CreatePrintRangeSaver() const
{
    USHORT nCount = GetTableCount();
    ScPrintRangeSaver* pNew = new ScPrintRangeSaver( nCount );
    for (USHORT i=0; i<nCount; i++)
        if (pTab[i])
            pTab[i]->FillPrintSaver( pNew->GetTabData(i) );
    return pNew;
}


void ScDocument::RestorePrintRanges( const ScPrintRangeSaver& rSaver )
{
    USHORT nCount = rSaver.GetTabCount();
    for (USHORT i=0; i<nCount; i++)
        if (pTab[i])
            pTab[i]->RestorePrintRanges( rSaver.GetTabData(i) );
}


BOOL ScDocument::NeedPageResetAfterTab( USHORT nTab ) const
{
    //  Die Seitennummern-Zaehlung fängt bei einer Tabelle neu an, wenn eine
    //  andere Vorlage als bei der vorherigen gesetzt ist (nur Namen vergleichen)
    //  und eine Seitennummer angegeben ist (nicht 0)

    if ( nTab < MAXTAB && pTab[nTab] && pTab[nTab+1] )
    {
        String aNew = pTab[nTab+1]->GetPageStyle();
        if ( aNew != pTab[nTab]->GetPageStyle() )
        {
            SfxStyleSheetBase* pStyle = pStylePool->Find( aNew, SFX_STYLE_FAMILY_PAGE );
            if ( pStyle )
            {
                const SfxItemSet& rSet = pStyle->GetItemSet();
                USHORT nFirst = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_FIRSTPAGENO)).GetValue();
                if ( nFirst != 0 )
                    return TRUE;        // Seitennummer in neuer Vorlage angegeben
            }
        }
    }

    return FALSE;       // sonst nicht
}



