/*************************************************************************
 *
 *  $RCSfile: cellsh3.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:14:06 $
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

#include "scitems.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "globstr.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "reffact.hxx"
#include "uiitems.hxx"
//CHINA001 #include "scendlg.hxx"
//CHINA001 #include "mtrindlg.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "cellsh.hxx"
#include "attrdlg.hrc"      // TP_ALIGNMENT
#include "inputhdl.hxx"
#include "editable.hxx"

#include "scabstdlg.hxx" //CHINA001

#define IS_EDITMODE() GetViewData()->HasEditView( GetViewData()->GetActivePart() )

inline long TwipsToHMM(long nTwips) { return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)   { return (nHMM * 72 + 63) / 127; }
inline long TwipsToEvenHMM(long nTwips) { return ( (nTwips * 127 + 72) / 144 ) * 2; }

//------------------------------------------------------------------

void ScCellShell::Execute( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SfxBindings&        rBindings   = pTabViewShell->GetViewFrame()->GetBindings();
    SfxApplication*     pSfxApp     = SFX_APP();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // der kommt beim MouseButtonUp
        pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    if ( IS_EDITMODE() )
    {
        switch ( nSlot )
        {
            //  beim Oeffnen eines Referenz-Dialogs darf die SubShell nicht umgeschaltet werden
            //  (beim Schliessen des Dialogs wird StopEditShell gerufen)
            case SID_OPENDLG_FUNCTION:
            case SID_OPENDLG_CHART:
                    //  #53318# inplace macht die EditShell Aerger...
                    //! kann nicht immer umgeschaltet werden ????
                    if (!pTabViewShell->GetViewFrame()->ISA(SfxInPlaceFrame))
                        pTabViewShell->SetDontSwitch(TRUE);         // EditShell nicht abschalten
                    // kein break

            case FID_CELL_FORMAT:
            case SID_ENABLE_HYPHENATION:
            case SID_DATA_SELECT:
            case SID_OPENDLG_CONSOLIDATE:
            case SID_OPENDLG_SOLVE:

                    pScMod->InputEnterHandler();
                    pTabViewShell->UpdateInputHandler();

                    pTabViewShell->SetDontSwitch(FALSE);

                    break;

            default:
                    break;
        }
    }

    switch ( nSlot )
    {



        case SID_ATTR_SIZE://XXX ???
            break;

        case SID_STATUS_SELMODE:
            if ( pReqArgs )
            {
                /* 0: STD   Click hebt Sel auf
                 * 1: ER    Click erweitert Selektion
                 * 2: ERG   Click definiert weitere Selektion
                 */
                UINT16 nMode = ((const SfxUInt16Item&)pReqArgs->Get( nSlot )).GetValue();

                switch ( nMode )
                {
                    case 1: nMode = KEY_SHIFT;  break;
                    case 2: nMode = KEY_MOD1;   break; // Control-Taste
                    case 0:
                    default:
                        nMode = 0;
                }

                pTabViewShell->LockModifiers( nMode );
            }
            else
            {
                //  no arguments (also executed by double click on the status bar controller):
                //  advance to next selection mode

                USHORT nModifiers = pTabViewShell->GetLockedModifiers();
                switch ( nModifiers )
                {
                    case KEY_SHIFT: nModifiers = KEY_MOD1;  break;      // EXT -> ADD
                    case KEY_MOD1:  nModifiers = 0;         break;      // ADD -> STD
                    default:        nModifiers = KEY_SHIFT; break;      // STD -> EXT
                }
                pTabViewShell->LockModifiers( nModifiers );
            }

            rBindings.Invalidate( SID_STATUS_SELMODE );
            rReq.Done();
            break;

        //  SID_STATUS_SELMODE_NORM wird nicht benutzt ???

        case SID_STATUS_SELMODE_NORM:
            pTabViewShell->LockModifiers( 0 );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        //  SID_STATUS_SELMODE_ERG / SID_STATUS_SELMODE_ERW als Toggles:

        case SID_STATUS_SELMODE_ERG:
            if ( pTabViewShell->GetLockedModifiers() & KEY_MOD1 )
                pTabViewShell->LockModifiers( 0 );
            else
                pTabViewShell->LockModifiers( KEY_MOD1 );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        case SID_STATUS_SELMODE_ERW:
            if ( pTabViewShell->GetLockedModifiers() & KEY_SHIFT )
                pTabViewShell->LockModifiers( 0 );
            else
                pTabViewShell->LockModifiers( KEY_SHIFT );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        case SID_ENTER_STRING:
            {
                if ( pReqArgs )
                {
                    String aStr( ((const SfxStringItem&)pReqArgs->
                                    Get( SID_ENTER_STRING )).GetValue() );

                    pTabViewShell->EnterData( GetViewData()->GetCurX(),
                                               GetViewData()->GetCurY(),
                                               GetViewData()->GetTabNo(),
                                               aStr );

                    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pTabViewShell );
                    if ( !pHdl || !pHdl->IsInEnterHandler() )
                    {
                        //  #101061# UpdateInputHandler is needed after the cell content
                        //  has changed, but if called from EnterHandler, UpdateInputHandler
                        //  will be called later when moving the cursor.

                        pTabViewShell->UpdateInputHandler();
                    }

                    rReq.Done();

                    //  hier kein GrabFocus, weil sonst auf dem Mac die Tabelle vor die
                    //  Seitenansicht springt, wenn die Eingabe nicht abgeschlossen war
                    //  (GrabFocus passiert in KillEditView)
                }
            }
            break;

        case SID_INSERT_MATRIX:
            {
                if ( pReqArgs )
                {
                    String aStr = ((const SfxStringItem&)pReqArgs->
                                    Get( SID_INSERT_MATRIX )).GetValue();
                    pTabViewShell->EnterMatrix( aStr );
                    rReq.Done();
                }
            }
            break;

        case FID_INPUTLINE_ENTER:
        case FID_INPUTLINE_BLOCK:
        case FID_INPUTLINE_MATRIX:
            {
                if( pReqArgs == 0 ) //XXX vorlaufiger HACK um GPF zu vermeiden
                    break;

                const ScInputStatusItem* pStatusItem
                    = (const ScInputStatusItem*)&pReqArgs->
                            Get( FID_INPUTLINE_STATUS );

                ScAddress aCursorPos = pStatusItem->GetPos();
                const EditTextObject* pData = pStatusItem->GetEditData();
                if (pData)
                {
                    if (nSlot == FID_INPUTLINE_BLOCK)
                        pTabViewShell->EnterBlock( String(), pData );
                    else
                        pTabViewShell->EnterData( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), pData );
                }
                else
                {
                    String aString = pStatusItem->GetString();
                    if (nSlot == FID_INPUTLINE_ENTER)
                    {
                        if (
                            aCursorPos.Col() == GetViewData()->GetCurX() &&
                            aCursorPos.Row() == GetViewData()->GetCurY() &&
                            aCursorPos.Tab() == GetViewData()->GetTabNo()
                            )
                        {
                            SfxStringItem   aItem( SID_ENTER_STRING, aString );

                            SfxBindings& rBindings = pTabViewShell->GetViewFrame()->GetBindings();
                            const SfxPoolItem* aArgs[2];
                            aArgs[0] = &aItem;
                            aArgs[1] = NULL;
                            rBindings.Execute( SID_ENTER_STRING, aArgs );
                        }
                        else
                        {
                            pTabViewShell->EnterData( aCursorPos.Col(),
                                                    aCursorPos.Row(),
                                                    aCursorPos.Tab(),
                                                    aString );
                            rReq.Done();
                        }
                    }
                    else if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, NULL );
                        rReq.Done();
                    }
                    else
                    {
                        pTabViewShell->EnterMatrix( aString );
                        rReq.Done();
                    }

                }

                //  hier kein GrabFocus, weil sonst auf dem Mac die Tabelle vor die
                //  Seitenansicht springt, wenn die Eingabe nicht abgeschlossen war
                //  (GrabFocus passiert in KillEditView)
            }
            break;


        case SID_OPENDLG_CHART:
            {
                BOOL bChartDlgIsEdit = ( nSlot == SID_OPENDLG_MODCHART );
                pTabViewShell->SetChartDlgEdit(bChartDlgIsEdit);

                if (bChartDlgIsEdit)
                {
                    pTabViewShell->SetEditChartName(pTabViewShell->GetSelectedChartName());
                    pTabViewShell->DrawDeselectAll();       // flackert sonst bei Ref-Input
                }

                pTabViewShell->ResetChartArea();

                USHORT          nId  = ScChartDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case SID_OPENDLG_FUNCTION:
            {
                USHORT nId = SID_OPENDLG_FUNCTION;
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
                rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONSOLIDATE:
            {
                USHORT          nId  = ScConsolidateDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case FID_CELL_FORMAT:
            {
                if ( pReqArgs != NULL )
                {
                    //----------------------------------
                    // Zellattribute ohne Dialog setzen:
                    //----------------------------------
                    SfxItemSet*     pEmptySet =
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );

                    SfxItemSet*     pNewSet =
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );

                    const SfxPoolItem*  pAttr = NULL;
                    USHORT              nWhich = 0;

                    for ( nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
                        if ( pReqArgs->GetItemState( nWhich, TRUE, &pAttr ) == SFX_ITEM_SET )
                            pNewSet->Put( *pAttr );

                    pTabViewShell->ApplyAttributes( pNewSet, pEmptySet );

                    delete pNewSet;
                    delete pEmptySet;

                    rReq.Done();
                }
                else if ( pReqArgs == NULL )
                {
                    pTabViewShell->ExecuteCellFormatDlg( rReq );
                }
            }
            break;

        case SID_ENABLE_HYPHENATION:
            pTabViewShell->ExecuteCellFormatDlg( rReq, TP_ALIGNMENT );
            break;

        case SID_OPENDLG_SOLVE:
            {
                USHORT          nId  = ScSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case SID_OPENDLG_TABOP:
            {
                USHORT          nId  = ScTabOpDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case SID_SCENARIOS:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                ScMarkData& rMark = GetViewData()->GetMarkData();
                SCTAB nTab = GetViewData()->GetTabNo();

                if ( pDoc->IsScenario(nTab) )
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {
                        if (   rReq.IsAPI()
                            || RET_YES ==
                               QueryBox( pTabViewShell->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                         ScGlobal::GetRscString(STR_UPDATE_SCENARIO) ).
                                        Execute() )
                        {
                            pTabViewShell->ExtendScenario();
                            rReq.Done();
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        ErrorBox aErrorBox( pTabViewShell->GetDialogParent(), WinBits(WB_OK | WB_DEF_OK),
                                            ScGlobal::GetRscString(STR_NOAREASELECTED) );
                        aErrorBox.Execute();
                    }
                }
                else
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {
                        SCTAB i=1;
                        String aBaseName;
                        String aName;
                        String aComment;
                        Color  aColor;
                        USHORT nFlags;

                        pDoc->GetName( nTab, aBaseName );
                        aBaseName += '_';
                        aBaseName += ScGlobal::GetRscString(STR_SCENARIO);
                        aBaseName += '_';

                        //  vorneweg testen, ob der Prefix als gueltig erkannt wird
                        //  wenn nicht, nur doppelte vermeiden
                        BOOL bPrefix = pDoc->ValidTabName( aBaseName );
                        DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");

                        while ( pDoc->IsScenario(nTab+i) )
                            i++;

                        BOOL bValid;
                        SCTAB nDummy;
                        do
                        {
                            aName = aBaseName;
                            aName += String::CreateFromInt32( i );
                            if (bPrefix)
                                bValid = pDoc->ValidNewTabName( aName );
                            else
                                bValid = !pDoc->GetTable( aName, nDummy );
                            ++i;
                        }
                        while ( !bValid && i <= 2*MAXTAB );

                        if ( pReqArgs != NULL )
                        {
                            String aName;
                            String aComment;
                            const SfxPoolItem* pItem;
                            if ( pReqArgs->GetItemState( SID_SCENARIOS, TRUE, &pItem ) == SFX_ITEM_SET )
                                aName = ((const SfxStringItem*)pItem)->GetValue();
                            if ( pReqArgs->GetItemState( SID_NEW_TABLENAME, TRUE, &pItem ) == SFX_ITEM_SET )
                                aComment = ((const SfxStringItem*)pItem)->GetValue();

                            aColor = Color( COL_LIGHTGRAY );        // Default
                            nFlags = 0;                             // nicht-TwoWay

                            pTabViewShell->MakeScenario( aName, aComment, aColor, nFlags );
                            if( ! rReq.IsAPI() )
                                rReq.Done();
                        }
                        else
                        {
                            BOOL bSheetProtected = pDoc->IsTabProtected(nTab);
                            //CHINA001 ScNewScenarioDlg* pNewDlg =
                            //CHINA001  new ScNewScenarioDlg( pTabViewShell->GetDialogParent(), aName, FALSE, bSheetProtected );
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                            AbstractScNewScenarioDlg* pNewDlg = pFact->CreateScNewScenarioDlg( pTabViewShell->GetDialogParent(), aName, ResId(RID_SCDLG_NEWSCENARIO), FALSE,bSheetProtected);
                            DBG_ASSERT(pNewDlg, "Dialog create fail!");//CHINA001
                            if ( pNewDlg->Execute() == RET_OK )
                            {
                                pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                pTabViewShell->MakeScenario( aName, aComment, aColor, nFlags );

                                rReq.AppendItem( SfxStringItem( SID_SCENARIOS, aName ) );
                                rReq.AppendItem( SfxStringItem( SID_NEW_TABLENAME, aComment ) );
                                rReq.Done();
                            }
                            delete pNewDlg;
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        pTabViewShell->ErrorMessage(STR_ERR_NEWSCENARIO);
                    }
                }
            }
            break;


        case SID_SELECTALL:
            {
                pTabViewShell->SelectAll();
                rReq.Done();
            }
            break;

        //----------------------------------------------------------------

        case FID_ROW_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_ROW_HEIGHT );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_DIRECT, HMMToTwips(rUInt16Item.GetValue()) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    ScViewData* pViewData  = GetViewData();
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    USHORT      nCurHeight = pViewData->GetDocument()->
                                                GetRowHeight( pViewData->GetCurY(),
                                                              pViewData->GetTabNo() );
//CHINA001                  ScMetricInputDlg* pDlg =
//CHINA001                  new ScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_MAN,
//CHINA001                  nCurHeight,
//CHINA001                  ScGlobal::nStdRowHeight,
//CHINA001                  eMetric,
//CHINA001                  2,
//CHINA001                  MAX_COL_HEIGHT );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_MAN,
                                                                                    nCurHeight,
                                                                                    ScGlobal::nStdRowHeight,
                                                                                    ResId(RID_SCDLG_ROW_MAN),
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_COL_HEIGHT);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_DIRECT, (USHORT)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_HEIGHT, (USHORT)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_ROW_OPT_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_ROW_OPT_HEIGHT );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_OPTIMAL, HMMToTwips(rUInt16Item.GetValue()) );
                    ScGlobal::nLastRowHeightExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

//CHINA001                  ScMetricInputDlg* pDlg =
//CHINA001                  new ScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_OPT,
//CHINA001                  ScGlobal::nLastRowHeightExtra,
//CHINA001                  0,
//CHINA001                  eMetric,
//CHINA001                  1,
//CHINA001                  MAX_EXTRA_HEIGHT );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_OPT,
                                                                                    ScGlobal::nLastRowHeightExtra,
                                                                                    0,
                                                                                    ResId(RID_SCDLG_ROW_OPT),
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_HEIGHT);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_OPTIMAL, (USHORT)nVal );
                        ScGlobal::nLastRowHeightExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_OPT_HEIGHT, (USHORT)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_COL_WIDTH );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_DIRECT, HMMToTwips(rUInt16Item.GetValue()) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    ScViewData* pViewData  = GetViewData();
                    USHORT      nCurHeight = pViewData->GetDocument()->
                                                GetColWidth( pViewData->GetCurX(),
                                                             pViewData->GetTabNo() );
//CHINA001                  ScMetricInputDlg* pDlg =
//CHINA001                  new ScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_MAN,
//CHINA001                  nCurHeight,
//CHINA001                  STD_COL_WIDTH,
//CHINA001                  eMetric,
//CHINA001                  2,
//CHINA001                  MAX_COL_WIDTH );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_MAN,
                                                                                    nCurHeight,
                                                                                    STD_COL_WIDTH,
                                                                                    ResId(RID_SCDLG_COL_MAN),
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_COL_WIDTH);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_DIRECT, (USHORT)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_WIDTH, (USHORT)TwipsToEvenHMM(nVal)) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_OPT_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_COL_OPT_WIDTH );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_OPTIMAL, HMMToTwips(rUInt16Item.GetValue()) );
                    ScGlobal::nLastColWidthExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

//CHINA001                  ScMetricInputDlg* pDlg =
//CHINA001                  new ScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_OPT,
//CHINA001                  ScGlobal::nLastColWidthExtra,
//CHINA001                  STD_EXTRA_WIDTH,
//CHINA001                  eMetric,
//CHINA001                  1,
//CHINA001                  MAX_EXTRA_WIDTH );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_OPT,
                                                                                    ScGlobal::nLastColWidthExtra,
                                                                                    STD_EXTRA_WIDTH,
                                                                                    ResId(RID_SCDLG_COL_OPT),
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_WIDTH);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_OPTIMAL, (USHORT)nVal );
                        ScGlobal::nLastColWidthExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_OPT_WIDTH, (USHORT)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();
                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_OPT_DIRECT:
            pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH );
            rReq.Done();
            break;

        case FID_ROW_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_ROW_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( FALSE, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;
        case FID_COL_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_COL_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( TRUE, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;

        //----------------------------------------------------------------


        case SID_CELL_FORMAT_RESET:
            {
                pTabViewShell->DeleteContents( IDF_HARDATTR | IDF_EDITATTR );
                rReq.Done();
            }
            break;

        case FID_MERGE_ON:
            {
                if ( !GetViewData()->GetDocument()->GetChangeTrack() )
                {
                    BOOL bMoveContents = FALSE;
                    BOOL bApi = rReq.IsAPI();
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                        pReqArgs->GetItemState(FID_MERGE_ON, TRUE, &pItem) == SFX_ITEM_SET )
                    {
                        DBG_ASSERT(pItem && pItem->ISA(SfxBoolItem), "falsches Item");
                        bMoveContents = ((const SfxBoolItem*)pItem)->GetValue();
                    }

                    if (pTabViewShell->MergeCells( bApi, bMoveContents ))
                    {
                        if (!bApi && bMoveContents)             // "ja" im Dialog geklickt
                        {
                            rReq.AppendItem( SfxBoolItem( FID_MERGE_ON, bMoveContents ) );
                        }
                        rReq.Done();
                    }
                }
            }
            break;
        case FID_MERGE_OFF:
            if ( !GetViewData()->GetDocument()->GetChangeTrack() )
            {
                if (pTabViewShell->RemoveMerge())
                    rReq.Done();
            }
            break;

        case SID_AUTOFORMAT:
            {
                Window* pDlgParent = pTabViewShell->GetDialogParent();
                SCCOL nStartCol;
                SCROW nStartRow;
                SCTAB nStartTab;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nEndTab;

                const ScMarkData& rMark = GetViewData()->GetMarkData();
                if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                    pTabViewShell->MarkDataArea( TRUE );

                GetViewData()->GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );

                if (   ( Abs((SCsCOL)nEndCol-(SCsCOL)nStartCol) > 1 )
                    && ( Abs((SCsROW)nEndRow-(SCsROW)nStartRow) > 1 ) )
                {
                    if ( pReqArgs )
                    {
                        const SfxStringItem& rNameItem = (const SfxStringItem&)pReqArgs->Get( SID_AUTOFORMAT );
                        ScAutoFormat* pFormat = ScGlobal::GetAutoFormat();
                        USHORT nIndex = pFormat->FindIndexPerName( rNameItem.GetValue() );

                        pTabViewShell->AutoFormat( nIndex );

                        if( ! rReq.IsAPI() )
                            rReq.Done();
                    }
                    else
                    {
                        ScGlobal::ClearAutoFormat();
                        ScAutoFormatData* pNewEntry = pTabViewShell->CreateAutoFormatData();
//CHINA001                      ScAutoFormatDlg*  pDlg      = new ScAutoFormatDlg(
//CHINA001                      pDlgParent,
//CHINA001                      ScGlobal::GetAutoFormat(),
//CHINA001                      pNewEntry,
//CHINA001                      GetViewData()->GetDocument() );
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                        AbstractScAutoFormatDlg* pDlg = pFact->CreateScAutoFormatDlg( pDlgParent, ScGlobal::GetAutoFormat(), pNewEntry,GetViewData()->GetDocument(), ResId(RID_SCDLG_AUTOFORMAT) );
                        DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                        if ( pDlg->Execute() == RET_OK )
                        {
                            ScEditableTester aTester( pTabViewShell );
                            if ( !aTester.IsEditable() )
                            {
                                pTabViewShell->ErrorMessage(aTester.GetMessageId());
                            }
                            else
                            {
                                pTabViewShell->AutoFormat( pDlg->GetIndex() );

                                rReq.AppendItem( SfxStringItem( SID_AUTOFORMAT, pDlg->GetCurrFormatName() ) );
                                rReq.Done();
                            }
                        }
                        delete pDlg;
                        delete pNewEntry;
                    }
                }
                else
                    ErrorBox( pDlgParent, WinBits( WB_OK | WB_DEF_OK ),
                              ScGlobal::GetRscString(STR_INVALID_AFAREA) ).Execute();
            }
            break;

        case SID_CANCEL:
            {
                if (GetViewData()->HasEditView(GetViewData()->GetActivePart()))
                    pScMod->InputCancelHandler();
                else if (pTabViewShell->HasPaintBrush())
                    pTabViewShell->ResetBrushDocument();            // abort format paint brush
                else if (pTabViewShell->HasHintWindow())
                    pTabViewShell->RemoveHintWindow();              // Eingabemeldung abschalten
                else
                {
                    SfxObjectShell* pObjSh = GetViewData()->GetSfxDocShell();
                    if (pObjSh->GetInPlaceObject() &&
                        pObjSh->GetInPlaceObject()->GetIPClient())
                    {
                        GetViewData()->GetDocShell()->
                            DoInPlaceActivate(FALSE);       // OLE beenden
                    }
                }

//              SetSumAssignMode(); //ScInputWindow
            }
            break;

        case SID_DATA_SELECT:
            pTabViewShell->StartDataSelect();
            break;

        case SID_DETECTIVE_FILLMODE:
            {
                BOOL bOldMode = pTabViewShell->IsAuditShell();
                pTabViewShell->SetAuditShell( !bOldMode );
                pTabViewShell->Invalidate( nSlot );
            }
            break;

        case SID_OPENDLG_CONDFRMT:
            {
                USHORT          nId  = ScCondFormatDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        //  ----------------------------------------------------------------

        case FID_INPUTLINE_STATUS:
            DBG_ERROR("Execute von InputLine-Status");
            break;


        case SID_STATUS_DOCPOS:
            {
                //! Navigator an-/ausschalten (wie im Writer) ???
                //!pViewData->GetDispatcher().Execute( SID_NAVIGATOR,
                //!                       SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
            }
            break;


        case SID_MARKAREA:
            // called from Basic at the hidden view to select a range in the visible view
            DBG_ERROR("old slot SID_MARKAREA");
            break;

        default:
            DBG_ERROR("Unbekannter Slot bei ScCellShell::Execute");
            break;
    }
}



