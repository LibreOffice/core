/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "sal/config.h"

#include "codemaker/commonjava.hxx"

#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/strbuf.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

namespace codemaker { namespace java {

OString translateUnoToJavaType(
    codemaker::UnoType::Sort sort, OString const & nucleus, bool referenceType)
{
    OStringBuffer buf;
    if (sort <= codemaker::UnoType::SORT_ANY) {
        OString const javaTypes[codemaker::UnoType::SORT_ANY + 1][2] = {
            { "void", "java/lang/Void" },
            { "boolean", "java/lang/Boolean" },
            { "byte", "java/lang/Byte" },
            { "short", "java/lang/Short" },
            { "short", "java/lang/Short" },
            { "int",  "java/lang/Integer" },
            { "int",  "java/lang/Integer" },
            { "long", "java/lang/Long" },
            { "long", "java/lang/Long" },
            { "float", "java/lang/Float" },
            { "double", "java/lang/Double" },
            { "char", "java/lang/Character" },
            { "java/lang/String", "java/lang/String" },
            { "com/sun/star/uno/Type", "com/sun/star/uno/Type" },
            { "java/lang/Object", "java/lang/Object" } };
        buf.append(javaTypes[sort][referenceType]);
    } else {
        if (nucleus == "com/sun/star/uno/XInterface") {
            buf.append("java/lang/Object");
        } else {
            
            
            buf.append(nucleus);
        }
    }
    return buf.makeStringAndClear();
}

OString translateUnoToJavaIdentifier(
    OString const & identifier, OString const & prefix)
{
    if (identifier == "abstract"
        || identifier == "assert" 
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
        || identifier == "enum" 
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
