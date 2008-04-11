/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sequenceasvector.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#define _COMPHELPER_SEQUENCEASVECTOR_HXX_

//_______________________________________________
// includes

#include <vector>
#include <algorithm>
#include <com/sun/star/uno/Sequence.hxx>

#ifndef _COM_SUN_STAR_BEANS_IllegalTypeException_HPP_
#include <com/sun/star/beans/IllegalTypeException.hpp>
#endif

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// definitions

/** @short  Implements a stl vector on top of any
            uno sequence.

    @descr  That provides the possibility to modify
            sequences very easy ...
            Of course this can be usefull only, if
            count of modifications is high, so copying
            of the sequence make sense!
 */
template< class TElementType >
class SequenceAsVector : public ::std::vector< TElementType >
{
    //-------------------------------------------
    // types

    public:

        //---------------------------------------
        /** @short  When inheriting from a template using typename is generally required when using
                    types from the base! */
        typedef typename ::std::vector< TElementType >::const_iterator const_iterator;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  default ctor, to create an empty list.
         */
        SequenceAsVector()
        {}

        //---------------------------------------
        /** @short  default dtor
         */
        ~SequenceAsVector()
        {}

        //---------------------------------------
        /** @short  creates a new deque from the given uno sequence.

            @param  lSource
                    contains the new items for this deque.
         */
        SequenceAsVector(const ::com::sun::star::uno::Sequence< TElementType >& lSource)
        {
            (*this) << lSource;
        }

        //---------------------------------------
        /** @short      creates a new instance from the given Any, which
                        of course must contain a valid sequence using the
                        right element type for every item.

            @attention  If the given Any is an empty one
                        (if its set to VOID), no exception
                        is thrown. In such case this instance will
                        be created as an empty one too!

            @param      aSource
                        this any must contain a suitable sequence. :-)

            @throw      A <type scope="com::sun::star::beans">IllegalTypeException</type>
                        if an unsupported element inside this Any
                        is given. An empty Any reset this instance!
         */
        SequenceAsVector(const ::com::sun::star::uno::Any& aSource)
        {
            (*this) << aSource;
        }

        //---------------------------------------
        /** @short  fill this instance from the given uno sequence.

            @param  lSource
                    contains the new items for this deque.
         */
        void operator<<(const ::com::sun::star::uno::Sequence< TElementType >& lSource)
        {
            this->clear();

                  sal_Int32     c       = lSource.getLength();
            const TElementType* pSource = lSource.getConstArray();

            for (sal_Int32 i=0; i<c; ++i)
                push_back(pSource[i]);
        }

        //---------------------------------------
        /** @short      fill this instance from the given Any, which
                        of course must contain a valid sequence using the
                        right element type for every item.

            @attention  If the given Any is an empty one
                        (if its set to VOID), no exception
                        is thrown. In such case this instance will
                        be created as an empty one too!

            @param      aSource
                        this any must contain a suitable sequence. :-)

            @throw      A <type scope="com::sun::star::beans">IllegalTypeException</type>
                        if an unsupported element inside this Any
                        is given. An empty Any reset this instance!
         */
        void operator<<(const ::com::sun::star::uno::Any& aSource)
        {
            // An empty Any reset this instance!
            if (!aSource.hasValue())
            {
                this->clear();
                return;
            }

            ::com::sun::star::uno::Sequence< TElementType > lSource;
            if (!(aSource >>= lSource))
                throw ::com::sun::star::beans::IllegalTypeException(
                        ::rtl::OUString::createFromAscii("SequenceAsVector operator<<(Any) was called with an unsupported Any type."),
                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());

            (*this) << lSource;
        }

        //---------------------------------------
        /** @short  converts this instance to an uno sequence.

            @param  lDestination
                    target sequence for converting.
         */
        void operator>>(::com::sun::star::uno::Sequence< TElementType >& lDestination) const
        {
            sal_Int32 c = (sal_Int32)this->size();
            lDestination.realloc(c);
            TElementType* pDestination = lDestination.getArray();

            sal_Int32 i = 0;
            for (typename std::vector<TElementType>::const_iterator pThis  = this->begin();
                                                                    pThis != this->end()  ;
                                                                    ++pThis           )
            {
                pDestination[i] = *pThis;
                ++i;
            }
        }

        //---------------------------------------
        /** @short  converts this instance to an uno any
                    which contains a suitable sequence
                    of items of this stl struct.

            @param  aDestination
                    target any for converting.
         */
        void operator>>(::com::sun::star::uno::Any& aDestination) const
        {
            sal_Int32                                       c            = (sal_Int32)this->size();
            ::com::sun::star::uno::Sequence< TElementType > lDestination(c);
            TElementType*                                   pDestination = lDestination.getArray();

            sal_Int32 i = 0;
            for (typename std::vector<TElementType>::const_iterator pThis  = this->begin();
                                                                    pThis != this->end()  ;
                                                                    ++pThis           )
            {
                pDestination[i] = *pThis;
                ++i;
            }

            aDestination <<= lDestination;
        }

        //---------------------------------------
        /** @short      converts this deque to a suitable uno
                        sequence which contains all items.

            @attention  It return a const sequence to prevent
                        the outside code against using of this
                        return value as [in/]out parameter for
                        direct function calls!
                        Of course it can be casted to non const
                        ... but then its a problem of the outside
                        code :-)

            @return     A (const!) sequence, which contains all items of
                        this deque.
         */
        const ::com::sun::star::uno::Sequence< TElementType > getAsConstList() const
        {
            ::com::sun::star::uno::Sequence< TElementType > lDestination;
            (*this) >> lDestination;
            return lDestination;
        }
};

} // namespace comphelper

#endif // _COMPHELPER_SEQUENCEASVECTOR_HXX_

