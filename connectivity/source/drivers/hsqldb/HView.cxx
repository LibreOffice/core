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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "hsqldb/HView.hxx"
#include "hsqldb/HTools.hxx"

#include "propertyids.hxx"

#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
/** === end UNO includes === **/

#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    /** === begin UNO using === **/
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
    /** === end UNO using === **/

    //====================================================================
    //= HView
    //====================================================================
    //--------------------------------------------------------------------
    HView::HView( const Reference< XConnection >& _rxConnection, sal_Bool _bCaseSensitive,
        const ::rtl::OUString& _rSchemaName, const ::rtl::OUString& _rName )
        :HView_Base( _bCaseSensitive, _rName, _rxConnection->getMetaData(), 0, ::rtl::OUString(), _rSchemaName, ::rtl::OUString() )
        ,m_xConnection( _rxConnection )
    {
    }

    //--------------------------------------------------------------------
    HView::~HView()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( HView, HView_Base, HView_IBASE )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( HView, HView_Base, HView_IBASE )

    //--------------------------------------------------------------------
    void SAL_CALL HView::alterCommand( const ::rtl::OUString& _rNewCommand ) throw (SQLException, RuntimeException)
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

        ::rtl::OUString sQualifiedName( ::dbtools::composeTableName(
            m_xMetaData, m_CatalogName, m_SchemaName, m_Name, true, ::dbtools::eInDataManipulation ) );

        ::utl::SharedUNOComponent< XStatement > xStatement; xStatement.set( m_xConnection->createStatement(), UNO_QUERY_THROW );

        // create a statement which can be used to re-create the original view, in case
        // dropping it succeeds, but creating it with a new statement fails
        ::rtl::OUStringBuffer aRestoreCommand;
        aRestoreCommand.appendAscii( "CREATE VIEW " );
        aRestoreCommand.append     ( sQualifiedName );
        aRestoreCommand.appendAscii( " AS " );
        aRestoreCommand.append     ( impl_getCommand_throw( true ) );
        ::rtl::OUString sRestoreCommand( aRestoreCommand.makeStringAndClear() );

        bool bDropSucceeded( false );
        try
        {
            // drop the existing view
            ::rtl::OUStringBuffer aCommand;
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

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    ::rtl::OUString HView::impl_getCommand_throw( bool _bAllowSQLException ) const
    {
        ::rtl::OUString sCommand;

        try
        {
            ::rtl::OUStringBuffer aCommand;
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
