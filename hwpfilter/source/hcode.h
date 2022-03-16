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

#pragma once

#include <sal/config.h>

#include <rtl/ustring.hxx>

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
DLLEXPORT OUString hstr2OUString(hchar const* hstr);
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
 * color인덱스 값과 음영값을 조합하여 스타오피스의 color로 변환
 */
DLLEXPORT OUString hcolor2str(uchar color, uchar shade, bool bIsChar = false);
DLLEXPORT OUString rgb2str(unsigned char red, unsigned char green, unsigned char blue);
DLLEXPORT OUString rgb2str(int32_t rgb);

DLLEXPORT OUString base64_encode_string( const uchar *buf, unsigned int len );
DLLEXPORT double calcAngle(double x1, double y1, double x2, double y2);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
