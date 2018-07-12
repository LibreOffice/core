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
#ifndef INCLUDED_RTL_BYTESEQ_H
#define INCLUDED_RTL_BYTESEQ_H

#include "sal/config.h"

#include "rtl/alloc.h"
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
    /** This enum value can be used to create a bytesequence with uninitialized data
    */
    BYTESEQ_NODEFAULT = 0xcafe
};

enum __ByteSequence_NoAcquire
{
    /** This enum value can be used to create a bytesequence from a C-Handle without
        acquiring the handle.
    */
    BYTESEQ_NOACQUIRE =
#if defined _MSC_VER
        int(0xcafebabe)
#else
        0xcafebabe
#endif
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
    static void * SAL_CALL operator new ( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete ( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new ( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete ( void *, void * )
        {}
    /// @endcond

    /** Default constructor: Creates an empty sequence.
    */
    inline ByteSequence();
    /** Copy constructor: Creates a copy of given sequence.

        @param rSeq another byte sequence
    */
    inline ByteSequence( const ByteSequence & rSeq );
#if defined LIBO_INTERNAL_ONLY
    inline ByteSequence( ByteSequence && rSeq );
#endif
    /** Copy constructor Creates a copy from the C-Handle.

        @param pSequence another byte sequence handle
    */
    inline ByteSequence( sal_Sequence *pSequence );
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
        over ownership. Eitherway the handle is released by the destructor.
        This ctor is useful, when working with a c-interface (it safes a pair of
        acquire and release call and is thus a performance optimization only).

        @param pSequence sequence handle to be taken over
        @param noacquire dummy parameter forcing explicit BYTESEQ_NOACQUIRE
    */
    inline ByteSequence( sal_Sequence *pSequence , enum __ByteSequence_NoAcquire noacquire );
    /** Destructor: Releases sequence handle. Last handle will free memory.
    */
    inline ~ByteSequence();

    /** Assignment operator: Acquires given sequence handle and releases a previously set handle.

        @param rSeq another byte sequence
        @return this sequence
    */
    inline ByteSequence & SAL_CALL operator = ( const ByteSequence & rSeq );
#if defined LIBO_INTERNAL_ONLY
    inline ByteSequence & SAL_CALL operator = ( ByteSequence && rSeq );
#endif

    /** Gets the length of sequence.

        @return length of sequence
    */
    sal_Int32 SAL_CALL getLength() const
        { return _pSequence->nElements; }

    /** Gets a pointer to byte array for READING. If the sequence has a length of 0, then the
        returned pointer is undefined.

        @return pointer to byte array
    */
    const sal_Int8 * SAL_CALL getConstArray() const
        { return reinterpret_cast<sal_Int8 *>(_pSequence->elements); }
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
        @return const C++ reference to byte at element of index nIndex
    */
    const sal_Int8 & SAL_CALL operator [] ( sal_Int32 nIndex ) const
        { return getConstArray()[ nIndex ]; }

    /** Equality operator: Compares two sequences.

        @param rSeq another byte sequence (right side)
        @return true if both sequences are equal, false otherwise
    */
    inline bool SAL_CALL operator == ( const ByteSequence & rSeq ) const;
    /** Unequality operator: Compares two sequences.

        @param rSeq another byte sequence (right side)
        @return false if both sequences are equal, true otherwise
    */
    inline bool SAL_CALL operator != ( const ByteSequence & rSeq ) const;

    /** Reallocates sequence to new length. If the sequence has a handle acquired by other sequences
        (reference count > 1), then the remaining elements are copied to a new sequence handle to
        keep value semantics!

        @param nSize new size of sequence
    */
    inline void SAL_CALL realloc( sal_Int32 nSize );

    /** Returns the UNnacquired C handle of the sequence

        @return UNacquired handle of the sequence
    */
    sal_Sequence * SAL_CALL getHandle() const
        { return _pSequence; }
    /** Returns the UNnacquired C handle of the sequence (for compatibility reasons)

        @return UNacquired handle of the sequence
    */
    sal_Sequence * SAL_CALL get() const
        { return _pSequence; }
};

}
#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
