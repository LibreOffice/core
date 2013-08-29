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


#undef SC_DLLIMPLEMENTATION

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "document.hxx"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "uiitems.hxx"
#include "userlist.hxx"
#include "rangeutl.hxx"
#include "crdlg.hxx"
#include "scresid.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"
#include "globstr.hrc"

#define _TPUSRLST_CXX
#include "tpusrlst.hxx"
#undef _TPUSRLST_CXX

// STATIC DATA -----------------------------------------------------------

#define CR  (sal_Unicode)13
#define LF  (sal_Unicode)10

static const sal_Unicode cDelimiter = ',';

//========================================================================
// Benutzerdefinierte Listen:


ScTpUserLists::ScTpUserLists( Window*               pParent,
                              const SfxItemSet&     rCoreAttrs )

    :   SfxTabPage      ( pParent,
                          "OptSortLists", "modules/scalc/ui/optsortlists.ui",
                          rCoreAttrs ),
        aStrQueryRemove ( ScGlobal::GetRscString( STR_QUERYREMOVE ) ),
        aStrCopyList    ( ScGlobal::GetRscString( STR_COPYLIST ) ),
        aStrCopyFrom    ( ScGlobal::GetRscString( STR_COPYFROM ) ),
        aStrCopyErr     ( ScGlobal::GetRscString( STR_COPYERR ) ),
        nWhichUserLists ( GetWhich( SID_SCUSERLISTS ) ),
        pUserLists      ( NULL ),
        pDoc            ( NULL ),
        pViewData       ( NULL ),
        pRangeUtil      ( new ScRangeUtil ),
        bModifyMode     ( false ),
        bCancelMode     ( false ),
        bCopyDone       ( false ),
        nCancelPos      ( 0 )
{
    get(mpFtLists, "listslabel");
    get(mpLbLists, "lists");
    get(mpFtEntries, "entrieslabel");
    get(mpEdEntries, "entries");
    get(mpFtCopyFrom, "copyfromlabel");
    get(mpEdCopyFrom, "copyfrom");
    get(mpBtnNew, "new");
    get(mpBtnDiscard, "discard");
    get(mpBtnAdd, "add");
    get(mpBtnModify, "modify");
    get(mpBtnRemove, "delete");
    get(mpBtnCopy, "copy");

    SetExchangeSupport();
    Init();
    Reset(rCoreAttrs);
}

// -----------------------------------------------------------------------

ScTpUserLists::~ScTpUserLists()
{
    delete pUserLists;
    delete pRangeUtil;
}

// -----------------------------------------------------------------------

void ScTpUserLists::Init()
{
    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, pSh);

    mpLbLists->SetSelectHdl   ( LINK( this, ScTpUserLists, LbSelectHdl ) );
    mpBtnNew->SetClickHdl     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mpBtnDiscard->SetClickHdl ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mpBtnAdd->SetClickHdl     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mpBtnModify->SetClickHdl  ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mpBtnRemove->SetClickHdl  ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mpEdEntries->SetModifyHdl ( LINK( this, ScTpUserLists, EdEntriesModHdl ) );

    if ( pViewSh )
    {
        SCTAB   nStartTab   = 0;
        SCTAB   nEndTab     = 0;
        SCCOL   nStartCol   = 0;
        SCROW   nStartRow   = 0;
        SCCOL   nEndCol     = 0;
        SCROW   nEndRow     = 0;

        pViewData = pViewSh->GetViewData();
        pDoc = pViewData->GetDocument();

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        aStrSelectedArea = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab
                ).Format(SCR_ABS_3D, pDoc);

        mpBtnCopy->SetClickHdl ( LINK( this, ScTpUserLists, BtnClickHdl ) );
        mpBtnCopy->Enable();
    }
    else
    {
        mpBtnCopy->Disable();
        mpFtCopyFrom->Disable();
        mpEdCopyFrom->Disable();
    }

}

// -----------------------------------------------------------------------

SfxTabPage* ScTpUserLists::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTpUserLists( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

void ScTpUserLists::Reset( const SfxItemSet& rCoreAttrs )
{
    const ScUserListItem& rUserListItem = (const ScUserListItem&)
                                           rCoreAttrs.Get( nWhichUserLists );
    const ScUserList*     pCoreList     = rUserListItem.GetUserList();

    OSL_ENSURE( pCoreList, "UserList not found :-/" );

    if ( pCoreList )
    {
        if ( !pUserLists )
            pUserLists = new ScUserList( *pCoreList );
        else
            *pUserLists = *pCoreList;

        if ( UpdateUserListBox() > 0 )
        {
            mpLbLists->SelectEntryPos( 0 );
            UpdateEntries( 0 );
        }
    }
    else if ( !pUserLists )
        pUserLists = new ScUserList;

    mpEdCopyFrom->SetText( aStrSelectedArea );

    if ( mpLbLists->GetEntryCount() == 0 )
    {
        mpFtLists->Disable();
        mpLbLists->Disable();
        mpFtEntries->Disable();
        mpEdEntries->Disable();
        mpBtnRemove->Disable();
    }

    mpBtnNew->Show();
    mpBtnDiscard->Hide();
    mpBtnAdd->Show();
    mpBtnModify->Hide();
    mpBtnAdd->Disable();
    mpBtnModify->Disable();

    if ( !bCopyDone && pViewData )
    {
        mpFtCopyFrom->Enable();
        mpEdCopyFrom->Enable();
        mpBtnCopy->Enable();
    }
}

// -----------------------------------------------------------------------

sal_Bool ScTpUserLists::FillItemSet( SfxItemSet& rCoreAttrs )
{
    // Modifikationen noch nicht uebernommen?
    // -> Click auf Add-Button simulieren

    if ( bModifyMode || bCancelMode )
        BtnClickHdl( mpBtnAdd );

    const ScUserListItem& rUserListItem = (const ScUserListItem&)
                                           GetItemSet().Get( nWhichUserLists );

    ScUserList* pCoreList       = rUserListItem.GetUserList();
    sal_Bool        bDataModified   = false;

    if ( (pUserLists == NULL) && (pCoreList == NULL) )
    {
        bDataModified = false;
    }
    else if ( pUserLists != NULL )
    {
        if ( pCoreList != NULL )
            bDataModified = (*pUserLists != *pCoreList);
        else
            bDataModified = sal_True;
    }

    if ( bDataModified )
    {
        ScUserListItem aULItem( nWhichUserLists );

        if ( pUserLists )
            aULItem.SetUserList( *pUserLists );

        rCoreAttrs.Put( aULItem );
    }

    return bDataModified;
}

// -----------------------------------------------------------------------

int ScTpUserLists::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

sal_uInt16 ScTpUserLists::UpdateUserListBox()
{
    mpLbLists->Clear();

    if ( !pUserLists ) return 0;

    //----------------------------------------------------------

    size_t nCount = pUserLists->size();
    String  aEntry;

    for ( size_t i=0; i<nCount; ++i )
    {
        aEntry = (*pUserLists)[i]->GetString();
        OSL_ENSURE( aEntry.Len() > 0, "Empty UserList-entry :-/" );
        mpLbLists->InsertEntry( aEntry );
    }

    return nCount;
}

// -----------------------------------------------------------------------

void ScTpUserLists::UpdateEntries( size_t nList )
{
    if ( !pUserLists ) return;

    //----------------------------------------------------------

    if ( nList < pUserLists->size() )
    {
        const ScUserListData* pList = (*pUserLists)[nList];
        sal_uInt16          nSubCount = pList->GetSubCount();
        String          aEntryListStr;

        for ( sal_uInt16 i=0; i<nSubCount; i++ )
        {
            if ( i!=0 )
                aEntryListStr += CR;
            aEntryListStr += String(pList->GetSubStr(i));
        }

        mpEdEntries->SetText(convertLineEnd(aEntryListStr, GetSystemLineEnd()));
    }
    else
    {
        OSL_FAIL( "Invalid ListIndex :-/" );
    }
}

// -----------------------------------------------------------------------

void ScTpUserLists::MakeListStr( String& rListStr )
{
    String  aStr;

    xub_StrLen nToken = comphelper::string::getTokenCount(rListStr, LF);

    for(xub_StrLen i=0;i<nToken;i++)
    {
        OUString aString = comphelper::string::strip(rListStr.GetToken(i, LF), ' ');
        aStr+=aString;
        aStr+=cDelimiter;
    }

    aStr = comphelper::string::strip(aStr, cDelimiter);
    xub_StrLen nLen = aStr.Len();

    rListStr.Erase();

    // Alle Doppelten cDelimiter entfernen:
    xub_StrLen c = 0;
    while ( c < nLen )
    {
        rListStr += aStr.GetChar(c);
        c++;

        if ( aStr.GetChar(c) == cDelimiter )
        {
            rListStr += aStr.GetChar(c);

            while ( (aStr.GetChar(c) == cDelimiter) && (c < nLen) )
                c++;
        }
    }

}

// -----------------------------------------------------------------------

void ScTpUserLists::AddNewList( const String& rEntriesStr )
{
    String theEntriesStr( rEntriesStr );

    if ( !pUserLists )
        pUserLists = new ScUserList;

    MakeListStr( theEntriesStr );

    pUserLists->push_back(new ScUserListData(theEntriesStr));
}

// -----------------------------------------------------------------------

void ScTpUserLists::CopyListFromArea( const ScRefAddress& rStartPos,
                                      const ScRefAddress& rEndPos )
{
    if ( bCopyDone ) return;

    //----------------------------------------------------------

    SCTAB   nTab            = rStartPos.Tab();
    SCCOL   nStartCol       = rStartPos.Col();
    SCROW   nStartRow       = rStartPos.Row();
    SCCOL   nEndCol         = rEndPos.Col();
    SCROW   nEndRow         = rEndPos.Row();
    sal_uInt16  nCellDir        = SCRET_COLS;
    sal_Bool    bValueIgnored   = false;

    if ( (nStartCol != nEndCol) && (nStartRow != nEndRow) )
    {
        nCellDir = ScColOrRowDlg( this, aStrCopyList, aStrCopyFrom ).Execute();
    }
    else if ( nStartCol != nEndCol )
        nCellDir = SCRET_ROWS;
    else
        nCellDir = SCRET_COLS;

    if ( nCellDir != RET_CANCEL )
    {
        String  aStrList;
        String  aStrField;

        if ( nCellDir == SCRET_COLS )
        {
            for ( SCCOL col=nStartCol; col<=nEndCol; col++ )
            {
                for ( SCROW row=nStartRow; row<=nEndRow; row++ )
                {
                    if ( pDoc->HasStringData( col, row, nTab ) )
                    {
                        aStrField = pDoc->GetString(col, row, nTab);

                        if ( aStrField.Len() > 0 )
                        {
                            aStrList += aStrField;
                            aStrList += '\n';
                        }
                    }
                    else
                        bValueIgnored = sal_True;
                }
                if ( aStrList.Len() > 0 )
                    AddNewList( aStrList );
                aStrList.Erase();
            }
        }
        else
        {
            for ( SCROW row=nStartRow; row<=nEndRow; row++ )
            {
                for ( SCCOL col=nStartCol; col<=nEndCol; col++ )
                {
                    if ( pDoc->HasStringData( col, row, nTab ) )
                    {
                        aStrField = pDoc->GetString(col, row, nTab);

                        if ( aStrField.Len() > 0 )
                        {
                            aStrList += aStrField;
                            aStrList += '\n';
                        }
                    }
                    else
                        bValueIgnored = sal_True;
                }
                if ( aStrList.Len() > 0 )
                    AddNewList( aStrList );
                aStrList.Erase();
            }
        }

        if ( bValueIgnored )
        {
            InfoBox( this, aStrCopyErr ).Execute();
        }
    }

    //----------------------------------------------------------

    bCopyDone = sal_True;

}

// -----------------------------------------------------------------------

void ScTpUserLists::ModifyList( sal_uInt16          nSelList,
                                const String&   rEntriesStr )
{
    if ( !pUserLists ) return;

    //----------------------------------------------------------

    String theEntriesStr( rEntriesStr );

    MakeListStr( theEntriesStr );

    (*pUserLists)[nSelList]->SetString( theEntriesStr );
}

// -----------------------------------------------------------------------

void ScTpUserLists::RemoveList( size_t nList )
{
    if (pUserLists && nList < pUserLists->size())
    {
        ScUserList::iterator itr = pUserLists->begin();
        ::std::advance(itr, nList);
        pUserLists->erase(itr);
    }
}

//-----------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScTpUserLists, LbSelectHdl, ListBox*, pLb )
{
    if ( pLb == mpLbLists )
    {
        sal_uInt16 nSelPos = mpLbLists->GetSelectEntryPos();
        if ( nSelPos != LISTBOX_ENTRY_NOTFOUND )
        {
            if ( !mpFtEntries->IsEnabled() )  mpFtEntries->Enable();
            if ( !mpEdEntries->IsEnabled() )  mpEdEntries->Enable();
            if ( !mpBtnRemove->IsEnabled() )  mpBtnRemove->Enable();
            if ( mpBtnAdd->IsEnabled() )
            {
                mpBtnAdd->Disable();
                mpBtnModify->Disable();
            }

            UpdateEntries( nSelPos );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpUserLists, BtnClickHdl, PushButton*, pBtn )
{
    if ( pBtn == mpBtnNew || pBtn == mpBtnDiscard )
    {
        if ( !bCancelMode )
        {
            nCancelPos = ( mpLbLists->GetEntryCount() > 0 )
                            ? mpLbLists->GetSelectEntryPos()
                            : 0;
            mpLbLists->SetNoSelection();
            mpFtLists->Disable();
            mpLbLists->Disable();
            mpFtEntries->Enable();
            mpEdEntries->Enable();
            mpEdEntries->SetText( EMPTY_STRING );
            mpEdEntries->GrabFocus();
            mpBtnAdd->Disable();
            mpBtnModify->Disable();
            mpBtnRemove->Disable();
            //-----------------------------
            if ( mpBtnCopy->IsEnabled() )
            {
                mpBtnCopy->Disable();
                mpFtCopyFrom->Disable();
                mpEdCopyFrom->Disable();
            }
            mpBtnNew->Hide();
            mpBtnDiscard->Show();
            bCancelMode = sal_True;
        }
        else // if ( bCancelMode )
        {
            if ( mpLbLists->GetEntryCount() > 0 )
            {
                mpLbLists->SelectEntryPos( nCancelPos );
                LbSelectHdl( mpLbLists );
                mpFtLists->Enable();
                mpLbLists->Enable();
            }
            else
            {
                mpFtEntries->Disable();
                mpEdEntries->Disable();
                mpEdEntries->SetText( EMPTY_STRING );
                mpBtnRemove->Disable();
            }
            mpBtnAdd->Disable();
            mpBtnModify->Disable();
            //-----------------------------
            if ( pViewData && !bCopyDone )
            {
                mpBtnCopy->Enable();
                mpFtCopyFrom->Enable();
                mpEdCopyFrom->Enable();
            }
            mpBtnNew->Show();
            mpBtnDiscard->Hide();
            bCancelMode = false;
            bModifyMode = false;
        }
    }
    else if (pBtn == mpBtnAdd || pBtn == mpBtnModify)
    {
        String theEntriesStr( mpEdEntries->GetText() );

        if ( !bModifyMode )
        {
            if ( theEntriesStr.Len() > 0 )
            {
                AddNewList( theEntriesStr );
                UpdateUserListBox();
                mpLbLists->SelectEntryPos( mpLbLists->GetEntryCount()-1 );
                LbSelectHdl( mpLbLists );
                mpFtLists->Enable();
                mpLbLists->Enable();
            }
            else
            {
                if ( mpLbLists->GetEntryCount() > 0 )
                {
                    mpLbLists->SelectEntryPos( nCancelPos );
                    LbSelectHdl( mpLbLists );
                    mpLbLists->Enable();
                    mpLbLists->Enable();
                }
            }

            mpBtnAdd->Disable();
            mpBtnModify->Disable();
            mpBtnRemove->Enable();
            mpBtnNew->Show();
            mpBtnDiscard->Hide();
            bCancelMode = false;
        }
        else // if ( bModifyMode )
        {
            sal_uInt16 nSelList = mpLbLists->GetSelectEntryPos();

            OSL_ENSURE( nSelList != LISTBOX_ENTRY_NOTFOUND, "Modify without List :-/" );

            if ( theEntriesStr.Len() > 0 )
            {
                ModifyList( nSelList, theEntriesStr );
                UpdateUserListBox();
                mpLbLists->SelectEntryPos( nSelList );
            }
            else
            {
                mpLbLists->SelectEntryPos( 0 );
                LbSelectHdl( mpLbLists );
            }

            mpBtnNew->Show();
            mpBtnDiscard->Hide();
            bCancelMode = false;
            mpBtnAdd->Show();
            mpBtnModify->Show();
            mpBtnAdd->Disable();
            mpBtnModify->Disable();
            bModifyMode = false;
            mpBtnRemove->Enable();
            mpFtLists->Enable();
            mpLbLists->Enable();
        }

        if ( pViewData && !bCopyDone )
        {
            mpBtnCopy->Enable();
            mpFtCopyFrom->Enable();
            mpEdCopyFrom->Enable();
        }
    }
    else if ( pBtn == mpBtnRemove )
    {
        if ( mpLbLists->GetEntryCount() > 0 )
        {
            sal_uInt16 nRemovePos   = mpLbLists->GetSelectEntryPos();
            String aMsg         ( aStrQueryRemove.GetToken( 0, '#' ) );

            aMsg += mpLbLists->GetEntry( nRemovePos );
            aMsg += aStrQueryRemove.GetToken( 1, '#' );


            if ( RET_YES == QueryBox( this,
                                      WinBits( WB_YES_NO | WB_DEF_YES ),
                                      aMsg
                                     ).Execute() )
            {
                RemoveList( nRemovePos );
                UpdateUserListBox();

                if ( mpLbLists->GetEntryCount() > 0 )
                {
                    mpLbLists->SelectEntryPos(
                        ( nRemovePos >= mpLbLists->GetEntryCount() )
                            ? mpLbLists->GetEntryCount()-1
                            : nRemovePos );
                    LbSelectHdl( mpLbLists );
                }
                else
                {
                    mpFtLists->Disable();
                    mpLbLists->Disable();
                    mpFtEntries->Disable();
                    mpEdEntries->Disable();
                    mpEdEntries->SetText( EMPTY_STRING );
                    mpBtnRemove->Disable();
                }
            }

            if ( pViewData && !bCopyDone && !mpBtnCopy->IsEnabled() )
            {
                mpBtnCopy->Enable();
                mpFtCopyFrom->Enable();
                mpEdCopyFrom->Enable();
            }
        }
    }
    else if ( pViewData && (pBtn == mpBtnCopy) )
    {
        if ( bCopyDone )
            return 0;

        //-----------------------------------------------------------

        ScRefAddress theStartPos;
        ScRefAddress theEndPos;
        String       theAreaStr( mpEdCopyFrom->GetText() );
        sal_Bool        bAreaOk = false;

        if ( theAreaStr.Len() > 0 )
        {
            bAreaOk = pRangeUtil->IsAbsArea( theAreaStr,
                                             pDoc,
                                             pViewData->GetTabNo(),
                                             &theAreaStr,
                                             &theStartPos,
                                             &theEndPos,
                                             pDoc->GetAddressConvention() );
            if ( !bAreaOk )
            {
                bAreaOk = pRangeUtil->IsAbsPos(  theAreaStr,
                                                 pDoc,
                                                 pViewData->GetTabNo(),
                                                 &theAreaStr,
                                                 &theStartPos,
                                                 pDoc->GetAddressConvention() );
                theEndPos = theStartPos;
            }
        }

        if ( bAreaOk )
        {
            CopyListFromArea( theStartPos, theEndPos );
            UpdateUserListBox();
            mpLbLists->SelectEntryPos( mpLbLists->GetEntryCount()-1 );
            LbSelectHdl( mpLbLists );
            mpEdCopyFrom->SetText( theAreaStr );
            mpEdCopyFrom->Disable();
            mpBtnCopy->Disable();
            mpFtCopyFrom->Disable();
        }
        else
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                      ScGlobal::GetRscString( STR_INVALID_TABREF )
                    ).Execute();
            mpEdCopyFrom->GrabFocus();
            mpEdCopyFrom->SetSelection( Selection( 0, SELECTION_MAX ) );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpUserLists, EdEntriesModHdl, VclMultiLineEdit*, pEd )
{
    if ( pEd != mpEdEntries )
        return 0;

    //-----------------------------------------------------------

    if ( mpBtnCopy->IsEnabled() )
    {
        mpBtnCopy->Disable();
        mpFtCopyFrom->Disable();
        mpEdCopyFrom->Disable();
    }

    if ( !mpEdEntries->GetText().isEmpty() )
    {
        if ( !bCancelMode && !bModifyMode )
        {
            mpBtnNew->Hide();
            mpBtnDiscard->Show();
            bCancelMode = sal_True;
            mpBtnAdd->Hide();
            mpBtnAdd->Enable();
            mpBtnModify->Show();
            mpBtnModify->Enable();
            bModifyMode = sal_True;
            mpBtnRemove->Disable();
            mpFtLists->Disable();
            mpLbLists->Disable();
        }
        else // if ( bCancelMode || bModifyMode )
        {
            if ( !mpBtnAdd->IsEnabled() )
            {
                mpBtnAdd->Enable();
                mpBtnModify->Enable();
            }
        }
    }
    else
    {
        if ( mpBtnAdd->IsEnabled() )
        {
            mpBtnAdd->Disable();
            mpBtnModify->Disable();
        }
    }

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
