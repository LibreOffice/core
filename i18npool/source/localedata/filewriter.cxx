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
#include "LocaleNode.hxx"

// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax

OFileWriter::OFileWriter(const char *pcFile, const char *locale ): theLocale(locale) {

    printf("file generated=%s\n", pcFile);
    m_f = fopen(pcFile, "w");
}

OFileWriter::~OFileWriter() {
    if(m_f)
        fclose( m_f );
}

void OFileWriter::writeInt(sal_Int16 nb) const
{
    fprintf(m_f, "%d", nb);
}

void OFileWriter::writeHexInt(sal_Int16 nb) const
{
    fprintf(m_f, "%x", nb);
}

void OFileWriter::writeAsciiString(const char* str) const
{
    fprintf(m_f, "%s", str);
}

void OFileWriter::writeStringCharacters(std::u16string_view str) const
{
    for(size_t i = 0; i < str.size(); i++)
        fprintf(m_f, "0x%x, ", str[i]);
}

void OFileWriter::writeOUStringLiteralCharacters(std::u16string_view str) const
{
    for(size_t i = 0; i < str.size(); i++)
        fprintf(m_f, "\\x%04x", str[i]);
}

void OFileWriter::writeOUStringRefFunction(const char *func, std::u16string_view useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
    const char* locale = aRefLocale.getStr();
    fprintf(m_f, "extern const OUString *  %s%s(sal_Int16& count);\n", func, locale);
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT const OUString *  %s%s(sal_Int16& count)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\treturn %s%s(count);\n}\n", func, locale);
}

void OFileWriter::writeOUStringRefFunction(const char *func, std::u16string_view useLocale, const char *to) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
    const char* locale = aRefLocale.getStr();
    fprintf(m_f, "extern OUString const * %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);\n", func, locale);
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT OUString const * %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\tto = %s;\n", to);
    fprintf(m_f, "\tconst sal_Unicode* tmp;\n");
    fprintf(m_f, "\treturn %s%s(count, from, tmp);\n}\n", func, locale);
}

void OFileWriter::writeOUStringFunction(const char *func, const char *count, const char *array) const
{
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT OUString const *  %s%s(sal_Int16& count)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\tcount = %s;\n", count);
    fprintf(m_f, "\treturn (OUString const *)%s;\n}\n", array);
}

void OFileWriter::writeOUStringFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const
{
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT OUString const * %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\tcount = %s;\n", count);
    fprintf(m_f, "\tfrom = %s;\n", from);
    fprintf(m_f, "\tto = %s;\n", to);
    fprintf(m_f, "\treturn (const OUString *)%s;\n}\n", array);
}

void OFileWriter::writeOUStringFunction2(const char *func, const char *style, const char* attr, const char *array) const
{
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT const OUString **  %s%s( sal_Int16& nStyles, sal_Int16& nAttributes )\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\tnStyles     = %s;\n", style);
    fprintf(m_f, "\tnAttributes = %s;\n", attr);
    fprintf(m_f, "\treturn (const OUString **)%s;\n}\n", array);
}

void OFileWriter::writeOUStringRefFunction2(const char *func, std::u16string_view useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
    const char* locale = aRefLocale.getStr();
    fprintf(m_f, "extern const OUString **  %s%s(sal_Int16& nStyles, sal_Int16& nAttributes);\n", func, locale);
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT const OUString **  %s%s(sal_Int16& nStyles, sal_Int16& nAttributes)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\treturn %s%s(nStyles, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeOUStringFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const
{
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT const OUString ***  %s%s( sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes )\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\tnStyles     = %s;\n", style);
    fprintf(m_f, "\tnLevels     = %s;\n", levels);
    fprintf(m_f, "\tnAttributes = %s;\n", attr);
    fprintf(m_f, "\treturn (const OUString ***) %s;\n}\n", array);
}

void OFileWriter::writeOUStringRefFunction3(const char *func, std::u16string_view useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
    const char* locale = aRefLocale.getStr();
    fprintf(m_f, "extern const OUString ***  %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);\n", func, locale);
    fprintf(m_f, "SAL_DLLPUBLIC_EXPORT const OUString ***  %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)\n{\n", func, theLocale.c_str());
    fprintf(m_f, "\treturn %s%s(nStyles, nLevels, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeOUStringLiteralIntParameter(const char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s%d = u\"\\x%x\";\n", pAsciiStr, count, val);
}

bool OFileWriter::writeOUStringLiteralDefaultParameter(const char* pAsciiStr, std::u16string_view str, sal_Int16 count) const
{
    bool bBool = str == u"true";
    fprintf(m_f,"static constexpr OUStringLiteral default%s%d = u\"\\x%x\";\n", pAsciiStr, count, bBool);
    return bBool;
}

void OFileWriter::writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s = u\"", pAsciiStr);
    writeOUStringLiteralCharacters(aChars);
    fprintf(m_f, "\";\n");
}

void OFileWriter::writeParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count) const
{
    fprintf(m_f, "static const sal_Unicode %s%d[] = {", pAsciiStr, count);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s%d = u\"", pAsciiStr, count);
    writeOUStringLiteralCharacters(aChars);
    fprintf(m_f, "\";\n");
}

void OFileWriter::writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count0, sal_Int16 count1) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s%d%d = u\"", pAsciiStr, count0, count1);
    writeOUStringLiteralCharacters(aChars);
    fprintf(m_f, "\";\n");
}

void OFileWriter::writeOUStringLiteralParameter(const char* pTagStr, const char* pAsciiStr, std::u16string_view aChars, const sal_Int16 count) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s%s%d = u\"", pTagStr, pAsciiStr, count);
    writeOUStringLiteralCharacters(aChars);
    fprintf(m_f, "\";\n");
}

void OFileWriter::writeOUStringLiteralParameter(const char* pTagStr, const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count0, sal_Int16 count1) const
{
    fprintf(m_f, "static constexpr OUStringLiteral %s%s%d%d = u\"", pTagStr, pAsciiStr, count0, count1);
    writeOUStringLiteralCharacters(aChars);
    fprintf(m_f, "\";\n");
}

void OFileWriter::closeOutput() const
{
    if(m_f)
    {
        fclose( m_f );
        const_cast< OFileWriter * > ( this )->m_f = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
