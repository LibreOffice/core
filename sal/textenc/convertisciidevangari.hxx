/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_TEXTENC_CONVERTISCIIDEVANGARI_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTISCIIDEVANGARI_HXX

#include <sal/types.h>

sal_Size ImplConvertIsciiDevanagariToUnicode(void const * pData,
    void * pContext, char const * pSrcBuf, sal_Size nSrcBytes,
    sal_Unicode * pDestBuf, sal_Size nDestChars, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes);

sal_Size ImplConvertUnicodeToIsciiDevanagari(void const * pData,
    void * pContext, sal_Unicode const * pSrcBuf, sal_Size nSrcChars,
    char * pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size * pSrcCvtChars);

void *ImplCreateIsciiDevanagariToUnicodeContext();

void ImplDestroyIsciiDevanagariToUnicodeContext(void * pContext);

void ImplResetIsciiDevanagariToUnicodeContext(void * pContext);

void *ImplCreateUnicodeToIsciiDevanagariContext();

void ImplResetUnicodeToIsciiDevanagariContext(void * pContext);

void ImplDestroyUnicodeToIsciiDevanagariContext(void * pContext);

#endif // INCLUDED_SAL_TEXTENC_CONVERTISCIIDEVANGARI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
