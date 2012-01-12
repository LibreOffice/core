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




#ifndef _OSL_NLSUPPORT_H_
#define _OSL_NLSUPPORT_H_

#   include <rtl/locale.h>
#   include <rtl/textenc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    Determines the text encoding used by the underlying platform for the
    specified locale.

    @param pLocale
    the locale to return the text encoding for. If this parameter is NULL,
    the default locale of the current process is used.

    @returns the rtl_TextEncoding that matches the platform specific encoding
    description or RTL_TEXTENCODING_DONTKNOW if no mapping is available.
*/

rtl_TextEncoding SAL_CALL osl_getTextEncodingFromLocale( rtl_Locale * pLocale );


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_NLSUPPORT_H_ */


