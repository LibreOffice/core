/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KServices.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:39:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "KDriver.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace connectivity::kab;
using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;

typedef Reference< XSingleServiceFactory > (SAL_CALL *createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames,
            rtl_ModuleCount* _pTemp
        );

//***************************************************************************************
//
// The following C Api must be provided!
// It consists in three functions that must be exported by the module
//

//---------------------------------------------------------------------------------------
void REGISTER_PROVIDER(
        const OUString& aServiceImplName,
        const Sequence< OUString>& Services,
        const Reference< ::com::sun::star::registry::XRegistryKey > & xKey)
{
    OUString aMainKeyName;
    aMainKeyName = OUString::createFromAscii("/");
    aMainKeyName += aServiceImplName;
    aMainKeyName += OUString::createFromAscii("/UNO/SERVICES");

    Reference< ::com::sun::star::registry::XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName) );
    OSL_ENSURE(xNewKey.is(), "KAB::component_writeInfo : could not create a registry key !");

    for (sal_Int32 i=0; i<Services.getLength(); ++i)
        xNewKey->createKey(Services[i]);
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
            xRet = creator( xServiceManager, sImplementationName,Factory, Services,0);
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
                uno_Environment **
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" sal_Bool SAL_CALL component_writeInfo(
                void*,
                void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        Reference< ::com::sun::star::registry::XRegistryKey > xKey(reinterpret_cast< ::com::sun::star::registry::XRegistryKey*>(pRegistryKey));

        REGISTER_PROVIDER(
            KabDriver::getImplementationName_Static(),
            KabDriver::getSupportedServiceNames_Static(), xKey);

        return sal_True;
    }
    catch (::com::sun::star::registry::InvalidRegistryException& )
    {
        OSL_ENSURE(sal_False, "KAB::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void*)
{
    void* pRet = 0;
    if (pServiceManager)
    {
        ProviderRequest aReq(pServiceManager,pImplementationName);

        aReq.CREATE_PROVIDER(
            KabDriver::getImplementationName_Static(),
            KabDriver::getSupportedServiceNames_Static(),
            &KabDriver::Create,
            ::cppu::createSingleFactory)
        ;

        if (aReq.xRet.is())
            aReq.xRet->acquire();

        pRet = aReq.getProvider();
    }

    return pRet;
};


