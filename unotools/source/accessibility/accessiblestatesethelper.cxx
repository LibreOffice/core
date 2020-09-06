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

#include <unotools/accessiblestatesethelper.hxx>
#include <tools/debug.hxx>

// defines how many states the bitfield can contain
// it has the size of 64 because I use a uInt64
#define BITFIELDSIZE 64

using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace
{
    bool lcl_contains(sal_uInt64 aStates, sal_uInt64 aState)
    {
        DBG_ASSERT(aState < BITFIELDSIZE, "the statesset is too small");
        sal_uInt64 aTempBitSet(1);
        aTempBitSet <<= aState;
        return ((aTempBitSet & aStates) != 0);
    }
}
//=====  internal  ============================================================

AccessibleStateSetHelper::AccessibleStateSetHelper ()
    : maStates(0)
{
}

AccessibleStateSetHelper::AccessibleStateSetHelper ( const sal_Int64 _nInitialStates )
    : maStates(_nInitialStates)
{
}

AccessibleStateSetHelper::AccessibleStateSetHelper (const AccessibleStateSetHelper& rHelper)
    : cppu::WeakImplHelper<XAccessibleStateSet>(rHelper),
      maStates(rHelper.maStates)
{
}

AccessibleStateSetHelper::~AccessibleStateSetHelper()
{
}

//=====  XAccessibleStateSet  ==============================================

    /** Checks whether the current state set is empty.

        @return
            Returns <TRUE/> if there is no state in this state set and
            <FALSE/> if there is at least one state set in it.
    */
sal_Bool SAL_CALL AccessibleStateSetHelper::isEmpty ()
{
    osl::MutexGuard aGuard (maMutex);
    return maStates == 0;
}

    /** Checks if the given state is a member of the state set of this
        object.

        @param aState
            The state for which to check membership.  This has to be one of
            the constants of <type>AccessibleStateType</type>.

        @return
            Returns <TRUE/> if the given state is a member of this object's
            state set and <FALSE/> otherwise.
    */
sal_Bool SAL_CALL AccessibleStateSetHelper::contains (sal_Int16 aState)
{
    osl::MutexGuard aGuard (maMutex);

    return lcl_contains(maStates, aState);
}

    /** Checks if all of the given states are in this object's state
        set.

        @param aStateSet
            This sequence of states is interpreted as set and every of its
            members, duplicates are ignored, is checked for membership in
            this object's state set.  Each state has to be one of the
            constants of <type>AccessibleStateType</type>.

        @return
            Returns <TRUE/> if all states of the given state set are members
            of this object's state set.  <FALSE/> is returned if at least
            one of the states in the given state is not a member of this
            object's state set.
    */
sal_Bool SAL_CALL AccessibleStateSetHelper::containsAll
    (const uno::Sequence<sal_Int16>& rStateSet)
{
    osl::MutexGuard aGuard (maMutex);
    return std::all_of(rStateSet.begin(), rStateSet.end(),
        [this](const sal_Int16 nState) { return lcl_contains(maStates, nState); });
}

uno::Sequence<sal_Int16> SAL_CALL AccessibleStateSetHelper::getStates()
{
    osl::MutexGuard aGuard(maMutex);
    uno::Sequence<sal_Int16> aRet(BITFIELDSIZE);
    sal_Int16* pSeq = aRet.getArray();
    sal_Int16 nStateCount(0);
    for (sal_Int16 i = 0; i < BITFIELDSIZE; ++i)
        if (lcl_contains(maStates, i))
        {
            *pSeq = i;
            ++pSeq;
            ++nStateCount;
        }
    aRet.realloc(nStateCount);
    return aRet;
}

void AccessibleStateSetHelper::AddState(sal_Int16 aState)
{
    osl::MutexGuard aGuard (maMutex);
    DBG_ASSERT(aState < BITFIELDSIZE, "the statesset is too small");
    sal_uInt64 aTempBitSet(1);
    aTempBitSet <<= aState;
    maStates |= aTempBitSet;
}

void AccessibleStateSetHelper::RemoveState(sal_Int16 aState)
{
    osl::MutexGuard aGuard (maMutex);
    DBG_ASSERT(aState < BITFIELDSIZE, "the statesset is too small");
    sal_uInt64 aTempBitSet(1);
    aTempBitSet <<= aState;
    aTempBitSet = ~aTempBitSet;
    maStates &= aTempBitSet;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< css::uno::Type> AccessibleStateSetHelper::getTypes()
{
    css::uno::Sequence< css::uno::Type> aTypeSequence {
        cppu::UnoType<XAccessibleStateSet>::get(),
        cppu::UnoType<lang::XTypeProvider>::get()
    };
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL AccessibleStateSetHelper::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
