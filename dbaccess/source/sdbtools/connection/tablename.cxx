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

#include "tablename.hxx"
#include "sdbt_resource.hrc"
#include "module_sdbt.hxx"
#include "sdbtstrings.hrc"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/sdb/tools/CompositionType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>
#include <tools/string.hxx>

namespace sdbtools
{

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
    using ::com::sun::star::uno::XComponentContext;

    namespace CompositionType = ::com::sun::star::sdb::tools::CompositionType;

    using namespace ::dbtools;

    // TableName
    struct TableName_Impl
    {
        SdbtClient      m_aModuleClient;    // keep the module alive as long as this instance lives

        OUString sCatalog;
        OUString sSchema;
        OUString sName;
    };

    // TableName
    TableName::TableName( const Reference<XComponentContext>& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
        ,m_pImpl( new TableName_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();

        setWeakConnection( _rxConnection );
    }

    TableName::~TableName()
    {
    }

    OUString SAL_CALL TableName::getCatalogName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sCatalog;
    }

    void SAL_CALL TableName::setCatalogName( const OUString& _catalogName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sCatalog = _catalogName;
    }

    OUString SAL_CALL TableName::getSchemaName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sSchema;
    }

    void SAL_CALL TableName::setSchemaName( const OUString& _schemaName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sSchema = _schemaName;
    }

    OUString SAL_CALL TableName::getTableName() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return m_pImpl->sName;
    }

    void SAL_CALL TableName::setTableName( const OUString& _tableName ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        m_pImpl->sName = _tableName;
    }

    OUString SAL_CALL TableName::getNameForSelect() throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        return composeTableNameForSelect( getConnection(), m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName );
    }

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
                OUString( SdbtRes( STR_NO_TABLE_OBJECT ) ),
                *this,
                0
            );

        try
        {
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_CATALOGNAME ) >>= m_pImpl->sCatalog );
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_SCHEMANAME ) >>= m_pImpl->sSchema );
            OSL_VERIFY( _table->getPropertyValue( PROPERTY_NAME ) >>= m_pImpl->sName );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& e )
        {
            throw IllegalArgumentException( e.Message, e.Context, 0 );
        }
    }

    namespace
    {
        /** translates a CompositionType into a EComposeRule
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
                    OUString( SdbtRes( STR_INVALID_COMPOSITION_TYPE ) ),
                    NULL,
                    0
                );

            return TypeTable[i].eComposeRule;
        }
    }

    OUString SAL_CALL TableName::getComposedName( ::sal_Int32 _Type, ::sal_Bool _Quote ) throw (IllegalArgumentException, RuntimeException)
    {
        EntryGuard aGuard( *this );

        return composeTableName(
            getConnection()->getMetaData(),
            m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName, _Quote,
            lcl_translateCompositionType_throw( _Type ) );
    }

    void SAL_CALL TableName::setComposedName( const OUString& _ComposedName, ::sal_Int32 _Type ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );

        qualifiedNameComponents(
            getConnection()->getMetaData(),
            _ComposedName,
            m_pImpl->sCatalog, m_pImpl->sSchema, m_pImpl->sName,
            lcl_translateCompositionType_throw( _Type ) );
    }

} // namespace sdbtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
