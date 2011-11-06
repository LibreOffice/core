/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
