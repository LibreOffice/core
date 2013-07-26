/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"
#include "rtl/textcvt.h"

#include <stdio.h>

struct Encoder
{
    rtl_UnicodeToTextConverter m_aConverter;
    bool m_bCapable;
    rtl_TextEncoding m_nEncoding;
    const char *m_pEncoding;
    Encoder(rtl_TextEncoding nEncoding, const char *pEncoding)
        : m_bCapable(true)
        , m_nEncoding(nEncoding)
        , m_pEncoding(pEncoding)
    {
        m_aConverter = rtl_createUnicodeToTextConverter(m_nEncoding);
    }
    ~Encoder()
    {
        rtl_destroyUnicodeToTextConverter(m_aConverter);
    }
    void checkSupports(sal_Unicode c)
    {
        sal_Char aTempArray[8];
        sal_Size nTempSize;
        sal_uInt32 nCvtInfo;

        sal_Size nChars = rtl_convertUnicodeToText(m_aConverter,
            NULL, &c, 1, aTempArray, sizeof(aTempArray),
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |  RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
             &nCvtInfo, &nTempSize);
        m_bCapable = nChars > 0;
    }
    void reset()
    {
        m_bCapable = true;
    }
    bool isOK() const
    {
        return m_bCapable;
    }
    const char* getName() const
    {
        return m_pEncoding;
    }

};

int main()
{
#   define EXP(x) Encoder(x, #x)

    Encoder aConverters[15] =
    {
        EXP(RTL_TEXTENCODING_MS_1361),
        EXP(RTL_TEXTENCODING_MS_950),
        EXP(RTL_TEXTENCODING_MS_949),
        EXP(RTL_TEXTENCODING_MS_936),
        EXP(RTL_TEXTENCODING_MS_932),
        EXP(RTL_TEXTENCODING_MS_874),
        EXP(RTL_TEXTENCODING_MS_1258),
        EXP(RTL_TEXTENCODING_MS_1257),
        EXP(RTL_TEXTENCODING_MS_1256),
        EXP(RTL_TEXTENCODING_MS_1255),
        EXP(RTL_TEXTENCODING_MS_1254),
        EXP(RTL_TEXTENCODING_MS_1253),
        EXP(RTL_TEXTENCODING_MS_1251),
        EXP(RTL_TEXTENCODING_MS_1250),
        EXP(RTL_TEXTENCODING_MS_1252)
    };

    printf("//Do not edit manually, generated from bestreversemap.cxx\n");
    printf("rtl_TextEncoding getBestMSEncodingByChar(sal_Unicode c)\n");
    printf("{\n");

    sal_Unicode c = 0;
    while (c < 0xFFFF)
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(aConverters); ++i)
            aConverters[i].reset();

        int nMostCapable = -1;

        while(c < 0xFFFF)
        {
            bool bSomethingCapable = false;
            for (size_t i = 0; i < SAL_N_ELEMENTS(aConverters); ++i)
            {
                if (aConverters[i].isOK())
                    aConverters[i].checkSupports(c);
                if (aConverters[i].isOK())
                {
                    bSomethingCapable = true;
                    nMostCapable = i;
                }
            }
            if (!bSomethingCapable)
                break;
            ++c;
        }
        sal_Unicode cEnd = c;
        printf("    if (c < 0x%x)\n", c);
        printf("        return %s;\n", aConverters[nMostCapable].getName());
        while(c < 0xFFFF)
        {
            bool bNothingCapable = true;
            for (size_t i = 0; i < SAL_N_ELEMENTS(aConverters); ++i)
            {
                aConverters[i].checkSupports(c);
                if (aConverters[i].isOK())
                {
                    bNothingCapable = false;
                    break;
                }
            }
            if (!bNothingCapable)
                break;
            ++c;
        }
        if (cEnd != c)
        {
            if (c < 0xFFFF)
            {
                printf("    if (c < 0x%x)\n", c);
                printf("        return RTL_TEXTENCODING_DONTKNOW;\n");
            }
            else
                printf("    return RTL_TEXTENCODING_DONTKNOW;\n");
        }
    }

    printf("}\n");
    fflush(stdout);

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
