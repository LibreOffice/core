/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef FORMS_ENTRYLISTHELPER_HXX
#define FORMS_ENTRYLISTHELPER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/form/binding/XListEntryListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>

//.........................................................................
namespace frm
{
//.........................................................................

    class OControlModel;
    class ControlModelLock;

    //=====================================================================
    //= OEntryListHelper
    //=====================================================================
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
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        m_aStringItems;     /// "overridden" StringItemList property value
        ::cppu::OInterfaceContainerHelper
                        m_aRefreshListeners;


    protected:
        OEntryListHelper( OControlModel& _rControlModel );
        OEntryListHelper( const OEntryListHelper& _rSource, OControlModel& _rControlModel );
        virtual ~OEntryListHelper( );

        /// returns the current string item list
        inline const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
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
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException) = 0;

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
        virtual void SAL_CALL setListEntrySource( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >& _rxSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource > SAL_CALL getListEntrySource(  ) throw (::com::sun::star::uno::RuntimeException);

        // XListEntryListener
        virtual void SAL_CALL entryChanged( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL entryRangeInserted( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL entryRangeRemoved( const ::com::sun::star::form::binding::ListEntryEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL allEntriesChanged( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XRefreshable
        virtual void SAL_CALL refresh() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRefreshListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);

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

//.........................................................................
}   // namespace frm
//.........................................................................


#endif // FORMS_ENTRYLISTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
