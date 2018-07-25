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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sal/main.h>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <vector>

using std::vector;


// For iOS, where we must strive for a minimal executable size, we
// keep the data produced by this utility not as large const tables in
// source code but instead as separate data files, to be bundled with
// an app, and mmapped in at run time.

// To test this easier on a desktop OS, just make sure
// DICT_JA_ZH_IN_DATAFILE is defined when building i18npool.

#ifdef DICT_JA_ZH_IN_DATAFILE
static sal_Int64 dataAreaOffset = 0;
static sal_Int64 lenArrayOffset = 0;
static sal_Int64 index1Offset = 0;
static sal_Int64 index2Offset = 0;
static sal_Int64 existMarkOffset = 0;
#endif

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
   pass it to the parent class." (from http://wiki.openoffice.org/wiki/
   /Documentation/DevGuide/OfficeDev/Implementing_a_New_Locale - 27/01/2011)
*/

// C-standard guarantees that static variables are automatically initialized to 0
static sal_uInt8 exists[0x2000];
static sal_uInt32 charArray[0x10000];

static inline void set_exists(sal_uInt32 index)
{
   exists[index>>3] |= 1 << (index & 0x07);
}

static inline void printIncludes(FILE* source_fp)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("/* !!!The file is generated automatically. DO NOT edit the file manually!!! */\n\n", source_fp);
    fputs("#include <sal/types.h>\n\n", source_fp);
#else
    (void) source_fp;
#endif
}

static inline void printFunctions(FILE* source_fp, const char *lang)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs ("#ifndef DISABLE_DYNLOADING\n", source_fp);
    fputs ("SAL_DLLPUBLIC_EXPORT const sal_uInt8* getExistMark() { return existMark; }\n", source_fp);
    fputs ("SAL_DLLPUBLIC_EXPORT const sal_Int16* getIndex1() { return index1; }\n", source_fp);
    fputs ("SAL_DLLPUBLIC_EXPORT const sal_Int32* getIndex2() { return index2; }\n", source_fp);
    fputs ("SAL_DLLPUBLIC_EXPORT const sal_Int32* getLenArray() { return lenArray; }\n", source_fp);
    fputs ("SAL_DLLPUBLIC_EXPORT const sal_Unicode* getDataArea() { return dataArea; }\n", source_fp);
    fputs ("#else\n", source_fp);
    fprintf (source_fp, "SAL_DLLPUBLIC_EXPORT const sal_uInt8* getExistMark_%s() { return existMark; }\n", lang);
    fprintf (source_fp, "SAL_DLLPUBLIC_EXPORT const sal_Int16* getIndex1_%s() { return index1; }\n", lang);
    fprintf (source_fp, "SAL_DLLPUBLIC_EXPORT const sal_Int32* getIndex2_%s() { return index2; }\n", lang);
    fprintf (source_fp, "SAL_DLLPUBLIC_EXPORT const sal_Int32* getLenArray_%s() { return lenArray; }\n", lang);
    fprintf (source_fp, "SAL_DLLPUBLIC_EXPORT const sal_Unicode* getDataArea_%s() { return dataArea; }\n", lang);
    fputs ("#endif\n", source_fp);
#else
    (void) source_fp;
    (void) lang;
#endif
}

static inline void printDataArea(FILE *dictionary_fp, FILE *source_fp, vector<sal_uInt32>& lenArray)
{
    // generate main dict. data array
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("static const sal_Unicode dataArea[] = {\n\t", source_fp);
#else
    dataAreaOffset = ftell(source_fp);
#endif
    sal_Char str[1024];
    sal_uInt32 lenArrayCurr = 0;
    sal_Unicode current = 0;

    while (fgets(str, 1024, dictionary_fp)) {
        // input file is in UTF-8 encoding
        // don't convert last new line character to Ostr.
        OUString Ostr(str, strlen(str) - 1, RTL_TEXTENCODING_UTF8);

        const sal_Int32 len = Ostr.getLength();

        sal_Int32 i=0;
        Ostr.iterateCodePoints(&i);
        if (len == i)
            continue;   // skip one character word

        if (Ostr[0] != current) {
            OSL_ENSURE( (Ostr[0] > current), "Dictionary file should be sorted");
            current = Ostr[0];
            charArray[current] = lenArray.size();
        }

        lenArray.push_back(lenArrayCurr);

        set_exists(Ostr[0]);
        // first character is stored in charArray, so start from second
        for (i = 1; i < len; i++, lenArrayCurr++) {
            set_exists(Ostr[i]);
#ifndef DICT_JA_ZH_IN_DATAFILE
            fprintf(source_fp, "0x%04x, ", Ostr[i]);
            if ((lenArrayCurr & 0x0f) == 0x0f)
                fputs("\n\t", source_fp);
#else
            sal_Unicode x = Ostr[i];
            fwrite(&x, sizeof(Ostr[i]), 1, source_fp);
#endif
        }
    }
    charArray[current+1] = lenArray.size();
    lenArray.push_back( lenArrayCurr ); // store last ending pointer
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("\n};\n", source_fp);
#endif
}

static inline void printLenArray(FILE* source_fp, const vector<sal_uInt32>& lenArray)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fprintf(source_fp, "static const sal_Int32 lenArray[] = {\n\t");
    fprintf(source_fp, "0x%x, ", 0); // insert one slat for skipping 0 in index2 array.
#else
    lenArrayOffset = ftell(source_fp);
    sal_uInt32 zero(0);
    fwrite(&zero, sizeof(zero), 1, source_fp);
#endif
    for (size_t k = 0; k < lenArray.size(); k++)
    {
        if( !(k & 0xf) )
            fputs("\n\t", source_fp);

#ifndef DICT_JA_ZH_IN_DATAFILE
        fprintf(source_fp, "0x%" SAL_PRIxUINT32 ", ", lenArray[k]);
#else
        fwrite(&lenArray[k], sizeof(lenArray[k]), 1, source_fp);
#endif
    }

#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("\n};\n", source_fp );
#endif
}

/* FIXME?: what happens if in every range i there is at least one charArray != 0
       => this will make index1[] = {0x00, 0x01, 0x02,... 0xfe, 0xff }
       => then in index2, the last range will be ignored incorrectly */
static inline void printIndex1(FILE *source_fp, sal_Int16 *set)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fprintf (source_fp, "static const sal_Int16 index1[] = {\n\t");
#else
    index1Offset = ftell(source_fp);
#endif

    sal_Int16 count = 0;
    for (sal_Int32 i = 0; i < 0x100; i++) {
        sal_Int32 j = 0;
        while( j < 0x100 && charArray[(i<<8) + j] == 0)
            j++;

        set[i] = (j < 0x100 ? count++ : 0xff);
#ifndef DICT_JA_ZH_IN_DATAFILE
        fprintf(source_fp, "0x%02x, ", set[i]);
        if ((i & 0x0f) == 0x0f)
            fputs ("\n\t", source_fp);
#else
        fwrite(&set[i], sizeof(set[i]), 1, source_fp);
#endif
    }

#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("};\n", source_fp);
#endif
}

static inline void printIndex2(FILE *source_fp, sal_Int16 const *set)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs ("static const sal_Int32 index2[] = {\n\t", source_fp);
#else
    index2Offset = ftell(source_fp);
#endif
    sal_Int32 prev = 0;
    for (sal_Int32 i = 0; i < 0x100; i++) {
        if (set[i] != 0xff) {
            for (sal_Int32 j = 0; j < 0x100; j++) {
                sal_Int32 k = (i<<8) + j;
                if (prev != 0 )
                    while( k < 0x10000 && charArray[k] == 0 )
                        k++;

                prev = charArray[(i<<8) + j];
#ifndef DICT_JA_ZH_IN_DATAFILE
                fprintf(source_fp, "0x%lx, ", static_cast<long unsigned int>(k < 0x10000 ? charArray[k] + 1 : 0));
                if ((j & 0x0f) == 0x0f)
                    fputs ("\n\t", source_fp);
#else
                sal_uInt32 n = (k < 0x10000 ? charArray[k] + 1 : 0);
                fwrite(&n, sizeof(n), 1, source_fp);
#endif
            }
#ifndef DICT_JA_ZH_IN_DATAFILE
            fputs ("\n\t", source_fp);
#endif
        }
    }
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs ("\n};\n", source_fp);
#endif
}

/* Generates a bitmask for the existence of sal_Unicode values in dictionary;
   it packs 8 sal_Bool values in 1 sal_uInt8 */
static inline void printExistsMask(FILE *source_fp)
{
#ifndef DICT_JA_ZH_IN_DATAFILE
    fprintf (source_fp, "static const sal_uInt8 existMark[] = {\n\t");
#else
    existMarkOffset = ftell(source_fp);
#endif
    for (unsigned int i = 0; i < 0x2000; i++)
    {
#ifndef DICT_JA_ZH_IN_DATAFILE
        fprintf(source_fp, "0x%02x, ", exists[i]);
        if ( (i & 0xf) == 0xf )
            fputs("\n\t", source_fp);
#else
        fwrite(&exists[i], sizeof(exists[i]), 1, source_fp);
#endif
    }

#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("\n};\n", source_fp);
#endif
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *dictionary_fp, *source_fp;

    if (argc == 1 || argc > 4)
    {
        fputs("3 arguments required: dictionary_file_name source_file_name language_code", stderr);
        exit(-1);
    }

    dictionary_fp = fopen(argv[1], "rb");   // open the source file for read;
    if (dictionary_fp == nullptr)
    {
        fprintf(stderr, "Opening the dictionary source file %s for reading failed: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    if(argc == 2)
        source_fp = stdout;
    else
    {
        // create the C source file to write
        source_fp = fopen(argv[2], "wb");
        if (source_fp == nullptr) {
            fclose(dictionary_fp);
            fprintf(stderr, "Opening %s for writing failed: %s\n", argv[2], strerror(errno));
            exit(1);
        }
    }

    vector<sal_uInt32> lenArray;   // stores the word boundaries in DataArea
    sal_Int16 set[0x100];

    printIncludes(source_fp);
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("extern \"C\" {\n", source_fp);
#endif
    printDataArea(dictionary_fp, source_fp, lenArray);
    printLenArray(source_fp, lenArray);
    printIndex1(source_fp, set);
    printIndex2(source_fp, set);
    printExistsMask(source_fp);
    printFunctions(source_fp, argv[3]);
#ifndef DICT_JA_ZH_IN_DATAFILE
    fputs("}\n", source_fp);
#else
    // Put pointers to the tables at the end of the file...
    fwrite(&dataAreaOffset, sizeof(dataAreaOffset), 1, source_fp);
    fwrite(&lenArrayOffset, sizeof(lenArrayOffset), 1, source_fp);
    fwrite(&index1Offset, sizeof(index1Offset), 1, source_fp);
    fwrite(&index2Offset, sizeof(index2Offset), 1, source_fp);
    fwrite(&existMarkOffset, sizeof(existMarkOffset), 1, source_fp);
#endif

    fclose(dictionary_fp);
    fclose(source_fp);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
