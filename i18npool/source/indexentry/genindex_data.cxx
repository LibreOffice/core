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
#include <rtl/ustring.hxx>

#define MAX_ADDRESS 0x30000
#define MAX_INDEX MAX_ADDRESS/0x100

/* Main Procedure */

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *fp;

    if (argc < 4) exit(-1);

    fp = fopen(argv[1], "rb");  // open the source file for read;
    if (fp == nullptr) {
        fprintf(stderr, "Opening the rule source file %s for reading failed: %s\n", argv[1], strerror(errno));
        exit(1);
    }


    sal_Int32 i, j, k;
    sal_Int32 address[MAX_ADDRESS];
    for (i=0; i<MAX_ADDRESS; i++) address[i]=-1;
    OUString sep=OUString('|');
    OUString result=sep;
    sal_Int32 max=0;

    sal_Char str[1024];
    while (fgets(str, 1024, fp)) {
        // don't convert last new line character to Ostr.
        sal_Int32 len = strlen(str) - 1;
        // skip comment line
        if (len == 0 || str[0] == '#')
            continue;

        // input file is in UTF-8 encoding
        OUString Ostr = OUString(str, len, RTL_TEXTENCODING_UTF8);
        len = Ostr.getLength();
        if (len == 0)
            continue; // skip empty line.

        sal_Int32 nPos=0;
        sal_uInt32 nChar = Ostr.iterateCodePoints(&nPos, 2);
        if (nChar >= MAX_ADDRESS) {
            printf("Code point 0x%lx exceeds MAX_ADDRESS 0x%x, Please increase MAX_ADDRESS", static_cast<long unsigned int>(nChar), MAX_ADDRESS);
            exit(1);
        }
        OUString key=Ostr.copy(nPos)+sep;
        sal_Int32 idx = result.indexOf(key);
        if (key.getLength() > max) max=key.getLength();
        if (idx >= 0) {
            address[nChar]=idx;
        } else {
            address[nChar]=result.getLength();
            result+=key;
        }
    }
    fclose(fp);

    fp = fopen(argv[2], "wb");
    if (fp == nullptr) {
        fprintf(stderr, "Opening %s for writing failed: %s\n", argv[2], strerror(errno));
        exit(1);
    }

    fprintf(fp, "/*\n");
    fprintf(fp, " * Copyright(c) 1999 - 2006, Sun Microsystems, Inc.\n");
    fprintf(fp, " * All Rights Reserved.\n");
    fprintf(fp, " */\n\n");
    fprintf(fp, "/* !!!The file is generated automatically. DONOT edit the file manually!!! */\n\n");
    fprintf(fp, "#include <sal/types.h>\n");
    fprintf(fp, "\nextern \"C\" {\n");

    sal_Int32 index[MAX_INDEX];
    sal_Int32 max_index=0;
    for (i=k=0; i<MAX_INDEX; i++) {
        index[i] = 0xFFFF;
        for (j=0; j<0x100; j++) {
            if (address[i*0x100+j] >=0) {
                max_index=i;
                index[i]=0x100*k++;
                break;
            }
        }
    }

    fprintf(fp, "\nstatic const sal_uInt16 idx1[] = {");
    for (i = k = 0; i <= max_index;  i++) {
        if (k++ % 16 == 0) fprintf(fp, "\n\t");
        fprintf(
            fp, "0x%04lx, ", sal::static_int_cast< unsigned long >(index[i]));
    }
    fprintf(fp, "\n};\n\n");

    sal_Int32 len=result.getLength();
    const sal_Unicode *ustr=result.getStr();
    fprintf(fp, "\nstatic const sal_uInt16 idx2[] = {");
    for (i = k = 0; i <= max_index; i++) {
        if (index[i] != 0xFFFF) {
            for (j = 0; j<0x100; j++) {
                if (k++ % 16 == 0) fprintf(fp, "\n\t");
                sal_Int32 ad=address[i*0x100+j];
                fprintf(
                    fp, "0x%04lx, ",
                    sal::static_int_cast< unsigned long >(
                        ad == -1 ? 0 : max == 2 ? ustr[ad] : ad));
            }
            fprintf(fp, "\n\t");
        }
    }
    fprintf(fp, "\n};\n\n");

    if (max == 2) {
        fprintf(fp, "\nstatic const sal_uInt16 *idx3 = NULL;\n\n");
    } else {
        fprintf(fp, "\nstatic const sal_uInt16 idx3[] = {");
        for (i = k = 0; i < len;  i++) {
            if (k++ % 16 == 0) fprintf(fp, "\n\t");
            fprintf(fp, "0x%04x, ", (sep.toChar() == ustr[i]) ? 0 : ustr[i]);
        }
        fprintf(fp, "\n};\n\n");
    }

    fprintf(fp, "const sal_uInt16** get_%s(sal_Int16 &max_index)\n{\n\tstatic const sal_uInt16 *idx[]={idx1, idx2, idx3};\n\tmax_index=0x%x;\n\treturn idx;\n}\n\n", argv[3], static_cast<unsigned int>(max_index));
    fprintf (fp, "}\n");

    fclose(fp);
    return 0;
}   // End of main

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
