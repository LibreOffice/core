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
#include "precompiled_sal.hxx"

/* TODO! This file should not be called textenc.c, because it is not the
   implementation of rtl/textenc.h.  Rather, it should be called
   gettextencodingdata.c. */
#include "gettextencodingdata.h"
#include "tenchelp.h"
#include "rtl/textenc.h"
#include <sal/macros.h>
#include "rtl/ustring.hxx"
#include "osl/module.h"

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

#define NOTABUNI_START 0xFF
#define NOTABUNI_END 0x00

#define NOTABCHAR_START 0xFFFF
#define NOTABCHAR_END 0x0000

#define SAME8090UNI_START 0x80
#define SAME8090UNI_END 0x9F
static sal_uInt16 const aImpl8090SameToUniTab[SAME8090UNI_END
                                                  - SAME8090UNI_START
                                                  + 1]
= { 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, /* 0x80 */
    0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, /* 0x90 */
    0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F };

#define SAME8090CHAR_START 0x0080
#define SAME8090CHAR_END 0x009F
static sal_uChar const aImpl8090SameToCharTab[SAME8090CHAR_END
                                                  - SAME8090CHAR_START
                                                  + 1]
    = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0x0080 */
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 0x0090 */
        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };

#define SAMEA0FFCHAR_START 0x00A0
#define SAMEA0FFCHAR_END 0x00FF
static sal_uChar const aImplA0FFSameToCharTab[SAMEA0FFCHAR_END
                                                  - SAMEA0FFCHAR_START
                                                  + 1]
    = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, /* 0x00A0 */
        0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, /* 0x00B0 */
        0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, /* 0x00C0 */
        0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, /* 0x00D0 */
        0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, /* 0x00E0 */
        0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, /* 0x00F0 */
        0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

static sal_uInt16 const aImplDoubleByteIdentifierTab[1] = { 0 };

#include "tcvtest1.tab"
#include "tcvtlat1.tab"
#include "tcvtuni1.tab"

extern "C" {
        typedef ImplTextEncodingData const *(*TextEncodingFunction) (rtl_TextEncoding nEncoding);
};

// Yes - we should use the unpleasant to use templatized
// sal:: doublecheckfoo thing here.
static TextEncodingFunction pTables;

#define DOSTRING( x )  #x
#define STRING( x )    DOSTRING( x )

ImplTextEncodingData const *
Impl_getTextEncodingData(rtl_TextEncoding nEncoding) SAL_THROW_EXTERN_C()
{
    switch(nEncoding)
    {
        case RTL_TEXTENCODING_ASCII_US:
            return &aImplUSASCIITextEncodingData; break;
        case RTL_TEXTENCODING_MS_1252:
            return &aImplMS1252TextEncodingData; break;
        case RTL_TEXTENCODING_UTF8:
            return &aImplUTF8TextEncodingData; break;
        case RTL_TEXTENCODING_ISO_8859_1:
            return &aImplISO88591TextEncodingData; break;

#ifndef TOOLS_STRING_MISSUSE
// ----------------------------------------------
// These are here temporarily since they are used in error
// all over the place
// ----------------------------------------------
        case RTL_TEXTENCODING_IBM_850:
            return &aImplIBM850TextEncodingData; break;
        case RTL_TEXTENCODING_IBM_857:
            return &aImplIBM857TextEncodingData; break;
        case RTL_TEXTENCODING_IBM_860:
            return &aImplIBM860TextEncodingData; break;
        case RTL_TEXTENCODING_IBM_861:
            return &aImplIBM861TextEncodingData; break;
        case RTL_TEXTENCODING_IBM_863:
            return &aImplIBM863TextEncodingData; break;
// ----------------------------------------------
#endif
        default:
            if (!pTables)
            {
                static char const pName[] = STRING(PLUGIN_NAME);
                oslModule aModule = osl_loadModuleAscii(pName, SAL_LOADMODULE_DEFAULT);

                if(aModule)
                {
                    static char const pSymbol[] = "Impl_getTextEncodingData";
                    pTables = (TextEncodingFunction)osl_getAsciiFunctionSymbol(aModule, pSymbol);
                }
            }
            if (pTables)
                return pTables(nEncoding);
//          else
//              fprintf (stderr, "missing text encoding library for %d\n", nEncoding);
            break;
    }
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
