/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
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
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // der kommt beim MouseButtonUp
        pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    if ( IS_EDITMODE() )
    {
        switch ( nSlot )
        {
            //  beim Oeffnen eines Referenz-Dialogs darf die SubShell nicht umgeschaltet werden
            //  (beim Schliessen des Dialogs wird StopEditShell gerufen)
            case SID_OPENDLG_FUNCTION:
                    //  #53318# inplace macht die EditShell Aerger...
                    //! kann nicht immer umgeschaltet werden ????
                    if (!pTabViewShell->GetViewFrame()->GetFrame().IsInPlace())
                        pTabViewShell->SetDontSwitch(sal_True);         // EditShell nicht abschalten
                    // kein break

            case FID_CELL_FORMAT:
            case SID_ENABLE_HYPHENATION:
            case SID_DATA_SELECT:
            case SID_OPENDLG_CONSOLIDATE:
            case SID_OPENDLG_SOLVE:
            case SID_OPENDLG_OPTSOLVER:

                    pScMod->InputEnterHandler();
                    pTabViewShell->UpdateInputHandler();

                    pTabViewShell->SetDontSwitch(sal_False);

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
                sal_uInt16 nMode = ((const SfxUInt16Item&)pReqArgs->Get( nSlot )).GetValue();

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

                sal_uInt16 nModifiers = pTabViewShell->GetLockedModifiers();
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
                String aString = pStatusItem->GetString();
                const EditTextObject* pData = pStatusItem->GetEditData();
                if (pData)
                {
                    if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, pData );
                    }
                    else if ( aString.Len() > 0 && ( aString.GetChar(0) == '=' || aString.GetChar(0) == '+' || aString.GetChar(0) == '-' ) )
                    {
                        pTabViewShell->EnterData( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), aString, sal_True, pData );
                    }
                    else
                    {
                        pTabViewShell->EnterData( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), pData );
                    }
                }
                else
                {
                    if (nSlot == FID_INPUTLINE_ENTER)
                    {
                        if (
                            aCursorPos.Col() == GetViewData()->GetCurX() &&
                            aCursorPos.Row() == GetViewData()->GetCurY() &&
                            aCursorPos.Tab() == GetViewData()->GetTabNo()
                            )
                        {
                            SfxStringItem   aItem( SID_ENTER_STRING, aString );

                            // SfxBindings& rBindings = pTabViewShell->GetViewFrame()->GetBindings();
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

        case SID_OPENDLG_FUNCTION:
            {
                sal_uInt16 nId = SID_OPENDLG_FUNCTION;
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
                rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONSOLIDATE:
            {
                sal_uInt16          nId  = ScConsolidateDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
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
                    sal_uInt16              nWhich = 0;

                    for ( nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
                        if ( pReqArgs->GetItemState( nWhich, sal_True, &pAttr ) == SFX_ITEM_SET )
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
                sal_uInt16          nId  = ScSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
            }
            break;

        case SID_OPENDLG_OPTSOLVER:
            {
                sal_uInt16 nId = ScOptSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
            }
            break;

        case SID_OPENDLG_TABOP:
            {
                sal_uInt16          nId  = ScTabOpDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
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
                        sal_uInt16 nFlags;

                        pDoc->GetName( nTab, aBaseName );
                        aBaseName += '_';
                        aBaseName += ScGlobal::GetRscString(STR_SCENARIO);
                        aBaseName += '_';

                        //  vorneweg testen, ob der Prefix als gueltig erkannt wird
                        //  wenn nicht, nur doppelte vermeiden
                        sal_Bool bPrefix = pDoc->ValidTabName( aBaseName );
                        DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");

                        while ( pDoc->IsScenario(nTab+i) )
                            i++;

                        sal_Bool bValid;
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
                            String aArgName;
                            String aArgComment;
                            const SfxPoolItem* pItem;
                            if ( pReqArgs->GetItemState( SID_SCENARIOS, sal_True, &pItem ) == SFX_ITEM_SET )
                                aArgName = ((const SfxStringItem*)pItem)->GetValue();
                            if ( pReqArgs->GetItemState( SID_NEW_TABLENAME, sal_True, &pItem ) == SFX_ITEM_SET )
                                aArgComment = ((const SfxStringItem*)pItem)->GetValue();

                            aColor = Color( COL_LIGHTGRAY );        // Default
                            nFlags = 0;                             // nicht-TwoWay

                            pTabViewShell->MakeScenario( aArgName, aArgComment, aColor, nFlags );
                            if( ! rReq.IsAPI() )
                                rReq.Done();
                        }
                        else
                        {
                            sal_Bool bSheetProtected = pDoc->IsTabProtected(nTab);
                            //CHINA001 ScNewScenarioDlg* pNewDlg =
                            //CHINA001  new ScNewScenarioDlg( pTabViewShell->GetDialogParent(), aName, sal_False, bSheetProtected );
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                            AbstractScNewScenarioDlg* pNewDlg = pFact->CreateScNewScenarioDlg( pTabViewShell->GetDialogParent(), aName, RID_SCDLG_NEWSCENARIO, sal_False,bSheetProtected);
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
                    pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_DIRECT,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    ScViewData* pData      = GetViewData();
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    sal_uInt16      nCurHeight = pData->GetDocument()->
                                                GetRowHeight( pData->GetCurY(),
                                                              pData->GetTabNo() );
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
                                                                                    RID_SCDLG_ROW_MAN,
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_COL_HEIGHT);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
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
                    pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_OPTIMAL,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
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
                                                                                    RID_SCDLG_ROW_OPT,
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_HEIGHT);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastRowHeightExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_OPT_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
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
                    pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    ScViewData* pData      = GetViewData();
                    sal_uInt16      nCurHeight = pData->GetDocument()->
                                                GetColWidth( pData->GetCurX(),
                                                             pData->GetTabNo() );
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
                                                                                    RID_SCDLG_COL_MAN,
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_COL_WIDTH);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal)) );
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
                    pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
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
                                                                                    RID_SCDLG_COL_OPT,
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_WIDTH);
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastColWidthExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_OPT_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();
                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_OPT_DIRECT:
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH );
            rReq.Done();
            break;

        case FID_ROW_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_ROW_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( sal_False, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;
        case FID_COL_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_COL_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_SHOW, 0 );
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
        case FID_MERGE_OFF:
        case FID_MERGE_TOGGLE:
        {
            if ( !GetViewData()->GetDocument()->GetChangeTrack() )
            {
                // test whether to merge or to split
                bool bMerge = false;
                switch( nSlot )
                {
                    case FID_MERGE_ON:
                        bMerge = true;
                    break;
                    case FID_MERGE_OFF:
                        bMerge = false;
                    break;
                    case FID_MERGE_TOGGLE:
                    {
                        SfxPoolItem* pItem = 0;
                        if( rBindings.QueryState( nSlot, pItem ) >= SFX_ITEM_DEFAULT )
                            bMerge = !static_cast< SfxBoolItem* >( pItem )->GetValue();
                    }
                    break;
                }

                if( bMerge )
                {
                    // merge - check if to move contents of covered cells
                    sal_Bool bMoveContents = sal_False;
                    sal_Bool bApi = rReq.IsAPI();
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                        pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
                    {
                        DBG_ASSERT(pItem && pItem->ISA(SfxBoolItem), "falsches Item");
                        bMoveContents = ((const SfxBoolItem*)pItem)->GetValue();
                    }

                    if (pTabViewShell->MergeCells( bApi, bMoveContents ))
                    {
                        if (!bApi && bMoveContents)             // "ja" im Dialog geklickt
                            rReq.AppendItem( SfxBoolItem( nSlot, bMoveContents ) );
                        rBindings.Invalidate( nSlot );
                        rReq.Done();
                    }
                }
                else
                {
                    // split cells
                    if (pTabViewShell->RemoveMerge())
                    {
                        rBindings.Invalidate( nSlot );
                        rReq.Done();
                    }
                }
                break;
            }
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
                    pTabViewShell->MarkDataArea( sal_True );

                GetViewData()->GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );

                if (   ( Abs((SCsCOL)nEndCol-(SCsCOL)nStartCol) > 1 )
                    && ( Abs((SCsROW)nEndRow-(SCsROW)nStartRow) > 1 ) )
                {
                    if ( pReqArgs )
                    {
                        const SfxStringItem& rNameItem = (const SfxStringItem&)pReqArgs->Get( SID_AUTOFORMAT );
                        ScAutoFormat* pFormat = ScGlobal::GetAutoFormat();
                        sal_uInt16 nIndex = pFormat->FindIndexPerName( rNameItem.GetValue() );

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

                        AbstractScAutoFormatDlg* pDlg = pFact->CreateScAutoFormatDlg( pDlgParent, ScGlobal::GetAutoFormat(), pNewEntry,GetViewData()->GetDocument(), RID_SCDLG_AUTOFORMAT );
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
                else if( ScViewUtil::IsFullScreen( *pTabViewShell ) )
                    ScViewUtil::SetFullScreen( *pTabViewShell, false );
                else
                {
                    // TODO/LATER: when is this code executed?
                    pTabViewShell->Escape();
                    //SfxObjectShell* pObjSh = GetViewData()->GetSfxDocShell();
                    //if (pObjSh->GetInPlaceObject() &&
                    //    pObjSh->GetInPlaceObject()->GetIPClient())
                    //{
                    //    GetViewData()->GetDocShell()->
                    //        DoInPlaceActivate(sal_False);       // OLE beenden
                    //}
                }

//              SetSumAssignMode(); //ScInputWindow
            }
            break;

        case SID_DATA_SELECT:
            pTabViewShell->StartDataSelect();
            break;

        case SID_DETECTIVE_FILLMODE:
            {
                sal_Bool bOldMode = pTabViewShell->IsAuditShell();
                pTabViewShell->SetAuditShell( !bOldMode );
                pTabViewShell->Invalidate( nSlot );
            }
            break;

        case SID_OPENDLG_CONDFRMT:
            {
                sal_uInt16          nId  = ScCondFormatDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
            }
            break;

        //  ----------------------------------------------------------------

        case FID_INPUTLINE_STATUS:
            DBG_ERROR("Execute von InputLine-Status");
            break;

        case SID_STATUS_DOCPOS:
            // Launch navigator.
            GetViewData()->GetDispatcher().Execute(
                SID_NAVIGATOR, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
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

