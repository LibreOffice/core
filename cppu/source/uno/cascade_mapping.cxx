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

#include <osl/interlck.h>
#include <rtl/ustring.hxx>
#include <uno/environment.hxx>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <uno/dispatcher.h>

#include <cppu/EnvDcp.hxx>

#include "cascade_mapping.hxx"

using namespace com::sun::star;

class MediatorMapping : public uno_Mapping
{
    oslInterlockedCount m_refCount;

    uno::Mapping        m_from2uno;
    uno::Mapping        m_uno2to;

    uno::Environment    m_from;
    uno::Environment    m_interm;
    uno::Environment    m_to;

public:
    void acquire();
    void release();

    void mapInterface(void                            ** ppOut,
                      void                             * pInterface,
                      typelib_InterfaceTypeDescription * pInterfaceTypeDescr);
    MediatorMapping(uno_Environment * pFrom,
                    uno_Environment * pInterm,
                    uno_Environment * pTo);
};

extern "C" {
static void s_acquire(uno_Mapping * mapping)
{
    MediatorMapping * pMediatorMapping = static_cast<MediatorMapping *>(mapping);
    pMediatorMapping->acquire();
}

static void s_release(uno_Mapping * mapping)
{
    MediatorMapping * pMediatorMapping = static_cast<MediatorMapping *>(mapping);
    pMediatorMapping->release();
}

static void s_mapInterface(
    uno_Mapping                      * mapping,
    void                            ** ppOut,
    void                             * pInterface,
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr)
{
    MediatorMapping   * pMediatorMapping  = static_cast<MediatorMapping *>(mapping);
    pMediatorMapping->mapInterface(ppOut, pInterface, pInterfaceTypeDescr);
}
}

MediatorMapping::MediatorMapping(uno_Environment * pFrom,
                                 uno_Environment * pInterm,
                                 uno_Environment * pTo)
    : m_refCount(0),
      m_from2uno(pFrom, pInterm),
      m_uno2to  (pInterm, pTo),
      m_from    (pFrom),
      m_interm  (pInterm),
      m_to      (pTo)
{
    if (!m_from2uno.get() || !m_uno2to.get())
        abort();

    uno_Mapping::acquire      = s_acquire;
    uno_Mapping::release      = s_release;
    uno_Mapping::mapInterface = s_mapInterface;
}

void MediatorMapping::acquire()
{
    osl_atomic_increment(&m_refCount);
}

void MediatorMapping::release()
{
    if (osl_atomic_decrement(&m_refCount) == 0)
    {
        ::uno_revokeMapping(this);
    }
}

extern "C" { static void s_mapInterface_v(va_list * pParam)
{
    void                            ** ppOut               = va_arg(*pParam, void **);
    void                             * pInterface          = va_arg(*pParam, void *);
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr = va_arg(*pParam, typelib_InterfaceTypeDescription *);
    uno_Mapping                      * pMapping            = va_arg(*pParam, uno_Mapping *);

    pMapping->mapInterface(pMapping, ppOut, pInterface, pInterfaceTypeDescr);
}}

void MediatorMapping::mapInterface(
    void                            ** ppOut,
    void                             * pInterface,
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr)
{
    if (*ppOut != nullptr)
    {
        uno_ExtEnvironment * env = m_to.get()->pExtEnv;
        OSL_ASSERT( env != nullptr );
        env->releaseInterface( env, *ppOut );
        *ppOut = nullptr;
    }

    void * ret = nullptr;
    uno_Interface * pUnoI = nullptr;

    m_from.invoke(s_mapInterface_v, &pUnoI, pInterface, pInterfaceTypeDescr, m_from2uno.get());

    m_uno2to.mapInterface(&ret, pUnoI, pInterfaceTypeDescr);

    if (pUnoI)
        m_interm.get()->pExtEnv->releaseInterface(m_interm.get()->pExtEnv, pUnoI);

    *ppOut = ret;
}

extern "C" { static void s_MediatorMapping_free(uno_Mapping * pMapping)
    SAL_THROW_EXTERN_C()
{
    delete static_cast<MediatorMapping *>(pMapping);
}}


static OUString getPrefix(OUString const & str1, OUString const & str2)
{
    sal_Int32 nIndex1 = 0;
    sal_Int32 nIndex2 = 0;
    sal_Int32 sim = 0;

    OUString token1;
    OUString token2;

    do
    {
        token1 = str1.getToken(0, ':', nIndex1);
        token2 = str2.getToken(0, ':', nIndex2);

        if (token1 == token2)
            sim += token1.getLength() + 1;
    }
    while(nIndex1 == nIndex2 && nIndex1 >= 0 && token1 == token2);

    OUString result;

    if (sim)
        result = str1.copy(0, sim - 1);

    return result;
}

//  OUString str1("abc:def:ghi");
//  OUString str2("abc:def");
//  OUString str3("abc");
//  OUString str4("");

//  OUString pref;

//  pref = getPrefix(str1, str1);
//  pref = getPrefix(str1, str2);
//  pref = getPrefix(str1, str3);
//  pref = getPrefix(str1, str4);

//  pref = getPrefix(str2, str1);
//  pref = getPrefix(str3, str1);
//  pref = getPrefix(str4, str1);


void getCascadeMapping(uno_Mapping     ** ppMapping,
                       uno_Environment  * pFrom,
                       uno_Environment  * pTo,
                       rtl_uString      * pAddPurpose)
{
    if (pAddPurpose && pAddPurpose->length)
        return;

    OUString uno_envType(UNO_LB_UNO);

    OUString from_envType    = cppu::EnvDcp::getTypeName(pFrom->pTypeName);
    OUString to_envType      = cppu::EnvDcp::getTypeName(pTo->pTypeName);
    OUString from_envPurpose = cppu::EnvDcp::getPurpose(pFrom->pTypeName);
    OUString to_envPurpose   = cppu::EnvDcp::getPurpose(pTo->pTypeName);

#ifdef LOG_CALLING_named_purpose_getMapping
    OString s_from_name = OUStringToOString(pFrom->pTypeName, RTL_TEXTENCODING_ASCII_US);
    OString s_to_name   = OUStringToOString(pTo->pTypeName,   RTL_TEXTENCODING_ASCII_US);

    std::cerr << __FUNCTION__ << " - creating mediation ";
    std::cerr << "pFrom: " << s_from_name.getStr();
    std::cerr <<" pTo: "   << s_to_name.getStr() << std::endl;
#endif

    if (from_envPurpose == to_envPurpose) // gcc:bla => uno:bla
        return;

    // reaching this point means, we need a mediated mapping!!!
    // we generally mediate via uno[:free]
    uno_Environment * pInterm = nullptr;

    // chained uno -> uno
    if (from_envType == uno_envType && to_envType == uno_envType)
    {
        OUString purpose = getPrefix(from_envPurpose, to_envPurpose);

        OUString uno_envDcp = uno_envType;
        uno_envDcp += purpose;

        // direct mapping possible?
        // uno:bla-->uno:bla:blubb
        if (from_envPurpose == purpose)
        {
            OUString rest = to_envPurpose.copy(purpose.getLength());

            sal_Int32 index = rest.indexOf(':', 1);
            if (index == -1)
            {
                uno_getMapping(ppMapping, pFrom, pTo, rest.copy(1).pData);
                return;
            }

            uno_envDcp += rest.copy(0, index);
        }
        else if (to_envPurpose == purpose)
        {
            OUString rest = from_envPurpose.copy(purpose.getLength());

            sal_Int32 index = rest.indexOf(':', 1);
            if (index == -1)
            {
                uno_getMapping(ppMapping, pFrom, pTo, rest.copy(1).pData);
                return;
            }

            uno_envDcp += rest.copy(0, index);
        }

        uno_getEnvironment(&pInterm, uno_envDcp.pData, nullptr);
    }
    else if (from_envType != uno_envType && to_envType == uno_envType) // <ANY> -> UNO ?
        // mediate via uno:purpose(fromEnv)
    {
        OUString     envDcp = uno_envType;

        envDcp += from_envPurpose;
        uno_getEnvironment(&pInterm, envDcp.pData, nullptr);
    }
    else if (from_envType == uno_envType && to_envType != uno_envType) // UNO -> <ANY>?
        // mediate via uno(context)
    {
        OUString     envDcp = uno_envType;

        envDcp += to_envPurpose;
        uno_getEnvironment(&pInterm, envDcp.pData, nullptr);
    }
    else // everything else
        // mediate via uno:purpose
    {
        OUString purpose = getPrefix(from_envPurpose, to_envPurpose);

        OUString uno_envDcp = uno_envType;
        uno_envDcp += purpose;

        uno_getEnvironment(&pInterm, uno_envDcp.pData, nullptr);
    }

    uno_Mapping * pMapping = new MediatorMapping(pFrom, pInterm, pTo);
    pInterm->release(pInterm);


    pMapping->acquire(pMapping);

    ::uno_registerMapping(&pMapping, s_MediatorMapping_free, pFrom, pTo, pAddPurpose);

    if (*ppMapping)
        (*ppMapping)->release(*ppMapping);

    *ppMapping = pMapping;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
