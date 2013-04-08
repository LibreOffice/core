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

#include "rtl/ref.hxx"
#include "sal/main.h"

#include "codemaker/typemanager.hxx"
#include "codemaker/generatedtypeset.hxx"
#include "javaoptions.hxx"
#include "javatype.hxx"

using ::rtl::OUString;
using ::rtl::OString;
sal_Bool produceAllTypes(RegistryKey& rTypeKey, sal_Bool bIsExtraType,
                         rtl::Reference< TypeManager > const & typeMgr,
                         codemaker::GeneratedTypeSet & generated,
                         JavaOptions* pOptions,
                         sal_Bool bFullScope)
{
    OString typeName = typeMgr->getTypeName(rTypeKey);

    if (!produceType(rTypeKey, bIsExtraType, typeMgr, generated, pOptions))
    {
        fprintf(stderr, "%s ERROR: %s\n",
                pOptions->getProgramName().getStr(),
                OString("cannot dump Type '" + typeName + "'").getStr());
        exit(99);
    }

    RegistryKeyList typeKeys = typeMgr->getTypeKeys(typeName);
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
                    if (!produceAllTypes(
                            subKey, (*iter).second,
                            typeMgr, generated, pOptions, sal_True))
                        return sal_False;
                } else
                {
                    if (!produceType(subKey, (*iter).second,
                                     typeMgr, generated, pOptions))
                        return sal_False;
                }
            }
        }

        ++iter;
    }

    return sal_True;
}

sal_Bool produceAllTypes(const OString& typeName,
                         rtl::Reference< TypeManager > const & typeMgr,
                         codemaker::GeneratedTypeSet & generated,
                         JavaOptions* pOptions,
                         sal_Bool bFullScope)
{
    if (!produceType(typeName, typeMgr, generated, pOptions))
    {
        fprintf(stderr, "%s ERROR: %s\n",
                pOptions->getProgramName().getStr(),
                OString("cannot dump Type '" + typeName + "'").getStr());
        exit(99);
    }

    RegistryKeyList typeKeys = typeMgr->getTypeKeys(typeName);
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
                    if (!produceAllTypes(
                            subKey, (*iter).second,
                            typeMgr, generated, pOptions, sal_True))
                        return sal_False;
                } else
                {
                    if (!produceType(subKey, (*iter).second,
                                     typeMgr, generated, pOptions))
                        return sal_False;
                }
            }
        }

        ++iter;
    }

    return sal_True;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    JavaOptions options;

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

    rtl::Reference< TypeManager > typeMgr(new TypeManager);

    if (!typeMgr->init(options.getInputFiles(), options.getExtraInputFiles()))
    {
        fprintf(stderr, "%s : init registries failed, check your registry files.\n", options.getProgramName().getStr());
        exit(99);
    }

    if (options.isValid("-B"))
    {
        typeMgr->setBase(options.getOption("-B"));
    }

    for (std::vector< rtl::OString >::const_iterator i(
             options.getExtraInputFiles().begin());
         i != options.getExtraInputFiles().end(); ++i)
    {
        typeMgr->loadProvider(b2u(*i), false);
    }
    for (std::vector< rtl::OString >::const_iterator i(
             options.getInputFiles().begin());
         i != options.getInputFiles().end(); ++i)
    {
        typeMgr->loadProvider(b2u(*i), true);
    }

    try
    {
        if (options.isValid("-T"))
        {
            OString tOption(options.getOption("-T"));
            sal_Int32 nIndex = 0;

            codemaker::GeneratedTypeSet generated;
            OString typeName, tmpName;
            sal_Bool ret = sal_False;
            do
            {
                typeName = tOption.getToken(0, ';', nIndex);

                sal_Int32 nPos = typeName.lastIndexOf( '.' );
                tmpName = typeName.copy( nPos != -1 ? nPos+1 : 0 );
                if (tmpName == "*")
                {
                    // produce this type and his scope.
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
                    ret = produceAllTypes(
                        tmpName, typeMgr, generated, &options, sal_True);
                } else
                {
                    // produce only this type
                    ret = produceType(
                        typeName.replace('.', '/'), typeMgr, generated,
                        &options);
                }

                if (!ret)
                {
                    fprintf(stderr, "%s ERROR: %s\n",
                            options.getProgramName().getStr(),
                            OString("cannot dump Type '" + typeName + "'").getStr());
                    exit(99);
                }
            } while( nIndex != -1 );
        } else
        {
            // produce all types
            codemaker::GeneratedTypeSet generated;
            if (!produceAllTypes("/", typeMgr, generated, &options, sal_True))
            {
                fprintf(stderr, "%s ERROR: %s\n",
                        options.getProgramName().getStr(),
                        "an error occurs while dumping all types.");
                exit(99);
            }
        }
    }
    catch( CannotDumpException& e)
    {
        fprintf(stderr, "%s ERROR: %s\n",
                options.getProgramName().getStr(),
                u2b(e.getMessage()).getStr());
        exit(99);
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
