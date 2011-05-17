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
#include "precompiled_i18npool.hxx"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sal/main.h>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <vector>
using std::vector;

using namespace ::rtl;

/* Utility gendict:

   "BreakIterator_CJK provides input string caching and dictionary searching for
   longest matching. You can provide a sorted dictionary (the encoding must be
   UTF-8) by creating the following file:
            i18npool/source/breakiterator/data/<language>.dict.

   The utility gendict will convert the file to C code, which will be compiled
   into a shared library for dynamic loading.

   All dictionary searching and loading is performed in the xdictionary class.
   The only thing you need to do is to derive your class from BreakIterator_CJK
   and create an instance of the xdictionary with the language name and
   pass it to the parent class." (from http://wiki.services.openoffice.org/wiki/
   /Documentation/DevGuide/OfficeDev/Implementing_a_New_Locale - 27/01/2011)
*/

// C-standard garantees that static variables are automatically initialized to 0
static sal_uInt8 exists[0x2000];
static sal_uInt32 charArray[0x10000];

static inline void set_exists(sal_uInt32 index)
{
   exists[index>>3] |= 1 << (index & 0x07);
}

static inline void printIncludes(FILE* source_fp)
{
    fputs("/* !!!The file is generated automatically. DO NOT edit the file manually!!! */\n\n", source_fp);
    fputs("#include <sal/types.h>\n\n", source_fp);
}

static inline void printFunctions(FILE* source_fp)
{
    fputs ("\tconst sal_uInt8* getExistMark() { return existMark; }\n", source_fp);
    fputs ("\tconst sal_Int16* getIndex1() { return index1; }\n", source_fp);
    fputs ("\tconst sal_Int32* getIndex2() { return index2; }\n", source_fp);
    fputs ("\tconst sal_Int32* getLenArray() { return lenArray; }\n", source_fp);
    fputs ("\tconst sal_Unicode* getDataArea() { return dataArea; }\n", source_fp);
}

static inline void printDataArea(FILE *dictionary_fp, FILE *source_fp, vector<sal_uInt32>& lenArray)
{
    // generate main dict. data array
    fputs("static const sal_Unicode dataArea[] = {\n\t", source_fp);
    sal_Char str[1024];
    sal_uInt32 lenArrayCurr = 0;
    sal_Unicode current = 0;

    while (fgets(str, 1024, dictionary_fp)) {
        // input file is in UTF-8 encoding
        // don't convert last new line character to Ostr.
        OUString Ostr((const sal_Char *)str, strlen(str) - 1, RTL_TEXTENCODING_UTF8);
        const sal_Unicode *u = Ostr.getStr();

        const sal_Int32 len = Ostr.getLength();

        sal_Int32 i=0;
        Ostr.iterateCodePoints(&i, 1);
        if (len == i)
            continue;   // skip one character word

        if (u[0] != current) {
            OSL_ENSURE( (u[0] > current), "Dictionary file should be sorted");
            current = u[0];
            charArray[current] = lenArray.size();
        }

        lenArray.push_back(lenArrayCurr);

        set_exists(u[0]);
        // first character is stored in charArray, so start from second
        for (i = 1; i < len; i++, lenArrayCurr++) {
            set_exists(u[i]);
            fprintf(source_fp, "0x%04x, ", u[i]);
            if ((lenArrayCurr & 0x0f) == 0x0f)
                fputs("\n\t", source_fp);
        }
    }
    lenArray.push_back( lenArrayCurr ); // store last ending pointer
    charArray[current+1] = lenArray.size();
    fputs("\n};\n", source_fp);
}

static inline void printLenArray(FILE* source_fp, const vector<sal_uInt32>& lenArray)
{
    fprintf(source_fp, "static const sal_Int32 lenArray[] = {\n\t");
    fprintf(source_fp, "0x%x, ", 0); // insert one slat for skipping 0 in index2 array.
    for (size_t k = 0; k < lenArray.size(); k++)
    {
        if( !(k & 0xf) )
            fputs("\n\t", source_fp);

        fprintf(source_fp, "0x%lx, ", static_cast<long unsigned int>(lenArray[k]));
    }
    fputs("\n};\n", source_fp );
}

/* FIXME?: what happens if in every range i there is at least one charArray != 0
       => this will make index1[] = {0x00, 0x01, 0x02,... 0xfe, 0xff }
       => then in index2, the last range will be ignored incorrectly */
static inline void printIndex1(FILE *source_fp, sal_Int16 *set)
{
    fprintf (source_fp, "static const sal_Int16 index1[] = {\n\t");
    sal_Int16 count = 0;
    for (sal_Int32 i = 0; i < 0x100; i++) {
        sal_Int32 j = 0;
        while( j < 0x100 && charArray[(i<<8) + j] == 0)
            j++;

        fprintf(source_fp, "0x%02x, ", set[i] = (j < 0x100 ? count++ : 0xff));
        if ((i & 0x0f) == 0x0f)
            fputs ("\n\t", source_fp);
    }
    fputs("};\n", source_fp);
}

static inline void printIndex2(FILE *source_fp, sal_Int16 *set)
{
    fputs ("static const sal_Int32 index2[] = {\n\t", source_fp);
    sal_Int32 prev = 0;
    for (sal_Int32 i = 0; i < 0x100; i++) {
        if (set[i] != 0xff) {
            for (sal_Int32 j = 0; j < 0x100; j++) {
                sal_Int32 k = (i<<8) + j;
                if (prev != 0 )
                    while( charArray[k] == 0 && k < 0x10000 )
                        k++;

                prev = charArray[(i<<8) + j];
                fprintf(source_fp, "0x%lx, ", static_cast<long unsigned int>(k < 0x10000 ? charArray[k] + 1 : 0));
                if ((j & 0x0f) == 0x0f)
                    fputs ("\n\t", source_fp);
            }
            fputs ("\n\t", source_fp);
        }
    }
    fputs ("\n};\n", source_fp);
}

/* Generates a bitmask for the existance of sal_Unicode values in dictionary;
   it packs 8 sal_Bool values in 1 sal_uInt8 */
static inline void printExistsMask(FILE *source_fp)
{
    fprintf (source_fp, "static const sal_uInt8 existMark[] = {\n\t");
    for (unsigned int i = 0; i < 0x2000; i++)
    {
        fprintf(source_fp, "0x%02x, ", exists[i]);
        if ( (i & 0xf) == 0xf )
            fputs("\n\t", source_fp);
    }
    fputs("\n};\n", source_fp);
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *dictionary_fp, *source_fp;

    if (argc == 1 || argc > 3)
    {
        fputs("2 arguments required: dictionary_file_name source_file_name", stderr);
        exit(-1);
    }

    dictionary_fp = fopen(argv[1], "rb");   // open the source file for read;
    if (dictionary_fp == NULL)
    {
        printf("Open the dictionary source file failed.");
        return -1;
    }

    if(argc == 2)
        source_fp = stdout;
    else
    {
        // create the C source file to write
        source_fp = fopen(argv[2], "wb");
        if (source_fp == NULL) {
            fclose(dictionary_fp);
            printf("Can't create the C source file.");
            return -1;
        }
    }

    vector<sal_uInt32> lenArray;   // stores the word boundaries in DataArea
    sal_Int16 set[0x100];

    printIncludes(source_fp);
    fputs("extern \"C\" {\n", source_fp);
        printDataArea(dictionary_fp, source_fp, lenArray);
        printLenArray(source_fp, lenArray);
        printIndex1(source_fp, set);
        printIndex2(source_fp, set);
        printExistsMask(source_fp);
        printFunctions(source_fp);
    fputs("}\n", source_fp);

    fclose(dictionary_fp);
    fclose(source_fp);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
