/*************************************************************************
 *
 *  $RCSfile: databasecontext.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:08:49 $
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

//============================================================
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
    return rtl::OUString::createFromAscii("com.sun.star.comp.dba.ODatabaseContext");
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
    return *(new ODatabaseSource(m_xServiceManager,this));
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
    for (   ObjectCache::iterator   aIter = m_aDatabaseObjects.begin();
            aIter != m_aDatabaseObjects.end();
            ++aIter
        )
    {
        aIter->second.second = WeakReferenceHelper();
        Reference< XComponent > xComp(aIter->second.first.get(), UNO_QUERY);
        if ( xComp.is() )
            xComp->dispose();
    }
    m_aDatabaseObjects.clear();
}

// XNamingService
//------------------------------------------------------------------------------
Reference< XInterface >  ODatabaseContext::getRegisteredObject(const rtl::OUString& _rName) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    if (!_rName.getLength())
        throw IllegalArgumentException();

    ::rtl::OUString sURL;
    // the config node where all pooling relevant info are stored under
    OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
        m_xServiceManager, getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);
    if ( aDbRegisteredNamesRoot.isValid() && aDbRegisteredNamesRoot.hasByName(_rName) )
    {
        OConfigurationNode aThisDriverSettings = aDbRegisteredNamesRoot.openNode(_rName);
        aThisDriverSettings.getNodeValue(getDbLocationNodeName()) >>= sURL;
        sURL = SvtPathOptions().SubstituteVariable(sURL);

        // check if URL is already loaded
        Reference< XInterface > xExistent = getObject(sURL);
        if ( xExistent.is() )
            return xExistent;
    }
    if ( !sURL.getLength() )
        throw IllegalArgumentException();

    return loadObjectFromURL(_rName,sURL);
}
// -----------------------------------------------------------------------------
Reference< XInterface > ODatabaseContext::loadObjectFromURL(const ::rtl::OUString& _rName,const ::rtl::OUString& _sURL)
{
    try
    {
        ::ucb::Content aContent(_sURL,Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        if ( !aContent.isDocument() )
            throw IllegalArgumentException(_sURL, Reference<XNamingService>(this),1);
    }
    catch(IllegalArgumentException)
    {
        throw;
    }
    catch(Exception)
    {
        throw IllegalArgumentException(_sURL, Reference<XNamingService>(this),1);
        OSL_ENSURE(0,"Exception catched!");
    }

    Reference<XImporter> xImporter(m_xServiceManager->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sdb.DBFilter"))),UNO_QUERY);
    Reference< XInterface > xExistent = *(new ODatabaseSource(*this, _rName, m_xServiceManager,this));
    xImporter->setTargetDocument(Reference<XComponent>(xExistent,UNO_QUERY));

    Reference<XFilter> xFilter(xImporter,UNO_QUERY);

    Sequence< PropertyValue > aArgs(1);
    aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
    aArgs[0].Value <<= _sURL;

    Reference<XModel> xModel(xExistent,UNO_QUERY);
    xModel->attachResource(_sURL,aArgs);

    xFilter->filter(aArgs);

    // check if we have any session persistent properties to initialize the new object with
    if ( m_aDatasourceProperties.end() != m_aDatasourceProperties.find(_sURL) )
    {   // yes, we do ....
        Reference< XPropertySet > xDSProps(xExistent, UNO_QUERY);
        if (xDSProps.is())
        {
            const Sequence< PropertyValue >& rSessionPersistentProps = m_aDatasourceProperties[_sURL];
            const PropertyValue* pSessionPersistentProps = rSessionPersistentProps.getConstArray();

            for (sal_Int32 i=0; i<rSessionPersistentProps.getLength(); ++i, ++pSessionPersistentProps)
            {
                try
                {
                    xDSProps->setPropertyValue(pSessionPersistentProps->Name, pSessionPersistentProps->Value);
                }
                catch(Exception&)
                {
                    DBG_ERROR("ODatabaseContext::getRegisteredObject: could not set a session-persistent property on the data source!");
                }
            }
        }
        else
            DBG_ERROR("ODatabaseContext::getRegisteredObject: missing an interface!");
    }
    return xExistent;
}

//------------------------------------------------------------------------------
void ODatabaseContext::registerObject(const rtl::OUString& _rName, const Reference< XInterface > & _rxObject) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed )
        throw DisposedException();

    Reference< XComponent > xComponent(_rxObject,UNO_QUERY);
    Reference<XModel> xModel(_rxObject,UNO_QUERY);
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
    registerPrivate(sURL,_rxObject);

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
        Reference< XInterface > xDataSource(aLookup->second.first.get(), UNO_QUERY);
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
        m_aDatabaseObjects.erase(aLookup);
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
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed )
        throw DisposedException();

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
            xExistent = aExistent->second.first.get();
            if (xExistent.is())
            {
                Reference< XComponent > xComponent(xExistent, UNO_QUERY);
                if ( xComponent.is() )
                {
                    Reference<XEventListener> xListenerHelper(aExistent->second.second.get(),UNO_QUERY);
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
        throw NoSuchElementException();

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
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

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
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed )
        throw DisposedException();
    if ( !_rName.getLength() )
        throw NoSuchElementException(_rName, *this);

    try
    {
        Reference< XInterface > xExistent = getObject(_rName);
        if ( xExistent.is() )
            return makeAny(xExistent);

        try
        {
            xExistent = getRegisteredObject(_rName);
        }
        catch(Exception)
        {
            // will throw an NoSuchElementException if neccessary
            if ( !xExistent.is() )
            {
                // try to load this as URL
                xExistent = loadObjectFromURL(_rName,_rName);
            }
        }
        return makeAny(xExistent);
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
        throw NoSuchElementException(_rName, *this);
    }
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > ODatabaseContext::getElementNames(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

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
    if ( DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

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
    {
        xExistent = aFind->second.first.get();
        if ( !xExistent.is() )
        {
            // the adapter still exists, but the object is already dead
            m_aDatabaseObjects.erase(aFind);
        }
    }
    return xExistent;
}
// -----------------------------------------------------------------------------
void ODatabaseContext::registerPrivate(const ::rtl::OUString& _sName, const Reference< XInterface >& _xObject)
{
    //  OSL_ENSURE(m_aDatabaseObjects.find(_sName) == m_aDatabaseObjects.end(),"Name already exists!");
    if ( m_aDatabaseObjects.find(_sName) == m_aDatabaseObjects.end() )
    {
        m_aDatabaseObjects.insert(
                ObjectCache::value_type(_sName,
                    ObjectCacheType(WeakReferenceHelper(_xObject),WeakReferenceHelper(*this))));
        // add as dispose listener to the data source object, so we know when it's dying to save the session-persistent
        // properties
        Reference< XComponent > xComponent(_xObject,UNO_QUERY);
        if ( xComponent.is() )
            xComponent->addEventListener(this);
    }
}
// -----------------------------------------------------------------------------
void ODatabaseContext::nameChangePrivate(const ::rtl::OUString& _sOldName, const ::rtl::OUString& _sNewName)
{
    registerPrivate(_sNewName,getObject(_sOldName));
}
//........................................................................
}   // namespace dbaccess
//........................................................................

