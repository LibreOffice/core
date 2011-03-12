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

#ifndef CONNECTIVITY_HVIEW_HXX
#define CONNECTIVITY_HVIEW_HXX

#include "connectivity/sdbcx/VView.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdbcx/XAlterView.hpp>
/** === end UNO includes === **/

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
            const ::rtl::OUString& _rSchemaName,
            const ::rtl::OUString& _rName
        );

        // UNO
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XAlterView
        virtual void SAL_CALL alterCommand( const ::rtl::OUString& NewCommand ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

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
        ::rtl::OUString impl_getCommand_throw( bool _bAllowSQLException ) const;

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
