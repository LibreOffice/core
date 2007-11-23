/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequence.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:24:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_SHARABLE_SEQUENCE_HXX
#define INCLUDED_SHARABLE_SEQUENCE_HXX

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#include "types.hxx"
#endif
#ifndef INCLUDED_SHARABLE_ANYDATA_HXX
#include "anydata.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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
