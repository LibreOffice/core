/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com::sun::star {
    namespace task {
        class XInteractionContinuation;
    }
}

template< class t1 >
bool setContinuation(
    css::uno::Reference< css::task::XInteractionContinuation > const & rContinuation,
    css::uno::Reference< t1 > * pContinuation)
{
    if (pContinuation && !pContinuation->is())
    {
        pContinuation->set(rContinuation, css::uno::UNO_QUERY);
        if (pContinuation->is())
            return true;
    }
    return false;
}

template< class t1, class t2 >
void getContinuations(
    css::uno::Sequence<
        css::uno::Reference< css::task::XInteractionContinuation > > const & rContinuations,
    css::uno::Reference< t1 > * pContinuation1,
    css::uno::Reference< t2 > * pContinuation2)
{
    for (const auto& rContinuation : rContinuations)
    {
        if (setContinuation(rContinuation, pContinuation1))
            continue;
        if (setContinuation(rContinuation, pContinuation2))
            continue;
    }
}

template< class t1, class t2, class t3 >
void getContinuations(
    css::uno::Sequence<
        css::uno::Reference< css::task::XInteractionContinuation > > const & rContinuations,
    css::uno::Reference< t1 > * pContinuation1,
    css::uno::Reference< t2 > * pContinuation2,
    css::uno::Reference< t3 > * pContinuation3)
{
    for (const auto& rContinuation : rContinuations)
    {
        if (setContinuation(rContinuation, pContinuation1))
            continue;
        if (setContinuation(rContinuation, pContinuation2))
            continue;
        if (setContinuation(rContinuation, pContinuation3))
            continue;
    }
}

template< class t1, class t2, class t3, class t4 >
void getContinuations(
    css::uno::Sequence<
        css::uno::Reference< css::task::XInteractionContinuation > > const &  rContinuations,
    css::uno::Reference< t1 > * pContinuation1,
    css::uno::Reference< t2 > * pContinuation2,
    css::uno::Reference< t3 > * pContinuation3,
    css::uno::Reference< t4 > * pContinuation4)
{
    for (const auto& rContinuation : rContinuations)
    {
        if (setContinuation(rContinuation, pContinuation1))
            continue;
        if (setContinuation(rContinuation, pContinuation2))
            continue;
        if (setContinuation(rContinuation, pContinuation3))
            continue;
        if (setContinuation(rContinuation, pContinuation4))
            continue;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
