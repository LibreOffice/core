/*************************************************************************
 *
 *  $RCSfile: viewling.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:05:50 $
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
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>      // fuer ITEMID_SPELLCHECK (muss _vor_ sfxdlg.hxx!)
#endif


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif


#ifndef _SVX_THESDLG_HXX //autogen
#include <svx/thesdlg.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX //autogen
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>               // fuer SpellPointer
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>                // CheckSpellChanges
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>              // Viewoptions
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>               // fuer Undo-Ids
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>              // STR_MULT_INTERACT_SPELL_WARN
#endif
#ifndef _SPLWRP_HXX
#include <splwrp.hxx>               //    "
#endif
#ifndef _HYP_HXX
#include <hyp.hxx>                  // Trennung
#endif
#ifndef _OLMENU_HXX
#include <olmenu.hxx>               // PopupMenu fuer OnlineSpelling
#endif
#ifndef _PAM_HXX
#include <pam.hxx>                  // Spelling: Multiselektion
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#ifndef _VIEW_HRC
#include <view.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
    Beschreibung:   Lingu-Dispatcher
 --------------------------------------------------------------------*/


void SwView::ExecLingu(SfxRequest &rReq)
{
    switch(rReq.GetSlot())
    {
        case FN_THESAURUS_DLG:
            StartThesaurus();
            break;
        case FN_SPELLING_DLG:
            SpellDocument( NULL, sal_False );
            break;
        case FN_ADD_UNKNOWN:
            {
                SpellDocument( NULL, sal_True );
            }
            break;
        case FN_HYPHENATE_OPT_DLG:
            HyphenateDocument();
            break;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}

/*--------------------------------------------------------------------
     Beschreibung: SpellCheck starten
 --------------------------------------------------------------------*/


void SwView::SpellDocument( const String* pStr, sal_Bool bAllRight )
{
    // do not spell if interactive spelling is active elsewhere
    if (GetWrtShell().HasSpellIter())
    {
        MessBox( 0, WB_OK, String( SW_RES( STR_SPELL_TITLE ) ),
                String( SW_RES( STR_MULT_INTERACT_SPELL_WARN ) ) ).Execute();
        return;
    }

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_SPELLING, aEmptyStr, pEditWin,
         RID_SVXERRCTX, DIALOG_MGR() );

    Reference< XSpellChecker1 >  xSpell = ::GetSpellChecker();
    if(!xSpell.is())
    {   // keine Arme keine Kekse
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_LINGUNOTEXISTS );
        return;
    }
    SpellKontext(sal_True);

    SwViewOption* pVOpt = (SwViewOption*)pWrtShell->GetViewOptions();
    sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

    sal_Bool bOldIns = pWrtShell->IsInsMode();
    pWrtShell->SetInsMode( sal_True );

    // den eigentlichen Inhalt pruefen
    _SpellDocument( pStr, bAllRight );

    pWrtShell->SetInsMode( bOldIns );

    SpellKontext(sal_False);

    // Ignorieren und ersetzen nur fuer einen Durchgang
    // im Zuge des OnlineSpellings wollen wir die IgnoreList beibehalten
    if (SvxGetChangeAllList().is())
        SvxGetChangeAllList()->clear();
    SvxSaveDictionaries( SvxGetDictionaryList() );

    pVOpt->SetIdle( bOldIdle );
    //SW_MOD()->CheckSpellChanges( sal_False, );

    // SpellCache loeschen, Speicher freigeben.
    // wg. Absturz in W95 verzichten wir auf die Freigabe...
//  pSpell->FlushAllLanguages();
}

/*--------------------------------------------------------------------
    Beschreibung:   Interne SpellFunktion
 --------------------------------------------------------------------*/


void SwView::_SpellDocument( const String* pStr, sal_Bool bAllRight )
{
    sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection() ||
        pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext();

    Reference< XSpellChecker1 >  xSpell = ::GetSpellChecker();
    Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bIsWrapReverse  = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_WRAP_REVERSE) ).getValue() : sal_False;
    sal_Bool bIsSpellSpecial = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_SPECIAL) ).getValue() : sal_True;

    sal_Bool    bStart = bSelection || ( bIsWrapReverse ?
                        pWrtShell->IsEndOfDoc() : pWrtShell->IsStartOfDoc() );
    sal_Bool    bOther = !bSelection && !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);

    if( bOther && !bIsSpellSpecial )
    // kein Sonderbereich eingeschaltet
    {
        // Ich will auch in Sonderbereichen trennen
        QueryBox aBox( &GetEditWin(), SW_RES( DLG_SPECIAL_FORCED ) );
        if( aBox.Execute() == RET_YES  &&  xProp.is())
        {
            sal_Bool bTrue = sal_True;
            Any aTmp(&bTrue, ::getBooleanCppuType());
            xProp->setPropertyValue( C2U(UPN_IS_SPELL_SPECIAL), aTmp );
        }
        else
            return; // Nein Es wird nicht gespellt
    }
    if( bAllRight )
    {
// nothing to be
//      pSpell->SetAllRight( bAllRight );
//      pSpell->NewDic( pStr ? *pStr : (const String&) aEmptyStr, LANGUAGE_NOLANGUAGE, sal_False );
    }
    {
        SvxDicListChgClamp aClamp( SvxGetDictionaryList() );
        SwSpellWrapper aWrap( this, xSpell, bStart, bAllRight,
                              bOther, bSelection );
        aWrap.SpellDocument();
    }
//  pSpell->SetAllRight( sal_False );

}


void SwView::SpellStart( SvxSpellArea eWhich, sal_Bool bStartDone, sal_Bool bEndDone )
{
    Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bIsWrapReverse = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_WRAP_REVERSE) ).getValue() : sal_False;

    SwDocPositions eStart = DOCPOS_START;
    SwDocPositions eEnde  = DOCPOS_END;
    SwDocPositions eCurr  = DOCPOS_CURR;
    switch ( eWhich )
    {
        case SVX_SPELL_BODY:
            if( bIsWrapReverse )
                eCurr = DOCPOS_END;
            else
                eCurr = DOCPOS_START;
            break;
        case SVX_SPELL_BODY_END:
            if( bIsWrapReverse )
            {
                if( bStartDone )
                    eStart = DOCPOS_CURR;
                eCurr = DOCPOS_END;
            }
            else if( bStartDone )
                eCurr = DOCPOS_START;
            break;
        case SVX_SPELL_BODY_START:
            if( !bIsWrapReverse )
            {
                if( bEndDone )
                    eEnde = DOCPOS_CURR;
                eCurr = DOCPOS_START;
            }
            else if( bEndDone )
                eCurr = DOCPOS_END;
            break;
        case SVX_SPELL_OTHER:
            if( bIsWrapReverse )
            {
                eStart = DOCPOS_OTHERSTART;
                eEnde  = DOCPOS_OTHEREND;
                eCurr = DOCPOS_OTHEREND;
            }
            else
            {
                eStart = DOCPOS_OTHERSTART;
                eEnde  = DOCPOS_OTHEREND;
                eCurr = DOCPOS_OTHERSTART;
            }
            break;
        default:
            ASSERT( !this, "SpellStart with unknown Area" );
    }
    pWrtShell->SpellStart( eStart, eEnde, eCurr );
}

/*--------------------------------------------------------------------
    Beschreibung: Fehlermeldung beim Spelling
 --------------------------------------------------------------------*/


// Der uebergebene Pointer nLang ist selbst der Wert
IMPL_LINK( SwView, SpellError, void *, nLang )
{
#ifdef DEBUG
    sal_Bool bFocus = GetEditWin().HasFocus();
#endif
    sal_uInt16 nPend = 0;

    if ( pWrtShell->ActionPend() )
    {
        pWrtShell->Push();
        pWrtShell->ClearMark();
        do
        {
            pWrtShell->EndAction();
            ++nPend;
        }
        while( pWrtShell->ActionPend() );
    }
    LanguageType eLang = (LanguageType)(sal_uInt32)nLang;
    String aErr(::GetLanguageString( eLang ) );

    SwEditWin &rEditWin = GetEditWin();
#ifdef DEBUG
    bFocus = rEditWin.HasFocus();
#endif
    sal_uInt16 nWaitCnt = 0;
    while( rEditWin.IsWait() )
    {
        rEditWin.LeaveWait();
        ++nWaitCnt;
    }
    if ( LANGUAGE_NONE == eLang )
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_NOLANGUAGE );
    else
        ErrorHandler::HandleError( *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );

    while( nWaitCnt )
    {
        rEditWin.EnterWait();
        --nWaitCnt;
    }
#ifdef DEBUG
    bFocus = GetEditWin().HasFocus();
#endif

    if ( nPend )
    {
        while( nPend-- )
            pWrtShell->StartAction();
        pWrtShell->Combine();
    }
#ifdef DEBUG
    if( !bFocus )
        GetEditWin().GrabFocus();
#endif

    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Spelling beenden und Cursor wiederherstellen
 --------------------------------------------------------------------*/


void SwView::SpellEnd()
{
    pWrtShell->SpellEnd();
    if( pWrtShell->IsExtMode() )
        pWrtShell->SetMark();
}


void SwView::HyphStart( SvxSpellArea eWhich )
{
    switch ( eWhich )
    {
        case SVX_SPELL_BODY:
            pWrtShell->HyphStart( DOCPOS_START, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_END:
            pWrtShell->HyphStart( DOCPOS_CURR, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_START:
            pWrtShell->HyphStart( DOCPOS_START, DOCPOS_CURR );
            break;
        case SVX_SPELL_OTHER:
            pWrtShell->HyphStart( DOCPOS_OTHERSTART, DOCPOS_OTHEREND );
            break;
        default:
            ASSERT( !this, "HyphStart with unknown Area" );
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Interaktive Trennung
 --------------------------------------------------------------------*/


void SwView::HyphenateDocument()
{
    // do not hyphenate if interactive hyphenationg is active elsewhere
    if (GetWrtShell().HasHyphIter())
    {
        MessBox( 0, WB_OK, String( SW_RES( STR_HYPH_TITLE ) ),
                String( SW_RES( STR_MULT_INTERACT_HYPH_WARN ) ) ).Execute();
        return;
    }

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_HYPHENATION, aEmptyStr, pEditWin,
         RID_SVXERRCTX, DIALOG_MGR() );

    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    if (!xHyph.is())
    {
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_LINGUNOTEXISTS );
        return;
    }

    if (pWrtShell->GetSelectionType() & (SwWrtShell::SEL_DRW_TXT|SwWrtShell::SEL_DRW))
    {
        // Silbentrennung in einem Draw-Objekt
        HyphenateDrawText();
    }
    else
    {
        SwViewOption* pVOpt = (SwViewOption*)pWrtShell->GetViewOptions();
        sal_Bool bOldIdle = pVOpt->IsIdle();
        pVOpt->SetIdle( sal_False );

        Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );


        pWrtShell->StartUndo(UNDO_INSATTR);         // spaeter gueltig

        sal_Bool bHyphSpecial = xProp.is() ?
                *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_SPECIAL) ).getValue() : sal_False;
        sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection() ||
            pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext();
        sal_Bool bOther = pWrtShell->HasOtherCnt() && bHyphSpecial && !bSelection;
        sal_Bool bStart = bSelection || ( !bOther && pWrtShell->IsStartOfDoc() );
        sal_Bool bStop = sal_False;
        if( !bOther && !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY) && !bSelection )
        // kein Sonderbereich eingeschaltet
        {
            // Ich will auch in Sonderbereichen trennen
            QueryBox aBox( &GetEditWin(), SW_RES( DLG_SPECIAL_FORCED ) );
            if( aBox.Execute() == RET_YES )
            {
                bOther = sal_True;
                if (xProp.is())
                {
                    sal_Bool bTrue = sal_True;
                    Any aTmp(&bTrue, ::getBooleanCppuType());
                    xProp->setPropertyValue( C2U(UPN_IS_HYPH_SPECIAL), aTmp );
                }
            }
            else
                bStop = sal_True; // Nein Es wird nicht getrennt
        }

        if( !bStop )
        {
            SwHyphWrapper aWrap( this, xHyph, bStart, bOther, bSelection );
            aWrap.SpellDocument();
            pWrtShell->EndUndo(UNDO_INSATTR);
        }
        pVOpt->SetIdle( bOldIdle );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Thesaurus starten
 --------------------------------------------------------------------*/


void SwView::StartThesaurus()
{
    if( pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext() )
        return;
    sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection();
    if( bSelection && !pWrtShell->IsSelOnePara() )
        return;

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_THESAURUS, aEmptyStr, pEditWin,
         RID_SVXERRCTX, DIALOG_MGR() );

    // Sprache rausholen
    //
    SfxItemSet aSet(pShell->GetPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE);
    pWrtShell->GetAttr(aSet);

    const SvxLanguageItem& rLang = (const SvxLanguageItem&)aSet.Get(
                                            RES_CHRATR_LANGUAGE );
    LanguageType eLang = rLang.GetLanguage();
    if ( ( eLang == LANGUAGE_SYSTEM ) &&
         ( ((eLang=SvxLocaleToLanguage( GetAppLocaleData().getLocale() ))==LANGUAGE_SYSTEM )
         && ( ( eLang=::GetSystemLanguage() ) == LANGUAGE_SYSTEM ) ) )
        eLang = LANGUAGE_DONTKNOW;

    if ( eLang == LANGUAGE_DONTKNOW || eLang == LANGUAGE_NONE )
    {
        SpellError( (void *) LANGUAGE_NONE );
        return;
    }

    SwViewOption* pVOpt = (SwViewOption*)pWrtShell->GetViewOptions();
    sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

#ifdef TL_NEVER
//!!! hier muß noch was getan werden... (Umsetzung der Funktionalitaet)
    // ErrorLink setzen, alten merken
    Link aOldLnk = pSpell->ChgErrorLink(LINK(this, SwView, SpellError));
#endif


    // get initial LookUp text
    String aTmp = bSelection ?
        pWrtShell->GetSelTxt() : pWrtShell->GetCurWord();

    Reference< XThesaurus >  xThes( ::GetThesaurus() );
    SvxThesaurusDialog *pDlg = NULL;

    if ( !xThes.is() || !xThes->hasLocale( SvxCreateLocale( eLang ) ) )
    {
        SpellError( (void *) eLang );
    }
    else
    {
        // create dialog
        {   //Scope for SwWait-Object
            SwWait aWait( *GetDocShell(), sal_True );
            pDlg = new SvxThesaurusDialog( &GetEditWin(),
                                           xThes, aTmp, eLang );
        }

        {
            // Hier wird der Thesaurus-Dialog im Applikationsfenster zentriert,
            // und zwar oberhalb oder unterhalb der Cursorposition, je nachdem,
            // wo mehr Platz ist.

            // Current Word:
            SwRect aRect( pWrtShell->GetCharRect() );
            Point aTopPos = aRect.Pos();
            Point aBtmPos( aTopPos.X(), aRect.Bottom() );
            aTopPos = GetEditWin().LogicToPixel( aTopPos );
            aTopPos = GetEditWin().OutputToScreenPixel( aTopPos );
            aBtmPos = GetEditWin().LogicToPixel( aBtmPos );
            aBtmPos = GetEditWin().OutputToScreenPixel( aBtmPos );
            // ::frame::Desktop:
            Rectangle aRct = GetEditWin().GetDesktopRectPixel();
            Point aWinTop( aRct.TopLeft() );
            Point aWinBtm( aRct.BottomRight() );
            if ( aTopPos.Y() - aWinTop.Y() > aWinBtm.Y() - aBtmPos.Y() )
                aWinBtm.Y() = aTopPos.Y();
            else
                aWinTop.Y() = aBtmPos.Y();

            Size aSz = pDlg->GetSizePixel();
            if ( aWinBtm.Y() - aWinTop.Y() > aSz.Height() )
            {
                aWinTop.X() = ( aWinTop.X() + aWinBtm.X() - aSz.Width() ) / 2;
                aWinTop.Y() = ( aWinTop.Y() + aWinBtm.Y() - aSz.Height() ) / 2;
                pDlg->SetPosPixel( aWinTop );
            }
        }

        if ( pDlg->Execute()== RET_OK )
        {
            sal_Bool bOldIns = pWrtShell->IsInsMode();
            pWrtShell->SetInsMode( sal_True );

            pWrtShell->StartAllAction();
            pWrtShell->StartUndo(UNDO_DELETE);

            if( !bSelection )
            {
                if(!pWrtShell->IsInWrd() && !pWrtShell->IsEndWrd() &&
                    !pWrtShell->IsSttWrd())
                    pWrtShell->PrvWrd();

                if(pWrtShell->IsEndWrd())
                    pWrtShell->Left();

                pWrtShell->SelWrd();
            }
#ifdef TL_NEVER
            String aTmp(pThes->GetPreStripped());
            aTmp += pDlg->GetWord();
            aTmp += pThes->GetPostStripped();

            pWrtShell->Insert(aTmp);
#endif
            pWrtShell->Insert( pDlg->GetWord() );

            pWrtShell->EndUndo(UNDO_DELETE);
            pWrtShell->EndAllAction();

            pWrtShell->SetInsMode( bOldIns );

        }
    }

    delete pDlg;

#ifdef TL_NEVER
    pSpell->ChgErrorLink( aOldLnk );
#endif
    pVOpt->SetIdle( bOldIdle );

}

/*--------------------------------------------------------------------
    Beschreibung:   Online-Vorschlaege anbieten
 *--------------------------------------------------------------------*/


sal_Bool SwView::ExecSpellPopup(const Point& rPt)
{
    sal_Bool bRet = sal_False;
    const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
    if( pVOpt->IsOnlineSpell() &&
        !pVOpt->IsHideSpell() &&
        !pWrtShell->IsSelection())
    {
        if (pWrtShell->GetSelectionType() & SwWrtShell::SEL_DRW_TXT)
            bRet = ExecDrwTxtSpellPopup(rPt);
        else if (!pWrtShell->IsSelFrmMode())
        {
            const sal_Bool bOldViewLock = pWrtShell->IsViewLocked();
            pWrtShell->LockView( sal_True );
            pWrtShell->Push();
            Reference< XSpellAlternatives >  xAlt( pWrtShell->GetCorrection(&rPt) );
            if ( xAlt.is() )
            {
                bRet = sal_True;
                pWrtShell->SttSelect();
                SwSpellPopup aPopup( pWrtShell, xAlt );
                aPopup.Execute(
                pEditWin,
                rPt);
            }

            pWrtShell->Pop( sal_False );
            pWrtShell->LockView( bOldViewLock );
        }
    }
    return bRet;
}

