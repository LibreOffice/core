/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "inputhdl.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/acorrcfg.hxx>
#include <svx/algitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include "editeng/misspellrange.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svl/zforlist.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/help.hxx>
#include <vcl/cursor.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/string.hxx>
#include <formula/formulahelper.hxx>

#include "inputwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
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
#include "appoptio.hxx"
#include "docoptio.hxx"
#include "validat.hxx"
#include "userlist.hxx"
#include "rfindlst.hxx"
#include "inputopt.hxx"
#include "simpleformulacalc.hxx"
#include "compiler.hxx"
#include "editable.hxx"
#include "funcdesc.hxx"
#include "markdata.hxx"
#include "tokenarray.hxx"


#define RANGEFIND_MAX   32

using namespace formula;



bool ScInputHandler::bOptLoaded = false;            
bool ScInputHandler::bAutoComplete = false;         

extern sal_uInt16 nEditAdjust;      

namespace {




const sal_Char pMinDelimiters[] = " !\"";

sal_Unicode lcl_getSheetSeparator(ScDocument* pDoc)
{
    ScCompiler aComp(pDoc, ScAddress());
    aComp.SetGrammar(pDoc->GetGrammar());
    return aComp.GetNativeAddressSymbol(ScCompiler::Convention::SHEET_SEPARATOR);
}

ScTypedCaseStrSet::const_iterator findText(
    const ScTypedCaseStrSet& rDataSet, ScTypedCaseStrSet::const_iterator itPos,
    const OUString& rStart, OUString& rResult, bool bBack)
{
    if (bBack)                                    
    {
        ScTypedCaseStrSet::const_reverse_iterator it = rDataSet.rbegin(), itEnd = rDataSet.rend();
        if (itPos != rDataSet.end())
        {
            size_t nPos = std::distance(rDataSet.begin(), itPos);
            size_t nRPos = rDataSet.size() - 1 - nPos;
            std::advance(it, nRPos);
            ++it;
        }

        for (; it != itEnd; ++it)
        {
            const ScTypedStrData& rData = *it;
            if (rData.GetStringType() == ScTypedStrData::Value)
                
                continue;

            if (!ScGlobal::GetpTransliteration()->isMatch(rStart, rData.GetString()))
                
                continue;

            rResult = rData.GetString();
            return (++it).base(); 
        }
    }
    else                                            
    {
        ScTypedCaseStrSet::const_iterator it = rDataSet.begin(), itEnd = rDataSet.end();
        if (itPos != rDataSet.end())
        {
            it = itPos;
            ++it;
        }

        for (; it != itEnd; ++it)
        {
            const ScTypedStrData& rData = *it;
            if (rData.GetStringType() == ScTypedStrData::Value)
                
                continue;

            if (!ScGlobal::GetpTransliteration()->isMatch(rStart, rData.GetString()))
                
                continue;

            rResult = rData.GetString();
            return it;
        }
    }

    return rDataSet.end(); 
}

OUString getExactMatch(const ScTypedCaseStrSet& rDataSet, const OUString& rString)
{
    ScTypedCaseStrSet::const_iterator it = rDataSet.begin(), itEnd = rDataSet.end();
    for (; it != itEnd; ++it)
    {
        const ScTypedStrData& rData = *it;
        if (rData.GetStringType() == ScTypedStrData::Value)
            continue;

        if (!ScGlobal::GetpTransliteration()->isEqual(rData.GetString(), rString))
            continue;

        return rData.GetString();
    }
    return rString;
}

void removeChars(OUString& rStr, sal_Unicode c)
{
    OUStringBuffer aBuf(rStr);
    for (sal_Int32 i = 0, n = aBuf.getLength(); i < n; ++i)
    {
        if (aBuf[i] == c)
            aBuf[i] = ' ';
    }
    rStr = aBuf.makeStringAndClear();
}

}

void ScInputHandler::InitRangeFinder( const OUString& rFormula )
{
    DeleteRangeFinder();
    ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    const sal_Unicode cSheetSep = lcl_getSheetSeparator(pDoc);

    if ( !pActiveViewSh || !SC_MOD()->GetInputOptions().GetRangeFinder() )
        return;

    OUString aDelimiters = ScEditUtil::ModifyDelimiters(
                            OUString::createFromAscii( pMinDelimiters ) );

    sal_Int32 nColon = aDelimiters.indexOf( ':' );
    if ( nColon != -1 )
        aDelimiters = aDelimiters.replaceAt( nColon, 1, "");             
    sal_Int32 nDot = aDelimiters.indexOf(cSheetSep);
    if ( nDot != -1 )
        aDelimiters = aDelimiters.replaceAt( nDot, 1 , "");               

    const sal_Unicode* pChar = rFormula.getStr();
    sal_Int32 nLen = rFormula.getLength();
    sal_Int32 nPos = 0;
    sal_Int32 nStart = 0;
    sal_uInt16 nCount = 0;
    ScRange aRange;
    while ( nPos < nLen && nCount < RANGEFIND_MAX )
    {
        
        while ( nPos<nLen && ScGlobal::UnicodeStrChr( aDelimiters.getStr(), pChar[nPos] ) )
        {
            if ( pChar[nPos] == '"' )                       
            {
                ++nPos;
                while (nPos<nLen && pChar[nPos] != '"')     
                    ++nPos;
            }
            ++nPos;                     
        }

        
        nStart = nPos;
handle_r1c1:
        while ( nPos<nLen && !ScGlobal::UnicodeStrChr( aDelimiters.getStr(), pChar[nPos] ) )
            ++nPos;

        
        
        
        if( nPos < nLen && nPos > 0 &&
            '-' == pChar[nPos] && '[' == pChar[nPos-1] &&
            formula::FormulaGrammar::CONV_XL_R1C1 == pDoc->GetAddressConvention() )
        {
            nPos++;
            goto handle_r1c1;
        }

        if ( nPos > nStart )
        {
            OUString aTest = rFormula.copy( nStart, nPos-nStart );
            const ScAddress::Details aAddrDetails( pDoc, aCursorPos );
            sal_uInt16 nFlags = aRange.ParseAny( aTest, pDoc, aAddrDetails );
            if ( nFlags & SCA_VALID )
            {
                
                if ( (nFlags & SCA_TAB_3D) == 0 )
                    aRange.aStart.SetTab( pActiveViewSh->GetViewData()->GetTabNo() );
                if ( (nFlags & SCA_TAB2_3D) == 0 )
                    aRange.aEnd.SetTab( aRange.aStart.Tab() );

                if ( ( nFlags & ( SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2 ) ) == 0 )
                {
                    
                    
                    sal_uInt16 nAbsFlags = nFlags & ( SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE );
                    nFlags |= nAbsFlags << 4;
                }

                if (!nCount)
                {
                    pEngine->SetUpdateMode( false );
                    pRangeFindList = new ScRangeFindList( pDocSh->GetTitle() );
                }

                ColorData nColorData = pRangeFindList->Insert( ScRangeFindData( aRange, nFlags, nStart, nPos ) );

                ESelection aSel( 0, nStart, 0, nPos );
                SfxItemSet aSet( pEngine->GetEmptyItemSet() );
                aSet.Put( SvxColorItem( Color( nColorData ),
                            EE_CHAR_COLOR ) );
                pEngine->QuickSetAttribs( aSet, aSel );
                ++nCount;
            }
        }

        
    }

    if (nCount)
    {
        pEngine->SetUpdateMode( true );

        pDocSh->Broadcast( SfxSimpleHint( SC_HINT_SHOWRANGEFINDER ) );
    }
}

void ScInputHandler::SetDocumentDisposing( bool b )
{
    mbDocumentDisposing = b;
}

static void lcl_Replace( EditView* pView, const OUString& rNewStr, const ESelection& rOldSel )
{
    if ( pView )
    {
        ESelection aOldSel = pView->GetSelection();
        if (aOldSel.HasRange())
            pView->SetSelection( ESelection( aOldSel.nEndPara, aOldSel.nEndPos,
                                             aOldSel.nEndPara, aOldSel.nEndPos ) );

        EditEngine* pEngine = pView->GetEditEngine();
        pEngine->QuickInsertText( rNewStr, rOldSel );

        
        
        pView->InsertText( EMPTY_OUSTRING, false );

        sal_Int32 nLen = pEngine->GetTextLen(0);
        ESelection aSel( 0, nLen, 0, nLen );
        pView->SetSelection( aSel );                
    }
}

void ScInputHandler::UpdateRange( sal_uInt16 nIndex, const ScRange& rNew )
{
    ScTabViewShell* pDocView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pDocView && pRangeFindList && nIndex < pRangeFindList->Count() )
    {
        ScRangeFindData* pData = pRangeFindList->GetObject( nIndex );
        sal_Int32 nOldStart = pData->nSelStart;
        sal_Int32 nOldEnd = pData->nSelEnd;

        ScRange aJustified = rNew;
        aJustified.Justify();           
        ScDocument* pDoc = pDocView->GetViewData()->GetDocument();
        const ScAddress::Details aAddrDetails( pDoc, aCursorPos );
        OUString aNewStr(aJustified.Format(pData->nFlags, pDoc, aAddrDetails));
        ESelection aOldSel( 0, nOldStart, 0, nOldEnd );

        DataChanging();

        lcl_Replace( pTopView, aNewStr, aOldSel );
        lcl_Replace( pTableView, aNewStr, aOldSel );

        bInRangeUpdate = true;
        DataChanged();
        bInRangeUpdate = false;

        long nDiff = aNewStr.getLength() - (long)(nOldEnd-nOldStart);

        pData->aRef = rNew;
        pData->nSelEnd = pData->nSelEnd + nDiff;

        sal_uInt16 nCount = (sal_uInt16) pRangeFindList->Count();
        for (sal_uInt16 i=nIndex+1; i<nCount; i++)
        {
            ScRangeFindData* pNext = pRangeFindList->GetObject( i );
            pNext->nSelStart = pNext->nSelStart + nDiff;
            pNext->nSelEnd   = pNext->nSelEnd   + nDiff;
        }
    }
    else
    {
        OSL_FAIL("UpdateRange: da fehlt was");
    }
}

void ScInputHandler::DeleteRangeFinder()
{
    ScTabViewShell* pPaintView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pRangeFindList && pPaintView )
    {
        ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
        pRangeFindList->SetHidden(true);
        pDocSh->Broadcast( SfxSimpleHint( SC_HINT_SHOWRANGEFINDER ) );  
        DELETEZ(pRangeFindList);
    }
}



inline OUString GetEditText(EditEngine* pEng)
{
    return ScEditUtil::GetSpaceDelimitedString(*pEng);
}

static void lcl_RemoveTabs(OUString& rStr)
{
    removeChars(rStr, '\t');
}

static void lcl_RemoveLineEnd(OUString& rStr)
{
    rStr = convertLineEnd(rStr, LINEEND_LF);
    removeChars(rStr, '\n');
}

static sal_Int32 lcl_MatchParenthesis( const OUString& rStr, sal_Int32 nPos )
{
    int nDir;
    sal_Unicode c1, c2 = 0;
    c1 = rStr[nPos];
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
        return -1;
    sal_Int32 nLen = rStr.getLength();
    const sal_Unicode* p0 = rStr.getStr();
    const sal_Unicode* p;
    const sal_Unicode* p1;
    sal_uInt16 nQuotes = 0;
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
    
    bool bLookInString = ((nQuotes % 2) != 0);
    bool bInString = bLookInString;
    p = p0 + nPos;
    p1 = (nDir < 0 ? p0 : p0 + nLen) ;
    sal_uInt16 nLevel = 1;
    while ( p != p1 && nLevel )
    {
        p += nDir;
        if ( *p == '\"' )
        {
            bInString = !bInString;
            if ( bLookInString && !bInString )
                p = p1;     
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
        return -1;
    return (sal_Int32) (p - p0);
}



ScInputHandler::ScInputHandler()
    :   pInputWin( NULL ),
        pEngine( NULL ),
        pTableView( NULL ),
        pTopView( NULL ),
        pColumnData( NULL ),
        pFormulaData( NULL ),
        pFormulaDataPara( NULL ),
        pTipVisibleParent( NULL ),
        nTipVisible( 0 ),
        pTipVisibleSecParent( NULL ),
        nTipVisibleSec( 0 ),
        nFormSelStart( 0 ),
        nFormSelEnd( 0 ),
        nAutoPar( 0 ),
        eMode( SC_INPUT_NONE ),
        bUseTab( false ),
        bTextValid( true ),
        bModified( false ),
        bSelIsRef( false ),
        bFormulaMode( false ),
        bInRangeUpdate( false ),
        bParenthesisShown( false ),
        bCreatingFuncView( false ),
        bInEnterHandler( false ),
        bCommandErrorShown( false ),
        bInOwnChange( false ),
        bProtected( false ),
        bCellHasPercentFormat( false ),
        bLastIsSymbol( false ),
        mbDocumentDisposing(false),
        nValidation( 0 ),
        eAttrAdjust( SVX_HOR_JUSTIFY_STANDARD ),
        aScaleX( 1,1 ),
        aScaleY( 1,1 ),
        pRefViewSh( NULL ),
        pLastPattern( NULL ),
        pEditDefaults( NULL ),
        pLastState( NULL ),
        pDelayTimer( NULL ),
        pRangeFindList( NULL )
{
    
    
    pActiveViewSh = NULL;

    
}

ScInputHandler::~ScInputHandler()
{
    
    

    if (!mbDocumentDisposing)          
        EnterHandler();                     

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
    delete pFormulaDataPara;
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

    bool bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    bool bInPlace = pActiveViewSh && pActiveViewSh->GetViewFrame()->GetFrame().IsInPlace();
    sal_uLong nCtrl = pEngine->GetControlWord();
    if ( bTextWysiwyg || bInPlace )
        nCtrl |= EE_CNTRL_FORMAT100;    
    else
        nCtrl &= ~EE_CNTRL_FORMAT100;   
    pEngine->SetControlWord( nCtrl );
    if ( bTextWysiwyg && pActiveViewSh )
        pEngine->SetRefDevice( pActiveViewSh->GetViewData()->GetDocument()->GetPrinter() );
    else
        pEngine->SetRefDevice( NULL );

    MapMode aMode( MAP_100TH_MM, Point(), aScaleX, aScaleY );
    pEngine->SetRefMapMode( aMode );

    
    
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
            ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();
            pEngine = new ScFieldEditEngine(pDoc, pDoc->GetEnginePool(), pDoc->GetEditPool());
        }
        else
            pEngine = new ScFieldEditEngine(NULL, EditEngine::CreatePool(), NULL, true);
        pEngine->SetWordDelimiters( ScEditUtil::ModifyDelimiters( pEngine->GetWordDelimiters() ) );
        UpdateRefDevice();      
        pEngine->SetPaperSize( Size( 1000000, 1000000 ) );
        pEditDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );

        pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_AUTOCORRECT );
        pEngine->SetModifyHdl( LINK( this, ScInputHandler, ModifyHdl ) );
    }
}

void ScInputHandler::UpdateAutoCorrFlag()
{
    sal_uLong nCntrl = pEngine->GetControlWord();
    sal_uLong nOld = nCntrl;

    

    bool bDisable = bLastIsSymbol || bFormulaMode;
    if ( bDisable )
        nCntrl &= ~EE_CNTRL_AUTOCORRECT;
    else
        nCntrl |= EE_CNTRL_AUTOCORRECT;

    if ( nCntrl != nOld )
        pEngine->SetControlWord(nCntrl);
}

void ScInputHandler::UpdateSpellSettings( bool bFromStartTab )
{
    if ( pActiveViewSh )
    {
        ScViewData* pViewData = pActiveViewSh->GetViewData();
        bool bOnlineSpell = pViewData->GetDocument()->GetDocOptions().IsAutoSpell();

        
        
        

        pEngine->SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );

        
        

        if ( bFromStartTab || eMode != SC_INPUT_NONE )
        {
            sal_uLong nCntrl = pEngine->GetControlWord();
            sal_uLong nOld = nCntrl;
            if( bOnlineSpell )
                nCntrl |= EE_CNTRL_ONLINESPELLING;
            else
                nCntrl &= ~EE_CNTRL_ONLINESPELLING;
            
            if ( pLastPattern && pLastPattern->IsSymbolFont() )
                nCntrl &= ~EE_CNTRL_AUTOCORRECT;
            else
                nCntrl |= EE_CNTRL_AUTOCORRECT;
            if ( nCntrl != nOld )
                pEngine->SetControlWord(nCntrl);

            ScDocument* pDoc = pViewData->GetDocument();
            pDoc->ApplyAsianEditSettings( *pEngine );
            pEngine->SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( pViewData->GetTabNo() ) );
            pEngine->SetFirstWordCapitalization( false );
        }

        
        

        if ( bOnlineSpell ) {
            com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1> xXSpellChecker1( LinguMgr::GetSpellChecker() );
            pEngine->SetSpeller( xXSpellChecker1 );
        }

        bool bHyphen = pLastPattern && ((const SfxBoolItem&)pLastPattern->GetItem(ATTR_HYPHENATE)).GetValue();
        if ( bHyphen ) {
            com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
            pEngine->SetHyphenator( xXHyphenator );
        }
    }
}

//

//



void ScInputHandler::GetFormulaData()
{
    if ( pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();

        if ( pFormulaData )
            pFormulaData->clear();
        else
        {
            pFormulaData = new ScTypedCaseStrSet;
            miAutoPosFormula = pFormulaData->end();
        }

        if( pFormulaDataPara )
            pFormulaDataPara->clear();
        else
            pFormulaDataPara = new ScTypedCaseStrSet;

        
        

        const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
        sal_uInt16 nMRUCount = rOpt.GetLRUFuncListCount();
        const sal_uInt16* pMRUList = rOpt.GetLRUFuncList();
        const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
        sal_uLong nListCount = pFuncList->GetCount();
        if (pMRUList)
        {
            for (sal_uInt16 i=0; i<nMRUCount; i++)
            {
                sal_uInt16 nId = pMRUList[i];
                for (sal_uLong j=0; j<nListCount; j++)
                {
                    const ScFuncDesc* pDesc = pFuncList->GetFunction( j );
                    if ( pDesc->nFIndex == nId && pDesc->pFuncName )
                    {
                        OUString aEntry = *pDesc->pFuncName;
                        aEntry += "()";
                        pFormulaData->insert(ScTypedStrData(aEntry, 0.0, ScTypedStrData::Standard));
                        break;                  
                    }
                }
            }
        }
        for(sal_uLong i=0;i<nListCount;i++)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction( i );
            if ( pDesc->pFuncName )
            {
                pDesc->initArgumentInfo();
                OUString aEntry = pDesc->getSignature();
                pFormulaDataPara->insert(ScTypedStrData(aEntry, 0.0, ScTypedStrData::Standard));
            }
        }
        pDoc->GetFormulaEntries( *pFormulaData );
        pDoc->GetFormulaEntries( *pFormulaDataPara );
    }
}

IMPL_LINK( ScInputHandler, ShowHideTipVisibleParentListener, VclWindowEvent*, pEvent )
{
    if( pEvent->GetId() == VCLEVENT_OBJECT_DYING || pEvent->GetId() == VCLEVENT_WINDOW_HIDE )
        HideTip();
    return 0;
}

IMPL_LINK( ScInputHandler, ShowHideTipVisibleSecParentListener, VclWindowEvent*, pEvent )
{
    if( pEvent->GetId() == VCLEVENT_OBJECT_DYING || pEvent->GetId() == VCLEVENT_WINDOW_HIDE )
        HideTipBelow();
    return 0;
}

void ScInputHandler::HideTip()
{
    if ( nTipVisible )
    {
        if (pTipVisibleParent)
            pTipVisibleParent->RemoveEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleParentListener ) );
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
        pTipVisibleParent = NULL;
    }
    aManualTip = OUString();
}
void ScInputHandler::HideTipBelow()
{
    if ( nTipVisibleSec )
    {
        if (pTipVisibleSecParent)
            pTipVisibleSecParent->RemoveEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleSecParentListener ) );
        Help::HideTip( nTipVisibleSec );
        nTipVisibleSec = 0;
        pTipVisibleSecParent = NULL;
    }
    aManualTip = OUString();
}

void ScInputHandler::ShowTipCursor()
{
    HideTip();
    HideTipBelow();
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    const sal_Unicode cSheetSep = lcl_getSheetSeparator(pDocSh->GetDocument());

    if ( bFormulaMode && pActiveView && pFormulaDataPara && pEngine->GetParagraphCount() == 1 )
    {
        OUString aFormula = pEngine->GetText( 0 );
        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();
        if( aSel.nEndPos )
        {
            if ( aFormula.getLength() < aSel.nEndPos )
                return;
            sal_Int32 nPos = aSel.nEndPos;
            OUString  aSelText = aFormula.copy( 0, nPos );
            sal_Int32   nNextFStart = 0;
            sal_Int32  nArgPos = 0;
            const IFunctionDescription* ppFDesc;
            ::std::vector< OUString> aArgs;
            sal_uInt16      nArgs;
            bool bFound = false;
            FormulaHelper aHelper(ScGlobal::GetStarCalcFunctionMgr());

            while( !bFound )
            {
                aSelText += ")";
                sal_Int32 nLeftParentPos = lcl_MatchParenthesis( aSelText, aSelText.getLength()-1 );
                if( nLeftParentPos != -1 )
                {
                    sal_Unicode c = ( nLeftParentPos > 0 ) ? aSelText[ nLeftParentPos-1 ] : 0;
                    if( !(comphelper::string::isalphaAscii(c)) )
                        continue;
                    nNextFStart = aHelper.GetFunctionStart( aSelText, nLeftParentPos, true);
                    if( aHelper.GetNextFunc( aSelText, false, nNextFStart, NULL, &ppFDesc, &aArgs ) )
                    {
                        if( !ppFDesc->getFunctionName().isEmpty() )
                        {
                            nArgPos = aHelper.GetArgStart( aSelText, nNextFStart, 0 );
                            nArgs = static_cast<sal_uInt16>(ppFDesc->getParameterCount());

                            OUString aNew;
                            ScTypedCaseStrSet::const_iterator it =
                                findText(*pFormulaDataPara, pFormulaDataPara->end(), ppFDesc->getFunctionName(), aNew, false);
                            if (it != pFormulaDataPara->end())
                            {
                                bool bFlag = false;
                                sal_uInt16 nActive = 0;
                                for( sal_uInt16 i=0; i < nArgs; i++ )
                                {
                                    sal_Int32 nLength = aArgs[i].getLength();
                                    if( nArgPos <= aSelText.getLength()-1 )
                                    {
                                        nActive = i+1;
                                        bFlag = true;
                                    }
                                    nArgPos+=nLength+1;
                                }
                                if( bFlag )
                                {
                                    sal_Int32 nCountSemicolon = comphelper::string::getTokenCount(aNew, cSep) - 1;
                                    sal_Int32 nCountDot = comphelper::string::getTokenCount(aNew, cSheetSep) - 1;
                                    sal_Int32 nStartPosition = 0;
                                    sal_Int32 nEndPosition = 0;

                                    if( !nCountSemicolon )
                                    {
                                        for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                        {
                                            sal_Unicode cNext = aNew[i];
                                            if( cNext == '(' )
                                            {
                                                nStartPosition = i+1;
                                            }
                                        }
                                    }
                                    else if( !nCountDot )
                                    {
                                        sal_uInt16 nCount = 0;
                                        for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                        {
                                            sal_Unicode cNext = aNew[i];
                                            if( cNext == '(' )
                                            {
                                                nStartPosition = i+1;
                                            }
                                            else if( cNext == cSep )
                                            {
                                                nCount ++;
                                                nEndPosition = i;
                                                if( nCount == nActive )
                                                {
                                                    break;
                                                }
                                                nStartPosition = nEndPosition+1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        sal_uInt16 nCount = 0;
                                        for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                        {
                                            sal_Unicode cNext = aNew[i];
                                            if( cNext == '(' )
                                            {
                                                nStartPosition = i+1;
                                            }
                                            else if( cNext == cSep )
                                            {
                                                nCount ++;
                                                nEndPosition = i;
                                                if( nCount == nActive )
                                                {
                                                    break;
                                                }
                                                nStartPosition = nEndPosition+1;
                                            }
                                            else if( cNext == cSheetSep )
                                            {
                                                continue;
                                            }
                                        }
                                    }

                                    if (nStartPosition > 0)
                                    {
                                        OUStringBuffer aBuf;
                                        aBuf.append(aNew.copy(0, nStartPosition));
                                        aBuf.append(static_cast<sal_Unicode>(0x25BA));
                                        aBuf.append(aNew.copy(nStartPosition));
                                        aNew = aBuf.makeStringAndClear();
                                        ShowTipBelow( aNew );
                                        bFound = true;
                                    }
                                }
                                else
                                {
                                    ShowTipBelow( aNew );
                                    bFound = true;
                                }
                            }
                        }
                    }
                }
                else
                {
                    sal_Int32 nPosition = 0;
                    OUString aText = pEngine->GetWord( 0, aSel.nEndPos-1 );
                    /* XXX: dubious, what is this condition supposed to exactly match? */
                    if (aSel.nEndPos <= aText.getLength() && aText[ aSel.nEndPos-1 ] == '=')
                    {
                        break;
                    }
                    OUString aNew;
                    nPosition = aText.getLength()+1;
                    ScTypedCaseStrSet::const_iterator it =
                        findText(*pFormulaDataPara, pFormulaDataPara->end(), aText, aNew, false);
                    if (it != pFormulaDataPara->end())
                    {
                        if( nPosition < aFormula.getLength() && aFormula[ nPosition ] =='(' )
                        {
                            ShowTipBelow( aNew );
                            bFound = true;
                        }
                        else
                            break;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}

void ScInputHandler::ShowTip( const OUString& rText )
{
    
    HideTip();
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
    {
        Point aPos;
        pTipVisibleParent = pActiveView->GetWindow();
        Cursor* pCur = pActiveView->GetCursor();
        if (pCur)
            aPos = pTipVisibleParent->LogicToPixel( pCur->GetPos() );
        aPos = pTipVisibleParent->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aPos );

        sal_uInt16 nAlign = QUICKHELP_LEFT|QUICKHELP_BOTTOM;
        nTipVisible = Help::ShowTip(pTipVisibleParent, aRect, rText, nAlign);
        pTipVisibleParent->AddEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleParentListener ) );
    }
}

void ScInputHandler::ShowTipBelow( const OUString& rText )
{
    HideTipBelow();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView )
    {
        Point aPos;
        pTipVisibleSecParent = pActiveView->GetWindow();
        Cursor* pCur = pActiveView->GetCursor();
        if ( pCur )
        {
            Point aLogicPos = pCur->GetPos();
            aLogicPos.Y() += pCur->GetHeight();
            aPos = pTipVisibleSecParent->LogicToPixel( aLogicPos );
        }
        aPos = pTipVisibleSecParent->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aPos );
        sal_uInt16 nAlign = QUICKHELP_LEFT | QUICKHELP_TOP | QUICKHELP_NOEVADEPOINTER;
        nTipVisibleSec = Help::ShowTip(pTipVisibleSecParent, aRect, rText, nAlign);
        pTipVisibleSecParent->AddEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleSecParentListener ) );
    }
}

void ScInputHandler::UseFormulaData()
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    const sal_Unicode cSheetSep = lcl_getSheetSeparator(pDocSh->GetDocument());

    
    if ( pActiveView && pFormulaData && pEngine->GetParagraphCount() == 1 )
    {
        OUString aTotal = pEngine->GetText( 0 );
        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();

        
        
        

        if ( aSel.nEndPos > aTotal.getLength() )
            return;

        

        if ( aSel.nEndPos > 0 )
        {
            sal_Int32 nPos = aSel.nEndPos;
            OUString  aFormula = aTotal.copy( 0, nPos );;
            sal_Int32   nLeftParentPos = 0;
            sal_Int32   nNextFStart = 0;
            sal_Int32  nArgPos = 0;
            const IFunctionDescription* ppFDesc;
            ::std::vector< OUString> aArgs;
            sal_uInt16      nArgs;
            bool bFound = false;

            OUString aText = pEngine->GetWord( 0, aSel.nEndPos-1 );
            if (!aText.isEmpty())
            {
                OUString aNew;
                miAutoPosFormula = pFormulaData->end();
                miAutoPosFormula = findText(*pFormulaData, miAutoPosFormula, aText, aNew, false);
                if (miAutoPosFormula != pFormulaData->end())
                {
                    ShowTip( aNew );
                    aAutoSearch = aText;
                }
            }
            FormulaHelper aHelper(ScGlobal::GetStarCalcFunctionMgr());

            while( !bFound )
            {
                aFormula += ")";
                nLeftParentPos = lcl_MatchParenthesis( aFormula, aFormula.getLength()-1 );
                if( nLeftParentPos == -1 )
                    break;

                
                sal_Unicode c = ( nLeftParentPos > 0 ) ? aFormula[ nLeftParentPos-1 ] : 0;
                if( !(comphelper::string::isalphaAscii(c)) )
                    continue;
                nNextFStart = aHelper.GetFunctionStart( aFormula, nLeftParentPos, true);
                if( aHelper.GetNextFunc( aFormula, false, nNextFStart, NULL, &ppFDesc, &aArgs ) )
                {
                    if( !ppFDesc->getFunctionName().isEmpty() )
                    {
                        nArgPos = aHelper.GetArgStart( aFormula, nNextFStart, 0 );
                        nArgs = static_cast<sal_uInt16>(ppFDesc->getParameterCount());
                        OUString aFuncName = ppFDesc->getFunctionName() + "(";
                        OUString aNew;
                        ScTypedCaseStrSet::const_iterator it =
                            findText(*pFormulaDataPara, pFormulaDataPara->end(), aFuncName, aNew, false);
                        if (it != pFormulaDataPara->end())
                        {
                            bool bFlag = false;
                            sal_uInt16 nActive = 0;
                            for( sal_uInt16 i=0; i < nArgs; i++ )
                            {
                                sal_Int32 nLength = aArgs[i].getLength();
                                if( nArgPos <= aFormula.getLength()-1 )
                                {
                                    nActive = i+1;
                                    bFlag = true;
                                }
                                nArgPos+=nLength+1;
                            }
                            if( bFlag )
                            {
                                sal_Int32 nCountSemicolon = comphelper::string::getTokenCount(aNew, cSep) - 1;
                                sal_Int32 nCountDot = comphelper::string::getTokenCount(aNew, cSheetSep) - 1;
                                sal_Int32 nStartPosition = 0;
                                sal_Int32 nEndPosition = 0;

                               if( !nCountSemicolon )
                               {
                                    for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                    {
                                        sal_Unicode cNext = aNew[i];
                                        if( cNext == '(' )
                                        {
                                            nStartPosition = i+1;
                                        }
                                    }
                                }
                                else if( !nCountDot )
                                {
                                    sal_uInt16 nCount = 0;
                                    for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                    {
                                        sal_Unicode cNext = aNew[i];
                                        if( cNext == '(' )
                                        {
                                            nStartPosition = i+1;
                                        }
                                        else if( cNext == cSep )
                                        {
                                            nCount ++;
                                            nEndPosition = i;
                                            if( nCount == nActive )
                                            {
                                                break;
                                            }
                                            nStartPosition = nEndPosition+1;
                                        }
                                    }
                                }
                                else
                                {
                                    sal_uInt16 nCount = 0;
                                    for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                    {
                                        sal_Unicode cNext = aNew[i];
                                        if( cNext == '(' )
                                        {
                                            nStartPosition = i+1;
                                        }
                                        else if( cNext == cSep )
                                        {
                                            nCount ++;
                                            nEndPosition = i;
                                            if( nCount == nActive )
                                            {
                                                break;
                                            }
                                            nStartPosition = nEndPosition+1;
                                        }
                                        else if( cNext == cSheetSep )
                                        {
                                            continue;
                                        }
                                    }
                                }

                                if (nStartPosition > 0)
                                {
                                    OUStringBuffer aBuf;
                                    aBuf.append(aNew.copy(0, nStartPosition));
                                    aBuf.append(static_cast<sal_Unicode>(0x25BA));
                                    aBuf.append(aNew.copy(nStartPosition));
                                    aNew = aBuf.makeStringAndClear();
                                    ShowTipBelow( aNew );
                                    bFound = true;
                                }
                            }
                            else
                            {
                                ShowTipBelow( aNew );
                                bFound = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ScInputHandler::NextFormulaEntry( bool bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pFormulaData )
    {
        OUString aNew;
        ScTypedCaseStrSet::const_iterator itNew = findText(*pFormulaData, miAutoPosFormula, aAutoSearch, aNew, bBack);
        if (itNew != pFormulaData->end())
        {
            miAutoPosFormula = itNew;
            ShowTip(aNew); 
        }
    }

    

    if (pActiveView)
        pActiveView->ShowCursor();
}

static void lcl_CompleteFunction( EditView* pView, const OUString& rInsert, bool& rParInserted )
{
    if (pView)
    {
        ESelection aSel = pView->GetSelection();
        --aSel.nStartPos;
        --aSel.nEndPos;
        pView->SetSelection(aSel);
        pView->SelectCurrentWord();

        OUString aInsStr = rInsert;
        sal_Int32 nInsLen = aInsStr.getLength();
        bool bDoParen = ( nInsLen > 1 && aInsStr[nInsLen-2] == '('
                                      && aInsStr[nInsLen-1] == ')' );
        if ( bDoParen )
        {
            
            

            ESelection aWordSel = pView->GetSelection();
            OUString aOld = pView->GetEditEngine()->GetText(0);
            
            if (aWordSel.nEndPos < aOld.getLength())
            {
                sal_Unicode cNext = aOld[aWordSel.nEndPos];
                if ( cNext == '(' )
                {
                    bDoParen = false;
                    aInsStr = aInsStr.copy( 0, nInsLen - 2 );   
                }
            }
        }

        pView->InsertText( aInsStr, false );

        if ( bDoParen )                         
        {
            aSel = pView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pView->SetSelection(aSel);

            rParInserted = true;
        }
    }
}

void ScInputHandler::PasteFunctionData()
{
    if (pFormulaData && miAutoPosFormula != pFormulaData->end())
    {
        const ScTypedStrData& rData = *miAutoPosFormula;
        const OUString& aInsert = rData.GetString();
        bool bParInserted = false;

        DataChanging();                         
        lcl_CompleteFunction( pTopView, aInsert, bParInserted );
        lcl_CompleteFunction( pTableView, aInsert, bParInserted );
        DataChanged();
        ShowTipCursor();

        if (bParInserted)
            AutoParAdded();
    }

    HideTip();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
        pActiveView->ShowCursor();
}

//

//

static OUString lcl_Calculate( const OUString& rFormula, ScDocument* pDoc, const ScAddress &rPos )
{
    
    

    if(rFormula.isEmpty())
        return OUString();

    boost::scoped_ptr<ScSimpleFormulaCalculator> pCalc( new ScSimpleFormulaCalculator( pDoc, rPos, rFormula ) );

    
    
    
    
    bool bColRowName = pCalc->HasColRowName();
    if ( bColRowName )
    {
        
        if ( pCalc->GetCode()->GetCodeLen() <= 1 )
        {   
            
            OUStringBuffer aBraced;
            aBraced.append('(');
            aBraced.append(rFormula);
            aBraced.append(')');
            pCalc.reset( new ScSimpleFormulaCalculator( pDoc, rPos, aBraced.makeStringAndClear() ) );
        }
        else
            bColRowName = false;
    }

    sal_uInt16 nErrCode = pCalc->GetErrCode();
    if ( nErrCode != 0 )
        return ScGlobal::GetErrorString(nErrCode);

    SvNumberFormatter& aFormatter = *(pDoc->GetFormatTable());
    OUString aValue;
    if ( pCalc->IsValue() )
    {
        double n = pCalc->GetValue();
        sal_uLong nFormat = aFormatter.GetStandardFormat( n, 0,
                pCalc->GetFormatType(), ScGlobal::eLnge );
        aFormatter.GetInputLineString( n, nFormat, aValue );
        
    }
    else
    {
        OUString aStr = pCalc->GetString().getString();
        sal_uLong nFormat = aFormatter.GetStandardFormat(
                pCalc->GetFormatType(), ScGlobal::eLnge);
        {
            Color* pColor;
            aFormatter.GetOutputString( aStr, nFormat,
                    aValue, &pColor );
        }

        aValue = "\"" + aValue + "\"";
        
    }

    ScRange aTestRange;
    if ( bColRowName || (aTestRange.Parse(rFormula) & SCA_VALID) )
        aValue = aValue + " ...";

    return aValue;
}

void ScInputHandler::FormulaPreview()
{
    OUString aValue;
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pActiveViewSh )
    {
        OUString aPart = pActiveView->GetSelected();
        if (aPart.isEmpty())
            aPart = pEngine->GetText(0);
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();
        aValue = lcl_Calculate( aPart, pDoc, aCursorPos );
    }

    if (!aValue.isEmpty())
    {
        ShowTip( aValue );          
        aManualTip = aValue;        
        if (pFormulaData)
            miAutoPosFormula = pFormulaData->end();
        if (pColumnData)
            miAutoPosColumn = pColumnData->end();
    }
}

void ScInputHandler::PasteManualTip()
{
    
    

    sal_Int32 nTipLen = aManualTip.getLength();
    sal_uInt32 const nTipLen2(sal::static_int_cast<sal_uInt32>(nTipLen));
    if ( nTipLen && ( nTipLen < 3 || aManualTip.copy( nTipLen2-3 ) != "..." ) )
    {
        DataChanging();                                     

        OUString aInsert = aManualTip;
        EditView* pActiveView = pTopView ? pTopView : pTableView;
        if (!pActiveView->HasSelection())
        {
            
            sal_Int32 nOldLen = pEngine->GetTextLen(0);
            ESelection aAllSel( 0, 0, 0, nOldLen );
            if ( pTopView )
                pTopView->SetSelection( aAllSel );
            if ( pTableView )
                pTableView->SetSelection( aAllSel );
        }

        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();
        OSL_ENSURE( !aSel.nStartPara && !aSel.nEndPara, "Zuviele Absaetze in Formel" );
        if ( !aSel.nStartPos )  
        {
            if ( aSel.nEndPos == pEngine->GetTextLen(0) )
            {
                
                if ( aInsert[0] == '"' )
                    aInsert = aInsert.copy(1);
                sal_Int32 nInsLen = aInsert.getLength();
                if ( aInsert.endsWith("\"") )
                    aInsert = aInsert.copy( 0, nInsLen-1 );
            }
            else if ( aSel.nEndPos )
            {
                
                

                aSel.nStartPos = 1;
                if ( pTopView )
                    pTopView->SetSelection( aSel );
                if ( pTableView )
                    pTableView->SetSelection( aSel );
            }
        }
        if ( pTopView )
            pTopView->InsertText( aInsert, true );
        if ( pTableView )
            pTableView->InsertText( aInsert, true );

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
    ++nAutoPar;     
}

bool ScInputHandler::CursorAtClosingPar()
{
    

    
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && !pActiveView->HasSelection() && bFormulaMode )
    {
        ESelection aSel = pActiveView->GetSelection();
        sal_Int32 nPos = aSel.nStartPos;
        OUString aFormula = pEngine->GetText(0);
        if ( nPos < aFormula.getLength() && aFormula[nPos] == ')' )
            return true;
    }
    return false;
}

void ScInputHandler::SkipClosingPar()
{
    
    

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
    {
        ESelection aSel = pActiveView->GetSelection();
        ++aSel.nStartPos;
        ++aSel.nEndPos;

        
        

        if ( pTopView )
            pTopView->SetSelection( aSel );
        if ( pTableView )
            pTableView->SetSelection( aSel );
    }

    OSL_ENSURE(nAutoPar, "SkipClosingPar: count is wrong");
    --nAutoPar;
}

//

//

void ScInputHandler::GetColData()
{
    if ( pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocShell()->GetDocument();

        if ( pColumnData )
            pColumnData->clear();
        else
        {
            pColumnData = new ScTypedCaseStrSet;
            miAutoPosColumn = pColumnData->end();
        }

        std::vector<ScTypedStrData> aEntries;
        pDoc->GetDataEntries(
            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), true, aEntries, true);
        if (!aEntries.empty())
            pColumnData->insert(aEntries.begin(), aEntries.end());
    }
}

void ScInputHandler::UseColData()           
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pColumnData )
    {
        

        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();

        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if ( aSel.nEndPara+1 == nParCnt )
        {
            sal_Int32 nParLen = pEngine->GetTextLen( aSel.nEndPara );
            if ( aSel.nEndPos == nParLen )
            {
                OUString aText = GetEditText(pEngine);
                if (!aText.isEmpty())
                {
                    OUString aNew;
                    miAutoPosColumn = pColumnData->end();
                    miAutoPosColumn = findText(*pColumnData, miAutoPosColumn, aText, aNew, false);
                    if (miAutoPosColumn != pColumnData->end())
                    {
                        
                        
                        
                        lcl_RemoveLineEnd( aNew );

                        
                        

                        
                        sal_Int32 nEdLen = pEngine->GetTextLen() + nParCnt - 1;
                        OUString aIns = aNew.copy(nEdLen);

                        
                        
                        ESelection aSelection( aSel.nEndPara, aSel.nEndPos + aIns.getLength(),
                                               aSel.nEndPara, aSel.nEndPos );

                        
                        if ( pTableView )
                        {
                            pTableView->InsertText( aIns, false );
                            pTableView->SetSelection( aSelection );
                        }
                        if ( pTopView )
                        {
                            pTopView->InsertText( aIns, false );
                            pTopView->SetSelection( aSelection );
                        }

                        aAutoSearch = aText;    

                        if (aText.getLength() == aNew.getLength())
                        {
                            
                            

                            OUString aDummy;
                            ScTypedCaseStrSet::const_iterator itNextPos =
                                findText(*pColumnData, miAutoPosColumn, aText, aDummy, false);
                            bUseTab = itNextPos != pColumnData->end();
                        }
                        else
                            bUseTab = true;
                    }
                }
            }
        }
    }
}

void ScInputHandler::NextAutoEntry( bool bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pColumnData )
    {
        if (miAutoPosColumn != pColumnData->end() && !aAutoSearch.isEmpty())
        {
            

            ESelection aSel = pActiveView->GetSelection();
            aSel.Adjust();
            sal_Int32 nParCnt = pEngine->GetParagraphCount();
            if ( aSel.nEndPara+1 == nParCnt && aSel.nStartPara == aSel.nEndPara )
            {
                OUString aText = GetEditText(pEngine);
                sal_Int32 nSelLen = aSel.nEndPos - aSel.nStartPos;
                sal_Int32 nParLen = pEngine->GetTextLen( aSel.nEndPara );
                if ( aSel.nEndPos == nParLen && aText.getLength() == aAutoSearch.getLength() + nSelLen )
                {
                    OUString aNew;
                    ScTypedCaseStrSet::const_iterator itNew =
                        findText(*pColumnData, miAutoPosColumn, aAutoSearch, aNew, bBack);

                    if (itNew != pColumnData->end())
                    {
                        
                        miAutoPosColumn = itNew;
                        bInOwnChange = true;        

                        lcl_RemoveLineEnd( aNew );
                        OUString aIns = aNew.copy(aAutoSearch.getLength());

                        
                        if ( pTableView )
                        {
                            pTableView->DeleteSelected();
                            pTableView->InsertText( aIns, false );
                            pTableView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.getLength(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }
                        if ( pTopView )
                        {
                            pTopView->DeleteSelected();
                            pTopView->InsertText( aIns, false );
                            pTopView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.getLength(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }

                        bInOwnChange = false;
                    }
                }
            }
        }
    }

    

    if (pActiveView)
        pActiveView->ShowCursor();
}

//

//

void ScInputHandler::UpdateParenthesis()
{
    

    

    bool bFound = false;
    if ( bFormulaMode && eMode != SC_INPUT_TOP )
    {
        if ( pTableView && !pTableView->HasSelection() )        
        {
            ESelection aSel = pTableView->GetSelection();
            if (aSel.nStartPos)
            {
                

                sal_Int32 nPos = aSel.nStartPos - 1;
                OUString aFormula = pEngine->GetText(0);
                sal_Unicode c = aFormula[nPos];
                if ( c == '(' || c == ')' )
                {
                    sal_Int32 nOther = lcl_MatchParenthesis( aFormula, nPos );
                    if ( nOther != -1 )
                    {
                        SfxItemSet aSet( pEngine->GetEmptyItemSet() );
                        aSet.Put( SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
                        

                        if (bParenthesisShown)
                        {
                            
                            sal_Int32 nCount = pEngine->GetParagraphCount();
                            for (sal_Int32 i=0; i<nCount; i++)
                                pEngine->QuickRemoveCharAttribs( i, EE_CHAR_WEIGHT );
                        }

                        ESelection aSelThis( 0,nPos, 0,nPos+1 );
                        pEngine->QuickSetAttribs( aSet, aSelThis );
                        ESelection aSelOther( 0,nOther, 0,nOther+1 );
                        pEngine->QuickSetAttribs( aSet, aSelOther );

                        
                        pTableView->InsertText( EMPTY_OUSTRING, false );

                        bFound = true;
                    }
                }
            }

            
            
        }
    }

    

    if ( bParenthesisShown && !bFound && pTableView )
    {
        sal_Int32 nCount = pEngine->GetParagraphCount();
        for (sal_Int32 i=0; i<nCount; i++)
            pTableView->RemoveCharAttribs( i, EE_CHAR_WEIGHT );
    }

    bParenthesisShown = bFound;
}

void ScInputHandler::ViewShellGone(ScTabViewShell* pViewSh)     
{
    if ( pViewSh == pActiveViewSh )
    {
        delete pLastState;
        pLastState = NULL;
        pLastPattern = NULL;
    }

    if ( pViewSh == pRefViewSh )
    {
        
        

        EnterHandler();
        bFormulaMode = false;
        pRefViewSh = NULL;
        SFX_APP()->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }

    pActiveViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );

    if ( pActiveViewSh && pActiveViewSh == pViewSh )
    {
        OSL_FAIL("pActiveViewSh weg");
        pActiveViewSh = NULL;
    }

    if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        UpdateRefDevice();      
}

void ScInputHandler::UpdateActiveView()
{
    ImplCreateEditEngine();

    
    
    
    

    Window* pShellWin = pActiveViewSh ?
                pActiveViewSh->GetWindowByPos( pActiveViewSh->GetViewData()->GetEditActivePart() ) :
                NULL;

    sal_uInt16 nCount = pEngine->GetViewCount();
    if (nCount > 0)
    {
        pTableView = pEngine->GetView(0);
        for (sal_uInt16 i=1; i<nCount; i++)
        {
            EditView* pThis = pEngine->GetView(i);
            Window* pWin = pThis->GetWindow();
            if ( pWin==pShellWin )
                pTableView = pThis;
        }
    }
    else
        pTableView = NULL;

    if (pInputWin && eMode == SC_INPUT_TOP )
        pTopView = pInputWin->GetEditView();
    else
        pTopView = NULL;
}

void ScInputHandler::StopInputWinEngine( bool bAll )
{
    if (pInputWin)
        pInputWin->StopEditEngine( bAll );

    pTopView = NULL;        
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
        pActiveViewSh->UpdateInputHandler( true );      
    else
        NotifyChange( pLastState, true );
}

void ScInputHandler::UpdateAdjust( sal_Unicode cTyped )
{
    SvxAdjust eSvxAdjust;
    switch (eAttrAdjust)
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            {
                bool bNumber = false;
                if (cTyped)                                     
                    bNumber = (cTyped>='0' && cTyped<='9');     
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
        default:    
            eSvxAdjust = SVX_ADJUST_LEFT;
            break;
    }

    bool bAsianVertical = pLastPattern &&
        ((const SfxBoolItem&)pLastPattern->GetItem( ATTR_STACKED )).GetValue() &&
        ((const SfxBoolItem&)pLastPattern->GetItem( ATTR_VERTICAL_ASIAN )).GetValue();
    if ( bAsianVertical )
    {
        
        eSvxAdjust = SVX_ADJUST_LEFT;
    }

    pEditDefaults->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
    pEngine->SetDefaults( *pEditDefaults );

    nEditAdjust = sal::static_int_cast<sal_uInt16>(eSvxAdjust);     

    pEngine->SetVertical( bAsianVertical );
}

void ScInputHandler::RemoveAdjust()
{
    

    bool bUndo = pEngine->IsUndoEnabled();
    if ( bUndo )
        pEngine->EnableUndo( false );

    
    
    pEngine->RemoveParaAttribs();

    if ( bUndo )
        pEngine->EnableUndo( true );

}

void ScInputHandler::RemoveRangeFinder()
{
    

    pEngine->SetUpdateMode(false);
    sal_Int32 nCount = pEngine->GetParagraphCount();   
    for (sal_Int32 i=0; i<nCount; i++)
        pEngine->QuickRemoveCharAttribs( i, EE_CHAR_COLOR );
    pEngine->SetUpdateMode(true);

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    pActiveView->ShowCursor( false, true );

    DeleteRangeFinder();        
}

bool ScInputHandler::StartTable( sal_Unicode cTyped, bool bFromCommand, bool bInputActivated )
{
    bool bNewTable = false;

    if (bModified || !ValidCol(aCursorPos.Col()))
        return false;

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
            aTester.TestSelectedBlock(
                pDoc, aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Col(), aCursorPos.Row(), rMark );

        bool bStartInputMode = true;

        if (!aTester.IsEditable())
        {
            bProtected = true;
            
            
            
            
            bool bShowError = (!bInputActivated || aTester.GetMessageId() != STR_PROTECTIONERR) &&
                !pActiveViewSh->GetViewData()->GetDocShell()->IsReadOnly();
            if (bShowError)
            {
                eMode = SC_INPUT_NONE;
                StopInputWinEngine( true );
                UpdateFormulaMode();
                if ( pActiveViewSh && ( !bFromCommand || !bCommandErrorShown ) )
                {
                    
                    
                    
                    
                    if ( bFromCommand )
                        bCommandErrorShown = true;

                    pActiveViewSh->GetActiveWin()->GrabFocus();
                    pActiveViewSh->ErrorMessage(aTester.GetMessageId());
                }
                bStartInputMode = false;
            }
        }

        if (bStartInputMode)
        {
            
            pEngine->SetUpdateMode( false );

            

            const ScPatternAttr* pPattern = pDoc->GetPattern( aCursorPos.Col(),
                                                              aCursorPos.Row(),
                                                              aCursorPos.Tab() );
            if (pPattern != pLastPattern)
            {
                

                const SfxItemSet& rAttrSet = pPattern->GetItemSet();
                const SfxPoolItem* pItem;

                if ( SFX_ITEM_SET == rAttrSet.GetItemState( ATTR_VALUE_FORMAT, true, &pItem ) )
                {
                    sal_uLong nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                    bCellHasPercentFormat = ( NUMBERFORMAT_PERCENT ==
                                              pDoc->GetFormatTable()->GetType( nFormat ) );
                }
                else
                    bCellHasPercentFormat = false; 

                

                if ( SFX_ITEM_SET == rAttrSet.GetItemState( ATTR_VALIDDATA, true, &pItem ) )
                    nValidation = ((const SfxUInt32Item*)pItem)->GetValue();
                else
                    nValidation = 0;

                

                
                
                

                
                
                

                pPattern->FillEditItemSet( pEditDefaults );
                pEngine->SetDefaults( *pEditDefaults );
                pLastPattern = pPattern;
                bLastIsSymbol = pPattern->IsSymbolFont();

                
                

                Color aBackCol = ((const SvxBrushItem&)
                                pPattern->GetItem( ATTR_BACKGROUND )).GetColor();
                ScModule* pScMod = SC_MOD();
                if ( aBackCol.GetTransparency() > 0 ||
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    aBackCol.SetColor( pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );
                pEngine->SetBackgroundColor( aBackCol );

                

                eAttrAdjust = (SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                GetItem(ATTR_HOR_JUSTIFY)).GetValue();
                if ( eAttrAdjust == SVX_HOR_JUSTIFY_REPEAT &&
                     static_cast<const SfxBoolItem&>(pPattern->GetItem(ATTR_LINEBREAK)).GetValue() )
                {
                    
                    eAttrAdjust = SVX_HOR_JUSTIFY_STANDARD;
                }
            }

            
            

            UpdateSpellSettings( true );    

            

            OUString aStr;
            if (bTextValid)
            {
                pEngine->SetText(aCurrentText);
                aStr = aCurrentText;
                bTextValid = false;
                aCurrentText = OUString();
            }
            else
                aStr = GetEditText(pEngine);

            if (aStr.startsWith("{=") && aStr.endsWith("}") )  
            {
                aStr = aStr.copy(1, aStr.getLength() -2);
                pEngine->SetText(aStr);
                if ( pInputWin )
                    pInputWin->SetTextString(aStr);
            }

            UpdateAdjust( cTyped );

            if ( bAutoComplete )
                GetColData();

            if ( !aStr.isEmpty() && ( aStr[0] == '=' || aStr[0] == '+' || aStr[0] == '-' ) &&
                 !cTyped && !bCreatingFuncView )
                InitRangeFinder(aStr);              

            bNewTable = true;       
        }
    }

    if (!bProtected && pInputWin)
        pInputWin->SetOkCancelMode();

    return bNewTable;
}

static void lcl_SetTopSelection( EditView* pEditView, ESelection& rSel )
{
    OSL_ENSURE( rSel.nStartPara==0 && rSel.nEndPara==0, "SetTopSelection: Para != 0" );

    EditEngine* pEngine = pEditView->GetEditEngine();
    sal_Int32 nCount = pEngine->GetParagraphCount();
    if (nCount > 1)
    {
        sal_Int32 nParLen = pEngine->GetTextLen(rSel.nStartPara);
        while (rSel.nStartPos > nParLen && rSel.nStartPara+1 < nCount)
        {
            rSel.nStartPos -= nParLen + 1;          
            nParLen = pEngine->GetTextLen(++rSel.nStartPara);
        }

        nParLen = pEngine->GetTextLen(rSel.nEndPara);
        while (rSel.nEndPos > nParLen && rSel.nEndPara+1 < nCount)
        {
            rSel.nEndPos -= nParLen + 1;            
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
    if (pSourceView)
    {
        bool bSelectionForTopView = false;
        if (pTopView && pTopView != pSourceView)
            bSelectionForTopView = true;
        bool bSelectionForTableView = false;
        if (pTableView && pTableView != pSourceView)
            bSelectionForTableView = true;
        if (bSelectionForTopView || bSelectionForTableView)
        {
            ESelection aSel(pSourceView->GetSelection());
            if (bSelectionForTopView)
                pTopView->SetSelection(aSel);
            if (bSelectionForTableView)
                lcl_SetTopSelection(pTableView, aSel);
        }
    }
    
    else if (pTopView && pTableView)
    {
        ESelection aSel(pTopView->GetSelection());
        lcl_SetTopSelection( pTableView, aSel );
    }
}

IMPL_LINK_NOARG(ScInputHandler, ModifyHdl)
{
    if ( !bInOwnChange && ( eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE ) &&
         pEngine && pEngine->GetUpdateMode() && pInputWin )
    {
        
        

        OUString aText;
        if ( pInputWin->IsMultiLineInput() )
            aText = ScEditUtil::GetMultilineString(*pEngine);
        else
            aText = GetEditText(pEngine);
        lcl_RemoveTabs(aText);
        pInputWin->SetTextString(aText);
    }
    return 0;
}

bool ScInputHandler::DataChanging( sal_Unicode cTyped, bool bFromCommand )      
{
    if (pActiveViewSh)
        pActiveViewSh->GetViewData()->SetPasteMode( SC_PASTE_NONE );
    bInOwnChange = true;                

    if ( eMode == SC_INPUT_NONE )
        return StartTable( cTyped, bFromCommand, false );
    else
        return false;
}

void ScInputHandler::DataChanged( bool bFromTopNotify, bool bSetModified )
{
    ImplCreateEditEngine();

    if (eMode==SC_INPUT_NONE)
        eMode = SC_INPUT_TYPE;

    if ( eMode == SC_INPUT_TOP && pTopView && !bFromTopNotify )
    {
        
        
        pTopView->GetEditEngine()->QuickFormatDoc( true );

        
        
        
        pTopView->ShowCursor();
    }

    if (bSetModified)
        bModified = true;
    bSelIsRef = false;

    if ( pRangeFindList && !bInRangeUpdate )
        RemoveRangeFinder();                    

    UpdateParenthesis();    

    if (eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE)
    {
        OUString aText;
        if ( pInputWin && pInputWin->IsMultiLineInput() )
            aText = ScEditUtil::GetMultilineString(*pEngine);
        else
            aText = GetEditText(pEngine);
        lcl_RemoveTabs(aText);

        if ( pInputWin )
            pInputWin->SetTextString( aText );
    }

        
        
        

    
    
    pEngine->QuickFormatDoc();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView && pActiveViewSh)
    {
        ScViewData* pViewData = pActiveViewSh->GetViewData();

        bool bNeedGrow = ( nEditAdjust != SVX_ADJUST_LEFT );        
        if (!bNeedGrow)
        {
                
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
            
            pViewData->EditGrowY();
            pViewData->EditGrowX();
        }
    }

    UpdateFormulaMode();
    bTextValid = false;         
    bInOwnChange = false;
}

void ScInputHandler::UpdateFormulaMode()
{
    SfxApplication* pSfxApp = SFX_APP();

    bool bIsFormula = !bProtected && pEngine->GetParagraphCount() == 1;
    if (bIsFormula)
    {
        const OUString& rText = pEngine->GetText(0);
        bIsFormula = !rText.isEmpty() &&
            (rText[0] == '=' || rText[0] == '+' || rText[0] == '-');
    }

    if ( bIsFormula )
    {
        if (!bFormulaMode)
        {
            bFormulaMode = true;
            pRefViewSh = pActiveViewSh;
            pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
            SC_MOD()->SetRefInputHdl(this);
            if (pInputWin)
                pInputWin->SetFormulaMode(true);

            if ( bAutoComplete )
                GetFormulaData();

            UpdateParenthesis();
            UpdateAutoCorrFlag();
        }
    }
    else        
    {
        if (bFormulaMode)
        {
            ShowRefFrame();
            bFormulaMode = false;
            pRefViewSh = NULL;
            pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
            SC_MOD()->SetRefInputHdl(NULL);
            if (pInputWin)
                pInputWin->SetFormulaMode(false);
            UpdateAutoCorrFlag();
        }
    }
}

void ScInputHandler::ShowRefFrame()
{
    
    
    
    ScTabViewShell* pVisibleSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    if ( pRefViewSh && pRefViewSh != pVisibleSh )
    {
        bool bFound = false;
        SfxViewFrame* pRefFrame = pRefViewSh->GetViewFrame();
        SfxViewFrame* pOneFrame = SfxViewFrame::GetFirst();
        while ( pOneFrame && !bFound )
        {
            if ( pOneFrame == pRefFrame )
                bFound = true;
            pOneFrame = SfxViewFrame::GetNext( *pOneFrame );
        }

        if (bFound)
        {
            
            

            pRefViewSh->SetActive();    

            
        }
        else
        {
            OSL_FAIL("ViewFrame fuer Referenzeingabe ist nicht mehr da");
        }
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

        rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
        rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
        rBindings.Invalidate( SID_SET_SUB_SCRIPT );
        rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
        rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );
    }
}

//

//

void ScInputHandler::SetMode( ScInputMode eNewMode )
{
    if ( eMode == eNewMode )
        return;

    ImplCreateEditEngine();

    if (bProtected)
    {
        eMode = SC_INPUT_NONE;
        StopInputWinEngine( true );
        if (pActiveViewSh)
            pActiveViewSh->GetActiveWin()->GrabFocus();
        return;
    }

    if (eNewMode != SC_INPUT_NONE && pActiveViewSh)
        
        pActiveViewSh->GetViewData()->SetPasteMode( SC_PASTE_NONE );

    bInOwnChange = true;                

    ScInputMode eOldMode = eMode;
    eMode = eNewMode;
    if (eOldMode == SC_INPUT_TOP && eNewMode != eOldMode)
        StopInputWinEngine( false );

    if (eMode==SC_INPUT_TOP || eMode==SC_INPUT_TABLE)
    {
        if (eOldMode == SC_INPUT_NONE)      
        {
            if (StartTable(0, false, eMode == SC_INPUT_TABLE))
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
            }
        }

        sal_Int32 nPara = pEngine->GetParagraphCount()-1;
        sal_Int32 nLen = pEngine->GetText(nPara).getLength();
        sal_uInt16 nCount = pEngine->GetViewCount();

        for (sal_uInt16 i=0; i<nCount; i++)
        {
            if ( eMode == SC_INPUT_TABLE && eOldMode == SC_INPUT_TOP )
            {
                
            }
            else
            {
                pEngine->GetView(i)->
                    SetSelection( ESelection( nPara, nLen, nPara, nLen ) );
            }
            pEngine->GetView(i)->ShowCursor(false);
        }
    }

    UpdateActiveView();
    if (eMode==SC_INPUT_TABLE || eMode==SC_INPUT_TYPE)
    {
        if (pTableView)
            pTableView->SetEditEngineUpdateMode(true);
    }
    else
    {
        if (pTopView)
            pTopView->SetEditEngineUpdateMode(true);
    }

    if (eNewMode != eOldMode)
        UpdateFormulaMode();

    bInOwnChange = false;
}





static bool lcl_IsNumber(const OUString& rString)
{
    sal_Int32 nLen = rString.getLength();
    for (sal_Int32 i=0; i<nLen; i++)
    {
        sal_Unicode c = rString[i];
        if ( c < '0' || c > '9' )
            return false;
    }
    return true;
}

static void lcl_SelectionToEnd( EditView* pView )
{
    if ( pView )
    {
        EditEngine* pEngine = pView->GetEditEngine();
        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if ( nParCnt == 0 )
            nParCnt = 1;
        ESelection aSel( nParCnt-1, pEngine->GetTextLen(nParCnt-1) );   
        pView->SetSelection( aSel );
    }
}

void ScInputHandler::EnterHandler( sal_uInt8 nBlockMode )
{
    
    

    if (bInEnterHandler) return;
    bInEnterHandler = true;
    bInOwnChange = true;                

    ImplCreateEditEngine();

    bool bMatrix = ( nBlockMode == SC_ENTER_MATRIX );

    SfxApplication* pSfxApp     = SFX_APP();
    EditTextObject* pObject     = NULL;
    ScPatternAttr*  pCellAttrs  = NULL;
    bool            bForget     = false;    

    OUString aString = GetEditText(pEngine);
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (bModified && pActiveView && !aString.isEmpty() && !lcl_IsNumber(aString))
    {
        if (pColumnData && miAutoPosColumn != pColumnData->end())
        {
            
            

            lcl_SelectionToEnd(pTopView);
            lcl_SelectionToEnd(pTableView);
        }

        Window* pFrameWin = pActiveViewSh ? pActiveViewSh->GetFrameWin() : NULL;

        if (pTopView)
            pTopView->CompleteAutoCorrect();    
        if (pTableView)
            pTableView->CompleteAutoCorrect(pFrameWin);
        aString = GetEditText(pEngine);
    }
    lcl_RemoveTabs(aString);

    

    if ( bModified && nValidation && pActiveViewSh )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
        const ScValidationData* pData = pDoc->GetValidationEntry( nValidation );
        if (pData && pData->HasErrMsg())
        {
            
            const ScPatternAttr* pPattern = pDoc->GetPattern( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
            bool bOk = pData->IsDataValid( aString, *pPattern, aCursorPos );

            if (!bOk)
            {
                if ( pActiveViewSh )                
                    pActiveViewSh->StopMarking();   

                    
                    

                Window* pParent = Application::GetDefDialogParent();
                if ( pData->DoError( pParent, aString, aCursorPos ) )
                    bForget = true;                 
            }
        }
    }

    

    if ( bModified && pActiveViewSh && !bForget )
    {
        ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
        ScDPObject* pDPObj = pDoc->GetDPAtCursor( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
        if ( pDPObj )
        {
            
            

            pActiveViewSh->DataPilotInput( aCursorPos, aString );
            bForget = true;
        }
    }

    std::vector<editeng::MisspellRanges> aMisspellRanges;
    pEngine->CompleteOnlineSpelling();
    bool bSpellErrors = !bFormulaMode && pEngine->HasOnlineSpellErrors();
    if ( bSpellErrors )
    {
        
        

        if ( pActiveViewSh )
        {
            ScDocument* pDoc = pActiveViewSh->GetViewData()->GetDocument();
            
            const ScPatternAttr* pPattern = pDoc->GetPattern( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
            if (pPattern)
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                
                sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
                double nVal;
                if ( pFormatter->IsNumberFormat( aString, nFormat, nVal ) )
                {
                    bSpellErrors = false;       
                }
            }
        }
    }

    
    
    
    pEngine->SetUpdateMode( false );

    if ( bModified && !bForget )            
    {
        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if ( nParCnt == 0 )
            nParCnt = 1;

        bool bUniformAttribs = true;
        SfxItemSet aPara1Attribs = pEngine->GetAttribs(0, 0, pEngine->GetTextLen(0));
        for (sal_Int32 nPara = 1; nPara < nParCnt; ++nPara)
        {
            SfxItemSet aPara2Attribs = pEngine->GetAttribs(nPara, 0, pEngine->GetTextLen(nPara));
            if (!(aPara1Attribs == aPara2Attribs))
            {
                
                bUniformAttribs = false;
                break;
            }
        }

        ESelection aSel( 0, 0, nParCnt-1, pEngine->GetTextLen(nParCnt-1) );
        SfxItemSet aOldAttribs = pEngine->GetAttribs( aSel );
        const SfxPoolItem* pItem = NULL;

        

        if ( pActiveViewSh && bUniformAttribs )
        {
            SfxItemSet* pCommonAttrs = NULL;
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, false, &pItem );
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

        

        RemoveAdjust();

        bool bAttrib = false;    
        

        if (nParCnt > 1)
            bAttrib = true;
        else
        {
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END && !bAttrib; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, false, &pItem );
                if (eState == SFX_ITEM_DONTCARE)
                    bAttrib = true;
                else if (eState == SFX_ITEM_SET)
                {
                    
                    if ( nId == EE_CHAR_ESCAPEMENT || nId == EE_CHAR_PAIRKERNING ||
                         nId == EE_CHAR_KERNING || nId == EE_CHAR_XMLATTRIBS )
                    {
                        if ( *pItem != pEditDefaults->Get(nId) )
                            bAttrib = true;
                    }
                }
            }

            

            SfxItemState eFieldState = aOldAttribs.GetItemState( EE_FEATURE_FIELD, false );
            if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
                bAttrib = true;

            

            SfxItemState eConvState = aOldAttribs.GetItemState( EE_FEATURE_NOTCONV, false );
            if ( eConvState == SFX_ITEM_DONTCARE || eConvState == SFX_ITEM_SET )
                bAttrib = true;

            
            
        }

        if (bSpellErrors)
            pEngine->GetAllMisspellRanges(aMisspellRanges);

        if (bMatrix)
            bAttrib = false;

        if (bAttrib)
        {
            pEngine->ClearSpellErrors();
            pObject = pEngine->CreateTextObject();
        }
        else if (bAutoComplete)         
        {
            
            if (pColumnData && aAutoSearch.getLength() < aString.getLength())
                aString = getExactMatch(*pColumnData, aString);
        }
    }

    
    
    
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();

        if (pExecuteSh)
        {
            pExecuteSh->SetTabNo(aCursorPos.Tab());
            pExecuteSh->ActiveGrabFocus();
        }

        bFormulaMode = false;
        pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = NULL;          
    DeleteRangeFinder();
    ResetAutoPar();

    bool bOldMod = bModified;

    bModified = false;
    bSelIsRef = false;
    eMode     = SC_INPUT_NONE;
    StopInputWinEngine(true);

    
    
    pLastPattern = NULL;

    if (bOldMod && !bProtected && !bForget)
    {
        

        if (aString.startsWith("="))
        {
            SvxAutoCorrect* pAuto = SvxAutoCorrCfg::Get().GetAutoCorrect();
            if ( pAuto )
            {
                OUString aReplace(pAuto->GetStartDoubleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::pLocaleData->getDoubleQuotationMarkStart();
                if( aReplace != "\"" )
                    aString = aString.replaceAll( aReplace, "\"" );

                aReplace = OUString(pAuto->GetEndDoubleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::pLocaleData->getDoubleQuotationMarkEnd();
                if( aReplace != "\"" )
                    aString = aString.replaceAll( aReplace, "\"" );

                aReplace = OUString(pAuto->GetStartSingleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::pLocaleData->getQuotationMarkStart();
                if( aReplace != "'" )
                    aString = aString.replaceAll( aReplace, "'" );

                aReplace = OUString(pAuto->GetEndSingleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::pLocaleData->getQuotationMarkEnd();
                if( aReplace != "'" )
                    aString = aString.replaceAll( aReplace, "'");
            }
        }

        pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW_NOPAINT ) );

        if ( pExecuteSh )
        {
            SfxBindings& rBindings = pExecuteSh->GetViewFrame()->GetBindings();

            sal_uInt16 nId = FID_INPUTLINE_ENTER;
            if ( nBlockMode == SC_ENTER_BLOCK )
                nId = FID_INPUTLINE_BLOCK;
            else if ( nBlockMode == SC_ENTER_MATRIX )
                nId = FID_INPUTLINE_MATRIX;

            ScInputStatusItem aItem( FID_INPUTLINE_STATUS,
                                     aCursorPos, aCursorPos, aCursorPos,
                                     aString, pObject );

            if (!aMisspellRanges.empty())
                aItem.SetMisspellRanges(&aMisspellRanges);

            const SfxPoolItem* aArgs[2];
            aArgs[0] = &aItem;
            aArgs[1] = NULL;
            rBindings.Execute( nId, aArgs );
        }

        delete pLastState;      
        pLastState = NULL;
    }
    else
        pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

    if ( bOldMod && pExecuteSh && pCellAttrs && !bForget )
    {
        
        pExecuteSh->ApplySelectionPattern( *pCellAttrs, true, true );
        pExecuteSh->AdjustBlockHeight();
    }

    delete pCellAttrs;
    delete pObject;

    HideTip();
    HideTipBelow();

    nFormSelStart = nFormSelEnd = 0;
    aFormText = OUString();

    bInOwnChange = false;
    bInEnterHandler = false;
}

void ScInputHandler::CancelHandler()
{
    bInOwnChange = true;                

    ImplCreateEditEngine();

    bModified = false;

    
    
    
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();
        if (pExecuteSh)
        {
            pExecuteSh->SetTabNo(aCursorPos.Tab());
            pExecuteSh->ActiveGrabFocus();
        }
        bFormulaMode = false;
        SFX_APP()->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
        SC_MOD()->SetRefInputHdl(NULL);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = NULL;          
    DeleteRangeFinder();
    ResetAutoPar();

    eMode = SC_INPUT_NONE;
    StopInputWinEngine( true );
    if (pExecuteSh)
        pExecuteSh->StopEditShell();

    aCursorPos.Set(MAXCOL+1,0,0);       
    pEngine->SetText(OUString());

    if ( !pLastState && pExecuteSh )
        pExecuteSh->UpdateInputHandler( true );     
    else
        NotifyChange( pLastState, true );

    nFormSelStart = nFormSelEnd = 0;
    aFormText = OUString();

    bInOwnChange = false;
}

bool ScInputHandler::IsModalMode( SfxObjectShell* pDocSh )
{
    

    return bFormulaMode && pRefViewSh
            && pRefViewSh->GetViewData()->GetDocument()->GetDocumentShell() != pDocSh
            && !pDocSh->HasName();
}

void ScInputHandler::AddRefEntry()
{
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             

    DataChanging();                         

    RemoveSelection();
    if (pTableView)
        pTableView->InsertText( OUString(cSep), false );
    if (pTopView)
        pTopView->InsertText( OUString(cSep), false );

    DataChanged();
}

void ScInputHandler::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    HideTip();

    bool bOtherDoc = ( pRefViewSh &&
                        pRefViewSh->GetViewData()->GetDocument() != pDoc );
    if (bOtherDoc)
        if (!pDoc->GetDocumentShell()->HasName())
        {
            
            

            return;
        }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             

    
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();
    if ( aSel.nStartPara == 0 && aSel.nStartPos == 0 )
        return;

    DataChanging();                         

            

    if (pTableView)
    {
        ESelection aTabSel = pTableView->GetSelection();
        if (aTabSel.nStartPos > aTabSel.nEndPos && aTabSel.nStartPara == aTabSel.nEndPara)
        {
            aTabSel.Adjust();
            pTableView->SetSelection(aTabSel);
        }
    }
    if (pTopView)
    {
        ESelection aTopSel = pTopView->GetSelection();
        if (aTopSel.nStartPos > aTopSel.nEndPos && aTopSel.nStartPara == aTopSel.nEndPara)
        {
            aTopSel.Adjust();
            pTopView->SetSelection(aTopSel);
        }
    }

    

    OUString aRefStr;
    const ScAddress::Details aAddrDetails( pDoc, aCursorPos );
    if (bOtherDoc)
    {
        

        OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

        OUString aTmp(rRef.Format(SCA_VALID|SCA_TAB_3D, pDoc, aAddrDetails));      

        SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
        
        OUString aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

        aRefStr = "\'";
        aRefStr += aFileName;
        aRefStr += "'#";
        aRefStr += aTmp;
    }
    else
    {
        if ( ( rRef.aStart.Tab() != aCursorPos.Tab() ||
                rRef.aStart.Tab() != rRef.aEnd.Tab() ) && pDoc )
            aRefStr = rRef.Format(SCA_VALID|SCA_TAB_3D, pDoc, aAddrDetails);
        else
            aRefStr = rRef.Format(SCA_VALID, pDoc, aAddrDetails);
    }

    if (pTableView || pTopView)
    {
        if (pTableView)
            pTableView->InsertText( aRefStr, true );
        if (pTopView)
            pTopView->InsertText( aRefStr, true );

        DataChanged();
    }

    bSelIsRef = true;
}

void ScInputHandler::InsertFunction( const OUString& rFuncName, bool bAddPar )
{
    if ( eMode == SC_INPUT_NONE )
    {
        OSL_FAIL("InsertFunction, nicht im Eingabemodus");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             

    DataChanging();                         

    OUString aText = rFuncName;
    if (bAddPar)
        aText += "()";

    if (pTableView)
    {
        pTableView->InsertText( aText, false );
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
        pTopView->InsertText( aText, false );
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
        OSL_FAIL("ClearText, nicht im Eingabemodus");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             

    DataChanging();                         

    OUString aEmpty;
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

bool ScInputHandler::KeyInput( const KeyEvent& rKEvt, bool bStartEdit /* = false */ )
{
    if (!bOptLoaded)
    {
        bAutoComplete = SC_MOD()->GetAppOptions().GetAutoComplete();
        bOptLoaded = true;
    }

    KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nModi  = aCode.GetModifier();
    bool bShift   = aCode.IsShift();
    bool bControl = aCode.IsMod1();
    bool bAlt     = aCode.IsMod2();
    sal_uInt16 nCode  = aCode.GetCode();
    sal_Unicode nChar = rKEvt.GetCharCode();

    if (bAlt && !bControl && nCode != KEY_RETURN)
        
        return false;

    if (!bControl && nCode == KEY_TAB)
    {
        
        EnterHandler();

        if (pActiveViewSh)
            pActiveViewSh->FindNextUnprot( bShift );
        return true;
    }

    bool bInputLine = ( eMode==SC_INPUT_TOP );

    bool bUsed = false;
    bool bSkip = false;
    bool bDoEnter = false;

    switch ( nCode )
    {
        case KEY_RETURN:
            if (bControl && !bShift && ( !bInputLine || ( pInputWin && pInputWin->IsMultiLineInput() ) ) )
                bDoEnter = true;
            else if (nModi == 0 && nTipVisible && pFormulaData && miAutoPosFormula != pFormulaData->end())
            {
                PasteFunctionData();
                bUsed = true;
            }
            else if ( nModi == 0 && nTipVisible && !aManualTip.isEmpty() )
            {
                PasteManualTip();
                bUsed = true;
            }
            else
            {
                sal_uInt8 nMode = SC_ENTER_NORMAL;
                if ( bShift && bControl )
                    nMode = SC_ENTER_MATRIX;
                else if ( bAlt )
                    nMode = SC_ENTER_BLOCK;
                EnterHandler( nMode );

                if (pActiveViewSh)
                    pActiveViewSh->MoveCursorEnter( bShift && !bControl );

                bUsed = true;
            }
            break;
        case KEY_TAB:
            if (bControl && !bAlt)
            {
                if (pFormulaData && nTipVisible && miAutoPosFormula != pFormulaData->end())
                {
                    

                    NextFormulaEntry( bShift );
                    bUsed = true;
                }
                else if (pColumnData && bUseTab && miAutoPosColumn != pColumnData->end())
                {
                    

                    NextAutoEntry( bShift );
                    bUsed = true;
                }
            }
            break;
        case KEY_ESCAPE:
            if ( nTipVisible )
            {
                HideTip();
                bUsed = true;
            }
            else if( nTipVisibleSec )
            {
                HideTipBelow();
                bUsed = true;
            }
            else if (eMode != SC_INPUT_NONE)
            {
                CancelHandler();
                bUsed = true;
            }
            else
                bSkip = true;
            break;
        case KEY_F2:
            if ( !bShift && !bControl && !bAlt && eMode == SC_INPUT_TABLE )
            {
                eMode = SC_INPUT_TYPE;
                bUsed = true;
            }
            break;
    }

    
    

    bool bCursorKey = EditEngine::DoesKeyMoveCursor(rKEvt);
    bool bInsKey = ( nCode == KEY_INSERT && !nModi );   
    if ( !bUsed && !bSkip && ( bDoEnter || EditEngine::DoesKeyChangeText(rKEvt) ||
                    ( eMode != SC_INPUT_NONE && ( bCursorKey || bInsKey ) ) ) )
    {
        HideTip();
        HideTipBelow();

        if (bSelIsRef)
        {
            RemoveSelection();
            bSelIsRef = false;
        }

        UpdateActiveView();
        bool bNewView = DataChanging( nChar );

        if (bProtected)                             
            bUsed = true;                           
        else                                        
        {
            if (bNewView )                          
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
                UpdateActiveView();
                if (eMode==SC_INPUT_NONE)
                    if (pTableView || pTopView)
                    {
                        OUString aStrLoP;

                        if ( bStartEdit && bCellHasPercentFormat && ((nChar >= '0' && nChar <= '9') || nChar == '-') )
                            aStrLoP = "%";

                        if (pTableView)
                        {
                            pTableView->GetEditEngine()->SetText( aStrLoP );
                            if ( !aStrLoP.isEmpty() )
                                pTableView->SetSelection( ESelection(0,0, 0,0) );   

                            
                            
                        }
                        if (pTopView)
                        {
                            pTopView->GetEditEngine()->SetText( aStrLoP );
                            if ( !aStrLoP.isEmpty() )
                                pTopView->SetSelection( ESelection(0,0, 0,0) );     
                        }
                    }
                SyncViews();
            }

            if (pTableView || pTopView)
            {
                if (bDoEnter)
                {
                    if (pTableView)
                        if( pTableView->PostKeyEvent( KeyEvent( CHAR_CR, KeyCode(KEY_RETURN) ) ) )
                            bUsed = true;
                    if (pTopView)
                        if( pTopView->PostKeyEvent( KeyEvent( CHAR_CR, KeyCode(KEY_RETURN) ) ) )
                            bUsed = true;
                }
                else if ( nAutoPar && nChar == ')' && CursorAtClosingPar() )
                {
                    SkipClosingPar();
                    bUsed = true;
                }
                else
                {
                    if (pTableView)
                    {
                        Window* pFrameWin = pActiveViewSh ? pActiveViewSh->GetFrameWin() : NULL;
                        if ( pTableView->PostKeyEvent( rKEvt, pFrameWin ) )
                            bUsed = true;
                    }
                    if (pTopView)
                        if ( pTopView->PostKeyEvent( rKEvt ) )
                            bUsed = true;
                }

                

                if ( bUsed && bAutoComplete )
                {
                    bUseTab = false;
                    if (pFormulaData)
                        miAutoPosFormula = pFormulaData->end();                       
                    if (pColumnData)
                        miAutoPosColumn = pColumnData->end();

                    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
                    if ( nChar && nChar != 8 && nChar != 127 &&     
                         KEYFUNC_CUT != eFunc)                      
                    {
                        if (bFormulaMode)
                            UseFormulaData();
                        else
                            UseColData();
                    }
                }

                
                
                if ( bUsed && nChar == '(' )
                    ResetAutoPar();

                if ( KEY_INSERT == nCode )
                {
                    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                    if (pViewFrm)
                        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
                }
                if( bUsed && bFormulaMode && ( bCursorKey || bInsKey || nCode == KEY_DELETE || nCode == KEY_BACKSPACE ) )
                {
                    ShowTipCursor();
                }
            }

            
            sal_Bool bSetModified = !bCursorKey;
            DataChanged(false, bSetModified);  
            InvalidateAttribs();        
        }
    }

    if (pTopView && eMode != SC_INPUT_NONE)
        SyncViews();

    return bUsed;
}

bool ScInputHandler::InputCommand( const CommandEvent& rCEvt, bool bForce )
{
    bool bUsed = false;

    if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        
        
        if ( eMode != SC_INPUT_NONE )
        {
            UpdateActiveView();
            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                else if (pTopView)                      
                    pTopView->Command( rCEvt );
                bUsed = true;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_QUERYCHARPOSITION )
    {
        if ( eMode != SC_INPUT_NONE )
        {
            UpdateActiveView();
            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                else if (pTopView)                      
                    pTopView->Command( rCEvt );
                bUsed = true;
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
                bOptLoaded = true;
            }

            HideTip();
            HideTipBelow();

            if ( bSelIsRef )
            {
                RemoveSelection();
                bSelIsRef = false;
            }

            UpdateActiveView();
            bool bNewView = DataChanging( 0, true );

            if (bProtected)                             
                bUsed = true;                           
            else                                        
            {
                if (bNewView)                           
                {
                    if (pActiveViewSh)
                        pActiveViewSh->GetViewData()->GetDocShell()->PostEditView( pEngine, aCursorPos );
                    UpdateActiveView();
                    if (eMode==SC_INPUT_NONE)
                        if (pTableView || pTopView)
                        {
                            OUString aStrLoP;
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

                    bUsed = true;

                    if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
                    {
                        

                        if (pFormulaData)
                            miAutoPosFormula = pFormulaData->end();
                        if (pColumnData)
                            miAutoPosColumn = pColumnData->end();

                        if (bFormulaMode)
                            UseFormulaData();
                        else
                            UseColData();
                    }
                }

                DataChanged();              
                InvalidateAttribs();        
            }
        }

        if (pTopView && eMode != SC_INPUT_NONE)
            SyncViews();
    }

    return bUsed;
}

void ScInputHandler::NotifyChange( const ScInputHdlState* pState,
                                   bool bForce, ScTabViewShell* pSourceSh,
                                   bool bStopEditing)
{
    
    
    if (bInEnterHandler)
        return;

    bool bRepeat = (pState == pLastState);
    if (!bRepeat && pState && pLastState)
        bRepeat = (*pState == *pLastState);
    if (bRepeat && !bForce)
        return;

    bInOwnChange = true;                

    if ( pState && !pLastState )        
        bForce = true;

    bool bHadObject = pLastState && pLastState->GetEditData();

    
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

            
            

            if ( !bFormulaMode && !pScMod->IsFormulaMode() && !pScMod->GetFormEditData() )
            {
                bool bIgnore = false;
                if ( bModified )
                {
                    if (pState->GetPos() != aCursorPos)
                    {
                        if (!bProtected)
                            EnterHandler();
                    }
                    else
                        bIgnore = true;
                }

                if ( !bIgnore )
                {
                    const ScAddress&        rSPos   = pState->GetStartPos();
                    const ScAddress&        rEPos   = pState->GetEndPos();
                    const EditTextObject*   pData   = pState->GetEditData();
                    OUString aString = pState->GetString();
                    bool bTxtMod = false;
                    ScDocShell* pDocSh = pActiveViewSh->GetViewData()->GetDocShell();
                    ScDocument* pDoc = pDocSh->GetDocument();

                    aCursorPos  = pState->GetPos();

                    if ( pData )
                        bTxtMod = true;
                    else if ( bHadObject )
                        bTxtMod = true;
                    else if ( bTextValid )
                        bTxtMod = ( !aString.equals(aCurrentText) );
                    else
                        bTxtMod = ( !aString.equals(GetEditText(pEngine)) );

                    if ( bTxtMod || bForce )
                    {
                        if (pData)
                        {
                            pEngine->SetText( *pData );
                            if ( pInputWin && pInputWin->IsMultiLineInput() )
                                aString = ScEditUtil::GetMultilineString(*pEngine);
                            else
                                aString = GetEditText(pEngine);
                            lcl_RemoveTabs(aString);
                            bTextValid = false;
                            aCurrentText = OUString();
                        }
                        else
                        {
                            aCurrentText = aString;
                            bTextValid = true;              
                        }

                        if ( pInputWin )
                            pInputWin->SetTextString(aString);
                    }

                    if ( pInputWin )                        
                    {
                        OUString aPosStr;
                        const ScAddress::Details aAddrDetails( pDoc, aCursorPos );

                        
                        

                        if ( pActiveViewSh )
                            pActiveViewSh->GetViewData()->GetDocument()->
                                GetRangeAtBlock( ScRange( rSPos, rEPos ), &aPosStr );

                        if ( aPosStr.isEmpty() )           
                        {
                            sal_uInt16 nFlags = 0;
                            if( aAddrDetails.eConv == formula::FormulaGrammar::CONV_XL_R1C1 )
                                nFlags |= SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE;
                            if ( rSPos != rEPos )
                            {
                                ScRange r(rSPos, rEPos);
                                nFlags |= (nFlags << 4);
                                aPosStr = r.Format(SCA_VALID | nFlags, pDoc, aAddrDetails);
                            }
                            else
                                aPosStr = aCursorPos.Format(SCA_VALID | nFlags, pDoc, aAddrDetails);
                        }

                        
                        bool bIsSuppressed = pInputWin->IsAccessibilityEventsSuppressed(false);
                        pInputWin->SetAccessibilityEventsSuppressed(true);
                        pInputWin->SetPosString(aPosStr);
                        pInputWin->SetAccessibilityEventsSuppressed(bIsSuppressed);
                        pInputWin->SetSumAssignMode();
                    }

                    if (bStopEditing)
                        SFX_APP()->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

                    
                    
                    

                    sal_uLong nCntrl = pEngine->GetControlWord();
                    if ( nCntrl & EE_CNTRL_ONLINESPELLING )
                        pEngine->SetControlWord( nCntrl & ~EE_CNTRL_ONLINESPELLING );

                    bModified = false;
                    bSelIsRef = false;
                    bProtected = false;
                    bCommandErrorShown = false;
                }
            }
        }

        if ( pInputWin)
        {
            if(!pScMod->IsFormulaMode()&& !pScMod->IsRefDialogOpen())   
            {                                                           
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
            {                                   
                if ( !pDelayTimer )             
                {                               
                    pDelayTimer = new Timer;
                    pDelayTimer->SetTimeout( 500 ); 
                    pDelayTimer->SetTimeoutHdl( LINK( this, ScInputHandler, DelayTimer ) );
                    pDelayTimer->Start();
                }
            }
        }
    }
    else 
    {
        if ( !pDelayTimer )
        {
            pDelayTimer = new Timer;
            pDelayTimer->SetTimeout( 500 ); 
            pDelayTimer->SetTimeoutHdl( LINK( this, ScInputHandler, DelayTimer ) );
            pDelayTimer->Start();
        }
    }

    HideTip();
    HideTipBelow();
    bInOwnChange = false;
}

void ScInputHandler::UpdateCellAdjust( SvxCellHorJustify eJust )
{
    eAttrAdjust = eJust;
    UpdateAdjust( 0 );
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
            

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if ( pViewFrm && pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
            {
                if ( pInputWin)
                {
                    pInputWin->EnableButtons( false );
                    pInputWin->Disable();
                }
            }
            else if ( !bFormulaMode )   
            {
                bInOwnChange = true;    

                pActiveViewSh = NULL;
                pEngine->SetText( EMPTY_OUSTRING );
                if ( pInputWin )
                {
                    pInputWin->SetPosString( EMPTY_OUSTRING );
                    pInputWin->SetTextString( EMPTY_OUSTRING );
                    pInputWin->Disable();
                }

                bInOwnChange = false;
            }
        }
    }
    return 0;
}

void ScInputHandler::InputSelection( EditView* pView )
{
    SyncViews( pView );
    ShowTipCursor();
    UpdateParenthesis();    

    
    ResetAutoPar();
}

void ScInputHandler::InputChanged( EditView* pView, bool bFromNotify )
{
    UpdateActiveView();

    
    bool bFromTopNotify = ( bFromNotify && pView == pTopView );

    bool bNewView = DataChanging();                     
    aCurrentText = pView->GetEditEngine()->GetText();   
    pEngine->SetText( aCurrentText );
    DataChanged( bFromTopNotify );
    bTextValid = true;      

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

const OUString& ScInputHandler::GetEditString()
{
    if (pEngine)
    {
        aCurrentText = pEngine->GetText();      
        bTextValid = true;
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

bool ScInputHandler::GetTextAndFields( ScEditEngineDefaulter& rDestEngine )
{
    bool bRet = false;
    if (pEngine)
    {
        

        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        SfxItemSet aSet = pEngine->GetAttribs( ESelection(0,0,nParCnt,0) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, false );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
        {
            

            EditTextObject* pObj = pEngine->CreateTextObject();
            rDestEngine.SetText(*pObj);
            delete pObj;

            

            for (sal_Int32 i=0; i<nParCnt; i++)
                rDestEngine.QuickRemoveCharAttribs( i );

            

            while ( nParCnt > 1 )
            {
                sal_Int32 nLen = rDestEngine.GetTextLen( 0 );
                ESelection aSel( 0,nLen, 1,0 );
                rDestEngine.QuickInsertText( OUString(' '), aSel );       
                --nParCnt;
            }

            bRet = true;
        }
    }
    return bRet;
}






void ScInputHandler::InputGetSelection( sal_Int32& rStart, sal_Int32& rEnd )
{
    rStart = nFormSelStart;
    rEnd = nFormSelEnd;
}



EditView* ScInputHandler::GetFuncEditView()
{
    UpdateActiveView();     

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
            bCreatingFuncView = true;       
            SetMode( SC_INPUT_TABLE );
            bCreatingFuncView = false;
            if ( pTableView )
                pTableView->GetEditEngine()->SetText( EMPTY_OUSTRING );
        }
        pView = pTableView;
    }

    return pView;
}



void ScInputHandler::InputSetSelection( sal_Int32 nStart, sal_Int32 nEnd )
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

    bModified = true;
}



void ScInputHandler::InputReplaceSelection( const OUString& rStr )
{
    if (!pRefViewSh)
        pRefViewSh = pActiveViewSh;

    OSL_ENSURE(nFormSelEnd>=nFormSelStart,"Selektion kaputt...");

    sal_Int32 nOldLen = nFormSelEnd - nFormSelStart;
    sal_Int32 nNewLen = rStr.getLength();

    OUStringBuffer aBuf(aFormText);
    if (nOldLen)
        aBuf.remove(nFormSelStart, nOldLen);
    if (nNewLen)
        aBuf.insert(nFormSelStart, rStr);

    aFormText = aBuf.makeStringAndClear();

    nFormSelEnd = nFormSelStart + nNewLen;

    EditView* pView = GetFuncEditView();
    if (pView)
    {
        pView->SetEditEngineUpdateMode( false );
        pView->GetEditEngine()->SetText( aFormText );
        pView->SetSelection( ESelection(0,nFormSelStart, 0,nFormSelEnd) );
        pView->SetEditEngineUpdateMode( true );
    }
    bModified = true;
}

void ScInputHandler::InputTurnOffWinEngine()
{
    bInOwnChange = true;                

    eMode = SC_INPUT_NONE;
    /* TODO: it would be better if there was some way to reset the input bar
     * engine instead of deleting and having it recreate through
     * GetFuncEditView(), but first least invasively let this fix fdo#71667 and
     * fdo#72278 without reintroducing fdo#69971. */
    StopInputWinEngine(true);

    bInOwnChange = false;
}





ScInputHdlState::ScInputHdlState( const ScAddress& rCurPos,
                                  const ScAddress& rStartPos,
                                  const ScAddress& rEndPos,
                                  const OUString& rString,
                                  const EditTextObject* pData )
    :   aCursorPos  ( rCurPos ),
        aStartPos   ( rStartPos ),
        aEndPos     ( rEndPos ),
        aString     ( rString ),
        pEditData   ( pData ? pData->Clone() : NULL )
{
}



ScInputHdlState::ScInputHdlState( const ScInputHdlState& rCpy )
    :   pEditData   ( NULL )
{
    *this = rCpy;
}



ScInputHdlState::~ScInputHdlState()
{
    delete pEditData;
}



bool ScInputHdlState::operator==( const ScInputHdlState& r ) const
{
    return (    (aStartPos  == r.aStartPos)
             && (aEndPos    == r.aEndPos)
             && (aCursorPos == r.aCursorPos)
             && (aString    == r.aString)
             && ScGlobal::EETextObjEqual( pEditData, r.pEditData ) );
}



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
