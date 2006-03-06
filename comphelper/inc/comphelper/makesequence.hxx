/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: makesequence.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-03-06 10:12:44 $
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

#if ! defined(COMPHELPER_MAKESEQUENCE_HXX_INCLUDED)
#define COMPHELPER_MAKESEQUENCE_HXX_INCLUDED

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/repetition.hpp"
#include "boost/preprocessor/arithmetic/add.hpp"

namespace comphelper {

/** Creates a uno::Sequence out of one parameter.
*/
template <typename T>
inline ::com::sun::star::uno::Sequence<T> makeSequence( T const& element )
{
    return ::com::sun::star::uno::Sequence<T>( &element, 1 );
}

#define COMPHELPER_MAKESEQUENCE_assign(z_, n_, unused_) \
    p[n_] = BOOST_PP_CAT(element, n_);

/** The following preprocessor repetitions generate functions like

    <pre>
        template <typename T>
        inline ::com::sun::star::uno::Sequence<T> makeSequence(
            T const& element0, T const& element1, ... );
    </pre>

    which make a sequence out of the passed elements.

    The maximum number of elements can be set by defining
    COMPHELPER_MAKESEQUENCE_MAX_ARGS; its default is 12.
*/
#define COMPHELPER_MAKESEQUENCE_make(z_, n_, unused_) \
template <typename T> \
inline ::com::sun::star::uno::Sequence<T> makeSequence( \
    BOOST_PP_ENUM_PARAMS(n_, T const& element) ) \
{ \
    ::com::sun::star::uno::Sequence<T> seq( n_ ); \
    T * p = seq.getArray(); \
    BOOST_PP_REPEAT(n_, COMPHELPER_MAKESEQUENCE_assign, ~) \
    return seq; \
}

#if ! defined(COMPHELPER_MAKESEQUENCE_MAX_ARGS)
#define COMPHELPER_MAKESEQUENCE_MAX_ARGS 12
#endif

BOOST_PP_REPEAT_FROM_TO(2, BOOST_PP_ADD(COMPHELPER_MAKESEQUENCE_MAX_ARGS, 1),
                        COMPHELPER_MAKESEQUENCE_make, ~)

#undef COMPHELPER_MAKESEQUENCE_MAX_ARGS
#undef COMPHELPER_MAKESEQUENCE_make
#undef COMPHELPER_MAKESEQUENCE_assign

} // namespace comphelper

#endif //  ! defined(COMPHELPER_MAKESEQUENCE_HXX_INCLUDED)

