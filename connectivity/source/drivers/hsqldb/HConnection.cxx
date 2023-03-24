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


#include <hsqldb/HConnection.hxx>
#include <hsqldb/HTools.hxx>
#include <bitmaps.hlst>

#include <connectivity/dbtools.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <resource/sharedresources.hxx>
#include <strings.hrc>

using ::com::sun::star::util::XFlushListener;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::sdbc::XStatement;
using ::com::sun::star::sdbc::XConnection;
using ::com::sun::star::sdbcx::XDataDefinitionSupplier;
using ::com::sun::star::sdbcx::XTablesSupplier;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::sdbc::XDriver;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::GraphicProvider;
using ::com::sun::star::graphic::XGraphicProvider;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::sdbc::XResultSet;
using ::com::sun::star::sdbc::XDatabaseMetaData;
using ::com::sun::star::sdbc::XDatabaseMetaData2;
using ::com::sun::star::sdbc::XRow;
using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
using ::com::sun::star::beans::PropertyValue;


namespace connectivity::hsqldb
{
    void SAL_CALL OHsqlConnection::disposing()
    {
        m_aFlushListeners.disposeAndClear( EventObject( *this ) );
        OHsqlConnection_BASE::disposing();
        OConnectionWrapper::disposing();
    }

    OHsqlConnection::OHsqlConnection( const Reference< XDriver >& _rxDriver,
        const Reference< XConnection >& _xConnection ,const Reference< XComponentContext >& _rxContext )
        :OHsqlConnection_BASE( m_aMutex )
        ,m_aFlushListeners( m_aMutex )
        ,m_xDriver( _rxDriver )
        ,m_xContext( _rxContext )
        ,m_bIni(true)
        ,m_bReadOnly(false)
    {
        setDelegation(_xConnection,_rxContext,m_refCount);
    }

    OHsqlConnection::~OHsqlConnection()
    {
        if ( !OHsqlConnection_BASE::rBHelper.bDisposed )
        {
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }

    IMPLEMENT_FORWARD_XINTERFACE2(OHsqlConnection,OHsqlConnection_BASE,OConnectionWrapper)
    IMPLEMENT_SERVICE_INFO(OHsqlConnection, "com.sun.star.sdbc.drivers.hsqldb.OHsqlConnection", "com.sun.star.sdbc.Connection")
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(OHsqlConnection,OHsqlConnection_BASE,OConnectionWrapper)


    ::osl::Mutex& OHsqlConnection::getMutex() const
    {
        return m_aMutex;
    }


    void OHsqlConnection::checkDisposed() const
    {
        ::connectivity::checkDisposed( rBHelper.bDisposed );
    }

    // XFlushable

    void SAL_CALL OHsqlConnection::flush(  )
    {
        MethodGuard aGuard( *this );

        try
        {
            if ( m_xConnection.is() )
            {
                if ( m_bIni )
                {
                    m_bIni = false;
                    Reference< XDatabaseMetaData2 > xMeta2(m_xConnection->getMetaData(),UNO_QUERY_THROW);
                    const Sequence< PropertyValue > aInfo = xMeta2->getConnectionInfo();
                    const PropertyValue* pIter = aInfo.getConstArray();
                    const PropertyValue* pEnd  = pIter + aInfo.getLength();
                    for(;pIter != pEnd;++pIter)
                    {
                        if ( pIter->Name == "readonly" )
                            m_bReadOnly = true;
                    }
                }
                try
                {
                    if ( !m_bReadOnly )
                    {
                        Reference< XStatement > xStmt( m_xConnection->createStatement(), css::uno::UNO_SET_THROW );
                        xStmt->execute( "CHECKPOINT DEFRAG" );
                    }
                }
                catch(const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
                }
            }

            EventObject aFlushedEvent( *this );
            m_aFlushListeners.notifyEach( &XFlushListener::flushed, aFlushedEvent );
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
        }
   }


    void SAL_CALL OHsqlConnection::addFlushListener( const Reference< XFlushListener >& l )
    {
        MethodGuard aGuard( *this );
        m_aFlushListeners.addInterface( l );
    }


    void SAL_CALL OHsqlConnection::removeFlushListener( const Reference< XFlushListener >& l )
    {
        MethodGuard aGuard( *this );
        m_aFlushListeners.removeInterface( l );
    }


    Reference< XGraphic > SAL_CALL OHsqlConnection::getTableIcon( const OUString& TableName, ::sal_Int32 /*_ColorMode*/ )
    {
        MethodGuard aGuard( *this );

        impl_checkExistingTable_throw( TableName );
        if ( !impl_isTextTable_nothrow( TableName ) )
            return nullptr;

        return impl_getTextTableIcon_nothrow();
    }


    Reference< XInterface > SAL_CALL OHsqlConnection::getTableEditor( const Reference< XDatabaseDocumentUI >& DocumentUI, const OUString& TableName )
    {
        MethodGuard aGuard( *this );

        impl_checkExistingTable_throw( TableName );
        if ( !impl_isTextTable_nothrow( TableName ) )
            return nullptr;

        if ( !DocumentUI.is() )
        {
            ::connectivity::SharedResources aResources;
            const OUString sError( aResources.getResourceString(STR_NO_DOCUMENTUI));
            throw IllegalArgumentException(
                sError,
                *this,
                0
            );
        } // if ( !_DocumentUI.is() )


//        Reference< XExecutableDialog > xEditor = impl_createLinkedTableEditor_throw( _DocumentUI, _TableName );
//        return xEditor.get();
        return nullptr;
        // editor not yet implemented in this CWS
    }


    Reference< XNameAccess > OHsqlConnection::impl_getTableContainer_throw()
    {
        Reference< XNameAccess > xTables;
        try
        {
            Reference< XConnection > xMe( *this, UNO_QUERY );
            Reference< XDataDefinitionSupplier > xDefinitionsSupp( m_xDriver, UNO_QUERY_THROW );
            Reference< XTablesSupplier > xTablesSupp( xDefinitionsSupp->getDataDefinitionByConnection( xMe ), css::uno::UNO_SET_THROW );
            xTables.set( xTablesSupp->getTables(), css::uno::UNO_SET_THROW );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            ::connectivity::SharedResources aResources;
            const OUString sError( aResources.getResourceString(STR_NO_TABLE_CONTAINER));
            throw WrappedTargetException( sError ,*this, anyEx );
        }

        SAL_WARN_IF( !xTables.is(), "connectivity.hsqldb", "OHsqlConnection::impl_getTableContainer_throw: post condition not met!" );
        return xTables;
    }

    //TODO: resource

    void OHsqlConnection::impl_checkExistingTable_throw( const OUString& _rTableName )
    {
        bool bDoesExist = false;
        try
        {
            Reference< XNameAccess > xTables( impl_getTableContainer_throw(), css::uno::UNO_SET_THROW );
            bDoesExist = xTables->hasByName( _rTableName );
        }
        catch( const Exception& )
        {
            // that's a serious error in impl_getTableContainer_throw, or hasByName, however, we're only
            // allowed to throw an IllegalArgumentException ourself
            DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
        }

        if ( !bDoesExist )
        {
            ::connectivity::SharedResources aResources;
            const OUString sError( aResources.getResourceStringWithSubstitution(
                STR_NO_TABLENAME,
                "$tablename$", _rTableName
            ));
            throw IllegalArgumentException( sError,*this, 0 );
        } // if ( !bDoesExist )
    }


    bool OHsqlConnection::impl_isTextTable_nothrow( const OUString& _rTableName )
    {
        bool bIsTextTable = false;
        try
        {
            Reference< XConnection > xMe( *this, UNO_QUERY_THROW );

            // split the fully qualified name
            Reference< XDatabaseMetaData > xMetaData( xMe->getMetaData(), css::uno::UNO_SET_THROW );
            OUString sCatalog, sSchema, sName;
            ::dbtools::qualifiedNameComponents( xMetaData, _rTableName, sCatalog, sSchema, sName, ::dbtools::EComposeRule::Complete );

            // get the table information
            OUStringBuffer sSQL( "SELECT HSQLDB_TYPE FROM INFORMATION_SCHEMA.SYSTEM_TABLES" );
            HTools::appendTableFilterCrit( sSQL, sCatalog, sSchema, sName, true );
            sSQL.append( " AND TABLE_TYPE = 'TABLE'" );

            Reference< XStatement > xStatement( xMe->createStatement(), css::uno::UNO_SET_THROW );
            Reference< XResultSet > xTableHsqlType( xStatement->executeQuery( sSQL.makeStringAndClear() ), css::uno::UNO_SET_THROW );

            if ( xTableHsqlType->next() )   // might not succeed in case of VIEWs
            {
                Reference< XRow > xValueAccess( xTableHsqlType, UNO_QUERY_THROW );
                OUString sTableType = xValueAccess->getString( 1 );
                bIsTextTable = sTableType == "TEXT";
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
        }

        return bIsTextTable;
    }


    Reference< XGraphic > OHsqlConnection::impl_getTextTableIcon_nothrow()
    {
        Reference< XGraphic > xGraphic;
        try
        {
            // create a graphic provider
            Reference< XGraphicProvider > xProvider;
            if ( m_xContext.is() )
                xProvider.set( GraphicProvider::create(m_xContext) );

            // ask the provider to obtain a graphic
            Sequence< PropertyValue > aMediaProperties{ comphelper::makePropertyValue(
                "URL", OUString(
                           // load the graphic from the global graphic repository
                           "private:graphicrepository/"
                           // the relative path within the images.zip
                           LINKED_TEXT_TABLE_IMAGE_RESOURCE)) };
            xGraphic = xProvider->queryGraphic( aMediaProperties );
            OSL_ENSURE( xGraphic.is(), "OHsqlConnection::impl_getTextTableIcon_nothrow: the provider did not give us a graphic object!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.hsqldb");
        }
        return xGraphic;
    }

} // namespace connectivity::hsqldb

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
