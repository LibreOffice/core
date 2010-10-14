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

#include "precompiled_bridges.hxx"
#include "sal/config.h"

#include <cstddef>
#include <cstring>
#include <map>
#include <utility>
#include <vector>

#include "bridges/cpp_uno/shared/arraypointer.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/genfunc.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "uno/any2.h"
#include "uno/data.h"
#include "uno/mapping.h"

#include "exceptions.hxx"
#include "flushcode.hxx"

namespace {

namespace css = com::sun::star;

typedef void (* Function)(void *);

Function toFunction(void * pointer) {
#pragma disable_warn
    return reinterpret_cast< Function >(pointer);
#pragma enable_warn
}

bool toUnoName(char const * rttiName, rtl::OUString * unoName) {
    rtl::OStringBuffer buf;
    for (;;) {
        char const * p = std::strchr(rttiName, ':');
        if (p == NULL) {
            buf.append(rttiName);
            break;
        }
        if (p - rttiName > SAL_MAX_INT32) {
            return false;
        }
        buf.append(rttiName, sal::static_int_cast< sal_Int32 >(p - rttiName));
        buf.append(".");
        while (*p == ':') {
            ++p;
        }
        rttiName = p;
    }
    *unoName = rtl::OStringToOUString(
        buf.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
        //TODO: check conversion failure
    return true;
}

class NistHash {
public:
    NistHash(rtl::OString const & text);

    sal_uInt32 hashdata[5];

private:
    static sal_uInt32 f1(sal_uInt32 x, sal_uInt32 y, sal_uInt32 z)
    { return z ^ (x & (y ^ z)); }

    static sal_uInt32 f2(sal_uInt32 x, sal_uInt32 y, sal_uInt32 z)
    { return x ^ y ^ z; }

    static sal_uInt32 f3(sal_uInt32 x, sal_uInt32 y, sal_uInt32 z)
    { return (x & y) + (z & (x ^ y)); }

    static sal_uInt32 rotl(sal_uInt32 value, sal_uInt32 bits)
    { return (value << bits) | (value >> (32 - bits)); }

    sal_uInt32 expand_nostore(sal_uInt32 index) {
        return data[index & 15] ^ data[(index - 14) & 15] ^
            data[(index - 8) & 15] ^ data[(index - 3) & 15];
    }

    sal_uInt32 expand_store(sal_uInt32 index) {
        return data[index & 15] ^= data[(index - 14) & 15] ^
            data[(index - 8) & 15] ^ data[(index - 3) & 15];
    }

    void subRound(
        sal_uInt32 a, sal_uInt32 & b, sal_uInt32 c, sal_uInt32 d,
        sal_uInt32 & e, sal_uInt32 constant, sal_uInt32 datum,
        sal_uInt32 function)
    {
        e += rotl(a, 5);
        switch (function) {
            case 1:
                e += f1(b, c, d);
                break;
            case 2:
            case 4:
                e += f2(b, c, d);
                break;
            case 3:
                e += f3(b, c, d);
                break;
        }
        e += constant + datum;
        b = rotl(b, 30);
    }

    void transform();

    sal_uInt32 data[16];
};

NistHash::NistHash(rtl::OString const & text) {
    hashdata[0] = 0x67452301;
    hashdata[1] = 0xefcdab89;
    hashdata[2] = 0x98badcfe;
    hashdata[3] = 0x10325476;
    hashdata[4] = 0xc3d2e1f0;
    char const * p = text.getStr();
    sal_Int32 n = text.getLength();
    while (n >= sizeof data) {
        std::memcpy(data, p, sizeof data);
        p += sizeof data;
        n -= sizeof data;
        transform();
    }
    std::memcpy(data, p, n);
    reinterpret_cast< unsigned char *>(data)[n++] = 0x80;
    if (n > sizeof data - 8) {
        std::memset(reinterpret_cast< char * >(data) + n, 0, sizeof data - n);
        transform();
        std::memset(data, 0, sizeof data  - 8);
    } else {
        std::memset(
            reinterpret_cast< char * >(data) + n, 0, sizeof data - 8 - n);
    }
    data[14] = 0;
    data[15] = text.getLength() << 3;
    transform();
}

void NistHash::transform() {
    sal_uInt32 const K2 = 0x5A827999;
    sal_uInt32 const K3 = 0x6ED9EBA1;
    sal_uInt32 const K5 = 0x8F1BBCDC;
    sal_uInt32 const K10 = 0xCA62C1D6;
    sal_uInt32 a = hashdata[0];
    sal_uInt32 b = hashdata[1];
    sal_uInt32 c = hashdata[2];
    sal_uInt32 d = hashdata[3];
    sal_uInt32 e = hashdata[4];
    subRound(a, b, c, d, e, K2, data[ 0], 1);
    subRound(e, a, b, c, d, K2, data[ 1], 1);
    subRound(d, e, a, b, c, K2, data[ 2], 1);
    subRound(c, d, e, a, b, K2, data[ 3], 1);
    subRound(b, c, d, e, a, K2, data[ 4], 1);
    subRound(a, b, c, d, e, K2, data[ 5], 1);
    subRound(e, a, b, c, d, K2, data[ 6], 1);
    subRound(d, e, a, b, c, K2, data[ 7], 1);
    subRound(c, d, e, a, b, K2, data[ 8], 1);
    subRound(b, c, d, e, a, K2, data[ 9], 1);
    subRound(a, b, c, d, e, K2, data[10], 1);
    subRound(e, a, b, c, d, K2, data[11], 1);
    subRound(d, e, a, b, c, K2, data[12], 1);
    subRound(c, d, e, a, b, K2, data[13], 1);
    subRound(b, c, d, e, a, K2, data[14], 1);
    subRound(a, b, c, d, e, K2, data[15], 1);
    subRound(e, a, b, c, d, K2, expand_store(16), 1);
    subRound(d, e, a, b, c, K2, expand_store(17), 1);
    subRound(c, d, e, a, b, K2, expand_store(18), 1);
    subRound(b, c, d, e, a, K2, expand_store(19), 1);
    subRound(a, b, c, d, e, K3, expand_store(20), 2);
    subRound(e, a, b, c, d, K3, expand_store(21), 2);
    subRound(d, e, a, b, c, K3, expand_store(22), 2);
    subRound(c, d, e, a, b, K3, expand_store(23), 2);
    subRound(b, c, d, e, a, K3, expand_store(24), 2);
    subRound(a, b, c, d, e, K3, expand_store(25), 2);
    subRound(e, a, b, c, d, K3, expand_store(26), 2);
    subRound(d, e, a, b, c, K3, expand_store(27), 2);
    subRound(c, d, e, a, b, K3, expand_store(28), 2);
    subRound(b, c, d, e, a, K3, expand_store(29), 2);
    subRound(a, b, c, d, e, K3, expand_store(30), 2);
    subRound(e, a, b, c, d, K3, expand_store(31), 2);
    subRound(d, e, a, b, c, K3, expand_store(32), 2);
    subRound(c, d, e, a, b, K3, expand_store(33), 2);
    subRound(b, c, d, e, a, K3, expand_store(34), 2);
    subRound(a, b, c, d, e, K3, expand_store(35), 2);
    subRound(e, a, b, c, d, K3, expand_store(36), 2);
    subRound(d, e, a, b, c, K3, expand_store(37), 2);
    subRound(c, d, e, a, b, K3, expand_store(38), 2);
    subRound(b, c, d, e, a, K3, expand_store(39), 2);
    subRound(a, b, c, d, e, K5, expand_store(40), 3);
    subRound(e, a, b, c, d, K5, expand_store(41), 3);
    subRound(d, e, a, b, c, K5, expand_store(42), 3);
    subRound(c, d, e, a, b, K5, expand_store(43), 3);
    subRound(b, c, d, e, a, K5, expand_store(44), 3);
    subRound(a, b, c, d, e, K5, expand_store(45), 3);
    subRound(e, a, b, c, d, K5, expand_store(46), 3);
    subRound(d, e, a, b, c, K5, expand_store(47), 3);
    subRound(c, d, e, a, b, K5, expand_store(48), 3);
    subRound(b, c, d, e, a, K5, expand_store(49), 3);
    subRound(a, b, c, d, e, K5, expand_store(50), 3);
    subRound(e, a, b, c, d, K5, expand_store(51), 3);
    subRound(d, e, a, b, c, K5, expand_store(52), 3);
    subRound(c, d, e, a, b, K5, expand_store(53), 3);
    subRound(b, c, d, e, a, K5, expand_store(54), 3);
    subRound(a, b, c, d, e, K5, expand_store(55), 3);
    subRound(e, a, b, c, d, K5, expand_store(56), 3);
    subRound(d, e, a, b, c, K5, expand_store(57), 3);
    subRound(c, d, e, a, b, K5, expand_store(58), 3);
    subRound(b, c, d, e, a, K5, expand_store(59), 3);
    subRound(a, b, c, d, e, K10, expand_store(60), 4);
    subRound(e, a, b, c, d, K10, expand_store(61), 4);
    subRound(d, e, a, b, c, K10, expand_store(62), 4);
    subRound(c, d, e, a, b, K10, expand_store(63), 4);
    subRound(b, c, d, e, a, K10, expand_store(64), 4);
    subRound(a, b, c, d, e, K10, expand_store(65), 4);
    subRound(e, a, b, c, d, K10, expand_store(66), 4);
    subRound(d, e, a, b, c, K10, expand_store(67), 4);
    subRound(c, d, e, a, b, K10, expand_store(68), 4);
    subRound(b, c, d, e, a, K10, expand_store(69), 4);
    subRound(a, b, c, d, e, K10, expand_store(70), 4);
    subRound(e, a, b, c, d, K10, expand_store(71), 4);
    subRound(d, e, a, b, c, K10, expand_store(72), 4);
    subRound(c, d, e, a, b, K10, expand_store(73), 4);
    subRound(b, c, d, e, a, K10, expand_store(74), 4);
    subRound(a, b, c, d, e, K10, expand_store(75), 4);
    subRound(e, a, b, c, d, K10, expand_store(76), 4);
    subRound(d, e, a, b, c, K10, expand_nostore(77), 4);
    subRound(c, d, e, a, b, K10, expand_nostore(78), 4);
    subRound(b, c, d, e, a, K10, expand_nostore(79), 4);
    hashdata[0] += a;
    hashdata[1] += b;
    hashdata[2] += c;
    hashdata[3] += d;
    hashdata[4] += e;
}

class RttiMap {
public:
    static __Crun::static_type_info const * get(
        typelib_CompoundTypeDescription const * type);

private:
    RttiMap(); // not defined
    RttiMap(RttiMap &); // not defined
    ~RttiMap(); // not defined
    void operator =(RttiMap &); // not defined

    struct Data {
        __Crun::static_type_info * info;
        rtl::OString cppName;
        std::vector< __Crun::class_base_descr > bases;
    };
    typedef std::map< rtl::OUString, Data > Map;

    static void toCppNames(
        rtl::OUString const & unoName, rtl::OString * cppName,
        rtl::OString * rttiName);

    static Data const & get_(typelib_CompoundTypeDescription const * type);

    static osl::Mutex m_mutex;
    static Map * m_map;
};

osl::Mutex RttiMap::m_mutex;
RttiMap::Map * RttiMap::m_map;

__Crun::static_type_info const * RttiMap::get(
    typelib_CompoundTypeDescription const * type)
{
    osl::MutexGuard g(m_mutex);
    if (m_map == NULL) {
        m_map = new Map; // leaked
    }
    return get_(type).info;
}

void RttiMap::toCppNames(
    rtl::OUString const & unoName, rtl::OString * cppName,
    rtl::OString * rttiName)
{
    OSL_ASSERT(cppName != NULL && rttiName != NULL);
    rtl::OStringBuffer bc;
    rtl::OStringBuffer br;
    br.append("__1n");
    for (sal_Int32 i = 0; i != -1;) {
        rtl::OUString tok(unoName.getToken(0, '.', i));
        bc.append(rtl::OUStringToOString(tok, RTL_TEXTENCODING_UTF8));
            // conversion should never fail, as tok should be well-formed ASCII
        if (i != -1) {
            bc.append("::");
        }
        sal_Int32 len = tok.getLength();
        sal_Int32 pos = br.getLength();
        for (sal_Int32 n = len / 26; n > 0; n /= 26) {
            br.insert(pos, static_cast< char >('a' + (n % 26)));
        }
        br.append(static_cast< char >('A' + (len % 26)));
        for (sal_Int32 j = 0; j < len; ++j) {
            sal_Unicode c = tok[j];
            OSL_ASSERT(
                c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c == '_' ||
                c >= 'a' && c <= 'z');
            if (c == 'Q') {
                br.append("QdD");
            } else {
                br.append(static_cast< char >(c));
            }
        }
    }
    br.append('_');
    *cppName = bc.makeStringAndClear();
    *rttiName = br.makeStringAndClear();
}

RttiMap::Data const & RttiMap::get_(
    typelib_CompoundTypeDescription const * type)
{
    rtl::OUString name(type->aBase.pTypeName);
    Map::iterator it(m_map->find(name));
    if (it == m_map->end()) {
        it = m_map->insert(std::make_pair(name, Data())).first;
        Data & data = it->second;
        rtl::OString rttiName;
        toCppNames(name, &data.cppName, &rttiName);
        data.info = new __Crun::static_type_info;
        data.info->ty_name = data.cppName.getStr() -
            reinterpret_cast< char * >(&data.info->ty_name);
        data.info->reserved = 0;
        NistHash hash(rttiName);
        data.info->type_hash[0] = hash.hashdata[0];
        data.info->type_hash[1] = hash.hashdata[1];
        data.info->type_hash[2] = hash.hashdata[2];
        data.info->type_hash[3] = hash.hashdata[3];
        data.info->flags = 0;
        data.info->cv_qualifiers = 0;
        if (type->pBaseTypeDescription != NULL) {
            data.bases = get_(type->pBaseTypeDescription).bases;
            OSL_ASSERT(!data.bases.empty());
            data.bases.back().offset = 0;
        }
        __Crun::class_base_descr last;
        last.type_hash[0] = data.info->type_hash[0];
        last.type_hash[1] = data.info->type_hash[1];
        last.type_hash[2] = data.info->type_hash[2];
        last.type_hash[3] = data.info->type_hash[3];
        last.offset = 0x8000000000000000;
        data.bases.push_back(last);
        data.info->base_table = reinterpret_cast< char * >(&data.bases[0]) -
            reinterpret_cast< char * >(&data.info->base_table);
    }
    return it->second;
}

void deleteException(
    void * exception, unsigned int * thunk, typelib_TypeDescription * type)
{
    uno_destructData(
        exception, type,
        reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release));
    typelib_typedescription_release(type);
    delete[] thunk;
}

}

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

void raiseException(uno_Any * exception, uno_Mapping * unoToCpp) {
    bridges::cpp_uno::shared::ArrayPointer< unsigned long > thunkPtr(
        new unsigned long[4]);
    typelib_TypeDescription * type = NULL;
    typelib_typedescriptionreference_getDescription(&type, exception->pType);
    __Crun::static_type_info const * rtti = RttiMap::get(
        reinterpret_cast< typelib_CompoundTypeDescription * >(type));
    void * exc = __Crun::ex_alloc(type->nSize);
    uno_copyAndConvertData(exc, exception->pData, type, unoToCpp);
    uno_any_destruct(exception, NULL);
    unsigned long * thunk = thunkPtr.release();
    // 0*4: rd %pc, %o1:
    // 1*4: ldx %o1, (6-0)*4, %o3:
    thunk[0] = 0x93414000D65A6018;
    // 2*4: jmpl %o3, %g0, %g0:
    // 3*4: ldx %o1, (4-0)*4, %o2:
    thunk[1] = 0x81C2C000D45A6010;
    // 4*4: .xword type:
    thunk[2] = reinterpret_cast< unsigned long >(type);
    // 6*4: .xword deleteException:
    thunk[3] = reinterpret_cast< unsigned long >(deleteException);
    flushCode(thunk, thunk + 4);
    __Crun::ex_throw(exc, rtti, toFunction(thunk));
}

void fillUnoException(
    void * cppException, char const * cppName, uno_Any * unoException,
    uno_Mapping * cppToUno)
{
    rtl::OUString name;
    typelib_TypeDescription * type = NULL;
    if (toUnoName(cppName, &name)) {
        typelib_typedescription_getByName(&type, name.pData);
    }
    if (type == NULL || type->eTypeClass != typelib_TypeClass_EXCEPTION) {
        css::uno::RuntimeException exc(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("Not a UNO exception type: ")) +
             name),
            css::uno::Reference< css::uno::XInterface >());
        uno_type_any_constructAndConvert(
            unoException, &exc, getCppuType(&exc).getTypeLibType(), cppToUno);
    } else {
        uno_any_constructAndConvert(unoException, cppException, type, cppToUno);
    }
    if (type != NULL) {
        typelib_typedescription_release(type);
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
