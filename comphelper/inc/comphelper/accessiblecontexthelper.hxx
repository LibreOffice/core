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

#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/broadcasthelper.hxx>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= IMutex
    //=====================================================================

    // This whole thingie here (own mutex classes and such) is a HACK. I hate the SolarMutex.
    // See below for more explanations ....

    /** abstract interface for implementing a mutex
    */
    class COMPHELPER_DLLPUBLIC IMutex
    {
    public:
        virtual ~IMutex();
        virtual void acquire() = 0;
        virtual void release() = 0;
    };

    //=====================================================================
    //= OMutexGuard
    //=====================================================================

    class OMutexGuard
    {
        IMutex* m_pMutex;
    public:
        inline OMutexGuard( IMutex* _pMutex )
            :m_pMutex( _pMutex )
        {
            if ( m_pMutex )
                m_pMutex->acquire();
        }

        inline ~OMutexGuard( )
        {
            if ( m_pMutex )
                m_pMutex->release();
        }
    };

    //=====================================================================
    //= OAccessibleContextHelper
    //=====================================================================

    class OContextHelper_Impl;
    typedef ::cppu::WeakAggComponentImplHelper2 <   ::com::sun::star::accessibility::XAccessibleContext,
                                                    ::com::sun::star::accessibility::XAccessibleEventBroadcaster
                                                >   OAccessibleContextHelper_Base;

    /** helper class for implementing an AccessibleContext
    */
    class COMPHELPER_DLLPUBLIC OAccessibleContextHelper
                :public ::comphelper::OBaseMutex
                ,public OAccessibleContextHelper_Base
    {
    private:
        OContextHelper_Impl*    m_pImpl;

    protected:
        OAccessibleContextHelper( );
        ~OAccessibleContextHelper( );

        /** ctor

            <p>If you need additional object safety for your class, and want to ensure that your own
            mutex is locked before the mutex this class provides is, than use this ctor.</p>

            <p>Beware that this is a hack. Unfortunately, OpenOffice.org has two different mutex hierarchies,
            which are not compatible. In addition, wide parts of the code (especially VCL) is not thread-safe,
            but instead relies on a <em>single global mutex</em>. As a consequence, components using
            directly or indirectly such code need to care for this global mutex. Yes, this is as ugly as
            anything.</p>

            <p>Note that the external lock is used as additional lock, not as the only one. The own mutex of the
            instance is used for internal actions, and every action which potentially involves external code
            (for instance every call to a virtual method overridden by derivees) is <em>additionally</em> and
            <em>first</em> guarded by with the external lock.</p>

            <p>Beware of the lifetime of the lock - you must ensure that the lock exists at least as long as
            the context does. A good approach to implement the lock may be to derive you own context
            not only from OAccessibleContextHelper, but also from IMutex.</p>

            <p>One more note. This lock is definately not used once the dtor is reached. Means whatever
            the dtor implementation does, it does <em>not</em> guard the external lock. See this as a contract.
            <br/>You should ensure the same thing for own derivees which do not supply the lock themself,
            but get them from yet another derivee.</p>
            @see forgetExternalLock
        */
        OAccessibleContextHelper( IMutex* _pExternalLock );

        /** late construction
        @param _rxAccessible
            the Accessible object which created this context.
            <p>If your derived implementation implements the XAccessible (and does not follow the proposed
            separation of XAccessible from XAccessibleContext), you may pass <code>this</code> here.</p>

            <p>The object is hold weak, so it's life time is not affected.</p>

            <p>The object is needed for performance reasons: for <method>getAccessibleIndexInParent</method>,
            all children (which are XAccessible's theirself) of our parent have to be asked. If we know our
            XAccessible, we can compare it with all the children, instead of asking all children for their
            context and comparing this context with ourself.</p>
        */
        void    lateInit( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxAccessible );

        /** retrieves the creator previously set with <method>lateInit</method>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                getAccessibleCreator( ) const;

        /** forgets the reference to the external lock, if present.

            <p>This means any further locking will not be guard the external lock anymore, never.</p>

            <p>To be used in derived classes which do not supply the external lock themself, but instead get
            them passed from own derivees (or clients).</p>
        */
        void    forgetExternalLock();

    public:
        // XAccessibleEventBroadcaster
        using WeakAggComponentImplHelperBase::addEventListener;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        using WeakAggComponentImplHelperBase::removeEventListener;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

        // XAccessibleContext - default implementations
        /** default implementation for retrieving the index of this object within the parent
            <p>This basic implementation here returns the index <code>i</code> of the child for which
                <code>&lt;parent&gt;.getAccessibleChild( i )</code> equals our creator.</p>
        */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
        /** default implementation for retrieving the locale
            <p>This basic implementation returns the locale of the parent context,
            as retrieved via getAccessibleParent()->getAccessibleContext.</p>
        */
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    public:
        // helper struct for granting selective access rights
        struct OAccessControl
        {
            friend class OContextEntryGuard;
            friend class OContextHelper_Impl;
            friend class OExternalLockGuard;
        private:
            OAccessControl() { }
        };

        // ensures that the object is alive
        inline  void            ensureAlive( const OAccessControl& ) const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );
        inline  IMutex*         getExternalLock( const OAccessControl& );
        inline  ::osl::Mutex&   GetMutex( const OAccessControl& );

    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        // helper
        /** notifies all AccessibleEventListeners of a certain event

        @precond    not too be called with our mutex locked
        @param  _nEventId
            the id of the even. See AccessibleEventType
        @param  _rOldValue
            the old value to be notified
        @param  _rNewValue
            the new value to be notified
        */
        virtual void SAL_CALL   NotifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                );

        // life time control
        /// checks whether the object is alive (returns <TRUE/> then) or disposed
        sal_Bool    isAlive() const;
        /// checks for beeing alive. If the object is already disposed (i.e. not alive), an exception is thrown.
        void        ensureAlive() const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );

        /** ensures that the object is disposed.
        @precond
            to be called from within the destructor of your derived class only!
        */
        void        ensureDisposed( );

        /** shortcut for retrieving the context of the parent (returned by getAccessibleParent)
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                    implGetParentContext() SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

        // access to the base class' broadcast helper/mutex
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return rBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return rBHelper; }
        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
        IMutex*                         getExternalLock( );
    };

    //---------------------------------------------------------------------
    inline  void OAccessibleContextHelper::ensureAlive( const OAccessControl& ) const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) )
    {
        ensureAlive();
    }

    //---------------------------------------------------------------------
    inline  IMutex* OAccessibleContextHelper::getExternalLock( const OAccessControl& )
    {
        return getExternalLock();
    }

    //---------------------------------------------------------------------
    inline  ::osl::Mutex& OAccessibleContextHelper::GetMutex( const OAccessControl& )
    {
        return GetMutex();
    }

    //=====================================================================
    //= OContextEntryGuard
    //=====================================================================
    typedef ::osl::ClearableMutexGuard  OContextEntryGuard_Base;
    /** helper class for guarding the entry into OAccessibleContextHelper methods.

        <p>The class has two responsibilities:
        <ul><li>it locks the mutex of an OAccessibleContextHelper instance, as long as the guard lives</li>
            <li>it checks if an given OAccessibleContextHelper instance is alive, else an exception is thrown
                our of the constructor of the guard</li>
        </ul>
        <br/>
        This makes it your first choice (hopefully :) for guarding any interface method implementations of
        you derived class.
        </p>
    */
    class OContextEntryGuard : public OContextEntryGuard_Base
    {
    public:
        /** constructs the guard

            <p>The given context (it's mutex, respectively) is locked, and an exception is thrown if the context
            is not alive anymore. In the latter case, of course, the mutex is freed, again.</p>

        @param _pContext
            the context which shall be guarded
        @precond <arg>_pContext</arg> != NULL
        */
        inline OContextEntryGuard( OAccessibleContextHelper* _pContext );

        /** destructs the guard.
            <p>The context (it's mutex, respectively) is unlocked.</p>
        */
        inline ~OContextEntryGuard();
    };

    //.....................................................................
    inline OContextEntryGuard::OContextEntryGuard( OAccessibleContextHelper* _pContext  )
        :OContextEntryGuard_Base( _pContext->GetMutex( OAccessibleContextHelper::OAccessControl() ) )
    {
        _pContext->ensureAlive( OAccessibleContextHelper::OAccessControl() );
    }

    //.....................................................................
    inline OContextEntryGuard::~OContextEntryGuard()
    {
    }

    //=====================================================================
    //= OExternalLockGuard
    //=====================================================================
    class OExternalLockGuard
            :public OMutexGuard
            ,public OContextEntryGuard
    {
    public:
        inline OExternalLockGuard( OAccessibleContextHelper* _pContext );
        inline ~OExternalLockGuard( );
    };

    //.....................................................................
    inline OExternalLockGuard::OExternalLockGuard( OAccessibleContextHelper* _pContext )
        :OMutexGuard( _pContext->getExternalLock( OAccessibleContextHelper::OAccessControl() ) )
        ,OContextEntryGuard( _pContext )
    {
        // #102438#
        // Only lock the external mutex,
        // release the ::osl::Mutex of the OAccessibleContextHelper instance.
        // If you call into another UNO object with locked ::osl::Mutex,
        // this may lead to dead locks.
        clear();
    }

    //.....................................................................
    inline OExternalLockGuard::~OExternalLockGuard( )
    {
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
