/*************************************************************************
 *
 *  $RCSfile: fubullet.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#define ITEMID_FONT             EE_CHAR_FONTINFO
#include <svx/fontitem.hxx>

#include "outlnvsh.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "fubullet.hxx"
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
#include <svtools/sbx.hxx>
#endif
#endif

TYPEINIT1( FuBullet, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuBullet::FuBullet( SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{

    SvxCharacterMap* pDlg = new SvxCharacterMap( NULL, FALSE );

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
        Outliner*     pOL = NULL;

        // je nach ViewShell Outliner und OutlinerView bestimmen
        if (pViewSh->ISA(SdDrawViewShell))
        {
            pOV = pView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = pView->GetTextEditOutliner();
            }
        }
        else if (pViewSh->ISA(SdOutlineViewShell))
        {
            pOL = ((SdOutlineView*)pView)->GetOutliner();
            pOV = ((SdOutlineView*)pView)->GetViewByWindow(
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



