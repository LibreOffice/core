/*************************************************************************
 *
 *  $RCSfile: databasecontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:19:39 $
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
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
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

#define REGISTRYFILE    "temporary dbaccess persistence.rdb"


//==========================================================================
DBG_NAME(ODatabaseContext);
//--------------------------------------------------------------------------

extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext()
{
    static OOneInstanceAutoRegistration< ODatabaseContext > aODatabaseContext_AutoRegistration;
}

//==========================================================================
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

    // get the registry
    implGetRegistry();
    DBG_ASSERT(m_xRegistry.is(), "ODatabaseContext::ODatabaseContext : could not get the registry !");
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

    Reference< XRegistryKey > xObjectBase = getDatabaseObjectKey(_rName, sal_False);
        // will throw a NoSuchElementException if no object with the given name exists

    Reference< XInterface > xNewObject = *(new ODatabaseSource(*this, xObjectBase, _rName, m_xServiceManager));
    m_aDatabaseObjects[_rName] = WeakReferenceHelper(xNewObject);

    return xNewObject;
}

//--------------------------------------------------------------------------
void ODatabaseContext::implGetRegistry()
{
    if (m_xRegistry.is())
        return;

    sal_Bool bSuccess = sal_False;
    try
    {
        m_xRegistry = Reference< XSimpleRegistry > (
                m_xServiceManager->createInstance(rtl::OUString::createFromAscii("com.sun.star.registry.SimpleRegistry")), UNO_QUERY);

        if (m_xRegistry.is())
        {
            // calc the registry file name
            String sRegistryFile;
            String sUserDirFallback;
            Reference< ::com::sun::star::frame::XConfigManager > xSofficeIni(
                m_xServiceManager->createInstance(rtl::OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")), UNO_QUERY);
            Reference< XSimpleRegistry >  xIniRegistry(xSofficeIni, UNO_QUERY);

            try
            {
                if (xIniRegistry.is())
                {
                    Reference< XRegistryKey >  xIniRoot = xIniRegistry->getRootKey();
                    Reference< XRegistryKey >  xDirectories = xIniRoot.is() ? xIniRoot->openKey(::rtl::OUString::createFromAscii("Directories")) : Reference< XRegistryKey > ();
                    if (xDirectories.is())
                    {
                        Reference< XRegistryKey >  xUserConfigKey = xDirectories->openKey(::rtl::OUString::createFromAscii("UserConfig-Path"));
                        if (xUserConfigKey.is())
                        {
                            rtl::OUString sValue = xUserConfigKey->getStringValue();
                            sRegistryFile = (const sal_Unicode*)xSofficeIni->substituteVariables(sValue);
                            sRegistryFile += DirEntry::GetAccessDelimiter();
                            sRegistryFile.AppendAscii(REGISTRYFILE);
                            sUserDirFallback = sRegistryFile;
                        }
                        if (sRegistryFile.Len() && !DirEntry(sRegistryFile).Exists())
                        {
                            sRegistryFile.Erase();  // in case of exceptions ...
                            // check the global config dir
                            Reference< XRegistryKey >  xGlobalConfigKey = xDirectories->openKey(::rtl::OUString::createFromAscii("Config-Dir"));
                            if (xGlobalConfigKey.is())
                            {
                                rtl::OUString sValue = xGlobalConfigKey->getStringValue();
                                sRegistryFile = (const sal_Unicode*)xSofficeIni->substituteVariables(sValue);
                                sRegistryFile += DirEntry::GetAccessDelimiter();
                                sRegistryFile.AppendAscii(REGISTRYFILE);
                            }
                        }
                        if (sRegistryFile.Len() && !DirEntry(sRegistryFile).Exists())
                            sRegistryFile.Erase();
                    }
                }
            }
            catch (InvalidRegistryException)
            {
                sRegistryFile.Erase();
            }


            // have we to create a new registry within the user dir ?
            sal_Bool bCreateIt = sal_False;
            if (!sRegistryFile.Len())
            {
                sRegistryFile = sUserDirFallback;
                bCreateIt = sal_True;
            }

            if (sRegistryFile.Len())
            {
                try
                {
                    m_xRegistry->open(UniString(sRegistryFile), sal_False, bCreateIt);
                    m_xRoot = m_xRegistry->getRootKey();
                    bSuccess = m_xRoot.is();
                }
                catch (InvalidRegistryException)
                {
                }
            }
        }
    }
    catch (Exception&)
    {
        // something went heavyly wrong (e.g. the instantiation of the SpecialConfigManager)
        bSuccess = sal_False;
    }

    if (!bSuccess)
    {
        if (m_xRegistry.is())
        {
            try { m_xRegistry->close(); } catch (InvalidRegistryException) {  };
        }
        m_xRegistry = NULL;
    }
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > ODatabaseContext::getObjectDescriptionKey(const ::rtl::OUString& _rTitle, sal_Bool _bCreate) throw()
{
    DBG_ASSERT(m_xRoot.is(), "ODatabaseContext::getDatabaseObjectKey : don't call without a registry root !");

    Reference< XRegistryKey > xDescriptionKey;
    if (m_xRoot.is())
    {
        try
        {
            ORegistryLevelEnumeration aEnum(m_xRoot);
            ::rtl::OUString sCurrentTitle;
            sal_Bool bFoundSomething = sal_False;
            while (aEnum.hasMoreElements())
            {
                xDescriptionKey = aEnum.nextElement();
                if (xDescriptionKey.is())
                {
                    try
                    {
                        Reference< XRegistryKey > xTitleKey = xDescriptionKey->openKey(CONFIGKEY_CONTAINERLEMENT_TITLE);
                        sCurrentTitle = xTitleKey->getStringValue();
                        if (sCurrentTitle.equals(_rTitle))
                            return xDescriptionKey;
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
            xDescriptionKey.clear();

            // nothing found
            if (_bCreate)
            {
                ::rtl::OUString sNewKeyName = getUniqueKeyName(m_xRoot, ::rtl::OUString::createFromAscii("obj_"));
                Reference< XRegistryKey > xNewKey = m_xRoot->createKey(sNewKeyName);
                if (xNewKey.is())
                {
                    try
                    {
                        // one sub key for the title
                        Reference< XRegistryKey > xTitleKey = xNewKey->createKey(CONFIGKEY_CONTAINERLEMENT_TITLE);
                        xTitleKey->setStringValue(_rTitle);
                        // one subkey for the object information
                        xNewKey->createKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
                        // no exceptions 'til here -> we can return the new key
                        xDescriptionKey = xNewKey;
                    }
                    catch(Exception&)
                    {
                        // we were not successfull in creating a new tree -> delete the incomplete parts
                        try { m_xRoot->deleteKey(sNewKeyName); } catch(Exception&) { }
                    }
                }
            }
        }
        catch (InvalidRegistryException)
        {
        }
        catch (InvalidValueException)
        {
            DBG_ERROR("ODatabaseContext::getObjectDescriptionKey : InvalidValueException ... how this ?");
                // this would mean the key hasn't a string value
        }
    }
    return xDescriptionKey;
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > ODatabaseContext::getDatabaseObjectKey(const ::rtl::OUString& _rTitle, sal_Bool _bCreate) throw (NoSuchElementException)
{
    Reference< XRegistryKey > xDescriptionKey = getObjectDescriptionKey(_rTitle, _bCreate);
    if (!xDescriptionKey.is())
        if (!_bCreate)
            throw NoSuchElementException();
        else
            // the creation of the essential root key failed
            return xDescriptionKey;

    Reference< XRegistryKey > xObjectKey;
    try
    {
        xObjectKey = xDescriptionKey->openKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
        DBG_ASSERT(xObjectKey.is() || !_bCreate, "ODatabaseContext::getDatabaseObjectKey : inconsitent state : there was a name sub key, but no object sub key !");

        if (!xObjectKey.is() && _bCreate)
        {
            xObjectKey = xDescriptionKey->createKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
        }
    }
    catch(Exception&)
    {
    }

    if (!xObjectKey.is())
        throw NoSuchElementException();

    return xObjectKey;
}

//------------------------------------------------------------------------------
void ODatabaseContext::registerObject(const rtl::OUString& _rName, const Reference< XInterface > & _rxObject) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_xRoot.is())
        throw InvalidRegistryException();
    if (DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    // solaris compiler needs a construct like this and can work with comphelper::getImplementation ...
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

    Reference< XRegistryKey > xObjectBase;
    try
    {
        xObjectBase = getDatabaseObjectKey(_rName, sal_False);
        // if we are here there already is an object registered under the given name
        throw ElementExistException();
    }
    catch(NoSuchElementException&)
    {
    }

    sal_Bool bSuccess = sal_False;
    try
    {
        Reference< XRegistryKey > xDescriptionKey = getObjectDescriptionKey(_rName, sal_True);
        if (xDescriptionKey.is())
        {
            xObjectBase = xDescriptionKey->openKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
                // getObjectDescriptionKey is expected to create this sub key when creating the new description key !

            pObjectImpl->inserted(*this, _rName, xObjectBase);
            pObjectImpl->flush();

            bSuccess = sal_True;
        }
    }
    catch(Exception&)
    {
    }

    if (!bSuccess)
        throw InvalidRegistryException();
}

//------------------------------------------------------------------------------
void ODatabaseContext::revokeObject(const rtl::OUString& _rName) throw( Exception, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_xRoot.is())
        throw InvalidRegistryException();
    if (DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XRegistryKey > xObjectKey = getObjectDescriptionKey(_rName, sal_False);
    if (!xObjectKey.is())
        throw NoSuchElementException();

    ObjectCacheIterator aExistent = m_aDatabaseObjects.find(_rName);
    if (aExistent != m_aDatabaseObjects.end())
    {
        Reference< XInterface > xExistent = aExistent->second.get();
        if (xExistent.is())
        {
            Reference< XComponent > xComp(xExistent, UNO_QUERY);
            if (xComp.is())
                xComp->dispose();

            // solaris compiler needs a construct like this and can work with comphelper::getImplementation ...
            Reference< XUnoTunnel > xTunnel(xExistent, UNO_QUERY);
            ODatabaseSource* pObjectImpl = NULL;
            if (xTunnel.is())
                pObjectImpl = reinterpret_cast<ODatabaseSource*> (xTunnel->getSomething(ODatabaseSource::getUnoTunnelImplementationId()));
            DBG_ASSERT(pObjectImpl, "ODatabaseContext::revokeObject : there is an object which is no ODatabaseSource !");
            if (pObjectImpl)
                pObjectImpl->removed();
        }
        m_aDatabaseObjects.erase(aExistent);
    }

    try
    {
        m_xRoot->deleteKey(getShortKeyName(xObjectKey));
    }
    catch(Exception&)
    {
        DBG_ERROR("ODatabaseContext::revokeObject : could not delete the registry key !");
        throw InvalidRegistryException();
    }
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
    MutexGuard aGuard(const_cast<ODatabaseContext*>(this)->m_aMutex);
    if (!m_xRoot.is())
        throw InvalidRegistryException();
    if (DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    ORegistryLevelEnumeration aEnum(m_xRoot);
    Reference< XRegistryKey > xLoop, xTitleKey, xObjectKey;
    while (aEnum.hasMoreElements())
    {
        try
        {
            xLoop = aEnum.nextElement();
            if (xLoop.is())
            {
                xTitleKey = xLoop->openKey(CONFIGKEY_CONTAINERLEMENT_TITLE);
                xObjectKey = xLoop->openKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
                if (xTitleKey.is() && xObjectKey.is())
                    return sal_True;
                else
                    DBG_ERROR("ODatabaseContext::hasElements : me thinks this is inconsistent !");
                    //  we never should have created a key which has no title or object sub key !
            }
        }
        catch (InvalidRegistryException)
        {
        }

    }
    return sal_False;
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
    if (!m_xRoot.is())
        throw InvalidRegistryException();
    if (DatabaseAccessContext_Base::rBHelper.bDisposed)
        throw DisposedException();

    Sequence< ::rtl::OUString > aNames;
    try
    {
        Sequence< Reference< XRegistryKey > > aKeys = m_xRoot->openKeys();
        const Reference< XRegistryKey >* pKeys = aKeys.getConstArray();
        aNames.realloc(aKeys.getLength());
        ::rtl::OUString* pNames = aNames.getArray();

        Reference< XRegistryKey > xTitleKey;

        for (sal_Int32 i=0; i<aKeys.getLength(); ++i, ++pKeys)
        {
            if (pKeys->is())
            {
                try
                {
                    xTitleKey = (*pKeys)->openKey(CONFIGKEY_CONTAINERLEMENT_TITLE);
                    if (xTitleKey.is())
                    {
                        ::rtl::OUString sName = xTitleKey->getStringValue();
                        *pNames = sName;
                        ++pNames;
                    }
                }
                catch(Exception&)
                {
                }
            }
        }

        aNames.realloc(pNames - aNames.getArray());
    }
    catch(InvalidRegistryException&)
    {
    }
    catch(InvalidValueException&)
    {
    }

    return aNames;
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseContext::hasByName(const rtl::OUString& _rName) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    try
    {
        Reference< XRegistryKey > xObjectKey = getDatabaseObjectKey(_rName, sal_False);
        return xObjectKey.is();
    }
    catch(NoSuchElementException&)
    {
    }
    return sal_False;
}

