/*************************************************************************
 *
 *  $RCSfile: documen3.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-23 06:58:19 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifdef WIN
// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
#define _BASEDLGS_HXX
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX ***

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX ***
//#define _SFXDOCINF_HXX
#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
#define _SFXDOCFILE_HXX
#define _VIEWFAC_HXX
#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX ***
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX

#endif  //WIN

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/langitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/zforlist.hxx>

#include "document.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "docpool.hxx"
#include "poolhelp.hxx"
#include "autoform.hxx"
#include "rangelst.hxx"
#include "chartarr.hxx"
#include "refupdat.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "tablink.hxx"
#include "markdata.hxx"
#include "conditio.hxx"
#include "dociter.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "scmod.hxx"        // SC_MOD
#include "inputopt.hxx"     // GetExpandRefs
#include "chartlis.hxx"
#include "sc.hrc"           // SID_LINK
#include "hints.hxx"
#include "dpobject.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScRangeName* ScDocument::GetRangeName()
{
    return pRangeName;
}

void ScDocument::SetRangeName( ScRangeName* pNewRangeName )
{
    if (pRangeName)
        delete pRangeName;
    pRangeName = pNewRangeName;
}

ScRangeData* ScDocument::GetRangeAtCursor(USHORT nCol, USHORT nRow, USHORT nTab,
                                            BOOL bStartOnly) const
{
    if ( pRangeName )
        return pRangeName->GetRangeAtCursor( ScAddress( nCol, nRow, nTab ), bStartOnly );
    else
        return NULL;
}

ScRangeData* ScDocument::GetRangeAtBlock( const ScRange& rBlock, String* pName ) const
{
    ScRangeData* pData = NULL;
    if ( pRangeName )
    {
        pData = pRangeName->GetRangeAtBlock( rBlock );
        if (pData && pName)
            *pName = pData->GetName();
    }
    return pData;
}

ScDBCollection* ScDocument::GetDBCollection() const
{
    return pDBCollection;
}

void ScDocument::SetDBCollection( ScDBCollection* pNewDBCollection, BOOL bRemoveAutoFilter )
{
    if ( bRemoveAutoFilter )
    {
        //  remove auto filter attribute if new db data don't contain auto filter flag
        //  start position is also compared, so bRemoveAutoFilter must not be set from ref-undo!

        if ( pDBCollection )
        {
            USHORT nOldCount = pDBCollection->GetCount();
            for (USHORT nOld=0; nOld<nOldCount; nOld++)
            {
                ScDBData* pOldData = (*pDBCollection)[nOld];
                if ( pOldData->HasAutoFilter() )
                {
                    ScRange aOldRange;
                    pOldData->GetArea( aOldRange );

                    BOOL bFound = FALSE;
                    USHORT nNewIndex = 0;
                    if ( pNewDBCollection &&
                        pNewDBCollection->SearchName( pOldData->GetName(), nNewIndex ) )
                    {
                        ScDBData* pNewData = (*pNewDBCollection)[nNewIndex];
                        if ( pNewData->HasAutoFilter() )
                        {
                            ScRange aNewRange;
                            pNewData->GetArea( aNewRange );
                            if ( aOldRange.aStart == aNewRange.aStart )
                                bFound = TRUE;
                        }
                    }

                    if ( !bFound )
                    {
                        aOldRange.aEnd.SetRow( aOldRange.aStart.Row() );
                        RemoveFlagsTab( aOldRange.aStart.Col(), aOldRange.aStart.Row(),
                                        aOldRange.aEnd.Col(),   aOldRange.aEnd.Row(),
                                        aOldRange.aStart.Tab(), SC_MF_AUTO );
                        if (pShell)
                            pShell->Broadcast( ScPaintHint( aOldRange, PAINT_GRID ) );
                    }
                }
            }
        }
    }

    if (pDBCollection)
        delete pDBCollection;
    pDBCollection = pNewDBCollection;
}

ScDBData* ScDocument::GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, bStartOnly);
    else
        return NULL;
}

ScDBData* ScDocument::GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
    else
        return NULL;
}

ScDPCollection* ScDocument::GetDPCollection()
{
    if (!pDPCollection)
        pDPCollection = new ScDPCollection(this);
    return pDPCollection;
}

ScDPObject* ScDocument::GetDPAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const
{
    if (!pDPCollection)
        return NULL;

    USHORT nCount = pDPCollection->GetCount();
    ScAddress aPos( nCol, nRow, nTab );
    for (USHORT i=0; i<nCount; i++)
        if ( (*pDPCollection)[i]->GetOutRange().In( aPos ) )
            return (*pDPCollection)[i];

    return NULL;
}

ScPivotCollection* ScDocument::GetPivotCollection() const
{
    return pPivotCollection;
}

void ScDocument::SetPivotCollection(ScPivotCollection* pNewPivotCollection)
{
    if ( pPivotCollection && pNewPivotCollection &&
            *pPivotCollection == *pNewPivotCollection )
    {
        delete pNewPivotCollection;
        return;
    }

    if (pPivotCollection)
        delete pPivotCollection;
    pPivotCollection = pNewPivotCollection;

    if (pPivotCollection)
    {
        USHORT nCount = pPivotCollection->GetCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScPivot* pPivot = (*pPivotCollection)[i];
            if (pPivot->CreateData())
                pPivot->ReleaseData();
        }
    }
}

ScPivot* ScDocument::GetPivotAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const
{
    if (pPivotCollection)
        return pPivotCollection->GetPivotAtCursor(nCol, nRow, nTab);
    else
        return NULL;
}

ScChartCollection* ScDocument::GetChartCollection() const
{
    return pChartCollection;
}

void ScDocument::SetChartCollection(ScChartCollection* pNewChartCollection)
{
    if (pChartCollection)
        delete pChartCollection;
    pChartCollection = pNewChartCollection;
}

void ScDocument::SetChartListenerCollection(
            ScChartListenerCollection* pNewChartListenerCollection,
            BOOL bSetChartRangeLists )
{
    ScChartListenerCollection* pOld = pChartListenerCollection;
    pChartListenerCollection = pNewChartListenerCollection;
    if ( pChartListenerCollection )
    {
        if ( pOld )
            pChartListenerCollection->SetDiffDirty( *pOld, bSetChartRangeLists );
        pChartListenerCollection->StartAllListeners();
    }
    delete pOld;
}

void ScDocument::SetScenario( USHORT nTab, BOOL bFlag )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetScenario(bFlag);
}

BOOL ScDocument::IsScenario( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->IsScenario();

    return FALSE;
}

void ScDocument::SetScenarioData( USHORT nTab, const String& rComment,
                                        const Color& rColor, USHORT nFlags )
{
    if (nTab<=MAXTAB && pTab[nTab] && pTab[nTab]->IsScenario())
    {
        pTab[nTab]->SetScenarioComment( rComment );
        pTab[nTab]->SetScenarioColor( rColor );
        pTab[nTab]->SetScenarioFlags( nFlags );
    }
}

void ScDocument::GetScenarioData( USHORT nTab, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const
{
    if (nTab<=MAXTAB && pTab[nTab] && pTab[nTab]->IsScenario())
    {
        pTab[nTab]->GetScenarioComment( rComment );
        rColor = pTab[nTab]->GetScenarioColor();
        rFlags = pTab[nTab]->GetScenarioFlags();
    }
}

BOOL ScDocument::IsLinked( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->IsLinked();
    return FALSE;
}

BOOL ScDocument::GetLinkMode( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetLinkMode();
    return SC_LINK_NONE;
}

const String& ScDocument::GetLinkDoc( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetLinkDoc();
    return EMPTY_STRING;
}

const String& ScDocument::GetLinkFlt( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetLinkFlt();
    return EMPTY_STRING;
}

const String& ScDocument::GetLinkOpt( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetLinkOpt();
    return EMPTY_STRING;
}

const String& ScDocument::GetLinkTab( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetLinkTab();
    return EMPTY_STRING;
}

void ScDocument::SetLink( USHORT nTab, BYTE nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetLink( nMode, rDoc, rFilter, rOptions, rTabName );
}

BOOL ScDocument::HasLink( const String& rDoc,
                            const String& rFilter, const String& rOptions ) const
{
    USHORT nCount = GetTableCount();
    for (USHORT i=0; i<nCount; i++)
        if (pTab[i]->IsLinked()
                && pTab[i]->GetLinkDoc() == rDoc
                && pTab[i]->GetLinkFlt() == rFilter
                && pTab[i]->GetLinkOpt() == rOptions)
            return TRUE;

    return FALSE;
}

BOOL ScDocument::LinkEmptyTab( USHORT& rTab, const String& aDocTab,
        const String& aFileName, const String& aTabName )
{
    if ( IsClipboard() )
    {
        DBG_ERRORFILE( "LinkExternalTab in Clipboard" );
        return FALSE;
    }
    rTab = 0;
    String  aFilterName;    // wird vom Loader gefuellt
    String  aOptions;       // Filter-Optionen

    if ( !InsertTab( SC_TAB_APPEND, aDocTab, TRUE ) )
    {
        DBG_ERRORFILE("can't insert external document table");
        return FALSE;
    }
    rTab = GetTableCount() - 1;

    BOOL bWasThere = HasLink( aFileName, aFilterName, aOptions );
    SetLink( rTab, SC_LINK_VALUE, aFileName, aFilterName, aOptions, aTabName );
    if ( !bWasThere )       // Link pro Quelldokument nur einmal eintragen
    {
        ScTableLink* pLink = new ScTableLink( pShell, aFileName, aFilterName, aOptions );
        pLink->SetInCreate( TRUE );
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName,
                                        &aFilterName );
        pLink->Update();
        pLink->SetInCreate( FALSE );
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }
    return TRUE;
}

BOOL ScDocument::LinkExternalTab( USHORT& rTab, const String& aDocTab,
        const String& aFileName, const String& aTabName )
{
    if ( IsClipboard() )
    {
        DBG_ERRORFILE( "LinkExternalTab in Clipboard" );
        return FALSE;
    }
    rTab = 0;
    String  aFilterName;        // wird vom Loader gefuellt
    String  aOptions;       // Filter-Optionen
    ScDocumentLoader aLoader( aFileName, aFilterName, aOptions,
        pExtDocOptions ? pExtDocOptions->nLinkCnt + 1 : 1 );
    if ( aLoader.IsError() )
        return FALSE;
    ScDocument* pSrcDoc = aLoader.GetDocument();

    //  Tabelle kopieren
    USHORT nSrcTab;
    if ( pSrcDoc->GetTable( aTabName, nSrcTab ) )
    {
        if ( !InsertTab( SC_TAB_APPEND, aDocTab, TRUE ) )
        {
            DBG_ERRORFILE("can't insert external document table");
            return FALSE;
        }
        rTab = GetTableCount() - 1;
        // nicht neu einfuegen, nur Ergebnisse
        TransferTab( pSrcDoc, nSrcTab, rTab, FALSE, TRUE );
    }
    else
        return FALSE;

    BOOL bWasThere = HasLink( aFileName, aFilterName, aOptions );
    SetLink( rTab, SC_LINK_VALUE, aFileName, aFilterName, aOptions, aTabName );
    if ( !bWasThere )       // Link pro Quelldokument nur einmal eintragen
    {
        ScTableLink* pLink = new ScTableLink( pShell, aFileName, aFilterName, aOptions );
        pLink->SetInCreate( TRUE );
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName,
                                        &aFilterName );
        pLink->Update();
        pLink->SetInCreate( FALSE );
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }
    return TRUE;
}

ScOutlineTable* ScDocument::GetOutlineTable( USHORT nTab, BOOL bCreate )
{
    ScOutlineTable* pVal = NULL;

    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            pVal = pTab[nTab]->GetOutlineTable();
            if (!pVal)
                if (bCreate)
                {
                    pTab[nTab]->StartOutlineTable();
                    pVal = pTab[nTab]->GetOutlineTable();
                }
        }

    return pVal;
}

BOOL ScDocument::SetOutlineTable( USHORT nTab, const ScOutlineTable* pNewOutline )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->SetOutlineTable(pNewOutline);

    return FALSE;
}

void ScDocument::DoAutoOutline( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );
}

BOOL ScDocument::TestRemoveSubTotals( USHORT nTab, const ScSubTotalParam& rParam )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->TestRemoveSubTotals( rParam );

    return FALSE;
}

void ScDocument::RemoveSubTotals( USHORT nTab, ScSubTotalParam& rParam )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->RemoveSubTotals( rParam );
}

BOOL ScDocument::DoSubTotals( USHORT nTab, ScSubTotalParam& rParam )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->DoSubTotals( rParam );

    return FALSE;
}

BOOL ScDocument::HasSubTotalCells( const ScRange& rRange )
{
    ScCellIterator aIter( this, rRange );
    ScBaseCell* pCell = aIter.GetFirst();
    while (pCell)
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->IsSubTotal() )
            return TRUE;

        pCell = aIter.GetNext();
    }
    return FALSE;   // none found
}

//  kopiert aus diesem Dokument die Zellen von Positionen, an denen in pPosDoc
//  auch Zellen stehen, nach pDestDoc

void ScDocument::CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc )
{
    USHORT nCount = GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
        if (pTab[nTab] && pPosDoc->pTab[nTab] && pDestDoc->pTab[nTab])
            pTab[nTab]->CopyUpdated( pPosDoc->pTab[nTab], pDestDoc->pTab[nTab] );
}

void ScDocument::CopyScenario( USHORT nSrcTab, USHORT nDestTab, BOOL bNewScenario )
{
    if (nSrcTab<=MAXTAB && nDestTab<=MAXTAB && pTab[nSrcTab] && pTab[nDestTab])
    {
        //  Flags fuer aktive Szenarios richtig setzen
        //  und aktuelle Werte in bisher aktive Szenarios zurueckschreiben

        ScRangeList aRanges = *pTab[nSrcTab]->GetScenarioRanges();
        USHORT nRangeCount = (USHORT)aRanges.Count();

        //  nDestTab ist die Zieltabelle
        for ( USHORT nTab = nDestTab+1;
                nTab<=MAXTAB && pTab[nTab] && pTab[nTab]->IsScenario();
                nTab++ )
        {
            if ( pTab[nTab]->IsActiveScenario() )       // auch wenn's dasselbe Szenario ist
            {
                BOOL bTouched = FALSE;
                for ( USHORT nR=0; nR<nRangeCount && !bTouched; nR++)
                {
                    ScRange aRange = *aRanges.GetObject(nR);
                    if ( pTab[nTab]->HasScenarioRange( aRange ) )
                        bTouched = TRUE;
                }
                if (bTouched)
                {
                    pTab[nTab]->SetActiveScenario(FALSE);
                    if ( pTab[nTab]->GetScenarioFlags() & SC_SCENARIO_TWOWAY )
                        pTab[nTab]->CopyScenarioFrom( pTab[nDestTab] );
                }
            }
        }

        pTab[nSrcTab]->SetActiveScenario(TRUE);     // da kommt's her...
        if (!bNewScenario)                          // Daten aus dem ausgewaehlten Szenario kopieren
        {
            BOOL bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
            pTab[nSrcTab]->CopyScenarioTo( pTab[nDestTab] );
            SetDirty();
            SetAutoCalc( bOldAutoCalc );
        }
    }
}

void ScDocument::MarkScenario( USHORT nSrcTab, USHORT nDestTab, ScMarkData& rDestMark,
                                BOOL bResetMark, USHORT nNeededBits ) const
{
    if (bResetMark)
        rDestMark.ResetMark();

    if (nSrcTab<=MAXTAB && pTab[nSrcTab])
        pTab[nSrcTab]->MarkScenarioIn( rDestMark, nNeededBits );

    rDestMark.SetAreaTab( nDestTab );
}

BOOL ScDocument::HasScenarioRange( USHORT nTab, const ScRange& rRange ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->HasScenarioRange( rRange );

    return FALSE;
}

const ScRangeList* ScDocument::GetScenarioRanges( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetScenarioRanges();

    return NULL;
}

BOOL ScDocument::IsActiveScenario( USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->IsActiveScenario();

    return FALSE;
}

void ScDocument::SetActiveScenario( USHORT nTab, BOOL bActive )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->SetActiveScenario( bActive );
}

BOOL ScDocument::TestCopyScenario( USHORT nSrcTab, USHORT nDestTab ) const
{
    if (nSrcTab<=MAXTAB && nDestTab<=MAXTAB)
        return pTab[nSrcTab]->TestCopyScenarioTo( pTab[nDestTab] );

    DBG_ERROR("falsche Tabelle bei TestCopyScenario");
    return FALSE;
}

void ScDocument::AddUnoObject( SfxListener& rObject )
{
    if (!pUnoBroadcaster)
        pUnoBroadcaster = new SfxBroadcaster;

    rObject.StartListening( *pUnoBroadcaster );
}

void ScDocument::RemoveUnoObject( SfxListener& rObject )
{
    if (pUnoBroadcaster)
        rObject.EndListening( *pUnoBroadcaster );
    else
        DBG_ERROR("kein Uno-Broadcaster??!?");
}

void ScDocument::BroadcastUno( const SfxHint &rHint )
{
    if (pUnoBroadcaster)
        pUnoBroadcaster->Broadcast( rHint );
}

void ScDocument::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz,
                                    ScDocument* pUndoDoc, BOOL bIncludeDraw )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        BOOL bExpandRefsOld = IsExpandRefs();
        if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
            SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
        USHORT i;
        USHORT iMax;
        if ( eUpdateRefMode == URM_COPY )
        {
            i = nTab1;
            iMax = nTab2;
        }
        else
        {
            ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            xColNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
            xRowNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
            pDBCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
            pRangeName->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if (pPivotCollection)
                pPivotCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
            if ( pDPCollection )
                pDPCollection->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            UpdateChartRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
            UpdateRefAreaLinks( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pCondFormList )
                pCondFormList->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pDetOpList )
                pDetOpList->UpdateReference( this, eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pUnoBroadcaster )
                pUnoBroadcaster->Broadcast( ScUpdateRefHint(
                                    eUpdateRefMode, aRange, nDx, nDy, nDz ) );
            i = 0;
            iMax = MAXTAB;
        }
        for ( ; i<=iMax; i++)
            if (pTab[i])
                pTab[i]->UpdateReference(
                    eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                    nDx, nDy, nDz, pUndoDoc, bIncludeDraw );

        if ( bIsEmbedded )
        {
            USHORT theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
            theCol1 = aEmbedRange.aStart.Col();
            theRow1 = aEmbedRange.aStart.Row();
            theTab1 = aEmbedRange.aStart.Tab();
            theCol2 = aEmbedRange.aEnd.Col();
            theRow2 = aEmbedRange.aEnd.Row();
            theTab2 = aEmbedRange.aEnd.Tab();
            if ( ScRefUpdate::Update( this, eUpdateRefMode, nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                        nDx,nDy,nDz, theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
            {
                aEmbedRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
            }
        }
        SetExpandRefs( bExpandRefsOld );

        // #30428# after moving, no clipboard move ref-updates are possible
        if ( eUpdateRefMode != URM_COPY && IsClipboardSource() )
        {
            ScDocument* pClipDoc = SC_MOD()->GetClipDoc();
            if (pClipDoc)
                pClipDoc->bCutMode = FALSE;
        }
    }
}

void ScDocument::UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc )
{
    DBG_ASSERT(pClipDoc->bIsClip, "UpdateTranspose: kein Clip");

    ScRange aSource = pClipDoc->aClipRange;         // Tab wird noch angepasst
    ScAddress aDest = rDestPos;

    USHORT nClipTab = 0;
    for (USHORT nDestTab=0; nDestTab<=MAXTAB && pTab[nDestTab]; nDestTab++)
        if (rMark.GetTableSelect(nDestTab))
        {
            while (!pClipDoc->pTab[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
            aSource.aStart.SetTab( nClipTab );
            aSource.aEnd.SetTab( nClipTab );
            aDest.SetTab( nDestTab );

            //  wie UpdateReference

            pRangeName->UpdateTranspose( aSource, aDest );      // vor den Zellen!
            for (USHORT i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->UpdateTranspose( aSource, aDest, pUndoDoc );

            nClipTab = (nClipTab+1) % (MAXTAB+1);
        }
}

void ScDocument::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    //! pDBCollection
    //! pPivotCollection
    //! UpdateChartRef

    pRangeName->UpdateGrow( rArea, nGrowX, nGrowY );
    pPivotCollection->UpdateGrow( rArea, nGrowX, nGrowY );

    for (USHORT i=0; i<=MAXTAB && pTab[i]; i++)
        pTab[i]->UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScDocument::Fill(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, const ScMarkData& rMark,
                        USHORT nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->Fill(nCol1, nRow1, nCol2, nRow2,
                                nFillCount, eFillDir, eFillCmd, eFillDateCmd,
                                nStepValue, nMaxValue);
}

String ScDocument::GetAutoFillPreview( const ScRange& rSource, USHORT nEndX, USHORT nEndY )
{
    USHORT nTab = rSource.aStart.Tab();
    if (pTab[nTab])
        return pTab[nTab]->GetAutoFillPreview( rSource, nEndX, nEndY );

    return EMPTY_STRING;
}

void ScDocument::AutoFormat( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nFormatNo, const ScMarkData& rMark )
{
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    for (USHORT i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo );
}

void ScDocument::GetAutoFormatData(USHORT nTab, USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    ScAutoFormatData& rData)
{
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            PutInOrder(nStartCol, nEndCol);
            PutInOrder(nStartRow, nEndRow);
            pTab[nTab]->GetAutoFormatData(nStartCol, nStartRow, nEndCol, nEndRow, rData);
        }
    }
}

// static
void ScDocument::GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
        USHORT& rCol, USHORT& rRow )
{
    USHORT nCommand = rSearchItem.GetCommand();
    BOOL bReplace = ( nCommand == SVX_SEARCHCMD_REPLACE ||
        nCommand == SVX_SEARCHCMD_REPLACE_ALL );
    if ( rSearchItem.GetBackward() )
    {
        if ( rSearchItem.GetRowDirection() )
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MAXCOL;
                rRow = MAXROW+1;
            }
            else if ( bReplace )
            {
                rCol = MAXCOL;
                rRow = MAXROW;
            }
            else
            {
                rCol = MAXCOL+1;
                rRow = MAXROW;
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MAXCOL+1;
                rRow = MAXROW;
            }
            else if ( bReplace )
            {
                rCol = MAXCOL;
                rRow = MAXROW;
            }
            else
            {
                rCol = MAXCOL;
                rRow = MAXROW+1;
            }
        }
    }
    else
    {
        if ( rSearchItem.GetRowDirection() )
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = 0;
                rRow = (USHORT) -1;
            }
            else if ( bReplace )
            {
                rCol = 0;
                rRow = 0;
            }
            else
            {
                rCol = (USHORT) -1;
                rRow = 0;
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = (USHORT) -1;
                rRow = 0;
            }
            else if ( bReplace )
            {
                rCol = 0;
                rRow = 0;
            }
            else
            {
                rCol = 0;
                rRow = (USHORT) -1;
            }
        }
    }
}

BOOL ScDocument::SearchAndReplace(const SvxSearchItem& rSearchItem,
                                USHORT& rCol, USHORT& rRow, USHORT& rTab,
                                ScMarkData& rMark,
                                String& rUndoStr, ScDocument* pUndoDoc)
{
    //!     getrennte Markierungen pro Tabelle verwalten !!!!!!!!!!!!!

    rMark.MarkToMulti();

    BOOL bFound = FALSE;
    if (VALIDTAB(rTab))
    {
        USHORT nCol;
        USHORT nRow;
        USHORT nTab;
        USHORT nCommand = rSearchItem.GetCommand();
        if ( nCommand == SVX_SEARCHCMD_FIND_ALL ||
             nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            for (nTab = 0; nTab <= MAXTAB; nTab++)
                if (pTab[nTab])
                {
                    if (rMark.GetTableSelect(nTab))
                    {
                        nCol = 0;
                        nRow = 0;
                        bFound |= pTab[nTab]->SearchAndReplace(
                                    rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc );
                    }
                }

            //  Markierung wird innen schon komplett gesetzt
        }
        else
        {
            nCol = rCol;
            nRow = rRow;
            if (rSearchItem.GetBackward())
            {
                for (nTab = rTab; ((short)nTab >= 0) && !bFound; nTab--)
                    if (pTab[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = pTab[nTab]->SearchAndReplace(
                                        rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc );
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );
                        }
                    }
            }
            else
            {
                for (nTab = rTab; (nTab <= MAXTAB) && !bFound; nTab++)
                    if (pTab[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = pTab[nTab]->SearchAndReplace(
                                        rSearchItem, nCol, nRow, rMark, rUndoStr, pUndoDoc );
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );
                        }
                    }
            }
        }
    }
    return bFound;
}

BOOL ScDocument::IsFiltered( USHORT nRow, USHORT nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsFiltered( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}

//  Outline anpassen

BOOL ScDocument::UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, USHORT nTab, BOOL bShow )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->UpdateOutlineCol( nStartCol, nEndCol, bShow );

    DBG_ERROR("missing tab");
    return FALSE;
}

BOOL ScDocument::UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bShow )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->UpdateOutlineRow( nStartRow, nEndRow, bShow );

    DBG_ERROR("missing tab");
    return FALSE;
}

void ScDocument::Sort(USHORT nTab, const ScSortParam& rSortParam, BOOL bKeepQuery)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
    {
        BOOL bOldDisableIdle = IsIdleDisabled();
        DisableIdle( TRUE );
        pTab[nTab]->Sort(rSortParam, bKeepQuery);
        DisableIdle( bOldDisableIdle );
    }
}

USHORT ScDocument::Query(USHORT nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->Query((ScQueryParam&)rQueryParam, bKeepSub);

    DBG_ERROR("missing tab");
    return 0;
}


BOOL ScDocument::ValidQuery( USHORT nRow, USHORT nTab, const ScQueryParam& rQueryParam )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->ValidQuery( nRow, rQueryParam );

    DBG_ERROR("missing tab");
    return FALSE;
}


void ScDocument::GetUpperCellString(USHORT nCol, USHORT nRow, USHORT nTab, String& rStr)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        pTab[nTab]->GetUpperCellString( nCol, nRow, rStr );
    else
        rStr.Erase();
}

BOOL ScDocument::CreateQueryParam(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nTab, ScQueryParam& rQueryParam)
{
    if ( nTab<=MAXTAB && pTab[nTab] )
        return pTab[nTab]->CreateQueryParam(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    DBG_ERROR("missing tab");
    return FALSE;
}

BOOL ScDocument::HasAutoFilter( USHORT nCurCol, USHORT nCurRow, USHORT nCurTab )
{
    ScDBData*       pDBData         = GetDBAtCursor( nCurCol, nCurRow, nCurTab );
    BOOL            bHasAutoFilter  = ( pDBData != NULL );

    if ( pDBData )
    {
        if ( pDBData->HasHeader() )
        {
            USHORT nCol;
            USHORT nRow;
            INT16  nFlag;

            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );
            nRow = aParam.nRow1;

            for ( nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAutoFilter; nCol++ )
            {
                nFlag = ((ScMergeFlagAttr*)
                            GetAttr( nCol, nRow, nCurTab, ATTR_MERGE_FLAG ))->
                                GetValue();

                if ( (nFlag & SC_MF_AUTO) == 0 )
                    bHasAutoFilter = FALSE;
            }
        }
        else
            bHasAutoFilter = FALSE;
    }

    return bHasAutoFilter;
}

BOOL ScDocument::HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nTab )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->HasColHeader( nStartCol, nStartRow, nEndCol, nEndRow );

    return FALSE;
}

BOOL ScDocument::HasRowHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nTab )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->HasRowHeader( nStartCol, nStartRow, nEndCol, nEndRow );

    return FALSE;
}

//
//  GetFilterEntries - Eintraege fuer AutoFilter-Listbox
//

BOOL ScDocument::GetFilterEntries( USHORT nCol, USHORT nRow, USHORT nTab, TypedStrCollection& rStrings )
{
    if ( nTab<=MAXTAB && pTab[nTab] && pDBCollection )
    {
        ScDBData* pDBData = pDBCollection->GetDBAtCursor(nCol, nRow, nTab, FALSE);  //!??
        if (pDBData)
        {
            USHORT nAreaTab;
            USHORT nStartCol;
            USHORT nStartRow;
            USHORT nEndCol;
            USHORT nEndRow;
            pDBData->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
            if (pDBData->HasHeader())
                ++nStartRow;

            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );
            rStrings.SetCaseSensitive( aParam.bCaseSens );

            pTab[nTab]->GetFilterEntries( nCol, nStartRow, nEndRow, rStrings );
            return TRUE;
        }
    }

    return FALSE;
}

//
//  GetFilterEntriesArea - Eintraege fuer Filter-Dialog
//

BOOL ScDocument::GetFilterEntriesArea( USHORT nCol, USHORT nStartRow, USHORT nEndRow,
                                        USHORT nTab, TypedStrCollection& rStrings )
{
    if ( nTab<=MAXTAB && pTab[nTab] )
    {
        pTab[nTab]->GetFilterEntries( nCol, nStartRow, nEndRow, rStrings );
        return TRUE;
    }

    return FALSE;
}

//
//  GetDataEntries - Eintraege fuer Auswahlliste-Listbox (keine Zahlen / Formeln)
//

BOOL ScDocument::GetDataEntries( USHORT nCol, USHORT nRow, USHORT nTab,
                                    TypedStrCollection& rStrings, BOOL bLimit )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetDataEntries( nCol, nRow, rStrings, bLimit );

    return FALSE;
}

//
//  GetFormulaEntries - Eintraege fuer Formel-AutoEingabe
//

//  Funktionen werden als 1 schon vom InputHandler eingefuegt
#define SC_STRTYPE_NAMES        2
#define SC_STRTYPE_DBNAMES      3
#define SC_STRTYPE_HEADERS      4

BOOL ScDocument::GetFormulaEntries( TypedStrCollection& rStrings )
{
    USHORT i;

    //
    //  Bereichsnamen
    //

    if ( pRangeName )
    {
        USHORT nRangeCount = pRangeName->GetCount();
        for ( i=0; i<nRangeCount; i++ )
        {
            ScRangeData* pData = (*pRangeName)[i];
            if (pData)
            {
                TypedStrData* pNew = new TypedStrData( pData->GetName(), 0.0, SC_STRTYPE_NAMES );
                if ( !rStrings.Insert(pNew) )
                    delete pNew;
            }
        }
    }

    //
    //  Datenbank-Bereiche
    //

    if ( pDBCollection )
    {
        USHORT nDBCount = pDBCollection->GetCount();
        for ( i=0; i<nDBCount; i++ )
        {
            ScDBData* pData = (*pDBCollection)[i];
            if (pData)
            {
                TypedStrData* pNew = new TypedStrData( pData->GetName(), 0.0, SC_STRTYPE_DBNAMES );
                if ( !rStrings.Insert(pNew) )
                    delete pNew;
            }
        }
    }

    //
    //  Inhalte von Beschriftungsbereichen
    //

    ScRangePairList* pLists[2];
    pLists[0] = GetColNameRanges();
    pLists[1] = GetRowNameRanges();
    for (USHORT nListNo=0; nListNo<2; nListNo++)
    {
        ScRangePairList* pList = pLists[nListNo];
        if (pList)
            for ( ScRangePair* pPair = pList->First(); pPair; pPair = pList->Next() )
            {
                ScRange aRange = pPair->GetRange(0);
                ScCellIterator aIter( this, aRange );
                for ( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
                    if ( pCell->HasStringData() )
                    {
                        String aStr = pCell->GetStringData();
                        TypedStrData* pNew = new TypedStrData( aStr, 0.0, SC_STRTYPE_HEADERS );
                        if ( !rStrings.Insert(pNew) )
                            delete pNew;
                    }
            }
    }

    return TRUE;
}


BOOL ScDocument::IsEmbedded() const
{
    return bIsEmbedded;
}

void ScDocument::GetEmbedded( ScTripel& rStart, ScTripel& rEnd ) const
{
    rStart.Put( aEmbedRange.aStart.Col(), aEmbedRange.aStart.Row(), aEmbedRange.aStart.Tab() );
    rEnd.Put( aEmbedRange.aEnd.Col(), aEmbedRange.aEnd.Row(), aEmbedRange.aEnd.Tab() );
}

Rectangle ScDocument::GetEmbeddedRect() const                       // 1/100 mm
{
    Rectangle aRect;
    ScTable* pTable = pTab[aEmbedRange.aStart.Tab()];
    if (!pTable)
    {
        DBG_ERROR("GetEmbeddedRect ohne Tabelle");
    }
    else
    {
        USHORT i;

        for (i=0; i<aEmbedRange.aStart.Col(); i++)
            aRect.Left() += pTable->GetColWidth(i);
        for (i=0; i<aEmbedRange.aStart.Row(); i++)
            aRect.Top() += pTable->GetRowHeight(i);
        aRect.Right() = aRect.Left();
        for (i=aEmbedRange.aStart.Col(); i<=aEmbedRange.aEnd.Col(); i++)
            aRect.Right() += pTable->GetColWidth(i);
        aRect.Bottom() = aRect.Top();
        for (i=aEmbedRange.aStart.Row(); i<=aEmbedRange.aEnd.Row(); i++)
            aRect.Bottom() += pTable->GetRowHeight(i);

        aRect.Left()   = (long) ( aRect.Left()   * HMM_PER_TWIPS );
        aRect.Right()  = (long) ( aRect.Right()  * HMM_PER_TWIPS );
        aRect.Top()    = (long) ( aRect.Top()    * HMM_PER_TWIPS );
        aRect.Bottom() = (long) ( aRect.Bottom() * HMM_PER_TWIPS );
    }
    return aRect;
}

void ScDocument::SetEmbedded( const ScTripel& rStart, const ScTripel& rEnd )
{
    bIsEmbedded = TRUE;
    aEmbedRange = ScRange( rStart, rEnd );
}

void ScDocument::ResetEmbedded()
{
    bIsEmbedded = FALSE;
    aEmbedRange = ScRange();
}

ScRange ScDocument::GetRange( USHORT nTab, const Rectangle& rMMRect )
{
    ScTable* pTable = pTab[nTab];
    if (!pTable)
    {
        DBG_ERROR("GetRange ohne Tabelle");
        return ScRange();
    }

    long nSize;
    long nTwips;
    long nAdd;
    BOOL bEnd;

    nSize = 0;
    nTwips = (long) (rMMRect.Left() / HMM_PER_TWIPS);

    USHORT nX1 = 0;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetColWidth(nX1);
        if (nSize+nAdd <= nTwips+1 && nX1<MAXCOL)
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = TRUE;
    }

    nTwips = (long) (rMMRect.Right() / HMM_PER_TWIPS);

    USHORT nX2 = nX1;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetColWidth(nX2);
        if (nSize+nAdd < nTwips && nX2<MAXCOL)
        {
            nSize += nAdd;
            ++nX2;
        }
        else
            bEnd = TRUE;
    }


    nSize = 0;
    nTwips = (long) (rMMRect.Top() / HMM_PER_TWIPS);

    USHORT nY1 = 0;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetRowHeight(nY1);
        if (nSize+nAdd <= nTwips+1 && nY1<MAXROW)
        {
            nSize += nAdd;
            ++nY1;
        }
        else
            bEnd = TRUE;
    }

    nTwips = (long) (rMMRect.Bottom() / HMM_PER_TWIPS);

    USHORT nY2 = nY1;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetRowHeight(nY2);
        if (nSize+nAdd < nTwips && nY2<MAXROW)
        {
            nSize += nAdd;
            ++nY2;
        }
        else
            bEnd = TRUE;
    }

    return ScRange( nX1,nY1,nTab, nX2,nY2,nTab );
}

void ScDocument::SetEmbedded( const Rectangle& rRect )          // aus VisArea (1/100 mm)
{
    bIsEmbedded = TRUE;
    aEmbedRange = GetRange( nVisibleTab, rRect );
}

//  VisArea auf Zellgrenzen anpassen

void lcl_SnapHor( ScTable* pTable, long& rVal, USHORT& rStartCol )
{
    USHORT nCol = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;
    while ( nCol<MAXCOL )
    {
        long nAdd = pTable->GetColWidth(nCol);
        if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
        {
            nSnap += nAdd;
            ++nCol;
        }
        else
            break;
    }
    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartCol = nCol;
}

void lcl_SnapVer( ScTable* pTable, long& rVal, USHORT& rStartRow )
{
    USHORT nRow = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;
    while ( nRow<MAXROW )
    {
        long nAdd = pTable->GetRowHeight(nRow);
        if ( nSnap + nAdd/2 < nTwips || nRow < rStartRow )
        {
            nSnap += nAdd;
            ++nRow;
        }
        else
            break;
    }
    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartRow = nRow;
}

void ScDocument::SnapVisArea( Rectangle& rRect ) const
{
    ScTable* pTable = pTab[nVisibleTab];
    if (!pTable)
    {
        DBG_ERROR("SetEmbedded ohne Tabelle");
        return;
    }

    USHORT nCol = 0;
    lcl_SnapHor( pTable, rRect.Left(), nCol );
    ++nCol;                                         // mindestens eine Spalte
    lcl_SnapHor( pTable, rRect.Right(), nCol );

    USHORT nRow = 0;
    lcl_SnapVer( pTable, rRect.Top(), nRow );
    ++nRow;                                         // mindestens eine Zeile
    lcl_SnapVer( pTable, rRect.Bottom(), nRow );
}

void ScDocument::SetDocProtection( BOOL bProtect, const uno::Sequence<sal_Int8>& rPasswd )
{
    bProtected = bProtect;
    aProtectPass = rPasswd;
}

void ScDocument::SetTabProtection( USHORT nTab, BOOL bProtect, const uno::Sequence<sal_Int8>& rPasswd )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetProtection( bProtect, rPasswd );
}

BOOL ScDocument::IsDocProtected() const
{
    return bProtected;
}

BOOL ScDocument::IsDocEditable() const
{
    // import into read-only document is possible - must be extended if other filters use api

    return !bProtected && ( !pShell || !pShell->IsReadOnly() || bImportingXML );
}

BOOL ScDocument::IsTabProtected( USHORT nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsProtected();

    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}

const uno::Sequence<sal_Int8>& ScDocument::GetDocPassword() const
{
    return aProtectPass;
}

const uno::Sequence<sal_Int8>& ScDocument::GetTabPassword( USHORT nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetPassword();

    DBG_ERROR("Falsche Tabellennummer");
    return aProtectPass;
}

const ScDocOptions& ScDocument::GetDocOptions() const
{
    DBG_ASSERT( pDocOptions, "No DocOptions! :-(" );
    return *pDocOptions;
}

void ScDocument::SetDocOptions( const ScDocOptions& rOpt )
{
    USHORT d,m,y;

    DBG_ASSERT( pDocOptions, "No DocOptions! :-(" );
    *pDocOptions = rOpt;
    rOpt.GetDate( d,m,y );

    SvNumberFormatter* pFormatter = xPoolHelper->GetFormTable();
    pFormatter->ChangeNullDate( d,m,y );
    pFormatter->ChangeStandardPrec( (USHORT)rOpt.GetStdPrecision() );
    pFormatter->SetYear2000( rOpt.GetYear2000() );
}

const ScViewOptions& ScDocument::GetViewOptions() const
{
    DBG_ASSERT( pViewOptions, "No ViewOptions! :-(" );
    return *pViewOptions;
}

void ScDocument::SetViewOptions( const ScViewOptions& rOpt )
{
    DBG_ASSERT( pViewOptions, "No ViewOptions! :-(" );
    *pViewOptions = rOpt;
}

void ScDocument::GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const
{
    rLatin = eLanguage;
    rCjk = eCjkLanguage;
    rCtl = eCtlLanguage;
}

void ScDocument::SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl )
{
    eLanguage = eLatin;
    eCjkLanguage = eCjk;
    eCtlLanguage = eCtl;
    if ( xPoolHelper.isValid() )
    {
        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        pPool->SetPoolDefaultItem( SvxLanguageItem( eLanguage, ATTR_FONT_LANGUAGE ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, ATTR_CJK_FONT_LANGUAGE ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, ATTR_CTL_FONT_LANGUAGE ) );
    }
}

Rectangle ScDocument::GetMMRect( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab )
{
    if (nTab > MAXTAB || !pTab[nTab])
    {
        DBG_ERROR("GetMMRect: falsche Tabelle");
        return Rectangle(0,0,0,0);
    }

    USHORT i;
    Rectangle aRect;

    for (i=0; i<nStartCol; i++)
        aRect.Left() += GetColWidth(i,nTab);
    for (i=0; i<nStartRow; i++)
        aRect.Top() += FastGetRowHeight(i,nTab);

    aRect.Right()  = aRect.Left();
    aRect.Bottom() = aRect.Top();

    for (i=nStartCol; i<=nEndCol; i++)
        aRect.Right() += GetColWidth(i,nTab);
    for (i=nStartRow; i<=nEndRow; i++)
        aRect.Bottom() += FastGetRowHeight(i,nTab);

    aRect.Left()    = (long)(aRect.Left()   * HMM_PER_TWIPS);
    aRect.Right()   = (long)(aRect.Right()  * HMM_PER_TWIPS);
    aRect.Top()     = (long)(aRect.Top()    * HMM_PER_TWIPS);
    aRect.Bottom()  = (long)(aRect.Bottom() * HMM_PER_TWIPS);

    return aRect;
}

void ScDocument::SetExtDocOptions( ScExtDocOptions* pNewOptions )
{
    delete pExtDocOptions;
    pExtDocOptions = pNewOptions;
}

void ScDocument::DoMergeContents( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow )
{
    String aEmpty;
    String aTotal;
    String aCellStr;
    USHORT nCol;
    USHORT nRow;
    ScPostIt aCellNote;
    String aNoteStr;
    BOOL bDoNote = FALSE;

    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
        for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            GetString(nCol,nRow,nTab,aCellStr);
            if (aCellStr.Len())
            {
                if (aTotal.Len())
                    aTotal += ' ';
                aTotal += aCellStr;
            }
            if (nCol != nStartCol || nRow != nStartRow)
                SetString(nCol,nRow,nTab,aEmpty);

            if (GetNote(nCol,nRow,nTab,aCellNote))
            {
                if (aNoteStr.Len())
                    aNoteStr += '\n';
                aNoteStr += aCellNote.GetText();

                if (nCol != nStartCol || nRow != nStartRow)
                {
                    if (aCellNote.IsShown())
                        ScDetectiveFunc( this, nTab ).HideComment( nCol, nRow );
                    SetNote(nCol,nRow,nTab,ScPostIt());
                    bDoNote = TRUE;
                }
                //! Autor/Datum beibehalten, wenn's nur eine Notiz war??
            }
        }

    SetString(nStartCol,nStartRow,nTab,aTotal);
    if (bDoNote)
        SetNote(nStartCol,nStartRow,nTab,ScPostIt(aNoteStr));
}

void ScDocument::DoMerge( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow )
{
    ScMergeAttr aAttr( nEndCol-nStartCol+1, nEndRow-nStartRow+1 );
    ApplyAttr( nStartCol, nStartRow, nTab, aAttr );

    if ( nEndCol > nStartCol )
        ApplyFlagsTab( nStartCol+1, nStartRow, nEndCol, nStartRow, nTab, SC_MF_HOR );
    if ( nEndRow > nStartRow )
        ApplyFlagsTab( nStartCol, nStartRow+1, nStartCol, nEndRow, nTab, SC_MF_VER );
    if ( nEndCol > nStartCol && nEndRow > nStartRow )
        ApplyFlagsTab( nStartCol+1, nStartRow+1, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );
}

void ScDocument::RemoveMerge( USHORT nCol, USHORT nRow, USHORT nTab )
{
    const ScMergeAttr* pAttr = (const ScMergeAttr*)
                                    GetAttr( nCol, nRow, nTab, ATTR_MERGE );

    if ( pAttr->GetColMerge() <= 1 && pAttr->GetRowMerge() <= 1 )
        return;

    USHORT nEndCol = nCol + pAttr->GetColMerge() - 1;
    USHORT nEndRow = nRow + pAttr->GetRowMerge() - 1;

    RemoveFlagsTab( nCol, nRow, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );

    const ScMergeAttr* pDefAttr = (const ScMergeAttr*)
                                        &xPoolHelper->GetDocPool()->GetDefaultItem( ATTR_MERGE );
    ApplyAttr( nCol, nRow, nTab, *pDefAttr );
}

void ScDocument::ExtendPrintArea( OutputDevice* pDev, USHORT nTab,
                    USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->ExtendPrintArea( pDev, nStartCol, nStartRow, rEndCol, nEndRow );
}

void ScDocument::IncSizeRecalcLevel( USHORT nTab )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->IncRecalcLevel();
}

void ScDocument::DecSizeRecalcLevel( USHORT nTab )
{
    if ( nTab <= MAXTAB && pTab[nTab] )
        pTab[nTab]->DecRecalcLevel();
}




