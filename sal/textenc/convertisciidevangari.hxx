/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

size_t ImplConvertIsciiDevanagariToUnicode(void const * pData,
    void * pContext, char const * pSrcBuf, size_t nSrcBytes,
    sal_Unicode * pDestBuf, size_t nDestChars, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, size_t * pSrcCvtBytes);

size_t ImplConvertUnicodeToIsciiDevanagari(void const * pData,
    void * pContext, sal_Unicode const * pSrcBuf, size_t nSrcChars,
    char * pDestBuf, size_t nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, size_t * pSrcCvtChars);

void *ImplCreateIsciiDevanagariToUnicodeContext();

void ImplDestroyIsciiDevanagariToUnicodeContext(void * pContext);

void ImplResetIsciiDevanagariToUnicodeContext(void * pContext);

void *ImplCreateUnicodeToIsciiDevanagariContext();

void ImplResetUnicodeToIsciiDevanagariContext(void * pContext);

void ImplDestroyUnicodeToIsciiDevanagariContext(void * pContext);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
