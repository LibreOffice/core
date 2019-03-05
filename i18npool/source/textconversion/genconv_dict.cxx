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

#include <vector>

static void make_hhc_char(FILE *sfp, FILE *cfp);
static void make_stc_char(FILE *sfp, FILE *cfp);
static void make_stc_word(FILE *sfp, FILE *cfp);

/* Main Procedure */

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *sfp, *cfp;

    if (argc < 4) exit(-1);


    sfp = fopen(argv[2], "rb"); // open the source file for read;
    if (sfp == nullptr)
    {
        fprintf(stderr, "Opening the dictionary source file %s for reading failed: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    // create the C source file to write
    cfp = fopen(argv[3], "wb");
    if (cfp == nullptr) {
        fclose(sfp);
        fprintf(stderr, "Opening %s for writing failed: %s\n", argv[3], strerror(errno));
        exit(1);
    }

    fprintf(cfp, "/*\n");
    fprintf(cfp, " * Copyright(c) 1999 - 2000, Sun Microsystems, Inc.\n");
    fprintf(cfp, " * All Rights Reserved.\n");
    fprintf(cfp, " */\n\n");
    fprintf(cfp, "/* !!!The file is generated automatically. DONOT edit the file manually!!! */\n\n");
    fprintf(cfp, "#include <sal/types.h>\n");
    fprintf(cfp, "#include <textconversion.hxx>\n");
    fprintf(cfp, "\nextern \"C\" {\n");

    if (strcmp(argv[1], "hhc_char") == 0)
        make_hhc_char(sfp, cfp);
    else if (strcmp(argv[1], "stc_char") == 0)
        make_stc_char(sfp, cfp);
    else if (strcmp(argv[1], "stc_word") == 0)
        make_stc_word(sfp, cfp);

    fprintf (cfp, "}\n");

    fclose(sfp);
    fclose(cfp);

    return 0;
} // end of main

// Hangul/Hanja character conversion
void make_hhc_char(FILE *sfp, FILE *cfp)
{
    sal_Int32 count, address, i, j, k;
    sal_Unicode Hanja2HangulData[0x10000];
    for (i = 0; i < 0x10000; i++) {
        Hanja2HangulData[i] = 0;
    }
    sal_uInt16 Hangul2HanjaData[10000][3];

    // generate main dict. data array
    fprintf(cfp, "\nstatic const sal_Unicode Hangul2HanjaData[] = {");

    sal_Char Cstr[1024];
    count = 0;
    address = 0;
    while (fgets(Cstr, 1024, sfp)) {
        // input file is in UTF-8 encoding (Hangul:Hanja)
        // don't convert last new line character to Ostr.
        OUString Ostr(Cstr, strlen(Cstr) - 1, RTL_TEXTENCODING_UTF8);
        sal_Int32  len = Ostr.getLength();

        Hangul2HanjaData[count][0] = Ostr[0];
        Hangul2HanjaData[count][1] = sal::static_int_cast<sal_uInt16>( address );
        Hangul2HanjaData[count][2] = sal::static_int_cast<sal_uInt16>( len - 2 );
        count++;

        for (i = 2; i < len; i++) {
            Hanja2HangulData[Ostr[i]] = Ostr[0];
            if (address++ % 16 == 0)
                fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", Ostr[i]);
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const i18npool::Hangul_Index Hangul2HanjaIndex[] = {\n");
    for (i = 0; i < count; i++)
        fprintf(cfp, "\t{ 0x%04x, 0x%04x, 0x%02x },\n",
                        Hangul2HanjaData[i][0],
                        Hangul2HanjaData[i][1],
                        Hangul2HanjaData[i][2]);
    fprintf(cfp, "};\n");

    fprintf(cfp, "\nstatic const sal_uInt16 Hanja2HangulIndex[] = {");

    address=0;
    for (i = 0; i < 0x10; i++) {
        fprintf(cfp, "\n\t");
        for (j = 0; j < 0x10; j++) {
            for (k = 0; k < 0x100; k++) {
                if (Hanja2HangulData[((i*0x10)+j)*0x100+k] != 0)
                    break;
            }
            fprintf(
                cfp, "0x%04lx, ",
                sal::static_int_cast< unsigned long >(
                    k < 0x100 ? (address++)*0x100 : 0xFFFF));
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_Unicode Hanja2HangulData[] = {");

    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 0x100; j++) {
            if (Hanja2HangulData[i*0x100+j] != 0)
                break;
        }
        if (j < 0x100) {
            for (j = 0; j < 0x10; j++) {
                fprintf(cfp, "\n\t");
                for (k = 0; k < 0x10; k++) {
                    sal_Unicode c = Hanja2HangulData[((i*0x10+j)*0x10)+k];
                    fprintf(cfp, "0x%04x, ", c ? c : 0xFFFF);
                }
            }
        }
    }
    fprintf(cfp, "\n};\n");

    // create function to return arrays
    fprintf (cfp, "\tconst sal_Unicode* getHangul2HanjaData() { return Hangul2HanjaData; }\n");
    fprintf (cfp, "\tconst i18npool::Hangul_Index* getHangul2HanjaIndex() { return Hangul2HanjaIndex; }\n");
    fprintf (cfp, "\tsal_Int16 getHangul2HanjaIndexCount() { return sizeof(Hangul2HanjaIndex) / sizeof(i18npool::Hangul_Index); }\n");
    fprintf (cfp, "\tconst sal_uInt16* getHanja2HangulIndex() { return Hanja2HangulIndex; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getHanja2HangulData() { return Hanja2HangulData; }\n");
}

// Simplified/Traditional Chinese character conversion
void make_stc_char(FILE *sfp, FILE *cfp)
{
    sal_Int32 address, i, j, k;
    sal_Unicode SChinese2TChineseData[0x10000];
    sal_Unicode SChinese2VChineseData[0x10000];
    sal_Unicode TChinese2SChineseData[0x10000];
    for (i = 0; i < 0x10000; i++) {
        SChinese2TChineseData[i] = 0;
        SChinese2VChineseData[i] = 0;
        TChinese2SChineseData[i] = 0;
    }

    sal_Char Cstr[1024];
    while (fgets(Cstr, 1024, sfp)) {
        // input file is in UTF-8 encoding (SChinese:TChinese)
        // don't convert last new line character to Ostr.
        OUString Ostr(Cstr, strlen(Cstr) - 1, RTL_TEXTENCODING_UTF8);
        sal_Int32  len = Ostr.getLength();
        if (Ostr[1] == 'v')
            SChinese2VChineseData[Ostr[0]] = Ostr[2];
        else {
            SChinese2TChineseData[Ostr[0]] = Ostr[2];
            if (SChinese2VChineseData[Ostr[0]] == 0)
                SChinese2VChineseData[Ostr[0]] = Ostr[2];
        }
        for (i = 2; i < len; i++)
            TChinese2SChineseData[Ostr[i]] = Ostr[0];
    }

    fprintf(cfp, "\nstatic const sal_uInt16 STC_CharIndex_S2T[] = {");

    address=0;
    for (i = 0; i < 0x10; i++) {
        fprintf(cfp, "\n\t");
        for (j = 0; j < 0x10; j++) {
            for (k = 0; k < 0x100; k++) {
                if (SChinese2TChineseData[((i*0x10)+j)*0x100+k] != 0)
                    break;
            }
            fprintf(
                cfp, "0x%04lx, ",
                sal::static_int_cast< unsigned long >(
                    k < 0x100 ? (address++)*0x100 : 0xFFFF));
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_Unicode STC_CharData_S2T[] = {");

    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 0x100; j++) {
            if (SChinese2TChineseData[i*0x100+j] != 0)
                break;
        }
        if (j < 0x100) {
            for (j = 0; j < 0x10; j++) {
                fprintf(cfp, "\n\t");
                for (k = 0; k < 0x10; k++) {
                    sal_Unicode c = SChinese2TChineseData[((i*0x10+j)*0x10)+k];
                    fprintf(cfp, "0x%04x, ", c ? c : 0xFFFF);
                }
            }
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_uInt16 STC_CharIndex_S2V[] = {");

    address=0;
    for (i = 0; i < 0x10; i++) {
        fprintf(cfp, "\n\t");
        for (j = 0; j < 0x10; j++) {
            for (k = 0; k < 0x100; k++) {
                if (SChinese2VChineseData[((i*0x10)+j)*0x100+k] != 0)
                    break;
            }
            fprintf(
                cfp, "0x%04lx, ",
                sal::static_int_cast< unsigned long >(
                    k < 0x100 ? (address++)*0x100 : 0xFFFF));
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_Unicode STC_CharData_S2V[] = {");

    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 0x100; j++) {
            if (SChinese2VChineseData[i*0x100+j] != 0)
                break;
        }
        if (j < 0x100) {
            for (j = 0; j < 0x10; j++) {
                fprintf(cfp, "\n\t");
                for (k = 0; k < 0x10; k++) {
                    sal_Unicode c = SChinese2VChineseData[((i*0x10+j)*0x10)+k];
                    fprintf(cfp, "0x%04x, ", c ? c : 0xFFFF);
                }
            }
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_uInt16 STC_CharIndex_T2S[] = {");

    address=0;
    for (i = 0; i < 0x10; i++) {
        fprintf(cfp, "\n\t");
        for (j = 0; j < 0x10; j++) {
            for (k = 0; k < 0x100; k++) {
                if (TChinese2SChineseData[((i*0x10)+j)*0x100+k] != 0)
                    break;
            }
            fprintf(
                cfp, "0x%04lx, ",
                sal::static_int_cast< unsigned long >(
                    k < 0x100 ? (address++)*0x100 : 0xFFFF));
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const sal_Unicode STC_CharData_T2S[] = {");

    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 0x100; j++) {
            if (TChinese2SChineseData[i*0x100+j] != 0)
                break;
        }
        if (j < 0x100) {
            for (j = 0; j < 0x10; j++) {
                fprintf(cfp, "\n\t");
                for (k = 0; k < 0x10; k++) {
                    sal_Unicode c = TChinese2SChineseData[((i*0x10+j)*0x10)+k];
                    fprintf(cfp, "0x%04x, ", c ? c : 0xFFFF);
                }
            }
        }
    }
    fprintf(cfp, "\n};\n");

    // create function to return arrays
    fprintf (cfp, "\tconst sal_uInt16* getSTC_CharIndex_S2T() { return STC_CharIndex_S2T; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getSTC_CharData_S2T() { return STC_CharData_S2T; }\n");
    fprintf (cfp, "\tconst sal_uInt16* getSTC_CharIndex_S2V() { return STC_CharIndex_S2V; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getSTC_CharData_S2V() { return STC_CharData_S2V; }\n");
    fprintf (cfp, "\tconst sal_uInt16* getSTC_CharIndex_T2S() { return STC_CharIndex_T2S; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getSTC_CharData_T2S() { return STC_CharData_T2S; }\n");
}


struct Index {
    sal_uInt16 address;
    sal_Int32 len;
    sal_Unicode *data;
};

extern "C" {
static int Index_comp(const void* s1, const void* s2)
{
    Index const *p1 = static_cast<Index const *>(s1), *p2 = static_cast<Index const *>(s2);
    int result = p1->len - p2->len;
    for (int i = 0; result == 0 && i < p1->len; i++)
        result = *(p1->data+i) - *(p2->data+i);
    return result;
}
}

// Simplified/Traditional Chinese word conversion
void make_stc_word(FILE *sfp, FILE *cfp)
{
    sal_Int32 count, i, length;
    sal_Unicode STC_WordData[0x10000];
    std::vector<Index> STC_WordEntry_S2T(0x10000);
    std::vector<Index> STC_WordEntry_T2S(0x10000);
    sal_Int32 count_S2T = 0, count_T2S = 0;
    sal_Int32 line = 0, char_total = 0;
    sal_Char Cstr[1024];

    while (fgets(Cstr, 1024, sfp)) {
        // input file is in UTF-8 encoding (SChinese:TChinese)
        // don't convert last new line character to Ostr.
        OUString Ostr(Cstr, strlen(Cstr) - 1, RTL_TEXTENCODING_UTF8);
        sal_Int32  len = Ostr.getLength();
        if (char_total + len + 1 > 0xFFFF) {
            fprintf(stderr, "Word Dictionary stc_word.dic is too big (line %ld)", sal::static_int_cast< long >(line));
            return;
        }
        sal_Int32 sep=-1, eq=-1, gt=-1, lt=-1;
        if (((sep = eq = Ostr.indexOf('=')) > 0) ||
            ((sep = gt = Ostr.indexOf('>')) > 0) ||
            ((sep = lt = Ostr.indexOf('<')) > 0)) {

            if (eq > 0 || gt > 0) {
                STC_WordEntry_S2T[count_S2T].address = sal::static_int_cast<sal_uInt16>( char_total );
                STC_WordEntry_S2T[count_S2T].len = sep;
                STC_WordEntry_S2T[count_S2T++].data = &STC_WordData[char_total];
            }
            if (eq > 0 || lt > 0) {
                STC_WordEntry_T2S[count_T2S].address = sal::static_int_cast<sal_uInt16>( char_total + sep + 1 );
                STC_WordEntry_T2S[count_T2S].len = len - sep - 1;
                STC_WordEntry_T2S[count_T2S++].data = &STC_WordData[char_total + sep + 1];
            }
            for (i = 0; i < len; i++)
                STC_WordData[char_total++] = (i == sep) ? 0 : Ostr[i];
            STC_WordData[char_total++] = 0;
        } else {
            fprintf(stderr, "Invalid entry in stc_word.dic (line %ld)", sal::static_int_cast< long >(line));
            return;
        }
        line++;
    }

    if (char_total > 0) {
        fprintf(cfp, "\nstatic const sal_Unicode STC_WordData[] = {");
        for (i = 0; i < char_total; i++) {
            if (i % 32 == 0) fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", STC_WordData[i]);
        }
        fprintf(cfp, "\n};\n");

        fprintf(cfp, "\nstatic sal_Int32 STC_WordData_Count = %ld;\n", sal::static_int_cast< long >(char_total));

        // create function to return arrays
        fprintf (cfp, "\tconst sal_Unicode* getSTC_WordData(sal_Int32& count) { count = STC_WordData_Count; return STC_WordData; }\n");
    } else {
        fprintf (cfp, "\tconst sal_Unicode* getSTC_WordData(sal_Int32& count) { count = 0; return NULL; }\n");
    }

    sal_uInt16 STC_WordIndex[0x100];

    if (count_S2T > 0) {
        qsort(&STC_WordEntry_S2T[0], count_S2T, sizeof(Index), Index_comp);

        fprintf(cfp, "\nstatic const sal_uInt16 STC_WordEntry_S2T[] = {");
        count = 0;
        length = 0;
        for (i = 0; i < count_S2T; i++) {
            if (i % 32 == 0) fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", STC_WordEntry_S2T[i].address);
            if (STC_WordEntry_S2T[i].len != length) {
                length = STC_WordEntry_S2T[i].len;
                while (count <= length)
                    STC_WordIndex[count++] = sal::static_int_cast<sal_uInt16>(i);
            }
        }
        fprintf(cfp, "\n};\n");
        STC_WordIndex[count++] = sal::static_int_cast<sal_uInt16>(i);

        fprintf(cfp, "\nstatic const sal_uInt16 STC_WordIndex_S2T[] = {");
        for (i = 0; i < count; i++) {
            if (i % 16 == 0) fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", STC_WordIndex[i]);
        }
        fprintf(cfp, "\n};\n");

        fprintf(cfp, "\nstatic sal_Int32 STC_WordIndex_S2T_Count = %ld;\n", sal::static_int_cast< long >(length));
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordEntry_S2T() { return STC_WordEntry_S2T; }\n");
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordIndex_S2T(sal_Int32& count) { count = STC_WordIndex_S2T_Count; return STC_WordIndex_S2T; }\n");
    } else {
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordEntry_S2T() { return NULL; }\n");
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordIndex_S2T(sal_Int32& count) { count = 0; return NULL; }\n");
    }

    if (count_T2S > 0) {
        qsort(&STC_WordEntry_T2S[0], count_T2S, sizeof(Index), Index_comp);

        fprintf(cfp, "\nstatic const sal_uInt16 STC_WordEntry_T2S[] = {");
        count = 0;
        length = 0;
        for (i = 0; i < count_T2S; i++) {
            if (i % 32 == 0) fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", STC_WordEntry_T2S[i].address);
            if (STC_WordEntry_T2S[i].len != length) {
                length = STC_WordEntry_T2S[i].len;
                while (count <= length)
                    STC_WordIndex[count++] = sal::static_int_cast<sal_uInt16>(i);
            }
        }
        STC_WordIndex[count++] = sal::static_int_cast<sal_uInt16>(i);
        fprintf(cfp, "\n};\n");

        fprintf(cfp, "\nstatic const sal_uInt16 STC_WordIndex_T2S[] = {");
        for (i = 0; i < count; i++) {
            if (i % 16 == 0) fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ",  STC_WordIndex[i]);
        }
        fprintf(cfp, "\n};\n");

        fprintf(cfp, "\nstatic sal_Int32 STC_WordIndex_T2S_Count = %ld;\n\n", sal::static_int_cast< long >(length));
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordEntry_T2S() { return STC_WordEntry_T2S; }\n");
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordIndex_T2S(sal_Int32& count) { count = STC_WordIndex_T2S_Count; return STC_WordIndex_T2S; }\n");
    } else {
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordEntry_T2S() { return NULL; }\n");
        fprintf (cfp, "\tconst sal_uInt16* getSTC_WordIndex_T2S(sal_Int32& count) { count = 0; return NULL; }\n");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
