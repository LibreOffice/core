/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_mpl.h>

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
#include "namedefdlg.hxx"
#include "solvrdlg.hxx"
#include "optsolver.hxx"
#include "tabopdlg.hxx"
#include "autoform.hxx"         // Core
#include "autofmt.hxx"          // Dialog
#include "consdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#if ! MPL_HAVE_SUBSET
#  include "pvlaydlg.hxx"
#endif
#include "areasdlg.hxx"
#include "rangeutl.hxx"
#include "crnrdlg.hxx"
#include "formula.hxx"
#include "formulacell.hxx"             // Input Status Edit-Zellen
#include "acredlin.hxx"
#include "highred.hxx"
#include "simpref.hxx"
#include "funcdesc.hxx"
#include "dpobject.hxx"
#include "markdata.hxx"
#include "reffact.hxx"
#include "condformatdlg.hxx"
#include "xmlsourcedlg.hxx"

#include "RandomNumberGeneratorDialog.hxx"
#include "SamplingDialog.hxx"
#include "DescriptiveStatisticsDialog.hxx"
#include "AnalysisOfVarianceDialog.hxx"
#include "CorrelationDialog.hxx"
#include "CovarianceDialog.hxx"

//------------------------------------------------------------------

void ScTabViewShell::SetCurRefDlgId( sal_uInt16 nNew )
{
    //  CurRefDlgId is stored in ScModule to find if a ref dialog is open,
    //  and in the view to identify the view that has opened the dialog
    nCurRefDlgId = nNew;
}

//ugly hack to call Define Name from Manage Names
void ScTabViewShell::SwitchBetweenRefDialogs(SfxModelessDialog* pDialog)
{
   sal_uInt16 nSlotId = SC_MOD()->GetCurRefDlgId();
   if (nSlotId == FID_DEFINE_NAME)
   {
        mbInSwitch = true;
        static_cast<ScNameDlg*>(pDialog)->GetRangeNames(maRangeMap);
        static_cast<ScNameDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDefDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd ? false : sal_True );
   }
   else if( nSlotId == FID_ADD_NAME )
   {
        static_cast<ScNameDefDlg*>(pDialog)->GetNewData(maName, maScope);
        static_cast<ScNameDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd ? false : sal_True );
   }
   else
   {

   }
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

    ScDocument* pDoc = GetViewData()->GetDocument();

    switch( nSlotId )
    {
        case FID_DEFINE_NAME:
        {
            if (!mbInSwitch)
            {
                pResult = new ScNameDlg( pB, pCW, pParent, GetViewData(),
                                     ScAddress( GetViewData()->GetCurX(),
                                                GetViewData()->GetCurY(),
                                                GetViewData()->GetTabNo() ) );
            }
            else
            {
                pResult = new ScNameDlg( pB, pCW, pParent, GetViewData(),
                                     ScAddress( GetViewData()->GetCurX(),
                                                GetViewData()->GetCurY(),
                                                GetViewData()->GetTabNo() ), &maRangeMap);
                static_cast<ScNameDlg*>(pResult)->SetEntry( maName, maScope);
                mbInSwitch = false;
            }
        }
        break;

        case FID_ADD_NAME:
        {
            if (!mbInSwitch)
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                pDoc->GetRangeNameMap(aRangeMap);
                pResult = new ScNameDefDlg( pB, pCW, pParent, GetViewData(), aRangeMap,
                                ScAddress( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(),
                                            GetViewData()->GetTabNo() ), true );
            }
            else
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                for (boost::ptr_map<OUString, ScRangeName>::iterator itr = maRangeMap.begin();
                        itr != maRangeMap.end(); ++itr)
                {
                    aRangeMap.insert(std::pair<OUString, ScRangeName*>(itr->first, itr->second));
                }
                pResult = new ScNameDefDlg( pB, pCW, pParent, GetViewData(), aRangeMap,
                                ScAddress( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(),
                                            GetViewData()->GetTabNo() ), false );
            }
        }
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
                            pDoc->GetConsolidateDlgData();

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
                MarkDataArea( false );

            pResult = new ScDbNameDlg( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_SPECIAL_FILTER:
        {
            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA );

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, SC_DBSEL_ROW_DOWN);
            pDBData->ExtendDataArea(pDoc);
            pDBData->GetQueryParam( aQueryParam );

            ScRange aArea;
            pDBData->GetArea(aArea);
            MarkRange(aArea, false);

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

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, SC_DBSEL_ROW_DOWN);
            pDBData->ExtendDataArea(pDoc);
            pDBData->GetQueryParam( aQueryParam );

            ScRange aArea;
            pDBData->GetArea(aArea);
            MarkRange(aArea, false);

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
                                      false, false, false );

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

        case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
        {
            pResult = new ScRandomNumberGeneratorDialog( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_SAMPLING_DIALOG:
        {
            pResult = new ScSamplingDialog( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_DESCRIPTIVE_STATISTICS_DIALOG:
        {
            pResult = new ScDescriptiveStatisticsDialog( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_ANALYSIS_OF_VARIANCE_DIALOG:
        {
            pResult = new ScAnalysisOfVarianceDialog( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_CORRELATION_DIALOG:
        {
            pResult = new ScCorrelationDialog( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_COVARIANCE_DIALOG:
        {
            pResult = new ScCovarianceDialog( pB, pCW, pParent, GetViewData() );
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
#if ! MPL_HAVE_SUBSET
            //  all settings must be in pDialogDPObject

            if( pDialogDPObject )
            {
                // Check for an existing datapilot output.
                ScViewData* pViewData = GetViewData();
                ScDPObject* pObj = pDoc->GetDPAtCursor(
                    pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());

                GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );
                pResult = new ScPivotLayoutDlg( pB, pCW, pParent, *pDialogDPObject, pObj == NULL);
            }
#endif
        }
        break;

        case SID_OPENDLG_EDIT_PRINTAREA:
        {
            pResult = new ScPrintAreasDlg( pB, pCW, pParent );
        }
        break;

        case SID_OPENDLG_FUNCTION:
        {
            //  Dialog schaut selber, was in der Zelle steht

            pResult = new ScFormulaDlg( pB, pCW, pParent, GetViewData(),ScGlobal::GetStarCalcFunctionMgr() );
        }
        break;

        case SID_MANAGE_XML_SOURCE:
        {
            pResult = new ScXMLSourceDlg(pB, pCW, pParent, pDoc);
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
        OSL_FAIL( "ScTabViewShell::CreateRefDialog: unbekannte ID" );
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
