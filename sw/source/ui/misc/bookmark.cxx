/*************************************************************************
 *
 *  $RCSfile: bookmark.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:35:10 $
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


#pragma hdrstop

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


#include "view.hxx"
#include "basesh.hxx"
#include "wrtsh.hxx"        //
#include "cmdid.h"
#include "bookmark.hxx"     // SwInsertBookmarkDlg
#include "bookmrk.hxx"      //  SwBookmark
#include "bookmark.hrc"
#include "misc.hrc"


const String BookmarkCombo::aForbiddenChars = String::CreateFromAscii("/\\@:*?\";,.#");


IMPL_LINK( SwInsertBookmarkDlg, ModifyHdl, BookmarkCombo *, pBox )
{
    BOOL bSelEntries = pBox->GetSelectEntryCount() != 0;
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    if(!bSelEntries)
    {
        String sTmp = pBox->GetText();
        USHORT nLen = sTmp.Len();
        String sMsg;
        for(USHORT i = 0; i < BookmarkCombo::aForbiddenChars.Len(); i++)
        {
            USHORT nTmpLen = sTmp.Len();
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

    for (USHORT i = aBookmarkBox.GetSelectEntryCount(); i; i-- )
        aBookmarkBox.RemoveEntry(aBookmarkBox.GetSelectEntryPos(i - 1));

    aBookmarkBox.SetText(aEmptyStr);
    aDeleteBtn.Enable(FALSE);   // keine weiteren Eintraege vorhanden
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
    // Textmarke einfuegen
    USHORT      nLen = aBookmarkBox.GetText().Len();
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

    for (USHORT nCount = aBookmarkBox.GetRemovedCount(); nCount > 0; nCount--)
    {
        String sRemoved = aBookmarkBox.GetRemovedEntry( nCount -1 ).aName;
        rSh.DelBookmark( sRemoved );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
    }
}

/*------------------------------------------------------------------------
     Beschreibung: CTOR
 -----------------------------------------------------------------------*/


SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :

    SvxStandardDialog(pParent,SW_RES(DLG_INSERT_BOOKMARK)),

    aBookmarkBox(this,SW_RES(CB_BOOKMARK)),
    aBookmarkFl(this,SW_RES(FL_BOOKMARK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aDeleteBtn(this,SW_RES(BT_DELETE)),
    rSh( rS ),
    rReq( rRequest )
{
    aBookmarkBox.SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    aBookmarkBox.EnableMultiSelection(TRUE);
    aBookmarkBox.EnableAutocomplete( TRUE, TRUE );

    aDeleteBtn.SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));

    // Combobox mit vorhandenen Bookmarks fuellen
    USHORT nCount = rSh.GetBookmarkCnt(TRUE);

    for( USHORT nId = 0; nId < nCount; nId++ )
    {
        SwBookmark& rBkmk = rSh.GetBookmark( nId, TRUE );
        aBookmarkBox.InsertEntry( SwBoxEntry( rBkmk.GetName(), nId ) );
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

USHORT BookmarkCombo::GetFirstSelEntryPos() const
{
    return GetSelEntryPos(0);
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

USHORT BookmarkCombo::GetNextSelEntryPos(USHORT nPos) const
{
    return GetSelEntryPos(nPos + 1);
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

USHORT BookmarkCombo::GetSelEntryPos(USHORT nPos) const
{
    char cSep = GetMultiSelectionSeparator();

    USHORT nCnt = GetText().GetTokenCount(cSep);

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

USHORT BookmarkCombo::GetSelectEntryCount() const
{
    USHORT nCnt = 0;

    USHORT nPos = GetFirstSelEntryPos();
    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return nCnt;
}

/*------------------------------------------------------------------------
     Beschreibung:
 -----------------------------------------------------------------------*/

String BookmarkCombo::GetSelectEntry( USHORT nSelIndex ) const
{
    USHORT nCnt = 0;
    USHORT nPos = GetFirstSelEntryPos();
    String sEntry;

    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        if (nSelIndex == nCnt)
        {
            char cSep = GetMultiSelectionSeparator();
            sEntry = GetText().GetToken(nPos, cSep);
            sEntry.EraseLeadingChars();
            sEntry.EraseTrailingChars();

            break;
        }
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return sEntry;
}

/*------------------------------------------------------------------------
     Beschreibung: Position in der Listbox (der ComboBox)
 -----------------------------------------------------------------------*/

USHORT BookmarkCombo::GetSelectEntryPos( USHORT nSelIndex ) const
{
    USHORT nCnt = 0;
    USHORT nPos = GetFirstSelEntryPos();

    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        if (nSelIndex == nCnt)
        {
            char cSep = GetMultiSelectionSeparator();
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



