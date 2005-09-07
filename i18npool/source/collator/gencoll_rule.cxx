/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gencoll_rule.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:07:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sal/main.h>
#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <unicode/tblcoll.h>

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
    fprintf(fp, "#include <collator_unicode.hxx>\n");
    fprintf(fp, "\nnamespace com { namespace sun { namespace star { namespace i18n {\n");

    // generate main dict. data array
    fprintf(fp, "\nstatic const sal_uInt8 %s[] = {", name);

    sal_Int32 count = 0;
    for (sal_Int32 i = 0; i < len; i++) {

        if (count++ % 16 == 0)
            fprintf(fp, "\n\t");

        fprintf(fp, "0x%04x, ", data[i]);
    }
    fprintf(fp, "\n};\n\n");

    fprintf(fp, "Collator_%s::Collator_%s()\n{\n", name, name);
    fprintf(fp, "\timplementationName = \"com.sun.star.i18n.Collator_%s\";\n", name);
    fprintf(fp, "\trulesImage = %s;\n}\n\n", name);

    fprintf (fp, "} } } }\n");

    fclose(fp);

}

U_CAPI uint8_t* U_EXPORT2
ucol_cloneRuleData(const UCollator *coll, int32_t *length, UErrorCode *status);

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

    RuleBasedCollator *coll = new RuleBasedCollator(Obuf.getStr(), status);

    if (U_SUCCESS(status)) {

        int32_t len = 0;
        //uint8_t *data = ucol_cloneRuleData(coll, &len, &status);
        uint8_t *data = coll->cloneRuleData(len, status);

        if (U_SUCCESS(status) && data != NULL)
            data_write(argv[2], argv[3], data, len);
        else {
            printf("Could not get rule data from collator\n");
        }
    } else {
        printf("\nRule parsering error\n");
    }

    if (coll)
        delete coll;

    exit(U_SUCCESS(status) ? 0 : 1);
    return 0;
}   // End of main
