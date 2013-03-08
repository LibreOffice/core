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


#include "sal/config.h"

#include "codemaker/codemaker.hxx"

#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/strbuf.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

namespace {

void checkNoTypeArguments(std::vector< rtl::OString > const & arguments) {
    if (!arguments.empty()) {
        throw CannotDumpException("Bad type information");
            //TODO
    }
}

}

namespace codemaker {

rtl::OString convertString(rtl::OUString const & string) {
    rtl::OString s;
    if (!string.convertToString(
            &s, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw CannotDumpException("Failure converting string from UTF-16 to UTF-8");
    }
    return s;
}

codemaker::UnoType::Sort decomposeAndResolve(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments)
{
    OSL_ASSERT(typeClass != 0 && name != 0 && rank != 0 && arguments != 0);
    *rank = 0;
    for (rtl::OString t(type);;) {
        sal_Int32 n = 0;
        *name = codemaker::UnoType::decompose(t, &n, arguments);
        if (n > SAL_MAX_INT32 - *rank) {
            throw CannotDumpException("Bad type information: " + type);
            //TODO
        }
        *rank += n;
        if (n > 0) {
            allowVoid = false;
            allowExtraEntities = false;
        }
        codemaker::UnoType::Sort sort = codemaker::UnoType::getSort(*name);
        switch (sort) {
        case codemaker::UnoType::SORT_VOID:
            if (!allowVoid) {
                throw CannotDumpException("Bad type information: " + type);
                //TODO
            }
        default:
            checkNoTypeArguments(*arguments);
            *typeClass = RT_TYPE_INVALID;
            return sort;

        case codemaker::UnoType::SORT_COMPLEX:
            typereg::Reader reader(manager.getTypeReader(*name));
            *typeClass = reader.getTypeClass();
            switch (*typeClass) {
            case RT_TYPE_ENUM:
            case RT_TYPE_INTERFACE:
                checkNoTypeArguments(*arguments);
                return sort;

            case RT_TYPE_STRUCT:
                if (!(allowExtraEntities && arguments->empty())
                    && (arguments->size() > SAL_MAX_UINT16
                        || (static_cast< sal_uInt16 >(arguments->size())
                            != reader.getReferenceCount())))
                {
                    throw CannotDumpException("Bad type information: " + type);
                    //TODO
                }
                return sort;

            case RT_TYPE_MODULE:
            case RT_TYPE_EXCEPTION:
            case RT_TYPE_SERVICE:
            case RT_TYPE_SINGLETON:
            case RT_TYPE_CONSTANTS:
                if (!allowExtraEntities) {
                    throw CannotDumpException("Bad type information: " + type);
                    //TODO
                }
                checkNoTypeArguments(*arguments);
                //TODO: check reader for consistency
                return sort;

            case RT_TYPE_TYPEDEF:
                checkNoTypeArguments(*arguments);
                if (reader.getSuperTypeCount() == 1
                    && reader.getFieldCount() == 0
                    && reader.getMethodCount() == 0
                    && reader.getReferenceCount() == 0)
                {
                    if (resolveTypedefs) {
                        t = convertString(reader.getSuperTypeName(0));
                        continue;
                    } else {
                        return sort;
                    }
                }
            default:
                throw CannotDumpException("Bad type information: " + type);
                //TODO
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
