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

#ifndef DBACCESS_OBJECTNAMEAPPROVAL_HXX
#define DBACCESS_OBJECTNAMEAPPROVAL_HXX

#include "containerapprove.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

namespace dbaccess
{

    //====================================================================
    //= ObjectNameApproval
    //====================================================================
    struct ObjectNameApproval_Impl;
    /** implementation of the IContainerApprove interface which approves
        elements for insertion into a query or tables container.

        The only check done by this instance is whether the query name is
        not already used, taking into account that in some databases, queries
        and tables share the same namespace.

        The class is not thread-safe.
    */
    class ObjectNameApproval : public IContainerApprove
    {
        ::std::auto_ptr< ObjectNameApproval_Impl >   m_pImpl;

    public:
        enum ObjectType
        {
            TypeQuery,
            TypeTable
        };

    public:
        /** constructs the instance

            @param _rxConnection
                the connection relative to which the names should be checked. This connection
                will be held weak. In case it is closed, subsequent calls to this instance's
                methods throw a DisposedException.
            @param _eType
                specifies which type of objects is to be approved with this instance
        */
        ObjectNameApproval(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            ObjectType _eType
        );
        virtual ~ObjectNameApproval();

        // IContainerApprove
        virtual void SAL_CALL approveElement( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    };

} // namespace dbaccess

#endif // DBACCESS_OBJECTNAMEAPPROVAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
