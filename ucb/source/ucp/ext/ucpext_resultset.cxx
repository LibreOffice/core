/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "precompiled_ucb.hxx"

#include "ucpext_resultset.hxx"
#include "ucpext_content.hxx"
#include "ucpext_datasupplier.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <ucbhelper/resultset.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::ucb::OpenCommandArgument2;
    using ::com::sun::star::ucb::XCommandEnvironment;
    /** === end UNO using === **/

    //==================================================================================================================
    //= ResultSet
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ResultSet::ResultSet( const Reference< XMultiServiceFactory >& i_rORB, const ::rtl::Reference< Content >& i_rContent,
            const OpenCommandArgument2& i_rCommand, const Reference< XCommandEnvironment >& i_rEnv )
        :ResultSetImplHelper( i_rORB, i_rCommand )
        ,m_xEnvironment( i_rEnv )
        ,m_xContent( i_rContent )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ResultSet::initStatic()
    {
        ::rtl::Reference< DataSupplier > pDataSupplier( new DataSupplier(
            m_xSMgr,
            m_xContent,
            m_aCommand.Mode
        ) );
        m_xResultSet1 = new ::ucbhelper::ResultSet(
            m_xSMgr,
            m_aCommand.Properties,
            pDataSupplier.get(),
            m_xEnvironment
        );
        pDataSupplier->fetchData();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ResultSet::initDynamic()
    {
        initStatic();
        m_xResultSet2 = m_xResultSet1;
    }

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
