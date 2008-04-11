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

#include "types.hxx"
#include "anydata.hxx"
#include <com/sun/star/uno/Sequence.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
    //-----------------------------------------------------------------------------
        typedef Vector Sequence; // alternative name
    //-----------------------------------------------------------------------------

        Sequence allocSequence(AnyData::TypeCode _aElementType, ::sal_Sequence const * _pSeqData);
        void     freeSequence(AnyData::TypeCode _aElementType, Sequence _aSeq);

        ::sal_Sequence * readSequence(AnyData::TypeCode _aElementType, Sequence _aSeq);
        ::com::sun::star::uno::Any readAnySequence(AnyData::TypeCode _aElementType, Sequence _aSeq);
    //-----------------------------------------------------------------------------

        Sequence allocBinary(::com::sun::star::uno::Sequence<sal_Int8> const & _aBinaryValue);
        void freeBinary(Sequence _aSeq);

        ::com::sun::star::uno::Sequence<sal_Int8> readBinary(Sequence _aSeq);
    //-----------------------------------------------------------------------------

        template <class ET>
        inline
        AnyData::TypeCode getElementTypeCode(::com::sun::star::uno::Sequence<ET> const & )
        {
            ::com::sun::star::uno::Type aElementType = ::getCppuType( static_cast<ET const *>(NULL) );
            return getTypeCode(aElementType);
        }

        template <class ET>
        Sequence allocSequence(::com::sun::star::uno::Sequence<ET> const & _aSeq)
        {
            AnyData::TypeCode aTC = getElementTypeCode(_aSeq);
            ::sal_Sequence const * pSeqData = _aSeq.get();
            return allocSequence(aTC, pSeqData);
        }

        template <class ET>
        void readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, Sequence _aSeq)
        {
            AnyData::TypeCode aElementType = getElementTypeCode(_rSeq);

            ::sal_Sequence * pNewSequence = readSequence(aElementType, _aSeq);

            if (!pNewSequence) return;

            ::com::sun::star::uno::Sequence<ET> aNewSequence(pNewSequence, SAL_NO_ACQUIRE);

            _rSeq = aNewSequence;
        }

        template <class ET>
        bool readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, AnyData::TypeCode _aElementType, Sequence _aSeq)
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
