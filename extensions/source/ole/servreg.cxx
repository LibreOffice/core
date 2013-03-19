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

// http://stackoverflow.com/questions/5839292/error-c1189-after-installing-visual-studio-2010
#define _WIN32_WINNT 0x0403


#include <rtl/unload.h>
#include <osl/time.h>
#include "ole2uno.hxx"
#include "servprov.hxx"
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
using namespace ole_adapter;
using namespace cppu;

using ::rtl::OUString;

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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL oleautobridge_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    OUString aImplName( OUString::createFromAscii( pImplName ) );
    Reference< XSingleServiceFactory > xFactory;
    Sequence<OUString> seqServiceNames;
    if (pServiceManager && aImplName.equals(  reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleConverter2")  ))
    {
        xFactory=  createSingleFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                         OUString("com.sun.star.comp.ole.OleConverter2"),
                                         ConverterProvider_CreateInstance2, seqServiceNames,
                                         &globalModuleCount.modCnt );
    }
    else if (pServiceManager && aImplName.equals(  reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleConverterVar1")  ))
    {
        xFactory= createSingleFactory( reinterpret_cast<XMultiServiceFactory*>(pServiceManager),
                                       OUString("com.sun.star.comp.ole.OleConverterVar1"),
                                       ConverterProvider_CreateInstanceVar1, seqServiceNames,
                                       &globalModuleCount.modCnt );
    }
    else if(pServiceManager && aImplName.equals(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleClient")))
    {
        xFactory= createSingleFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                       OUString("com.sun.star.comp.ole.OleClient"),
                                       OleClient_CreateInstance, seqServiceNames,
                                       &globalModuleCount.modCnt);
    }
    else if(pServiceManager && aImplName.equals(reinterpret_cast<const sal_Unicode*>(L"com.sun.star.comp.ole.OleServer")))
    {
        xFactory= createOneInstanceFactory( reinterpret_cast< XMultiServiceFactory*>(pServiceManager),
                                            OUString("com.sun.star.comp.ole.OleServer"),
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

extern "C"  SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
oleautobridge_component_canUnload( TimeValue* libUnused)
{
    return globalModuleCount.canUnload( &globalModuleCount, libUnused);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
