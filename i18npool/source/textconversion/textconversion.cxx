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

#include <assert.h>
#include <cppuhelper/supportsservice.hxx>
#include <textconversion.hxx>

using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#endif

TextConversionService::TextConversionService(const char *pImplName)
    : implementationName(pImplName)
{
#ifndef DISABLE_DYNLOADING
#ifdef SAL_DLLPREFIX
    OUString lib(SAL_DLLPREFIX"textconv_dict" SAL_DLLEXTENSION);
#else
    OUString lib("textconv_dict" SAL_DLLEXTENSION);
#endif
    hModule = osl_loadModuleRelative(
        &thisModule, lib.pData, SAL_LOADMODULE_DEFAULT );
#endif
}

TextConversionService::~TextConversionService()
{
#ifndef DISABLE_DYNLOADING
    if (hModule) osl_unloadModule(hModule);
#endif
}

#ifndef DISABLE_DYNLOADING

static void* nullFunc()
{
    return nullptr;
}

oslGenericFunction SAL_CALL
TextConversionService::getFunctionBySymbol(const sal_Char* func)
{
    if (hModule)
        return osl_getFunctionSymbol(hModule, OUString::createFromAscii(func).pData);
    else
        return reinterpret_cast< oslGenericFunction >(nullFunc);
}

#endif

OUString SAL_CALL
TextConversionService::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString::createFromAscii(implementationName);
}

sal_Bool SAL_CALL
TextConversionService::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
TextConversionService::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(implementationName);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
