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

#include <codemaker/commoncpp.hxx>

#include <codemaker/options.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <vector>

namespace codemaker::cpp {

OString scopedCppName(OString const & type, bool ns_alias)
{
    char c('/');
    sal_Int32 nPos = type.lastIndexOf( c );
    if (nPos == -1) {
        nPos = type.lastIndexOf( '.' );
        if (nPos == -1)
            return type;

        c = '.';
    }

    OStringBuffer tmpBuf(type.getLength()*2);
    nPos = 0;
    do
    {
        tmpBuf.append("::" + type.getToken(0, c, nPos));
    } while( nPos != -1 );

    OString s(tmpBuf.makeStringAndClear());
    if (ns_alias && s.startsWith("::com::sun::star::", &s))
    {
        s = "::css::" + s; // nicer shorthand
    }

    return s;
}

OString translateUnoToCppType(
    codemaker::UnoType::Sort sort, std::u16string_view nucleus)
{
    OStringBuffer buf;
    if (sort <= codemaker::UnoType::Sort::Any) {
        static char const * const cppTypes[static_cast<int>(codemaker::UnoType::Sort::Any) + 1] = {
            "void", "::sal_Bool", "::sal_Int8", "::sal_Int16", "::sal_uInt16",
            "::sal_Int32", "::sal_uInt32", "::sal_Int64", "::sal_uInt64",
            "float", "double", "::sal_Unicode", "rtl::OUString",
            "::css::uno::Type", "::css::uno::Any" };
        buf.append(cppTypes[static_cast<int>(sort)]);
    } else {
        if (sort == codemaker::UnoType::Sort::Interface
            && nucleus == u"com.sun.star.uno.XInterface")
        {
            buf.append("::css::uno::XInterface");
        } else {
            //TODO: check that nucleus is a valid (UTF-8) identifier
            buf.append(u2b(nucleus));
        }
    }
    return buf.makeStringAndClear();
}

OString translateUnoToCppIdentifier(
    OString const & unoIdentifier, std::string_view prefix,
    IdentifierTranslationMode transmode, OString const * forbidden)
{
    if (// Keywords:
        unoIdentifier == "asm"
        || unoIdentifier == "auto"
        || unoIdentifier == "bool"
        || unoIdentifier == "break"
        || unoIdentifier == "case"
        || unoIdentifier == "catch"
        || unoIdentifier == "char"
        || unoIdentifier == "class"
        || unoIdentifier == "const"
        /* unoIdentifier == "const_cast" */
        || unoIdentifier == "continue"
        || unoIdentifier == "default"
        || unoIdentifier == "delete"
        || unoIdentifier == "do"
        || unoIdentifier == "double"
        /* unoIdentifier == "dynamic_cast" */
        || unoIdentifier == "else"
        || unoIdentifier == "enum"
        || unoIdentifier == "explicit"
        || unoIdentifier == "export"
        || unoIdentifier == "extern"
        || unoIdentifier == "false"
        || unoIdentifier == "float"
        || unoIdentifier == "for"
        || unoIdentifier == "friend"
        || unoIdentifier == "goto"
        || unoIdentifier == "if"
        || unoIdentifier == "inline"
        || unoIdentifier == "int"
        || unoIdentifier == "long"
        || unoIdentifier == "mutable"
        || unoIdentifier == "namespace"
        || unoIdentifier == "new"
        || unoIdentifier == "operator"
        || unoIdentifier == "private"
        || unoIdentifier == "protected"
        || unoIdentifier == "public"
        || unoIdentifier == "register"
        /* unoIdentifier == "reinterpret_cast" */
        || unoIdentifier == "return"
        || unoIdentifier == "short"
        || unoIdentifier == "signed"
        || unoIdentifier == "sizeof"
        || unoIdentifier == "static"
        /* unoIdentifier == "static_cast" */
        || unoIdentifier == "struct"
        || unoIdentifier == "switch"
        || unoIdentifier == "template"
        || unoIdentifier == "this"
        || unoIdentifier == "throw"
        || unoIdentifier == "true"
        || unoIdentifier == "try"
        || unoIdentifier == "typedef"
        || unoIdentifier == "typeid"
        || unoIdentifier == "typename"
        || unoIdentifier == "union"
        || unoIdentifier == "unsigned"
        || unoIdentifier == "using"
        || unoIdentifier == "virtual"
        || unoIdentifier == "void"
        || unoIdentifier == "volatile"
        /* unoIdentifier == "wchar_t" */
        || unoIdentifier == "while"
        // Alternative representations:
        || unoIdentifier == "and"
        /* unoIdentifier == "and_eq" */
        || unoIdentifier == "bitand"
        || unoIdentifier == "bitor"
        || unoIdentifier == "compl"
        || unoIdentifier == "not"
        /* unoIdentifier == "not_eq" */
        || unoIdentifier == "or"
        /* unoIdentifier == "or_eq" */
        || unoIdentifier == "xor"
        /* unoIdentifier == "xor_eq" */
        // Standard macros:
        || (unoIdentifier == "BUFSIZ"
                || unoIdentifier == "CLOCKS_PER_SEC"
                || unoIdentifier == "EDOM"
                || unoIdentifier == "EOF"
                || unoIdentifier == "ERANGE"
                || unoIdentifier == "EXIT_FAILURE"
                || unoIdentifier == "EXIT_SUCCESS"
                || unoIdentifier == "FILENAME_MAX"
                || unoIdentifier == "FOPEN_MAX"
                || unoIdentifier == "HUGE_VAL"
                || unoIdentifier == "LC_ALL"
                || unoIdentifier == "LC_COLLATE"
                || unoIdentifier == "LC_CTYPE"
                || unoIdentifier == "LC_MONETARY"
                || unoIdentifier == "LC_NUMERIC"
                || unoIdentifier == "LC_TIME"
                || unoIdentifier == "L_tmpnam"
                || unoIdentifier == "MB_CUR_MAX"
                || unoIdentifier == "NULL"
                || unoIdentifier == "RAND_MAX"
                || unoIdentifier == "SEEK_CUR"
                || unoIdentifier == "SEEK_END"
                || unoIdentifier == "SEEK_SET"
                || unoIdentifier == "SIGABRT"
                || unoIdentifier == "SIGFPE"
                || unoIdentifier == "SIGILL"
                || unoIdentifier == "SIGINT"
                || unoIdentifier == "SIGSEGV"
                || unoIdentifier == "SIGTERM"
                || unoIdentifier == "SIG_DFL"
                || unoIdentifier == "SIG_ERR"
                || unoIdentifier == "SIG_IGN"
                || unoIdentifier == "TMP_MAX"
                || unoIdentifier == "WCHAR_MAX"
                || unoIdentifier == "WCHAR_MIN"
                || unoIdentifier == "WEOF"
                /* unoIdentifier == "_IOFBF" */
                /* unoIdentifier == "_IOLBF" */
                /* unoIdentifier == "_IONBF" */
                || unoIdentifier == "assert"
                || unoIdentifier == "errno"
                || unoIdentifier == "offsetof"
                || unoIdentifier == "setjmp"
                || unoIdentifier == "stderr"
                || unoIdentifier == "stdin"
                || unoIdentifier == "stdout"
                /* unoIdentifier == "va_arg" */
                /* unoIdentifier == "va_end" */
                /* unoIdentifier == "va_start" */
                // Standard values:
                || unoIdentifier == "CHAR_BIT"
                || unoIdentifier == "CHAR_MAX"
                || unoIdentifier == "CHAR_MIN"
                || unoIdentifier == "DBL_DIG"
                || unoIdentifier == "DBL_EPSILON"
                || unoIdentifier == "DBL_MANT_DIG"
                || unoIdentifier == "DBL_MAX"
                || unoIdentifier == "DBL_MAX_10_EXP"
                || unoIdentifier == "DBL_MAX_EXP"
                || unoIdentifier == "DBL_MIN"
                || unoIdentifier == "DBL_MIN_10_EXP"
                || unoIdentifier == "DBL_MIN_EXP"
                || unoIdentifier == "FLT_DIG"
                || unoIdentifier == "FLT_EPSILON"
                || unoIdentifier == "FLT_MANT_DIG"
                || unoIdentifier == "FLT_MAX"
                || unoIdentifier == "FLT_MAX_10_EXP"
                || unoIdentifier == "FLT_MAX_EXP"
                || unoIdentifier == "FLT_MIN"
                || unoIdentifier == "FLT_MIN_10_EXP"
                || unoIdentifier == "FLT_MIN_EXP"
                || unoIdentifier == "FLT_RADIX"
                || unoIdentifier == "FLT_ROUNDS"
                || unoIdentifier == "INT_MAX"
                || unoIdentifier == "INT_MIN"
                || unoIdentifier == "LDBL_DIG"
                || unoIdentifier == "LDBL_EPSILON"
                || unoIdentifier == "LDBL_MANT_DIG"
                || unoIdentifier == "LDBL_MAX"
                || unoIdentifier == "LDBL_MAX_10_EXP"
                || unoIdentifier == "LDBL_MAX_EXP"
                || unoIdentifier == "LDBL_MIN"
                || unoIdentifier == "LDBL_MIN_10_EXP"
                || unoIdentifier == "LDBL_MIN_EXP"
                || unoIdentifier == "LONG_MAX"
                || unoIdentifier == "LONG_MIN"
                || unoIdentifier == "MB_LEN_MAX"
                || unoIdentifier == "SCHAR_MAX"
                || unoIdentifier == "SCHAR_MIN"
                || unoIdentifier == "SHRT_MAX"
                || unoIdentifier == "SHRT_MIN"
                || unoIdentifier == "UCHAR_MAX"
                || unoIdentifier == "UINT_MAX"
                || unoIdentifier == "ULONG_MAX"
                || unoIdentifier == "USHRT_MAX")
            || (transmode == IdentifierTranslationMode::Global
                && (// Standard types:
                    /* unoIdentifier == "clock_t" */
                    /* unoIdentifier == "div_t" */
                    unoIdentifier == "FILE"
                    /* unoIdentifier == "fpos_t" */
                    /* unoIdentifier == "jmp_buf" */
                    || unoIdentifier == "lconv"
                    /* unoIdentifier == "ldiv_t" */
                    /* unoIdentifier == "mbstate_t" */
                    /* unoIdentifier == "ptrdiff_t" */
                    /* unoIdentifier == "sig_atomic_t" */
                    /* unoIdentifier == "size_t" */
                    /* unoIdentifier == "time_t" */
                    || unoIdentifier == "tm"
                    /* unoIdentifier == "va_list" */
                    /* unoIdentifier == "wctrans_t" */
                    /* unoIdentifier == "wctype_t" */
                    /* unoIdentifier == "wint_t" */
                    // Standard namespaces:
                    || unoIdentifier == "std"))
            // Others:
            || unoIdentifier == "NDEBUG"
            || (forbidden != nullptr && unoIdentifier == *forbidden) )
    {
        return OString::Concat(prefix) + "_" + unoIdentifier;
    } else {
        return unoIdentifier;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
