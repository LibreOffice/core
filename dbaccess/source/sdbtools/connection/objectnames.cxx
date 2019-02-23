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

#include "objectnames.hxx"
#include <core_resource.hxx>

#include <strings.hrc>

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>

#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlerror.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <memory>

namespace sdbtools
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;

    namespace CommandType = ::com::sun::star::sdb::CommandType;
    namespace ErrorCondition = ::com::sun::star::sdb::ErrorCondition;

    // INameValidation
    class INameValidation
    {
    public:
        virtual bool validateName( const OUString& _rName ) = 0;
        virtual void validateName_throw( const OUString& _rName ) = 0;

        virtual ~INameValidation() { }
    };
    typedef std::shared_ptr< INameValidation >   PNameValidation;

    // PlainExistenceCheck
    class PlainExistenceCheck : public INameValidation
    {
    private:
        Reference< XConnection >                m_xConnection;
        Reference< XNameAccess >                m_xContainer;

    public:
        PlainExistenceCheck( const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxContainer )
            :m_xConnection( _rxConnection )
            ,m_xContainer( _rxContainer )
        {
            OSL_ENSURE( m_xContainer.is(), "PlainExistenceCheck::PlainExistenceCheck: this will crash!" );
        }

        // INameValidation
        virtual bool validateName( const OUString& _rName ) override
        {
            return !m_xContainer->hasByName( _rName );
        }

        virtual void validateName_throw( const OUString& _rName ) override
        {
            if ( validateName( _rName ) )
                return;

            ::connectivity::SQLError aErrors;
            SQLException aError( aErrors.getSQLException( ErrorCondition::DB_OBJECT_NAME_IS_USED, m_xConnection, _rName ) );

            ::dbtools::DatabaseMetaData aMeta( m_xConnection );
            if ( aMeta.supportsSubqueriesInFrom() )
            {
                OUString sNeedDistinctNames( DBA_RES( STR_QUERY_AND_TABLE_DISTINCT_NAMES ) );
                aError.NextException <<= SQLException( sNeedDistinctNames, m_xConnection, OUString(), 0, Any() );
            }

            throw aError;
        }
    };

    // TableValidityCheck
    class TableValidityCheck : public INameValidation
    {
        const Reference< XConnection >        m_xConnection;

    public:
        TableValidityCheck( const Reference< XConnection >& _rxConnection )
            :m_xConnection( _rxConnection )
        {
        }

        virtual bool validateName( const OUString& _rName ) override
        {
            ::dbtools::DatabaseMetaData aMeta( m_xConnection );
            if  ( !aMeta.restrictIdentifiersToSQL92() )
                return true;

            OUString sCatalog, sSchema, sName;
            ::dbtools::qualifiedNameComponents(
                m_xConnection->getMetaData(), _rName, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InTableDefinitions );

            OUString sExtraNameCharacters( m_xConnection->getMetaData()->getExtraNameCharacters() );
            return !(   ( !sCatalog.isEmpty() && !::dbtools::isValidSQLName( sCatalog, sExtraNameCharacters ) )
                     || ( !sSchema.isEmpty() && !::dbtools::isValidSQLName( sSchema, sExtraNameCharacters ) )
                     || ( !sName.isEmpty() && !::dbtools::isValidSQLName( sName, sExtraNameCharacters ) ));
        }

        virtual void validateName_throw( const OUString& _rName ) override
        {
            if ( validateName( _rName ) )
                return;

            ::connectivity::SQLError aErrors;
            aErrors.raiseException( ErrorCondition::DB_INVALID_SQL_NAME, m_xConnection, _rName );
        }
    };

    // QueryValidityCheck
    class QueryValidityCheck : public INameValidation
    {
        const Reference< XConnection >          m_xConnection;

    public:
        QueryValidityCheck( const Reference< XConnection >& _rxConnection )
            :m_xConnection( _rxConnection )
        {
        }

        static ::connectivity::ErrorCondition validateName_getErrorCondition( const OUString& _rName )
        {
            if  (   ( _rName.indexOf( u'"'      ) >= 0 )
                ||  ( _rName.indexOf( u'\''     ) >= 0 )
                ||  ( _rName.indexOf( u'`'      ) >= 0 )
                ||  ( _rName.indexOf( u'\x0091' ) >= 0 )
                ||  ( _rName.indexOf( u'\x0092' ) >= 0 )
                ||  ( _rName.indexOf( u'\x00B4' ) >= 0 )  // removed unparsable chars
                )
                return ErrorCondition::DB_QUERY_NAME_WITH_QUOTES;

            if ( _rName.indexOf( '/') >= 0 )
                return ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES;

            return 0;
        }

        virtual bool validateName( const OUString& _rName ) override
        {
            return validateName_getErrorCondition( _rName ) == 0;
        }

        virtual void validateName_throw( const OUString& _rName ) override
        {
            ::connectivity::ErrorCondition nErrorCondition = validateName_getErrorCondition( _rName );
            if ( nErrorCondition != 0 )
            {
                ::connectivity::SQLError aErrors;
                aErrors.raiseException( nErrorCondition, m_xConnection );
            }
        }
    };

    // CombinedNameCheck
    class CombinedNameCheck : public INameValidation
    {
    private:
        PNameValidation  m_pPrimary;
        PNameValidation  m_pSecondary;

    public:
        CombinedNameCheck(const PNameValidation& _pPrimary, const PNameValidation& _pSecondary)
            :m_pPrimary( _pPrimary )
            ,m_pSecondary( _pSecondary )
        {
            OSL_ENSURE( m_pPrimary.get() && m_pSecondary.get(), "CombinedNameCheck::CombinedNameCheck: this will crash!" );
        }

        // INameValidation
        virtual bool validateName( const OUString& _rName ) override
        {
            return m_pPrimary->validateName( _rName ) && m_pSecondary->validateName( _rName );
        }

        virtual void validateName_throw( const OUString& _rName ) override
        {
            m_pPrimary->validateName_throw( _rName );
            m_pSecondary->validateName_throw( _rName );
        }
    };

    // NameCheckFactory
    class NameCheckFactory
    {
    public:
        NameCheckFactory(const NameCheckFactory&) = delete;
        const NameCheckFactory& operator=(const NameCheckFactory&) = delete;
        /** creates an INameValidation instance which can be used to check the existence of query or table names

            @param  _nCommandType
                the type of objects (CommandType::TABLE or CommandType::QUERY) of which names shall be checked for existence

            @param  _rxConnection
                the connection relative to which the names are to be checked. Must be an SDB-level connection

            @throws IllegalArgumentException
                if the given connection is no SDB-level connection

            @throws IllegalArgumentException
                if the given command type is neither CommandType::TABLE or CommandType::QUERY
        */
        static  PNameValidation  createExistenceCheck(
                    sal_Int32 _nCommandType,
                    const Reference< XConnection >& _rxConnection
                );

        /** creates an INameValidation instance which can be used to check the validity of a query or table name

            @param  _nCommandType
                the type of objects (CommandType::TABLE or CommandType::QUERY) of which names shall be validated

            @param  _rxConnection
                the connection relative to which the names are to be checked. Must be an SDB-level connection

            @throws IllegalArgumentException
                if the given connection is no SDB-level connection

            @throws IllegalArgumentException
                if the given command type is neither CommandType::TABLE or CommandType::QUERY
        */
        static  PNameValidation  createValidityCheck(
                    const sal_Int32 _nCommandType,
                    const Reference< XConnection >& _rxConnection
                );

    private:
        static  void    verifyCommandType( sal_Int32 _nCommandType );
    };

    void NameCheckFactory::verifyCommandType( sal_Int32 _nCommandType )
    {
        if  (   ( _nCommandType != CommandType::TABLE )
            &&  ( _nCommandType != CommandType::QUERY )
            )
            throw IllegalArgumentException(
                DBA_RES( STR_INVALID_COMMAND_TYPE ),
                nullptr,
                0
            );
    }

    PNameValidation  NameCheckFactory::createExistenceCheck( sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection )
    {
        verifyCommandType( _nCommandType );

        ::dbtools::DatabaseMetaData aMeta( _rxConnection );

        Reference< XNameAccess > xTables, xQueries;
        try
        {
            Reference< XTablesSupplier > xSuppTables( _rxConnection, UNO_QUERY_THROW );
            Reference< XQueriesSupplier > xQueriesSupplier( _rxConnection, UNO_QUERY_THROW );
            xTables.set( xSuppTables->getTables(), UNO_QUERY_THROW );
            xQueries.set( xQueriesSupplier->getQueries(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            throw IllegalArgumentException(
                DBA_RES( STR_CONN_WITHOUT_QUERIES_OR_TABLES ),
                nullptr,
                0
            );
        }

        PNameValidation pTableCheck( new PlainExistenceCheck( _rxConnection, xTables ) );
        PNameValidation pQueryCheck( new PlainExistenceCheck( _rxConnection, xQueries ) );
        PNameValidation pReturn;

        if ( aMeta.supportsSubqueriesInFrom() )
            pReturn.reset( new CombinedNameCheck( pTableCheck, pQueryCheck ) );
        else if ( _nCommandType == CommandType::TABLE )
            pReturn = pTableCheck;
        else
            pReturn = pQueryCheck;
        return pReturn;
    }

    PNameValidation  NameCheckFactory::createValidityCheck( sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection )
    {
        verifyCommandType( _nCommandType );

        Reference< XDatabaseMetaData > xMeta;
        try
        {
            xMeta.set( _rxConnection->getMetaData(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            throw IllegalArgumentException(
                "The connection could not provide its database's meta data.",
                nullptr,
                0
            );
        }

        if ( _nCommandType == CommandType::TABLE )
            return PNameValidation( new TableValidityCheck( _rxConnection ) );
        return PNameValidation( new QueryValidityCheck( _rxConnection ) );
    }

    // ObjectNames
    ObjectNames::ObjectNames( const Reference<XComponentContext>& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
    {
        setWeakConnection( _rxConnection );
    }

    ObjectNames::~ObjectNames()
    {
    }

    OUString SAL_CALL ObjectNames::suggestName( ::sal_Int32 CommandType, const OUString& BaseName )
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( CommandType, getConnection() ) );

        OUString sBaseName( BaseName );
        if ( sBaseName.isEmpty() )
        {
            if ( CommandType == CommandType::TABLE )
                sBaseName = DBA_RES(STR_BASENAME_TABLE);
            else
                sBaseName = DBA_RES(STR_BASENAME_QUERY);
        }
        else if( CommandType == CommandType::QUERY )
        {
            sBaseName=sBaseName.replace('/', '_');
        }

        OUString sName( sBaseName );
        sal_Int32 i = 1;
        while ( !pNameCheck->validateName( sName ) )
        {
            sName = sBaseName + " " + OUString::number(++i);
        }

        return sName;
    }

    OUString SAL_CALL ObjectNames::convertToSQLName( const OUString& Name )
    {
        EntryGuard aGuard( *this );
        Reference< XDatabaseMetaData > xMeta( getConnection()->getMetaData(), UNO_QUERY_THROW );
        return ::dbtools::convertName2SQLName( Name, xMeta->getExtraNameCharacters() );
    }

    sal_Bool SAL_CALL ObjectNames::isNameUsed( ::sal_Int32 CommandType, const OUString& Name )
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( CommandType, getConnection()) );
        return !pNameCheck->validateName( Name );
    }

    sal_Bool SAL_CALL ObjectNames::isNameValid( ::sal_Int32 CommandType, const OUString& Name )
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createValidityCheck( CommandType, getConnection()) );
        return pNameCheck->validateName( Name );
    }

    void SAL_CALL ObjectNames::checkNameForCreate( ::sal_Int32 CommandType, const OUString& Name )
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( CommandType, getConnection() ) );
        pNameCheck->validateName_throw( Name );

        pNameCheck = NameCheckFactory::createValidityCheck( CommandType, getConnection() );
        pNameCheck->validateName_throw( Name );
    }

} // namespace sdbtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
