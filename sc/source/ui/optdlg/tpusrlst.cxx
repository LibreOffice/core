/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#undef SC_DLLIMPLEMENTATION

//------------------------------------------------------------------
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

static USHORT pUserListsRanges[] =
{
    SID_SCUSERLISTS,
    SID_SCUSERLISTS,
    0
};

static const sal_Unicode cDelimiter = ',';


//========================================================================
// Benutzerdefinierte Listen:


ScTpUserLists::ScTpUserLists( Window*               pParent,
                              const SfxItemSet&     rCoreAttrs )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_USERLISTS ),
                          rCoreAttrs ),
        aFtLists        ( this, ScResId( FT_LISTS ) ),
        aLbLists        ( this, ScResId( LB_LISTS ) ),
        aFtEntries      ( this, ScResId( FT_ENTRIES ) ),
        aEdEntries      ( this, ScResId( ED_ENTRIES ) ),
        aFtCopyFrom     ( this, ScResId( FT_COPYFROM ) ),
        aEdCopyFrom     ( this, ScResId( ED_COPYFROM ) ),
        aBtnNew         ( this, ScResId( BTN_NEW ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnCopy        ( this, ScResId( BTN_COPY ) ),
        aStrQueryRemove ( ScResId( STR_QUERYREMOVE ) ),
        aStrNew         ( aBtnNew.GetText() ),
        aStrCancel      ( ScResId( STR_DISMISS ) ),
        aStrAdd         ( ScResId( SCSTR_ADD ) ),
        aStrModify      ( ScResId( SCSTR_MODIFY ) ),
        aStrCopyList    ( ScResId( STR_COPYLIST ) ),
        aStrCopyFrom    ( ScResId( STR_COPYFROM ) ),
        aStrCopyErr     ( ScResId( STR_COPYERR ) ),
        nWhichUserLists ( GetWhich( SID_SCUSERLISTS ) ),
        pUserLists      ( NULL ),
        pDoc            ( NULL ),
        pViewData       ( NULL ),
        pRangeUtil      ( new ScRangeUtil ),
        bModifyMode     ( FALSE ),
        bCancelMode     ( FALSE ),
        bCopyDone       ( FALSE ),
        nCancelPos      ( 0 )
{
    SetExchangeSupport();
    Init();
    FreeResource();
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

    aLbLists.SetSelectHdl   ( LINK( this, ScTpUserLists, LbSelectHdl ) );
    aBtnNew.SetClickHdl     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    aBtnNew.SetClickHdl     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScTpUserLists, BtnClickHdl ) );
    aEdEntries.SetModifyHdl ( LINK( this, ScTpUserLists, EdEntriesModHdl ) );


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

        ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab
                ).Format( aStrSelectedArea, SCR_ABS_3D, pDoc );

        aBtnCopy.SetClickHdl ( LINK( this, ScTpUserLists, BtnClickHdl ) );
        aBtnCopy.Enable();
    }
    else
    {
        aBtnCopy.Disable();
        aFtCopyFrom.Disable();
        aEdCopyFrom.Disable();
    }
}

// -----------------------------------------------------------------------

USHORT* ScTpUserLists::GetRanges()
{
    return pUserListsRanges;
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

    DBG_ASSERT( pCoreList, "UserList not found :-/" );

    if ( pCoreList )
    {
        if ( !pUserLists )
            pUserLists = new ScUserList( *pCoreList );
        else
            *pUserLists = *pCoreList;

        if ( UpdateUserListBox() > 0 )
        {
            aLbLists.SelectEntryPos( 0 );
            UpdateEntries( 0 );
        }
    }
    else if ( !pUserLists )
        pUserLists = new ScUserList;

    aEdCopyFrom.SetText( aStrSelectedArea );

    if ( aLbLists.GetEntryCount() == 0 )
    {
        aFtLists    .Disable();
        aLbLists    .Disable();
        aFtEntries  .Disable();
        aEdEntries  .Disable();
        aBtnRemove  .Disable();
    }

    aBtnNew.SetText( aStrNew );
    aBtnAdd.SetText( aStrAdd );
    aBtnAdd.Disable();

    if ( !bCopyDone && pViewData )
    {
        aFtCopyFrom .Enable();
        aEdCopyFrom .Enable();
        aBtnCopy    .Enable();
    }
}

// -----------------------------------------------------------------------

BOOL ScTpUserLists::FillItemSet( SfxItemSet& rCoreAttrs )
{
    // Modifikationen noch nicht uebernommen?
    // -> Click auf Add-Button simulieren

    if ( bModifyMode || bCancelMode )
        BtnClickHdl( &aBtnAdd );

    const ScUserListItem& rUserListItem = (const ScUserListItem&)
                                           GetItemSet().Get( nWhichUserLists );

    ScUserList* pCoreList       = rUserListItem.GetUserList();
    BOOL        bDataModified   = FALSE;

    if ( (pUserLists == NULL) && (pCoreList == NULL) )
    {
        bDataModified = FALSE;
    }
    else if ( pUserLists != NULL )
    {
        if ( pCoreList != NULL )
            bDataModified = (*pUserLists != *pCoreList);
        else
            bDataModified = TRUE;
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

USHORT ScTpUserLists::UpdateUserListBox()
{
    aLbLists.Clear();

    if ( !pUserLists ) return 0;

    //----------------------------------------------------------

    USHORT  nCount = pUserLists->GetCount();
    String  aEntry;

    if ( nCount > 0 )
    {
        for ( USHORT i=0; i<nCount; i++ )
        {
            aEntry = (*pUserLists)[i]->GetString();
            DBG_ASSERT( aEntry.Len() > 0, "Empty UserList-entry :-/" );
            aLbLists.InsertEntry( aEntry );
        }
    }

    return nCount;
}

// -----------------------------------------------------------------------

void ScTpUserLists::UpdateEntries( USHORT nList )
{
    if ( !pUserLists ) return;

    //----------------------------------------------------------

    if ( nList < pUserLists->GetCount() )
    {
        ScUserListData* pList     = (*pUserLists)[nList];
        USHORT          nSubCount = pList->GetSubCount();
        String          aEntryListStr;

        for ( USHORT i=0; i<nSubCount; i++ )
        {
            if ( i!=0 )
                aEntryListStr += CR;
            aEntryListStr += pList->GetSubStr( i );
        }

        aEntryListStr.ConvertLineEnd();
        aEdEntries.SetText( aEntryListStr );
    }
    else
    {
        OSL_FAIL( "Invalid ListIndex :-/" );
    }
}

// -----------------------------------------------------------------------

void ScTpUserLists::MakeListStr( String& rListStr )
{
    String  aInputStr(rListStr);
    String  aStr;

    xub_StrLen  nLen    = aStr.Len();
    xub_StrLen  c       = 0;

    aInputStr.ConvertLineEnd( LINEEND_LF );

    xub_StrLen nToken=rListStr.GetTokenCount(LF);

    for(xub_StrLen i=0;i<nToken;i++)
    {
        String aString=rListStr.GetToken(i,LF);
        aString.EraseLeadingChars(' ');
        aString.EraseTrailingChars(' ');
        aStr+=aString;
        aStr+=cDelimiter;
    }

    aStr.EraseLeadingChars( cDelimiter );
    aStr.EraseTrailingChars( cDelimiter );
    nLen = aStr.Len();

    rListStr.Erase();

    // Alle Doppelten cDelimiter entfernen:
    c=0;
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

    if ( !pUserLists->Insert( new ScUserListData( theEntriesStr ) ) )
    {
        OSL_FAIL( "Entry could not be inserted :-/" );
    }
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
    USHORT  nCellDir        = SCRET_COLS;
    BOOL    bValueIgnored   = FALSE;

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
                        pDoc->GetString( col, row, nTab, aStrField );

                        if ( aStrField.Len() > 0 )
                        {
                            aStrList += aStrField;
                            aStrList += '\n';
                        }
                    }
                    else
                        bValueIgnored = TRUE;
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
                        pDoc->GetString( col, row, nTab, aStrField );

                        if ( aStrField.Len() > 0 )
                        {
                            aStrList += aStrField;
                            aStrList += '\n';
                        }
                    }
                    else
                        bValueIgnored = TRUE;
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

    bCopyDone = TRUE;

}

// -----------------------------------------------------------------------

void ScTpUserLists::ModifyList( USHORT          nSelList,
                                const String&   rEntriesStr )
{
    if ( !pUserLists ) return;

    //----------------------------------------------------------

    String theEntriesStr( rEntriesStr );

    MakeListStr( theEntriesStr );

    (*pUserLists)[nSelList]->SetString( theEntriesStr );
}

// -----------------------------------------------------------------------

void ScTpUserLists::RemoveList( USHORT nList )
{
    if ( pUserLists ) pUserLists->AtFree( nList );
}

//-----------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScTpUserLists, LbSelectHdl, ListBox*, pLb )
{
    if ( pLb == &aLbLists )
    {
        USHORT nSelPos = aLbLists.GetSelectEntryPos();
        if ( nSelPos != LISTBOX_ENTRY_NOTFOUND )
        {
            if ( !aFtEntries.IsEnabled() )  aFtEntries.Enable();
            if ( !aEdEntries.IsEnabled() )  aEdEntries.Enable();
            if ( !aBtnRemove.IsEnabled() )  aBtnRemove.Enable();
            if (  aBtnAdd.IsEnabled() )     aBtnAdd.Disable();

            UpdateEntries( nSelPos );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpUserLists, BtnClickHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnNew )
    {
        if ( !bCancelMode )
        {
            nCancelPos = ( aLbLists.GetEntryCount() > 0 )
                            ? aLbLists.GetSelectEntryPos()
                            : 0;
            aLbLists.SetNoSelection();
            aFtLists.Disable();
            aLbLists.Disable();
            aFtEntries.Enable();
            aEdEntries.Enable();
            aEdEntries.SetText( EMPTY_STRING );
            aEdEntries.GrabFocus();
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            //-----------------------------
            if ( aBtnCopy.IsEnabled() )
            {
                aBtnCopy.Disable();
                aFtCopyFrom.Disable();
                aEdCopyFrom.Disable();
            }
            aBtnNew.SetText( aStrCancel );
            bCancelMode = TRUE;
        }
        else // if ( bCancelMode )
        {
            if ( aLbLists.GetEntryCount() > 0 )
            {
                aLbLists.SelectEntryPos( nCancelPos );
                LbSelectHdl( &aLbLists );
                aFtLists.Enable();
                aLbLists.Enable();
            }
            else
            {
                aFtEntries.Disable();
                aEdEntries.Disable();
                aEdEntries.SetText( EMPTY_STRING );
                aBtnRemove.Disable();
            }
            aBtnAdd.Disable();
            //-----------------------------
            if ( pViewData && !bCopyDone )
            {
                aBtnCopy.Enable();
                aFtCopyFrom.Enable();
                aEdCopyFrom.Enable();
            }
            aBtnNew.SetText( aStrNew );
            bCancelMode = FALSE;
            bModifyMode = FALSE;
        }
    }
    else if ( pBtn == &aBtnAdd )
    {
        String theEntriesStr( aEdEntries.GetText() );

        if ( !bModifyMode )
        {
            if ( theEntriesStr.Len() > 0 )
            {
                AddNewList( theEntriesStr );
                UpdateUserListBox();
                aLbLists.SelectEntryPos( aLbLists.GetEntryCount()-1 );
                LbSelectHdl( &aLbLists );
                aFtLists.Enable();
                aLbLists.Enable();
            }
            else
            {
                if ( aLbLists.GetEntryCount() > 0 )
                {
                    aLbLists.SelectEntryPos( nCancelPos );
                    LbSelectHdl( &aLbLists );
                    aLbLists.Enable();
                    aLbLists.Enable();
                }
            }

            aBtnAdd.Disable();
            aBtnRemove.Enable();
            aBtnNew.SetText( aStrNew );
            bCancelMode = FALSE;
        }
        else // if ( bModifyMode )
        {
            USHORT nSelList = aLbLists.GetSelectEntryPos();

            DBG_ASSERT( nSelList != LISTBOX_ENTRY_NOTFOUND, "Modify without List :-/" );

            if ( theEntriesStr.Len() > 0 )
            {
                ModifyList( nSelList, theEntriesStr );
                UpdateUserListBox();
                aLbLists.SelectEntryPos( nSelList );
            }
            else
            {
                aLbLists.SelectEntryPos( 0 );
                LbSelectHdl( &aLbLists );
            }

            aBtnNew.SetText( aStrNew ); bCancelMode = FALSE;
            aBtnAdd.SetText( aStrAdd ); bModifyMode = FALSE;
            aBtnAdd.Disable();
            aBtnRemove.Enable();
            aFtLists.Enable();
            aLbLists.Enable();
        }

        if ( pViewData && !bCopyDone )
        {
            aBtnCopy.Enable();
            aFtCopyFrom.Enable();
            aEdCopyFrom.Enable();
        }
    }
    else if ( pBtn == &aBtnRemove )
    {
        if ( aLbLists.GetEntryCount() > 0 )
        {
            USHORT nRemovePos   = aLbLists.GetSelectEntryPos();
            String aMsg         ( aStrQueryRemove.GetToken( 0, '#' ) );

            aMsg += aLbLists.GetEntry( nRemovePos );
            aMsg += aStrQueryRemove.GetToken( 1, '#' );


            if ( RET_YES == QueryBox( this,
                                      WinBits( WB_YES_NO | WB_DEF_YES ),
                                      aMsg
                                     ).Execute() )
            {
                RemoveList( nRemovePos );
                UpdateUserListBox();

                if ( aLbLists.GetEntryCount() > 0 )
                {
                    aLbLists.SelectEntryPos(
                        ( nRemovePos >= aLbLists.GetEntryCount() )
                            ? aLbLists.GetEntryCount()-1
                            : nRemovePos );
                    LbSelectHdl( &aLbLists );
                }
                else
                {
                    aFtLists.Disable();
                    aLbLists.Disable();
                    aFtEntries.Disable();
                    aEdEntries.Disable();
                    aEdEntries.SetText( EMPTY_STRING );
                    aBtnRemove.Disable();
                }
            }

            if ( pViewData && !bCopyDone && !aBtnCopy.IsEnabled() )
            {
                aBtnCopy.Enable();
                aFtCopyFrom.Enable();
                aEdCopyFrom.Enable();
            }
        }
    }
    else if ( pViewData && (pBtn == &aBtnCopy) )
    {
        if ( bCopyDone )
            return 0;

        //-----------------------------------------------------------

        ScRefAddress theStartPos;
        ScRefAddress theEndPos;
        String      theAreaStr( aEdCopyFrom.GetText() );
        BOOL        bAreaOk = FALSE;

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
            aLbLists.SelectEntryPos( aLbLists.GetEntryCount()-1 );
            LbSelectHdl( &aLbLists );
            aEdCopyFrom .SetText( theAreaStr );
            aEdCopyFrom .Disable();
            aBtnCopy    .Disable();
            aFtCopyFrom .Disable();
        }
        else
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                      ScGlobal::GetRscString( STR_INVALID_TABREF )
                    ).Execute();
            aEdCopyFrom.GrabFocus();
            aEdCopyFrom.SetSelection( Selection( 0, SELECTION_MAX ) );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpUserLists, EdEntriesModHdl, MultiLineEdit*, pEd )
{
    if ( pEd != &aEdEntries )
        return 0;

    //-----------------------------------------------------------

    if ( aBtnCopy.IsEnabled() )
    {
        aBtnCopy    .Disable();
        aFtCopyFrom .Disable();
        aEdCopyFrom .Disable();
    }

    if ( aEdEntries.GetText().Len() > 0 )
    {
        if ( !bCancelMode && !bModifyMode )
        {
            aBtnNew.SetText( aStrCancel );  bCancelMode = TRUE;
            aBtnAdd.SetText( aStrModify );  bModifyMode = TRUE;
            aBtnAdd.Enable();
            aBtnRemove.Disable();
            aFtLists.Disable();
            aLbLists.Disable();
        }
        else // if ( bCancelMode || bModifyMode )
        {
            if ( !aBtnAdd.IsEnabled() ) aBtnAdd.Enable();
        }
    }
    else
    {
        if ( aBtnAdd.IsEnabled() ) aBtnAdd.Disable();
    }

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
