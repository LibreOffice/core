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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_ENTRYLISTHELPER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_ENTRYLISTHELPER_HXX

#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/form/binding/XListEntryListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>


namespace frm
{


    class OControlModel;
    class ControlModelLock;


    //= OEntryListHelper

    typedef ::cppu::ImplHelper3 <      ::com::sun::star::form::binding::XListEntrySink
                                ,   ::com::sun::star::form::binding::XListEntryListener
                                ,   ::com::sun::star::util::XRefreshable
                                >   OEntryListHelper_BASE;

    class OEntryListHelper : public OEntryListHelper_BASE
    {
    private:
        OControlModel&  m_rControlModel;

        ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >
                        m_xListSource;      /// our external list source
        ::com::sun::star::uno::Sequence< OUString >
                        m_aStringItems;     /// "overridden" StringItemList property value
        ::cppu::OInterfaceContainerHelper
                        m_aRefreshListeners;


    protected:
        OEntryListHelper( OControlModel& _rControlModel );
        OEntryListHelper( const OEntryListHelper& _rSource, OControlModel& _rControlModel );
        virtual ~OEntryListHelper( );

        /// returns the current string item list
        inline const ::com::sun::star::uno::Sequence< OUString >&
                    getStringItemList() const { return m_aStringItems; }
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >&
                    getExternalListEntrySource() const { return m_xListSource; }

        /// determines whether we actually have an external list source
        inline bool hasExternalListSource( ) const { return m_xListSource.is(); }

        /** handling the XEventListener::disposing call for the case where
            our list source is being disposed
            @return
                <TRUE/> if and only if the disposed object was our list source, and so the
                event was handled
        */
        bool        handleDisposing( const ::com::sun::star::lang::EventObject& _rEvent );

        /** to be called by derived classes' instances when they're being disposed
        */
        void        disposing( );

        // prevent method hiding
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) = 0;

        /** helper for implementing convertFastPropertyValue( StringItemList )

            <p>The signature of this method and the return type have the same semantics
            as convertFastPropertyValue.</p>
        */
        sal_Bool    convertNewListSourceProperty(
                        ::com::sun::star::uno::Any& _rConvertedValue,
                        ::com::sun::star::uno::Any& _rOldValue,
                        const ::com::sun::star::uno::Any& _rValue
                    )
                    SAL_THROW( ( ::com::sun::star::lang::IllegalArgumentException ) );

        /** helper for implementing setFastPropertyValueNoBroadcast

            <p>Will internally call stringItemListChanged after the new item list
            has been set.</p>

            @precond
                not to be called when we have an external list source
            @see hasExternalListSource
        */
        void        setNewStringItemList( const ::com::sun::star::uno::Any& _rValue, ControlModelLock& _rInstanceLock );

        /** announces that the list of entries has changed.

            <p>Derived classes have to override this. Most probably, they'll set the new
            as model property.</p>

            @pure
            @see getStringItemList
        */
        virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock ) = 0;

        /** called whenever a connection to a new external list source has been established
        */
        virtual void    connectedExternalListSource( );

        /** called whenever a connection to a new external list source has been revoked
        */
        virtual void    disconnectedExternalListSource( );

        /** called when XRefreshable::refresh has been called, and we do *not* have an external
            list source
        */
        virtual void    refreshInternalEntryList() = 0;

    private:
        // XListEntrySink
        virtual void SAL_CALL setListEntrySource( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >& _rxSource ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource > SAL_CALL getListEntrySource(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XListEntryListener
        virtual void SAL_CALL entryChanged( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL entryRangeInserted( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL entryRangeRemoved( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL allEntriesChanged( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XRefreshable
        virtual void SAL_CALL refresh() throw(::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception);

    private:
        /** disconnects from the active external list source, if present
            @see connectExternalListSource
        */
        void        disconnectExternalListSource( );

        /** connects to a new external list source
            @param _rxSource
                the new list source. Must not be <NULL/>
            @see disconnectExternalListSource
        */
        void        connectExternalListSource(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >& _rxSource,
                        ControlModelLock& _rInstanceLock
                    );

        /** refreshes our list entries

            In case we have an external list source, its used to obtain the new entries, and then
            stringItemListChanged is called to give the derived class the possibility to
            react on this.

            In case we do not have an external list source, refreshInternalEntryList is called.
        */
        void        impl_lock_refreshList( ControlModelLock& _rInstanceLock );

    private:
        OEntryListHelper();                                     // never implemented
        OEntryListHelper( const OEntryListHelper& );            // never implemented
        OEntryListHelper& operator=( const OEntryListHelper& ); // never implemented
    };


}   // namespace frm



#endif // INCLUDED_FORMS_SOURCE_COMPONENT_ENTRYLISTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
