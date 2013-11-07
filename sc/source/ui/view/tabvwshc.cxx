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



//------------------------------------------------------------------

// INCLUDE ---------------------------------------------------------------
#include "scitems.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/dispatch.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "pivot.hxx"
#include "namedlg.hxx"
#include "solvrdlg.hxx"
#include "optsolver.hxx"
#include "tabopdlg.hxx"
#include "autoform.hxx"         // Core
#include "autofmt.hxx"          // Dialog
#include "consdlg.hxx"
//CHINA001 #include "sortdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#include "pvlaydlg.hxx"
#include "areasdlg.hxx"
#include "condfrmt.hxx"
#include "rangeutl.hxx"
#include "crnrdlg.hxx"
#include "formula.hxx"
#include "cell.hxx"             // Input Status Edit-Zellen
#include "acredlin.hxx"
#include "highred.hxx"
#include "simpref.hxx"
#include "funcdesc.hxx"
#include "dpobject.hxx"

//------------------------------------------------------------------

void ScTabViewShell::SetCurRefDlgId( sal_uInt16 nNew )
{
    //  CurRefDlgId is stored in ScModule to find if a ref dialog is open,
    //  and in the view to identify the view that has opened the dialog
    nCurRefDlgId = nNew;
}

SfxModelessDialog* ScTabViewShell::CreateRefDialog(
                        SfxBindings* pB, SfxChildWindow* pCW, SfxChildWinInfo* pInfo,
                        Window* pParent, sal_uInt16 nSlotId )
{
    //  Dialog nur aufmachen, wenn ueber ScModule::SetRefDialog gerufen, damit
    //  z.B. nach einem Absturz offene Ref-Dialoge nicht wiederkommen (#42341#).

    if ( SC_MOD()->GetCurRefDlgId() != nSlotId )
        return NULL;

    if ( nCurRefDlgId != nSlotId )
    {
        //  the dialog has been opened in a different view
        //  -> lock the dispatcher for this view (modal mode)

        GetViewData()->GetDispatcher().Lock( sal_True );    // lock is reset when closing dialog
        return NULL;
    }

    SfxModelessDialog* pResult = 0;

    if(pCW)
        pCW->SetHideNotDelete(sal_True);

    switch( nSlotId )
    {
        case FID_DEFINE_NAME:
        pResult = new ScNameDlg( pB, pCW, pParent, GetViewData(),
                                 ScAddress( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(),
                                            GetViewData()->GetTabNo() ) );
        break;

        case SID_DEFINE_COLROWNAMERANGES:
        {
            pResult = new ScColRowNameRangesDlg( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_OPENDLG_CONSOLIDATE:
        {
            SfxItemSet aArgSet( GetPool(),
                                SCITEM_CONSOLIDATEDATA,
                                SCITEM_CONSOLIDATEDATA );

            const ScConsolidateParam* pDlgData =
                            GetViewData()->GetDocument()->GetConsolidateDlgData();

            if ( !pDlgData )
            {
                ScConsolidateParam  aConsParam;
                SCCOL nStartCol, nEndCol;
                SCROW nStartRow, nEndRow;
                SCTAB nStartTab, nEndTab;

                GetViewData()->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                              nEndCol,   nEndRow,   nEndTab );

                PutInOrder( nStartCol, nEndCol );
                PutInOrder( nStartRow, nEndRow );
                PutInOrder( nStartTab, nEndTab );

                aConsParam.nCol = nStartCol;
                aConsParam.nRow = nStartRow;
                aConsParam.nTab = nStartTab;

                aArgSet.Put( ScConsolidateItem( SCITEM_CONSOLIDATEDATA,
                                                &aConsParam ) );
            }
            else
            {
                aArgSet.Put( ScConsolidateItem( SCITEM_CONSOLIDATEDATA, pDlgData ) );
            }
            pResult = new ScConsolidateDlg( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_DEFINE_DBNAME:
        {
            //  wenn auf einem bestehenden Bereich aufgerufen, den markieren
            GetDBData( sal_True, SC_DB_OLD );
            const ScMarkData& rMark = GetViewData()->GetMarkData();
            if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                MarkDataArea( sal_False );

            pResult = new ScDbNameDlg( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_SPECIAL_FILTER:
        {
            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA );

            ScDBData* pDBData = GetDBData( sal_True, SC_DB_MAKE, SC_DBSEL_ROW_DOWN);
            pDBData->GetQueryParam( aQueryParam );

            ScQueryItem aItem( SCITEM_QUERYDATA, GetViewData(), &aQueryParam );
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
                aItem.SetAdvancedQuerySource( &aAdvSource );

            aArgSet.Put( aItem );

            // aktuelle Tabelle merken (wg. RefInput im Dialog)
            GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );

            pResult = new ScSpecialFilterDlg( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_FILTER:
        {

            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA );

            ScDBData* pDBData = GetDBData( sal_True, SC_DB_MAKE, SC_DBSEL_ROW_DOWN);
            pDBData->GetQueryParam( aQueryParam );

            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA,
                                      GetViewData(),
                                      &aQueryParam ) );

            // aktuelle Tabelle merken (wg. RefInput im Dialog)
            GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );

            pResult = new ScFilterDlg( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_OPENDLG_TABOP:
        {
            ScViewData*  pViewData  = GetViewData();
            ScRefAddress  aCurPos   ( pViewData->GetCurX(),
                                      pViewData->GetCurY(),
                                      pViewData->GetTabNo(),
                                      sal_False, sal_False, sal_False );

            pResult = new ScTabOpDlg( pB, pCW, pParent, pViewData->GetDocument(), aCurPos );
        }
        break;

        case SID_OPENDLG_SOLVE:
        {
            ScViewData*  pViewData  = GetViewData();
            ScAddress aCurPos(  pViewData->GetCurX(),
                                pViewData->GetCurY(),
                                pViewData->GetTabNo());
            pResult = new ScSolverDlg( pB, pCW, pParent, pViewData->GetDocument(), aCurPos );
        }
        break;

        case SID_OPENDLG_OPTSOLVER:
        {
            ScViewData* pViewData = GetViewData();
            ScAddress aCurPos( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());
            pResult = new ScOptSolverDlg( pB, pCW, pParent, pViewData->GetDocShell(), aCurPos );
        }
        break;

        case SID_OPENDLG_PIVOTTABLE:
        {
            //  all settings must be in pDialogDPObject

            if( pDialogDPObject )
            {
                GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );
                pResult = new ScPivotLayoutDlg( pB, pCW, pParent, *pDialogDPObject );
            }
        }
        break;

        case SID_OPENDLG_EDIT_PRINTAREA:
        {
            pResult = new ScPrintAreasDlg( pB, pCW, pParent );
        }
        break;

        case SID_OPENDLG_CONDFRMT:
        {
            ScViewData* pViewData = GetViewData();

            ScDocument* pDoc = pViewData->GetDocument();
            const ScConditionalFormat* pForm = pDoc->GetCondFormat(
                pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() );

            // aktuelle Tabelle merken (wg. RefInput im Dialog)
            pViewData->SetRefTabNo( pViewData->GetTabNo() );

            pResult = new ScConditionalFormatDlg( pB, pCW, pParent, pDoc, pForm );
        }
        break;

        case SID_OPENDLG_FUNCTION:
        {
            //  Dialog schaut selber, was in der Zelle steht

            pResult = new ScFormulaDlg( pB, pCW, pParent, GetViewData(),ScGlobal::GetStarCalcFunctionMgr() );
        }
        break;

        case FID_CHG_SHOW:
        {
            //  Dialog schaut selber, was in der Zelle steht

            pResult = new ScHighlightChgDlg( pB, pCW, pParent, GetViewData() );
        }
        break;

        case WID_SIMPLE_REF:
        {
            //  Dialog schaut selber, was in der Zelle steht

            ScViewData* pViewData = GetViewData();
            pViewData->SetRefTabNo( pViewData->GetTabNo() );
            pResult = new ScSimpleRefDlg( pB, pCW, pParent, pViewData );
        }
        break;


        default:
        DBG_ERROR( "ScTabViewShell::CreateRefDialog: unbekannte ID" );
        break;
    }

    if (pResult)
    {
        //  Die Dialoge gehen immer mit eingeklapptem Zusaetze-Button auf,
        //  darum muss die Groesse ueber das Initialize gerettet werden
        //  (oder den Zusaetze-Status mit speichern !!!)

        Size aSize = pResult->GetSizePixel();
        pResult->Initialize( pInfo );
        pResult->SetSizePixel(aSize);
    }

    return pResult;
}



