/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    // disallow use of copy-constructor and assignment operator
    SwLinguServiceEventListener(const SwLinguServiceEventListener &);
    SwLinguServiceEventListener & operator = (const SwLinguServiceEventListener &);

public:
    SwLinguServiceEventListener();
    virtual ~SwLinguServiceEventListener();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::uno::RuntimeException);

    // XDictionaryListEventListener
    virtual void SAL_CALL processDictionaryListEvent( const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent) throw( ::com::sun::star::uno::RuntimeException );

    // XLinguServiceEventListener
    virtual void SAL_CALL processLinguServiceEvent( const ::com::sun::star::linguistic2::LinguServiceEvent& rLngSvcEvent ) throw(::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& rEventObj ) throw(::com::sun::star::uno::RuntimeException);
};


#endif

