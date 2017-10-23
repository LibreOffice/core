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
#ifndef INCLUDED_RTL_BYTESEQ_HXX
#define INCLUDED_RTL_BYTESEQ_HXX

#include "rtl/byteseq.h"

#include <cstddef>
#include <new>

namespace rtl
{


inline ByteSequence::ByteSequence()
    : _pSequence( NULL )
{
    ::rtl_byte_sequence_construct( &_pSequence, 0 );
}

inline ByteSequence::ByteSequence( const ByteSequence & rSeq )
    : _pSequence( NULL )
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
}

#if defined LIBO_INTERNAL_ONLY
inline ByteSequence::ByteSequence( ByteSequence && rSeq )
    : _pSequence(rSeq._pSequence)
{
    rSeq._pSequence = nullptr;
}
#endif

inline ByteSequence::ByteSequence( sal_Sequence *pSequence)
    : _pSequence( pSequence )
{
    ::rtl_byte_sequence_acquire( pSequence );
}

inline ByteSequence::ByteSequence( const sal_Int8 * pElements, sal_Int32 len )
    : _pSequence( NULL )
{
    ::rtl_byte_sequence_constructFromArray( &_pSequence, pElements, len );
    if (_pSequence == NULL)
        throw ::std::bad_alloc();
}

inline ByteSequence::ByteSequence( sal_Int32 len, enum __ByteSequence_NoDefault )
    : _pSequence( NULL )
{
    ::rtl_byte_sequence_constructNoDefault( &_pSequence, len );
    if (_pSequence == NULL)
        throw ::std::bad_alloc();
}

inline ByteSequence::ByteSequence( sal_Sequence *pSequence, enum __ByteSequence_NoAcquire )
    : _pSequence( pSequence )
{
}

inline ByteSequence::ByteSequence( sal_Int32 len )
    : _pSequence( NULL )
{
    ::rtl_byte_sequence_construct( &_pSequence, len );
    if (_pSequence == NULL)
        throw ::std::bad_alloc();
}

inline ByteSequence::~ByteSequence()
{
    ::rtl_byte_sequence_release( _pSequence );
}

inline ByteSequence & ByteSequence::operator = ( const ByteSequence & rSeq )
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
    return *this;
}

#if defined LIBO_INTERNAL_ONLY
inline ByteSequence & ByteSequence::operator = ( ByteSequence && rSeq )
{
    ::rtl_byte_sequence_release(_pSequence);
    _pSequence = rSeq._pSequence;
    rSeq._pSequence = nullptr;
    return *this;
}
#endif

inline bool ByteSequence::operator == ( const ByteSequence & rSeq ) const
{
    return ::rtl_byte_sequence_equals( _pSequence, rSeq._pSequence );
}

inline sal_Int8 * ByteSequence::getArray()
{
    ::rtl_byte_sequence_reference2One( &_pSequence );
    if (_pSequence == NULL)
        throw ::std::bad_alloc();
    return reinterpret_cast<sal_Int8 *>(_pSequence->elements);
}

inline void ByteSequence::realloc( sal_Int32 nSize )
{
    ::rtl_byte_sequence_realloc( &_pSequence, nSize );
    if (_pSequence == NULL)
        throw ::std::bad_alloc();
}

inline sal_Int8 & ByteSequence::operator [] ( sal_Int32 nIndex )
{
    return getArray()[ nIndex ];
}

inline bool ByteSequence::operator != ( const ByteSequence & rSeq ) const
{
    return (! operator == ( rSeq ));
}

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
