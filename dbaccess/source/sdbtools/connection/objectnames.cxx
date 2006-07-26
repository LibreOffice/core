/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectnames.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:47:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_OBJECTNAMES_HXX
#include "objectnames.hxx"
#endif

#ifndef DBACCESS_MODULE_SDBT_HXX
#include "module_sdbt.hxx"
#endif
#ifndef DBACCESS_SDBT_RESOURCE_HRC
#include "sdbt_resource.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
/** === end UNO includes === **/

#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

    namespace
    {
        //----------------------------------------------------------------
        void lcl_throwNameIsUsedError( const ::rtl::OUString& _Name, const Reference< XConnection >& _Connection )
        {
            String sNameIsUsed( SdbtRes( STR_NAME_ALREADY_USED_IN_DB ) );
            sNameIsUsed.SearchAndReplaceAllAscii( "$name$", _Name );

            Any aDetails;
            ::dbtools::DatabaseMetaData aMeta( _Connection );
            if ( aMeta.supportsSubqueriesInFrom() )
            {
                String sNeedDistinctNames( SdbtRes( STR_QUERY_AND_TABLE_DISTINCT_NAMES ) );
                aDetails <<= SQLException( sNeedDistinctNames, _Connection, ::rtl::OUString(), 0, Any() );
            }

            ::dbtools::throwGenericSQLException( sNameIsUsed, _Connection, aDetails );
        }

        //----------------------------------------------------------------
        void lcl_throwNoSQLIdentifierError( const ::rtl::OUString& _Name, const Reference< XConnection >& _Connection )
        {
            String sNoSQLIdentifier( SdbtRes( STR_NO_VALID_SQL_NAME ) );
            sNoSQLIdentifier.SearchAndReplaceAllAscii( "$name$", _Name );
            ::dbtools::throwGenericSQLException( sNoSQLIdentifier, _Connection, Any() );
        }
    }

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
        Reference< XConnection >    m_xConnection;
        Reference< XNameAccess >    m_xContainer;

    public:
        PlainExistenceCheck( const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxContainer )
            :m_xConnection( _rxConnection )
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
            if ( !validateName( _rName ) )
                lcl_throwNameIsUsedError( _rName, m_xConnection );
        }
    };

    //====================================================================
    //= TableValidityCheck
    //====================================================================
    class TableValidityCheck : public INameValidation
    {
        Reference< XConnection >  m_xConnection;

    public:
        TableValidityCheck( const Reference< XConnection >& _rxConnection )
            :m_xConnection( _rxConnection )
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
            if ( !validateName( _rName ) )
                lcl_throwNoSQLIdentifierError( _rName, m_xConnection );
        }
    };

    //====================================================================
    //= QueryValidityCheck
    //====================================================================
    class QueryValidityCheck : public INameValidation
    {
        Reference< XConnection >  m_xConnection;

    public:
        QueryValidityCheck( const Reference< XConnection >& _rxConnection )
            :m_xConnection( _rxConnection )
        {
        }

        virtual bool validateName( const ::rtl::OUString& _rName )
        {
            if  (   ( _rName.indexOf( (sal_Unicode)34  ) >= 0 )  // "
                ||  ( _rName.indexOf( (sal_Unicode)39  ) >= 0 )  // '
                ||  ( _rName.indexOf( (sal_Unicode)96  ) >= 0 )  // `
                ||  ( _rName.indexOf( (sal_Unicode)145 ) >= 0 )  // ‘
                ||  ( _rName.indexOf( (sal_Unicode)146 ) >= 0 )  // ’
                ||  ( _rName.indexOf( (sal_Unicode)180 ) >= 0 )  // ´
                )
                return false;
            return true;
        }

        virtual void validateName_throw( const ::rtl::OUString& _rName )
        {
            if ( !validateName( _rName ) )
            {
                String sError( SdbtRes( STR_NO_QUOTES_IN_QUERY_NAMES ) );
                ::dbtools::throwGenericSQLException( sError, m_xConnection );
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

            @param  _nCommandType
                the type of objects (CommandType::TABLE or CommandType::QUERY) of which names shall be checked for existence

            @param  _rxConnection
                the connection relative to which the names are to be checked. Must be an SDB-level connection

            @throws IllegalArgumentException
                if the given connection is no SDB-level connection

            @throws IllegalArgumentException
                if the given command type is neither CommandType::TABLE or CommandType::QUERY
        */
        static  PNameValidation  createExistenceCheck( sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection );

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
        static  PNameValidation  createValidityCheck( sal_Int32 _nCommandType, const Reference< XConnection >& _rxConnection );

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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Only com.sun.star.sdb.CommandType.TABLE and com.sun.star.sdb.CommandType.QUERY are allowed." ) ),
                NULL,
                0
            );
            // TODO: resource
    }

    //--------------------------------------------------------------------
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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The given connection is no valid query and/or tables supplier." ) ),
                NULL,
                0
            );
            // TODO: resource
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

    //--------------------------------------------------------------------
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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The connection could not provide its database's meta data." ) ),
                NULL,
                0
            );
            // TODO: resource
        }

        if ( _nCommandType == CommandType::TABLE )
            return PNameValidation( new TableValidityCheck( _rxConnection ) );
        return PNameValidation( new QueryValidityCheck( _rxConnection ) );
    }

    //====================================================================
    //= ObjectNames_Impl
    //====================================================================
    struct ObjectNames_Impl
    {
        SdbtClient      m_aModuleClient;    // keep the module alive as long as this instance lives
    };

    //====================================================================
    //= ObjectNames
    //====================================================================
    //--------------------------------------------------------------------
    ObjectNames::ObjectNames( const Reference< XConnection >& _rxConnection )
        :m_pImpl( new ObjectNames_Impl )
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

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( _CommandType, getConnection() ) );

        String sBaseName( _BaseName );
        if ( sBaseName.Len() == 0 )
            if ( _CommandType == CommandType::TABLE )
                sBaseName = String( SdbtRes( STR_BASENAME_TABLE ) );
            else
                sBaseName = String( SdbtRes( STR_BASENAME_QUERY ) );

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

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( _CommandType, getConnection()) );
        return !pNameCheck->validateName( _Name );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL ObjectNames::isNameValid( ::sal_Int32 _CommandType, const ::rtl::OUString& _Name ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createValidityCheck( _CommandType, getConnection()) );
        return pNameCheck->validateName( _Name );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ObjectNames::checkNameForCreate( ::sal_Int32 _CommandType, const ::rtl::OUString& _Name ) throw (SQLException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        PNameValidation pNameCheck( NameCheckFactory::createExistenceCheck( _CommandType, getConnection() ) );
        pNameCheck->validateName_throw( _Name );

        pNameCheck = NameCheckFactory::createValidityCheck( _CommandType, getConnection() );
        pNameCheck->validateName_throw( _Name );
    }

//........................................................................
} // namespace sdbtools
//........................................................................

