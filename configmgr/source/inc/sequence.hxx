/*************************************************************************
 *
 *  $RCSfile: sequence.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-02-11 14:29:07 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

        Sequence allocSequence(memory::Allocator const& _anAllocator, AnyData::TypeCode _aElementType, ::sal_Sequence const * _pSeqData);
    //    Sequence copySequence(memory::Allocator const& _anAllocator, AnyData::TypeCode _aElementType, Sequence _aSeq);
        void     freeSequence(memory::Allocator const& _anAllocator, AnyData::TypeCode _aElementType, Sequence _aSeq);

        ::sal_Sequence * readSequence(memory::Accessor const& _anAccessor, AnyData::TypeCode _aElementType, Sequence _aSeq);
        ::com::sun::star::uno::Any readAnySequence(memory::Accessor const& _anAccessor, AnyData::TypeCode _aElementType, Sequence _aSeq);
    //-----------------------------------------------------------------------------

        Sequence allocBinary(memory::Allocator const& _anAllocator, ::com::sun::star::uno::Sequence<sal_Int8> const & _aBinaryValue);
    //    Sequence copyBinary(memory::Allocator const& _anAllocator, Sequence _aSeq);
        void freeBinary(memory::Allocator const& _anAllocator, Sequence _aSeq);

        ::com::sun::star::uno::Sequence<sal_Int8> readBinary(memory::Accessor const& _anAccessor, Sequence _aSeq);
    //-----------------------------------------------------------------------------

        template <class ET>
        inline
        AnyData::TypeCode getElementTypeCode(::com::sun::star::uno::Sequence<ET> const & )
        {
            ::com::sun::star::uno::Type aElementType = ::getCppuType( static_cast<ET const *>(NULL) );
            return getTypeCode(aElementType);
        }

        template <class ET>
        Sequence allocSequence(memory::Allocator const& _anAllocator, ::com::sun::star::uno::Sequence<ET> const & _aSeq)
        {
            AnyData::TypeCode aTC = getElementTypeCode(_aSeq);
            ::sal_Sequence const * pSeqData = _aSeq.get();
            return allocSequence(_anAllocator, aTC, pSeqData);
        }

        template <class ET>
        void readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, memory::Accessor const& _anAccessor, Sequence _aSeq)
        {
            AnyData::TypeCode aElementType = getElementTypeCode(_rSeq);

            ::sal_Sequence * pNewSequence = readSequence(_anAccessor, aElementType, _aSeq);

            if (!pNewSequence) return;

            ::com::sun::star::uno::Sequence<ET> aNewSequence(pNewSequence, SAL_NO_ACQUIRE);

            _rSeq = aNewSequence;
        }

        template <class ET>
        bool readSequence(::com::sun::star::uno::Sequence<ET> & _rSeq, memory::Accessor const& _anAccessor, AnyData::TypeCode _aElementType, Sequence _aSeq)
        {
            if (getElementTypeCode(_rSeq) != _aElementType) return false;

            ::sal_Sequence * pNewSequence = readSequence(_anAccessor, _aElementType, _aSeq);

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
