/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servreg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:48:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <rtl/unload.h>
#include <osl/time.h>
#include "ole2uno.hxx"
#include "servprov.hxx"
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
using namespace rtl;
using namespace ole_adapter;
using namespace cppu;

namespace ole_adapter
{
rtl_StandardModuleCount globalModuleCount= MODULE_COUNT_INIT;



Reference<XInterface> SAL_CALL ConverterProvider_CreateInstance2(   const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleConverter_Impl2( xSMgr);
    return xService;
}

Reference<XInterface> SAL_CALL ConverterProvider_CreateInstanceVar1(    const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleConverter_Impl2( xSMgr, UNO_OBJECT_WRAPPER_REMOTE_OPT, IUNKNOWN_WRAPPER_IMPL);
    return xService;
}

Reference<XInterface> SAL_CALL OleClient_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw(Exception)
{
    Reference<XInterface> xService = *new OleClient_Impl( xSMgr);
    return xService;
}

Reference<XInterface> SAL_CALL OleServer_CreateInstance( const Reference<XMultiServiceFactory> & xSMgr)
                            throw (Exception)
{
    Reference<XInterface > xService = *new OleServer_Impl(xSMgr);
    return xService;
}
} // end namespace

extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    OUString aImplName( OUString::createFromAscii( pImplName ) );
    Reference< XSingleServiceFactory > xFactory;
    Sequence<OUString> seqServiceNames;
    if (pServiceManager && aImplName.equals(  reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleConverter2")  ))
    {
        xFactory=  createSingleFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ole.OleConverter2")),
                                         ConverterProvider_CreateInstance2, seqServiceNames,
                                         &globalModuleCount.modCnt );
    }
    else if (pServiceManager && aImplName.equals(  reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleConverterVar1")  ))
    {
        xFactory= createSingleFactory( reinterpret_cast<XMultiServiceFactory*>(pServiceManager),
                                       OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ole.OleConverterVar1")),
                                       ConverterProvider_CreateInstanceVar1, seqServiceNames,
                                       &globalModuleCount.modCnt );
    }
    else if(pServiceManager && aImplName.equals(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleClient")))
    {
        xFactory= createSingleFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                       OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ole.OleClient")),
                                       OleClient_CreateInstance, seqServiceNames,
                                       &globalModuleCount.modCnt);
    }
    else if(pServiceManager && aImplName.equals(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleServer")))
    {
        xFactory= createOneInstanceFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ole.OleServer")),
                                            OleServer_CreateInstance, seqServiceNames,
                                            &globalModuleCount.modCnt);
    }

    if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}


extern "C" sal_Bool SAL_CALL component_writeInfo(   void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            //deprecated
            Reference<XRegistryKey> xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(reinterpret_cast<const sal_Unicode*>(L"/com.sun.star.comp.ole.OleConverter2/UNO/SERVICES"));
            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.OleBridgeSupplier2"));

            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.BridgeSupplier"));

            //deprecated
            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(reinterpret_cast<const sal_Unicode*>(L"/com.sun.star.comp.ole.OleConverterVar1/UNO/SERVICES"));
            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.OleBridgeSupplierVar1"));

            //deprecated
            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(reinterpret_cast<const sal_Unicode*>(L"/com.sun.star.comp.ole.OleClient/UNO/SERVICES"));
            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.OleObjectFactory"));

            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.Factory"));
            //deprecated
            xNewKey =
                reinterpret_cast<XRegistryKey*>( pRegistryKey)->createKey(reinterpret_cast<const sal_Unicode*>(L"/com.sun.star.comp.ole.OleServer/UNO/SERVICES"));
            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.OleApplicationRegistration"));

            xNewKey->createKey(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.bridge.oleautomation.ApplicationRegistration"));

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


extern "C"  sal_Bool component_canUnload( TimeValue* libUnused)
{
    return globalModuleCount.canUnload( &globalModuleCount, libUnused);
}
