/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef INCLUDED_UUI_GETCONTINUATIONS_HXX
#define INCLUDED_UUI_GETCONTINUATIONS_HXX

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"

namespace com { namespace sun { namespace star {
    namespace task {
        class XInteractionContinuation;
    }
} } }

template< class t1 >
bool setContinuation(
    com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > const & rContinuation,
    com::sun::star::uno::Reference< t1 > * pContinuation)
{
    if (pContinuation && !pContinuation->is())
    {
        pContinuation->set(rContinuation, com::sun::star::uno::UNO_QUERY);
        if (pContinuation->is())
            return true;
    }
    return false;
}

template< class t1, class t2 >
void getContinuations(
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
                rContinuations,
    com::sun::star::uno::Reference< t1 > * pContinuation1,
    com::sun::star::uno::Reference< t2 > * pContinuation2)
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (setContinuation(rContinuations[i], pContinuation1))
            continue;
        if (setContinuation(rContinuations[i], pContinuation2))
            continue;
    }
}

template< class t1, class t2, class t3 >
void getContinuations(
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
                rContinuations,
    com::sun::star::uno::Reference< t1 > * pContinuation1,
    com::sun::star::uno::Reference< t2 > * pContinuation2,
    com::sun::star::uno::Reference< t3 > * pContinuation3)
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (setContinuation(rContinuations[i], pContinuation1))
            continue;
        if (setContinuation(rContinuations[i], pContinuation2))
            continue;
        if (setContinuation(rContinuations[i], pContinuation3))
            continue;
    }
}

template< class t1, class t2, class t3, class t4 >
void getContinuations(
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
                rContinuations,
    com::sun::star::uno::Reference< t1 > * pContinuation1,
    com::sun::star::uno::Reference< t2 > * pContinuation2,
    com::sun::star::uno::Reference< t3 > * pContinuation3,
    com::sun::star::uno::Reference< t4 > * pContinuation4)
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (setContinuation(rContinuations[i], pContinuation1))
            continue;
        if (setContinuation(rContinuations[i], pContinuation2))
            continue;
        if (setContinuation(rContinuations[i], pContinuation3))
            continue;
        if (setContinuation(rContinuations[i], pContinuation4))
            continue;
    }
}

#endif /* INCLUDED_UUI_GETCONTINUATIONS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
