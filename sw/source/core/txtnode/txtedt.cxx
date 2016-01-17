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

#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemiter.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/hangulhanja.hxx>
#include <SwSmartTagMgr.hxx>
#include <linguistic/lngprops.hxx>
#include <officecfg/Office/Writer.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/charclass.hxx>
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <splargs.hxx>
#include <viewopt.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <txatbase.hxx>
#include <charatr.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <SwGrammarMarkUp.hxx>
#include <rootfrm.hxx>

#include <breakit.hxx>
#include <crstate.hxx>
#include <UndoOverwrite.hxx>
#include <txatritr.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <scriptinfo.hxx>
#include <docstat.hxx>
#include <editsh.hxx>
#include <unotextmarkup.hxx>
#include <txtatr.hxx>
#include <fmtautofmt.hxx>
#include <istyleaccess.hxx>
#include <unicode/uchar.h>
#include <DocumentSettingManager.hxx>

#include <unomid.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include <vector>
#include <utility>

#include <unotextrange.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;

// Wir ersparen uns in Hyphenate ein GetFrame()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTextNode *pLinguNode;
extern       SwTextFrame  *pLinguFrame;

/*
 * This has basically the same function as SwScriptInfo::MaskHiddenRanges,
 * only for deleted redlines
 */

static sal_Int32
lcl_MaskRedlines( const SwTextNode& rNode, OUStringBuffer& rText,
                         sal_Int32 nStt, sal_Int32 nEnd,
                         const sal_Unicode cChar )
{
    sal_Int32 nNumOfMaskedRedlines = 0;

    const SwDoc& rDoc = *rNode.GetDoc();

    for ( size_t nAct = rDoc.getIDocumentRedlineAccess().GetRedlinePos( rNode, USHRT_MAX ); nAct < rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(); ++nAct )
    {
        const SwRangeRedline* pRed = rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ];

        if ( pRed->Start()->nNode > rNode.GetIndex() )
            break;

        if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
        {
            sal_Int32 nRedlineEnd;
            sal_Int32 nRedlineStart;

            pRed->CalcStartEnd( rNode.GetIndex(), nRedlineStart, nRedlineEnd );

            if ( nRedlineEnd < nStt || nRedlineStart > nEnd )
                continue;

            while ( nRedlineStart < nRedlineEnd && nRedlineStart < nEnd )
            {
                if ( nRedlineStart >= nStt && nRedlineStart < nEnd )
                {
                    rText[nRedlineStart] = cChar;
                    ++nNumOfMaskedRedlines;
                }
                ++nRedlineStart;
            }
        }
    }

    return nNumOfMaskedRedlines;
}

/**
 * Used for spell checking. Deleted redlines and hidden characters are masked
 */
static bool
lcl_MaskRedlinesAndHiddenText( const SwTextNode& rNode, OUStringBuffer& rText,
                                      sal_Int32 nStt, sal_Int32 nEnd,
                                      const sal_Unicode cChar = CH_TXTATR_INWORD,
                                      bool bCheckShowHiddenChar = true )
{
    sal_Int32 nRedlinesMasked = 0;
    sal_Int32 nHiddenCharsMasked = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( rDoc.getIDocumentRedlineAccess().GetRedlineMode() );

    // If called from word count or from spell checking, deleted redlines
    // should be masked:
    if ( bShowChg )
    {
        nRedlinesMasked = lcl_MaskRedlines( rNode, rText, nStt, nEnd, cChar );
    }

    const bool bHideHidden = !SW_MOD()->GetViewOption(rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE))->IsShowHiddenChar();

    // If called from word count, we want to mask the hidden ranges even
    // if they are visible:
    if ( !bCheckShowHiddenChar || bHideHidden )
    {
        nHiddenCharsMasked =
            SwScriptInfo::MaskHiddenRanges( rNode, rText, nStt, nEnd, cChar );
    }

    return (nRedlinesMasked > 0) || (nHiddenCharsMasked > 0);
}

/**
 * Used for spell checking. Calculates a rectangle for repaint.
 */
static SwRect lcl_CalculateRepaintRect( SwTextFrame& rTextFrame, sal_Int32 nChgStart, sal_Int32 nChgEnd )
{
    SwRect aRect;

    SwTextNode *pNode = rTextFrame.GetTextNode();

    SwNodeIndex aNdIdx( *pNode );
    SwPosition aPos( aNdIdx, SwIndex( pNode, nChgEnd ) );
    SwCursorMoveState aTmpState( MV_NONE );
    aTmpState.m_b2Lines = true;
    rTextFrame.GetCharRect( aRect, aPos, &aTmpState );
    // information about end of repaint area
    Sw2LinesPos* pEnd2Pos = aTmpState.m_p2Lines;

    const SwTextFrame *pEndFrame = &rTextFrame;

    while( pEndFrame->HasFollow() &&
           nChgEnd >= pEndFrame->GetFollow()->GetOfst() )
        pEndFrame = pEndFrame->GetFollow();

    if ( pEnd2Pos )
    {
        // we are inside a special portion, take left border
        SWRECTFN( pEndFrame )
        (aRect.*fnRect->fnSetTop)( (pEnd2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pEndFrame->IsRightToLeft() )
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetLeft)() );
        else
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetRight)() );
        (aRect.*fnRect->fnSetWidth)( 1 );
        (aRect.*fnRect->fnSetHeight)( (pEnd2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pEnd2Pos;
    }

    aTmpState.m_p2Lines = nullptr;
    SwRect aTmp;
    aPos = SwPosition( aNdIdx, SwIndex( pNode, nChgStart ) );
    rTextFrame.GetCharRect( aTmp, aPos, &aTmpState );

    // i63141: GetCharRect(..) could cause a formatting,
    // during the formatting SwTextFrames could be joined, deleted, created...
    // => we have to reinit pStartFrame and pEndFrame after the formatting
    const SwTextFrame* pStartFrame = &rTextFrame;
    while( pStartFrame->HasFollow() &&
           nChgStart >= pStartFrame->GetFollow()->GetOfst() )
        pStartFrame = pStartFrame->GetFollow();
    pEndFrame = pStartFrame;
    while( pEndFrame->HasFollow() &&
           nChgEnd >= pEndFrame->GetFollow()->GetOfst() )
        pEndFrame = pEndFrame->GetFollow();

    // information about start of repaint area
    Sw2LinesPos* pSt2Pos = aTmpState.m_p2Lines;
    if ( pSt2Pos )
    {
        // we are inside a special portion, take right border
        SWRECTFN( pStartFrame )
        (aTmp.*fnRect->fnSetTop)( (pSt2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pStartFrame->IsRightToLeft() )
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
        else
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
        (aTmp.*fnRect->fnSetWidth)( 1 );
        (aTmp.*fnRect->fnSetHeight)( (pSt2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pSt2Pos;
    }

    bool bSameFrame = true;

    if( rTextFrame.HasFollow() )
    {
        if( pEndFrame != pStartFrame )
        {
            bSameFrame = false;
            SwRect aStFrame( pStartFrame->PaintArea() );
            {
                SWRECTFN( pStartFrame )
                (aTmp.*fnRect->fnSetLeft)( (aStFrame.*fnRect->fnGetLeft)() );
                (aTmp.*fnRect->fnSetRight)( (aStFrame.*fnRect->fnGetRight)() );
                (aTmp.*fnRect->fnSetBottom)( (aStFrame.*fnRect->fnGetBottom)() );
            }
            aStFrame = pEndFrame->PaintArea();
            {
                SWRECTFN( pEndFrame )
                (aRect.*fnRect->fnSetTop)( (aStFrame.*fnRect->fnGetTop)() );
                (aRect.*fnRect->fnSetLeft)( (aStFrame.*fnRect->fnGetLeft)() );
                (aRect.*fnRect->fnSetRight)( (aStFrame.*fnRect->fnGetRight)() );
            }
            aRect.Union( aTmp );
            while( true )
            {
                pStartFrame = pStartFrame->GetFollow();
                if( pStartFrame == pEndFrame )
                    break;
                aRect.Union( pStartFrame->PaintArea() );
            }
        }
    }
    if( bSameFrame )
    {
        SWRECTFN( pStartFrame )
        if( (aTmp.*fnRect->fnGetTop)() == (aRect.*fnRect->fnGetTop)() )
            (aRect.*fnRect->fnSetLeft)( (aTmp.*fnRect->fnGetLeft)() );
        else
        {
            SwRect aStFrame( pStartFrame->PaintArea() );
            (aRect.*fnRect->fnSetLeft)( (aStFrame.*fnRect->fnGetLeft)() );
            (aRect.*fnRect->fnSetRight)( (aStFrame.*fnRect->fnGetRight)() );
            (aRect.*fnRect->fnSetTop)( (aTmp.*fnRect->fnGetTop)() );
        }

        if( aTmp.Height() > aRect.Height() )
            aRect.Height( aTmp.Height() );
    }

    return aRect;
}

/**
 * Used for automatic styles. Used during RstAttr.
 */
static bool lcl_HaveCommonAttributes( IStyleAccess& rStyleAccess,
                                      const SfxItemSet* pSet1,
                                      sal_uInt16 nWhichId,
                                      const SfxItemSet& rSet2,
                                      std::shared_ptr<SfxItemSet>& pStyleHandle )
{
    bool bRet = false;

    SfxItemSet* pNewSet = nullptr;

    if ( !pSet1 )
    {
        OSL_ENSURE( nWhichId, "lcl_HaveCommonAttributes not used correctly" );
        if ( SfxItemState::SET == rSet2.GetItemState( nWhichId, false ) )
        {
            pNewSet = rSet2.Clone();
            pNewSet->ClearItem( nWhichId );
        }
    }
    else if ( pSet1->Count() )
    {
        SfxItemIter aIter( *pSet1 );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            if ( SfxItemState::SET == rSet2.GetItemState( pItem->Which(), false ) )
            {
                if ( !pNewSet )
                    pNewSet = rSet2.Clone();
                pNewSet->ClearItem( pItem->Which() );
            }

            if( aIter.IsAtEnd() )
                break;

            pItem = aIter.NextItem();
        }
    }

    if ( pNewSet )
    {
        if ( pNewSet->Count() )
            pStyleHandle = rStyleAccess.getAutomaticStyle( *pNewSet, IStyleAccess::AUTO_STYLE_CHAR );
        delete pNewSet;
        bRet = true;
    }

    return bRet;
}

/** Delete all attributes
 *
 * 5 cases:
 * 1) The attribute is completely in the deletion range:
 *    -> delete it
 * 2) The end of the attribute is in the deletion range:
 *    -> delete it, then re-insert it with new end
 * 3) The start of the attribute is in the deletion range:
 *    -> delete it, then re-insert it with new start
 * 4) The attribute contains the deletion range:
 *       Split, i.e.,
 *    -> Delete, re-insert from old start to start of deletion range
 *    -> insert new attribute from end of deletion range to old end
 * 5) The attribute is outside the deletion range
 *    -> nothing to do
 *
 * @param rIdx starting position
 * @param nLen length of the deletion
 * @param nthat ???
 * @param pSet ???
 * @param bInclRefToxMark ???
 */

void SwTextNode::RstTextAttr(
    const SwIndex &rIdx,
    const sal_Int32 nLen,
    const sal_uInt16 nWhich,
    const SfxItemSet* pSet,
    const bool bInclRefToxMark,
    const bool bExactRange )
{
    if ( !GetpSwpHints() )
        return;

    sal_Int32 nStt = rIdx.GetIndex();
    sal_Int32 nEnd = nStt + nLen;
    {
        // enlarge range for the reset of text attributes in case of an overlapping input field
        const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt( nStt, RES_TXTATR_INPUTFIELD, PARENT ));
        if ( pTextInputField == nullptr )
        {
            pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt(nEnd, RES_TXTATR_INPUTFIELD, PARENT ));
        }
        if ( pTextInputField != nullptr )
        {
            if ( nStt > pTextInputField->GetStart() )
            {
                nStt = pTextInputField->GetStart();
            }
            if ( nEnd < *(pTextInputField->End()) )
            {
                nEnd = *(pTextInputField->End());
            }
        }
    }

    bool bChanged = false;

    // nMin and nMax initialized to maximum / minimum (inverse)
    sal_Int32 nMin = m_Text.getLength();
    sal_Int32 nMax = nStt;
    const bool bNoLen = nMin == 0;

    // We have to remember the "new" attributes that have
    // been introduced by splitting surrounding attributes (case 2,3,4).
    std::vector<SwTextAttr *> newAttributes;
    std::vector<SwTextAttr *> delAttributes;

    // iterate over attribute array until start of attribute is behind deletion range
    size_t i = 0;
    sal_Int32 nAttrStart;
    SwTextAttr *pHt = nullptr;
    while ( (i < m_pSwpHints->Count())
            && ( ( ( nAttrStart = m_pSwpHints->Get(i)->GetStart()) < nEnd )
                 || nLen==0 ) && !bExactRange)
    {
        pHt = m_pSwpHints->Get(i);

        // attributes without end stay in!
        // but consider <bInclRefToxMark> used by Undo
        sal_Int32* const pAttrEnd = pHt->GetEnd();
        const bool bKeepAttrWithoutEnd =
            pAttrEnd == nullptr
            && ( !bInclRefToxMark
                 || ( RES_TXTATR_REFMARK != pHt->Which()
                      && RES_TXTATR_TOXMARK != pHt->Which()
                      && RES_TXTATR_META != pHt->Which()
                      && RES_TXTATR_METAFIELD != pHt->Which() ) );
        if ( bKeepAttrWithoutEnd )
        {

            i++;
            continue;
        }
        // attributes with content stay in
        if ( pHt->HasContent() )
        {
            ++i;
            continue;
        }

        // Default behavior is to process all attributes:
        bool bSkipAttr = false;
        std::shared_ptr<SfxItemSet> pStyleHandle;

        // 1. case: We want to reset only the attributes listed in pSet:
        if ( pSet )
        {
            bSkipAttr = SfxItemState::SET != pSet->GetItemState( pHt->Which(), false );
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                // if the current attribute is an autostyle, we have to check if the autostyle
                // and pSet have any attributes in common. If so, pStyleHandle will contain
                // a handle to AutoStyle / pSet:
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), pSet, 0, *static_cast<const SwFormatAutoFormat&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( nWhich )
        {
            // 2. case: We want to reset only the attributes with WhichId nWhich:
            bSkipAttr = nWhich != pHt->Which();
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), nullptr, nWhich, *static_cast<const SwFormatAutoFormat&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( !bInclRefToxMark )
        {
            // 3. case: Reset all attributes except from ref/toxmarks:
            // skip hints with CH_TXTATR here
            // (deleting those is ONLY allowed for UNDO!)
            bSkipAttr = RES_TXTATR_REFMARK   == pHt->Which()
                     || RES_TXTATR_TOXMARK   == pHt->Which()
                     || RES_TXTATR_META      == pHt->Which()
                     || RES_TXTATR_METAFIELD == pHt->Which();
        }

        if ( bSkipAttr )
        {
            i++;
            continue;
        }

        if (nStt <= nAttrStart)     // Case: 1,3,5
        {
            const sal_Int32 nAttrEnd = pAttrEnd != nullptr
                                        ? *pAttrEnd
                                        : nAttrStart;
            if (nEnd > nAttrStart
                || (nEnd == nAttrEnd && nEnd == nAttrStart)) // Case: 1,3
            {
                if ( nMin > nAttrStart )
                    nMin = nAttrStart;
                if ( nMax < nAttrEnd )
                    nMax = nAttrEnd;
                // If only a no-extent hint is deleted, no resorting is needed
                bChanged = bChanged || nEnd > nAttrStart || bNoLen;
                if (nAttrEnd <= nEnd)   // Case: 1
                {
                    delAttributes.push_back(pHt);

                    if ( pStyleHandle.get() )
                    {
                        SwTextAttr* pNew = MakeTextAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nAttrEnd );
                        newAttributes.push_back(pNew);
                    }
                }
                else    // Case: 3
                {
                    bChanged = true;
                    m_pSwpHints->NoteInHistory( pHt );
                    // UGLY: this may temporarily destroy the sorting!
                    pHt->GetStart() = nEnd;
                    m_pSwpHints->NoteInHistory( pHt, true );

                    if ( pStyleHandle.get() && nAttrStart < nEnd )
                    {
                        SwTextAttr* pNew = MakeTextAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nEnd );
                        newAttributes.push_back(pNew);
                    }
                }
            }
        }
        else if (pAttrEnd != nullptr)         // Case: 2,4,5
        {
            if (*pAttrEnd > nStt)       // Case: 2,4
            {
                if (*pAttrEnd < nEnd)   // Case: 2
                {
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = true;

                    const sal_Int32 nAttrEnd = *pAttrEnd;

                    m_pSwpHints->NoteInHistory( pHt );
                    // UGLY: this may temporarily destroy the sorting!
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, true );

                    if ( pStyleHandle.get() )
                    {
                        SwTextAttr* pNew = MakeTextAttr( *GetDoc(),
                            *pStyleHandle, nStt, nAttrEnd );
                        newAttributes.push_back(pNew);
                    }
                }
                else if (nLen)  // Case: 4
                {
                    // for Length 0 both hints would be merged again by
                    // InsertHint, so leave them alone!
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = true;
                    const sal_Int32 nTmpEnd = *pAttrEnd;
                    m_pSwpHints->NoteInHistory( pHt );
                    // UGLY: this may temporarily destroy the sorting!
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, true );

                    if ( pStyleHandle.get() && nStt < nEnd )
                    {
                        SwTextAttr* pNew = MakeTextAttr( *GetDoc(),
                            *pStyleHandle, nStt, nEnd );
                        newAttributes.push_back(pNew);
                    }

                    if( nEnd < nTmpEnd )
                    {
                        SwTextAttr* pNew = MakeTextAttr( *GetDoc(),
                            pHt->GetAttr(), nEnd, nTmpEnd );
                        if ( pNew )
                        {
                            SwTextCharFormat* pCharFormat = dynamic_cast<SwTextCharFormat*>(pHt);
                            if ( pCharFormat )
                                static_txtattr_cast<SwTextCharFormat*>(pNew)->SetSortNumber(pCharFormat->GetSortNumber());

                            newAttributes.push_back(pNew);
                        }
                    }
                }
            }
        }
        ++i;
    }

    if (bExactRange)
    {
        // Only delete the hints which start at nStt and end at nEnd.
        for (i = 0; i < m_pSwpHints->Count(); ++i)
        {
            SwTextAttr* pHint = m_pSwpHints->Get(i);
            if ( (isTXTATR_WITHEND(pHint->Which()) && RES_TXTATR_AUTOFMT != pHint->Which())
                || pHint->GetStart() != nStt)
                continue;

            const sal_Int32* pHintEnd = pHint->GetEnd();
            if (!pHintEnd || *pHintEnd != nEnd)
                continue;

            delAttributes.push_back(pHint);
        }
    }

    if (bChanged && !delAttributes.empty())
    {   // Delete() calls GetStartOf() - requires sorted hints!
        m_pSwpHints->Resort();
    }

    // delay deleting the hints because it re-sorts the hints array
    for (SwTextAttr *const pDel : delAttributes)
    {
        m_pSwpHints->Delete(pDel);
        DestroyAttr(pDel);
    }

    // delay inserting the hints because it re-sorts the hints array
    for (SwTextAttr *const pNew : newAttributes)
    {
        InsertHint(pNew, SetAttrMode::NOHINTADJUST);
    }

    TryDeleteSwpHints();

    if (bChanged)
    {
        if ( HasHints() )
        {   // possibly sometimes Resort would be sufficient, but...
            m_pSwpHints->MergePortions(*this);
        }

        // TextFrame's respond to aHint, others to aNew
        SwUpdateAttr aHint(
            nMin,
            nMax,
            0);

        NotifyClients( nullptr, &aHint );
        SwFormatChg aNew( GetFormatColl() );
        NotifyClients( nullptr, &aNew );
    }
}

sal_Int32 clipIndexBounds(const OUString &rStr, sal_Int32 nPos)
{
    if (nPos < 0)
        return 0;
    if (nPos > rStr.getLength())
        return rStr.getLength();
    return nPos;
}

// Aktuelles Wort zurueckliefern:
// Wir suchen immer von links nach rechts, es wird also das Wort
// vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
// Absatzes, dann wird das erste Wort zurueckgeliefert.
// Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
// einen leeren String.
OUString SwTextNode::GetCurWord( sal_Int32 nPos ) const
{
    assert(nPos <= m_Text.getLength()); // invalid index

    if (m_Text.isEmpty())
        return m_Text;

    Boundary aBndry;
    const uno::Reference< XBreakIterator > &rxBreak = g_pBreakIt->GetBreakIter();
    if (rxBreak.is())
    {
        sal_Int16 nWordType = WordType::DICTIONARY_WORD;
        lang::Locale aLocale( g_pBreakIt->GetLocale( GetLang( nPos ) ) );
#if OSL_DEBUG_LEVEL > 1
        sal_Bool bBegin = rxBreak->isBeginWord( m_Text, nPos, aLocale, nWordType );
        sal_Bool bEnd   = rxBreak->isEndWord  ( m_Text, nPos, aLocale, nWordType );
        (void)bBegin;
        (void)bEnd;
#endif
        aBndry =
            rxBreak->getWordBoundary( m_Text, nPos, aLocale, nWordType, true );

        // if no word was found use previous word (if any)
        if (aBndry.startPos == aBndry.endPos)
        {
            aBndry = rxBreak->previousWord( m_Text, nPos, aLocale, nWordType );
        }
    }

    // check if word was found and if it uses a symbol font, if so
    // enforce returning an empty string
    if (aBndry.endPos != aBndry.startPos && IsSymbol( aBndry.startPos ))
        aBndry.endPos = aBndry.startPos;

    // can have -1 as start/end of bounds not found
    aBndry.startPos = clipIndexBounds(m_Text, aBndry.startPos);
    aBndry.endPos = clipIndexBounds(m_Text, aBndry.endPos);

    return m_Text.copy(aBndry.startPos,
                       aBndry.endPos - aBndry.startPos);
}

SwScanner::SwScanner( const SwTextNode& rNd, const OUString& rText,
    const LanguageType* pLang, const ModelToViewHelper& rConvMap,
    sal_uInt16 nType, sal_Int32 nStart, sal_Int32 nEnde, bool bClp )
    : rNode( rNd )
    , aPreDashReplacementText(rText)
    , pLanguage( pLang )
    , m_ModelToView( rConvMap )
    , nLen( 0 )
    , nOverriddenDashCount( 0 )
    , nWordType( nType )
    , bClip( bClp )
{
    OSL_ENSURE( !aPreDashReplacementText.isEmpty(), "SwScanner: EmptyString" );
    nStartPos = nBegin = nStart;
    nEndPos = nEnde;

    //MSWord f.e has special emdash and endash behaviour in that they break
    //words for the purposes of word counting, while a hyphen etc. doesn't.

    //The default configuration treats emdash/endash as a word break, but
    //additional ones can be added in under tools->options
    if (nWordType == i18n::WordType::WORD_COUNT)
    {
        OUString sDashes = officecfg::Office::Writer::WordCount::AdditionalSeparators::get();
        OUStringBuffer aBuf(aPreDashReplacementText);
        for (sal_Int32 i = nStartPos; i < nEndPos; ++i)
        {
            if (i < 0)
                continue;
            sal_Unicode cChar = aBuf[i];
            if (sDashes.indexOf(cChar) != -1)
            {
                aBuf[i] = ' ';
                ++nOverriddenDashCount;
            }
        }
        aText = aBuf.makeStringAndClear();
    }
    else
        aText = aPreDashReplacementText;

    assert(aPreDashReplacementText.getLength() == aText.getLength());

    if ( pLanguage )
    {
        aCurrLang = *pLanguage;
    }
    else
    {
        ModelToViewHelper::ModelPosition aModelBeginPos =
            m_ModelToView.ConvertToModelPosition( nBegin );
        aCurrLang = rNd.GetLang( aModelBeginPos.mnPos );
    }
}

namespace
{
    //fdo#45271 for Asian words count characters instead of words
    sal_Int32 forceEachAsianCodePointToWord(const OUString &rText, sal_Int32 nBegin, sal_Int32 nLen)
    {
        if (nLen > 1)
        {
            const uno::Reference< XBreakIterator > &rxBreak = g_pBreakIt->GetBreakIter();

            sal_uInt16 nCurrScript = rxBreak->getScriptType( rText, nBegin );

            sal_Int32 indexUtf16 = nBegin;
            rText.iterateCodePoints(&indexUtf16);

            //First character is Asian, consider it a word :-(
            if (nCurrScript == i18n::ScriptType::ASIAN)
            {
                nLen = indexUtf16 - nBegin;
                return nLen;
            }

            //First character was not Asian, consider appearance of any Asian character
            //to be the end of the word
            while (indexUtf16 < nBegin + nLen)
            {
                nCurrScript = rxBreak->getScriptType( rText, indexUtf16 );
                if (nCurrScript == i18n::ScriptType::ASIAN)
                {
                    nLen = indexUtf16 - nBegin;
                    return nLen;
                }
                rText.iterateCodePoints(&indexUtf16);
            }
        }
        return nLen;
    }
}

bool SwScanner::NextWord()
{
    nBegin = nBegin + nLen;
    Boundary aBound;

    CharClass& rCC = GetAppCharClass();
    LanguageTag aOldLanguageTag = rCC.getLanguageTag();

    while ( true )
    {
        // skip non-letter characters:
        while (nBegin < aText.getLength())
        {
            if (nBegin >= 0 && !u_isspace(aText[nBegin]))
            {
                if ( !pLanguage )
                {
                    const sal_uInt16 nNextScriptType = g_pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
                    ModelToViewHelper::ModelPosition aModelBeginPos =
                        m_ModelToView.ConvertToModelPosition( nBegin );
                    aCurrLang = rNode.GetLang( aModelBeginPos.mnPos, 1, nNextScriptType );
                }

                if ( nWordType != i18n::WordType::WORD_COUNT )
                {
                    rCC.setLanguageTag( LanguageTag( g_pBreakIt->GetLocale( aCurrLang )) );
                    if ( rCC.isLetterNumeric(OUString(aText[nBegin])) )
                        break;
                }
                else
                    break;
            }
            ++nBegin;
        }

        if ( nBegin >= aText.getLength() || nBegin >= nEndPos )
            return false;

        // get the word boundaries
        aBound = g_pBreakIt->GetBreakIter()->getWordBoundary( aText, nBegin,
                g_pBreakIt->GetLocale( aCurrLang ), nWordType, true );
        OSL_ENSURE( aBound.endPos >= aBound.startPos, "broken aBound result" );

        // we don't want to include preceding text
        // to count words in text with mixed script punctuation correctly,
        // but we want to include preceding symbols (eg. percent sign, section sign,
        // degree sign defined by dict_word_hu to spell check their affixed forms).
        if (nWordType == i18n::WordType::WORD_COUNT && aBound.startPos < nBegin)
            aBound.startPos = nBegin;

        //no word boundaries could be found
        if(aBound.endPos == aBound.startPos)
            return false;

        //if a word before is found it has to be searched for the next
        if(aBound.endPos == nBegin)
            ++nBegin;
        else
            break;
    } // end while( true )

    rCC.setLanguageTag( aOldLanguageTag );

    // #i89042, as discussed with HDU: don't evaluate script changes for word count. Use whole word.
    if ( nWordType == i18n::WordType::WORD_COUNT )
    {
        nBegin = std::max(aBound.startPos, nBegin);
        nLen   = 0;
        if (aBound.endPos > nBegin)
            nLen = aBound.endPos - nBegin;
    }
    else
    {
        // we have to differenciate between these cases:
        if ( aBound.startPos <= nBegin )
        {
            OSL_ENSURE( aBound.endPos >= nBegin, "Unexpected aBound result" );

            // restrict boundaries to script boundaries and nEndPos
            const sal_uInt16 nCurrScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
            OUString aTmpWord = aText.copy( nBegin, aBound.endPos - nBegin );
            const sal_Int32 nScriptEnd = nBegin +
                g_pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = std::min( aBound.endPos, nScriptEnd );

            // restrict word start to last script change position
            sal_Int32 nScriptBegin = 0;
            if ( aBound.startPos < nBegin )
            {
                // search from nBegin backwards until the next script change
                aTmpWord = aText.copy( aBound.startPos,
                                       nBegin - aBound.startPos + 1 );
                nScriptBegin = aBound.startPos +
                    g_pBreakIt->GetBreakIter()->beginOfScript( aTmpWord, nBegin - aBound.startPos,
                                                    nCurrScript );
            }

            nBegin = std::max( aBound.startPos, nScriptBegin );
            nLen = nEnd - nBegin;
        }
        else
        {
            const sal_uInt16 nCurrScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, aBound.startPos );
            OUString aTmpWord = aText.copy( aBound.startPos,
                                             aBound.endPos - aBound.startPos );
            const sal_Int32 nScriptEnd = aBound.startPos +
                g_pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = std::min( aBound.endPos, nScriptEnd );
            nBegin = aBound.startPos;
            nLen = nEnd - nBegin;
        }
    }

    // optionally clip the result of getWordBoundaries:
    if ( bClip )
    {
        aBound.startPos = std::max( aBound.startPos, nStartPos );
        aBound.endPos = std::min( aBound.endPos, nEndPos );
        nBegin = aBound.startPos;
        nLen = aBound.endPos - nBegin;
    }

    if( ! nLen )
        return false;

    if ( nWordType == i18n::WordType::WORD_COUNT )
        nLen = forceEachAsianCodePointToWord(aText, nBegin, nLen);

    aWord = aPreDashReplacementText.copy( nBegin, nLen );

    return true;
}

bool SwTextNode::Spell(SwSpellArgs* pArgs)
{
    // Die Aehnlichkeiten zu SwTextFrame::_AutoSpell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!

    // modify string according to redline information and hidden text
    const OUString aOldText( m_Text );
    OUStringBuffer buf(m_Text);
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText(*this, buf, 0, m_Text.getLength());
    if (bRestoreString)
    {   // ??? UGLY: is it really necessary to modify m_Text here?
        m_Text = buf.makeStringAndClear();
    }

    sal_Int32 nBegin = ( pArgs->pStartNode != this )
        ? 0
        : pArgs->pStartIdx->GetIndex();

    sal_Int32 nEnd = ( pArgs->pEndNode != this )
            ? m_Text.getLength()
            : pArgs->pEndIdx->GetIndex();

    pArgs->xSpellAlt = nullptr;

    // 4 cases:

    // 1. IsWrongDirty = 0 and GetWrong = 0
    //      Everything is checked and correct
    // 2. IsWrongDirty = 0 and GetWrong = 1
    //      Everything is checked and errors are identified in the wrong list
    // 3. IsWrongDirty = 1 and GetWrong = 0
    //      Nothing has been checked
    // 4. IsWrongDirty = 1 and GetWrong = 1
    //      Text has been checked but there is an invalid range in the wrong list

    // Nothing has to be done for case 1.
    if ( ( IsWrongDirty() || GetWrong() ) && m_Text.getLength() )
    {
        if (nBegin > m_Text.getLength())
        {
            nBegin = m_Text.getLength();
        }
        if (nEnd > m_Text.getLength())
        {
            nEnd = m_Text.getLength();
        }

        if(!IsWrongDirty())
        {
            const sal_Int32 nTemp = GetWrong()->NextWrong( nBegin );
            if(nTemp > nEnd)
            {
                // reset original text
                if ( bRestoreString )
                {
                    m_Text = aOldText;
                }
                return false;
            }
            if(nTemp > nBegin)
                nBegin = nTemp;

        }

        // In case 2. we pass the wrong list to the scanned, because only
        // the words in the wrong list have to be checked
        SwScanner aScanner( *this, m_Text, nullptr, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord() )
        {
            const OUString& rWord = aScanner.GetWord();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            if( rWord.getLength() > 0 && LANGUAGE_NONE != eActLang )
            {
                if (pArgs->xSpeller.is())
                {
                    SvxSpellWrapper::CheckSpellLang( pArgs->xSpeller, eActLang );
                    pArgs->xSpellAlt = pArgs->xSpeller->spell( rWord, eActLang,
                                            Sequence< PropertyValue >() );
                }
                if( (pArgs->xSpellAlt).is() )
                {
                    if( IsSymbol( aScanner.GetBegin() ) )
                    {
                        pArgs->xSpellAlt = nullptr;
                    }
                    else
                    {
                        // make sure the selection build later from the data
                        // below does not include "in word" character to the
                        // left and right in order to preserve those. Therefore
                        // count those "in words" in order to modify the
                        // selection accordingly.
                        const sal_Unicode* pChar = rWord.getStr();
                        sal_Int32 nLeft = 0;
                        while (pChar && *pChar++ == CH_TXTATR_INWORD)
                            ++nLeft;
                        pChar = rWord.getLength() ? rWord.getStr() + rWord.getLength() - 1 : nullptr;
                        sal_Int32 nRight = 0;
                        while (pChar && *pChar-- == CH_TXTATR_INWORD)
                            ++nRight;

                        pArgs->pStartNode = this;
                        pArgs->pEndNode = this;
                        pArgs->pStartIdx->Assign(this, aScanner.GetEnd() - nRight );
                        pArgs->pEndIdx->Assign(this, aScanner.GetBegin() + nLeft );
                    }
                }
            }
        }
    }

    // reset original text
    if ( bRestoreString )
    {
        m_Text = aOldText;
    }

    return pArgs->xSpellAlt.is();
}

void SwTextNode::SetLanguageAndFont( const SwPaM &rPaM,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const vcl::Font *pFont,  sal_uInt16 nFontWhichId )
{
    sal_uInt16 aRanges[] = {
            nLangWhichId, nLangWhichId,
            nFontWhichId, nFontWhichId,
            0, 0, 0 };
    if (!pFont)
        aRanges[2] = aRanges[3] = 0;    // clear entries with font WhichId

    SwEditShell *pEditShell = GetDoc()->GetEditShell();
    SfxItemSet aSet( pEditShell->GetAttrPool(), aRanges );
    aSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    OSL_ENSURE( pFont, "target font missing?" );
    if (pFont)
    {
        SvxFontItem aFontItem = static_cast<const SvxFontItem&>( aSet.Get( nFontWhichId ) );
        aFontItem.SetFamilyName(   pFont->GetFamilyName());
        aFontItem.SetFamily(       pFont->GetFamily());
        aFontItem.SetStyleName(    pFont->GetStyleName());
        aFontItem.SetPitch(        pFont->GetPitch());
        aFontItem.SetCharSet( pFont->GetCharSet() );
        aSet.Put( aFontItem );
    }

    GetDoc()->getIDocumentContentOperations().InsertItemSet( rPaM, aSet );
    // SetAttr( aSet );    <- Does not set language attribute of empty paragraphs correctly,
    //                     <- because since there is no selection the flag to garbage
    //                     <- collect all attributes is set, and therefore attributes spanned
    //                     <- over empty selection are removed.

}

bool SwTextNode::Convert( SwConversionArgs &rArgs )
{
    // get range of text within node to be converted
    // (either all the text or the text within the selection
    // when the conversion was started)
    const sal_Int32 nTextBegin = ( rArgs.pStartNode == this )
        ? ::std::min(rArgs.pStartIdx->GetIndex(), m_Text.getLength())
        : 0;

    const sal_Int32 nTextEnd = ( rArgs.pEndNode == this )
        ?  ::std::min(rArgs.pEndIdx->GetIndex(), m_Text.getLength())
        :  m_Text.getLength();

    rArgs.aConvText.clear();

    // modify string according to redline information and hidden text
    const OUString aOldText( m_Text );
    OUStringBuffer buf(m_Text);
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText(*this, buf, 0, m_Text.getLength());
    if (bRestoreString)
    {   // ??? UGLY: is it really necessary to modify m_Text here?
        m_Text = buf.makeStringAndClear();
    }

    bool    bFound  = false;
    sal_Int32  nBegin  = nTextBegin;
    sal_Int32  nLen = 0;
    LanguageType nLangFound = LANGUAGE_NONE;
    if (m_Text.isEmpty())
    {
        if (rArgs.bAllowImplicitChangesForNotConvertibleText)
        {
            // create SwPaM with mark & point spanning empty paragraph
            //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
            SwPaM aCurPaM( *this, 0 );

            SetLanguageAndFont( aCurPaM,
                    rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                    rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }
    }
    else
    {
        SwLanguageIterator aIter( *this, nBegin );

        // Implicit changes require setting new attributes, which in turn destroys
        // the attribute sequence on that aIter iterates. We store the necessary
        // coordinates and apply those changes after iterating through the text.
        typedef std::pair<sal_Int32, sal_Int32> ImplicitChangesRange;
        std::vector<ImplicitChangesRange> aImplicitChanges;

        // find non zero length text portion of appropriate language
        do {
            nLangFound = aIter.GetLanguage();
            bool bLangOk =  (nLangFound == rArgs.nConvSrcLang) ||
                                (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 editeng::HangulHanjaConversion::IsChinese( rArgs.nConvSrcLang ));

            sal_Int32 nChPos = aIter.GetChgPos();
            // the position at the end of the paragraph is COMPLETE_STRING and
            // thus must be cut to the end of the actual string.
            assert(nChPos != -1);
            if (nChPos == -1 || nChPos == COMPLETE_STRING)
            {
                nChPos = m_Text.getLength();
            }

            nLen = nChPos - nBegin;
            bFound = bLangOk && nLen > 0;
            if (!bFound)
            {
                // create SwPaM with mark & point spanning the attributed text
                //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
                SwPaM aCurPaM( *this, nBegin );
                aCurPaM.SetMark();
                aCurPaM.GetPoint()->nContent = nBegin + nLen;

                // check script type of selected text
                SwEditShell *pEditShell = GetDoc()->GetEditShell();
                pEditShell->Push();             // save current cursor on stack
                pEditShell->SetSelection( aCurPaM );
                bool bIsAsianScript = (SvtScriptType::ASIAN == pEditShell->GetScriptType());
                pEditShell->Pop( false );   // restore cursor from stack

                if (!bIsAsianScript && rArgs.bAllowImplicitChangesForNotConvertibleText)
                {
                    // Store for later use
                    aImplicitChanges.push_back(ImplicitChangesRange(nBegin, nBegin+nLen));
                }
                nBegin = nChPos;    // start of next language portion
            }
        } while (!bFound && aIter.Next());  /* loop while nothing was found and still sth is left to be searched */

        // Apply implicit changes, if any, now that aIter is no longer used
        for (size_t i = 0; i < aImplicitChanges.size(); ++i)
        {
            SwPaM aPaM( *this, aImplicitChanges[i].first );
            aPaM.SetMark();
            aPaM.GetPoint()->nContent = aImplicitChanges[i].second;
            SetLanguageAndFont( aPaM, rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE, rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }

    }

    // keep resulting text within selection / range of text to be converted
    if (nBegin < nTextBegin)
        nBegin = nTextBegin;
    if (nBegin + nLen > nTextEnd)
        nLen = nTextEnd - nBegin;
    bool bInSelection = nBegin < nTextEnd;

    if (bFound && bInSelection)     // convertible text found within selection/range?
    {
        OSL_ENSURE( !m_Text.isEmpty(), "convertible text portion missing!" );
        rArgs.aConvText     = m_Text.copy(nBegin, nLen);
        rArgs.nConvTextLang = nLangFound;

        // position where to start looking in next iteration (after current ends)
        rArgs.pStartNode = this;
        rArgs.pStartIdx->Assign(this, nBegin + nLen );
        // end position (when we have travelled over the whole document)
        rArgs.pEndNode = this;
        rArgs.pEndIdx->Assign(this, nBegin );
    }

    // restore original text
    if ( bRestoreString )
    {
        m_Text = aOldText;
    }

    return !rArgs.aConvText.isEmpty();
}

// Die Aehnlichkeiten zu SwTextNode::Spell sind beabsichtigt ...
// ACHTUNG: Ev. Bugs in beiden Routinen fixen!
SwRect SwTextFrame::_AutoSpell( const SwContentNode* pActNode, sal_Int32 nActPos )
{
    SwRect aRect;
#if OSL_DEBUG_LEVEL > 1
    static bool bStop = false;
    if ( bStop )
        return aRect;
#endif
    // Die Aehnlichkeiten zu SwTextNode::Spell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!
    SwTextNode *pNode = GetTextNode();
    if( pNode != pActNode || !nActPos )
        nActPos = COMPLETE_STRING;

    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    // modify string according to redline information and hidden text
    const OUString aOldText( pNode->GetText() );
    OUStringBuffer buf(pNode->m_Text);
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText(*pNode, buf, 0, pNode->GetText().getLength());
    if (bRestoreString)
    {   // ??? UGLY: is it really necessary to modify m_Text here?
        pNode->m_Text = buf.makeStringAndClear();
    }

    // a change of data indicates that at least one word has been modified
    const bool bRedlineChg = (pNode->GetText().getStr() != aOldText.getStr());

    sal_Int32 nBegin = 0;
    sal_Int32 nEnd = pNode->GetText().getLength();
    sal_Int32 nInsertPos = 0;
    sal_Int32 nChgStart = COMPLETE_STRING;
    sal_Int32 nChgEnd = 0;
    sal_Int32 nInvStart = COMPLETE_STRING;
    sal_Int32 nInvEnd = 0;

    const bool bAddAutoCmpl = pNode->IsAutoCompleteWordDirty() &&
                                  SwViewOption::IsAutoCompleteWords();

    if( pNode->GetWrong() )
    {
        nBegin = pNode->GetWrong()->GetBeginInv();
        if( COMPLETE_STRING != nBegin )
        {
            nEnd = std::max(pNode->GetWrong()->GetEndInv(), pNode->GetText().getLength());
        }

        // get word around nBegin, we start at nBegin - 1
        if ( COMPLETE_STRING != nBegin )
        {
            if ( nBegin )
                --nBegin;

            LanguageType eActLang = pNode->GetLang( nBegin );
            Boundary aBound =
                g_pBreakIt->GetBreakIter()->getWordBoundary( pNode->GetText(), nBegin,
                    g_pBreakIt->GetLocale( eActLang ),
                    WordType::DICTIONARY_WORD, true );
            nBegin = aBound.startPos;
        }

        // get the position in the wrong list
        nInsertPos = pNode->GetWrong()->GetWrongPos( nBegin );

        // sometimes we have to skip one entry
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
                nInsertPos++;
    }

    bool bFresh = nBegin < nEnd;
    bool bPending(false);

    if( bFresh )
    {
        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        uno::Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );
        SwDoc* pDoc = pNode->GetDoc();

        SwScanner aScanner( *pNode, pNode->GetText(), nullptr, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD, nBegin, nEnd);

        while( aScanner.NextWord() )
        {
            const OUString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            sal_Int32 nLen = aScanner.GetLen();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            bool bSpell = xSpell.is() && xSpell->hasLanguage( eActLang );
            if( bSpell && !rWord.isEmpty() )
            {
                // check for: bAlter => xHyphWord.is()
                OSL_ENSURE(!bSpell || xSpell.is(), "NULL pointer");

                if( !xSpell->isValid( rWord, eActLang, Sequence< PropertyValue >() ) )
                {
                    sal_Int32 nSmartTagStt = nBegin;
                    sal_Int32 nDummy = 1;
                    if ( !pNode->GetSmartTags() || !pNode->GetSmartTags()->InWrongWord( nSmartTagStt, nDummy ) )
                    {
                        if( !pNode->GetWrong() )
                        {
                            pNode->SetWrong( new SwWrongList( WRONGLIST_SPELL ) );
                            pNode->GetWrong()->SetInvalid( 0, nEnd );
                        }
                        SwWrongList::FreshState const eState(pNode->GetWrong()->Fresh(
                            nChgStart, nChgEnd, nBegin, nLen, nInsertPos, nActPos));
                        switch (eState)
                        {
                            case SwWrongList::FreshState::FRESH:
                                pNode->GetWrong()->Insert(OUString(), nullptr, nBegin, nLen, nInsertPos++);
                                break;
                            case SwWrongList::FreshState::CURSOR:
                                bPending = true; // fall-through to mark as invalid
                            case SwWrongList::FreshState::NOTHING:
                                nInvStart = nBegin;
                                nInvEnd = nBegin + nLen;
                                break;
                        }
                    }
                }
                else if( bAddAutoCmpl && rACW.GetMinWordLen() <= rWord.getLength() )
                {
                    if ( bRedlineChg )
                    {
                        OUString rNewWord( rWord );
                        rACW.InsertWord( rNewWord, *pDoc );
                    }
                    else
                        rACW.InsertWord( rWord, *pDoc );
                }
            }
        }
    }

    // reset original text
    // i63141 before calling GetCharRect(..) with formatting!
    if ( bRestoreString )
    {
        pNode->m_Text = aOldText;
    }
    if( pNode->GetWrong() )
    {
        if( bFresh )
            pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                                      nEnd, 0, nInsertPos, nActPos );

        // Calculate repaint area:

        if( nChgStart < nChgEnd )
        {
            aRect = lcl_CalculateRepaintRect( *this, nChgStart, nChgEnd );

            // fdo#71558 notify misspelled word to accessibility
            SwViewShell* pViewSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
            if( pViewSh )
                pViewSh->InvalidateAccessibleParaAttrs( *this );
        }

        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty(
            (COMPLETE_STRING != pNode->GetWrong()->GetBeginInv())
                ? ((bPending)
                    ? SwTextNode::WrongState::PENDING
                    : SwTextNode::WrongState::TODO)
                : SwTextNode::WrongState::DONE);
        if( !pNode->GetWrong()->Count() && ! pNode->IsWrongDirty() )
            pNode->SetWrong( nullptr );
    }
    else
        pNode->SetWrongDirty(SwTextNode::WrongState::DONE);

    if( bAddAutoCmpl )
        pNode->SetAutoCompleteWordDirty( false );

    return aRect;
}

/** Function: SmartTagScan

    Function scans words in current text and checks them in the
    smarttag libraries. If the check returns true to bounds of the
    recognized words are stored into a list that is used later for drawing
    the underline.

    @param pActNode ???
    @param nActPos ???
    @return SwRect Repaint area
*/
SwRect SwTextFrame::SmartTagScan( SwContentNode* /*pActNode*/, sal_Int32 /*nActPos*/ )
{
    SwRect aRet;
    SwTextNode *pNode = GetTextNode();
    const OUString& rText = pNode->GetText();

    // Iterate over language portions
    SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();

    SwWrongList* pSmartTagList = pNode->GetSmartTags();

    sal_Int32 nBegin = 0;
    sal_Int32 nEnd = rText.getLength();

    if ( pSmartTagList )
    {
        if ( pSmartTagList->GetBeginInv() != COMPLETE_STRING )
        {
            nBegin = pSmartTagList->GetBeginInv();
            nEnd = std::min( pSmartTagList->GetEndInv(), rText.getLength() );

            if ( nBegin < nEnd )
            {
                const LanguageType aCurrLang = pNode->GetLang( nBegin );
                const css::lang::Locale aCurrLocale = g_pBreakIt->GetLocale( aCurrLang );
                nBegin = g_pBreakIt->GetBreakIter()->beginOfSentence( rText, nBegin, aCurrLocale );
                nEnd = g_pBreakIt->GetBreakIter()->endOfSentence(rText, nEnd, aCurrLocale);
                if (nEnd > rText.getLength() || nEnd < 0)
                    nEnd = rText.getLength();
            }
        }
    }

    const sal_uInt16 nNumberOfEntries = pSmartTagList ? pSmartTagList->Count() : 0;
    sal_uInt16 nNumberOfRemovedEntries = 0;
    sal_uInt16 nNumberOfInsertedEntries = 0;

    // clear smart tag list between nBegin and nEnd:
    if ( 0 != nNumberOfEntries )
    {
        sal_Int32 nChgStart = COMPLETE_STRING;
        sal_Int32 nChgEnd = 0;
        const sal_uInt16 nCurrentIndex = pSmartTagList->GetWrongPos( nBegin );
        pSmartTagList->Fresh( nChgStart, nChgEnd, nBegin, nEnd - nBegin, nCurrentIndex, COMPLETE_STRING );
        nNumberOfRemovedEntries = nNumberOfEntries - pSmartTagList->Count();
    }

    if ( nBegin < nEnd )
    {
        // Expand the string:
        const ModelToViewHelper aConversionMap(*pNode /*TODO - replace or expand fields for smart tags?*/);
        OUString aExpandText = aConversionMap.getViewText();

        // Ownership ov ConversionMap is passed to SwXTextMarkup object!
        uno::Reference<text::XTextMarkup> const xTextMarkup =
             new SwXTextMarkup(pNode, aConversionMap);

        css::uno::Reference< css::frame::XController > xController = pNode->GetDoc()->GetDocShell()->GetController();

        SwPosition start(*pNode, nBegin);
        SwPosition end  (*pNode, nEnd);
        Reference< css::text::XTextRange > xRange = SwXTextRange::CreateXTextRange(*pNode->GetDoc(), start, &end);

        rSmartTagMgr.RecognizeTextRange(xRange, xTextMarkup, xController);

        sal_Int32 nLangBegin = nBegin;
        sal_Int32 nLangEnd;

        // smart tag recognition has to be done for each language portion:
        SwLanguageIterator aIter( *pNode, nLangBegin );

        do
        {
            const LanguageType nLang = aIter.GetLanguage();
            const css::lang::Locale aLocale = g_pBreakIt->GetLocale( nLang );
            nLangEnd = std::min<sal_Int32>( nEnd, aIter.GetChgPos() );

            const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nLangBegin );
            const sal_Int32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nLangEnd );

            rSmartTagMgr.RecognizeString(aExpandText, xTextMarkup, xController, aLocale, nExpandBegin, nExpandEnd - nExpandBegin );

            nLangBegin = nLangEnd;
        }
        while ( aIter.Next() && nLangEnd < nEnd );

        pSmartTagList = pNode->GetSmartTags();

        const sal_uInt16 nNumberOfEntriesAfterRecognize = pSmartTagList ? pSmartTagList->Count() : 0;
        nNumberOfInsertedEntries = nNumberOfEntriesAfterRecognize - ( nNumberOfEntries - nNumberOfRemovedEntries );
    }

    if( pSmartTagList )
    {
        // Update WrongList stuff
        pSmartTagList->SetInvalid( COMPLETE_STRING, 0 );
        pNode->SetSmartTagDirty( COMPLETE_STRING != pSmartTagList->GetBeginInv() );

        if( !pSmartTagList->Count() && !pNode->IsSmartTagDirty() )
            pNode->SetSmartTags( nullptr );

        // Calculate repaint area:
#if OSL_DEBUG_LEVEL > 1
        const sal_uInt16 nNumberOfEntriesAfterRecognize2 = pSmartTagList->Count();
        (void) nNumberOfEntriesAfterRecognize2;
#endif
        if ( nBegin < nEnd && ( 0 != nNumberOfRemovedEntries ||
                                0 != nNumberOfInsertedEntries ) )
        {
            aRet = lcl_CalculateRepaintRect( *this, nBegin, nEnd );
        }
    }
    else
        pNode->SetSmartTagDirty( false );

    return aRet;
}

// Wird vom CollectAutoCmplWords gerufen
void SwTextFrame::CollectAutoCmplWrds( SwContentNode* pActNode, sal_Int32 nActPos )
{
    SwTextNode *pNode = GetTextNode();
    if( pNode != pActNode || !nActPos )
        nActPos = COMPLETE_STRING;

    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    sal_Int32  nBegin = 0;
    sal_Int32  nEnd = pNode->GetText().getLength();
    sal_Int32  nLen;
    bool bACWDirty = false;

    if( nBegin < nEnd )
    {
        int nCnt = 200;
        SwScanner aScanner( *pNode, pNode->GetText(), nullptr, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD, nBegin, nEnd );
        while( aScanner.NextWord() )
        {
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();
            if( rACW.GetMinWordLen() <= nLen )
            {
                const OUString& rWord = aScanner.GetWord();

                if( nActPos < nBegin || ( nBegin + nLen ) < nActPos )
                {
                    if( rACW.GetMinWordLen() <= rWord.getLength() )
                        rACW.InsertWord( rWord, *pDoc );
                }
                else
                    bACWDirty = true;
            }
            if( !--nCnt )
            {
                // don't wait for TIMER here, so we can finish big paragraphs
                if (Application::AnyInput(VCL_INPUT_ANY & VclInputFlags(~VclInputFlags::TIMER)))
                    return;
                nCnt = 100;
            }
        }
    }

    if (!bACWDirty)
        pNode->SetAutoCompleteWordDirty( false );
}

/** Findet den TextFrame und sucht dessen CalcHyph */
bool SwTextNode::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    // Abkuerzung: am Absatz ist keine Sprache eingestellt:
    if ( LANGUAGE_NONE == sal_uInt16( GetSwAttrSet().GetLanguage().GetLanguage() )
         && USHRT_MAX == GetLang(0, m_Text.getLength()))
    {
        if( !rHyphInf.IsCheck() )
            rHyphInf.SetNoLang( true );
        return false;
    }

    SwTextFrame *pFrame = ::sw::SwHyphIterCacheLastTextFrame(this,
        [&rHyphInf, this]() {
            return static_cast<SwTextFrame*>(this->getLayoutFrame(
                this->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                rHyphInf.GetCursorPos()));
        });
    if( pFrame )
        pFrame = &(pFrame->GetFrameAtOfst( rHyphInf.nStart ));
    else
    {
        // 4935: Seit der Trennung ueber Sonderbereiche sind Faelle
        // moeglich, in denen kein Frame zum Node vorliegt.
        // Also keinOSL_ENSURE
        OSL_ENSURE( pFrame, "!SwTextNode::Hyphenate: can't find any frame" );
        return false;
    }

    while( pFrame )
    {
        if( pFrame->Hyphenate( rHyphInf ) )
        {
            // Das Layout ist nicht robust gegen "Direktformatierung"
            // (7821, 7662, 7408); vgl. layact.cxx,
            // SwLayAction::_TurboAction(), if( !pCnt->IsValid() ...
            pFrame->SetCompletePaint();
            return true;
        }
        pFrame = pFrame->GetFollow();
        if( pFrame )
        {
            rHyphInf.nEnd = rHyphInf.nEnd - (pFrame->GetOfst() - rHyphInf.nStart);
            rHyphInf.nStart = pFrame->GetOfst();
        }
    }
    return false;
}

namespace
{
    struct swTransliterationChgData
    {
        sal_Int32               nStart;
        sal_Int32               nLen;
        OUString                sChanged;
        Sequence< sal_Int32 >   aOffsets;
    };
}

// change text to Upper/Lower/Hiragana/Katagana/...
void SwTextNode::TransliterateText(
    utl::TransliterationWrapper& rTrans,
    sal_Int32 nStt, sal_Int32 nEnd,
    SwUndoTransliterate* pUndo )
{
    if (nStt < nEnd && g_pBreakIt->GetBreakIter().is())
    {
        // since we don't use Hiragana/Katakana or half-width/full-width transliterations here
        // it is fine to use ANYWORD_IGNOREWHITESPACES. (ANY_WORD btw is broken and will
        // occasionaly miss words in consecutive sentences). Also with ANYWORD_IGNOREWHITESPACES
        // text like 'just-in-time' will be converted to 'Just-In-Time' which seems to be the
        // proper thing to do.
        const sal_Int16 nWordType = WordType::ANYWORD_IGNOREWHITESPACES;

        // In order to have less trouble with changing text size, e.g. because
        // of ligatures or German small sz being resolved, we need to process
        // the text replacements from end to start.
        // This way the offsets for the yet to be changed words will be
        // left unchanged by the already replaced text.
        // For this we temporarily save the changes to be done in this vector
        std::vector< swTransliterationChgData >   aChanges;
        swTransliterationChgData                  aChgData;

        if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::TITLE_CASE)
        {
            // for 'capitalize every word' we need to iterate over each word

            Boundary aSttBndry;
            Boundary aEndBndry;
            aSttBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                        GetText(), nStt,
                        g_pBreakIt->GetLocale( GetLang( nStt ) ),
                        nWordType,
                        true /*prefer forward direction*/);
            aEndBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                        GetText(), nEnd,
                        g_pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType,
                        false /*prefer backward direction*/);

            // prevent backtracking to the previous word if selection is at word boundary
            if (aSttBndry.endPos <= nStt)
            {
                aSttBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetText(), aSttBndry.endPos,
                        g_pBreakIt->GetLocale( GetLang( aSttBndry.endPos ) ),
                        nWordType);
            }
            // prevent advancing to the next word if selection is at word boundary
            if (aEndBndry.startPos >= nEnd)
            {
                aEndBndry = g_pBreakIt->GetBreakIter()->previousWord(
                        GetText(), aEndBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aEndBndry.startPos ) ),
                        nWordType);
            }

            Boundary aCurWordBndry( aSttBndry );
            while (aCurWordBndry.startPos <= aEndBndry.startPos)
            {
                nStt = aCurWordBndry.startPos;
                nEnd = aCurWordBndry.endPos;
                const sal_Int32 nLen = nEnd - nStt;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(
                            GetText(), GetLang(nStt), nStt, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
                {
                    aChgData.nStart     = nStt;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                aCurWordBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetText(), nEnd,
                        g_pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType);
            }
        }
        else if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::SENTENCE_CASE)
        {
            // for 'sentence case' we need to iterate sentence by sentence

            sal_Int32 nLastStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                    GetText(), nEnd,
                    g_pBreakIt->GetLocale( GetLang( nEnd ) ) );
            sal_Int32 nLastEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                    GetText(), nLastStart,
                    g_pBreakIt->GetLocale( GetLang( nLastStart ) ) );

            // extend nStt, nEnd to the current sentence boundaries
            sal_Int32 nCurrentStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                    GetText(), nStt,
                    g_pBreakIt->GetLocale( GetLang( nStt ) ) );
            sal_Int32 nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                    GetText(), nCurrentStart,
                    g_pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );

            // prevent backtracking to the previous sentence if selection starts at end of a sentence
            if (nCurrentEnd <= nStt)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the next real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetText(), nCurrentEnd,
                        g_pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        i18n::WordType::DICTIONARY_WORD);

                // now get new current sentence boundaries
                nCurrentStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                        GetText(), aBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetText(), nCurrentStart,
                        g_pBreakIt->GetLocale( GetLang( nCurrentStart) ) );
            }
            // prevent advancing to the next sentence if selection ends at start of a sentence
            if (nLastStart >= nEnd)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the previous real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = g_pBreakIt->GetBreakIter()->previousWord(
                        GetText(), nLastStart,
                        g_pBreakIt->GetLocale( GetLang( nLastStart) ),
                        i18n::WordType::DICTIONARY_WORD);
                nLastEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetText(), aBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                if (nCurrentEnd > nLastEnd)
                    nCurrentEnd = nLastEnd;
            }

            while (nCurrentStart < nLastEnd)
            {
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(GetText(),
                    GetLang(nCurrentStart), nCurrentStart, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                Boundary aFirstWordBndry;
                aFirstWordBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetText(), nCurrentEnd,
                        g_pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        nWordType);
                nCurrentStart = aFirstWordBndry.startPos;
                nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetText(), nCurrentStart,
                        g_pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );
            }
        }
        else
        {
            // here we may transliterate over complete language portions...

            SwLanguageIterator* pIter;
            if( rTrans.needLanguageForTheMode() )
                pIter = new SwLanguageIterator( *this, nStt );
            else
                pIter = nullptr;

            sal_Int32 nEndPos = 0;
            sal_uInt16 nLang = LANGUAGE_NONE;
            do {
                if( pIter )
                {
                    nLang = pIter->GetLanguage();
                    nEndPos = pIter->GetChgPos();
                    if( nEndPos > nEnd )
                        nEndPos = nEnd;
                }
                else
                {
                    nLang = LANGUAGE_SYSTEM;
                    nEndPos = nEnd;
                }
                const sal_Int32 nLen = nEndPos - nStt;

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(
                            m_Text, nLang, nStt, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
                {
                    aChgData.nStart     = nStt;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                nStt = nEndPos;
            } while( nEndPos < nEnd && pIter && pIter->Next() );
            delete pIter;
        }

        if (!aChanges.empty())
        {
            // now apply the changes from end to start to leave the offsets of the
            // yet unchanged text parts remain the same.
            size_t nSum(0);
            for (size_t i = 0; i < aChanges.size(); ++i)
            {   // check this here since AddChanges cannot be moved below
                // call to ReplaceTextOnly
                swTransliterationChgData & rData =
                    aChanges[ aChanges.size() - 1 - i ];
                nSum += rData.sChanged.getLength() - rData.nLen;
                if (nSum > static_cast<size_t>(GetSpaceLeft()))
                {
                    SAL_WARN("sw.core", "SwTextNode::ReplaceTextOnly: "
                            "node text with insertion > node capacity.");
                    return;
                }
                if (pUndo)
                    pUndo->AddChanges( *this, rData.nStart, rData.nLen, rData.aOffsets );
                ReplaceTextOnly( rData.nStart, rData.nLen, rData.sChanged, rData.aOffsets );
            }
        }
    }
}

void SwTextNode::ReplaceTextOnly( sal_Int32 nPos, sal_Int32 nLen,
                                const OUString & rText,
                                const Sequence<sal_Int32>& rOffsets )
{
    assert(rText.getLength() - nLen <= GetSpaceLeft());

    m_Text = m_Text.replaceAt(nPos, nLen, rText);

    sal_Int32 nTLen = rText.getLength();
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    // now look for no 1-1 mapping -> move the indizies!
    sal_Int32 nMyOff = nPos;
    for( sal_Int32 nI = 0; nI < nTLen; ++nI )
    {
        const sal_Int32 nOff = pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            // something is inserted
            sal_Int32 nCnt = 1;
            while( nI + nCnt < nTLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt );
            nMyOff = nOff;
            //nMyOff -= nCnt;
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            // something is deleted
            Update( SwIndex( this, nMyOff+1 ), nOff - nMyOff, true );
            nMyOff = nOff;
        }
        ++nMyOff;
    }
    if( nMyOff < nLen )
        // something is deleted at the end
        Update( SwIndex( this, nMyOff ), nLen - nMyOff, true );

    // notify the layout!
    SwDelText aDelHint( nPos, nTLen );
    NotifyClients( nullptr, &aDelHint );

    SwInsText aHint( nPos, nTLen );
    NotifyClients( nullptr, &aHint );
}

// the return values allows us to see if we did the heavy-
// lifting required to actually break and count the words.
bool SwTextNode::CountWords( SwDocStat& rStat,
                            sal_Int32 nStt, sal_Int32 nEnd ) const
{
    if( nStt > nEnd )
    {   // bad call
        return false;
    }
    if (IsInRedlines())
    {   //not counting txtnodes used to hold deleted redline content
        return false;
    }
    bool bCountAll = ( (0 == nStt) && (GetText().getLength() == nEnd) );
    ++rStat.nAllPara; // #i93174#: count _all_ paragraphs
    if ( IsHidden() )
    {   // not counting hidden paras
        return false;
    }
    // count words in numbering string if started at beginning of para:
    bool bCountNumbering = nStt == 0;
    bool bHasBullet = false, bHasNumbering = false;
    OUString sNumString;
    if (bCountNumbering)
    {
        sNumString = GetNumString();
        bHasNumbering = !sNumString.isEmpty();
        if (!bHasNumbering)
            bHasBullet = HasBullet();
        bCountNumbering = bHasNumbering || bHasBullet;
    }

    if( nStt == nEnd && !bCountNumbering)
    {   // unnumbered empty node or empty selection
        return false;
    }

    // count of non-empty paras
    ++rStat.nPara;

    // Shortcut when counting whole paragraph and current count is clean
    if ( bCountAll && !IsWordCountDirty() )
    {
        // accumulate into DocStat record to return the values
        rStat.nWord += GetParaNumberOfWords();
        rStat.nAsianWord += GetParaNumberOfAsianWords();
        rStat.nChar += GetParaNumberOfChars();
        rStat.nCharExcludingSpaces += GetParaNumberOfCharsExcludingSpaces();
        return false;
    }

    // ConversionMap to expand fields, remove invisible and redline deleted text for scanner
    const ModelToViewHelper aConversionMap(*this, ExpandMode::ExpandFields | ExpandMode::ExpandFootnote | ExpandMode::HideInvisible | ExpandMode::HideDeletions);
    OUString aExpandText = aConversionMap.getViewText();

    if (aExpandText.isEmpty() && !bCountNumbering)
    {
        return false;
    }

    // map start and end points onto the ConversionMap
    const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nStt );
    const sal_Int32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nEnd );

    //do the count
    // all counts exclude hidden paras and hidden+redlined within para
    // definition of space/white chars in SwScanner (and BreakIter!)
    // uses both u_isspace and BreakIter getWordBoundary in SwScanner
    sal_uInt32 nTmpWords = 0;        // count of all words
    sal_uInt32 nTmpAsianWords = 0;   //count of all Asian codepoints
    sal_uInt32 nTmpChars = 0;        // count of all chars
    sal_uInt32 nTmpCharsExcludingSpaces = 0;  // all non-white chars

    // count words in masked and expanded text:
    if (!aExpandText.isEmpty())
    {
        if (g_pBreakIt->GetBreakIter().is())
        {
            // zero is NULL for pLanguage -----------v               last param = true for clipping
            SwScanner aScanner( *this, aExpandText, nullptr, aConversionMap, i18n::WordType::WORD_COUNT,
                                nExpandBegin, nExpandEnd, true );

            // used to filter out scanner returning almost empty strings (len=1; unichar=0x0001)
            const OUString aBreakWord( CH_TXTATR_BREAKWORD );

            while ( aScanner.NextWord() )
            {
                if( !aExpandText.match(aBreakWord, aScanner.GetBegin() ))
                {
                    ++nTmpWords;
                    const OUString &rWord = aScanner.GetWord();
                    if (g_pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                        ++nTmpAsianWords;
                    nTmpCharsExcludingSpaces += g_pBreakIt->getGraphemeCount(rWord);
                }
            }

            nTmpCharsExcludingSpaces += aScanner.getOverriddenDashCount();
        }

        nTmpChars = g_pBreakIt->getGraphemeCount(aExpandText, nExpandBegin, nExpandEnd);
    }

    // no nTmpCharsExcludingSpaces adjust needed neither for blanked out MaskedChars
    // nor for mid-word selection - set scanner bClip = true at creation

    // count outline number label - ? no expansion into map
    // always counts all of number-ish label
    if (bHasNumbering) // count words in numbering string
    {
        LanguageType aLanguage = GetLang( 0 );

        SwScanner aScanner( *this, sNumString, &aLanguage, ModelToViewHelper(),
                            i18n::WordType::WORD_COUNT, 0, sNumString.getLength(), true );

        while ( aScanner.NextWord() )
        {
            ++nTmpWords;
            const OUString &rWord = aScanner.GetWord();
            if (g_pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                ++nTmpAsianWords;
            nTmpCharsExcludingSpaces += g_pBreakIt->getGraphemeCount(rWord);
        }

        nTmpCharsExcludingSpaces += aScanner.getOverriddenDashCount();
        nTmpChars += g_pBreakIt->getGraphemeCount(sNumString);
    }
    else if ( bHasBullet )
    {
        ++nTmpWords;
        ++nTmpChars;
        ++nTmpCharsExcludingSpaces;
    }

    // If counting the whole para then update cached values and mark clean
    if ( bCountAll )
    {
        SetParaNumberOfWords( nTmpWords );
        SetParaNumberOfAsianWords( nTmpAsianWords );
        SetParaNumberOfChars( nTmpChars );
        SetParaNumberOfCharsExcludingSpaces( nTmpCharsExcludingSpaces );
        SetWordCountDirty( false );
    }
    // accumulate into DocStat record to return the values
    rStat.nWord += nTmpWords;
    rStat.nAsianWord += nTmpAsianWords;
    rStat.nChar += nTmpChars;
    rStat.nCharExcludingSpaces += nTmpCharsExcludingSpaces;

    return true;
}

// Paragraph statistics start -->

struct SwParaIdleData_Impl
{
    SwWrongList* pWrong;                // for spell checking
    SwGrammarMarkUp* pGrammarCheck;     // for grammar checking /  proof reading
    SwWrongList* pSmartTags;
    sal_uLong nNumberOfWords;
    sal_uLong nNumberOfAsianWords;
    sal_uLong nNumberOfChars;
    sal_uLong nNumberOfCharsExcludingSpaces;
    bool bWordCountDirty;
    SwTextNode::WrongState eWrongDirty; ///< online spell checking needed/done?
    bool bGrammarCheckDirty;
    bool bSmartTagDirty;
    bool bAutoComplDirty;               // die ACompl-Liste muss angepasst werden

    SwParaIdleData_Impl() :
        pWrong              ( nullptr ),
        pGrammarCheck       ( nullptr ),
        pSmartTags          ( nullptr ),
        nNumberOfWords      ( 0 ),
        nNumberOfAsianWords ( 0 ),
        nNumberOfChars      ( 0 ),
        nNumberOfCharsExcludingSpaces ( 0 ),
        bWordCountDirty     ( true ),
        eWrongDirty         ( SwTextNode::WrongState::TODO ),
        bGrammarCheckDirty  ( true ),
        bSmartTagDirty      ( true ),
        bAutoComplDirty     ( true ) {};
};

void SwTextNode::InitSwParaStatistics( bool bNew )
{
    if ( bNew )
    {
        m_pParaIdleData_Impl = new SwParaIdleData_Impl;
    }
    else if ( m_pParaIdleData_Impl )
    {
        delete m_pParaIdleData_Impl->pWrong;
        delete m_pParaIdleData_Impl->pGrammarCheck;
        delete m_pParaIdleData_Impl->pSmartTags;
        delete m_pParaIdleData_Impl;
        m_pParaIdleData_Impl = nullptr;
    }
}

void SwTextNode::SetWrong( SwWrongList* pNew, bool bDelete )
{
    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pWrong;
        }
        m_pParaIdleData_Impl->pWrong = pNew;
    }
}

SwWrongList* SwTextNode::GetWrong()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pWrong : nullptr;
}

// #i71360#
const SwWrongList* SwTextNode::GetWrong() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pWrong : nullptr;
}

void SwTextNode::SetGrammarCheck( SwGrammarMarkUp* pNew, bool bDelete )
{
    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pGrammarCheck;
        }
        m_pParaIdleData_Impl->pGrammarCheck = pNew;
    }
}

SwGrammarMarkUp* SwTextNode::GetGrammarCheck()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pGrammarCheck : nullptr;
}

void SwTextNode::SetSmartTags( SwWrongList* pNew, bool bDelete )
{
    OSL_ENSURE( !pNew || SwSmartTagMgr::Get().IsSmartTagsEnabled(),
            "Weird - we have a smart tag list without any recognizers?" );

    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pSmartTags;
        }
        m_pParaIdleData_Impl->pSmartTags = pNew;
    }
}

SwWrongList* SwTextNode::GetSmartTags()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pSmartTags : nullptr;
}

void SwTextNode::SetParaNumberOfWords( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfWords = nNew;
    }
}

sal_uLong SwTextNode::GetParaNumberOfWords() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfWords : 0;
}

void SwTextNode::SetParaNumberOfAsianWords( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfAsianWords = nNew;
    }
}

sal_uLong SwTextNode::GetParaNumberOfAsianWords() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfAsianWords : 0;
}

void SwTextNode::SetParaNumberOfChars( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfChars = nNew;
    }
}

sal_uLong SwTextNode::GetParaNumberOfChars() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfChars : 0;
}

void SwTextNode::SetWordCountDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bWordCountDirty = bNew;
    }
}

sal_uLong SwTextNode::GetParaNumberOfCharsExcludingSpaces() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfCharsExcludingSpaces : 0;
}

void SwTextNode::SetParaNumberOfCharsExcludingSpaces( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfCharsExcludingSpaces = nNew;
    }
}

bool SwTextNode::IsWordCountDirty() const
{
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bWordCountDirty;
}

void SwTextNode::SetWrongDirty(WrongState eNew) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->eWrongDirty = eNew;
    }
}

auto SwTextNode::GetWrongDirty() const -> WrongState
{
    return (m_pParaIdleData_Impl) ? m_pParaIdleData_Impl->eWrongDirty : WrongState::DONE;
}

bool SwTextNode::IsWrongDirty() const
{
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->eWrongDirty != WrongState::DONE;
}

void SwTextNode::SetGrammarCheckDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bGrammarCheckDirty = bNew;
    }
}

bool SwTextNode::IsGrammarCheckDirty() const
{
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bGrammarCheckDirty;
}

void SwTextNode::SetSmartTagDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bSmartTagDirty = bNew;
    }
}

bool SwTextNode::IsSmartTagDirty() const
{
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bSmartTagDirty;
}

void SwTextNode::SetAutoCompleteWordDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bAutoComplDirty = bNew;
    }
}

bool SwTextNode::IsAutoCompleteWordDirty() const
{
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bAutoComplDirty;
}

// <-- Paragraph statistics end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
