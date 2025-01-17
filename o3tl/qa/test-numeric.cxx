/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <o3tl/numeric.hxx>
#include <sal/types.h>
#include <sal/config.h>

static_assert(o3tl::convertToHex<int>(' ') == -1);
static_assert(o3tl::convertToHex<int>('*') == -1);
static_assert(o3tl::convertToHex<int>('?') == -1);
static_assert(o3tl::convertToHex<int>('1') == 0x01);
static_assert(o3tl::convertToHex<int>('9') == 0x09);
static_assert(o3tl::convertToHex<int>('a') == 0x0a);
static_assert(o3tl::convertToHex<int>('A') == 0x0A);
static_assert(o3tl::convertToHex<int>('f') == 0x0f);
static_assert(o3tl::convertToHex<int>('F') == 0x0f);
static_assert(o3tl::convertToHex<int>('g') == -1);
static_assert(o3tl::convertToHex<int>('G') == -1);
static_assert(o3tl::convertToHex<sal_uInt8>('G') == 0xff);

static_assert(o3tl::convertToHex<int>(u' ') == -1);
static_assert(o3tl::convertToHex<int>(u'*') == -1);
static_assert(o3tl::convertToHex<int>(u'?') == -1);
static_assert(o3tl::convertToHex<int>(u'1') == 0x01);
static_assert(o3tl::convertToHex<int>(u'9') == 0x09);
static_assert(o3tl::convertToHex<int>(u'a') == 0x0a);
static_assert(o3tl::convertToHex<int>(u'A') == 0x0A);
static_assert(o3tl::convertToHex<int>(u'f') == 0x0f);
static_assert(o3tl::convertToHex<int>(u'F') == 0x0f);
static_assert(o3tl::convertToHex<int>(u'g') == -1);
static_assert(o3tl::convertToHex<int>(u'G') == -1);

static_assert(o3tl::convertToHex<sal_uInt8>(u'*') == 0xff);
static_assert(o3tl::convertToHex<sal_uInt8>(u'A') == 0x0a);
static_assert(o3tl::convertToHex<sal_uInt8>(u'G') == 0xff);

static_assert(o3tl::convertToHex<sal_uInt8>('0', '1') == 0x01);
static_assert(o3tl::convertToHex<sal_uInt8>('0', 'A') == 0x0a);
static_assert(o3tl::convertToHex<sal_uInt8>('0', 'F') == 0x0f);
static_assert(o3tl::convertToHex<sal_uInt8>('1', '0') == 0x10);
static_assert(o3tl::convertToHex<sal_uInt8>('A', '0') == 0xa0);
static_assert(o3tl::convertToHex<sal_uInt8>('F', '0') == 0xf0);

static_assert(o3tl::convertToHex<sal_uInt8>(u'0', u'1') == 0x01);
static_assert(o3tl::convertToHex<sal_uInt8>(u'0', u'A') == 0x0a);
static_assert(o3tl::convertToHex<sal_uInt8>(u'0', u'F') == 0x0f);
static_assert(o3tl::convertToHex<sal_uInt8>(u'1', u'0') == 0x10);
static_assert(o3tl::convertToHex<sal_uInt8>(u'A', u'0') == 0xa0);
static_assert(o3tl::convertToHex<sal_uInt8>(u'F', u'0') == 0xf0);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
