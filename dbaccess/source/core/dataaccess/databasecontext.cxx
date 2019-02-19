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

#include <config_features.h>

#include <apitools.hxx>
#include <strings.hrc>
#include <core_resource.hxx>
#include <databasecontext.hxx>
#include "databasedocument.hxx"
#include "databaseregistrations.hxx"
#include "datasource.hxx"
#include <stringconstants.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <basic/basmgr.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <svl/filenotation.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/confignode.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <vector>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star;
using namespace ::cppu;
using namespace ::osl;
using namespace ::utl;

using ::com::sun::star::task::InteractionClassification_ERROR;
using ::com::sun::star::ucb::IOErrorCode_NO_FILE;
using ::com::sun::star::ucb::InteractiveIOException;
using ::com::sun::star::ucb::IOErrorCode_NOT_EXISTING;
using ::com::sun::star::ucb::IOErrorCode_NOT_EXISTING_PATH;

namespace dbaccess
{

        typedef ::cppu::WeakImplHelper<   XTerminateListener
                                      >   DatabaseDocumentLoader_Base;
        class DatabaseDocumentLoader : public DatabaseDocumentLoader_Base
        {
        private:
            Reference< XDesktop2 >               m_xDesktop;
            std::vector< const ODatabaseModelImpl* >  m_aDatabaseDocuments;

        public:
            explicit DatabaseDocumentLoader( const Reference<XComponentContext> & rxContext);

            void append(const ODatabaseModelImpl& _rModelImpl )
            {
                m_aDatabaseDocuments.emplace_back(&_rModelImpl);
            }

            void remove(const ODatabaseModelImpl& _rModelImpl)
            {
                m_aDatabaseDocuments.erase(std::find(m_aDatabaseDocuments.begin(), m_aDatabaseDocuments.end(), &_rModelImpl));
            }


        private:
            // XTerminateListener
            virtual void SAL_CALL queryTermination( const lang::EventObject& Event ) override;
            virtual void SAL_CALL notifyTermination( const lang::EventObject& Event ) override;
            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
        };

        DatabaseDocumentLoader::DatabaseDocumentLoader( const Reference<XComponentContext> & rxContext )
        {
            try
            {
                m_xDesktop.set( Desktop::create(rxContext) );
                m_xDesktop->addTerminateListener( this );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        void SAL_CALL DatabaseDocumentLoader::queryTermination( const lang::EventObject& /*Event*/ )
        {
            std::vector< const ODatabaseModelImpl* > aCpy(m_aDatabaseDocuments);
            for( const auto& pCopy : aCpy )
            {
                try
                {
                    const Reference< XModel2 > xMod( pCopy->getModel_noCreate(),
                                                     UNO_QUERY_THROW );
                    if( !xMod->getControllers()->hasMoreElements() )
                    {
                        Reference< util::XCloseable > xClose( xMod,
                                                              UNO_QUERY_THROW );
                        xClose->close( false );
                    }
                }
                catch( const CloseVetoException& )
                {
                    throw TerminationVetoException();
                }
            }
        }

        void SAL_CALL DatabaseDocumentLoader::notifyTermination( const lang::EventObject& /*Event*/ )
        {
        }

        void SAL_CALL DatabaseDocumentLoader::disposing( const lang::EventObject& /*Source*/ )
        {
        }

// ODatabaseContext

ODatabaseContext::ODatabaseContext( const Reference< XComponentContext >& _rxContext )
    :DatabaseAccessContext_Base(m_aMutex)
    ,m_aContext( _rxContext )
    ,m_aContainerListeners(m_aMutex)
{
    m_xDatabaseDocumentLoader = new DatabaseDocumentLoader( _rxContext );

#if HAVE_FEATURE_SCRIPTING
    ::basic::BasicManagerRepository::registerCreationListener( *this );
#endif

    osl_atomic_increment( &m_refCount );
    {
        m_xDBRegistrationAggregate.set( createDataSourceRegistrations( m_aContext ), UNO_SET_THROW );
        m_xDatabaseRegistrations.set( m_xDBRegistrationAggregate, UNO_QUERY_THROW );

        m_xDBRegistrationAggregate->setDelegator( *this );
    }
    osl_atomic_decrement( &m_refCount );
}

ODatabaseContext::~ODatabaseContext()
{
#if HAVE_FEATURE_SCRIPTING
    ::basic::BasicManagerRepository::revokeCreationListener( *this );
#endif

    m_xDatabaseDocumentLoader.clear();
    m_xDBRegistrationAggregate->setDelegator( nullptr );
    m_xDBRegistrationAggregate.clear();
    m_xDatabaseRegistrations.clear();
}

// Helper
OUString ODatabaseContext::getImplementationName_static()
{
    return OUString("com.sun.star.comp.dba.ODatabaseContext");
}

Reference< XInterface > ODatabaseContext::Create(const Reference< XComponentContext >& _rxContext)
{
    return *( new ODatabaseContext( _rxContext ) );
}

Sequence< OUString > ODatabaseContext::getSupportedServiceNames_static()
{
    Sequence<OUString> aSNS { "com.sun.star.sdb.DatabaseContext" };
    return aSNS;
}

// XServiceInfo
OUString ODatabaseContext::getImplementationName(  )
{
    return getImplementationName_static();
}

sal_Bool ODatabaseContext::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > ODatabaseContext::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_static();
}

Reference< XInterface > ODatabaseContext::impl_createNewDataSource()
{
    ::rtl::Reference pImpl( new ODatabaseModelImpl( m_aContext, *this ) );
    Reference< XDataSource > xDataSource( pImpl->getOrCreateDataSource() );

    return xDataSource.get();
}

Reference< XInterface > SAL_CALL ODatabaseContext::createInstance(  )
{
    // for convenience of the API user, we ensure the document is fully initialized (effectively: XLoadable::initNew
    // has been called at the DatabaseDocument).
    return impl_createNewDataSource();
}

Reference< XInterface > SAL_CALL ODatabaseContext::createInstanceWithArguments( const Sequence< Any >& _rArguments )
{
    ::comphelper::NamedValueCollection aArgs( _rArguments );
    OUString sURL = aArgs.getOrDefault( INFO_POOLURL, OUString() );

    Reference< XInterface > xDataSource;
    if ( !sURL.isEmpty() )
        xDataSource = getObject( sURL );

    if ( !xDataSource.is() )
        xDataSource = impl_createNewDataSource();

    return xDataSource;
}

// DatabaseAccessContext_Base
void ODatabaseContext::disposing()
{
    // notify our listener
    css::lang::EventObject aDisposeEvent(static_cast< XContainer* >(this));
    m_aContainerListeners.disposeAndClear(aDisposeEvent);

    // dispose the data sources
    // disposing seems to remove elements, so work on copy for valid iterators
    ObjectCache objCopy(m_aDatabaseObjects);
    for (auto const& elem : objCopy)
    {
        rtl::Reference< ODatabaseModelImpl > obj(elem.second);
            // make sure obj is acquired and does not delete itself from within
            // dispose()
        obj->dispose();
    }
    m_aDatabaseObjects.clear();
}

// XNamingService
Reference< XInterface >  ODatabaseContext::getRegisteredObject(const OUString& _rName)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    OUString sURL( getDatabaseLocation( _rName ) );

    if ( sURL.isEmpty() )
        // there is a registration for this name, but no URL
        throw IllegalArgumentException();

    // check if URL is already loaded
    Reference< XInterface > xExistent = getObject( sURL );
    if ( xExistent.is() )
        return xExistent;

    return loadObjectFromURL( _rName, sURL );
}

Reference< XInterface > ODatabaseContext::loadObjectFromURL(const OUString& _rName,const OUString& _sURL)
{
    INetURLObject aURL( _sURL );

    if ( aURL.GetProtocol() == INetProtocol::NotValid )
        throw NoSuchElementException( _rName, *this );

    bool bEmbeddedDataSource = aURL.isSchemeEqualTo(INetProtocol::VndSunStarPkg);
    try
    {
        if (!bEmbeddedDataSource)
        {
            ::ucbhelper::Content aContent( _sURL, nullptr, comphelper::getProcessComponentContext() );
            if ( !aContent.isDocument() )
                throw InteractiveIOException(
                    _sURL, *this, InteractionClassification_ERROR, IOErrorCode_NO_FILE
                );
        }
    }
    catch ( const InteractiveIOException& e )
    {
        if  (   ( e.Code == IOErrorCode_NO_FILE )
            ||  ( e.Code == IOErrorCode_NOT_EXISTING )
            ||  ( e.Code == IOErrorCode_NOT_EXISTING_PATH )
            )
        {
            // #i40463# #i39187#
            OUString sErrorMessage( DBA_RES( RID_STR_FILE_DOES_NOT_EXIST ) );
            ::svt::OFileNotation aTransformer( _sURL );

            SQLException aError;
            aError.Message = sErrorMessage.replaceAll( "$file$", aTransformer.get( ::svt::OFileNotation::N_SYSTEM ) );

            throw WrappedTargetException( _sURL, *this, makeAny( aError ) );
        }
        throw WrappedTargetException( _sURL, *this, ::cppu::getCaughtException() );
    }
    catch( const Exception& )
    {
        throw WrappedTargetException( _sURL, *this, ::cppu::getCaughtException() );
    }

    OSL_ENSURE( m_aDatabaseObjects.find( _sURL ) == m_aDatabaseObjects.end(),
        "ODatabaseContext::loadObjectFromURL: not intended for already-cached objects!" );

    ::rtl::Reference< ODatabaseModelImpl > pModelImpl;
    {
        pModelImpl.set( new ODatabaseModelImpl( _rName, m_aContext, *this ) );

        Reference< XModel > xModel( pModelImpl->createNewModel_deliverOwnership(), UNO_SET_THROW );
        Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );

        ::comphelper::NamedValueCollection aArgs;
        aArgs.put( "URL", _sURL );
        aArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );
        aArgs.put( "InteractionHandler", task::InteractionHandler::createWithParent(m_aContext, nullptr) );
        if (bEmbeddedDataSource)
        {
            // In this case the host contains the real path, and the path is the embedded stream name.
            auto const uri = css::uri::UriReferenceFactory::create(m_aContext)->parse(_sURL);
            if (uri.is() && uri->isAbsolute() && uri->isHierarchical()
                && uri->hasAuthority() && !uri->hasQuery() && !uri->hasFragment())
            {
                auto const auth = uri->getAuthority();
                auto const decAuth = rtl::Uri::decode(
                    auth, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8);
                if (auth.isEmpty() == decAuth.isEmpty()) {
                    // Decoding of auth to UTF-8 succeeded:
                    OUString sBaseURI = decAuth + uri->getPath();
                    aArgs.put("BaseURI", sBaseURI);
                } else {
                    SAL_WARN(
                        "dbaccess.core",
                        "<" << _sURL << "> cannot be parse as vnd.sun.star.pkg URL");
                }
            } else {
                SAL_WARN(
                    "dbaccess.core", "<" << _sURL << "> cannot be parse as vnd.sun.star.pkg URL");
            }
        }

        Sequence< PropertyValue > aResource( aArgs.getPropertyValues() );
        xLoad->load( aResource );
        xModel->attachResource( _sURL, aResource );

        ::utl::CloseableComponent aEnsureClose( xModel );
    }

    setTransientProperties( _sURL, *pModelImpl );

    return pModelImpl->getOrCreateDataSource().get();
}

void ODatabaseContext::appendAtTerminateListener(const ODatabaseModelImpl& _rDataSourceModel)
{
    m_xDatabaseDocumentLoader->append(_rDataSourceModel);
}

void ODatabaseContext::removeFromTerminateListener(const ODatabaseModelImpl& _rDataSourceModel)
{
    m_xDatabaseDocumentLoader->remove(_rDataSourceModel);
}

void ODatabaseContext::setTransientProperties(const OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel )
{
    if ( m_aDatasourceProperties.end() == m_aDatasourceProperties.find(_sURL) )
        return;
    try
    {
        OUString sAuthFailedPassword;
        Reference< XPropertySet > xDSProps( _rDataSourceModel.getOrCreateDataSource(), UNO_QUERY_THROW );
        const Sequence< PropertyValue >& rSessionPersistentProps = m_aDatasourceProperties[_sURL];
        for ( auto const & prop : rSessionPersistentProps )
        {
            if ( prop.Name == "AuthFailedPassword" )
            {
                OSL_VERIFY( prop.Value >>= sAuthFailedPassword );
            }
            else
            {
                xDSProps->setPropertyValue( prop.Name, prop.Value );
            }
        }

        _rDataSourceModel.m_sFailedPassword = sAuthFailedPassword;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void ODatabaseContext::registerObject(const OUString& _rName, const Reference< XInterface > & _rxObject)
{
    if ( _rName.isEmpty() )
        throw IllegalArgumentException( OUString(), *this, 1 );

    Reference< XDocumentDataSource > xDocDataSource( _rxObject, UNO_QUERY );
    Reference< XModel > xModel( xDocDataSource.is() ? xDocDataSource->getDatabaseDocument() : Reference< XOfficeDatabaseDocument >(), UNO_QUERY );
    if ( !xModel.is() )
        throw IllegalArgumentException( OUString(), *this, 2 );

    OUString sURL = xModel->getURL();
    if ( sURL.isEmpty() )
        throw IllegalArgumentException( DBA_RES( RID_STR_DATASOURCE_NOT_STORED ), *this, 2 );

    { // avoid deadlocks: lock m_aMutex after checking arguments
        MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

        registerDatabaseLocation( _rName, sURL );

        ODatabaseSource::setName( xDocDataSource, _rName, ODatabaseSource::DBContextAccess() );
    }

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rName), makeAny(_rxObject), Any());
    m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
}

void ODatabaseContext::storeTransientProperties( ODatabaseModelImpl& _rModelImpl)
{
    Reference< XPropertySet > xSource( _rModelImpl.getOrCreateDataSource(), UNO_QUERY );
    ::comphelper::NamedValueCollection aRememberProps;

    try
    {
        // get the info about the properties, check which ones are transient and not readonly
        Reference< XPropertySetInfo > xSetInfo;
        if (xSource.is())
            xSetInfo = xSource->getPropertySetInfo();
        Sequence< Property > aProperties;
        if (xSetInfo.is())
            aProperties = xSetInfo->getProperties();

        for ( const Property& rProperty : aProperties )
        {
            if  (   ( ( rProperty.Attributes & PropertyAttribute::TRANSIENT) != 0 )
                &&  ( ( rProperty.Attributes & PropertyAttribute::READONLY) == 0 )
                )
            {
                // found such a property
                aRememberProps.put( rProperty.Name, xSource->getPropertyValue( rProperty.Name ) );
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    // additionally, remember the "failed password", which is not available as property
    // #i86178#
    aRememberProps.put( "AuthFailedPassword", _rModelImpl.m_sFailedPassword );

    OUString sDocumentURL( _rModelImpl.getURL() );
    if ( m_aDatabaseObjects.find( sDocumentURL ) != m_aDatabaseObjects.end() )
    {
        m_aDatasourceProperties[ sDocumentURL ] = aRememberProps.getPropertyValues();
    }
    else if ( m_aDatabaseObjects.find( _rModelImpl.m_sName ) != m_aDatabaseObjects.end() )
    {
        OSL_FAIL( "ODatabaseContext::storeTransientProperties: a database document register by name? This shouldn't happen anymore!" );
            // all the code should have been changed so that registration is by URL only
        m_aDatasourceProperties[ _rModelImpl.m_sName ] = aRememberProps.getPropertyValues();
    }
    else
    {
        OSL_ENSURE(  sDocumentURL.isEmpty()  &&  _rModelImpl.m_sName.isEmpty() ,
            "ODatabaseContext::storeTransientProperties: a non-empty data source which I do not know?!" );
    }
}

void SAL_CALL ODatabaseContext::addContainerListener( const Reference< XContainerListener >& _rxListener )
{
    m_aContainerListeners.addInterface(_rxListener);
}

void SAL_CALL ODatabaseContext::removeContainerListener( const Reference< XContainerListener >& _rxListener )
{
    m_aContainerListeners.removeInterface(_rxListener);
}

void ODatabaseContext::revokeObject(const OUString& _rName)
{
    ClearableMutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    OUString sURL = getDatabaseLocation( _rName );

    revokeDatabaseLocation( _rName );
        // will throw if something goes wrong

    if ( m_aDatabaseObjects.find( _rName ) != m_aDatabaseObjects.end() )
    {
        m_aDatasourceProperties[ sURL ] = m_aDatasourceProperties[ _rName ];
    }

    // check if URL is already loaded
    ObjectCache::const_iterator aExistent = m_aDatabaseObjects.find( sURL );
    if ( aExistent != m_aDatabaseObjects.end() )
        m_aDatabaseObjects.erase( aExistent );

    // notify our container listeners
    ContainerEvent aEvent( *this, makeAny( _rName ), Any(), Any() );
    aGuard.clear();
    m_aContainerListeners.notifyEach( &XContainerListener::elementRemoved, aEvent );
}

sal_Bool SAL_CALL ODatabaseContext::hasRegisteredDatabase( const OUString& Name )
{
    return m_xDatabaseRegistrations->hasRegisteredDatabase( Name );
}

Sequence< OUString > SAL_CALL ODatabaseContext::getRegistrationNames()
{
    return m_xDatabaseRegistrations->getRegistrationNames();
}

OUString SAL_CALL ODatabaseContext::getDatabaseLocation( const OUString& Name )
{
    return m_xDatabaseRegistrations->getDatabaseLocation( Name );
}

void SAL_CALL ODatabaseContext::registerDatabaseLocation( const OUString& Name, const OUString& Location )
{
    m_xDatabaseRegistrations->registerDatabaseLocation( Name, Location );
}

void SAL_CALL ODatabaseContext::revokeDatabaseLocation( const OUString& Name )
{
    m_xDatabaseRegistrations->revokeDatabaseLocation( Name );
}

void SAL_CALL ODatabaseContext::changeDatabaseLocation( const OUString& Name, const OUString& NewLocation )
{
    m_xDatabaseRegistrations->changeDatabaseLocation( Name, NewLocation );
}

sal_Bool SAL_CALL ODatabaseContext::isDatabaseRegistrationReadOnly( const OUString& Name )
{
    return m_xDatabaseRegistrations->isDatabaseRegistrationReadOnly( Name );
}

void SAL_CALL ODatabaseContext::addDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener )
{
    m_xDatabaseRegistrations->addDatabaseRegistrationsListener( Listener );
}

void SAL_CALL ODatabaseContext::removeDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener )
{
    m_xDatabaseRegistrations->removeDatabaseRegistrationsListener( Listener );
}

// css::container::XElementAccess
Type ODatabaseContext::getElementType(  )
{
    return cppu::UnoType<XDataSource>::get();
}

sal_Bool ODatabaseContext::hasElements()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    return 0 != getElementNames().getLength();
}

// css::container::XEnumerationAccess
Reference< css::container::XEnumeration >  ODatabaseContext::createEnumeration()
{
    MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByName(static_cast<XNameAccess*>(this));
}

// css::container::XNameAccess
Any ODatabaseContext::getByName(const OUString& _rName)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);
    if ( _rName.isEmpty() )
        throw NoSuchElementException(_rName, *this);

    try
    {
        Reference< XInterface > xExistent = getObject( _rName );
        if ( xExistent.is() )
            return makeAny( xExistent );

        // see whether this is an registered name
        OUString sURL;
        if ( hasRegisteredDatabase( _rName ) )
        {
            sURL = getDatabaseLocation( _rName );
            // is the object cached under its URL?
            xExistent = getObject( sURL );
        }
        else
            // interpret the name as URL
            sURL = _rName;

        if ( !xExistent.is() )
            // try to load this as URL
            xExistent = loadObjectFromURL( _rName, sURL );
        return makeAny( xExistent );
    }
    catch (const NoSuchElementException&)
    {   // let these exceptions through
        throw;
    }
    catch (const WrappedTargetException&)
    {   // let these exceptions through
        throw;
    }
    catch (const RuntimeException&)
    {   // let these exceptions through
        throw;
    }
    catch (const Exception&)
    {   // exceptions other than the specified ones -> wrap
        Any aError = ::cppu::getCaughtException();
        throw WrappedTargetException(_rName, *this, aError );
    }
}

Sequence< OUString > ODatabaseContext::getElementNames()
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    return getRegistrationNames();
}

sal_Bool ODatabaseContext::hasByName(const OUString& _rName)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    return hasRegisteredDatabase( _rName );
}

Reference< XInterface > ODatabaseContext::getObject( const OUString& _rURL )
{
    ObjectCache::const_iterator aFind = m_aDatabaseObjects.find( _rURL );
    Reference< XInterface > xExistent;
    if ( aFind != m_aDatabaseObjects.end() )
        xExistent = aFind->second->getOrCreateDataSource();
    return xExistent;
}

void ODatabaseContext::registerDatabaseDocument( ODatabaseModelImpl& _rModelImpl )
{
    OUString sURL( _rModelImpl.getURL() );
    SAL_INFO("dbaccess.core", "DatabaseContext: registering " << sURL);
    if ( m_aDatabaseObjects.find( sURL ) == m_aDatabaseObjects.end() )
    {
        m_aDatabaseObjects[ sURL ] = &_rModelImpl;
        setTransientProperties( sURL, _rModelImpl );
    }
    else
        OSL_FAIL( "ODatabaseContext::registerDatabaseDocument: already have an object registered for this URL!" );
}

void ODatabaseContext::revokeDatabaseDocument( const ODatabaseModelImpl& _rModelImpl )
{
    const OUString& sURL( _rModelImpl.getURL() );
    SAL_INFO("dbaccess.core", "DatabaseContext: deregistering " << sURL);
    m_aDatabaseObjects.erase( sURL );
}

void ODatabaseContext::databaseDocumentURLChange( const OUString& _rOldURL, const OUString& _rNewURL )
{
    SAL_INFO("dbaccess.core", "DatabaseContext: changing registrations from " << _rOldURL <<
             " to " << _rNewURL);
    ObjectCache::const_iterator oldPos = m_aDatabaseObjects.find( _rOldURL );
    ENSURE_OR_THROW( oldPos != m_aDatabaseObjects.end(), "illegal old database document URL" );
    ObjectCache::const_iterator newPos = m_aDatabaseObjects.find( _rNewURL );
    ENSURE_OR_THROW( newPos == m_aDatabaseObjects.end(), "illegal new database document URL" );

    m_aDatabaseObjects[ _rNewURL ] = oldPos->second;
    m_aDatabaseObjects.erase( oldPos );
}

sal_Int64 SAL_CALL ODatabaseContext::getSomething( const Sequence< sal_Int8 >& rId )
{
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

Sequence< sal_Int8 > ODatabaseContext::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

void ODatabaseContext::onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) _rxForDocument;
    (void) _rBasicManager;
#else
    // if it's a database document ...
    Reference< XOfficeDatabaseDocument > xDatabaseDocument( _rxForDocument, UNO_QUERY );
    // ... or a sub document of a database document ...
    if ( !xDatabaseDocument.is() )
    {
        Reference< XChild > xDocAsChild( _rxForDocument, UNO_QUERY );
        if ( xDocAsChild.is() )
            xDatabaseDocument.set( xDocAsChild->getParent(), UNO_QUERY );
    }

    // ... whose BasicManager has just been created, then add the global DatabaseDocument variable to its scope.
    if ( xDatabaseDocument.is() )
        _rBasicManager.SetGlobalUNOConstant( "ThisDatabaseDocument", makeAny( xDatabaseDocument ) );
#endif
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
