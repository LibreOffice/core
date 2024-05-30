/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <o3tl/any.hxx>

#include <unoflatpara.hxx>

#include <strings.hrc>
#include <hintids.hxx>
#include <osl/diagnose.h>
#include <unotools/linguprops.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/langitem.hxx>
#include <editeng/SpellPortions.hxx>
#include <svl/languageoptions.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <viewopt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <mdiexp.hxx>
#include <cntfrm.hxx>
#include <splargs.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <txatbase.hxx>
#include <txtfrm.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

namespace {

class SwLinguIter
{
    SwEditShell* m_pSh;
public:
    std::optional<SwPosition> m_oStart;
    std::optional<SwPosition> m_oEnd;
    std::optional<SwPosition> m_oCurr;
    std::optional<SwPosition> m_oCurrX;
    sal_uInt16 m_nCursorCount;

    SwLinguIter();

    SwEditShell* GetSh() { return m_pSh; }

    sal_uInt16& GetCursorCnt() { return m_nCursorCount; }

    // for the UI:
    void Start_( SwEditShell *pSh, SwDocPositions eStart,
                SwDocPositions eEnd );
    void End_(bool bRestoreSelection = true);
};

// #i18881# to be able to identify the positions of the changed words
// the content positions of each portion need to be saved
struct SpellContentPosition
{
    sal_Int32 nLeft;
    sal_Int32 nRight;
};

}

typedef std::vector<SpellContentPosition>  SpellContentPositions;

namespace {

class SwSpellIter : public SwLinguIter
{
    uno::Reference<XSpellChecker1> m_xSpeller;
    svx::SpellPortions m_aLastPortions;

    SpellContentPositions m_aLastPositions;
    bool m_bBackToStartOfSentence;

    void    CreatePortion(uno::Reference< XSpellAlternatives > const & xAlt,
                const linguistic2::ProofreadingResult* pGrammarResult,
                bool bIsField, bool bIsHidden);

    void    AddPortion(uno::Reference< XSpellAlternatives > const & xAlt,
                       const linguistic2::ProofreadingResult* pGrammarResult,
                       const SpellContentPositions& rDeletedRedlines);
public:
    SwSpellIter()
        : m_bBackToStartOfSentence(false)
    {
    }

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    bool                                SpellSentence(svx::SpellPortions& rPortions, bool bIsGrammarCheck);
    void                                ToSentenceStart();
    const svx::SpellPortions& GetLastPortions() const { return m_aLastPortions; }
    const SpellContentPositions& GetLastPositions() const { return m_aLastPositions; }
};

/// used for text conversion
class SwConvIter : public SwLinguIter
{
    SwConversionArgs& m_rArgs;

public:
    explicit SwConvIter(SwConversionArgs& rConvArgs)
        : m_rArgs(rConvArgs)
    {
    }

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );
};

class SwHyphIter : public SwLinguIter
{
    // With that we save a GetFrame() in Hyphenate //TODO: does it actually matter?
    const SwTextNode *m_pLastNode;
    SwTextFrame  *m_pLastFrame;
    friend SwTextFrame * sw::SwHyphIterCacheLastTextFrame(SwTextNode const * pNode, const sw::Creator& rCreator);

    bool m_bOldIdle;
    static void DelSoftHyph( SwPaM &rPam );

public:
    SwHyphIter()
        : m_pLastNode(nullptr)
        , m_pLastFrame(nullptr)
        , m_bOldIdle(false)
    {
    }

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );
    void End();

    void Ignore();

    uno::Any    Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    static bool IsAuto();
    void InsertSoftHyph( const sal_Int32 nHyphPos );
    void ShowSelection();
};

}

static SwSpellIter* g_pSpellIter = nullptr;
static SwConvIter*  g_pConvIter = nullptr;
static SwHyphIter*  g_pHyphIter = nullptr;

SwLinguIter::SwLinguIter()
    : m_pSh(nullptr)
    , m_nCursorCount(0)
{
    // TODO missing: ensurance of re-entrance, OSL_ENSURE( etc.
}

void SwLinguIter::Start_( SwEditShell *pShell, SwDocPositions eStart,
                            SwDocPositions eEnd )
{
    // TODO missing: ensurance of re-entrance, locking
    if (m_pSh)
        return;

    bool bSetCurr;

    m_pSh = pShell;

    CurrShell aCurr(m_pSh);

    OSL_ENSURE(!m_oEnd, "SwLinguIter::Start_ without End?");

    SwPaM* pCursor = m_pSh->GetCursor();

    if( pShell->HasSelection() || pCursor != pCursor->GetNext() )
    {
        bSetCurr = m_oCurr.has_value();
        m_nCursorCount = m_pSh->GetCursorCnt();
        if (m_pSh->IsTableMode())
            m_pSh->TableCursorToCursor();

        m_pSh->Push();
        sal_uInt16 n;
        for (n = 0; n < m_nCursorCount; ++n)
        {
            m_pSh->Push();
            m_pSh->DestroyCursor();
        }
        m_pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
    else
    {
        bSetCurr = false;
        m_nCursorCount = 1;
        m_pSh->Push();
        m_pSh->SetLinguRange(eStart, eEnd);
    }

    pCursor = m_pSh->GetCursor();
    if ( *pCursor->GetPoint() > *pCursor->GetMark() )
        pCursor->Exchange();

    m_oStart.emplace(*pCursor->GetPoint());
    m_oEnd.emplace(*pCursor->GetMark());
    if( bSetCurr )
    {
        m_oCurr.emplace( *m_oStart );
        m_oCurrX.emplace( *m_oCurr );
    }

    pCursor->SetMark();
}

void SwLinguIter::End_(bool bRestoreSelection)
{
    if (!m_pSh)
        return;

    OSL_ENSURE(m_oEnd, "SwLinguIter::End_ without end?");
    if(bRestoreSelection)
    {
        while (m_nCursorCount--)
            m_pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);

        m_pSh->KillPams();
        m_pSh->ClearMark();
    }
    m_oStart.reset();
    m_oEnd.reset();
    m_oCurr.reset();
    m_oCurrX.reset();

    m_pSh = nullptr;
}

void SwSpellIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;

    m_xSpeller = ::GetSpellChecker();
    if (m_xSpeller.is())
        Start_( pShell, eStart, eEnd );
    m_aLastPortions.clear();
    m_aLastPositions.clear();
}

// This method is the origin of SwEditShell::SpellContinue()
uno::Any SwSpellIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    //!!
    //!! Please check SwConvIter also when modifying this
    //!!

    uno::Any    aSpellRet;
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aSpellRet;

    OSL_ENSURE( m_oEnd, "SwSpellIter::Continue without start?");

    uno::Reference< uno::XInterface >  xSpellRet;
    bool bGoOn = true;
    do {
        SwPaM *pCursor = pMySh->GetCursor();
        if ( !pCursor->HasMark() )
            pCursor->SetMark();

        *pMySh->GetCursor()->GetPoint() = *m_oCurr;
        *pMySh->GetCursor()->GetMark() = *m_oEnd;
        pMySh->GetDoc()->Spell(*pMySh->GetCursor(), m_xSpeller, pPageCnt, pPageSt, false,
                               pMySh->GetLayout())
            >>= xSpellRet;
        bGoOn = GetCursorCnt() > 1;
        if( xSpellRet.is() )
        {
            bGoOn = false;
            m_oCurr.emplace( *pCursor->GetPoint()  );
            m_oCurrX.emplace( *pCursor->GetMark() );
        }
        if( bGoOn )
        {
            pMySh->Pop(SwCursorShell::PopMode::DeleteCurrent);
            pCursor = pMySh->GetCursor();
            if ( *pCursor->GetPoint() > *pCursor->GetMark() )
                pCursor->Exchange();
            m_oStart.emplace( *pCursor->GetPoint() );
            m_oEnd.emplace( *pCursor->GetMark() );
            m_oCurr.emplace( *m_oStart );
            m_oCurrX.emplace( *m_oCurr );
            pCursor->SetMark();
            --GetCursorCnt();
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
    Start_( pShell, eStart, eEnd );
}

uno::Any SwConvIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    //!!
    //!! Please check SwSpellIter also when modifying this
    //!!

    uno::Any    aConvRet{ OUString() };
    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return aConvRet;

    OSL_ENSURE( m_oEnd, "SwConvIter::Continue() without Start?");

    OUString aConvText;
    bool bGoOn = true;
    do {
        SwPaM *pCursor = pMySh->GetCursor();
        if ( !pCursor->HasMark() )
            pCursor->SetMark();

        *pMySh->GetCursor()->GetPoint() = *m_oCurr;
        *pMySh->GetCursor()->GetMark() = *m_oEnd;

        // call function to find next text portion to be converted
        uno::Reference< linguistic2::XSpellChecker1 > xEmpty;
        pMySh->GetDoc()->Spell(*pMySh->GetCursor(), xEmpty, pPageCnt, pPageSt, false,
                               pMySh->GetLayout(), &m_rArgs)
            >>= aConvText;

        bGoOn = GetCursorCnt() > 1;
        if( !aConvText.isEmpty() )
        {
            bGoOn = false;

            m_oCurr.emplace( *pCursor->GetPoint() );
            m_oCurrX.emplace( *pCursor->GetMark() );
        }
        if( bGoOn )
        {
            pMySh->Pop(SwCursorShell::PopMode::DeleteCurrent);
            pCursor = pMySh->GetCursor();
            if ( *pCursor->GetPoint() > *pCursor->GetMark() )
                pCursor->Exchange();
            m_oStart.emplace( *pCursor->GetPoint() );
            m_oEnd.emplace( *pCursor->GetMark() );
            m_oCurr.emplace( *m_oStart );
            m_oCurrX.emplace( *m_oCurr );
            pCursor->SetMark();
            --GetCursorCnt();
        }
    }while ( bGoOn );
    return Any( aConvText );
}

bool SwHyphIter::IsAuto()
{
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    return xProp.is() && *o3tl::doAccess<bool>(xProp->getPropertyValue(
                                UPN_IS_HYPH_AUTO ));
}

void SwHyphIter::ShowSelection()
{
    SwEditShell *pMySh = GetSh();
    if( pMySh )
    {
        pMySh->StartAction();
        // Caution! Due to EndAction() formatting is started which can lead to the fact that new
        // words are added to/set in the Hyphenator. Thus: save!
        pMySh->EndAction();
    }
}

void SwHyphIter::Start( SwEditShell *pShell, SwDocPositions eStart, SwDocPositions eEnd )
{
    // robust
    if( GetSh() || m_oEnd )
    {
        OSL_ENSURE( !GetSh(), "SwHyphIter::Start: missing HyphEnd()" );
        return;
    }

    // nothing to do (at least not in the way as in the "else" part)
    m_bOldIdle = pShell->GetViewOptions()->IsIdle();
    pShell->GetViewOptions()->SetIdle( false );
    Start_( pShell, eStart, eEnd );
}

// restore selections
void SwHyphIter::End()
{
    if( !GetSh() )
        return;
    GetSh()->GetViewOptions()->SetIdle(m_bOldIdle);
    End_();
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
        SwPaM *pCursor;
        do {
            OSL_ENSURE( m_oEnd, "SwHyphIter::Continue without Start?" );
            pCursor = pMySh->GetCursor();
            if ( !pCursor->HasMark() )
                pCursor->SetMark();
            if ( *pCursor->GetPoint() < *pCursor->GetMark() )
            {
                pCursor->Exchange();
                pCursor->SetMark();
            }

            if ( *pCursor->End() <= *m_oEnd )
            {
                *pCursor->GetMark() = *m_oEnd;

                // Do we need to break the word at the current cursor position?
                const Point aCursorPos( pMySh->GetCharRect().Pos() );
                xHyphWord = pMySh->GetDoc()->Hyphenate( pCursor, aCursorPos,
                                                       pPageCnt, pPageSt );
            }

            if( bAuto && xHyphWord.is() )
            {
                SwEditShell::InsertSoftHyph( xHyphWord->getHyphenationPos() + 1);
            }
        } while( bAuto && xHyphWord.is() ); //end of do-while
        bGoOn = !xHyphWord.is() && GetCursorCnt() > 1;

        if( bGoOn )
        {
            pMySh->Pop(SwCursorShell::PopMode::DeleteCurrent);
            pCursor = pMySh->GetCursor();
            if ( *pCursor->GetPoint() > *pCursor->GetMark() )
                pCursor->Exchange();
            m_oEnd.emplace( *pCursor->End() );
            pCursor->SetMark();
            --GetCursorCnt();
        }
    } while ( bGoOn );
    aHyphRet <<= xHyphWord;
    return aHyphRet;
}

/// ignore hyphenation
void SwHyphIter::Ignore()
{
    SwEditShell *pMySh = GetSh();
    SwPaM *pCursor = pMySh->GetCursor();

    // delete old SoftHyphen
    DelSoftHyph( *pCursor );

    // and continue
    pCursor->Start()->SetContent( pCursor->End()->GetContentIndex() );
    pCursor->SetMark();
}

void SwHyphIter::DelSoftHyph( SwPaM &rPam )
{
    const SwPosition* pStt = rPam.Start();
    const sal_Int32 nStart = pStt->GetContentIndex();
    const sal_Int32 nEnd   = rPam.End()->GetContentIndex();
    SwTextNode *pNode = pStt->GetNode().GetTextNode();
    pNode->DelSoftHyph( nStart, nEnd );
}

void SwHyphIter::InsertSoftHyph( const sal_Int32 nHyphPos )
{
    SwEditShell *pMySh = GetSh();
    OSL_ENSURE( pMySh,  "SwHyphIter::InsertSoftHyph: missing HyphStart()");
    if( !pMySh )
        return;

    SwPaM *pCursor = pMySh->GetCursor();
    auto [pSttPos, pEndPos] = pCursor->StartEnd(); // SwPosition*

    const sal_Int32 nLastHyphLen = m_oEnd->GetContentIndex() -
                          pSttPos->GetContentIndex();

    if( pSttPos->GetNode() != pEndPos->GetNode() || !nLastHyphLen )
    {
        OSL_ENSURE( pSttPos->GetNode() == pEndPos->GetNode(),
                "SwHyphIter::InsertSoftHyph: node warp during hyphenation" );
        OSL_ENSURE(nLastHyphLen, "SwHyphIter::InsertSoftHyph: missing HyphContinue()");
        *pSttPos = *pEndPos;
        return;
    }

    pMySh->StartAction();
    {
        SwDoc *pDoc = pMySh->GetDoc();
        DelSoftHyph( *pCursor );
        pSttPos->AdjustContent( +nHyphPos );
        SwPaM aRg( *pSttPos );
        pDoc->getIDocumentContentOperations().InsertString( aRg, OUString(CHAR_SOFTHYPHEN) );
    }
    // revoke selection
    pCursor->DeleteMark();
    pMySh->EndAction();
    pCursor->SetMark();
}

namespace sw {

SwTextFrame *
SwHyphIterCacheLastTextFrame(SwTextNode const * pNode, const sw::Creator& create)
{
    assert(g_pHyphIter);
    if (pNode != g_pHyphIter->m_pLastNode || !g_pHyphIter->m_pLastFrame)
    {
        g_pHyphIter->m_pLastNode = pNode;
        g_pHyphIter->m_pLastFrame = create();
    }
    return g_pHyphIter->m_pLastFrame;
}

}

bool SwEditShell::HasLastSentenceGotGrammarChecked()
{
    bool bTextWasGrammarChecked = false;
    if (g_pSpellIter)
    {
        svx::SpellPortions aLastPortions( g_pSpellIter->GetLastPortions() );
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

bool SwEditShell::HasConvIter()
{
    return nullptr != g_pConvIter;
}

bool SwEditShell::HasHyphIter()
{
    return nullptr != g_pHyphIter;
}

void SwEditShell::SetLinguRange( SwDocPositions eStart, SwDocPositions eEnd )
{
    SwPaM *pCursor = GetCursor();
    MakeFindRange( eStart, eEnd, pCursor );
    if( *pCursor->GetPoint() > *pCursor->GetMark() )
        pCursor->Exchange();
}

void SwEditShell::SpellStart(
        SwDocPositions eStart, SwDocPositions eEnd, SwDocPositions eCurr,
        SwConversionArgs *pConvArgs )
{
    SwLinguIter *pLinguIter = nullptr;

    // do not spell if interactive spelling is active elsewhere
    if (!pConvArgs && !g_pSpellIter)
    {
        g_pSpellIter = new SwSpellIter;
        pLinguIter = g_pSpellIter;
    }
    // do not do text conversion if it is active elsewhere
    if (pConvArgs && !g_pConvIter)
    {
        g_pConvIter = new SwConvIter( *pConvArgs );
        pLinguIter = g_pConvIter;
    }

    if (pLinguIter)
    {
        SwCursor* pSwCursor = GetCursor();

        pLinguIter->m_oCurr.emplace( *pSwCursor->GetPoint() );
        pSwCursor->FillFindPos( eCurr, *pLinguIter->m_oCurr );

        pLinguIter->m_oCurrX.emplace( *pLinguIter->m_oCurr );
    }

    if (!pConvArgs && g_pSpellIter)
        g_pSpellIter->Start( this, eStart, eEnd );
    if (pConvArgs && g_pConvIter)
        g_pConvIter->Start( this, eStart, eEnd );
}

void SwEditShell::SpellEnd( SwConversionArgs const *pConvArgs, bool bRestoreSelection )
{
    if (!pConvArgs && g_pSpellIter && g_pSpellIter->GetSh() == this)
    {
        g_pSpellIter->End_(bRestoreSelection);
        delete g_pSpellIter;
        g_pSpellIter = nullptr;
    }
    if (pConvArgs && g_pConvIter && g_pConvIter->GetSh() == this)
    {
        g_pConvIter->End_();
        delete g_pConvIter;
        g_pConvIter = nullptr;
    }
}

/// @returns SPL_ return values as in splchk.hxx
uno::Any SwEditShell::SpellContinue(
        sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
        SwConversionArgs const *pConvArgs )
{
    uno::Any aRes;

    if ((!pConvArgs && g_pSpellIter->GetSh() != this) ||
        ( pConvArgs && g_pConvIter->GetSh() != this))
        return aRes;

    if( pPageCnt && !*pPageCnt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        *pPageCnt = nEndPage;
        if( nEndPage )
            ::StartProgress( STR_STATSTR_SPELL, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    OSL_ENSURE(  pConvArgs || g_pSpellIter, "SpellIter missing" );
    OSL_ENSURE( !pConvArgs || g_pConvIter,  "ConvIter missing" );
    //JP 18.07.95: prevent displaying selection on error messages. NO StartAction so that all
    //             Paints are also disabled.
    ++mnStartAction;
    OUString aRet;
    uno::Reference< uno::XInterface >  xRet;
    if (pConvArgs)
    {
        g_pConvIter->Continue( pPageCnt, pPageSt ) >>= aRet;
        aRes <<= aRet;
    }
    else
    {
        g_pSpellIter->Continue( pPageCnt, pPageSt ) >>= xRet;
        aRes <<= xRet;
    }
    --mnStartAction;

    if( !aRet.isEmpty() || xRet.is() )
    {
        // then make awt::Selection again visible
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
 *    - if no selections existent:
 *      - create new selection reaching until document end
 * 2) HyphContinue
 *    - add nLastHyphLen onto SelectionStart
 *    - iterate over all selected areas
 *      - pDoc->Hyphenate() iterates over all Nodes of a selection
 *          - pTextNode->Hyphenate() calls SwTextFrame::Hyphenate of the EditShell
 *              - SwTextFrame:Hyphenate() iterates over all rows of the Pam
 *                  - LineIter::Hyphenate() sets the Hyphenator and the Pam based on
 *                    the to be separated word.
 *    - Returns true if there is a hyphenation and false if the Pam is processed.
 *      - If true, show the selected word and set nLastHyphLen.
 *      - If false, delete current selection and select next one. Returns HYPH_OK if no more.
 * 3) InsertSoftHyph (might be called by UI if needed)
 *    - Place current cursor and add attribute.
 * 4) HyphEnd
 *    - Restore old cursor, EndAction
 */
void SwEditShell::HyphStart( SwDocPositions eStart, SwDocPositions eEnd )
{
    // do not hyphenate if interactive hyphenation is active elsewhere
    if (!g_pHyphIter)
    {
        g_pHyphIter = new SwHyphIter;
        g_pHyphIter->Start( this, eStart, eEnd );
    }
}

/// restore selections
void SwEditShell::HyphEnd()
{
    assert(g_pHyphIter);
    if (g_pHyphIter->GetSh() == this)
    {
        g_pHyphIter->End();
        delete g_pHyphIter;
        g_pHyphIter = nullptr;
    }
}

/// @returns HYPH_CONTINUE if hyphenation, HYPH_OK if selected area was processed.
uno::Reference< uno::XInterface >
    SwEditShell::HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    assert(g_pHyphIter);
    if (g_pHyphIter->GetSh() != this)
        return nullptr;

    if( pPageCnt && !*pPageCnt && !*pPageSt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        if( nEndPage > 14 )
        {
            *pPageCnt = nEndPage;
            ::StartProgress( STR_STATSTR_HYPHEN, 0, nEndPage, GetDoc()->GetDocShell());
        }
        else                // here we once and for all suppress StatLineStartPercent
            *pPageSt = 1;
    }

    //JP 18.07.95: prevent displaying selection on error messages. NO StartAction so that all
    //             Paints are also disabled.
    ++mnStartAction;
    uno::Reference< uno::XInterface >  xRet;
    g_pHyphIter->Continue( pPageCnt, pPageSt ) >>= xRet;
    --mnStartAction;

    if( xRet.is() )
        g_pHyphIter->ShowSelection();

    return xRet;
}

/** Insert soft hyphen
 *
 * @param nHyphPos Offset in the to be separated word
 */
void SwEditShell::InsertSoftHyph( const sal_Int32 nHyphPos )
{
    assert(g_pHyphIter);
    g_pHyphIter->InsertSoftHyph( nHyphPos );
}

/// ignore hyphenation
void SwEditShell::HyphIgnore()
{
    assert(g_pHyphIter);
    //JP 18.07.95: prevent displaying selection on error messages. NO StartAction so that all
    //             Paints are also disabled.
    ++mnStartAction;
    g_pHyphIter->Ignore();
    --mnStartAction;

    g_pHyphIter->ShowSelection();
}

void SwEditShell::HandleCorrectionError(const OUString& aText, SwPosition aPos, sal_Int32 nBegin,
                                        sal_Int32 nLen, const Point* pPt,
                                        SwRect& rSelectRect)
{
    // save the start and end positions of the line and the starting point
    SwNode const& rNode(GetCursor()->GetPoint()->GetNode());
    Push();
    LeftMargin();
    const sal_Int32 nLineStart = &rNode == &GetCursor()->GetPoint()->GetNode()
        ? GetCursor()->GetPoint()->GetContentIndex()
        : 0;
    RightMargin();
    const sal_Int32 nLineEnd = &rNode == &GetCursor()->GetPoint()->GetNode()
        ? GetCursor()->GetPoint()->GetContentIndex()
        : rNode.GetTextNode()->Len();
    Pop(PopMode::DeleteCurrent);

    // make sure the selection build later from the data below does
    // not "in word" character to the left and right in order to
    // preserve those. Therefore count those "in words" in order to
    // modify the selection accordingly.
    const sal_Unicode* pChar = aText.getStr();
    sal_Int32 nLeft = 0;
    while (*pChar++ == CH_TXTATR_INWORD)
        ++nLeft;
    pChar = aText.getLength() ? aText.getStr() + aText.getLength() - 1 : nullptr;
    sal_Int32 nRight = 0;
    while (pChar && *pChar-- == CH_TXTATR_INWORD)
        ++nRight;

    aPos.SetContent( nBegin + nLeft );
    SwPaM* pCursor = GetCursor();
    *pCursor->GetPoint() = aPos;
    pCursor->SetMark();
    ExtendSelection( true, nLen - nLeft - nRight );
    // don't determine the rectangle in the current line
    const sal_Int32 nWordStart = (nBegin + nLeft) < nLineStart ? nLineStart : nBegin + nLeft;
    // take one less than the line end - otherwise the next line would be calculated
    const sal_Int32 nWordEnd = (nBegin + nLen - nLeft - nRight) > nLineEnd
                            ? nLineEnd : (nBegin + nLen - nLeft - nRight);
    Push();
    pCursor->DeleteMark();
    SwPosition& rPtPos = *GetCursor()->GetPoint();
    rPtPos.SetContent(nWordStart);
    SwRect aStartRect;
    SwCursorMoveState aState;
    aState.m_bRealWidth = true;
    SwContentNode* pContentNode = pCursor->GetPointContentNode();
    std::pair<Point, bool> tmp;
    if (pPt)
    {
        tmp.first = *pPt;
        tmp.second = false;
    }
    SwContentFrame *const pContentFrame = pContentNode->getLayoutFrame(GetLayout(), pCursor->GetPoint(), pPt ? &tmp : nullptr);

    pContentFrame->GetCharRect( aStartRect, *pCursor->GetPoint(), &aState );
    rPtPos.SetContent(nWordEnd - 1);
    SwRect aEndRect;
    pContentFrame->GetCharRect( aEndRect, *pCursor->GetPoint(),&aState );
    rSelectRect = aStartRect.Union( aEndRect );
    Pop(PopMode::DeleteCurrent);
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
        return nullptr;
    SwPaM* pCursor = GetCursor();
    SwPosition aPos( *pCursor->GetPoint() );
    SwCursorMoveState eTmpState( CursorMoveState::SetOnlyText );
    eTmpState.m_bPosMatchesBounds = true; // treat last half of character same as first half
    SwTextNode *pNode = nullptr;
    SwWrongList *pWrong = nullptr;
    if (pPt && GetLayout()->GetModelPositionForViewPoint( &aPos, *const_cast<Point*>(pPt), &eTmpState ))
        pNode = aPos.GetNode().GetTextNode();
    if (nullptr == pNode)
        pNode = pCursor->GetPointNode().GetTextNode();
    if (nullptr != pNode)
        pWrong = pNode->GetWrong();
    if (nullptr != pWrong && !pNode->IsInProtectSect())
    {
        sal_Int32 nBegin = aPos.GetContentIndex();
        sal_Int32 nLen = 1;
        if (pWrong->InWrongWord(nBegin, nLen) && !pNode->IsSymbolAt(nBegin))
        {
            const OUString aText(pNode->GetText().copy(nBegin, nLen));
            // TODO: this doesn't handle fieldmarks properly
            ModelToViewHelper const aConversionMap(*pNode, GetLayout(),
                ExpandMode::ExpandFields | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode
                | ExpandMode::HideFieldmarkCommands
                | (GetLayout()->IsHideRedlines() ? ExpandMode::HideDeletions : ExpandMode(0))
                | (GetViewOptions()->IsShowHiddenChar() ? ExpandMode(0) : ExpandMode::HideInvisible));
            auto const nBeginView(aConversionMap.ConvertToViewPosition(nBegin));
            OUString const aWord(aConversionMap.getViewText().copy(nBeginView,
                aConversionMap.ConvertToViewPosition(nBegin+nLen) - nBeginView));

            uno::Reference< XSpellChecker1 >  xSpell( ::GetSpellChecker() );
            if( xSpell.is() )
            {
                LanguageType eActLang = pNode->GetLang( nBegin, nLen );
                if( xSpell->hasLanguage( static_cast<sal_uInt16>(eActLang) ))
                {
                    // restrict the maximal number of suggestions displayed
                    // in the context menu.
                    // Note: That could of course be done by clipping the
                    // resulting sequence but the current third party
                    // implementations result differs greatly if the number of
                    // suggestions to be returned gets changed. Statistically
                    // it gets much better if told to return e.g. only 7 strings
                    // than returning e.g. 16 suggestions and using only the
                    // first 7. Thus we hand down the value to use to that
                    // implementation here by providing an additional parameter.
                    Sequence< PropertyValue > aPropVals ( { comphelper::makePropertyValue( UPN_MAX_NUMBER_OF_SUGGESTIONS, sal_Int16(7)) } );

                    xSpellAlt = xSpell->spell( aWord, static_cast<sal_uInt16>(eActLang), aPropVals );
                }
            }

            if ( xSpellAlt.is() )   // error found?
            {
                HandleCorrectionError( aText, std::move(aPos), nBegin, nLen, pPt, rSelectRect );
            }
        }
    }
    return xSpellAlt;
}

bool SwEditShell::GetGrammarCorrection(
    linguistic2::ProofreadingResult /*out*/ &rResult, // the complete result
    sal_Int32 /*out*/ &rErrorPosInText,               // offset of error position in string that was grammar checked...
    sal_Int32 /*out*/ &rErrorIndexInResult,           // index of error in rResult.aGrammarErrors
    uno::Sequence< OUString > /*out*/ &rSuggestions,  // suggestions to be used for the error found
    const Point *pPt, SwRect &rSelectRect )
{
    bool bRes = false;

    if( IsTableMode() )
        return bRes;

    SwPaM* pCursor = GetCursor();
    SwPosition aPos( *pCursor->GetPoint() );
    SwCursorMoveState eTmpState( CursorMoveState::SetOnlyText );
    eTmpState.m_bPosMatchesBounds = true; // treat last half of character same as first half
    SwTextNode *pNode = nullptr;
    SwGrammarMarkUp *pWrong = nullptr;
    if (pPt && GetLayout()->GetModelPositionForViewPoint( &aPos, *const_cast<Point*>(pPt), &eTmpState ))
        pNode = aPos.GetNode().GetTextNode();
    if (nullptr == pNode)
        pNode = pCursor->GetPointNode().GetTextNode();
    if (nullptr != pNode)
        pWrong = pNode->GetGrammarCheck();
    if (nullptr != pWrong && !pNode->IsInProtectSect())
    {
        sal_Int32 nBegin = aPos.GetContentIndex();
        sal_Int32 nLen = 1;
        if (pWrong->InWrongWord(nBegin, nLen))
        {
            const OUString aText(pNode->GetText().copy(nBegin, nLen));

            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( mxDoc->GetGCIterator() );
            if (xGCIterator.is())
            {
                uno::Reference< lang::XComponent > xDoc = mxDoc->GetDocShell()->GetBaseModel();

                // Expand the string:
                const ModelToViewHelper aConversionMap(*pNode, GetLayout());
                const OUString& aExpandText = aConversionMap.getViewText();
                // get XFlatParagraph to use...
                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *pNode, aExpandText, aConversionMap );

                // get error position of cursor in XFlatParagraph
                rErrorPosInText = aConversionMap.ConvertToViewPosition( nBegin );

                const sal_Int32 nStartOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceStart( nBegin ) );
                const sal_Int32 nEndOfSentence = aConversionMap.ConvertToViewPosition( pWrong->getSentenceEnd( nBegin ) );

                rResult = xGCIterator->checkSentenceAtPosition(
                        xDoc, xFlatPara, aExpandText, lang::Locale(), nStartOfSentence,
                        nEndOfSentence == COMPLETE_STRING ? aExpandText.getLength() : nEndOfSentence,
                        rErrorPosInText );
                bRes = true;

                // get suggestions to use for the specific error position
                rSuggestions.realloc( 0 );
                // return suggestions for first error that includes the given error position
                auto pError = std::find_if(std::cbegin(rResult.aErrors), std::cend(rResult.aErrors),
                    [rErrorPosInText, nLen](const linguistic2::SingleProofreadingError &rError) {
                        return rError.nErrorStart <= rErrorPosInText
                            && rErrorPosInText + nLen <= rError.nErrorStart + rError.nErrorLength
                            && rError.aSuggestions.size() > 0; });
                if (pError != std::cend(rResult.aErrors))
                {
                    rSuggestions = pError->aSuggestions;
                    rErrorIndexInResult = static_cast<sal_Int32>(std::distance(std::cbegin(rResult.aErrors), pError));
                }
            }

            if (rResult.aErrors.hasElements())    // error found?
            {
                HandleCorrectionError( aText, std::move(aPos), nBegin, nLen, pPt, rSelectRect );
            }
        }
    }

    return bRes;
}

bool SwEditShell::SpellSentence(svx::SpellPortions& rPortions, bool bIsGrammarCheck)
{
    OSL_ENSURE(  g_pSpellIter, "SpellIter missing" );
    if (!g_pSpellIter)
        return false;
    bool bRet = g_pSpellIter->SpellSentence(rPortions, bIsGrammarCheck);

    // make Selection visible - this should simply move the
    // cursor to the end of the sentence
    StartAction();
    EndAction();
    return bRet;
}

///make SpellIter start with the current sentence when called next time
void SwEditShell::PutSpellingToSentenceStart()
{
    OSL_ENSURE(  g_pSpellIter, "SpellIter missing" );
    if (!g_pSpellIter)
        return;
    g_pSpellIter->ToSentenceStart();
}

static sal_uInt32 lcl_CountRedlines(const svx::SpellPortions& rLastPortions)
{
    return static_cast<sal_uInt32>(std::count_if(rLastPortions.begin(), rLastPortions.end(),
        [](const svx::SpellPortion& rPortion) { return rPortion.bIsHidden; }));
}

void SwEditShell::MoveContinuationPosToEndOfCheckedSentence()
{
    // give hint that continuation position for spell/grammar checking is
    // at the end of this sentence
    if (g_pSpellIter)
    {
        g_pSpellIter->m_oCurr.emplace( *g_pSpellIter->m_oCurrX );
    }
}

void SwEditShell::ApplyChangedSentence(const svx::SpellPortions& rNewPortions, bool bRecheck)
{
    // Note: rNewPortions.size() == 0 is valid and happens when the whole
    // sentence got removed in the dialog

    OSL_ENSURE(  g_pSpellIter, "SpellIter missing" );
    if (!g_pSpellIter ||
        g_pSpellIter->GetLastPortions().empty()) // no portions -> no text to be changed
        return;

    const SpellPortions& rLastPortions = g_pSpellIter->GetLastPortions();
    const SpellContentPositions  rLastPositions = g_pSpellIter->GetLastPositions();
    OSL_ENSURE(!rLastPortions.empty() &&
            rLastPortions.size() == rLastPositions.size(),
            "last vectors of spelling results are not set or not equal");

    // iterate over the new portions, beginning at the end to take advantage of the previously
    // saved content positions

    mxDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::UI_TEXT_CORRECTION, nullptr );
    StartAction();

    SwPaM *pCursor = GetCursor();
    // save cursor position (which should be at the end of the current sentence)
    // for later restoration
    Push();

    sal_uInt32 nRedlinePortions = lcl_CountRedlines(rLastPortions);
    if((rLastPortions.size() - nRedlinePortions) == rNewPortions.size())
    {
        OSL_ENSURE( !rNewPortions.empty(), "rNewPortions should not be empty here" );
        OSL_ENSURE( !rLastPortions.empty(), "rLastPortions should not be empty here" );
        OSL_ENSURE( !rLastPositions.empty(), "rLastPositions should not be empty here" );

        // the simple case: the same number of elements on both sides
        // each changed element has to be applied to the corresponding source element
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
            if ( !pCursor->HasMark() )
                pCursor->SetMark();
            pCursor->GetPoint()->SetContent( aCurrentOldPosition->nLeft );
            pCursor->GetMark()->SetContent( aCurrentOldPosition->nRight );
            sal_uInt16 nScriptType = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( aCurrentNewPortion->eLanguage );
            sal_uInt16 nLangWhichId = RES_CHRATR_LANGUAGE;
            switch(nScriptType)
            {
                case css::i18n::ScriptType::ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                case css::i18n::ScriptType::COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
            }
            if(aCurrentNewPortion->sText != aCurrentOldPortion->sText)
            {
                // change text ...
                // ... and apply language if necessary
                if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
                    SetAttrItem( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId) );

                // if there is a comment inside the original word, don't delete it:
                // but keep it at the end of the replacement
                ReplaceKeepComments(aCurrentNewPortion->sText);
            }
            else if(aCurrentNewPortion->eLanguage != aCurrentOldPortion->eLanguage)
            {
                // apply language
                SetAttrItem( SvxLanguageItem(aCurrentNewPortion->eLanguage, nLangWhichId) );
            }
            else if( aCurrentNewPortion->bIgnoreThisError )
            {
                // add the 'ignore' markup to the TextNode's grammar ignore markup list
                IgnoreGrammarErrorAt( *pCursor );
                OSL_FAIL("TODO: add ignore mark to text node");
            }
        }
        while(aCurrentNewPortion != rNewPortions.begin());
    }
    else
    {
        OSL_ENSURE( !rLastPositions.empty(), "rLastPositions should not be empty here" );

        // select the complete sentence
        SpellContentPositions::const_iterator aCurrentEndPosition = rLastPositions.end();
        --aCurrentEndPosition;
        SpellContentPositions::const_iterator aCurrentStartPosition = rLastPositions.begin();
        pCursor->GetPoint()->SetContent( aCurrentStartPosition->nLeft );
        pCursor->GetMark()->SetContent( aCurrentEndPosition->nRight );

        // delete the sentence completely
        mxDoc->getIDocumentContentOperations().DeleteAndJoin(*pCursor);
        for(const auto& rCurrentNewPortion : rNewPortions)
        {
            // set the language attribute
            SvtScriptType nScriptType = GetScriptType();
            sal_uInt16 nLangWhichId = RES_CHRATR_LANGUAGE;
            switch(nScriptType)
            {
                case SvtScriptType::ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                case SvtScriptType::COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
                default: break;
            }
            SfxItemSet aSet(GetAttrPool(), nLangWhichId, nLangWhichId);
            GetCurAttr( aSet );
            const SvxLanguageItem& rLang = static_cast<const SvxLanguageItem& >(aSet.Get(nLangWhichId));
            if(rLang.GetLanguage() != rCurrentNewPortion.eLanguage)
                SetAttrItem( SvxLanguageItem(rCurrentNewPortion.eLanguage, nLangWhichId) );
            // insert the new string
            mxDoc->getIDocumentContentOperations().InsertString(*pCursor, rCurrentNewPortion.sText);

            // set the cursor to the end of the inserted string
            *pCursor->Start() = *pCursor->End();
        }
    }

    // restore cursor to the end of the sentence
    // (will work also if the sentence length has changed,
    // since cursors get updated automatically!)
    Pop(PopMode::DeleteCurrent);

    // collapse cursor to the end of the modified sentence
    *pCursor->Start() = *pCursor->End();
    if (bRecheck)
    {
        // in grammar check the current sentence has to be checked again
        GoStartSentence();
    }
    // set continuation position for spell/grammar checking to the end of this sentence
    g_pSpellIter->m_oCurr.emplace( *pCursor->Start() );

    mxDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::UI_TEXT_CORRECTION, nullptr );
    EndAction();

}
/** Collect all deleted redlines of the current text node
 *  beginning at the start of the cursor position
 */
static SpellContentPositions lcl_CollectDeletedRedlines(SwEditShell const * pSh)
{
    SpellContentPositions aRedlines;
    SwDoc* pDoc = pSh->GetDoc();
    const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( pDoc->getIDocumentRedlineAccess().GetRedlineFlags() );
    if ( bShowChg )
    {
        SwPaM *pCursor = pSh->GetCursor();
        const SwPosition* pStartPos = pCursor->Start();
        const SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();

        SwRedlineTable::size_type nAct = pDoc->getIDocumentRedlineAccess().GetRedlinePos( *pTextNode, RedlineType::Any );
        const sal_Int32 nStartIndex = pStartPos->GetContentIndex();
        for ( ; nAct < pDoc->getIDocumentRedlineAccess().GetRedlineTable().size(); nAct++ )
        {
            const SwRangeRedline* pRed = pDoc->getIDocumentRedlineAccess().GetRedlineTable()[ nAct ];

            if ( pRed->Start()->GetNode() > *pTextNode )
                break;

            if( RedlineType::Delete == pRed->GetType() )
            {
                sal_Int32 nStart_, nEnd_;
                pRed->CalcStartEnd( pTextNode->GetIndex(), nStart_, nEnd_ );
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

/// remove the redline positions after the current selection
static void lcl_CutRedlines( SpellContentPositions& aDeletedRedlines, SwEditShell const * pSh )
{
    if(!aDeletedRedlines.empty())
    {
        SwPaM *pCursor = pSh->GetCursor();
        const SwPosition* pEndPos = pCursor->End();
        const sal_Int32 nEnd = pEndPos->GetContentIndex();
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
        auto aIter = std::find_if_not(rDeletedRedlines.begin(), rDeletedRedlines.end(),
            [nSearchFrom](const SpellContentPosition& rPos) { return rPos.nLeft < nSearchFrom; });
        if (aIter != rDeletedRedlines.end())
            aRet = *aIter;
    }
    return aRet;
}

bool SwSpellIter::SpellSentence(svx::SpellPortions& rPortions, bool bIsGrammarCheck)
{
    bool bRet = false;
    m_aLastPortions.clear();
    m_aLastPositions.clear();

    SwEditShell *pMySh = GetSh();
    if( !pMySh )
        return false;

    OSL_ENSURE( m_oEnd, "SwSpellIter::SpellSentence without Start?");

    uno::Reference< XSpellAlternatives >  xSpellRet;
    linguistic2::ProofreadingResult aGrammarResult;
    bool bGoOn = true;
    bool bGrammarErrorFound = false;
    do {
        SwPaM *pCursor = pMySh->GetCursor();
        if ( !pCursor->HasMark() )
            pCursor->SetMark();

        *pCursor->GetPoint() = *m_oCurr;
        *pCursor->GetMark() = *m_oEnd;

        if (m_bBackToStartOfSentence)
        {
            pMySh->GoStartSentence();
            m_bBackToStartOfSentence = false;
        }
        uno::Any aSpellRet = pMySh->GetDoc()->Spell(*pCursor, m_xSpeller, nullptr, nullptr,
                                                    bIsGrammarCheck, pMySh->GetLayout());
        aSpellRet >>= xSpellRet;
        aSpellRet >>= aGrammarResult;
        bGoOn = GetCursorCnt() > 1;
        bGrammarErrorFound = aGrammarResult.aErrors.hasElements();
        if( xSpellRet.is() || bGrammarErrorFound )
        {
            bGoOn = false;

            m_oCurr.emplace( *pCursor->GetPoint() );
            m_oCurrX.emplace( *pCursor->GetMark() );
        }
        if( bGoOn )
        {
            pMySh->Pop(SwCursorShell::PopMode::DeleteCurrent);
            pCursor = pMySh->GetCursor();
            if ( *pCursor->GetPoint() > *pCursor->GetMark() )
                pCursor->Exchange();
            m_oStart.emplace( *pCursor->GetPoint() );
            m_oEnd.emplace( *pCursor->GetMark() );
            m_oCurr.emplace( *m_oStart );
            m_oCurrX.emplace( *m_oCurr );
            pCursor->SetMark();
            --GetCursorCnt();
        }
    } while ( bGoOn );

    if(xSpellRet.is() || bGrammarErrorFound)
    {
        // an error has been found
        // To fill the spell portions the beginning of the sentence has to be found
        SwPaM *pCursor = pMySh->GetCursor();
        // set the mark to the right if necessary
        if ( *pCursor->GetPoint() > *pCursor->GetMark() )
            pCursor->Exchange();
        // the cursor has to be collapsed on the left to go to the start of the sentence - if sentence ends inside of the error
        pCursor->DeleteMark();
        pCursor->SetMark();
        bool bStartSent = pMySh->GoStartSentence();
        SpellContentPositions aDeletedRedlines = lcl_CollectDeletedRedlines(pMySh);
        if(bStartSent)
        {
            // create a portion from the start part
            AddPortion(nullptr, nullptr, aDeletedRedlines);
        }
        // Set the cursor to the error already found
        *pCursor->GetPoint() = *m_oCurrX;
        *pCursor->GetMark() = *m_oCurr;
        AddPortion(xSpellRet, &aGrammarResult, aDeletedRedlines);

        // save the end position of the error to continue from here
        SwPosition aSaveStartPos = *pCursor->End();
        // determine the end of the current sentence
        if ( *pCursor->GetPoint() < *pCursor->GetMark() )
            pCursor->Exchange();
        // again collapse to start marking after the end of the error
        pCursor->DeleteMark();
        pCursor->SetMark();

        pMySh->GoEndSentence();
        if( bGrammarErrorFound )
        {
            const ModelToViewHelper aConversionMap(static_cast<SwTextNode&>(pCursor->GetPointNode()), pMySh->GetLayout());
            const OUString& aExpandText = aConversionMap.getViewText();
            sal_Int32 nSentenceEnd =
                aConversionMap.ConvertToViewPosition( aGrammarResult.nBehindEndOfSentencePosition );
            // remove trailing space
            if( aExpandText[nSentenceEnd - 1] == ' ' )
                --nSentenceEnd;
            if( pCursor->End()->GetContentIndex() < nSentenceEnd )
            {
                pCursor->End()->SetContent(nSentenceEnd);
            }
        }

        lcl_CutRedlines( aDeletedRedlines, pMySh );
        // save the 'global' end of the spellchecking
        const SwPosition aSaveEndPos = *m_oEnd;
        // set the sentence end as 'local' end
        m_oEnd.emplace( *pCursor->End() );

        *pCursor->GetPoint() = aSaveStartPos;
        *pCursor->GetMark() = *m_oEnd;
        // now the rest of the sentence has to be searched for errors
        // for each error the non-error text between the current and the last error has
        // to be added to the portions - if necessary broken into same-language-portions
        if( !bGrammarErrorFound ) //in grammar check there's only one error returned
        {
            do
            {
                xSpellRet = nullptr;
                // don't search for grammar errors here anymore!
                pMySh->GetDoc()->Spell(*pCursor, m_xSpeller, nullptr, nullptr, false,
                                       pMySh->GetLayout())
                    >>= xSpellRet;
                if ( *pCursor->GetPoint() > *pCursor->GetMark() )
                    pCursor->Exchange();
                m_oCurr.emplace( *pCursor->GetPoint() );
                m_oCurrX.emplace( *pCursor->GetMark() );

                // if an error has been found go back to the text preceding the error
                if(xSpellRet.is())
                {
                    *pCursor->GetPoint() = aSaveStartPos;
                    *pCursor->GetMark() = *m_oCurr;
                }
                // add the portion
                AddPortion(nullptr, nullptr, aDeletedRedlines);

                if(xSpellRet.is())
                {
                    *pCursor->GetPoint() = *m_oCurr;
                    *pCursor->GetMark() = *m_oCurrX;
                    AddPortion(xSpellRet, nullptr, aDeletedRedlines);
                    // move the cursor to the end of the error string
                    *pCursor->GetPoint() = *m_oCurrX;
                    // and save the end of the error as new start position
                    aSaveStartPos = *m_oCurrX;
                    // and the end of the sentence
                    *pCursor->GetMark() = *m_oEnd;
                }
                // if the end of the sentence has already been reached then break here
                if(*m_oCurrX >= *m_oEnd)
                    break;
            }
            while(xSpellRet.is());
        }
        else
        {
            // go to the end of sentence as the grammar check returned it
            // at this time the Point is behind the grammar error
            // and the mark points to the sentence end as
            if ( *pCursor->GetPoint() < *pCursor->GetMark() )
                pCursor->Exchange();
        }

        // the part between the last error and the end of the sentence has to be added
        *pMySh->GetCursor()->GetPoint() = *m_oEnd;
        if(*m_oCurrX < *m_oEnd)
        {
            AddPortion(nullptr, nullptr, aDeletedRedlines);
        }
        // set the shell cursor to the end of the sentence to prevent a visible selection
        *pCursor->GetMark() = *m_oEnd;
        if( !bIsGrammarCheck )
        {
            // set the current position to the end of the sentence
            m_oCurr.emplace( *m_oEnd );
        }
        // restore the 'global' end
        m_oEnd.emplace( aSaveEndPos );
        rPortions = m_aLastPortions;
        bRet = true;
    }
    else
    {
        // if no error could be found the selection has to be corrected - at least if it's not in the body
        *pMySh->GetCursor()->GetPoint() = *m_oEnd;
        pMySh->GetCursor()->DeleteMark();
    }

    return bRet;
}

void SwSpellIter::ToSentenceStart() { m_bBackToStartOfSentence = true; }

static LanguageType lcl_GetLanguage(SwEditShell& rSh)
{
    SvtScriptType nScriptType = rSh.GetScriptType();
    TypedWhichId<SvxLanguageItem> nLangWhichId = RES_CHRATR_LANGUAGE;

    switch(nScriptType)
    {
        case SvtScriptType::ASIAN : nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
        case SvtScriptType::COMPLEX : nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
        default: break;
    }
    SfxItemSet aSet(rSh.GetAttrPool(), nLangWhichId, nLangWhichId);
    rSh.GetCurAttr( aSet );
    const SvxLanguageItem& rLang = aSet.Get(nLangWhichId);
    return rLang.GetLanguage();
}

/// create a text portion at the given position
void SwSpellIter::CreatePortion(uno::Reference< XSpellAlternatives > const & xAlt,
                        const linguistic2::ProofreadingResult* pGrammarResult,
        bool bIsField, bool bIsHidden)
{
    svx::SpellPortion aPortion;
    OUString sText;
    GetSh()->GetSelectedText( sText );
    if(sText.isEmpty())
        return;

    // in case of redlined deletions the selection of an error is not the same as the _real_ word
    if(xAlt.is())
        aPortion.sText = xAlt->getWord();
    else if(pGrammarResult)
    {
        aPortion.bIsGrammarError = true;
        if(pGrammarResult->aErrors.hasElements())
        {
            aPortion.aGrammarError = pGrammarResult->aErrors[0];
            aPortion.sText = pGrammarResult->aText.copy( aPortion.aGrammarError.nErrorStart, aPortion.aGrammarError.nErrorLength );
            aPortion.xGrammarChecker = pGrammarResult->xProofreader;
            auto pProperty = std::find_if(std::cbegin(pGrammarResult->aProperties), std::cend(pGrammarResult->aProperties),
                [](const beans::PropertyValue& rProperty) { return rProperty.Name == "DialogTitle"; });
            if (pProperty != std::cend(pGrammarResult->aProperties))
                pProperty->Value >>= aPortion.sDialogTitle;
        }
    }
    else
        aPortion.sText = sText;
    aPortion.eLanguage = lcl_GetLanguage(*GetSh());
    aPortion.bIsField = bIsField;
    aPortion.bIsHidden = bIsHidden;
    aPortion.xAlternatives = xAlt;
    SpellContentPosition aPosition;
    SwPaM *pCursor = GetSh()->GetCursor();
    aPosition.nLeft = pCursor->Start()->GetContentIndex();
    aPosition.nRight = pCursor->End()->GetContentIndex();
    m_aLastPortions.push_back(aPortion);
    m_aLastPositions.push_back(aPosition);
}

void    SwSpellIter::AddPortion(uno::Reference< XSpellAlternatives > const & xAlt,
                                const linguistic2::ProofreadingResult* pGrammarResult,
                                const SpellContentPositions& rDeletedRedlines)
{
    SwEditShell *pMySh = GetSh();
    OUString sText;
    pMySh->GetSelectedText( sText );
    if(sText.isEmpty())
        return;

    if(xAlt.is() || pGrammarResult != nullptr)
    {
        CreatePortion(xAlt, pGrammarResult, false, false);
    }
    else
    {
        SwPaM *pCursor = GetSh()->GetCursor();
        if ( *pCursor->GetPoint() > *pCursor->GetMark() )
            pCursor->Exchange();
        // save the start and end positions
        SwPosition aStart(*pCursor->GetPoint());
        SwPosition aEnd(*pCursor->GetMark());
        // iterate over the text to find changes in language
        // set the mark equal to the point
        *pCursor->GetMark() = aStart;
        SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
        LanguageType eStartLanguage = lcl_GetLanguage(*GetSh());
        SpellContentPosition  aNextRedline = lcl_FindNextDeletedRedline(
                    rDeletedRedlines, aStart.GetContentIndex() );
        if( aNextRedline.nLeft == aStart.GetContentIndex() )
        {
            // select until the end of the current redline
            const sal_Int32 nEnd = aEnd.GetContentIndex() < aNextRedline.nRight ?
                        aEnd.GetContentIndex() : aNextRedline.nRight;
            pCursor->GetPoint()->SetContent( nEnd );
            CreatePortion(xAlt, pGrammarResult, false, true);
            aStart = *pCursor->End();
            // search for next redline
            aNextRedline = lcl_FindNextDeletedRedline(
                        rDeletedRedlines, aStart.GetContentIndex() );
        }
        while(*pCursor->GetPoint() < aEnd)
        {
            // #125786 in table cell with fixed row height the cursor might not move forward
            if(!GetSh()->Right(1, SwCursorSkipMode::Cells))
                break;

            bool bField = false;
            // read the character at the current position to check if it's a field
            sal_Unicode const cChar =
                pTextNode->GetText()[pCursor->GetMark()->GetContentIndex()];
            if( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar)
            {
                const SwTextAttr* pTextAttr = pTextNode->GetTextAttrForCharAt(
                    pCursor->GetMark()->GetContentIndex() );
                const sal_uInt16 nWhich = pTextAttr
                    ? pTextAttr->Which()
                    : RES_TXTATR_END;
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
            else if (cChar == CH_TXT_ATR_FORMELEMENT)
            {
                SwPosition aPos(*pCursor->GetMark());
                bField = pMySh->GetDoc()->getIDocumentMarkAccess()->getDropDownFor(aPos);
            }

            LanguageType eCurLanguage = lcl_GetLanguage(*GetSh());
            bool bRedline = aNextRedline.nLeft == pCursor->GetPoint()->GetContentIndex();
            // create a portion if the next character
            //  - is a field,
            //  - is at the beginning of a deleted redline
            //  - has a different language
            if(bField || bRedline || eCurLanguage != eStartLanguage)
            {
                eStartLanguage = eCurLanguage;
                // go one step back - the cursor currently selects the first character
                // with a different language
                // in the case of redlining it's different
                if(eCurLanguage != eStartLanguage || bField)
                    *pCursor->GetPoint() = *pCursor->GetMark();
                // set to the last start
                *pCursor->GetMark() = aStart;
                // create portion should only be called if a selection exists
                // there's no selection if there's a field at the beginning
                if(*pCursor->Start() != *pCursor->End())
                    CreatePortion(xAlt, pGrammarResult, false, false);
                aStart = *pCursor->End();
                // now export the field - if there is any
                if(bField)
                {
                    *pCursor->GetMark() = *pCursor->GetPoint();
                    GetSh()->Right(1, SwCursorSkipMode::Cells);
                    CreatePortion(xAlt, pGrammarResult, true, false);
                    aStart = *pCursor->End();
                }
            }
            // if a redline start then create a portion for it
            if(bRedline)
            {
                *pCursor->GetMark() = *pCursor->GetPoint();
                // select until the end of the current redline
                const sal_Int32 nEnd = aEnd.GetContentIndex() < aNextRedline.nRight ?
                            aEnd.GetContentIndex() : aNextRedline.nRight;
                pCursor->GetPoint()->SetContent( nEnd );
                CreatePortion(xAlt, pGrammarResult, false, true);
                aStart = *pCursor->End();
                // search for next redline
                aNextRedline = lcl_FindNextDeletedRedline(
                            rDeletedRedlines, aStart.GetContentIndex() );
            }
            *pCursor->GetMark() = *pCursor->GetPoint();
        }
        pCursor->SetMark();
        *pCursor->GetMark() = aStart;
        CreatePortion(xAlt, pGrammarResult, false, false);
    }
}

void SwEditShell::IgnoreGrammarErrorAt( SwPaM& rErrorPosition )
{
    SwTextNode *pNode;
    SwWrongList *pWrong;
    SwNodeIndex aIdx(rErrorPosition.Start()->GetNode());
    SwNodeIndex aEndIdx(rErrorPosition.Start()->GetNode());
    sal_Int32 nStart = rErrorPosition.Start()->GetContentIndex();
    sal_Int32 nEnd = COMPLETE_STRING;
    while( aIdx <= aEndIdx )
    {
        pNode = aIdx.GetNode().GetTextNode();
        if( pNode ) {
            if( aIdx == aEndIdx )
                nEnd = rErrorPosition.End()->GetContentIndex();
            pWrong = pNode->GetGrammarCheck();
            if( pWrong )
                pWrong->RemoveEntry( nStart, nEnd );
            pWrong = pNode->GetWrong();
            if( pWrong )
                pWrong->RemoveEntry( nStart, nEnd );
            SwTextFrame::repaintTextFrames( *pNode );
        }
        ++aIdx;
        nStart = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
