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

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sal/main.h>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>

#include <unicode/tblcoll.h>

/* Main Procedure */

void data_write(char* file, char* name, sal_uInt8 *data, sal_Int32 len)
{
    FILE *fp = fopen(file, "wb");
    if (fp == nullptr) {
        fprintf(stderr, "Opening %s for writing failed: %s\n", file, strerror(errno));
        exit(1);
    }

    fprintf(fp, "/*\n");
    fprintf(fp, " * Copyright(c) 1999 - 2000, Sun Microsystems, Inc.\n");
    fprintf(fp, " * All Rights Reserved.\n");
    fprintf(fp, " */\n\n");
    fprintf(fp, "/* !!!The file is generated automatically. DONOT edit the file manually!!! */\n\n");
    fprintf(fp, "#include <sal/types.h>\n");
    fprintf(fp, "\nextern \"C\" {\n");

    // generate main dict. data array
    fprintf(fp, "\nstatic const sal_uInt8 %s[] = {", name);

    sal_Int32 count = 0;
    for (sal_Int32 i = 0; i < len; i++) {

        if (count++ % 16 == 0)
            fprintf(fp, "\n\t");

        fprintf(fp, "0x%04x, ", data[i]);
    }
    fprintf(fp, "\n};\n\n");

    fprintf(fp, "#ifndef DISABLE_DYNLOADING\n");
    fprintf(fp, "SAL_DLLPUBLIC_EXPORT const sal_uInt8* get_%s() { return %s; }\n", name, name);
    fprintf(fp, "SAL_DLLPUBLIC_EXPORT size_t get_%s_length() { return sizeof(%s); }\n", name, name);
    fprintf(fp, "#else\n");
    fprintf(fp, "SAL_DLLPUBLIC_EXPORT const sal_uInt8* get_collator_data_%s() { return %s; }\n", name, name);
    fprintf(fp, "SAL_DLLPUBLIC_EXPORT size_t get_collator_data_%s_length() { return sizeof(%s); }\n", name, name);
    fprintf(fp, "#endif\n");
    fprintf(fp, "\n");
    fprintf (fp, "}\n");

    fclose(fp);

}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *fp;

    if (argc < 4) exit(-1);

    fp = fopen(argv[1], "rb");  // open the source file for read;
    if (fp == nullptr){
        fprintf(stderr, "Opening the rule source file %s for reading failed: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    sal_Char str[1024];
    OUStringBuffer Obuf;
    while (fgets(str, 1024, fp)) {
        // don't convert last new line character to Ostr.
        sal_Int32 len = strlen(str) - 1;
        // skip comment line
        if (len == 0 || str[0] == '#')
            continue;

        // input file is in UTF-8 encoding
        OUString Ostr = OUString(str, len, RTL_TEXTENCODING_UTF8).trim();

        len = Ostr.getLength();
        if (len == 0)
            continue; // skip empty line.

        Obuf.append(Ostr);
    }
    fclose(fp);

    UErrorCode status = U_ZERO_ERROR;
    //UParseError parseError;
    //UCollator *coll = ucol_openRules(Obuf.getStr(), Obuf.getLength(), UCOL_OFF,
    //        UCOL_DEFAULT_STRENGTH, &parseError, &status);

    RuleBasedCollator *coll = new RuleBasedCollator(reinterpret_cast<const UChar *>(Obuf.getStr()), status);

    if (U_SUCCESS(status)) {
        std::vector<uint8_t> data;
        int32_t len = coll->cloneBinary(nullptr, 0, status);
        if (status == U_BUFFER_OVERFLOW_ERROR) {
            data.resize(len);
            status = U_ZERO_ERROR;
            len = coll->cloneBinary(data.data(), len, status);
        }
        if (U_SUCCESS(status))
            data_write(argv[2], argv[3], data.data(), len);
        else {
            printf("Could not get rule data from collator\n");
        }
    } else {
        printf("\nRule parsering error\n");
    }

    delete coll;

    return U_SUCCESS(status) ? 0 : 1;
}   // End of main

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
