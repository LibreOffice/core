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
#include "precompiled_dbaccess.hxx"

#include "objectnames.hxx"

#include "module_sdbt.hxx"
#include "sdbt_resource.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
/** === end UNO includes === **/

#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlerror.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace sdbtools
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/

    namespace CommandType = ::com::sun::star::sdb::CommandType;
    namespace ErrorCondition = ::com::sun::star::sdb::ErrorCondition;

    //====================================================================
    //= INameValidation
    //====================================================================
    class INameValidation
    {
    public:
        virtual bool validateName( const ::rtl::OUString& _rName ) = 0;
        virtual void validateName_throw( const ::rtl::OUString& _rName ) = 0;

        virtual ~INameValidation() { }
    };
    typedef ::boost::shared_ptr< INameValidation >   PNameValidation;

    //====================================================================
    //= PlainExistenceCheck
    //====================================================================
    class PlainExistenceCheck : public INameValidation
    {
    private:
        const ::comphelper::ComponentContext    m_aContext;
        Reference< XConnection >                m_xConnection;
        Reference< XNameAccess >                m_xContainer;

    public:
        PlainExistenceCheck( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxContainer )
            :m_aContext( _rContext )
            ,m_xConnection( _rxConnection )
            ,m_xContainer( _rxContainer )
        {
            OSL_ENSURE( m_xContainer.is(), "PlainExistenceCheck::PlainExistenceCheck: this will crash!" );
        }

        // INameValidation
        virtual bool validateName( const ::rtl::OUString& _rName )
        {
            return !m_xContainer->hasByName( _rName );
        }

        virtual void validateName_throw( const ::rtl::OUString& _rName )
        {
            if ( validateName( _rName ) )
                return;

            ::connectivity::SQLError aErrors( m_aContext );
            SQLException aError( aErrors.getSQLException( ErrorCondition::DB_OBJECT_NAME_IS_USED, m_xConnection, _rName ) );

            ::dbtools::DatabaseMetaData aMeta( m_xConnection );
            if ( aMeta.supportsSubqueriesInFrom() )
            {
                String sNeedDistinctNames( SdbtRes( STR_QUERY_AND_TABLE_DISTINCT_NAMES ) );
                aError.NextException <<= SQLException( sNeedDistinctNames, m_xConnection, ::rtl::OUString(), 0, Any() );
            }

            throw aError;
        }
    };

    //====================================================================
    //= TableValidityCheck
    //====================================================================
    class TableValidityCheck : public INameValidation
    {
        const ::comphelper::ComponentContext  m_aContext;
        const Reference< XConnection >        m_xConnection;

    public:
        TableValidityCheck( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection )
            :m_aContext( _rContext )
            ,m_xConnection( _rxConnection )
        {
        }

        virtual bool validateName( const ::rtl::OUString& _rName )
        {
            ::dbtools::DatabaseMetaData aMeta( m_xConnection );
            if  ( !aMeta.restrictIdentifiersToSQL92() )
                return true;

            ::rtl::OUString sCatalog, sSchema, sName;
            ::dbtools::qualifiedNameComponents(
                m_xConnection->getMetaData(), _rName, sCatalog, sSchema, sName, ::dbtools::eInTableDefinitions );

            ::rtl::OUString sExtraNameCharacters( m_xConnection->getMetaData()->getExtraNameCharacters() );
            if  (   ( sCatalog.getLength() && !::dbtools::isValidSQLName( sCatalog, sExtraNameCharacters ) )
                ||  ( sSchema.getLength() && !::dbtools::isValidSQLName( sSchema, sExtraNameCharacters ) )
                ||  ( sName.getLength() && !::dbtools::isValidSQLName( sName, sExtraNameCharacters ) )
                )
                return false;

            return true;
        }

        virtual void validateName_throw( const ::rtl::OUString& _rName )
        {
            if ( validateName( _rName ) )
                return;

            ::connectivity::SQLError aErrors( m_aContext );
            aErrors.raiseException( ErrorCondition::DB_INVALID_SQL_NAME, m_xConnection, _rName );
        }
    };

    //====================================================================
    //= QueryValidityCheck
    //====================================================================
    class QueryValidityCheck : public INameValidation
    {
        const ::comphelper::ComponentContext    m_aContext;
        const Reference< XConnection >          m_xConnection;

    public:
        QueryValidityCheck( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection )
            :m_aContext( _rContext )
            ,m_xConnection( _rxConnection )
        {
        }

        inline ::connectivity::ErrorCondition validateName_getErrorCondition( const ::rtl::OUString& _rName )
        {
            if  (   ( _rName.indexOf( (sal_Unicode)34  ) >= 0 )  // "
                ||  ( _rName.indexOf( (sal_Unicode)39  ) >= 0 )  // '
                ||  ( _rName.indexOf( (sal_Unicode)96  ) >= 0 )  //
                ||  ( _rName.indexOf( (sal_Unicode)145 ) >= 0 )  //
                ||  ( _rName.indexOf( (sal_Unicode)146 ) >= 0 )  //
                ||  ( _rName.indexOf( (sal_Unicode)180 ) >= 0 )  // removed unparsable chars
                )
                return ErrorCondition::DB_QUERY_NAME_WITH_QUOTES;

            if ( _rName.indexOf( '/') >= 0 )
                return ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES;

            return 0;
        }

        virtual bool validateName( const ::rtl::OUString& _rName )
        {
            if ( validateName_getErrorCondition( _rName ) != 0 )
                return false;
            return true;
        }

        virtual void validateName_throw( const ::rtl::OUString& _rName )
        {
            ::connectivity::ErrorCondition nErrorCondition = validateName_getErrorCondition( _rName );
            if ( nErrorCondition != 0 )
            {
                ::connectivity::SQLError aErrors( m_aContext );
                aErrors.raiseException( nErrorCondition, m_xConnection );
            }
        }
    };

    //====================================================================
    //= CombinedNameCheck
    //====================================================================
    class CombinedNameCheck : public INameValidation
    {
    private:
        PNameValidation  m_pPrimary;
        PNameValidation  m_pSecondary;

    public:
        CombinedNameCheck( PNameValidation _pPrimary, PNameValidation _pSecondary )
            :m_pPrimary( _pPrimary )
            ,m_pSecondary( _pSecondary )
        {
            OSL_ENSURE( m_pPrimary.get() && m_pSecondary.get(), "CombinedNameCheck::CombinedNameCheck: this will crash!" );
        }

        // INameValidation
        virtual bool validateName( const ::rtl::OUString& _rName )
        {
            return m_pPrimary->validateName( _rName ) && m_pSecondary->validateName( _rName );
        }

        virtual void validateName_throw( const ::rtl::OUString& _rName )
        {
            m_pPrimary->validateName_throw( _rName );
            m_pSecondary->validateName_throw( _rName );
        }
    };

    //====================================================================
    //= NameCheckFactory
    //====================================================================
    class NameCheckFactory
    {
    public:
        /** creates an INameValidation instance which can be used to check the existence of query or table names

            @param _rContext
                the component's context

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
                    const ::comphelper::ComponentContext& _rContext,
                    sal_Int32 _nCommandType,
                    const Reference< XConnection >& _rxConnection
                );

        /** creates an INameValidation instance which can be used to check the validity of a query or table name

            @param _rContext
                the component's context

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
                    const ::comphelper::ComponentContext& _rContext,
                    const sal_Int32 _nCommandType,
                    const Reference< XConnection >& _rxConnection
                );

    private:
        NameCheckFactory();                                     // never implemented

    private:
        static  void    verifyCommandType( sal_Int32 _nCommandType );
    };

    //--------------------------------------------------------------------
    void NameCheckFactory::verifyCommandType( sal_Int32 _nCommandType )
    {
        if  (   ( _nCommandType != CommandType::TABLE )
            &&  ( _nCommandType != CommandType::QUERY )
            )
            throw IllegalArgumentException(
                String( SdbtRes( STR_INVALID_COMMAND_TYPE ) ),
                NULL,
                0
            );
    }

    //--------------------------------------------------------------------
    PNameValidation  NameCheckFactory::createExistenceCheck( const ::comphelper::ComponentContext& _rContext, sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection )
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
                String( SdbtRes( STR_CONN_WITHOUT_QUERIES_OR_TABLES ) ),
                NULL,
                0
            );
        }

        PNameValidation pTableCheck( new PlainExistenceCheck( _rContext, _rxConnection, xTables ) );
        PNameValidation pQueryCheck( new PlainExistenceCheck( _rContext, _rxConnection, xQueries ) );
        PNameValidation pReturn;

        if ( aMeta.supportsSubqueriesInFrom() )
            pReturn.reset( new CombinedNameCheck( pTableCheck, pQueryCheck ) );
        else if ( _nCommandType == CommandType::TABLE )
            pReturn = pTableCheck;
        else
            pReturn = pQueryCheck;
        return pReturn;
    }

    //--------------------------------------------------------------------
    PNameValidation  NameCheckFactory::createValidityCheck( const ::comphelper::ComponentContext& _rContext, sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection )
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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The connection could not provide its database's meta data." ) ),
                NULL,
                0
            );
        }

        if ( _nCommandType == CommandType::TABLE )
            return PNameValidation( new TableValidityCheck( _rContext, _rxConnection ) );
        return PNameValidation( new QueryValidityCheck( _rContext, _rxConnection ) );
    }

    //====================================================================
    //= ObjectNames_Impl
    //====================================================================
    struct ObjectNames_Impl
    {
        SdbtClient  m_aModuleClient;    // keep the module alive as long as this instance lives
    };

    //====================================================================
    //= ObjectNames
    //====================================================================
    //--------------------------------------------------------------------
    ObjectNames::ObjectNames( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
        ,m_pImpl( new ObjectNames_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();
        setWeakConnection( _rxConnection );
    }

    //--------------------------------------------------------------------
    ObjectNames::~ObjectNames()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ObjectNames::suggestName( ::sal_Int32 _CommandType, const ::rtl::OUString& _BaseName ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( getContext(), _CommandType, getConnection() ) );

        String sBaseName( _BaseName );
        if ( sBaseName.Len() == 0 )
        {
            if ( _CommandType == CommandType::TABLE )
                sBaseName = String( SdbtRes( STR_BASENAME_TABLE ) );
            else
                sBaseName = String( SdbtRes( STR_BASENAME_QUERY ) );
        }

        ::rtl::OUString sName( sBaseName );
        sal_Int32 i = 1;
        while ( !pNameCheck->validateName( sName ) )
        {
            ::rtl::OUStringBuffer aNameBuffer;
            aNameBuffer.append( sBaseName );
            aNameBuffer.appendAscii( " " );
            aNameBuffer.append( (sal_Int32)++i );
            sName = aNameBuffer.makeStringAndClear();
        }

        return sName;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ObjectNames::convertToSQLName( const ::rtl::OUString& Name ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        Reference< XDatabaseMetaData > xMeta( getConnection()->getMetaData(), UNO_QUERY_THROW );
        return ::dbtools::convertName2SQLName( Name, xMeta->getExtraNameCharacters() );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL ObjectNames::isNameUsed( ::sal_Int32 _CommandType, const ::rtl::OUString& _Name ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( getContext(), _CommandType, getConnection()) );
        return !pNameCheck->validateName( _Name );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL ObjectNames::isNameValid( ::sal_Int32 _CommandType, const ::rtl::OUString& _Name ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createValidityCheck( getContext(), _CommandType, getConnection()) );
        return pNameCheck->validateName( _Name );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ObjectNames::checkNameForCreate( ::sal_Int32 _CommandType, const ::rtl::OUString& _Name ) throw (SQLException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( getContext(), _CommandType, getConnection() ) );
        pNameCheck->validateName_throw( _Name );

        pNameCheck = NameCheckFactory::createValidityCheck( getContext(), _CommandType, getConnection() );
        pNameCheck->validateName_throw( _Name );
    }

//........................................................................
} // namespace sdbtools
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
