/*************************************************************************
 *
 *  $RCSfile: hhcwrp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:50:25 $
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

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>         // fuer Undo-Ids
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif


#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_RUBYADJUST_HPP_
#include <com/sun/star/text/RubyAdjust.hpp>
#endif

#ifndef _HHCWRP_HXX
#include <hhcwrp.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // Progress
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _INDEX_HXX
#include <index.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif

#ifndef _OLMENU_HRC
#include <olmenu.hrc>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define CHAR_PAR_BRK    ((sal_Char) 0x0D)

//////////////////////////////////////////////////////////////////////
//     Beschreibung: Ggf. Rahmen/Objektshell abschalten

static void lcl_ActivateTextShell( SwWrtShell & rWrtSh )
{
    if( rWrtSh.IsSelFrmMode() || rWrtSh.IsObjSelected() )
        rWrtSh.EnterStdMode();
}

//////////////////////////////////////////////////////////////////////

SwHHCWrapper::SwHHCWrapper(
        SwView* pSwView,
        const uno::Reference< lang::XMultiServiceFactory >& rxMSF,
        const lang::Locale& rLocale,
        sal_Bool bStart, sal_Bool bOther, sal_Bool bSelection ) :
    svx::HangulHanjaConversion( &pSwView->GetEditWin(), rxMSF, rLocale ),
    rWrtShell( pSwView->GetWrtShell() )
{
    nLastPos        = 0;
    nUnitOffset     = 0;
    pView           = pSwView;
    pWin            = &pSwView->GetEditWin();
    bIsStart        = bStart;
    bIsOtherCntnt   = bStartChk     = bOther;
    bIsConvSpecial  = sal_True;
    bIsSelection    = bSelection;
    bInfoBox        = sal_False;
    bReverse        = sal_False;
    bStartDone = bOther || ( !bReverse && bStart );
    bEndDone   = bReverse && bStart && !bOther;
    nPageCount = nPageStart = 0;
    bIsCrsrSelection    = sal_False;
}


SwHHCWrapper::~SwHHCWrapper()
{
    rWrtShell.SetCareWin( NULL );
    if( nPageCount )
        ::EndProgress( pView->GetDocShell() );
/*
    if( bInfoBox )
        InfoBox(&pView->GetEditWin(), String(SW_RES(STR_SPELL_OK)) ).Execute();
*/
}


void SwHHCWrapper::GetNextPortion( OUString& /* [out] */ rNextPortion )
{
    if (bIsCrsrSelection)
    {
        rNextPortion = aConvText;
        aConvText = OUString();  // make iteraton stop next time
        //bIsCrsrSelection = sal_False;
    }
    else
    {
        FindConvText_impl();
        rNextPortion = aConvText;
    }

    nUnitOffset  = 0;

    // build last pos from currently selected text
    SwPaM* pCrsr = rWrtShell.GetCrsr();
    nLastPos =  pCrsr->Start()->nContent.GetIndex();
}


void SwHHCWrapper::SelectNewUnit_impl( sal_Int32 nUnitStart, sal_Int32 nUnitEnd )
{
    SwPaM *pCrsr = rWrtShell.GetCrsr();
    //DBG_ASSERT( pLastPos, "last pos not set" );
    //if (pLastPos)
    {
        pCrsr->GetPoint()->nContent = nLastPos;
        pCrsr->DeleteMark();
    }
    rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_False,
                  (USHORT) (nUnitOffset + nUnitStart), sal_True );
    pCrsr->SetMark();
    rWrtShell.Right( CRSR_SKIP_CHARS, /*bExpand*/ sal_True,
                  (USHORT) (nUnitEnd - nUnitStart), sal_True );
}


void SwHHCWrapper::HandleNewUnit(
        const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd )
{
    DBG_ASSERT( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(nUnitStart >= 0 && nUnitEnd >= nUnitStart))
        return;

    lcl_ActivateTextShell( rWrtShell );

    rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    rWrtShell.EndAllAction();
}


void SwHHCWrapper::ReplaceUnit(
         const sal_Int32 nUnitStart, const sal_Int32 nUnitEnd,
         const OUString& rReplaceWith,
         ReplacementAction eAction )
{
    static OUString aBracketedStart( C2U( "(" ) );
    static OUString aBracketedEnd( C2U( ")" ) );

    DBG_ASSERT( nUnitStart >= 0 && nUnitEnd >= nUnitStart, "wrong arguments" );
    if (!(nUnitStart >= 0 && nUnitEnd >= nUnitStart))
        return;

    lcl_ActivateTextShell( rWrtShell );

    // Das aktuelle Wort austauschen
    rWrtShell.StartAllAction();

    // select current unit
    SelectNewUnit_impl( nUnitStart, nUnitEnd );

    OUString aOrigTxt( rWrtShell.GetSelTxt() );
    OUString aNewTxt( rReplaceWith );
    SwFmtRuby *pRuby = 0;
    sal_Bool bRubyBelow = sal_False;
    switch (eAction)
    {
        case eExchange :
        break;
        case eReplacementBracketed :
        {
            (((aNewTxt = aOrigTxt) += aBracketedStart) += rReplaceWith) += aBracketedEnd;
        }
        break;
        case eOriginalBracketed :
        {
            (((aNewTxt = rReplaceWith) += aBracketedStart) += aOrigTxt) += aBracketedEnd;
        }
        break;
        case eReplacementAbove  :
        {
            pRuby = new SwFmtRuby( rReplaceWith );
            pRuby->SetText( aOrigTxt );
        }
        break;
        case eOriginalAbove :
        {
            pRuby = new SwFmtRuby( aOrigTxt );
            pRuby->SetText( rReplaceWith );
        }
        break;
        case eReplacementBelow :
        {
            pRuby = new SwFmtRuby( rReplaceWith );
            pRuby->SetText( aOrigTxt );
            bRubyBelow = sal_True;
        }
        break;
        case eOriginalBelow :
        {
            pRuby = new SwFmtRuby( aOrigTxt );
            pRuby->SetText( rReplaceWith );
            bRubyBelow = sal_True;
        }
        break;
        default:
            DBG_ERROR( "unexpected case" );
    }
    nUnitOffset += nUnitStart + aNewTxt.getLength();

    if (pRuby)
    {
        rWrtShell.StartUndo( UNDO_SETRUBYATTR );
        pRuby->SetPosition( bRubyBelow );
        pRuby->SetAdjustment( RubyAdjust_CENTER );
        //!! the following seem not to be needed
        //pRuby->SetCharFmtName( const String& rNm );
        //pRuby->SetCharFmtId( USHORT nNew );
        SwPaM *pPaM = rWrtShell.GetCrsr();
        rWrtShell.SetAttr(*pRuby);
        delete pRuby;
        rWrtShell.EndUndo( UNDO_SETRUBYATTR );
    }
    else
    {
        rWrtShell.StartUndo( UNDO_OVERWRITE );
        rWrtShell.Delete();
        rWrtShell.Insert( aNewTxt );
        rWrtShell.EndUndo( UNDO_OVERWRITE );
    }

    rWrtShell.EndAllAction();
}


void SwHHCWrapper::Convert()
{
    if (rWrtShell.HasSelection())
    {
        //!! workaround for selected text (works in one paragraph only)
        String aTxt;
        rWrtShell.GetSelectedText( aTxt, GETSELTXT_PARABRK_KEEP );
        xub_StrLen nPos = aTxt.Search( CHAR_PAR_BRK );
        if (STRING_NOTFOUND != nPos)
            aTxt.Erase( nPos );
        aConvText = aTxt;
        bIsCrsrSelection = sal_True;;
    }

    if ( bIsOtherCntnt )
        ConvStart_impl( SVX_SPELL_OTHER );
    else
    {
        bStartChk = bReverse;
        ConvStart_impl( bReverse ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
    }

    ConvertDocument();

    ConvEnd_impl();
}


sal_Bool SwHHCWrapper::ConvNext_impl( )
{
    //! modified version of SvxSpellWrapper::SpellNext

    sal_Bool bActRev = sal_False;

    // bActRev ist die Richtung nach dem Spellen, bReverse die am Anfang.
    if( bActRev == bReverse )
    {                           // Keine Richtungsaenderung, also ist
        if( bStartChk )         // der gewuenschte Bereich ( bStartChk )
            bStartDone = sal_True;  // vollstaendig abgearbeitet.
        else
            bEndDone = sal_True;
    }
    else if( bReverse == bStartChk ) // Bei einer Richtungsaenderung kann
    {                          // u.U. auch ein Bereich abgearbeitet sein.
        if( bStartChk )        // Sollte der vordere Teil rueckwaerts gespellt
            bEndDone = sal_True;   // werden und wir kehren unterwegs um, so ist
        else                   // der hintere Teil abgearbeitet (und umgekehrt).
            bStartDone = sal_True;
    }

    bReverse = bActRev;
    if( bIsOtherCntnt && bStartDone && bEndDone ) // Dokument komplett geprueft?
    {
        bInfoBox = sal_True;
        return sal_False;
    }

    //ResMgr* pMgr = DIALOG_MGR();
    sal_Bool bGoOn = sal_False;

    if ( bIsOtherCntnt )
    {
        bStartChk = sal_False;
        ConvStart_impl( SVX_SPELL_BODY );
        bGoOn = sal_True;
    }
    else if ( bStartDone && bEndDone )
    {
        // Bodybereich erledigt, Frage nach Sonderbereich
        if( bIsConvSpecial && HasOtherCnt_impl() )
        {
            ConvStart_impl( SVX_SPELL_OTHER );
            bIsOtherCntnt = bGoOn = sal_True;
        }
        else
            bInfoBox = sal_True;
    }
    else
    {
        // Ein BODY_Bereich erledigt, Frage nach dem anderen BODY_Bereich
/*
        //pWin->LeaveWait();

        sal_uInt16 nResId = bReverse ? RID_SVXQB_BW_CONTINUE : RID_SVXQB_CONTINUE;
        QueryBox aBox( pWin, ResId( nResId, pMgr ) );
        if ( aBox.Execute() != RET_YES )
        {
            // Verzicht auf den anderen Bereich, ggf. Frage nach Sonderbereich
            //pWin->EnterWait();
            bStartDone = bEndDone = sal_True;
            return SpellNext();
        }
        else
        {
*/
            bStartChk = !bStartDone;
            ConvStart_impl( bStartChk ? SVX_SPELL_BODY_START : SVX_SPELL_BODY_END );
            bGoOn = sal_True;
/*
        }
        pWin->EnterWait();
*/
    }
    return bGoOn;
}


sal_Bool SwHHCWrapper::FindConvText_impl()
{
    //! modified version of SvxSpellWrapper::FindSpellError

    //ShowLanguageErrors();

    sal_Bool bFound = sal_False;

    pWin->EnterWait();
    sal_Bool bSpell = sal_True;

    while ( bSpell )
    {
        bFound = ConvContinue_impl();
        if (bFound)
        {
            bSpell = sal_False;
        }
        else
        {
            ConvEnd_impl();
            bSpell = ConvNext_impl();
        }
    }
    pWin->LeaveWait();
    return bFound;
}


sal_Bool SwHHCWrapper::HasOtherCnt_impl()
{
    return bIsSelection ? sal_False : rWrtShell.HasOtherCnt();
}


void SwHHCWrapper::ConvStart_impl( SvxSpellArea eSpell )
{
    //SetDrawObj( SVX_SPELL_OTHER == eSpell );
    pView->SpellStart( eSpell, bStartDone, bEndDone, TRUE );
}


void SwHHCWrapper::ConvEnd_impl()
{
    pView->SpellEnd( TRUE );
    //ShowLanguageErrors();
}


sal_Bool SwHHCWrapper::ConvContinue_impl()
{
    sal_Bool bProgress = /* !bDrawObj && */ !bIsSelection;
    aConvText = OUString();
    uno::Any  aRet = bProgress ?
        pView->GetWrtShell().SpellContinue( &nPageCount, &nPageStart, TRUE ) :
        pView->GetWrtShell().SpellContinue( &nPageCount, NULL, TRUE );
    aRet >>= aConvText;
    return aConvText.getLength() != 0;
}

//////////////////////////////////////////////////////////////////////

