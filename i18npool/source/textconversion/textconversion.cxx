/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <assert.h>
#include <textconversion.hxx>

using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

extern "C" { static void SAL_CALL thisModule() {} }

TextConversion::TextConversion()
{
#ifdef SAL_DLLPREFIX
    OUString lib(RTL_CONSTASCII_USTRINGPARAM(SAL_DLLPREFIX"textconv_dict"SAL_DLLEXTENSION));
#else
    OUString lib(RTL_CONSTASCII_USTRINGPARAM("textconv_dict"SAL_DLLEXTENSION));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
