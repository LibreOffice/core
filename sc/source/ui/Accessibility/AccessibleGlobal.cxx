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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include "precompiled_sc.hxx"
#include "AccessibleGlobal.hxx"

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::std::set;

ScAccessibleStateSet::ScAccessibleStateSet()
{
}

ScAccessibleStateSet::~ScAccessibleStateSet()
{
}

// XAccessibleStateSet

sal_Bool SAL_CALL ScAccessibleStateSet::isEmpty() throw (RuntimeException)
{
    return maStates.empty();
}

sal_Bool SAL_CALL ScAccessibleStateSet::contains(sal_Int16 nState)
    throw (RuntimeException)
{
    return maStates.count(nState) != 0;
}

sal_Bool SAL_CALL ScAccessibleStateSet::containsAll(
    const Sequence<sal_Int16>& aStateSet) throw (RuntimeException)
{
    sal_Int32 n = aStateSet.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        if (!maStates.count(aStateSet[i]))
            // This state is not set.
            return false;
    }
    // All specified states are set.
    return true;
}

Sequence<sal_Int16> SAL_CALL ScAccessibleStateSet::getStates()
    throw (RuntimeException)
{
    Sequence<sal_Int16> aSeq(0);
    set<sal_Int16>::const_iterator itr = maStates.begin(), itrEnd = maStates.end();
    for (size_t i = 0; itr != itrEnd; ++itr, ++i)
    {
        aSeq.realloc(i+1);
        aSeq[i] = *itr;
    }
    return aSeq;
}

void ScAccessibleStateSet::insert(sal_Int16 nState)
{
    maStates.insert(nState);
}

void ScAccessibleStateSet::clear()
{
    maStates.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
