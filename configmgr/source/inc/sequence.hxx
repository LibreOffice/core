/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sequence.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_SHARABLE_SEQUENCE_HXX
#define INCLUDED_SHARABLE_SEQUENCE_HXX

#include "anydata.hxx"
#include <com/sun/star/uno/Sequence.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
    //-----------------------------------------------------------------------------

        sal_uInt8 * allocSequence(sal_uInt8 _aElementType, ::sal_Sequence const * _pSeqData);
        void     freeSequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq);

        ::sal_Sequence * readSequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq);
        ::com::sun::star::uno::Any readAnySequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq);
    //-----------------------------------------------------------------------------

        sal_uInt8 * allocBinary(::com::sun::star::uno::Sequence<sal_Int8> const & _aBinaryValue);
        void freeBinary(sal_uInt8 * _aSeq);

        ::com::sun::star::uno::Sequence<sal_Int8> readBinary(sal_uInt8 * _aSeq);
    //-----------------------------------------------------------------------------

        template <class ET>
        inline
        sal_uInt8 getElementTypeCode(::com::sun::star::uno::Sequence<ET> const & )
        {
            ::com::sun::star::uno::Type aElementType = ::getCppuType( static_cast<ET const *>(NULL) );
            return getTypeCode(aElementType);
        }

        template <class ET>
        sal_uInt8 * allocSequence(::com::sun::star::uno::Sequence<ET> const & _aSeq)
        {
            sal_uInt8 aTC = getElementTypeCode(_aSeq);
            ::sal_Sequence const * pSeqData = _aSeq.get();
            return allocSequence(aTC, pSeqData);
        }

        template <class ET>
        void readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, sal_uInt8 * _aSeq)
        {
            sal_uInt8 aElementType = getElementTypeCode(_rSeq);

            ::sal_Sequence * pNewSequence = readSequence(aElementType, _aSeq);

            if (!pNewSequence) return;

            ::com::sun::star::uno::Sequence<ET> aNewSequence(pNewSequence, SAL_NO_ACQUIRE);

            _rSeq = aNewSequence;
        }

        template <class ET>
        bool readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, sal_uInt8 _aElementType, sal_uInt8 * _aSeq)
        {
            if (getElementTypeCode(_rSeq) != _aElementType) return false;

            ::sal_Sequence * pNewSequence = readSequence(_aElementType, _aSeq);

            if (!pNewSequence) return false;

            ::com::sun::star::uno::Sequence<ET> aNewSequence(pNewSequence, SAL_NO_ACQUIRE);

            _rSeq = aNewSequence;

            return true;
        }
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif //  INCLUDED_SHARABLE_SEQUENCE_HXX
