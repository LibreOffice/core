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

#include <memory>

#include <com/sun/star/util/SearchFlags.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <editeng/outliner.hxx>

#include <wrtsh.hxx>
#include <txatritr.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <txtatr.hxx>
#include <txtfld.hxx>
#include <swcrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <dcontact.hxx>
#include <pamtyp.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoInsert.hxx>
#include <breakit.hxx>
#include <docsh.hxx>
#include <PostItMgr.hxx>
#include <viewsh.hxx>
#include <view.hxx>

using namespace ::com::sun::star;
using namespace util;

static OUString
lcl_CleanStr(const SwTextNode& rNd, sal_Int32 const nStart, sal_Int32& rEnd,
             std::vector<sal_Int32> &rArr, bool const bRemoveSoftHyphen, bool const bRemoveCommentAnchors)
{
    OUStringBuffer buf(rNd.GetText());
    rArr.clear();

    const SwpHints *pHts = rNd.GetpSwpHints();

    size_t n = 0;
    sal_Int32 nSoftHyphen = nStart;
    sal_Int32 nHintStart = -1;
    bool bNewHint       = true;
    bool bNewSoftHyphen = true;
    const sal_Int32 nEnd = rEnd;
    std::vector<sal_Int32> aReplaced;

    do
    {
        if ( bNewHint )
            nHintStart = pHts && n < pHts->Count() ?
                         pHts->Get(n)->GetStart() :
                         -1;

        if ( bNewSoftHyphen )
        {
            nSoftHyphen = bRemoveSoftHyphen
                    ?  rNd.GetText().indexOf(CHAR_SOFTHYPHEN, nSoftHyphen)
                    : -1;
        }

        bNewHint       = false;
        bNewSoftHyphen = false;
        sal_Int32 nStt = 0;

        // Check if next stop is a hint.
        if ( nHintStart>=0
            && (-1 == nSoftHyphen || nHintStart < nSoftHyphen)
            && nHintStart < nEnd )
        {
            nStt = nHintStart;
            bNewHint = true;
        }
        // Check if next stop is a soft hyphen.
        else if (   -1 != nSoftHyphen
                 && (-1 == nHintStart || nSoftHyphen < nHintStart)
                 && nSoftHyphen < nEnd)
        {
            nStt = nSoftHyphen;
            bNewSoftHyphen = true;
        }
        // If nSoftHyphen == nHintStart, the current hint *must* be a hint with an end.
        else if (-1 != nSoftHyphen && nSoftHyphen == nHintStart)
        {
            nStt = nSoftHyphen;
            bNewHint = true;
            bNewSoftHyphen = true;
        }
        else
            break;

        const sal_Int32 nCurrent = nStt - rArr.size();

        if ( bNewHint )
        {
            const SwTextAttr* pHt = pHts->Get(n);
            if ( pHt->HasDummyChar() && (nStt >= nStart) )
            {
                switch( pHt->Which() )
                {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                case RES_TXTATR_FIELD:
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_META:
                case RES_TXTATR_METAFIELD:
                    {
                        // (1998) they are desired as separators and
                        // belong not any longer to a word.
                        // they should also be ignored at a
                        // beginning/end of a sentence if blank. Those are
                        // simply removed if first. If at the end, we keep the
                        // replacement and remove afterwards all at a string's
                        // end (might be normal 0x7f).
                        const bool bEmpty = pHt->Which() != RES_TXTATR_FIELD
                            || (static_txtattr_cast<SwTextField const*>(pHt)->GetFormatField().GetField()->ExpandField(true, nullptr).isEmpty());
                        if ( bEmpty && nStart == nCurrent )
                        {
                            rArr.push_back( nCurrent );
                            --rEnd;
                            buf.remove(nCurrent, 1);
                        }
                        else
                        {
                            if ( bEmpty )
                                aReplaced.push_back( nCurrent );
                            buf[nCurrent] = '\x7f';
                        }
                    }
                    break;
                case RES_TXTATR_ANNOTATION:
                    {
                        if( bRemoveCommentAnchors )
                        {
                            rArr.push_back( nCurrent );
                            --rEnd;
                            buf.remove( nCurrent, 1 );
                        }
                    }
                    break;
                default:
                    OSL_FAIL( "unknown case in lcl_CleanStr" );
                    break;
                }
            }
            ++n;
        }

        if ( bNewSoftHyphen )
        {
            rArr.push_back( nCurrent );
            --rEnd;
            buf.remove(nCurrent, 1);
            ++nSoftHyphen;
        }
    }
    while ( true );

    for( std::vector<sal_Int32>::size_type i = aReplaced.size(); i; )
    {
        const sal_Int32 nTmp = aReplaced[ --i ];
        if (nTmp == buf.getLength() - 1)
        {
            buf.truncate(nTmp);
            rArr.push_back( nTmp );
            --rEnd;
        }
    }

    return buf.makeStringAndClear();
}

// skip all non SwPostIts inside the array
size_t GetPostIt(sal_Int32 aCount,const SwpHints *pHts)
{
    size_t aIndex = 0;
    while (aCount)
    {
        for (size_t i = 0; i < pHts->Count(); ++i )
        {
            aIndex++;
            const SwTextAttr* pTextAttr = pHts->Get(i);
            if ( pTextAttr->Which() == RES_TXTATR_ANNOTATION )
            {
                aCount--;
                if (!aCount)
                    break;
            }
        }
    }
    // throw away all following non postits
    for( size_t i = aIndex; i < pHts->Count(); ++i )
    {
        const SwTextAttr* pTextAttr = pHts->Get(i);
        if ( pTextAttr->Which() == RES_TXTATR_ANNOTATION )
            break;
        else
            aIndex++;
    }
    return aIndex;
}

bool SwPaM::Find( const i18nutil::SearchOptions2& rSearchOpt, bool bSearchInNotes , utl::TextSearch& rSText,
                  SwMoveFnCollection const & fnMove, const SwPaM * pRegion,
                  bool bInReadOnly )
{
    if( rSearchOpt.searchString.isEmpty() )
        return false;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    const bool bSrchForward = &fnMove == &fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rContentIdx = pPam->GetPoint()->nContent;

    // If bFound is true then the string was found and is between nStart and nEnd
    bool bFound = false;
    // start position in text or initial position
    bool bFirst = true;
    SwContentNode * pNode;

    const bool bRegSearch = SearchAlgorithms2::REGEXP == rSearchOpt.AlgorithmType2;
    const bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( rSearchOpt.searchString == "^$" ||
                          rSearchOpt.searchString == "$^" );
    const bool bChkParaEnd = bRegSearch && rSearchOpt.searchString == "$";

    SvxSearchItem aSearchItem(SID_SEARCH_ITEM); // this is a very expensive operation (calling configmgr etc.)
    aSearchItem.SetSearchOptions(rSearchOpt);
    aSearchItem.SetBackward(!bSrchForward);

    // LanguageType eLastLang = 0;
    while( nullptr != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTextNode() )
        {
            SwTextNode& rTextNode = *pNode->GetTextNode();
            sal_Int32 nTextLen = rTextNode.GetText().getLength();
            sal_Int32 nEnd;
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnd = pPam->GetMark()->nContent.GetIndex();
            else
                nEnd = bSrchForward ? nTextLen : 0;
            sal_Int32 nStart = rContentIdx.GetIndex();

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we
            // split the text into separate pieces and search for text inside
            // the pieces as well as inside the fields
            const SwpHints *pHts = rTextNode.GetpSwpHints();

            // count PostItFields by looping over all fields
            sal_Int32 aNumberPostits = 0;
            sal_Int32 aIgnore = 0;
            if (pHts && bSearchInNotes)
            {
                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

                for( size_t i = 0; i < pHts->Count(); ++i )
                {
                    const SwTextAttr* pTextAttr = pHts->Get(i);
                    if ( pTextAttr->Which()==RES_TXTATR_ANNOTATION )
                    {
                        const sal_Int32 aPos = pTextAttr->GetStart();
                        if ( (aPos >= nStart) && (aPos <= nEnd) )
                            aNumberPostits++;
                        else
                        {
                            if (bSrchForward)
                                aIgnore++;
                        }
                    }
                }

                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

            }

            SwDocShell *const pDocShell = pNode->GetDoc()->GetDocShell();
            SwWrtShell *const pWrtShell = pDocShell ? pDocShell->GetWrtShell() : nullptr;
            SwPostItMgr *const pPostItMgr = pWrtShell ? pWrtShell->GetPostItMgr() : nullptr;

            // If there is an active text edit, then search there.
            bool bEndedTextEdit = false;
            SdrView* pSdrView = pWrtShell ? pWrtShell->GetDrawView() : nullptr;
            if (pSdrView)
            {
                // If the edited object is not anchored to this node, then ignore it.
                SdrObject* pObject = pSdrView->GetTextEditObject();
                if (pObject)
                {
                    if (SwFrameFormat* pFrameFormat = FindFrameFormat(pObject))
                    {
                        const SwPosition* pPosition = pFrameFormat->GetAnchor().GetContentAnchor();
                        if (!pPosition || pPosition->nNode.GetIndex() != pNode->GetIndex())
                            pObject = nullptr;
                    }
                }

                if (pObject)
                {
                    sal_uInt16 nResult = pSdrView->GetTextEditOutlinerView()->StartSearchAndReplace(aSearchItem);
                    if (!nResult)
                    {
                        // If not found, end the text edit.
                        pSdrView->SdrEndTextEdit();
                        const Point aPoint(pSdrView->GetAllMarkedRect().TopLeft());
                        pSdrView->UnmarkAll();
                        pWrtShell->CallSetCursor(&aPoint, true);
                        pWrtShell->Edit();
                        bEndedTextEdit = true;
                    }
                    else
                    {
                        bFound = true;
                        break;
                    }
                }
            }

            if (comphelper::LibreOfficeKit::isActive())
            {
                // Writer and editeng selections are not supported in parallel.
                SvxSearchItem* pSearchItem = SwView::GetSearchItem();
                // If we just finished search in shape text, don't attempt to do that again.
                if (!bEndedTextEdit && !(pSearchItem && pSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL))
                {
                    // If there are any shapes anchored to this node, search there.
                    SwPaM aPaM(pNode->GetDoc()->GetNodes().GetEndOfContent());
                    aPaM.GetPoint()->nNode = rTextNode;
                    aPaM.GetPoint()->nContent.Assign(aPaM.GetPoint()->nNode.GetNode().GetTextNode(), nStart);
                    aPaM.SetMark();
                    aPaM.GetMark()->nNode = rTextNode.GetIndex() + 1;
                    aPaM.GetMark()->nContent.Assign(aPaM.GetMark()->nNode.GetNode().GetTextNode(), 0);
                    if (pNode->GetDoc()->getIDocumentDrawModelAccess().Search(aPaM, aSearchItem) && pSdrView)
                    {
                        if (SdrObject* pObject = pSdrView->GetTextEditObject())
                        {
                            if (SwFrameFormat* pFrameFormat = FindFrameFormat(pObject))
                            {
                                const SwPosition* pPosition = pFrameFormat->GetAnchor().GetContentAnchor();
                                if (pPosition)
                                {
                                    // Set search position to the shape's anchor point.
                                    *GetPoint() = *pPosition;
                                    GetPoint()->nContent.Assign(pPosition->nNode.GetNode().GetContentNode(), 0);
                                    SetMark();
                                    bFound = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            sal_Int32 aStart = 0;
            // do we need to finish a note?
            if (pPostItMgr && pPostItMgr->HasActiveSidebarWin())
            {
                if (bSearchInNotes)
                {
                    if (bSrchForward)
                        aStart++;
                    else
                    {
                        if (aNumberPostits)
                            --aNumberPostits;
                    }
                    //search inside, finish and put focus back into the doc
                    if (pPostItMgr->FinishSearchReplace(rSearchOpt,bSrchForward))
                    {
                        bFound = true ;
                        break;
                    }
                }
                else
                {
                    pPostItMgr->SetActiveSidebarWin(nullptr);
                }
            }

            if (aNumberPostits)
            {
                // now we have to split
                sal_Int32 nStartInside = 0;
                sal_Int32 nEndInside = 0;
                sal_Int32 aLoop= bSrchForward ? aStart : aNumberPostits;

                while ( (aLoop>=0) && (aLoop<=aNumberPostits))
                {
                    if (bSrchForward)
                    {
                        nStartInside = aLoop==0 ? nStart : pHts->Get(GetPostIt(aLoop+aIgnore-1,pHts))->GetStart()+1;
                        nEndInside = aLoop==aNumberPostits ? nEnd : pHts->Get(GetPostIt(aLoop+aIgnore,pHts))->GetStart();
                        nTextLen = nEndInside - nStartInside;
                    }
                    else
                    {
                        nStartInside =  aLoop==aNumberPostits ? nStart : pHts->Get(GetPostIt(aLoop+aIgnore,pHts))->GetStart();
                        nEndInside = aLoop==0 ? nEnd : pHts->Get(GetPostIt(aLoop+aIgnore-1,pHts))->GetStart()+1;
                        nTextLen = nStartInside - nEndInside;
                    }
                    // search inside the text between a note
                    bFound = DoSearch( rSearchOpt, rSText, fnMove, bSrchForward,
                                       bRegSearch, bChkEmptyPara, bChkParaEnd,
                                       nStartInside, nEndInside, nTextLen, pNode,
                                       pPam );
                    if ( bFound )
                        break;
                    else
                    {
                        // we should now be right in front of a note, search inside
                        if ( (bSrchForward && (GetPostIt(aLoop + aIgnore,pHts) < pHts->Count()) ) || ( !bSrchForward && (aLoop!=0) ))
                        {
                            const SwTextAttr* pTextAttr = bSrchForward
                                    ? pHts->Get(GetPostIt(aLoop+aIgnore,pHts))
                                    : pHts->Get(GetPostIt(aLoop+aIgnore-1,pHts));
                            if (pPostItMgr && pPostItMgr->SearchReplace(
                                    static_txtattr_cast<SwTextField const*>(pTextAttr)->GetFormatField(),rSearchOpt,bSrchForward))
                            {
                                bFound = true ;
                                break;
                            }
                        }
                    }
                    aLoop = bSrchForward ? aLoop+1 : aLoop-1;
                }
            }
            else
            {
                // if there is no SwPostItField inside or searching inside notes
                // is disabled, we search the whole length just like before
                bFound = DoSearch( rSearchOpt, rSText, fnMove, bSrchForward,
                                   bRegSearch, bChkEmptyPara, bChkParaEnd,
                                   nStart, nEnd, nTextLen, pNode, pPam );
            }
            if (bFound)
                break;
        }
    }
    delete pPam;
    return bFound;
}

bool SwPaM::DoSearch( const i18nutil::SearchOptions2& rSearchOpt, utl::TextSearch& rSText,
                      SwMoveFnCollection const & fnMove, bool bSrchForward, bool bRegSearch,
                      bool bChkEmptyPara, bool bChkParaEnd,
                      sal_Int32 &nStart, sal_Int32 &nEnd, sal_Int32 nTextLen,
                      SwNode* pNode, SwPaM* pPam)
{
    bool bFound = false;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    const SwNode* pSttNd = &rNdIdx.GetNode();
    OUString sCleanStr;
    std::vector<sal_Int32> aFltArr;
    LanguageType eLastLang = LANGUAGE_SYSTEM;
    // if the search string contains a soft hyphen,
    // we don't strip them from the text:
    bool bRemoveSoftHyphens = true;
    // if the search string contains a comment, we don't strip them from the text
    const bool bRemoveCommentAnchors = rSearchOpt.searchString.indexOf( CH_TXTATR_INWORD ) == -1;

    if ( bRegSearch )
    {
        if (   -1 != rSearchOpt.searchString.indexOf("\\xAD")
            || -1 != rSearchOpt.searchString.indexOf("\\x{00AD}")
            || -1 != rSearchOpt.searchString.indexOf("\\u00AD")
            || -1 != rSearchOpt.searchString.indexOf("\\U000000AD")
            || -1 != rSearchOpt.searchString.indexOf("\\N{SOFT HYPHEN}"))
        {
             bRemoveSoftHyphens = false;
        }
    }
    else
    {
        if ( 1 == rSearchOpt.searchString.getLength() &&
             CHAR_SOFTHYPHEN == rSearchOpt.searchString.toChar() )
             bRemoveSoftHyphens = false;
    }

    if( bSrchForward )
        sCleanStr = lcl_CleanStr(*pNode->GetTextNode(), nStart, nEnd,
                        aFltArr, bRemoveSoftHyphens, bRemoveCommentAnchors);
    else
        sCleanStr = lcl_CleanStr(*pNode->GetTextNode(), nEnd, nStart,
                        aFltArr, bRemoveSoftHyphens, bRemoveCommentAnchors);

    SwScriptIterator* pScriptIter = nullptr;
    sal_uInt16 nSearchScript = 0;
    sal_uInt16 nCurrScript = 0;

    if (SearchAlgorithms2::APPROXIMATE == rSearchOpt.AlgorithmType2)
    {
        pScriptIter = new SwScriptIterator( sCleanStr, nStart, bSrchForward );
        nSearchScript = g_pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
    }

    const sal_Int32 nStringEnd = nEnd;
    bool bZeroMatch = false;    // zero-length match, i.e. only $ anchor as regex
    while ( ((bSrchForward && nStart < nStringEnd) ||
            (! bSrchForward && nStart > nStringEnd)) && !bZeroMatch )
    {
        // SearchAlgorithms_APPROXIMATE works on a per word base so we have to
        // provide the text searcher with the correct locale, because it uses
        // the break-iterator
        if ( pScriptIter )
        {
            nEnd = pScriptIter->GetScriptChgPos();
            nCurrScript = pScriptIter->GetCurrScript();
            if ( nSearchScript == nCurrScript )
            {
                const LanguageType eCurrLang =
                        pNode->GetTextNode()->GetLang( bSrchForward ?
                                                      nStart :
                                                      nEnd );

                if ( eCurrLang != eLastLang )
                {
                    const lang::Locale aLocale(
                            g_pBreakIt->GetLocale( eCurrLang ) );
                    rSText.SetLocale( utl::TextSearch::UpgradeToSearchOptions2( rSearchOpt), aLocale );
                    eLastLang = eCurrLang;
                }
            }
            pScriptIter->Next();
        }
        sal_Int32 nProxyStart = nStart;
        sal_Int32 nProxyEnd = nEnd;
        if( nSearchScript == nCurrScript &&
                (rSText.*fnMove.fnSearch)( sCleanStr, &nProxyStart, &nProxyEnd, nullptr ) &&
                !(bZeroMatch = (nProxyStart == nProxyEnd)))
        {
            nStart = nProxyStart;
            nEnd = nProxyEnd;
            // set section correctly
            *GetPoint() = *pPam->GetPoint();
            SetMark();

            // adjust start and end
            if( !aFltArr.empty() )
            {
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { std::swap(nStart, nEnd); }

                sal_Int32 nNew = nStart;
                for (size_t n = 0; n < aFltArr.size() && aFltArr[ n ] <= nStart; ++n )
                {
                    ++nNew;
                }

                nStart = nNew;
                nNew = nEnd;
                for( size_t n = 0; n < aFltArr.size() && aFltArr[ n ] < nEnd; ++n )
                {
                    ++nNew;
                }

                nEnd = nNew;
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { std::swap(nStart, nEnd); }
            }
            GetMark()->nContent = nStart;
            GetPoint()->nContent = nEnd;

            // if backward search, switch point and mark
            if( !bSrchForward )
                Exchange();
            bFound = true;
            break;
        }
        else
        {
            nEnd = nProxyEnd;
        }
        nStart = nEnd;
    }

    delete pScriptIter;

    if ( bFound )
        return true;
    else if( ( bChkEmptyPara && !nStart && !nTextLen ) || bChkParaEnd)
    {
        *GetPoint() = *pPam->GetPoint();
        GetPoint()->nContent = bChkParaEnd ? nTextLen : 0;
        SetMark();
        /* FIXME: this condition does not work for !bSrchForward backward
         * search, it probably never did. (pSttNd != &rNdIdx.GetNode())
         * is never true in this case. */
        if( (bSrchForward || pSttNd != &rNdIdx.GetNode()) &&
            Move( fnMoveForward, GoInContent ) &&
            (!bSrchForward || pSttNd != &GetPoint()->nNode.GetNode()) &&
            1 == std::abs( static_cast<int>( GetPoint()->nNode.GetIndex() -
                             GetMark()->nNode.GetIndex()) ) )
        {
            // if backward search, switch point and mark
            if( !bSrchForward )
                Exchange();
            return true;
        }
    }
    return bFound;
}

/// parameters for search and replace in text
struct SwFindParaText : public SwFindParas
{
    const i18nutil::SearchOptions2& m_rSearchOpt;
    SwCursor& m_rCursor;
    utl::TextSearch m_aSText;
    bool m_bReplace;
    bool m_bSearchInNotes;

    SwFindParaText( const i18nutil::SearchOptions2& rOpt, bool bSearchInNotes, bool bRepl, SwCursor& rCursor )
        : m_rSearchOpt( rOpt ), m_rCursor( rCursor ), m_aSText( utl::TextSearch::UpgradeToSearchOptions2( rOpt) ),
        m_bReplace( bRepl ), m_bSearchInNotes( bSearchInNotes )
    {}
    virtual int Find( SwPaM* , SwMoveFnCollection const & , const SwPaM*, bool bInReadOnly ) override;
    virtual bool IsReplaceMode() const override;
    virtual ~SwFindParaText();
};

SwFindParaText::~SwFindParaText()
{
}

int SwFindParaText::Find( SwPaM* pCursor, SwMoveFnCollection const & fnMove,
                          const SwPaM* pRegion, bool bInReadOnly )
{
    if( bInReadOnly && m_bReplace )
        bInReadOnly = false;

    const bool bFnd = pCursor->Find( m_rSearchOpt, m_bSearchInNotes, m_aSText, fnMove, pRegion, bInReadOnly );

    if( bFnd && m_bReplace ) // replace string
    {
        // use replace method in SwDoc
        const bool bRegExp(SearchAlgorithms2::REGEXP == m_rSearchOpt.AlgorithmType2);
        SwIndex& rSttCntIdx = pCursor->Start()->nContent;
        const sal_Int32 nSttCnt = rSttCntIdx.GetIndex();
        // add to shell-cursor-ring so that the regions will be moved eventually
        SwPaM* pPrev(nullptr);
        if( bRegExp )
        {
            pPrev = const_cast<SwPaM*>(pRegion)->GetPrev();
            const_cast<SwPaM*>(pRegion)->GetRingContainer().merge( m_rCursor.GetRingContainer() );
        }

        std::unique_ptr<OUString> pRepl( bRegExp
                ? ReplaceBackReferences( m_rSearchOpt, pCursor ) : nullptr );
        bool const bReplaced =
            m_rCursor.GetDoc()->getIDocumentContentOperations().ReplaceRange(
                *pCursor,
                (pRepl.get()) ? *pRepl : m_rSearchOpt.replaceString,
                bRegExp );
        m_rCursor.SaveTableBoxContent( pCursor->GetPoint() );

        if( bRegExp )
        {
            // and remove region again
            SwPaM* p;
            SwPaM* pNext(const_cast<SwPaM*>(pRegion));
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( const_cast<SwPaM*>(pRegion) );
            } while( p != pPrev );
        }
        if (bRegExp && !bReplaced)
        {   // fdo#80715 avoid infinite loop if join failed
            bool bRet = ((&fnMoveForward == &fnMove) ? &GoNextPara : &GoPrevPara)
                (*pCursor, fnMove);
            (void) bRet;
            assert(bRet); // if join failed, next node must be SwTextNode
        }
        else
            pCursor->Start()->nContent = nSttCnt;
        return FIND_NO_RING;
    }
    return bFnd ? FIND_FOUND : FIND_NOT_FOUND;
}

bool SwFindParaText::IsReplaceMode() const
{
    return m_bReplace;
}

sal_uLong SwCursor::Find( const i18nutil::SearchOptions2& rSearchOpt, bool bSearchInNotes,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          bool& bCancel, FindRanges eFndRngs, bool bReplace )
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link<bool,void> aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link<bool,void>() );

    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::REPLACE, nullptr );
    }

    bool bSearchSel = 0 != (rSearchOpt.searchFlag & SearchFlags::REG_NOT_BEGINOFLINE);
    if( bSearchSel )
        eFndRngs = static_cast<FindRanges>(eFndRngs | FindRanges::InSel);
    SwFindParaText aSwFindParaText( rSearchOpt, bSearchInNotes, bReplace, *this );
    sal_uLong nRet = FindAll( aSwFindParaText, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        SwRewriter rewriter(MakeUndoReplaceRewriter(
                nRet, rSearchOpt.searchString, rSearchOpt.replaceString));
        pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::REPLACE, & rewriter );
    }
    return nRet;
}

OUString *ReplaceBackReferences( const i18nutil::SearchOptions2& rSearchOpt, SwPaM* pPam )
{
    OUString *pRet = nullptr;
    if( pPam && pPam->HasMark() &&
        SearchAlgorithms2::REGEXP == rSearchOpt.AlgorithmType2 )
    {
        const SwContentNode* pTextNode = pPam->GetContentNode();
        const bool bParaEnd = rSearchOpt.searchString == "$" || rSearchOpt.searchString == "^$" || rSearchOpt.searchString == "$^";
        if ( pTextNode && pTextNode->IsTextNode() && (bParaEnd || pTextNode == pPam->GetContentNode( false )) )
        {
            utl::TextSearch aSText( utl::TextSearch::UpgradeToSearchOptions2( rSearchOpt) );
            OUString rStr = pTextNode->GetTextNode()->GetText();
            sal_Int32 nStart = pPam->Start()->nContent.GetIndex();
            sal_Int32 nEnd = pPam->End()->nContent.GetIndex();
            SearchResult aResult;
            if ( bParaEnd || aSText.SearchForward( rStr, &nStart, &nEnd, &aResult ) )
            {
                if ( bParaEnd )
                {
                    rStr = "\\n";
                    aResult.subRegExpressions = 1;
                    aResult.startOffset.realloc(1);
                    aResult.endOffset.realloc(1);
                    aResult.startOffset[0] = 0;
                    aResult.endOffset[0] = rStr.getLength();
                }
                OUString aReplaceStr( rSearchOpt.replaceString );
                aSText.ReplaceBackReferences( aReplaceStr, rStr, aResult );
                pRet = new OUString( aReplaceStr );
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
