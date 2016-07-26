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
#include "sc.hrc"
#include "globstr.hrc"
#include "tpusrlst.hxx"

#define CR  (sal_Unicode)13
#define LF  (sal_Unicode)10

static const sal_Unicode cDelimiter = ',';

// Benutzerdefinierte Listen:

ScTpUserLists::ScTpUserLists( vcl::Window*               pParent,
                              const SfxItemSet&     rCoreAttrs )

    :   SfxTabPage      ( pParent,
                          "OptSortLists", "modules/scalc/ui/optsortlists.ui",
                          &rCoreAttrs ),
        aStrQueryRemove ( ScGlobal::GetRscString( STR_QUERYREMOVE ) ),
        aStrCopyList    ( ScGlobal::GetRscString( STR_COPYLIST ) ),
        aStrCopyFrom    ( ScGlobal::GetRscString( STR_COPYFROM ) ),
        aStrCopyErr     ( ScGlobal::GetRscString( STR_COPYERR ) ),
        nWhichUserLists ( GetWhich( SID_SCUSERLISTS ) ),
        pUserLists      ( nullptr ),
        pDoc            ( nullptr ),
        pViewData       ( nullptr ),
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
    Reset(&rCoreAttrs);
}

ScTpUserLists::~ScTpUserLists()
{
    disposeOnce();
}

void ScTpUserLists::dispose()
{
    delete pUserLists;
    mpFtLists.clear();
    mpLbLists.clear();
    mpFtEntries.clear();
    mpEdEntries.clear();
    mpFtCopyFrom.clear();
    mpEdCopyFrom.clear();
    mpBtnNew.clear();
    mpBtnDiscard.clear();
    mpBtnAdd.clear();
    mpBtnModify.clear();
    mpBtnRemove.clear();
    mpBtnCopy.clear();
    SfxTabPage::dispose();
}

void ScTpUserLists::Init()
{
    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pSh );

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

        pViewData = &pViewSh->GetViewData();
        pDoc = pViewData->GetDocument();

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        aStrSelectedArea = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab
                ).Format(ScRefFlags::RANGE_ABS_3D, pDoc);

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

VclPtr<SfxTabPage> ScTpUserLists::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<ScTpUserLists>::Create( pParent, *rAttrSet );
}

void ScTpUserLists::Reset( const SfxItemSet* rCoreAttrs )
{
    const ScUserListItem& rUserListItem = static_cast<const ScUserListItem&>(
                                           rCoreAttrs->Get( nWhichUserLists ));
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

bool ScTpUserLists::FillItemSet( SfxItemSet* rCoreAttrs )
{
    // Modifikationen noch nicht uebernommen?
    // -> Click auf Add-Button simulieren

    if ( bModifyMode || bCancelMode )
        BtnClickHdl( mpBtnAdd );

    const ScUserListItem& rUserListItem = static_cast<const ScUserListItem&>(
                                           GetItemSet().Get( nWhichUserLists ));

    ScUserList* pCoreList       = rUserListItem.GetUserList();
    bool        bDataModified   = false;

    if ( (pUserLists == nullptr) && (pCoreList == nullptr) )
    {
        bDataModified = false;
    }
    else if ( pUserLists != nullptr )
    {
        if ( pCoreList != nullptr )
            bDataModified = (*pUserLists != *pCoreList);
        else
            bDataModified = true;
    }

    if ( bDataModified )
    {
        ScUserListItem aULItem( nWhichUserLists );

        if ( pUserLists )
            aULItem.SetUserList( *pUserLists );

        rCoreAttrs->Put( aULItem );
    }

    return bDataModified;
}

DeactivateRC ScTpUserLists::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

size_t ScTpUserLists::UpdateUserListBox()
{
    mpLbLists->Clear();

    if ( !pUserLists ) return 0;

    size_t nCount = pUserLists->size();
    OUString  aEntry;

    for ( size_t i=0; i<nCount; ++i )
    {
        aEntry = (*pUserLists)[i].GetString();
        OSL_ENSURE( !aEntry.isEmpty(), "Empty UserList-entry :-/" );
        mpLbLists->InsertEntry( aEntry );
    }

    return nCount;
}

void ScTpUserLists::UpdateEntries( size_t nList )
{
    if ( !pUserLists ) return;

    if ( nList < pUserLists->size() )
    {
        const ScUserListData& rList = (*pUserLists)[nList];
        std::size_t nSubCount = rList.GetSubCount();
        OUString          aEntryListStr;

        for ( size_t i=0; i<nSubCount; i++ )
        {
            if ( i!=0 )
                aEntryListStr += OUStringLiteral1<CR>();
            aEntryListStr += rList.GetSubStr(i);
        }

        mpEdEntries->SetText(convertLineEnd(aEntryListStr, GetSystemLineEnd()));
    }
    else
    {
        OSL_FAIL( "Invalid ListIndex :-/" );
    }
}

void ScTpUserLists::MakeListStr( OUString& rListStr )
{
    OUString  aStr;

    sal_Int32 nToken = comphelper::string::getTokenCount(rListStr, LF);

    for(sal_Int32 i=0; i<nToken; i++)
    {
        OUString aString = comphelper::string::strip(rListStr.getToken(i, LF), ' ');
        aStr += aString;
        aStr += OUStringLiteral1<cDelimiter>();
    }

    aStr = comphelper::string::strip(aStr, cDelimiter);
    sal_Int32 nLen = aStr.getLength();

    rListStr.clear();

    // Alle Doppelten cDelimiter entfernen:
    sal_Int32 c = 0;
    while ( c < nLen )
    {
        rListStr += OUString(aStr[c]);
        ++c;

        if ((c < nLen) && (aStr[c] == cDelimiter))
        {
            rListStr += OUString(aStr[c]);

            while ((c < nLen) && (aStr[c] == cDelimiter))
                ++c;
        }
    }

}

void ScTpUserLists::AddNewList( const OUString& rEntriesStr )
{
    OUString theEntriesStr( rEntriesStr );

    if ( !pUserLists )
        pUserLists = new ScUserList;

    MakeListStr( theEntriesStr );

    pUserLists->push_back(new ScUserListData(theEntriesStr));
}

void ScTpUserLists::CopyListFromArea( const ScRefAddress& rStartPos,
                                      const ScRefAddress& rEndPos )
{
    if ( bCopyDone ) return;

    SCTAB   nTab            = rStartPos.Tab();
    SCCOL   nStartCol       = rStartPos.Col();
    SCROW   nStartRow       = rStartPos.Row();
    SCCOL   nEndCol         = rEndPos.Col();
    SCROW   nEndRow         = rEndPos.Row();
    sal_uInt16  nCellDir        = SCRET_COLS;

    if ( (nStartCol != nEndCol) && (nStartRow != nEndRow) )
    {
        nCellDir = ScopedVclPtrInstance<ScColOrRowDlg>(this, aStrCopyList, aStrCopyFrom)->Execute();
    }
    else if ( nStartCol != nEndCol )
        nCellDir = SCRET_ROWS;
    else
        nCellDir = SCRET_COLS;

    if ( nCellDir != RET_CANCEL )
    {
        bool bValueIgnored = false;
        OUString  aStrList;
        OUString  aStrField;

        if ( nCellDir == SCRET_COLS )
        {
            for ( SCCOL col=nStartCol; col<=nEndCol; col++ )
            {
                for ( SCROW row=nStartRow; row<=nEndRow; row++ )
                {
                    if ( pDoc->HasStringData( col, row, nTab ) )
                    {
                        aStrField = pDoc->GetString(col, row, nTab);

                        if ( !aStrField.isEmpty() )
                        {
                            aStrList += aStrField;
                            aStrList += "\n";
                        }
                    }
                    else
                        bValueIgnored = true;
                }
                if ( !aStrList.isEmpty() )
                    AddNewList( aStrList );
                aStrList.clear();
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

                        if ( !aStrField.isEmpty() )
                        {
                            aStrList += aStrField;
                            aStrList += "\n";
                        }
                    }
                    else
                        bValueIgnored = true;
                }
                if ( !aStrList.isEmpty() )
                    AddNewList( aStrList );
                aStrList.clear();
            }
        }

        if ( bValueIgnored )
        {
            ScopedVclPtrInstance<InfoBox>(this, aStrCopyErr)->Execute();
        }
    }

    bCopyDone = true;

}

void ScTpUserLists::ModifyList( size_t            nSelList,
                                const OUString&   rEntriesStr )
{
    if ( !pUserLists ) return;

    OUString theEntriesStr( rEntriesStr );

    MakeListStr( theEntriesStr );

    (*pUserLists)[nSelList].SetString( theEntriesStr );
}

void ScTpUserLists::RemoveList( size_t nList )
{
    if (pUserLists && nList < pUserLists->size())
    {
        ScUserList::iterator itr = pUserLists->begin();
        ::std::advance(itr, nList);
        pUserLists->erase(itr);
    }
}

// Handler:

IMPL_LINK_TYPED( ScTpUserLists, LbSelectHdl, ListBox&, rLb, void )
{
    if ( &rLb == mpLbLists )
    {
        sal_Int32 nSelPos = mpLbLists->GetSelectEntryPos();
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
}

IMPL_LINK_TYPED( ScTpUserLists, BtnClickHdl, Button*, pBtn, void )
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
            mpEdEntries->SetText( EMPTY_OUSTRING );
            mpEdEntries->GrabFocus();
            mpBtnAdd->Disable();
            mpBtnModify->Disable();
            mpBtnRemove->Disable();

            if ( mpBtnCopy->IsEnabled() )
            {
                mpBtnCopy->Disable();
                mpFtCopyFrom->Disable();
                mpEdCopyFrom->Disable();
            }
            mpBtnNew->Hide();
            mpBtnDiscard->Show();
            bCancelMode = true;
        }
        else // if ( bCancelMode )
        {
            if ( mpLbLists->GetEntryCount() > 0 )
            {
                mpLbLists->SelectEntryPos( nCancelPos );
                LbSelectHdl( *mpLbLists.get() );
                mpFtLists->Enable();
                mpLbLists->Enable();
            }
            else
            {
                mpFtEntries->Disable();
                mpEdEntries->Disable();
                mpEdEntries->SetText( EMPTY_OUSTRING );
                mpBtnRemove->Disable();
            }
            mpBtnAdd->Disable();
            mpBtnModify->Disable();

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
        OUString theEntriesStr( mpEdEntries->GetText() );

        if ( !bModifyMode )
        {
            if ( !theEntriesStr.isEmpty() )
            {
                AddNewList( theEntriesStr );
                UpdateUserListBox();
                mpLbLists->SelectEntryPos( mpLbLists->GetEntryCount()-1 );
                LbSelectHdl( *mpLbLists.get() );
                mpFtLists->Enable();
                mpLbLists->Enable();
            }
            else
            {
                if ( mpLbLists->GetEntryCount() > 0 )
                {
                    mpLbLists->SelectEntryPos( nCancelPos );
                    LbSelectHdl( *mpLbLists.get() );
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
            sal_Int32 nSelList = mpLbLists->GetSelectEntryPos();

            OSL_ENSURE( nSelList != LISTBOX_ENTRY_NOTFOUND, "Modify without List :-/" );

            if ( !theEntriesStr.isEmpty() )
            {
                ModifyList( nSelList, theEntriesStr );
                UpdateUserListBox();
                mpLbLists->SelectEntryPos( nSelList );
            }
            else
            {
                mpLbLists->SelectEntryPos( 0 );
                LbSelectHdl( *mpLbLists.get() );
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
            sal_Int32 nRemovePos   = mpLbLists->GetSelectEntryPos();
            OUString aMsg         ( aStrQueryRemove.getToken( 0, '#' ) );

            aMsg += mpLbLists->GetEntry( nRemovePos );
            aMsg += aStrQueryRemove.getToken( 1, '#' );

            if ( RET_YES == ScopedVclPtrInstance<QueryBox>( this,
                                      WinBits( WB_YES_NO | WB_DEF_YES ),
                                      aMsg
                                     )->Execute() )
            {
                RemoveList( nRemovePos );
                UpdateUserListBox();

                if ( mpLbLists->GetEntryCount() > 0 )
                {
                    mpLbLists->SelectEntryPos(
                        ( nRemovePos >= mpLbLists->GetEntryCount() )
                            ? mpLbLists->GetEntryCount()-1
                            : nRemovePos );
                    LbSelectHdl( *mpLbLists.get() );
                }
                else
                {
                    mpFtLists->Disable();
                    mpLbLists->Disable();
                    mpFtEntries->Disable();
                    mpEdEntries->Disable();
                    mpEdEntries->SetText( EMPTY_OUSTRING );
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
            return;

        ScRefAddress theStartPos;
        ScRefAddress theEndPos;
        OUString     theAreaStr( mpEdCopyFrom->GetText() );
        bool     bAreaOk = false;

        if ( !theAreaStr.isEmpty() )
        {
            bAreaOk = ScRangeUtil::IsAbsArea( theAreaStr,
                                             pDoc,
                                             pViewData->GetTabNo(),
                                             &theAreaStr,
                                             &theStartPos,
                                             &theEndPos,
                                             pDoc->GetAddressConvention() );
            if ( !bAreaOk )
            {
                bAreaOk = ScRangeUtil::IsAbsPos(  theAreaStr,
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
            LbSelectHdl( *mpLbLists.get() );
            mpEdCopyFrom->SetText( theAreaStr );
            mpEdCopyFrom->Disable();
            mpBtnCopy->Disable();
            mpFtCopyFrom->Disable();
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>(this,
                      ScGlobal::GetRscString( STR_INVALID_TABREF )
                    )->Execute();
            mpEdCopyFrom->GrabFocus();
            mpEdCopyFrom->SetSelection( Selection( 0, SELECTION_MAX ) );
        }
    }
}

IMPL_LINK_TYPED( ScTpUserLists, EdEntriesModHdl, Edit&, rEd, void )
{
    if ( &rEd != mpEdEntries )
        return;

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
            bCancelMode = true;
            mpBtnAdd->Hide();
            mpBtnAdd->Enable();
            mpBtnModify->Show();
            mpBtnModify->Enable();
            bModifyMode = true;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
