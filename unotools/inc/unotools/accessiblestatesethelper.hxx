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
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#define _UTL_ACCESSIBLESTATESETHELPER_HXX_

#include "unotools/unotoolsdllapi.h"

#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/servicehelper.hxx>

class AccessibleStateSetHelperImpl;

//=========================================================================
//= XAccessibleStateSet helper classes
//=========================================================================

//... namespace utl .......................................................
namespace utl
{
//.........................................................................

/** @descr
        This base class provides an implementation of the
        <code>AccessibleStateSet</code> service.
*/
class UNOTOOLS_DLLPUBLIC AccessibleStateSetHelper
    :   public cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleStateSet
        >
{
public:
    //=====  internal  ========================================================

    AccessibleStateSetHelper ();
    /** constructs an object with some states initially set

        <p>This ctor is compatible with
        <method scope="comphelper">OAccessibleImplementationAccess::implGetForeignControlledStates</method></p>

        @param _nInitialStates
            is a bit mask. Every bit 2^n means that the state number n (as got from the
            AccessibleStateType constants) should be set initially.
    */
    AccessibleStateSetHelper ( const sal_Int64 _nInitialStates );

    AccessibleStateSetHelper ( const AccessibleStateSetHelper& rHelper );
protected:
    virtual ~AccessibleStateSetHelper   (void);
public:

    //=====  XAccessibleStateSet  ==============================================

    /** Checks whether the current state set is empty.

        @return
            Returns <TRUE/> if there is no state in this state set and
            <FALSE/> if there is at least one state set in it.
    */
    virtual sal_Bool SAL_CALL isEmpty ()
        throw (::com::sun::star::uno::RuntimeException);

    /** Checks if the given state is a member of the state set of this
        object.

        @param aState
            The state for which to check membership.  This has to be one of
            the constants of <type>AccessibleStateType</type>.

        @return
            Returns <TRUE/> if the given state is a memeber of this object's
            state set and <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL contains (sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

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
    virtual sal_Bool SAL_CALL containsAll (
        const ::com::sun::star::uno::Sequence<sal_Int16>& rStateSet)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a sequence of all states.
    */
    virtual com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getStates()
        throw (::com::sun::star::uno::RuntimeException);

    /** Adds a state to the set.
    */
    void    AddState(sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

    /** Removes a state from the set if the set contains the state, otherwise nothing is done.
    */
    void    RemoveState(sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

    /** Compares the set with the set given by rComparativeValue and puts the results
        into rOldStates and rNewStates.

        rOldStates contains after call all states which are in the own set and
        not in the comparative set.

        rNewStates contains after call all states which are in the comparative
         set and not in the own set.
    */
    sal_Bool Compare(const AccessibleStateSetHelper& rComparativeValue,
                        AccessibleStateSetHelper& rOldStates,
                        AccessibleStateSetHelper& rNewStates)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ===================================================

    /** Returns a sequence of all supported interfaces.
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Mutex guarding this object.
    ::osl::Mutex maMutex;

private:
    /// The implementation of this helper interface.
    AccessibleStateSetHelperImpl*   mpHelperImpl;
};

//.........................................................................
}
//... namespace utl .......................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
