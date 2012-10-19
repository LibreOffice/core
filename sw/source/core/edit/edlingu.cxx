/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
#include <rootfrm.hxx>      // SwRootFrm
#include <pam.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <ndtxt.hxx>        // AdjHyphPos
#include <viewopt.hxx>      // HyphStart/End
#include <viscrs.hxx>       // SwShellCrsr
#include <SwGrammarMarkUp.hxx>      // SwWrongList
#include <mdiexp.hxx>       // Statusanzeige
#include <statstr.hrc>      // StatLine-String
#include <cntfrm.hxx>
#include <crsskip.hxx>
#include <splargs.hxx>
#include <redline.hxx>      // SwRedline
#include <docary.hxx>       // SwRedlineTbl
#include <docsh.hxx>
#include <txatbase.hxx>
#include <txtfrm.hxx>

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

/*************************************************************************
 *                     class SwLinguIter
 *************************************************************************/

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

    // Der UI-Bauchladen:
    void _Start( SwEditShell *pSh, SwDocPositions eStart,
                SwDocPositions eEnd );
    void _End(bool bRestoreSelection = true);
};

/*************************************************************************
 *                     class SwSpellIter
 *************************************************************************/

// #i18881# to be able to identify the postions of the changed words
// the content positions of each portion need to be saved
struct SpellContentPosition
{
    sal_uInt16 nLeft;
    sal_uInt16 nRight;
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

/*************************************************************************
 *                     class SwConvIter
 * used for text conversion
 *************************************************************************/

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

/*************************************************************************
 *                     class SwHyphIter
 *************************************************************************/

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
    void InsertSoftHyph( const xub_StrLen nHyphPos );
    void ShowSelection();
};

static SwSpellIter* pSpellIter = 0;
static SwConvIter*  pConvIter = 0;
static SwHyphIter*  pHyphIter = 0;

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in txtedt.cxx stehen extern-Deklarationen auf diese Pointer!
const SwTxtNode *pLinguNode;
      SwTxtFrm  *pLinguFrm;

/*************************************************************************
 *                      SwLinguIter::SwLinguIter
 *************************************************************************/

SwLinguIter::SwLinguIter()
    : pSh( 0 ), pStart( 0 ), pEnd( 0 ), pCurr( 0 ), pCurrX( 0 )
{
    // @@@ es fehlt: Sicherstellen der Reentrance, OSL_ENSURE( etc.
}

/*************************************************************************
 *                      SwLinguIter::Start
 *************************************************************************/



void SwLinguIter::_Start( SwEditShell *pShell, SwDocPositions eStart,
                            SwDocPositions eEnd )
{
    // es fehlt: Sicherstellen der Reentrance, Locking
    if( pSh )
        return;

    sal_Bool bSetCurr;

    pSh = pShell;

    SET_CURR_SHELL( pSh );

    OSL_ENSURE( !pEnd, "LinguStart ohne End?");

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
        bSetCurr = sal_False;
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

/*************************************************************************
 *                      SwLinguIter::End
 *************************************************************************/



void SwLinguIter::_End(bool bRestoreSelection)
{
    if( !pSh )
        return;

    OSL_ENSURE( pEnd, "SwEditShell::SpellEnd() ohne Start?");
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

/*************************************************************************
 *               virtual SwSpellIter::Start()
 *************************************************************************/



void SwSpellIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;

     uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    xSpeller = ::GetSpellChecker();
    if ( xSpeller.is() )
        _Start( pShell, eStart, eEnd );
    aLastPortions.clear();
    aLastPositions.clear();
}

/*************************************************************************
 *                   SwSpellIter::Continue
 *************************************************************************/

// SwSpellIter::Continue ist das alte Original von
// SwEditShell::SpellContinue()

uno::Any SwSpellIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    //!!
    //!! Please check SwConvIter also when modifying this
    //!!

    uno::Any    aSpellRet;
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aSpellRet;

    OSL_ENSURE( GetEnd(), "SwEditShell::SpellContinue() ohne Start?");

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

/*************************************************************************
 *               virtual SwConvIter::Start()
 *************************************************************************/



void SwConvIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;
    _Start( pShell, eStart, eEnd );
}

/*************************************************************************
 *                   SwConvIter::Continue
 *************************************************************************/

uno::Any SwConvIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    //!!
    //!! Please check SwSpellIter also when modifying this
    //!!

    uno::Any    aConvRet( makeAny( rtl::OUString() ) );
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aConvRet;

    OSL_ENSURE( GetEnd(), "SwConvIter::Continue() ohne Start?");

    rtl::OUString aConvText;
    bool bGoOn = true;
    do {
        SwPaM *pCrsr = pMySh->GetCrsr();
        if ( !pCrsr->HasMark() )
            pCrsr->SetMark();

        *pMySh->GetCrsr()->GetPoint() = *GetCurr();
        *pMySh->GetCrsr()->GetMark() = *GetEnd();

        // call function to find next text portion to be converted
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


/*************************************************************************
 *                   SwHyphIter
 *************************************************************************/


sal_Bool SwHyphIter::IsAuto()
{
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    return xProp.is() ? *(sal_Bool*)xProp->getPropertyValue(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UPN_IS_HYPH_AUTO)) ).getValue()
                      : sal_False;
}


void SwHyphIter::ShowSelection()
{
    SwEditShell *pMySh = GetSh();
    if( pMySh )
    {
        pMySh->StartAction();
        // Ganz fatal: durch das EndAction() werden Formatierungen
        // angeregt, die dazu fuehren koennen, dass im Hyphenator
        // neue Worte eingestellt werden. Deswegen sichern!
        pMySh->EndAction();
    }
}

/*************************************************************************
 *               virtual SwHyphIter::Start()
 *************************************************************************/



void SwHyphIter::Start( SwEditShell *pShell, SwDocPositions eStart, SwDocPositions eEnd )
{
    // robust
    if( GetSh() || GetEnd() )
    {
        OSL_ENSURE( !GetSh(), "+SwEditShell::HyphStart: missing HyphEnd()" );
        return;
    }

// nothing to be done (at least not in the way as in the "else" part)
    bOldIdle = pShell->GetViewOptions()->IsIdle();
    ((SwViewOption*)pShell->GetViewOptions())->SetIdle( sal_False );
    _Start( pShell, eStart, eEnd );
}

/*************************************************************************
 *                 virtual SwHyphIter::End
 *************************************************************************/

// Selektionen wiederherstellen



void SwHyphIter::End()
{
    if( !GetSh() )
        return;
    ((SwViewOption*)GetSh()->GetViewOptions())->SetIdle( bOldIdle );
    _End();
}

/*************************************************************************
 *                   SwHyphIter::Continue
 *************************************************************************/

uno::Any SwHyphIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    uno::Any    aHyphRet;
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aHyphRet;

    const sal_Bool bAuto = IsAuto();
     uno::Reference< XHyphenatedWord >  xHyphWord;
    bool bGoOn = false;
    do {
        SwPaM *pCrsr;
        do {
            OSL_ENSURE( GetEnd(), "SwEditShell::SpellContinue() ohne Start?" );
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

                // Muss an der aktuellen Cursorpos das Wort getrennt werden ?
                const Point aCrsrPos( pMySh->GetCharRect().Pos() );
                xHyphWord = pMySh->GetDoc()->Hyphenate( pCrsr, aCrsrPos,
                                                       pPageCnt, pPageSt );
            }

            if( bAuto && xHyphWord.is() )
            {
                pMySh->InsertSoftHyph( xHyphWord->getHyphenationPos() + 1);
            }
        } while( bAuto && xHyphWord.is() ); //end of do-while
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

/*************************************************************************
 *                  SwHyphIter::HyphIgnore
 *************************************************************************/

// Beschreibung: Trennstelle ignorieren

void SwHyphIter::Ignore()
{
    SwEditShell *pMySh = GetSh();
    SwPaM *pCrsr = pMySh->GetCrsr();

    // Alten SoftHyphen loeschen
    DelSoftHyph( *pCrsr );

    // und weiter
    pCrsr->Start()->nContent = pCrsr->End()->nContent;
    pCrsr->SetMark();
}

/*************************************************************************
 *                        SwHyphIter::DelSoftHyph
 *************************************************************************/

void SwHyphIter::DelSoftHyph( SwPaM &rPam )
{
    const SwPosition* pStt = rPam.Start();
    const xub_StrLen nStart = pStt->nContent.GetIndex();
    const xub_StrLen nEnd   = rPam.End()->nContent.GetIndex();
    SwTxtNode *pNode = pStt->nNode.GetNode().GetTxtNode();
    pNode->DelSoftHyph( nStart, nEnd );
}

/*************************************************************************
 *                  SwHyphIter::InsertSoftHyph
 *************************************************************************/


void SwHyphIter::InsertSoftHyph( const xub_StrLen nHyphPos )
{
    SwEditShell *pMySh = GetSh();
    OSL_ENSURE( pMySh,  "+SwEditShell::InsertSoftHyph: missing HyphStart()");
    if( !pMySh )
        return;

    SwPaM *pCrsr = pMySh->GetCrsr();
    SwPosition* pSttPos = pCrsr->Start();
    SwPosition* pEndPos = pCrsr->End();

    xub_StrLen nLastHyphLen = GetEnd()->nContent.GetIndex() -
                          pSttPos->nContent.GetIndex();

    if( pSttPos->nNode != pEndPos->nNode || !nLastHyphLen )
    {
        OSL_ENSURE( pSttPos->nNode == pEndPos->nNode,
                "+SwEditShell::InsertSoftHyph: node warp during hyphenation" );
        OSL_ENSURE(nLastHyphLen, "+SwEditShell::InsertSoftHyph: missing HyphContinue()");
        *pSttPos = *pEndPos;
        return;
    }

    pMySh->StartAction();
    {
        SwDoc *pDoc = pMySh->GetDoc();
        DelSoftHyph( *pCrsr );
        pSttPos->nContent += nHyphPos;
        SwPaM aRg( *pSttPos );
        pDoc->InsertString( aRg, rtl::OUString(CHAR_SOFTHYPHEN) );
        // Durch das Einfuegen des SoftHyphs ist ein Zeichen hinzugekommen
//JP 18.07.95: warum, ist doch ein SwIndex, dieser wird doch mitverschoben !!
//        pSttPos->nContent++;
    }
    // Die Selektion wird wieder aufgehoben
    pCrsr->DeleteMark();
    pMySh->EndAction();
    pCrsr->SetMark();
}

// --------------------- Methoden der SwEditShell ------------------------

bool SwEditShell::HasLastSentenceGotGrammarChecked() const
{
    bool bTextWasGrammarChecked = false;
    if (pSpellIter)
    {
        ::svx::SpellPortions aLastPortions( pSpellIter->GetLastPortions() );
        for (size_t i = 0;  i < aLastPortions.size() && !bTextWasGrammarChecked;  ++i)
        {
            // bIsGrammarError is also true if the text was only checked but no
            // grammar error was found. (That is if a ProofreadingResult was obtained in
            // SwDoc::Spell and in turn bIsGrammarError was set in SwSpellIter::CreatePortion)
            if (aLastPortions[i].bIsGrammarError)
                bTextWasGrammarChecked = true;
        }
    }
    return bTextWasGrammarChecked;
}

/*************************************************************************
 *                      SwEditShell::HasConvIter
 *************************************************************************/

sal_Bool SwEditShell::HasConvIter() const
{
    return 0 != pConvIter;
}

/*************************************************************************
 *                      SwEditShell::HasHyphIter
 *************************************************************************/

sal_Bool SwEditShell::HasHyphIter() const
{
    return 0 != pHyphIter;
}

/*************************************************************************
 *                      SwEditShell::SetFindRange
 *************************************************************************/

void SwEditShell::SetLinguRange( SwDocPositions eStart, SwDocPositions eEnd )
{
    SwPaM *pCrsr = GetCrsr();
    MakeFindRange( static_cast<sal_uInt16>(eStart), static_cast<sal_uInt16>(eEnd), pCrsr );
    if( *pCrsr->GetPoint() > *pCrsr->GetMark() )
        pCrsr->Exchange();
}

/*************************************************************************
 *                  SwEditShell::SpellStart
 *************************************************************************/

void SwEditShell::SpellStart(
        SwDocPositions eStart, SwDocPositions eEnd, SwDocPositions eCurr,
        SwConversionArgs *pConvArgs )
{
    SwLinguIter *pLinguIter = 0;

    // do not spell if interactive spelling is active elsewhere
    if (!pConvArgs && !pSpellIter)
    {
        OSL_ENSURE( !pSpellIter, "wer ist da schon am spellen?" );
        pSpellIter = new SwSpellIter;
        pLinguIter = pSpellIter;
    }
    // do not do text conversion if it is active elsewhere
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

/*************************************************************************
 *                  SwEditShell::SpellEnd
 *************************************************************************/

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

/*************************************************************************
 *                  SwEditShell::SpellContinue
 *************************************************************************/

// liefert Rueckgabewerte entsprechend SPL_ in splchk.hxx

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
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
    rtl::OUString aRet;
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
    --nStartAction;

    if( !aRet.isEmpty() || xRet.is() )
    {
        // dann die awt::Selection sichtbar machen
        StartAction();
        EndAction();
    }
    return aRes;
}
/*************************************************************************
 *                  SwEditShell::HyphStart
 *************************************************************************/

/* Interaktive Trennung, BP 10.03.93
 *
 * 1) HyphStart
 *    - Aufheben aller Selektionen
 *    - Sichern des aktuellen Cursors
 *    - falls keine Selektion vorhanden:
 *      - neue Selektion bis zum Dokumentende
 * 2) HyphContinue
 *    - nLastHyphLen wird auf den Selektionsstart addiert
 *    - iteriert ueber alle selektierten Bereiche
 *      - pDoc->Hyphenate() iteriert ueber alle Nodes der Selektion
 *          - pTxtNode->Hyphenate() ruft das SwTxtFrm::Hyphenate zur EditShell
 *              - SwTxtFrm:Hyphenate() iteriert ueber die Zeilen des Pams
 *                  - LineIter::Hyphenate() stellt den Hyphenator
 *                    und den Pam auf das zu trennende Wort ein.
 *    - Es gibt nur zwei Returnwerte sal_True, wenn eine Trennstelle anliegt
 *      und sal_False, wenn der Pam abgearbeitet wurde.
 *    - Bei sal_True wird das selektierte Wort zur Anzeige gebracht und
 *      nLastHyphLen gesetzt.
 *    - Bei sal_False wird die aktuelle Selektion geloescht und die naechste
 *      zur aktuellen gewaehlt. Return HYPH_OK, wenn keine mehr vorhanden.
 * 3) InsertSoftHyph (wird ggf. von der UI gerufen)
 *    - Der aktuelle Cursor wird plaziert und das Attribut eingefuegt.
 * 4) HyphEnd
 *    - Wiederherstellen des alten Cursors, EndAction
 */



void SwEditShell::HyphStart( SwDocPositions eStart, SwDocPositions eEnd )
{
    // do not hyphenate if interactive hyphenationg is active elsewhere
    if (!pHyphIter)
    {
        OSL_ENSURE( !pHyphIter, "wer ist da schon am hyphinieren?" );
        pHyphIter = new SwHyphIter;
        pHyphIter->Start( this, eStart, eEnd );
    }
}

/*************************************************************************
 *                  SwEditShell::HyphEnd
 *************************************************************************/

// Selektionen wiederherstellen



void SwEditShell::HyphEnd()
{
    if (pHyphIter->GetSh() == this)
    {
        OSL_ENSURE( pHyphIter, "wo ist mein Iterator?" );
        pHyphIter->End();
        delete pHyphIter, pHyphIter = 0;
    }
}

/*************************************************************************
 *                  SwEditShell::HyphContinue
 *************************************************************************/

// Returnwerte: (BP: ich wuerde es genau umdrehen, aber die UI wuenscht es so)
// HYPH_CONTINUE, wenn eine Trennstelle anliegt
// HYPH_OK, wenn der selektierte Bereich abgearbeitet wurde.


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
        else                // Hiermit unterdruecken wir ein fuer allemal
            *pPageSt = 1;   // das StatLineStartPercent
    }

    OSL_ENSURE( pHyphIter, "wo ist mein Iterator?" );
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
    uno::Reference< uno::XInterface >  xRet;
    pHyphIter->Continue( pPageCnt, pPageSt ) >>= xRet;
    --nStartAction;

    if( xRet.is() )
        pHyphIter->ShowSelection();

    return xRet;
}


/*************************************************************************
 *                  SwEditShell::InsertSoftHyph
 *************************************************************************/

// Zum Einfuegen des SoftHyphens, Position ist der Offset
// innerhalb des getrennten Wortes.


void SwEditShell::InsertSoftHyph( const xub_StrLen nHyphPos )
{
    OSL_ENSURE( pHyphIter, "wo ist mein Iterator?" );
    pHyphIter->InsertSoftHyph( nHyphPos );
}


/*************************************************************************
 *                  SwEditShell::HyphIgnore
 *************************************************************************/

// Beschreibung: Trennstelle ignorieren

void SwEditShell::HyphIgnore()
{
    OSL_ENSURE( pHyphIter, "wo ist mein Iterator?" );
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
    pHyphIter->Ignore();
    --nStartAction;

    pHyphIter->ShowSelection();
}

/*************************************************************************
 *                  SwEditShell::GetCorrection()
 * liefert eine Liste von Vorschlaegen fuer falsch geschriebene Worte,
 * ein NULL-Pointer signalisiert, dass das Wort richtig geschrieben ist,
 * eine leere Liste, dass das Wort zwar unbekannt ist, aber keine Alternativen
 * geliefert werden koennen.
 *************************************************************************/


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
        xub_StrLen nBegin = aPos.nContent.GetIndex();
        xub_StrLen nLen = 1;
        if( pWrong->InWrongWord(nBegin,nLen) && !pNode->IsSymbol(nBegin) )
        {
            String aText( pNode->GetTxt().Copy( nBegin, nLen ) );
            String aWord( aText );
            aWord = comphelper::string::remove(aWord, CH_TXTATR_BREAKWORD);
            aWord = comphelper::string::remove(aWord, CH_TXTATR_INWORD);

            uno::Reference< XSpellChecker1 >  xSpell( ::GetSpellChecker() );
            if( xSpell.is() )
            {
                LanguageType eActLang = (LanguageType)pNode->GetLang( nBegin, nLen );
                if( xSpell->hasLanguage( eActLang ))
                {
                    // restrict the maximal number of suggestions displayed
                    // in the context menu.
                    // Note: That could of course be done by clipping the
                    // resulting sequence but the current third party
                    // implementations result differs greatly if the number of
                    // suggestions to be retuned gets changed. Statistically
                    // it gets much better if told to return e.g. only 7 strings
                    // than returning e.g. 16 suggestions and using only the
                    // first 7. Thus we hand down the value to use to that
                    // implementation here by providing an additional parameter.
                    Sequence< PropertyValue > aPropVals(1);
                    PropertyValue &rVal = aPropVals.getArray()[0];
                    rVal.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UPN_MAX_NUMBER_OF_SUGGESTIONS ));
                    rVal.Value <<= (sal_Int16) 7;

                    xSpellAlt = xSpell->spell( aWord, eActLang, aPropVals );
                }
            }

            if ( xSpellAlt.is() )   // error found?
            {
                //save the start and end positons of the line and the starting point
                Push();
                LeftMargin();
                xub_StrLen nLineStart = GetCrsr()->GetPoint()->nContent.GetIndex();
                RightMargin();
                xub_StrLen nLineEnd = GetCrsr()->GetPoint()->nContent.GetIndex();
                Pop(sal_False);

                // make sure the selection build later from the data below does
                // not "in word" character to the left and right in order to
                // preserve those. Therefore count those "in words" in order to
                // modify the selection accordingly.
                const sal_Unicode* pChar = aText.GetBuffer();
                xub_StrLen nLeft = 0;
                while (pChar && *pChar++ == CH_TXTATR_INWORD)
                    ++nLeft;
                pChar = aText.Len() ? aText.GetBuffer() + aText.Len() - 1 : 0;
                xub_StrLen nRight = 0;
                while (pChar && *pChar-- == CH_TXTATR_INWORD)
                    ++nRight;

                aPos.nContent = nBegin + nLeft;
                pCrsr = GetCrsr();
                *pCrsr->GetPoint() = aPos;
                pCrsr->SetMark();
                ExtendSelection( sal_True, nLen - nLeft - nRight );
                //no determine the rectangle in the current line
                xub_StrLen nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
                //take one less than the line end - otherwise the next line would be calculated
                xub_StrLen nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd ? nLineEnd: (nBegin + nLen - nLeft - nRight);
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
    linguistic2::ProofreadingResult /*out*/ &rResult,    // the complete result
    sal_Int32 /*out*/ &rErrorPosInText,                     // offset of error position in string that was grammar checked...
    sal_Int32 /*out*/ &rErrorIndexInResult,                 // index of error in rResult.aGrammarErrors
    uno::Sequence< rtl::OUString > /*out*/ &rSuggestions,   // suggestions to be used for the error found
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
        xub_StrLen nBegin = aPos.nContent.GetIndex();
        xub_StrLen nLen = 1;
        if (pWrong->InWrongWord(nBegin, nLen))
        {
            String aText( pNode->GetTxt().Copy( nBegin, nLen ) );

            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( pDoc->GetGCIterator() );
            if (xGCIterator.is())
            {
//                LanguageType eActLang = (LanguageType)pNode->GetLang( nBegin, nLen );
                uno::Reference< lang::XComponent > xDoc( pDoc->GetDocShell()->GetBaseModel(), uno::UNO_QUERY );

                // Expand the string:
                const ModelToViewHelper aConversionMap(*pNode);
                rtl::OUString aExpandText = aConversionMap.getViewText();
                // get XFlatParagraph to use...
                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *pNode, aExpandText, aConversionMap );

                // get error position of cursor in XFlatParagraph
                rErrorPosInText = aConversionMap.ConvertToViewPosition( nBegin );

                sal_Int32 nStartOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceStart( nBegin ) );
                sal_Int32 nEndOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceEnd( nBegin ) );
                if( nEndOfSentence == STRING_LEN )
                {
                        nEndOfSentence = aExpandText.getLength();
                }

                rResult = xGCIterator->checkSentenceAtPosition(
                        xDoc, xFlatPara, aExpandText, lang::Locale(), nStartOfSentence, nEndOfSentence, rErrorPosInText );
                bRes = true;

                // get suggestions to use for the specific error position
                sal_Int32 nErrors = rResult.aErrors.getLength();
                rSuggestions.realloc( 0 );
                for (sal_Int32 i = 0;  i < nErrors; ++i )
                {
                    // return suggestions for first error that includes the given error position
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

            if (rResult.aErrors.getLength() > 0)    // error found?
            {
                //save the start and end positons of the line and the starting point
                Push();
                LeftMargin();
                xub_StrLen nLineStart = GetCrsr()->GetPoint()->nContent.GetIndex();
                RightMargin();
                xub_StrLen nLineEnd = GetCrsr()->GetPoint()->nContent.GetIndex();
                Pop(sal_False);

                // make sure the selection build later from the data below does
                // not include "in word" character to the left and right in
                // order to preserve those. Therefore count those "in words" in
                // order to modify the selection accordingly.
                const sal_Unicode* pChar = aText.GetBuffer();
                xub_StrLen nLeft = 0;
                while (pChar && *pChar++ == CH_TXTATR_INWORD)
                    ++nLeft;
                pChar = aText.Len() ? aText.GetBuffer() + aText.Len() - 1 : 0;
                xub_StrLen nRight = 0;
                while (pChar && *pChar-- == CH_TXTATR_INWORD)
                    ++nRight;

                aPos.nContent = nBegin + nLeft;
                pCrsr = GetCrsr();
                *pCrsr->GetPoint() = aPos;
                pCrsr->SetMark();
                ExtendSelection( sal_True, nLen - nLeft - nRight );
                //no determine the rectangle in the current line
                xub_StrLen nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
                //take one less than the line end - otherwise the next line would be calculated
                xub_StrLen nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd ? nLineEnd: (nBegin + nLen - nLeft - nRight);
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

    // make Selection visible - this should simply move the
    // cursor to the end of the sentence
    StartAction();
    EndAction();
    return bRet;
}
/*-------------------------------------------------------------------------
    make SpellIter start with the current sentence when called next time
  -----------------------------------------------------------------------*/
void SwEditShell::PutSpellingToSentenceStart()
{
    OSL_ENSURE(  pSpellIter, "SpellIter missing" );
    if(!pSpellIter)
        return;
    pSpellIter->ToSentenceStart();
}

static sal_uInt32 lcl_CountRedlines(
                            const ::svx::SpellPortions& rLastPortions)
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
    // give hint that continuation position for spell/grammar checking is
    // at the end of this sentence
    if (pSpellIter)
    {
        pSpellIter->SetCurr( new SwPosition( *pSpellIter->GetCurrX() ) );
        pSpellIter->ContinueAfterThisSentence();
    }
}


void SwEditShell::ApplyChangedSentence(const ::svx::SpellPortions& rNewPortions, bool bRecheck)
{
    // Note: rNewPortions.size() == 0 is valid and happens when the whole
    // sentence got removed in the dialog

    OSL_ENSURE(  pSpellIter, "SpellIter missing" );
    if(pSpellIter &&
       pSpellIter->GetLastPortions().size() > 0)    // no portions -> no text to be changed
    {
        const SpellPortions& rLastPortions = pSpellIter->GetLastPortions();
        const SpellContentPositions  rLastPositions = pSpellIter->GetLastPositions();
        OSL_ENSURE(rLastPortions.size() > 0 &&
                rLastPortions.size() == rLastPositions.size(),
                "last vectors of spelling results are not set or not equal");

        // iterate over the new portions, beginning at the end to take advantage of the previously
        // saved content positions

        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_UI_TEXT_CORRECTION, NULL );
        StartAction();

        SwPaM *pCrsr = GetCrsr();
        // save cursor position (which should be at the end of the current sentence)
        // for later restoration
        Push();

        sal_uInt32 nRedlinePortions = lcl_CountRedlines(rLastPortions);
        if((rLastPortions.size() - nRedlinePortions) == rNewPortions.size())
        {
            OSL_ENSURE( !rNewPortions.empty(), "rNewPortions should not be empty here" );
            OSL_ENSURE( !rLastPortions.empty(), "rLastPortions should not be empty here" );
            OSL_ENSURE( !rLastPositions.empty(), "rLastPositions should not be empty here" );

            //the simple case: the same number of elements on both sides
            //each changed element has to be applied to the corresponding source element
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.end();
            SpellPortions::const_iterator aCurrentOldPortion = rLastPortions.end();
            SpellContentPositions::const_iterator aCurrentOldPosition = rLastPositions.end();
            do
            {
                --aCurrentNewPortion;
                --aCurrentOldPortion;
                --aCurrentOldPosition;
                //jump over redline portions
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
                    //change text ...
                    pDoc->DeleteAndJoin(*pCrsr);
                    // ... and apply language if necessary
                    if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                        SetAttr( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId), nLangWhichId );
                    pDoc->InsertString(*pCrsr, aCurrentNewPortion->sText);
                }
                else if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                {
                    //apply language
                    SetAttr( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId), nLangWhichId );
                }
                else if( aCurrentNewPortion->bIgnoreThisError )
                {
                    //add the 'ignore' markup to the TextNode's grammar ignore markup list
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

            //select the complete sentence
            SpellContentPositions::const_iterator aCurrentEndPosition = rLastPositions.end();
            --aCurrentEndPosition;
            SpellContentPositions::const_iterator aCurrentStartPosition = rLastPositions.begin();
            pCrsr->GetPoint()->nContent = aCurrentStartPosition->nLeft;
            pCrsr->GetMark()->nContent = aCurrentEndPosition->nRight;

            //delete the sentence completely
            pDoc->DeleteAndJoin(*pCrsr);
            svx::SpellPortions::const_iterator aCurrentNewPortion = rNewPortions.begin();
            while(aCurrentNewPortion != rNewPortions.end())
            {
                //set the language attribute
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
                    SetAttr( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId) );
                //insert the new string
                pDoc->InsertString(*pCrsr, aCurrentNewPortion->sText);

                //set the cursor to the end of the inserted string
                *pCrsr->Start() = *pCrsr->End();
                ++aCurrentNewPortion;
            }
        }

        // restore cursor to the end of the sentence
        // (will work also if the sentence length has changed,
        // since cursors get updated automatically!)
        Pop( sal_False );

        // collapse cursor to the end of the modified sentence
        *pCrsr->Start() = *pCrsr->End();
        if (bRecheck)
        {
            //in grammar check the current sentence has to be checked again
            GoStartSentence();
        }
        // set continuation position for spell/grammar checking to the end of this sentence
        pSpellIter->SetCurr( new SwPosition( *pCrsr->Start() ) );

        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_UI_TEXT_CORRECTION, NULL );
        EndAction();
    }
}
/*-------------------------------------------------------------------------
    collect all deleted redlines of the current text node beginning at the
    start of the cursor position
  -----------------------------------------------------------------------*/
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
        const xub_StrLen nStartIndex = pStartPos->nContent.GetIndex();
        for ( ; nAct < pDoc->GetRedlineTbl().size(); nAct++ )
        {
            const SwRedline* pRed = pDoc->GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > pTxtNode->GetIndex() )
                break;

            if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
            {
                xub_StrLen nStart, nEnd;
                pRed->CalcStartEnd( pTxtNode->GetIndex(), nStart, nEnd );
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
/*-------------------------------------------------------------------------
    remove the redline positions after the current selection
  -----------------------------------------------------------------------*/
static void lcl_CutRedlines( SpellContentPositions& aDeletedRedlines, SwEditShell* pSh )
{
    if(!aDeletedRedlines.empty())
    {
        SwPaM *pCrsr = pSh->GetCrsr();
        const SwPosition* pEndPos = pCrsr->End();
        xub_StrLen nEnd = pEndPos->nContent.GetIndex();
        while(!aDeletedRedlines.empty() &&
                aDeletedRedlines.back().nLeft > nEnd)
        {
            aDeletedRedlines.pop_back();
        }
    }
}

static SpellContentPosition  lcl_FindNextDeletedRedline(
        const SpellContentPositions& rDeletedRedlines,
        xub_StrLen nSearchFrom )
{
    SpellContentPosition aRet;
    aRet.nLeft = aRet.nRight = STRING_MAXLEN;
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

    OSL_ENSURE( GetEnd(), "SwEditShell::SpellSentence() ohne Start?");

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
    }
    while ( bGoOn );
    if(xSpellRet.is() || bGrammarErrorFound)
    {
        //an error has been found
        //To fill the spell portions the beginning of the sentence has to be found
        SwPaM *pCrsr = pMySh->GetCrsr();
        //set the mark to the right if necessary
        if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
            pCrsr->Exchange();
        //the cursor has to be collapsed on the left to go to the start of the sentence - if sentence ends inside of the error
        pCrsr->DeleteMark();
        pCrsr->SetMark();
        sal_Bool bStartSent = 0 != pMySh->GoStartSentence();
        SpellContentPositions aDeletedRedlines = lcl_CollectDeletedRedlines(pMySh);
        if(bStartSent)
        {
            //create a portion from the start part
            AddPortion(0, 0, aDeletedRedlines);
        }
        //Set the cursor to the error already found
        *pCrsr->GetPoint() = *GetCurrX();
        *pCrsr->GetMark() = *GetCurr();
        AddPortion(xSpellRet, &aGrammarResult, aDeletedRedlines);


        //save the end position of the error to continue from here
        SwPosition aSaveStartPos = *pCrsr->End();
        //determine the end of the current sentence
        if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
            pCrsr->Exchange();
        //again collapse to start marking after the end of the error
        pCrsr->DeleteMark();
        pCrsr->SetMark();

        pMySh->GoEndSentence();
        if( bGrammarErrorFound )
        {
            const ModelToViewHelper aConversionMap(*(SwTxtNode*)pCrsr->GetNode());
            rtl::OUString aExpandText = aConversionMap.getViewText();
            xub_StrLen nSentenceEnd = (xub_StrLen)aConversionMap.ConvertToViewPosition( aGrammarResult.nBehindEndOfSentencePosition );
            // remove trailing space
            if( aExpandText[nSentenceEnd - 1] == ' ' )
                --nSentenceEnd;
            if( pCrsr->End()->nContent.GetIndex() < nSentenceEnd )
            {
                pCrsr->End()->nContent.Assign(
                    pCrsr->End()->nNode.GetNode().GetCntntNode(), nSentenceEnd);
            }
        }

        lcl_CutRedlines( aDeletedRedlines, pMySh );
        //save the 'global' end of the spellchecking
        const SwPosition aSaveEndPos = *GetEnd();
        //set the sentence end as 'local' end
        SetEnd( new SwPosition( *pCrsr->End() ));

        *pCrsr->GetPoint() = aSaveStartPos;
        *pCrsr->GetMark() = *GetEnd();
        //now the rest of the sentence has to be searched for errors
        // for each error the non-error text between the current and the last error has
        // to be added to the portions - if necessary broken into same-language-portions
        if( !bGrammarErrorFound ) //in grammar check there's only one error returned
        {
            do
            {
                xSpellRet = 0;
                // don't search for grammar errors here anymore!
                pMySh->GetDoc()->Spell(*pCrsr,
                            xSpeller, 0, 0, false ) >>= xSpellRet;
                if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                    pCrsr->Exchange();
                SetCurr( new SwPosition( *pCrsr->GetPoint() ));
                SetCurrX( new SwPosition( *pCrsr->GetMark() ));

                //if an error has been found go back to the text
                //preceeding the error
                if(xSpellRet.is())
                {
                    *pCrsr->GetPoint() = aSaveStartPos;
                    *pCrsr->GetMark() = *GetCurr();
                }
                //add the portion
                AddPortion(0, 0, aDeletedRedlines);

                if(xSpellRet.is())
                {
                    *pCrsr->GetPoint() = *GetCurr();
                    *pCrsr->GetMark() = *GetCurrX();
                    AddPortion(xSpellRet, 0, aDeletedRedlines);
                    //move the cursor to the end of the error string
                    *pCrsr->GetPoint() = *GetCurrX();
                    //and save the end of the error as new start position
                    aSaveStartPos = *GetCurrX();
                    //and the end of the sentence
                    *pCrsr->GetMark() = *GetEnd();
                }
                // if the end of the sentence has already been reached then break here
                if(*GetCurrX() >= *GetEnd())
                    break;
            }
            while(xSpellRet.is());
        }
        else
        {
            //go to the end of sentence as the grammar check returned it
            // at this time the Point is behind the grammar error
            // and the mark points to the sentence end as
            if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
                pCrsr->Exchange();
        }

        // the part between the last error and the end of the sentence has to be added
        *pMySh->GetCrsr()->GetPoint() = *GetEnd();
        if(*GetCurrX() < *GetEnd())
        {
            AddPortion(0, 0, aDeletedRedlines);
        }
        //set the shell cursor to the end of the sentence to prevent a visible selection
        *pCrsr->GetMark() = *GetEnd();
        if( !bIsGrammarCheck )
        {
            //set the current position to the end of the sentence
            SetCurr( new SwPosition(*GetEnd()) );
        }
        //restore the 'global' end
        SetEnd( new SwPosition(aSaveEndPos) );
        rPortions = aLastPortions;
        bRet = true;
    }
    else
    {
        //if no error could be found the selection has to be corrected - at least if it's not in the body
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
/*-------------------------------------------------------------------------
    create a text portion at the given position
  -----------------------------------------------------------------------*/
void SwSpellIter::CreatePortion(uno::Reference< XSpellAlternatives > xAlt,
                        linguistic2::ProofreadingResult* pGrammarResult,
        bool bIsField, bool bIsHidden)
{
    svx::SpellPortion aPortion;
    String sText;
    GetSh()->GetSelectedText( sText );
    if(sText.Len())
    {
        //in case of redlined deletions the selection of an error is not
        //the same as the _real_ word
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
    String sText;
    pMySh->GetSelectedText( sText );
    if(sText.Len())
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
            //save the start and end positions
            SwPosition aStart(*pCrsr->GetPoint());
            SwPosition aEnd(*pCrsr->GetMark());
            //iterate over the text to find changes in language
            //set the mark equal to the point
            *pCrsr->GetMark() = aStart;
            SwTxtNode* pTxtNode = pCrsr->GetNode()->GetTxtNode();
            LanguageType eStartLanguage = lcl_GetLanguage(*GetSh());
            SpellContentPosition  aNextRedline = lcl_FindNextDeletedRedline(
                        rDeletedRedlines, aStart.nContent.GetIndex() );
            if( aNextRedline.nLeft == aStart.nContent.GetIndex() )
            {
                //select until the end of the current redline
                xub_StrLen nEnd = aEnd.nContent.GetIndex() < aNextRedline.nRight ?
                            aEnd.nContent.GetIndex() : aNextRedline.nRight;
                pCrsr->GetPoint()->nContent.Assign( pTxtNode, nEnd );
                CreatePortion(xAlt, pGrammarResult, false, true);
                aStart = *pCrsr->End();
                //search for next redline
                aNextRedline = lcl_FindNextDeletedRedline(
                            rDeletedRedlines, aStart.nContent.GetIndex() );
            }
            while(*pCrsr->GetPoint() < aEnd)
            {
                //#125786 in table cell with fixed row height the cursor might not move forward
                if(!GetSh()->Right(1, CRSR_SKIP_CELLS))
                    break;

                bool bField = false;
                //read the character at the current position to check if it's a field
                sal_Unicode cChar = pTxtNode->GetTxt().GetChar( pCrsr->GetMark()->nContent.GetIndex() );
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
                        case RES_TXTATR_FTN:
                        case RES_TXTATR_FLYCNT:
                            bField = true;
                            break;
                    }
                }

                LanguageType eCurLanguage = lcl_GetLanguage(*GetSh());
                bool bRedline = aNextRedline.nLeft == pCrsr->GetPoint()->nContent.GetIndex();
                // create a portion if the next character
                //  - is a field,
                //  - is at the beginning of a deleted redline
                //  - has a different language
                if(bField || bRedline || eCurLanguage != eStartLanguage)
                {
                    eStartLanguage = eCurLanguage;
                    //go one step back - the cursor currently selects the first character
                    //with a different language
                    //in the case of redlining it's different
                    if(eCurLanguage != eStartLanguage || bField)
                        *pCrsr->GetPoint() = *pCrsr->GetMark();
                    //set to the last start
                    *pCrsr->GetMark() = aStart;
                    //create portion should only be called if a selection exists
                    //there's no selection if there's a field at the beginning
                    if(*pCrsr->Start() != *pCrsr->End())
                        CreatePortion(xAlt, pGrammarResult, false, false);
                    aStart = *pCrsr->End();
                    //now export the field - if there is any
                    if(bField)
                    {
                        *pCrsr->GetMark() = *pCrsr->GetPoint();
                        GetSh()->Right(1, CRSR_SKIP_CELLS);
                        CreatePortion(xAlt, pGrammarResult, true, false);
                        aStart = *pCrsr->End();
                    }
                }
                // if a redline start then create a portion for it
                if(bRedline)
                {
                    *pCrsr->GetMark() = *pCrsr->GetPoint();
                    //select until the end of the current redline
                    xub_StrLen nEnd = aEnd.nContent.GetIndex() < aNextRedline.nRight ?
                                aEnd.nContent.GetIndex() : aNextRedline.nRight;
                    pCrsr->GetPoint()->nContent.Assign( pTxtNode, nEnd );
                    CreatePortion(xAlt, pGrammarResult, false, true);
                    aStart = *pCrsr->End();
                    //search for next redline
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
    xub_StrLen nStart = rErrorPosition.Start()->nContent.GetIndex();
    xub_StrLen nEnd = STRING_LEN;
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
