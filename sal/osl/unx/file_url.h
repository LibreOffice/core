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

#ifndef INCLUDED_FILE_URL_H
#define INCLUDED_FILE_URL_H

#include "osl/file.h"

#ifdef __cplusplus
extern "C"
{
#endif

oslFileError osl_getSystemPathFromFileURL_Ex(rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath);

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL);

int UnicodeToText(char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen);

int TextToUnicode(const char* text, size_t text_buffer_size, sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* #define INCLUDED_FILE_URL_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
