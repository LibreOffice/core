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


#include "hsqldb/HConnection.hxx"
#include "hsqldb/HTools.hxx"
#include "hsqlui.hrc"

#include <connectivity/dbtools.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicColorMode.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>

#include <comphelper/listenernotification.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

#include "resource/sharedresources.hxx"
#include "resource/hsqldb_res.hrc"

/** === begin UNO using === **/
using ::com::sun::star::util::XFlushListener;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::sdbc::XStatement;
using ::com::sun::star::sdbc::XConnection;
using ::com::sun::star::sdbcx::XDataDefinitionSupplier;
using ::com::sun::star::sdbcx::XTablesSupplier;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::ServiceNotRegisteredException;
using ::com::sun::star::sdbc::XDriver;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::GraphicProvider;
using ::com::sun::star::graphic::XGraphicProvider;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::ui::dialogs::XExecutableDialog;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::sdbc::XResultSet;
using ::com::sun::star::sdbc::XDatabaseMetaData;
using ::com::sun::star::sdbc::XDatabaseMetaData2;
using ::com::sun::star::sdbc::XRow;
using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
using ::com::sun::star::beans::PropertyValue;
/** === end UNO using === **/
namespace GraphicColorMode = ::com::sun::star::graphic::GraphicColorMode;

namespace connectivity { namespace hsqldb
{
    // =============================================================================
    // = FlushListeners
    // =============================================================================
    typedef ::comphelper::OListenerContainerBase< XFlushListener, EventObject > FlushListeners_Base;
    class FlushListeners : public FlushListeners_Base
    {
    public:
        FlushListeners( ::osl::Mutex& _rMutex ) :FlushListeners_Base( _rMutex ) { }

    protected:
        virtual bool    implTypedNotify(
                            const Reference< XFlushListener >& _rxListener,
                            const EventObject& _rEvent
                        )   SAL_THROW( ( Exception ) );
    };

    // -----------------------------------------------------------------------------
    bool FlushListeners::implTypedNotify( const Reference< XFlushListener >& _rxListener, const EventObject& _rEvent ) SAL_THROW( ( Exception ) )
    {
        _rxListener->flushed( _rEvent );
        return true;    // continue notifying the other listeners, if any
    }

    // =============================================================================
    // = OHsqlConnection
    // =============================================================================
    // -----------------------------------------------------------------------------
    void SAL_CALL OHsqlConnection::disposing(void)
    {
        m_aFlushListeners.disposeAndClear( EventObject( *this ) );
        OHsqlConnection_BASE::disposing();
        OConnectionWrapper::disposing();
    }
    // -----------------------------------------------------------------------------
    OHsqlConnection::OHsqlConnection( const Reference< XDriver > _rxDriver,
        const Reference< XConnection >& _xConnection ,const Reference< XMultiServiceFactory>& _xORB )
        :OHsqlConnection_BASE( m_aMutex )
        ,m_aFlushListeners( m_aMutex )
        ,m_xDriver( _rxDriver )
        ,m_xORB( _xORB )
        ,m_bIni(true)
        ,m_bReadOnly(false)
    {
        setDelegation(_xConnection,_xORB,m_refCount);
    }
    // -----------------------------------------------------------------------------
    OHsqlConnection::~OHsqlConnection()
    {
        if ( !OHsqlConnection_BASE::rBHelper.bDisposed )
        {
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2(OHsqlConnection,OHsqlConnection_BASE,OConnectionWrapper)
    IMPLEMENT_SERVICE_INFO(OHsqlConnection, "com.sun.star.sdbc.drivers.hsqldb.OHsqlConnection", "com.sun.star.sdbc.Connection")
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(OHsqlConnection,OHsqlConnection_BASE,OConnectionWrapper)

    //--------------------------------------------------------------------
    ::osl::Mutex& OHsqlConnection::getMutex() const
    {
        return m_aMutex;
    }

    //--------------------------------------------------------------------
    void OHsqlConnection::checkDisposed() const
    {
        ::connectivity::checkDisposed( rBHelper.bDisposed );
    }

    // XFlushable
    //--------------------------------------------------------------------
    void SAL_CALL OHsqlConnection::flush(  ) throw (RuntimeException)
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
                        if ( pIter->Name.compareToAscii("readonly") == 0 )
                            m_bReadOnly = true;
                    }
                }
                if ( !m_bReadOnly )
                {
                    Reference< XStatement > xStmt( m_xConnection->createStatement(), UNO_QUERY_THROW );
                    xStmt->execute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CHECKPOINT DEFRAG" ) ) );
                }
            }

            EventObject aFlushedEvent( *this );
            m_aFlushListeners.notifyEach( &XFlushListener::flushed, aFlushedEvent );
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
   }

    //--------------------------------------------------------------------
    void SAL_CALL OHsqlConnection::addFlushListener( const Reference< XFlushListener >& l ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aFlushListeners.addInterface( l );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OHsqlConnection::removeFlushListener( const Reference< XFlushListener >& l ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aFlushListeners.removeInterface( l );
    }

    // -------------------------------------------------------------------
    Reference< XGraphic > SAL_CALL OHsqlConnection::getTableIcon( const ::rtl::OUString& _TableName, ::sal_Int32 /*_ColorMode*/ ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );

        impl_checkExistingTable_throw( _TableName );
        if ( !impl_isTextTable_nothrow( _TableName ) )
            return NULL;

        return impl_getTextTableIcon_nothrow();
    }

    // -------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OHsqlConnection::getTableEditor( const Reference< XDatabaseDocumentUI >& _DocumentUI, const ::rtl::OUString& _TableName ) throw (IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        MethodGuard aGuard( *this );

        impl_checkExistingTable_throw( _TableName );
        if ( !impl_isTextTable_nothrow( _TableName ) )
            return NULL;

        if ( !_DocumentUI.is() )
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceString(STR_NO_DOCUMENTUI));
            throw IllegalArgumentException(
                sError,
                *this,
                0
            );
        } // if ( !_DocumentUI.is() )


//        Reference< XExecutableDialog > xEditor = impl_createLinkedTableEditor_throw( _DocumentUI, _TableName );
//        return xEditor.get();
        return NULL;
        // editor not yet implemented in this CWS
    }

    // -------------------------------------------------------------------
    Reference< XNameAccess > OHsqlConnection::impl_getTableContainer_throw()
    {
        Reference< XNameAccess > xTables;
        try
        {
            Reference< XConnection > xMe( *this, UNO_QUERY );
            Reference< XDataDefinitionSupplier > xDefinitionsSupp( m_xDriver, UNO_QUERY_THROW );
            Reference< XTablesSupplier > xTablesSupp( xDefinitionsSupp->getDataDefinitionByConnection( xMe ), UNO_QUERY_THROW );
            xTables.set( xTablesSupp->getTables(), UNO_QUERY_THROW );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceString(STR_NO_TABLE_CONTAINER));
            throw WrappedTargetException( sError ,*this, ::cppu::getCaughtException() );
        }

        OSL_POSTCOND( xTables.is(), "OHsqlConnection::impl_getTableContainer_throw: post condition not met!" );
        return xTables;
    }

    //TODO: resource
    // -------------------------------------------------------------------
    void OHsqlConnection::impl_checkExistingTable_throw( const ::rtl::OUString& _rTableName )
    {
        bool bDoesExist = false;
        try
        {
            Reference< XNameAccess > xTables( impl_getTableContainer_throw(), UNO_QUERY_THROW );
            if ( xTables.is() )
                bDoesExist = xTables->hasByName( _rTableName );
        }
        catch( const Exception& )
        {
            // that's a serious error in impl_getTableContainer_throw, or hasByName, however, we're only
            // allowed to throw an IllegalArgumentException ourself
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !bDoesExist )
        {
            ::connectivity::SharedResources aResources;
            const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
                STR_NO_TABLENAME,
                "$tablename$", _rTableName
            ));
            throw IllegalArgumentException( sError,*this, 0 );
        } // if ( !bDoesExist )
    }

    // -------------------------------------------------------------------
    bool OHsqlConnection::impl_isTextTable_nothrow( const ::rtl::OUString& _rTableName )
    {
        bool bIsTextTable = false;
        try
        {
            Reference< XConnection > xMe( *this, UNO_QUERY_THROW );

            // split the fully qualified name
            Reference< XDatabaseMetaData > xMetaData( xMe->getMetaData(), UNO_QUERY_THROW );
            ::rtl::OUString sCatalog, sSchema, sName;
            ::dbtools::qualifiedNameComponents( xMetaData, _rTableName, sCatalog, sSchema, sName, ::dbtools::eComplete );

            // get the table information
            ::rtl::OUStringBuffer sSQL;
            sSQL.appendAscii( "SELECT HSQLDB_TYPE FROM INFORMATION_SCHEMA.SYSTEM_TABLES" );
            HTools::appendTableFilterCrit( sSQL, sCatalog, sSchema, sName, true );
            sSQL.appendAscii( " AND TABLE_TYPE = 'TABLE'" );

            Reference< XStatement > xStatement( xMe->createStatement(), UNO_QUERY_THROW );
            Reference< XResultSet > xTableHsqlType( xStatement->executeQuery( sSQL.makeStringAndClear() ), UNO_QUERY_THROW );

            if ( xTableHsqlType->next() )   // might not succeed in case of VIEWs
            {
                Reference< XRow > xValueAccess( xTableHsqlType, UNO_QUERY_THROW );
                ::rtl::OUString sTableType = xValueAccess->getString( 1 );
                bIsTextTable = sTableType == "TEXT";
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return bIsTextTable;
    }

    // -------------------------------------------------------------------
    Reference< XGraphic > OHsqlConnection::impl_getTextTableIcon_nothrow()
    {
        Reference< XGraphic > xGraphic;
        try
        {
            // create a graphic provider
            Reference< XGraphicProvider > xProvider;
            if ( m_xORB.is() )
                xProvider.set( GraphicProvider::create(::comphelper::getComponentContext(m_xORB)) );

            // assemble the image URL
            ::rtl::OUStringBuffer aImageURL;
            // load the graphic from the global graphic repository
            aImageURL.appendAscii( "private:graphicrepository/" );
            // the relative path within the images.zip
            aImageURL.appendAscii( "database/" );
            aImageURL.appendAscii( LINKED_TEXT_TABLE_IMAGE_RESOURCE );
            // the name of the graphic to use
            ::rtl::OUString sImageURL( aImageURL.makeStringAndClear() );

            // ask the provider to obtain a graphic
            Sequence< PropertyValue > aMediaProperties( 1 );
            aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
            aMediaProperties[0].Value <<= sImageURL;
            xGraphic = xProvider->queryGraphic( aMediaProperties );
            OSL_ENSURE( xGraphic.is(), "OHsqlConnection::impl_getTextTableIcon_nothrow: the provider did not give us a graphic object!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xGraphic;
    }

} } // namespace connectivity::hsqldb

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
