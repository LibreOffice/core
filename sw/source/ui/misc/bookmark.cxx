/*************************************************************************
 *
 *  $RCSfile: bookmark.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:44 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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

    if (nLen && (aBookmarkBox.GetEntryPos(aTmpEntry) == COMBOBOX_ENTRY_NOTFOUND))
    {
        String sEntry(aBookmarkBox.GetText());
        sEntry.EraseAllChars(aBookmarkBox.GetMultiSelectionSeparator());

        rSh.SetBookmark( KeyCode(), sEntry, aEmptyStr );
    }

    for (USHORT nCount = aBookmarkBox.GetRemovedCount(); nCount > 0; nCount--)
    {
        rSh.DelBookmark( aBookmarkBox.GetRemovedEntry( nCount -1 ).aName );
    }
}

/*------------------------------------------------------------------------
     Beschreibung: CTOR
 -----------------------------------------------------------------------*/


SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS ) :

    SvxStandardDialog(pParent,SW_RES(DLG_INSERT_BOOKMARK)),

    aBookmarkBox(this,SW_RES(CB_BOOKMARK)),
    aBookmarkFrm(this,SW_RES(GB_BOOKMARK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aDeleteBtn(this,SW_RES(BT_DELETE)),
    rSh( rS )
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
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        String sKey = pKEvt->GetCharCode();
        if(STRING_NOTFOUND != aForbiddenChars.Search(sKey))
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = SwComboBox::PreNotify( rNEvt );
    return nHandled;
}
/*------------------------------------------------------------------------

      $Log: not supported by cvs2svn $
      Revision 1.50  2000/09/18 16:05:55  willem.vandorp
      OpenOffice header added.

      Revision 1.49  2000/05/10 11:53:42  os
      Basic API removed

      Revision 1.48  2000/04/18 15:08:16  os
      UNICODE

      Revision 1.47  1999/07/02 07:13:56  OS
      #63003# NameWarning also in StarOne rename dialog


      Rev 1.46   02 Jul 1999 09:13:56   OS
   #63003# NameWarning also in StarOne rename dialog

      Rev 1.45   18 Jun 1999 13:04:58   OS
   #63003# prevent illegal characters from beeing pasted

      Rev 1.44   25 Feb 1999 13:43:04   OS
   #62407# AutoComplete casesensitive

      Rev 1.43   08 Feb 1999 15:04:12   OS
   #61421# einige Sonderzeichen in Textmarken nicht erlauben

      Rev 1.42   08 Oct 1998 15:45:34   OM
   #57748# Multiselektion beim Loeschen von Bookmarks

      Rev 1.41   23 Feb 1998 07:28:18   OS
   GetBookmark/ Cnt mit Bookmark-Flag

      Rev 1.40   24 Nov 1997 16:47:42   MA
   includes

      Rev 1.39   03 Nov 1997 13:22:42   MA
   precomp entfernt

      Rev 1.38   08 Aug 1997 17:28:20   OM
   Headerfile-Umstellung

      Rev 1.37   30 Jul 1997 18:33:30   HJS
   includes

      Rev 1.36   07 Apr 1997 16:08:26   MH
   chg: header

      Rev 1.35   20 Feb 1997 10:00:44   MA
   #36242# richtig enablen

      Rev 1.34   14 Jan 1997 09:22:28   TRI
   includes wegen Internal Compiler Error

      Rev 1.33   11 Nov 1996 11:05:46   MA
   ResMgr

      Rev 1.32   02 Oct 1996 18:28:58   MA
   Umstellung Enable/Disable

      Rev 1.31   28 Aug 1996 14:12:20   OS
   includes

      Rev 1.30   06 Feb 1996 15:21:06   JP
   Link Umstellung 305

      Rev 1.29   24 Nov 1995 16:58:44   OM
   PCH->PRECOMPILED

      Rev 1.28   08 Nov 1995 13:31:40   JP
   Umstellung zur 301: Change -> Set

      Rev 1.27   24 Oct 1995 17:05:06   OS
   Textmarken einfuegen/loeschen recordable

      Rev 1.26   17 Oct 1995 20:54:50   JP
   IsMark oder IsBookmark am Bookmark benutzen

      Rev 1.25   30 Aug 1995 14:00:54   MA
   fix: sexport'iert

      Rev 1.24   21 Aug 1995 09:32:42   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.23   05 Jul 1995 19:08:14   JP
   SwBookmarkFUNC gibts nicht mehr, rufe Methoden an die Shell-Methoden

      Rev 1.22   20 Mar 1995 19:28:20   OS
   unbenutzte Funktionen entfernt

      Rev 1.21   26 Oct 1994 12:18:00   ER
   add: PCH (missed the misc)

 -----------------------------------------------------------------------*/



