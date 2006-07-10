/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablename.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:20:15 $
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

#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX
#include "tablename.hxx"
#endif

#ifndef DBACCESS_MODULE_SDBT_HXX
#include "module_sdbt.hxx"
#endif
#ifndef DBACCESS_SHARED_SDBTSTRINGS_HRC
#include "sdbtstrings.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_TOOLS_COMPOSITIONTYPE_HPP_
#include <com/sun/star/sdb/tools/CompositionType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

//........................................................................
namespace sdbtools
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::beans::XPropertySetInfo;
    /** === end UNO using === **/

    namespace CompositionType = ::com::sun::star::sdb::tools::CompositionType;

    using namespace ::dbtools;

    //====================================================================
    //= TableName
    //====================================================================
    struct TableName_Impl
    {
        SdbtClient      m_aModuleClient;    // keep the module alive as long as this instance lives

        ::rtl::OUString sCatalog;
        ::rtl::OUString sSchema;
        ::rtl::OUString sName;
    };

    //====================================================================
    //= TableName
    //====================================================================
    //--------------------------------------------------------------------
    TableName::TableName( const Reference< XConnection >& _rxConnection )
        :m_pImpl( new TableName_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();

        setWeakConnection( _rxConnection );
    }

    //--------------------------------------------------------------------
    TableName::~TableName()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL TableName::getCatalogName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sCatalog;
    }

    //--------------------------------------------------------------------
    void SAL_CALL TableName::setCatalogName( const ::rtl::OUString& _catalogName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sCatalog = _catalogName;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL TableName::getSchemaName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sSchema;
    }

    //--------------------------------------------------------------------
    void SAL_CALL TableName::setSchemaName( const ::rtl::OUString& _schemaName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sSchema = _schemaName;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL TableName::getTableName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sName;
    }

    //--------------------------------------------------------------------
    void SAL_CALL TableName::setTableName( const ::rtl::OUString& _tableName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sName = _tableName;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL TableName::getNameForSelect() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return composeTableNameForSelect( getConnection(), m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName );
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > SAL_CALL TableName::getTable() throw (NoSuchElementException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        Reference< XTablesSupplier > xSuppTables( getConnection(), UNO_QUERY_THROW );
        Reference< XNameAccess > xTables( xSuppTables->getTables(), UNO_QUERY_THROW );

        Reference< XPropertySet > xTable;
        try
        {
            xTable.set( xTables->getByName( getComposedName( CompositionType::Complete, sal_False ) ), UNO_QUERY_THROW );
        }
        catch( const WrappedTargetException& )
        {
            throw NoSuchElementException();
        }
        catch( const RuntimeException& ) { throw; }
        catch( const NoSuchElementException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            throw NoSuchElementException();
        }

        return xTable;
    }

    //--------------------------------------------------------------------
    void SAL_CALL TableName::setTable( const Reference< XPropertySet >& _table ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        Reference< XPropertySetInfo > xPSI( _table, UNO_QUERY );
        if  (   !xPSI.is()
            ||  !xPSI->hasPropertyByName( PROPERTY_CATALOGNAME )
            ||  !xPSI->hasPropertyByName( PROPERTY_SCHEMANAME )
            ||  !xPSI->hasPropertyByName( PROPERTY_NAME )
            )
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The given object is no table object." ) ),
                *this,
                0
            );
            // TODO: resource

        try
        {
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_CATALOGNAME ) >>= m_pImpl->sCatalog );
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_SCHEMANAME ) >>= m_pImpl->sSchema );
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_NAME ) >>= m_pImpl->sName );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "An unknown error occured while examining the given table object." ) ),
                *this,
                0
            );
            // TODO: resource
        }
    }

    //--------------------------------------------------------------------
    namespace
    {
        /** translates a CopmositionType into a EComposeRule
            @throws IllegalArgumentException
                if the given value does not denote a valid CompositionType
        */
        EComposeRule lcl_translateCompositionType_throw( sal_Int32 _nType )
        {
            struct
            {
                sal_Int32       nCompositionType;
                EComposeRule    eComposeRule;
            }   TypeTable[] =
            {
                { CompositionType::ForTableDefinitions,      eInTableDefinitions },
                { CompositionType::ForIndexDefinitions,      eInIndexDefinitions },
                { CompositionType::ForDataManipulation,      eInDataManipulation },
                { CompositionType::ForProcedureCalls,        eInProcedureCalls },
                { CompositionType::ForPrivilegeDefinitions,  eInPrivilegeDefinitions },
                { CompositionType::ForPrivilegeDefinitions,  eComplete }
            };

            bool found = false;
            size_t i = 0;
            for ( ; ( i < sizeof( TypeTable ) / sizeof( TypeTable[0] ) ) && !found; ++i )
                if ( TypeTable[i].nCompositionType == _nType )
                    found = true;
            if ( !found )
                throw IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid composition type (not from com.sun.star.sdb.tools.CompositionType)" ) ),
                    NULL,
                    0
                );
                // TODO: resource

            return TypeTable[i].eComposeRule;
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL TableName::getComposedName( ::sal_Int32 _Type, ::sal_Bool _Quote ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        return composeTableName(
            getConnection()->getMetaData(),
            m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName, _Quote,
            lcl_translateCompositionType_throw( _Type ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL TableName::setComposedName( const ::rtl::OUString& _ComposedName, ::sal_Int32 _Type ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );

        qualifiedNameComponents(
            getConnection()->getMetaData(),
            _ComposedName,
            m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName,
            lcl_translateCompositionType_throw( _Type ) );
    }

//........................................................................
} // namespace sdbtools
//........................................................................

