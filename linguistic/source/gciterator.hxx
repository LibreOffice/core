/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gciterator.hxx,v $
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

#ifndef _LINGUISTIC_GRAMMARCHECKINGITERATOR_HXX_
#define _LINGUISTIC_GRAMMARCHECKINGITERATOR_HXX_

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingIterator.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>

#include <map>
#include <deque>


//////////////////////////////////////////////////////////////////////


struct FPEntry
{
    // flat paragraph iterator
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIterator > m_xParaIterator;

    // flat paragraph
    ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > m_xPara;

    // document ID to identify different documents
    sal_Int32     m_nDocId;

    // the starting position to be checked
    sal_Int32     m_nStartIndex;

    // the flag to identify whether the document does automatical grammar checking
    sal_Bool      m_bAutomatic;

    FPEntry()
        : m_nDocId( 0 )
        , m_nStartIndex( 0 )
        , m_bAutomatic( 0 )
    {
    }
};


///////////////////////////////////////////////////////////////////////////


class GrammarCheckingIterator:
    public cppu::WeakImplHelper5
    <
        ::com::sun::star::linguistic2::XGrammarCheckingIterator,
        ::com::sun::star::linguistic2::XLinguServiceEventListener,
        ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    m_xMSF;


    //the queue is keeping track of all senteces to be checked
    //every element of this queue is a FlatParagraphEntry struct-object
    typedef std::deque< FPEntry > FPQueue_t;

    // queue for entries to be processed
    FPQueue_t       m_aFPEntriesQueue;

    // the flag to end the endless loop
    sal_Bool        m_bEnd;

    // parameter ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponen > --> the document
    // parameter sal_Int32 --> DocId to indentify the document
    typedef std::map< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >, sal_Int32 > DocMap_t;
    DocMap_t        m_aDocIdMap;

    // parameter ::rtl::OUString --> implementation name
    // parameter ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > --> list of locales supported by service
//    typedef std::map< ::rtl::OUString, ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > > GCLocales_t;
//    GCLocales_t     m_aGCLocalesByService;

    // language -> implname mapping
    typedef std::map< LanguageType, ::rtl::OUString > GCImplNames_t;
    GCImplNames_t   m_aGCImplNamesByLang;

    // implname -> UNO reference mapping
    typedef std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XGrammarChecker > > GCReferences_t;
    GCReferences_t  m_aGCReferencesByService;

    sal_Bool        m_bGCServicesChecked;
    sal_Int32       m_nDocIdCounter;
    sal_Int32       m_nCurCheckedDocId;
    sal_Int32       m_nLastEndOfSentencePos;
    osl::Condition  m_aWakeUpThread;
    osl::Condition  m_aRequestEndThread;

    //! beware of initilization order !
    osl::Mutex                          m_aMutex;
    cppu::OInterfaceContainerHelper     m_aEventListeners;
    cppu::OInterfaceContainerHelper     m_aNotifyListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > m_xBreakIterator;
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >  m_xUpdateAccess;

    sal_Int32 NextDocId();
    sal_Int32 GetOrCreateDocId( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &xComp );

    void AddEntry(
            ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraphIterator > xFlatParaIterator,
            ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > xFlatPara,
            sal_Int32 nDocId, sal_Int32 nStartIndex, sal_Bool bAutomatic );

    void ProcessResult( const ::com::sun::star::linguistic2::GrammarCheckingResult &rRes, sal_Int32 nSentenceStartPos,
            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIterator > &rxFlatParagraphIterator,
            bool bIsAutomaticChecking );

    sal_Int32 GetSuggestedEndOfSentence( const ::rtl::OUString &rText, sal_Int32 nSentenceStartPos, const ::com::sun::star::lang::Locale &rLocale );

//    void GetConfiguredGCSvcs_Impl();
    void GetMatchingGCSvcs_Impl();
//    void GetAvailableGCSvcs_Impl();
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XGrammarChecker > GetGrammarChecker( const ::com::sun::star::lang::Locale & rLocale );

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >   GetUpdateAccess() const;

    // disallow use of copy c-tor and assignment operator
    GrammarCheckingIterator( const GrammarCheckingIterator & );
    GrammarCheckingIterator & operator = ( const GrammarCheckingIterator & );

public:

    void DequeueAndCheck();

    explicit GrammarCheckingIterator( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rxMgr );
    virtual ~GrammarCheckingIterator();

    // XGrammarCheckingIterator
    virtual void SAL_CALL startGrammarChecking( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIteratorProvider >& xIteratorProvider, ::sal_Bool bAutomatic ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::linguistic2::GrammarCheckingResult SAL_CALL checkGrammarAtPos( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xFlatPara, const ::rtl::OUString& aText, const ::com::sun::star::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePos, ::sal_Int32 nSuggestedEndOfSentencePos, ::sal_Int32 nErrorPosInPara ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getEndOfSentence( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xFlatPara, ::sal_Int32 nSentenceStartPos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isGrammarChecking( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc, ::sal_Bool bAutomatic ) throw (::com::sun::star::uno::RuntimeException);

    // XLinguServiceEventListener
    virtual void SAL_CALL processLinguServiceEvent( const ::com::sun::star::linguistic2::LinguServiceEvent& aLngSvcEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual ::sal_Bool SAL_CALL addLinguServiceEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguServiceEventListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL removeLinguServiceEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguServiceEventListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};


///////////////////////////////////////////////////////////////////////////

#endif

