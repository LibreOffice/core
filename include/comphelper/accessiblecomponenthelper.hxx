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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX

#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext2.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{

    //= OAccessibleComponentHelper

    /** a helper class for implementing an AccessibleContext which at the same time
        supports an XAccessibleExtendedComponent interface.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleComponentHelper
        : public cppu::BaseMutex,
          public cppu::WeakComponentImplHelper<css::accessibility::XAccessibleContext2,
                                               css::accessibility::XAccessibleEventBroadcaster,
                                               css::accessibility::XAccessibleExtendedComponent>
    {
        friend class OContextEntryGuard;
    private:
        AccessibleEventNotifier::TClientId m_nClientId;

    protected:
        virtual ~OAccessibleComponentHelper( ) override;

        OAccessibleComponentHelper( );

    private:
        OAccessibleComponentHelper(OAccessibleComponentHelper const &) = delete;
        OAccessibleComponentHelper(OAccessibleComponentHelper &&) = delete;
        void operator =(OAccessibleComponentHelper const &) = delete;
        void operator =(OAccessibleComponentHelper &&) = delete;

    public:
        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
        virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override = 0;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override = 0;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override = 0;
        virtual OUString SAL_CALL getAccessibleName(  ) override = 0;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override = 0;
        virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override = 0;

        // XAccessibleContext2 - default implementation
        virtual OUString SAL_CALL getAccessibleId(  ) override;

        // XAccessibleContext - default implementations
        /** default implementation for retrieving the index of this object within the parent
            <p>This basic implementation here returns the index <code>i</code> of the child for which
                <code>&lt;parent&gt;.getAccessibleChild(i).getAccessibleContext()</code> returns
                a reference to this OAccessibleComponentHelper object.</p>
        */
        virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
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
                    const css::uno::Any& _rNewValue,
                    sal_Int32 nIndexHint = -1
                );

        /// returns whether any accessible listeners are registered
        bool hasAccessibleListeners() const;

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

    protected:
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        ///
        /// @throws css::uno::RuntimeException
        virtual css::awt::Rectangle implGetBounds(  ) = 0;


    public:
        // XAccessibleComponent - default implementations which can be implemented using <method>implGetBounds</method>
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override final;
        virtual css::awt::Point SAL_CALL getLocation(  ) override final;
        /* note: getLocationOnScreen relies on a valid parent (XAccessibleContext::getParent()->getAccessibleContext()),
           which itself implements XAccessibleComponent

            @throws css::uno::RuntimeException
        */
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
        virtual css::awt::Size SAL_CALL getSize(  ) override final;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) override final;

        // XAccessibleExtendedComponent - default implementations returning empty strings
        virtual OUString SAL_CALL getTitledBorderText() override;
        virtual OUString SAL_CALL getToolTipText() override;
    };

}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
