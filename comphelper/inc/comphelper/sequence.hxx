/*************************************************************************
 *
 *  $RCSfile: sequence.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

        OSL_ENSHURE(0 <= _nPos && _nPos < nLength, "invalid index");

        for(int i = _nPos+1; i < nLength; ++i)
        {
            _rSeq[i-1] = _rSeq[i];
        }

        _rSeq.realloc(nLength-1);
    }
//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_SEQUENCE_HXX_

