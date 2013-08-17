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

#include "objectnameapproval.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/exc_hlp.hxx>

namespace dbaccess
{

    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::WeakReference;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::sdb::tools::XConnectionTools;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdb::tools::XObjectNames;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::sdbc::SQLException;

    namespace CommandType = com::sun::star::sdb::CommandType;

    // ObjectNameApproval_Impl
    struct ObjectNameApproval_Impl
    {
        WeakReference< XConnection >        aConnection;
        sal_Int32                           nCommandType;
    };

    // ObjectNameApproval
    ObjectNameApproval::ObjectNameApproval( const Reference< XConnection >& _rxConnection, ObjectType _eType )
        :m_pImpl( new ObjectNameApproval_Impl )
    {
        m_pImpl->aConnection = _rxConnection;
        m_pImpl->nCommandType = _eType == TypeQuery ? CommandType::QUERY : CommandType::TABLE;
    }

    ObjectNameApproval::~ObjectNameApproval()
    {
    }

    void SAL_CALL ObjectNameApproval::approveElement( const OUString& _rName, const Reference< XInterface >& /*_rxElement*/ )
    {
        Reference< XConnection > xConnection( m_pImpl->aConnection );
        if ( !xConnection.is() )
            throw DisposedException();

        Reference< XConnectionTools > xConnectionTools( xConnection, UNO_QUERY_THROW );
        Reference< XObjectNames > xObjectNames( xConnectionTools->getObjectNames(), UNO_QUERY_THROW );
        xObjectNames->checkNameForCreate( m_pImpl->nCommandType, _rName );
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
