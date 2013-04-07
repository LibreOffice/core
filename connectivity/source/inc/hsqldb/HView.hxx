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

#ifndef CONNECTIVITY_HVIEW_HXX
#define CONNECTIVITY_HVIEW_HXX

#include "connectivity/sdbcx/VView.hxx"

#include <com/sun/star/sdbcx/XAlterView.hpp>

#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    //====================================================================
    //= HView
    //====================================================================
    typedef ::connectivity::sdbcx::OView                                HView_Base;
    typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XAlterView >  HView_IBASE;
    class HView :public HView_Base
                ,public HView_IBASE
    {
    public:
        HView(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            sal_Bool _bCaseSensitive,
            const OUString& _rSchemaName,
            const OUString& _rName
        );

        // UNO
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XAlterView
        virtual void SAL_CALL alterCommand( const OUString& NewCommand ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~HView();

    protected:
        // OPropertyContainer
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;

    private:
        /** retrieves the current command of the View

            @throws ::com::sun::star::lang::WrappedTargetException
                if an error occurs while retrieving the command from the database and
                <arg>_bAllowSQLExceptin</arg> is <FALSE/>
            @throws ::com::sun::star::sdbc::SQLException
                if an error occurs while retrieving the command from the database and
                <arg>_bAllowSQLException</arg> is <TRUE/>
        */
        OUString impl_getCommand_throw( bool _bAllowSQLException ) const;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
    private:
        using HView_Base::getFastPropertyValue;
    };

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

#endif // CONNECTIVITY_HVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
