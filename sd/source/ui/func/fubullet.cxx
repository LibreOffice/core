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
#include "precompiled_sd.hxx"


#include "fubullet.hxx"

#ifndef _BINDING_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#include <editeng/eeitem.hxx>
#include <svl/poolitem.hxx>
#include <editeng/fontitem.hxx>
#include "OutlineViewShell.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include <svx/svdoutl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <svl/ctloptions.hxx>
#include <svl/itempool.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "drawview.hxx"

#include "app.hrc"

namespace sd {

const sal_Unicode CHAR_HARDBLANK    =   ((sal_Unicode)0x00A0);
const sal_Unicode CHAR_HARDHYPHEN   =   ((sal_Unicode)0x2011);
const sal_Unicode CHAR_SOFTHYPHEN   =   ((sal_Unicode)0x00AD);
const sal_Unicode CHAR_RLM          =   ((sal_Unicode)0x200F);
const sal_Unicode CHAR_LRM          =   ((sal_Unicode)0x200E);
const sal_Unicode CHAR_ZWSP         =   ((sal_Unicode)0x200B);
const sal_Unicode CHAR_ZWNBSP       =   ((sal_Unicode)0x2060);

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
    if( rReq.GetSlot() == SID_CHARMAP )
        InsertSpecialCharacter(rReq);
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
    if (dynamic_cast< DrawViewShell* >(mpViewShell))
    {
        pOV = mpView->GetTextEditOutlinerView();
        if (pOV)
            pOL = mpView->GetTextEditOutliner();
    }
    else if (dynamic_cast< OutlineViewShell* >(mpViewShell))
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
        pOL->SetUpdateMode(false);

        // remove old selected text
        pOV->InsertText( aEmptyStr );

        // prepare undo
        ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
        rUndoMgr.EnterListAction(String(SdResId(STR_UNDO_INSERT_SPECCHAR)),
                                    aEmptyStr );

        // insert given text
        String aStr( cMark );
        pOV->InsertText( cMark, true);

        ESelection aSel = pOV->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOV->SetSelection(aSel);

        rUndoMgr.LeaveListAction();

        // restart repainting
        pOL->SetUpdateMode(true);
        pOV->ShowCursor();
    }
}

void FuBullet::InsertSpecialCharacter( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if( pArgs )
        pArgs->GetItemState(mpDoc->GetItemPool().GetWhich(SID_CHARMAP), false, &pItem);

    String aChars, aFontName;
    Font aFont;
    if ( pItem )
    {
        aChars = ((const SfxStringItem*)pItem)->GetValue();
        const SfxPoolItem* pFtItem = NULL;
        pArgs->GetItemState( mpDoc->GetItemPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
        const SfxStringItem* pFontItem = dynamic_cast< const SfxStringItem* >(pFtItem );
        if ( pFontItem )
        {
            aFontName = pFontItem->GetValue();
            aFont = Font( aFontName, Size(1,1) );
        }
        else
        {
            SfxItemSet aFontAttr( mpDoc->GetItemPool() );
            mpView->GetAttributes( aFontAttr );
            const SvxFontItem* pFItem = (const SvxFontItem*)aFontAttr.GetItem( SID_ATTR_CHAR_FONT );
            if( pFItem )
                aFont = Font( pFItem->GetFamilyName(), pFItem->GetStyleName(), Size( 1, 1 ) );
        }
    }

    if (!aChars.Len() )
    {
        SfxAllItemSet aSet( mpDoc->GetItemPool() );
        aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SfxItemSet aFontAttr( mpDoc->GetItemPool() );
        mpView->GetAttributes( aFontAttr );
        const SvxFontItem* pFontItem = (const SvxFontItem*)aFontAttr.GetItem( SID_ATTR_CHAR_FONT );
        if( pFontItem )
            aSet.Put( *pFontItem );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractDialog* pDlg = pFact ? pFact->CreateSfxDialog( &mpView->GetViewShell()->GetViewFrame()->GetWindow(), aSet,
            mpView->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface(),
            RID_SVXDLG_CHARMAP ) : 0;
        if( !pDlg )
            return;

        // Wenn Zeichen selektiert ist kann es angezeigt werden
        // pDLg->SetFont( );
        // pDlg->SetChar( );
        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pCItem, SfxStringItem, SID_CHARMAP );
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFItem, SvxFontItem, SID_ATTR_CHAR_FONT );
            if ( pFItem )
            {
                aFont.SetName( pFItem->GetFamilyName() );
                aFont.SetStyleName( pFItem->GetStyleName() );
                aFont.SetCharSet( pFItem->GetCharSet() );
                aFont.SetPitch( pFItem->GetPitch() );
            }

            if ( pCItem )
                aChars  = pCItem->GetValue();
        }

        delete( pDlg );
    }

    if( aChars.Len() )
    {
        OutlinerView* pOV = NULL;
        ::Outliner*   pOL = NULL;

        // je nach ViewShell Outliner und OutlinerView bestimmen
        if(mpViewShell && dynamic_cast< DrawViewShell* >(mpViewShell))
        {
            pOV = mpView->GetTextEditOutlinerView();
            if (pOV)
            {
                pOL = mpView->GetTextEditOutliner();
            }
        }
        else if(mpViewShell && dynamic_cast< OutlineViewShell* >(mpViewShell))
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
            pOL->SetUpdateMode(false);

            // alte Attributierung merken;
            // dazu vorher selektierten Bereich loeschen, denn der muss eh weg
            // und so gibt es immer eine eindeutige Attributierung (und da es
            // kein DeleteSelected() an der OutlinerView gibt, wird durch
            // Einfuegen eines Leerstrings geloescht)
            pOV->InsertText( aEmptyStr );

            SfxItemSet aOldSet( mpDoc->GetItemPool(), EE_CHAR_FONTINFO, EE_CHAR_FONTINFO, 0 );
            aOldSet.Put( pOV->GetAttribs() );

            ::svl::IUndoManager& rUndoMgr =  pOL->GetUndoManager();
            rUndoMgr.EnterListAction(String(SdResId(STR_UNDO_INSERT_SPECCHAR)),
                                     aEmptyStr );
            pOV->InsertText(aChars, true);

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
            pOL->SetUpdateMode(true);
            pOV->ShowCursor();
        }
    }
}

void FuBullet::GetSlotState( SfxItemSet& rSet, ViewShell* pViewShell, SfxViewFrame* pViewFrame )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHARMAP ) ||
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
            rSet.DisableItem(SID_CHARMAP);

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
