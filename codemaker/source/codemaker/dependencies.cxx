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


#include "codemaker/dependencies.hxx"

#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

using codemaker::Dependencies;

namespace {

struct Bad {};

}

Dependencies::Dependencies(
    rtl::Reference< TypeManager > const & manager, OString const & type):
    m_voidDependency(false), m_booleanDependency(false),
    m_byteDependency(false), m_shortDependency(false),
    m_unsignedShortDependency(false), m_longDependency(false),
    m_unsignedLongDependency(false), m_hyperDependency(false),
    m_unsignedHyperDependency(false), m_floatDependency(false),
    m_doubleDependency(false), m_charDependency(false),
    m_stringDependency(false), m_typeDependency(false), m_anyDependency(false),
    m_sequenceDependency(false)
{
    typereg::Reader reader(manager->getTypeReader(type));
    m_valid = reader.isValid();
    if (m_valid) {
        // Not everything is checked for consistency, just things that are cheap
        // to test:
        try {
            RTTypeClass tc = reader.getTypeClass();
            if (tc != RT_TYPE_SERVICE) {
                for (sal_Int16 i = 0; i < reader.getSuperTypeCount(); ++i) {
                    insert(reader.getSuperTypeName(i), true);
                }
            }
            if (tc != RT_TYPE_ENUM) {
                for (sal_Int16 i = 0; i < reader.getFieldCount(); ++i) {
                    if ((reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE)
                        == 0)
                    {
                        insert(reader.getFieldTypeName(i), false);
                    }
                }
            }
            for (sal_Int16 i = 0; i < reader.getMethodCount(); ++i) {
                insert(reader.getMethodReturnTypeName(i), false);
                for (sal_Int16 j = 0; j < reader.getMethodParameterCount(i);
                      ++j)
                {
                    if ((reader.getMethodParameterFlags(i, j) & RT_PARAM_REST)
                        != 0)
                    {
                        m_sequenceDependency = true;
                    }
                    insert(reader.getMethodParameterTypeName(i, j), false);
                }
                for (sal_Int16 j = 0; j < reader.getMethodExceptionCount(i);
                      ++j)
                {
                    insert(reader.getMethodExceptionTypeName(i, j), false);
                }
            }
            for (sal_Int16 i = 0; i < reader.getReferenceCount(); ++i) {
                if (reader.getReferenceSort(i) != RT_REF_TYPE_PARAMETER) {
                    insert(reader.getReferenceTypeName(i), false);
                }
            }
        } catch (Bad &) {
            m_map.clear();
            m_valid = false;
            m_voidDependency = false;
            m_booleanDependency = false;
            m_byteDependency = false;
            m_shortDependency = false;
            m_unsignedShortDependency = false;
            m_longDependency = false;
            m_unsignedLongDependency = false;
            m_hyperDependency = false;
            m_unsignedHyperDependency = false;
            m_floatDependency = false;
            m_doubleDependency = false;
            m_charDependency = false;
            m_stringDependency = false;
            m_typeDependency = false;
            m_anyDependency = false;
            m_sequenceDependency = false;
        }
    }
}

Dependencies::~Dependencies()
{}

void Dependencies::insert(OUString const & type, bool base) {
    OString t;
    if (!type.convertToString(
            &t, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw Bad();
    }
    insert(t, base);
}

void Dependencies::insert(OString const & type, bool base) {
    sal_Int32 rank;
    std::vector< OString > args;
    OString t(UnoType::decompose(type, &rank, &args));
    if (rank > 0) {
        m_sequenceDependency = true;
    }
    switch (UnoType::getSort(t)) {
    case UnoType::SORT_VOID:
        if (rank != 0 || !args.empty()) {
            throw Bad();
        }
        m_voidDependency = true;
        break;

    case UnoType::SORT_BOOLEAN:
        if (!args.empty()) {
            throw Bad();
        }
        m_booleanDependency = true;
        break;

    case UnoType::SORT_BYTE:
        if (!args.empty()) {
            throw Bad();
        }
        m_byteDependency = true;
        break;

    case UnoType::SORT_SHORT:
        if (!args.empty()) {
            throw Bad();
        }
        m_shortDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_SHORT:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedShortDependency = true;
        break;

    case UnoType::SORT_LONG:
        if (!args.empty()) {
            throw Bad();
        }
        m_longDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_LONG:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedLongDependency = true;
        break;

    case UnoType::SORT_HYPER:
        if (!args.empty()) {
            throw Bad();
        }
        m_hyperDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_HYPER:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedHyperDependency = true;
        break;

    case UnoType::SORT_FLOAT:
        if (!args.empty()) {
            throw Bad();
        }
        m_floatDependency = true;
        break;

    case UnoType::SORT_DOUBLE:
        if (!args.empty()) {
            throw Bad();
        }
        m_doubleDependency = true;
        break;

    case UnoType::SORT_CHAR:
        if (!args.empty()) {
            throw Bad();
        }
        m_charDependency = true;
        break;

    case UnoType::SORT_STRING:
        if (!args.empty()) {
            throw Bad();
        }
        m_stringDependency = true;
        break;

    case UnoType::SORT_TYPE:
        if (!args.empty()) {
            throw Bad();
        }
        m_typeDependency = true;
        break;

    case UnoType::SORT_ANY:
        if (!args.empty()) {
            throw Bad();
        }
        m_anyDependency = true;
        break;

    case UnoType::SORT_COMPLEX:
        {
            for (std::vector< OString >::iterator i(args.begin());
                 i != args.end(); ++i)
            {
                insert(*i, false);
            }
            Map::iterator i(m_map.find(t));
            if (i == m_map.end()) {
                m_map.insert(
                    Map::value_type(t, base ? KIND_BASE : KIND_NO_BASE));
            } else if (base) {
                i->second = KIND_BASE;
            }
            break;
        }

    default:
        OSL_ASSERT(false);
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
