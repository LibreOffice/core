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
#include    <rtl/alloc.h>
#include    <rtl/ustring.hxx>
#include    <rtl/strbuf.hxx>

#include    "rdbtype.hxx"
#include    "rdboptions.hxx"

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
sal_Bool isBaseType(const OString& type)
{
    if ( type.equals("long") ||
          type.equals("short") ||
          type.equals("hyper") ||
          type.equals("string") ||
          type.equals("boolean") ||
          type.equals("char") ||
          type.equals("byte") ||
          type.equals("any") ||
          type.equals("type") ||
          type.equals("float") ||
          type.equals("double") ||
          type.equals("octet") ||
          type.equals("void") ||
          type.equals("unsigned long") ||
          type.equals("unsigned short") ||
          type.equals("unsigned hyper") )
        return sal_True;

    return sal_False;
}

sal_Bool produceDependedTypes(const OString& typeName,
                              TypeManager& typeMgr,
                              TypeDependency& typeDependencies,
                              RdbOptions* pOptions,
                              FileStream& o,
                              RegistryKey& regKey,
                              StringSet& filterTypes)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_True;

    TypeUsingSet usingSet(typeDependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();
    OString sTypeName;
    sal_Int32 index = 0;
    while (iter != usingSet.end())
    {
        sTypeName = (*iter).m_type;
        if ((index = sTypeName.lastIndexOf(']')) > 0)
            sTypeName = sTypeName.copy(index + 1);

        if ( !isBaseType(sTypeName) )
        {
            if (!produceType(sTypeName,
                                typeMgr,
                             typeDependencies,
                             pOptions,
                             o, regKey,
                             filterTypes,
                             sal_True))
            {
                fprintf(stderr, "%s ERROR: %s\n",
                        pOptions->getProgramName().getStr(),
                        OString("cannot dump Type '" + sTypeName + "'").getStr());
                cleanUp(sal_True);
                exit(99);
            }
        }
        ++iter;
    }

    return ret;
}

//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     RdbOptions* pOptions,
                     FileStream& o,
                     RegistryKey& regKey,
                     StringSet& filterTypes,
                     sal_Bool bDepend)
    throw( CannotDumpException )
{
    if (typeDependencies.isGenerated(typeName) )
        return sal_True;
    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName, bDepend))
        return sal_False;

    if ( !checkFilterTypes(typeName) )
    {
        if ( pOptions->generateTypeList() )
        {
            o << typeName.getStr() << "\n";
        } else
        {
            TypeReader reader = typeMgr.getTypeReader(typeName);

            if (!reader.isValid())
            {
                if (typeName.equals("/"))
                {
                    return sal_True;
                } else
                {
                    return sal_False;
                }
            }
            RegistryKey typeKey;
            if ( regKey.createKey( OStringToOUString(typeName, RTL_TEXTENCODING_UTF8), typeKey) )
            {
                return sal_False;
            }

            if ( typeKey.setValue(OUString(), RG_VALUETYPE_BINARY, (void*)reader.getBlop(), reader.getBlopSize()) )
            {
                return sal_False;
            }

        }
    }

    typeDependencies.setGenerated(typeName);
    sal_Bool ret = produceDependedTypes(typeName, typeMgr, typeDependencies,
                                        pOptions, o, regKey, filterTypes);

    return ret;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
