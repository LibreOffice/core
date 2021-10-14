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

#include <cppuhelper/typeprovider.hxx>

#include <rtl/uuid.h>
#include <osl/mutex.hxx>

#include <algorithm>

using namespace osl;
using namespace com::sun::star::uno;

namespace cppu
{

// suppress spurious warning triggered by SAL_DEPRECATED in class declaration
#if defined _MSC_VER && !defined __clang__
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

OImplementationId::~OImplementationId()
{
    delete _pSeq;
}

#if defined _MSC_VER && !defined __clang__
#pragma warning(pop)
#endif

Sequence< sal_Int8 > OImplementationId::getImplementationId() const
{
    if (! _pSeq)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! _pSeq)
        {
            Sequence< sal_Int8 > * pSeq = new Sequence< sal_Int8 >( 16 );
            ::rtl_createUuid( reinterpret_cast<sal_uInt8 *>(pSeq->getArray()), nullptr, _bUseEthernetAddress );
            _pSeq = pSeq;
        }
    }
    return *_pSeq;
}

namespace
{
sal_Int32 TypeSeqLen(const Sequence<Type>& s) { return s.getLength(); }
template <class... Args> sal_Int32 TypeSeqLen(const Type&, Args... args)
{
    return 1 + TypeSeqLen(args...);
}

void PutToTypeSeq(Type* p, const Sequence<Type>& s) { std::copy(s.begin(), s.end(), p); }
template <class... Args> void PutToTypeSeq(Type* p, const Type& t, Args... args)
{
    *p = t;
    PutToTypeSeq(p + 1, args...);
}

template <class... Args> Sequence<Type> InitTypeSeq(Args... args)
{
    Sequence<Type> s(TypeSeqLen(args...));
    PutToTypeSeq(s.getArray(), args...);
    return s;
}
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Type & rT8,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rT8, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Type & rT8,
    const Type & rT9,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rT8, rT9, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Type & rT8,
    const Type & rT9,
    const Type & rT10,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rT8, rT9, rT10, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Type & rT8,
    const Type & rT9,
    const Type & rT10,
    const Type & rT11,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rT8, rT9, rT10, rT11, rMore))
{
}

OTypeCollection::OTypeCollection(
    const Type & rT1,
    const Type & rT2,
    const Type & rT3,
    const Type & rT4,
    const Type & rT5,
    const Type & rT6,
    const Type & rT7,
    const Type & rT8,
    const Type & rT9,
    const Type & rT10,
    const Type & rT11,
    const Type & rT12,
    const Sequence< Type > & rMore )
    : _aTypes(InitTypeSeq(rT1, rT2, rT3, rT4, rT5, rT6, rT7, rT8, rT9, rT10, rT11, rT12, rMore))
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
