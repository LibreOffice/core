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


#include "cachedrowset.hxx"
#include "services.hxx"
#include "frm_strings.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>

#include <tools/diagnose_ex.h>


namespace frm
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XStatement;

    namespace ResultSetType = ::com::sun::star::sdbc::ResultSetType;

    struct CachedRowSet_Data
    {
        OUString                 sCommand;
        bool                        bEscapeProcessing;
        Reference< XConnection >        xConnection;

        bool                            bStatementDirty;

        CachedRowSet_Data()
            :sCommand()
            ,bEscapeProcessing( false )
            ,xConnection()
            ,bStatementDirty( true )
        {
        }
    };

    CachedRowSet::CachedRowSet()
        :m_pData( new CachedRowSet_Data )
    {
    }


    CachedRowSet::~CachedRowSet()
    {
        dispose();
    }


    void CachedRowSet::setCommand( const OUString& _rCommand )
    {
        if ( m_pData->sCommand == _rCommand )
            return;

        m_pData->sCommand = _rCommand;
        m_pData->bStatementDirty = true;
    }


    void CachedRowSet::setCommandFromQuery( const OUString& _rQueryName )
    {
        Reference< XQueriesSupplier > xSupplyQueries( m_pData->xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess >      xQueries      ( xSupplyQueries->getQueries(), UNO_QUERY_THROW );
        Reference< XPropertySet >     xQuery        ( xQueries->getByName( _rQueryName ), UNO_QUERY_THROW );

        bool bEscapeProcessing( false );
        OSL_VERIFY( xQuery->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
        setEscapeProcessing( bEscapeProcessing );

        OUString sCommand;
        OSL_VERIFY( xQuery->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
        setCommand( sCommand );
    }


    void CachedRowSet::setEscapeProcessing ( const bool _bEscapeProcessing )
    {
        if ( m_pData->bEscapeProcessing == _bEscapeProcessing )
            return;

        m_pData->bEscapeProcessing = _bEscapeProcessing;
        m_pData->bStatementDirty = true;
    }


    void CachedRowSet::setConnection( const Reference< XConnection >& _rxConnection )
    {
        if ( m_pData->xConnection == _rxConnection )
            return;

        m_pData->xConnection = _rxConnection;
        m_pData->bStatementDirty = true;
    }


    Reference< XResultSet > CachedRowSet::execute()
    {
        Reference< XResultSet > xResult;
        try
        {
            OSL_PRECOND( m_pData->xConnection.is(), "CachedRowSet::execute: how am I expected to do this without a connection?" );
            if ( !m_pData->xConnection.is() )
                return xResult;

            Reference< XStatement > xStatement( m_pData->xConnection->createStatement(), UNO_SET_THROW );
            Reference< XPropertySet > xStatementProps( xStatement, UNO_QUERY_THROW );
            xStatementProps->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, makeAny( m_pData->bEscapeProcessing ) );
            xStatementProps->setPropertyValue( PROPERTY_RESULTSET_TYPE, makeAny( ResultSetType::FORWARD_ONLY ) );

            xResult.set( xStatement->executeQuery( m_pData->sCommand ), UNO_SET_THROW );
            m_pData->bStatementDirty = false;
        }
        catch( const SQLException& )
        {
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xResult;
    }


    bool CachedRowSet::isDirty() const
    {
        return m_pData->bStatementDirty;
    }


    void CachedRowSet::dispose()
    {
        try
        {
            m_pData.reset( new CachedRowSet_Data );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
