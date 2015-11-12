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



// INCLUDE ---------------------------------------------------------------
#include <editeng/eeitem.hxx>

#include "scitems.hxx"
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svl/stritem.hxx>
#include <vcl/sound.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "impex.hxx"
#include "editsh.hxx"
#include "dociter.hxx"
#include "inputhdl.hxx"
#include <svx/srchdlg.hxx>
#include "document.hxx"

//==================================================================

String __EXPORT ScTabViewShell::GetSelectionText( sal_Bool bWholeWord )
{
    String aStrSelection;

    if ( pEditShell && pEditShell == GetMySubShell() )
    {
        aStrSelection = pEditShell->GetSelectionText( bWholeWord );
    }
    else
    {
        ScRange aRange;

        if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            if ( bInFormatDialog && aRange.aStart.Row() != aRange.aEnd.Row() )
            {
                // Range auf eine Datenzeile begrenzen
                // (#48613# nur wenn der Aufruf aus einem Format-Dialog kommt)
                ScHorizontalCellIterator aIter( pDoc, aRange.aStart.Tab(),
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row() );
                SCCOL nCol;
                SCROW nRow;
                if ( aIter.GetNext( nCol, nRow ) )
                {
                    aRange.aStart.SetCol( nCol );
                    aRange.aStart.SetRow( nRow );
                    aRange.aEnd.SetRow( nRow );
                }
                else
                    aRange.aEnd = aRange.aStart;
            }
            else
            {
                // #i111531# with 1M rows it was necessary to limit the range
                // to the actually used data area.
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                SCTAB nTab1, nTab2;
                aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                bool bShrunk;
                pDoc->ShrinkToUsedDataArea( bShrunk, nTab1, nCol1, nRow1, nCol2, nRow2, false);
                if (bShrunk)
                {
                    aRange.aStart.SetCol( nCol1 );
                    aRange.aStart.SetRow( nRow1 );
                    aRange.aEnd.SetCol( nCol2 );
                    aRange.aEnd.SetRow( nRow2 );
                }
            }

            ScImportExport aObj( pDoc, aRange );
            aObj.SetFormulas( GetViewData()->GetOptions().GetOption( VOPT_FORMULAS ) );
            rtl::OUString aExportOUString;
            aObj.ExportString( aExportOUString );
            aStrSelection = aExportOUString;

            aStrSelection.ConvertLineEnd( LINEEND_CR );

            //  Tab/CR durch Space ersetzen, wenn fuer Dialog oder per Basic/SelectionTextExt,
            //  oder wenn es eine einzelne Zeile ist.
            //  Sonst mehrzeilig mit Tabs beibehalten (z.B. Mail oder Basic/SelectionText).
            //  Fuer Mail werden die Tabs dann spaeter in (mehrere) Spaces gewandelt.

            if ( bInFormatDialog || bWholeWord || aRange.aEnd.Row() == aRange.aStart.Row() )
            {
                xub_StrLen nAt;
                while (  (nAt = aStrSelection.Search( CHAR_CR )) != STRING_NOTFOUND )
                    aStrSelection.SetChar( nAt, ' ' );
                while (  (nAt = aStrSelection.Search( '\t' )) != STRING_NOTFOUND )
                    aStrSelection.SetChar( nAt, ' ' );

                aStrSelection.EraseTrailingChars( ' ' );
            }
        }
    }

    return aStrSelection;
}

//------------------------------------------------------------------------

void ScTabViewShell::InsertURL( const String& rName, const String& rURL, const String& rTarget,
                                sal_uInt16 nMode )
{
    SvxLinkInsertMode eMode = (SvxLinkInsertMode) nMode;
    sal_Bool bAsText = ( eMode != HLINK_BUTTON );       // Default ist jetzt Text

    if ( bAsText )
    {
        if ( GetViewData()->IsActive() )
        {
            //  if the view is active, always use InsertURLField, which starts EditMode
            //  and selects the URL, so it can be changed from the URL bar / dialog

            InsertURLField( rName, rURL, rTarget );
        }
        else
        {
            //  #91216# if the view is not active, InsertURLField doesn't work
            //  -> use InsertBookmark to directly manipulate cell content
            //  bTryReplace=sal_True -> if cell contains only one URL, replace it

            SCCOL nPosX = GetViewData()->GetCurX();
            SCROW nPosY = GetViewData()->GetCurY();
            InsertBookmark( rName, rURL, nPosX, nPosY, &rTarget, sal_True );
        }
    }
    else
    {
        SC_MOD()->InputEnterHandler();
        InsertURLButton( rName, rURL, rTarget );
    }
}

//------------------------------------------------------------------------

// wenn CLOOKs: -> mit <editview.hxx> <flditem.hxx>in neue tabvwsh

void lcl_SelectFieldAfterInsert( EditView& rView )
{
    ESelection aSel = rView.GetSelection();
    if ( aSel.nStartPos == aSel.nEndPos && aSel.nStartPos > 0 )
    {
        //  Cursor is behind the inserted field -> extend selection to the left

        --aSel.nStartPos;
        rView.SetSelection( aSel );
    }
}

void ScTabViewShell::InsertURLField( const String& rName, const String& rURL, const String& rTarget )
{
    SvxURLField aURLField( rURL, rName, SVXURLFORMAT_REPR );
    aURLField.SetTargetFrame( rTarget );
    SvxFieldItem aURLItem( aURLField, EE_FEATURE_FIELD );

    ScViewData*     pViewData   = GetViewData();
    ScModule*       pScMod      = SC_MOD();
    ScInputHandler* pHdl        = pScMod->GetInputHdl( pViewData->GetViewShell() );

    sal_Bool bSelectFirst = sal_False;
    if ( !pScMod->IsEditMode() )
    {
        if ( !SelectionEditable() )
        {
            // no error message (may be called from drag&drop)
            Sound::Beep();
            return;
        }

        // single url in cell is shown in the dialog and replaced
        bSelectFirst = HasBookmarkAtCursor( NULL );
        pScMod->SetInputMode( SC_INPUT_TABLE );
    }

    EditView*       pTopView    = pHdl->GetTopView();
    EditView*       pTableView  = pHdl->GetTableView();
    DBG_ASSERT( pTopView || pTableView, "No EditView" );

    if ( bSelectFirst )
    {
        if ( pTopView )
            pTopView->SetSelection( ESelection(0,0,0,1) );
        if ( pTableView )
            pTableView->SetSelection( ESelection(0,0,0,1) );
    }

    pHdl->DataChanging();

    if ( pTopView )
    {
        pTopView->InsertField( aURLItem );
        lcl_SelectFieldAfterInsert( *pTopView );
    }
    if ( pTableView )
    {
        pTableView->InsertField( aURLItem );
        lcl_SelectFieldAfterInsert( *pTableView );
    }

    pHdl->DataChanged();
}

void ScTabViewShell::ExecSearch( SfxRequest& rReq )
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();
    const SfxPoolItem*  pItem;

    switch ( nSlot )
    {
        case FID_SEARCH_ON:
            bSearchJustOpened = sal_True;
            break;

        case FID_SEARCH_NOW:
            {
                if ( pReqArgs &&
                     SFX_ITEM_SET == pReqArgs->GetItemState(SID_SEARCH_ITEM, sal_False, &pItem) )
                {
                    DBG_ASSERT( pItem->ISA(SvxSearchItem), "falsches Item" );
                    const SvxSearchItem* pSearchItem = (const SvxSearchItem*) pItem;

                    ScGlobal::SetSearchItem( *pSearchItem );
                    //SearchAndReplace( pSearchItem, sal_True, rReq.IsAPI() );
                    sal_Bool bSuccess = SearchAndReplace( pSearchItem, sal_True, rReq.IsAPI() );
                    if ( Application::IsAccessibilityEnabled() )
                    {
                        SvxSearchDialog* pSearchDlg =
                            ((SvxSearchDialog*)(SfxViewFrame::Current()->GetChildWindow(
                            SvxSearchDialogWrapper::GetChildWindowId())->GetWindow()));
                        if( pSearchDlg )
                        {
                            ScTabView* pTabView = GetViewData()->GetView();
                            if( pTabView )
                            {
                                Window* pWin = pTabView->GetActiveWin();
                                if( pWin )
                                {
                                    pSearchDlg->SetDocWin( pWin );
                                    pSearchDlg->SetSrchFlag( bSuccess );
                                }
                            }
                        }
                    }
                    rReq.Done();
                }
            }
            break;

        case SID_SEARCH_ITEM:
            if (pReqArgs && SFX_ITEM_SET ==
                            pReqArgs->GetItemState(SID_SEARCH_ITEM, sal_False, &pItem))
            {
                //  Search-Item merken
                DBG_ASSERT( pItem->ISA(SvxSearchItem), "falsches Item" );
                ScGlobal::SetSearchItem( *(const SvxSearchItem*) pItem );
            }
            else
            {
                DBG_ERROR("SID_SEARCH_ITEM ohne Parameter");
            }
            break;
        case FID_SEARCH:
        case FID_REPLACE:
        case FID_REPLACE_ALL:
        case FID_SEARCH_ALL:
            {
                if (pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState(nSlot, sal_False, &pItem))
                {
                    //  SearchItem holen

                    SvxSearchItem aSearchItem = ScGlobal::GetSearchItem();

                    //  SearchItem fuellen

                    aSearchItem.SetSearchString(((SfxStringItem*)pItem)->GetValue());
                    if(SFX_ITEM_SET == pReqArgs->GetItemState(FN_PARAM_1, sal_False, &pItem))
                        aSearchItem.SetReplaceString(((SfxStringItem*)pItem)->GetValue());

                    if (nSlot == FID_SEARCH)
                        aSearchItem.SetCommand(SVX_SEARCHCMD_FIND);
                    else if(nSlot == FID_REPLACE)
                        aSearchItem.SetCommand(SVX_SEARCHCMD_REPLACE);
                    else if(nSlot == FID_REPLACE_ALL)
                        aSearchItem.SetCommand(SVX_SEARCHCMD_REPLACE_ALL);
                    else
                        aSearchItem.SetCommand(SVX_SEARCHCMD_FIND_ALL);

                    //  Request ausfuehren (dabei wird das SearchItem gespeichert)

                    aSearchItem.SetWhich(SID_SEARCH_ITEM);
                    GetViewData()->GetDispatcher().Execute( FID_SEARCH_NOW,
                            rReq.IsAPI() ? SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON :
                                            SFX_CALLMODE_STANDARD,
                            &aSearchItem, 0L );
                    if ( Application::IsAccessibilityEnabled() )
                    {
                        SvxSearchDialog* pSearchDlg =
                            ((SvxSearchDialog*)(SfxViewFrame::Current()->GetChildWindow(
                            SvxSearchDialogWrapper::GetChildWindowId())->GetWindow()));
                        if( pSearchDlg )
                        {
                            ScTabView* pTabView = GetViewData()->GetView();
                            if( pTabView )
                            {
                                Window* pWin = pTabView->GetActiveWin();
                                if( pWin )
                                {
                                    pSearchDlg->SetDocWin( pWin );
                                    pSearchDlg->SetSrchFlag();
                                }
                            }
                        }
                    }
                }
                else
                {
                    GetViewData()->GetDispatcher().Execute(
                            SID_SEARCH_DLG, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD );
                }
            }
            break;
        case FID_REPEAT_SEARCH:
            {
                //  nochmal mit ScGlobal::GetSearchItem()

                SvxSearchItem aSearchItem = ScGlobal::GetSearchItem();
                aSearchItem.SetWhich(SID_SEARCH_ITEM);
                GetViewData()->GetDispatcher().Execute( FID_SEARCH_NOW,
                        rReq.IsAPI() ? SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON :
                                        SFX_CALLMODE_STANDARD,
                        &aSearchItem, 0L );
            }
            break;
//      case FID_SEARCH_COUNT:
    }
}

//--------------------------------------------------------------------






