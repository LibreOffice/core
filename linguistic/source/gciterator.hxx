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

#ifndef INCLUDED_LINGUISTIC_SOURCE_GCITERATOR_HXX
#define INCLUDED_LINGUISTIC_SOURCE_GCITERATOR_HXX

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.h>
#include <rtl/instance.hxx>

#include <map>
#include <deque>

#include "defs.hxx"



struct FPEntry
{
    // flat paragraph iterator
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIterator > m_xParaIterator;

    // flat paragraph
    ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > m_xPara;

    // document ID to identify different documents
    OUString m_aDocId;

    // the starting position to be checked
    sal_Int32       m_nStartIndex;

    // the flag to identify whether the document does automatical grammar checking
    bool        m_bAutomatic;

    FPEntry()
        : m_aDocId()
        , m_nStartIndex( 0 )
        , m_bAutomatic( false )
    {
    }
};




class GrammarCheckingIterator:
    public cppu::WeakImplHelper
    <
        ::com::sun::star::linguistic2::XProofreadingIterator,
        ::com::sun::star::linguistic2::XLinguServiceEventListener,
        ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >,
    public LinguDispatcher
{
    //the queue is keeping track of all sentences to be checked
    //every element of this queue is a FlatParagraphEntry struct-object
    typedef std::deque< FPEntry > FPQueue_t;

    // queue for entries to be processed
    FPQueue_t       m_aFPEntriesQueue;

    // the flag to end the endless loop
    bool        m_bEnd;

    // Note that it must be the pointer and not the uno-reference to check if it is the same implementation object
    typedef std::map< XComponent *, OUString > DocMap_t;
    DocMap_t        m_aDocIdMap;


    // language -> implname mapping
    typedef std::map< LanguageType, OUString > GCImplNames_t;
    GCImplNames_t   m_aGCImplNamesByLang;

    // implname -> UNO reference mapping
    typedef std::map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > > GCReferences_t;
    GCReferences_t  m_aGCReferencesByService;

    OUString m_aCurCheckedDocId;
    bool        m_bGCServicesChecked;
    sal_Int32       m_nDocIdCounter;
    osl::Condition  m_aWakeUpThread;
    oslThread       m_thread;

    //! beware of initilization order !
    struct MyMutex : public rtl::Static< osl::Mutex, MyMutex > {};
    cppu::OInterfaceContainerHelper     m_aEventListeners;
    cppu::OInterfaceContainerHelper     m_aNotifyListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > m_xBreakIterator;
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >  m_xUpdateAccess;

    void TerminateThread();

    sal_Int32 NextDocId();
    OUString GetOrCreateDocId( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &xComp );

    void AddEntry(
            ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraphIterator > xFlatParaIterator,
            ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > xFlatPara,
            const OUString &rDocId, sal_Int32 nStartIndex, bool bAutomatic );

    void ProcessResult( const ::com::sun::star::linguistic2::ProofreadingResult &rRes,
            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIterator > &rxFlatParagraphIterator,
            bool bIsAutomaticChecking );

    sal_Int32 GetSuggestedEndOfSentence( const OUString &rText, sal_Int32 nSentenceStartPos, const ::com::sun::star::lang::Locale &rLocale );

    void GetConfiguredGCSvcs_Impl();
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > GetGrammarChecker( const ::com::sun::star::lang::Locale & rLocale );

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >   GetUpdateAccess() const;

    GrammarCheckingIterator( const GrammarCheckingIterator & ) = delete;
    GrammarCheckingIterator & operator = ( const GrammarCheckingIterator & ) = delete;

public:

    void DequeueAndCheck();

    explicit GrammarCheckingIterator();
    virtual ~GrammarCheckingIterator();

    // XProofreadingIterator
    virtual void SAL_CALL startProofreading( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDocument, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIteratorProvider >& xIteratorProvider ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::linguistic2::ProofreadingResult SAL_CALL checkSentenceAtPosition( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDocument, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xFlatParagraph, const OUString& aText, const ::com::sun::star::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePosition, ::sal_Int32 nSuggestedBehindEndOfSentencePosition, ::sal_Int32 nErrorPositionInParagraph ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL resetIgnoreRules(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isProofreading( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDocument ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinguServiceEventListener
    virtual void SAL_CALL processLinguServiceEvent( const ::com::sun::star::linguistic2::LinguServiceEvent& aLngSvcEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguServiceEventListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguServiceEventListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // LinguDispatcher
    virtual void SetServiceList( const ::com::sun::star::lang::Locale &rLocale, const ::com::sun::star::uno::Sequence< OUString > &rSvcImplNames ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const SAL_OVERRIDE;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
