/*************************************************************************
 *
 *  $RCSfile: databasecontext.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-25 07:42:28 $
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
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
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

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;

#define REGISTRYFILE    "dbaccess.rdb"


//==========================================================================

extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext()
{
    static OOneInstanceAutoRegistration< dbaccess::ODatabaseContext > aODatabaseContext_AutoRegistration;
}

//........................................................................
namespace dbaccess
{
//........................................................................

//============================================================
//= ODatabaseContext
//==========================================================================
DBG_NAME(ODatabaseContext)
//--------------------------------------------------------------------------
Reference< XInterface >
    ODatabaseContext_CreateInstance(const Reference< XMultiServiceFactory >  & xServiceManager)
{
    Reference< XInterface >  xService(*new ODatabaseContext(xServiceManager));
    return xService;
}

//--------------------------------------------------------------------------
ODatabaseContext::ODatabaseContext(const Reference< XMultiServiceFactory >  & xServiceManager)
                       :DatabaseAccessContext_Base(m_aMutex)
                       ,m_xServiceManager(xServiceManager)
{
    DBG_CTOR(ODatabaseContext,NULL);

    // get our the configuration root node
    m_aRootNode = OConfigurationTreeRoot::createWithServiceFactory(m_xServiceManager,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.DataAccess/DataSources")), -1, OConfigurationTreeRoot::CM_PREFER_UPDATABLE);

    DBG_ASSERT(m_aRootNode.isSetNode(), "ODatabaseContext::ODatabaseContext: our node should be a set!");
    // enable auto name encoding
    m_aRootNode.setEscape(m_aRootNode.isSetNode());
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
    return rtl::OUString::createFromAscii("com.sun.star.sdb.ODatabaseContext");
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

// ::com::sun::star::lang::XUnoTunnel
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODatabaseContext::getUnoTunnelImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL ODatabaseContext::getSomething(const Sequence<sal_Int8>& _rIdentifier) throw( RuntimeException )
{
    if (_rIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}

// DatabaseAccessContext_Base
//------------------------------------------------------------------------------
void ODatabaseContext::disposing()
{
    for (   ObjectCache::iterator   aIter = m_aDatabaseObjects.begin();
            aIter != m_aDatabaseObjects.end();
            ++aIter
        )
    {
        Reference< XComponent > xComp(aIter->second.get(), UNO_QUERY);
        if (xComp.is())
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

    ObjectCacheIterator aExistent = m_aDatabaseObjects.find(_rName);
    if (aExistent != m_aDatabaseObjects.end())
    {
        Reference< XInterface > xExistent = aExistent->second.get();
        if (xExistent.is())
            return xExistent;
        // the adapter still exists, but the object is already dead
        m_aDatabaseObjects.erase(aExistent);
    }

    OConfigurationNode aObjectBase = getObjectNode(_rName, sal_False);
    if (!aObjectBase.isValid())
        throw NoSuchElementException((::rtl::OUString::createFromAscii("There is no object named ") += _rName) += ::rtl::OUString::createFromAscii("."), Reference<XNamingService>(this));

    Reference< XInterface > xNewObject = *(new ODatabaseSource(*this, aObjectBase, _rName, m_xServiceManager));
    m_aDatabaseObjects[_rName] = WeakReferenceHelper(xNewObject);

    return xNewObject;
}

//--------------------------------------------------------------------------
OConfigurationNode ODatabaseContext::getObjectNode(const ::rtl::OUString& _rTitle, sal_Bool _bCreate) throw()
{
    DBG_ASSERT(m_aRootNode.isValid(), "ODatabaseContext::getObjectNode : don't call without a configuration node!");
    if (!m_aRootNode.isValid())
        return OConfigurationNode();

    // if the node exists, just return it
    if (m_aRootNode.hasByName(_rTitle))
        return m_aRootNode.openNode(_rTitle);

    // if we're not allowed to create it -> outta here
    if (!_bCreate)
        return OConfigurationNode();

    // create the node
    DBG_ASSERT(m_aRootNode.isSetNode(), "ODatabaseContext::getObjectNode: the top-level context node is no set node!");

    // the configuration does not support different types of operations in one transaction, so we must commit
    // before and after we create the new node, to ensure, that every transaction we ever do contains only
    // one type of operation (insert, remove, update)
    OSL_VERIFY(m_aRootNode.commit());
    OConfigurationNode aReturn = m_aRootNode.createNode(_rTitle);
    OSL_VERIFY(m_aRootNode.commit());
    return aReturn;
}

//------------------------------------------------------------------------------
void ODatabaseContext::registerObject(const rtl::OUString& _rName, const Reference< XInterface > & _rxObject) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_aRootNode.isValid() || DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    // solaris compiler needs a construct like this and can't work with comphelper::getImplementation ...
    Reference< XUnoTunnel > xTunnel(_rxObject, UNO_QUERY);
    ODatabaseSource* pObjectImpl = NULL;
    if (xTunnel.is())
        pObjectImpl = reinterpret_cast<ODatabaseSource*> (xTunnel->getSomething(ODatabaseSource::getUnoTunnelImplementationId()));
    if (!pObjectImpl)
        throw IllegalArgumentException();

    if (pObjectImpl->isContainerElement())
        // we do not allow to change an object's registration name in this way
        throw IllegalArgumentException();

    if (!_rName.getLength())
        throw IllegalArgumentException();

    if (m_aRootNode.hasByName(_rName))
        throw ElementExistException();


    sal_Bool bSuccess = sal_False;
    OConfigurationNode aObjectNode = getObjectNode(_rName, sal_True);
    if (!aObjectNode.isValid())
        // TODO: a better error message
        throw InvalidRegistryException();

    pObjectImpl->inserted(*this, _rName, aObjectNode.cloneAsRoot());
    pObjectImpl->flush();

    // add the object to our bag
    m_aDatabaseObjects[_rName] = WeakReferenceHelper(_rxObject);
}

//------------------------------------------------------------------------------
void ODatabaseContext::revokeObject(const rtl::OUString& _rName) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_aRootNode.isValid() || DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    OConfigurationNode aObjectNode = getObjectNode(_rName, sal_False);
    if (!aObjectNode.isValid())
        throw NoSuchElementException();

    ObjectCacheIterator aExistent = m_aDatabaseObjects.find(_rName);
    if (aExistent != m_aDatabaseObjects.end())
    {
        Reference< XInterface > xExistent = aExistent->second.get();
        if (xExistent.is())
        {
            // solaris compiler needs a construct like this and can't work with comphelper::getImplementation ...
            Reference< XUnoTunnel > xTunnel(xExistent, UNO_QUERY);
            ODatabaseSource* pObjectImpl = NULL;
            if (xTunnel.is())
                pObjectImpl = reinterpret_cast<ODatabaseSource*> (xTunnel->getSomething(ODatabaseSource::getUnoTunnelImplementationId()));
            DBG_ASSERT(pObjectImpl, "ODatabaseContext::revokeObject : there is an object which is no ODatabaseSource !");
            if (pObjectImpl)
                pObjectImpl->removed();
        }
        m_aDatabaseObjects.erase(aExistent);

        // the configuration does not support different types of operations in one transaction, so we must commit
        // before and after we create the new node, to ensure, that every transaction we ever do contains only
        // one type of operation (insert, remove, update)
        OSL_VERIFY(m_aRootNode.commit());
        if (!m_aRootNode.removeNode(_rName))
            throw Exception(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("An unexpected und unknown error occured.")), static_cast<XNamingService*>(this));
        OSL_VERIFY(m_aRootNode.commit());
    }
    else
        DBG_ERROR("ODatabaseContext::revokeObject: inconsistent state!");
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
    if (!m_aRootNode.isValid() || DatabaseAccessContext_Base::rBHelper.bDisposed)
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
    try
    {
        Reference< XDataSource > xObject(getRegisteredObject(_rName), UNO_QUERY);
            // will throw an NoSuchElementException if neccessary
        return makeAny(xObject);
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
        throw WrappedTargetException(rtl::OUString(), *this, makeAny(e));
    }
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > ODatabaseContext::getElementNames(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_aRootNode.isValid() || DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    return m_aRootNode.getNodeNames();
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::hasByName(const rtl::OUString& _rName) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_aRootNode.isValid() || DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    return m_aRootNode.hasByName(_rName);
}

//........................................................................
}   // namespace dbaccess
//........................................................................

