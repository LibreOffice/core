﻿/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * ���Ľ�Ʈ���� �ϼ�����Ʈ������ ��ȯ�Ѵ�.
 */
DLLEXPORT ::std::string hstr2ksstr(hchar const* hstr);

/**
 * �ѱ��� ������ �� �ִ� char����Ʈ���� ���Ľ�Ʈ������ ��ȯ�Ѵ�.
 */
DLLEXPORT hchar_string kstr2hstr(uchar const* src);

/**
 * hwp�� ��θ� unix���·� �ٲ۴�.
 */
DLLEXPORT ::std::string urltounix(const char *src);

/**
 * hwp�� ��θ� windows���·� �ٲ۴�.
 */
#ifdef _WIN32
DLLEXPORT ::std::string urltowin(const char *src);
#endif
/**
 *  Transfer interger to string following format
 */
DLLEXPORT char* Int2Str(int value, const char *format, char *buf);

/**
 * color�ε��� ���� �������� �����Ͽ� ��Ÿ���ǽ��� color�� ��ȯ
 */
DLLEXPORT char *hcolor2str(uchar color, uchar shade, char *buf, bool bIsChar = false);

DLLEXPORT char *base64_encode_string( const uchar *buf, unsigned int len );
DLLEXPORT double calcAngle(int x1, int y1, int x2, int y2);


#endif                                            /* _HCODE_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
