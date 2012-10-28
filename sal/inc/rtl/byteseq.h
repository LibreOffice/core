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
#ifndef _RTL_BYTESEQ_H_
#define _RTL_BYTESEQ_H_

#include "sal/config.h"

#include "rtl/alloc.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Assures that the reference count of the given byte sequence is one. Otherwise a new copy
    of the sequence is created with a reference count of one.

    @param ppSequence sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_reference2One(
    sal_Sequence ** ppSequence )
    SAL_THROW_EXTERN_C();

/** Reallocates length of byte sequence.

    @param ppSequence sequence
    @param nSize new size of sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_realloc(
    sal_Sequence ** ppSequence, sal_Int32 nSize )
    SAL_THROW_EXTERN_C();

/** Acquires the byte sequence

    @param pSequence sequence, that is to be acquired
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_acquire(
    sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C();

/** Releases the byte sequence. If the refcount drops to zero, the sequence is freed.

    @param pSequence sequence, that is to be released; invalid after call
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_release(
    sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C();

/** Constructs a bytes sequence with length nLength. All bytes are set to zero.

    @param ppSequence inout sequence; on entry *ppSequence may be null, otherwise it is released;
                      after the call, *ppSequence contains the newly constructed sequence
    @param nLength    length of new sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_construct(
    sal_Sequence **ppSequence , sal_Int32 nLength )
    SAL_THROW_EXTERN_C();

/** Constructs a bytes sequence with length nLength. The data is not initialized.

    @param ppSequence inout sequence; on entry *ppSequence may be null, otherwise it is released;
                      after the call, *ppSequence contains the newly constructed sequence
    @param nLength    length of new sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_constructNoDefault(
    sal_Sequence **ppSequence , sal_Int32 nLength )
    SAL_THROW_EXTERN_C();

/** Constructs a byte sequence with length nLength and copies nLength bytes from pData.

    @param ppSequence inout sequence; on entry *ppSequence may be null, otherwise it is released;
                      after the call, *ppSequence contains the newly constructed sequence
    @param pData      initial data
    @param nLength    length of new sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_constructFromArray(
    sal_Sequence **ppSequence, const sal_Int8 *pData , sal_Int32 nLength )
    SAL_THROW_EXTERN_C();

/** Assigns the byte sequence pSequence to *ppSequence.

    @param ppSequence inout sequence; on entry *ppSequence may be null, otherwise it is released;
                      after the call, *ppSequence references pSequence
    @param pSequence  the source sequence
*/
SAL_DLLPUBLIC void SAL_CALL rtl_byte_sequence_assign(
    sal_Sequence **ppSequence , sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C();

/** Compares two byte sequences.

    @return true, if the data within the sequences are identical; false otherwise
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_byte_sequence_equals(
    sal_Sequence *pSequence1 , sal_Sequence *pSequence2 )
    SAL_THROW_EXTERN_C();

/** Returns the data array pointer of the sequence.

    @return read-pointer to the data array of the sequence. If rtl_byte_sequence_reference2One()
            has been called before, the pointer may be casted to a non const pointer and
            the sequence may be modified
*/
SAL_DLLPUBLIC const sal_Int8 *SAL_CALL rtl_byte_sequence_getConstArray(
    sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C();

/** Returns the length of the sequence

    @param pSequence sequence handle
    @return length of the sequence
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_byte_sequence_getLength(
    sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
namespace rtl
{

enum __ByteSequence_NoDefault
{
    /** This enum value can be used to create a bytesequence with uninitalized data
    */
    BYTESEQ_NODEFAULT = 0xcafe
};

enum __ByteSequence_NoAcquire
{
    /** This enum value can be used to create a bytesequence from a C-Handle without
        acquiring the handle.
    */
    BYTESEQ_NOACQUIRE = 0xcafebabe
};

/** C++ class representing a SAL byte sequence.
    C++ Sequences are reference counted and shared, so the sequence keeps a handle to its data.
    To keep value semantics, copies are only generated if the sequence is to be modified
    (new handle).
*/
class SAL_WARN_UNUSED ByteSequence
{
    /** sequence handle
    */
    sal_Sequence * _pSequence;

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new ( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new ( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

    /** Default constructor: Creates an empty sequence.
    */
    inline ByteSequence() SAL_THROW(());
    /** Copy constructor: Creates a copy of given sequence.

        @param rSeq another byte sequence
    */
    inline ByteSequence( const ByteSequence & rSeq ) SAL_THROW(());
    /** Copy constructor Creates a copy from the C-Handle.

        @param pSequence another byte sequence handle
    */
    inline ByteSequence( sal_Sequence *pSequence ) SAL_THROW(());
    /** Constructor: Creates a copy of given data bytes.

        @param pElements an array of bytes
        @param len number of bytes
    */
    inline ByteSequence( const sal_Int8 * pElements, sal_Int32 len );
    /** Constructor: Creates sequence of given length and initializes all bytes to 0.

        @param len initial sequence length
    */
    inline ByteSequence( sal_Int32 len );
    /** Constructor: Creates sequence of given length and does NOT initialize data.
                     Use this ctor for performance optimization only.

        @param len initial sequence length
        @param nodefault dummy parameter forcing explicit BYTESEQ_NODEFAULT
    */
    inline ByteSequence( sal_Int32 len , enum __ByteSequence_NoDefault nodefault );
    /** Constructor:
        Creates a sequence from a C-Handle without acquiring the handle, thus taking
        over owenership. Eitherway the handle is release by the destructor.
        This ctor is useful, when working with a c-interface (it safes a pair of
        acquire and release call and is thus a performance optimization only).

        @param pSequence sequence handle to be taken over
        @param noacquire dummy parameter forcing explicit BYTESEQ_NOACQUIRE
    */
    inline ByteSequence( sal_Sequence *pSequence , enum __ByteSequence_NoAcquire noacquire ) SAL_THROW(());
    /** Destructor: Releases sequence handle. Last handle will free memory.
    */
    inline ~ByteSequence() SAL_THROW(());

    /** Assignment operator: Acquires given sequence handle and releases a previously set handle.

        @param rSeq another byte sequence
        @return this sequence
    */
    inline ByteSequence & SAL_CALL operator = ( const ByteSequence & rSeq ) SAL_THROW(());

    /** Gets the length of sequence.

        @return length of sequence
    */
    inline sal_Int32 SAL_CALL getLength() const SAL_THROW(())
        { return _pSequence->nElements; }

    /** Gets a pointer to byte array for READING. If the sequence has a length of 0, then the
        returned pointer is undefined.

        @return pointer to byte array
    */
    inline const sal_Int8 * SAL_CALL getConstArray() const SAL_THROW(())
        { return (const sal_Int8 *)_pSequence->elements; }
    /** Gets a pointer to elements array for READING AND WRITING. In general if the sequence
        has a handle acquired by other sequences (reference count > 1), then a new sequence is
        created copying all bytes to keep value semantics!
        If the sequence has a length of 0, then the returned pointer is undefined.

        @return pointer to elements array
    */
    inline sal_Int8 * SAL_CALL getArray();

    /** Non-const index operator:
        Obtains a reference to byte indexed at given position.
        In general if the sequence has a handle acquired by other
        sequences (reference count > 1), then a new sequence is created
        copying all bytes to keep value semantics!

        @attention
        The implementation does NOT check for array bounds!

        @param nIndex index
        @return non-const C++ reference to element at index nIndex
    */
    inline sal_Int8 & SAL_CALL operator [] ( sal_Int32 nIndex );

    /** Const index operator: Obtains a reference to byte indexed at given position.
                              The implementation does NOT check for array bounds!

        @param nIndex index
        @return const C++ reference to byte at element of indenx nIndex
    */
    inline const sal_Int8 & SAL_CALL operator [] ( sal_Int32 nIndex ) const SAL_THROW(())
        { return getConstArray()[ nIndex ]; }

    /** Equality operator: Compares two sequences.

        @param rSeq another byte sequence (right side)
        @return true if both sequences are equal, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const ByteSequence & rSeq ) const SAL_THROW(());
    /** Unequality operator: Compares two sequences.

        @param rSeq another byte sequence (right side)
        @return false if both sequences are equal, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const ByteSequence & rSeq ) const SAL_THROW(());

    /** Reallocates sequence to new length. If the sequence has a handle acquired by other sequences
        (reference count > 1), then the remaining elements are copied to a new sequence handle to
        keep value semantics!

        @param nSize new size of sequence
    */
    inline void SAL_CALL realloc( sal_Int32 nSize );

    /** Returns the UNnacquired C handle of the sequence

        @return UNacquired handle of the sequence
    */
    inline sal_Sequence * SAL_CALL getHandle() const SAL_THROW(())
        { return _pSequence; }
    /** Returns the UNnacquired C handle of the sequence (for compatibility reasons)

        @return UNacquired handle of the sequence
    */
    inline sal_Sequence * SAL_CALL get() const SAL_THROW(())
        { return _pSequence; }
};

}
#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
