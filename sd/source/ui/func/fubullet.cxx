/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fubullet.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:46:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fubullet.hxx"

#ifndef _BINDING_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
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
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif

#ifdef IRIX
#ifndef _SBXCLASS_HXX
#include <basic/sbx.hxx>
#endif
#endif
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#include "app.hrc"

namespace sd {

const sal_Unicode CHAR_HARDBLANK    =   ((sal_Unicode)0x00A0);
const sal_Unicode CHAR_HARDHYPHEN   =   ((sal_Unicode)0x2011);
const sal_Unicode CHAR_SOFTHYPHEN   =   ((sal_Unicode)0x00AD);
const sal_Unicode CHAR_RLM          =   ((sal_Unicode)0x200F);
const sal_Unicode CHAR_LRM          =   ((sal_Unicode)0x200E);
const sal_Unicode CHAR_ZWSP         =   ((sal_Unicode)0x200B);
const sal_Unicode CHAR_ZWNBSP       =   ((sal_Unicode)0x2060);

TYPEINIT1( FuBullet, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuBullet::FuBullet (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* _pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, _pView, pDoc, rReq)
{
}

FunctionReference FuBullet::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuBullet( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuBullet::DoExecute( SfxRequest& rReq )
{
    if( rReq.GetSlot() == SID_BULLET )
        InsertSpecialCharacter();
    else
    {
        sal_Unicode cMark = 0;
        switch( rReq.GetSlot() )
        {
            case FN_INSERT_SOFT_HYPHEN: cMark = CHAR_SOFTHYPHEN ; break;
            case FN_INSERT_HARDHYPHEN:  cMark = CHAR_HARDHYPHEN ; break;
            case FN_INSERT_HARD_SPACE:  cMark = CHAR_HARDBLANK ; break;
            case SID_INSERT_RLM : cMark = CHAR_RLM ; break;
            case SID_INSERT_LRM : cMark = CHAR_LRM ; break;
            case SID_INSERT_ZWSP : cMark = CHAR_ZWSP ; break;
            case SID_INSERT_ZWNBSP: cMark = CHAR_ZWNBSP; break;
        }

        DBG_ASSERT( cMark != 0, "FuBullet::FuBullet(), illegal slot used!" );

        if( cMark )
            InsertFormattingMark( cMark );
    }

}

void FuBullet::InsertFormattingMark( sal_Unicode cMark )
{
    OutlinerView* pOV = NULL;
    ::Outliner*   pOL = NULL;

    // depending on ViewShell set Outliner and OutlinerView
    if (mpViewShell->ISA(DrawViewShell))
    {
        pOV = mpView->GetTextEditOutlinerView();
        if (pOV)
            pOL = mpView->GetTextEditOutliner();
    }
    else if (mpViewShell->ISA(OutlineViewShell))
    {
        pOL = static_cast<OutlineView*>(mpView)->GetOutliner();
        pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
            mpViewShell->GetActiveWindow());
    }

    // insert string
    if(pOV && pOL)
    {
        // prevent flickering
        pOV->HideCursor();
        pOL->SetUpdateMode(FALSE);

        // remove old selected text
        pOV->InsertText( aEmptyStr );

        // prepare undo
        SfxUndoManager& rUndoMgr =  pOL->GetUndoManager();
        rUndoMgr.EnterListAction(String(SdResId(STR_UNDO_INSERT_SPECCHAR)),
                                    aEmptyStr );

        // insert given text
        String aStr( cMark );
        pOV->InsertText( cMark, TRUE);

        ESelection aSel = pOV->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOV->SetSelection(aSel);

        rUndoMgr.LeaveListAction();

        // restart repainting
        pOL->SetUpdateMode(TRUE);
        pOV->ShowCursor();
    }
}

void FuBullet::InsertSpecialCharacter()
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    AbstractSvxCharacterMap* pDlg = pFact ? pFact->CreateSvxCharacterMap( NULL,  RID_SVXDLG_CHARMAP, FALSE ) : 0;

    if( !pDlg )
        return;

    SfxItemSet aFontAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aFontAttr );
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
        if(mpViewShell && mpViewShell->ISA(DrawViewShell))
        {
            pOV = mpView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = mpView->GetTextEditOutliner();
            }
        }
        else if(mpViewShell && mpViewShell->ISA(OutlineViewShell))
        {
            pOL = static_cast<OutlineView*>(mpView)->GetOutliner();
            pOV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                mpViewShell->GetActiveWindow());
        }

        // Sonderzeichen einfuegen
        if (pOV)
        {
            // nicht flackern
            pOV->HideCursor();
            pOL->SetUpdateMode(FALSE);

            // alte Attributierung merken;
            // dazu vorher selektierten Bereich loeschen, denn der muss eh weg
            // und so gibt es immer eine eindeutige Attributierung (und da es
            // kein DeleteSelected() an der OutlinerView gibt, wird durch
            // Einfuegen eines Leerstrings geloescht)
            pOV->InsertText( aEmptyStr );

            SfxItemSet aOldSet( mpDoc->GetPool(), EE_CHAR_FONTINFO, EE_CHAR_FONTINFO, 0 );
            aOldSet.Put( pOV->GetAttribs() );

            SfxUndoManager& rUndoMgr =  pOL->GetUndoManager();
            rUndoMgr.EnterListAction(String(SdResId(STR_UNDO_INSERT_SPECCHAR)),
                                     aEmptyStr );
            pOV->InsertText(aString, TRUE);

            // attributieren (Font setzen)
            SfxItemSet aSet(pOL->GetEmptyItemSet());
            SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                   aFont.GetStyleName(), aFont.GetPitch(),
                                   aFont.GetCharSet(),
                                   EE_CHAR_FONTINFO);
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

void FuBullet::GetSlotState( SfxItemSet& rSet, ViewShell* pViewShell, SfxViewFrame* pViewFrame )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BULLET ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_SOFT_HYPHEN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_HARDHYPHEN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( FN_INSERT_HARD_SPACE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_RLM ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_LRM ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_ZWNBSP ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_INSERT_ZWSP ))
    {
        ::sd::View* pView = pViewShell ? pViewShell->GetView() : 0;
        OutlinerView* pOLV = pView ? pView->GetTextEditOutlinerView() : 0;

        const bool bTextEdit = pOLV;

        SvtCTLOptions aCTLOptions;
        const sal_Bool bCtlEnabled = aCTLOptions.IsCTLFontEnabled();

        if(!bTextEdit )
        {
            rSet.DisableItem(FN_INSERT_SOFT_HYPHEN);
            rSet.DisableItem(FN_INSERT_HARDHYPHEN);
            rSet.DisableItem(FN_INSERT_HARD_SPACE);
        }

        if( !bTextEdit && (dynamic_cast<OutlineViewShell*>( pViewShell ) == 0) )
            rSet.DisableItem(SID_BULLET);

        if(!bTextEdit || !bCtlEnabled )
        {
            rSet.DisableItem(SID_INSERT_RLM);
            rSet.DisableItem(SID_INSERT_LRM);
            rSet.DisableItem(SID_INSERT_ZWNBSP);
            rSet.DisableItem(SID_INSERT_ZWSP);
        }

        if( pViewFrame )
        {
            SfxBindings& rBindings = pViewFrame->GetBindings();

            rBindings.SetVisibleState( SID_INSERT_RLM, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_LRM, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_ZWNBSP, bCtlEnabled );
            rBindings.SetVisibleState( SID_INSERT_ZWSP, bCtlEnabled );
        }
    }
}
} // end of namespace sd
