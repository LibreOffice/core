/*************************************************************************
 *
 *  $RCSfile: configunoreg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-10 22:45:38 $
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

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#include "confsvccomponent.hxx"
#endif
#ifndef CONFIGMGR_API_PROVIDER2_HXX_
#include "confprovider2.hxx"
#endif
#ifndef CONFIGMGR_API_ADMINPROVIDER_HXX_
#include "adminprovider.hxx"
#endif


#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


/********************************************************************************************/

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;
using configmgr::ServiceInfo;
using configmgr::AsciiServiceName;

#define THISREF() static_cast< ::cppu::OWeakObject* >(this)


typedef Reference< XSingleServiceFactory > (SAL_CALL * createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames
        );

// ***************************************************************************************

namespace configmgr
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::cppu;
    using namespace ::osl;

//=======================================================================================
//= OProviderFactory
//=======================================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XSingleServiceFactory > OProviderFactory_Base;
    /** a special factory for the configuration provider, which implements some kind of
        "shared multiple instances" factory.
    */
    class OProviderFactory : public OProviderFactory_Base
    {
    protected:
        ::osl::Mutex                        m_aMutex;
        ::cppu::ComponentInstantiation      m_pObjectCreator;
        Reference< XMultiServiceFactory >   m_xORB;
        Reference< XInterface >             m_xDefaultProvider;

        typedef ::com::sun::star::uno::WeakReference< XInterface >  ProviderReference;
        DECLARE_STL_USTRINGACCESS_MAP(ProviderReference, UserSpecificProvider);
        UserSpecificProvider    m_aUserProvider;

    public:
        OProviderFactory(
            const Reference< XMultiServiceFactory >& _rxORB,
            ::cppu::ComponentInstantiation _pObjectCreator);

        virtual Reference< XInterface > SAL_CALL createInstance(  ) throw(Exception, RuntimeException);
        virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

        Reference< XInterface > createProvider();
        Reference< XInterface > createProviderWithArguments(const Sequence< Any >& _rArguments);

    protected:
        void    ensureDefaultProvider();
    };

    //=======================================================================================
    //= OProviderFactory
    //=======================================================================================
    //---------------------------------------------------------------------------------------
    OProviderFactory::OProviderFactory(const Reference< XMultiServiceFactory >& _rxORB, ::cppu::ComponentInstantiation _pObjectCreator)
        :m_pObjectCreator(_pObjectCreator)
        ,m_xORB(_rxORB)
    {
    }

    //---------------------------------------------------------------------------------------
    void OProviderFactory::ensureDefaultProvider()
    {
        MutexGuard aGuard(m_aMutex);
        if (m_xDefaultProvider.is())
            return;
        m_xDefaultProvider = (*m_pObjectCreator)(m_xORB);

        // initialize it with an empty sequence, thus we ensure the provider establishes it's connection
        Reference< XInitialization > xProvInit(m_xDefaultProvider, UNO_QUERY);
        if (xProvInit.is())
            xProvInit->initialize(Sequence< Any >());

        // TODO : we need direct access to the provider implementation. This way we won't need the XInitialization
        // interface, and we don't need this weird behaviour of initializing with an empty argument sequence
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProvider()
    {
        MutexGuard aGuard(m_aMutex);
        ensureDefaultProvider();
        return m_xDefaultProvider;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > OProviderFactory::createProviderWithArguments(const Sequence< Any >& _rArguments)
    {
        MutexGuard aGuard(m_aMutex);
        ::rtl::OUString sUser;
        // #78409
        // if a provider is queried with a password, we always create a new instance for him,
        // as don't wan't to the passwords for the user.
        ::rtl::OUString sPassword;
        bool bPasswordUsed = false;

        const Any* pArguments = _rArguments.getConstArray();
        PropertyValue aCurrentArg;
        for (sal_Int32 i=0; i<_rArguments.getLength(); ++i, ++pArguments)
        {
            if (!((*pArguments) >>= aCurrentArg))
                throw IllegalArgumentException(::rtl::OUString::createFromAscii("Arguments have to be com.sun.star.beans.PropertyValue's."), NULL, i);
            if (0 == aCurrentArg.Name.compareToAscii("user"))
            {
                if (!(aCurrentArg.Value >>= sUser) || !sUser.getLength())
                    throw IllegalArgumentException(::rtl::OUString::createFromAscii("The user name specified is invalid."), NULL, i);
            }

            // sesions which query for a password are always one instance
            if (0 == aCurrentArg.Name.compareToAscii("password"))
            {
                bPasswordUsed = true;
                if (!(aCurrentArg.Value >>= sPassword))
                    throw IllegalArgumentException(::rtl::OUString::createFromAscii("The password specified is invalid."), NULL, i);
            }
        }

        Reference< XInterface > xThisUsersProvider;
        if (!bPasswordUsed)
        {
            UserSpecificProviderIterator aExistentProvider = m_aUserProvider.find(sUser);
            if (m_aUserProvider.end() != aExistentProvider)
                xThisUsersProvider = aExistentProvider->second;
        }

        if (!xThisUsersProvider.is())
        {
            xThisUsersProvider = (*m_pObjectCreator)(m_xORB);
            if (!bPasswordUsed)
                m_aUserProvider[sUser] = xThisUsersProvider;

            // initialize it
            Reference< XInitialization > xProvInit(xThisUsersProvider, UNO_QUERY);
            if (xProvInit.is())
                xProvInit->initialize(_rArguments);
        }

        return xThisUsersProvider;
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OProviderFactory::createInstance(  ) throw(Exception, RuntimeException)
    {
        // default provider
        return createProvider();
    }

    //---------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OProviderFactory::createInstanceWithArguments( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException)
    {
        return createProviderWithArguments(_rArguments);
    }

    //=======================================================================================
    Reference< XSingleServiceFactory > SAL_CALL createProviderFactory(
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames
        )
    {
        OSL_ENSHURE(0 == rComponentName.compareToAscii(OConfigurationProvider::staticServiceInfo.implementationName),
            "configmgr::createProviderFactory : invalid argument !");
        return new OProviderFactory(rServiceManager, pCreateFunction);
    }

}   // namespace configmgr

// ***************************************************************************************
//
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//

//---------------------------------------------------------------------------------------
void REGISTER_PROVIDER(
        const OUString& aServiceImplName,
        const Sequence<OUString>& Services,
        const Reference< XRegistryKey > & xKey)
{
    OUString aMainKeyName(OUString(RTL_CONSTASCII_USTRINGPARAM("/")));
    aMainKeyName += aServiceImplName;
    aMainKeyName += OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

    Reference< XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName) );
    OSL_ENSHURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    for (sal_uInt32 i=0; i<Services.getLength(); ++i)
        xNewKey->createKey(Services[i]);
}


//---------------------------------------------------------------------------------------
void RegisterService(
        const ServiceInfo* pInfo,
        const Reference< XRegistryKey > & xKey)
{
    if (pInfo == 0 || pInfo->serviceNames==0 || pInfo->implementationName==0)
        return;

    OUString aMainKeyName(OUString(RTL_CONSTASCII_USTRINGPARAM("/")));
    aMainKeyName += OUString::createFromAscii(pInfo->implementationName);
    aMainKeyName += OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

    Reference< XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName) );
    OSL_ENSHURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    AsciiServiceName const* p = pInfo->serviceNames;
    if (p != 0)
        for( ; *p; ++p)
        {
            xNewKey->createKey(OUString::createFromAscii(*p));
        }
}

//---------------------------------------------------------------------------------------
struct ProviderRequest
{
    Reference< XSingleServiceFactory > xRet;
    Reference< XMultiServiceFactory > const xServiceManager;
    OUString const sImplementationName;

    ProviderRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    )
    : xServiceManager(reinterpret_cast<XMultiServiceFactory*>(pServiceManager))
    , sImplementationName(OUString::createFromAscii(pImplementationName))
    {
    }

    inline
    sal_Bool CREATE_PROVIDER(
                const OUString& Implname,
                const Sequence< OUString > & Services,
                ::cppu::ComponentInstantiation Factory,
                createFactoryFunc creator
            )
    {
        if (!xRet.is() && (Implname == sImplementationName))
        try
        {
            OSL_ENSHURE(!xRet.is(), "CREATE_PROVIDER : invalid : already have a return value !");
            if (xRet.is())
                xRet->release();

            xRet = creator( xServiceManager, sImplementationName,Factory, Services);
            OSL_ENSHURE(xRet.is(), "CREATE_PROVIDER : invalid return value !");

            if (xRet.is())
                xRet->acquire();
                // we want to transport the interface pointer as flat C void pointer, so this prevents deletion
        }
        catch(...)
        {
        }
        return xRet.is();
    }

    inline
    sal_Bool CreateProvider(
                const ServiceInfo* pInfo,
                ::cppu::ComponentInstantiation Factory,
                createFactoryFunc creator
            )
    {
        OSL_ENSHURE(!xRet.is(), "CreateProvider : invalid : we already have a return value !");
        if (xRet.is())
            return true; //xRet->release();

        if (!xRet.is() && pInfo!=0 && (0 == sImplementationName.compareToAscii(pInfo->implementationName)))
        try
        {
            const Sequence< OUString > Services=  configmgr::ServiceComponentImpl::getServiceNames(pInfo);

            xRet = creator( xServiceManager, OUString::createFromAscii(pInfo->implementationName),Factory, Services);
            OSL_ENSHURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");

            if (xRet.is())
                xRet->acquire();
                // we want to transport the interface pointer as flat C void pointer, so this prevents deletion
        }
        catch(...)
        {
        }
        return xRet.is();
    }

    void* getProvider() const { return xRet.get(); }
};

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **ppEnv
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" sal_Bool SAL_CALL component_writeInfo(
                void* pServiceManager,
                void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        Reference< XRegistryKey > xKey(reinterpret_cast<XRegistryKey*>(pRegistryKey));

        RegisterService(&configmgr::OConfigurationProvider::staticServiceInfo, xKey);
        RegisterService(&configmgr::OAdminProvider::staticServiceInfo, xKey);

        RegisterService(configmgr::getConfigurationRegistryServiceInfo(), xKey);

        // im/export
        RegisterService(configmgr::getDataExportServiceInfo(), xKey);
        RegisterService(configmgr::getDataImportServiceInfo(), xKey);

        return sal_True;
    }
    catch (::com::sun::star::registry::InvalidRegistryException& )
    {
        OSL_ENSHURE(sal_False, "SBA::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    void* pRet = 0;
    if (pServiceManager)
    {
        ProviderRequest aReq(pServiceManager,pImplementationName);

        aReq.CreateProvider(
            &configmgr::OConfigurationProvider::staticServiceInfo,
            &configmgr::instantiateConfigProvider,
            ::configmgr::createProviderFactory)
        ||
        aReq.CreateProvider(
            &configmgr::OAdminProvider::staticServiceInfo,
            &configmgr::instantiateAdminProvider,
            ::cppu::createSingleFactory)
        ||
        aReq.CreateProvider(
            configmgr::getConfigurationRegistryServiceInfo(),
            &configmgr::instantiateConfigRegistry,
            ::cppu::createSingleFactory)
        ||
            /* Export */
        aReq.CreateProvider(
            configmgr::getDataExportServiceInfo(),
            &configmgr::instantiateDataExport,
            ::cppu::createSingleFactory)
        ||
            /* Import */
        aReq.CreateProvider(
            configmgr::getDataImportServiceInfo(),
            &configmgr::instantiateDataImport,
            ::cppu::createSingleFactory)
        ||
        false;

        pRet = aReq.getProvider();
    }

    return pRet;
};
//---------------------------------------------------------------------------------------

