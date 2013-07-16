/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <sal/types.h>
#include "FDriver.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

using namespace connectivity::firebird;
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
// The required C-Api must be provided!
// It contains of 3 special functions that have to be exported.
//

//---------------------------------------------------------------------------------------
void REGISTER_PROVIDER(
        const OUString& aServiceImplName,
        const Sequence< OUString>& Services,
        const Reference< ::com::sun::star::registry::XRegistryKey > & xKey)
{
    OUString aMainKeyName("/");
    aMainKeyName += aServiceImplName;
    aMainKeyName += OUString("/UNO/SERVICES");

    Reference< ::com::sun::star::registry::XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName) );
    OSL_ENSURE(xNewKey.is(), "FIREBIRD::component_writeInfo : could not create a registry key !");

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
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL firebird_sdbc_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    (void) pRegistryKey;
    void* pRet = 0;
    if (pServiceManager)
    {
        ProviderRequest aReq(pServiceManager,pImplementationName);

        aReq.CREATE_PROVIDER(
            FirebirdDriver::getImplementationName_Static(),
            FirebirdDriver::getSupportedServiceNames_Static(),
            FirebirdDriver_CreateInstance, ::cppu::createSingleFactory)
        ;

        if(aReq.xRet.is())
            aReq.xRet->acquire();

        pRet = aReq.getProvider();
    }

    return pRet;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
