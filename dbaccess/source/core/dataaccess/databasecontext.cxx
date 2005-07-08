/*************************************************************************
 *
 *  $RCSfile: databasecontext.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:35:59 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_
#include "databasecontext.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_INTERACTIONCLASSIFICATION_HPP_
#include <com/sun/star/task/InteractionClassification.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef COMPHELPER_EVENTLISTENERHELPER_HXX
#include <comphelper/evtlistenerhlp.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#include <svtools/filenotation.hxx>
#ifndef _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_
#include "databasedocument.hxx"
#endif

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
    static ::dbaccess::OOneInstanceAutoRegistration< ::dbaccess::ODatabaseContext > aODatabaseContext_AutoRegistration;
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
    }

//= ODatabaseContext
//==========================================================================
DBG_NAME(ODatabaseContext)
//--------------------------------------------------------------------------
Reference< XInterface >
ODatabaseContext_CreateInstance(const Reference< XMultiServiceFactory >  & xServiceManager)
{
    return (*new ODatabaseContext(xServiceManager));
}

//--------------------------------------------------------------------------
ODatabaseContext::ODatabaseContext(const Reference< XMultiServiceFactory >  & xServiceManager)
                       :DatabaseAccessContext_Base(m_aMutex)
                       ,m_xServiceManager(xServiceManager)
                       ,m_aContainerListeners(m_aMutex)
{
    DBG_CTOR(ODatabaseContext,NULL);
}

//--------------------------------------------------------------------------
ODatabaseContext::~ODatabaseContext()
{
    DBG_DTOR(ODatabaseContext,NULL);
}

// Helper
//------------------------------------------------------------------------------
rtl::OUString ODatabaseContext::getImplementationName_Static() throw( RuntimeException )

{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.dba.ODatabaseContext"));
}

//------------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return ODatabaseContext_CreateInstance(_rxFactory);
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > ODatabaseContext::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_DATABASECONTEXT;
    return aSNS;
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODatabaseContext::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseContext::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseContext::createInstance(  ) throw (Exception, RuntimeException)
{
    ::rtl::Reference<ODatabaseModelImpl> pImpl(new ODatabaseModelImpl(m_xServiceManager));
    pImpl->m_pDBContext = this;
    Reference< XDataSource > xDataSource( pImpl->getDataSource() );
    return xDataSource.get();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseContext::createInstanceWithArguments( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
{
    const Any* pIter = _rArguments.getConstArray();
    const Any* pEnd = pIter + _rArguments.getLength();
    NamedValue aValue;
    Reference< XInterface > xExistent;
    ::rtl::OUString sURL;
    for (; pIter != pEnd; ++pIter)
    {
        if ( (*pIter >>= aValue) && aValue.Name == INFO_POOLURL && (aValue.Value >>= sURL) )
        {
            xExistent = getObject(sURL);
            break;
        }
    }
    if ( !xExistent.is() )
        xExistent = createInstance();

    return xExistent;
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
        OSL_ENSURE(aIter->second->m_refCount != 0,"Object is already disposed");
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
        m_xServiceManager, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);
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
        ::ucb::Content aContent(_sURL,Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        if ( !aContent.isDocument() )
            throw InteractiveIOException(
                _sURL, *this, InteractionClassification_ERROR, IOErrorCode_NO_FILE
            );
    }
    catch(InteractiveIOException e)
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
        throw WrappedTargetException( _sURL, Reference< XNamingService >( this ), makeAny( e ) );
    }
    catch(Exception e)
    {
        throw WrappedTargetException( _sURL, Reference<XNamingService>(this), makeAny( e ) );
    }

    Reference< XInterface > xExistent;
    ObjectCache::iterator aFind = m_aDatabaseObjects.find(_sURL);
    if ( aFind != m_aDatabaseObjects.end() ) // we found a object registered under the URL
    { // register it under the new name
        m_aDatabaseObjects.insert(ObjectCache::value_type(_rName,aFind->second));
        xExistent = aFind->second->getDataSource();
        m_aDatabaseObjects.erase(aFind);
    }
    if ( !xExistent.is() )
    {
        ::rtl::Reference<ODatabaseModelImpl> pImpl(new ODatabaseModelImpl(_rName,m_xServiceManager,this));
        xExistent = pImpl->getDataSource().get();

        Sequence< PropertyValue > aArgs(1);
        aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
        aArgs[0].Value <<= _sURL;

        Reference<XModel> xModel = pImpl->createNewModel_deliverOwnership();
        DBG_ASSERT( xModel.is(), "ODatabaseContext::loadObjectFromURL: no model?" );
        // calls registerPrivate in attachResource
        xModel->attachResource(_sURL,aArgs);

        // since the model has been newly created, we're its owner. However, we do not
        // really need it, we ust used it for loading the document. So, dispose it to prevent
        // leaks
        // #i50905# / 2005-06-20 / frank.schoenheit@sun.com
        ::comphelper::disposeComponent( xModel );
    }

    setTransientProperties(_sURL,xExistent);
    return xExistent;
}
// -----------------------------------------------------------------------------
void ODatabaseContext::setTransientProperties(const ::rtl::OUString& _sURL, const Reference< XInterface > & _rxObject)
{
    // check if we have any session persistent properties to initialize the new object with
    if ( m_aDatasourceProperties.end() != m_aDatasourceProperties.find(_sURL) )
    {   // yes, we do ....
        Reference< XPropertySet > xDSProps(_rxObject, UNO_QUERY);
        if (xDSProps.is())
        {
            const Sequence< PropertyValue >& rSessionPersistentProps = m_aDatasourceProperties[_sURL];
            const PropertyValue* pSessionPersistentProps = rSessionPersistentProps.getConstArray();
            try
            {

                for (sal_Int32 i=0; i<rSessionPersistentProps.getLength(); ++i, ++pSessionPersistentProps)
                {
                    xDSProps->setPropertyValue(pSessionPersistentProps->Name, pSessionPersistentProps->Value);
                }
            }
            catch(Exception&)
            {
                DBG_ERROR("ODatabaseContext::setTransientProperties: could not set a session-persistent property on the data source!");
            }
        }
        else
            DBG_ERROR("ODatabaseContext::setTransientProperties: missing an interface!");
    }
}

//------------------------------------------------------------------------------
void ODatabaseContext::registerObject(const rtl::OUString& _rName, const Reference< XInterface > & _rxObject) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(DatabaseAccessContext_Base::rBHelper.bDisposed);

    Reference<XDocumentDataSource> xDocumentDataSource(_rxObject,UNO_QUERY);
    Reference< XModel > xModel(xDocumentDataSource.is() ? xDocumentDataSource->getDatabaseDocument() : Reference< XOfficeDatabaseDocument >(),UNO_QUERY);
    Reference< XComponent > xComponent(_rxObject,UNO_QUERY);
    if ( !_rName.getLength() || !xComponent.is() || !xModel.is() )
        throw IllegalArgumentException(::rtl::OUString(),*this,1);

    ::rtl::OUString sURL = xModel->getURL();
    if ( !sURL.getLength() )
        throw IllegalArgumentException(DBACORE_RESSTRING( RID_STR_DATASOURCE_NOT_STORED ),*this,2);

    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

    if ( aDbRegisteredNamesRoot.isValid() )
    {
        OConfigurationNode aThisDriverSettings;
        // the sub-node for this driver
        if (aDbRegisteredNamesRoot.hasByName(_rName))
            aThisDriverSettings = aDbRegisteredNamesRoot.openNode(_rName);
        else
            aThisDriverSettings = aDbRegisteredNamesRoot.createNode(_rName);

        // set the values
        aThisDriverSettings.setNodeValue(getDbNameNodeName(), makeAny(_rName));
        aThisDriverSettings.setNodeValue(getDbLocationNodeName(), makeAny(sURL));
        aDbRegisteredNamesRoot.commit();
    }

//  registerPrivate(sURL,_rxObject);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rName), makeAny(_rxObject), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}

//------------------------------------------------------------------------------
void SAL_CALL ODatabaseContext::disposing( const com::sun::star::lang::EventObject& _rSource ) throw(RuntimeException)
{
    Reference< XInterface > xSource(_rSource.Source, UNO_QUERY);
        // the query is for normalizing, else it could be XInterface part of any XInterface-derived concrete interface
        // the object implements
    ObjectCacheIterator aLookup;
    for (   aLookup = m_aDatabaseObjects.begin();
            aLookup != m_aDatabaseObjects.end();
            ++aLookup
        )
    {
        Reference< XInterface > xDataSource(aLookup->second->getDataSource(false), UNO_QUERY);
        if ( xDataSource == xSource )
            break;
    }
    DBG_ASSERT(aLookup != m_aDatabaseObjects.end(), "ODatabaseContext::disposing(EventObject): where does this come from (not from my data sources)?");
    if (aLookup != m_aDatabaseObjects.end())
    {
        Sequence< PropertyValue > aRememberProps;

        try
        {
            // get the info about the properties, check which ones are transient and not readonly
            Reference< XPropertySet > xSourceProps(xSource, UNO_QUERY);
            Reference< XPropertySetInfo > xSetInfo;
            if (xSourceProps.is())
                xSetInfo = xSourceProps->getPropertySetInfo();
            Sequence< Property > aProperties;
            if (xSetInfo.is())
                aProperties = xSetInfo->getProperties();

            if (aProperties.getLength())
            {
                const Property* pProperties = aProperties.getConstArray();
                for (sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties)
                {
                    if  (   ((pProperties->Attributes & PropertyAttribute::TRANSIENT) != 0)
                        &&  ((pProperties->Attributes & PropertyAttribute::READONLY) == 0)
                        )
                    {
                        // found such a property
                        sal_Int32 nTilNow = aRememberProps.getLength();
                        aRememberProps.realloc(nTilNow + 1);
                        aRememberProps[nTilNow] = PropertyValue(pProperties->Name, 0, xSourceProps->getPropertyValue(pProperties->Name), PropertyState_DIRECT_VALUE);
                    }
                }
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("ODatabaseContext::disposing(EventObject): could not collect the session-persistent properties!");
        }

        m_aDatasourceProperties[aLookup->first] = aRememberProps;
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
        m_xServiceManager, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);
    if ( aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName(_rName) )
    {
        OConfigurationNode aThisDriverSettings = aDbRegisteredNamesRoot.openNode(_rName);
        ::rtl::OUString sURL;
        aThisDriverSettings.getNodeValue(getDbLocationNodeName()) >>= sURL;
        sURL = SvtPathOptions().SubstituteVariable(sURL);

        // check if URL is already loaded
        ObjectCacheIterator aExistent = m_aDatabaseObjects.find(sURL);
        if ( aExistent != m_aDatabaseObjects.end() )
        {
            xExistent = aExistent->second->getDataSource(false);
            if (xExistent.is())
            {
                Reference< XComponent > xComponent(xExistent, UNO_QUERY);
                if ( xComponent.is() )
                {
                    Reference<XEventListener> xListenerHelper(*this,UNO_QUERY);
                    xComponent->removeEventListener(xListenerHelper);
                }
            }
            m_aDatabaseObjects.erase(aExistent);
        }
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
        m_xServiceManager, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

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
        m_xServiceManager, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

    return aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName(_rName);
}
// -----------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::getObject(const ::rtl::OUString& _rName)
{
    ObjectCacheIterator aFind = m_aDatabaseObjects.find(_rName);
    Reference< XInterface > xExistent;
    if ( aFind != m_aDatabaseObjects.end() )
        xExistent = aFind->second->getDataSource();
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

        Reference<XDataSource> xDs = _pModelImpl->getDataSource();
        setTransientProperties(_sName,xDs);
        // add as dispose listener to the data source object, so we know when it's dying to save the session-persistent
        // properties
        Reference< XComponent > xComponent(xDs,UNO_QUERY);
        if ( xComponent.is() )
            xComponent->addEventListener(this);
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
//........................................................................
}   // namespace dbaccess
//........................................................................

