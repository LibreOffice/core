/*************************************************************************
 *
 *  $RCSfile: sequence.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:51:14 $
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

#ifndef _COMPHELPER_SEQUENCE_HXX_
#define _COMPHELPER_SEQUENCE_HXX_

#include <algorithm> // copy algorithm

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;

    //-------------------------------------------------------------------------
    /** search the given string within the given sequence, return the positions where it was found.
        if _bOnlyFirst is sal_True, only the first occurence will be returned.
    */
    staruno::Sequence<sal_Int16> findValue(const staruno::Sequence< ::rtl::OUString >& _rList, const ::rtl::OUString& _rValue, sal_Bool _bOnlyFirst = sal_False);

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
    class SAL_NO_VTABLE IIterator
    {
    public:
        virtual sal_Bool hasMoreElements() const = 0;
        virtual ::com::sun::star::uno::Any  nextElement() = 0;
    };
    /** a helper class for iterating through a sequence
    */
    template <class TYPE>
    class OSequenceIterator : public IIterator
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

        virtual sal_Bool hasMoreElements() const;
        virtual ::com::sun::star::uno::Any  nextElement();

    protected:
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
    #ifdef DBG_UTIL
        sal_Bool bSuccess =
    #endif
        _rSequenceAny >>= aContainer;
    #ifdef DBG_UTIL
        OSL_ENSURE(bSuccess, "OSequenceIterator::OSequenceIterator: invalid Any!");
    #endif
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
    /** Copy from a plain C/C++ array into a Sequence

        @tpl SrcType
        Array element type. Must be assignable to DstType

        @tpl DstType
        Sequence element type. Must be assignable from SrcType

        @param i_pArray
        Valid pointer to at least num elements of type SrcType

        @param num
        Number of array elements to copy

        @return the resulting Sequence

        @attention when copying from e.g. a double array to a
        Sequence<int>, no proper rounding will be performed, but the
        values will be truncated. There's currently no measure to
        prevent or detect precision loss, overflow or truncation.
     */
    template < typename SrcType, typename DstType >
    ::com::sun::star::uno::Sequence< DstType > arrayToSequence( const SrcType* i_pArray, int num )
    {
        ::com::sun::star::uno::Sequence< DstType > result( num );
        ::std::copy( i_pArray, i_pArray+num, result.getArray() );
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
    template < typename SrcType, typename DstType >
    DstType* sequenceToArray( DstType* io_pArray, const ::com::sun::star::uno::Sequence< SrcType >& i_Sequence )
    {
        ::std::copy( i_Sequence.getArray(), i_Sequence.getArray()+i_Sequence.getLength(), io_pArray );
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

        @attention when copying from e.g. a vector<double> to a
        Sequence<int>, no proper rounding will be performed, but the
        values will be truncated. There's currently no measure to
        prevent or detect precision loss, overflow or truncation.
     */
    template < typename SrcType, typename DstType >
    ::com::sun::star::uno::Sequence< DstType > containerToSequence( const SrcType& i_Container )
    {
        ::com::sun::star::uno::Sequence< DstType > result( i_Container.size() );
        ::std::copy( i_Container.begin(), i_Container.end(), result.getArray() );
        return result;
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

        @return a pointer to the generated container

        @attention when copying from e.g. a Sequence<double> to a
        vector<int>, no proper rounding will be performed, but the
        values will be truncated. There's currently no measure to
        prevent or detect precision loss, overflow or truncation.
     */
    template < typename SrcType, typename DstType >
    DstType sequenceToContainer( const ::com::sun::star::uno::Sequence< SrcType >& i_Sequence )
    {
        DstType result( i_Sequence.getLength() );
        ::std::copy( i_Sequence.getArray(), i_Sequence.getArray()+i_Sequence.getLength(), result.begin() );
        return result;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_SEQUENCE_HXX_

