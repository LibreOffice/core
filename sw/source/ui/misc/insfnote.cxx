/*************************************************************************
 *
 *  $RCSfile: insfnote.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:36:30 $
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
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
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
        rSh.Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
        rSh.StartUndo( UNDO_START );
        SwFmtFtn aNote( aEndNoteBtn.IsChecked() );
        aNote.SetNumStr( aStr );

        if( rSh.SetCurFtn( aNote ) && bExtCharAvailable )
        {
            rSh.Right(CRSR_SKIP_CHARS, TRUE, 1, FALSE );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetAttr( aSet );
            SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
            SvxFontItem aFont( rFont.GetFamily(), aFontName,
                               rFont.GetStyleName(), rFont.GetPitch(),
                               eCharSet );
            aSet.Put( aFont );
            rSh.SetAttr( aSet, SETATTR_DONTEXPAND );
            rSh.ResetSelect(0, FALSE);
            rSh.Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
        }
        rSh.EndUndo( UNDO_END );
        rSh.EndAction();
    }
    else
    {
/*
        rSh.StartUndo( UIUNDO_INSERT_FOOTNOTE );
        rSh.InsertFootnote( aStr, aEndNoteBtn.IsChecked(), !bExtCharAvailable );

        if ( bExtCharAvailable )
        {
            rSh.Left( CRSR_SKIP_CHARS, TRUE, 1, FALSE );
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
*/
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
    aNumberFL      (this,SW_RES(FL_NUMBER)),
    aFtnBtn         (this,SW_RES(RB_TYPE_FTN)),
    aEndNoteBtn     (this,SW_RES(RB_TYPE_ENDNOTE)),
    aTypeFL        (this,SW_RES(FL_TYPE)),

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

            rSh.Right(CRSR_SKIP_CHARS, TRUE, 1, FALSE );
            SfxItemSet aSet( rSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
            rSh.GetAttr( aSet );
            const SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );

            aFont = aNumberCharEdit.GetFont();
            aFontName = rFont.GetFamilyName();
            eCharSet = rFont.GetCharSet();
            aFont.SetName(aFontName);
            aFont.SetCharSet(eCharSet);
            bExtCharAvailable = TRUE;
            rSh.Left( CRSR_SKIP_CHARS, FALSE, 1, FALSE );
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

    rSh.Right(CRSR_SKIP_CHARS, TRUE, 1, FALSE );

    rSh.EndAction();
}

