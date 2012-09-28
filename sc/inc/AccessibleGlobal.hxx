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

#ifndef SC_ACCESSIBLEGLOBAL_HXX
#define SC_ACCESSIBLEGLOBAL_HXX

#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include "cppuhelper/implbase1.hxx"

#include <set>


/// Generic XAccessibleStateSet implementation.
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
