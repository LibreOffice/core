/*************************************************************************
 *
 *  $RCSfile: tabvwshe.cxx,v $
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

// INCLUDE ---------------------------------------------------------------

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svtools/stritem.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "impex.hxx"
#include "editsh.hxx"
#include "dociter.hxx"
#include "inputhdl.hxx"

//==================================================================

String __EXPORT ScTabViewShell::GetSelectionText( BOOL bWholeWord )
{
    String aStrSelection;

    if ( pEditShell && pEditShell == GetMySubShell() )
    {
        aStrSelection = pEditShell->GetSelectionText( bWholeWord );
    }
    else
    {
        ScRange aRange;

        if ( GetViewData()->GetSimpleArea( aRange ) )
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            if ( bInFormatDialog && aRange.aStart.Row() != aRange.aEnd.Row() )
            {
                // Range auf eine Datenzeile begrenzen
                // (#48613# nur wenn der Aufruf aus einem Format-Dialog kommt)
                ScHorizontalCellIterator aIter( pDoc, aRange.aStart.Tab(),
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row() );
                USHORT nCol, nRow;
                if ( aIter.GetNext( nCol, nRow ) )
                {
                    aRange.aStart.SetCol( nCol );
                    aRange.aStart.SetRow( nRow );
                    aRange.aEnd.SetRow( nRow );
                }
                else
                    aRange.aEnd = aRange.aStart;
            }

            ScImportExport aObj( pDoc, aRange );
            aObj.SetFormulas( GetViewData()->GetOptions().GetOption( VOPT_FORMULAS ) );
            aObj.ExportString( aStrSelection );

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
                                USHORT nMode )
{
    SvxLinkInsertMode eMode = (SvxLinkInsertMode) nMode;
    BOOL bAsText = ( eMode != HLINK_BUTTON );       // Default ist jetzt Text

    if ( bAsText )
    {
        //  InsertBookmark ?
        InsertURLField( rName, rURL, rTarget );
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
    SvxFieldItem aURLItem( aURLField );

    ScViewData*     pViewData   = GetViewData();
    ScTabView*      pView       = pViewData->GetView();
    ScModule*       pScMod      = SC_MOD();
    ScInputHandler* pHdl        = pScMod->GetInputHdl();

    BOOL bSelectFirst = FALSE;
    if ( !pScMod->IsEditMode() )
    {
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
    SfxBindings&        rBindings   = GetViewFrame()->GetBindings();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();
    const SfxPoolItem*  pItem;

    switch ( nSlot )
    {
        case FID_SEARCH_NOW:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     SFX_ITEM_SET == pReqArgs->GetItemState(SID_SEARCH_ITEM, FALSE, &pItem) )
                {
                    DBG_ASSERT( pItem->ISA(SvxSearchItem), "falsches Item" );
                    const SvxSearchItem* pSearchItem = (const SvxSearchItem*) pItem;

                    ScGlobal::SetSearchItem( *pSearchItem );
                    SearchAndReplace( pSearchItem, TRUE, rReq.IsAPI() );
                    rReq.Done();
                }
            }
            break;

        case SID_SEARCH_ITEM:
            if (pReqArgs && SFX_ITEM_SET ==
                            pReqArgs->GetItemState(SID_SEARCH_ITEM, FALSE, &pItem))
            {
                //  Search-Item merken
                DBG_ASSERT( pItem->ISA(SvxSearchItem), "falsches Item" );
                ScGlobal::SetSearchItem( *(const SvxSearchItem*) pItem );
            }
            else
                DBG_ERROR("SID_SEARCH_ITEM ohne Parameter");
            break;
        case FID_SEARCH:
        case FID_REPLACE:
        case FID_REPLACE_ALL:
        case FID_SEARCH_ALL:
            {
                if (pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState(nSlot, FALSE, &pItem))
                {
                    //  SearchItem holen

                    SvxSearchItem aSearchItem = ScGlobal::GetSearchItem();

                    //  SearchItem fuellen

                    aSearchItem.SetSearchString(((SfxStringItem*)pItem)->GetValue());
                    if(SFX_ITEM_SET == pReqArgs->GetItemState(FN_PARAM_1, FALSE, &pItem))
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
                    SFX_DISPATCHER().Execute( FID_SEARCH_NOW,
                            rReq.IsAPI() ? SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON :
                                            SFX_CALLMODE_STANDARD,
                            &aSearchItem, 0L );
                }
                else
                {
                    SFX_DISPATCHER().Execute(
                            SID_SEARCH_DLG, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD );
                }
            }
            break;
        case FID_REPEAT_SEARCH:
            {
                //  nochmal mit ScGlobal::GetSearchItem()

                SvxSearchItem aSearchItem = ScGlobal::GetSearchItem();
                aSearchItem.SetWhich(SID_SEARCH_ITEM);
                SFX_DISPATCHER().Execute( FID_SEARCH_NOW,
                        rReq.IsAPI() ? SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON :
                                        SFX_CALLMODE_STANDARD,
                        &aSearchItem, 0L );
            }
            break;
//      case FID_SEARCH_COUNT:
    }
}

//--------------------------------------------------------------------






