/*************************************************************************
 *
 *  $RCSfile: byteseq.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-16 09:48:21 $
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
#ifndef _RTL_BYTESEQ_H_
#define _RTL_BYTESEQ_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** Assures that the reference count of the given byte sequence is one.
    Otherwise a new copy of the sequence is created with a reference count of one.
    <br>
    @param ppSequence sequence
*/
void SAL_CALL rtl_byte_sequence_reference2One(
    sal_Sequence ** ppSequence );

/** Reallocates length of byte sequence.
    <br>
    @param ppSequence sequence
    @param nSize new size of sequence
*/
void SAL_CALL rtl_byte_sequence_realloc(
    sal_Sequence ** ppSequence, sal_Int32 nSize );

/** Acquires the byte sequence <br>
    @param pSequence sequence, that shall be acquired.
 */
void SAL_CALL rtl_byte_sequence_acquire( sal_Sequence *pSequence );

/** Releases the byte sequence. If the refcount drops to zero,
 *  the sequence is freed.<br>

    @param pSequence sequence, that shall be released. Invalid after call.
 */
void SAL_CALL rtl_byte_sequence_release( sal_Sequence *pSequence );

/** Constructs a bytes sequence with length nLength. All bytes are set to zero.
 *
 * @param On entry *ppSequence may be null , otherwise it is released.
 *        After the call, ppSequence contains the newly constructed sequence.
 *
 *
 **/
void SAL_CALL rtl_byte_sequence_construct( sal_Sequence **ppSequence , sal_Int32 nLength );

/** Constructs a bytes sequence with length nLength. The data is not initialized.
 *
 * @param ppSequence contains the newly constructed sequence.
 *        *ppSequence is released on entry if needed.
 *
 **/
void SAL_CALL rtl_byte_sequence_constructNoDefault(
    sal_Sequence **ppSequence , sal_Int32 nLength );

/** Constructs a byte sequence with length nLength and copies nLength bytes from pData.
 *
 * @param ppSequence contains the newly constructed sequence.
 *        *ppSequence is released on entry if needed.
 **/
void SAL_CALL rtl_byte_sequence_constructFromArray(
    sal_Sequence **ppSequence, const sal_Int8 *pData , sal_Int32 nLength );

/** Assigns the byte sequence pSequence to ppSequence.
 *
 * @param On entry *ppSequence may be null , otherwise it is released.
 *        ppSequence contains after the call a copy of pSequence.
 * @param pSequence the source sequence
 **/
void SAL_CALL rtl_byte_sequence_assign( sal_Sequence **ppSequence , sal_Sequence *pSequence );

/** Compares two byte sequences.
 *
 * @return sal_False, if the data within the sequences are different. <br>
 *         sal_True, if the data within the sequences are identical <br>
 **/
sal_Bool SAL_CALL rtl_byte_sequence_equals( sal_Sequence *pSequence1 , sal_Sequence *pSequence2 );


/** Returns the data pointer of the sequence.
 *
 * @return const pointer to the data of the sequence. If rtl_byte_sequence_reference2One
 *         has been called before, the pointer may be casted to a non const pointer and
 *         the sequence may be modified.
 **/
const sal_Int8 *SAL_CALL rtl_byte_sequence_getConstArray( sal_Sequence *pSequence );

/** Returns the length of the sequence
 *
 *
 **/
sal_Int32 SAL_CALL rtl_byte_sequence_getLength( sal_Sequence *pSequence );

#ifdef __cplusplus
}
namespace rtl
{

enum __ByteSequence_NoDefault
{
    /** This enum value can be used to create a bytesequence with uninitalized data.
        <br>
    */
    BYTESEQ_NODEFAULT = 0xcafe
};

enum __ByteSequence_NoAcquire
{
    /** This enum value can be used to create a bytesequence from a C-Handle without
        acquiring the handle.
        <br>
    */
    BYTESEQ_NOACQUIRE = 0xcafebabe
};
/** C++ class representing a SAL byte sequence.<br>
    C++ Sequences are reference counted and shared, so the sequence keeps a handle
    to its data.
    To keep value semantics, copies are only generated if the sequence is to be modified
    (new handle).
    <br>
*/
class ByteSequence
{
    /** sequence handle<br>
    */
    sal_Sequence * _pSequence;

public:
    /** Default constructor:
        Creates an empty sequence.
        <br>
    */
    inline ByteSequence();
    /** Copy constructor:
        Creates a copy of given sequence.
        <br>
        @param rSeq another byte sequence
    */
    inline ByteSequence( const ByteSequence & rSeq );
    /** Copy constructor
        Creates a copy from the C-Handle.
    */
    inline ByteSequence( sal_Sequence *pSequence );
    /** Constructor:
        Creates a copy of given bytes.
        <br>
        @param pElement an array of bytes
        @param len number of bytes
    */
    inline ByteSequence( const sal_Int8 * pElements, sal_Int32 len );
    /** Constructor:
        Creates sequence of given length and initializes all bytes to 0.
        <br>
        @param len initial sequence length
    */
    inline ByteSequence( sal_Int32 len );
    /** Constructor:
        Creates sequence of given length and does NOT initialize data. Use
        this ctor for performance optimization only.
        <br>
        @param len initial sequence length
        @param dummy parameter
    */
    inline ByteSequence( sal_Int32 len , enum __ByteSequence_NoDefault nodefault );
    /** Constructor:
        Creates a sequence from a C-Handle without acquiring the handle. Eitherway
        the handle is release by the destructor. This ctor is useful, when working
        with a c-interface (it safes a pair of acquire and release call and is thus
        a performance optimization only ).
        <br>
    */
    inline ByteSequence( sal_Sequence *pSequence , enum __ByteSequence_NoAcquire noacquire );
    /** Destructor:
        Releases sequence handle. Last handle will free memory.
        <br>
    */
    inline ~ByteSequence();

    /** Assignment operator:
        Acquires given sequence handle and releases previously set handle.
        <br>
        @param rSeq another byte sequence
        @return this sequence
    */
    inline ByteSequence & SAL_CALL operator = ( const ByteSequence & rSeq );

    /** Gets length of sequence.
        <br>
        @return length of sequence
    */
    inline sal_Int32 SAL_CALL getLength() const
        { return _pSequence->nElements; }

    /** Gets a pointer to byte array for <b>reading</b>.
        If the sequence has a length of 0, then the returned pointer is undefined.
        <br>
        @return pointer to byte array
    */
    inline const sal_Int8 * SAL_CALL getConstArray() const
        { return (const sal_Int8 *)_pSequence->elements; }
    /** Gets a pointer to elements array for <b>reading and writing</b>.<br>
        In general if the sequence has a handle acquired by other sequences
        (reference count > 1), then a new sequence is created copying
        all bytes to keep value semantics!<br>
        If the sequence has a length of 0, then the returned pointer is undefined.
        <br>
        @return pointer to elements array
    */
    inline sal_Int8 * SAL_CALL getArray();

    /** Non-const index operator:
        Obtains a reference to byte indexed at given position.<br>
        The implementation does <b>not</b> check for array bounds!<br>
        In general if the sequence has a handle acquired by other sequences
        (reference count > 1), then a new sequence is created copying
        all bytes to keep value semantics!
        <br>
        @param nIndex index
        @return non-const C++ reference to element
    */
    inline sal_Int8 & SAL_CALL operator [] ( sal_Int32 nIndex )
        { return getArray()[ nIndex ]; }
    /** Const index operator:
        Obtains a reference to byte indexed at given position.<br>
        The implementation does <b>not</b> check for array bounds!<br>
        <br>
        @param nIndex index
        @return const C++ reference to byte
    */
    inline const sal_Int8 & SAL_CALL operator [] ( sal_Int32 nIndex ) const
        { return getConstArray()[ nIndex ]; }

    /** Equality operator:
        Compares two sequences.
        <br>
        @param rSeq another byte sequence (right side)
        @return true if both sequences are equal, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const ByteSequence & rSeq ) const;
    /** Unequality operator:
        Compares two sequences.
        <br>
        @param rSeq another byte sequence (right side)
        @return false if both sequences are equal, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const ByteSequence & rSeq ) const
        { return (! operator == ( rSeq )); }

    /** Reallocates sequence to new length.
        If the sequence has a handle acquired by other sequences
        (reference count > 1), then the remaining elements are copied
        to a new sequence handle to keep value semantics!
        <br>
        @param nSize new size of sequence
    */
    inline void SAL_CALL realloc( sal_Int32 nSize );

    /** Returns the (unacquired) C-Handle of the sequence
     *
     **/
    inline sal_Sequence * SAL_CALL getHandle() const
        { return _pSequence; }
};

}
#endif
#endif
