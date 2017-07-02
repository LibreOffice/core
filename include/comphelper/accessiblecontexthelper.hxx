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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLECONTEXTHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLECONTEXTHELPER_HXX

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/solarmutex.hxx>
#include <memory>


namespace comphelper
{


    //= OAccessibleContextHelper


    class OContextHelper_Impl;
    typedef ::cppu::WeakAggComponentImplHelper2 <   css::accessibility::XAccessibleContext,
                                                    css::accessibility::XAccessibleEventBroadcaster
                                                >   OAccessibleContextHelper_Base;

    /** helper class for implementing an AccessibleContext
    */
    class COMPHELPER_DLLPUBLIC OAccessibleContextHelper
                :public ::cppu::BaseMutex
                ,public OAccessibleContextHelper_Base
    {
        friend class OContextEntryGuard;
    private:
        std::unique_ptr<OContextHelper_Impl>    m_pImpl;

    protected:
        virtual ~OAccessibleContextHelper( ) override;

        OAccessibleContextHelper( );

        /** late construction
        @param _rxAccessible
            the Accessible object which created this context.
            <p>If your derived implementation implements the XAccessible (and does not follow the proposed
            separation of XAccessible from XAccessibleContext), you may pass <code>this</code> here.</p>

            <p>The object is hold weak, so its life time is not affected.</p>

            <p>The object is needed for performance reasons: for <method>getAccessibleIndexInParent</method>,
            all children (which are XAccessible's theirself) of our parent have to be asked. If we know our
            XAccessible, we can compare it with all the children, instead of asking all children for their
            context and comparing this context with ourself.</p>
        */
        void    lateInit( const css::uno::Reference< css::accessibility::XAccessible >& _rxAccessible );

        /** retrieves the creator previously set with <method>lateInit</method>
        */
        css::uno::Reference< css::accessibility::XAccessible >
                getAccessibleCreator( ) const;

    public:
        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override = 0;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override = 0;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override = 0;
        virtual OUString SAL_CALL getAccessibleName(  ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override = 0;

        // XAccessibleContext - default implementations
        /** default implementation for retrieving the index of this object within the parent
            <p>This basic implementation here returns the index <code>i</code> of the child for which
                <code>&lt;parent&gt;.getAccessibleChild( i )</code> equals our creator.</p>
        */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
        /** default implementation for retrieving the locale
            <p>This basic implementation returns the locale of the parent context,
            as retrieved via getAccessibleParent()->getAccessibleContext.</p>
        */
        virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    protected:
        // helper
        /** notifies all AccessibleEventListeners of a certain event

        @precond    not to be called with our mutex locked
        @param  _nEventId
            the id of the event. See AccessibleEventType
        @param  _rOldValue
            the old value to be notified
        @param  _rNewValue
            the new value to be notified
        */
        void NotifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                );

        // life time control
        /// checks whether the object is alive (returns <TRUE/> then) or disposed
        bool    isAlive() const;
        /// checks for being alive. If the object is already disposed (i.e. not alive), an exception is thrown.
        void        ensureAlive() const;

        /** ensures that the object is disposed.
        @precond
            to be called from within the destructor of your derived class only!
        */
        void        ensureDisposed( );

        /** shortcut for retrieving the context of the parent (returned by getAccessibleParent)
        */
        css::uno::Reference< css::accessibility::XAccessibleContext >
                    implGetParentContext();

        // access to the base class' broadcast helper/mutex
        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
    };


    //= OContextEntryGuard

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
    class OContextEntryGuard : public ::osl::ClearableMutexGuard
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
    };


    inline OContextEntryGuard::OContextEntryGuard( OAccessibleContextHelper* _pContext  )
        : ::osl::MutexGuard( _pContext->GetMutex() )
    {
        _pContext->ensureAlive();
    }


    //= OExternalLockGuard

    class OExternalLockGuard
            :public osl::Guard<SolarMutex>
            ,public OContextEntryGuard
    {
    public:
        inline OExternalLockGuard( OAccessibleContextHelper* _pContext );
    };


    inline OExternalLockGuard::OExternalLockGuard( OAccessibleContextHelper* _pContext )
        :osl::Guard<SolarMutex>( SolarMutex::get() )
        ,OContextEntryGuard( _pContext )
    {
        // Only lock the external mutex,
        // release the ::osl::Mutex of the OAccessibleContextHelper instance.
        // If you call into another UNO object with locked ::osl::Mutex,
        // this may lead to dead locks.
        clear();
    }


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLECONTEXTHELPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
