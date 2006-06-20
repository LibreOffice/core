/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: byteseq.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:14:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _RTL_BYTESEQ_HXX_
#define _RTL_BYTESEQ_HXX_

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_BYTESEQ_H_
#include <rtl/byteseq.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#if ! defined EXCEPTIONS_OFF
#include <new>
#endif


namespace rtl
{

//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence() SAL_THROW( () )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_construct( &_pSequence, 0 );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const ByteSequence & rSeq ) SAL_THROW( () )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence) SAL_THROW( () )
    : _pSequence( pSequence )
{
    ::rtl_byte_sequence_acquire( pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const sal_Int8 * pElements, sal_Int32 len )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_constructFromArray( &_pSequence, pElements, len );
#if ! defined EXCEPTIONS_OFF
    if (_pSequence == 0)
        throw ::std::bad_alloc();
#endif
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len, enum __ByteSequence_NoDefault )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_constructNoDefault( &_pSequence, len );
#if ! defined EXCEPTIONS_OFF
    if (_pSequence == 0)
        throw ::std::bad_alloc();
#endif
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence, enum __ByteSequence_NoAcquire ) SAL_THROW( () )
    : _pSequence( pSequence )
{
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len )
    : _pSequence( 0 )
{
    ::rtl_byte_sequence_construct( &_pSequence, len );
#if ! defined EXCEPTIONS_OFF
    if (_pSequence == 0)
        throw ::std::bad_alloc();
#endif
}
//__________________________________________________________________________________________________
inline ByteSequence::~ByteSequence() SAL_THROW( () )
{
    ::rtl_byte_sequence_release( _pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence & ByteSequence::operator = ( const ByteSequence & rSeq ) SAL_THROW( () )
{
    ::rtl_byte_sequence_assign( &_pSequence, rSeq._pSequence );
    return *this;
}
//__________________________________________________________________________________________________
inline sal_Bool ByteSequence::operator == ( const ByteSequence & rSeq ) const SAL_THROW( () )
{
    return ::rtl_byte_sequence_equals( _pSequence, rSeq._pSequence );
}
//__________________________________________________________________________________________________
inline sal_Int8 * ByteSequence::getArray()
{
    ::rtl_byte_sequence_reference2One( &_pSequence );
#if ! defined EXCEPTIONS_OFF
    if (_pSequence == 0)
        throw ::std::bad_alloc();
#endif
    return (sal_Int8 *)_pSequence->elements;
}
//__________________________________________________________________________________________________
inline void ByteSequence::realloc( sal_Int32 nSize )
{
    ::rtl_byte_sequence_realloc( &_pSequence, nSize );
#if ! defined EXCEPTIONS_OFF
    if (_pSequence == 0)
        throw ::std::bad_alloc();
#endif
}
//__________________________________________________________________________________________________
inline sal_Int8 & ByteSequence::operator [] ( sal_Int32 nIndex )
{
    return getArray()[ nIndex ];
}
//__________________________________________________________________________________________________
inline sal_Bool ByteSequence::operator != ( const ByteSequence & rSeq ) const SAL_THROW( () )
{
    return (! operator == ( rSeq ));
}

}
#endif
