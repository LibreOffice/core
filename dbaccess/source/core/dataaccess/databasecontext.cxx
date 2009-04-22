/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: databasecontext.cxx,v $
 * $Revision: 1.43.4.3 $
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

#include "apitools.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "databasecontext.hxx"
#include "databasedocument.hxx"
#include "datasource.hxx"
#include "dbastrings.hrc"
#include "module_dba.hxx"

/** === being UNO includes === **/
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/util/XCloseable.hpp>
/** === end UNO includes === **/

#include <basic/basmgr.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/evtlistenerhlp.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svtools/filenotation.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/confignode.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <list>
#include <boost/bind.hpp>

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

//==========================================================================

extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext()
{
    static ::dba::OLegacySingletonRegistration< ::dbaccess::ODatabaseContext > aODatabaseContext_AutoRegistration;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    namespace
    {
        //--------------------------------------------------------------------
        const ::rtl::OUString& getDbRegisteredNamesNodeName()
        {
            static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("org.openoffice.Office.DataAccess/RegisteredNames");
            return s_sNodeName;
        }

        //--------------------------------------------------------------------
        const ::rtl::OUString& getDbNameNodeName()
        {
            static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Name");
            return s_sNodeName;
        }

        //--------------------------------------------------------------------
        const ::rtl::OUString& getDbLocationNodeName()
        {
            static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Location");
            return s_sNodeName;
        }
        // -----------------------------------------------------------------------------
    }
    // .............................................................................
        typedef ::cppu::WeakImplHelper1 <   XTerminateListener
                                        >   DatabaseDocumentLoader_Base;
        class DatabaseDocumentLoader : public DatabaseDocumentLoader_Base
        {
        private:
            Reference< XDesktop >               m_xDesktop;
            ::std::list< const ODatabaseModelImpl* >  m_aDatabaseDocuments;

        public:
            DatabaseDocumentLoader( const comphelper::ComponentContext& _aContext);

            inline void append(const ODatabaseModelImpl& _rModelImpl )
            {
                m_aDatabaseDocuments.push_back(&_rModelImpl);
            }
            inline void remove(const ODatabaseModelImpl& _rModelImpl) { m_aDatabaseDocuments.remove(&_rModelImpl); }

        private:
            // XTerminateListener
            virtual void SAL_CALL queryTermination( const lang::EventObject& Event ) throw (TerminationVetoException, RuntimeException);
            virtual void SAL_CALL notifyTermination( const lang::EventObject& Event ) throw (RuntimeException);
            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        };

        // .............................................................................
        DatabaseDocumentLoader::DatabaseDocumentLoader( const comphelper::ComponentContext& _aContext )
        {
            acquire();
            try
            {
                m_xDesktop.set( _aContext.createComponent( (rtl::OUString)SERVICE_FRAME_DESKTOP ), UNO_QUERY_THROW );
                m_xDesktop->addTerminateListener( this );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        struct TerminateFunctor : ::std::unary_function<ODatabaseModelImpl* , void>
        {
            void operator()( const ODatabaseModelImpl* _pModelImpl ) const
            {
                try
                {
                    const Reference< XModel2> xModel( _pModelImpl ->getModel_noCreate(),UNO_QUERY_THROW );
                    if ( !xModel->getControllers()->hasMoreElements() )
                    {
                        Reference<util::XCloseable> xCloseable(xModel,UNO_QUERY_THROW);
                        xCloseable->close(sal_False);
                    } // if ( !xModel->getControllers()->hasMoreElements() )
                }
                catch(const CloseVetoException&)
                {
                    throw TerminationVetoException();
                }
            }
        };
        // .............................................................................
        void SAL_CALL DatabaseDocumentLoader::queryTermination( const lang::EventObject& /*Event*/ ) throw (TerminationVetoException, RuntimeException)
        {
            ::std::list< const ODatabaseModelImpl* > aCopy(m_aDatabaseDocuments);
            ::std::for_each(aCopy.begin(),aCopy.end(),TerminateFunctor());
        }

        // .............................................................................
        void SAL_CALL DatabaseDocumentLoader::notifyTermination( const lang::EventObject& /*Event*/ ) throw (RuntimeException)
        {
        }
        // .............................................................................
        void SAL_CALL DatabaseDocumentLoader::disposing( const lang::EventObject& /*Source*/ ) throw (RuntimeException)
        {
        }

//= ODatabaseContext
//==========================================================================
//--------------------------------------------------------------------------
ODatabaseContext::ODatabaseContext( const Reference< XComponentContext >& _rxContext )
    :DatabaseAccessContext_Base(m_aMutex)
    ,m_aContext( _rxContext )
    ,m_aContainerListeners(m_aMutex)
{
    m_pDatabaseDocumentLoader = new DatabaseDocumentLoader( m_aContext );
    ::basic::BasicManagerRepository::registerCreationListener( *this );
}

//--------------------------------------------------------------------------
ODatabaseContext::~ODatabaseContext()
{
    ::basic::BasicManagerRepository::revokeCreationListener( *this );
    if ( m_pDatabaseDocumentLoader )
        m_pDatabaseDocumentLoader->release();
}

// Helper
//------------------------------------------------------------------------------
rtl::OUString ODatabaseContext::getImplementationName_static() throw( RuntimeException )

{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.dba.ODatabaseContext"));
}

//------------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::Create(const Reference< XComponentContext >& _rxContext)
{
    return *( new ODatabaseContext( _rxContext ) );
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > ODatabaseContext::getSupportedServiceNames_static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_DATABASECONTEXT;
    return aSNS;
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODatabaseContext::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseContext::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_static();
}

//--------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::impl_createNewDataSource()
{
    ::rtl::Reference<ODatabaseModelImpl> pImpl( new ODatabaseModelImpl( m_aContext.getLegacyServiceFactory(), *this ) );
    Reference< XDataSource > xDataSource( pImpl->getOrCreateDataSource() );

    return xDataSource.get();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseContext::createInstance(  ) throw (Exception, RuntimeException)
{
    // for convenience of the API user, we ensure the document is fully initialized (effectively: XLoadable::initNew
    // has been called at the DatabaseDocument).
    return impl_createNewDataSource();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseContext::createInstanceWithArguments( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
{
    ::comphelper::NamedValueCollection aArgs( _rArguments );
    ::rtl::OUString sURL = aArgs.getOrDefault( (::rtl::OUString)INFO_POOLURL, ::rtl::OUString() );

    Reference< XInterface > xDataSource;
    if ( sURL.getLength() )
        xDataSource = getObject( sURL );

    if ( !xDataSource.is() )
        xDataSource = impl_createNewDataSource();

    return xDataSource;
}
// DatabaseAccessContext_Base
//------------------------------------------------------------------------------
void ODatabaseContext::disposing()
{
    // notify our listener
    com::sun::star::lang::EventObject aDisposeEvent(static_cast< XContainer* >(this));
    m_aContainerListeners.disposeAndClear(aDisposeEvent);

    // dispose the data sources
    ObjectCache::iterator aEnd = m_aDatabaseObjects.end();
    for (   ObjectCache::iterator   aIter = m_aDatabaseObjects.begin();
            aIter != aEnd;
            ++aIter
        )
    {
        aIter->second->dispose();
    }
    m_aDatabaseObjects.clear();
}

//------------------------------------------------------------------------------
bool ODatabaseContext::getURLForRegisteredObject( const ::rtl::OUString& _rRegisteredName, ::rtl::OUString& _rURL )
{
    if ( !_rRegisteredName.getLength() )
        throw IllegalArgumentException();

    // the config node where all pooling relevant info are stored under
    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_aContext.getLegacyServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);
    if ( aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName( _rRegisteredName ) )
    {
        OConfigurationNode aRegisterObj = aDbRegisteredNamesRoot.openNode( _rRegisteredName );
        aRegisterObj.getNodeValue(getDbLocationNodeName()) >>= _rURL;
        _rURL = SvtPathOptions().SubstituteVariable( _rURL );
        return true;
    }
    return false;
}

// XNamingService
//------------------------------------------------------------------------------
Reference< XInterface >  ODatabaseContext::getRegisteredObject(const rtl::OUString& _rName) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    ::rtl::OUString sURL;
    if ( !getURLForRegisteredObject( _rName, sURL ) )
        throw NoSuchElementException(_rName, *this);

    if ( !sURL.getLength() )
        // there is a registration for this name, but no URL
        throw IllegalArgumentException();

    // check if URL is already loaded
    Reference< XInterface > xExistent = getObject( sURL );
    if ( xExistent.is() )
        return xExistent;

    return loadObjectFromURL( _rName, sURL );
}
// -----------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::loadObjectFromURL(const ::rtl::OUString& _rName,const ::rtl::OUString& _sURL)
{
    INetURLObject aURL( _sURL );
    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        throw NoSuchElementException(_rName, *this);

    try
    {
        ::ucbhelper::Content aContent(_sURL,Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        if ( !aContent.isDocument() )
            throw InteractiveIOException(
                _sURL, *this, InteractionClassification_ERROR, IOErrorCode_NO_FILE
            );
    }
    catch ( const InteractiveIOException& e )
    {
        if  (   ( e.Code == IOErrorCode_NO_FILE )
            ||  ( e.Code == IOErrorCode_NOT_EXISTING )
            ||  ( e.Code == IOErrorCode_NOT_EXISTING_PATH )
            )
        {
            // #i40463# #i39187#
            String sErrorMessage( DBACORE_RESSTRING( RID_STR_FILE_DOES_NOT_EXIST ) );
            ::svt::OFileNotation aTransformer( _sURL );
            sErrorMessage.SearchAndReplaceAscii( "$file$", aTransformer.get( ::svt::OFileNotation::N_SYSTEM ) );

            SQLException aError;
            aError.Message = sErrorMessage;

            throw WrappedTargetException( _sURL, Reference< XNamingService >( this ), makeAny( aError ) );
        }
        throw WrappedTargetException( _sURL, Reference< XNamingService >( this ), ::cppu::getCaughtException() );
    }
    catch( const Exception& )
    {
        throw WrappedTargetException( _sURL, Reference<XNamingService>(this), ::cppu::getCaughtException() );
    }

    ::rtl::Reference< ODatabaseModelImpl > pExistent;
    ObjectCache::iterator aFind = m_aDatabaseObjects.find(_sURL);

    if ( aFind != m_aDatabaseObjects.end() ) // we found a object registered under the URL
    {   // register it under the new name
        pExistent = aFind->second;
        m_aDatabaseObjects.insert( ObjectCache::value_type( _rName, pExistent.get() ) );
        m_aDatabaseObjects.erase( aFind );
    }

    if ( !pExistent.get() )
    {
        pExistent.set( new ODatabaseModelImpl( _rName, m_aContext.getLegacyServiceFactory(), *this ) );

        Reference< XModel > xModel( pExistent->createNewModel_deliverOwnership( false ), UNO_SET_THROW );
        Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );

        ::comphelper::NamedValueCollection aArgs;
        aArgs.put( "URL", _sURL );
        aArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );
        aArgs.put( "InteractionHandler", m_aContext.createComponent( "com.sun.star.sdb.InteractionHandler" ) );

        Sequence< PropertyValue > aResource( aArgs.getPropertyValues() );
        xLoad->load( aResource );
        xModel->attachResource( _sURL, aResource );

        ::utl::CloseableComponent aEnsureClose( xModel );
    } // if ( !pExistent.get() )

    setTransientProperties( _sURL, *pExistent );

    return pExistent->getOrCreateDataSource().get();
}
// -----------------------------------------------------------------------------
void ODatabaseContext::appendAtTerminateListener(const ODatabaseModelImpl& _rDataSourceModel)
{
    m_pDatabaseDocumentLoader->append(_rDataSourceModel);
}
// -----------------------------------------------------------------------------
void ODatabaseContext::removeFromTerminateListener(const ODatabaseModelImpl& _rDataSourceModel)
{
    m_pDatabaseDocumentLoader->remove(_rDataSourceModel);
}
// -----------------------------------------------------------------------------
void ODatabaseContext::setTransientProperties(const ::rtl::OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel )
{
    if ( m_aDatasourceProperties.end() == m_aDatasourceProperties.find(_sURL) )
        return;
    try
    {
        ::rtl::OUString sAuthFailedPassword;
        Reference< XPropertySet > xDSProps( _rDataSourceModel.getOrCreateDataSource(), UNO_QUERY_THROW );
        const Sequence< PropertyValue >& rSessionPersistentProps = m_aDatasourceProperties[_sURL];
        const PropertyValue* pProp = rSessionPersistentProps.getConstArray();
        const PropertyValue* pPropsEnd = rSessionPersistentProps.getConstArray() + rSessionPersistentProps.getLength();
        for ( ; pProp != pPropsEnd; ++pProp )
        {
            if ( pProp->Name.equalsAscii( "AuthFailedPassword" ) )
            {
                OSL_VERIFY( pProp->Value >>= sAuthFailedPassword );
            }
            else
            {
                xDSProps->setPropertyValue( pProp->Name, pProp->Value );
            }
        }

        _rDataSourceModel.m_sFailedPassword = sAuthFailedPassword;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void ODatabaseContext::registerObject(const rtl::OUString& _rName, const Reference< XInterface > & _rxObject) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    if ( !_rName.getLength() )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

    Reference< XDocumentDataSource > xDocDataSource( _rxObject, UNO_QUERY );
    Reference< XModel > xModel( xDocDataSource.is() ? xDocDataSource->getDatabaseDocument() : Reference< XOfficeDatabaseDocument >(), UNO_QUERY );
    if ( !xModel.is() )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );

    ::rtl::OUString sURL = xModel->getURL();
    if ( !sURL.getLength() )
        throw IllegalArgumentException( DBACORE_RESSTRING( RID_STR_DATASOURCE_NOT_STORED ), *this, 2 );

    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

    if ( aDbRegisteredNamesRoot.isValid() )
    {
        OConfigurationNode oDataSourceRegistration;
        // the sub-node for the concrete registration
        if (aDbRegisteredNamesRoot.hasByName(_rName))
            oDataSourceRegistration = aDbRegisteredNamesRoot.openNode(_rName);
        else
            oDataSourceRegistration = aDbRegisteredNamesRoot.createNode(_rName);

        // set the values
        oDataSourceRegistration.setNodeValue(getDbNameNodeName(), makeAny(_rName));
        oDataSourceRegistration.setNodeValue(getDbLocationNodeName(), makeAny(sURL));
        aDbRegisteredNamesRoot.commit();
    }

    ODatabaseSource::setName( xDocDataSource, _rName, ODatabaseSource::DBContextAccess() );

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rName), makeAny(_rxObject), Any());
    m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
}

//------------------------------------------------------------------------------
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

        if (aProperties.getLength())
        {
            const Property* pProperties = aProperties.getConstArray();
            for ( sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties )
            {
                if  (   ( ( pProperties->Attributes & PropertyAttribute::TRANSIENT) != 0 )
                    &&  ( ( pProperties->Attributes & PropertyAttribute::READONLY) == 0 )
                    )
                {
                    // found such a property
                    aRememberProps.put( pProperties->Name, xSource->getPropertyValue( pProperties->Name ) );
                }
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // additionally, remember the "failed password", which is not available as property
    // #i86178# / 2008-02-19 / frank.schoenheit@sun.com
    aRememberProps.put( "AuthFailedPassword", _rModelImpl.m_sFailedPassword );

    ::rtl::OUString sDocumentURL( _rModelImpl.getURL() );
    if ( m_aDatabaseObjects.find( sDocumentURL ) != m_aDatabaseObjects.end() )
    {
        m_aDatasourceProperties[ sDocumentURL ] = aRememberProps.getPropertyValues();
    }
    else if ( m_aDatabaseObjects.find( _rModelImpl.m_sName ) != m_aDatabaseObjects.end() )
    {
        m_aDatasourceProperties[ _rModelImpl.m_sName ] = aRememberProps.getPropertyValues();
    }
    else
    {
        OSL_ENSURE( ( sDocumentURL.getLength() == 0 ) && ( _rModelImpl.m_sName.getLength() == 0 ),
            "ODatabaseContext::storeTransientProperties: a non-empty data source which I do not know?!" );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseContext::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    m_aContainerListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseContext::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    m_aContainerListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void ODatabaseContext::revokeObject(const rtl::OUString& _rName) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    Reference< XInterface > xExistent;

    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_aContext.getLegacyServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);
    if ( aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName(_rName) )
    {
        OConfigurationNode aThisDriverSettings = aDbRegisteredNamesRoot.openNode(_rName);
        ::rtl::OUString sURL;
        aThisDriverSettings.getNodeValue(getDbLocationNodeName()) >>= sURL;
        sURL = SvtPathOptions().SubstituteVariable(sURL);

        // check if URL is already loaded
        ObjectCacheIterator aExistent = m_aDatabaseObjects.find(sURL);
        if ( aExistent != m_aDatabaseObjects.end() )
            m_aDatabaseObjects.erase(aExistent);
        if (!aDbRegisteredNamesRoot.removeNode(_rName))
            throw Exception(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("An unexpected und unknown error occured.")), static_cast<XNamingService*>(this));
        aDbRegisteredNamesRoot.commit();
    }
    else
        throw NoSuchElementException(_rName,*this);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rName), Any(), makeAny(xExistent));
        // note that xExistent may be empty, in case somebody removed the data source while it is not alive at this moment
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementRemoved(aEvent);
}

// ::com::sun::star::container::XElementAccess
//------------------------------------------------------------------------------
Type ODatabaseContext::getElementType(  ) throw(RuntimeException)
{
    return::getCppuType(static_cast<Reference<XDataSource>*>(NULL));
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::hasElements(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    return 0 != getElementNames().getLength();
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XEnumeration >  ODatabaseContext::createEnumeration(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByName(static_cast<XNameAccess*>(this));
}

// ::com::sun::star::container::XNameAccess
//------------------------------------------------------------------------------
Any ODatabaseContext::getByName(const rtl::OUString& _rName) throw( NoSuchElementException,
                                                          WrappedTargetException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);
    if ( !_rName.getLength() )
        throw NoSuchElementException(_rName, *this);

    try
    {
        Reference< XInterface > xExistent = getObject(_rName);
        if ( xExistent.is() )
            return makeAny(xExistent);

        // see whether this is an registered name
        ::rtl::OUString sURL;
        if ( getURLForRegisteredObject( _rName, sURL ) )
        {
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
    catch (NoSuchElementException&)
    {   // let these exceptions through
        throw;
    }
    catch (WrappedTargetException&)
    {   // let these exceptions through
        throw;
    }
    catch (RuntimeException&)
    {   // let these exceptions through
        throw;
    }
    catch (Exception& e)
    {   // exceptions other than the speciafied ones -> wrap
        throw WrappedTargetException(_rName, *this, makeAny( e ) );
    }
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > ODatabaseContext::getElementNames(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    DECLARE_STL_USTRINGACCESS_MAP( bool , TNameMap);
    TNameMap aRet;

    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_aContext.getLegacyServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

    Sequence< ::rtl::OUString> aSeq;
    if ( aDbRegisteredNamesRoot.isValid() )
    {
        aSeq = aDbRegisteredNamesRoot.getNodeNames();
    } // if ( aDbRegisteredNamesRoot.isValid() )

    return aSeq;
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::hasByName(const rtl::OUString& _rName) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_aContext.getLegacyServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

    return aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName(_rName);
}
// -----------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::getObject(const ::rtl::OUString& _rName)
{
    ObjectCacheIterator aFind = m_aDatabaseObjects.find(_rName);
    Reference< XInterface > xExistent;
    if ( aFind != m_aDatabaseObjects.end() )
        xExistent = aFind->second->getOrCreateDataSource();
    return xExistent;
}
// -----------------------------------------------------------------------------
void ODatabaseContext::registerPrivate(const ::rtl::OUString& _sName
                                       ,const ::rtl::Reference<ODatabaseModelImpl>& _pModelImpl)
{
    //  OSL_ENSURE(m_aDatabaseObjects.find(_sName) == m_aDatabaseObjects.end(),"Name already exists!");
    if ( m_aDatabaseObjects.find(_sName) == m_aDatabaseObjects.end() )
    {
        m_aDatabaseObjects.insert(ObjectCache::value_type(_sName,_pModelImpl.get()));
        setTransientProperties( _sName, *_pModelImpl );
    }
}
// -----------------------------------------------------------------------------
void ODatabaseContext::deregisterPrivate(const ::rtl::OUString& _sName)
{
    m_aDatabaseObjects.erase(_sName);
}
// -----------------------------------------------------------------------------
void ODatabaseContext::nameChangePrivate(const ::rtl::OUString& _sOldName, const ::rtl::OUString& _sNewName)
{
    ObjectCache::iterator aFind = m_aDatabaseObjects.find(_sOldName);
    registerPrivate(_sNewName,aFind->second);
    m_aDatabaseObjects.erase(aFind);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL ODatabaseContext::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > ODatabaseContext::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// -----------------------------------------------------------------------------
void ODatabaseContext::onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager )
{
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
}

//........................................................................
}   // namespace dbaccess
//........................................................................
