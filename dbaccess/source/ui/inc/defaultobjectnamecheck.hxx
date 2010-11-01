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

#ifndef DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX

#include "objectnamecheck.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <memory>
#include <boost/noncopyable.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= HierarchicalNameCheck
    //====================================================================
    struct HierarchicalNameCheck_Impl;
    /** class implementing the IObjectNameCheck interface, and checking given object names
        against a hierarchical name container
    */
    class HierarchicalNameCheck :public ::boost::noncopyable
                                ,public IObjectNameCheck
    {
    private:
        std::auto_ptr< HierarchicalNameCheck_Impl > m_pImpl;

    public:
        /** constructs a HierarchicalNameCheck instance
        @param _rxNames
            the hierarchic container of named objects, against which given names should be
            checked
        @param _rRelativeRoot
            the root in the hierarchy against which given names should be checked
        @throws ::com::sun::star::lang::IllegalArgumentException
            if the given container is <NULL/>
        */
        HierarchicalNameCheck(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& _rxNames,
            const ::rtl::OUString& _rRelativeRoot
        );

        ~HierarchicalNameCheck();

        // IObjectNameCheck overridables
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const;

    private:
        HierarchicalNameCheck();                                            // never implemented
    };

    //====================================================================
    //= DynamicTableOrQueryNameCheck
    //====================================================================
    struct DynamicTableOrQueryNameCheck_Impl;
    /** class implementing the IObjectNameCheck interface, and checking a given name
        for being valid as either a query or a table name.

        The class can be parametrized to act as either table name or query name validator.

        For databases which support queries in queries, the name check is implicitly extended
        to both queries and tables, no matter which category is checked. This prevents, for
        such databases, that users can create a query with the name of an existing table,
        or vice versa.

        @seealso dbtools::DatabaseMetaData::supportsSubqueriesInFrom
        @seealso com::sun::star::sdb::tools::XObjectNames::checkNameForCreate
    */
    class DynamicTableOrQueryNameCheck  :public ::boost::noncopyable
                                        ,public IObjectNameCheck
    {
    private:
        std::auto_ptr< DynamicTableOrQueryNameCheck_Impl > m_pImpl;

    public:
        /** constructs a DynamicTableOrQueryNameCheck instance
        @param _rxSdbLevelConnection
            a connection supporting the css.sdb.Connection service, in other word, it
            does expose the XTablesSupplier and XQueriesSupplier interfaces.
        @param _nCommandType
            specifies whether table names or query names should be checked. Only valid values
            are CommandType::TABLE and CommandType::QUERY.
        @throws ::com::sun::star::lang::IllegalArgumentException
            if the given connection is <NULL/>, or the given command type is neither
            CommandType::TABLE nor CommandType::QUERY.
        */
        DynamicTableOrQueryNameCheck(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxSdbLevelConnection,
            sal_Int32 _nCommandType
        );

        ~DynamicTableOrQueryNameCheck();

        // IObjectNameCheck overridables
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const;

    private:
        DynamicTableOrQueryNameCheck();                                                // never implemented
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_DEFAULTOBJECTNAMECHECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
