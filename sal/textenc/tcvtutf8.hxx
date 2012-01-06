/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_SAL_TEXTENC_TCVTUTF8_HXX
#define INCLUDED_SAL_TEXTENC_TCVTUTF8_HXX

#include "sal/config.h"

#include "sal/types.h"

void * ImplCreateUtf8ToUnicodeContext();

void ImplResetUtf8ToUnicodeContext(void * pContext);

void ImplDestroyUtf8ToUnicodeContext(void * pContext);

sal_Size ImplConvertUtf8ToUnicode(
    void const * pData, void * pContext, char const * pSrcBuf,
    sal_Size nSrcBytes, sal_Unicode * pDestBuf, sal_Size nDestChars,
    sal_uInt32 nFlags, sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes);

void * ImplCreateUnicodeToUtf8Context();

void ImplResetUnicodeToUtf8Context(void * pContext);

void ImplDestroyUnicodeToUtf8Context(void * pContext);

sal_Size ImplConvertUnicodeToUtf8(
    void const * pData, void * pContext, sal_Unicode const * pSrcBuf,
    sal_Size nSrcChars, char * pDestBuf, sal_Size nDestBytes, sal_uInt32 nFlags,
    sal_uInt32 * pInfo, sal_Size* pSrcCvtChars);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
