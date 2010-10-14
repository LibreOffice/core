/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _COMPHELPER_STLUNOITERATOR_HXX
#define _COMPHELPER_STLUNOITERATOR_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>


namespace comphelper
{
    /**
    @short stl-container-like access to an existing ::com::sun::star::uno::Sequence
    @descr These template functions allows using an existing
    ::com::sun::star::uno::Sequence using stl algorithms. They provides
    standard-compliant mutable random access iterators. Because random access
    iterators are the most generic iterators defined by the stl, any stl algorithm
    can be applied to the Sequence (excluding algorithms requiring output
    iterators).
    <p>
    Example: (creating a ::std::list from a ::com::sun::star::uno::Sequence)
    <code>
    ::com::sun::star::uno::Sequence<sal_Int32> aSeq(10);
    ::std::list stl_list(stl_begin(aSeq), stl_end(aSeq));
    </code>
    <p>
    Example: (sorting ::com::sun::star::uno::Sequence inplace)
    <code>
    ::com::sun::star::uno::Sequence<sal_Int32> aSeq(10);
    ::std::sort(stl_begin(aSeq), stl_seq.end(aSeq));
    </code>
    <p>
    Example: (counting occurrences of 4711 in a ::com::sun::star::uno::Sequence)
    <code>
    ::com::sun::star::uno::Sequence<sal_Int32> aSeq(10);
    sal_Int32 count = 0;
    ::std::count(stl_begin(aSeq), stl_end(aSeq), 4711, count);
    </code>
    <p>

    @see http://www.sgi.com/tech/stl/Container.html
    @see http://www.sgi.com/tech/stl/Sequence.html
    @see http://www.sgi.com/tech/stl/RandomAccessIterator.html
    */

    template <typename V>
    V* stl_begin(::com::sun::star::uno::Sequence<V>& rSeq)
        { return rSeq.getArray(); }

    template <typename V>
    V* stl_end(::com::sun::star::uno::Sequence<V>& rSeq)
        { return rSeq.getArray() + rSeq.getLength(); }

    template <typename V>
    const V* stl_begin(const ::com::sun::star::uno::Sequence<V>& rSeq)
        { return rSeq.getConstArray(); }

    template <typename V>
    const V* stl_end(const ::com::sun::star::uno::Sequence<V>& rSeq)
        { return rSeq.getConstArray() + rSeq.getLength(); }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
