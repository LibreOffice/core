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

#ifndef SC_ACCESSIBLEGLOBAL_HXX
#define SC_ACCESSIBLEGLOBAL_HXX

#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include "cppuhelper/implbase1.hxx"

#include <set>

/**
 * Generic XAccessibleStateSet implementation.
 */
class ScAccessibleStateSet : public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleStateSet >
{
public:
    ScAccessibleStateSet();
    virtual ~ScAccessibleStateSet();

    // XAccessibleStateSet

    virtual ::sal_Bool SAL_CALL isEmpty()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL contains(sal_Int16 nState)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL containsAll(
        const ::com::sun::star::uno::Sequence<sal_Int16>& aStateSet)
            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getStates()
        throw (::com::sun::star::uno::RuntimeException);

    // Non-UNO Methods

    void insert(sal_Int16 nState);
    void clear();

private:
    ::std::set<sal_Int16> maStates;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
