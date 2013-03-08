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


#include <stdio.h>

#include "sal/main.h"

#include "codemaker/typemanager.hxx"
#include "codemaker/generatedtypeset.hxx"

#include "cppuoptions.hxx"
#include "cpputype.hxx"

using ::rtl::OString;
using ::rtl::OUString;

namespace {

void failed(rtl::OString const & typeName, CppuOptions * options) {
    fprintf(stderr, "%s ERROR: %s\n", options->getProgramName().getStr(),
            rtl::OString("cannot dump Type '" + typeName + "'").getStr());
    exit(99);
}

void produce(
    RegistryKey& rTypeKey, bool bIsExtraType, TypeManager const & typeMgr,
    codemaker::GeneratedTypeSet & generated, CppuOptions * options)
{
    if (!produceType(rTypeKey, bIsExtraType, typeMgr, generated, options)) {
        OString typeName = typeMgr.getTypeName(rTypeKey);
        failed(typeName, options);
    }
}

void produce(
    rtl::OString const & typeName, TypeManager const & typeMgr,
    codemaker::GeneratedTypeSet & generated, CppuOptions * options)
{
    if (!produceType(typeName, typeMgr, generated, options)) {
        failed(typeName, options);
    }
}

void produceAllTypes(RegistryKey& rTypeKey, bool bIsExtraType,
                         TypeManager const & typeMgr,
                         codemaker::GeneratedTypeSet & generated,
                         CppuOptions* pOptions,
                         sal_Bool bFullScope)
    throw( CannotDumpException )
{
    OString typeName = typeMgr.getTypeName(rTypeKey);

    produce(rTypeKey, bIsExtraType, typeMgr, generated, pOptions);

    RegistryKeyList typeKeys = typeMgr.getTypeKeys(typeName);
    RegistryKeyList::const_iterator iter = typeKeys.begin();
    RegistryKey key, subKey;
    RegistryKeyArray subKeys;

    while (iter != typeKeys.end())
    {
        key = (*iter).first;

        if (!(*iter).second  && !key.openSubKeys(OUString(), subKeys))
        {
            for (sal_uInt32 i = 0; i < subKeys.getLength(); i++)
            {
                subKey = subKeys.getElement(i);
                if (bFullScope)
                {
                    produceAllTypes(subKey, (*iter).second, typeMgr,
                                    generated, pOptions, true);
                } else
                {
                    produce(subKey, (*iter).second,
                            typeMgr, generated, pOptions);
                }
            }
        }

        ++iter;
    }
}

void produceAllTypes(const OString& typeName,
                     TypeManager const & typeMgr,
                     codemaker::GeneratedTypeSet & generated,
                     CppuOptions* pOptions,
                     sal_Bool bFullScope)
    throw( CannotDumpException )
{
    produce(typeName, typeMgr, generated, pOptions);

    RegistryKeyList typeKeys = typeMgr.getTypeKeys(typeName);
    RegistryKeyList::const_iterator iter = typeKeys.begin();
    RegistryKey key, subKey;
    RegistryKeyArray subKeys;

    while (iter != typeKeys.end())
    {
        key = (*iter).first;
        if (!(*iter).second  && !key.openSubKeys(OUString(), subKeys))
        {
            for (sal_uInt32 i = 0; i < subKeys.getLength(); i++)
            {
                subKey = subKeys.getElement(i);
                if (bFullScope)
                {
                    produceAllTypes(subKey, (*iter).second, typeMgr,
                                    generated, pOptions, true);
                } else
                {
                    produce(subKey, (*iter).second,
                            typeMgr, generated, pOptions);
                }
            }
        }

        ++iter;
    }
}

}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    CppuOptions options;

    try
    {
        if (!options.initOptions(argc, argv))
        {
            exit(1);
        }
    }
    catch( IllegalArgument& e)
    {
        fprintf(stderr, "Illegal option: %s\n", e.m_message.getStr());
        exit(99);
    }

    RegistryTypeManager typeMgr;

    if (!typeMgr.init(options.getInputFiles(), options.getExtraInputFiles()))
    {
        fprintf(stderr, "%s : init registries failed, check your registry files.\n", options.getProgramName().getStr());
        exit(99);
    }

    if (options.isValid("-B"))
    {
        typeMgr.setBase(options.getOption("-B"));
    }

    codemaker::GeneratedTypeSet generated;
    try
    {
        if (options.isValid("-T"))
        {
            OString tOption(options.getOption("-T"));

            OString typeName, tmpName;
            sal_Int32 nIndex = 0;
            do
            {
                typeName = tOption.getToken(0, ';', nIndex);

                sal_Int32 nPos = typeName.lastIndexOf( '.' );
                tmpName = typeName.copy( nPos != -1 ? nPos+1 : 0 );
                if (tmpName == "*")
                {
                    // produce this type and his scope
                    if (typeName == "*")
                    {
                        tmpName = "/";
                    } else
                    {
                        tmpName = typeName.copy(0, typeName.lastIndexOf('.')).replace('.', '/');
                        if (tmpName.isEmpty())
                            tmpName = "/";
                        else
                            tmpName = tmpName.replace('.', '/');
                    }
                    // related to task #116780# the scope is recursively
                    // generated.  bFullScope = true
                    produceAllTypes(
                        tmpName, typeMgr, generated, &options, true);
                } else
                {
                    // produce only this type
                    produce(
                        typeName.replace('.', '/'), typeMgr, generated, &options);
                }
            } while( nIndex != -1 );
        } else
        {
            // produce all types
            produceAllTypes("/", typeMgr, generated, &options, true);
        }
        // C++ header files generated for the following UNO types are included
        // in header files in cppu/inc/com/sun/star/uno (Any.hxx, Reference.hxx,
        // Type.h), so it seems best to always generate those C++ header files:
        produce("com/sun/star/uno/RuntimeException", typeMgr, generated, &options);
        produce("com/sun/star/uno/TypeClass", typeMgr, generated, &options);
        produce("com/sun/star/uno/XInterface", typeMgr, generated, &options);
    }
    catch( CannotDumpException& e)
    {
        fprintf(stderr, "%s ERROR: %s\n",
                options.getProgramName().getStr(),
                e.m_message.getStr());
        exit(99);
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
