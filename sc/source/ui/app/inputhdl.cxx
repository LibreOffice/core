/*************************************************************************
 *
 *  $RCSfile: inputhdl.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:35:19 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/acorrcfg.hxx>
#include <svx/algitem.hxx>
#include <svx/adjitem.hxx>
#include <svx/brshitem.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/colritem.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/editview.hxx>
#include <svx/escpitem.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/langitem.hxx>
#include <svx/svxacorr.hxx>
#include <svx/unolingu.hxx>
#include <svx/wghtitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/sound.hxx>
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

#ifndef _SV_CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif


#include "inputwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
//#include "appmain.hxx"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "patattr.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "editutil.hxx"
#include "collect.hxx"
#include "appoptio.hxx"
#include "docoptio.hxx"
#include "validat.hxx"
#include "userlist.hxx"
#include "rfindlst.hxx"
#include "inputopt.hxx"
#include "cell.hxx"             // fuer Formel-Preview
#include "compiler.hxx"         // fuer Formel-Preview
#include "editable.hxx"

#define _INPUTHDL_CXX
#include "inputhdl.hxx"

//  max. Ranges im RangeFinder
#define RANGEFIND_MAX   32

// STATIC DATA -----------------------------------------------------------

BOOL ScInputHandler::bOptLoaded = FALSE;            // App-Optionen ausgewertet
BOOL ScInputHandler::bAutoComplete = FALSE;         // wird in KeyInput gesetzt

//  delimiters (in addition to ScEditUtil) needed for range finder:
//  only characters that are allowed in formulas next to references
//  and the quotation mark (so string constants can be skipped)

static const sal_Char __FAR_DATA pMinDelimiters[] = " !\"";

extern USHORT nEditAdjust;      //! Member an ViewData

//==================================================================

void ScInputHandler::InitRangeFinder( const String& rFormula )
{
    DeleteRangeFinder();

    if ( !pActiveViewSh || !SC_MOD()->GetInputOptions().GetRangeFinder() )
        return;
    ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

//  String aDelimiters = pEngine->GetWordDelimiters();
    String aDelimiters = ScEditUtil::ModifyDelimiters(
                            String::CreateFromAscii( pMinDelimiters ) );

    xub_StrLen nColon = aDelimiters.Search(':');
    if ( nColon != STRING_NOTFOUND )
        aDelimiters.Erase( nColon, 1 );             // Delimiter ohne Doppelpunkt
    xub_StrLen nDot = aDelimiters.Search('.');
    if ( nDot != STRING_NOTFOUND )
        aDelimiters.Erase( nDot, 1 );               // Delimiter ohne Punkt

    const sal_Unicode* pChar = rFormula.GetBuffer();
    xub_StrLen nLen = rFormula.Len();
    xub_StrLen nPos = 0;
    xub_StrLen nStart = 0;
    USHORT nCount = 0;
    ScRange aRange;
    while ( nPos < nLen && nCount < RANGEFIND_MAX )
    {
        //  Trenner ueberlesen
        while ( nPos<nLen && ScGlobal::UnicodeStrChr( aDelimiters.GetBuffer(), pChar[nPos] ) )
        {
            if ( pChar[nPos] == '"' )                       // String
            {
                ++nPos;
                while (nPos<nLen && pChar[nPos] != '"')     // bis zum Ende ueberlesen
                    ++nPos;
            }
            ++nPos;                     // Trennzeichen oder schliessender Quote
        }

        //  Text zwischen Trennern
        nStart = nPos;
        while ( nPos<nLen && !ScGlobal::UnicodeStrChr( aDelimiters.GetBuffer(), pChar[nPos] ) )
            ++nPos;

        if ( nPos > nStart )
        {
            String aTest = rFormula.Copy( nStart, nPos-nStart );
            USHORT nFlags = aRange.ParseAny( aTest, pDoc );
            if ( nFlags & SCA_VALID )
            {
                //  Tabelle setzen, wenn nicht angegeben
                if ( (nFlags & SCA_TAB_3D) == 0 )
                    aRange.aStart.SetTab( pActiveViewSh->GetViewData()->GetTabNo() );
                if ( (nFlags & SCA_TAB2_3D) == 0 )
                    aRange.aEnd.SetTab( aRange.aStart.Tab() );

                if (!nCount)
                {
                    pEngine->SetUpdateMode( FALSE );
                    pRangeFindList = new ScRangeFindList( pDocSh->GetTitle() );
                }

                ScRangeFindData* pNew = new ScRangeFindData( aRange, nFlags, nStart, nPos );
                pRangeFindList->Insert( pNew );

                ESelection aSel( 0, nStart, 0, nPos );
                SfxItemSet aSet( pEngine->GetEmptyItemSet() );
                aSet.Put( SvxColorItem( Color( ScRangeFindList::GetColorName( nCount ) ),
                            EE_CHAR_COLOR ) );
                pEngine->QuickSetAttribs( aSet, aSel );
                ++nCount;
            }
        }

        //  letzten Trenner nicht ueberlesen, koennte ja ein Quote sein (?)
    }

    if (nCount)
    {
        pEngine->SetUpdateMode( TRUE );

        pDocSh->Broadcast( SfxSimpleHint( SC_HINT_SHOWRANGEFINDER ) );
    }
}

void lcl_Replace( EditView* pView, const String& rNewStr, const ESelection& rOldSel )
{
    if ( pView )
    {
        ESelection aOldSel = pView->GetSelection();
        if (aOldSel.HasRange())
            pView->SetSelection( ESelection( aOldSel.nEndPara, aOldSel.nEndPos,
                                             aOldSel.nEndPara, aOldSel.nEndPos ) );

        EditEngine* pEngine = pView->GetEditEngine();
        pEngine->QuickInsertText( rNewStr, rOldSel );

        //  Dummy-InsertText fuer Update und Paint
        //  dafuer muss oben die Selektion aufgehoben werden (vor QuickInsertText)
        pView->InsertText( EMPTY_STRING, FALSE );

        xub_StrLen nLen = pEngine->GetTextLen(0);
        ESelection aSel( 0, nLen, 0, nLen );
        pView->SetSelection( aSel );                // Cursor ans Ende
    }
}

void ScInputHandler::UpdateRange( USHORT nIndex, const ScRange& rNew )
{
    ScTabViewShell* pDocView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pDocView && pRangeFindList && nIndex < pRangeFindList->Count() )
    {
        ScRangeFindData* pData = pRangeFindList->GetObject( nIndex );
        xub_StrLen nOldStart = pData->nSelStart;
        xub_StrLen nOldEnd = pData->nSelEnd;

        ScRange aJustified = rNew;
        aJustified.Justify();           // Ref in der Formel immer richtigherum anzeigen
        String aNewStr;
        aJustified.Format( aNewStr, pData->nFlags, pDocView->GetViewData()->GetDocument() );
        ESelection aOldSel( 0, nOldStart, 0, nOldEnd );

        DataChanging();

        lcl_Replace( pTopView, aNewStr, aOldSel );
        lcl_Replace( pTableView, aNewStr, aOldSel );

        bInRangeUpdate = TRUE;
        DataChanged();
        bInRangeUpdate = FALSE;

        long nDiff = aNewStr.Len() - (long)(nOldEnd-nOldStart);

        pData->aRef = rNew;
        pData->nSelEnd = (xub_StrLen)(pData->nSelEnd + nDiff);

        USHORT nCount = (USHORT) pRangeFindList->Count();
        for (USHORT i=nIndex+1; i<nCount; i++)
        {
            ScRangeFindData* pNext = pRangeFindList->GetObject( i );
            pNext->nSelStart = (xub_StrLen)(pNext->nSelStart + nDiff);
            pNext->nSelEnd   = (xub_StrLen)(pNext->nSelEnd   + nDiff);
        }
    }
    else
        DBG_ERROR("UpdateRange: da fehlt was");
}

void ScInputHandler::DeleteRangeFinder()
{
    ScTabViewShell* pPaintView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pRangeFindList && pPaintView )
    {
        ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
        pRangeFindList->SetHidden(TRUE);
        pDocSh->Broadcast( SfxSimpleHint( SC_HINT_SHOWRANGEFINDER ) );  // wegnehmen
        DELETEZ(pRangeFindList);
    }
}

//==================================================================

inline String GetEditText(EditEngine* pEng)
{
    return ScEditUtil::GetSpaceDelimitedString(*pEng);
}

void lcl_RemoveTabs(String& rStr)
{
    xub_StrLen nPos;
    while ( (nPos=rStr.Search('\t')) != STRING_NOTFOUND )
        rStr.SetChar( nPos, ' ' );
}

void lcl_RemoveLineEnd(String& rStr)
{
    rStr.ConvertLineEnd(LINEEND_LF);
    xub_StrLen nPos;
    while ( (nPos=rStr.Search('\n')) != STRING_NOTFOUND )
        rStr.SetChar( nPos, ' ' );
}

//==================================================================

ScInputHandler::ScInputHandler()
    :   eMode( SC_INPUT_NONE ),
        bModified( FALSE ),
        bFormulaMode( FALSE ),
        bSelIsRef( FALSE ),
        bInRangeUpdate( FALSE ),
        pInputWin( NULL ),
        pRefViewSh( NULL ),
        pLastPattern( NULL ),
        pLastState( NULL ),
        bLastIsSymbol( FALSE ),
        bProtected( FALSE ),
        bTextValid( TRUE ),
        bCellHasPercentFormat( FALSE ),
        nValidation( 0 ),
        nAttrAdjust( SVX_HOR_JUSTIFY_STANDARD ),
        aScaleX( 1,1 ),
        aScaleY( 1,1 ),
        pTopView( NULL ),
        pTableView( NULL ),
        pEngine( NULL ),
        pEditDefaults( NULL ),
        pDelayTimer( NULL ),
        pColumnData( NULL ),
        pFormulaData( NULL ),
        nTipVisible( 0 ),
        nAutoPos( SCPOS_INVALID ),
        bUseTab( FALSE ),
        nFormSelStart( 0 ),
        nFormSelEnd( 0 ),
        nAutoPar( 0 ),
        pRangeFindList( NULL ),
        bParenthesisShown( FALSE ),
        bCreatingFuncView( FALSE ),
        bInEnterHandler( FALSE ),
        bCommandErrorShown( FALSE ),
        bInOwnChange( FALSE )
{
    //  The InputHandler is constructed with the view, so SfxViewShell::Current
    //  doesn't have the right view yet. pActiveViewSh is updated in NotifyChange.
    pActiveViewSh = NULL;

    //  Bindings (nur noch fuer Invalidate benutzt) werden bei Bedarf aktuell geholt
}

__EXPORT ScInputHandler::~ScInputHandler()
{
    //  Wenn dies der Applikations-InputHandler ist, wird der dtor erst nach SfxApplication::Main
    //  gerufen, darf sich also auf keine Sfx-Funktionen mehr verlassen

    if ( !SFX_APP()->IsDowning() )          // inplace
        EnterHandler();                     // Eingabe noch abschliessen

    if (SC_MOD()->GetRefInputHdl()==this)
        SC_MOD()->SetRefInputHdl(NULL);

    if ( pInputWin && pInputWin->GetInputHandler() == this )
        pInputWin->SetInputHandler( NULL );

    delete pRangeFindList;
    delete pEditDefaults;
    delete pEngine;
    delete pLastState;
    delete pDelayTimer;
    delete pColumnData;
    delete pFormulaData;
}

void ScInputHandler::SetRefScale( const Fraction& rX, const Fraction& rY )
{
    if ( rX != aScaleX || rY != aScaleY )
    {
        aScaleX = rX;
        aScaleY = rY;
        if (pEngine)
        {
            MapMode aMode( MAP_100TH_MM, Point(), aScaleX, aScaleY );
            pEngine->SetRefMapMode( aMode );
        }
    }
}

void ScInputHandler::UpdateRefDevice()
{
    if (!pEngine)
        return;

    BOOL bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    if ( bTextWysiwyg && pActiveViewSh )
        pEngine->SetRefDevice( pActiveViewSh->GetViewData()->GetDocument()->GetPrinter() );
    else
        pEngine->SetRefDevice( NULL );

    MapMode aMode( MAP_100TH_MM, Point(), aScaleX, aScaleY );
    pEngine->SetRefMapMode( aMode );

    //  SetRefDevice(NULL) uses VirtualDevice, SetRefMapMode forces creation of a local VDev,
    //  so the DigitLanguage can be safely modified (might use an own VDev instead of NULL).
    if ( !( bTextWysiwyg && pActiveViewSh ) )
    {
        pEngine->GetRefDevice()->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    }
}

void ScInputHandler::ImplCreateEditEngine()
{
    if ( !pEngine )
    {
        if ( pActiveViewSh )
        {
            const ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();
            pEngine = new ScFieldEditEngine( pDoc->GetEnginePool(), pDoc->GetEditPool() );
        }
        else
            pEngine = new ScFieldEditEngine( EditEngine::CreatePool(), NULL, TRUE );
        pEngine->SetWordDelimiters( ScEditUtil::ModifyDelimiters( pEngine->GetWordDelimiters() ) );
        UpdateRefDevice();      // also sets MapMode
        pEngine->SetPaperSize( Size( 1000000, 1000000 ) );
        pEditDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );

        pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_AUTOCORRECT );
        pEngine->SetModifyHdl( LINK( this, ScInputHandler, ModifyHdl ) );
    }
}

void ScInputHandler::UpdateAutoCorrFlag()
{
    ULONG nCntrl = pEngine->GetControlWord();
    ULONG nOld = nCntrl;

    //  don't use pLastPattern here (may be invalid because of AutoStyle)

    BOOL bDisable = bLastIsSymbol || bFormulaMode;
    if ( bDisable )
        nCntrl &= ~EE_CNTRL_AUTOCORRECT;
    else
        nCntrl |= EE_CNTRL_AUTOCORRECT;

    if ( nCntrl != nOld )
        pEngine->SetControlWord(nCntrl);
}

void ScInputHandler::UpdateSpellSettings( BOOL bFromStartTab )
{
    if ( pActiveViewSh )
    {
        ScViewData* pViewData = pActiveViewSh->GetViewData();
        BOOL bOnlineSpell = pViewData->GetDocument()->GetDocOptions().IsAutoSpell();
        BOOL bHideSpell = pViewData->GetOptions().IsHideAutoSpell();

        //  SetDefaultLanguage is independent of the language attributes,
        //  ScGlobal::GetEditDefaultLanguage is always used.
        //  It must be set every time in case the office language was changed.

        pEngine->SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );

        //  if called for changed options, update flags only if already editing
        //  if called from StartTable, always update flags

        if ( bFromStartTab || eMode != SC_INPUT_NONE )
        {
            ULONG nCntrl = pEngine->GetControlWord();
            ULONG nOld = nCntrl;
            if( bHideSpell )
                nCntrl |= EE_CNTRL_NOREDLINES;
            else
                nCntrl &= ~EE_CNTRL_NOREDLINES;
            if( bOnlineSpell )
                nCntrl |= EE_CNTRL_ONLINESPELLING;
            else
                nCntrl &= ~EE_CNTRL_ONLINESPELLING;
            // kein AutoCorrect auf Symbol-Font (EditEngine wertet Default nicht aus)
            if ( pLastPattern && pLastPattern->IsSymbolFont() )
                nCntrl &= ~EE_CNTRL_AUTOCORRECT;
            else
                nCntrl |= EE_CNTRL_AUTOCORRECT;
            if ( nCntrl != nOld )
                pEngine->SetControlWord(nCntrl);

            ScDocument* pDoc = pViewData->GetDocument();
            pEngine->SetForbiddenCharsTable( pDoc->GetForbiddenCharacters() );
            pEngine->SetAsianCompressionMode( pDoc->GetAsianCompression() );
            pEngine->SetKernAsianPunctuation( pDoc->GetAsianKerning() );
            pEngine->SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( pViewData->GetTabNo() ) );
        }

        //  language is set separately, so the speller is needed only if online
        //  spelling is active

        if ( bOnlineSpell ) {
            com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1> xXSpellChecker1( LinguMgr::GetSpellChecker() );
            pEngine->SetSpeller( xXSpellChecker1 );
        }

        BOOL bHyphen = pLastPattern && ((const SfxBoolItem&)pLastPattern->GetItem(ATTR_HYPHENATE)).GetValue();
        if ( bHyphen ) {
            com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
            pEngine->SetHyphenator( xXHyphenator );
        }
    }
}

//
//      Funktionen/Bereichsnamen etc. als Tip-Hilfe
//

#define SC_STRTYPE_FUNCTIONS    1
//  die anderen Typen sind in ScDocument::GetFormulaEntries festgelegt

void ScInputHandler::GetFormulaData()
{
    if ( pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();

        if ( pFormulaData )
            pFormulaData->FreeAll();
        else
            pFormulaData = new TypedStrCollection;

        //      MRU-Funktionen aus dem Funktions-Autopiloten
        //      wie in ScPosWnd::FillFunctions (inputwin.cxx)

        const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
        USHORT nMRUCount = rOpt.GetLRUFuncListCount();
        const USHORT* pMRUList = rOpt.GetLRUFuncList();
        if (pMRUList)
        {
            const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
            ULONG nListCount = pFuncList->GetCount();
            ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
            for (USHORT i=0; i<nMRUCount; i++)
            {
                USHORT nId = pMRUList[i];
                for (ULONG j=0; j<nListCount; j++)
                {
                    const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                    if ( pDesc->nFIndex == nId && pDesc->pFuncName )
                    {
                        String aEntry = *pDesc->pFuncName;
                        aEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
                        TypedStrData* pData = new TypedStrData( aEntry, 0.0, SC_STRTYPE_FUNCTIONS );
                        if (!pFormulaData->Insert(pData))
                            delete pData;
                        break;                  // nicht weitersuchen
                    }
                }
            }
        }

        pDoc->GetFormulaEntries( *pFormulaData );
    }
}

void ScInputHandler::HideTip()
{
    if ( nTipVisible )
    {
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
    }
    aManualTip.Erase();
}

void ScInputHandler::ShowTip( const String& rText )
{
    //  aManualTip muss hinterher von aussen gesetzt werden
    HideTip();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
    {
        Point aPos;
        Window* pWin = pActiveView->GetWindow();
        Cursor* pCur = pActiveView->GetCursor();
        if (pCur)
            aPos = pWin->LogicToPixel( pCur->GetPos() );
        aPos = pWin->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aPos );

        USHORT nAlign = QUICKHELP_LEFT|QUICKHELP_BOTTOM;
        nTipVisible = Help::ShowTip(pWin, aRect, rText, nAlign);
    }
}

void ScInputHandler::UseFormulaData()
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;

    //  Formeln duerfen nur 1 Absatz haben
    if ( pActiveView && pFormulaData && pEngine->GetParagraphCount() == 1 )
    {
        String aTotal = pEngine->GetText( (USHORT) 0 );
        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();

        //  #59348# Durch Differenzen zwischen Tabelle und Eingabezeile
        //  (z.B. Clipboard mit Zeilenumbruechen) kann es sein, dass die Selektion
        //  nicht mehr zur EditEngine passt. Dann halt kommentarlos abbrechen:

        if ( aSel.nEndPos > aTotal.Len() )
            return;

        //  steht der Cursor am Ende eines Wortes?

        if ( aSel.nEndPos > 0 && (
                aSel.nEndPos == aTotal.Len() ||
                pEngine->GetWordDelimiters().Search( aTotal.GetChar(aSel.nEndPos) ) != STRING_NOTFOUND ) )
        {
            String aText = pEngine->GetWord( 0, aSel.nEndPos-1 );
            if (aText.Len())
            {
                String aNew;
                nAutoPos = SCPOS_INVALID;   // nix
                if ( pFormulaData->FindText( aText, aNew, nAutoPos, FALSE ) )
                {
                    ShowTip( aNew );        //  als QuickHelp anzeigen
                    aAutoSearch = aText;    // zum Weitersuchen - nAutoPos ist gesetzt
                }
            }
        }
    }
}

void ScInputHandler::NextFormulaEntry( BOOL bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pFormulaData )
    {
        String aNew;
        if ( pFormulaData->FindText( aAutoSearch, aNew, nAutoPos, bBack ) )
            ShowTip( aNew );        //  als QuickHelp anzeigen
    }

    //  bei Tab wird vorher immer HideCursor gerufen

    if (pActiveView)
        pActiveView->ShowCursor();
}

void lcl_CompleteFunction( EditView* pView, const String& rInsert, BOOL& rParInserted )
{
    if (pView)
    {
        ESelection aSel = pView->GetSelection();
        --aSel.nStartPos;
        --aSel.nEndPos;
        pView->SetSelection(aSel);
        pView->SelectCurrentWord();

        String aInsStr = rInsert;
        xub_StrLen nInsLen = aInsStr.Len();
        BOOL bDoParen = ( nInsLen > 1 && aInsStr.GetChar(nInsLen-2) == '('
                                      && aInsStr.GetChar(nInsLen-1) == ')' );
        if ( bDoParen )
        {
            //  Klammern hinter Funktionsnamen nicht einfuegen, wenn direkt dahinter
            //  schon eine Klammer steht (z.B. wenn der Funktionsname geaendert wurde,
            //  #39393#).

            ESelection aWordSel = pView->GetSelection();
            String aOld = pView->GetEditEngine()->GetText((USHORT)0);
            sal_Unicode cNext = aOld.GetChar(aWordSel.nEndPos);
            if ( cNext == '(' )
            {
                bDoParen = FALSE;
                aInsStr.Erase( nInsLen - 2 );   // Klammern weglassen
            }
        }

        pView->InsertText( aInsStr, FALSE );

        if ( bDoParen )                         // Cursor zwischen die Klammern setzen
        {
            aSel = pView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pView->SetSelection(aSel);

            rParInserted = TRUE;
        }
    }
}

void ScInputHandler::PasteFunctionData()
{
    if ( pFormulaData && nAutoPos != SCPOS_INVALID )
    {
        TypedStrData* pData = (*pFormulaData)[nAutoPos];
        if (pData)
        {
            String aInsert = pData->GetString();
            BOOL bParInserted = FALSE;

            DataChanging();                         // kann nicht neu sein
            lcl_CompleteFunction( pTopView, aInsert, bParInserted );
            lcl_CompleteFunction( pTableView, aInsert, bParInserted );
            DataChanged();

            if (bParInserted)
                AutoParAdded();
        }
    }

    HideTip();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
        pActiveView->ShowCursor();
}

//
//      Selektion berechnen und als Tip-Hilfe anzeigen
//

String lcl_Calculate( const String& rFormula, ScDocument* pDoc, const ScAddress &rPos )
{
    //!     mit ScFormulaDlg::CalcValue zusammenfassen und ans Dokument verschieben !!!!
    //!     (Anfuehrungszeichen bei Strings werden nur hier eingefuegt)

    String aValue;

    if (rFormula.Len())
    {
        ScFormulaCell* pCell = new ScFormulaCell( pDoc, rPos, rFormula );

        // #35521# HACK! um bei ColRowNames kein #REF! zu bekommen,
        // wenn ein Name eigentlich als Bereich in die Gesamt-Formel
        // eingefuegt wird, bei der Einzeldarstellung aber als
        // single-Zellbezug interpretiert wird
        BOOL bColRowName = pCell->HasColRowName();
        if ( bColRowName )
        {
            // ColRowName im RPN-Code?
            if ( pCell->GetCode()->GetCodeLen() <= 1 )
            {   // ==1: einzelner ist als Parameter immer Bereich
                // ==0: es waere vielleicht einer, wenn..
                String aBraced( '(' );
                aBraced += rFormula;
                aBraced += ')';
                delete pCell;
                pCell = new ScFormulaCell( pDoc, rPos, aBraced );
            }
            else
                bColRowName = FALSE;
        }

        USHORT nErrCode = pCell->GetErrCode();
        if ( nErrCode == 0 )
        {
            SvNumberFormatter& aFormatter = *(pDoc->GetFormatTable());
            Color* pColor;
            if ( pCell->IsValue() )
            {
                double n = pCell->GetValue();
                ULONG nFormat = aFormatter.GetStandardFormat( n, 0,
                                pCell->GetFormatType(), ScGlobal::eLnge );
                aFormatter.GetInputLineString( n, nFormat, aValue );
                //! display OutputString but insert InputLineString
            }
            else
            {
                String aStr;

                pCell->GetString( aStr );
                ULONG nFormat = aFormatter.GetStandardFormat(
                                pCell->GetFormatType(), ScGlobal::eLnge);
                aFormatter.GetOutputString( aStr, nFormat,
                                            aValue, &pColor );

                aValue.Insert('"',0);   // in Anfuehrungszeichen
                aValue+='"';
                //! Anfuehrungszeichen im String escapen ????
            }

            ScRange aTestRange;
            if ( bColRowName || (aTestRange.Parse(rFormula) & SCA_VALID) )
                aValue.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ..." ));       // Bereich
        }
        else
            aValue = ScGlobal::GetErrorString(nErrCode);
        delete pCell;
    }

    return aValue;
}

void ScInputHandler::FormulaPreview()
{
    String aValue;
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pActiveViewSh )
    {
        String aPart = pActiveView->GetSelected();
        if (!aPart.Len())
            aPart = pEngine->GetText((USHORT)0);
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();
        aValue = lcl_Calculate( aPart, pDoc, aCursorPos );
    }

    if (aValue.Len())
    {
        ShowTip( aValue );          //  als QuickHelp anzeigen
        aManualTip = aValue;        //  nach ShowTip setzen
        nAutoPos = SCPOS_INVALID;   //  Formel-Autocomplete aufheben
    }
}

void ScInputHandler::PasteManualTip()
{
    //  drei Punkte am Ende -> Bereichsreferenz -> nicht einfuegen
    //  (wenn wir mal Matrix-Konstanten haben, kann das geaendert werden)

    xub_StrLen nTipLen = aManualTip.Len();
    if ( nTipLen && ( nTipLen < 3 || !aManualTip.Copy( nTipLen-3 ).EqualsAscii("...") ) )
    {
        DataChanging();                                     // kann nicht neu sein

        String aInsert = aManualTip;
        EditView* pActiveView = pTopView ? pTopView : pTableView;
        if (!pActiveView->HasSelection())
        {
            //  nichts selektiert -> alles selektieren
            xub_StrLen nOldLen = pEngine->GetTextLen(0);
            ESelection aAllSel( 0, 0, 0, nOldLen );
            if ( pTopView )
                pTopView->SetSelection( aAllSel );
            if ( pTableView )
                pTableView->SetSelection( aAllSel );
        }

        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();
        DBG_ASSERT( !aSel.nStartPara && !aSel.nEndPara, "Zuviele Absaetze in Formel" );
        if ( !aSel.nStartPos )  // Selektion ab Anfang?
        {
            if ( aSel.nEndPos == pEngine->GetTextLen(0) )
            {
                //  alles selektiert -> Anfuehrungszeichen weglassen
                if ( aInsert.GetChar(0) == '"' )
                    aInsert.Erase(0,1);
                xub_StrLen nInsLen = aInsert.Len();
                if ( nInsLen && aInsert.GetChar(nInsLen-1) == '"' )
                    aInsert.Erase( nInsLen-1 );
            }
            else if ( aSel.nEndPos )
            {
                //  nicht alles selektiert -> Gleichheitszeichen nicht ueberschreiben
                //! doppelte Gleichheitszeichen auch ???

                aSel.nStartPos = 1;
                if ( pTopView )
                    pTopView->SetSelection( aSel );
                if ( pTableView )
                    pTableView->SetSelection( aSel );
            }
        }
        if ( pTopView )
            pTopView->InsertText( aInsert, TRUE );
        if ( pTableView )
            pTableView->InsertText( aInsert, TRUE );

        DataChanged();
    }

    HideTip();
}

void ScInputHandler::ResetAutoPar()
{
    nAutoPar = 0;
}

void ScInputHandler::AutoParAdded()
{
    ++nAutoPar;     //  closing parenthesis can be overwritten
}

BOOL ScInputHandler::CursorAtClosingPar()
{
    //  test if the cursor is before a closing parenthesis

    //  selection from SetReference has been removed before
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && !pActiveView->HasSelection() && bFormulaMode )
    {
        ESelection aSel = pActiveView->GetSelection();
        xub_StrLen nPos = aSel.nStartPos;
        String aFormula = pEngine->GetText((USHORT)0);
        if ( nPos < aFormula.Len() && aFormula.GetChar(nPos) == ')' )
            return TRUE;
    }
    return FALSE;
}

void ScInputHandler::SkipClosingPar()
{
    //  this is called when a ')' is typed and the cursor is before a ')'
    //  that can be overwritten -> just set the cursor behind the ')'

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
    {
        ESelection aSel = pActiveView->GetSelection();
        ++aSel.nStartPos;
        ++aSel.nEndPos;

        //  this is in a formula (only one paragraph), so the selection
        //  can be used directly for the TopView

        if ( pTopView )
            pTopView->SetSelection( aSel );
        if ( pTableView )
            pTableView->SetSelection( aSel );
    }

    DBG_ASSERT(nAutoPar, "SkipClosingPar: count is wrong");
    --nAutoPar;
}

//
//      Auto-Eingabe
//

void ScInputHandler::GetColData()
{
    if ( pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();

        if ( pColumnData )
            pColumnData->FreeAll();
        else
        {
            pColumnData = new TypedStrCollection;
            pColumnData->SetCaseSensitive( TRUE );      // equal strings are handled in FindText
        }

        pDoc->GetDataEntries( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                                *pColumnData, TRUE );
    }
}

void ScInputHandler::UseColData()           // beim Tippen
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pColumnData )
    {
        //  nur anpassen, wenn Cursor am Ende steht

        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();

        USHORT nParCnt = pEngine->GetParagraphCount();
        if ( aSel.nEndPara+1 == nParCnt )
        {
            xub_StrLen nParLen = pEngine->GetTextLen( aSel.nEndPara );
            if ( aSel.nEndPos == nParLen )
            {
                String aText = GetEditText(pEngine);
                if (aText.Len())
                {
                    String aNew;
                    nAutoPos = SCPOS_INVALID;   // nix
                    if ( pColumnData->FindText( aText, aNew, nAutoPos, FALSE ) )
                    {
                        //  #45434# durch dBase Import etc. koennen Umbrueche im String sein,
                        //  das wuerde hier mehrere Absaetze ergeben -> nicht gut
                        //! GetExactMatch funktioniert dann auch nicht
                        lcl_RemoveLineEnd( aNew );

                        //  Absaetze beibehalten, nur den Rest anfügen
                        //! genaue Ersetzung im EnterHandler !!!

                        // ein Space zwischen Absaetzen:
                        ULONG nEdLen = pEngine->GetTextLen() + nParCnt - 1;
                        String aIns = aNew.Copy( (xub_StrLen)nEdLen );

                        //  selection must be "backwards", so the cursor stays behind the last
                        //  typed character
                        ESelection aSelection( aSel.nEndPara, aSel.nEndPos + aIns.Len(),
                                               aSel.nEndPara, aSel.nEndPos );

                        //  when editing in input line, apply to both edit views
                        if ( pTableView )
                        {
                            pTableView->InsertText( aIns, FALSE );
                            pTableView->SetSelection( aSelection );
                        }
                        if ( pTopView )
                        {
                            pTopView->InsertText( aIns, FALSE );
                            pTopView->SetSelection( aSelection );
                        }

                        aAutoSearch = aText;    // zum Weitersuchen - nAutoPos ist gesetzt

                        if ( aText.Len() == aNew.Len() )
                        {
                            //  Wenn der eingegebene Text gefunden wurde, TAB nur dann
                            //  verschlucken, wenn noch etwas kommt

                            String aDummy;
                            USHORT nNextPos = nAutoPos;
                            bUseTab = pColumnData->FindText( aText, aDummy, nNextPos, FALSE );
                        }
                        else
                            bUseTab = TRUE;
                    }
                }
            }
        }
    }
}

void ScInputHandler::NextAutoEntry( BOOL bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pColumnData )
    {
        if ( nAutoPos != SCPOS_INVALID && aAutoSearch.Len() )
        {
            //  stimmt die Selektion noch? (kann per Maus geaendert sein)

            ESelection aSel = pActiveView->GetSelection();
            aSel.Adjust();
            USHORT nParCnt = pEngine->GetParagraphCount();
            if ( aSel.nEndPara+1 == nParCnt && aSel.nStartPara == aSel.nEndPara )
            {
                String aText = GetEditText(pEngine);
                xub_StrLen nSelLen = aSel.nEndPos - aSel.nStartPos;
                xub_StrLen nParLen = pEngine->GetTextLen( aSel.nEndPara );
                if ( aSel.nEndPos == nParLen && aText.Len() == aAutoSearch.Len() + nSelLen )
                {
                    String aNew;
                    if ( pColumnData->FindText( aAutoSearch, aNew, nAutoPos, bBack ) )
                    {
                        bInOwnChange = TRUE;        // disable ModifyHdl (reset below)

                        lcl_RemoveLineEnd( aNew );
                        String aIns = aNew.Copy( aAutoSearch.Len() );

                        //  when editing in input line, apply to both edit views
                        if ( pTableView )
                        {
                            pTableView->DeleteSelected();
                            pTableView->InsertText( aIns, FALSE );
                            pTableView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.Len(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }
                        if ( pTopView )
                        {
                            pTopView->DeleteSelected();
                            pTopView->InsertText( aIns, FALSE );
                            pTopView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.Len(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }

                        bInOwnChange = FALSE;
                    }
                    else
                    {
                        // mehr gibts nicht

                        Sound::Beep();
                    }
                }
            }
        }
    }

    //  bei Tab wird vorher immer HideCursor gerufen

    if (pActiveView)
        pActiveView->ShowCursor();
}

//
//      Klammern hervorheben
//

xub_StrLen lcl_MatchParenthesis( const String& rStr, xub_StrLen nPos )
{
    int nDir;
    sal_Unicode c1, c2;
    c1 = rStr.GetChar( nPos );
    switch ( c1 )
    {
        case '(' :
            c2 = ')';
            nDir = 1;
        break;
        case ')' :
            c2 = '(';
            nDir = -1;
        break;
        case '<' :
            c2 = '>';
            nDir = 1;
        break;
        case '>' :
            c2 = '<';
            nDir = -1;
        break;
        case '{' :
            c2 = '}';
            nDir = 1;
        break;
        case '}' :
            c2 = '{';
            nDir = -1;
        break;
        case '[' :
            c2 = ']';
            nDir = 1;
        break;
        case ']' :
            c2 = '[';
            nDir = -1;
        break;
        default:
            nDir = 0;
    }
    if ( !nDir )
        return STRING_NOTFOUND;
    xub_StrLen nLen = rStr.Len();
    const sal_Unicode* p0 = rStr.GetBuffer();
    register const sal_Unicode* p;
    const sal_Unicode* p1;
    USHORT nQuotes = 0;
    if ( nPos < nLen / 2 )
    {
        p = p0;
        p1 = p0 + nPos;
    }
    else
    {
        p = p0 + nPos;
        p1 = p0 + nLen;
    }
    while ( p < p1 )
    {
        if ( *p++ == '\"' )
            nQuotes++;
    }
    // ungerade Anzahl Quotes: wir befinden uns in einem String
    BOOL bLookInString = ((nQuotes % 2) != 0);
    BOOL bInString = bLookInString;
    p = p0 + nPos;
    p1 = (nDir < 0 ? p0 : p0 + nLen) ;
    USHORT nLevel = 1;
    while ( p != p1 && nLevel )
    {
        p += nDir;
        if ( *p == '\"' )
        {
            bInString = !bInString;
            if ( bLookInString && !bInString )
                p = p1;     // das war's dann
        }
        else if ( bInString == bLookInString )
        {
            if ( *p == c1 )
                nLevel++;
            else if ( *p == c2 )
                nLevel--;
        }
    }
    if ( nLevel )
        return STRING_NOTFOUND;
    return (xub_StrLen) (p - p0);
}

void ScInputHandler::UpdateParenthesis()
{
    //  Klammern suchen

    //! Klammer-Hervorhebung einzeln abschaltbar ????

    BOOL bFound = FALSE;
    if ( bFormulaMode && eMode != SC_INPUT_TOP )
    {
        if ( pTableView && !pTableView->HasSelection() )        // Selektion ist immer unten
        {
            ESelection aSel = pTableView->GetSelection();
            if (aSel.nStartPos)
            {
                //  Das Zeichen links vom Cursor wird angeschaut

                xub_StrLen nPos = aSel.nStartPos - 1;
                String aFormula = pEngine->GetText((USHORT)0);
                sal_Unicode c = aFormula.GetChar(nPos);
                if ( c == '(' || c == ')' )
                {
                    xub_StrLen nOther = lcl_MatchParenthesis( aFormula, nPos );
                    if ( nOther != STRING_NOTFOUND )
                    {
                        SfxItemSet aSet( pEngine->GetEmptyItemSet() );
                        aSet.Put( SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
                        //! Unterscheidung, wenn die Zelle schon fett ist !!!!

                        if (bParenthesisShown)
                        {
                            //  alte Hervorhebung wegnehmen
                            USHORT nCount = pEngine->GetParagraphCount();
                            for (USHORT i=0; i<nCount; i++)
                                pEngine->QuickRemoveCharAttribs( i, EE_CHAR_WEIGHT );
                        }

                        ESelection aSelThis( 0,nPos, 0,nPos+1 );
                        pEngine->QuickSetAttribs( aSet, aSelThis );
                        ESelection aSelOther( 0,nOther, 0,nOther+1 );
                        pEngine->QuickSetAttribs( aSet, aSelOther );

                        //  Dummy-InsertText fuer Update und Paint (Selektion ist leer)
                        pTableView->InsertText( EMPTY_STRING, FALSE );

                        bFound = TRUE;
                    }
                }
            }

            //  mark parenthesis right of cursor if it will be overwritten (nAutoPar)
            //  with different color (COL_LIGHTBLUE) ??
        }
    }

    //  alte Hervorhebung wegnehmen, wenn keine neue gesetzt

    if ( bParenthesisShown && !bFound && pTableView )
    {
        USHORT nCount = pEngine->GetParagraphCount();
        for (USHORT i=0; i<nCount; i++)
            pTableView->RemoveCharAttribs( i, EE_CHAR_WEIGHT );
    }

    bParenthesisShown = bFound;
}

void ScInputHandler::ViewShellGone(ScTabViewShell* pViewSh)     // wird synchron aufgerufen!
{
    if ( pViewSh == pActiveViewSh )
    {
        delete pLastState;
        pLastState = NULL;
        pLastPattern = NULL;
    }

    if ( pViewSh == pRefViewSh )
    {
        //! Die Eingabe kommt aus dem EnterHandler nicht mehr an
        //  Trotzdem wird immerhin der Editmodus beendet

        EnterHandler();
        bFormulaMode = FALSE;
        pRefViewSh = NULL;
        SFX_APP()->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(FALSE);
        UpdateAutoCorrFlag();
    }

    pActiveViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );

    if ( pActiveViewSh && pActiveViewSh == pViewSh )
    {
        DBG_ERROR("pActiveViewSh weg");
        pActiveViewSh = NULL;
    }

    if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        UpdateRefDevice();      // don't keep old document's printer as RefDevice
}

void ScInputHandler::UpdateActiveView()
{
    ImplCreateEditEngine();
    Window* pShellWin = pActiveViewSh ? pActiveViewSh->GetActiveWin() : NULL;

    USHORT nCount = pEngine->GetViewCount();
    if (nCount > 0)
    {
        pTableView = pEngine->GetView(0);
        for (USHORT i=1; i<nCount; i++)
        {
            EditView* pThis = pEngine->GetView(i);
            Window* pWin = pThis->GetWindow();
            if ( pWin==pShellWin || pWin->HasFocus() )
                pTableView = pThis;
        }
    }
    else
        pTableView = NULL;

    if (pInputWin)
        pTopView = pInputWin->GetEditView();
    else
        pTopView = NULL;
}

void ScInputHandler::StopInputWinEngine( BOOL bAll )
{
    if (pInputWin)
        pInputWin->StopEditEngine( bAll );

    pTopView = NULL;        // invalid now
}

void ScInputHandler::ActivateInputWindow( const String&     rText,
                                          const ESelection& rSel )
{
    if ( pInputWin )
        if ( !pInputWin->IsActive() )
            pTopView = pInputWin->ActivateEdit( rText, rSel );
}

EditView* ScInputHandler::GetActiveView()
{
    UpdateActiveView();
    return pTopView ? pTopView : pTableView;
}

void ScInputHandler::ForgetLastPattern()
{
    pLastPattern = NULL;
    if ( !pLastState && pActiveViewSh )
        pActiveViewSh->UpdateInputHandler( TRUE );      // Status neu holen
    else
        NotifyChange( pLastState, TRUE );
}

void ScInputHandler::UpdateAdjust( sal_Unicode cTyped )
{
    SvxAdjust eSvxAdjust;
    switch (nAttrAdjust)
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            {
                BOOL bNumber = FALSE;
                if (cTyped)                                     // neu angefangen
                    bNumber = (cTyped>='0' && cTyped<='9');     // nur Ziffern sind Zahlen
                else if ( pActiveViewSh )
                {
                    ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();
                    bNumber = ( pDoc->GetCellType( aCursorPos ) == CELLTYPE_VALUE );
                }
                eSvxAdjust = bNumber ? SVX_ADJUST_RIGHT : SVX_ADJUST_LEFT;
            }
            break;
        case SVX_HOR_JUSTIFY_BLOCK:
            eSvxAdjust = SVX_ADJUST_BLOCK;
            break;
        case SVX_HOR_JUSTIFY_CENTER:
            eSvxAdjust = SVX_ADJUST_CENTER;
            break;
        case SVX_HOR_JUSTIFY_RIGHT:
            eSvxAdjust = SVX_ADJUST_RIGHT;
            break;
        default:    // SVX_HOR_JUSTIFY_LEFT
            eSvxAdjust = SVX_ADJUST_LEFT;
            break;
    }

    BOOL bAsianVertical = pLastPattern &&
        ((const SfxBoolItem&)pLastPattern->GetItem( ATTR_STACKED )).GetValue() &&
        ((const SfxBoolItem&)pLastPattern->GetItem( ATTR_VERTICAL_ASIAN )).GetValue();
    if ( bAsianVertical )
    {
        //  always edit at top of cell -> LEFT when editing vertically
        eSvxAdjust = SVX_ADJUST_LEFT;
    }

    pEditDefaults->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
    pEngine->SetDefaults( *pEditDefaults );

    nEditAdjust = eSvxAdjust;       //! an ViewData setzen oder beim PostEditView

    pEngine->SetVertical( bAsianVertical );
}

void ScInputHandler::RemoveAdjust()
{
    //  harte Ausrichtungs-Attribute loeschen

    BOOL bUndo = pEngine->IsUndoEnabled();
    if ( bUndo )
        pEngine->EnableUndo( FALSE );

    //  RemoveParaAttribs removes all paragraph attributes, including EE_PARA_JUST
#if 0
    BOOL bChange = FALSE;
    USHORT nCount = pEngine->GetParagraphCount();
    for (USHORT i=0; i<nCount; i++)
    {
        const SfxItemSet& rOld = pEngine->GetParaAttribs( i );
        if ( rOld.GetItemState( EE_PARA_JUST ) == SFX_ITEM_SET )
        {
            SfxItemSet aNew( rOld );
            aNew.ClearItem( EE_PARA_JUST );
            pEngine->SetParaAttribs( i, aNew );
            bChange = TRUE;
        }
    }
#endif

    //  #89403# non-default paragraph attributes (e.g. from clipboard)
    //  must be turned into character attributes
    pEngine->RemoveParaAttribs();

    if ( bUndo )
        pEngine->EnableUndo( TRUE );

    // ER 31.08.00  Only called in EnterHandler, don't change view anymore.
#if 0
    if (bChange)
    {
        EditView* pActiveView = pTopView ? pTopView : pTableView;
        pActiveView->ShowCursor( FALSE, TRUE );
    }
#endif
}

void ScInputHandler::RemoveRangeFinder()
{
    //  pRangeFindList und Farben loeschen

    pEngine->SetUpdateMode(FALSE);
    USHORT nCount = pEngine->GetParagraphCount();   // koennte gerade neu eingefuegt worden sein
    for (USHORT i=0; i<nCount; i++)
        pEngine->QuickRemoveCharAttribs( i, EE_CHAR_COLOR );
    pEngine->SetUpdateMode(TRUE);

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    pActiveView->ShowCursor( FALSE, TRUE );

    DeleteRangeFinder();        // loescht die Liste und die Markierungen auf der Tabelle
}

BOOL ScInputHandler::StartTable( sal_Unicode cTyped, BOOL bFromCommand )
{
    // returns TRUE if a new edit mode was started

    BOOL bNewTable = FALSE;

    if (!bModified && ValidCol(aCursorPos.Col()))
    {
        if (pActiveViewSh)
        {
            ImplCreateEditEngine();
            UpdateActiveView();
            SyncViews();

            ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();

            const ScMarkData& rMark = pActiveViewSh->GetViewData()->GetMarkData();
            ScEditableTester aTester;
            if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                aTester.TestSelection( pDoc, rMark );
            else
                aTester.TestSelectedBlock( pDoc, aCursorPos.Col(),aCursorPos.Row(),
                                                 aCursorPos.Col(),aCursorPos.Row(), rMark );
            if ( aTester.IsEditable() )
            {
                // UpdateMode is enabled again in ScViewData::SetEditEngine (and not needed otherwise)
                pEngine->SetUpdateMode( FALSE );

                //  Attribute in EditEngine uebernehmen

                const ScPatternAttr* pPattern = pDoc->GetPattern( aCursorPos.Col(),
                                                                  aCursorPos.Row(),
                                                                  aCursorPos.Tab() );
                if (pPattern != pLastPattern)
                {
                    //  Prozent-Format?

                    const SfxItemSet& rAttrSet = pPattern->GetItemSet();
                    const SfxPoolItem* pItem;

                    if ( SFX_ITEM_SET == rAttrSet.GetItemState( ATTR_VALUE_FORMAT, TRUE, &pItem ) )
                    {
                        ULONG nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                        bCellHasPercentFormat = ( NUMBERFORMAT_PERCENT ==
                                                  pDoc->GetFormatTable()->GetType( nFormat ) );
                    }
                    else
                        bCellHasPercentFormat = FALSE; // Default: kein Prozent

                    //  Gueltigkeit angegeben?

                    if ( SFX_ITEM_SET == rAttrSet.GetItemState( ATTR_VALIDDATA, TRUE, &pItem ) )
                        nValidation = ((const SfxUInt32Item*)pItem)->GetValue();
                    else
                        nValidation = 0;

                    //  EditEngine Defaults

                    //  Hier auf keinen Fall SetParaAttribs, weil die EditEngine evtl.
                    //  schon gefuellt ist (bei Edit-Zellen).
                    //  SetParaAttribs wuerde dann den Inhalt aendern

                    //! ER 30.08.00  The SetDefaults is now (since MUST/src602
                    //! EditEngine changes) implemented as a SetParaAttribs.
                    //! Any problems?

                    pPattern->FillEditItemSet( pEditDefaults );
                    pEngine->SetDefaults( *pEditDefaults );
                    pLastPattern = pPattern;
                    bLastIsSymbol = pPattern->IsSymbolFont();

                    //  Background color must be known for automatic font color.
                    //  For transparent cell background, the document background color must be used.

                    Color aBackCol = ((const SvxBrushItem&)
                                    pPattern->GetItem( ATTR_BACKGROUND )).GetColor();
                    ScModule* pScMod = SC_MOD();
                    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
                    if ( aBackCol.GetTransparency() > 0 ||
                            Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                        aBackCol.SetColor( pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );
                    pEngine->SetBackgroundColor( aBackCol );

                    //  Ausrichtung

                    nAttrAdjust = ((const SvxHorJustifyItem&)pPattern->
                                    GetItem(ATTR_HOR_JUSTIFY)).GetValue();
                }

                //  UpdateSpellSettings enables online spelling if needed
                //  -> also call if attributes are unchanged

                UpdateSpellSettings( TRUE );    // uses pLastPattern

                //  Edit-Engine fuellen

                String aStr;
                if (bTextValid)
                {
                    pEngine->SetText(aCurrentText);
                    aStr = aCurrentText;
                    bTextValid = FALSE;
                    aCurrentText.Erase();
                }
                else
                    aStr = GetEditText(pEngine);

                if (aStr.Len() > 3 &&                   // Matrix-Formel ?
                    aStr.GetChar(0) == '{' &&
                    aStr.GetChar(1) == '=' &&
                    aStr.GetChar(aStr.Len()-1) == '}')
                {
                    aStr.Erase(0,1);
                    aStr.Erase(aStr.Len()-1,1);
                    pEngine->SetText(aStr);
                    if ( pInputWin )
                        pInputWin->SetTextString(aStr);
                }

                UpdateAdjust( cTyped );

                if ( bAutoComplete )
                    GetColData();

                if ( aStr.GetChar(0) == '=' && !cTyped && !bCreatingFuncView )
                    InitRangeFinder(aStr);              // Formel wird editiert -> RangeFinder

                bNewTable = TRUE;       //  -> PostEditView-Aufruf
            }
            else
            {
                bProtected = TRUE;
                eMode = SC_INPUT_NONE;
                StopInputWinEngine( TRUE );
                UpdateFormulaMode();
                if ( pActiveViewSh && ( !bFromCommand || !bCommandErrorShown ) )
                {
                    //  #97673# Prevent repeated error messages for the same cell from command events
                    //  (for keyboard events, multiple messages are wanted).
                    //  Set the flag before showing the error message because the command handler
                    //  for the next IME command may be called when showing the dialog.
                    if ( bFromCommand )
                        bCommandErrorShown = TRUE;

                    pActiveViewSh->GetActiveWin()->GrabFocus();
                    pActiveViewSh->ErrorMessage(aTester.GetMessageId());
                }
            }
        }

        if (!bProtected && pInputWin)
            pInputWin->SetOkCancelMode();
    }

    return bNewTable;
}

void lcl_SetTopSelection( EditView* pEditView, ESelection& rSel )
{
    DBG_ASSERT( rSel.nStartPara==0 && rSel.nEndPara==0, "SetTopSelection: Para != 0" );

    EditEngine* pEngine = pEditView->GetEditEngine();
    USHORT nCount = pEngine->GetParagraphCount();
    if (nCount > 1)
    {
        xub_StrLen nParLen = pEngine->GetTextLen(rSel.nStartPara);
        while (rSel.nStartPos > nParLen && rSel.nStartPara+1 < nCount)
        {
            rSel.nStartPos -= nParLen + 1;          // incl. Leerzeichen vom Umbruch
            nParLen = pEngine->GetTextLen(++rSel.nStartPara);
        }

        nParLen = pEngine->GetTextLen(rSel.nEndPara);
        while (rSel.nEndPos > nParLen && rSel.nEndPara+1 < nCount)
        {
            rSel.nEndPos -= nParLen + 1;            // incl. Leerzeichen vom Umbruch
            nParLen = pEngine->GetTextLen(++rSel.nEndPara);
        }
    }

    ESelection aSel = pEditView->GetSelection();

    if (   rSel.nStartPara != aSel.nStartPara || rSel.nEndPara != aSel.nEndPara
        || rSel.nStartPos  != aSel.nStartPos  || rSel.nEndPos  != aSel.nEndPos )
        pEditView->SetSelection( rSel );
}

void ScInputHandler::SyncViews( EditView* pSourceView )
{
    ESelection aSel;

    if (pSourceView)
    {
        aSel = pSourceView->GetSelection();
        if (pTopView && pTopView != pSourceView)
            pTopView->SetSelection( aSel );
        if (pTableView && pTableView != pSourceView)
            lcl_SetTopSelection( pTableView, aSel );
    }
    else if (pTopView && pTableView)
    {
        aSel = pTopView->GetSelection();
        lcl_SetTopSelection( pTableView, aSel );
    }
}

void ScInputHandler::SetAllUpdateMode( BOOL bUpdate )
{
    ImplCreateEditEngine();
    pEngine->SetUpdateMode( bUpdate );
}

IMPL_LINK( ScInputHandler, ModifyHdl, void *, EMPTYARG )
{
    if ( !bInOwnChange && ( eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE ) &&
         pEngine && pEngine->GetUpdateMode() && pInputWin )
    {
        //  #102745# update input line from ModifyHdl for changes that are not
        //  wrapped by DataChanging/DataChanged calls (like Drag&Drop)

        String aText = GetEditText(pEngine);
        lcl_RemoveTabs(aText);
        pInputWin->SetTextString(aText);
    }
    return 0;
}

BOOL ScInputHandler::DataChanging( sal_Unicode cTyped, BOOL bFromCommand )      // return TRUE = new view created
{
    bInOwnChange = TRUE;                // disable ModifyHdl (reset in DataChanged)

    if ( eMode == SC_INPUT_NONE )
        return StartTable( cTyped, bFromCommand );
    else
        return FALSE;
}

void ScInputHandler::DataChanged( BOOL bFromTopNotify )
{
    ImplCreateEditEngine();

    if (eMode==SC_INPUT_NONE)
        eMode = SC_INPUT_TYPE;

    if ( eMode == SC_INPUT_TOP && pTopView && !bFromTopNotify )
    {
        //  table EditEngine is formatted below, input line needs formatting after paste
        //  #i20282# not when called from the input line's modify handler
        pTopView->GetEditEngine()->QuickFormatDoc( TRUE );

        //  #i23720# QuickFormatDoc hides the cursor, but can't show it again because it
        //  can't safely access the EditEngine's current view, so the cursor has to be
        //  shown again here.
        pTopView->ShowCursor();
    }

    bModified = TRUE;
    bSelIsRef = FALSE;

    if ( pRangeFindList && !bInRangeUpdate )
        RemoveRangeFinder();                    // Attribute und Markierung loeschen

    UpdateParenthesis();    //  Hervorhebung der Klammern neu

    // ER 31.08.00  New SetDefaults sets ParaAttribs, don't clear them away ...
//  RemoveAdjust();     //  #40255# harte Ausrichtungs-Attribute loeschen

    if (eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE)
    {
        String aText = GetEditText(pEngine);
        lcl_RemoveTabs(aText);

        if ( pInputWin )
            pInputWin->SetTextString(aText);
    }

        //  wenn der Cursor vor dem Absatzende steht, werden Teile rechts rausgeschoben
        //  (unabhaengig von eMode)     -> View anpassen!
        //  wenn der Cursor am Ende steht, reicht der Status-Handler an der ViewData

    //  #93767# first make sure the status handler is called now if the cursor
    //  is outside the visible area
    pEngine->QuickFormatDoc();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView && pActiveViewSh)
    {
        ScViewData* pViewData = pActiveViewSh->GetViewData();

        BOOL bNeedGrow = ( nEditAdjust != SVX_ADJUST_LEFT );        // rechtsbuendig immer
        if (!bNeedGrow)
        {
                //  Cursor vor dem Ende?
            ESelection aSel = pActiveView->GetSelection();
            aSel.Adjust();
            bNeedGrow = ( aSel.nEndPos != pEngine->GetTextLen(aSel.nEndPara) );
        }
        if (!bNeedGrow)
        {
            bNeedGrow = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
        }
        if (bNeedGrow)
        {
            // adjust inplace view
            pViewData->EditGrowY();
            pViewData->EditGrowX();
        }
    }

    UpdateFormulaMode();
    bTextValid = FALSE;         // Aenderungen sind nur in der Edit-Engine
    bInOwnChange = FALSE;
}

void ScInputHandler::UpdateFormulaMode()
{
    SfxApplication* pSfxApp = SFX_APP();

    if ( pEngine->GetParagraphCount()==1 && pEngine->GetText((USHORT)0).GetChar(0) == '='
        && !bProtected )
    {
        if (!bFormulaMode)
        {
            bFormulaMode = TRUE;
            pRefViewSh = pActiveViewSh;
            pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
            SC_MOD()->SetRefInputHdl(this);
            if (pInputWin)
                pInputWin->SetFormulaMode(TRUE);

            if ( bAutoComplete )
                GetFormulaData();

            UpdateParenthesis();
            UpdateAutoCorrFlag();
        }
    }
    else        // ausschalten
    {
        if (bFormulaMode)
        {
            ShowRefFrame();
            bFormulaMode = FALSE;
            pRefViewSh = NULL;
            pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
            SC_MOD()->SetRefInputHdl(NULL);
            if (pInputWin)
                pInputWin->SetFormulaMode(FALSE);
            UpdateAutoCorrFlag();
        }
    }
}

void ScInputHandler::ShowRefFrame()
{
    pActiveViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    if ( pRefViewSh && pRefViewSh != pActiveViewSh )
    {
        BOOL bFound = FALSE;
        SfxViewFrame* pRefFrame = pRefViewSh->GetViewFrame();
        SfxViewFrame* pOneFrame = SfxViewFrame::GetFirst();
        while ( pOneFrame && !bFound )
        {
            if ( pOneFrame == pRefFrame )
                bFound = TRUE;
            pOneFrame = SfxViewFrame::GetNext( *pOneFrame );
        }

        if (bFound)
        {
            //  Hier wird sich darauf verlassen, dass Activate synchron funktioniert
            //  (dabei wird pActiveViewSh umgesetzt)

            pRefViewSh->SetActive();    // Appear und SetViewFrame

            //  pLastState wird im NotifyChange aus dem Activate richtig gesetzt
        }
        else
            DBG_ERROR("ViewFrame fuer Referenzeingabe ist nicht mehr da");
    }
}

void ScInputHandler::RemoveSelection()
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (!pActiveView)
        return;

    ESelection aSel = pActiveView->GetSelection();
    aSel.nStartPara = aSel.nEndPara;
    aSel.nStartPos  = aSel.nEndPos;
    if (pTableView)
        pTableView->SetSelection( aSel );
    if (pTopView)
        pTopView->SetSelection( aSel );
}

void ScInputHandler::InvalidateAttribs()
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
    {
        SfxBindings& rBindings = pViewFrm->GetBindings();

        rBindings.Invalidate( SID_ATTR_CHAR_FONT );
        rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
        rBindings.Invalidate( SID_ATTR_CHAR_COLOR );

        rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
        rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
        rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
        rBindings.Invalidate( SID_ULINE_VAL_NONE );
        rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
        rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
        rBindings.Invalidate( SID_ULINE_VAL_DOTTED );

        rBindings.Invalidate( SID_HYPERLINK_GETLINK );
    }
}


//
//      --------------- public Methoden --------------------------------------------
//

void ScInputHandler::SetMode( ScInputMode eNewMode )
{
    if ( eMode == eNewMode )
        return;

    ImplCreateEditEngine();

    if (bProtected)
    {
        eMode = SC_INPUT_NONE;
        StopInputWinEngine( TRUE );
        if (pActiveViewSh)
            pActiveViewSh->GetActiveWin()->GrabFocus();
        return;
    }

    bInOwnChange = TRUE;                // disable ModifyHdl (reset below)

    ScInputMode eOldMode = eMode;
    eMode = eNewMode;
    if (eOldMode == SC_INPUT_TOP && eNewMode != eOldMode)
        StopInputWinEngine( FALSE );

    if (eMode==SC_INPUT_TOP || eMode==SC_INPUT_TABLE)
    {
        if (eOldMode == SC_INPUT_NONE)      // not when switching between modes
        {
            if (StartTable(0, FALSE))       // 0 = look at existing document content for text or number
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
            }
        }

        USHORT nPara    = pEngine->GetParagraphCount()-1;
        xub_StrLen nLen = pEngine->GetText(nPara).Len();
        USHORT nCount   = pEngine->GetViewCount();

        for (USHORT i=0; i<nCount; i++)
        {
            if ( eMode == SC_INPUT_TABLE && eOldMode == SC_INPUT_TOP )
            {
                //  Selektion bleibt
            }
            else
            {
                pEngine->GetView(i)->
                    SetSelection( ESelection( nPara, nLen, nPara, nLen ) );
            }
            pEngine->GetView(i)->ShowCursor(FALSE);
        }
    }

    UpdateActiveView();
    if (eMode==SC_INPUT_TABLE || eMode==SC_INPUT_TYPE)
    {
        if (pTableView)
            pTableView->SetEditEngineUpdateMode(TRUE);
    }
    else
    {
        if (pTopView)
            pTopView->SetEditEngineUpdateMode(TRUE);
    }

    if (eNewMode != eOldMode)
        UpdateFormulaMode();

    bInOwnChange = FALSE;
}

//----------------------------------------------------------------------------------------

//  lcl_IsNumber - TRUE, wenn nur Ziffern (dann keine Autokorrektur)

BOOL lcl_IsNumber(const String& rString)
{
    xub_StrLen nLen = rString.Len();
    for (xub_StrLen i=0; i<nLen; i++)
    {
        sal_Unicode c = rString.GetChar(i);
        if ( c < '0' || c > '9' )
            return FALSE;
    }
    return TRUE;
}

void ScInputHandler::EnterHandler( BYTE nBlockMode )
{
    //  #62806# Bei Makro-Aufrufen fuer Gueltigkeit kann Tod und Teufel passieren,
    //  darum dafuer sorgen, dass EnterHandler nicht verschachtelt gerufen wird:

    if (bInEnterHandler) return;
    bInEnterHandler = TRUE;
    bInOwnChange = TRUE;                // disable ModifyHdl (reset below)

    ImplCreateEditEngine();

    BOOL bMatrix = ( nBlockMode == SC_ENTER_MATRIX );

    SfxApplication* pSfxApp     = SFX_APP();
    EditTextObject* pObject     = NULL;
    ScPatternAttr*  pCellAttrs  = NULL;
    BOOL            bAttrib     = FALSE;    // Formatierung vorhanden ?
    BOOL            bForget     = FALSE;    // wegen Gueltigkeit streichen ?

    String aString = GetEditText(pEngine);
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (bModified && pActiveView && aString.Len() && !lcl_IsNumber(aString))
    {
        if (pTopView)
            pTopView->CompleteAutoCorrect();    // #59759# CompleteAutoCorrect fuer beide Views
        if (pTableView)
            pTableView->CompleteAutoCorrect();
        aString = GetEditText(pEngine);
    }
    lcl_RemoveTabs(aString);

    //  Test, ob zulaessig (immer mit einfachem String)

    if ( bModified && nValidation && pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
        const ScValidationData* pData = pDoc->GetValidationEntry( nValidation );
        if (pData && pData->HasErrMsg())
        {
            const ScPatternAttr* pPattern = pLastPattern;
            if (!pPattern)
            {
                DBG_ERROR("kein Pattern !??!");
                pPattern = pDoc->GetPattern( aCursorPos.Col(), aCursorPos.Row(),
                                              aCursorPos.Tab() );
            }
            BOOL bOk = pData->IsDataValid( aString, *pPattern, aCursorPos );

            if (!bOk)
            {
                if ( pActiveViewSh )                // falls aus MouseButtonDown gekommen
                    pActiveViewSh->StopMarking();   // (die InfoBox verschluckt das MouseButtonUp)

                    //! es gibt noch Probleme, wenn die Eingabe durch Aktivieren einer
                    //! anderen View ausgeloest wurde

                Window* pParent = Application::GetDefDialogParent();
                if ( pData->DoError( pParent, aString, aCursorPos ) )
                    bForget = TRUE;                 // Eingabe nicht uebernehmen
            }
        }
    }

    // check for input into DataPilot table

    if ( bModified && pActiveViewSh && !bForget )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
        ScDPObject* pDPObj = pDoc->GetDPAtCursor( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
        if ( pDPObj )
        {
            // any input within the DataPilot table is either a valid renaming
            // or an invalid action - normal cell input is always aborted

            pActiveViewSh->DataPilotInput( aCursorPos, aString );
            bForget = TRUE;
        }
    }

    pEngine->CompleteOnlineSpelling();
    BOOL bSpellErrors = !bFormulaMode && pEngine->HasOnlineSpellErrors();
    if ( bSpellErrors )
    {
        //  #i3820# If the spell checker flags numerical input as error,
        //  it still has to be treated as number, not EditEngine object.

        if ( pLastPattern && pActiveViewSh )
        {
            ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
            // without conditional format, as in ScColumn::SetString
            ULONG nFormat = pLastPattern->GetNumberFormat( pFormatter );
            double nVal;
            if ( pFormatter->IsNumberFormat( aString, nFormat, nVal ) )
            {
                bSpellErrors = FALSE;       // ignore the spelling errors
            }
        }
    }

    //  After RemoveAdjust, the EditView must not be repainted (has wrong font size etc).
    //  SetUpdateMode must come after CompleteOnlineSpelling.
    //  The view is hidden in any case below (Broadcast).
    pEngine->SetUpdateMode( FALSE );

    if ( bModified && !bForget )            // was wird eingeben (Text/Objekt) ?
    {
        USHORT nParCnt = pEngine->GetParagraphCount();
        if ( nParCnt == 0 )
            nParCnt = 1;
        ESelection aSel( 0, 0, nParCnt-1, pEngine->GetTextLen(nParCnt-1) );
        SfxItemSet aOldAttribs = pEngine->GetAttribs( aSel );
        const SfxPoolItem* pItem = NULL;

        //  find common (cell) attributes before RemoveAdjust

        if ( pActiveViewSh )
        {
            SfxItemSet* pCommonAttrs = NULL;
            for (USHORT nId = EE_CHAR_START; nId <= EE_CHAR_END; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, FALSE, &pItem );
                if ( eState == SFX_ITEM_SET &&
                        nId != EE_CHAR_ESCAPEMENT && nId != EE_CHAR_PAIRKERNING &&
                        nId != EE_CHAR_KERNING && nId != EE_CHAR_XMLATTRIBS &&
                            *pItem != pEditDefaults->Get(nId) )
                {
                    if ( !pCommonAttrs )
                        pCommonAttrs = new SfxItemSet( pEngine->GetEmptyItemSet() );
                    pCommonAttrs->Put( *pItem );
                }
            }

            if ( pCommonAttrs )
            {
                ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
                pCellAttrs = new ScPatternAttr( pDoc->GetPool() );
                pCellAttrs->GetFromEditItemSet( pCommonAttrs );
                delete pCommonAttrs;
            }
        }

        //  clear ParaAttribs (including adjustment)

        RemoveAdjust();

        //  check if EditObject is needed

        if ( bSpellErrors || nParCnt > 1 )
            bAttrib = TRUE;
        else
        {
            for (USHORT nId = EE_CHAR_START; nId <= EE_CHAR_END && !bAttrib; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, FALSE, &pItem );
                if (eState == SFX_ITEM_DONTCARE)
                    bAttrib = TRUE;
                else if (eState == SFX_ITEM_SET)
                {
                    //  keep same items in EditEngine as in ScEditAttrTester
                    if ( nId == EE_CHAR_ESCAPEMENT || nId == EE_CHAR_PAIRKERNING ||
                         nId == EE_CHAR_KERNING || nId == EE_CHAR_XMLATTRIBS )
                    {
                        if ( *pItem != pEditDefaults->Get(nId) )
                            bAttrib = TRUE;
                    }
                }
            }

            //  Feldbefehle enthalten?

            SfxItemState eFieldState = aOldAttribs.GetItemState( EE_FEATURE_FIELD, FALSE );
            if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
                bAttrib = TRUE;

            //  not converted characters?

            SfxItemState eConvState = aOldAttribs.GetItemState( EE_FEATURE_NOTCONV, FALSE );
            if ( eConvState == SFX_ITEM_DONTCARE || eConvState == SFX_ITEM_SET )
                bAttrib = TRUE;

            //  Formeln immer als Formeln erkennen (#38309#)
            //  (der Test vorher ist trotzdem noetig wegen Zell-Attributen)

            if ( bAttrib && bFormulaMode )
                bAttrib = FALSE;
        }

        if (bMatrix)
            bAttrib = FALSE;

        if (bAttrib)
        {
            ULONG nCtrl = pEngine->GetControlWord();
            ULONG nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
            if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                pEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );
            pObject = pEngine->CreateTextObject();
        }
        else if (bAutoComplete)         // Gross-/Kleinschreibung anpassen
        {
            if (pColumnData)
                pColumnData->GetExactMatch( aString );

            //! effizienter in der Liste suchen (ScUserList, nur einmal ToUpper)

            USHORT nIndex;
            ScUserListData* pData = ScGlobal::GetUserList()->GetData(aString);
            if ( pData && pData->GetSubIndex( aString, nIndex ) )
                aString = pData->GetSubStr( nIndex );
        }
    }

    //  don't rely on ShowRefFrame switching the active view synchronously
    //  execute the function directly on the correct view's bindings instead
    //  pRefViewSh is reset in ShowRefFrame - get pointer before ShowRefFrame call
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();

        if (pExecuteSh)
            pExecuteSh->SetTabNo(aCursorPos.Tab());

        bFormulaMode = FALSE;
        pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(FALSE);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = NULL;          // auch ohne FormulaMode wegen Funktions-AP
    DeleteRangeFinder();
    ResetAutoPar();

    BOOL bOldMod = bModified;

    bModified = FALSE;
    bSelIsRef = FALSE;
    eMode     = SC_INPUT_NONE;
    StopInputWinEngine( TRUE );

    if (bOldMod && !bProtected && !bForget)
    {
        //  keine typographische Anfuehrungszeichen in Formeln

        if ( aString.GetChar(0) == '=' )
        {
            SvxAutoCorrect* pAuto = SvxAutoCorrCfg::Get()->GetAutoCorrect();
            if ( pAuto )
            {
                sal_Unicode cReplace = pAuto->GetStartDoubleQuote();
                if( !cReplace )
                    cReplace = ScGlobal::pLocaleData->getDoubleQuotationMarkStart().GetChar(0);
                if ( cReplace != '"' )
                    aString.SearchAndReplaceAll( cReplace, '"' );

                cReplace = pAuto->GetEndDoubleQuote();
                if( !cReplace )
                    cReplace = ScGlobal::pLocaleData->getDoubleQuotationMarkEnd().GetChar(0);
                if ( cReplace != '"' )
                    aString.SearchAndReplaceAll( cReplace, '"' );

                cReplace = pAuto->GetStartSingleQuote();
                if( !cReplace )
                    cReplace = ScGlobal::pLocaleData->getQuotationMarkStart().GetChar(0);
                if ( cReplace != '\'' )
                    aString.SearchAndReplaceAll( cReplace, '\'' );

                cReplace = pAuto->GetEndSingleQuote();
                if( !cReplace )
                    cReplace = ScGlobal::pLocaleData->getQuotationMarkEnd().GetChar(0);
                if ( cReplace != '\'' )
                    aString.SearchAndReplaceAll( cReplace, '\'' );
            }
        }

        pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW_NOPAINT ) );

        if ( pExecuteSh )
        {
            SfxBindings& rBindings = pExecuteSh->GetViewFrame()->GetBindings();

            USHORT nId = FID_INPUTLINE_ENTER;
            if ( nBlockMode == SC_ENTER_BLOCK )
                nId = FID_INPUTLINE_BLOCK;
            else if ( nBlockMode == SC_ENTER_MATRIX )
                nId = FID_INPUTLINE_MATRIX;

            ScInputStatusItem aItem( FID_INPUTLINE_STATUS,
                                     aCursorPos, aCursorPos, aCursorPos,
                                     aString, pObject );
            const SfxPoolItem* aArgs[2];
            aArgs[0] = &aItem;
            aArgs[1] = NULL;
            rBindings.Execute( nId, aArgs );
        }

        delete pLastState;      // pLastState enthaelt noch den alten Text
        pLastState = NULL;
    }
    else
        pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

    if ( bOldMod && pExecuteSh && pCellAttrs && !bForget )
    {
        //  mit Eingabe zusammenfassen ?
        pExecuteSh->ApplySelectionPattern( *pCellAttrs, TRUE, TRUE );
        pExecuteSh->AdjustBlockHeight();
    }

    delete pCellAttrs;
    delete pObject;

    HideTip();

    nFormSelStart = nFormSelEnd = 0;
    aFormText.Erase();

    bInOwnChange = FALSE;
    bInEnterHandler = FALSE;
}

void ScInputHandler::CancelHandler()
{
    bInOwnChange = TRUE;                // disable ModifyHdl (reset below)

    ImplCreateEditEngine();

    bModified = FALSE;

    //  don't rely on ShowRefFrame switching the active view synchronously
    //  execute the function directly on the correct view's bindings instead
    //  pRefViewSh is reset in ShowRefFrame - get pointer before ShowRefFrame call
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();
        if (pExecuteSh)
            pExecuteSh->SetTabNo(aCursorPos.Tab());
        bFormulaMode = FALSE;
        SFX_APP()->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(FALSE);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = NULL;          // auch ohne FormulaMode wegen Funktions-AP
    DeleteRangeFinder();
    ResetAutoPar();

    eMode = SC_INPUT_NONE;
    StopInputWinEngine( TRUE );
    if (pExecuteSh)
        pExecuteSh->StopEditShell();

    aCursorPos.Set(MAXCOL+1,0,0);       // Flag, dass ungueltig
    pEngine->SetText(String());

    if ( !pLastState && pExecuteSh )
        pExecuteSh->UpdateInputHandler( TRUE );     // Status neu holen
    else
        NotifyChange( pLastState, TRUE );

    nFormSelStart = nFormSelEnd = 0;
    aFormText.Erase();

    bInOwnChange = FALSE;
}

BOOL ScInputHandler::IsModalMode( SfxObjectShell* pDocSh )
{
    //  Referenzen auf unbenanntes Dokument gehen nicht

    return bFormulaMode && pRefViewSh
            && pRefViewSh->GetViewData()->GetDocument()->GetDocumentShell() != pDocSh
            && !pDocSh->HasName();
}

void ScInputHandler::AddRefEntry()
{
    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // z.B. FillMode

    DataChanging();                         // kann nicht neu sein

    RemoveSelection();
    if (pTableView)
        pTableView->InsertText( ';', FALSE );
    if (pTopView)
        pTopView->InsertText( ';', FALSE );

    DataChanged();
}

void ScInputHandler::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    HideTip();

    BOOL bOtherDoc = ( pRefViewSh &&
                        pRefViewSh->GetViewData()->GetDocument() != pDoc );
    if (bOtherDoc)
        if (!pDoc->GetDocumentShell()->HasName())
        {
            //  Referenzen auf unbenanntes Dokument gehen nicht
            //  (SetReference sollte dann auch nicht gerufen werden)

            return;
        }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // z.B. FillMode

    //  nie das "=" ueberschreiben!
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();
    if ( aSel.nStartPara == 0 && aSel.nStartPos == 0 )
        return;

    DataChanging();                         // kann nicht neu sein

            //  Selektion umdrehen, falls rueckwaerts (noetig ???)

    if (pTableView)
    {
        ESelection aSel = pTableView->GetSelection();
        if (aSel.nStartPos > aSel.nEndPos && aSel.nStartPara == aSel.nEndPara)
        {
            aSel.Adjust();
            pTableView->SetSelection(aSel);
        }
    }
    if (pTopView)
    {
        ESelection aSel = pTopView->GetSelection();
        if (aSel.nStartPos > aSel.nEndPos && aSel.nStartPara == aSel.nEndPara)
        {
            aSel.Adjust();
            pTopView->SetSelection(aSel);
        }
    }

    //  String aus Referenz erzeugen

    String aRefStr;
    if (bOtherDoc)
    {
        //  Referenz auf anderes Dokument

        DBG_ASSERT(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

        String aTmp;
        rRef.Format( aTmp, SCA_VALID|SCA_TAB_3D, pDoc );        // immer 3d

        SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
        String aFileName = pObjSh->GetMedium()->GetName();

        aRefStr = '\'';
        aRefStr += aFileName;
        aRefStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "'#" ));
        aRefStr += aTmp;
    }
    else
    {
        if ( ( rRef.aStart.Tab() != aCursorPos.Tab() ||
                rRef.aStart.Tab() != rRef.aEnd.Tab() ) && pDoc )
            rRef.Format( aRefStr, SCA_VALID|SCA_TAB_3D, pDoc );
        else
            rRef.Format( aRefStr, SCA_VALID );
    }

    if (pTableView || pTopView)
    {
        if (pTableView)
            pTableView->InsertText( aRefStr, TRUE );
        if (pTopView)
            pTopView->InsertText( aRefStr, TRUE );

        DataChanged();
    }

    bSelIsRef = TRUE;
}

void ScInputHandler::InsertFunction( const String& rFuncName, BOOL bAddPar )
{
    if ( eMode == SC_INPUT_NONE )
    {
        DBG_ERROR("InsertFunction, nicht im Eingabemodus");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // z.B. FillMode

    DataChanging();                         // kann nicht neu sein

    String aText = rFuncName;
    if (bAddPar)
        aText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));

    if (pTableView)
    {
        pTableView->InsertText( aText, FALSE );
        if (bAddPar)
        {
            ESelection aSel = pTableView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pTableView->SetSelection(aSel);
        }
    }
    if (pTopView)
    {
        pTopView->InsertText( aText, FALSE );
        if (bAddPar)
        {
            ESelection aSel = pTopView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pTopView->SetSelection(aSel);
        }
    }

    DataChanged();

    if (bAddPar)
        AutoParAdded();
}

void ScInputHandler::ClearText()
{
    if ( eMode == SC_INPUT_NONE )
    {
        DBG_ERROR("ClearText, nicht im Eingabemodus");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // z.B. FillMode

    DataChanging();                         // darf nicht neu sein

    String aEmpty;
    if (pTableView)
    {
        pTableView->GetEditEngine()->SetText( aEmpty );
        pTableView->SetSelection( ESelection(0,0, 0,0) );
    }
    if (pTopView)
    {
        pTopView->GetEditEngine()->SetText( aEmpty );
        pTopView->SetSelection( ESelection(0,0, 0,0) );
    }

    DataChanged();
}

BOOL ScInputHandler::KeyInput( const KeyEvent& rKEvt, BOOL bStartEdit /* = FALSE */ )
{
    if (!bOptLoaded)
    {
        bAutoComplete = SC_MOD()->GetAppOptions().GetAutoComplete();
        bOptLoaded = TRUE;
    }

    KeyCode aCode = rKEvt.GetKeyCode();
    USHORT nModi  = aCode.GetModifier();
    BOOL bShift   = aCode.IsShift();
    BOOL bControl = aCode.IsMod1();
    BOOL bAlt     = aCode.IsMod2();
    USHORT nCode  = aCode.GetCode();
    sal_Unicode nChar = rKEvt.GetCharCode();

    //  Alt-Return wird gebraucht, alle anderen Alt's und Ctrl-Tab nicht:
#ifndef MAC
    if (( bAlt && !bControl && nCode != KEY_RETURN ) ||
            ( bControl && aCode.GetCode() == KEY_TAB ))
        return FALSE;
#endif
    //  auf'm Mac nicht, sonst werden druckbare Zeichen unterdrueckt

    BOOL bInputLine = ( eMode==SC_INPUT_TOP );

    BOOL bUsed = FALSE;
    BOOL bSkip = FALSE;
    BOOL bDoEnter = FALSE;

    switch ( nCode )
    {
        case KEY_RETURN:
            if (bControl && !bShift && !bInputLine)
                bDoEnter = TRUE;
            else if ( nModi == 0 && nTipVisible && pFormulaData && nAutoPos != SCPOS_INVALID )
            {
                PasteFunctionData();
                bUsed = TRUE;
            }
            else if ( nModi == 0 && nTipVisible && aManualTip.Len() )
            {
                PasteManualTip();
                bUsed = TRUE;
            }
            else
            {
                BYTE nMode = SC_ENTER_NORMAL;
                if ( bShift && bControl )
                    nMode = SC_ENTER_MATRIX;
                else if ( bAlt )
                    nMode = SC_ENTER_BLOCK;
                EnterHandler( nMode );

                if (pActiveViewSh)
                    pActiveViewSh->MoveCursorEnter( bShift && !bControl );

                bUsed = TRUE;
            }
            break;
        case KEY_TAB:
            if (!bControl && !bAlt)
            {
                if ( pFormulaData && nTipVisible && nAutoPos != SCPOS_INVALID )
                {
                    //  blaettern

                    NextFormulaEntry( bShift );
                }
                else if ( pColumnData && bUseTab && nAutoPos != SCPOS_INVALID )
                {
                    //  in den Eintraegen der AutoEingabe blaettern

                    NextAutoEntry( bShift );
                }
                else
                {
                    EnterHandler();

                    //  TabKeyInput gibt auf manchen Rechnern unter W95 Stackueberlaeufe,
                    //  darum direkter Aufruf:
                    if (pActiveViewSh)
                        pActiveViewSh->FindNextUnprot( bShift );
                }
                bUsed = TRUE;
            }
            break;
        case KEY_ESCAPE:
            if ( nTipVisible )
            {
                HideTip();
                bUsed = TRUE;
            }
            else if (eMode != SC_INPUT_NONE)
            {
                CancelHandler();
                bUsed = TRUE;
            }
            else
                bSkip = TRUE;
            break;
        case KEY_F2:
            if ( !bShift && !bControl && !bAlt && eMode == SC_INPUT_TABLE )
            {
                eMode = SC_INPUT_TYPE;
                bUsed = TRUE;
            }
            break;
    }

    //  Cursortasten nur ausfuehren, wenn schon im Edit-Modus
    //  z.B. wegen Shift-Ctrl-PageDn (ist nicht als Accelerator definiert)

    BOOL bCursorKey = EditEngine::DoesKeyMoveCursor(rKEvt);
    BOOL bInsKey = ( nCode == KEY_INSERT && !nModi );   // Insert wie Cursortasten behandeln
    if ( !bUsed && !bSkip && ( bDoEnter || EditEngine::DoesKeyChangeText(rKEvt) ||
                    ( eMode != SC_INPUT_NONE && ( bCursorKey || bInsKey ) ) ) )
    {
        HideTip();

        if (bSelIsRef)
        {
            RemoveSelection();
            bSelIsRef = FALSE;
        }

        UpdateActiveView();
        BOOL bNewView = DataChanging( nChar );

        if (bProtected)                             // Zelle geschuetzt?
            bUsed = TRUE;                           // Key-Event nicht weiterleiten
        else                                        // Aenderungen erlaubt
        {
            if (bNewView )                          // neu anlegen
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
                UpdateActiveView();
                if (eMode==SC_INPUT_NONE)
                    if (pTableView || pTopView)
                    {
                        String aStrLoP;

                        if ( bStartEdit && bCellHasPercentFormat && ((nChar >= '0' && nChar <= '9') || nChar == '-') )
                            aStrLoP = '%';

                        if (pTableView)
                        {
                            pTableView->GetEditEngine()->SetText( aStrLoP );
                            if ( aStrLoP.Len() )
                                pTableView->SetSelection( ESelection(0,0, 0,0) );   // before the '%'

                            // don't call SetSelection if the string is empty anyway,
                            // to avoid breaking the bInitial handling in ScViewData::EditGrowY
                        }
                        if (pTopView)
                        {
                            pTopView->GetEditEngine()->SetText( aStrLoP );
                            if ( aStrLoP.Len() )
                                pTopView->SetSelection( ESelection(0,0, 0,0) );     // before the '%'
                        }
                    }
                SyncViews();
            }

            if (pTableView || pTopView)
            {
//              pActiveView->SetEditEngineUpdateMode(TRUE);         //! gibt Muell !!!!

                if (bDoEnter)
                {
                    if (pTableView)
                        if( pTableView->PostKeyEvent( KeyEvent( CHAR_CR, KeyCode(KEY_RETURN) ) ) )
                            bUsed = TRUE;
                    if (pTopView)
                        if( pTopView->PostKeyEvent( KeyEvent( CHAR_CR, KeyCode(KEY_RETURN) ) ) )
                            bUsed = TRUE;
                }
                else if ( nAutoPar && nChar == ')' && CursorAtClosingPar() )
                {
                    SkipClosingPar();
                    bUsed = TRUE;
                }
                else
                {
                    if (pTableView)
                        if ( pTableView->PostKeyEvent( rKEvt ) )
                            bUsed = TRUE;
                    if (pTopView)
                        if ( pTopView->PostKeyEvent( rKEvt ) )
                            bUsed = TRUE;
                }

                //  Auto-Eingabe:

                if ( bUsed && bAutoComplete )
                {
                    bUseTab = FALSE;
                    nAutoPos = SCPOS_INVALID;                       // nicht weitersuchen
                    if ( nChar && nChar != 8 && nChar != 127 )      // nicht BS und Delete
                    {
                        if (bFormulaMode)
                            UseFormulaData();
                        else
                            UseColData();
                    }
                }

                //  when the selection is changed manually or an opening parenthesis
                //  is typed, stop overwriting parentheses
                if ( bUsed && ( nChar == '(' || bCursorKey ) )
                    ResetAutoPar();

                if ( KEY_INSERT == nCode )
                {
                    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                    if (pViewFrm)
                        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
                }
            }

            DataChanged();              //  ruft auch UpdateParenthesis()
            InvalidateAttribs();        //! in DataChanged ?
        }
    }

    if (pTopView && eMode != SC_INPUT_NONE)
        SyncViews();

    return bUsed;
}

BOOL ScInputHandler::InputCommand( const CommandEvent& rCEvt, BOOL bForce )
{
    BOOL bUsed = FALSE;

    if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        //  #90346# for COMMAND_CURSORPOS, do as little as possible, because
        //  with remote VCL, even a ShowCursor will generate another event.
        if ( eMode != SC_INPUT_NONE )
        {
            UpdateActiveView();
            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                else if (pTopView)                      // call only once
                    pTopView->Command( rCEvt );
                bUsed = TRUE;
            }
        }
    }
    else
    {
        if ( bForce || eMode != SC_INPUT_NONE )
        {
            if (!bOptLoaded)
            {
                bAutoComplete = SC_MOD()->GetAppOptions().GetAutoComplete();
                bOptLoaded = TRUE;
            }

            HideTip();

            if ( bSelIsRef )
            {
                RemoveSelection();
                bSelIsRef = FALSE;
            }

            UpdateActiveView();
            BOOL bNewView = DataChanging( 0, TRUE );

            if (bProtected)                             // cell protected
                bUsed = TRUE;                           // event is used
            else                                        // changes allowed
            {
                if (bNewView)                           // create new edit view
                {
                    if (pActiveViewSh)
                        pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
                    UpdateActiveView();
                    if (eMode==SC_INPUT_NONE)
                        if (pTableView || pTopView)
                        {
                            String aStrLoP;
                            if (pTableView)
                            {
                                pTableView->GetEditEngine()->SetText( aStrLoP );
                                pTableView->SetSelection( ESelection(0,0, 0,0) );
                            }
                            if (pTopView)
                            {
                                pTopView->GetEditEngine()->SetText( aStrLoP );
                                pTopView->SetSelection( ESelection(0,0, 0,0) );
                            }
                        }
                    SyncViews();
                }

                if (pTableView || pTopView)
                {
                    if (pTableView)
                        pTableView->Command( rCEvt );
                    if (pTopView)
                        pTopView->Command( rCEvt );

                    bUsed = TRUE;

                    if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
                    {
                        //  AutoInput after ext text input

                        nAutoPos = SCPOS_INVALID;
                        if (bFormulaMode)
                            UseFormulaData();
                        else
                            UseColData();
                    }
                }

                DataChanged();              //  calls UpdateParenthesis()
                InvalidateAttribs();        //! in DataChanged ?
            }
        }

        if (pTopView && eMode != SC_INPUT_NONE)
            SyncViews();
    }

    return bUsed;
}

void ScInputHandler::NotifyChange( const ScInputHdlState* pState,
                                   BOOL bForce, ScTabViewShell* pSourceSh,
                                   BOOL bStopEditing)
{
    //  #62806# Wenn der Aufruf aus einem Makro-Aufruf im EnterHandler kommt,
    //  gleich abbrechen und nicht den Status durcheinander bringen
    if (bInEnterHandler)
        return;

    BOOL bRepeat = (pState == pLastState);
    if (!bRepeat && pState && pLastState)
        bRepeat = (*pState == *pLastState);
    if (bRepeat && !bForce)
        return;

    bInOwnChange = TRUE;                // disable ModifyHdl (reset below)

    if ( pState && !pLastState )        // wieder enablen
        bForce = TRUE;

    BOOL bHadObject = pLastState && pLastState->GetEditData();

    //! Before EditEngine gets eventually created (so it gets the right pools)
    if ( pSourceSh )
        pActiveViewSh = pSourceSh;
    else
        pActiveViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());

    ImplCreateEditEngine();

    if ( pState != pLastState )
    {
        delete pLastState;
        pLastState = pState ? new ScInputHdlState( *pState ) : NULL;
    }

    if ( pState && pActiveViewSh )
    {
        ScModule* pScMod = SC_MOD();

        if ( pState )
        {
            BOOL bIgnore = FALSE;

            //  hier auch fremde Referenzeingabe beruecksichtigen (z.B. Funktions-AP),
            //  FormEditData falls gerade von der Hilfe auf Calc umgeschaltet wird:

            if ( !bFormulaMode && !pScMod->IsFormulaMode() && !pScMod->GetFormEditData() )
            {
                if ( bModified )
                {
                    if (pState->GetPos() != aCursorPos)
                    {
                        if (!bProtected)
                            EnterHandler();
                    }
                    else
                        bIgnore = TRUE;
                }

                if ( !bIgnore /* || bRepeat */ )
                {
                    const ScAddress&        rSPos   = pState->GetStartPos();
                    const ScAddress&        rEPos   = pState->GetEndPos();
                    const EditTextObject*   pData   = pState->GetEditData();
                    String                  aString = pState->GetString();
                    BOOL                    bTxtMod = FALSE;

                    aCursorPos  = pState->GetPos();

                    if ( pData /* || bRepeat */ )
                        bTxtMod = TRUE;
                    else if ( bHadObject )
                        bTxtMod = TRUE;
                    else if ( bTextValid )
                        bTxtMod = ( aString != aCurrentText );
                    else
                        bTxtMod = ( aString != GetEditText(pEngine) );

                    if ( bTxtMod || bForce )
                    {
                        if (pData)
                        {
                            pEngine->SetText( *pData );
                            aString = GetEditText(pEngine);
                            lcl_RemoveTabs(aString);
                            bTextValid = FALSE;
                            aCurrentText.Erase();
                        }
                        else
                        {
                            aCurrentText = aString;
                            bTextValid = TRUE;              //! erst nur als String merken
                        }

                        if ( pInputWin )
                            pInputWin->SetTextString(aString);
                    }

                    if ( pInputWin )                        // Bereichsanzeige
                    {
                        String aPosStr;

                        //  Ist der Bereich ein Name?
                        //! per Timer suchen ???

                        if ( pActiveViewSh )
                            pActiveViewSh->GetViewData()->GetDocument()->
                                GetRangeAtBlock( ScRange( rSPos, rEPos ), &aPosStr );

                        if ( !aPosStr.Len() )           // kein Name -> formatieren
                        {
                            if ( rSPos != rEPos )
                            {
                                String aStr;
                                rSPos.Format( aPosStr, SCA_VALID );
                                aPosStr += ':';
                                rEPos.Format( aStr, SCA_VALID );
                                aPosStr += aStr;
                            }
                            else
                                aCursorPos.Format( aPosStr, SCA_VALID );
                        }

                        pInputWin->SetPosString(aPosStr);
                        pInputWin->SetSumAssignMode();
                    }

                    if (bStopEditing)
                        SFX_APP()->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

                    //  As long as the content is not edited, turn off online spelling.
                    //  Online spelling is turned back on in StartTable, after setting
                    //  the right language from cell attributes.

                    ULONG nCntrl = pEngine->GetControlWord();
                    if ( nCntrl & EE_CNTRL_ONLINESPELLING )
                        pEngine->SetControlWord( nCntrl & ~EE_CNTRL_ONLINESPELLING );

                    bModified = FALSE;
                    bSelIsRef = FALSE;
                    bProtected = FALSE;
                    bCommandErrorShown = FALSE;
                }
            }
        }

//      bProtected = FALSE;

        if ( pInputWin)
        {
            if(!pScMod->IsFormulaMode()&& !pScMod->IsRefDialogOpen())   //BugID 54702
            {                                                           //Wenn RefDialog offen, dann nicht enablen
                if ( !pInputWin->IsEnabled())
                {
                    pInputWin->Enable();
                    if(pDelayTimer )
                    {
                        DELETEZ( pDelayTimer );
                    }
                }
            }
            else if(pScMod->IsRefDialogOpen())
            {                                   // Da jedes Dokument eigenes InputWin hat, sollte
                if ( !pDelayTimer )             // nochmals Timer gestartet werden, da sonst Ein-
                {                               // gabezeile evt. noch aktiv ist.
                    pDelayTimer = new Timer;
                    pDelayTimer->SetTimeout( 500 ); // 100ms Verzoegerung
                    pDelayTimer->SetTimeoutHdl( LINK( this, ScInputHandler, DelayTimer ) );
                    pDelayTimer->Start();
                }
            }
        }
    }
    else // !pState || !pActiveViewSh
    {
        if ( !pDelayTimer )
        {
            pDelayTimer = new Timer;
            pDelayTimer->SetTimeout( 500 ); // 100ms Verzoegerung
            pDelayTimer->SetTimeoutHdl( LINK( this, ScInputHandler, DelayTimer ) );
            pDelayTimer->Start();
        }
    }

    HideTip();
    bInOwnChange = FALSE;
}

void ScInputHandler::ResetDelayTimer()
{
    if(pDelayTimer!=NULL)
    {
        DELETEZ( pDelayTimer );

        if ( pInputWin)
        {
            pInputWin->Enable();
        }
    }
}

IMPL_LINK( ScInputHandler, DelayTimer, Timer*, pTimer )
{
    if ( pTimer == pDelayTimer )
    {
        DELETEZ( pDelayTimer );

        if ( NULL == pLastState || SC_MOD()->IsFormulaMode() || SC_MOD()->IsRefDialogOpen())
        {
            //! new method at ScModule to query if function autopilot is open

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if ( pViewFrm && pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
            {
                if ( pInputWin)
                {
                    pInputWin->EnableButtons( FALSE );
                    pInputWin->Disable();
                }
            }
            else if ( !bFormulaMode )   // #39210# Formel auch z.B. bei Hilfe behalten
            {
                bInOwnChange = TRUE;    // disable ModifyHdl (reset below)

                pActiveViewSh = NULL;
                pEngine->SetText( EMPTY_STRING );
                if ( pInputWin )
                {
                    pInputWin->SetPosString( EMPTY_STRING );
                    pInputWin->SetTextString( EMPTY_STRING );
                    pInputWin->Disable();
                }

                bInOwnChange = FALSE;
            }
        }
    }
    return 0;
}

void ScInputHandler::InputSelection( EditView* pView )
{
    SyncViews( pView );

    UpdateParenthesis();    //  Selektion geaendert -> Klammer-Hervorhebung neu

    //  when the selection is changed manually, stop overwriting parentheses
    ResetAutoPar();
}

void ScInputHandler::InputChanged( EditView* pView, BOOL bFromNotify )
{
    ESelection aSelection = pView->GetSelection();

    UpdateActiveView();

    // #i20282# DataChanged needs to know if this is from the input line's modify handler
    BOOL bFromTopNotify = ( bFromNotify && pView == pTopView );

    BOOL bNewView = DataChanging();                     //! kann das hier ueberhaupt sein?
    aCurrentText = pView->GetEditEngine()->GetText();   // auch den String merken
    pEngine->SetText( aCurrentText );
    DataChanged( bFromTopNotify );
    bTextValid = TRUE;      // wird in DataChanged auf FALSE gesetzt

    if ( pActiveViewSh )
    {
        ScViewData* pViewData = pActiveViewSh->GetViewData();
        if ( bNewView )
            pViewData->GetDocShell()->PostEditView( pEngine, aCursorPos );

        pViewData->EditGrowY();
        pViewData->EditGrowX();
    }

    SyncViews( pView );
}

const String& ScInputHandler::GetEditString()
{
    if (pEngine)
    {
        aCurrentText = pEngine->GetText();      // immer neu aus Engine
        bTextValid = TRUE;
    }

    return aCurrentText;
}

Size ScInputHandler::GetTextSize()
{
    Size aSize;
    if ( pEngine )
        aSize = Size( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );

    return aSize;
}

BOOL ScInputHandler::GetTextAndFields( ScEditEngineDefaulter& rDestEngine )
{
    BOOL bRet = FALSE;
    if (pEngine)
    {
        //  Feldbefehle enthalten?

        USHORT nParCnt = pEngine->GetParagraphCount();
        SfxItemSet aSet = pEngine->GetAttribs( ESelection(0,0,nParCnt,0) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, FALSE );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
        {
            //  Inhalt kopieren

            EditTextObject* pObj = pEngine->CreateTextObject();
            rDestEngine.SetText(*pObj);
            delete pObj;

            //  Attribute loeschen

            for (USHORT i=0; i<nParCnt; i++)
                rDestEngine.QuickRemoveCharAttribs( i );

            //  Absaetze zusammenfassen

            while ( nParCnt > 1 )
            {
                xub_StrLen nLen = rDestEngine.GetTextLen( (USHORT)0 );
                ESelection aSel( 0,nLen, 1,0 );
                rDestEngine.QuickInsertText( ' ', aSel );       // Umbruch durch Space ersetzen
                --nParCnt;
            }

            bRet = TRUE;
        }
    }
    return bRet;
}


//------------------------------------------------------------------------
// Methoden fuer FunktionsAutopiloten:
// InputGetSelection, InputSetSelection, InputReplaceSelection, InputGetFormulaStr
//------------------------------------------------------------------------

void ScInputHandler::InputGetSelection( xub_StrLen& rStart, xub_StrLen& rEnd )
{
    rStart = nFormSelStart;
    rEnd = nFormSelEnd;
}

//------------------------------------------------------------------------

EditView* ScInputHandler::GetFuncEditView()
{
    UpdateActiveView();     // wegen pTableView

    EditView* pView = NULL;
    if ( pInputWin )
    {
        pInputWin->MakeDialogEditView();
        pView = pInputWin->GetEditView();
    }
    else
    {
        if ( eMode != SC_INPUT_TABLE )
        {
            bCreatingFuncView = TRUE;       // RangeFinder nicht anzeigen
            SetMode( SC_INPUT_TABLE );
            bCreatingFuncView = FALSE;
            if ( pTableView )
                pTableView->GetEditEngine()->SetText( EMPTY_STRING );
        }
        pView = pTableView;
    }

    return pView;
}

//------------------------------------------------------------------------

void ScInputHandler::InputSetSelection( xub_StrLen nStart, xub_StrLen nEnd )
{
    if ( nStart <= nEnd )
    {
        nFormSelStart = nStart;
        nFormSelEnd = nEnd;
    }
    else
    {
        nFormSelEnd = nStart;
        nFormSelStart = nEnd;
    }

    EditView* pView = GetFuncEditView();
    if (pView)
        pView->SetSelection( ESelection(0,nStart, 0,nEnd) );

    bModified = TRUE;
}

//------------------------------------------------------------------------

void ScInputHandler::InputReplaceSelection( const String& rStr )
{
    if (!pRefViewSh)
        pRefViewSh = pActiveViewSh;

    DBG_ASSERT(nFormSelEnd>=nFormSelStart,"Selektion kaputt...");

    xub_StrLen nOldLen = nFormSelEnd-nFormSelStart;
    xub_StrLen nNewLen = rStr.Len();
    if (nOldLen)
        aFormText.Erase( nFormSelStart, nOldLen );
    if (nNewLen)
        aFormText.Insert( rStr, nFormSelStart );
    nFormSelEnd = nFormSelStart + nNewLen;

    EditView* pView = GetFuncEditView();
    if (pView)
    {
        pView->SetEditEngineUpdateMode( FALSE );
//      pView->InsertText( rStr, TRUE );
        pView->GetEditEngine()->SetText( aFormText );
        pView->SetSelection( ESelection(0,nFormSelStart, 0,nFormSelEnd) );
        pView->SetEditEngineUpdateMode( TRUE );
    }
    bModified = TRUE;
}

//------------------------------------------------------------------------

String ScInputHandler::InputGetFormulaStr()
{
    return aFormText;   //! eigene Membervariable?
}

//========================================================================
//  ScInputHdlState
//========================================================================

ScInputHdlState::ScInputHdlState( const ScAddress& rCurPos,
                                  const ScAddress& rStartPos,
                                  const ScAddress& rEndPos,
                                  const String& rString,
                                  const EditTextObject* pData )
    :   aCursorPos  ( rCurPos ),
        aStartPos   ( rStartPos ),
        aEndPos     ( rEndPos ),
        aString     ( rString ),
        pEditData   ( pData ? pData->Clone() : NULL )
{
}

//------------------------------------------------------------------------

ScInputHdlState::ScInputHdlState( const ScInputHdlState& rCpy )
    :   pEditData   ( NULL )
{
    *this = rCpy;
}

//------------------------------------------------------------------------

ScInputHdlState::~ScInputHdlState()
{
    delete pEditData;
}

//------------------------------------------------------------------------

int ScInputHdlState::operator==( const ScInputHdlState& r ) const
{
    return (    (aStartPos  == r.aStartPos)
             && (aEndPos    == r.aEndPos)
             && (aCursorPos == r.aCursorPos)
             && (aString    == r.aString)
             && ScGlobal::EETextObjEqual( pEditData, r.pEditData ) );
}

//------------------------------------------------------------------------

ScInputHdlState& ScInputHdlState::operator=( const ScInputHdlState& r )
{
    delete pEditData;

    aCursorPos  = r.aCursorPos;
    aStartPos   = r.aStartPos;
    aEndPos     = r.aEndPos;
    aString     = r.aString;
    pEditData   = r.pEditData ? r.pEditData->Clone() : NULL;

    return *this;
}




