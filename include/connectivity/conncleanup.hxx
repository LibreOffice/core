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

#ifndef INCLUDED_CONNECTIVITY_CONNCLEANUP_HXX
#define INCLUDED_CONNECTIVITY_CONNCLEANUP_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <connectivity/dbtoolsdllapi.hxx>


namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::sdbc { class XRowSet; }
namespace com::sun::star::sdbc { class XConnection; }

namespace dbtools
{


    //= OAutoConnectionDisposer

    typedef ::cppu::WeakImplHelper <   css::beans::XPropertyChangeListener,
                                       css::sdbc::XRowSetListener
                                   >   OAutoConnectionDisposer_Base;

    class OOO_DLLPUBLIC_DBTOOLS OAutoConnectionDisposer : public OAutoConnectionDisposer_Base
    {
        css::uno::Reference< css::sdbc::XConnection >
                    m_xOriginalConnection;
        css::uno::Reference< css::sdbc::XRowSet > m_xRowSet; // needed to add as listener
        bool    m_bRSListening          : 1; // true when we're listening on rowset
        bool    m_bPropertyListening    : 1; // true when we're listening for property changes

    public:
        /** constructs an object
            <p>The connection given will be set on the rowset (as ActiveConnection), and the object adds itself as property
            change listener for the connection. Once somebody sets a new ActiveConnection, the old one (the one given
            here) will be disposed.</p>
        */
        OAutoConnectionDisposer(
            const css::uno::Reference< css::sdbc::XRowSet >& _rxRowSet,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
            );

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& _rEvent ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& _rSource ) override;

        // XRowSetListener
        virtual void SAL_CALL cursorMoved( const css::lang::EventObject& event ) override;
        virtual void SAL_CALL rowChanged( const css::lang::EventObject& event ) override;
        virtual void SAL_CALL rowSetChanged( const css::lang::EventObject& event ) override;

    private:
        void clearConnection();

        void        startRowSetListening();
        void        stopRowSetListening();
        bool        isRowSetListening() const { return m_bRSListening; }

        void        startPropertyListening( const css::uno::Reference< css::beans::XPropertySet >& _rxProps );
        void        stopPropertyListening( const css::uno::Reference< css::beans::XPropertySet >& _rxEventSource );
    };


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_CONNCLEANUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
