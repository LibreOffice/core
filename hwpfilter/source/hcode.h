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



/* $Id: hcode.h,v 1.3 2008-04-10 12:03:17 rt Exp $ */

#ifndef _HCODE_H_
#define _HCODE_H_

#include "hwplib.h"

/**
 * Codetype of Korean
 * KSSM - Johap, KS - Wansung
 */
enum { KSSM, KS, UNICODE };
/**
 *   Transfer combination-code for internal using of hwp to ascii
 */
DLLEXPORT int hcharconv(hchar ch, hchar *dest, int codeType) ;

DLLEXPORT int   kssm_hangul_to_ucs2(hchar ch, hchar *dest) ;
DLLEXPORT hchar ksc5601_han_to_ucs2 (hchar);
DLLEXPORT hchar ksc5601_sym_to_ucs2 (hchar);
DLLEXPORT hchar_string hstr2ucsstr(hchar const* hstr);
/**
 * 한컴스트링을 완성형스트링으로 변환한다.
 */
DLLEXPORT ::std::string hstr2ksstr(hchar const* hstr);

/**
 * 한글을 포함할 수 있는 char형스트링을 한컴스트링으로 변환한다.
 */
DLLEXPORT hchar_string kstr2hstr(uchar const* src);

/**
 * hwp의 경로를 unix형태로 바꾼다.
 */
DLLEXPORT ::std::string urltounix(const char *src);

/**
 * hwp의 경로를 windows형태로 바꾼다.
 */
#ifdef _WIN32
DLLEXPORT ::std::string urltowin(const char *src);
#endif
/**
 *  Transfer integer to string following format
 */
DLLEXPORT char* Int2Str(int value, const char *format, char *buf);

/**
 * color인덱스 값과 음영값을 조합하여 스타오피스의 color로 변환
 */
DLLEXPORT char *hcolor2str(uchar color, uchar shade, char *buf, bool bIsChar = false);

DLLEXPORT char *base64_encode_string( const uchar *buf, unsigned int len );
DLLEXPORT double calcAngle(int x1, int y1, int x2, int y2);


#endif                                            /* _HCODE_H_ */
