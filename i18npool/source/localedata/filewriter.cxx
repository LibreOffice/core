/*************************************************************************
 *
 *  $RCSfile: filewriter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: khong $ $Date: 2002-07-11 17:24:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "LocaleNode.hxx"
//-----------------------------------------
// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax
//-----------------------------------------
OFileWriter::OFileWriter(const char *pcFile, const char *locale ) {

    strncpy( m_pcFile , pcFile, 1024 );
    printf("file generated=%s\n", m_pcFile);
    m_f = fopen( m_pcFile , "w" );
    strcpy(theLocale, locale);
}

OFileWriter::~OFileWriter() {
    if(m_f)
        fclose( m_f );
}

void OFileWriter::writeInt(sal_Int16 nb) const
{
    fprintf(m_f, "%d", nb);
}

void OFileWriter::writeAsciiString(const char* str) const
{
    fprintf(m_f, "%s", str);
}

void OFileWriter::writeStringCharacters(const ::rtl::OUString& str) const
{
    for(int i = 0; i < str.getLength(); i++)
        fprintf(m_f, "0x%x, ", str[i]);
}

void OFileWriter::writeFunction(const char *func, const char *count, const char *array) const
{
    fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count)\n{\n", func, theLocale);
    fprintf(m_f, "\tcount = %s;\n", count);
    fprintf(m_f, "\treturn (sal_Unicode**)%s;\n}\n", array);
}

void OFileWriter::writeUseFunction(const char *func, const ::rtl::OUString& useLocale) const
{
    const char* locale = OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US).getStr();
    fprintf(m_f, "extern sal_Unicode **  SAL_CALL %s%s(sal_Int16& count);\n", func, locale);
    fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count)\n{\n", func, theLocale);
    fprintf(m_f, "\treturn %s%s(count);\n}\n", func, locale);
}

void OFileWriter::writeFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const
{
    fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale);
    fprintf(m_f, "\tcount = %s;\n", count);
    fprintf(m_f, "\tfrom = %s;\n", from);
    fprintf(m_f, "\tto = %s;\n", to);
    fprintf(m_f, "\treturn (sal_Unicode**)%s;\n}\n", array);
}

void OFileWriter::writeUseFunction(const char *func, const ::rtl::OUString& useLocale, const char *from, const char *to) const
{
    const char* locale = OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US).getStr();
    fprintf(m_f, "extern sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);\n", func, locale);
    fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale);
    fprintf(m_f, "\tfrom = %s;\n", from);
    fprintf(m_f, "\tto = %s;\n", to);
    fprintf(m_f, "\tconst sal_Unicode* str;\n");
    fprintf(m_f, "\treturn %s%s(count, str, str);\n}\n", func, locale);
}

void OFileWriter::writeFunction2(const char *func, const char *style, const char* attr, const char *array) const
{
    fprintf(m_f, "const sal_Unicode ***  SAL_CALL %s%s( sal_Int16& nStyles, sal_Int16& nAttributes )\n{\n", func, theLocale);
    fprintf(m_f, "\tnStyles     = %s;\n", style);
    fprintf(m_f, "\tnAttributes = %s;\n", attr);
    fprintf(m_f, "\treturn %s;\n}\n", array);
}

void OFileWriter::writeUseFunction2(const char *func, const ::rtl::OUString& useLocale) const
{
    const char* locale = OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US).getStr();
    fprintf(m_f, "extern const sal_Unicode ***  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nAttributes);\n", func, locale);
    fprintf(m_f, "const sal_Unicode ***  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nAttributes)\n{\n", func, theLocale);
    fprintf(m_f, "\treturn %s%s(nStyles, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const
{
    fprintf(m_f, "const sal_Unicode ****  SAL_CALL %s%s( sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes )\n{\n", func, theLocale);
    fprintf(m_f, "\tnStyles     = %s;\n", style);
    fprintf(m_f, "\tnLevels     = %s;\n", levels);
    fprintf(m_f, "\tnAttributes = %s;\n", attr);
    fprintf(m_f, "\treturn %s;\n}\n", array);
}

void OFileWriter::writeUseFunction3(const char *func, const ::rtl::OUString& useLocale) const
{
    const char* locale = OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US).getStr();
    fprintf(m_f, "extern const sal_Unicode ****  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);\n", func, locale);
    fprintf(m_f, "const sal_Unicode ****  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)\n{\n", func, theLocale);
    fprintf(m_f, "\treturn %s%s(nStyles, nLevels, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeIntParameter(const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const
{
    fprintf(m_f, "static const sal_Unicode %s%d[] = {%d};\n", pAsciiStr, count, val);
}

void OFileWriter::writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str, sal_Int16 count) const
{
    fprintf(m_f,"static const sal_Unicode default%s%d[] = {%d};\n", pAsciiStr, count,
        str.equalsAscii("true") ? 1 : 0);
}

void OFileWriter::writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str) const
{
    fprintf(m_f,"static const sal_Unicode default%s[] = {%d};\n", pAsciiStr,
        str.equalsAscii("true") ? 1 : 0);
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const
{
    fprintf(m_f, "static const sal_Unicode %s[] = {", pAsciiStr);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count) const
{
    fprintf(m_f, "static const sal_Unicode %s%d[] = {", pAsciiStr, count);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const
{
    fprintf(m_f, "static const sal_Unicode %s%d%d[] = {", pAsciiStr, count0, count1);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, const sal_Int16 count) const
{
    fprintf(m_f, "static const sal_Unicode %s%s%d[] = {", pTagStr, pAsciiStr, count);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const
{
    fprintf(m_f, "static const sal_Unicode %s%s[] = {", pTagStr, pAsciiStr);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const
{
    fprintf(m_f, "static const sal_Unicode %s%s%d%d[] = {", pTagStr, pAsciiStr, count0, count1);
    writeStringCharacters(aChars);
    fprintf(m_f, "0x0};\n");
}

void OFileWriter::flush(void) const
{
    fflush( m_f );
}

void OFileWriter::closeOutput(void) const
{
    if(m_f)
    {
        fclose( m_f );
        const_cast< OFileWriter * > ( this )->m_f = 0;
    }
}

