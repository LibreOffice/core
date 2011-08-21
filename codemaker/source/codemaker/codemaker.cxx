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
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
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
        throw CannotDumpException(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "Failure converting string from UTF-16 to UTF-8")));
    }
    return s;
}

rtl::OString errorMsg(rtl::OString const & desc, rtl::OString const & type) {
    rtl::OStringBuffer msg(128);
    msg.append(desc);
    msg.append(type);
    return msg.makeStringAndClear();
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
            throw CannotDumpException(
                errorMsg(rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information: ")),
                    type));
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
                throw CannotDumpException(
                    errorMsg(rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information: ")),
                        type));
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
                    throw CannotDumpException(
                        errorMsg(rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("Bad type information: ")),
                            type));
                    //TODO
                }
                return sort;

            case RT_TYPE_MODULE:
            case RT_TYPE_EXCEPTION:
            case RT_TYPE_SERVICE:
            case RT_TYPE_SINGLETON:
            case RT_TYPE_CONSTANTS:
                if (!allowExtraEntities) {
                    throw CannotDumpException(
                        errorMsg(rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("Bad type information: ")),
                            type));
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
                throw CannotDumpException(
                    errorMsg(rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information: ")),
                        type));
                //TODO
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
