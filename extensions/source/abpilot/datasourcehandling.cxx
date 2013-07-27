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


#include "abpresid.hrc"
#include "abptypes.hxx"
#include "componentmodule.hxx"
#include "datasourcehandling.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/stdtext.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::utl;
    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;

    //=====================================================================
    struct PackageAccessControl { };

    //=====================================================================
    //---------------------------------------------------------------------
    static Reference< XDatabaseContext > lcl_getDataSourceContext( const Reference< XComponentContext >& _rxContext ) SAL_THROW (( Exception ))
    {
        Reference<XDatabaseContext> xContext = DatabaseContext::create(_rxContext);
        return xContext;
    }

    //---------------------------------------------------------------------
    /// creates a new data source and inserts it into the context
    static void lcl_implCreateAndInsert(
        const Reference< XComponentContext >& _rxContext, const OUString& _rName,
        Reference< XPropertySet >& /* [out] */ _rxNewDataSource ) SAL_THROW (( ::com::sun::star::uno::Exception ))
    {
        //.............................................................
        // get the data source context
        Reference< XDatabaseContext > xContext = lcl_getDataSourceContext( _rxContext );

        DBG_ASSERT( !xContext->hasByName( _rName ), "lcl_implCreateAndInsert: name already used!" );
        (void)_rName;

        //.............................................................
        // create a new data source
        Reference< XPropertySet > xNewDataSource;
        if (xContext.is())
            xNewDataSource = Reference< XPropertySet >( xContext->createInstance(), UNO_QUERY );
        DBG_ASSERT( xNewDataSource.is(), "lcl_implCreateAndInsert: could not create a new data source!" );

        //.............................................................
        // insert the data source into the context
        DBG_ASSERT( xContext.is(), "lcl_implCreateAndInsert: missing an interface on the context (XNamingService)!" );
        if (xContext.is())
        {
            //  xDynamicContext->registerObject( _rName, xNewDataSource );
            _rxNewDataSource = xNewDataSource;
        }
    }

    //---------------------------------------------------------------------
    /// creates and inserts a data source, and sets it's URL property to the string given
    static ODataSource lcl_implCreateAndSetURL(
        const Reference< XComponentContext >& _rxORB, const OUString& _rName,
        const sal_Char* _pInitialAsciiURL ) SAL_THROW (( ))
    {
        ODataSource aReturn( _rxORB );
        try
        {
            // create the new data source
            Reference< XPropertySet > xNewDataSource;
            lcl_implCreateAndInsert( _rxORB, _rName, xNewDataSource );

            //.............................................................
            // set the URL property
            if (xNewDataSource.is())
            {
                xNewDataSource->setPropertyValue(
                    OUString( "URL" ),
                    makeAny( OUString::createFromAscii( _pInitialAsciiURL ) )
                );
            }

            aReturn.setDataSource( xNewDataSource, _rName,PackageAccessControl() );
        }
        catch(const Exception&)
        {
            OSL_FAIL( "lcl_implCreateAndSetURL: caught an exception while creating the data source!" );
        }

        return aReturn;
    }
    //---------------------------------------------------------------------
    void lcl_registerDataSource(
        const Reference< XComponentContext >& _rxORB, const OUString& _sName,
        const OUString& _sURL ) SAL_THROW (( ::com::sun::star::uno::Exception ))
    {
        OSL_ENSURE( !_sName.isEmpty(), "lcl_registerDataSource: invalid name!" );
        OSL_ENSURE( !_sURL.isEmpty(), "lcl_registerDataSource: invalid URL!" );
        try
        {
            Reference< XDatabaseContext > xRegistrations( DatabaseContext::create(_rxORB) );
            if ( xRegistrations->hasRegisteredDatabase( _sName ) )
                xRegistrations->changeDatabaseLocation( _sName, _sURL );
            else
                xRegistrations->registerDatabaseLocation( _sName, _sURL );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //=====================================================================
    //= ODataSourceContextImpl
    //=====================================================================
    struct ODataSourceContextImpl
    {
        Reference< XComponentContext >      xORB;
        Reference< XNameAccess >            xContext;           /// the UNO data source context
        StringBag                           aDataSourceNames;   /// for quicker name checks (without the UNO overhead)

        ODataSourceContextImpl( const Reference< XComponentContext >& _rxORB ) : xORB( _rxORB ) { }
        ODataSourceContextImpl( const ODataSourceContextImpl& _rSource )
            :xORB       ( _rSource.xORB )
            ,xContext   ( _rSource.xContext )
        {
        }
    };

    //=====================================================================
    //= ODataSourceContext
    //=====================================================================
    //---------------------------------------------------------------------
    ODataSourceContext::ODataSourceContext(const Reference< XComponentContext >& _rxORB)
        :m_pImpl( new ODataSourceContextImpl( _rxORB ) )
    {
        try
        {
            // create the UNO context
            m_pImpl->xContext = Reference<XNameAccess>(
                      lcl_getDataSourceContext( _rxORB ),
                      UNO_QUERY_THROW);

            if (m_pImpl->xContext.is())
            {
                // collect the data source names
                Sequence< OUString > aDSNames = m_pImpl->xContext->getElementNames();
                const OUString* pDSNames = aDSNames.getConstArray();
                const OUString* pDSNamesEnd = pDSNames + aDSNames.getLength();

                for ( ;pDSNames != pDSNamesEnd; ++pDSNames )
                    m_pImpl->aDataSourceNames.insert( *pDSNames );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ODataSourceContext::ODataSourceContext: caught an exception!" );
        }
    }
    ODataSourceContext::~ODataSourceContext()
    {
        delete(m_pImpl);
    }

    //---------------------------------------------------------------------
    OUString& ODataSourceContext::disambiguate(OUString& _rDataSourceName)
    {
        OUString sCheck( _rDataSourceName );
        ConstStringBagIterator aPos = m_pImpl->aDataSourceNames.find( sCheck );

        sal_Int32 nPostFix = 1;
        while ( ( m_pImpl->aDataSourceNames.end() != aPos ) && ( nPostFix < 65535 ) )
        {   // there already is a data source with this name
            sCheck = _rDataSourceName;
            sCheck += OUString::valueOf( nPostFix++ );

            aPos = m_pImpl->aDataSourceNames.find( sCheck );
        }

        _rDataSourceName = sCheck;
        return _rDataSourceName;
    }

    //---------------------------------------------------------------------
    void ODataSourceContext::getDataSourceNames( StringBag& _rNames ) const SAL_THROW (( ))
    {
        _rNames = m_pImpl->aDataSourceNames;
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewLDAP( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:ldap:" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewMORK( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:mozilla" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewThunderbird( const OUString& _rName ) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:thunderbird" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewEvolutionLdap( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:ldap" );
    }
    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewEvolutionGroupwise( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:groupwise" );
    }
    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewEvolution( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:local" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewKab( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:kab" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewMacab( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:macab" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewOutlook( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlook" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewOE( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlookexp" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewDBase( const OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:dbase:" );
    }

    //=====================================================================
    //= ODataSourceImpl
    //=====================================================================
    struct ODataSourceImpl
    {
    public:
        Reference< XComponentContext >          xORB;               /// the service factory
        Reference< XPropertySet >               xDataSource;        /// the UNO data source
        ::utl::SharedUNOComponent< XConnection >
                                                xConnection;
        StringBag                               aTables;            // the cached table names
        OUString                         sName;
        sal_Bool                                bTablesUpToDate;    // table name cache up-to-date?

        ODataSourceImpl( const Reference< XComponentContext >& _rxORB )
            :xORB( _rxORB )
            ,bTablesUpToDate( sal_False )
        {
        }

        ODataSourceImpl( const ODataSourceImpl& _rSource );
    };

    //---------------------------------------------------------------------
    ODataSourceImpl::ODataSourceImpl( const ODataSourceImpl& _rSource )
        :xORB( _rSource.xORB )
        ,xDataSource( _rSource.xDataSource )
        ,xConnection( _rSource.xConnection )
        ,aTables( _rSource.aTables )
        ,sName( _rSource.sName )
        ,bTablesUpToDate( _rSource.bTablesUpToDate )
    {
    }

    //=====================================================================
    //= ODataSource
    //=====================================================================
    //---------------------------------------------------------------------
    ODataSource::ODataSource( const ODataSource& _rSource )
        :m_pImpl( NULL )
    {
        *this = _rSource;
    }

    //---------------------------------------------------------------------
    ODataSource& ODataSource::operator=( const ODataSource& _rSource )
    {
        if( this != &_rSource )
        {
            delete m_pImpl;
            m_pImpl = new ODataSourceImpl( *_rSource.m_pImpl );
        }
        return *this;
    }

    //---------------------------------------------------------------------
    ODataSource::ODataSource( const Reference< XComponentContext >& _rxORB )
        :m_pImpl(new ODataSourceImpl(_rxORB))
    {
    }

    //---------------------------------------------------------------------
    ODataSource::~ODataSource( )
    {
        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    void ODataSource::store() SAL_THROW (( ))
    {
        if (!isValid())
            // nothing to do
            return;
        try
        {
            Reference< XDocumentDataSource > xDocAccess( m_pImpl->xDataSource, UNO_QUERY );
            Reference< XStorable > xStorable;
            if ( xDocAccess.is() )
                xStorable = xStorable.query( xDocAccess->getDatabaseDocument() );
            OSL_ENSURE( xStorable.is(),"DataSource is no XStorable!" );
            if ( xStorable.is() )
                xStorable->storeAsURL(m_pImpl->sName,Sequence<PropertyValue>());
        }
        catch(const Exception&)
        {
            OSL_FAIL( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
        }
    }
    //---------------------------------------------------------------------
    void ODataSource::registerDataSource( const OUString& _sRegisteredDataSourceName) SAL_THROW (( ))
    {
        if (!isValid())
            // nothing to do
            return;

        try
        {
            // invalidate ourself
            lcl_registerDataSource(m_pImpl->xORB,_sRegisteredDataSourceName,m_pImpl->sName);
        }
        catch(const Exception&)
        {
            OSL_FAIL( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
        }
    }

    //---------------------------------------------------------------------
    void ODataSource::setDataSource( const Reference< XPropertySet >& _rxDS,const OUString& _sName, PackageAccessControl )
    {
        if (m_pImpl->xDataSource.get() == _rxDS.get())
            // nothing to do
            return;

        if ( isConnected() )
            disconnect();

        m_pImpl->sName = _sName;
        m_pImpl->xDataSource = _rxDS;
    }

    //---------------------------------------------------------------------
    void ODataSource::remove() SAL_THROW (( ))
    {
        if (!isValid())
            // nothing to do
            return;

        try
        {
            // invalidate ourself
            m_pImpl->xDataSource.clear();
        }
        catch(const Exception&)
        {
            OSL_FAIL( "ODataSource::remove: caught an exception while creating the data source!" );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::rename( const OUString& _rName ) SAL_THROW (( ))
    {
        if (!isValid())
            // nothing to do
            return sal_False;

        m_pImpl->sName = _rName;
        return sal_True;
    }

    //---------------------------------------------------------------------
    OUString ODataSource::getName() const SAL_THROW (( ))
    {
        if ( !isValid() )
            return OUString();
        return m_pImpl->sName;
    }

    //---------------------------------------------------------------------
    bool ODataSource::hasTable( const OUString& _rTableName ) const
    {
        if ( !isConnected() )
            return false;

        const StringBag& aTables( getTableNames() );
        return aTables.find( _rTableName ) != aTables.end();
    }

    //---------------------------------------------------------------------
    const StringBag& ODataSource::getTableNames() const SAL_THROW (( ))
    {
        m_pImpl->aTables.clear();
        if ( !isConnected() )
        {
            OSL_FAIL( "ODataSource::getTableNames: not connected!" );
        }
        else
        {
            try
            {
                // get the tables container from the connection
                Reference< XTablesSupplier > xSuppTables( m_pImpl->xConnection.getTyped(), UNO_QUERY );
                Reference< XNameAccess > xTables;
                if ( xSuppTables.is( ) )
                    xTables = xSuppTables->getTables();
                DBG_ASSERT( xTables.is(), "ODataSource::getTableNames: could not retrieve the tables container!" );

                // get the names
                Sequence< OUString > aTableNames;
                if ( xTables.is( ) )
                    aTableNames = xTables->getElementNames( );

                // copy the names
                const OUString* pTableNames = aTableNames.getConstArray();
                const OUString* pTableNamesEnd = pTableNames + aTableNames.getLength();
                for (;pTableNames < pTableNamesEnd; ++pTableNames)
                    m_pImpl->aTables.insert( *pTableNames );
            }
            catch(const Exception&)
            {
            }
        }

        // now the table cache is up-to-date
        m_pImpl->bTablesUpToDate = sal_True;
        return m_pImpl->aTables;
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::connect( Window* _pMessageParent ) SAL_THROW (( ))
    {
        if ( isConnected( ) )
            // nothing to do
            return sal_True;

        // ................................................................
        // create the interaction handler (needed for authentication and error handling)
        Reference< XInteractionHandler > xInteractions;
        try
        {
            xInteractions.set(
                InteractionHandler::createWithParent(m_pImpl->xORB, 0),
                UNO_QUERY);
        }
        catch(const Exception&)
        {
        }

        // ................................................................
        // failure to create the interaction handler is a serious issue ...
        if (!xInteractions.is())
        {
            OUString s_sInteractionHandlerServiceName("com.sun.star.task.InteractionHandler");
            if ( _pMessageParent )
                ShowServiceNotAvailableError( _pMessageParent, s_sInteractionHandlerServiceName, sal_True );
            return sal_False;
        }

        // ................................................................
        // open the connection
        Any aError;
        Reference< XConnection > xConnection;
        try
        {
            Reference< XCompletedConnection > xComplConn( m_pImpl->xDataSource, UNO_QUERY );
            DBG_ASSERT( xComplConn.is(), "ODataSource::connect: missing the XCompletedConnection interface on the data source!" );
            if ( xComplConn.is() )
                xConnection = xComplConn->connectWithCompletion( xInteractions );
        }
        catch( const SQLContext& e ) { aError <<= e; }
        catch( const SQLWarning& e ) { aError <<= e; }
        catch( const SQLException& e ) { aError <<= e; }
        catch( const Exception& )
        {
            OSL_FAIL( "ODataSource::connect: caught a generic exception!" );
        }

        // ................................................................
        // handle errors
        if ( aError.hasValue() && _pMessageParent )
        {
            try
            {
                SQLException aException;
                  aError >>= aException;
                  if ( aException.Message.isEmpty() )
                  {
                    // prepend some context info
                    SQLContext aDetailedError;
                    aDetailedError.Message = ModuleRes(RID_STR_NOCONNECTION).toString();
                    aDetailedError.Details = ModuleRes(RID_STR_PLEASECHECKSETTINGS).toString();
                    aDetailedError.NextException = aError;
                    // handle (aka display) the new context info
                    xInteractions->handle( new OInteractionRequest( makeAny( aDetailedError ) ) );
                  }
                  else
                  {
                      // handle (aka display) the original error
                    xInteractions->handle( new OInteractionRequest( makeAny( aException ) ) );
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "ODataSource::connect: caught an exception while trying to display the error!" );
            }
        }

        if ( !xConnection.is() )
            return sal_False;

        // ................................................................
        // success
        m_pImpl->xConnection.reset( xConnection );
        m_pImpl->aTables.clear();
        m_pImpl->bTablesUpToDate = sal_False;

        return sal_True;
    }

    //---------------------------------------------------------------------
    void ODataSource::disconnect( ) SAL_THROW (( ))
    {
        m_pImpl->xConnection.clear();
        m_pImpl->aTables.clear();
        m_pImpl->bTablesUpToDate = sal_False;
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::isConnected( ) const SAL_THROW (( ))
    {
        return m_pImpl->xConnection.is();
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::isValid() const SAL_THROW (( ))
    {
        return m_pImpl && m_pImpl->xDataSource.is();
    }
    //---------------------------------------------------------------------
    Reference< XPropertySet > ODataSource::getDataSource() const SAL_THROW (( ))
    {
        return m_pImpl ? m_pImpl->xDataSource : Reference< XPropertySet >();
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
