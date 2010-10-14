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
#include "precompiled_codemaker.hxx"

#include <stdio.h>

#include "sal/main.h"

#include <codemaker/typemanager.hxx>
#include <codemaker/dependency.hxx>

#include "cunooptions.hxx"
#include "cunotype.hxx"

using namespace rtl;

sal_Bool produceAllTypes(const OString& typeName,
                         TypeManager& typeMgr,
                         TypeDependency& typeDependencies,
                         CunoOptions* pOptions,
                         sal_Bool bFullScope)
    throw( CannotDumpException )
{
    if (!produceType(typeName, typeMgr, typeDependencies, pOptions))
    {
        fprintf(stderr, "%s ERROR: %s\n",
                pOptions->getProgramName().getStr(),
                OString("cannot dump Type '" + typeName + "'").getStr());
        exit(99);
    }

    RegistryKey typeKey = typeMgr.getTypeKey(typeName);
    RegistryKeyNames subKeys;

    if (typeKey.getKeyNames(OUString(), subKeys))
        return sal_False;

    OString tmpName;
    for (sal_uInt32 i=0; i < subKeys.getLength(); i++)
    {
        tmpName = OUStringToOString(subKeys.getElement(i), RTL_TEXTENCODING_UTF8);

        if (pOptions->isValid("-B"))
            tmpName = tmpName.copy(tmpName.indexOf('/', 1) + 1);
        else
            tmpName = tmpName.copy(1);

        if (bFullScope)
        {
            if (!produceAllTypes(tmpName, typeMgr, typeDependencies, pOptions, sal_True))
                return sal_False;
        } else
        {
            if (!produceType(tmpName, typeMgr, typeDependencies, pOptions))
                return sal_False;
        }
    }

    return sal_True;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    CunoOptions options;

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
    TypeDependency      typeDependencies;

    if (!typeMgr.init(!options.isValid("-T"), options.getInputFiles()))
    {
        fprintf(stderr, "%s : init registries failed, check your registry files.\n", options.getProgramName().getStr());
        exit(99);
    }

    if (options.isValid("-B"))
    {
        typeMgr.setBase(options.getOption("-B"));
    }

    try
    {
        if (options.isValid("-T"))
        {
            OString tOption(options.getOption("-T"));

            OString typeName, tmpName;
            sal_Bool ret = sal_False;
            sal_Int32 nIndex = 0;
            do
            {
                typeName = tOption.getToken(0, ';', nIndex);

                sal_Int32 nPos = typeName.lastIndexOf( '.' );
                tmpName = typeName.copy( nPos != -1 ? nPos+1 : 0 );
                if (tmpName == "*")
                {
                    // produce this type and his scope, but the scope is not recursively  generated.
                    if (typeName.equals("*"))
                    {
                        tmpName = "/";
                    } else
                    {
                        tmpName = typeName.copy(0, typeName.lastIndexOf('.')).replace('.', '/');
                        if (tmpName.getLength() == 0)
                            tmpName = "/";
                        else
                            tmpName.replace('.', '/');
                    }
                    ret = produceAllTypes(tmpName, typeMgr, typeDependencies, &options, sal_False);
                } else
                {
                    // produce only this type
                    ret = produceType(typeName.replace('.', '/'), typeMgr, typeDependencies, &options);
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
            if (!produceAllTypes("/", typeMgr, typeDependencies, &options, sal_True))
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
                e.m_message.getStr());
        exit(99);
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
