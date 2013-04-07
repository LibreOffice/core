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

#ifndef _COMPHELPER_SEQUENCE_HXX_
#define _COMPHELPER_SEQUENCE_HXX_

#include <algorithm> // copy algorithm
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include "comphelper/comphelperdllapi.h"

#include <vector>

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;

    //-------------------------------------------------------------------------
    /** search the given string within the given sequence, return the positions where it was found.
        if _bOnlyFirst is sal_True, only the first occurrence will be returned.
    */
    COMPHELPER_DLLPUBLIC staruno::Sequence<sal_Int16> findValue(const staruno::Sequence< OUString >& _rList, const OUString& _rValue, sal_Bool _bOnlyFirst = sal_False);

    /** Checks if the name exists
     *
     * \param Value   The value to search for.
     * \param _aList  The list in which to search for the value.
     * \return <TRUE/> if the value can be found, otherwise <FALSE/>.
     */
    COMPHELPER_DLLPUBLIC sal_Bool existsValue(const OUString& Value,const ::com::sun::star::uno::Sequence< OUString >& _aList);


    //-------------------------------------------------------------------------
    namespace internal
    {
        template <class T>
        void implCopySequence(const T* _pSource, T*& _pDest, sal_Int32 _nSourceLen)
        {
            for (sal_Int32 i=0; i<_nSourceLen; ++i, ++_pSource, ++_pDest)
                *_pDest = *_pSource;
        }
    }
    //-------------------------------------------------------------------------
    /// concat two sequences
    template <class T>
     staruno::Sequence<T> concatSequences(const staruno::Sequence<T>& _rLeft, const staruno::Sequence<T>& _rRight)
    {
        sal_Int32 nLeft(_rLeft.getLength()), nRight(_rRight.getLength());
        const T* pLeft = _rLeft.getConstArray();
        const T* pRight = _rRight.getConstArray();

        sal_Int32 nReturnLen(nLeft + nRight);
        staruno::Sequence<T> aReturn(nReturnLen);
        T* pReturn = aReturn.getArray();

        internal::implCopySequence(pLeft, pReturn, nLeft);
        internal::implCopySequence(pRight, pReturn, nRight);

        return aReturn;
    }

    //-------------------------------------------------------------------------
    /// concat three sequences
    template <class T>
    staruno::Sequence<T> concatSequences(const staruno::Sequence<T>& _rLeft, const staruno::Sequence<T>& _rMiddle, const staruno::Sequence<T>& _rRight)
    {
        sal_Int32 nLeft(_rLeft.getLength()), nMiddle(_rMiddle.getLength()), nRight(_rRight.getLength());
        const T* pLeft = _rLeft.getConstArray();
        const T* pMiddle = _rMiddle.getConstArray();
        const T* pRight = _rRight.getConstArray();

        sal_Int32 nReturnLen(nLeft + nMiddle + nRight);
        staruno::Sequence<T> aReturn(nReturnLen);
        T* pReturn = aReturn.getArray();

        internal::implCopySequence(pLeft, pReturn, nLeft);
        internal::implCopySequence(pMiddle, pReturn, nMiddle);
        internal::implCopySequence(pRight, pReturn, nRight);

        return aReturn;
    }

    //-------------------------------------------------------------------------
    /// remove a specified element from a sequences
    template<class T>
    void removeElementAt(staruno::Sequence<T>& _rSeq, sal_Int32 _nPos)
    {
        sal_uInt32 nLength = _rSeq.getLength();

        OSL_ENSURE(0 <= _nPos && (sal_uInt32)_nPos < nLength, "invalid index");

        for (sal_uInt32 i = (sal_uInt32)_nPos + 1; i < nLength; ++i)
        {
            _rSeq[i-1] = _rSeq[i];
        }

        _rSeq.realloc(nLength-1);
    }

    //=====================================================================
    //= iterating through sequences
    //=====================================================================
    /** a helper class for iterating through a sequence
    */
    template <class TYPE>
    class OSequenceIterator
    {
        const TYPE* m_pElements;
        sal_Int32   m_nLen;
        const TYPE* m_pCurrent;

    public:
        /** contrcuct a sequence iterator from a sequence
        */
        OSequenceIterator(const ::com::sun::star::uno::Sequence< TYPE >& _rSeq);
        /** contrcuct a sequence iterator from a Any containing a sequence
        */
        OSequenceIterator(const ::com::sun::star::uno::Any& _rSequenceAny);

        sal_Bool hasMoreElements() const;
        ::com::sun::star::uno::Any  nextElement();

    private:
        void construct(const ::com::sun::star::uno::Sequence< TYPE >& _rSeq);
    };

    //---------------------------------------------------------------------
    template <class TYPE>
    OSequenceIterator<TYPE>::OSequenceIterator(const ::com::sun::star::uno::Sequence< TYPE >& _rSeq)
        :m_pElements(NULL)
        ,m_nLen(0)
        ,m_pCurrent(NULL)
    {
        construct(_rSeq);
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    OSequenceIterator<TYPE>::OSequenceIterator(const ::com::sun::star::uno::Any& _rSequenceAny)
        :m_pElements(NULL)
        ,m_nLen(0)
        ,m_pCurrent(NULL)
    {
        ::com::sun::star::uno::Sequence< TYPE > aContainer;
        sal_Bool bSuccess = _rSequenceAny >>= aContainer;
        OSL_ENSURE(bSuccess, "OSequenceIterator::OSequenceIterator: invalid Any!");
        (void)bSuccess;
        construct(aContainer);
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    void OSequenceIterator<TYPE>::construct(const ::com::sun::star::uno::Sequence< TYPE >& _rSeq)
    {
        m_pElements = _rSeq.getConstArray();
        m_nLen = _rSeq.getLength();
        m_pCurrent = m_pElements;
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    sal_Bool OSequenceIterator<TYPE>::hasMoreElements() const
    {
        return m_pCurrent - m_pElements < m_nLen;
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    ::com::sun::star::uno::Any OSequenceIterator<TYPE>::nextElement()
    {
        return ::com::sun::star::uno::makeAny(*m_pCurrent++);
    }

    //-------------------------------------------------------------------------
    /** Copy from a plain C/C++ array into a Sequence.

        @tpl SrcType
        Array element type. Must be assignable to DstType

        @tpl DstType
        Sequence element type. Must be assignable from SrcType

        @param i_pArray
        Valid pointer to at least num elements of type SrcType

        @param nNum
        Number of array elements to copy

        @return the resulting Sequence

        @attention when copying from e.g. a double array to a
        Sequence<int>, no proper rounding will be performed, but the
        values will be truncated. There's currently no measure to
        prevent or detect precision loss, overflow or truncation.
     */
    template < typename DstType, typename SrcType >
    ::com::sun::star::uno::Sequence< DstType > arrayToSequence( const SrcType* i_pArray, sal_Int32 nNum )
    {
        ::com::sun::star::uno::Sequence< DstType > result( nNum );
        ::std::copy( i_pArray, i_pArray+nNum, result.getArray() );
        return result;
    }

    //-------------------------------------------------------------------------
    /** Copy from a Sequence into a plain C/C++ array

        @tpl SrcType
        Sequence element type. Must be assignable to DstType

        @tpl DstType
        Array element type. Must be assignable from SrcType

        @param io_pArray
        Valid pointer to at least i_Sequence.getLength() elements of
        type DstType

        @param i_Sequence
        Reference to a Sequence of SrcType elements

        @return a pointer to the array

        @attention when copying from e.g. a Sequence<double> to an int
        array, no proper rounding will be performed, but the values
        will be truncated. There's currently no measure to prevent or
        detect precision loss, overflow or truncation.
     */
    template < typename DstType, typename SrcType >
    DstType* sequenceToArray( DstType* io_pArray, const ::com::sun::star::uno::Sequence< SrcType >& i_Sequence )
    {
        ::std::copy( i_Sequence.getConstArray(), i_Sequence.getConstArray()+i_Sequence.getLength(), io_pArray );
        return io_pArray;
    }

    //-------------------------------------------------------------------------
    /** Copy from a container into a Sequence

        @tpl SrcType
        Container type. This type must fulfill the STL container
        concept, in particular, the size(), begin() and end() methods
        must be available and have the usual semantics.

        @tpl DstType
        Sequence element type. Must be assignable from SrcType's
        elements

        @param i_Container
        Reference to the input contain with elements of type SrcType

        @return the generated Sequence

        @attention this function always performs a copy. Furthermore,
        when copying from e.g. a vector<double> to a Sequence<int>, no
        proper rounding will be performed, but the values will be
        truncated. There's currently no measure to prevent or detect
        precision loss, overflow or truncation.
     */
    template < typename DstType, typename SrcType >
    ::com::sun::star::uno::Sequence< DstType > containerToSequence( const SrcType& i_Container )
    {
        ::com::sun::star::uno::Sequence< DstType > result( i_Container.size() );
        ::std::copy( i_Container.begin(), i_Container.end(), result.getArray() );
        return result;
    }

    template <typename T>
    inline ::com::sun::star::uno::Sequence<T> containerToSequence(
        ::std::vector<T> const& v )
    {
        return ::com::sun::star::uno::Sequence<T>(
            v.empty() ? 0 : &v[0], static_cast<sal_Int32>(v.size()) );
    }

    //-------------------------------------------------------------------------
    /** Copy from a Sequence into a container

        @tpl SrcType
        Sequence element type. Must be assignable to SrcType's
        elements

        @tpl DstType
        Container type. This type must fulfill the STL container and
        sequence concepts, in particular, the begin(), end() and the
        unary constructor DstType(int) methods must be available and
        have the usual semantics.

        @param i_Sequence
        Reference to a Sequence of SrcType elements

        @return the generated container

        @attention this function always performs a copy. Furthermore,
        when copying from e.g. a Sequence<double> to a vector<int>, no
        proper rounding will be performed, but the values will be
        truncated. There's currently no measure to prevent or detect
        precision loss, overflow or truncation.
     */
    template < typename DstType, typename SrcType >
    DstType sequenceToContainer( const ::com::sun::star::uno::Sequence< SrcType >& i_Sequence )
    {
        DstType result( i_Sequence.getLength() );
        ::std::copy( i_Sequence.getConstArray(), i_Sequence.getConstArray()+i_Sequence.getLength(), result.begin() );
        return result;
    }
    //-------------------------------------------------------------------------
    /** Copy from a Sequence into an existing container

        This potentially saves a needless extra copy operation over
        the whole container, as it passes the target object by
        reference.

        @tpl SrcType
        Sequence element type. Must be assignable to SrcType's
        elements

        @tpl DstType
        Container type. This type must fulfill the STL container and
        sequence concepts, in particular, the begin(), end() and
        resize(int) methods must be available and have the usual
        semantics.

        @param o_Output
        Reference to the target container

        @param i_Sequence
        Reference to a Sequence of SrcType elements

        @return a non-const reference to the given container

        @attention this function always performs a copy. Furthermore,
        when copying from e.g. a Sequence<double> to a vector<int>, no
        proper rounding will be performed, but the values will be
        truncated. There's currently no measure to prevent or detect
        precision loss, overflow or truncation.
     */
    template < typename DstType, typename SrcType >
    DstType& sequenceToContainer( DstType& o_Output, const ::com::sun::star::uno::Sequence< SrcType >& i_Sequence )
    {
        o_Output.resize( i_Sequence.getLength() );
        ::std::copy( i_Sequence.getConstArray(), i_Sequence.getConstArray()+i_Sequence.getLength(), o_Output.begin() );
        return o_Output;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


#endif // _COMPHELPER_SEQUENCE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
