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


#include "hsqldb/HView.hxx"
#include "hsqldb/HTools.hxx"

#include "propertyids.hxx"

#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"

#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XStatement;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::sdbc::XRow;

    //====================================================================
    //= HView
    //====================================================================

    HView::HView( const Reference< XConnection >& _rxConnection, sal_Bool _bCaseSensitive,
        const OUString& _rSchemaName, const OUString& _rName )
        :HView_Base( _bCaseSensitive, _rName, _rxConnection->getMetaData(), 0, OUString(), _rSchemaName, OUString() )
        ,m_xConnection( _rxConnection )
    {
    }


    HView::~HView()
    {
    }


    IMPLEMENT_FORWARD_XINTERFACE2( HView, HView_Base, HView_IBASE )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( HView, HView_Base, HView_IBASE )


    void SAL_CALL HView::alterCommand( const OUString& _rNewCommand ) throw (SQLException, RuntimeException)
    {
        // not really atomic ... as long as we do not have something like
        //   ALTER VIEW <name> TO <command>
        // in HSQL, we need to do it this way ...
        //
        // I can imagine scenarios where this fails, e.g. when dropping the view
        // succeedes, re-creating it fails, some other thread alters a table which
        // the view was based on, and then we try to restore the view with the
        // original command, which then fails, too.
        //
        // However, there's not much chance to prevent this kind of errors without
        // backend support.

        OUString sQualifiedName( ::dbtools::composeTableName(
            m_xMetaData, m_CatalogName, m_SchemaName, m_Name, true, ::dbtools::eInDataManipulation ) );

        ::utl::SharedUNOComponent< XStatement > xStatement; xStatement.set( m_xConnection->createStatement(), UNO_QUERY_THROW );

        // create a statement which can be used to re-create the original view, in case
        // dropping it succeeds, but creating it with a new statement fails
        OUStringBuffer aRestoreCommand;
        aRestoreCommand.appendAscii( "CREATE VIEW " );
        aRestoreCommand.append     ( sQualifiedName );
        aRestoreCommand.appendAscii( " AS " );
        aRestoreCommand.append     ( impl_getCommand_throw( true ) );
        OUString sRestoreCommand( aRestoreCommand.makeStringAndClear() );

        bool bDropSucceeded( false );
        try
        {
            // drop the existing view
            OUStringBuffer aCommand;
            aCommand.appendAscii( "DROP VIEW " );
            aCommand.append     ( sQualifiedName );
            xStatement->execute( aCommand.makeStringAndClear() );
            bDropSucceeded = true;

            // create a new one with the same name
            aCommand.appendAscii( "CREATE VIEW " );
            aCommand.append     ( sQualifiedName );
            aCommand.appendAscii( " AS " );
            aCommand.append     ( _rNewCommand );
            xStatement->execute( aCommand.makeStringAndClear() );
        }
        catch( const SQLException& )
        {
            if ( bDropSucceeded )
                // drop succeeded, but creation failed -> re-create the view with the original
                // statemnet
                xStatement->execute( sRestoreCommand );
            throw;
        }
        catch( const RuntimeException& )
        {
            if ( bDropSucceeded )
                xStatement->execute( sRestoreCommand );
            throw;
        }
        catch( const Exception& )
        {
            if ( bDropSucceeded )
                xStatement->execute( sRestoreCommand );
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void SAL_CALL HView::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        if ( _nHandle == PROPERTY_ID_COMMAND )
        {
            // retrieve the very current command, don't rely on the base classes cached value
            // (which we initialized empty, anyway)
            _rValue <<= impl_getCommand_throw( false );
            return;
        }

        HView_Base::getFastPropertyValue( _rValue, _nHandle );
    }


    OUString HView::impl_getCommand_throw( bool _bAllowSQLException ) const
    {
        OUString sCommand;

        try
        {
            OUStringBuffer aCommand;
            aCommand.appendAscii( "SELECT VIEW_DEFINITION FROM INFORMATION_SCHEMA.SYSTEM_VIEWS " );
            HTools::appendTableFilterCrit( aCommand, m_CatalogName, m_SchemaName, m_Name, false );
            ::utl::SharedUNOComponent< XStatement > xStatement; xStatement.set( m_xConnection->createStatement(), UNO_QUERY_THROW );
            Reference< XResultSet > xResult( xStatement->executeQuery( aCommand.makeStringAndClear() ), UNO_QUERY_THROW );
            if ( !xResult->next() )
            {
                // hmm. There is no view view the name as we know it. Can only mean some other instance
                // dropped this view meanwhile ...
                throw DisposedException();
            }

            Reference< XRow > xRow( xResult, UNO_QUERY_THROW );
            sCommand = xRow->getString( 1 );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const SQLException& e )
        {
            if ( _bAllowSQLException )
                throw;
            throw WrappedTargetException( e.Message, static_cast< XAlterView* >( const_cast< HView* >( this ) ), ::cppu::getCaughtException() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sCommand;
    }

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
