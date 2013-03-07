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

OString translateUnoToJavaType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    OString const & nucleus, bool referenceType)
{
    OStringBuffer buf;
    if (sort == codemaker::UnoType::SORT_COMPLEX) {
        if (typeClass == RT_TYPE_INTERFACE && nucleus == "com/sun/star/uno/XInterface")
        {
            buf.append("java/lang/Object");
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
        return prefix + "_" + identifier;
    } else {
        return identifier;
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
