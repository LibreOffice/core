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
#include <tools/lineend.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <global.hxx>
#include <document.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <uiitems.hxx>
#include <userlist.hxx>
#include <rangeutl.hxx>
#include <crdlg.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <tpusrlst.hxx>
#include <scui_def.hxx>

#define CR  u'\x000D'
#define LF  u'\x000A'

const sal_Unicode cDelimiter = ',';

// Benutzerdefinierte Listen:

ScTpUserLists::ScTpUserLists( weld::Container* pPage, weld::DialogController* pController,
                              const SfxItemSet&     rCoreAttrs )
    : SfxTabPage(pPage, pController, "modules/scalc/ui/optsortlists.ui", "OptSortLists",
                          &rCoreAttrs )
    , mxFtLists(m_xBuilder->weld_label("listslabel"))
    , mxLbLists(m_xBuilder->weld_tree_view("lists"))
    , mxFtEntries(m_xBuilder->weld_label("entrieslabel"))
    , mxEdEntries(m_xBuilder->weld_text_view("entries"))
    , mxFtCopyFrom(m_xBuilder->weld_label("copyfromlabel"))
    , mxEdCopyFrom(m_xBuilder->weld_entry("copyfrom"))
    , mxBtnNew(m_xBuilder->weld_button("new"))
    , mxBtnDiscard(m_xBuilder->weld_button("discard"))
    , mxBtnAdd(m_xBuilder->weld_button("add"))
    , mxBtnModify(m_xBuilder->weld_button("modify"))
    , mxBtnRemove(m_xBuilder->weld_button("delete"))
    , mxBtnCopy(m_xBuilder->weld_button("copy"))
    , aStrQueryRemove ( ScResId( STR_QUERYREMOVE ) )
    , aStrCopyList    ( ScResId( STR_COPYLIST ) )
    , aStrCopyFrom    ( ScResId( STR_COPYFROM ) )
    , aStrCopyErr     ( ScResId( STR_COPYERR ) )
    , nWhichUserLists ( GetWhich( SID_SCUSERLISTS ) )
    , pDoc            ( nullptr )
    , pViewData       ( nullptr )
    , bModifyMode     ( false )
    , bCancelMode     ( false )
    , bCopyDone       ( false )
    , nCancelPos      ( 0 )
{
    SetExchangeSupport();
    Init();
    Reset(&rCoreAttrs);
}

ScTpUserLists::~ScTpUserLists()
{
}

void ScTpUserLists::Init()
{
    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pSh );

    mxLbLists->connect_changed   ( LINK( this, ScTpUserLists, LbSelectHdl ) );
    mxBtnNew->connect_clicked     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mxBtnDiscard->connect_clicked ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mxBtnAdd->connect_clicked     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mxBtnModify->connect_clicked  ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mxBtnRemove->connect_clicked  ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    mxEdEntries->connect_changed ( LINK( this, ScTpUserLists, EdEntriesModHdl ) );

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
                ).Format(*pDoc, ScRefFlags::RANGE_ABS_3D);

        mxBtnCopy->connect_clicked ( LINK( this, ScTpUserLists, BtnClickHdl ) );
        mxBtnCopy->set_sensitive(true);
    }
    else
    {
        mxBtnCopy->set_sensitive(false);
        mxFtCopyFrom->set_sensitive(false);
        mxEdCopyFrom->set_sensitive(false);
    }

}

std::unique_ptr<SfxTabPage> ScTpUserLists::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<ScTpUserLists>(pPage, pController, *rAttrSet);
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
            pUserLists.reset( new ScUserList( *pCoreList ) );
        else
            *pUserLists = *pCoreList;

        if ( UpdateUserListBox() > 0 )
        {
            mxLbLists->select( 0 );
            UpdateEntries( 0 );
        }
    }
    else if ( !pUserLists )
        pUserLists.reset( new ScUserList );

    mxEdCopyFrom->set_text( aStrSelectedArea );

    if ( mxLbLists->n_children() == 0 )
    {
        mxFtLists->set_sensitive(false);
        mxLbLists->set_sensitive(false);
        mxFtEntries->set_sensitive(false);
        mxEdEntries->set_sensitive(false);
        mxBtnRemove->set_sensitive(false);
    }

    mxBtnNew->show();
    mxBtnDiscard->hide();
    mxBtnAdd->show();
    mxBtnModify->hide();
    mxBtnAdd->set_sensitive(false);
    mxBtnModify->set_sensitive(false);

    if ( !bCopyDone && pViewData )
    {
        mxFtCopyFrom->set_sensitive(true);
        mxEdCopyFrom->set_sensitive(true);
        mxBtnCopy->set_sensitive(true);
    }
}

bool ScTpUserLists::FillItemSet( SfxItemSet* rCoreAttrs )
{
    // Changes aren't saved?
    // -> simulate click of Add-Button

    if ( bModifyMode || bCancelMode )
        BtnClickHdl(*mxBtnAdd);

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
    mxLbLists->clear();

    if ( !pUserLists ) return 0;

    size_t nCount = pUserLists->size();
    OUString  aEntry;

    for ( size_t i=0; i<nCount; ++i )
    {
        aEntry = (*pUserLists)[i].GetString();
        OSL_ENSURE( !aEntry.isEmpty(), "Empty UserList-entry :-/" );
        mxLbLists->append_text( aEntry );
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
        OUStringBuffer aEntryListStr;

        for ( size_t i=0; i<nSubCount; i++ )
        {
            if ( i!=0 )
                aEntryListStr.append(CR);
            aEntryListStr.append(rList.GetSubStr(i));
        }

        mxEdEntries->set_text(convertLineEnd(aEntryListStr.makeStringAndClear(), GetSystemLineEnd()));
    }
    else
    {
        OSL_FAIL( "Invalid ListIndex :-/" );
    }
}

void ScTpUserLists::MakeListStr( OUString& rListStr )
{
    if (rListStr.isEmpty())
        return;

    OUStringBuffer aStr;

    for(sal_Int32 nIdx=0; nIdx>=0;)
    {
        aStr.append(comphelper::string::strip(rListStr.getToken(0, LF, nIdx), ' '));
        aStr.append(cDelimiter);
    }

    aStr.strip(cDelimiter);
    sal_Int32 nLen = aStr.getLength();

    rListStr.clear();

    // delete all duplicates of cDelimiter
    sal_Int32 c = 0;
    while ( c < nLen )
    {
        rListStr += OUStringChar(aStr[c]);
        ++c;

        if ((c < nLen) && (aStr[c] == cDelimiter))
        {
            rListStr += OUStringChar(aStr[c]);

            while ((c < nLen) && (aStr[c] == cDelimiter))
                ++c;
        }
    }

}

void ScTpUserLists::AddNewList( const OUString& rEntriesStr )
{
    OUString theEntriesStr( rEntriesStr );

    if ( !pUserLists )
        pUserLists.reset( new ScUserList );

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
        ScColOrRowDlg aDialog(GetFrameWeld(), aStrCopyList, aStrCopyFrom);
        nCellDir = aDialog.run();
    }
    else if ( nStartCol != nEndCol )
        nCellDir = SCRET_ROWS;
    else
        nCellDir = SCRET_COLS;

    if ( nCellDir != RET_CANCEL )
    {
        bool bValueIgnored = false;

        if ( nCellDir == SCRET_COLS )
        {
            for ( SCCOL col=nStartCol; col<=nEndCol; col++ )
            {
                OUStringBuffer aStrList;
                for ( SCROW row=nStartRow; row<=nEndRow; row++ )
                {
                    if ( pDoc->HasStringData( col, row, nTab ) )
                    {
                        OUString aStrField = pDoc->GetString(col, row, nTab);

                        if ( !aStrField.isEmpty() )
                        {
                            aStrList.append(aStrField).append("\n");
                        }
                    }
                    else
                        bValueIgnored = true;
                }
                if ( !aStrList.isEmpty() )
                    AddNewList( aStrList.makeStringAndClear() );
            }
        }
        else
        {
            for ( SCROW row=nStartRow; row<=nEndRow; row++ )
            {
                OUStringBuffer aStrList;
                for ( SCCOL col=nStartCol; col<=nEndCol; col++ )
                {
                    if ( pDoc->HasStringData( col, row, nTab ) )
                    {
                        OUString aStrField = pDoc->GetString(col, row, nTab);

                        if ( !aStrField.isEmpty() )
                        {
                            aStrList.append(aStrField).append("\n");
                        }
                    }
                    else
                        bValueIgnored = true;
                }
                if ( !aStrList.isEmpty() )
                    AddNewList( aStrList.makeStringAndClear() );
            }
        }

        if ( bValueIgnored )
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          aStrCopyErr));
            xInfoBox->run();
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

IMPL_LINK( ScTpUserLists, LbSelectHdl, weld::TreeView&, rLb, void )
{
    if ( &rLb != mxLbLists.get() )
        return;

    sal_Int32 nSelPos = mxLbLists->get_selected_index();
    if ( nSelPos == -1 )
        return;

    if ( !mxFtEntries->get_sensitive() )  mxFtEntries->set_sensitive(true);
    if ( !mxEdEntries->get_sensitive() )  mxEdEntries->set_sensitive(true);
    if ( !mxBtnRemove->get_sensitive() )  mxBtnRemove->set_sensitive(true);
    if ( mxBtnAdd->get_sensitive() )
    {
        mxBtnAdd->set_sensitive(false);
        mxBtnModify->set_sensitive(false);
    }

    UpdateEntries( nSelPos );
}

IMPL_LINK( ScTpUserLists, BtnClickHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == mxBtnNew.get() || &rBtn == mxBtnDiscard.get())
    {
        if ( !bCancelMode )
        {
            nCancelPos = ( mxLbLists->n_children() > 0 )
                            ? mxLbLists->get_selected_index()
                            : 0;
            mxLbLists->unselect_all();
            mxFtLists->set_sensitive(false);
            mxLbLists->set_sensitive(false);
            mxFtEntries->set_sensitive(true);
            mxEdEntries->set_sensitive(true);
            mxEdEntries->set_text( EMPTY_OUSTRING );
            mxEdEntries->grab_focus();
            mxBtnAdd->set_sensitive(false);
            mxBtnModify->set_sensitive(false);
            mxBtnRemove->set_sensitive(false);

            if ( mxBtnCopy->get_sensitive() )
            {
                mxBtnCopy->set_sensitive(false);
                mxFtCopyFrom->set_sensitive(false);
                mxEdCopyFrom->set_sensitive(false);
            }
            mxBtnNew->hide();
            mxBtnDiscard->show();
            bCancelMode = true;
        }
        else // if ( bCancelMode )
        {
            if ( mxLbLists->n_children() > 0 )
            {
                mxLbLists->select( nCancelPos );
                LbSelectHdl( *mxLbLists );
                mxFtLists->set_sensitive(true);
                mxLbLists->set_sensitive(true);
            }
            else
            {
                mxFtEntries->set_sensitive(false);
                mxEdEntries->set_sensitive(false);
                mxEdEntries->set_text( EMPTY_OUSTRING );
                mxBtnRemove->set_sensitive(false);
            }
            mxBtnAdd->set_sensitive(false);
            mxBtnModify->set_sensitive(false);

            if ( pViewData && !bCopyDone )
            {
                mxBtnCopy->set_sensitive(true);
                mxFtCopyFrom->set_sensitive(true);
                mxEdCopyFrom->set_sensitive(true);
            }
            mxBtnNew->show();
            mxBtnDiscard->hide();
            bCancelMode = false;
            bModifyMode = false;
        }
    }
    else if (&rBtn == mxBtnAdd.get() || &rBtn == mxBtnModify.get())
    {
        OUString theEntriesStr( mxEdEntries->get_text() );

        if ( !bModifyMode )
        {
            if ( !theEntriesStr.isEmpty() )
            {
                AddNewList( theEntriesStr );
                UpdateUserListBox();
                mxLbLists->select( mxLbLists->n_children()-1 );
                LbSelectHdl( *mxLbLists );
                mxFtLists->set_sensitive(true);
                mxLbLists->set_sensitive(true);
            }
            else
            {
                if ( mxLbLists->n_children() > 0 )
                {
                    mxLbLists->select( nCancelPos );
                    LbSelectHdl( *mxLbLists );
                    mxLbLists->set_sensitive(true);
                    mxLbLists->set_sensitive(true);
                }
            }

            mxBtnAdd->set_sensitive(false);
            mxBtnModify->set_sensitive(false);
            mxBtnRemove->set_sensitive(true);
            mxBtnNew->show();
            mxBtnDiscard->hide();
            bCancelMode = false;
        }
        else // if ( bModifyMode )
        {
            sal_Int32 nSelList = mxLbLists->get_selected_index();

            OSL_ENSURE( nSelList != -1 , "Modify without List :-/" );

            if ( !theEntriesStr.isEmpty() )
            {
                ModifyList( nSelList, theEntriesStr );
                UpdateUserListBox();
                mxLbLists->select( nSelList );
            }
            else
            {
                mxLbLists->select( 0 );
                LbSelectHdl( *mxLbLists );
            }

            mxBtnNew->show();
            mxBtnDiscard->hide();
            bCancelMode = false;
            mxBtnAdd->show();
            mxBtnModify->show();
            mxBtnAdd->set_sensitive(false);
            mxBtnModify->set_sensitive(false);
            bModifyMode = false;
            mxBtnRemove->set_sensitive(true);
            mxFtLists->set_sensitive(true);
            mxLbLists->set_sensitive(true);
        }

        if ( pViewData && !bCopyDone )
        {
            mxBtnCopy->set_sensitive(true);
            mxFtCopyFrom->set_sensitive(true);
            mxEdCopyFrom->set_sensitive(true);
        }
    }
    else if ( &rBtn == mxBtnRemove.get() )
    {
        if ( mxLbLists->n_children() > 0 )
        {
            sal_Int32 nRemovePos   = mxLbLists->get_selected_index();
            OUString aMsg = aStrQueryRemove.getToken( 0, '#' )
                          + mxLbLists->get_text( nRemovePos )
                          + aStrQueryRemove.getToken( 1, '#' );

            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           aMsg));
            xQueryBox->set_default_response(RET_YES);

            if (RET_YES == xQueryBox->run())
            {
                RemoveList( nRemovePos );
                UpdateUserListBox();

                if ( mxLbLists->n_children() > 0 )
                {
                    mxLbLists->select(
                        ( nRemovePos >= mxLbLists->n_children() )
                            ? mxLbLists->n_children()-1
                            : nRemovePos );
                    LbSelectHdl( *mxLbLists );
                }
                else
                {
                    mxFtLists->set_sensitive(false);
                    mxLbLists->set_sensitive(false);
                    mxFtEntries->set_sensitive(false);
                    mxEdEntries->set_sensitive(false);
                    mxEdEntries->set_text( EMPTY_OUSTRING );
                    mxBtnRemove->set_sensitive(false);
                }
            }

            if ( pViewData && !bCopyDone && !mxBtnCopy->get_sensitive() )
            {
                mxBtnCopy->set_sensitive(true);
                mxFtCopyFrom->set_sensitive(true);
                mxEdCopyFrom->set_sensitive(true);
            }
        }
    }
    else if ( pViewData && (&rBtn == mxBtnCopy.get()) )
    {
        if ( bCopyDone )
            return;

        ScRefAddress theStartPos;
        ScRefAddress theEndPos;
        OUString     theAreaStr( mxEdCopyFrom->get_text() );
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
            mxLbLists->select( mxLbLists->n_children()-1 );
            LbSelectHdl( *mxLbLists );
            mxEdCopyFrom->set_text( theAreaStr );
            mxEdCopyFrom->set_sensitive(false);
            mxBtnCopy->set_sensitive(false);
            mxFtCopyFrom->set_sensitive(false);
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                        VclMessageType::Warning, VclButtonsType::Ok,
                        ScResId(STR_INVALID_TABREF)));

            xBox->run();
            mxEdCopyFrom->grab_focus();
            mxEdCopyFrom->select_region(0, -1);
        }
    }
}

IMPL_LINK( ScTpUserLists, EdEntriesModHdl, weld::TextView&, rEd, void )
{
    if ( &rEd != mxEdEntries.get() )
        return;

    if ( mxBtnCopy->get_sensitive() )
    {
        mxBtnCopy->set_sensitive(false);
        mxFtCopyFrom->set_sensitive(false);
        mxEdCopyFrom->set_sensitive(false);
    }

    if ( !mxEdEntries->get_text().isEmpty() )
    {
        if ( !bCancelMode && !bModifyMode )
        {
            mxBtnNew->hide();
            mxBtnDiscard->show();
            bCancelMode = true;
            mxBtnAdd->hide();
            mxBtnAdd->set_sensitive(true);
            mxBtnModify->show();
            mxBtnModify->set_sensitive(true);
            bModifyMode = true;
            mxBtnRemove->set_sensitive(false);
            mxFtLists->set_sensitive(false);
            mxLbLists->set_sensitive(false);
        }
        else // if ( bCancelMode || bModifyMode )
        {
            if ( !mxBtnAdd->get_sensitive() )
            {
                mxBtnAdd->set_sensitive(true);
                mxBtnModify->set_sensitive(true);
            }
        }
    }
    else
    {
        if ( mxBtnAdd->get_sensitive() )
        {
            mxBtnAdd->set_sensitive(false);
            mxBtnModify->set_sensitive(false);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
