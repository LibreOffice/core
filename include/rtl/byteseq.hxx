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
#ifndef _RTL_BYTESEQ_HXX_
#define _RTL_BYTESEQ_HXX_

#include <osl/interlck.h>
#include <rtl/byteseq.h>
#include <rtl/alloc.h>

#include <new>

namespace rtl
{

//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence() SAL_THROW(())
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_construct( &_pSequence, 0 );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const ByteSequence & rSeq ) SAL_THROW(())
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence) SAL_THROW(())
    : _pSequence( pSequence )
{
    ::rtl_byte_sequence_acquire( pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const sal_Int8 * pElements, sal_Int32 len )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_constructFromArray( &_pSequence, pElements, len );
    if (_pSequence == 0)
        throw ::std::bad_alloc();
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len, enum __ByteSequence_NoDefault )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_constructNoDefault( &_pSequence, len );
    if (_pSequence == 0)
        throw ::std::bad_alloc();
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence, enum __ByteSequence_NoAcquire ) SAL_THROW(())
    : _pSequence( pSequence )
{
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_construct( &_pSequence, len );
    if (_pSequence == 0)
        throw ::std::bad_alloc();
}
//__________________________________________________________________________________________________
inline ByteSequence::~ByteSequence() SAL_THROW(())
{
    ::rtl_byte_sequence_release( _pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence & ByteSequence::operator = ( const ByteSequence & rSeq ) SAL_THROW(())
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
    return *this;
}
//__________________________________________________________________________________________________
inline sal_Bool ByteSequence::operator == ( const ByteSequence & rSeq ) const SAL_THROW(())
{
    return ::rtl_byte_sequence_equals( _pSequence, rSeq._pSequence );
}
//__________________________________________________________________________________________________
inline sal_Int8 * ByteSequence::getArray()
{
    ::rtl_byte_sequence_reference2One( &_pSequence );
    if (_pSequence == 0)
        throw ::std::bad_alloc();
    return (sal_Int8 *)_pSequence->elements;
}
//__________________________________________________________________________________________________
inline void ByteSequence::realloc( sal_Int32 nSize )
{
    ::rtl_byte_sequence_realloc( &_pSequence, nSize );
    if (_pSequence == 0)
        throw ::std::bad_alloc();
}
//__________________________________________________________________________________________________
inline sal_Int8 & ByteSequence::operator [] ( sal_Int32 nIndex )
{
    return getArray()[ nIndex ];
}
//__________________________________________________________________________________________________
inline sal_Bool ByteSequence::operator != ( const ByteSequence & rSeq ) const SAL_THROW(())
{
    return (! operator == ( rSeq ));
}

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
