/*************************************************************************
 *
 *  $RCSfile: cellsh2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-10 19:05:10 $
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

//------------------------------------------------------------------

#define _BIGINT_HXX
//#define _BASEDLGS_HXX ***
#define _CACHESTR_HXX
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
//#define _FILTER_HXX
#define _FONTDLG_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
//#define _PRNDLG_HXX
#define _POLY_HXX
#define _PRVWIN_HXX
#define _QUEUE_HXX
//#define _RULER_HXX
#define _SCRWIN_HXX
#define _SOUND_HXX
#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDMENU_HXX
#define _TABBAR_HXX
//#define _TREELIST_HXX
#define _SFXBASIC_HXX
#define _SFX_DOCFILE_HXX
//#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXIMGMGR_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW         //
#define _SI_NOCONTROL

#define _SVBOXITM_HXX
//#define _SVCONTNR_HXX
#define _SVDATTR_HXX
#define _SVDXOUT_HXX
#define _SVDEC_HXX
#define _SVDIO_HXX
#define _SVDLAYER_HXX
#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX


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


// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/request.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/sbxcore.hxx>
#include <svtools/whiter.hxx>
#include <svtools/zforlist.hxx>
#include <offmgr/sbaitems.hxx>
#include <offmgr/sbasltid.hrc>
#include <vcl/msgbox.hxx>

#include "cellsh.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "dbfunc.hxx"
#include "dbdocfun.hxx"
#include "lbseldlg.hxx"
#include "sortdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#include "subtdlg.hxx"
#include "reffact.hxx"
#include "pvlaydlg.hxx"
#include "validat.hxx"
#include "scresid.hxx"
#include "validate.hxx"
#include "pivot.hxx"
#include "dpobject.hxx"
#include "dapitype.hxx"
#include "dapidata.hxx"
#include "dpsdbtab.hxx"     // ScImportSourceDesc
#include "dpshttab.hxx"     // ScSheetSourceDesc

using namespace com::sun::star;

//#include "strindlg.hxx"       //! Test !!!!!

//static ScArea aPivotSource;       //! wohin? (ueber den Dialog retten)


#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

//------------------------------------------------------------------

void ScCellShell::ExecuteDB( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    USHORT nSlotId = rReq.GetSlot();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    SfxApplication*     pSfxApp     = SFX_APP();
    ScModule*           pScMod      = SC_MOD();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    switch ( nSlotId )
    {
        case SID_IMPORT_DATA:
            {
                //! was wird mit dem Adressbuch?
#if 0
                if( pScApp->GetSbaObject()->IsAddrPIOpen() )
                {
                    pViewData->GetDispatcher().Execute( SID_SBA_ADDRPI,
                                            SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
                }
#endif

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( SID_IMPORT_DATA, &pItem ) )
                    {
                        String  aDBName, aSQLCommand;

                        aDBName = ((const SfxStringItem*)pItem)->GetValue();

                        if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                            aSQLCommand = ((const SfxStringItem*)pItem)->GetValue();


                        ScImportParam aImportParam;
                        ScDBData* pDBData = pTabViewShell->GetDBData(TRUE,SC_DB_IMPORT);        // Namen vergeben
                        pDBData->GetImportParam( aImportParam );
                        aImportParam.aDBName    = aDBName;
                        aImportParam.aStatement = aSQLCommand;
                        aImportParam.bSql       = TRUE;             // kein Name, sondern Sql
                        aImportParam.bImport    = TRUE;

                        pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );

                        rReq.Done();
                    }
                    else
                    {
                        rReq.Ignore();
                    }
                }
                else            // DB-Browser anzeigen
                {
                    ScImportParam aImportParam;
                    ScDBData* pDBData = pTabViewShell->GetDBData(TRUE,SC_DB_OLD);       // nicht neu anlegen
                    if (pDBData)
                        pDBData->GetImportParam( aImportParam );

                    ScDBDocFunc::ShowInBeamer( aImportParam, pTabViewShell->GetViewFrame() );
                }
            }
            break;

        case SID_REIMPORT_DATA:
            {
                BOOL bOk = FALSE;
                ScDBData* pDBData = pTabViewShell->GetDBData(TRUE,SC_DB_OLD);
                if (pDBData)
                {
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    //! Undo ??
                        bOk = TRUE;
                    }
                }

                if (!bOk && ! rReq.IsAPI() )
                    pTabViewShell->ErrorMessage(STR_REIMPORT_EMPTY);

                if( bOk )
                    rReq.Done();
            }
            break;

        case SID_REFRESH_DBAREA:
            {
                ScDBData* pDBData = pTabViewShell->GetDBData(TRUE,SC_DB_OLD);
                if (pDBData)
                {
                    //  Import wiederholen wie SID_REIMPORT_DATA

                    BOOL bContinue = TRUE;
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        bContinue = pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    //! Undo ??

                        //  markieren (Groesse kann sich geaendert haben)
                        ScRange aNewRange;
                        pDBData->GetArea(aNewRange);
                        pTabViewShell->MarkRange(aNewRange);
                    }

                    if ( bContinue )        // #41905# Fehler beim Import -> Abbruch
                    {
                        //  interne Operationen, wenn welche gespeichert

                        if ( pDBData->HasQueryParam() || pDBData->HasSortParam() ||
                                                          pDBData->HasSubTotalParam() )
                            pTabViewShell->RepeatDB();

                        //  Pivottabellen die den Bereich als Quelldaten haben

                        ScRange aRange;
                        pDBData->GetArea(aRange);
                        GetViewData()->GetDocShell()->RefreshPivotTables(aRange);
                    }
                }
                rReq.Done();
            }
            break;

        case SID_SBA_BRW_INSERT:
            {
                if (pReqArgs)
                {
                    const SfxStringItem &rDBNameItem = (const SfxStringItem&)
                                                        pReqArgs->Get(SID_ATTR_SBA_DATABASE);
                    const SfxStringItem &rStatementItem = (const SfxStringItem&)
                                                        pReqArgs->Get(SID_ATTR_SBA_STATEMENT);
                    const SbaSelectionItem &rSelectionItem = (const SbaSelectionItem&)
                                                        pReqArgs->Get(SID_ATTR_SBA_SELECTION);

                    DBG_ASSERT( rDBNameItem.ISA(SfxStringItem), "invalid argument type" );
                    DBG_ASSERT( rStatementItem.ISA(SfxStringItem), "invalid argument type" );
                    DBG_ASSERT( rSelectionItem.ISA(SbaSelectionItem), "invalid argument type" );

                    ScViewData* pViewData   = GetViewData();
                    ScImportParam aImParam;
                    aImParam.nCol1 = aImParam.nCol2 = pViewData->GetCurX();
                    aImParam.nRow1 = aImParam.nRow2 = pViewData->GetCurY();
                    aImParam.bImport = TRUE;
                    aImParam.aDBName = rDBNameItem.GetValue();
                    aImParam.aStatement = rStatementItem.GetValue();

                    ScDBDocFunc( *pViewData->GetDocShell() ).
                        DoImport( pViewData->GetTabNo(), aImParam,
                                        rSelectionItem.GetSelectionList(), TRUE, TRUE );
                    rReq.Done();
                }
                else
                    DBG_ERROR( "arguments expected" );
            }
            break;

        case SID_SUBTOTALS:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    pTabViewShell->DoSubTotals( ((const ScSubTotalItem&) pArgs->Get( SCITEM_SUBTDATA )).
                                    GetSubTotalData() );
                    rReq.Done();
                }
                else
                {
                    ScSubTotalDlg*  pDlg = NULL;
                    ScSubTotalParam aSubTotalParam;
                    SfxItemSet      aArgSet( GetPool(), SCITEM_SUBTDATA, SCITEM_SUBTDATA );

                    ScDBData* pDBData = pTabViewShell->GetDBData();
                    pDBData->GetSubTotalParam( aSubTotalParam );
                    aSubTotalParam.bRemoveOnly = FALSE;

                    aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, GetViewData(), &aSubTotalParam ) );
                    pDlg = new ScSubTotalDlg( pTabViewShell->GetDialogParent(), &aArgSet );
                    pDlg->SetCurPageId(1);

                    short bResult = pDlg->Execute();

                    if ( (bResult == RET_OK) || (bResult == SCRET_REMOVE) )
                    {
                        const SfxItemSet* pOutSet = NULL;

                        if ( bResult == RET_OK )
                        {
                            pOutSet = pDlg->GetOutputItemSet();
                            aSubTotalParam =
                                ((const ScSubTotalItem&)
                                    pOutSet->Get( SCITEM_SUBTDATA )).
                                        GetSubTotalData();
                        }
                        else // if (bResult == SCRET_REMOVE)
                        {
                            pOutSet = &aArgSet;
                            aSubTotalParam.bRemoveOnly = TRUE;
                            aSubTotalParam.bReplace    = TRUE;
                            aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA,
                                                         GetViewData(),
                                                         &aSubTotalParam ) );
                        }

                        pTabViewShell->DoSubTotals( aSubTotalParam );
                        rReq.Done( *pOutSet );
                    }
                    delete pDlg;
                }
            }
            break;

        case SID_SORT_DESCENDING:
        case SID_SORT_ASCENDING:
            {
                SfxItemSet  aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );
                ScSortParam aSortParam;
                ScDBData*   pDBData = pTabViewShell->GetDBData();

                pDBData->GetSortParam( aSortParam );
                aSortParam.bHasHeader       = FALSE;
                aSortParam.bByRow           = TRUE;
                aSortParam.bCaseSens        = FALSE;
                aSortParam.bIncludePattern  = FALSE;
                aSortParam.bInplace         = TRUE;
                aSortParam.bDoSort[0]       = TRUE;
                aSortParam.nField[0]        = aSortParam.nCol1;
                aSortParam.bAscending[0]    = (nSlotId == SID_SORT_ASCENDING);

                for ( USHORT i=1; i<MAXSORT; i++ )
                    aSortParam.bDoSort[i] = FALSE;

                aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );

                pTabViewShell->UISort( aSortParam );        // Teilergebnisse bei Bedarf neu

                rReq.Done( aArgSet );
            }
            break;

        case SID_SORT:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                if ( pArgs )        // Basic
                {
                    ScSortParam aSortParam;
                    ScDBData* pDBData = pTabViewShell->GetDBData();
                    pDBData->GetSortParam( aSortParam );
                    aSortParam.bInplace = TRUE;             // von Basic immer

                    const SfxPoolItem* pItem;
                    if ( pArgs->GetItemState( SID_SORT_BYROW, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bByRow = ((const SfxBoolItem*)pItem)->GetValue();
                    if ( pArgs->GetItemState( SID_SORT_HASHEADER, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bHasHeader = ((const SfxBoolItem*)pItem)->GetValue();
                    if ( pArgs->GetItemState( SID_SORT_CASESENS, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bCaseSens = ((const SfxBoolItem*)pItem)->GetValue();
                    if ( pArgs->GetItemState( SID_SORT_ATTRIBS, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bIncludePattern = ((const SfxBoolItem*)pItem)->GetValue();
                    if ( pArgs->GetItemState( SID_SORT_USERDEF, TRUE, &pItem ) == SFX_ITEM_SET )
                    {
                        USHORT nUserIndex = ((const SfxUInt16Item*)pItem)->GetValue();
                        aSortParam.bUserDef = ( nUserIndex != 0 );
                        if ( nUserIndex )
                            aSortParam.nUserIndex = nUserIndex - 1;     // Basic: 1-basiert
                    }

                    USHORT nField0 = 0;
                    if ( pArgs->GetItemState( FN_PARAM_1, TRUE, &pItem ) == SFX_ITEM_SET )
                        nField0 = ((const SfxUInt16Item*)pItem)->GetValue();
                    aSortParam.bDoSort[0] = ( nField0 != 0 );
                    aSortParam.nField[0] = nField0 ? (nField0-1) : 0;
                    if ( pArgs->GetItemState( FN_PARAM_2, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bAscending[0] = ((const SfxBoolItem*)pItem)->GetValue();
                    USHORT nField1 = 0;
                    if ( pArgs->GetItemState( FN_PARAM_3, TRUE, &pItem ) == SFX_ITEM_SET )
                        nField1 = ((const SfxUInt16Item*)pItem)->GetValue();
                    aSortParam.bDoSort[1] = ( nField1 != 0 );
                    aSortParam.nField[1] = nField1 ? (nField1-1) : 0;
                    if ( pArgs->GetItemState( FN_PARAM_4, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bAscending[1] = ((const SfxBoolItem*)pItem)->GetValue();
                    USHORT nField2 = 0;
                    if ( pArgs->GetItemState( FN_PARAM_5, TRUE, &pItem ) == SFX_ITEM_SET )
                        nField2 = ((const SfxUInt16Item*)pItem)->GetValue();
                    aSortParam.bDoSort[2] = ( nField2 != 0 );
                    aSortParam.nField[2] = nField2 ? (nField2-1) : 0;
                    if ( pArgs->GetItemState( FN_PARAM_6, TRUE, &pItem ) == SFX_ITEM_SET )
                        aSortParam.bAscending[2] = ((const SfxBoolItem*)pItem)->GetValue();

                    // Teilergebnisse bei Bedarf neu
                    pTabViewShell->UISort( aSortParam );
                    rReq.Done();
                }
                else
                {
                    ScSortDlg*  pDlg = NULL;
                    ScSortParam aSortParam;
                    SfxItemSet  aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );

                    ScDBData* pDBData = pTabViewShell->GetDBData();
                    pDBData->GetSortParam( aSortParam );

                    aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );
                    pDlg = new ScSortDlg( pTabViewShell->GetDialogParent(), &aArgSet );
                    pDlg->SetCurPageId(1);

                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        const ScSortParam& rOutParam = ((const ScSortItem&)
                                       pOutSet->Get( SCITEM_SORTDATA )).GetSortData();

                        // Teilergebnisse bei Bedarf neu
                        pTabViewShell->UISort( rOutParam );

                        if ( rOutParam.bInplace )
                        {
                            rReq.AppendItem( SfxBoolItem( SID_SORT_BYROW,
                                                rOutParam.bByRow ) );
                            rReq.AppendItem( SfxBoolItem( SID_SORT_HASHEADER,
                                                rOutParam.bHasHeader ) );
                            rReq.AppendItem( SfxBoolItem( SID_SORT_CASESENS,
                                                rOutParam.bCaseSens ) );
                            rReq.AppendItem( SfxBoolItem( SID_SORT_ATTRIBS,
                                                rOutParam.bIncludePattern ) );
                            USHORT nUser = rOutParam.bUserDef ? ( rOutParam.nUserIndex + 1 ) : 0;
                            rReq.AppendItem( SfxUInt16Item( SID_SORT_USERDEF, nUser ) );
                            if ( rOutParam.bDoSort[0] )
                            {
                                rReq.AppendItem( SfxUInt16Item( FN_PARAM_1,
                                                    rOutParam.nField[0] + 1 ) );
                                rReq.AppendItem( SfxBoolItem( FN_PARAM_2,
                                                    rOutParam.bAscending[0] ) );
                            }
                            if ( rOutParam.bDoSort[1] )
                            {
                                rReq.AppendItem( SfxUInt16Item( FN_PARAM_3,
                                                    rOutParam.nField[1] + 1 ) );
                                rReq.AppendItem( SfxBoolItem( FN_PARAM_4,
                                                    rOutParam.bAscending[1] ) );
                            }
                            if ( rOutParam.bDoSort[2] )
                            {
                                rReq.AppendItem( SfxUInt16Item( FN_PARAM_5,
                                                    rOutParam.nField[2] + 1 ) );
                                rReq.AppendItem( SfxBoolItem( FN_PARAM_6,
                                                    rOutParam.bAscending[2] ) );
                            }
                        }

                        rReq.Done();
                    }

                    delete pDlg;
                }
            }
            break;

/*
            {

                USHORT          nId  = ScPivotLayoutWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pSfxApp->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );

            }
            break;
*/

        case SID_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    DBG_ERROR("SID_FILTER with arguments?");
                    pTabViewShell->Query( ((const ScQueryItem&)
                            pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, TRUE );
                    rReq.Done();
                }
                else
                {
                    USHORT          nId  = ScFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
                }
            }
            break;

        case SID_SPECIAL_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    DBG_ERROR("SID_SPECIAL_FILTER with arguments?");
                    pTabViewShell->Query( ((const ScQueryItem&)
                            pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, TRUE );
                    rReq.Done();
                }
                else
                {
                    USHORT          nId  = ScSpecialFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
                }
            }
            break;

        case FID_FILTER_OK:
            {
                const SfxItemSet*  pOutSet = rReq.GetArgs();
                const ScQueryItem& rItem   = (const ScQueryItem&)
                                             pOutSet->Get( SCITEM_QUERYDATA );

                USHORT nCurTab = GetViewData()->GetTabNo();
                USHORT nRefTab = GetViewData()->GetRefTabNo();

                // Wenn RefInput auf andere Tabelle als Datentabelle umgeschaltet
                // hat wieder zurueckschalten:

                if ( nCurTab != nRefTab )
                {
                    pTabViewShell->SetTabNo( nRefTab );
                    pTabViewShell->PaintExtras();
                }

                ScRange aAdvSource;
                if (rItem.GetAdvancedQuerySource(aAdvSource))
                    pTabViewShell->Query( rItem.GetQueryData(), &aAdvSource, TRUE );
                else
                    pTabViewShell->Query( rItem.GetQueryData(), NULL, TRUE );
                rReq.Done( *pOutSet );
            }
            break;

        case SID_UNFILTER:
            {
                ScQueryParam aParam;
                ScDBData*    pDBData = pTabViewShell->GetDBData();

                pDBData->GetQueryParam( aParam );
                USHORT nEC = aParam.GetEntryCount();
                for (USHORT i=0; i<nEC; i++)
                    aParam.GetEntry(i).bDoQuery = FALSE;
                aParam.bDuplicate = TRUE;
                pTabViewShell->Query( aParam, NULL, TRUE );
                rReq.Done();
            }
            break;

        case SID_AUTO_FILTER:
            pTabViewShell->ToggleAutoFilter();
            rReq.Done();
            break;

        case SID_AUTOFILTER_HIDE:
            pTabViewShell->HideAutoFilter();
            rReq.Done();
            break;

        case SID_PIVOT_TABLE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET ==
                        pReqArgs->GetItemState( SCITEM_PIVOTDATA, TRUE, &pItem ) )
                {
                    USHORT nCurTab = GetViewData()->GetTabNo();
                    USHORT nRefTab = GetViewData()->GetRefTabNo();

                    // Wenn RefInput auf andere Tabelle als Datentabelle umgeschaltet
                    // hat wieder zurueckschalten:

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    const ScDPObject* pDPObject = pTabViewShell->GetDialogDPObject();
                    if ( pDPObject )
                    {
                        const ScPivotItem* pPItem = (const ScPivotItem*)pItem;
                        pTabViewShell->MakePivotTable(
                                            pPItem->GetData(),
                                            pPItem->GetDestRange(),
                                            pPItem->IsNewSheet(),
                                            *pDPObject );
                    }
                    rReq.Done();
                }
                else if (rReq.IsAPI())
                    SbxBase::SetError(SbxERR_BAD_PARAMETER);
            }
            break;

        case SID_OPENDLG_PIVOTTABLE:
            {
                ScViewData* pViewData = GetViewData();
                ScDocument* pDoc = pViewData->GetDocument();

                ScDPObject* pNewDPObject = NULL;

                // ScPivot is no longer used...
                ScDPObject* pDPObj = pDoc->GetDPAtCursor(
                                            pViewData->GetCurX(), pViewData->GetCurY(),
                                            pViewData->GetTabNo() );
                if ( pDPObj )   // on an existing table?
                {
                    pNewDPObject = new ScDPObject( *pDPObj );
                }
                else            // create new table
                {
                    //  select database range or data
                    pTabViewShell->GetDBData( TRUE, SC_DB_OLD );
                    if ( !GetViewData()->GetMarkData().IsMarked() )
                        pTabViewShell->MarkDataArea( FALSE );

                    //  output to cursor position for non-sheet data
                    ScAddress aDestPos( pViewData->GetCurX(), pViewData->GetCurY(),
                                            pViewData->GetTabNo() );

                    //  first select type of source data

                    BOOL bEnableExt = ScDPObject::HasRegisteredSources();
                    ScDataPilotSourceTypeDlg* pTypeDlg = new ScDataPilotSourceTypeDlg(
                                                pTabViewShell->GetDialogParent(), bEnableExt );
                    if ( pTypeDlg->Execute() == RET_OK )
                    {
                        if ( pTypeDlg->IsExternal() )
                        {
                            uno::Sequence<rtl::OUString> aSources = ScDPObject::GetRegisteredSources();
                            ScDataPilotServiceDlg* pServDlg = new ScDataPilotServiceDlg(
                                                pTabViewShell->GetDialogParent(), aSources );
                            if ( pServDlg->Execute() == RET_OK )
                            {
                                ScDPServiceDesc aServDesc(
                                        pServDlg->GetServiceName(),
                                        pServDlg->GetParSource(),
                                        pServDlg->GetParName(),
                                        pServDlg->GetParUser(),
                                        pServDlg->GetParPass() );
                                pNewDPObject = new ScDPObject( pDoc );
                                pNewDPObject->SetServiceData( aServDesc );
                            }
                            delete pServDlg;
                        }
                        else if ( pTypeDlg->IsDatabase() )
                        {
                            ScDataPilotDatabaseDlg* pDataDlg = new ScDataPilotDatabaseDlg(
                                                            pTabViewShell->GetDialogParent() );
                            if ( pDataDlg->Execute() == RET_OK )
                            {
                                ScImportSourceDesc aImpDesc;
                                pDataDlg->GetValues( aImpDesc );
                                pNewDPObject = new ScDPObject( pDoc );
                                pNewDPObject->SetImportDesc( aImpDesc );
                            }
                            delete pDataDlg;
                        }
                        else        // selection
                        {
                            //! use database ranges (select before type dialog?)
                            ScRange aRange;
                            if ( GetViewData()->GetSimpleArea( aRange ) )
                            {
                                ScSheetSourceDesc aShtDesc;
                                aShtDesc.aSourceRange = aRange;
                                pNewDPObject = new ScDPObject( pDoc );
                                pNewDPObject->SetSheetDesc( aShtDesc );

                                //  output below source data
                                if ( aRange.aEnd.Row()+2 <= MAXROW - 4 )
                                    aDestPos = ScAddress( aRange.aStart.Col(),
                                                            aRange.aEnd.Row()+2,
                                                            aRange.aStart.Tab() );
                            }
                        }
                    }
                    delete pTypeDlg;

                    if ( pNewDPObject )
                        pNewDPObject->SetOutRange( aDestPos );

#if 0
                    ScDBData*   pDBData = pTabViewShell->GetDBData();
                    String      aErrMsg;

                    pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

                    bAreaOk = TRUE;
                    if ( nRow2-nRow1 < 1 )
                    {
                        // "mindestens eine Datenzeile"
                        pTabViewShell->ErrorMessage(STR_PIVOT_INVALID_DBAREA);
                        bAreaOk = FALSE;
                    }
                    else if (!pDBData->HasHeader())
                    {
                        if ( MessBox( pTabViewShell->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       // "StarCalc"
                                ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 )     // Koepfe aus erster Zeile?
                            ).Execute() == RET_YES )
                        {
                            pDBData->SetHeader( TRUE );     //! Undo ??
                        }
                        else
                            bAreaOk = FALSE;
                    }
#endif
                }

                pTabViewShell->SetDialogDPObject( pNewDPObject );   // is copied
                if ( pNewDPObject )
                {
                    //  start layout dialog

                    USHORT nId  = ScPivotLayoutWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );
                    pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
                }
                delete pNewDPObject;
            }
            break;

        case SID_DEFINE_DBNAME:
            {

                USHORT          nId  = ScDbNameDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );

            }
            break;

        case SID_SELECT_DB:
            {
                if ( pReqArgs )
                {
                    const SfxStringItem* pItem =
                        (const SfxStringItem*)&pReqArgs->Get( SID_SELECT_DB );

                    if( pItem )
                    {
                        pTabViewShell->GotoDBArea( pItem->GetValue() );
                        rReq.Done();
                    }
                    else
                        DBG_ERROR("NULL");
                }
                else
                {
                    ScDocument*     pDoc   = GetViewData()->GetDocument();
                    ScDBCollection* pDBCol = pDoc->GetDBCollection();

                    if ( pDBCol )
                    {
                        const String    aStrNoName( ScGlobal::GetRscString(STR_DB_NONAME) );
                        List            aList;
                        USHORT          nDBCount = pDBCol->GetCount();
                        ScDBData*       pDbData  = NULL;
                        String*         pDBName  = NULL;

                        for ( USHORT i=0; i < nDBCount; i++ )
                        {
                            pDbData = (ScDBData*)(pDBCol->At( i ));
                            if ( pDbData )
                            {
                                pDBName = new String;
                                pDbData->GetName( *pDBName );

                                if ( *pDBName != aStrNoName )
                                    aList.Insert( pDBName );
                                else
                                    DELETEZ(pDBName);
                            }
                        }

                        ScSelEntryDlg* pDlg =
                            new ScSelEntryDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_SELECTDB,
                                               String(ScResId(SCSTR_SELECTDB)),
                                               String(ScResId(SCSTR_AREAS)),
                                               aList );

                        if ( pDlg->Execute() == RET_OK )
                        {
                            pTabViewShell->GotoDBArea( pDlg->GetSelectEntry() );
                            rReq.Done();
                        }

                        delete pDlg;

                        void* pEntry = aList.First();
                        while ( pEntry )
                        {
                            delete (String*) aList.Remove( pEntry );
                            pEntry = aList.Next();
                        }
                    }
                }
            }
            break;

        case FID_VALIDATION:
            {
                const SfxPoolItem* pItem;
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    DBG_ERROR("spaeter...");
                }
                else
                {
                    SfxItemSet aArgSet( GetPool(), FID_VALID_MODE, FID_VALID_ERRTEXT );

                    ScValidationMode eMode = SC_VALID_ANY;
                    ScConditionMode eOper = SC_COND_EQUAL;
                    String aExpr1, aExpr2;
                    BOOL bBlank = TRUE;
                    BOOL bShowHelp = FALSE;
                    String aHelpTitle, aHelpText;
                    BOOL bShowError = FALSE;
                    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
                    String aErrTitle, aErrText;

                    ScDocument* pDoc = GetViewData()->GetDocument();
                    USHORT nCurX = GetViewData()->GetCurX();
                    USHORT nCurY = GetViewData()->GetCurY();
                    USHORT nTab = GetViewData()->GetTabNo();
                    ScAddress aCursorPos( nCurX, nCurY, nTab );
                    ULONG nIndex = ((SfxUInt32Item*)pDoc->GetAttr(
                                nCurX, nCurY, nTab, ATTR_VALIDDATA ))->GetValue();
                    if ( nIndex )
                    {
                        const ScValidationData* pOldData = pDoc->GetValidationEntry( nIndex );
                        if ( pOldData )
                        {
                            eMode = pOldData->GetDataMode();
                            eOper = pOldData->GetOperation();
                            ULONG nNumFmt = 0;
                            if ( eMode == SC_VALID_DATE || eMode == SC_VALID_TIME )
                            {
                                short nType = ( eMode == SC_VALID_DATE ) ? NUMBERFORMAT_DATE
                                                                         : NUMBERFORMAT_TIME;
                                nNumFmt = pDoc->GetFormatTable()->GetStandardFormat(
                                                                    nType, ScGlobal::eLnge );
                            }
                            aExpr1 = pOldData->GetExpression( aCursorPos, 0, nNumFmt );
                            aExpr2 = pOldData->GetExpression( aCursorPos, 1, nNumFmt );
                            bBlank = pOldData->IsIgnoreBlank();

                            bShowHelp = pOldData->GetInput( aHelpTitle, aHelpText );
                            bShowError = pOldData->GetErrMsg( aErrTitle, aErrText, eErrStyle );

                            aArgSet.Put( SfxAllEnumItem( FID_VALID_MODE,        eMode ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE,    eOper ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE1,      aExpr1 ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE2,      aExpr2 ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_BLANK,       bBlank ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWHELP,    bShowHelp ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTITLE,   aHelpTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTEXT,    aHelpText ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWERR,     bShowError ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE,    eErrStyle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTITLE,    aErrTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTEXT,     aErrText ) );
                        }
                    }

                    ScValidationDlg* pDlg = new ScValidationDlg( NULL, &aArgSet );
                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                        if ( pOutSet->GetItemState( FID_VALID_MODE, TRUE, &pItem ) == SFX_ITEM_SET )
                            eMode = (ScValidationMode) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_CONDMODE, TRUE, &pItem ) == SFX_ITEM_SET )
                            eOper = (ScConditionMode) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_VALUE1, TRUE, &pItem ) == SFX_ITEM_SET )
                            aExpr1 = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_VALUE2, TRUE, &pItem ) == SFX_ITEM_SET )
                            aExpr2 = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_BLANK, TRUE, &pItem ) == SFX_ITEM_SET )
                            bBlank = ((const SfxBoolItem*)pItem)->GetValue();

                        if ( pOutSet->GetItemState( FID_VALID_SHOWHELP, TRUE, &pItem ) == SFX_ITEM_SET )
                            bShowHelp = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_HELPTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
                            aHelpTitle = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_HELPTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
                            aHelpText = ((const SfxStringItem*)pItem)->GetValue();

                        if ( pOutSet->GetItemState( FID_VALID_SHOWERR, TRUE, &pItem ) == SFX_ITEM_SET )
                            bShowError = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRSTYLE, TRUE, &pItem ) == SFX_ITEM_SET )
                            eErrStyle = (ScValidErrorStyle) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
                            aErrTitle = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
                            aErrText = ((const SfxStringItem*)pItem)->GetValue();

                        USHORT nTab = GetViewData()->GetTabNo();
                        ScValidationData aData( eMode, eOper, aExpr1, aExpr2, pDoc, aCursorPos );
                        aData.SetIgnoreBlank( bBlank );
                        if (bShowHelp)
                            aData.SetInput(aHelpTitle, aHelpText);
                        else
                            aData.ResetInput();
                        if (bShowError)
                            aData.SetError(aErrTitle, aErrText, eErrStyle);
                        else
                            aData.ResetError();

                        pTabViewShell->SetValidation( aData );
                        rReq.Done( *pOutSet );
                    }
                    delete pDlg;
                }
            }
            break;
        }
}

void __EXPORT ScCellShell::GetDBState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();
    ScDocument* pDoc        = pDocSh->GetDocument();
    USHORT      nPosX       = pViewData->GetCurX();
    USHORT      nPosY       = pViewData->GetCurY();
    USHORT      nTab        = pViewData->GetTabNo();

    BOOL bAutoFilter;
    BOOL bAutoFilterTested = FALSE;

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_REFRESH_DBAREA:
                {
                    //  importierte Daten ohne Selektion
                    //  oder Filter,Sortierung,Teilergebis (auch ohne Import)
                    BOOL bOk = FALSE;
                    ScDBData* pDBData = pTabViewShell->GetDBData(FALSE,SC_DB_OLD);
                    if (pDBData && pDoc->GetChangeTrack() == NULL)
                    {
                        if ( pDBData->HasImportParam() )
                            bOk = !pDBData->HasImportSelection();
                        else
                        {
                            bOk = pDBData->HasQueryParam() ||
                                  pDBData->HasSortParam() ||
                                  pDBData->HasSubTotalParam();
                            if (!bOk)
                            {
                                //  Pivottabelle mit den Daten als Quellbereich ?
                                ScRange aDataRange;
                                pDBData->GetArea(aDataRange);
                                ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
                                USHORT nCount = pPivotCollection ? pPivotCollection->GetCount() : 0;
                                for (USHORT i=0; i<nCount; i++)
                                {
                                    ScPivot* pTemp = (*pPivotCollection)[i];
                                    if ( pTemp && pTemp->GetSrcArea().Intersects( aDataRange ) )
                                        bOk = TRUE;
                                }
                            }
                        }
                    }
                    if (!bOk)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_FILTER:
            case SID_SPECIAL_FILTER:
                {
                    ScMarkData& rMark = GetViewData()->GetMarkData();

                    if (rMark.IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;


                //Bei Redlining und Multiselektion Disablen
            case SID_SORT_ASCENDING:
            case SID_SORT_DESCENDING:
            case SCITEM_SORTDATA:
            case SCITEM_SUBTDATA:
            case SID_OPENDLG_PIVOTTABLE:
                {
                    //! move ReadOnly check to idl flags

                    ScMarkData& rMark = GetViewData()->GetMarkData();
                    if ( pDocSh->IsReadOnly() || pDoc->GetChangeTrack()!=NULL ||
                            rMark.IsMultiMarked() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_REIMPORT_DATA:
                {
                    //  nur importierte Daten ohne Selektion
                    ScDBData* pDBData = pTabViewShell->GetDBData(FALSE,SC_DB_OLD);
                    if (!pDBData || !pDBData->HasImportParam() || pDBData->HasImportSelection() ||
                        pDoc->GetChangeTrack()!=NULL)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_IMPORT_DATA:
                {
                    if(pDoc->GetChangeTrack()!=NULL)
                        rSet.DisableItem( nWhich );
/*                  Import (F4) ist immer moeglich, Default ist Adressbuch

                    //  importierte Daten auch mit Selektion
                    ScDBData* pDBData = GetDBData(FALSE,SC_DB_OLD);
                    if (!pDBData || !pDBData->HasImportParam())
                        rSet.DisableItem( nWhich );
*/
                }
                break;
            case SID_SBA_BRW_INSERT:
                {
                    //  SBA will ein BOOL-Item, damit ueberhaupt enabled

                    BOOL bEnable = TRUE;
                    rSet.Put(SfxBoolItem(nWhich, bEnable));
                }
                break;

            case SID_AUTO_FILTER:
            case SID_AUTOFILTER_HIDE:
                {
                    if (!bAutoFilterTested)
                    {
                        bAutoFilter = pDoc->HasAutoFilter( nPosX, nPosY, nTab );
                        bAutoFilterTested = TRUE;
                    }
                    if ( nWhich == SID_AUTO_FILTER )
                    {
                        ScMarkData& rMark = GetViewData()->GetMarkData();

                        if (rMark.IsMultiMarked())
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
                            rSet.Put( SfxBoolItem( nWhich, bAutoFilter ) );
                    }
                    else
                        if (!bAutoFilter)
                            rSet.DisableItem( nWhich );
                }
                break;

            case SID_UNFILTER:
                {
                    USHORT nStartCol, nStartRow, nStartTab;
                    USHORT nEndCol,   nEndRow,   nEndTab;
                    BOOL bAnyQuery = FALSE;

                    ScMarkData& rMark = GetViewData()->GetMarkData();
                    BOOL bSelected = rMark.IsMarked();

                    if ( bSelected )
                    {
                        GetViewData()->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                                      nEndCol,   nEndRow,   nEndTab   );
                        if (nStartCol==nEndCol && nStartRow==nEndRow)
                            bSelected = FALSE;
                    }
                    else
                    {
                        nStartCol = GetViewData()->GetCurX();
                        nStartRow = GetViewData()->GetCurY();
                        nStartTab = GetViewData()->GetTabNo();
                    }

                    ScDBData* pDBData = bSelected
                                ? pDoc->GetDBAtArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow )
                                : pDoc->GetDBAtCursor( nStartCol, nStartRow, nStartTab );

                    if ( pDBData )
                    {
                        ScQueryParam aParam;
                        pDBData->GetQueryParam( aParam );
                        if ( aParam.GetEntry(0).bDoQuery )
                            bAnyQuery = TRUE;
                    }

                    if ( !bAnyQuery )
                        rSet.DisableItem( nWhich );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}



