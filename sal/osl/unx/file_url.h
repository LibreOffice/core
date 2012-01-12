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
