/*************************************************************************
 *
 *  $RCSfile: docsh5.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-09 10:26:00 $
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

#define _BASEDLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
//#define _COLOR_HXX
//#define _CTRLBOX_HXX
//#define _CTRLTOOL_HXX
#define _DIALOGS_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
//#define _FIELD_HXX
#define _FILDLG_HXX
//#define _FILTER_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
//#define _GRAPH_HXX
//#define _GDIMTF_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
//#define _MDIFRM_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
//#define _MAPMOD_HXX
#define _OUTLINER_HXX
//#define _PAL_HXX
#define _PASSWD_HXX
//#define _PRNDLG_HXX   //
#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
//#define _SELENG_HXX
#define _SETBRW_HXX
//#define _SOUND_HXX
#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _SYSDLG_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
//#define _TREELIST_HXX
//#define _VALUESET_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCATTR_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
//#define _VIEWFAC_HXX


#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
//#define _SFX_SAVEOPT_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPP_HXX
#define _SFXBASIC_HXX
//#define _SFXCTRLITEM_HXX
//#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
//#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX
#define _SFX_TEMPLDLG_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW
#define _SI_NOCONTROL

#define _SV_NOXSOUND
#define _SVDATTR_HXX
#define _SVDETC_HXX
#define _SVDIO_HXX
#define _SVDRAG_HXX
#define _SVDLAYER_HXX
#define _SVDXOUT_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX


#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop


#include "scitems.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/smplhint.hxx>


// INCLUDE ---------------------------------------------------------------

#include "docsh.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "undodat.hxx"
#include "undotab.hxx"
#include "undoblk.hxx"
//#include "pivot.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "consoli.hxx"
#include "dbcolect.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "docpool.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "waitoff.hxx"

// ---------------------------------------------------------------------------

//
//  ehemalige viewfunc/dbfunc Methoden
//

void ScDocShell::ErrorMessage( USHORT nGlobStrId )
{
    //! StopMarking an der (aktiven) View?

    Window* pParent = GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    BOOL bFocus = pParent && pParent->HasFocus();

    if(nGlobStrId==STR_PROTECTIONERR)
    {
        if(IsReadOnly())
        {
            nGlobStrId=STR_READONLYERR;
        }
    }

    InfoBox aBox( pParent, ScGlobal::GetRscString( nGlobStrId ) );
    aBox.Execute();
    if (bFocus)
        pParent->GrabFocus();
}

BOOL ScDocShell::IsEditable() const
{
    // import into read-only document is possible - must be extended if other filters use api

    return !IsReadOnly() || aDocument.IsImportingXML();
}

void ScDocShell::DBAreaDeleted( USHORT nTab, USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    ScDocShellModificator aModificator( *this );
    aDocument.RemoveFlagsTab( nX1, nY1, nX2, nY1, nTab, SC_MF_AUTO );
    PostPaint( nX1, nY1, nTab, nX2, nY1, nTab, PAINT_GRID );
    aModificator.SetDocumentModified();
}

ScDBData* lcl_GetDBNearCursor( ScDBCollection* pColl, USHORT nCol, USHORT nRow, USHORT nTab )
{
    //! nach document/dbcolect verschieben

    if (!pColl)
        return NULL;

    ScDBData* pNoNameData = NULL;
    ScDBData* pNearData = NULL;
    USHORT nCount = pColl->GetCount();
    String aNoName = ScGlobal::GetRscString( STR_DB_NONAME );
    USHORT nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow;
    for (USHORT i = 0; i < nCount; i++)
    {
        ScDBData* pDB = (*pColl)[i];
        pDB->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab && nCol+1 >= nStartCol && nCol <= nEndCol+1 &&
                                 nRow+1 >= nStartRow && nRow <= nEndRow+1 )
        {
            if ( pDB->GetName() == aNoName )
                pNoNameData = pDB;
            else if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if (!pNearData)
                    pNearData = pDB;    // ersten angrenzenden Bereich merken
            }
            else
                return pDB;             // nicht "unbenannt" und Cursor steht wirklich drin
        }
    }
    if (pNearData)
        return pNearData;               // angrenzender, wenn nichts direkt getroffen
    return pNoNameData;                 // "unbenannt" nur zurueck, wenn sonst nichts gefunden
}

ScDBData* ScDocShell::GetDBData( const ScRange& rMarked, ScGetDBMode eMode, BOOL bForceMark )
{
    USHORT nCol = rMarked.aStart.Col();
    USHORT nRow = rMarked.aStart.Row();
    USHORT nTab = rMarked.aStart.Tab();

    USHORT nStartCol = nCol;
    USHORT nStartRow = nRow;
    USHORT nStartTab = nTab;
    USHORT nEndCol = rMarked.aEnd.Col();
    USHORT nEndRow = rMarked.aEnd.Row();
    USHORT nEndTab = rMarked.aEnd.Tab();

    //  Wegen #49655# nicht einfach GetDBAtCursor: Der zusammenhaengende Datenbereich
    //  fuer "unbenannt" (GetDataArea) kann neben dem Cursor legen, also muss auch ein
    //  benannter DB-Bereich dort gesucht werden.

    ScDBData* pData = aDocument.GetDBAtArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
    if (!pData)
        pData = lcl_GetDBNearCursor( aDocument.GetDBCollection(), nCol, nRow, nTab );

    BOOL bSelected = ( bForceMark || rMarked.aStart != rMarked.aEnd );

    BOOL bUseThis = FALSE;
    if (pData)
    {
        //      Bereich nehmen, wenn nichts anderes markiert

        USHORT nDummy;
        USHORT nOldCol1;
        USHORT nOldRow1;
        USHORT nOldCol2;
        USHORT nOldRow2;
        pData->GetArea( nDummy, nOldCol1,nOldRow1, nOldCol2,nOldRow2 );
        BOOL bIsNoName = ( pData->GetName() == ScGlobal::GetRscString( STR_DB_NONAME ) );

        if (!bSelected)
        {
            bUseThis = TRUE;
            if ( bIsNoName && eMode == SC_DB_MAKE )
            {
                //  wenn nichts markiert, "unbenannt" auf zusammenhaengenden Bereich anpassen
                nStartCol = nCol;
                nStartRow = nRow;
                nEndCol = nStartCol;
                nEndRow = nStartRow;
                aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE );
                if ( nOldCol1 != nStartCol || nOldCol2 != nEndCol || nOldRow1 != nStartRow )
                    bUseThis = FALSE;               // passt gar nicht
                else if ( nOldRow2 != nEndRow )
                {
                    //  Bereich auf neue End-Zeile erweitern
                    pData->SetArea( nTab, nOldCol1,nOldRow1, nOldCol2,nEndRow );
                }
            }
        }
        else
        {
            if ( nOldCol1 == nStartCol && nOldRow1 == nStartRow &&
                 nOldCol2 == nEndCol && nOldRow2 == nEndRow )               // genau markiert?
                bUseThis = TRUE;
            else
                bUseThis = FALSE;           // immer Markierung nehmen (Bug 11964)
        }

        //      fuer Import nie "unbenannt" nehmen

        if ( bUseThis && eMode == SC_DB_IMPORT && bIsNoName )
            bUseThis = FALSE;
    }

    if ( bUseThis )
    {
        pData->GetArea( nStartTab, nStartCol,nStartRow, nEndCol,nEndRow );
        nEndTab = nStartTab;
    }
    else if ( eMode == SC_DB_OLD )
    {
        pData = NULL;                           // nichts gefunden
        nStartCol = nEndCol = nCol;
        nStartRow = nEndRow = nRow;
        nStartTab = nEndTab = nTab;
//      bMark = FALSE;                          // nichts zu markieren
    }
    else
    {
        if ( bSelected )
        {
//          bMark = FALSE;
        }
        else
        {                                       // zusammenhaengender Bereich
            nStartCol = nCol;
            nStartRow = nRow;
            nEndCol = nStartCol;
            nEndRow = nStartRow;
            aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE );
        }

        BOOL bHasHeader = aDocument.HasColHeader( nStartCol,nStartRow, nEndCol,nEndRow, nTab );

        ScDBData* pNoNameData;
        USHORT nNoNameIndex;
        ScDBCollection* pColl = aDocument.GetDBCollection();
        if ( eMode != SC_DB_IMPORT &&
                pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            pNoNameData = (*pColl)[nNoNameIndex];

            USHORT nOldX1;                                  // alten Bereich sauber wegnehmen
            USHORT nOldY1;                                  //! (UNDO ???)
            USHORT nOldX2;
            USHORT nOldY2;
            USHORT nOldTab;
            pNoNameData->GetArea( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );
            DBAreaDeleted( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );

            pNoNameData->SetSortParam( ScSortParam() );             // Parameter zuruecksetzen
            pNoNameData->SetQueryParam( ScQueryParam() );
            pNoNameData->SetSubTotalParam( ScSubTotalParam() );

            pNoNameData->SetArea( nTab, nStartCol,nStartRow, nEndCol,nEndRow );     // neu setzen
            pNoNameData->SetByRow( TRUE );
            pNoNameData->SetHeader( bHasHeader );
            pNoNameData->SetAutoFilter( FALSE );
        }
        else
        {
            ScDBCollection* pUndoColl = NULL;

            String aNewName;
            if (eMode==SC_DB_IMPORT)
            {
                aDocument.CompileDBFormula( TRUE );         // CreateFormulaString
                pUndoColl = new ScDBCollection( *pColl );   // Undo fuer Import1-Bereich

                String aImport = ScGlobal::GetRscString( STR_DBNAME_IMPORT );
                long nCount = 0;
                USHORT nDummy;
                do
                {
                    ++nCount;
                    aNewName = aImport;
                    aNewName += String::CreateFromInt32( nCount );
                }
                while (pColl->SearchName( aNewName, nDummy ));
            }
            else
                aNewName = ScGlobal::GetRscString( STR_DB_NONAME );
            pNoNameData = new ScDBData( aNewName, nTab,
                                nStartCol,nStartRow, nEndCol,nEndRow,
                                TRUE, bHasHeader );
            pColl->Insert( pNoNameData );

            if ( pUndoColl )
            {
                aDocument.CompileDBFormula( FALSE );        // CompileFormulaString

                ScDBCollection* pRedoColl = new ScDBCollection( *pColl );
                GetUndoManager()->AddUndoAction( new ScUndoDBData( this, pUndoColl, pRedoColl ) );
            }

            //  neuen Bereich am Sba anmelden nicht mehr noetig

            //  "Import1" etc am Navigator bekanntmachen
            if (eMode==SC_DB_IMPORT)
                SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        }
        pData = pNoNameData;
    }

//  if (bMark)
//      MarkRange( ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab ), FALSE );

    return pData;
}


        //  Hoehen anpassen
        //! mit docfunc zusammenfassen

BOOL ScDocShell::AdjustRowHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab )
{
    VirtualDevice aVDev;
    Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aLogic.X() / 1000.0;
    double nPPTY = aLogic.Y() / 1000.0;
    nPPTX /= GetOutputFactor();         // noetig fuer Bildschirm/VDev
    Fraction aZoom(1,1);
    BOOL bChange = aDocument.SetOptimalHeight( nStartRow,nEndRow, nTab, 0, &aVDev,
                                                nPPTX,nPPTY, aZoom,aZoom, FALSE );
    if (bChange)
        PostPaint( 0,nStartRow,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID|PAINT_LEFT );

    return bChange;
}

void ScDocShell::PivotUpdate( ScPivot* pOldPivot, ScPivot* pNewPivot, BOOL bRecord, BOOL bApi )
{
    DBG_ERRORFILE("PivotUpdate is obsolete!");
}

void ScDocShell::RefreshPivotTables( const ScRange& rSource )
{
    //! rename to RefreshDataPilotTables?

    ScDPCollection* pColl = aDocument.GetDPCollection();
    if ( pColl )
    {
        //  DataPilotUpdate doesn't modify the collection order like PivotUpdate did,
        //  so a simple loop can be used.

        USHORT nCount = pColl->GetCount();
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDPObject* pOld = (*pColl)[i];
            if ( pOld )
            {
                const ScSheetSourceDesc* pSheetDesc = pOld->GetSheetDesc();
                if ( pSheetDesc && pSheetDesc->aSourceRange.Intersects( rSource ) )
                {
                    ScDPObject* pNew = new ScDPObject( *pOld );
                    ScDBDocFunc aFunc( *this );
                    aFunc.DataPilotUpdate( pOld, pNew, TRUE, FALSE );
                    delete pNew;    // DataPilotUpdate copies settings from "new" object
                }
            }
        }
    }
}

String lcl_GetAreaName( ScDocument* pDoc, ScArea* pArea )
{
    String aName;
    BOOL bOk = FALSE;
    ScDBData* pData = pDoc->GetDBAtArea( pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                        pArea->nColEnd, pArea->nRowEnd );
    if (pData)
    {
        pData->GetName( aName );
        if ( aName != ScGlobal::GetRscString( STR_DB_NONAME ) )
            bOk = TRUE;
    }

    if (!bOk)
        pDoc->GetName( pArea->nTab, aName );

    return aName;
}

void ScDocShell::DoConsolidate( const ScConsolidateParam& rParam, BOOL bRecord )
{
    ScConsData aData;

    USHORT nPos;
    USHORT nColSize = 0;
    USHORT nRowSize = 0;
    BOOL bErr = FALSE;
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea* pArea = rParam.ppDataAreas[nPos];
        nColSize = Max( nColSize, USHORT( pArea->nColEnd - pArea->nColStart + 1 ) );
        nRowSize = Max( nRowSize, USHORT( pArea->nRowEnd - pArea->nRowStart + 1 ) );

                                        // Test, ob Quelldaten verschoben wuerden
        if (rParam.bReferenceData)
            if (pArea->nTab == rParam.nTab && pArea->nRowEnd >= rParam.nRow)
                bErr = TRUE;
    }

    if (bErr)
    {
        InfoBox aBox( GetDialogParent(),
                ScGlobal::GetRscString( STR_CONSOLIDATE_ERR1 ) );
        aBox.Execute();
        return;
    }

    //      ausfuehren

    WaitObject aWait( GetDialogParent() );
    ScDocShellModificator aModificator( *this );

    ScRange aOldDest;
    ScDBData* pDestData = aDocument.GetDBAtCursor( rParam.nCol, rParam.nRow, rParam.nTab, TRUE );
    if (pDestData)
        pDestData->GetArea(aOldDest);

    aData.SetSize( nColSize, nRowSize );
    aData.SetFlags( rParam.eFunction, rParam.bByCol, rParam.bByRow, rParam.bReferenceData );
    if ( rParam.bByCol || rParam.bByRow )
        for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
        {
            ScArea* pArea = rParam.ppDataAreas[nPos];
            aData.AddFields( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                        pArea->nColEnd, pArea->nRowEnd );
        }
    aData.DoneFields();
    for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
    {
        ScArea* pArea = rParam.ppDataAreas[nPos];
        aData.AddData( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
                                                    pArea->nColEnd, pArea->nRowEnd );
        aData.AddName( lcl_GetAreaName(&aDocument,pArea) );
    }

    aData.GetSize( nColSize, nRowSize );
    if (bRecord && nColSize && nRowSize)
    {
        ScDBData* pUndoData = pDestData ? new ScDBData(*pDestData) : NULL;

        USHORT nDestTab = rParam.nTab;
        ScArea aDestArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol+nColSize-1, rParam.nRow+nRowSize-1 );
        if (rParam.bByCol) ++aDestArea.nColEnd;
        if (rParam.bByRow) ++aDestArea.nRowEnd;

        if (rParam.bReferenceData)
        {
            USHORT nTabCount = aDocument.GetTableCount();
            USHORT nInsertCount = aData.GetInsertCount();

            // alte Outlines
            ScOutlineTable* pTable = aDocument.GetOutlineTable( nDestTab );
            ScOutlineTable* pUndoTab = pTable ? new ScOutlineTable( *pTable ) : NULL;

            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( &aDocument, 0, nTabCount-1, FALSE, TRUE );

            // Zeilenstatus
            aDocument.CopyToDocument( 0,0,nDestTab, MAXCOL,MAXROW,nDestTab,
                                    IDF_NONE, FALSE, pUndoDoc );

            // alle Formeln
            aDocument.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        IDF_FORMULA, FALSE, pUndoDoc );

            // komplette Ausgangszeilen
            aDocument.CopyToDocument( 0,aDestArea.nRowStart,nDestTab,
                                    MAXCOL,aDestArea.nRowEnd,nDestTab,
                                    IDF_ALL, FALSE, pUndoDoc );

            // alten Ausgabebereich
            if (pDestData)
                aDocument.CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );

            GetUndoManager()->AddUndoAction(
                    new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
                                            TRUE, nInsertCount, pUndoTab, pUndoData ) );
        }
        else
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( &aDocument, aDestArea.nTab, aDestArea.nTab );

            aDocument.CopyToDocument( aDestArea.nColStart, aDestArea.nRowStart, aDestArea.nTab,
                                    aDestArea.nColEnd, aDestArea.nRowEnd, aDestArea.nTab,
                                    IDF_ALL, FALSE, pUndoDoc );

            // alten Ausgabebereich
            if (pDestData)
                aDocument.CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );

            GetUndoManager()->AddUndoAction(
                    new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
                                            FALSE, 0, NULL, pUndoData ) );
        }
    }

    if (pDestData)                                      // Zielbereich loeschen / anpassen
    {
        aDocument.DeleteAreaTab(aOldDest, IDF_CONTENTS);
        pDestData->SetArea( rParam.nTab, rParam.nCol, rParam.nRow,
                            rParam.nCol + nColSize - 1, rParam.nRow + nRowSize - 1 );
        pDestData->SetHeader( rParam.bByRow );
    }

    aData.OutputToDocument( &aDocument, rParam.nCol, rParam.nRow, rParam.nTab );

    USHORT nPaintStartCol = rParam.nCol;
    USHORT nPaintStartRow = rParam.nRow;
    USHORT nPaintEndCol = nPaintStartCol + nColSize - 1;
    USHORT nPaintEndRow = nPaintStartRow + nRowSize - 1;
    USHORT nPaintFlags = PAINT_GRID;
    if (rParam.bByCol)
        ++nPaintEndRow;
    if (rParam.bByRow)
        ++nPaintEndCol;
    if (rParam.bReferenceData)
    {
        nPaintStartCol = 0;
        nPaintEndCol = MAXCOL;
        nPaintEndRow = MAXROW;
        nPaintFlags |= PAINT_LEFT | PAINT_SIZE;
    }
    if (pDestData)
    {
        if ( aOldDest.aEnd.Col() > nPaintEndCol )
            nPaintEndCol = aOldDest.aEnd.Col();
        if ( aOldDest.aEnd.Row() > nPaintEndRow )
            nPaintEndRow = aOldDest.aEnd.Row();
    }
    PostPaint( nPaintStartCol, nPaintStartRow, rParam.nTab,
                nPaintEndCol, nPaintEndRow, rParam.nTab, nPaintFlags );
    aModificator.SetDocumentModified();
}

void ScDocShell::UseScenario( USHORT nTab, const String& rName, BOOL bRecord )
{
    if (!aDocument.IsScenario(nTab))
    {
        USHORT  nTabCount = aDocument.GetTableCount();
        USHORT  nSrcTab = USHRT_MAX;
        USHORT  nEndTab = nTab;
        String aCompare;
        while ( nEndTab+1 < nTabCount && aDocument.IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            if (nSrcTab > MAXTAB)           // noch auf der Suche nach dem Szenario?
            {
                aDocument.GetName( nEndTab, aCompare );
                if (aCompare == rName)
                    nSrcTab = nEndTab;      // gefunden
            }
        }
        if (nSrcTab <= MAXTAB)
        {
            if ( aDocument.TestCopyScenario( nSrcTab, nTab ) )          // Zellschutz testen
            {
                ScDocShellModificator aModificator( *this );
                ScMarkData aScenMark;
                aDocument.MarkScenario( nSrcTab, nTab, aScenMark );
                ScRange aMultiRange;
                aScenMark.GetMultiMarkArea( aMultiRange );
                USHORT nStartCol = aMultiRange.aStart.Col();
                USHORT nStartRow = aMultiRange.aStart.Row();
                USHORT nEndCol = aMultiRange.aEnd.Col();
                USHORT nEndRow = aMultiRange.aEnd.Row();

                if (bRecord)
                {
                    ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pUndoDoc->InitUndo( &aDocument, nTab,nEndTab );             // auch alle Szenarien
                    //  angezeigte Tabelle:
                    aDocument.CopyToDocument( nStartCol,nStartRow,nTab,
                                    nEndCol,nEndRow,nTab, IDF_ALL,TRUE, pUndoDoc, &aScenMark );
                    //  Szenarien
                    for (USHORT i=nTab+1; i<=nEndTab; i++)
                    {
                        pUndoDoc->SetScenario( i, TRUE );
                        String aComment;
                        Color  aColor;
                        USHORT nScenFlags;
                        aDocument.GetScenarioData( i, aComment, aColor, nScenFlags );
                        pUndoDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
                        BOOL bActive = aDocument.IsActiveScenario( i );
                        pUndoDoc->SetActiveScenario( i, bActive );
                        //  Bei Zurueckkopier-Szenarios auch Inhalte
                        if ( nScenFlags & SC_SCENARIO_TWOWAY )
                            aDocument.CopyToDocument( 0,0,i, MAXCOL,MAXROW,i,
                                                        IDF_ALL,FALSE, pUndoDoc );
                    }

                    GetUndoManager()->AddUndoAction(
                        new ScUndoUseScenario( this, aScenMark,
                                        ScArea( nTab,nStartCol,nStartRow,nEndCol,nEndRow ),
                                        pUndoDoc, rName ) );
                }

                aDocument.CopyScenario( nSrcTab, nTab );
                aDocument.SetDirty();

                //  alles painten, weil in anderen Bereichen das aktive Szenario
                //  geaendert sein kann
                //! nur, wenn sichtbare Rahmen vorhanden?
                PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
                aModificator.SetDocumentModified();
            }
            else
            {
                InfoBox aBox(GetDialogParent(),
                    ScGlobal::GetRscString( STR_PROTECTIONERR ) );
                aBox.Execute();
            }
        }
        else
        {
            InfoBox aBox(GetDialogParent(),
                ScGlobal::GetRscString( STR_SCENARIO_NOTFOUND ) );
            aBox.Execute();
        }
    }
    else
        DBG_ERROR( "UseScenario auf Szenario-Blatt" );
}

void ScDocShell::ModifyScenario( USHORT nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags )
{
    //  Undo
    String aOldName;
    aDocument.GetName( nTab, aOldName );
    String aOldComment;
    Color aOldColor;
    USHORT nOldFlags;
    aDocument.GetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );
    GetUndoManager()->AddUndoAction(
        new ScUndoScenarioFlags( this, nTab,
                aOldName, rName, aOldComment, rComment,
                aOldColor, rColor, nOldFlags, nFlags ) );

    //  ausfuehren
    ScDocShellModificator aModificator( *this );
    aDocument.RenameTab( nTab, rName );
    aDocument.SetScenarioData( nTab, rComment, rColor, nFlags );
    PostPaintGridAll();
    aModificator.SetDocumentModified();

    if ( rName != aOldName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_SELECT_SCENARIO );
}

USHORT ScDocShell::MakeScenario( USHORT nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags,
                                    ScMarkData& rMark, BOOL bRecord )
{
    rMark.MarkToMulti();
    if (rMark.IsMultiMarked())
    {
        USHORT nNewTab = nTab + 1;
        while (aDocument.IsScenario(nNewTab))
            ++nNewTab;

        BOOL bCopyAll = ( (nFlags & SC_SCENARIO_COPYALL) != 0 );
        const ScMarkData* pCopyMark = NULL;
        if (!bCopyAll)
            pCopyMark = &rMark;

        ScDocShellModificator aModificator( *this );

        if (aDocument.CopyTab( nTab, nNewTab, pCopyMark ))
        {
            if (bRecord)
            {
                ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( &aDocument, nTab,nTab );
                GetUndoManager()->AddUndoAction(
                        new ScUndoMakeScenario( this, nTab, nNewTab, pUndoDoc,
                                                rName, rComment, rColor, nFlags, rMark ));
            }

            aDocument.RenameTab( nNewTab, rName, FALSE );           // ohne Formel-Update
            aDocument.SetScenario( nNewTab, TRUE );
            aDocument.SetScenarioData( nNewTab, rComment, rColor, nFlags );

            ScMarkData aDestMark = rMark;
            aDestMark.SelectOneTable( nNewTab );

            //!     auf Filter / Buttons / Merging testen !

            ScPatternAttr aProtPattern( aDocument.GetPool() );
            aProtPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
            aDocument.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nNewTab, aProtPattern );

            ScPatternAttr aPattern( aDocument.GetPool() );
            aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
            aPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
            aDocument.ApplySelectionPattern( aPattern, aDestMark );

            if (!bCopyAll)
                aDocument.SetVisible( nNewTab, FALSE );

            //  dies ist dann das aktive Szenario
            aDocument.CopyScenario( nNewTab, nTab, TRUE );  // TRUE - nicht aus Szenario kopieren

            if (nFlags & SC_SCENARIO_SHOWFRAME)
                PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );  // Rahmen painten
            PostPaintExtras();                                          // Tabellenreiter
            aModificator.SetDocumentModified();

            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

            return nNewTab;
        }
    }
    return nTab;
}

BOOL ScDocShell::MoveTable( USHORT nSrcTab, USHORT nDestTab, BOOL bCopy, BOOL bRecord )
{
    ScDocShellModificator aModificator( *this );

    if (bCopy)
    {
        if (bRecord)
            aDocument.BeginDrawUndo();          // drawing layer must do its own undo actions

        if (!aDocument.CopyTab( nSrcTab, nDestTab ))
        {
            //! EndDrawUndo?
            return FALSE;
        }
        else if (bRecord)
        {
            SvUShorts aSrcList;
            SvUShorts aDestList;
            aSrcList.Insert(nSrcTab,0);
            aDestList.Insert(nDestTab,0);
            GetUndoManager()->AddUndoAction(
                    new ScUndoCopyTab( this, aSrcList, aDestList ) );
        }

        Broadcast( ScTablesHint( SC_TAB_COPIED, nSrcTab, nDestTab ) );
    }
    else
    {
        if ( aDocument.GetChangeTrack() )
            return FALSE;

        if ( nSrcTab<nDestTab && nDestTab!=SC_TAB_APPEND )
            nDestTab--;

        if ( nSrcTab == nDestTab )
        {
            //! allow only for api calls?
            return TRUE;    // nothing to do, but valid
        }

        if (!aDocument.MoveTab( nSrcTab, nDestTab ))
            return FALSE;
        else if (bRecord)
        {
            SvUShorts aSrcList;
            SvUShorts aDestList;
            aSrcList.Insert(nSrcTab,0);
            aDestList.Insert(nDestTab,0);
            GetUndoManager()->AddUndoAction(
                    new ScUndoMoveTab( this, aSrcList, aDestList ) );
        }

        Broadcast( ScTablesHint( SC_TAB_MOVED, nSrcTab, nDestTab ) );
    }

    PostPaintGridAll();
    PostPaintExtras();
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    return TRUE;
}



