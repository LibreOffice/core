/*************************************************************************
 *
 *  $RCSfile: docsh4.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-28 19:30:31 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop


// INCLUDE ---------------------------------------------------------------
#if STLPORT_VERSION>=321
#include <math.h>       // prevent conflict between exception and std::exception
#endif

#include "scitems.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sbxcore.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/topfrm.hxx>
#include <offmgr/ofaitem.hxx>
#include <offmgr/sbaitems.hxx>
#include <sot/formats.hxx>
#include <svtools/printdlg.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <tools/multisel.hxx>
#include <svx/drawitem.hxx>
#include <svx/fmview.hxx>
#include <svx/pageitem.hxx>
#include <sch/schdll.hxx>
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif


#include "docsh.hxx"
#include "docfunc.hxx"
#include "sc.hrc"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "appoptio.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "styledlg.hxx"
#include "hfedtdlg.hxx"
#include "dbdocfun.hxx"
#include "printfun.hxx"              // DrawToDev
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "dataobj.hxx"
#include "impex.hxx"
#include "attrib.hxx"
#include "corodlg.hxx"
#include "undodat.hxx"
#include "autostyl.hxx"
#include "undocell.hxx"
#include "undotab.hxx"
#include "inputhdl.hxx"
#include "dbcolect.hxx"
#include "servobj.hxx"
#include "rangenam.hxx"
#include "scmod.hxx"
#include "scendlg.hxx"
#include "chgviset.hxx"
#include "reffact.hxx"
#include "chartlis.hxx"
#include "waitoff.hxx"
#include "tablink.hxx"      // ScDocumentLoader statics
#include "drwlayer.hxx"
#include "docoptio.hxx"
#include "undostyl.hxx"

//------------------------------------------------------------------

#define IS_SHARE_HEADER(set) \
    ((SfxBoolItem&) \
        ((SvxSetItem&)(set).Get(ATTR_PAGE_HEADERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

#define IS_SHARE_FOOTER(set) \
    ((SfxBoolItem&) \
        ((SvxSetItem&)(set).Get(ATTR_PAGE_FOOTERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

#define SC_PREVIEW_SIZE_X   10000
#define SC_PREVIEW_SIZE_Y   12400


//------------------------------------------------------------------

BOOL ScDocShell::GetTabParam( const SfxItemSet* pArgs, USHORT nId, USHORT& rTab )
{
    BOOL bOk = TRUE;
    rTab = 0;                   // zur Sicherheit...

    const SfxPoolItem* pTabItem;
    if ( pArgs && pArgs->GetItemState( nId, TRUE, &pTabItem ) == SFX_ITEM_SET )
    {
        //  Parameter sind 1-based !!!
        USHORT nItemTab = ((const SfxUInt16Item*)pTabItem)->GetValue();
        USHORT nTabCount = aDocument.GetTableCount();
        if (nItemTab>0 && nItemTab<=nTabCount)
            rTab = nItemTab-1;
        else
            bOk = FALSE;
    }
    else                        // nichts angegeben -> sichtbare
    {
        ScTabViewShell* pVisibleSh = GetBestViewShell();
        if ( pVisibleSh )
            rTab = pVisibleSh->GetViewData()->GetTabNo();
        else
            bOk = FALSE;
    }

    return bOk;
}

//------------------------------------------------------------------

void ScDocShell::Execute( SfxRequest& rReq )
{
    //  SID_SC_RANGE (Range),
    //  SID_SC_CELLTEXT (CellText),
    //  SID_SC_CELLS (Cells) - removed (old Basic)

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    SfxBindings* pBindings = GetViewBindings();
    BOOL bUndo (aDocument.IsUndoEnabled());

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_SC_SETTEXT:
        {
            const SfxPoolItem* pColItem;
            const SfxPoolItem* pRowItem;
            const SfxPoolItem* pTabItem;
            const SfxPoolItem* pTextItem;
            if( pReqArgs && IS_AVAILABLE( FN_PARAM_1, &pColItem ) &&
                            IS_AVAILABLE( FN_PARAM_2, &pRowItem ) &&
                            IS_AVAILABLE( FN_PARAM_3, &pTabItem ) &&
                            IS_AVAILABLE( SID_SC_SETTEXT, &pTextItem ) )
            {
                //  Parameter sind 1-based !!!
                USHORT nCol = ((SfxUInt16Item*)pColItem)->GetValue() - 1;
                USHORT nRow = ((SfxUInt16Item*)pRowItem)->GetValue() - 1;
                USHORT nTab = ((SfxUInt16Item*)pTabItem)->GetValue() - 1;

                USHORT nTabCount = aDocument.GetTableCount();
                if ( nCol <= MAXCOL && nRow <= MAXROW && nTab < nTabCount )
                {
                    if ( aDocument.IsBlockEditable( nTab, nCol,nRow, nCol, nRow ) )
                    {
                        String aVal = ((const SfxStringItem*)pTextItem)->GetValue();
                        aDocument.SetString( nCol, nRow, nTab, aVal );

                        PostPaintCell( nCol, nRow, nTab );
                        SetDocumentModified();

                        rReq.Done();
                        break;
                    }
                    else                // geschuetzte Zelle
                    {
                        SbxBase::SetError( SbxERR_BAD_PARAMETER );      //! welchen Fehler ?
                        break;
                    }
                }
            }
            SbxBase::SetError( SbxERR_NO_OBJECT );
        }
        break;


        //  SID_SBA_QRY_CHANGETARGET gibts nicht mehr - auch in idl raus

        case SID_SBA_IMPORT:
        {
            if (pReqArgs)
            {
                const sal_Unicode cSbaSep = 11;     // Trennzeichen

                const SfxPoolItem* pItem;
                String sSbaData, sTarget;
                if ( pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET )
                    sSbaData = ((const SfxStringItem*)pItem)->GetValue();
                if ( pReqArgs->GetItemState( FN_PARAM_1, TRUE, &pItem ) == SFX_ITEM_SET )
                    sTarget = ((const SfxStringItem*)pItem)->GetValue();

                BOOL bIsNewArea = TRUE;         // Default TRUE (keine Nachfrage)
                if ( pReqArgs->GetItemState( FN_PARAM_2, TRUE, &pItem ) == SFX_ITEM_SET )
                    bIsNewArea = ((const SfxBoolItem*)pItem)->GetValue();

                String sDBName  = sSbaData.GetToken(0,cSbaSep);     // Datenbankname
                String sDBTable = sSbaData.GetToken(1,cSbaSep);     // Tabellen- oder Query-Name
                String sTabFlag = sSbaData.GetToken(2,cSbaSep);
                String sDBSql   = sSbaData.GetToken(3,cSbaSep);     // SQL im Klartext

                BYTE nType = ScDbTable;     // "0" oder "1"
                if ( sTabFlag.EqualsAscii("0") )        // "0" = Query, "1" = Table (Default)
                    nType = ScDbQuery;

                SbaSelectionListRef pSelectionList = new SbaSelectionList;
                xub_StrLen nCount = sSbaData.GetTokenCount(cSbaSep);

                for (xub_StrLen i = 4; i < nCount; i++)
                {
                    String aSelItem = sSbaData.GetToken(i,cSbaSep);
                    if (aSelItem.Len())
                    {
                        void *pPtr = (void*)aSelItem.ToInt32();
                        pSelectionList->Insert( pPtr, LIST_APPEND );
                    }
                }

                // bei Bedarf neuen Datenbankbereich anlegen
                BOOL bMakeArea = FALSE;
                if (bIsNewArea)
                {
                    ScDBCollection* pDBColl = aDocument.GetDBCollection();
                    USHORT nDummy;
                    if ( !pDBColl || !pDBColl->SearchName( sTarget, nDummy ) )
                    {
                        ScAddress aPos;
                        if ( aPos.Parse( sTarget, &aDocument ) & SCA_VALID )
                        {
                            bMakeArea = TRUE;
                            if (bUndo)
                            {
                                String aStrImport = ScGlobal::GetRscString( STR_UNDO_IMPORTDATA );
                                GetUndoManager()->EnterListAction( aStrImport, aStrImport );
                            }

                            ScDBData* pDBData = GetDBData( ScRange(aPos), SC_DB_IMPORT, FALSE );
                            DBG_ASSERT(pDBData, "kann DB-Daten nicht anlegen");
                            sTarget = pDBData->GetName();
                        }
                    }
                }

                // nachfragen, bevor alter DB-Bereich ueberschrieben wird
                BOOL bDo = TRUE;
                if (!bIsNewArea)
                {
                    String aTemplate = ScGlobal::GetRscString( STR_IMPORT_REPLACE );
                    String aMessage = aTemplate.GetToken( 0, '#' );
                    aMessage += sTarget;
                    aMessage += aTemplate.GetToken( 1, '#' );

                    QueryBox aBox( 0, WinBits(WB_YES_NO | WB_DEF_YES), aMessage );
                    bDo = ( aBox.Execute() == RET_YES );
                }

                if (bDo)
                {
                    ScDBDocFunc(*this).
                        UpdateImport( sTarget, sDBName, sDBTable, sDBSql,
                                        TRUE, nType, pSelectionList );
                    rReq.Done();

                    //  UpdateImport aktualisiert auch die internen Operationen
                }
                else
                    rReq.Ignore();

                if ( bMakeArea && bUndo)
                    GetUndoManager()->LeaveListAction();
            }
            else
                DBG_ERROR( "arguments expected" );
        }
        break;

        case SID_CHART_SOURCE:
        case SID_CHART_ADDSOURCE:
            if (pReqArgs)
            {
                ScDocument* pDoc = GetDocument();
                BOOL bUndo (pDoc->IsUndoEnabled());
                const   SfxPoolItem* pItem;
                String  aChartName, aRangeName;

                ScRange         aSingleRange;
                ScRangeListRef  aRangeListRef;
                BOOL            bMultiRange = FALSE;

                BOOL bColHeaders = TRUE;
                BOOL bRowHeaders = TRUE;
                BOOL bColInit = FALSE;
                BOOL bRowInit = FALSE;
                BOOL bAddRange = (nSlot == SID_CHART_ADDSOURCE);

                if( IS_AVAILABLE( SID_CHART_NAME, &pItem ) )
                    aChartName = ((const SfxStringItem*)pItem)->GetValue();

                if( IS_AVAILABLE( SID_CHART_SOURCE, &pItem ) )
                    aRangeName = ((const SfxStringItem*)pItem)->GetValue();

                if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                {
                    bColHeaders = ((const SfxBoolItem*)pItem)->GetValue();
                    bColInit = TRUE;
                }
                if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
                {
                    bRowHeaders = ((const SfxBoolItem*)pItem)->GetValue();
                    bRowInit = TRUE;
                }

                BOOL bValid = ( aSingleRange.ParseAny( aRangeName, pDoc ) & SCA_VALID ) != 0;
                if (!bValid)
                {
                    aRangeListRef = new ScRangeList;
                    aRangeListRef->Parse( aRangeName, pDoc );
                    if ( aRangeListRef->Count() )
                    {
                        bMultiRange = TRUE;
                        aSingleRange = *aRangeListRef->GetObject(0);    // fuer Header
                        bValid = TRUE;
                    }
                    else
                        aRangeListRef.Clear();
                }

                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                if (pViewSh && bValid && aChartName.Len() != 0 )
                {
                    Window* pParent = pViewSh->GetDialogParent();
                    Window* pDataWin = pViewSh->GetActiveWin();

                    USHORT nCol1 = aSingleRange.aStart.Col();
                    USHORT nRow1 = aSingleRange.aStart.Row();
                    USHORT nCol2 = aSingleRange.aEnd.Col();
                    USHORT nRow2 = aSingleRange.aEnd.Row();
                    USHORT nTab = aSingleRange.aStart.Tab();

                    //! immer oder gar nicht begrenzen ???
                    if (!bMultiRange)
                        aDocument.LimitChartArea( nTab, nCol1,nRow1, nCol2,nRow2 );

                                        // Dialog fuer Spalten/Zeilenkoepfe
                    BOOL bOk = TRUE;
                    if ( !bAddRange && ( !bColInit || !bRowInit ) )
                    {
                                                // Spalten/Zeilenkoepfe testen wie in chartarr
                        USHORT i;
                        if (!bColInit)
                            for (i=nCol1; i<=nCol2 && bColHeaders; i++)
                                if (aDocument.HasValueData( i, nRow1, nTab ))
                                    bColHeaders = FALSE;
                        if (!bRowInit)
                            for (i=nRow1; i<=nRow2 && bRowHeaders; i++)
                                if (aDocument.HasValueData( nCol1, i, nTab ))
                                    bRowHeaders = FALSE;

                        ScColRowLabelDlg aDlg( pParent, bRowHeaders, bColHeaders );
                        if ( aDlg.Execute() == RET_OK )
                        {
                            bColHeaders = aDlg.IsRow();     // Spaltenkoepfe = 1. Zeile
                            bRowHeaders = aDlg.IsCol();

                            rReq.AppendItem(SfxBoolItem(FN_PARAM_1, bColHeaders));
                            rReq.AppendItem(SfxBoolItem(FN_PARAM_2, bRowHeaders));
                        }
                        else
                            bOk = FALSE;
                    }

                    if (bOk)            // ausfuehren
                    {
                        if (bMultiRange)
                        {
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aRangeListRef,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            aDocument.UpdateChartArea( aChartName, aRangeListRef,
                                                        bColHeaders, bRowHeaders, bAddRange,
                                                        pDataWin );
                        }
                        else
                        {
                            ScRange aNewRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aNewRange,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            aDocument.UpdateChartArea( aChartName, aNewRange,
                                                        bColHeaders, bRowHeaders, bAddRange,
                                                        pDataWin );
                        }
                    }
                }
                else
                    DBG_ERROR("UpdateChartArea: keine ViewShell oder falsche Daten");
                rReq.Done();
            }
            else
                DBG_ERROR("SID_CHART_SOURCE ohne Argumente");
            break;

        case FID_AUTO_CALC:
            {
                BOOL bNewVal;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( nSlot, TRUE, &pItem ) )
                    bNewVal = ((const SfxBoolItem*)pItem)->GetValue();
                else
                    bNewVal = !aDocument.GetAutoCalc();     // Toggle fuer Menue
                aDocument.SetAutoCalc( bNewVal );
                SetDocumentModified();
                if (pBindings)
                {
                    pBindings->Invalidate( FID_AUTO_CALC );
//                  pBindings->Invalidate( FID_RECALC );        // jetzt immer enabled
                }
                rReq.Done();
            }
            break;
        case FID_RECALC:
            DoRecalc( rReq.IsAPI() );
            rReq.Done();
            break;
        case FID_HARD_RECALC:
            DoHardRecalc( rReq.IsAPI() );
            rReq.Done();
            break;
        case SID_UPDATETABLINKS:
            {
                ScDocument* pDoc = GetDocument();

                ScLkUpdMode nSet=pDoc->GetLinkMode();

                USHORT nDlgRet=RET_NO;
                if(nSet==LM_UNKNOWN)
                {
                    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
                    nSet=aAppOptions.GetLinkMode();
                }

                if(nSet==LM_ON_DEMAND)
                {
                    QueryBox aBox( GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                             ScGlobal::GetRscString(STR_RELOAD_TABLES) );

                    nDlgRet=aBox.Execute();
                }

                if (nDlgRet == RET_YES || nSet==LM_ALWAYS)
                {
                    ReloadTabLinks();
                    aDocument.UpdateDdeLinks();
                    aDocument.UpdateAreaLinks();

                    //! Test, ob Fehler
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_REIMPORT_AFTER_LOAD:
            {
                //  wird nach dem Laden aufgerufen, wenn DB-Bereiche mit
                //  weggelassenen Daten enthalten sind

                BOOL bDone = FALSE;
                ScRange aRange;
                ScDBCollection* pDBColl = aDocument.GetDBCollection();
                ScTabViewShell* pViewSh = GetBestViewShell();
                DBG_ASSERT(pViewSh,"SID_REIMPORT_AFTER_LOAD: keine View");
                if (pViewSh && pDBColl)
                {
                    QueryBox aBox( GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                             ScGlobal::GetRscString(STR_REIMPORT_AFTER_LOAD) );
                    if (aBox.Execute() == RET_YES)
                    {
                        for (USHORT i=0; i<pDBColl->GetCount(); i++)
                        {
                            ScDBData* pDBData = (*pDBColl)[i];
                            if ( pDBData->IsStripData() &&
                                    pDBData->HasImportParam() && !pDBData->HasImportSelection() )
                            {
                                pDBData->GetArea(aRange);
                                pViewSh->MarkRange(aRange);

                                //  Import und interne Operationen wie SID_REFRESH_DBAREA
                                //  (Abfrage auf Import hier nicht noetig)

                                ScImportParam aImportParam;
                                pDBData->GetImportParam( aImportParam );
                                BOOL bContinue = pViewSh->ImportData( aImportParam );
                                pDBData->SetImportParam( aImportParam );

                                //  markieren (Groesse kann sich geaendert haben)
                                pDBData->GetArea(aRange);
                                pViewSh->MarkRange(aRange);

                                if ( bContinue )    // #41905# Fehler beim Import -> Abbruch
                                {
                                    //  interne Operationen, wenn welche gespeichert

                                    if ( pDBData->HasQueryParam() || pDBData->HasSortParam() ||
                                                                      pDBData->HasSubTotalParam() )
                                        pViewSh->RepeatDB();

                                    //  Pivottabellen die den Bereich als Quelldaten haben

                                    RefreshPivotTables(aRange);
                                }
                            }
                        }
                        bDone = TRUE;
                    }
                }

                if ( !bDone && pDBColl )
                {
                    //  wenn nicht, dann aber die abhaengigen Formeln updaten
                    //! auch fuer einzelne Bereiche, die nicht aktualisiert werden koennen

                    aDocument.CalcAll();        //! nur die abhaengigen
                    PostDataChanged();
                }

                if (bDone)
                    rReq.Done();
                else
                    rReq.Ignore();
            }
            break;


        case SID_AUTO_STYLE:
            DBG_ERROR("use ScAutoStyleHint instead of SID_AUTO_STYLE");
            break;

        case SID_GET_COLORTABLE:
            {
                //  passende ColorTable ist per PutItem gesetzt worden
                SvxColorTableItem* pColItem = (SvxColorTableItem*)GetItem(SID_COLOR_TABLE);
                XColorTable* pTable = pColItem->GetColorTable();
                rReq.SetReturnValue(OfaPtrItem(SID_GET_COLORTABLE, pTable));
            }
            break;

        case FID_CHG_RECORD:
            {
                ScDocument* pDoc = GetDocument();
                if(pDoc!=NULL)
                {
                    if(pDoc->GetChangeTrack()!=NULL)
                    {
                        WarningBox  aBox( GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_NO),
                                ScGlobal::GetRscString( STR_END_REDLINING ) );

                        if(aBox.Execute()==RET_YES)
                        {
                            pDoc->EndChangeTracking();
                            PostPaintGridAll();
                        }
                    }
                    else
                    {
                        pDoc->StartChangeTracking();
                        ScChangeViewSettings aChangeViewSet;
                        aChangeViewSet.SetShowChanges(TRUE);
                        pDoc->SetChangeViewSettings(aChangeViewSet);
                    }

                    //  update "accept changes" dialog
                    //! notify all views

                    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                    if ( pViewFrm && pViewFrm->HasChildWindow(FID_CHG_ACCEPT) )
                    {
                        SfxChildWindow* pChild = pViewFrm->GetChildWindow(FID_CHG_ACCEPT);
                        if (pChild)
                        {
                            ((ScAcceptChgDlgWrapper*)pChild)->ReInitDlg();
                        }
                    }
                    // Slots invalidieren
                    if (pBindings)
                        pBindings->InvalidateAll(FALSE);
                }
            }
            break;

        case SID_DOCUMENT_MERGE:
        case SID_DOCUMENT_COMPARE:
            {
                //! bei COMPARE eine Warnung, dass alte Changes verlorengehen ??!?!?

                SfxApplication* pApp = SFX_APP();
                const SfxPoolItem* pItem;
                SfxMedium* pMed = NULL;
                if ( pReqArgs &&
                     pReqArgs->GetItemState( SID_FILE_NAME, TRUE, &pItem ) == SFX_ITEM_SET &&
                     pItem->ISA(SfxStringItem) )
                {
                    String aFileName = ((const SfxStringItem*)pItem)->GetValue();

                    String aFilterName;
                    if ( pReqArgs->GetItemState( SID_FILTER_NAME, TRUE, &pItem ) == SFX_ITEM_SET &&
                         pItem->ISA(SfxStringItem) )
                    {
                        aFilterName = ((const SfxStringItem*)pItem)->GetValue();
                    }
                    String aOptions;
                    if ( pReqArgs->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) == SFX_ITEM_SET &&
                         pItem->ISA(SfxStringItem) )
                    {
                        aOptions = ((const SfxStringItem*)pItem)->GetValue();
                    }
                    short nVersion = 0;
                    if ( pReqArgs->GetItemState( SID_VERSION, TRUE, &pItem ) == SFX_ITEM_SET &&
                         pItem->ISA(SfxInt16Item) )
                    {
                        nVersion = ((const SfxInt16Item*)pItem)->GetValue();
                    }

                    //  kein Filter angegeben -> Detection
                    if ( !aFilterName.Len() )
                        ScDocumentLoader::GetFilterName( aFileName, aFilterName, aOptions );

                    //  filter name from dialog contains application prefix,
                    //  GetFilter needs name without the prefix.
                    ScDocumentLoader::RemoveAppPrefix( aFilterName );

                    const SfxFilter* pFilter = pApp->GetFilter( ScDocShell::Factory(), aFilterName );
                    SfxItemSet* pSet = new SfxAllItemSet( pApp->GetPool() );
                    if ( aOptions.Len() )
                        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
                    if ( nVersion != 0 )
                        pSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
                    pMed = new SfxMedium( aFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
                }
                else
                {
                    pMed = pApp->InsertDocumentDialog( 0, ScDocShell::Factory() );
                    if ( pMed )
                    {
                        //  kompletten Request zum Aufzeichnen zusammenbasteln
                        rReq.AppendItem( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
                        if ( nSlot == SID_DOCUMENT_COMPARE )
                        {
                            // Filter und Options nur bei Compare
                            if (pMed->GetFilter())
                                rReq.AppendItem( SfxStringItem( SID_FILTER_NAME,
                                                pMed->GetFilter()->GetFilterName() ) );
                            String aOptions = ScDocumentLoader::GetOptions(*pMed);
                            if (aOptions.Len())
                                rReq.AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
                        }
                        SfxItemSet* pSet = pMed->GetItemSet();
                        if ( pSet &&
                             pSet->GetItemState( SID_VERSION, TRUE, &pItem ) == SFX_ITEM_SET &&
                             pItem->ISA(SfxInt16Item) )
                        {
                            rReq.AppendItem( *pItem );
                        }
                    }
                }

                if ( pMed )     // nun wirklich ausfuehren...
                {
                    SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

                    ScDocShell* pOtherDocSh = new ScDocShell;
                    SvEmbeddedObjectRef aDocShTablesRef = pOtherDocSh;
                    pOtherDocSh->DoLoad( pMed );
                    ULONG nErr = pOtherDocSh->GetErrorCode();
                    if (nErr)
                        ErrorHandler::HandleError( nErr );          // auch Warnings

                    if ( !pOtherDocSh->GetError() )                 // nur Errors
                    {
                        BOOL bHadTrack = ( aDocument.GetChangeTrack() != NULL );

                        if ( nSlot == SID_DOCUMENT_COMPARE )
                            CompareDocument( *pOtherDocSh->GetDocument() );
                        else
                            MergeDocument( *pOtherDocSh->GetDocument() );

                        //  show "accept changes" dialog
                        //! get view for this document!

                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                            pViewFrm->ShowChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId(),TRUE); //@51669
                        SfxBindings* pBindings = GetViewBindings();
                        if (pBindings)
                            pBindings->Invalidate(FID_CHG_ACCEPT);

                        rReq.SetReturnValue( SfxInt32Item( nSlot, 0 ) );        //! ???????
                        rReq.Done();

                        if (!bHadTrack)         //  neu eingeschaltet -> auch anzeigen
                        {
                            ScChangeViewSettings* pOldSet = aDocument.GetChangeViewSettings();
                            if ( !pOldSet || !pOldSet->ShowChanges() )
                            {
                                ScChangeViewSettings aChangeViewSet;
                                aChangeViewSet.SetShowChanges(TRUE);
                                aDocument.SetChangeViewSettings(aChangeViewSet);
                            }
                        }
                    }
                    pOtherDocSh->DoClose();     // delete passiert mit der Ref
                }
            }
            break;

        case SID_DELETE_SCENARIO:
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    if ( pItem->ISA(SfxStringItem) )
                    {
                        String aName = ((const SfxStringItem*)pItem)->GetValue();
                        USHORT nTab;
                        if (aDocument.GetTable( aName, nTab ))
                        {
                            //  DeleteTable von viewfunc nach docfunc verschieben!

                            ScTabViewShell* pSh = GetBestViewShell();
                            if ( pSh )
                            {
                                //! SetTabNo in DeleteTable weglassen?
                                USHORT nDispTab = pSh->GetViewData()->GetTabNo();
                                pSh->DeleteTable( nTab );
                                pSh->SetTabNo(nDispTab);
                                rReq.Done();
                            }
                        }
                    }
                }
            }
            break;

        case SID_EDIT_SCENARIO:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    if ( pItem->ISA(SfxStringItem) )
                    {
                        String aName = ((const SfxStringItem*)pItem)->GetValue();
                        USHORT nTab;
                        if (aDocument.GetTable( aName, nTab ))
                        {
                            if (aDocument.IsScenario(nTab))
                            {
                                String aComment;
                                Color aColor;
                                USHORT nFlags;
                                aDocument.GetScenarioData( nTab, aComment, aColor, nFlags );

                                //! anderen Titel am Dialog setzen
                                ScNewScenarioDlg* pNewDlg =
                                    new ScNewScenarioDlg( GetDialogParent(), aName, TRUE );
                                pNewDlg->SetScenarioData( aName, aComment, aColor, nFlags );
                                if ( pNewDlg->Execute() == RET_OK )
                                {
                                    pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                    ModifyScenario( nTab, aName, aComment, aColor, nFlags );
                                    rReq.Done();
                                }
                                delete pNewDlg;
                            }
                        }
                    }
                }
            }
            break;

        case SID_ATTR_YEAR2000 :
        {
            const SfxPoolItem* pItem;
            if ( pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET )
            {
                if ( pItem->ISA(SfxUInt16Item) )
                {
                    UINT16 nY2k = ((SfxUInt16Item*)pItem)->GetValue();
                    // immer an den DocOptions setzen, damit das auch fuer SO50
                    // gespeichert wird (und alle Abfragen bisher auch darauf laufen).
                    // SetDocOptions propagiert das an den NumberFormatter
                    ScDocOptions aDocOpt( aDocument.GetDocOptions() );
                    aDocOpt.SetYear2000( nY2k );
                    aDocument.SetDocOptions( aDocOpt );
                    // die FormShell soll es mitbekommen
                    ScTabViewShell* pSh = GetBestViewShell();
                    if ( pSh )
                    {
                        FmFormShell* pFSh = pSh->GetFormShell();
                        if ( pFSh )
                            pFSh->SetY2KState( nY2k );
                    }
                }
            }
        }
        break;

        default:
        {
            // kleiner (?) Hack -> forward der Slots an TabViewShell
            ScTabViewShell* pSh = GetBestViewShell();
            if ( pSh )
                pSh->Execute( rReq );
            else
                SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        }
    }
}

//------------------------------------------------------------------

void ScDocShell::DoRecalc( BOOL bApi )
{
    BOOL bDone = FALSE;
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl(pSh);
        if ( pHdl && pHdl->IsInputMode() && pHdl->IsFormulaMode() && !bApi )
        {
            pHdl->FormulaPreview();     // Teilergebnis als QuickHelp
            bDone = TRUE;
        }
        else
        {
            pSh->UpdateInputLine();     // InputEnterHandler
            pSh->UpdateInputHandler();
        }
    }
    if (!bDone)                         // sonst Dokument neu berechnen
    {
        WaitObject aWaitObj( GetDialogParent() );
        aDocument.CalcFormulaTree();
        if ( pSh )
            pSh->UpdateCharts(TRUE);

        //  #47939# Wenn es Charts gibt, dann alles painten, damit nicht
        //  PostDataChanged und die Charts nacheinander kommen und Teile
        //  doppelt gepainted werden.

        ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
        if ( pCharts && pCharts->GetCount() )
            PostPaintGridAll();
        else
            PostDataChanged();
    }
}

void ScDocShell::DoHardRecalc( BOOL bApi )
{
    WaitObject aWaitObj( GetDialogParent() );
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        pSh->UpdateInputLine();     // InputEnterHandler
        pSh->UpdateInputHandler();
    }
    aDocument.CalcAll();
    GetDocFunc().DetectiveRefresh();    // erzeugt eigenes Undo
    if ( pSh )
        pSh->UpdateCharts(TRUE);
    PostPaintGridAll();
}

//------------------------------------------------------------------

void ScDocShell::DoAutoStyle( const ScRange& rRange, const String& rStyle )
{
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    ScStyleSheet* pStyleSheet =
        pStylePool->FindCaseIns( rStyle, SFX_STYLE_FAMILY_PARA );
    if (!pStyleSheet)
        pStyleSheet = (ScStyleSheet*)
            pStylePool->Find( ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA );
    if (pStyleSheet)
    {
        DBG_ASSERT(rRange.aStart.Tab() == rRange.aEnd.Tab(),
                        "DoAutoStyle mit mehreren Tabellen");
        USHORT nTab = rRange.aStart.Tab();
        USHORT nStartCol = rRange.aStart.Col();
        USHORT nStartRow = rRange.aStart.Row();
        USHORT nEndCol = rRange.aEnd.Col();
        USHORT nEndRow = rRange.aEnd.Row();
        aDocument.ApplyStyleAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, *pStyleSheet );
        aDocument.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        PostPaint( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab, PAINT_GRID );
    }
}

//------------------------------------------------------------------

void ScDocShell::NotifyStyle( const SfxStyleSheetHint& rHint )
{
    USHORT nId = rHint.GetHint();
    const SfxStyleSheetBase* pStyle = rHint.GetStyleSheet();
    if (!pStyle)
        return;

    if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PAGE )
    {
        if ( nId == SFX_STYLESHEET_MODIFIED )
        {
            ScDocShellModificator aModificator( *this );

            String aNewName = pStyle->GetName();
            String aOldName = aNewName;
            BOOL bExtended = rHint.ISA(SfxStyleSheetHintExtended);      // Name geaendert?
            if (bExtended)
                aOldName = ((SfxStyleSheetHintExtended&)rHint).GetOldName();

            if ( aNewName != aOldName )
                aDocument.RenamePageStyleInUse( aOldName, aNewName );

            USHORT nTabCount = aDocument.GetTableCount();
            for (USHORT nTab=0; nTab<nTabCount; nTab++)
                if (aDocument.GetPageStyle(nTab) == aNewName)   // schon auf neu angepasst
                {
                    aDocument.PageStyleModified( nTab, aNewName );
                    ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                }

            aModificator.SetDocumentModified();

            if (bExtended)
            {
                SfxBindings* pBindings = GetViewBindings();
                if (pBindings)
                {
                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                    pBindings->Invalidate( SID_STYLE_FAMILY4 );
                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                }
            }
        }
    }

    //  alles andere geht ueber Slots...
}

//  wie in printfun.cxx
#define ZOOM_MIN    10

void ScDocShell::SetPrintZoom( USHORT nTab, USHORT nScale, USHORT nPages )
{
    BOOL bUndo(aDocument.IsUndoEnabled());
    String aStyleName = aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        ScDocShellModificator aModificator( *this );

        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        if (bUndo)
        {
            USHORT nOldScale = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALE)).GetValue();
            USHORT nOldPages = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
            GetUndoManager()->AddUndoAction( new ScUndoPrintZoom(
                            this, nTab, nOldScale, nOldPages, nScale, nPages ) );
        }

        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

        ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
        aPrintFunc.UpdatePages();
        aModificator.SetDocumentModified();

        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( FID_RESET_PRINTZOOM );
    }
}

BOOL ScDocShell::AdjustPrintZoom( const ScRange& rRange )
{
    BOOL bChange = FALSE;
    USHORT nTab = rRange.aStart.Tab();

    String aStyleName = aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        BOOL bHeaders = ((const SfxBoolItem&)rSet.Get(ATTR_PAGE_HEADERS)).GetValue();
        USHORT nOldScale = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALE)).GetValue();
        USHORT nOldPages = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
        const ScRange* pRepeatCol = aDocument.GetRepeatColRange( nTab );
        const ScRange* pRepeatRow = aDocument.GetRepeatRowRange( nTab );

        //  benoetigte Skalierung fuer Selektion ausrechnen

        USHORT nNewScale = nOldScale;
        USHORT i;

        long nBlkTwipsX = 0;
        if (bHeaders)
            nBlkTwipsX += (long) PRINT_HEADER_WIDTH;
        USHORT nStartCol = rRange.aStart.Col();
        USHORT nEndCol = rRange.aEnd.Col();
        if ( pRepeatCol && nStartCol >= pRepeatCol->aStart.Col() )
        {
            for ( i=pRepeatCol->aStart.Col(); i<=pRepeatCol->aEnd.Col(); i++ )
                nBlkTwipsX += aDocument.GetColWidth( i, nTab );
            if ( nStartCol <= pRepeatCol->aEnd.Col() )
                nStartCol = pRepeatCol->aEnd.Col() + 1;
        }
        for ( i=nStartCol; i<=nEndCol; i++ )
            nBlkTwipsX += aDocument.GetColWidth( i, nTab );

        long nBlkTwipsY = 0;
        if (bHeaders)
            nBlkTwipsY += (long) PRINT_HEADER_HEIGHT;
        USHORT nStartRow = rRange.aStart.Row();
        USHORT nEndRow = rRange.aEnd.Row();
        if ( pRepeatRow && nStartRow >= pRepeatRow->aStart.Row() )
        {
            for ( i=pRepeatRow->aStart.Row(); i<=pRepeatRow->aEnd.Row(); i++ )
                nBlkTwipsY += aDocument.FastGetRowHeight( i, nTab );
            if ( nStartRow <= pRepeatRow->aEnd.Row() )
                nStartRow = pRepeatRow->aEnd.Row() + 1;
        }
        for ( i=nStartRow; i<=nEndRow; i++ )
            nBlkTwipsY += aDocument.FastGetRowHeight( i, nTab );

        Size aPhysPage;
        long nHdr, nFtr;
        ScPrintFunc aOldPrFunc( this, GetPrinter(), nTab );
        aOldPrFunc.GetScaleData( aPhysPage, nHdr, nFtr );
        nBlkTwipsY += nHdr + nFtr;

        long nNeeded = Min( aPhysPage.Width()  * 100 / nBlkTwipsX,
                            aPhysPage.Height() * 100 / nBlkTwipsY );
        if ( nNeeded < ZOOM_MIN )
            nNeeded = ZOOM_MIN;         // Begrenzung
        if ( nNeeded < (long) nNewScale )
            nNewScale = (USHORT) nNeeded;

        bChange = ( nNewScale != nOldScale || nOldPages != 0 );
        if ( bChange )
            SetPrintZoom( nTab, nNewScale, 0 );
    }
    return bChange;
}

void ScDocShell::PageStyleModified( const String& rStyleName, BOOL bApi )
{
    ScDocShellModificator aModificator( *this );

    BOOL bWarn = FALSE;

    USHORT nTabCount = aDocument.GetTableCount();
    USHORT nUseTab = MAXTAB+1;
    for (USHORT nTab=0; nTab<nTabCount && nUseTab>MAXTAB; nTab++)
        if ( aDocument.GetPageStyle(nTab) == rStyleName &&
                ( !bApi || aDocument.GetPageSize(nTab).Width() ) )
            nUseTab = nTab;
                                // bei bApi nur, wenn Umbrueche schon angezeigt

    if (nUseTab<=MAXTAB)        // nicht verwendet -> nichts zu tun
    {
        ScPrintFunc aPrintFunc( this, GetPrinter(), nUseTab );  //! ohne CountPages auskommen
        if (!aPrintFunc.UpdatePages())                          //  setzt Umbrueche auf allen Tabs
            bWarn = TRUE;

        if (bWarn && !bApi)
        {
            ScWaitCursorOff aWaitOff( GetDialogParent() );
            InfoBox aInfoBox(GetDialogParent(),
                             ScGlobal::GetRscString(STR_PRINT_INVALID_AREA));
            aInfoBox.Execute();
        }
    }

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( FID_RESET_PRINTZOOM );
}

void ScDocShell::ExecutePageStyle( SfxViewShell& rCaller,
                                   SfxRequest&   rReq,
                                   USHORT        nCurTab )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_STATUS_PAGESTYLE:  // Click auf StatusBar-Control
        case SID_FORMATPAGE:
            {
                if ( pReqArgs != NULL )
                {
                }
                else if ( pReqArgs == NULL )
                {
                    BOOL bUndo(aDocument.IsUndoEnabled());
                    String aOldName = aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aOldName, SFX_STYLE_FAMILY_PAGE );

                    DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        ScStyleSaveData aOldData;
                        if (bUndo)
                            aOldData.InitFromStyle( pStyleSheet );

                        SfxItemSet&     rStyleSet = pStyleSheet->GetItemSet();

                        ScStyleDlg* pDlg = new ScStyleDlg( GetDialogParent(),
                                                           *pStyleSheet,
                                                           RID_SCDLG_STYLES_PAGE );

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            WaitObject aWait( GetDialogParent() );

                            String aNewName = pStyleSheet->GetName();
                            if ( aNewName != aOldName &&
                                aDocument.RenamePageStyleInUse( aOldName, aNewName ) )
                            {
                                SfxBindings* pBindings = GetViewBindings();
                                if (pBindings)
                                {
                                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                                }
                            }

                            if ( pOutSet )
                                aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            // merken fuer GetState():
                            GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, bHeaderOn, bFooterOn );
                            rCaller.GetViewFrame()->GetBindings().Invalidate( SID_HFEDIT );

                            ScStyleSaveData aNewData;
                            aNewData.InitFromStyle( pStyleSheet );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                        new ScUndoModifyStyle( this, SFX_STYLE_FAMILY_PAGE,
                                                    aOldData, aNewData ) );
                            }

                            PageStyleModified( aNewName, FALSE );
                            rReq.Done();
                        }
                        delete pDlg;

                        rStyleSet.ClearItem( ATTR_PAGE_PAPERTRAY );
                    }
                }
            }
            break;

        case SID_HFEDIT:
            {
                if ( pReqArgs != NULL )
                {
                }
                else if ( pReqArgs == NULL )
                {
                    String aStr( aDocument.GetPageStyle( nCurTab ) );

                    ScStyleSheetPool* pStylePool
                        = aDocument.GetStyleSheetPool();

                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aStr, SFX_STYLE_FAMILY_PAGE );

                    DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet&  rStyleSet = pStyleSheet->GetItemSet();

                        SvxPageUsage eUsage =
                            SvxPageUsage( ((const SvxPageItem&)
                                            rStyleSet.Get( ATTR_PAGE )).
                                                GetPageUsage() );
                        BOOL bShareHeader = IS_SHARE_HEADER(rStyleSet);
                        BOOL bShareFooter = IS_SHARE_FOOTER(rStyleSet);
                        USHORT nResId = 0;

                        switch ( eUsage )
                        {
                            case SVX_PAGE_LEFT:
                            case SVX_PAGE_RIGHT:
                            {
                                if ( bHeaderOn && bFooterOn )
                                    nResId = RID_SCDLG_HFEDIT;
                                else if ( SVX_PAGE_RIGHT == eUsage )
                                {
                                    if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                                else
                                {
                                    //  #69193a# respect "shared" setting
                                    if ( !bHeaderOn && bFooterOn )
                                        nResId = bShareFooter ?
                                                    RID_SCDLG_HFEDIT_RIGHTFOOTER :
                                                    RID_SCDLG_HFEDIT_LEFTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = bShareHeader ?
                                                    RID_SCDLG_HFEDIT_RIGHTHEADER :
                                                    RID_SCDLG_HFEDIT_LEFTHEADER;
                                }
                            }
                            break;

                            case SVX_PAGE_MIRROR:
                            case SVX_PAGE_ALL:
                            default:
                            {
                                if ( !bShareHeader && !bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_ALL;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if ( bShareHeader && bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT;
                                    else
                                    {
                                        if ( !bHeaderOn && bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                        else if ( bHeaderOn && !bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                    }
                                }
                                else if ( !bShareHeader &&  bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SFTR;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if (  bShareHeader && !bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SHDR;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                            }
                        }

                        ScHFEditDlg* pDlg
                             = new ScHFEditDlg( SFX_APP()->GetViewFrame(),
                                                GetDialogParent(),
                                                rStyleSet,
                                                aStr,
                                                nResId );

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            if ( pOutSet )
                                aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            SetDocumentModified();
                            rReq.Done();
                        }
                        delete pDlg;
                    }
                }
            }
            break;

        default:
        break;
    }
}

void ScDocShell::GetStatePageStyle( SfxViewShell&   rCaller,
                                    SfxItemSet&     rSet,
                                    USHORT          nCurTab )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_STATUS_PAGESTYLE:
                rSet.Put( SfxStringItem( nWhich, aDocument.GetPageStyle( nCurTab ) ) );
                break;

            case SID_HFEDIT:
                {
                    String              aStr        = aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool*   pStylePool  = aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( aStr, SFX_STYLE_FAMILY_PAGE );

                    DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();

                        GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, bHeaderOn, bFooterOn );

                        if ( !bHeaderOn && !bFooterOn )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScDocShell::PreparePrint( PrintDialog* pPrintDialog, ScMarkData* pMarkData )
{
    SfxPrinter* pPrinter = GetPrinter();
    if ( !pPrinter ) return;

    delete pOldJobSetup;                        // gesetzt nur bei Fehler in StartJob()
    pOldJobSetup = new ScJobSetup( pPrinter );  // Einstellungen merken

    //  Einstellungen fuer die erste gedruckte Seite muessen hier (vor StartJob) gesetzt werden
    //! Selection etc. mit Print() zusammenfassen !!!
    //! Seiten nur einmal zaehlen

    BOOL bAllTabs = TRUE;
    USHORT nTabCount = aDocument.GetTableCount();
    USHORT nTab;
    long nTotalPages = 0;                       // alle Seiten
    long aPageArr[MAXTAB+1];                    // Seiten pro Tabelle
    for ( nTab=0; nTab<nTabCount; nTab++ )      // nPages und nTotalPages ermitteln
    {
        ScPrintFunc aPrintFunc( this, pPrinter, nTab );
        long nThisTab = aPrintFunc.GetTotalPages();
        aPageArr[nTab] = nThisTab;
        nTotalPages += nThisTab;
    }

    MultiSelection aPageRanges;         // zu druckende Seiten
    aPageRanges.SetTotalRange( Range( 0, RANGE_MAX ) );
    aPageRanges.Select( Range(1,nTotalPages) );
    PrintDialogRange eDlgOption = pPrintDialog ? pPrintDialog->GetCheckedRange() : PRINTDIALOG_ALL;
    switch ( eDlgOption )
    {
        case PRINTDIALOG_RANGE:
            aPageRanges = MultiSelection( pPrintDialog->GetRangeText() );
            break;

        case PRINTDIALOG_SELECTION:
            //  pMarkedRange interessiert hier nicht
            bAllTabs = FALSE;
            break;
    }

    if (!bAllTabs)          // Gesamt-Seitenzahl bei Selektion
    {
        nTotalPages = 0;
        for (nTab=0; nTab<nTabCount; nTab++)
            if ( !pMarkData || pMarkData->GetTableSelect(nTab) )
                nTotalPages += aPageArr[nTab];
        aPageRanges.Select( Range(1,nTotalPages) );
    }

    BOOL bFound = FALSE;        // erste Seite gefunden
    long nTabStart = 0;
    for ( nTab=0; nTab<nTabCount && !bFound; nTab++ )
    {
        if ( bAllTabs || !pMarkData || pMarkData->GetTableSelect( nTab ) )
        {
            ScPrintFunc aPrintFunc( this, pPrinter, nTab );

            long nNext = nTabStart + aPageArr[nTab];
            BOOL bSelected = FALSE;
            for (long nP=nTabStart+1; nP<=nNext; nP++)  // 1-basiert
                if (aPageRanges.IsSelected( nP ))       // eine Seite von dieser Tabelle selektiert?
                    bSelected = TRUE;

            if (bSelected)
            {
                aPrintFunc.ApplyPrintSettings();        // dann Settings fuer diese Tabelle
                bFound = TRUE;
            }
            nTabStart = nNext;
        }
    }
}

void ScDocShell::Print( SfxProgress& rProgress, PrintDialog* pPrintDialog,
                        ScMarkData* pMarkData, BOOL bForceSelected )
{
    SfxPrinter* pPrinter = GetPrinter();
    if ( !pPrinter ) return;

    PrintDialogRange eDlgOption = PRINTDIALOG_ALL;
    if ( bForceSelected )
        eDlgOption = PRINTDIALOG_SELECTION;

    ScRange* pMarkedRange = NULL;

    BOOL bAllTabs = TRUE;
    USHORT nTabCount = aDocument.GetTableCount();
    USHORT nTab;
    long nTotalPages = 0;                       // alle Seiten
    long aPageArr[MAXTAB+1];                    // Seiten pro Tabelle
    for ( nTab=0; nTab<nTabCount; nTab++ )      // nPages und nTotalPages ermitteln
    {
        ScPrintFunc aPrintFunc( this, pPrinter, nTab );
        long nThisTab = aPrintFunc.GetTotalPages();
        aPageArr[nTab] = nThisTab;
        nTotalPages += nThisTab;
    }

    MultiSelection aPageRanges;         // zu druckende Seiten
    aPageRanges.SetTotalRange( Range( 0, RANGE_MAX ) );
    aPageRanges.Select( Range(1,nTotalPages) );
    if ( pPrintDialog )
        eDlgOption = pPrintDialog->GetCheckedRange();
    switch ( eDlgOption )
    {
        case PRINTDIALOG_RANGE:
            aPageRanges = MultiSelection( pPrintDialog->GetRangeText() );
            break;

        case PRINTDIALOG_SELECTION:
            if ( pMarkData && ( pMarkData->IsMarked() || pMarkData->IsMultiMarked() ) )
            {
                pMarkData->MarkToMulti();
                pMarkedRange = new ScRange;
                pMarkData->GetMultiMarkArea( *pMarkedRange );
                pMarkData->MarkToSimple();
            }
            bAllTabs = FALSE;
            break;

        //case PRINTDIALOG_ALL:
        //default:
    }

    if (!bAllTabs)          // Gesamt-Seitenzahl bei Selektion
    {
        nTotalPages = 0;
        for (nTab=0; nTab<nTabCount; nTab++)
            if ( !pMarkData || pMarkData->GetTableSelect(nTab) )
                nTotalPages += aPageArr[nTab];
        aPageRanges.Select( Range(1,nTotalPages) );
    }

    USHORT nCollateCopies = 1;
    if ( pPrintDialog && pPrintDialog->IsCollateEnabled() && pPrintDialog->IsCollateChecked() )
        nCollateCopies = pPrintDialog->GetCopyCount();

    for ( USHORT n=0; n<nCollateCopies; n++ )
    {
        long nTabStart = 0;
        long nDisplayStart = 0;
        long nAttrPage = 1;

        for ( nTab=0; nTab<nTabCount; nTab++ )
        {
            if ( bAllTabs || !pMarkData || pMarkData->GetTableSelect( nTab ) )
            {
                FmFormView* pDrawView = NULL;
                Rectangle aFull( 0, 0, LONG_MAX, LONG_MAX );
                if ( aDocument.HasControl( nTab, aFull ) )
                {
                    ScDrawLayer* pModel = aDocument.GetDrawLayer();     // ist nicht NULL
                    pDrawView = new FmFormView( pModel, pPrinter );
                    pDrawView->ShowPagePgNum( nTab, Point() );
                    pDrawView->SetPrintPreview( TRUE );
                }

                ScPrintFunc aPrintFunc( this, pPrinter, nTab, nAttrPage, nTotalPages, pMarkedRange );
                aPrintFunc.SetDrawView( pDrawView );
                aPrintFunc.DoPrint( aPageRanges, nTabStart, nDisplayStart, &rProgress );

                nTabStart += aPageArr[nTab];
                if ( aDocument.NeedPageResetAfterTab(nTab) )
                    nDisplayStart = 0;
                else
                    nDisplayStart += aPageArr[nTab];
                nAttrPage = aPrintFunc.GetFirstPageNo();    // behalten oder aus Vorlage

                delete pDrawView;
            }
        }
    }

    delete pMarkedRange;

    if (pOldJobSetup)
    {
        pPrinter->SetOrientation( pOldJobSetup->eOrientation );
        pPrinter->SetPaperBin   ( pOldJobSetup->nPaperBin );
        pPrinter->SetPaper      ( pOldJobSetup->ePaper );

        if ( PAPER_USER == pOldJobSetup->ePaper )
        {
            pPrinter->SetMapMode( pOldJobSetup->aUserMapMode );
            pPrinter->SetPaperSizeUser( pOldJobSetup->aUserSize );
        }

        delete pOldJobSetup;
        pOldJobSetup = NULL;
    }

    PostPaintGridAll();                 //! nur wenn geaendert
}

void ScDocShell::GetState( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case FID_AUTO_CALC:
                if ( (BOOL) aDocument.GetHardRecalcState() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, aDocument.GetAutoCalc() ) );
                break;

            case FID_CHG_RECORD:
                {
                    ScDocument* pDoc = GetDocument();
                    if(pDoc->GetChangeTrack()!=NULL)
                        rSet.Put( SfxBoolItem( nWhich, TRUE));
                    else
                        rSet.Put( SfxBoolItem( nWhich, FALSE));
                }
                break;

            //  Wenn eine Formel editiert wird, muss FID_RECALC auf jeden Fall enabled sein.
            //  Recalc fuer das Doc war mal wegen #29898# disabled, wenn AutoCalc an war,
            //  ist jetzt wegen #41540# aber auch immer enabled.
//          case FID_RECALC:
//              if ( aDocument.GetAutoCalc() )
//                  rSet.DisableItem( nWhich );
//              break;

            case SID_TABLES_COUNT:
                rSet.Put( SfxUInt16Item( nWhich, aDocument.GetTableCount() ) );
                break;

            case SID_ATTR_YEAR2000 :
                rSet.Put( SfxUInt16Item( nWhich,
                    aDocument.GetDocOptions().GetYear2000() ) );
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScDocShell::GetSbxState( SfxItemSet &rSet )
{
    //  SID_SC_SELECTION (Selection),
    //  SID_SC_ACTIVECELL (ActiveCell),
    //  SID_SC_ACTIVETAB (ActiveTable),
    //  SID_TABLES_GET (Tables),
    //  SID_PIVOT_GET (DataPilotTables) - removed (old Basic)

        //
        //  Wenn hier Slots von der View-Shell executed werden, muss auch der
        //  GetState weitergeleitet werden!
        //

    ScTabViewShell* pVisibleSh = GetBestViewShell();        // sichtbare View
    if ( pVisibleSh )
        pVisibleSh->GetState( rSet );
}

void __EXPORT ScDocShell::Draw( OutputDevice* pDev, const JobSetup & rSetup, USHORT nAspect )
{
//  bIsOle = TRUE;      // jetzt ueber den CreateMode

    USHORT nVisTab = aDocument.GetVisibleTab();
    if (!aDocument.HasTable(nVisTab))
        return;

    if ( nAspect == ASPECT_THUMBNAIL )
    {
        Rectangle aBoundRect = GetVisArea( ASPECT_THUMBNAIL );
        ScViewData aTmpData( this, NULL );
        aTmpData.SetTabNo(nVisTab);
        aDocument.SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &aDocument, pDev, 1.0, aBoundRect, &aTmpData, TRUE );
    }
    else
    {
        Rectangle aBoundRect = SfxInPlaceObject::GetVisArea();
        ScViewData aTmpData( this, NULL );
        aTmpData.SetTabNo(nVisTab);
        aDocument.SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &aDocument, pDev, 1.0, aBoundRect, &aTmpData, TRUE );
    }
}

Rectangle __EXPORT ScDocShell::GetVisArea( USHORT nAspect ) const
{
    if ( eShellMode == SFX_CREATE_MODE_ORGANIZER )
    {
        //  ohne Inhalte wissen wir auch nicht, wie gross die Inhalte sind
        //  leeres Rechteck zurueckgeben, das wird dann nach dem Laden berechnet
        return Rectangle();
    }

    if( nAspect == ASPECT_THUMBNAIL )
    {
//      Rectangle aArea( 0,0, 3175,3175 );                          //  120x120 Pixel in 1:1
        Rectangle aArea( 0,0, SC_PREVIEW_SIZE_X,SC_PREVIEW_SIZE_Y );
        aDocument.SnapVisArea( aArea );
        return aArea;
    }
    else if( nAspect == ASPECT_CONTENT && eShellMode != SFX_CREATE_MODE_EMBEDDED )
    {
        //  Visarea holen wie nach Load

        USHORT nVisTab = aDocument.GetVisibleTab();
        if (!aDocument.HasTable(nVisTab))
        {
            nVisTab = 0;
            ((ScDocShell*)this)->aDocument.SetVisibleTab(nVisTab);
        }
        USHORT nStartCol,nStartRow;
        aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
        USHORT nEndCol,nEndRow;
        aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
        if (nStartCol>nEndCol)
            nStartCol = nEndCol;
        if (nStartRow>nEndRow)
            nStartRow = nEndRow;
        Rectangle aNewArea = ((ScDocument&)aDocument)
                                .GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab );
        ((ScDocShell*)this)->SvEmbeddedObject::SetVisArea( aNewArea );
        return aNewArea;
    }
    else
        return SfxInPlaceObject::GetVisArea( nAspect );
}

SvDataMemberObjectRef __EXPORT ScDocShell::CreateSnapshot()
{
    ScDocShell* pNewSh = new ScDocShell;
    SvEmbeddedObjectRef aDocShellRef = pNewSh;
    pNewSh->DoInitNew(NULL);
    pNewSh->ResetEmpty();
    ScDocument* pDestDoc = pNewSh->GetDocument();
    pDestDoc->RenameTab( 0,
                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("______42_____")),
                FALSE );

    //  Kopieren (wie in ScDataObject::GetCalcData):
    //  - Tabellen
    //  - Seitenvorlage

    if (aDocument.GetDrawLayer())
        pNewSh->MakeDrawLayer();

    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    ScStyleSheetPool* pDestPool = pDestDoc->GetStyleSheetPool();

    USHORT nCount = aDocument.GetTableCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
    {
        pDestDoc->MakeTable( nTab );
        pDestDoc->TransferTab( &aDocument, nTab, nTab );        // nicht einfuegen
        String aStyleName = aDocument.GetPageStyle( nTab );
        //  CopyStyleFrom kopiert SetItems mit richtigem Pool
        pDestPool->CopyStyleFrom( pStylePool, aStyleName, SFX_STYLE_FAMILY_PAGE );
    }

    pDestDoc->DeleteTab( nCount );      // vorher kann die einzige Tab nicht geloescht werden

    aDocument.CopyDdeLinks( pDestDoc );                     // Werte von DDE-Links kopieren

    pDestDoc->SetViewOptions( aDocument.GetViewOptions() );

    pDestDoc->SetVisibleTab( aDocument.GetVisibleTab() );
    // hier muss auch der Start angepasst werden
    pNewSh->SetVisAreaOrSize( SfxInPlaceObject::GetVisArea(), TRUE );

    return new ScDataObject( pNewSh );
}

void ScDocShell::GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                            USHORT            nCurTab,
                                            BOOL&             rbHeader,
                                            BOOL&             rbFooter )
{
    if ( !pStyleSet )
    {
        ScStyleSheetPool*  pStylePool  = aDocument.GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStylePool->
                                            Find( aDocument.GetPageStyle( nCurTab ),
                                                  SFX_STYLE_FAMILY_PAGE );

        DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

        if ( pStyleSheet )
            pStyleSet = &pStyleSheet->GetItemSet();
        else
            rbHeader = rbFooter = FALSE;
    }

    DBG_ASSERT( pStyleSet, "PageStyle-Set not found! :-(" );

    //--------------------------------------------------------------------

    const SvxSetItem*   pSetItem = NULL;
    const SfxItemSet*   pSet     = NULL;

    pSetItem = (const SvxSetItem*) &pStyleSet->Get( ATTR_PAGE_HEADERSET );
    pSet     = &pSetItem->GetItemSet();
    rbHeader = ((const SfxBoolItem&)pSet->Get(ATTR_PAGE_ON)).GetValue();

    pSetItem = (const SvxSetItem*) &pStyleSet->Get( ATTR_PAGE_FOOTERSET );
    pSet     = &pSetItem->GetItemSet();
    rbFooter = ((const SfxBoolItem&)pSet->Get(ATTR_PAGE_ON)).GetValue();
}

long __EXPORT ScDocShell::DdeGetData( const String& rItem,
                                      const String& rMimeType,
                                      ::com::sun::star::uno::Any & rValue )
{
    if( FORMAT_STRING == SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
        if( rItem.EqualsIgnoreCaseAscii( "Format" ) )
        {
            ByteString aFmtByte( aDdeTextFmt, gsl_getSystemTextEncoding() );
            rValue <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
                                        (sal_Int8*)aFmtByte.GetBuffer(),
                                        aFmtByte.Len() + 1 );
            return 1;
        }
        ScImportExport aObj( &aDocument, rItem );
        if ( !aObj.IsRef() )
            return 0;                           // ungueltiger Bereich

        if( aDdeTextFmt.GetChar(0) == 'F' )
            aObj.SetFormulas( TRUE );
        if( aDdeTextFmt.EqualsAscii( "SYLK" ) ||
            aDdeTextFmt.EqualsAscii( "FSYLK" ) )
        {
            ByteString aData;
            if( aObj.ExportByteString( aData, gsl_getSystemTextEncoding(),
                                        SOT_FORMATSTR_ID_SYLK ) )
            {
                rValue <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
                                            (sal_Int8*)aData.GetBuffer(),
                                            aData.Len() + 1 );
                return 1;
            }
            else
                return 0;
        }
        if( aDdeTextFmt.EqualsAscii( "CSV" ) ||
            aDdeTextFmt.EqualsAscii( "FCSV" ) )
            aObj.SetSeparator( ',' );
        return aObj.ExportData( rMimeType, rValue ) ? 1 : 0;
    }

    ScImportExport aObj( &aDocument, rItem );
    if( aObj.IsRef() )
        return aObj.ExportData( rMimeType, rValue ) ? 1 : 0;
    return 0;
}

long __EXPORT ScDocShell::DdeSetData( const String& rItem,
                                        const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
{
    if( FORMAT_STRING == SotExchange::GetFormatIdFromMimeType( rMimeType ))
    {
        if( rItem.EqualsIgnoreCaseAscii( "Format" ) )
        {
            ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
            if( rValue >>= aSeq )
            {
                aDdeTextFmt = String( (const sal_Char*)aSeq.getConstArray(),
                                        aSeq.getLength(),
                                        gsl_getSystemTextEncoding() );
                aDdeTextFmt.ToUpperAscii();
                return 1;
            }
            return 0;
        }
        ScImportExport aObj( &aDocument, rItem );
        if( aDdeTextFmt.GetChar(0) == 'F' )
            aObj.SetFormulas( TRUE );
        if( aDdeTextFmt.EqualsAscii( "SYLK" ) ||
            aDdeTextFmt.EqualsAscii( "FSYLK" ) )
        {
            ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
            if( rValue >>= aSeq )
            {
                String aData( (const sal_Char*)aSeq.getConstArray(),
                              aSeq.getLength(), gsl_getSystemTextEncoding() );
                return aObj.ImportString( aData, SOT_FORMATSTR_ID_SYLK ) ? 1 : 0;
            }
            return 0;
        }
        if( aDdeTextFmt.EqualsAscii( "CSV" ) ||
            aDdeTextFmt.EqualsAscii( "FCSV" ) )
            aObj.SetSeparator( ',' );
        return aObj.ImportData( rMimeType, rValue ) ? 1 : 0;
    }
    ScImportExport aObj( &aDocument, rItem );
    if( aObj.IsRef() )
        return aObj.ImportData( rMimeType, rValue ) ? 1 : 0;
    return 0;
}

::so3::SvLinkSource* __EXPORT ScDocShell::DdeCreateLinkSource( const String& rItem )
{
    //  only check for valid item string - range is parsed again in ScServerObject ctor

    //  named range?
    String aPos = rItem;
    ScRangeName* pRange = aDocument.GetRangeName();
    if( pRange )
    {
        USHORT nPos;
        if( pRange->SearchName( aPos, nPos ) )
        {
            ScRangeData* pData = (*pRange)[ nPos ];
            if( pData->HasType( RT_REFAREA )
                || pData->HasType( RT_ABSAREA )
                || pData->HasType( RT_ABSPOS ) )
                pData->GetSymbol( aPos );           // continue with the name's contents
        }
    }
    ScRange aRange;
    BOOL bValid = ( ( aRange.Parse( aPos, &aDocument ) & SCA_VALID ) ||
                    ( aRange.aStart.Parse( aPos, &aDocument ) & SCA_VALID ) );

    ScServerObject* pObj = NULL;            // NULL = error
    if ( bValid )
        pObj = new ScServerObject( this, rItem );

    //  GetLinkManager()->InsertServer() is in the ScServerObject ctor

    return pObj;
}

//------------------------------------------------------------------

ScViewData* ScDocShell::GetViewData()
{
    SfxViewShell* pCur = SfxViewShell::Current();
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,pCur);
    return pViewSh ? pViewSh->GetViewData() : NULL;
}

//------------------------------------------------------------------

USHORT ScDocShell::GetCurTab()
{
    //! this must be made non-static and use a ViewShell from this document!

    ScViewData* pViewData = GetViewData();

    return pViewData ? pViewData->GetTabNo() : 0;
}

ScTabViewShell* ScDocShell::GetBestViewShell()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    // falsches Doc?
    if( pViewSh && pViewSh->GetViewData()->GetDocShell() != this )
        pViewSh = NULL;
    if( !pViewSh )
    {
        // 1. ViewShell suchen
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame) );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            pViewSh = PTR_CAST(ScTabViewShell,p);
        }
    }
    return pViewSh;
}

SfxBindings* ScDocShell::GetViewBindings()
{
    //  used to invalidate slots after changes to this document

    SfxViewShell* pViewSh = GetBestViewShell();
    if (pViewSh)
        return &pViewSh->GetViewFrame()->GetBindings();
    else
        return NULL;
}

//------------------------------------------------------------------

ScDocShell* ScDocShell::GetShellByNum( USHORT nDocNo )      // static
{
    ScDocShell* pFound = NULL;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    USHORT nShellCnt = 0;

    while ( pShell && !pFound )
    {
        if ( pShell->Type() == TYPE(ScDocShell) )
        {
            if ( nShellCnt == nDocNo )
                pFound = (ScDocShell*) pShell;
            else
                ++nShellCnt;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }

    return pFound;
}

//------------------------------------------------------------------

IMPL_LINK( ScDocShell, ChartSelectionHdl, ChartSelectionInfo*, pInfo )
{
    if (!pInfo)
        return 0;

    //  die View suchen, auf der das Objekt aktiv ist

    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    while (pFrame)
    {
        SfxViewShell* pSh = pFrame->GetViewShell();
        if (pSh && pSh->ISA(ScTabViewShell))
        {
            ScTabViewShell* pViewSh = (ScTabViewShell*)pSh;
            SfxInPlaceClient* pClient = pViewSh->GetIPClient();
            if ( pClient && pClient->IsInPlaceActive() )
            {
                SvInPlaceObjectRef xIPObj = pClient->GetIPObj();
                if (xIPObj.Is())
                {
                    SchMemChart* pMemChart = SchDLL::GetChartData(xIPObj);
                    if (pMemChart)
                        return pViewSh->DoChartSelection( *pInfo, *pMemChart );
                }
            }
        }
        pFrame = SfxViewFrame::GetNext( *pFrame, this );
    }

    return 0;
}



