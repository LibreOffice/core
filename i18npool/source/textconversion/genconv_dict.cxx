/*************************************************************************
 *
 *  $RCSfile: genconv_dict.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:53:26 $
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
#include <stdlib.h>
#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/strbuf.hxx>

using namespace ::rtl;

/* Main Procedure */

int SAL_CALL main(int argc, char* argv[]) {
    FILE *sfp, *cfp;

    if (argc < 3) exit(-1);

    sfp = fopen(argv[1], "rb"); // open the source file for read;
    if (sfp == NULL)
        printf("Open the dictionary source file failed.");

    // create the C source file to write
    cfp = fopen(argv[2], "wb");
    if (cfp == NULL) {
        fclose(sfp);
        printf("Can't create the C source file.");
    }

    fprintf(cfp, "/*\n");
    fprintf(cfp, " * Copyright(c) 1999 - 2000, Sun Microsystems, Inc.\n");
    fprintf(cfp, " * All Rights Reserved.\n");
    fprintf(cfp, " */\n\n");
    fprintf(cfp, "/* !!!The file is generated automatically. DONOT edit the file manually!!! */\n\n");
    fprintf(cfp, "#include <sal/types.h>\n");
    fprintf(cfp, "#include <textconversion.hxx>\n");
    fprintf(cfp, "\nnamespace com { namespace sun { namespace star { namespace i18n {\n");

    sal_Int32 count, address, i, j, k;
    sal_Unicode Hanja2HangulData[0x10000];
    for (i = 0; i < 0x10000; i++) {
        Hanja2HangulData[i] = 0;
    }
    sal_uInt16 Hangul2HanjaData[10000][3];

    // generate main dict. data array
    fprintf(cfp, "\nstatic const sal_Unicode Hangul2HanjaData[] = {");

    sal_Char str[1024];
    count = 0;
    address = 0;
    while (fgets(str, 1024, sfp)) {
        // input file is in UTF-8 encoding (Hangul:Hanja)
        // don't convert last new line character to Ostr.
        OUString Ostr((const sal_Char *)str, strlen(str) - 1, RTL_TEXTENCODING_UTF8);
        const sal_Unicode *str = Ostr.getStr();
        sal_Int32  len = Ostr.getLength();

        Hangul2HanjaData[count][0] = str[0];
        Hangul2HanjaData[count][1] = address;
        Hangul2HanjaData[count][2] = len - 2;
        count++;

        for (i = 2; i < len; i++) {
            Hanja2HangulData[str[i]] = str[0];
            if (address++ % 16 == 0)
                fprintf(cfp, "\n\t");
            fprintf(cfp, "0x%04x, ", str[i]);
        }
    }
    fprintf(cfp, "\n};\n");

    fprintf(cfp, "\nstatic const Hangul_Index Hangul2HanjaIndex[] = {\n");
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
            fprintf(cfp, "0x%04x, ", k < 0x100 ? (address++)*0x100 : 0xFFFF);
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
    fprintf (cfp, "\tconst Hangul_Index* getHangul2HanjaIndex() { return Hangul2HanjaIndex; }\n");
    fprintf (cfp, "\tconst sal_Int16 getHangul2HanjaIndexCount() { return sizeof(Hangul2HanjaIndex) / sizeof(Hangul_Index); }\n");
    fprintf (cfp, "\tconst sal_uInt16* getHanja2HangulIndex() { return Hanja2HangulIndex; }\n");
    fprintf (cfp, "\tconst sal_Unicode* getHanja2HangulData() { return Hanja2HangulData; }\n");
    fprintf (cfp, "} } } }\n");

    fclose(sfp);
    fclose(cfp);

    return 0;
}   // End of main
