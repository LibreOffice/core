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

#ifndef INCLUDED_FILE_URL_H
#define INCLUDED_FILE_URL_H

#include "osl/file.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************
 * osl_getSystemPathFromFileURL_Ex
 *************************************************/

#define FURL_ALLOW_RELATIVE sal_True
#define FURL_DENY_RELATIVE  sal_False

oslFileError osl_getSystemPathFromFileURL_Ex(rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath, sal_Bool bAllowRelative);

/**************************************************
 * FileURLToPath
 *************************************************/

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL);

/***************************************************
 * UnicodeToText
 **************************************************/

int UnicodeToText(char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen);

/***************************************************
 * TextToUniCode
 **************************************************/

int TextToUnicode(const char* text, size_t text_buffer_size, sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* #define INCLUDED_FILE_URL_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
