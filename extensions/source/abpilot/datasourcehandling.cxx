/*************************************************************************
 *
 *  $RCSfile: datasourcehandling.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:36:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_DATASOURCEHANDLING_HXX
#include "datasourcehandling.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef EXTENSIONS_ABP_ABPTYPES_HXX
#include "abptypes.hxx"
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
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
    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbRegisteredNamesNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("org.openoffice.Office.DataAccess/RegisteredNames");
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbNameNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Name");
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbLocationNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Location");
        return s_sNodeName;
    }
    //---------------------------------------------------------------------
    static Reference< XNameAccess > lcl_getDataSourceContext( const Reference< XMultiServiceFactory >& _rxORB ) SAL_THROW (( Exception ))
    {
        Reference< XNameAccess > xContext( _rxORB->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ) ), UNO_QUERY );
        DBG_ASSERT(xContext.is(), "lcl_getDataSourceContext: could not access the data source context!");
        return xContext;
    }

    //---------------------------------------------------------------------
    /// creates a new data source and inserts it into the context
    static void lcl_implCreateAndInsert(
        const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rName,
        Reference< XPropertySet >& /* [out] */ _rxNewDataSource ) SAL_THROW (( ::com::sun::star::uno::Exception ))
    {
        //.............................................................
        // get the data source context
        Reference< XNameAccess > xContext = lcl_getDataSourceContext( _rxORB );

        DBG_ASSERT( !xContext->hasByName( _rName ), "lcl_implCreateAndInsert: name already used!" );

        //.............................................................
        // create a new data source
        Reference< XSingleServiceFactory > xFactory( xContext, UNO_QUERY );
        Reference< XPropertySet > xNewDataSource;
        if (xFactory.is())
            xNewDataSource = Reference< XPropertySet >( xFactory->createInstance(), UNO_QUERY );
        DBG_ASSERT( xNewDataSource.is(), "lcl_implCreateAndInsert: could not create a new data source!" );

        //.............................................................
        // insert the data source into the context
        Reference< XNamingService > xDynamicContext( xContext, UNO_QUERY );
        DBG_ASSERT( xDynamicContext.is(), "lcl_implCreateAndInsert: missing an interface on the context (XNamingService)!" );
        if (xDynamicContext.is())
        {
            //  xDynamicContext->registerObject( _rName, xNewDataSource );
            _rxNewDataSource = xNewDataSource;
        }
    }

    //---------------------------------------------------------------------
    /// creates and inserts a data source, and sets it's URL property to the string given
    static ODataSource lcl_implCreateAndSetURL(
        const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rName,
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
                    ::rtl::OUString::createFromAscii( "URL" ),
                    makeAny( ::rtl::OUString::createFromAscii( _pInitialAsciiURL ) )
                );
            }

            aReturn.setDataSource( xNewDataSource, _rName,PackageAccessControl() );
        }
        catch(const Exception&)
        {
            DBG_ERROR( "lcl_implCreateAndSetURL: caught an exception while creating the data source!" );
        }

        return aReturn;
    }
    //---------------------------------------------------------------------
    void lcl_registerDataSource(
        const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _sName,
        const ::rtl::OUString& _sURL ) SAL_THROW (( ::com::sun::star::uno::Exception ))
    {
        // the config node where all pooling relevant info are stored under
        OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
            _rxORB, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

        if (!aDbRegisteredNamesRoot.isValid())
            // already asserted by the OConfigurationTreeRoot
            return;

        OSL_ENSURE(_sName.getLength(),"No Name given!");
        OSL_ENSURE(_sURL.getLength(),"No URL given!");

        OConfigurationNode aThisDriverSettings;
        if ( aDbRegisteredNamesRoot.hasByName(_sName) )
            aThisDriverSettings = aDbRegisteredNamesRoot.openNode(_sName);
        else
            aThisDriverSettings = aDbRegisteredNamesRoot.createNode(_sName);

        // set the values
        aThisDriverSettings.setNodeValue(getDbNameNodeName(), makeAny(_sName));
        aThisDriverSettings.setNodeValue(getDbLocationNodeName(), makeAny(_sURL));

        aDbRegisteredNamesRoot.commit();
    }

    //=====================================================================
    //= ODataSourceContextImpl
    //=====================================================================
    struct ODataSourceContextImpl
    {
        Reference< XMultiServiceFactory >   xORB;
        Reference< XNameAccess >            xContext;           /// the UNO data source context
        StringBag                           aDataSourceNames;   /// for quicker name checks (without the UNO overhead)

        ODataSourceContextImpl( const Reference< XMultiServiceFactory >& _rxORB ) : xORB( _rxORB ) { }
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
    ODataSourceContext::ODataSourceContext(const Reference< XMultiServiceFactory >& _rxORB)
        :m_pImpl( new ODataSourceContextImpl( _rxORB ) )
    {
        try
        {
            // create the UNO context
            m_pImpl->xContext = lcl_getDataSourceContext( _rxORB );

            if (m_pImpl->xContext.is())
            {
                // collect the data source names
                Sequence< ::rtl::OUString > aDSNames = m_pImpl->xContext->getElementNames();
                const ::rtl::OUString* pDSNames = aDSNames.getConstArray();
                const ::rtl::OUString* pDSNamesEnd = pDSNames + aDSNames.getLength();

                for ( ;pDSNames != pDSNamesEnd; ++pDSNames )
                    m_pImpl->aDataSourceNames.insert( *pDSNames );
            }
        }
        catch( const Exception& )
        {
            DBG_ERROR( "ODataSourceContext::ODataSourceContext: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    ::rtl::OUString& ODataSourceContext::disambiguate(::rtl::OUString& _rDataSourceName)
    {
        ::rtl::OUString sCheck( _rDataSourceName );
        ConstStringBagIterator aPos = m_pImpl->aDataSourceNames.find( sCheck );

        sal_Int32 nPostFix = 1;
        while ( ( m_pImpl->aDataSourceNames.end() != aPos ) && ( nPostFix < 65535 ) )
        {   // there already is a data source with this name
            sCheck = _rDataSourceName;
            sCheck += ::rtl::OUString::valueOf( nPostFix++ );

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
    ODataSource ODataSourceContext::createNewLDAP( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:ldap:" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewMORK( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:mozilla" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewEvolution( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewOutlook( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlook" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewOE( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlookexp" );
    }

    //---------------------------------------------------------------------
    ODataSource ODataSourceContext::createNewDBase( const ::rtl::OUString& _rName) SAL_THROW (( ))
    {
        return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:dbase:" );
    }

    //=====================================================================
    //= OSharedConnection
    //=====================================================================
    class OSharedConnection : ::rtl::IReference
    {
    protected:
        Reference< XConnection >        m_xConnection;
        oslInterlockedCount             m_refCount;

    protected:
        ~OSharedConnection();
        OSharedConnection( const OSharedConnection& );      // never implemented
        OSharedConnection& operator=( OSharedConnection& ); // never implemented

    public:
        OSharedConnection();
        OSharedConnection( const Reference< XConnection >& _rxConn );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        // access to the real connection
        const Reference< XConnection >& getConnection() const { return m_xConnection; }

        sal_Bool is() const { return m_xConnection.is() ; }
    };

    //---------------------------------------------------------------------
    OSharedConnection::OSharedConnection()
        :m_refCount(0)
    {
    }

    //---------------------------------------------------------------------
    OSharedConnection::OSharedConnection( const Reference< XConnection >& _rxConn )
        :m_xConnection(_rxConn)
        ,m_refCount(0)
    {
    }

    //---------------------------------------------------------------------
    OSharedConnection::~OSharedConnection()
    {
        try
        {
            Reference< XComponent > xConnComp( m_xConnection, UNO_QUERY );
            if (xConnComp.is())
                xConnComp->dispose();
        }
        catch(const Exception&)
        {
            DBG_ERROR("OSharedConnection::~OSharedConnection: could not dispose the connection!");
        }
    }

    //---------------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSharedConnection::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //---------------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSharedConnection::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //=====================================================================
    //= ODataSourceImpl
    //=====================================================================
    struct ODataSourceImpl
    {
    public:
        Reference< XMultiServiceFactory >       xORB;               /// the service factory
        Reference< XPropertySet >               xDataSource;        /// the UNO data source
        ::rtl::Reference< OSharedConnection >   xConnection;        /// the connection
        StringBag                               aTables;            // the cached table names
        ::rtl::OUString                         sName;
        sal_Bool                                bTablesUpToDate;    // table name cache up-to-date?

        ODataSourceImpl( const Reference< XMultiServiceFactory >& _rxORB )
            :xORB( _rxORB )
            ,xConnection( new OSharedConnection )
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
    ODataSource::ODataSource( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rName )
        :m_pImpl(new ODataSourceImpl(_rxORB))
    {
        try
        {
            // get the data source context
            Reference< XNameAccess > xContext = lcl_getDataSourceContext( m_pImpl->xORB );

            // retrieve the UNO data source
            if (xContext.is())
                xContext->getByName( _rName ) >>= m_pImpl->xDataSource;
        }
        catch(const Exception&)
        {
            DBG_ERROR("ODataSource::ODataSource: could not access the requested data source (caught an exception)!");
        }
    }

    //---------------------------------------------------------------------
    ODataSource::ODataSource( const ODataSource& _rSource )
        :m_pImpl( NULL )
    {
        *this = _rSource;
    }

    //---------------------------------------------------------------------
    ODataSource& ODataSource::operator=( const ODataSource& _rSource )
    {
        delete m_pImpl;
        m_pImpl = new ODataSourceImpl( *_rSource.m_pImpl );

        return *this;
    }

    //---------------------------------------------------------------------
    ODataSource::ODataSource( const Reference< XMultiServiceFactory >& _rxORB )
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
            Reference<XStorable> xStorable(m_pImpl->xDataSource,UNO_QUERY);
            OSL_ENSURE( xStorable.is(),"DataSource is no XStorable!" );
            if ( xStorable.is() )
            {
                xStorable->storeAsURL(m_pImpl->sName,Sequence<PropertyValue>());
            }
        }
        catch(const Exception&)
        {
            DBG_ERROR( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
        }
    }
    //---------------------------------------------------------------------
    void ODataSource::registerDataSource( const ::rtl::OUString& _sRegisteredDataSourceName) SAL_THROW (( ))
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
            DBG_ERROR( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
        }
    }

    //---------------------------------------------------------------------
    void ODataSource::setDataSource( const Reference< XPropertySet >& _rxDS,const ::rtl::OUString& _sName, PackageAccessControl )
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
            DBG_ERROR( "ODataSource::remove: caught an exception while creating the data source!" );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::rename( const ::rtl::OUString& _rName ) SAL_THROW (( ))
    {
        if (!isValid())
            // nothing to do
            return sal_False;

        m_pImpl->sName = _rName;
        return sal_True;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString ODataSource::getName() const SAL_THROW (( ))
    {
        if ( !isValid() )
            return ::rtl::OUString();
        return m_pImpl->sName;
    }

    //---------------------------------------------------------------------
    const StringBag& ODataSource::getTableNames() const SAL_THROW (( ))
    {
        m_pImpl->aTables.clear();
        if ( !isConnected() )
        {
            DBG_ERROR( "ODataSource::getTableNames: not connected!" );
        }
        else
        {
            try
            {
                // get the tables container from the connection
                Reference< XTablesSupplier > xSuppTables( m_pImpl->xConnection->getConnection(), UNO_QUERY );
                Reference< XNameAccess > xTables;
                if ( xSuppTables.is( ) )
                    xTables = xSuppTables->getTables();
                DBG_ASSERT( xTables.is(), "ODataSource::getTableNames: could not retrieve the tables container!" );

                // get the names
                Sequence< ::rtl::OUString > aTableNames;
                if ( xTables.is( ) )
                    aTableNames = xTables->getElementNames( );

                // copy the names
                const ::rtl::OUString* pTableNames = aTableNames.getConstArray();
                const ::rtl::OUString* pTableNamesEnd = pTableNames + aTableNames.getLength();
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
        static ::rtl::OUString s_sInteractionHandlerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.InteractionHandler");
        Reference< XInteractionHandler > xInteractions;
        try
        {
            xInteractions = Reference< XInteractionHandler >(
                m_pImpl->xORB->createInstance( s_sInteractionHandlerServiceName ),
                UNO_QUERY
            );
        }
        catch(const Exception&)
        {
        }

        // ................................................................
        // failure to create the interaction handler is a serious issue ...
        if (!xInteractions.is())
        {
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
            DBG_ERROR( "ODataSource::connect: caught a generic exception!" );
        }

        // ................................................................
        // handle errors
        if ( aError.hasValue() && _pMessageParent )
        {
            try
            {
                SQLException aException;
                  aError >>= aException;
                  if ( !aException.Message.getLength() )
                  {
                    // prepend some context info
                    SQLContext aDetailedError;
                    aDetailedError.Message = String( ModuleRes( RID_STR_NOCONNECTION ) );
                    aDetailedError.Details = String( ModuleRes( RID_STR_PLEASECHECKSETTINGS ) );
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
                DBG_ERROR( "ODataSource::connect: caught an exception while trying to display the error!" );
            }
        }

        if ( !xConnection.is() )
            return sal_False;

        // ................................................................
        // success
        m_pImpl->xConnection = new OSharedConnection( xConnection );
        m_pImpl->aTables.clear();
        m_pImpl->bTablesUpToDate = sal_False;

        return sal_True;
    }

    //---------------------------------------------------------------------
    void ODataSource::disconnect( ) SAL_THROW (( ))
    {
        m_pImpl->xConnection = NULL;
        m_pImpl->aTables.clear();
        m_pImpl->bTablesUpToDate = sal_False;
    }

    //---------------------------------------------------------------------
    sal_Bool ODataSource::isConnected( ) const SAL_THROW (( ))
    {
        return m_pImpl->xConnection.is() && m_pImpl->xConnection->is();
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

