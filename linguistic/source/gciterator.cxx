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

#include <sal/macros.h>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/SingleProofreadingError.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/text/TextMarkupDescriptor.hpp>
#include <com/sun/star/text/XTextMarkup.hpp>
#include <com/sun/star/text/XMultiTextMarkup.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <sal/config.h>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/factory.hxx>
#include <i18npool/mslangid.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>

#include <deque>
#include <map>
#include <vector>

#include "linguistic/misc.hxx"
#include "defs.hxx"
#include "lngopt.hxx"

#include "gciterator.hxx"

using ::rtl::OUString;
using namespace linguistic;
using namespace ::com::sun::star;

// forward declarations
static ::rtl::OUString GrammarCheckingIterator_getImplementationName() throw();
static uno::Sequence< OUString > GrammarCheckingIterator_getSupportedServiceNames() throw();



// white space list: obtained from the fonts.config.txt of a Linux system.
static sal_Unicode aWhiteSpaces[] =
{
    0x0020,   /* SPACE */
    0x00a0,   /* NO-BREAK SPACE */
    0x00ad,   /* SOFT HYPHEN */
    0x115f,   /* HANGUL CHOSEONG FILLER */
    0x1160,   /* HANGUL JUNGSEONG FILLER */
    0x1680,   /* OGHAM SPACE MARK */
    0x2000,   /* EN QUAD */
    0x2001,   /* EM QUAD */
    0x2002,   /* EN SPACE */
    0x2003,   /* EM SPACE */
    0x2004,   /* THREE-PER-EM SPACE */
    0x2005,   /* FOUR-PER-EM SPACE */
    0x2006,   /* SIX-PER-EM SPACE */
    0x2007,   /* FIGURE SPACE */
    0x2008,   /* PUNCTUATION SPACE */
    0x2009,   /* THIN SPACE */
    0x200a,   /* HAIR SPACE */
    0x200b,   /* ZERO WIDTH SPACE */
    0x200c,   /* ZERO WIDTH NON-JOINER */
    0x200d,   /* ZERO WIDTH JOINER */
    0x200e,   /* LEFT-TO-RIGHT MARK */
    0x200f,   /* RIGHT-TO-LEFT MARK */
    0x2028,   /* LINE SEPARATOR */
    0x2029,   /* PARAGRAPH SEPARATOR */
    0x202a,   /* LEFT-TO-RIGHT EMBEDDING */
    0x202b,   /* RIGHT-TO-LEFT EMBEDDING */
    0x202c,   /* POP DIRECTIONAL FORMATTING */
    0x202d,   /* LEFT-TO-RIGHT OVERRIDE */
    0x202e,   /* RIGHT-TO-LEFT OVERRIDE */
    0x202f,   /* NARROW NO-BREAK SPACE */
    0x205f,   /* MEDIUM MATHEMATICAL SPACE */
    0x2060,   /* WORD JOINER */
    0x2061,   /* FUNCTION APPLICATION */
    0x2062,   /* INVISIBLE TIMES */
    0x2063,   /* INVISIBLE SEPARATOR */
    0x206A,   /* INHIBIT SYMMETRIC SWAPPING */
    0x206B,   /* ACTIVATE SYMMETRIC SWAPPING */
    0x206C,   /* INHIBIT ARABIC FORM SHAPING */
    0x206D,   /* ACTIVATE ARABIC FORM SHAPING */
    0x206E,   /* NATIONAL DIGIT SHAPES */
    0x206F,   /* NOMINAL DIGIT SHAPES */
    0x3000,   /* IDEOGRAPHIC SPACE */
    0x3164,   /* HANGUL FILLER */
    0xfeff,   /* ZERO WIDTH NO-BREAK SPACE */
    0xffa0,   /* HALFWIDTH HANGUL FILLER */
    0xfff9,   /* INTERLINEAR ANNOTATION ANCHOR */
    0xfffa,   /* INTERLINEAR ANNOTATION SEPARATOR */
    0xfffb    /* INTERLINEAR ANNOTATION TERMINATOR */
};

static int nWhiteSpaces = sizeof( aWhiteSpaces ) / sizeof( aWhiteSpaces[0] );

static bool lcl_IsWhiteSpace( sal_Unicode cChar )
{
    bool bFound = false;
    for (int i = 0;  i < nWhiteSpaces && !bFound;  ++i)
    {
        if (cChar == aWhiteSpaces[i])
            bFound = true;
    }
    return bFound;
}

static sal_Int32 lcl_SkipWhiteSpaces( const OUString &rText, sal_Int32 nStartPos )
{
    // note having nStartPos point right behind the string is OK since that one
    // is a correct end-of-sentence position to be returned from a grammar checker...

    const sal_Int32 nLen = rText.getLength();
    bool bIllegalArgument = false;
    if (nStartPos < 0)
    {
        bIllegalArgument = true;
        nStartPos = 0;
    }
    if (nStartPos > nLen)
    {
        bIllegalArgument = true;
        nStartPos = nLen;
    }
    if (bIllegalArgument)
    {
        DBG_ASSERT( 0, "lcl_SkipWhiteSpaces: illegal arguments" );
    }

    sal_Int32 nRes = nStartPos;
    if (0 <= nStartPos && nStartPos < nLen)
    {
        const sal_Unicode *pText = rText.getStr() + nStartPos;
        while (nStartPos < nLen && lcl_IsWhiteSpace( *pText ))
            ++pText;
        nRes = pText - rText.getStr();
    }

    DBG_ASSERT( 0 <= nRes && nRes <= nLen, "lcl_SkipWhiteSpaces return value out of range" );
    return nRes;
}

static sal_Int32 lcl_BacktraceWhiteSpaces( const OUString &rText, sal_Int32 nStartPos )
{
    // note: having nStartPos point right behind the string is OK since that one
    // is a correct end-of-sentence position to be returned from a grammar checker...

    const sal_Int32 nLen = rText.getLength();
    bool bIllegalArgument = false;
    if (nStartPos < 0)
    {
        bIllegalArgument = true;
        nStartPos = 0;
    }
    if (nStartPos > nLen)
    {
        bIllegalArgument = true;
        nStartPos = nLen;
    }
    if (bIllegalArgument)
    {
        DBG_ASSERT( 0, "lcl_BacktraceWhiteSpaces: illegal arguments" );
    }

    sal_Int32 nRes = nStartPos;
    sal_Int32 nPosBefore = nStartPos - 1;
    const sal_Unicode *pStart = rText.getStr();
    if (0 <= nPosBefore && nPosBefore < nLen && lcl_IsWhiteSpace( pStart[ nPosBefore ] ))
    {
        nStartPos = nPosBefore;
        if (0 <= nStartPos && nStartPos < nLen)
        {
            const sal_Unicode *pText = rText.getStr() + nStartPos;
            while (pText > pStart && lcl_IsWhiteSpace( *pText ))
                --pText;
            // now add 1 since we want to point to the first char after the last char in the sentence...
            nRes = pText - pStart + 1;
        }
    }

    DBG_ASSERT( 0 <= nRes && nRes <= nLen, "lcl_BacktraceWhiteSpaces return value out of range" );
    return nRes;
}


extern "C" void workerfunc (void * gci)
{
    ((GrammarCheckingIterator*)gci)->DequeueAndCheck();
}

static lang::Locale lcl_GetPrimaryLanguageOfSentence(
    uno::Reference< text::XFlatParagraph > xFlatPara,
    sal_Int32 nStartIndex )
{
    //get the language of the first word
    return xFlatPara->getLanguageOfText( nStartIndex, 1 );
}


GrammarCheckingIterator::GrammarCheckingIterator( const uno::Reference< lang::XMultiServiceFactory > & rxMgr ) :
    m_xMSF( rxMgr ),
    m_bEnd( sal_False ),
    m_aCurCheckedDocId(),
    m_bGCServicesChecked( sal_False ),
    m_nDocIdCounter( 0 ),
    m_nLastEndOfSentencePos( -1 ),
    m_aEventListeners( MyMutex::get() ),
    m_aNotifyListeners( MyMutex::get() )
{
    m_thread = osl_createThread( workerfunc, this );
}


GrammarCheckingIterator::~GrammarCheckingIterator()
{
    TerminateThread();
}

void GrammarCheckingIterator::TerminateThread()
{
    oslThread t;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
        t = m_thread;
        m_thread = 0;
        m_bEnd = sal_True;
        m_aWakeUpThread.set();
    }
    if (t != 0)
    {
        osl_joinWithThread(t);
        osl_destroyThread(t);
    }
}

sal_Int32 GrammarCheckingIterator::NextDocId()
{
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
    m_nDocIdCounter += 1;
    return m_nDocIdCounter;
}


OUString GrammarCheckingIterator::GetOrCreateDocId(
    const uno::Reference< lang::XComponent > &xComponent )
{
    // internal method; will always be called with locked mutex

    OUString aRes;
    if (xComponent.is())
    {
        if (m_aDocIdMap.find( xComponent.get() ) != m_aDocIdMap.end())
        {
            // return already existing entry
            aRes = m_aDocIdMap[ xComponent.get() ];
        }
        else // add new entry
        {
            sal_Int32 nRes = NextDocId();
            aRes = OUString::valueOf( nRes );
            m_aDocIdMap[ xComponent.get() ] = aRes;
            xComponent->addEventListener( this );
        }
    }
    return aRes;
}


void GrammarCheckingIterator::AddEntry(
    uno::WeakReference< text::XFlatParagraphIterator > xFlatParaIterator,
    uno::WeakReference< text::XFlatParagraph > xFlatPara,
    const OUString & rDocId,
    sal_Int32 nStartIndex,
    sal_Bool bAutomatic )
{
    // we may not need/have a xFlatParaIterator (e.g. if checkGrammarAtPos was called)
    // but we always need a xFlatPara...
    uno::Reference< text::XFlatParagraph > xPara( xFlatPara );
    if (xPara.is())
    {
        FPEntry aNewFPEntry;
        aNewFPEntry.m_xParaIterator = xFlatParaIterator;
        aNewFPEntry.m_xPara         = xFlatPara;
        aNewFPEntry.m_aDocId        = rDocId;
        aNewFPEntry.m_nStartIndex   = nStartIndex;
        aNewFPEntry.m_bAutomatic    = bAutomatic;

        // add new entry to the end of this queue
        ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
        m_aFPEntriesQueue.push_back( aNewFPEntry );

        // wake up the thread in order to do grammar checking
        m_aWakeUpThread.set();
    }
}


void GrammarCheckingIterator::ProcessResult(
    const linguistic2::ProofreadingResult &rRes,
    const uno::Reference< text::XFlatParagraphIterator > &rxFlatParagraphIterator,
    bool bIsAutomaticChecking )
{
    DBG_ASSERT( rRes.xFlatParagraph.is(), "xFlatParagraph is missing" );
     //no guard necessary as no members are used
    sal_Bool bContinueWithNextPara = sal_False;
    if (!rRes.xFlatParagraph.is() || rRes.xFlatParagraph->isModified())
    {
        // if paragraph was modified/deleted meanwhile continue with the next one...
        bContinueWithNextPara = sal_True;
    }
    else    // paragraph is still unchanged...
    {
        // mark found errors...

        sal_Int32 nTextLen = rRes.aText.getLength();
        bool bBoundariesOk = 0 <= rRes.nStartOfSentencePosition     && rRes.nStartOfSentencePosition <= nTextLen &&
                             0 <= rRes.nBehindEndOfSentencePosition && rRes.nBehindEndOfSentencePosition <= nTextLen &&
                             0 <= rRes.nStartOfNextSentencePosition && rRes.nStartOfNextSentencePosition <= nTextLen &&
                             rRes.nStartOfSentencePosition      <= rRes.nBehindEndOfSentencePosition &&
                             rRes.nBehindEndOfSentencePosition  <= rRes.nStartOfNextSentencePosition;
        (void) bBoundariesOk;
        DBG_ASSERT( bBoundariesOk, "inconsistent sentence boundaries" );
        uno::Sequence< linguistic2::SingleProofreadingError > aErrors = rRes.aErrors;

        uno::Reference< text::XMultiTextMarkup > xMulti( rRes.xFlatParagraph, uno::UNO_QUERY );
        if (xMulti.is())    // use new API for markups
        {
            try
            {
                // length = number of found errors + 1 sentence markup
                sal_Int32 nErrors = rRes.aErrors.getLength();
                uno::Sequence< text::TextMarkupDescriptor > aDescriptors( nErrors + 1 );
                text::TextMarkupDescriptor * pDescriptors = aDescriptors.getArray();

                // at pos 0 .. nErrors-1 -> all grammar errors
                for (sal_Int32 i = 0;  i < nErrors;  ++i)
                {
                    const linguistic2::SingleProofreadingError &rError = rRes.aErrors[i];
                    text::TextMarkupDescriptor &rDesc = aDescriptors[i];

                    rDesc.nType   = rError.nErrorType;
                    rDesc.nOffset = rError.nErrorStart;
                    rDesc.nLength = rError.nErrorLength;

                    // the proofreader may return SPELLING but right now our core
                    // does only handle PROOFREADING if the result is from the proofreader...
                    // (later on we may wish to color spelling errors found by the proofreader
                    // differently for example. But no special handling right now.
                    if (rDesc.nType == text::TextMarkupType::SPELLCHECK)
                        rDesc.nType = text::TextMarkupType::PROOFREADING;
                }

                // at pos nErrors -> sentence markup
                // nSentenceLength: includes the white-spaces following the sentence end...
                const sal_Int32 nSentenceLength = rRes.nStartOfNextSentencePosition - rRes.nStartOfSentencePosition;
                pDescriptors[ nErrors ].nType   = text::TextMarkupType::SENTENCE;
                pDescriptors[ nErrors ].nOffset = rRes.nStartOfSentencePosition;
                pDescriptors[ nErrors ].nLength = nSentenceLength;

                xMulti->commitMultiTextMarkup( aDescriptors ) ;
            }
            catch (lang::IllegalArgumentException &)
            {
                OSL_FAIL( "commitMultiTextMarkup: IllegalArgumentException exception caught" );
            }
        }

        // other sentences left to be checked in this paragraph?
        if (rRes.nStartOfNextSentencePosition < rRes.aText.getLength())
        {
            AddEntry( rxFlatParagraphIterator, rRes.xFlatParagraph, rRes.aDocumentIdentifier, rRes.nStartOfNextSentencePosition, bIsAutomaticChecking );
        }
        else    // current paragraph finished
        {
            // set "already checked" flag for the current flat paragraph
            if (rRes.xFlatParagraph.is())
                rRes.xFlatParagraph->setChecked( text::TextMarkupType::PROOFREADING, true );

            bContinueWithNextPara = sal_True;
        }
    }

    if (bContinueWithNextPara)
    {
        // we need to continue with the next paragraph
        uno::Reference< text::XFlatParagraph > xFlatParaNext;
        if (rxFlatParagraphIterator.is())
            xFlatParaNext = rxFlatParagraphIterator->getNextPara();
        {
            AddEntry( rxFlatParagraphIterator, xFlatParaNext, rRes.aDocumentIdentifier, 0, bIsAutomaticChecking );
        }
    }
}


uno::Reference< linguistic2::XProofreader > GrammarCheckingIterator::GetGrammarChecker(
    const lang::Locale &rLocale )
{
    (void) rLocale;
    uno::Reference< linguistic2::XProofreader > xRes;

    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );

    // check supported locales for each grammarchecker if not already done
    if (!m_bGCServicesChecked)
    {
        GetConfiguredGCSvcs_Impl();
        m_bGCServicesChecked = sal_True;
    }

    const LanguageType nLang = MsLangId::convertLocaleToLanguage( rLocale );
    GCImplNames_t::const_iterator aLangIt( m_aGCImplNamesByLang.find( nLang ) );
    if (aLangIt != m_aGCImplNamesByLang.end())  // matching configured language found?
    {
        OUString aSvcImplName( aLangIt->second );
        GCReferences_t::const_iterator aImplNameIt( m_aGCReferencesByService.find( aSvcImplName ) );
        if (aImplNameIt != m_aGCReferencesByService.end())  // matching impl name found?
        {
            xRes = aImplNameIt->second;
        }
        else    // the service is to be instatiated here for the first time...
        {
            try
            {
                uno::Reference< lang::XMultiServiceFactory > xMgr(
                        comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                uno::Reference< linguistic2::XProofreader > xGC(
                        xMgr->createInstance( aSvcImplName ), uno::UNO_QUERY_THROW );
                uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xGC, uno::UNO_QUERY_THROW );

                if (xSuppLoc->hasLocale( rLocale ))
                {
                    m_aGCReferencesByService[ aSvcImplName ] = xGC;
                    xRes = xGC;

                    uno::Reference< linguistic2::XLinguServiceEventBroadcaster > xBC( xGC, uno::UNO_QUERY );
                    if (xBC.is())
                        xBC->addLinguServiceEventListener( this );
                }
                else
                {
                    DBG_ASSERT( 0, "grammar checker does not support required locale" );
                }
            }
            catch (uno::Exception &)
            {
                DBG_ASSERT( 0, "instantiating grammar checker failed" );
            }
        }
    }
    // ---- THREAD SAFE END ----

    return xRes;
}


void GrammarCheckingIterator::DequeueAndCheck()
{
    uno::Sequence< sal_Int32 >      aLangPortions;
    uno::Sequence< lang::Locale >   aLangPortionsLocale;

    for (;;)
    {
        // ---- THREAD SAFE START ----
        bool bQueueEmpty = false;
        {
            ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
            if (m_bEnd)
            {
                break;
            }
            bQueueEmpty = m_aFPEntriesQueue.empty();
        }
        // ---- THREAD SAFE END ----

        if (!bQueueEmpty)
        {
            uno::Reference< text::XFlatParagraphIterator > xFPIterator;
            uno::Reference< text::XFlatParagraph > xFlatPara;
            FPEntry aFPEntryItem;
            OUString aCurDocId;
            sal_Bool bModified = sal_False;
            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
                aFPEntryItem        = m_aFPEntriesQueue.front();
                xFPIterator         = aFPEntryItem.m_xParaIterator;
                xFlatPara           = aFPEntryItem.m_xPara;
                m_aCurCheckedDocId  = aFPEntryItem.m_aDocId;
                aCurDocId = m_aCurCheckedDocId;

                m_aFPEntriesQueue.pop_front();
            }
            // ---- THREAD SAFE END ----

            if (xFlatPara.is() && xFPIterator.is())
            {
                OUString aCurTxt( xFlatPara->getText() );
                lang::Locale aCurLocale = lcl_GetPrimaryLanguageOfSentence( xFlatPara, aFPEntryItem.m_nStartIndex );

                bModified = xFlatPara->isModified();
                if (!bModified)
                {
                    // ---- THREAD SAFE START ----
                    ::osl::ClearableGuard< ::osl::Mutex > aGuard( MyMutex::get() );

                    sal_Int32 nStartPos = aFPEntryItem.m_nStartIndex;
                    sal_Int32 nSuggestedEnd = GetSuggestedEndOfSentence( aCurTxt, nStartPos, aCurLocale );
                    DBG_ASSERT( (nSuggestedEnd == 0 && aCurTxt.isEmpty()) || nSuggestedEnd > nStartPos,
                            "nSuggestedEndOfSentencePos calculation failed?" );

                    linguistic2::ProofreadingResult aRes;

                    uno::Reference< linguistic2::XProofreader > xGC( GetGrammarChecker( aCurLocale ), uno::UNO_QUERY );
                    if (xGC.is())
                    {
                        aGuard.clear();
                        uno::Sequence< beans::PropertyValue > aEmptyProps;
                        aRes = xGC->doProofreading( aCurDocId, aCurTxt, aCurLocale, nStartPos, nSuggestedEnd, aEmptyProps );

                        //!! work-around to prevent looping if the grammar checker
                        //!! failed to properly identify the sentence end
                        if (
                            aRes.nBehindEndOfSentencePosition <= nStartPos &&
                            aRes.nBehindEndOfSentencePosition != nSuggestedEnd
                           )
                        {
                            DBG_ASSERT( 0, "!! Grammarchecker failed to provide end of sentence !!" );
                            aRes.nBehindEndOfSentencePosition = nSuggestedEnd;
                        }

                        aRes.xFlatParagraph      = xFlatPara;
                        aRes.nStartOfSentencePosition = nStartPos;
                    }
                    else
                    {
                        // no grammar checker -> no error
                        // but we need to provide the data below in order to continue with the next sentence
                        aRes.aDocumentIdentifier = aCurDocId;
                        aRes.xFlatParagraph      = xFlatPara;
                        aRes.aText               = aCurTxt;
                        aRes.aLocale             = aCurLocale;
                        aRes.nStartOfSentencePosition       = nStartPos;
                        aRes.nBehindEndOfSentencePosition   = nSuggestedEnd;
                    }
                    aRes.nStartOfNextSentencePosition = lcl_SkipWhiteSpaces( aCurTxt, aRes.nBehindEndOfSentencePosition );
                    aRes.nBehindEndOfSentencePosition = lcl_BacktraceWhiteSpaces( aCurTxt, aRes.nStartOfNextSentencePosition );

                    //guard has to be cleared as ProcessResult calls out of this class
                    aGuard.clear();
                    ProcessResult( aRes, xFPIterator, aFPEntryItem.m_bAutomatic );
                    // ---- THREAD SAFE END ----
                }
                else
                {
                    // the paragraph changed meanwhile... (and maybe is still edited)
                    // thus we simply continue to ask for the next to be checked.
                    uno::Reference< text::XFlatParagraph > xFlatParaNext( xFPIterator->getNextPara() );
                    AddEntry( xFPIterator, xFlatParaNext, aCurDocId, 0, aFPEntryItem.m_bAutomatic );
                }
            }

            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
                m_aCurCheckedDocId  = OUString();
            }
            // ---- THREAD SAFE END ----
        }
        else
        {
            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
                if (m_bEnd)
                {
                    break;
                }
                // Check queue state again
                if (m_aFPEntriesQueue.empty())
                    m_aWakeUpThread.reset();
            }
            // ---- THREAD SAFE END ----

            //if the queue is empty
            // IMPORTANT: Don't call condition.wait() with locked
            // mutex. Otherwise you would keep out other threads
            // to add entries to the queue! A condition is thread-
            // safe implemented.
            m_aWakeUpThread.wait();
        }
    }
}


void SAL_CALL GrammarCheckingIterator::startProofreading(
    const uno::Reference< ::uno::XInterface > & xDoc,
    const uno::Reference< text::XFlatParagraphIteratorProvider > & xIteratorProvider )
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    // get paragraph to start checking with
    const bool bAutomatic = true;
    uno::Reference<text::XFlatParagraphIterator> xFPIterator = xIteratorProvider->getFlatParagraphIterator(
            text::TextMarkupType::PROOFREADING, bAutomatic );
    uno::Reference< text::XFlatParagraph > xPara( xFPIterator.is()? xFPIterator->getFirstPara() : NULL );
    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );

    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
    if (xPara.is() && xComponent.is())
    {
        OUString aDocId = GetOrCreateDocId( xComponent );

        // create new entry and add it to queue
        AddEntry( xFPIterator, xPara, aDocId, 0, bAutomatic );
    }
    // ---- THREAD SAFE END ----
}


linguistic2::ProofreadingResult SAL_CALL GrammarCheckingIterator::checkSentenceAtPosition(
    const uno::Reference< uno::XInterface >& xDoc,
    const uno::Reference< text::XFlatParagraph >& xFlatPara,
    const OUString& rText,
    const lang::Locale& rLocale,
    sal_Int32 nStartOfSentencePos,
    sal_Int32 nSuggestedEndOfSentencePos,
    sal_Int32 nErrorPosInPara )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    (void) rLocale;

    // for the context menu...

    linguistic2::ProofreadingResult aRes;

    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );
    if (xFlatPara.is() && xComponent.is() &&
        ( nErrorPosInPara < 0 || nErrorPosInPara < rText.getLength()))
    {
        // iterate through paragraph until we find the sentence we are interested in
        linguistic2::ProofreadingResult aTmpRes;
        sal_Int32 nStartPos = nStartOfSentencePos >= 0 ? nStartOfSentencePos : 0;

        bool bFound = false;
        do
        {
            lang::Locale aCurLocale = lcl_GetPrimaryLanguageOfSentence( xFlatPara, nStartPos );
            sal_Int32 nOldStartOfSentencePos = nStartPos;
            uno::Reference< linguistic2::XProofreader > xGC;
            OUString aDocId;

            // ---- THREAD SAFE START ----
            {
                ::osl::ClearableGuard< ::osl::Mutex > aGuard( MyMutex::get() );
                aDocId = GetOrCreateDocId( xComponent );
                nSuggestedEndOfSentencePos = GetSuggestedEndOfSentence( rText, nStartPos, aCurLocale );
                DBG_ASSERT( nSuggestedEndOfSentencePos > nStartPos, "nSuggestedEndOfSentencePos calculation failed?" );

                xGC = GetGrammarChecker( aCurLocale );
            }
            // ---- THREAD SAFE START ----
            sal_Int32 nEndPos = -1;
            if (xGC.is())
            {
                uno::Sequence< beans::PropertyValue > aEmptyProps;
                aTmpRes = xGC->doProofreading( aDocId, rText, aCurLocale, nStartPos, nSuggestedEndOfSentencePos, aEmptyProps );

                //!! work-around to prevent looping if the grammar checker
                //!! failed to properly identify the sentence end
                if (aTmpRes.nBehindEndOfSentencePosition <= nStartPos)
                {
                    DBG_ASSERT( 0, "!! Grammarchecker failed to provide end of sentence !!" );
                    aTmpRes.nBehindEndOfSentencePosition = nSuggestedEndOfSentencePos;
                }

                aTmpRes.xFlatParagraph           = xFlatPara;
                aTmpRes.nStartOfSentencePosition = nStartPos;
                nEndPos = aTmpRes.nBehindEndOfSentencePosition;

                if ((nErrorPosInPara< 0 || nStartPos <= nErrorPosInPara) && nErrorPosInPara < nEndPos)
                    bFound = true;
            }
            if (nEndPos == -1) // no result from grammar checker
                nEndPos = nSuggestedEndOfSentencePos;
            nStartPos = lcl_SkipWhiteSpaces( rText, nEndPos );
            aTmpRes.nBehindEndOfSentencePosition = nEndPos;
            aTmpRes.nStartOfNextSentencePosition = nStartPos;
            aTmpRes.nBehindEndOfSentencePosition = lcl_BacktraceWhiteSpaces( rText, aTmpRes.nStartOfNextSentencePosition );

            // prevent endless loop by forcefully advancing if needs be...
            if (nStartPos <= nOldStartOfSentencePos)
            {
                DBG_ASSERT( 0, "end-of-sentence detection failed?" );
                nStartPos = nOldStartOfSentencePos + 1;
            }
        }
        while (!bFound && nStartPos < rText.getLength());

        if (bFound && !xFlatPara->isModified())
            aRes = aTmpRes;
    }

    return aRes;
}


sal_Int32 GrammarCheckingIterator::GetSuggestedEndOfSentence(
    const OUString &rText,
    sal_Int32 nSentenceStartPos,
    const lang::Locale &rLocale )
{
    // internal method; will always be called with locked mutex

    if (!m_xBreakIterator.is())
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        m_xBreakIterator = i18n::BreakIterator::create(xContext);
    }
    sal_Int32 nTextLen = rText.getLength();
    sal_Int32 nEndPosition;
    sal_Int32 nTmpStartPos = nSentenceStartPos;
    do
    {
        nEndPosition = nTextLen;
        if (nTmpStartPos < nTextLen)
            nEndPosition = m_xBreakIterator->endOfSentence( rText, nTmpStartPos, rLocale );
        if (nEndPosition < 0)
            nEndPosition = nTextLen;

        ++nTmpStartPos;
    }
    while (nEndPosition <= nSentenceStartPos && nEndPosition < nTextLen);
    if (nEndPosition > nTextLen)
        nEndPosition = nTextLen;
    return nEndPosition;
}


void SAL_CALL GrammarCheckingIterator::resetIgnoreRules(  )
throw (uno::RuntimeException)
{
    GCReferences_t::iterator aIt( m_aGCReferencesByService.begin() );
    while (aIt != m_aGCReferencesByService.end())
    {
        uno::Reference< linguistic2::XProofreader > xGC( aIt->second );
        if (xGC.is())
            xGC->resetIgnoreRules();
        ++aIt;
    }
}


sal_Bool SAL_CALL GrammarCheckingIterator::isProofreading(
    const uno::Reference< uno::XInterface >& xDoc )
throw (uno::RuntimeException)
{
    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );

    sal_Bool bRes = sal_False;

    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );
    if (xComponent.is())
    {
        // if the component was already used in one of the two calls to check text
        // i.e. in startGrammarChecking or checkGrammarAtPos it will be found in the
        // m_aDocIdMap unless the document already disposed.
        // If it is not found then it is not yet being checked (or requested to being checked)
        const DocMap_t::const_iterator aIt( m_aDocIdMap.find( xComponent.get() ) );
        if (aIt != m_aDocIdMap.end())
        {
            // check in document is checked automatically in the background...
            OUString aDocId = aIt->second;
            if (!m_aCurCheckedDocId.isEmpty() && m_aCurCheckedDocId == aDocId)
            {
                // an entry for that document was dequed and is currently being checked.
                bRes = sal_True;
            }
            else
            {
                // we need to check if there is an entry for that document in the queue...
                // That is the document is going to be checked sooner or later.

                sal_Int32 nSize = m_aFPEntriesQueue.size();
                for (sal_Int32 i = 0; i < nSize && !bRes; ++i)
                {
                    if (aDocId == m_aFPEntriesQueue[i].m_aDocId)
                        bRes = sal_True;
                }
            }
        }
    }
    // ---- THREAD SAFE END ----

    return bRes;
}


void SAL_CALL GrammarCheckingIterator::processLinguServiceEvent(
    const linguistic2::LinguServiceEvent& rLngSvcEvent )
throw (uno::RuntimeException)
{
    if (rLngSvcEvent.nEvent == linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN)
    {
        try
        {
             uno::Reference< uno::XInterface > xThis( dynamic_cast< XLinguServiceEventBroadcaster * >(this) );
             linguistic2::LinguServiceEvent aEvent( xThis, linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN );
             m_aNotifyListeners.notifyEach(
                    &linguistic2::XLinguServiceEventListener::processLinguServiceEvent,
                    aEvent);
        }
        catch (uno::RuntimeException &)
        {
             throw;
        }
        catch (const ::uno::Exception &rE)
        {
             (void) rE;
             // ignore
             DBG_WARNING1("processLinguServiceEvent: exception:\n%s",
                 OUStringToOString(rE.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}


sal_Bool SAL_CALL GrammarCheckingIterator::addLinguServiceEventListener(
    const uno::Reference< linguistic2::XLinguServiceEventListener >& xListener )
throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        m_aNotifyListeners.addInterface( xListener );
    }
    return sal_True;
}


sal_Bool SAL_CALL GrammarCheckingIterator::removeLinguServiceEventListener(
    const uno::Reference< linguistic2::XLinguServiceEventListener >& xListener )
throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        m_aNotifyListeners.removeInterface( xListener );
    }
    return sal_True;
}


void SAL_CALL GrammarCheckingIterator::dispose()
throw (uno::RuntimeException)
{
    lang::EventObject aEvt( (linguistic2::XProofreadingIterator *) this );
    m_aEventListeners.disposeAndClear( aEvt );

    TerminateThread();

    // ---- THREAD SAFE START ----
    {
        ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );

        // releaase all UNO references

        m_xMSF.clear();
        m_xBreakIterator.clear();

        // clear containers with UNO references AND have those references released
        GCReferences_t  aTmpEmpty1;
        DocMap_t        aTmpEmpty2;
        FPQueue_t       aTmpEmpty3;
        m_aGCReferencesByService.swap( aTmpEmpty1 );
        m_aDocIdMap.swap( aTmpEmpty2 );
        m_aFPEntriesQueue.swap( aTmpEmpty3 );
    }
    // ---- THREAD SAFE END ----
}


void SAL_CALL GrammarCheckingIterator::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        m_aEventListeners.addInterface( xListener );
    }
}


void SAL_CALL GrammarCheckingIterator::removeEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        m_aEventListeners.removeInterface( xListener );
    }
}


void SAL_CALL GrammarCheckingIterator::disposing( const lang::EventObject &rSource )
throw (uno::RuntimeException)
{
    // if the component (document) is disposing release all references
    //!! There is no need to remove entries from the queue that are from this document
    //!! since the respectives xFlatParagraphs should become invalid (isModified() == true)
    //!! and the call to xFlatParagraphIterator->getNextPara() will result in an empty reference.
    //!! And if an entry is currently checked by a grammar checker upon return the results
    //!! should be ignored.
    //!! Also GetOrCreateDocId will not use that very same Id again...
    //!! All of the above resulting in that we only have to get rid of the implementation pointer here.
    uno::Reference< lang::XComponent > xDoc( rSource.Source, uno::UNO_QUERY );
    if (xDoc.is())
    {
        // ---- THREAD SAFE START ----
        ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
        m_aDocIdMap.erase( xDoc.get() );
        // ---- THREAD SAFE END ----
    }
}


uno::Reference< util::XChangesBatch > GrammarCheckingIterator::GetUpdateAccess() const
{
    if (!m_xUpdateAccess.is())
    {
        try
        {
            // get configuration provider
            uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider =
                    configuration::theDefaultProvider::get( xContext );

            // get configuration update access
            beans::PropertyValue aValue;
            aValue.Name  = "nodepath";
            aValue.Value = uno::makeAny( ::rtl::OUString("org.openoffice.Office.Linguistic/ServiceManager") );
            uno::Sequence< uno::Any > aProps(1);
            aProps[0] <<= aValue;
            m_xUpdateAccess = uno::Reference< util::XChangesBatch >(
                    xConfigurationProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess", aProps ),
                        uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
    }

    return m_xUpdateAccess;
}


void GrammarCheckingIterator::GetConfiguredGCSvcs_Impl()
{
    GCImplNames_t   aTmpGCImplNamesByLang;

    try
    {
        // get node names (locale iso strings) for configured grammar checkers
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( "GrammarCheckerList" ), uno::UNO_QUERY_THROW );
        const uno::Sequence< OUString > aElementNames( xNA->getElementNames() );
        const OUString *pElementNames = aElementNames.getConstArray();

        sal_Int32 nLen = aElementNames.getLength();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aImplNames;
            uno::Any aTmp( xNA->getByName( pElementNames[i] ) );
            if (aTmp >>= aImplNames)
            {
                if (aImplNames.getLength() > 0)
                {
                    // only the first entry is used, there should be only one grammar checker per language
                    const OUString aImplName( aImplNames[0] );
                    const LanguageType nLang = MsLangId::convertIsoStringToLanguage( pElementNames[i] );
                    aTmpGCImplNamesByLang[ nLang ] = aImplName;
                }
            }
            else
            {
                DBG_ASSERT( 0, "failed to get aImplNames. Wrong type?" );
            }
        }
    }
    catch (uno::Exception &)
    {
        DBG_ASSERT( 0, "exception caught. Failed to get configured services" );
    }

    {
        // ---- THREAD SAFE START ----
        ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );
        m_aGCImplNamesByLang     = aTmpGCImplNamesByLang;
        // ---- THREAD SAFE END ----
    }
}




sal_Bool SAL_CALL GrammarCheckingIterator::supportsService(
    const OUString & rServiceName )
throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == rServiceName )
            return sal_True;
    return sal_False;
}


OUString SAL_CALL GrammarCheckingIterator::getImplementationName(  ) throw (uno::RuntimeException)
{
    return GrammarCheckingIterator_getImplementationName();
}


uno::Sequence< OUString > SAL_CALL GrammarCheckingIterator::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return GrammarCheckingIterator_getSupportedServiceNames();
}


void GrammarCheckingIterator::SetServiceList(
    const lang::Locale &rLocale,
    const uno::Sequence< OUString > &rSvcImplNames )
{
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );

    LanguageType nLanguage = LocaleToLanguage( rLocale );
    OUString aImplName;
    if (rSvcImplNames.getLength() > 0)
        aImplName = rSvcImplNames[0];   // there is only one grammar checker per language

    if (nLanguage != LANGUAGE_NONE && nLanguage != LANGUAGE_DONTKNOW)
    {
        if (!aImplName.isEmpty())
            m_aGCImplNamesByLang[ nLanguage ] = aImplName;
        else
            m_aGCImplNamesByLang.erase( nLanguage );
    }
}


uno::Sequence< OUString > GrammarCheckingIterator::GetServiceList(
    const lang::Locale &rLocale ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( MyMutex::get() );

    uno::Sequence< OUString > aRes(1);

    OUString aImplName;     // there is only one grammar checker per language
    LanguageType nLang  = LocaleToLanguage( rLocale );
    GCImplNames_t::const_iterator aIt( m_aGCImplNamesByLang.find( nLang ) );
    if (aIt != m_aGCImplNamesByLang.end())
        aImplName = aIt->second;

    if (!aImplName.isEmpty())
        aRes[0] = aImplName;
    else
        aRes.realloc(0);

    return aRes;
}


LinguDispatcher::DspType GrammarCheckingIterator::GetDspType() const
{
    return DSP_GRAMMAR;
}




static OUString GrammarCheckingIterator_getImplementationName() throw()
{
    return ::rtl::OUString( "com.sun.star.lingu2.ProofreadingIterator" );
}


static uno::Sequence< OUString > GrammarCheckingIterator_getSupportedServiceNames() throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = SN_GRAMMARCHECKINGITERATOR ;
    return aSNS;
}


static uno::Reference< uno::XInterface > SAL_CALL GrammarCheckingIterator_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & rxSMgr )
throw(uno::Exception)
{
    return static_cast< ::cppu::OWeakObject * >(new GrammarCheckingIterator( rxSMgr ));
}


void * SAL_CALL GrammarCheckingIterator_getFactory(
    const sal_Char *pImplName,
    lang::XMultiServiceFactory *pServiceManager,
    void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    if ( !GrammarCheckingIterator_getImplementationName().compareToAscii( pImplName ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                GrammarCheckingIterator_getImplementationName(),
                GrammarCheckingIterator_createInstance,
                GrammarCheckingIterator_getSupportedServiceNames());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
