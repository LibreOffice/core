/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: makesequence.hxx,v $
 * $Revision: 1.3 $
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

#if ! defined(COMPHELPER_MAKESEQUENCE_HXX_INCLUDED)
#define COMPHELPER_MAKESEQUENCE_HXX_INCLUDED

#include "com/sun/star/uno/Sequence.hxx"
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

