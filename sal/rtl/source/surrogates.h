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



#ifndef INCLUDED_SAL_RTL_SOURCE_SURROGATES_H
#define INCLUDED_SAL_RTL_SOURCE_SURROGATES_H

#include "sal/config.h"

#define SAL_RTL_FIRST_HIGH_SURROGATE 0xD800
#define SAL_RTL_LAST_HIGH_SURROGATE 0xDBFF
#define SAL_RTL_FIRST_LOW_SURROGATE 0xDC00
#define SAL_RTL_LAST_LOW_SURROGATE 0xDFFF

#define SAL_RTL_IS_HIGH_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_HIGH_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_HIGH_SURROGATE)

#define SAL_RTL_IS_LOW_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_LOW_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_LOW_SURROGATE)

#define SAL_RTL_COMBINE_SURROGATES(high, low) \
    ((((high) - SAL_RTL_FIRST_HIGH_SURROGATE) << 10) + \
     ((low) - SAL_RTL_FIRST_LOW_SURROGATE) + 0x10000)

#endif
