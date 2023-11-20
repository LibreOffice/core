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


#include <sal/config.h>

#include <codemaker/commonjava.hxx>

#include <codemaker/options.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>

#include <rtl/strbuf.h>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <vector>

namespace codemaker::java {

OString translateUnoToJavaType(
    codemaker::UnoType::Sort sort, std::string_view nucleus, bool referenceType)
{
    OStringBuffer buf(128);
    if (sort <= codemaker::UnoType::Sort::Any) {
        OString const javaTypes[static_cast<int>(codemaker::UnoType::Sort::Any) + 1][2] = {
            { "void"_ostr, "java/lang/Void"_ostr },
            { "boolean"_ostr, "java/lang/Boolean"_ostr },
            { "byte"_ostr, "java/lang/Byte"_ostr },
            { "short"_ostr, "java/lang/Short"_ostr },
            { "short"_ostr, "java/lang/Short"_ostr },
            { "int"_ostr,  "java/lang/Integer"_ostr },
            { "int"_ostr,  "java/lang/Integer"_ostr },
            { "long"_ostr, "java/lang/Long"_ostr },
            { "long"_ostr, "java/lang/Long"_ostr },
            { "float"_ostr, "java/lang/Float"_ostr },
            { "double"_ostr, "java/lang/Double"_ostr },
            { "char"_ostr, "java/lang/Character"_ostr },
            { "java/lang/String"_ostr, "java/lang/String"_ostr },
            { "com/sun/star/uno/Type"_ostr, "com/sun/star/uno/Type"_ostr },
            { "java/lang/Object"_ostr, "java/lang/Object"_ostr } };
        buf.append(javaTypes[static_cast<int>(sort)][referenceType]);
    } else {
        if (nucleus == "com/sun/star/uno/XInterface") {
            buf.append("java/lang/Object");
        } else {
            //TODO: check that nucleus is a valid (Java-modified UTF-8)
            // identifier
            buf.append(nucleus);
        }
    }
    return buf.makeStringAndClear();
}

OString translateUnoToJavaIdentifier(
    OString const & identifier, std::string_view prefix)
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
        return OString::Concat(prefix) + "_" + identifier;
    } else {
        return identifier;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
