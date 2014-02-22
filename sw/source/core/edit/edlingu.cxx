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

#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <comphelper/string.hxx>

#include <unoflatpara.hxx>

#include <comcore.hrc>
#include <hintids.hxx>
#include <linguistic/lngprops.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <editeng/SpellPortions.hxx>
#include <editeng/scripttypeitem.hxx>
#include <charatr.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <viewopt.hxx>
#include <viscrs.hxx>
#include <SwGrammarMarkUp.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <cntfrm.hxx>
#include <crsskip.hxx>
#include <splargs.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <txatbase.hxx>
#include <txtfrm.hxx>

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

class SwLinguIter
{
    SwEditShell *pSh;
    SwPosition  *pStart;
    SwPosition  *pEnd;
    SwPosition  *pCurr;
    SwPosition  *pCurrX;
    sal_uInt16 nCrsrCnt;
public:
    SwLinguIter();

    inline SwEditShell *GetSh()             { return pSh; }
    inline const SwEditShell *GetSh() const { return pSh; }

    inline const SwPosition *GetEnd() const { return pEnd; }
    inline void SetEnd( SwPosition* pNew ){ delete pEnd; pEnd = pNew; }

    inline const SwPosition *GetStart() const { return pStart; }
    inline void SetStart( SwPosition* pNew ){ delete pStart; pStart = pNew; }

    inline const SwPosition *GetCurr() const { return pCurr; }
    inline void SetCurr( SwPosition* pNew ){ delete pCurr; pCurr = pNew; }

    inline const SwPosition *GetCurrX() const { return pCurrX; }
    inline void SetCurrX( SwPosition* pNew ){ delete pCurrX; pCurrX = pNew; }

    inline sal_uInt16& GetCrsrCnt(){ return nCrsrCnt; }

    
    void _Start( SwEditShell *pSh, SwDocPositions eStart,
                SwDocPositions eEnd );
    void _End(bool bRestoreSelection = true);
};



struct SpellContentPosition
{
    sal_Int32 nLeft;
    sal_Int32 nRight;
};

typedef std::vector<SpellContentPosition>  SpellContentPositions;

class SwSpellIter : public SwLinguIter
{
    uno::Reference< XSpellChecker1 >    xSpeller;
    ::svx::SpellPortions                aLastPortions;

    SpellContentPositions               aLastPositions;
    bool                                bBackToStartOfSentence;
    bool                                bMoveToEndOfSentence;


    void    CreatePortion(uno::Reference< XSpellAlternatives > xAlt,
                linguistic2::ProofreadingResult* pGrammarResult,
                bool bIsField, bool bIsHidden);

    void    AddPortion(uno::Reference< XSpellAlternatives > xAlt,
                       linguistic2::ProofreadingResult* pGrammarResult,
                       const SpellContentPositions& rDeletedRedlines);
public:
    SwSpellIter() :
        bBackToStartOfSentence(false), bMoveToEndOfSentence(false) {}

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    bool                                SpellSentence(::svx::SpellPortions& rPortions, bool bIsGrammarCheck);
    void                                ToSentenceStart();
    const ::svx::SpellPortions          GetLastPortions() const { return aLastPortions;}
    SpellContentPositions               GetLastPositions() const {return aLastPositions;}
    void                                ContinueAfterThisSentence() { bMoveToEndOfSentence = true; }
};


class SwConvIter : public SwLinguIter
{
    SwConversionArgs &rArgs;
public:
    SwConvIter( SwConversionArgs &rConvArgs ) :
        rArgs( rConvArgs )
    {}

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );
};

class SwHyphIter : public SwLinguIter
{
    sal_Bool bOldIdle;
    void DelSoftHyph( SwPaM &rPam );

public:
    SwHyphIter() : bOldIdle(sal_False) {}

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );
    void End();

    void Ignore();

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    sal_Bool IsAuto();
    void InsertSoftHyph( const sal_Int32 nHyphPos );
    void ShowSelection();
};

static SwSpellIter* pSpellIter = 0;
static SwConvIter*  pConvIter = 0;
static SwHyphIter*  pHyphIter = 0;



const SwTxtNode *pLinguNode;
      SwTxtFrm  *pLinguFrm;

SwLinguIter::SwLinguIter()
    : pSh( 0 ), pStart( 0 ), pEnd( 0 ), pCurr( 0 ), pCurrX( 0 )
{
    
}

void SwLinguIter::_Start( SwEditShell *pShell, SwDocPositions eStart,
                            SwDocPositions eEnd )
{
    
    if( pSh )
        return;

    bool bSetCurr;

    pSh = pShell;

    SET_CURR_SHELL( pSh );

    OSL_ENSURE( !pEnd, "SwLinguIter::_Start without End?");

    SwPaM *pCrsr = pSh->GetCrsr();

    if( pShell->HasSelection() || pCrsr != pCrsr->GetNext() )
    {
        bSetCurr = 0 != GetCurr();
        nCrsrCnt = pSh->GetCrsrCnt();
        if( pSh->IsTableMode() )
            pSh->TblCrsrToCursor();

        pSh->Push();
        sal_uInt16 n;
        for( n = 0; n < nCrsrCnt; ++n )
        {
            pSh->Push();
            pSh->DestroyCrsr();
        }
        pSh->Pop( sal_False );
    }
    else
    {
        bSetCurr = false;
        nCrsrCnt = 1;
        pSh->Push();
        pSh->SetLinguRange( eStart, eEnd );
    }

    pCrsr = pSh->GetCrsr();
    if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
        pCrsr->Exchange();

    pStart = new SwPosition( *pCrsr->GetPoint() );
    pEnd = new SwPosition( *pCrsr->GetMark() );
    if( bSetCurr )
    {
        SwPosition* pNew = new SwPosition( *GetStart() );
        SetCurr( pNew );
        pNew = new SwPosition( *pNew );
        SetCurrX( pNew );
    }

    pCrsr->SetMark();

    pLinguFrm = 0;
    pLinguNode = 0;
}

void SwLinguIter::_End(bool bRestoreSelection)
{
    if( !pSh )
        return;

    OSL_ENSURE( pEnd, "SwLinguIter::_End without end?");
    if(bRestoreSelection)
    {
        while( nCrsrCnt-- )
            pSh->Pop( sal_False );

        pSh->KillPams();
        pSh->ClearMark();
    }
    DELETEZ(pStart);
    DELETEZ(pEnd);
    DELETEZ(pCurr);
    DELETEZ(pCurrX);

    pSh = 0;
}

void SwSpellIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;

    xSpeller = ::GetSpellChecker();
    if ( xSpeller.is() )
        _Start( pShell, eStart, eEnd );
    aLastPortions.clear();
    aLastPositions.clear();
}


uno::Any SwSpellIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    
    
    

    uno::Any    aSpellRet;
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aSpellRet;

    OSL_ENSURE( GetEnd(), "SwSpellIter::Continue without start?");

    uno::Reference< uno::XInterface >  xSpellRet;
    bool bGoOn = true;
    do {
        SwPaM *pCrsr = pMySh->GetCrsr();
        if ( !pCrsr->HasMark() )
            pCrsr->SetMark();

        uno::Reference< beans::XPropertySet >  xProp( GetLinguPropertySet() );
        *pMySh->GetCrsr()->GetPoint() = *GetCurr();
        *pMySh->GetCrsr()->GetMark() = *GetEnd();
        pMySh->GetDoc()->Spell(*pMySh->GetCrsr(),
                    xSpeller, pPageCnt, pPageSt, false ) >>= xSpellRet;
        bGoOn = GetCrsrCnt() > 1;
        if( xSpellRet.is() )
        {
            bGoOn = false;
            SwPosition* pNewPoint = new SwPosition( *pCrsr->GetPoint() );
            SwPosition* pNewMark = new SwPosition( *pCrsr->GetMark() );
            SetCurr( pNewPoint );
            SetCurrX( pNewMark );
        }
        if( bGoOn )
        {
            pMySh->Pop( sal_False );
            pCrsr = pMySh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition( *pCrsr->GetPoint() );
            SetStart( pNew );
            pNew = new SwPosition( *pCrsr->GetMark() );
            SetEnd( pNew );
            pNew = new SwPosition( *GetStart() );
            SetCurr( pNew );
            pNew = new SwPosition( *pNew );
            SetCurrX( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    }while ( bGoOn );
    aSpellRet <<= xSpellRet;
    return aSpellRet;
}

void SwConvIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;
    _Start( pShell, eStart, eEnd );
}

uno::Any SwConvIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    
    
    

    uno::Any    aConvRet( makeAny( OUString() ) );
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aConvRet;

    OSL_ENSURE( GetEnd(), "SwConvIter::Continue() without Start?");

    OUString aConvText;
    bool bGoOn = true;
    do {
        SwPaM *pCrsr = pMySh->GetCrsr();
        if ( !pCrsr->HasMark() )
            pCrsr->SetMark();

        *pMySh->GetCrsr()->GetPoint() = *GetCurr();
        *pMySh->GetCrsr()->GetMark() = *GetEnd();

        
        uno::Reference< linguistic2::XSpellChecker1 > xEmpty;
        pMySh->GetDoc()->Spell( *pMySh->GetCrsr(),
                    xEmpty, pPageCnt, pPageSt, false, &rArgs ) >>= aConvText;

        bGoOn = GetCrsrCnt() > 1;
        if( !aConvText.isEmpty() )
        {
            bGoOn = false;
            SwPosition* pNewPoint = new SwPosition( *pCrsr->GetPoint() );
            SwPosition* pNewMark = new SwPosition( *pCrsr->GetMark() );

            SetCurr( pNewPoint );
            SetCurrX( pNewMark );
        }
        if( bGoOn )
        {
            pMySh->Pop( sal_False );
            pCrsr = pMySh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition( *pCrsr->GetPoint() );
            SetStart( pNew );
            pNew = new SwPosition( *pCrsr->GetMark() );
            SetEnd( pNew );
            pNew = new SwPosition( *GetStart() );
            SetCurr( pNew );
            pNew = new SwPosition( *pNew );
            SetCurrX( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    }while ( bGoOn );
    return makeAny( aConvText );
}

sal_Bool SwHyphIter::IsAuto()
{
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    return xProp.is() ? *(sal_Bool*)xProp->getPropertyValue(
                                OUString(UPN_IS_HYPH_AUTO) ).getValue()
                      : sal_False;
}

void SwHyphIter::ShowSelection()
{
    SwEditShell *pMySh = GetSh();
    if( pMySh )
    {
        pMySh->StartAction();
        
        
        pMySh->EndAction();
    }
}

void SwHyphIter::Start( SwEditShell *pShell, SwDocPositions eStart, SwDocPositions eEnd )
{
    
    if( GetSh() || GetEnd() )
    {
        OSL_ENSURE( !GetSh(), "SwHyphIter::Start: missing HyphEnd()" );
        return;
    }

    
    bOldIdle = pShell->GetViewOptions()->IsIdle();
    ((SwViewOption*)pShell->GetViewOptions())->SetIdle( sal_False );
    _Start( pShell, eStart, eEnd );
}


void SwHyphIter::End()
{
    if( !GetSh() )
        return;
    ((SwViewOption*)GetSh()->GetViewOptions())->SetIdle( bOldIdle );
    _End();
}

uno::Any SwHyphIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    uno::Any    aHyphRet;
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aHyphRet;

    const bool bAuto = IsAuto();
     uno::Reference< XHyphenatedWord >  xHyphWord;
    bool bGoOn = false;
    do {
        SwPaM *pCrsr;
        do {
            OSL_ENSURE( GetEnd(), "SwHyphIter::Continue without Start?" );
            pCrsr = pMySh->GetCrsr();
            if ( !pCrsr->HasMark() )
                pCrsr->SetMark();
            if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
            {
                pCrsr->Exchange();
                pCrsr->SetMark();
            }

            if ( *pCrsr->End() <= *GetEnd() )
            {
                *pCrsr->GetMark() = *GetEnd();

                
                const Point aCrsrPos( pMySh->GetCharRect().Pos() );
                xHyphWord = pMySh->GetDoc()->Hyphenate( pCrsr, aCrsrPos,
                                                       pPageCnt, pPageSt );
            }

            if( bAuto && xHyphWord.is() )
            {
                pMySh->InsertSoftHyph( xHyphWord->getHyphenationPos() + 1);
            }
        } while( bAuto && xHyphWord.is() ); 
        bGoOn = !xHyphWord.is() && GetCrsrCnt() > 1;

        if( bGoOn )
        {
            pMySh->Pop( sal_False );
            pCrsr = pMySh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition(*pCrsr->End());
            SetEnd( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    } while ( bGoOn );
    aHyphRet <<= xHyphWord;
    return aHyphRet;
}


void SwHyphIter::Ignore()
{
    SwEditShell *pMySh = GetSh();
    SwPaM *pCrsr = pMySh->GetCrsr();

    
    DelSoftHyph( *pCrsr );

    
    pCrsr->Start()->nContent = pCrsr->End()->nContent;
    pCrsr->SetMark();
}

void SwHyphIter::DelSoftHyph( SwPaM &rPam )
{
    const SwPosition* pStt = rPam.Start();
    const sal_Int32 nStart = pStt->nContent.GetIndex();
    const sal_Int32 nEnd   = rPam.End()->nContent.GetIndex();
    SwTxtNode *pNode = pStt->nNode.GetNode().GetTxtNode();
    pNode->DelSoftHyph( nStart, nEnd );
}

void SwHyphIter::InsertSoftHyph( const sal_Int32 nHyphPos )
{
    SwEditShell *pMySh = GetSh();
    OSL_ENSURE( pMySh,  "SwHyphIter::InsertSoftHyph: missing HyphStart()");
    if( !pMySh )
        return;

    SwPaM *pCrsr = pMySh->GetCrsr();
    SwPosition* pSttPos = pCrsr->Start();
    SwPosition* pEndPos = pCrsr->End();

    const sal_Int32 nLastHyphLen = GetEnd()->nContent.GetIndex() -
                          pSttPos->nContent.GetIndex();

    if( pSttPos->nNode != pEndPos->nNode || !nLastHyphLen )
    {
        OSL_ENSURE( pSttPos->nNode == pEndPos->nNode,
                "SwHyphIter::InsertSoftHyph: node warp during hyphenation" );
        OSL_ENSURE(nLastHyphLen, "SwHyphIter::InsertSoftHyph: missing HyphContinue()");
        *pSttPos = *pEndPos;
        return;
    }

    pMySh->StartAction();
    {
        SwDoc *pDoc = pMySh->GetDoc();
        DelSoftHyph( *pCrsr );
        pSttPos->nContent += nHyphPos;
        SwPaM aRg( *pSttPos );
        pDoc->InsertString( aRg, OUString(CHAR_SOFTHYPHEN) );
    }
    
    pCrsr->DeleteMark();
    pMySh->EndAction();
    pCrsr->SetMark();
}

bool SwEditShell::HasLastSentenceGotGrammarChecked() const
{
    bool bTextWasGrammarChecked = false;
    if (pSpellIter)
    {
        ::svx::SpellPortions aLastPortions( pSpellIter->GetLastPortions() );
        for (size_t i = 0;  i < aLastPortions.size() && !bTextWasGrammarChecked;  ++i)
        {
            
            
            
            if (aLastPortions[i].bIsGrammarError)
                bTextWasGrammarChecked = true;
        }
    }
    return bTextWasGrammarChecked;
}

sal_Bool SwEditShell::HasConvIter() const
{
    return 0 != pConvIter;
}

sal_Bool SwEditShell::HasHyphIter() const
{
    return 0 != pHyphIter;
}

void SwEditShell::SetLinguRange( SwDocPositions eStart, SwDocPositions eEnd )
{
    SwPaM *pCrsr = GetCrsr();
    MakeFindRange( static_cast<sal_uInt16>(eStart), static_cast<sal_uInt16>(eEnd), pCrsr );
    if( *pCrsr->GetPoint() > *pCrsr->GetMark() )
        pCrsr->Exchange();
}

void SwEditShell::SpellStart(
        SwDocPositions eStart, SwDocPositions eEnd, SwDocPositions eCurr,
        SwConversionArgs *pConvArgs )
{
    SwLinguIter *pLinguIter = 0;

    
    if (!pConvArgs && !pSpellIter)
    {
        OSL_ENSURE( !pSpellIter, "wer ist da schon am spellen?" );
        pSpellIter = new SwSpellIter;
        pLinguIter = pSpellIter;
    }
    
    if (pConvArgs && !pConvIter)
    {
        OSL_ENSURE( !pConvIter, "text conversion already active!" );
        pConvIter = new SwConvIter( *pConvArgs );
        pLinguIter = pConvIter;
    }

    if (pLinguIter)
    {
        SwCursor* pSwCrsr = GetSwCrsr();

        SwPosition *pTmp = new SwPosition( *pSwCrsr->GetPoint() );
        pSwCrsr->FillFindPos( eCurr, *pTmp );
        pLinguIter->SetCurr( pTmp );

        pTmp = new SwPosition( *pTmp );
        pLinguIter->SetCurrX( pTmp );
    }

    if (!pConvArgs && pSpellIter)
        pSpellIter->Start( this, eStart, eEnd );
    if (pConvArgs && pConvIter)
        pConvIter->Start( this, eStart, eEnd );
}

void SwEditShell::SpellEnd( SwConversionArgs *pConvArgs, bool bRestoreSelection )
{
    if (!pConvArgs && pSpellIter && pSpellIter->GetSh() == this)
    {
        OSL_ENSURE( pSpellIter, "wo ist mein Iterator?" );
        pSpellIter->_End(bRestoreSelection);
        delete pSpellIter, pSpellIter = 0;
    }
    if (pConvArgs && pConvIter && pConvIter->GetSh() == this)
    {
        OSL_ENSURE( pConvIter, "wo ist mein Iterator?" );
        pConvIter->_End();
        delete pConvIter, pConvIter = 0;
    }
}


uno::Any SwEditShell::SpellContinue(
        sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
        SwConversionArgs *pConvArgs )
{
    uno::Any aRes;

    if ((!pConvArgs && pSpellIter->GetSh() != this) ||
        ( pConvArgs && pConvIter->GetSh() != this))
        return aRes;

    if( pPageCnt && !*pPageCnt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        *pPageCnt = nEndPage;
        if( nEndPage )
            ::StartProgress( STR_STATSTR_SPELL, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    OSL_ENSURE(  pConvArgs || pSpellIter, "SpellIter missing" );
    OSL_ENSURE( !pConvArgs || pConvIter,  "ConvIter missing" );
    
    
    ++mnStartAction;
    OUString aRet;
    uno::Reference< uno::XInterface >  xRet;
    if (pConvArgs)
    {
        pConvIter->Continue( pPageCnt, pPageSt ) >>= aRet;
        aRes <<= aRet;
    }
    else
    {
        pSpellIter->Continue( pPageCnt, pPageSt ) >>= xRet;
        aRes <<= xRet;
    }
    --mnStartAction;

    if( !aRet.isEmpty() || xRet.is() )
    {
        
        StartAction();
        EndAction();
    }
    return aRes;
}

/* Interactive Hyphenation (BP 10.03.93)
 *
 * 1) HyphStart
 *    - Revoke all Selections
 *    - Save current Cursor
 *    - if no selections existant:
 *      - create new selection reaching until document end
 * 2) HyphContinue
 *    - add nLastHyphLen onto SelectionStart
 *    - iterate over all selected areas
 *      - pDoc->Hyphenate() iterates over all Nodes of a selection
 *          - pTxtNode->Hyphenate() calls SwTxtFrm::Hyphenate of the EditShell
 *              - SwTxtFrm:Hyphenate() iterates over all rows of the Pam
 *                  - LineIter::Hyphenate() sets the Hyphenator and the Pam based on
 *                    the to be separated word.
 *    - Returns sal_True if there is a hyphenation and sal_False if the Pam is processed.
 *      - If sal_True, show the selected word and set nLastHyphLen.
 *      - If sal_False, delete current selection and select next one. Returns HYPH_OK if no more.
 * 3) InsertSoftHyph (might be called by UI if needed)
 *    - Place current cursor and add attribute.
 * 4) HyphEnd
 *    - Restore old cursor, EndAction
 */
void SwEditShell::HyphStart( SwDocPositions eStart, SwDocPositions eEnd )
{
    
    if (!pHyphIter)
    {
        OSL_ENSURE( !pHyphIter, "wer ist da schon am hyphinieren?" );
        pHyphIter = new SwHyphIter;
        pHyphIter->Start( this, eStart, eEnd );
    }
}


void SwEditShell::HyphEnd()
{
    if (pHyphIter->GetSh() == this)
    {
        OSL_ENSURE( pHyphIter, "No Iterator" );
        pHyphIter->End();
        delete pHyphIter, pHyphIter = 0;
    }
}


uno::Reference< uno::XInterface >
    SwEditShell::HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    if (pHyphIter->GetSh() != this)
        return 0;

    if( pPageCnt && !*pPageCnt && !*pPageSt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        if( nEndPage > 14 )
        {
            *pPageCnt = nEndPage;
            ::StartProgress( STR_STATSTR_HYPHEN, 0, nEndPage, GetDoc()->GetDocShell());
        }
        else                
            *pPageSt = 1;
    }

    OSL_ENSURE( pHyphIter, "No Iterator" );
    
    
    ++mnStartAction;
    uno::Reference< uno::XInterface >  xRet;
    pHyphIter->Continue( pPageCnt, pPageSt ) >>= xRet;
    --mnStartAction;

    if( xRet.is() )
        pHyphIter->ShowSelection();

    return xRet;
}


/** Insert soft hyphen
 *
 * @param nHyphPos Offset in the to be separated word
 */
void SwEditShell::InsertSoftHyph( const sal_Int32 nHyphPos )
{
    OSL_ENSURE( pHyphIter, "wo ist mein Iterator?" );
    pHyphIter->InsertSoftHyph( nHyphPos );
}


void SwEditShell::HyphIgnore()
{
    OSL_ENSURE( pHyphIter, "No Iterator" );
    
    
    ++mnStartAction;
    pHyphIter->Ignore();
    --mnStartAction;

    pHyphIter->ShowSelection();
}

/** Get a list of potential corrections for misspelled word.
 *
 * If empty, word is unknown but there are no corrections available.
 * If NULL then the word is not misspelled but correct.
 *
 * @brief SwEditShell::GetCorrection
 * @return list or NULL pointer
 */
uno::Reference< XSpellAlternatives >
    SwEditShell::GetCorrection( const Point* pPt, SwRect& rSelectRect )
{
     uno::Reference< XSpellAlternatives >  xSpellAlt;

    if( IsTableMode() )
        return NULL;
    SwPaM* pCrsr = GetCrsr();
    SwPosition aPos( *pCrsr->GetPoint() );
     Point aPt( *pPt );
    SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
    SwTxtNode *pNode;
    SwWrongList *pWrong;
    if( GetLayout()->GetCrsrOfst( &aPos, aPt, &eTmpState ) &&
        0 != (pNode = aPos.nNode.GetNode().GetTxtNode()) &&
        0 != (pWrong = pNode->GetWrong()) &&
        !pNode->IsInProtectSect() )
    {
        sal_Int32 nBegin = aPos.nContent.GetIndex();
        sal_Int32 nLen = 1;
        if( pWrong->InWrongWord(nBegin,nLen) && !pNode->IsSymbol(nBegin) )
        {
            const OUString aText(pNode->GetTxt().copy(nBegin, nLen));
            OUString aWord( aText );
            aWord = comphelper::string::remove(aWord, CH_TXTATR_BREAKWORD);
            aWord = comphelper::string::remove(aWord, CH_TXTATR_INWORD);

            uno::Reference< XSpellChecker1 >  xSpell( ::GetSpellChecker() );
            if( xSpell.is() )
            {
                LanguageType eActLang = (LanguageType)pNode->GetLang( nBegin, nLen );
                if( xSpell->hasLanguage( eActLang ))
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    Sequence< PropertyValue > aPropVals(1);
                    PropertyValue &rVal = aPropVals.getArray()[0];
                    rVal.Name = OUString( UPN_MAX_NUMBER_OF_SUGGESTIONS );
                    rVal.Value <<= (sal_Int16) 7;

                    xSpellAlt = xSpell->spell( aWord, eActLang, aPropVals );
                }
            }

            if ( xSpellAlt.is() )   
            {
                
                Push();
                LeftMargin();
                const sal_Int32 nLineStart = GetCrsr()->GetPoint()->nContent.GetIndex();
                RightMargin();
                const sal_Int32 nLineEnd = GetCrsr()->GetPoint()->nContent.GetIndex();
                Pop(sal_False);

                
                
                
                
                const sal_Unicode* pChar = aText.getStr();
                sal_Int32 nLeft = 0;
                while (pChar && *pChar++ == CH_TXTATR_INWORD)
                    ++nLeft;
                pChar = aText.getLength() ? aText.getStr() + aText.getLength() - 1 : 0;
                sal_Int32 nRight = 0;
                while (pChar && *pChar-- == CH_TXTATR_INWORD)
                    ++nRight;

                aPos.nContent = nBegin + nLeft;
                pCrsr = GetCrsr();
                *pCrsr->GetPoint() = aPos;
                pCrsr->SetMark();
                ExtendSelection( sal_True, nLen - nLeft - nRight );
                
                const sal_Int32 nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
                
                const sal_Int32 nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd
                                        ? nLineEnd : (nBegin + nLen - nLeft - nRight);
                Push();
                pCrsr->DeleteMark();
                SwIndex& rContent = GetCrsr()->GetPoint()->nContent;
                rContent = nWordStart;
                SwRect aStartRect;
                SwCrsrMoveState aState;
                aState.bRealWidth = sal_True;
                SwCntntNode* pCntntNode = pCrsr->GetCntntNode();
                SwCntntFrm *pCntntFrame = pCntntNode->getLayoutFrm( GetLayout(), pPt, pCrsr->GetPoint(), sal_False);

                pCntntFrame->GetCharRect( aStartRect, *pCrsr->GetPoint(), &aState );
                rContent = nWordEnd - 1;
                SwRect aEndRect;
                pCntntFrame->GetCharRect( aEndRect, *pCrsr->GetPoint(),&aState );
                rSelectRect = aStartRect.Union( aEndRect );
                Pop(sal_False);
            }
        }
    }
    return xSpellAlt;
}

bool SwEditShell::GetGrammarCorrection(
    linguistic2::ProofreadingResult /*out*/ &rResult, 
    sal_Int32 /*out*/ &rErrorPosInText,               
    sal_Int32 /*out*/ &rErrorIndexInResult,           
    uno::Sequence< OUString > /*out*/ &rSuggestions,  
    const Point *pPt, SwRect &rSelectRect )
{
    bool bRes = false;

    if( IsTableMode() )
        return bRes;

    SwPaM* pCrsr = GetCrsr();
    SwPosition aPos( *pCrsr->GetPoint() );
    Point aPt( *pPt );
    SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
    SwTxtNode *pNode;
    SwGrammarMarkUp *pWrong;
    if( GetLayout()->GetCrsrOfst( &aPos, aPt, &eTmpState ) &&
        0 != (pNode = aPos.nNode.GetNode().GetTxtNode()) &&
        0 != (pWrong = pNode->GetGrammarCheck()) &&
        !pNode->IsInProtectSect() )
    {
        sal_Int32 nBegin = aPos.nContent.GetIndex();
        sal_Int32 nLen = 1;
        if (pWrong->InWrongWord(nBegin, nLen))
        {
            const OUString aText(pNode->GetTxt().copy(nBegin, nLen));

            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( mpDoc->GetGCIterator() );
            if (xGCIterator.is())
            {
                uno::Reference< lang::XComponent > xDoc( mpDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY );

                
                const ModelToViewHelper aConversionMap(*pNode);
                OUString aExpandText = aConversionMap.getViewText();
                
                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *pNode, aExpandText, aConversionMap );

                
                rErrorPosInText = aConversionMap.ConvertToViewPosition( nBegin );

                const sal_Int32 nStartOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceStart( nBegin ) );
                const sal_Int32 nEndOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceEnd( nBegin ) );

                rResult = xGCIterator->checkSentenceAtPosition(
                        xDoc, xFlatPara, aExpandText, lang::Locale(), nStartOfSentence,
                        nEndOfSentence == COMPLETE_STRING ? aExpandText.getLength() : nEndOfSentence,
                        rErrorPosInText );
                bRes = true;

                
                sal_Int32 nErrors = rResult.aErrors.getLength();
                rSuggestions.realloc( 0 );
                for (sal_Int32 i = 0;  i < nErrors; ++i )
                {
                    
                    const linguistic2::SingleProofreadingError &rError = rResult.aErrors[i];
                    if (rError.nErrorStart <= rErrorPosInText &&
                        rErrorPosInText + nLen <= rError.nErrorStart + rError.nErrorLength)
                    {
                        rSuggestions = rError.aSuggestions;
                        rErrorIndexInResult = i;
                        break;
                    }
                }
            }

            if (rResult.aErrors.getLength() > 0)    
            {
                
                Push();
                LeftMargin();
                const sal_Int32 nLineStart = GetCrsr()->GetPoint()->nContent.GetIndex();
                RightMargin();
                const sal_Int32 nLineEnd = GetCrsr()->GetPoint()->nContent.GetIndex();
                Pop(sal_False);

                
                
                
                
                const sal_Unicode* pChar = aText.getStr();
                sal_Int32 nLeft = 0;
                while (pChar && *pChar++ == CH_TXTATR_INWORD)
                    ++nLeft;
                pChar = aText.getLength() ? aText.getStr() + aText.getLength() - 1 : 0;
                sal_Int32 nRight = 0;
                while (pChar && *pChar-- == CH_TXTATR_INWORD)
                    ++nRight;

                aPos.nContent = nBegin + nLeft;
                pCrsr = GetCrsr();
                *pCrsr->GetPoint() = aPos;
                pCrsr->SetMark();
                ExtendSelection( sal_True, nLen - nLeft - nRight );
                
                const sal_Int32 nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
                
                const sal_Int32 nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd
                                        ? nLineEnd : (nBegin + nLen - nLeft - nRight);
                Push();
                pCrsr->DeleteMark();
                SwIndex& rContent = GetCrsr()->GetPoint()->nContent;
                rContent = nWordStart;
                SwRect aStartRect;
                SwCrsrMoveState aState;
                aState.bRealWidth = sal_True;
                SwCntntNode* pCntntNode = pCrsr->GetCntntNode();
                SwCntntFrm *pCntntFrame = pCntntNode->getLayoutFrm( GetLayout(), pPt, pCrsr->GetPoint(), sal_False);

                pCntntFrame->GetCharRect( aStartRect, *pCrsr->GetPoint(), &aState );
                rContent = nWordEnd - 1;
                SwRect aEndRect;
                pCntntFrame->GetCharRect( aEndRect, *pCrsr->GetPoint(),&aState );
                rSelectRect = aStartRect.Union( aEndRect );
                Pop(sal_False);
            }
        }
    }

    return bRes;
}

bool SwEditShell::SpellSentence(::svx::SpellPortions& rPortions, bool bIsGrammarCheck)
{
    OSL_ENSURE(  pSpellIter, "SpellIter missing" );
    if(!pSpellIter)
        return false;
    bool bRet = pSpellIter->SpellSentence(rPortions, bIsGrammarCheck);

    
    
    StartAction();
    EndAction();
    return bRet;
}


void SwEditShell::PutSpellingToSentenceStart()
{
    OSL_ENSURE(  pSpellIter, "SpellIter missing" );
    if(!pSpellIter)
        return;
    pSpellIter->ToSentenceStart();
}

static sal_uInt32 lcl_CountRedlines(const ::svx::SpellPortions& rLastPortions)
{
    sal_uInt32 nRet = 0;
    SpellPortions::const_iterator aIter = rLastPortions.begin();
    for( ; aIter != rLastPortions.end(); ++aIter)
    {
        if( aIter->bIsHidden )
            ++nRet;
    }
    return nRet;
}

void SwEditShell::MoveContinuationPosToEndOfCheckedSentence()
{
    
    
    if (pSpellIter)
    {
        pSpellIter->SetCurr( new SwPosition( *pSpellIter->GetCurrX() ) );
        pSpellIter->ContinueAfterThisSentence();
    }
}

void SwEditShell::ApplyChangedSentence(const ::svx::SpellPortions& rNewPortions, bool bRecheck)
{
    
    

    OSL_ENSURE(  pSpellIter, "SpellIter missing" );
    if(pSpellIter &&
       pSpellIter->GetLastPortions().size() > 0)    
    {
        const SpellPortions& rLastPortions = pSpellIter->GetLastPortions();
        const SpellContentPositions  rLastPositions = pSpellIter->GetLastPositions();
        OSL_ENSURE(rLastPortions.size() > 0 &&
                rLastPortions.size() == rLastPositions.size(),
                "last vectors of spelling results are not set or not equal");

        
        

        mpDoc->GetIDocumentUndoRedo().StartUndo( UNDO_UI_TEXT_CORRECTION, NULL );
        StartAction();

        SwPaM *pCrsr = GetCrsr();
        
        
        Push();

        sal_uInt32 nRedlinePortions = lcl_CountRedlines(rLastPortions);
        if((rLastPortions.size() - nRedlinePortions) == rNewPortions.size())
        {
            OSL_ENSURE( !rNewPortions.empty(), "rNewPortions should not be empty here" );
            OSL_ENSURE( !rLastPortions.empty(), "rLastPortions should not be empty here" );
            OSL_ENSURE( !rLastPositions.empty(), "rLastPositions should not be empty here" );

            
            
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.end();
            SpellPortions::const_iterator aCurrentOldPortion = rLastPortions.end();
            SpellContentPositions::const_iterator aCurrentOldPosition = rLastPositions.end();
            do
            {
                --aCurrentNewPortion;
                --aCurrentOldPortion;
                --aCurrentOldPosition;
                
                while(aCurrentOldPortion->bIsHidden)
                {
                    if (aCurrentOldPortion  != rLastPortions.begin() &&
                        aCurrentOldPosition != rLastPositions.begin())
                    {
                        --aCurrentOldPortion;
                        --aCurrentOldPosition;
                    }
                    else
                    {
                        OSL_FAIL("ApplyChangedSentence: iterator positions broken" );
                        break;
                    }
                }
                if ( !pCrsr->HasMark() )
                    pCrsr->SetMark();
                pCrsr->GetPoint()->nContent = aCurrentOldPosition->nLeft;
                pCrsr->GetMark()->nContent = aCurrentOldPosition->nRight;
                sal_uInt16 nScriptType = GetI18NScriptTypeOfLanguage( aCurrentNewPortion->eLanguage );
                sal_uInt16 nLangWhichId = RES_CHRATR_LANGUAGE;
                switch(nScriptType)
                {
                    case SCRIPTTYPE_ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                    case SCRIPTTYPE_COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
                }
                if(aCurrentNewPortion->sText != aCurrentOldPortion->sText)
                {
                    
                    mpDoc->DeleteAndJoin(*pCrsr);
                    
                    if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                        SetAttrItem( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId), nLangWhichId );
                    mpDoc->InsertString(*pCrsr, aCurrentNewPortion->sText);
                }
                else if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                {
                    
                    SetAttrItem( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId), nLangWhichId );
                }
                else if( aCurrentNewPortion->bIgnoreThisError )
                {
                    
                    IgnoreGrammarErrorAt( *pCrsr );
                    OSL_FAIL("TODO: add ignore mark to text node");
                }
                if(aCurrentNewPortion == rNewPortions.begin())
                    break;
            }
            while(aCurrentNewPortion != rNewPortions.begin());
        }
        else
        {
            OSL_ENSURE( !rLastPositions.empty(), "rLastPositions should not be empty here" );

            
            SpellContentPositions::const_iterator aCurrentEndPosition = rLastPositions.end();
            --aCurrentEndPosition;
            SpellContentPositions::const_iterator aCurrentStartPosition = rLastPositions.begin();
            pCrsr->GetPoint()->nContent = aCurrentStartPosition->nLeft;
            pCrsr->GetMark()->nContent = aCurrentEndPosition->nRight;

            
            mpDoc->DeleteAndJoin(*pCrsr);
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.begin();
            while(aCurrentNewPortion != rNewPortions.end())
            {
                
                sal_uInt16 nScriptType = GetScriptType();
                sal_uInt16 nLangWhichId = RES_CHRATR_LANGUAGE;
                switch(nScriptType)
                {
                    case SCRIPTTYPE_ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                    case SCRIPTTYPE_COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
                }
                SfxItemSet aSet(GetAttrPool(), nLangWhichId, nLangWhichId, 0);
                GetCurAttr( aSet );
                const SvxLanguageItem& rLang = static_cast<const SvxLanguageItem& >(aSet.Get(nLangWhichId));
                if(rLang.GetLanguage() != aCurrentNewPortion->eLanguage)
                    SetAttrItem( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId) );
                
                mpDoc->InsertString(*pCrsr, aCurrentNewPortion->sText);

                
                *pCrsr->Start() = *pCrsr->End();
                ++aCurrentNewPortion;
            }
        }

        
        
        
        Pop( sal_False );

        
        *pCrsr->Start() = *pCrsr->End();
        if (bRecheck)
        {
            
            GoStartSentence();
        }
        
        pSpellIter->SetCurr( new SwPosition( *pCrsr->Start() ) );

        mpDoc->GetIDocumentUndoRedo().EndUndo( UNDO_UI_TEXT_CORRECTION, NULL );
        EndAction();
    }
}
/** Collect all deleted redlines of the current text node
 *  beginning at the start of the cursor position
 */
static SpellContentPositions lcl_CollectDeletedRedlines(SwEditShell* pSh)
{
    SpellContentPositions aRedlines;
    SwDoc* pDoc = pSh->GetDoc();
    const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( pDoc->GetRedlineMode() );
    if ( bShowChg )
    {
        SwPaM *pCrsr = pSh->GetCrsr();
        const SwPosition* pStartPos = pCrsr->Start();
        const SwTxtNode* pTxtNode = pCrsr->GetNode()->GetTxtNode();

        sal_uInt16 nAct = pDoc->GetRedlinePos( *pTxtNode, USHRT_MAX );
        const sal_Int32 nStartIndex = pStartPos->nContent.GetIndex();
        for ( ; nAct < pDoc->GetRedlineTbl().size(); nAct++ )
        {
            const SwRangeRedline* pRed = pDoc->GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > pTxtNode->GetIndex() )
                break;

            if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
            {
                sal_Int32 nStart_, nEnd_;
                pRed->CalcStartEnd( pTxtNode->GetIndex(), nStart_, nEnd_ );
                sal_Int32 nStart = nStart_;
                sal_Int32 nEnd = nEnd_;
                if(nStart >= nStartIndex || nEnd >= nStartIndex)
                {
                    SpellContentPosition aAdd;
                    aAdd.nLeft = nStart;
                    aAdd.nRight = nEnd;
                    aRedlines.push_back(aAdd);
                }
            }
        }
    }
    return aRedlines;
}


static void lcl_CutRedlines( SpellContentPositions& aDeletedRedlines, SwEditShell* pSh )
{
    if(!aDeletedRedlines.empty())
    {
        SwPaM *pCrsr = pSh->GetCrsr();
        const SwPosition* pEndPos = pCrsr->End();
        const sal_Int32 nEnd = pEndPos->nContent.GetIndex();
        while(!aDeletedRedlines.empty() &&
                aDeletedRedlines.back().nLeft > nEnd)
        {
            aDeletedRedlines.pop_back();
        }
    }
}

static SpellContentPosition  lcl_FindNextDeletedRedline(
        const SpellContentPositions& rDeletedRedlines,
        sal_Int32 nSearchFrom )
{
    SpellContentPosition aRet;
    aRet.nLeft = aRet.nRight = SAL_MAX_INT32;
    if(!rDeletedRedlines.empty())
    {
        SpellContentPositions::const_iterator aIter = rDeletedRedlines.begin();
        for( ; aIter != rDeletedRedlines.end(); ++aIter)
        {
            if(aIter->nLeft < nSearchFrom)
                continue;
            aRet = *aIter;
            break;
        }
    }
    return aRet;
}

bool SwSpellIter::SpellSentence(::svx::SpellPortions& rPortions, bool bIsGrammarCheck)
{
    bool bRet = false;
    aLastPortions.clear();
    aLastPositions.clear();

    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return false;

    OSL_ENSURE( GetEnd(), "SwSpellIter::SpellSentence without Start?");

    uno::Reference< XSpellAlternatives >  xSpellRet;
    linguistic2::ProofreadingResult aGrammarResult;
    bool bGoOn = true;
    bool bGrammarErrorFound = false;
    do {
        SwPaM *pCrsr = pMySh->GetCrsr();
        if ( !pCrsr->HasMark() )
            pCrsr->SetMark();

        *pCrsr->GetPoint() = *GetCurr();
        *pCrsr->GetMark() = *GetEnd();

        if( bBackToStartOfSentence )
        {
            pMySh->GoStartSentence();
            bBackToStartOfSentence = false;
        }
        uno::Any aSpellRet =
        pMySh->GetDoc()->Spell(*pCrsr,
                    xSpeller, 0, 0, bIsGrammarCheck );
        aSpellRet >>= xSpellRet;
        aSpellRet >>= aGrammarResult;
        bGoOn = GetCrsrCnt() > 1;
        bGrammarErrorFound = aGrammarResult.aErrors.getLength() > 0;
        if( xSpellRet.is() || bGrammarErrorFound )
        {
            bGoOn = false;
            SwPosition* pNewPoint = new SwPosition( *pCrsr->GetPoint() );
            SwPosition* pNewMark = new SwPosition( *pCrsr->GetMark() );

            SetCurr( pNewPoint );
            SetCurrX( pNewMark );
        }
        if( bGoOn )
        {
            pMySh->Pop( sal_False );
            pCrsr = pMySh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition( *pCrsr->GetPoint() );
            SetStart( pNew );
            pNew = new SwPosition( *pCrsr->GetMark() );
            SetEnd( pNew );
            pNew = new SwPosition( *GetStart() );
            SetCurr( pNew );
            pNew = new SwPosition( *pNew );
            SetCurrX( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    } while ( bGoOn );

    if(xSpellRet.is() || bGrammarErrorFound)
    {
        
        
        SwPaM *pCrsr = pMySh->GetCrsr();
        
        if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
            pCrsr->Exchange();
        
        pCrsr->DeleteMark();
        pCrsr->SetMark();
        bool bStartSent = 0 != pMySh->GoStartSentence();
        SpellContentPositions aDeletedRedlines = lcl_CollectDeletedRedlines(pMySh);
        if(bStartSent)
        {
            
            AddPortion(0, 0, aDeletedRedlines);
        }
        
        *pCrsr->GetPoint() = *GetCurrX();
        *pCrsr->GetMark() = *GetCurr();
        AddPortion(xSpellRet, &aGrammarResult, aDeletedRedlines);

        
        SwPosition aSaveStartPos = *pCrsr->End();
        
        if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
            pCrsr->Exchange();
        
        pCrsr->DeleteMark();
        pCrsr->SetMark();

        pMySh->GoEndSentence();
        if( bGrammarErrorFound )
        {
            const ModelToViewHelper aConversionMap(*(SwTxtNode*)pCrsr->GetNode());
            OUString aExpandText = aConversionMap.getViewText();
            sal_Int32 nSentenceEnd =
                aConversionMap.ConvertToViewPosition( aGrammarResult.nBehindEndOfSentencePosition );
            
            if( aExpandText[nSentenceEnd - 1] == ' ' )
                --nSentenceEnd;
            if( pCrsr->End()->nContent.GetIndex() < nSentenceEnd )
            {
                pCrsr->End()->nContent.Assign(
                    pCrsr->End()->nNode.GetNode().GetCntntNode(), nSentenceEnd);
            }
        }

        lcl_CutRedlines( aDeletedRedlines, pMySh );
        
        const SwPosition aSaveEndPos = *GetEnd();
        
        SetEnd( new SwPosition( *pCrsr->End() ));

        *pCrsr->GetPoint() = aSaveStartPos;
        *pCrsr->GetMark() = *GetEnd();
        
        
        
        if( !bGrammarErrorFound ) 
        {
            do
            {
                xSpellRet = 0;
                
                pMySh->GetDoc()->Spell(*pCrsr,
                            xSpeller, 0, 0, false ) >>= xSpellRet;
                if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                    pCrsr->Exchange();
                SetCurr( new SwPosition( *pCrsr->GetPoint() ));
                SetCurrX( new SwPosition( *pCrsr->GetMark() ));

                
                if(xSpellRet.is())
                {
                    *pCrsr->GetPoint() = aSaveStartPos;
                    *pCrsr->GetMark() = *GetCurr();
                }
                
                AddPortion(0, 0, aDeletedRedlines);

                if(xSpellRet.is())
                {
                    *pCrsr->GetPoint() = *GetCurr();
                    *pCrsr->GetMark() = *GetCurrX();
                    AddPortion(xSpellRet, 0, aDeletedRedlines);
                    
                    *pCrsr->GetPoint() = *GetCurrX();
                    
                    aSaveStartPos = *GetCurrX();
                    
                    *pCrsr->GetMark() = *GetEnd();
                }
                
                if(*GetCurrX() >= *GetEnd())
                    break;
            }
            while(xSpellRet.is());
        }
        else
        {
            
            
            
            if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
                pCrsr->Exchange();
        }

        
        *pMySh->GetCrsr()->GetPoint() = *GetEnd();
        if(*GetCurrX() < *GetEnd())
        {
            AddPortion(0, 0, aDeletedRedlines);
        }
        
        *pCrsr->GetMark() = *GetEnd();
        if( !bIsGrammarCheck )
        {
            
            SetCurr( new SwPosition(*GetEnd()) );
        }
        
        SetEnd( new SwPosition(aSaveEndPos) );
        rPortions = aLastPortions;
        bRet = true;
    }
    else
    {
        
        *pMySh->GetCrsr()->GetPoint() = *GetEnd();
        pMySh->GetCrsr()->DeleteMark();
    }

    return bRet;
}

void SwSpellIter::ToSentenceStart()
{
    bBackToStartOfSentence = true;
}

static LanguageType lcl_GetLanguage(SwEditShell& rSh)
{
    sal_uInt16 nScriptType = rSh.GetScriptType();
    sal_uInt16 nLangWhichId = RES_CHRATR_LANGUAGE;

    switch(nScriptType)
    {
        case SCRIPTTYPE_ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
        case SCRIPTTYPE_COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
    }
    SfxItemSet aSet(rSh.GetAttrPool(), nLangWhichId, nLangWhichId, 0);
    rSh.GetCurAttr( aSet );
    const SvxLanguageItem& rLang = static_cast<const SvxLanguageItem& >(aSet.Get(nLangWhichId));
    return rLang.GetLanguage();
}


void SwSpellIter::CreatePortion(uno::Reference< XSpellAlternatives > xAlt,
                        linguistic2::ProofreadingResult* pGrammarResult,
        bool bIsField, bool bIsHidden)
{
    svx::SpellPortion aPortion;
    OUString sText;
    GetSh()->GetSelectedText( sText );
    if(!sText.isEmpty())
    {
        
        if(xAlt.is())
            aPortion.sText = xAlt->getWord();
        else if(pGrammarResult)
        {
            aPortion.bIsGrammarError = true;
            if(pGrammarResult->aErrors.getLength())
            {
                aPortion.aGrammarError = pGrammarResult->aErrors[0];
                aPortion.sText = pGrammarResult->aText.copy( aPortion.aGrammarError.nErrorStart, aPortion.aGrammarError.nErrorLength );
                aPortion.xGrammarChecker = pGrammarResult->xProofreader;
                const beans::PropertyValue* pProperties = pGrammarResult->aProperties.getConstArray();
                for( sal_Int32 nProp = 0; nProp < pGrammarResult->aProperties.getLength(); ++nProp )
                {
                    if ( pProperties->Name == "DialogTitle" )
                    {
                        pProperties->Value >>= aPortion.sDialogTitle;
                        break;
                    }
                }
            }
        }
        else
            aPortion.sText = sText;
        aPortion.eLanguage = lcl_GetLanguage(*GetSh());
        aPortion.bIsField = bIsField;
        aPortion.bIsHidden = bIsHidden;
        aPortion.xAlternatives = xAlt;
        SpellContentPosition aPosition;
        SwPaM *pCrsr = GetSh()->GetCrsr();
        aPosition.nLeft = pCrsr->Start()->nContent.GetIndex();
        aPosition.nRight = pCrsr->End()->nContent.GetIndex();
        aLastPortions.push_back(aPortion);
        aLastPositions.push_back(aPosition);
    }
}

void    SwSpellIter::AddPortion(uno::Reference< XSpellAlternatives > xAlt,
                                linguistic2::ProofreadingResult* pGrammarResult,
                                const SpellContentPositions& rDeletedRedlines)
{
    SwEditShell *pMySh = GetSh();
    OUString sText;
    pMySh->GetSelectedText( sText );
    if(!sText.isEmpty())
    {
        if(xAlt.is() || pGrammarResult != 0)
        {
            CreatePortion(xAlt, pGrammarResult, false, false);
        }
        else
        {
            SwPaM *pCrsr = GetSh()->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            
            SwPosition aStart(*pCrsr->GetPoint());
            SwPosition aEnd(*pCrsr->GetMark());
            
            
            *pCrsr->GetMark() = aStart;
            SwTxtNode* pTxtNode = pCrsr->GetNode()->GetTxtNode();
            LanguageType eStartLanguage = lcl_GetLanguage(*GetSh());
            SpellContentPosition  aNextRedline = lcl_FindNextDeletedRedline(
                        rDeletedRedlines, aStart.nContent.GetIndex() );
            if( aNextRedline.nLeft == aStart.nContent.GetIndex() )
            {
                
                const sal_Int32 nEnd = aEnd.nContent.GetIndex() < aNextRedline.nRight ?
                            aEnd.nContent.GetIndex() : aNextRedline.nRight;
                pCrsr->GetPoint()->nContent.Assign( pTxtNode, nEnd );
                CreatePortion(xAlt, pGrammarResult, false, true);
                aStart = *pCrsr->End();
                
                aNextRedline = lcl_FindNextDeletedRedline(
                            rDeletedRedlines, aStart.nContent.GetIndex() );
            }
            while(*pCrsr->GetPoint() < aEnd)
            {
                
                if(!GetSh()->Right(1, CRSR_SKIP_CELLS))
                    break;

                bool bField = false;
                
                sal_Unicode const cChar =
                    pTxtNode->GetTxt()[pCrsr->GetMark()->nContent.GetIndex()];
                if( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar)
                {
                    const SwTxtAttr* pTxtAttr = pTxtNode->GetTxtAttrForCharAt(
                        pCrsr->GetMark()->nContent.GetIndex() );
                    const sal_uInt16 nWhich = pTxtAttr
                        ? pTxtAttr->Which()
                        : static_cast<sal_uInt16>(RES_TXTATR_END);
                    switch (nWhich)
                    {
                        case RES_TXTATR_FIELD:
                        case RES_TXTATR_ANNOTATION:
                        case RES_TXTATR_FTN:
                        case RES_TXTATR_FLYCNT:
                            bField = true;
                            break;
                    }
                }

                LanguageType eCurLanguage = lcl_GetLanguage(*GetSh());
                bool bRedline = aNextRedline.nLeft == pCrsr->GetPoint()->nContent.GetIndex();
                
                
                
                
                if(bField || bRedline || eCurLanguage != eStartLanguage)
                {
                    eStartLanguage = eCurLanguage;
                    
                    
                    
                    if(eCurLanguage != eStartLanguage || bField)
                        *pCrsr->GetPoint() = *pCrsr->GetMark();
                    
                    *pCrsr->GetMark() = aStart;
                    
                    
                    if(*pCrsr->Start() != *pCrsr->End())
                        CreatePortion(xAlt, pGrammarResult, false, false);
                    aStart = *pCrsr->End();
                    
                    if(bField)
                    {
                        *pCrsr->GetMark() = *pCrsr->GetPoint();
                        GetSh()->Right(1, CRSR_SKIP_CELLS);
                        CreatePortion(xAlt, pGrammarResult, true, false);
                        aStart = *pCrsr->End();
                    }
                }
                
                if(bRedline)
                {
                    *pCrsr->GetMark() = *pCrsr->GetPoint();
                    
                    const sal_Int32 nEnd = aEnd.nContent.GetIndex() < aNextRedline.nRight ?
                                aEnd.nContent.GetIndex() : aNextRedline.nRight;
                    pCrsr->GetPoint()->nContent.Assign( pTxtNode, nEnd );
                    CreatePortion(xAlt, pGrammarResult, false, true);
                    aStart = *pCrsr->End();
                    
                    aNextRedline = lcl_FindNextDeletedRedline(
                                rDeletedRedlines, aStart.nContent.GetIndex() );
                }
                *pCrsr->GetMark() = *pCrsr->GetPoint();
            }
            pCrsr->SetMark();
            *pCrsr->GetMark() = aStart;
            CreatePortion(xAlt, pGrammarResult, false, false);
        }
    }
}

void SwEditShell::IgnoreGrammarErrorAt( SwPaM& rErrorPosition )
{
    SwTxtNode *pNode;
    SwWrongList *pWrong;
    SwNodeIndex aIdx = rErrorPosition.Start()->nNode;
    SwNodeIndex aEndIdx = rErrorPosition.Start()->nNode;
    sal_Int32 nStart = rErrorPosition.Start()->nContent.GetIndex();
    sal_Int32 nEnd = COMPLETE_STRING;
    while( aIdx <= aEndIdx )
    {
        pNode = aIdx.GetNode().GetTxtNode();
        if( pNode ) {
            if( aIdx == aEndIdx )
                nEnd = rErrorPosition.End()->nContent.GetIndex();
            pWrong = pNode->GetGrammarCheck();
            if( pWrong )
                pWrong->RemoveEntry( nStart, nEnd );
            pWrong = pNode->GetWrong();
            if( pWrong )
                pWrong->RemoveEntry( nStart, nEnd );
            SwTxtFrm::repaintTextFrames( *pNode );
        }
        ++aIdx;
        nStart = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
