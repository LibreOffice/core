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

#ifndef _CONNECTIVITY_CONNCLEANUP_HXX_
#define _CONNECTIVITY_CONNCLEANUP_HXX_

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=====================================================================
    //= OAutoConnectionDisposer
    //=====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::beans::XPropertyChangeListener,
                                        ::com::sun::star::sdbc::XRowSetListener
                                    >   OAutoConnectionDisposer_Base;

    class OOO_DLLPUBLIC_DBTOOLS OAutoConnectionDisposer : public OAutoConnectionDisposer_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    m_xOriginalConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > m_xRowSet; // needed to add as listener
        sal_Bool    m_bRSListening          : 1; // true when we're listening on rowset
        sal_Bool    m_bPropertyListening    : 1; // true when we're listening for property changes

    public:
        /** constructs an object
            <p>The connection given will be set on the rowset (as ActiveConnection), and the object adds itself as property
            change listener for the connection. Once somebody sets a new ActiveConnection, the old one (the one given
            here) will be disposed.</p>
        */
        OAutoConnectionDisposer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
            );

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XRowSetListener
        virtual void SAL_CALL cursorMoved( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowSetChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);

    private:
        void clearConnection();

        void        startRowSetListening();
        void        stopRowSetListening();
        sal_Bool    isRowSetListening() const { return m_bRSListening; }

        void        startPropertyListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps );
        void        stopPropertyListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxEventSource );
        sal_Bool    isPropertyListening() const { return m_bPropertyListening; }
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_CONNCLEANUP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
