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

#include "ucpext_resultset.hxx"
#include "ucpext_content.hxx"
#include "ucpext_datasupplier.hxx"

#include <ucbhelper/resultset.hxx>


namespace ucb::ucp::ext
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::ucb::OpenCommandArgument2;
    using ::com::sun::star::ucb::XCommandEnvironment;


    //= ResultSet


    ResultSet::ResultSet( const Reference< XComponentContext >& rxContext, const ::rtl::Reference< Content >& i_rContent,
            const OpenCommandArgument2& i_rCommand, const Reference< XCommandEnvironment >& i_rEnv )
        :ResultSetImplHelper( rxContext, i_rCommand )
        ,m_xEnvironment( i_rEnv )
        ,m_xContent( i_rContent )
    {
    }


    void ResultSet::initStatic()
    {
        ::rtl::Reference< DataSupplier > pDataSupplier( new DataSupplier(
            m_xContext,
            m_xContent
        ) );
        m_xResultSet1 = new ::ucbhelper::ResultSet(
            m_xContext,
            m_aCommand.Properties,
            pDataSupplier,
            m_xEnvironment
        );
        pDataSupplier->fetchData();
    }


    void ResultSet::initDynamic()
    {
        initStatic();
        m_xResultSet2 = m_xResultSet1;
    }


} // namespace ucb::ucp::ext


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
