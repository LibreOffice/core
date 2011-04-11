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
