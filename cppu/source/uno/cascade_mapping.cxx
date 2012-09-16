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

#include "osl/interlck.h"
#include "rtl/ustring.hxx"
#include "uno/environment.hxx"
#include "uno/mapping.hxx"
#include "uno/dispatcher.h"

#include "cppu/EnvDcp.hxx"


#ifdef LOG_LIFECYLE_MediatorMapping
#  include <iostream>
#  define LOG_LIFECYLE_MediatorMapping_emit(x) x

#else
#  define LOG_LIFECYLE_MediatorMapping_emit(x)

#endif


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
    void acquire(void);
    void release(void);

    void mapInterface(void                            ** ppOut,
                      void                             * pInterface,
                      typelib_InterfaceTypeDescription * pInterfaceTypeDescr);
    MediatorMapping(uno_Environment * pFrom,
                    uno_Environment * pInterm,
                    uno_Environment * pTo);
    ~MediatorMapping();
};

extern "C" {
static void SAL_CALL s_acquire(uno_Mapping * mapping)
{
    MediatorMapping * pMediatorMapping = static_cast<MediatorMapping *>(mapping);
    pMediatorMapping->acquire();
}

static void SAL_CALL s_release(uno_Mapping * mapping)
{
    MediatorMapping * pMediatorMapping = static_cast<MediatorMapping *>(mapping);
    pMediatorMapping->release();
}

static void SAL_CALL s_mapInterface(
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
    LOG_LIFECYLE_MediatorMapping_emit(std::cerr << __FUNCTION__ << std::endl);

    if (!m_from2uno.get() || !m_uno2to.get())
        abort();

    uno_Mapping::acquire      = s_acquire;
    uno_Mapping::release      = s_release;
    uno_Mapping::mapInterface = s_mapInterface;
}

MediatorMapping::~MediatorMapping()
{
    LOG_LIFECYLE_MediatorMapping_emit(std::cerr << __FUNCTION__ << std::endl);
}

void MediatorMapping::acquire(void)
{
    LOG_LIFECYLE_MediatorMapping_emit(std::cerr << __FUNCTION__ << std::endl);

    osl_atomic_increment(&m_refCount);
}

void MediatorMapping::release(void)
{
    LOG_LIFECYLE_MediatorMapping_emit(std::cerr << __FUNCTION__ << std::endl);

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
    if (*ppOut != 0)
    {
        uno_ExtEnvironment * env = m_to.get()->pExtEnv;
        OSL_ASSERT( env != 0 );
        env->releaseInterface( env, *ppOut );
        *ppOut = NULL;
    }

    void * ret = 0;
    uno_Interface * pUnoI = 0;

    m_from.invoke(s_mapInterface_v, &pUnoI, pInterface, pInterfaceTypeDescr, m_from2uno.get());

    m_uno2to.mapInterface(&ret, pUnoI, pInterfaceTypeDescr);

    if (pUnoI)
        m_interm.get()->pExtEnv->releaseInterface(m_interm.get()->pExtEnv, pUnoI);

    *ppOut = ret;
}

extern "C" { static void SAL_CALL s_MediatorMapping_free(uno_Mapping * pMapping)
    SAL_THROW_EXTERN_C()
{
    delete static_cast<MediatorMapping *>(pMapping);
}}



static rtl::OUString getPrefix(rtl::OUString const & str1, rtl::OUString const & str2)
{
    sal_Int32 nIndex1 = 0;
    sal_Int32 nIndex2 = 0;
    sal_Int32 sim = 0;

    rtl::OUString token1;
    rtl::OUString token2;

    do
    {
        token1 = str1.getToken(0, ':', nIndex1);
        token2 = str2.getToken(0, ':', nIndex2);

        if (token1.equals(token2))
            sim += token1.getLength() + 1;
    }
    while(nIndex1 == nIndex2 && nIndex1 >= 0 && token1.equals(token2));

    rtl::OUString result;

    if (sim)
        result = str1.copy(0, sim - 1);

    return result;
}

//  rtl::OUString str1(RTL_CONSTASCII_USTRINGPARAM("abc:def:ghi"));
//  rtl::OUString str2(RTL_CONSTASCII_USTRINGPARAM("abc:def"));
//  rtl::OUString str3(RTL_CONSTASCII_USTRINGPARAM("abc"));
//  rtl::OUString str4(RTL_CONSTASCII_USTRINGPARAM(""));

//  rtl::OUString pref;

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

    rtl::OUString uno_envType(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));

    rtl::OUString from_envType    = cppu::EnvDcp::getTypeName(pFrom->pTypeName);
    rtl::OUString to_envType      = cppu::EnvDcp::getTypeName(pTo->pTypeName);
    rtl::OUString from_envPurpose = cppu::EnvDcp::getPurpose(pFrom->pTypeName);
    rtl::OUString to_envPurpose   = cppu::EnvDcp::getPurpose(pTo->pTypeName);

#ifdef LOG_CALLING_named_purpose_getMapping
    rtl::OString s_from_name = rtl::OUStringToOString(pFrom->pTypeName, RTL_TEXTENCODING_ASCII_US);
    rtl::OString s_to_name   = rtl::OUStringToOString(pTo->pTypeName,   RTL_TEXTENCODING_ASCII_US);

    std::cerr << __FUNCTION__ << " - creating mediation ";
    std::cerr << "pFrom: " << s_from_name.getStr();
    std::cerr <<" pTo: "   << s_to_name.getStr() << std::endl;
#endif

    if (from_envPurpose == to_envPurpose) // gcc:bla => uno:bla
        return;

    // reaching this point means, we need a mediated mapping!!!
    // we generall mediate via uno[:free]
    uno_Environment * pInterm = NULL;

    // chained uno -> uno
    if (from_envType == uno_envType && to_envType == uno_envType)
    {
        rtl::OUString purpose = getPrefix(from_envPurpose, to_envPurpose);

        rtl::OUString uno_envDcp = uno_envType;
        uno_envDcp += purpose;

        // direct mapping possible?
        // uno:bla-->uno:bla:blubb
        if (from_envPurpose.equals(purpose))
        {
            rtl::OUString rest = to_envPurpose.copy(purpose.getLength());

            sal_Int32 index = rest.indexOf(':', 1);
            if (index == -1)
            {
                uno_getMapping(ppMapping, pFrom, pTo, rest.copy(1).pData);
                return;
            }

            uno_envDcp += rest.copy(0, index);
        }
        else if (to_envPurpose.equals(purpose))
        {
            rtl::OUString rest = from_envPurpose.copy(purpose.getLength());

            sal_Int32 index = rest.indexOf(':', 1);
            if (index == -1)
            {
                uno_getMapping(ppMapping, pFrom, pTo, rest.copy(1).pData);
                return;
            }

            uno_envDcp += rest.copy(0, index);
        }

        uno_getEnvironment(&pInterm, uno_envDcp.pData, NULL);
    }
    else if (from_envType != uno_envType && to_envType == uno_envType) // <ANY> -> UNO ?
        // mediate via uno:purpose(fromEnv)
    {
        rtl::OUString     envDcp = uno_envType;

        envDcp += from_envPurpose;
         uno_getEnvironment(&pInterm, envDcp.pData, NULL);
    }
    else if (from_envType == uno_envType && to_envType != uno_envType) // UNO -> <ANY>?
        // mediate via uno(context)
    {
        rtl::OUString     envDcp = uno_envType;

         envDcp += to_envPurpose;
        uno_getEnvironment(&pInterm, envDcp.pData, NULL);
    }
    else // everything else
        // mediate via uno:purpose
    {
        rtl::OUString purpose = getPrefix(from_envPurpose, to_envPurpose);

        rtl::OUString uno_envDcp = uno_envType;
        uno_envDcp += purpose;

        uno_getEnvironment(&pInterm, uno_envDcp.pData, NULL);
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
