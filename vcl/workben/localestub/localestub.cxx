/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

extern "C" const sal_Unicode* getSTC_CharData_T2S() { return nullptr; }
extern "C" const sal_uInt16* getSTC_CharIndex_T2S() { return nullptr; }
extern "C" const sal_Unicode* getSTC_CharData_S2V() { return nullptr; }
extern "C" const sal_uInt16* getSTC_CharIndex_S2V() { return nullptr; }
extern "C" const sal_Unicode* getSTC_CharData_S2T() { return nullptr; }
extern "C" const sal_uInt16* getSTC_CharIndex_S2T() { return nullptr; }
extern "C" const sal_Unicode* getSTC_WordData(sal_Int32&) { return nullptr; }
extern "C" const sal_uInt16* getSTC_WordIndex_T2S(sal_Int32&) { return nullptr; }
extern "C" const sal_uInt16* getSTC_WordEntry_T2S() { return nullptr; }
extern "C" const sal_uInt16* getSTC_WordIndex_S2T(sal_Int32&) { return nullptr; }
extern "C" const sal_uInt16* getSTC_WordEntry_S2T() { return nullptr; }
extern "C" sal_uInt16** get_zh_zhuyin() { return nullptr; }
extern "C" sal_uInt16** get_zh_pinyin() { return nullptr; }
extern "C" const sal_Unicode* getHangul2HanjaData() { return nullptr; }
extern "C" const void* getHangul2HanjaIndex() { return nullptr; }
extern "C" sal_Int16 getHangul2HanjaIndexCount() { return 0; }
extern "C" const sal_uInt16* getHanja2HangulIndex() { return nullptr; }
extern "C" const sal_Unicode* getHanja2HangulData() { return 0; }
extern "C" sal_uInt16** get_indexdata_ko_dict(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_TW_radical(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_TW_stroke(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_radical(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_pinyin(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_stroke(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_indexdata_zh_zhuyin(sal_Int16*) { return nullptr; }
extern "C" sal_uInt16** get_ko_phonetic(sal_Int16*) { return nullptr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
