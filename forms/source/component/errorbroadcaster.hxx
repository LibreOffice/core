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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_ERRORBROADCASTER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_ERRORBROADCASTER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>


namespace frm
{

    typedef ::cppu::ImplHelper  <   css::sdb::XSQLErrorBroadcaster
                                >   OErrorBroadcaster_BASE;

    class OErrorBroadcaster : public OErrorBroadcaster_BASE
    {
    private:
        ::cppu::OBroadcastHelper&           m_rBHelper;
        ::comphelper::OInterfaceContainerHelper2  m_aErrorListeners;

    protected:
        explicit OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper );
        virtual ~OErrorBroadcaster( );

        void SAL_CALL disposing();

        void SAL_CALL onError( const css::sdbc::SQLException& _rException, const OUString& _rContextDescription );
        void SAL_CALL onError( const css::sdb::SQLErrorEvent& _rException );

    protected:
    // XSQLErrorBroadcaster
        virtual void SAL_CALL addSQLErrorListener( const css::uno::Reference< css::sdb::XSQLErrorListener >& _rListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSQLErrorListener( const css::uno::Reference< css::sdb::XSQLErrorListener >& _rListener ) throw (css::uno::RuntimeException, std::exception) override;
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_ERRORBROADCASTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
