/*************************************************************************
 *
 *  $RCSfile: tabvwshc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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
//#define _SVTABBX_HXX
//#define _SVTREEBOX_HXX
//#define _SVTREELIST_HXX

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
#include <vcl/msgbox.hxx>
#include <sfx2/childwin.hxx>

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
#include "tabopdlg.hxx"
#include "autoform.hxx"         // Core
#include "autofmt.hxx"          // Dialog
#include "consdlg.hxx"
#include "sortdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#include "pvlaydlg.hxx"
#include "chartdlg.hxx"
#include "areasdlg.hxx"
#include "condfrmt.hxx"
#include "rangeutl.hxx"
#include "crnrdlg.hxx"
#include "formula.hxx"
#include "cell.hxx"             // Input Status Edit-Zellen
#include "acredlin.hxx"
#include "highred.hxx"
#include "simpref.hxx"
#include "dpobject.hxx"

// -----------------------------------------------------------------------

//!     nach document verschieben !!!

BOOL lcl_IsValueCol( ScDocument* pDoc, USHORT nCol, USHORT nRow1, USHORT nRow2, USHORT nTab )
{
    BOOL bVal = TRUE;
    for (USHORT nRow=nRow1; nRow<=nRow2 && bVal; nRow++)
        if (pDoc->HasStringData( nCol, nRow, nTab ))
            bVal = FALSE;
    return bVal;
}

//------------------------------------------------------------------

SfxModelessDialog* ScTabViewShell::CreateRefDialog(
                        SfxBindings* pB, SfxChildWindow* pCW, SfxChildWinInfo* pInfo,
                        Window* pParent, USHORT nSlotId )
{
    //  Dialog nur aufmachen, wenn ueber ScModule::SetRefDialog gerufen, damit
    //  z.B. nach einem Absturz offene Ref-Dialoge nicht wiederkommen (#42341#).

    if ( SC_MOD()->GetCurRefDlgId() != nSlotId )
        return NULL;

    SfxModelessDialog* pResult = 0;

    if(pCW)
        pCW->SetHideNotDelete(TRUE);

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

        case SID_OPENDLG_CHART:
        {
            pResult = new ScChartDlg( pB, pCW, pParent, GetViewData() );
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
                USHORT              nStartCol, nStartRow, nStartTab,
                                    nEndCol,   nEndRow,   nEndTab;

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
            GetDBData( TRUE, SC_DB_OLD );
            if ( !GetViewData()->GetMarkData().IsMarked() )
                MarkDataArea( FALSE );

            pResult = new ScDbNameDlg( pB, pCW, pParent, GetViewData() );
        }
        break;

        case SID_SPECIAL_FILTER:
        {
            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA );

            ScDBData* pDBData = GetDBData();
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

            ScDBData* pDBData = GetDBData();
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
            ScRefTripel  aCurPos    ( pViewData->GetCurX(),
                                      pViewData->GetCurY(),
                                      pViewData->GetTabNo(),
                                      FALSE, FALSE, FALSE );

            pResult = new ScTabOpDlg( pB, pCW, pParent, pViewData->GetDocument(), aCurPos );
        }
        break;

        case SID_OPENDLG_SOLVE:
        {
            ScViewData*  pViewData  = GetViewData();
            ScRefTripel  aCurPos    ( pViewData->GetCurX(),
                                      pViewData->GetCurY(),
                                      pViewData->GetTabNo(),
                                      FALSE, FALSE, FALSE );

            pResult = new ScSolverDlg( pB, pCW, pParent, pViewData->GetDocument(), aCurPos );
        }
        break;

        case SID_OPENDLG_PIVOTTABLE:
        {
            //  all settings must be in pDialogDPObject

            GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );
            pResult = new ScPivotLayoutDlg( pB, pCW, pParent, pDialogDPObject );

#if 0
            ScDocument*     pDoc = GetViewData()->GetDocument();
            USHORT          nTab  = 0;
            USHORT          nCol1 = 0;
            USHORT          nCol2 = 0;
            USHORT          nRow1 = 0;
            USHORT          nRow2 = 0;

            ScPivot*        pPivot = pDoc->GetPivotAtCursor(
                                        GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(),
                                        GetViewData()->GetTabNo() );
            ScDPObject*     pDPObj = pDoc->GetDPAtCursor(
                                        GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(),
                                        GetViewData()->GetTabNo() );
            ScPivotParam    aPivotParam;

            if ( pPivot ) // befinden wir uns auf einer Pivot-Tabelle?
            {
                ScArea aArea;
                ScQueryParam aQuery;
                pPivot->GetParam( aPivotParam, aQuery, aArea );
                pPivot->GetSrcArea( nCol1, nRow1, nCol2, nRow2, nTab);

                // Quelldatenbereich markieren
                if ( nTab == GetViewData()->GetTabNo() )
                    MarkRange( ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab ) );
            }
            else if ( pDPObj && pDPObj->IsSheetData() )
            {
                //  get (old) ScPivotParam

                pDPObj->FillOldParam( aPivotParam );
                ScRange aSource = pDPObj->GetSourceRange();
                nCol1 = aSource.aStart.Col();
                nRow1 = aSource.aStart.Row();
                nCol2 = aSource.aEnd.Col();
                nRow2 = aSource.aEnd.Row();
                nTab  = aSource.aStart.Tab();

                if ( nTab == GetViewData()->GetTabNo() )
                    MarkRange( ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab ) );
            }
            else // neue Pivot-Tabelle erzeugen
            {
                ScDBData*   pDBData = GetDBData();
                String      aErrMsg;

                pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );


                if (nRow2+2 <= MAXROW - 4)              // Default-Ausgabebereich
                {                                       // min. Tabelle hat 5 Zeilen
                    aPivotParam.nCol = nCol1;
                    aPivotParam.nRow = nRow2+2;
                    aPivotParam.nTab = nTab;
                }
                else
                {
                    aPivotParam.nCol = 0;
                    aPivotParam.nRow = 0;
                    aPivotParam.nTab = MAXTAB+1;
                }

            }

            SfxItemSet aArgSet( GetPool(),
                                SCITEM_PIVOTDATA,
                                SCITEM_PIVOTDATA );

            // Ermitteln der Ueberschriften:
            String      aFieldName;
            USHORT      nLabelCount = nCol2-nCol1+1;
            short       nCol        = nCol1;
            BOOL        bIsValue    = FALSE;
            LabelData** aLabelArr   = new LabelData*[nLabelCount];

            for ( USHORT i=0; i<nLabelCount; i++ )
            {
                pDoc->GetString( nCol, nRow1, nTab, aFieldName );
                if ( !aFieldName )
                    aFieldName = ColToAlpha( nCol );
                bIsValue = lcl_IsValueCol( pDoc, nCol, nRow1+1, nRow2, nTab );
                aLabelArr[i] = new LabelData( aFieldName, nCol, bIsValue );
                nCol++;
            }

            aPivotParam.SetLabelData( aLabelArr, nLabelCount );
            aArgSet.Put( ScPivotItem( SCITEM_PIVOTDATA, &aPivotParam ) );

            // aktuelle Tabelle merken (wg. RefInput im Dialog)
            GetViewData()->SetRefTabNo( GetViewData()->GetTabNo() );

            *pPivotSource = ScArea( nTab, nCol1,nRow1, nCol2,nRow2 );
            pResult = new ScPivotLayoutDlg( pB, pCW, pParent, aArgSet );


            for ( USHORT p=0; p<nLabelCount; p++ )
                delete aLabelArr[p];
            delete [] aLabelArr;
#endif
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

            pResult = new ScFormulaDlg( pB, pCW, pParent, GetViewData() );
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

            pResult = new ScSimpleRefDlg( pB, pCW, pParent, GetViewData() );
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



