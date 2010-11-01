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

#include "codemaker/commonjava.hxx"

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

namespace codemaker { namespace java {

rtl::OString translateUnoToJavaType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & nucleus, bool referenceType)
{
    rtl::OStringBuffer buf;
    if (sort == codemaker::UnoType::SORT_COMPLEX) {
        if (typeClass == RT_TYPE_INTERFACE
            && nucleus == rtl::OString("com/sun/star/uno/XInterface"))
        {
            buf.append(RTL_CONSTASCII_STRINGPARAM("java/lang/Object"));
        } else {
            //TODO: check that nucleus is a valid (Java-modified UTF-8)
            // identifier
            buf.append(nucleus);
        }
    } else {
        rtl::OString const javaTypes[codemaker::UnoType::SORT_ANY + 1][2] = {
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("void")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Void")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("boolean")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Boolean")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("byte")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Byte")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("short")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Short")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("short")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Short")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("int")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("int")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("long")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Long")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("long")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Long")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("float")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Float")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("double")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Double")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("char")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Character")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/String")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/String")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")) } };
        buf.append(javaTypes[sort][referenceType]);
    }
    return buf.makeStringAndClear();
}

rtl::OString translateUnoToJavaIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix)
{
    if (identifier == "abstract"
        || identifier == "assert" // since Java 1.4
        || identifier == "boolean"
        || identifier == "break"
        || identifier == "byte"
        || identifier == "case"
        || identifier == "catch"
        || identifier == "char"
        || identifier == "class"
        || identifier == "const"
        || identifier == "continue"
        || identifier == "default"
        || identifier == "do"
        || identifier == "double"
        || identifier == "else"
        || identifier == "enum" // probable addition in Java 1.5
        || identifier == "extends"
        || identifier == "final"
        || identifier == "finally"
        || identifier == "float"
        || identifier == "for"
        || identifier == "goto"
        || identifier == "if"
        || identifier == "implements"
        || identifier == "import"
        || identifier == "instanceof"
        || identifier == "int"
        || identifier == "interface"
        || identifier == "long"
        || identifier == "native"
        || identifier == "new"
        || identifier == "package"
        || identifier == "private"
        || identifier == "protected"
        || identifier == "public"
        || identifier == "return"
        || identifier == "short"
        || identifier == "static"
        || identifier == "strictfp"
        || identifier == "super"
        || identifier == "switch"
        || identifier == "synchronized"
        || identifier == "this"
        || identifier == "throw"
        || identifier == "throws"
        || identifier == "transient"
        || identifier == "try"
        || identifier == "void"
        || identifier == "volatile"
        || identifier == "while")
    {
        rtl::OStringBuffer buf(prefix);
        buf.append('_');
        buf.append(identifier);
        return buf.makeStringAndClear();
    } else {
        return identifier;
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
