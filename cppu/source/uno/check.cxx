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

#include <sal/config.h>

#include <cassert>

#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/any2.h>


namespace {

#if defined( SAL_W32)
#pragma pack(push, 8)
#endif

struct C1
{
    sal_Int16 n1;
};
struct C2 : public C1
{
    sal_Int32 n2 CPPU_GCC3_ALIGN( C1 );
};
struct C3 : public C2
{
    double d3;
    sal_Int32 n3;
};
struct C4 : public C3
{
    sal_Int32 n4 CPPU_GCC3_ALIGN( C3 );
    double d4;
};
struct C5 : public C4
{
    sal_Int64 n5;
    sal_Bool b5;
};
struct C6 : public C1
{
    C5 c6 CPPU_GCC3_ALIGN( C1 );
    sal_Bool b6;
};

struct D
{
    sal_Int16 d;
    sal_Int32 e;
};
struct E
{
    sal_Bool a;
    sal_Bool b;
    sal_Bool c;
    sal_Int16 d;
    sal_Int32 e;
};

struct M
{
    sal_Int32   n;
    sal_Int16   o;
};

struct N : public M
{
    sal_Int16   p CPPU_GCC3_ALIGN( M );
};
struct N2
{
    M m;
    sal_Int16   p;
};

struct O : public M
{
    double  p;
    sal_Int16 q;
};
struct O2 : public O
{
    sal_Int16 p2 CPPU_GCC3_ALIGN( O );
};

struct P : public N
{
    double  p2;
};

struct empty
{
};
struct second : public empty
{
    int a;
};

struct AlignSize_Impl
{
    sal_Int16   nInt16;
    double      dDouble;
};

struct Char1
{
    char c1;
};
struct Char2 : public Char1
{
    char c2 CPPU_GCC3_ALIGN( Char1 );
};
struct Char3 : public Char2
{
    char c3 CPPU_GCC3_ALIGN( Char2 );
};
struct Char4
{
    Char3 chars;
    char c;
};
enum Enum
{
    v = SAL_MAX_ENUM
};

#ifdef SAL_W32
#   pragma pack(pop)
#endif

static_assert( static_cast<sal_Bool>(true) == sal_True,
               "must be binary compatible" );
static_assert( static_cast<sal_Bool>(false) == sal_False,
               "must be binary compatible" );
#if SAL_TYPES_ALIGNMENT8 == 2
static_assert(offsetof(AlignSize_Impl, dDouble) == 2, "offsetof(AlignSize_Impl, dDouble) != 2");
static_assert(sizeof(AlignSize_Impl) == 10, "sizeof(AlignSize_Impl) != 10");
#elif SAL_TYPES_ALIGNMENT8 == 4
static_assert(offsetof(AlignSize_Impl, dDouble) == 4, "offsetof(AlignSize_Impl, dDouble) != 4");
static_assert(sizeof(AlignSize_Impl) == 12, "sizeof(AlignSize_Impl) != 12");
#elif SAL_TYPES_ALIGNMENT8 == 8
static_assert(offsetof(AlignSize_Impl, dDouble) == 8, "offsetof(AlignSize_Impl, dDouble) != 8");
static_assert(sizeof(AlignSize_Impl) == 16, "sizeof(AlignSize_Impl) != 16");
#else
# error unexpected alignment of 8 byte types
#endif

// sequence
static_assert((SAL_SEQUENCE_HEADER_SIZE % 8) == 0, "binary compatibility test failed: (SAL_SEQUENCE_HEADER_SIZE % 8) == 0!!!");
// enum
static_assert(sizeof(Enum) == sizeof(sal_Int32), "binary compatibility test failed: (sizeof(Enum) == sizeof(sal_Int32))");
// any
static_assert(sizeof(void *) >= sizeof(sal_Int32), "binary compatibility test failed: (sizeof(void *) >= sizeof(sal_Int32))");
static_assert(sizeof(uno_Any) == sizeof(void *) * 3, "binary compatibility test failed: (sizeof(uno_Any) == sizeof(void *) * 3");
static_assert(offsetof(uno_Any, pType) == 0, "offsetof(uno_Any, pType) != 0");
static_assert(offsetof(uno_Any, pData) == 1 * sizeof(void *), "offsetof(uno_Any, pTData) != (1 * sizeof(void *))");
static_assert(offsetof(uno_Any, pReserved) == 2 * sizeof(void *), "offsetof(uno_Any, pReserved) != (2 * sizeof(void *))");
// string
static_assert(sizeof(OUString) == sizeof(rtl_uString *), "binary compatibility test failed: sizeof(OUString) != sizeof(rtl_uString *)");
// struct
#if SAL_TYPES_ALIGNMENT8 == 2
static_assert(sizeof(M) == 6, "sizeof(M) != 6");
static_assert(sizeof(N) == 8, "sizeof(N) != 8");
static_assert(sizeof(N2) == 8, "sizeof(N2) != 8");
static_assert(offsetof(N2, p) == 6, "offsetof(N2, p) != 6");
#else
static_assert(sizeof(M) == 8, "sizeof(M) != 8");
static_assert(sizeof(N) == 12, "sizeof(N) != 12");
static_assert(sizeof(N2) == 12, "sizeof(N2) != 12");
static_assert(offsetof(N2, p) == 8, "offsetof(N2, p) != 8");
#endif
static_assert(offsetof(M, o) == 4, "offsetof(M, o) != 4");

#if SAL_TYPES_ALIGNMENT8 == 2
static_assert(sizeof(O) == 16, "sizeof(O) != 16");
#elif SAL_TYPES_ALIGNMENT8 == 4
static_assert(sizeof(O) == 20, "sizeof(O) != 20");
#elif SAL_TYPES_ALIGNMENT8 == 8
static_assert(sizeof(O) == 24, "sizeof(O) != 24");
#else
# error unexpected alignment of 8 byte types
#endif

#if SAL_TYPES_ALIGNMENT8 == 2
static_assert(sizeof(C2) == 6, "sizeof(C2) != 6");
static_assert(sizeof(D) == 6, "sizeof(D) != 6");
static_assert(offsetof(D, e) == 2, "offsetof(D, e) != 2");
static_assert(offsetof(E, e) == 6, "offsetof(E, e) != 6");
#else
static_assert(sizeof(C2) == 8, "sizeof(C2) != 8");
static_assert(sizeof(D) == 8, "sizeof(D) != 8");
static_assert(offsetof(D, e) == 4, "offsetof(D, e) != 4");
static_assert(offsetof(E, e) == 8, "offsetof(E, e) != 8");
#endif

static_assert(sizeof(C1) == 2, "sizeof(C1) != 2");
static_assert(offsetof(E, d) == 4, "offsetof(E, d) != 4");

#if SAL_TYPES_ALIGNMENT8 == 2
static_assert(sizeof(C3) == 18, "sizeof(C3) != 18");
static_assert(sizeof(C4) == 30, "sizeof(C4) != 30");
static_assert(sizeof(C5) == 40, "sizeof(C5) != 40");
static_assert(sizeof(C6) == 44, "sizeof(C6) != 44");

static_assert(sizeof(O2) == 18, "sizeof(O2) != 18");
#elif SAL_TYPES_ALIGNMENT8 == 4
static_assert(sizeof(C3) == 20, "sizeof(C3) != 20");
static_assert(sizeof(C4) == 32, "sizeof(C4) != 32");
static_assert(sizeof(C5) == 44, "sizeof(C5) != 44");
static_assert(sizeof(C6) == 52, "sizeof(C6) != 52");

static_assert(sizeof(O2) == 24, "sizeof(O2) != 24");
#elif SAL_TYPES_ALIGNMENT8 == 8
static_assert(sizeof(C3) == 24, "sizeof(C3) != 24");
static_assert(sizeof(C4) == 40, "sizeof(C4) != 40");
static_assert(sizeof(C5) == 56, "sizeof(C5) != 56");
static_assert(sizeof(C6) == 72, "sizeof(C6) != 72");

static_assert(sizeof(O2) == 32, "sizeof(O2) != 32");
#else
# error unexpected alignment of 8 byte types
#endif

static_assert(sizeof(Char3) == 3, "sizeof(Char3) != 3");

#if SAL_TYPES_ALIGNMENT8 == 2
// max alignment is 2
static_assert(sizeof(P) == 16, "sizeof(P) != 16");
#elif SAL_TYPES_ALIGNMENT8 == 4
// max alignment is 4
static_assert(sizeof(P) == 20, "sizeof(P) != 20");
#elif SAL_TYPES_ALIGNMENT8 == 8
// alignment of P is 8, because of P[] ...
static_assert(sizeof(P) == 24, "sizeof(P) != 24");
static_assert(sizeof(second) == sizeof(int), "sizeof(second) != sizeof(int)");
#else
# error unexpected alignment of 8 byte types
#endif

#if OSL_DEBUG_LEVEL > 0

#define OFFSET_OF( s, m ) reinterpret_cast< size_t >(reinterpret_cast<char *>(&reinterpret_cast<s *>(16)->m) -16)

class BinaryCompatible_Impl
{
public:
    BinaryCompatible_Impl();
};
BinaryCompatible_Impl::BinaryCompatible_Impl()
{
    assert(OFFSET_OF(N, p) == 8);

    assert(OFFSET_OF(C2, n2) == 4);

#if SAL_TYPES_ALIGNMENT8 == 2
    assert(OFFSET_OF(C3, d3) == 6);
    assert(OFFSET_OF(C3, n3) == 14);
    assert(OFFSET_OF(C4, n4) == 18);
    assert(OFFSET_OF(C4, d4) == 22);
    assert(OFFSET_OF(C5, n5) == 30);
    assert(OFFSET_OF(C5, b5) == 38);
    assert(OFFSET_OF(C6, c6) == 2);
    assert(OFFSET_OF(C6, b6) == 42);

    assert(OFFSET_OF(O2, p2) == 16);
#elif SAL_TYPES_ALIGNMENT8 == 4
    assert(OFFSET_OF(C3, d3) == 8);
    assert(OFFSET_OF(C3, n3) == 16);
    assert(OFFSET_OF(C4, n4) == 20);
    assert(OFFSET_OF(C4, d4) == 24);
    assert(OFFSET_OF(C5, n5) == 32);
    assert(OFFSET_OF(C5, b5) == 40);
    assert(OFFSET_OF(C6, c6) == 4);
    assert(OFFSET_OF(C6, b6) == 48);

    assert(OFFSET_OF(O2, p2) == 20);
#elif SAL_TYPES_ALIGNMENT8 == 8
    assert(OFFSET_OF(C3, d3) == 8);
    assert(OFFSET_OF(C3, n3) == 16);
    assert(OFFSET_OF(C4, n4) == 24);
    assert(OFFSET_OF(C4, d4) == 32);
    assert(OFFSET_OF(C5, n5) == 40);
    assert(OFFSET_OF(C5, b5) == 48);
    assert(OFFSET_OF(C6, c6) == 8);
    assert(OFFSET_OF(C6, b6) == 64);

    assert(OFFSET_OF(O2, p2) == 24);
#else
# error unexpected alignment of 8 byte types
#endif

    assert(OFFSET_OF(Char4, c) == 3);
}

static BinaryCompatible_Impl aTest;

#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
