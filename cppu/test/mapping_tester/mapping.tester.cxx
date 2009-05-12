/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mapping.tester.cxx,v $
 * $Revision: 1.3 $
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

#define CPPU_TEST_MAPPING_TESTER_TESTS_IMPL


#include <iostream>

#include "sal/main.h"
#include "osl/module.hxx"
#include "rtl/ustring.hxx"
#include "uno/environment.hxx"
#include "uno/mapping.hxx"

#include "../ObjectFactory/ObjectFactory.hxx"
#include "Mapping.tester.hxx"

using namespace ::com::sun::star;



static uno::Mapping s_getMapping(rtl::OUString    const & from_envDcp,
                                 rtl::OUString    const & to_envDcp,
                                 uno::Environment       * pSourceEnv,
                                 uno::Environment       * pTargetEnv)
{
    uno::Environment sourceEnv(from_envDcp);
    uno::Environment targetEnv(to_envDcp);

    uno::Mapping mapping(sourceEnv, targetEnv);

    if (pSourceEnv)
        *pSourceEnv = sourceEnv;

    if (pTargetEnv)
        *pTargetEnv = targetEnv;

    return mapping;
}

static void * s_mapObject(rtl::OUString   const  & from_envDcp,
                          rtl::OUString   const  & to_envDcp,
                          void                   * object,
                          uno::Environment       * pSourceEnv,
                          uno::Environment       * pTargetEnv)
{
    uno::Mapping mapping(s_getMapping(from_envDcp, to_envDcp, pSourceEnv, pTargetEnv));

    void * mapped_object = NULL;

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;

    TYPELIB_DANGER_GET( (typelib_TypeDescription **) &pTXInterfaceDescr, type_XInterface );
    mapping.mapInterface(&mapped_object, object, pTXInterfaceDescr);
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *) pTXInterfaceDescr );

    return mapped_object;
}


static rtl::OUString s_test_regetMapping(void)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tmapping reget test        --> "));

    uno::Mapping mapping1(g_from_envDcp, g_to_envDcp);
    uno::Mapping mapping2(g_from_envDcp, g_to_envDcp);

    if (mapping2.get() == mapping1.get())
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));

    else
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));

    result += g_result;

    return result;
}

static rtl::OUString s_test_mapObject(Callee * pCallee)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tobject map test           --> "));

    void * pObject = createObject(g_to_envDcp, pCallee);

    uno::Environment sourceEnv;
    uno::Environment targetEnv;
    void * mapped_object = s_mapObject(g_to_envDcp, g_from_envDcp, pObject, &sourceEnv, &targetEnv);
    sourceEnv.get()->pExtEnv->releaseInterface(sourceEnv.get()->pExtEnv, pObject);
    sourceEnv.clear();

    if (mapped_object)
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));

    else
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));

    targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, mapped_object);

    result += g_result;

    return result;
}

static rtl::OUString s_test_remapObject(Callee * pCallee)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tobject remap test         --> "));

    void * pObject = createObject(g_to_envDcp, pCallee);

    uno::Environment sourceEnv;
    uno::Environment targetEnv;
    void * mapped_object1 = s_mapObject(g_to_envDcp, g_from_envDcp, pObject, &sourceEnv, &targetEnv);
    void * mapped_object2 = s_mapObject(g_to_envDcp, g_from_envDcp, pObject, NULL, NULL);
    sourceEnv.get()->pExtEnv->releaseInterface(sourceEnv.get()->pExtEnv, pObject);
    sourceEnv.clear();

    if (mapped_object2 == mapped_object1)
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));

    else
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));

    targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, mapped_object1);
    targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, mapped_object2);

    result += g_result;

    return result;
}

static rtl::OUString s_test_mapBackObject(Callee * pCallee)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tobject map back test      --> "));

    void * pObject = createObject(g_to_envDcp, pCallee);

    uno::Environment to_sourceEnv;
    uno::Environment to_targetEnv;
    void * mapped_object = s_mapObject(g_to_envDcp,
                                       g_from_envDcp,
                                       pObject,
                                       &to_sourceEnv,
                                       &to_targetEnv);

    uno::Environment back_sourceEnv;
    uno::Environment back_targetEnv;
     void * mapped_back_object = s_mapObject(g_from_envDcp,
                                            g_to_envDcp,
                                            mapped_object,
                                            &back_sourceEnv,
                                            &back_targetEnv);

    if (back_targetEnv.get() != to_sourceEnv.get())
        abort();

    if (back_sourceEnv.get() != to_targetEnv.get())
        abort();

    if (pObject == mapped_back_object)
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));

    else
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));

    to_sourceEnv  .get()->pExtEnv->releaseInterface(to_sourceEnv  .get()->pExtEnv, pObject);
    to_targetEnv  .get()->pExtEnv->releaseInterface(to_targetEnv  .get()->pExtEnv, mapped_object);
    back_targetEnv.get()->pExtEnv->releaseInterface(back_targetEnv.get()->pExtEnv, mapped_back_object);

    result += g_result;

    return result;
}

static rtl::OUString s_test_objectRegistered(Callee * pCallee)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tobject registered test    --> "));

    void * pObject = createObject(g_to_envDcp, pCallee);

    uno::Environment sourceEnv;
    uno::Environment targetEnv;
    void * mapped_object = s_mapObject(g_to_envDcp,
                                       g_from_envDcp,
                                       pObject,
                                       &sourceEnv,
                                       &targetEnv);

    if (sourceEnv.get() == targetEnv.get())
    {
        if (mapped_object != pObject)
            abort();
    }
    else
    {
        typelib_TypeDescriptionReference * type_XInterface =
            * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

        typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;
        TYPELIB_DANGER_GET( (typelib_TypeDescription **) &pTXInterfaceDescr, type_XInterface );

        rtl_uString * pOId = 0;
        targetEnv.get()->pExtEnv->getObjectIdentifier(targetEnv.get()->pExtEnv, &pOId, mapped_object);

        void * pOut = NULL;

        targetEnv.get()->pExtEnv->getRegisteredInterface(targetEnv.get()->pExtEnv,
                                                         &pOut,
                                                         pOId,
                                                         pTXInterfaceDescr);

        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *) pTXInterfaceDescr );

        if (mapped_object == pOut)
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OKAY\n"));

        else
            result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILED\n"));


        targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, pOut);
    }

    sourceEnv.get()->pExtEnv->releaseInterface(sourceEnv.get()->pExtEnv, pObject);
    targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, mapped_object);

    result += g_result;

    return result;
}


// static void s_test_mappingNotEqual(char const * source1Name, char const * dest1Name,
//                                 char const * source2Name, char const * dest2Name)
// {
//  rtl::OUString source1EnvName(source1Name, rtl_str_getLength(source1Name), RTL_TEXTENCODING_ASCII_US);
//  rtl::OUString dest1EnvName  (dest1Name,   rtl_str_getLength(dest1Name),   RTL_TEXTENCODING_ASCII_US);

//  rtl::OUString source2EnvName(source2Name, rtl_str_getLength(source2Name), RTL_TEXTENCODING_ASCII_US);
//  rtl::OUString dest2EnvName  (dest2Name,   rtl_str_getLength(dest2Name),   RTL_TEXTENCODING_ASCII_US);

//  uno::Mapping mapping1(s_getMapping(source1EnvName, dest1EnvName, NULL, NULL));
//  uno::Mapping mapping2(s_getMapping(source2EnvName, dest2EnvName, NULL, NULL));

//  if (mapping2.get() == mapping1.get())
//  {
// //       fprintf(stderr, "***************** can not reget mapping\n");
//      abort();
//  }
// }

static rtl::OUString s_test_call(Callee * pCallee)
{
    g_result = rtl::OUString();
    g_custom = rtl::OUString();

    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\tobject call test    --> "));

    void * pObject = createObject(g_to_envDcp, pCallee);

    uno::Environment sourceEnv;
    uno::Environment targetEnv;
    void * mapped_object = s_mapObject(g_to_envDcp, g_from_envDcp, pObject, &sourceEnv, &targetEnv);
    sourceEnv.get()->pExtEnv->releaseInterface(sourceEnv.get()->pExtEnv, pObject);
    sourceEnv.clear();

    g_check = 1;
    callObject(g_from_envDcp, mapped_object);
    g_check = 0;

    targetEnv.get()->pExtEnv->releaseInterface(targetEnv.get()->pExtEnv, mapped_object);

    result += g_result;

    return result;
}


static rtl::OString test_mapping_a(char const * pfrom_envDcp,
                                   char const * pto_envDcp,
                                   Callee     * pCallee,
                                   char const * pRef)
{
      g_from_envDcp = rtl::OUString(pfrom_envDcp, rtl_str_getLength(pfrom_envDcp), RTL_TEXTENCODING_ASCII_US);
     g_to_envDcp   = rtl::OUString(pto_envDcp,   rtl_str_getLength(pto_envDcp),   RTL_TEXTENCODING_ASCII_US);
     g_ref         = rtl::OUString(pRef,         rtl_str_getLength(pRef),         RTL_TEXTENCODING_ASCII_US);

    rtl::OUString result;

    g_check = 0;

    result += s_test_regetMapping    ();
      result += s_test_objectRegistered(pCallee);
       result += s_test_remapObject     (pCallee);
      result += s_test_mapBackObject   (pCallee);
       result += s_test_mapObject       (pCallee);
    result += s_test_call            (pCallee);

    return rtl::OUStringToOString(result, RTL_TEXTENCODING_ASCII_US);
}


static rtl::OString s_replaceCPP(rtl::OString const & str)
{
    rtl::OString result;

    sal_Int32 index_old = 0;
    sal_Int32 index     = str.indexOf("CPP", index_old);
    while (index != -1)
    {
        result += str.copy(index_old, index - index_old);
        result += rtl::OString(CPPU_STRINGIFY(CPPU_ENV));

        index_old = index + 3;
        index = str.indexOf("CPP", index_old);
    }
    result += str.copy(index_old);

    return result;
}


extern "C" { static void s_dummy(rtl_uString *  pMethod_name)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\t\tmethod: "));
    result += rtl::OUString(pMethod_name);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" -dummy-\n"));

    g_result = result;
}}

static Callee * s_pCustomCallee = s_dummy;

extern "C" { static void s_callee(rtl_uString *  pMethod_name)
{
    if (rtl::OUString(pMethod_name).equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::queryInterface"))))
    {
        rtl::OUString tmp;

        if (!g_from_envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))))
        {
            tmp += g_from_envDcp;
            tmp += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-->"));
        }

        tmp += g_custom;

        if (!g_to_envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))))
        {
            tmp += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-->"));
            tmp += g_to_envDcp;
        }

        g_custom = tmp;
        s_pCustomCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::queryInterface")).pData);
    }
    else
        s_pCustomCallee(pMethod_name);
}}


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int ret = 0;
    rtl::OString message;

    if (argc == 3 || argc == 6)
    {
        rtl::OString from_envDcp(argv[1]);
        from_envDcp = s_replaceCPP(from_envDcp);

        rtl::OString to_envDcp(argv[2]);
        to_envDcp = s_replaceCPP(to_envDcp);

        osl::Module module;

        rtl::OString ref;
        if (argc == 6 && rtl::OString(argv[3]).equals(rtl::OString("-s")))
        {
            rtl::OUString libName(argv[4], rtl_str_getLength(argv[4]), RTL_TEXTENCODING_ASCII_US);
            libName += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SAL_DLLEXTENSION));
            module.load(libName);

            s_pCustomCallee = (Callee *)module.getFunctionSymbol(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CALLEE")));
            if (!s_pCustomCallee)
                abort();

            ref = s_replaceCPP(rtl::OString(argv[5]));
        }

        message += rtl::OString("TESTING: ");
        message += from_envDcp;
        message += rtl::OString("-->");
        message += to_envDcp;
        message += rtl::OString("\n");

        message += test_mapping_a(from_envDcp.getStr(), to_envDcp.getStr(), s_callee, ref);

        if (message.indexOf(rtl::OString("FAILED")) == -1)
            message += rtl::OString("TESTS PASSED\n");

        else
        {
            message += rtl::OString("TESTS _NOT_ PASSED\n");
            ret = -1;
        }
    }
    else
        message = "Usage: <source env. dcp> <destination env. dcp> [-s testfun <ref. string>]\n";


    std::cout << message.getStr();

    return ret;
}
