/*************************************************************************
 *
 *  $RCSfile: insfnote.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:45 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX //autogen
#include <svx/charmap.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#include <swundo.hxx>
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _INSFNOTE_HXX
#include <insfnote.hxx>
#endif

#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _INSFNOTE_HRC
#include <insfnote.hrc>
#endif

static BOOL bFootnote = TRUE;

/*------------------------------------------------------------------------
 Beschreibung:  Einfuegen der Fussnote durch OK
------------------------------------------------------------------------*/

void __EXPORT SwInsFootNoteDlg::Apply()
{
    String aStr;

    if ( aNumberCharBtn.IsChecked() )
        aStr = aNumberCharEdit.GetText();

    if ( bEdit )
    {
        rSh.StartAction();
        rSh.Left();
        rSh.StartUndo( UNDO_START );
        SwFmtFtn aNote( aEndNoteBtn.IsChecked() );
        aNote.SetNumStr( aStr );

        if( rSh.SetCurFtn( aNote ) && bExtCharAvailable )
        {
            rSh.Right(TRUE);
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetAttr( aSet );
            SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               eCharSet );
            aSet.Put( aFont );
            rSh.SetAttr( aSet, SETATTR_DONTEXPAND );
            rSh.ResetSelect(0, FALSE);
            rSh.Left();
        }
        rSh.EndUndo( UNDO_END );
        rSh.EndAction();
    }
    else
    {
        rSh.StartUndo( UIUNDO_INSERT_FOOTNOTE );
        rSh.InsertFootnote( aStr, aEndNoteBtn.IsChecked(), !bExtCharAvailable );

        if ( bExtCharAvailable )
        {
            rSh.Left( TRUE );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetAttr( aSet );
            SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               eCharSet );
            aSet.Put( aFont );
            rSh.SetAttr( aSet, SETATTR_DONTEXPAND );
            // zur Bearbeitung des Fussnotentextes
            rSh.ResetSelect(0, FALSE);
            rSh.GotoFtnTxt();
        }
        rSh.EndUndo( UIUNDO_INSERT_FOOTNOTE );
    }

    bFootnote = aFtnBtn.IsChecked();
}


/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwInsFootNoteDlg, NumberCharHdl, Button *, EMPTYARG )
{
    aNumberCharEdit.GrabFocus();
    aOkBtn.Enable( aNumberCharEdit.GetText().Len() || bExtCharAvailable );
    return 0;
}
IMPL_LINK_INLINE_END( SwInsFootNoteDlg, NumberCharHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwInsFootNoteDlg, NumberEditHdl, void *, EMPTYARG )
{
    aNumberCharBtn.Check( TRUE );
    aOkBtn.Enable( 0 != aNumberCharEdit.GetText().Len() );

    return 0;
}
IMPL_LINK_INLINE_END( SwInsFootNoteDlg, NumberEditHdl, void *, EMPTYARG )

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwInsFootNoteDlg, NumberAutoBtnHdl, Button *, EMPTYARG )
{
    aOkBtn.Enable( TRUE );
    return 0;
}
IMPL_LINK_INLINE_END( SwInsFootNoteDlg, NumberAutoBtnHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwInsFootNoteDlg, NumberExtCharHdl, Button *, EMPTYARG )
{
    aNumberCharBtn.Check( TRUE );

    SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
    rSh.GetAttr( aSet );
    const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

    SvxCharacterMap* pDlg = new SvxCharacterMap(this, FALSE);

    Font aDlgFont( pDlg->GetCharFont() );
    aDlgFont.SetName( rFont.GetFamilyName() );
    aDlgFont.SetCharSet( rFont.GetCharSet() );

    pDlg->SetCharFont( aDlgFont );
    if (RET_OK == pDlg->Execute())
    {
        String sExtChars(pDlg->GetCharacters());

        aFontName = pDlg->GetCharFont().GetName();
        eCharSet  = pDlg->GetCharFont().GetCharSet();
        aNumberCharEdit.SetText( sExtChars );
        aNumberCharEdit.SetFont( pDlg->GetCharFont() );
        bExtCharAvailable = TRUE;
        aOkBtn.Enable(0 != aNumberCharEdit.GetText().Len());
    }
    delete pDlg;

    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwInsFootNoteDlg, NextPrevHdl, Button *, pBtn )
{
    Apply();

    // Hier zur naechsten Fuss/Endnote wandern
    rSh.ResetSelect(0, FALSE);
    if (pBtn == &aNextBT)
        rSh.GotoNextFtnAnchor();
    else
        rSh.GotoPrevFtnAnchor();

    Init();

    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwInsFootNoteDlg::SwInsFootNoteDlg(Window *pParent, SwWrtShell &rShell, BOOL bEd) :

    SvxStandardDialog(pParent,SW_RES(DLG_INS_FOOTNOTE)),

    rSh(rShell),
    bEdit(bEd),
    bExtCharAvailable(FALSE),
    aNumberAutoBtn  (this,SW_RES(RB_NUMBER_AUTO)),
    aNumberCharBtn  (this,SW_RES(RB_NUMBER_CHAR)),
    aNumberCharEdit (this,SW_RES(ED_NUMBER_CHAR)),
    aNumberExtChar  (this,SW_RES(BT_NUMBER_CHAR)),
    aNumberFrm      (this,SW_RES(GB_NUMBER)),
    aFtnBtn         (this,SW_RES(RB_TYPE_FTN)),
    aEndNoteBtn     (this,SW_RES(RB_TYPE_ENDNOTE)),
    aTypeBox        (this,SW_RES(GB_TYPE)),

    aOkBtn          (this,SW_RES(BT_OK)),
    aCancelBtn      (this,SW_RES(BT_CANCEL)),
    aHelpBtn        (this,SW_RES(BT_HELP)),
    aPrevBT         (this,SW_RES(BT_PREV)),
    aNextBT         (this,SW_RES(BT_NEXT))
{
    aNumberAutoBtn.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberAutoBtnHdl));
    aNumberExtChar.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberExtCharHdl));
    aNumberCharBtn.SetClickHdl(LINK(this,SwInsFootNoteDlg,NumberCharHdl));
    aNumberCharEdit.SetModifyHdl(LINK(this,SwInsFootNoteDlg,NumberEditHdl));
    aNumberCharEdit.SetMaxTextLen(10);
    aNumberCharEdit.Enable();

    aPrevBT.SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));
    aNextBT.SetClickHdl(LINK(this, SwInsFootNoteDlg, NextPrevHdl));

    FreeResource();
    rSh.SetCareWin(this);

    if (bEdit)
    {
        Init();

        aPrevBT.Show();
        aNextBT.Show();
    }
}


/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwInsFootNoteDlg::~SwInsFootNoteDlg()
{
    rSh.SetCareWin(0);

    if (bEdit)
        rSh.ResetSelect(0, FALSE);
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwInsFootNoteDlg::Init()
{
    SwFmtFtn aFtnNote;
    String sNumStr;
    Font aFont;
    bExtCharAvailable = FALSE;

    rSh.StartAction();

    if( rSh.GetCurFtn( &aFtnNote ))
    {
        if(aFtnNote.GetNumStr().Len())
        {
            sNumStr = aFtnNote.GetNumStr();

            rSh.Right(TRUE);
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetAttr( aSet );
            const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

            aFont = aNumberCharEdit.GetFont();
            aFontName = rFont.GetFamilyName();
            eCharSet = rFont.GetCharSet();
            aFont.SetName(aFontName);
            aFont.SetCharSet(eCharSet);
            bExtCharAvailable = TRUE;
            rSh.Left();
        }
        bFootnote = !aFtnNote.IsEndNote();
    }
    aNumberCharEdit.SetFont(aFont);

    BOOL bNumChar = sNumStr.Len() != 0;

    aNumberCharEdit.SetText(sNumStr);
    aNumberCharBtn.Check(bNumChar);
    aNumberAutoBtn.Check(!bNumChar);
    if (bNumChar)
        aNumberCharEdit.GrabFocus();

    if (bFootnote)
        aFtnBtn.Check();
    else
        aEndNoteBtn.Check();

    BOOL bNext = rSh.GotoNextFtnAnchor();

    if (bNext)
        rSh.GotoPrevFtnAnchor();

    BOOL bPrev = rSh.GotoPrevFtnAnchor();

    if (bPrev)
        rSh.GotoNextFtnAnchor();

    aPrevBT.Enable(bPrev);
    aNextBT.Enable(bNext);

    rSh.Right(TRUE);

    rSh.EndAction();
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.46  2000/09/18 16:05:57  willem.vandorp
    OpenOffice header added.

    Revision 1.45  2000/05/10 11:53:43  os
    Basic API removed

    Revision 1.44  2000/02/11 14:56:21  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.43  1999/12/22 19:50:26  jp
    Bug #71204#: SetCurFtn - return change state

    Revision 1.42  1998/04/02 15:17:46  OM
    Fussnoten: Traveling u.a.


      Rev 1.41   02 Apr 1998 17:17:46   OM
   Fussnoten: Traveling u.a.

      Rev 1.40   26 Feb 1998 19:01:10   MA
   #47622# richtig initialisieren

      Rev 1.39   09 Feb 1998 09:55:12   OM
   #46894# Letzten Mode merken

      Rev 1.38   12 Dec 1997 14:46:04   MA
   undo chgftn jetzt in der core

      Rev 1.37   12 Dec 1997 11:04:16   TJ
   include

      Rev 1.36   10 Dec 1997 13:08:56   MA
   weiteres fuer Fuss-/Endnoten

      Rev 1.35   24 Nov 1997 16:47:44   MA
   includes

      Rev 1.34   03 Nov 1997 13:22:42   MA
   precomp entfernt

      Rev 1.33   15 Aug 1997 12:19:34   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.32   07 Apr 1997 16:08:26   MH
   chg: header

      Rev 1.31   11 Nov 1996 11:05:44   MA
   ResMgr

      Rev 1.30   28 Aug 1996 14:12:10   OS
   includes

      Rev 1.29   17 Jul 1996 14:09:04   JP
   SS von GetCurFtn geaendert

      Rev 1.28   06 Feb 1996 15:21:08   JP
   Link Umstellung 305

      Rev 1.27   24 Nov 1995 16:58:44   OM
   PCH->PRECOMPILED

      Rev 1.26   08 Nov 1995 13:31:42   JP
   Umstellung zur 301: Change -> Set

      Rev 1.25   24 Oct 1995 18:19:22   MA
   chg: AutoCheck bei Edit

      Rev 1.24   23 Oct 1995 17:28:00   OS
   Insert/EditFootnote recordable

      Rev 1.23   12 Sep 1995 17:29:32   OM
   Helpbutton eingefuegt

      Rev 1.22   30 Aug 1995 14:00:46   MA
   fix: __EXPORT'iert

      Rev 1.21   24 Aug 1995 14:33:22   MA
   swstddlg -> svxstandarddialog

      Rev 1.20   21 Jun 1995 16:17:32   OS
   Max. Fussnotenlaenge auf 10 begrenzt

------------------------------------------------------------------------*/


