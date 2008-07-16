/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gciterator.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_linguistic.hxx"

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingIterator.hpp>
#include <com/sun/star/linguistic2/SingleGrammarError.hpp>
#include <com/sun/star/linguistic2/GrammarCheckingResult.hpp>
#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/text/XTextMarkup.hpp>
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
#include <cppuhelper/extract.hxx>
#include <cppuhelper/factory.hxx>
#include <vcl/unohelp.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/processfactory.hxx>

#include <deque>
#include <map>
#include <vector>

#include "misc.hxx"
#include "defs.hxx"
#include "lngopt.hxx"

#include "gciterator.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;

// forward declarations
static ::rtl::OUString GrammarCheckingIterator_getImplementationName() throw();
static uno::Sequence< OUString > GrammarCheckingIterator_getSupportedServiceNames() throw();


//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////
/*
class MyThread : punlic osl::Thread
{
    void run ()
    {
        DequeueAndCheck();
    }

    void own_terminate ()
    {
        m_bEnd = true;
        wait (3000);
        terminate ();
    }
}

MyThread m_aQueue;

vois startGrammarChecking()
{
    if (!m_aQueue.isRunning ())
        m_aQueue.create ();
}

void stopGrammarChecking ()
{
    if (m_aQueue.isRunning ())
        m_aQueue.own_terminate ();
}
*/

GrammarCheckingIterator::GrammarCheckingIterator( const uno::Reference< lang::XMultiServiceFactory > & rxMgr ) :
    m_xMSF( rxMgr ),
    m_bEnd( sal_False ),
    m_bGCServicesChecked( sal_False ),
    m_nDocIdCounter( 0 ),
    m_nCurCheckedDocId( - 1 ),
    m_nLastEndOfSentencePos( -1 ),
    m_aMutex(),
    m_aEventListeners( m_aMutex ),
    m_aNotifyListeners( m_aMutex )
{
    osl_createThread( workerfunc, this );
}


GrammarCheckingIterator::~GrammarCheckingIterator()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
}


sal_Int32 GrammarCheckingIterator::NextDocId()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
    m_nDocIdCounter += 1;
    return m_nDocIdCounter;
}


sal_Int32 GrammarCheckingIterator::GetOrCreateDocId(
    const uno::Reference< lang::XComponent > &xComponent )
{
    // internal method; will always be called with locked mutex

    sal_Int32 nRes = -1;
    if (xComponent.is())
    {
        if (m_aDocIdMap.find( xComponent ) != m_aDocIdMap.end())
        {
            // add new entry
            nRes = m_aDocIdMap[ xComponent ];
            xComponent->addEventListener( this );
        }
        else // return already existing entry
        {
            nRes = NextDocId();
            m_aDocIdMap[ xComponent ] = nRes;
        }
    }
    return nRes;
}


void GrammarCheckingIterator::AddEntry(
    uno::WeakReference< text::XFlatParagraphIterator > xFlatParaIterator,
    uno::WeakReference< text::XFlatParagraph > xFlatPara,
    sal_Int32 nDocId,
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
        aNewFPEntry.m_nDocId        = nDocId;
        aNewFPEntry.m_nStartIndex   = nStartIndex;
        aNewFPEntry.m_bAutomatic    = bAutomatic;

        // add new entry to the end of this queue
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aFPEntriesQueue.push_back( aNewFPEntry );

        // wake up the thread in order to do grammar checking
        m_aWakeUpThread.set();
    }
}


void GrammarCheckingIterator::ProcessResult(
    const linguistic2::GrammarCheckingResult &rRes,
    sal_Int32 nSentenceStartPos,
    const uno::Reference< text::XFlatParagraphIterator > &rxFlatParagraphIterator,
    bool bIsAutomaticChecking )
{
    DBG_ASSERT( rRes.xFlatParagraph.is(), "xFlatParagraph is missing" );

    sal_Bool bContinueWithNextPara = sal_False;
    if (!rRes.xFlatParagraph.is() || rRes.xFlatParagraph->isModified())
    {
        // if paragraph was modified/deleted meanwhile continue with the next one...
        bContinueWithNextPara = sal_True;
    }
    else    // paragraph is still unchanged...
    {
        // mark found errors...
        bool bBoundariesOk = 0 <= nSentenceStartPos && nSentenceStartPos <= rRes.nEndOfSentencePos;
        DBG_ASSERT( bBoundariesOk, "inconsistent sentence boundaries" );
        uno::Sequence< linguistic2::SingleGrammarError > aErrors = rRes.aGrammarErrors;
        if (aErrors.getLength() > 0 && bBoundariesOk)
        {
            for (sal_Int16 i = 0; i < aErrors.getLength(); ++i)
            {
                linguistic2::SingleGrammarError aError = aErrors[aErrors.getLength() - 1 - i];

                OUString aIdentifier;
                uno::Reference< container::XStringKeyMap > xStringKeyMap;

                // display the grammar error mark
                rRes.xFlatParagraph->commitTextMarkup( aError.nErrorType, aIdentifier, aError.nErrorStart, aError.nErrorLength, xStringKeyMap );

                // commit sentence markup to identify sentence boundaries
                const sal_Int32 nSentenceLength = rRes.nEndOfSentencePos - nSentenceStartPos;
                rRes.xFlatParagraph->commitTextMarkup( text::TextMarkupType::SENTENCE, aIdentifier, nSentenceStartPos, nSentenceLength, xStringKeyMap );
            }
        }

        // other sentences left to be checked in this paragraph?
        if (rRes.nEndOfSentencePos < rRes.aText.getLength())
        {
            AddEntry( rxFlatParagraphIterator, rRes.xFlatParagraph, rRes.nDocumentId, rRes.nEndOfSentencePos, bIsAutomaticChecking );
        }
        else    // current paragraph finished
        {
            // set "already checked" flag for the current flat paragraph
            if (rRes.xFlatParagraph.is())
                rRes.xFlatParagraph->setChecked( text::TextMarkupType::GRAMMAR, true );

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
            AddEntry( rxFlatParagraphIterator, xFlatParaNext, rRes.nDocumentId, 0, bIsAutomaticChecking );
        }
    }
}


uno::Reference< linguistic2::XGrammarChecker > GrammarCheckingIterator::GetGrammarChecker(
    const lang::Locale &rLocale )
{
    (void) rLocale;
    uno::Reference< linguistic2::XGrammarChecker > xRes;

    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    // check supported locales for each grammarchecker if not already done
    if (!m_bGCServicesChecked)
    {
        //GetAvailableGCSvcs_Impl();
        //GetConfiguredGCSvcs_Impl();
        GetMatchingGCSvcs_Impl();
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
                        utl::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                uno::Reference< linguistic2::XGrammarChecker > xGC(
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

    // ---- THREAD SAFE START ----
    bool bEnd = false;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        bEnd = m_bEnd;
    }
    // ---- THREAD SAFE END ----
    while (!bEnd)
    {
        // ---- THREAD SAFE START ----
        bool bQueueEmpty = false;
        {
            ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
            bQueueEmpty = m_aFPEntriesQueue.empty();
        }
        // ---- THREAD SAFE END ----

        if (!bQueueEmpty)
        {
            uno::Reference< text::XFlatParagraphIterator > xFPIterator;
            uno::Reference< text::XFlatParagraph > xFlatPara;
            FPEntry aFPEntryItem;
            sal_Int32 nCurDocId = - 1;
            sal_Bool bModified = sal_False;
            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
                aFPEntryItem        = m_aFPEntriesQueue.front();
                xFPIterator         = aFPEntryItem.m_xParaIterator;
                xFlatPara           = aFPEntryItem.m_xPara;
                m_nCurCheckedDocId  = aFPEntryItem.m_nDocId;
                nCurDocId = m_nCurCheckedDocId;

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
                    ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );

                    sal_Int32 nStartPos = aFPEntryItem.m_nStartIndex;
                    sal_Int32 nSuggestedEnd = GetSuggestedEndOfSentence( aCurTxt, nStartPos, aCurLocale );

                    linguistic2::GrammarCheckingResult aRes;

                    uno::Reference< linguistic2::XGrammarChecker > xGC( GetGrammarChecker( aCurLocale ), uno::UNO_QUERY );
                    if (xGC.is())
                    {
                        aGuard.clear();
                        aRes = xGC->doGrammarChecking( nCurDocId, aCurTxt, aCurLocale, nStartPos, nSuggestedEnd, aLangPortions, aLangPortionsLocale );
                        aRes.xFlatParagraph      = xFlatPara;
                        aRes.nStartOfSentencePos = nStartPos;
                    }
                    else
                    {
                        // no grammar checker -> no error
                        // but we need to provide the data below in order to continue with the next sentence
                        aRes.nDocumentId         = nCurDocId;
                        aRes.xFlatParagraph      = xFlatPara;
                        aRes.aText               = aCurTxt;
                        aRes.aLocale             = aCurLocale;
                        aRes.nStartOfSentencePos = nStartPos;
                        aRes.nEndOfSentencePos   = nSuggestedEnd;
                    }
                    ProcessResult( aRes, nStartPos, xFPIterator, aFPEntryItem.m_bAutomatic );
                    // ---- THREAD SAFE END ----
                }
                else
                {
                    // the paragraph changed meanwhile... (and maybe is still edited)
                    // thus we simply continue to ask for the next to be checked.
                    uno::Reference< text::XFlatParagraph > xFlatParaNext( xFPIterator->getNextPara() );
                    AddEntry( xFPIterator, xFlatParaNext, nCurDocId, 0, aFPEntryItem.m_bAutomatic );
                }
            }

            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
                m_nCurCheckedDocId  = -1;
            }
            // ---- THREAD SAFE END ----
        }
        else
        {
            // ---- THREAD SAFE START ----
            {
                ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
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

        // ---- THREAD SAFE START ----
        {
            ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
            bEnd = m_bEnd;
        }
        // ---- THREAD SAFE END ----
    }

    //!! This one must be the very last statement to call in this function !!
    m_aRequestEndThread.set();
}


void SAL_CALL GrammarCheckingIterator::startGrammarChecking(
    const uno::Reference< ::uno::XInterface > & xDoc,
    const uno::Reference< text::XFlatParagraphIteratorProvider > & xIteratorProvider,
    sal_Bool bAutomatic)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    // get paragraph to start checking with
    uno::Reference<text::XFlatParagraphIterator> xFPIterator = xIteratorProvider->getFlatParagraphIterator(
            text::TextMarkupType::GRAMMAR, bAutomatic );
    uno::Reference< text::XFlatParagraph > xPara( xFPIterator.is()? xFPIterator->getFirstPara() : NULL );
    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );

    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
    if (xPara.is() && xComponent.is())
    {
        sal_Int32 nDocId = GetOrCreateDocId( xComponent );

        // create new entry and add it to queue
        AddEntry( xFPIterator, xPara, nDocId, 0, bAutomatic );
    }
    // ---- THREAD SAFE END ----
}


linguistic2::GrammarCheckingResult SAL_CALL GrammarCheckingIterator::checkGrammarAtPos(
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

    linguistic2::GrammarCheckingResult aRes;

    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );
    if (xFlatPara.is() && xComponent.is() &&
        ( nErrorPosInPara < 0 || nErrorPosInPara < rText.getLength()))
    {
        uno::Sequence< sal_Int32 >      aLangPortions;
        uno::Sequence< lang::Locale >   aLangPortionsLocale;

        // iterate through paragraph until we find the sentence we are interested in
        linguistic2::GrammarCheckingResult aTmpRes;
        sal_Int32 nStartPos = nErrorPosInPara > 0 ? 0 : nStartOfSentencePos;
        bool bFound = false;
        do
        {
            lang::Locale aCurLocale = lcl_GetPrimaryLanguageOfSentence( xFlatPara, nStartPos );
            sal_Int32 nOldStartOfSentencePos = nStartPos;
            uno::Reference< linguistic2::XGrammarChecker > xGC;
            sal_Int32 nDocId = -1;

            // ---- THREAD SAFE START ----
            {
                ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
                nDocId = GetOrCreateDocId( xComponent );
                nSuggestedEndOfSentencePos = GetSuggestedEndOfSentence( rText, nStartPos, aCurLocale );

                xGC = GetGrammarChecker( aCurLocale );
            }
            // ---- THREAD SAFE START ----
            sal_Int32 nEndPos = -1;
            if (xGC.is())
            {
                aTmpRes = xGC->doGrammarChecking( nDocId, rText, aCurLocale, nStartPos, nSuggestedEndOfSentencePos, aLangPortions, aLangPortionsLocale );
                aTmpRes.xFlatParagraph      = xFlatPara;
                aTmpRes.nStartOfSentencePos = nStartPos;
                nEndPos = aTmpRes.nEndOfSentencePos;

                if ((nErrorPosInPara< 0 || nStartPos <= nErrorPosInPara) && nErrorPosInPara < nEndPos)
                    bFound = true;
            }
            if (nEndPos == -1) // no result from grammar checker
                nEndPos = nSuggestedEndOfSentencePos;
            nStartPos = nEndPos;

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

    uno::Reference< i18n::XBreakIterator > xBreakIterator;
    if (!m_xBreakIterator.is())
    {
        m_xBreakIterator = vcl::unohelper::CreateBreakIterator();
    }
    sal_Int32 nTextLen = rText.getLength();
    sal_Int32 nEndPosition = nTextLen;
    if (m_xBreakIterator.is())
    {
        sal_Int32 nTmpStartPos = nSentenceStartPos;
        do
        {
            nEndPosition = nTextLen;
            if (nTmpStartPos < nTextLen)
                nEndPosition = m_xBreakIterator->endOfSentence( rText, nTmpStartPos, rLocale );
            ++nTmpStartPos;
        }
        while (nEndPosition <= nSentenceStartPos && nEndPosition < nTextLen);
        if (nEndPosition > nTextLen)
            nEndPosition = nTextLen;
    }
    return nEndPosition;
}


sal_Int32 SAL_CALL GrammarCheckingIterator::getEndOfSentence(
    const uno::Reference< text::XFlatParagraph >& xFlatPara,
    sal_Int32 nSentenceStartPos )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
    (void) xFlatPara;
    (void)nSentenceStartPos;
    return 0;
}


sal_Bool SAL_CALL GrammarCheckingIterator::isGrammarChecking(
    const uno::Reference< uno::XInterface >& xDoc,
    sal_Bool bAutomatic )
throw (uno::RuntimeException)
{
    // ---- THREAD SAFE START ----
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    (void) bAutomatic;
    sal_Bool bRes = sal_False;

    uno::Reference< lang::XComponent > xComponent( xDoc, uno::UNO_QUERY );
    if (xComponent.is())
    {
        sal_Int32 nDocId = m_aDocIdMap[ xComponent ];
        if (m_nCurCheckedDocId >= 0 && m_nCurCheckedDocId == nDocId)
            bRes = sal_True;
        else if (m_aDocIdMap.find( xComponent ) != m_aDocIdMap.end())
        {
            sal_Int32 nSize = m_aFPEntriesQueue.size();
            for (sal_Int32 i = 0; i < nSize && !bRes; ++i)
            {
                if (nDocId == m_aFPEntriesQueue[i].m_nDocId /*&& m_aFPEntriesQueue[i].m_bAutomatic == bAutomatic*/)
                    bRes = sal_True;
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
    if (rLngSvcEvent.nEvent == linguistic2::LinguServiceEventFlags::GRAMMAR_CHECK_AGAIN)
    {
        try
        {
             uno::Reference< uno::XInterface > xThis(*this);
             linguistic2::LinguServiceEvent aEvent( xThis, linguistic2::LinguServiceEventFlags::GRAMMAR_CHECK_AGAIN );
             m_aNotifyListeners.notifyEach(
                    &linguistic2::XLinguServiceEventListener::processLinguServiceEvent,
                    aEvent);
        }
        catch (uno::RuntimeException &)
        {
             throw;
        }
        catch (::uno::Exception &rE)
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
//        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
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
//        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aNotifyListeners.removeInterface( xListener );
    }
    return sal_True;
}


void SAL_CALL GrammarCheckingIterator::dispose()
throw (uno::RuntimeException)
{
    lang::EventObject aEvt( (linguistic2::XGrammarCheckingIterator *) this );
    m_aEventListeners.disposeAndClear( aEvt );

    //
    // now end the thread...
    //
    m_aRequestEndThread.reset();
    // ---- THREAD SAFE START ----
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_bEnd = sal_True;
    }
    // ---- THREAD SAFE END ----
    m_aWakeUpThread.set();
    const TimeValue aTime = { 3, 0 };   // wait 3 seconds...
    m_aRequestEndThread.wait( &aTime );
    // if the call ends because of time-out we will end anyway...


    // ---- THREAD SAFE START ----
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

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
//        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aEventListeners.addInterface( xListener );
    }
}


void SAL_CALL GrammarCheckingIterator::removeEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
throw (uno::RuntimeException)
{
    if (xListener.is())
    {
//        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
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
    //!! All of the above resulting in that we only have to get rid of all references here.
    uno::Reference< lang::XComponent > xDoc( rSource.Source, uno::UNO_QUERY );
    if (xDoc.is())
    {
        // ---- THREAD SAFE START ----
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aDocIdMap.erase( xDoc );
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
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider;
            uno::Reference< lang::XMultiServiceFactory > xMgr = utl::getProcessServiceFactory();
            if (xMgr.is())
            {
                xConfigurationProvider = uno::Reference< lang::XMultiServiceFactory > (
                        xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.configuration.ConfigurationProvider" ) ) ),
                        uno::UNO_QUERY_THROW ) ;
            }

            // get configuration update access
            beans::PropertyValue aValue;
            aValue.Name  = A2OU( "nodepath" );
            aValue.Value = uno::makeAny( A2OU("org.openoffice.Office.Linguistic/ServiceManager") );
            uno::Sequence< uno::Any > aProps(1);
            aProps[0] <<= aValue;
            m_xUpdateAccess = uno::Reference< util::XChangesBatch >(
                    xConfigurationProvider->createInstanceWithArguments(
                        A2OU( "com.sun.star.configuration.ConfigurationUpdateAccess" ), aProps ),
                        uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
    }

    return m_xUpdateAccess;
}

/*
void GrammarCheckingIterator::GetConfiguredGCSvcs_Impl()
{
    GCImplNames_t   aTmpGCImplNamesByLang;

    try
    {
        // get node names (locale iso strings) for configured grammar checkers
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("GrammarCheckerList") ), uno::UNO_QUERY_THROW );
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
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aGCImplNamesByLang     = aTmpGCImplNamesByLang;
        // ---- THREAD SAFE END ----
    }
}
*/

void GrammarCheckingIterator::GetMatchingGCSvcs_Impl()
{
    GCImplNames_t   aTmpGCImplNamesByLang;

    try
    {
        // get node names (locale iso strings) for configured grammar checkers
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("GrammarCheckers") ), uno::UNO_QUERY_THROW );
        const uno::Sequence< OUString > aGCImplNames( xNA->getElementNames() );
        const OUString *pGCImplNames = aGCImplNames.getConstArray();

        sal_Int32 nLen = aGCImplNames.getLength();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Reference< container::XNameAccess > xTmpNA( xNA->getByName( pGCImplNames[i] ), uno::UNO_QUERY_THROW );
            uno::Any aTmp( xTmpNA->getByName( A2OU("Locales") ) );
            uno::Sequence< OUString >    aIsoLocaleNames;
            if (aTmp >>= aIsoLocaleNames)
            {
                const OUString *pIsoLocaleNames = aIsoLocaleNames.getConstArray();
                for (sal_Int32 k = 0;  k < aIsoLocaleNames.getLength();  ++k)
                {
                    // if there are more grammar checkers for one language, for the time being,
                    // the last one found here will win...
                    const LanguageType nLang = MsLangId::convertIsoStringToLanguage( pIsoLocaleNames[k] );
                    aTmpGCImplNamesByLang[ nLang ] = pGCImplNames[i];
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
        DBG_ASSERT( 0, "exception caught. Failed to get matching grammar checker services" );
    }

    {
        // ---- THREAD SAFE START ----
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_aGCImplNamesByLang     = aTmpGCImplNamesByLang;
        // ---- THREAD SAFE END ----
    }
}


/*
void GrammarCheckingIterator::GetAvailableGCSvcs_Impl()
{
    // internal method; will always be called with locked mutex
    if (m_xMSF.is())
    {
        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( m_xMSF, uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration( A2OU( SN_GRAMMARCHECKER ) );

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleComponentFactory > xCompFactory;
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                uno::Reference< uno::XComponentContext > xContext;
                uno::Reference< beans::XPropertySet > xProps( m_xMSF, uno::UNO_QUERY );
                xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;

                if ( xContext.is() &&
                        (cppu::extractInterface( xCompFactory, aCurrent ) ||
                         cppu::extractInterface( xFactory, aCurrent )) )
                {
                    try
                    {
                        uno::Reference< linguistic2::XGrammarChecker > xSvc( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        if (xSvc.is())
                        {
                            OUString    aImplName;
                            uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                            if (xInfo.is())
                                aImplName = xInfo->getImplementationName();
                            DBG_ASSERT( aImplName.getLength(), "empty implementation name" );
                            uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                            DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                            if (xSuppLoc.is() && aImplName.getLength() > 0)
                            {
                                uno::Sequence< lang::Locale > aLocaleSequence( xSuppLoc->getLocales() );
                                // ---- THREAD SAFE START ----
                                ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
                                m_aGCLocalesByService[ aImplName ] = aLocaleSequence;
                                m_aGCReferencesByService[ aImplName ] = xSvc;
                                // ---- THREAD SAFE END ----
                            }
                        }
                    }
                    catch (uno::Exception &)
                    {
                        DBG_ASSERT( 0, "instantiating grammar checker failed" );
                    }
                }
            }
        }
    }
}
*/


sal_Bool SAL_CALL GrammarCheckingIterator::supportsService(
    const OUString & rServiceName )
throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == rServiceName )
            return TRUE;
    return FALSE;
}


OUString SAL_CALL GrammarCheckingIterator::getImplementationName(  ) throw (uno::RuntimeException)
{
    return GrammarCheckingIterator_getImplementationName();
}


uno::Sequence< OUString > SAL_CALL GrammarCheckingIterator::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return GrammarCheckingIterator_getSupportedServiceNames();
}


///////////////////////////////////////////////////////////////////////////


static OUString GrammarCheckingIterator_getImplementationName() throw()
{
    return A2OU( "com.sun.star.lingu2.GrammarCheckingIterator" );
}


static uno::Sequence< OUString > GrammarCheckingIterator_getSupportedServiceNames() throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = A2OU( "com.sun.star.linguistic2.GrammarCheckingIterator" );
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


sal_Bool SAL_CALL GrammarCheckingIterator_writeInfo(
    void * /*pServiceManager*/,
    registry::XRegistryKey * pRegistryKey )
{
    try
    {
        OUString aImpl( '/' );
        aImpl += GrammarCheckingIterator_getImplementationName().getStr();
        aImpl += A2OU( "/UNO/SERVICES" );
        uno::Reference< registry::XRegistryKey > xNewKey = pRegistryKey->createKey( aImpl );
        uno::Sequence< OUString > aServices = GrammarCheckingIterator_getSupportedServiceNames();
        for( sal_Int32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch (uno::Exception &)
    {
        return sal_False;
    }
}

