/*************************************************************************
 *
 *  $RCSfile: tpusrlst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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
        aLbLists        ( this, ScResId( LB_LISTS ) ),
        aEdEntries      ( this, ScResId( ED_ENTRIES ) ),
        aEdCopyFrom     ( this, ScResId( ED_COPYFROM ) ),
        aFtLists        ( this, ScResId( FT_LISTS ) ),
        aFtEntries      ( this, ScResId( FT_ENTRIES ) ),
        aFtCopyFrom     ( this, ScResId( FT_COPYFROM ) ),
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
        //
        pRangeUtil      ( new ScRangeUtil ),
        nWhichUserLists ( GetWhich( SID_SCUSERLISTS ) ),
        pUserLists      ( NULL ),
        pDoc            ( NULL ),
        pViewData       ( NULL ),
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

__EXPORT ScTpUserLists::~ScTpUserLists()
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
        USHORT  nStartTab   = 0;
        USHORT  nEndTab     = 0;
        USHORT  nStartCol   = 0;
        USHORT  nStartRow   = 0;
        USHORT  nEndCol     = 0;
        USHORT  nEndRow     = 0;

        pViewData = pViewSh->GetViewData();
        pDoc = pViewData->GetDocument();

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        pRangeUtil->MakeAreaString( ScArea( nStartTab,
                                            nStartCol, nStartRow,
                                            nEndCol,   nEndRow ),
                                    aStrSelectedArea,
                                    pDoc );

        aBtnCopy.SetClickHdl ( LINK( this, ScTpUserLists, BtnClickHdl ) );
        aBtnCopy.Enable();
    }
    else
    {
        aBtnCopy.Disable();
        aFtCopyFrom.Disable();
        aEdCopyFrom.Disable();
    }

//  aLbLists.GrabFocus();
}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScTpUserLists::GetRanges()
{
    return pUserListsRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTpUserLists::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTpUserLists( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

void __EXPORT ScTpUserLists::Reset( const SfxItemSet& rCoreAttrs )
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

//  aLbLists.GrabFocus();
}

// -----------------------------------------------------------------------

BOOL __EXPORT ScTpUserLists::FillItemSet( SfxItemSet& rCoreAttrs )
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

int __EXPORT ScTpUserLists::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

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
        DBG_ERROR( "Invalid ListIndex :-/" );
}

// -----------------------------------------------------------------------

void ScTpUserLists::MakeListStr( String& rListStr )
{
    String  aInputStr(rListStr);
    String  aStr;

    xub_StrLen  nLen    = aStr.Len();
    xub_StrLen  nFound  = 0;
    xub_StrLen  c       = 0;

    aInputStr.ConvertLineEnd( LINEEND_LF );
    //aStr.EraseAllChars( ' ' );

    xub_StrLen nToken=rListStr.GetTokenCount(LF);

    for(xub_StrLen i=0;i<nToken;i++)
    {
        String aString=rListStr.GetToken(i,LF);
        aString.EraseLeadingChars(' ');
        aString.EraseTrailingChars(' ');
        aStr+=aString;
        aStr+=cDelimiter;
    }

    /*
    // '\n' durch cDelimiter ersetzen:
    for ( c=0;
          (c < nLen) && (nFound != STRING_NOTFOUND);
          c++ )
    {
        nFound = aStr.Search( LF, nFound );
        if ( nFound != STRING_NOTFOUND )
            aStr[nFound] = cDelimiter;
    }
    */

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
        DBG_ERROR( "Entry could not be inserted :-/" );
}

// -----------------------------------------------------------------------

void ScTpUserLists::CopyListFromArea( const ScRefTripel& rStartPos,
                                      const ScRefTripel& rEndPos )
{
    if ( bCopyDone ) return;

    //----------------------------------------------------------

    USHORT  nTab            = rStartPos.GetTab();
    USHORT  nStartCol       = rStartPos.GetCol();
    USHORT  nStartRow       = rStartPos.GetRow();
    USHORT  nEndCol         = rEndPos.GetCol();
    USHORT  nEndRow         = rEndPos.GetRow();
    USHORT  nCellDir        = SCRET_COLS;
    BOOL    bValueIgnored   = FALSE;

    if ( (nStartCol != nEndCol) && (nStartRow != nEndRow) )
        nCellDir = ScColOrRowDlg( this, aStrCopyList, aStrCopyFrom ).Execute();
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
            for ( USHORT col=nStartCol; col<=nEndCol; col++ )
            {
                for ( USHORT row=nStartRow; row<=nEndRow; row++ )
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
            for ( USHORT row=nStartRow; row<=nEndRow; row++ )
            {
                for ( USHORT col=nStartCol; col<=nEndCol; col++ )
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
        if ( bCopyDone ) return NULL;

        //-----------------------------------------------------------

        ScRefTripel theStartPos;
        ScRefTripel theEndPos;
        String      theAreaStr( aEdCopyFrom.GetText() );
        BOOL        bAreaOk = FALSE;

        if ( theAreaStr.Len() > 0 )
        {
            bAreaOk = pRangeUtil->IsAbsArea( theAreaStr,
                                             pDoc,
                                             pViewData->GetTabNo(),
                                             &theAreaStr,
                                             &theStartPos,
                                             &theEndPos );
            if ( !bAreaOk )
            {
                bAreaOk = pRangeUtil->IsAbsPos(  theAreaStr,
                                                 pDoc,
                                                 pViewData->GetTabNo(),
                                                 &theAreaStr,
                                                 &theStartPos );
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

    return NULL;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTpUserLists, EdEntriesModHdl, MultiLineEdit*, pEd )
{
    if ( pEd != &aEdEntries ) return NULL;

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

    return NULL;
}



