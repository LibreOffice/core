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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>


#include "view.hxx"
#include "basesh.hxx"
#include "wrtsh.hxx"        //
#include "cmdid.h"
#include "bookmark.hxx"     // SwInsertBookmarkDlg
#include "IMark.hxx"
#include "bookmark.hrc"
#include "misc.hrc"


const String BookmarkCombo::aForbiddenChars = String::CreateFromAscii("/\\@:*?\";,.#");


IMPL_LINK( SwInsertBookmarkDlg, ModifyHdl, BookmarkCombo *, pBox )
{
    sal_Bool bSelEntries = pBox->GetSelectEntryCount() != 0;
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    if(!bSelEntries)
    {
        String sTmp = pBox->GetText();
        sal_uInt16 nLen = sTmp.Len();
        String sMsg;
        for(sal_uInt16 i = 0; i < BookmarkCombo::aForbiddenChars.Len(); i++)
        {
            sal_uInt16 nTmpLen = sTmp.Len();
            sTmp.EraseAllChars(BookmarkCombo::aForbiddenChars.GetChar(i));
            if(sTmp.Len() != nTmpLen)
                sMsg += BookmarkCombo::aForbiddenChars.GetChar(i);
        }
        if(sTmp.Len() != nLen)
        {
            pBox->SetText(sTmp);
            String sWarning(sRemoveWarning);
            sWarning += sMsg;
            InfoBox(this, sWarning).Execute();
        }


    }

    aOkBtn.Enable(!bSelEntries);    // neue Textmarke
    aDeleteBtn.Enable(bSelEntries); // loeschbar?

    return 0;
}

/*------------------------------------------------------------------------
     Beschreibung: Callback zum Loeschen einer Textmarke
 -----------------------------------------------------------------------*/

IMPL_LINK( SwInsertBookmarkDlg, DeleteHdl, Button *, EMPTYARG )
{
    // Textmarken aus der ComboBox entfernen

    for (sal_uInt16 i = aBookmarkBox.GetSelectEntryCount(); i; i-- )
        aBookmarkBox.RemoveEntry(aBookmarkBox.GetSelectEntryPos(i - 1));

    aBookmarkBox.SetText(aEmptyStr);
    aDeleteBtn.Enable(sal_False);   // keine weiteren Eintraege vorhanden
    // aBookmarkBox.SetText(aEmptyStr);

    aOkBtn.Enable();            // Im OK Handler wird geloescht
    return 0;
}

/*------------------------------------------------------------------------
     Beschreibung: Callback fuer OKButton. Fuegt eine neue Textmarke
     an die akt. Position ein. Geloeschte Textmarken werden auch am Modell
     entfernt.
 -----------------------------------------------------------------------*/


void SwInsertBookmarkDlg::Apply()
{
    //at first remove deleted bookmarks to prevent multiple bookmarks with the same
    //name
    for (sal_uInt16 nCount = aBookmarkBox.GetRemovedCount(); nCount > 0; nCount--)
    {
        String sRemoved = aBookmarkBox.GetRemovedEntry( nCount -1 ).GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark( pMarkAccess->findMark(sRemoved) );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
    }

    // Textmarke einfuegen
    sal_uInt16      nLen = aBookmarkBox.GetText().Len();
    SwBoxEntry  aTmpEntry(aBookmarkBox.GetText(), 0 );

    if ( nLen && (aBookmarkBox.GetEntryPos(aTmpEntry) == COMBOBOX_ENTRY_NOTFOUND) )
    {
        String sEntry(aBookmarkBox.GetText());
        sEntry.EraseAllChars(aBookmarkBox.GetMultiSelectionSeparator());

        rSh.SetBookmark( KeyCode(), sEntry, aEmptyStr );
        rReq.AppendItem( SfxStringItem( FN_INSERT_BOOKMARK, sEntry ) );
        rReq.Done();
    }

    if ( !rReq.IsDone() )
        rReq.Ignore();

}

/*------------------------------------------------------------------------
     Beschreibung: CTOR
 -----------------------------------------------------------------------*/


SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :

    SvxStandardDialog(pParent,SW_RES(DLG_INSERT_BOOKMARK)),
    aBookmarkFl(this,SW_RES(FL_BOOKMARK)),
    aBookmarkBox(this,SW_RES(CB_BOOKMARK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aDeleteBtn(this,SW_RES(BT_DELETE)),
    rSh( rS ),
    rReq( rRequest )
{
    aBookmarkBox.SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    aBookmarkBox.EnableMultiSelection(sal_True);
    aBookmarkBox.EnableAutocomplete( sal_True, sal_True );

    aDeleteBtn.SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));

    // Combobox mit vorhandenen Bookmarks fuellen
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    sal_uInt16 nId = 0;
    for( IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ppBookmark++)
    {
        if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
            aBookmarkBox.InsertEntry( SwBoxEntry( ppBookmark->get()->GetName(), nId++ ) );
    }
    FreeResource();
    sRemoveWarning = String(SW_RES(STR_REMOVE_WARNING));
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

BookmarkCombo::BookmarkCombo( Window* pWin, const ResId& rResId ) :
    SwComboBox(pWin, rResId)
{
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

sal_uInt16 BookmarkCombo::GetFirstSelEntryPos() const
{
    return GetSelEntryPos(0);
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

sal_uInt16 BookmarkCombo::GetNextSelEntryPos(sal_uInt16 nPos) const
{
    return GetSelEntryPos(nPos + 1);
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

sal_uInt16 BookmarkCombo::GetSelEntryPos(sal_uInt16 nPos) const
{
    sal_Unicode cSep = GetMultiSelectionSeparator();

    sal_uInt16 nCnt = GetText().GetTokenCount(cSep);

    for (; nPos < nCnt; nPos++)
    {
        String sEntry(GetText().GetToken(nPos, cSep));
        sEntry.EraseLeadingChars();
        sEntry.EraseTrailingChars();
        if (GetEntryPos(sEntry) != COMBOBOX_ENTRY_NOTFOUND)
            return nPos;
    }

    return COMBOBOX_ENTRY_NOTFOUND;
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

sal_uInt16 BookmarkCombo::GetSelectEntryCount() const
{
    sal_uInt16 nCnt = 0;

    sal_uInt16 nPos = GetFirstSelEntryPos();
    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return nCnt;
}

/*------------------------------------------------------------------------
     Beschreibung: Position in der Listbox (der ComboBox)
 -----------------------------------------------------------------------*/

sal_uInt16 BookmarkCombo::GetSelectEntryPos( sal_uInt16 nSelIndex ) const
{
    sal_uInt16 nCnt = 0;
    sal_uInt16 nPos = GetFirstSelEntryPos();

    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        if (nSelIndex == nCnt)
        {
            sal_Unicode cSep = GetMultiSelectionSeparator();
            String sEntry(GetText().GetToken(nPos, cSep));
            sEntry.EraseLeadingChars();
            sEntry.EraseTrailingChars();

            return GetEntryPos(sEntry);
        }
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return COMBOBOX_ENTRY_NOTFOUND;
}
/* -----------------05.02.99 08:39-------------------
 *
 * --------------------------------------------------*/
long BookmarkCombo::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if( EVENT_KEYINPUT == rNEvt.GetType() &&
         rNEvt.GetKeyEvent()->GetCharCode() )
    {
        String sKey( rNEvt.GetKeyEvent()->GetCharCode() );
        if(STRING_NOTFOUND != aForbiddenChars.Search(sKey))
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = SwComboBox::PreNotify( rNEvt );
    return nHandled;
}



