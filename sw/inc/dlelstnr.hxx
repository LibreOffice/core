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

#ifndef _DLELSTNR_HXX_
#define _DLELSTNR_HXX_

#include <cppuhelper/weak.hxx>
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations


namespace com { namespace sun { namespace star {
    namespace linguistic2 {
        class XDictionaryList;
        class XLinguServiceManager;
        class XProofreadingIterator;
    }
    namespace frame {
        class XTerminateListener;
    }
} } }

///////////////////////////////////////////////////////////////////////////
// SwLinguServiceEventListener
// is a EventListener that triggers spellchecking
// and hyphenation when relevant changes (to the
// dictionaries of the dictionary list, or properties) were made.
//

class SwLinguServiceEventListener :
    public cppu::WeakImplHelper2
    <
        com::sun::star::linguistic2::XLinguServiceEventListener,
        com::sun::star::frame::XTerminateListener
    >
{
    com::sun::star::uno::Reference<
        com::sun::star::frame::XDesktop >                       xDesktop;
    com::sun::star::uno::Reference<
        com::sun::star::linguistic2::XLinguServiceManager >     xLngSvcMgr;
    com::sun::star::uno::Reference<
        com::sun::star::linguistic2::XProofreadingIterator >    xGCIterator;

    /// disallow use of copy-constructor and assignment operator
    SwLinguServiceEventListener(const SwLinguServiceEventListener &);
    SwLinguServiceEventListener & operator = (const SwLinguServiceEventListener &);

public:
    SwLinguServiceEventListener();
    virtual ~SwLinguServiceEventListener();

    /// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::uno::RuntimeException);

    /// XDictionaryListEventListener
    virtual void SAL_CALL processDictionaryListEvent( const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent) throw( ::com::sun::star::uno::RuntimeException );

    /// XLinguServiceEventListener
    virtual void SAL_CALL processLinguServiceEvent( const ::com::sun::star::linguistic2::LinguServiceEvent& rLngSvcEvent ) throw(::com::sun::star::uno::RuntimeException);

    /// XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
