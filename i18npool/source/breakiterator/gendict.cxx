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

using namespace ::rtl;

/* Main Procedure */

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *sfp, *cfp;

    if (argc < 3) exit(-1);

    sfp = fopen(argv[1], "rb"); // open the source file for read;
    if (sfp == NULL)
    {
        printf("Open the dictionary source file failed.");
        return -1;
    }

    // create the C source file to write
    cfp = fopen(argv[2], "wb");
    if (cfp == NULL) {
        fclose(sfp);
        printf("Can't create the C source file.");
        return -1;
    }

    fprintf(cfp, "/*\n");
    fprintf(cfp, " * Copyright(c) 1999 - 2000, Sun Microsystems, Inc.\n");
    fprintf(cfp, " * All Rights Reserved.\n");
    fprintf(cfp, " */\n\n");
    fprintf(cfp, "/* !!!The file is generated automatically. DONOT edit the file manually!!! */\n\n");
    fprintf(cfp, "#include <sal/types.h>\n\n");
    fprintf(cfp, "extern \"C\" {\n");

    sal_Int32 count, i, j;
    sal_Int32 lenArrayCurr = 0, lenArrayCount = 0, lenArrayLen = 0, *lenArray = NULL, charArray[0x10000];
    sal_Bool exist[0x10000];
    for (i = 0; i < 0x10000; i++) {
        exist[i] = sal_False;
        charArray[i] = 0;
    }

    // generate main dict. data array
    fprintf(cfp, "static const sal_Unicode dataArea[] = {");
    sal_Char str[1024];
    sal_Unicode current = 0;
    count = 0;
    while (fgets(str, 1024, sfp)) {
        // input file is in UTF-8 encoding
        // don't convert last new line character to Ostr.
        OUString Ostr((const sal_Char *)str, strlen(str) - 1, RTL_TEXTENCODING_UTF8);
        const sal_Unicode *u = Ostr.getStr();

        sal_Int32 len = Ostr.getLength();

        i=0;
        Ostr.iterateCodePoints(&i, 1);
        if (len == i) continue; // skip one character word

        if (*u != current) {
        if (*u < current)
        printf("u %x, current %x, count %d, lenArrayCount %d\n", *u, current,
                    sal::static_int_cast<int>(count), sal::static_int_cast<int>(lenArrayCount));
        current = *u;
        charArray[current] = lenArrayCount;
        }

        if (lenArrayLen <= lenArrayCount+1)
        lenArray = (sal_Int32*) realloc(lenArray, (lenArrayLen += 1000) * sizeof(sal_Int32));
        lenArray[lenArrayCount++] = lenArrayCurr;

        exist[u[0]] = sal_True;
        for (i = 1; i < len; i++) {     // start from second character,
        exist[u[i]] = sal_True;     // since the first character is captured in charArray.
        lenArrayCurr++;
        if ((count++) % 0x10 == 0)
            fprintf(cfp, "\n\t");
        fprintf(cfp, "0x%04x, ", u[i]);
        }
    }
    lenArray[lenArrayCount++] = lenArrayCurr; // store last ending pointer
    charArray[current+1] = lenArrayCount;
    fprintf(cfp, "\n};\n");

    // generate lenArray
    fprintf(cfp, "static const sal_Int32 lenArray[] = {\n\t");
    count = 1;
    fprintf(cfp, "0x%x, ", 0); // insert one slat for skipping 0 in index2 array.
    for (i = 0; i < lenArrayCount; i++) {
        fprintf(cfp, "0x%lx, ", static_cast<long unsigned int>(lenArray[i]));
        if (count == 0xf) {
        count = 0;
        fprintf(cfp, "\n\t");
        } else count++;
    }
    fprintf(cfp, "\n};\n");

    free(lenArray);

    // generate index1 array
    fprintf (cfp, "static const sal_Int16 index1[] = {\n\t");
    sal_Int16 set[0x100];
    count = 0;
    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 0x100; j++)
        if (charArray[(i*0x100) + j] != 0)
            break;

        fprintf(cfp, "0x%02x, ", set[i] = (j < 0x100 ? sal::static_int_cast<sal_Int16>(count++) : 0xff));
        if ((i+1) % 0x10 == 0)
        fprintf (cfp, "\n\t");
    }
    fprintf (cfp, "};\n");

    // generate index2 array
    fprintf (cfp, "static const sal_Int32 index2[] = {\n\t");
    sal_Int32 prev = 0;
    for (i = 0; i < 0x100; i++) {
        if (set[i] != 0xff) {
        for (j = 0; j < 0x100; j++) {
            sal_Int32 k = (i*0x100) + j;
            if (prev != 0 && charArray[k] == 0) {
            for (k++; k < 0x10000; k++)
                if (charArray[k] != 0)
                break;
            }
            prev = charArray[(i*0x100) + j];
            fprintf(
                cfp, "0x%lx, ",
                sal::static_int_cast< unsigned long >(
                    k < 0x10000 ? charArray[k] + 1 : 0));
            if ((j+1) % 0x10 == 0)
            fprintf (cfp, "\n\t");
        }
        fprintf (cfp, "\n\t");
        }
    }
    fprintf (cfp, "\n};\n");

    // generate existMark array
    count = 0;
    fprintf (cfp, "static const sal_uInt8 existMark[] = {\n\t");
    for (i = 0; i < 0x1FFF; i++) {
        sal_uInt8 bit = 0;
        for (j = 0; j < 8; j++)
        if (exist[i * 8 + j])
            bit |= 1 << j;
        fprintf(cfp, "0x%02x, ", bit);
        if (count == 0xf) {
        count = 0;
        fprintf(cfp, "\n\t");
        } else count++;
    }
    fprintf (cfp, "\n};\n");

    // create function to return arrays
    fprintf (cfp, "\tconst sal_uInt8* getExistMark() { return existMark; }\n");
    fprintf (cfp, "\tconst sal_Int16* getIndex1() { return index1; }\n");
    fprintf (cfp, "\tconst sal_Int32* getIndex2() { return index2; }\n");
    fprintf (cfp, "\tconst sal_Int32* getLenArray() { return lenArray; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getDataArea() { return dataArea; }\n");
    fprintf (cfp, "}\n");

    fclose(sfp);
    fclose(cfp);

    return 0;
}   // End of main

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
