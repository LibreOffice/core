/*************************************************************************
 *
 *  $RCSfile: byteseq.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


namespace rtl
{

//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence()
    : _pSequence( 0 )
{
    rtl_byte_sequence_construct( &_pSequence , 0 );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const ByteSequence & rSeq ) :
    _pSequence( 0 )
{
    rtl_byte_sequence_assign( &_pSequence , rSeq._pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence) :
    _pSequence( pSequence )
{
    rtl_byte_sequence_acquire( pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( const sal_Int8 * pElements, sal_Int32 len )
    : _pSequence( 0 )
{
    rtl_byte_sequence_constructFromArray( &_pSequence, pElements , len );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len, enum __ByteSequence_NoDefault value )
    : _pSequence( 0 )
{
    rtl_byte_sequence_constructNoDefault( &_pSequence , len );
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Sequence *pSequence, enum __ByteSequence_NoAcquire value )
    : _pSequence( pSequence )
{
}
//__________________________________________________________________________________________________
inline ByteSequence::ByteSequence( sal_Int32 len )
    : _pSequence( 0 )
{
    rtl_byte_sequence_construct( &_pSequence , len );
}
//__________________________________________________________________________________________________
inline ByteSequence::~ByteSequence()
{
    rtl_byte_sequence_release( _pSequence );
}
//__________________________________________________________________________________________________
inline ByteSequence & ByteSequence::operator = ( const ByteSequence & rSeq )
{
    rtl_byte_sequence_assign( &_pSequence , rSeq._pSequence );
    return *this;
}
//__________________________________________________________________________________________________
inline sal_Bool ByteSequence::operator == ( const ByteSequence & rSeq ) const
{
    if (_pSequence == rSeq._pSequence)
        return sal_True;
    if (_pSequence->nElements != rSeq._pSequence->nElements)
        return sal_False;
    return (0 == ::rtl_compareMemory( _pSequence->elements, rSeq._pSequence->elements, _pSequence->nElements ));
}
//__________________________________________________________________________________________________
inline sal_Int8 * ByteSequence::getArray()
{
    ::rtl_byte_sequence_reference2One( &_pSequence );
    return (sal_Int8 *)_pSequence->elements;
}
//__________________________________________________________________________________________________
inline void ByteSequence::realloc( sal_Int32 nSize )
{
    ::rtl_byte_sequence_realloc( &_pSequence, nSize );
}

}
#endif
