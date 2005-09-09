/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fubullet.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:34:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#include "fubullet.hxx"

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#define ITEMID_FONT             EE_CHAR_FONTINFO
#include <svx/fontitem.hxx>

#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX //autogen
#include <svx/charmap.hxx>
#endif

#ifdef IRIX
#ifndef _SBXCLASS_HXX
#include <basic/sbx.hxx>
#endif
#endif
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

namespace sd {

TYPEINIT1( FuBullet, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuBullet::FuBullet (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{

    //CHINA001 SvxCharacterMap* pDlg = new SvxCharacterMap( NULL, FALSE );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    AbstractSvxCharacterMap* pDlg = pFact->CreateSvxCharacterMap( NULL,  ResId(RID_SVXDLG_CHARMAP), FALSE );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

    SfxItemSet aFontAttr( pDoc->GetPool() );
    pView->GetAttributes( aFontAttr );
    const SvxFontItem* pFontItem = (const SvxFontItem*)aFontAttr.GetItem( SID_ATTR_CHAR_FONT );
    if( pFontItem )
    {
        Font aCurrentFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), Size( 1, 1 ) );
        pDlg->SetFont( aCurrentFont );
    }

    // Wenn Zeichen selektiert ist kann es angezeigt werden
    // pDLg->SetFont( );
    // pDlg->SetChar( );
    USHORT nResult = pDlg->Execute();

    //char c;
    String aString;

    Font aFont;

    if( nResult == RET_OK )
    {
        aFont = pDlg->GetCharFont();
        aString = pDlg->GetCharacters();
    }
    delete( pDlg );

    if( nResult == RET_OK )
    {
        OutlinerView* pOV = NULL;
        ::Outliner*   pOL = NULL;

        // je nach ViewShell Outliner und OutlinerView bestimmen
        if (pViewSh->ISA(DrawViewShell))
        {
            pOV = pView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = pView->GetTextEditOutliner();
            }
        }
        else if (pViewSh->ISA(OutlineViewShell))
        {
            pOL = static_cast<OutlineView*>(pView)->GetOutliner();
            pOV = static_cast<OutlineView*>(pView)->GetViewByWindow(
                pViewShell->GetActiveWindow());
        }

        // Sonderzeichen einfuegen
        if (pOV)
        {
            String aEmptyStr;
            // nicht flackern
            pOV->HideCursor();
            pOL->SetUpdateMode(FALSE);

            // alte Attributierung merken;
            // dazu vorher selektierten Bereich loeschen, denn der muss eh weg
            // und so gibt es immer eine eindeutige Attributierung (und da es
            // kein DeleteSelected() an der OutlinerView gibt, wird durch
            // Einfuegen eines Leerstrings geloescht)
            pOV->InsertText( aEmptyStr );

            SfxItemSet aOldSet( pDoc->GetPool(), ITEMID_FONT, ITEMID_FONT, 0 );
            aOldSet.Put( pOV->GetAttribs() );

            SfxUndoManager& rUndoMgr =  pOL->GetUndoManager();
            rUndoMgr.EnterListAction(String(SdResId(STR_UNDO_INSERT_SPECCHAR)),
                                     aEmptyStr );
            pOV->InsertText(aString, TRUE);

            // attributieren (Font setzen)
            SfxItemSet aSet(pOL->GetEmptyItemSet());
            SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                   aFont.GetStyleName(), aFont.GetPitch(),
                                   aFont.GetCharSet());
            aSet.Put(aFontItem);
            aSet.Put(aFontItem, EE_CHAR_FONTINFO_CJK);
            aSet.Put(aFontItem, EE_CHAR_FONTINFO_CTL);
            pOV->SetAttribs(aSet);

            ESelection aSel = pOV->GetSelection();
            aSel.nStartPara = aSel.nEndPara;
            aSel.nStartPos = aSel.nEndPos;
            pOV->SetSelection(aSel);

            // nicht mit Sonderzeichenattributierung weiterschreiben
            pOV->GetOutliner()->QuickSetAttribs(aOldSet, aSel);

            rUndoMgr.LeaveListAction();

            // ab jetzt wieder anzeigen
            pOL->SetUpdateMode(TRUE);
            pOV->ShowCursor();
        }
    }
}

} // end of namespace sd
