/*************************************************************************
 *
 *  $RCSfile: Sequence.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:55 $
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
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#define _COM_SUN_STAR_UNO_SEQUENCE_H_

#ifndef _CPPU_MACROS_HXX_
#include <cppu/macros.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_SEQUENCE2_H_
#include <uno/sequence2.h>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif


namespace rtl
{
class ByteSequence;
}

/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

/** Template C++ class representing an IDL sequence<>. Template argument is
    sequence element type<br>
    C++ Sequences are reference counted and shared, so the sequence keeps a handle
    to its data.
    To keep value semantics, copies are only generated if the sequence is to be modified
    (new handle).
    <br>
*/
template< class E >
class Sequence
{
    /** sequence handle<br>
    */
    uno_Sequence * _pSequence;

public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    // static pointer to typelib type
    static typelib_TypeDescriptionReference * s_pType;

    /** typedefs the element type of the sequence
        <br>
    */
    typedef E ElementType;

    /** Default constructor:
        Creates an empty sequence.
        <br>
    */
    inline Sequence< E >() SAL_THROW( () );
    /** Copy constructor:
        Creates a copy of given sequence.
        <br>
        @param rSeq another sequence of same type
    */
    inline Sequence< E >( const Sequence< E > & rSeq ) SAL_THROW( () );
    /** Constructor:
        Takes over ownership of given sequence.
        <br>
        @param pSequence a sequence
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Sequence< E >( uno_Sequence * pSequence, __sal_NoAcquire ) SAL_THROW( () );
    /** Constructor:
        Creates a copy of given elements.
        <br>
        @param pElement an array of elements
        @param len length of array
    */
    inline Sequence< E >( const E * pElements, sal_Int32 len ) SAL_THROW( () );
    /** Constructor:
        Creates a default constructed sequence of given length.
        <br>
        @param len initial sequence length
    */
    inline Sequence< E >( sal_Int32 len ) SAL_THROW( () );
    /** Destructor:
        Releases sequence handle. Last handle will destruct elements and free memory.
        <br>
    */
    inline ~Sequence< E >() SAL_THROW( () );

    /** Assignment operator:
        Acquires given sequence handle and releases previously set handle.
        <br>
        @param rSeq another sequence of same type
        @return this sequence
    */
    inline Sequence< E > & SAL_CALL operator = ( const Sequence< E > & rSeq ) SAL_THROW( () );

    /** Gets type of elements.
        <br>
        @return type of element
    */
    inline const Type & getElementType() const SAL_THROW( () )
        { return ::getCppuType( (const ElementType *)0 ); }

    /** Gets length of sequence.
        <br>
        @return length of sequence
    */
    inline sal_Int32 SAL_CALL getLength() const SAL_THROW( () )
        { return _pSequence->nElements; }
    /** Tests whether the sequence has elements, i.e. elements count is greater than zero.
        <br>
        @return true, if elements count is greater than zero
    */
    inline sal_Bool SAL_CALL hasElements() const SAL_THROW( () )
        { return (_pSequence->nElements > 0); }

    /** Gets a pointer to elements array for <b>reading</b>.
        If the sequence has a length of 0, then the returned pointer is undefined.
        <br>
        @return pointer to elements array
    */
    inline const E * SAL_CALL getConstArray() const SAL_THROW( () )
        { return reinterpret_cast< const E * >( _pSequence->elements ); }
    /** Gets a pointer to elements array for <b>reading and writing</b>.<br>
        In general if the sequence has a handle acquired by other sequences
        (reference count > 1), then a new sequence is created copy constructing
        all elements to keep value semantics!<br>
        If the sequence has a length of 0, then the returned pointer is undefined.
        <br>
        @return pointer to elements array
    */
    inline E * SAL_CALL getArray() SAL_THROW( () );

    /** Non-const index operator:
        Obtains a reference to element indexed at given position.<br>
        The implementation does <b>not</b> check for array bounds!<br>
        In general if the sequence has a handle acquired by other sequences
        (reference count > 1), then a new sequence is created copy constructing
        all elements to keep value semantics!
        <br>
        @param nIndex index
        @return non-const C++ reference to element
    */
    inline E & SAL_CALL operator [] ( sal_Int32 nIndex ) SAL_THROW( () );
    /** Const index operator:
        Obtains a reference to element indexed at given position.<br>
        The implementation does <b>not</b> check for array bounds!<br>
        <br>
        @param nIndex index
        @return const C++ reference to element
    */
    inline const E & SAL_CALL operator [] ( sal_Int32 nIndex ) const SAL_THROW( () );

    /** Equality operator:
        Compares two sequences.
        <br>
        @param rSeq another sequence of same type (right side)
        @return true if both sequences are equal, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const Sequence< E > & rSeq ) const SAL_THROW( () );
    /** Unequality operator:
        Compares two sequences.
        <br>
        @param rSeq another sequence of same type (right side)
        @return false if both sequences are equal, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const Sequence< E > & rSeq ) const SAL_THROW( () )
        { return (! operator == ( rSeq )); }

    /** Reallocates sequence to new length.
        If the new length is smaller than the former, then upper elements
        will be destructed (and their memory freed).
        If the new length is greater than the former, then upper (new) elements
        are default constructed.<br>
        If the sequence has a handle acquired by other sequences
        (reference count > 1), then the remaining elements are copy constructed
        to a new sequence handle to keep value semantics!
        <br>
        @param nSize new size of sequence
    */
    inline void SAL_CALL realloc( sal_Int32 nSize ) SAL_THROW( () );
};

/** Creates a UNO byte sequence from a SAL byte sequence.
    <br>
    @param rByteSequence a byte sequence
    @return a UNO byte sequence
*/
inline ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL toUnoSequence(
    const ::rtl::ByteSequence & rByteSequence ) SAL_THROW( () );

}
}
}
}

/** Gets the meta type of IDL sequence.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL sequence
*/
template< class S >
inline const ::com::sun::star::uno::Type &
SAL_CALL getCppuType( const ::com::sun::star::uno::Sequence< S > * ) SAL_THROW( () );

/** Gets the meta type of IDL <b>sequence< char ></b>.
    This function has been introduced due to ambiguities with unsigned short.
    <br>
    @param dummy typed pointer for function signature
    @return type of IDL <b>sequence< char ></b>
*/
inline const ::com::sun::star::uno::Type &
SAL_CALL getCharSequenceCppuType() SAL_THROW( () );

#endif
