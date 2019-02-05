/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sal/main.h>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>

#include "warnings_guard_unicode_tblcoll.h"

U_CAPI void U_EXPORT2 uprv_free(void *mem);

using namespace ::rtl;

/* Main Procedure */

void data_write(char* file, char* name, sal_uInt8 *data, sal_Int32 len)
{
    FILE *fp = fopen(file, "wb");
    if (fp == NULL) {
        printf("Can't create the C source file.");
        return;
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

    fprintf(fp, "SAL_DLLPUBLIC_EXPORT const sal_uInt8* get_%s() { return %s; }\n\n", name, name);
    fprintf (fp, "}\n");

    fclose(fp);

}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    FILE *fp;

    if (argc < 4) exit(-1);

    fp = fopen(argv[1], "rb");  // open the source file for read;
    if (fp == NULL)
        printf("Open the rule source file failed.");


    sal_Char str[1024];
    OUStringBuffer Obuf;
    while (fgets(str, 1024, fp)) {
        // don't convert last new line character to Ostr.
        sal_Int32 len = strlen(str) - 1;
        // skip comment line
        if (len == 0 || str[0] == '#')
            continue;

        // input file is in UTF-8 encoding
        OUString Ostr = OUString((const sal_Char *)str, len, RTL_TEXTENCODING_UTF8).trim();

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

    RuleBasedCollator *coll = new RuleBasedCollator(reinterpret_cast<const UChar *>(Obuf.getStr()), status);    // UChar != sal_Unicode in MinGW

    if (U_SUCCESS(status)) {

        int32_t len = 0;
        uint8_t *data = coll->cloneRuleData(len, status);

        if (U_SUCCESS(status) && data != NULL)
            data_write(argv[2], argv[3], data, len);
        else {
            printf("Could not get rule data from collator\n");
        }

    if (data) uprv_free(data);
    } else {
        printf("\nRule parsering error\n");
    }

    if (coll)
        delete coll;

    return U_SUCCESS(status) ? 0 : 1;
}   // End of main
