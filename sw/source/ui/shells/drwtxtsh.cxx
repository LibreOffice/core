/*************************************************************************
 *
 *  $RCSfile: drwtxtsh.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-09 12:01:33 $
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

#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX
#include <svx/charmap.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVX_TEXTANIM_HXX //autogen
#include <svx/textanim.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <xdef.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HDL_
#include <com/sun/star/i18n/TransliterationModules.hdl>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>               // fuer SpellPointer
#endif
#ifndef _DRWTXTSH_HXX
#include <drwtxtsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#define SwDrawTextShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif

SFX_IMPL_INTERFACE(SwDrawTextShell, SfxShell, SW_RES(STR_SHELLNAME_DRAW_TEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRWTXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TEXT_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_DRWTXT));
    SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawTextShell,SfxShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::Init()
{
    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();
    pOutliner = pSdrView->GetTextEditOutliner();
    pOLV = pSdrView->GetTextEditOutlinerView();
    ULONG nCtrl = pOutliner->GetControlWord();
    nCtrl |= EE_CNTRL_AUTOCORRECT;

    SetUndoManager(&pOutliner->GetUndoManager());

    // jetzt versuchen wir mal ein AutoSpell

    const SwViewOption* pVOpt = rSh.GetViewOptions();
    if(pVOpt->IsOnlineSpell())
    {
        nCtrl |= EE_CNTRL_ONLINESPELLING|EE_CNTRL_ALLOWBIGOBJS;
        if(pVOpt->IsHideSpell())
            nCtrl |= EE_CNTRL_NOREDLINES;
    }
    else
        nCtrl &= ~(EE_CNTRL_ONLINESPELLING|EE_CNTRL_NOREDLINES);

    pOutliner->SetControlWord(nCtrl);
    pOLV->ShowCursor();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwDrawTextShell::SwDrawTextShell(SwView &rV) :
    SfxShell(&rV),
    rView(rV)
{
    SwWrtShell &rSh = GetShell();
    SetPool(rSh.GetAttrPool().GetSecondaryPool());

    Init();

    rSh.NoEdit(TRUE);
    SetName(String::CreateFromAscii("ObjectText"));
    SetHelpId(SW_DRWTXTSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



__EXPORT SwDrawTextShell::~SwDrawTextShell()
{
    if ( GetView().GetCurShell() == this )
        rView.ResetSubShell();

    //MA 13. Nov. 96: Das kommt durchaus vor #33141#:
    //(doppel-)Klick von einem Texteditmode in ein anderes Objekt, zwischendurch
    //wird eine andere (Draw-)Shell gepusht, die alte aber noch nicht deletet.
    //Dann wird vor dem Flush wieder ein DrawTextShell gepusht und der Mode ist
    //eingeschaltet. In diesem Moment wird der Dispatcher geflusht und die alte
    //DrawTextShell zerstoert.
//  ASSERT( !pSdrView->IsTextEdit(), "TextEdit in DTor DrwTxtSh?" );
//    if (pSdrView->IsTextEdit())
//      GetShell().EndTextEdit();   // Danebengeklickt, Ende mit Edit

//    GetShell().Edit();
}

SwWrtShell& SwDrawTextShell::GetShell()
{
    return rView.GetWrtShell();
}


/*--------------------------------------------------------------------
    Beschreibung:   Slots mit dieser Statusmethode disablen
 --------------------------------------------------------------------*/

void SwDrawTextShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/



void SwDrawTextShell::SetAttrToMarked(const SfxItemSet& rAttr)
{
    Rectangle aNullRect;
    Rectangle aOutRect = pOLV->GetOutputArea();

    if (aNullRect != aOutRect)
    {
        GetShell().GetDrawView()->SetAttributes(rAttr);
//      Init();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



BOOL SwDrawTextShell::IsTextEdit()
{
    if (!pOutliner || !pOLV)
        Init();

    if (!pOutliner || !pOLV)
        return (FALSE);

    return (pSdrView->IsTextEdit());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecFontWork(SfxRequest& rReq)
{
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( rReq.GetArgs() )
    {
        pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                ((const SfxBoolItem&) (rReq.GetArgs()->
                                Get(SID_FONTWORK))).GetValue());
    }
    else
        pVFrame->ToggleChildWindow(SvxFontWorkChildWindow::GetChildWindowId());

    pVFrame->GetBindings().Invalidate(SID_FONTWORK);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateFontWork(SfxItemSet& rSet)
{
    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();
    rSet.Put(SfxBoolItem(SID_FONTWORK, GetView().GetViewFrame()->HasChildWindow(nId)));
}

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/



void SwDrawTextShell::ExecFormText(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();

    const SdrMarkList& rMarkList = pDrView->GetMarkList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit( TRUE );
            GetView().AttrChangedNotify(&rSh);
        }

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)(GetView().GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());
        }
        else
            pDrView->SetAttributes(rSet);
    }

}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/



void SwDrawTextShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( pVFrame->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
#define XATTR_ANZ 12
        static const USHORT nXAttr[ XATTR_ANZ ] =
        {   XATTR_FORMTXTSTYLE, XATTR_FORMTXTADJUST, XATTR_FORMTXTDISTANCE,
            XATTR_FORMTXTSTART, XATTR_FORMTXTMIRROR, XATTR_FORMTXTSTDFORM,
            XATTR_FORMTXTHIDEFORM, XATTR_FORMTXTOUTLINE, XATTR_FORMTXTSHADOW,
            XATTR_FORMTXTSHDWCOLOR, XATTR_FORMTXTSHDWXVAL, XATTR_FORMTXTSHDWYVAL
        };
        for( USHORT i = 0; i < XATTR_ANZ; )
            rSet.DisableItem( nXAttr[ i++ ] );
    }
    else
    {
        if ( pDlg )
            pDlg->SetColorTable(OFF_APP()->GetStdColorTable());

        pDrView->GetAttributes( rSet );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecDrawLingu(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    if( rSh.GetDrawView()->GetMarkList().GetMarkCount() )
    {
        LanguageType nLang = ((const SvxLanguageItem&)rSh.GetDefault(
                                   GetWhichOfScript( RES_CHRATR_LANGUAGE,
                                GetScriptTypeOfLanguage( LANGUAGE_SYSTEM )) )
                                ).GetLanguage();
        switch(rReq.GetSlot())
        {
        case FN_THESAURUS_DLG:
            pOLV->StartThesaurus( nLang );
            break;

        case FN_SPELLING_DLG:
            pOLV->StartSpeller( nLang );
            break;

        default:
            ASSERT(!this, "unexpected slot-id");
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecDraw(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();


    switch (rReq.GetSlot())
    {
        case FN_INSERT_SYMBOL:  // Sonderzeichen einfuegen
            InsertSymbol();
            break;

        case SID_SELECTALL:
        {
            ULONG nParaCount = pOutliner->GetParagraphCount();
            if (nParaCount > 0)
                pOLV->SelectRange(0L, USHORT(nParaCount) );
        }
        break;

        case FN_FORMAT_RESET:   // Harte Textattributierung l”schen
        {
            SfxItemSet aEmptyAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);
            pSdrView->SetAttributes(aEmptyAttr, TRUE);
        }
        break;

        case FN_ESCAPE:
            if (pSdrView->IsTextEdit())
            {
                // Shellwechsel!
                rSh.EndTextEdit();
                rSh.GetView().AttrChangedNotify(&rSh);
                return;
            }
            break;

        case FN_DRAWTEXT_ATTR_DLG:
            {
                SfxItemSet aNewAttr( pSdrView->GetModel()->GetItemPool() );
                pSdrView->GetAttributes( aNewAttr );
                SvxTextTabDialog* pDlg = new SvxTextTabDialog(
                                &(GetView().GetViewFrame()->GetWindow()),
                                &aNewAttr, pSdrView );
                USHORT nResult = pDlg->Execute();

                if (nResult == RET_OK)
                {
                    if (pSdrView->HasMarkedObj())
                        pSdrView->SetAttributes(*pDlg->GetOutputItemSet());
                }
                delete( pDlg );
            }
            break;

        default:
            ASSERT(!this, "unexpected slot-id");
            return;
    }

    GetView().GetViewFrame()->GetBindings().InvalidateAll(FALSE);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();
}

/*--------------------------------------------------------------------
    Beschreibung:   Undo ausfuehren
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecUndo(SfxRequest &rReq)
{
    if ( !IsTextEdit() )
        return;

    SfxViewFrame *pSfxViewFrame = GetView().GetViewFrame();
    pSfxViewFrame->ExecuteSlot(rReq, pSfxViewFrame->GetInterface());
}

/*--------------------------------------------------------------------
    Beschreibung:   Zustand Undo
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateUndo(SfxItemSet &rSet)
{
    if ( !IsTextEdit() )
        return;

    SfxViewFrame *pSfxViewFrame = GetView().GetViewFrame();
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        pSfxViewFrame->GetSlotState(nWhich, pSfxViewFrame->GetInterface(), &rSet );
        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::ExecTransliteration( SfxRequest & rReq )
{
    using namespace ::com::sun::star::i18n;
    {
        sal_uInt32 nMode = 0;

        switch( rReq.GetSlot() )
        {
        case SID_TRANSLITERATE_UPPER:
            nMode = TransliterationModules_LOWERCASE_UPPERCASE;
            break;
        case SID_TRANSLITERATE_LOWER:
            nMode = TransliterationModules_UPPERCASE_LOWERCASE;
            break;

        case SID_TRANSLITERATE_HALFWIDTH:
            nMode = TransliterationModules_FULLWIDTH_HALFWIDTH;
            break;
        case SID_TRANSLITERATE_FULLWIDTH:
            nMode = TransliterationModules_HALFWIDTH_FULLWIDTH;
            break;

        case SID_TRANSLITERATE_HIRAGANA:
            nMode = TransliterationModules_KATAKANA_HIRAGANA;
            break;
        case SID_TRANSLITERATE_KATAGANA:
            nMode = TransliterationModules_HIRAGANA_KATAKANA;
            break;

        default:
            ASSERT(!this, "falscher Dispatcher");
        }

        if( nMode )
            pOLV->TransliterateText( nMode );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Sonderzeichen einfuegen (siehe SDraw: FUBULLET.CXX)
 --------------------------------------------------------------------*/

void SwDrawTextShell::InsertSymbol()
{
    SvxCharacterMap* pDlg = new SvxCharacterMap( NULL, FALSE );

    // Wenn Zeichen selektiert ist kann es angezeigt werden
    // pDLg->SetFont( );
    // pDlg->SetChar( );
    USHORT nResult = pDlg->Execute();

    //char c;
    String s;

    Font aFont;

    if( nResult == RET_OK )
    {
        aFont = pDlg->GetCharFont();
        s     = pDlg->GetCharacters();
    }
    delete( pDlg );

    if( nResult == RET_OK )
    {
        // Sonderzeichen einfuegen
        if (pOLV)
        {
            // nicht flackern
            pOLV->HideCursor();
            pOutliner->SetUpdateMode(FALSE);

            SfxItemSet aOldSet(pOLV->GetAttribs());
            SvxFontItem aOldFontItem((const SvxFontItem&)aOldSet.Get(EE_CHAR_FONTINFO));

            // String einfuegen
            pOLV->InsertText(s, TRUE);

            // attributieren (Font setzen)
            SfxItemSet aSet(pOutliner->GetEmptyItemSet());
            SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                   aFont.GetStyleName(), aFont.GetPitch(),
                                   aFont.GetCharSet(),
                                   EE_CHAR_FONTINFO);
            aSet.Put(aFontItem);
            pOLV->SetAttribs(aSet);

            // Selektion loeschen
            ESelection aSel(pOLV->GetSelection());
            aSel.nStartPara = aSel.nEndPara;
            aSel.nStartPos = aSel.nEndPos;
            pOLV->SetSelection(aSel);

            // Alten Font restaurieren
            aSet.Put(aOldFontItem);
            pOLV->SetAttribs(aSet);

            // ab jetzt wieder anzeigen
            pOutliner->SetUpdateMode(TRUE);
            pOLV->ShowCursor();
        }
    }
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.2  2001/02/21 17:38:28  jp
      use new function GetWhichOfScript/GetScriptTypeOfLanguage

      Revision 1.1.1.1  2000/09/18 17:14:46  hr
      initial import

      Revision 1.113  2000/09/18 16:06:03  willem.vandorp
      OpenOffice header added.

      Revision 1.112  2000/09/08 08:12:51  os
      Change: Set/Toggle/Has/Knows/Show/GetChildWindow

      Revision 1.111  2000/09/07 15:59:29  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.110  2000/05/26 07:21:32  os
      old SW Basic API Slots removed

      Revision 1.109  2000/05/10 11:53:01  os
      Basic API removed

      Revision 1.108  2000/04/18 14:58:23  os
      UNICODE

      Revision 1.107  1999/08/04 09:09:52  JP
      have to change: Outliner -> SdrOutlines


      Rev 1.106   04 Aug 1999 11:09:52   JP
   have to change: Outliner -> SdrOutlines

      Rev 1.105   21 Jun 1999 15:20:24   JP
   Interface changes: SdrView::GetAttributes

      Rev 1.104   10 Jun 1999 13:16:36   JP
   have to change: no AppWin from SfxApp

      Rev 1.103   02 Feb 1999 13:17:36   JP
   Bug #61335#: Schnistelle zu SwView geaendert/aufgeraeumt

      Rev 1.102   12 Oct 1998 10:13:20   OM
   #57790# Dialog nicht mehr AppModal

      Rev 1.101   06 May 1998 16:20:42   OS
   AttrChangedNotify nach Shellwechsel nicht ueber this aufrufen

      Rev 1.100   29 Nov 1997 15:52:14   MA
   includes

      Rev 1.99   24 Nov 1997 09:47:00   MA
   includes

      Rev 1.98   03 Nov 1997 13:55:46   MA
   precomp entfernt

      Rev 1.97   30 Sep 1997 16:30:46   TJ
   include

      Rev 1.96   01 Sep 1997 13:23:56   OS
   DLL-Umstellung

      Rev 1.95   11 Aug 1997 08:34:16   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.94   06 Aug 1997 14:19:50   TRI
   VCL: GetpApp() statt pApp

      Rev 1.93   05 Aug 1997 13:29:54   MH
   chg: header

      Rev 1.92   25 Jun 1997 09:10:18   MA
   #40965#, SubShell abmelden

      Rev 1.91   19 Jun 1997 16:39:16   MA
   fix: reste vom DrawText-Umbau

      Rev 1.90   19 Jun 1997 16:05:42   HJS
   includes

      Rev 1.89   17 Jun 1997 15:54:12   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.88   04 Jun 1997 11:30:26   TRI
   svwin.h nur unter WIN oder WNT includen

      Rev 1.87   04 Jun 1997 09:54:48   NF
   Includes...

      Rev 1.86   02 May 1997 20:15:40   NF
   includes...

      Rev 1.85   08 Apr 1997 10:52:22   OM
   Fehlende Includes

      Rev 1.84   02 Apr 1997 14:07:00   JP
   Bug #38398#: StateUndo - Itemset weitergeben

      Rev 1.83   19 Mar 1997 12:48:20   AMA
   Fix #37035: Speller am Outliner setzen.

      Rev 1.82   24 Feb 1997 16:53:08   OM
   Silbentrennung in Draw-Objekten

      Rev 1.81   16 Dec 1996 10:41:50   OM
   Cursor nach Initialisierung anzeigen

      Rev 1.80   11 Dec 1996 10:50:58   MA
   Warnings

      Rev 1.79   26 Nov 1996 16:53:06   OS
   Erste Schritte fuer OnlineSpelling in  DrawTextObjekten

      Rev 1.78   19 Nov 1996 16:02:30   NF
   defines...

      Rev 1.77   18 Nov 1996 16:05:50   OS
   Set/GetControlWord an der OutlinerView benutzen

      Rev 1.76   18 Nov 1996 15:46:40   OS
   AutoKorrektur auch im DrawText

      Rev 1.75   15 Nov 1996 13:34:02   OS
   SvxTextTabDialog angebunden

      Rev 1.74   13 Nov 1996 20:29:36   MA
   #33141# nicht den Mode verlassen

      Rev 1.73   07 Nov 1996 16:22:52   OM
   Font nach Einfuegen:Sonderzeichen restaurieren

      Rev 1.72   07 Nov 1996 15:28:18   OM
   RefDevice am Outliner setzen

      Rev 1.71   06 Nov 1996 16:41:26   OS
   SID_SW_START statt RID_APP_START

      Rev 1.70   26 Sep 1996 15:41:54   OM
   Neues Parent fuer Sonderzeichen-Dlg

      Rev 1.69   28 Aug 1996 15:55:00   OS
   includes

      Rev 1.68   15 Aug 1996 13:44:22   JP
   svdraw.hxx entfernt

      Rev 1.67   31 May 1996 11:09:30   NF
   CLOOKS

      Rev 1.66   28 May 1996 11:56:16   TRI
   CLOOKS

      Rev 1.65   21 May 1996 08:58:00   NF
   CLOOKS

      Rev 1.64   22 Mar 1996 15:17:00   TRI
   sfxiiter.hxx included

      Rev 1.63   29 Feb 1996 21:14:40   HJS
   clooks

      Rev 1.62   19 Feb 1996 18:30:58   MA
   chg: EndTextEdit ueber die fesh

      Rev 1.61   16 Feb 1996 09:14:48   SWG
   CLOOK's

      Rev 1.60   07 Feb 1996 08:07:22   OS
   SvxTextAttrDialog hat jetzt endgueltig 3 Parameter

      Rev 1.59   01 Feb 1996 08:07:18   SWG
   TextAttrDialog wieder mal als nicht-Emergency-Version

*************************************************************************/



