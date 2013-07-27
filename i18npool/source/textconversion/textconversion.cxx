/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <assert.h>
#include <textconversion.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

extern "C" { static void SAL_CALL thisModule() {} }

TextConversion::TextConversion()
{
#ifdef SAL_DLLPREFIX
    OUString lib=OUString::createFromAscii( SAL_DLLPREFIX "textconv_dict" SAL_DLLEXTENSION);
#else
    OUString lib=OUString::createFromAscii( "textconv_dict" SAL_DLLEXTENSION);
#endif
    hModule = osl_loadModuleRelative(
        &thisModule, lib.pData, SAL_LOADMODULE_DEFAULT );
}

TextConversion::~TextConversion()
{
    if (hModule) osl_unloadModule(hModule);
}

static void* nullFunc()
{
    return NULL;
}

oslGenericFunction SAL_CALL
TextConversion::getFunctionBySymbol(const sal_Char* func)
{
    if (hModule)
        return osl_getFunctionSymbol(hModule, OUString::createFromAscii(func).pData);
    else
        return reinterpret_cast< oslGenericFunction >(nullFunc);
}

OUString SAL_CALL
TextConversion::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(implementationName);
}

sal_Bool SAL_CALL
TextConversion::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.equalsAscii(implementationName);
}

Sequence< OUString > SAL_CALL
TextConversion::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(implementationName);
    return aRet;
}

} } } }
