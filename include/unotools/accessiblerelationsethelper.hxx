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

#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#define _UTL_ACCESSIBLERELATIONSETHELPER_HXX_

#include "unotools/unotoolsdllapi.h"

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/servicehelper.hxx>

class AccessibleRelationSetHelperImpl;

//=========================================================================
//= XAccessibleRelationSet helper classes
//=========================================================================

//... namespace utl .......................................................
namespace utl
{
//.........................................................................

/** @descr
        This base class provides an implementation of the
        <code>AccessibleRelationSet</code> service.
*/
class UNOTOOLS_DLLPUBLIC AccessibleRelationSetHelper
    :   public cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleRelationSet
        >
{
public:
    //=====  internal  ========================================================
    AccessibleRelationSetHelper ();
    AccessibleRelationSetHelper (const AccessibleRelationSetHelper& rHelper);
protected:
    virtual ~AccessibleRelationSetHelper    (void);
public:

    //=====  XAccessibleRelationSet  ==========================================

    /** Returns the number of relations in this relation set.

        @return
            Returns the number of relations or zero if there are none.
    */
    virtual sal_Int32 SAL_CALL getRelationCount(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the relation of this relation set that is specified by
        the given index.

        @param nIndex
            This index specifies the relatio to return.

        @return
            For a valid index, i.e. inside the range 0 to the number of
            relations minus one, the returned value is the requested
            relation.  If the index is invalid then the returned relation
            has the type INVALID.

    */
    virtual ::com::sun::star::accessibility::AccessibleRelation SAL_CALL
        getRelation( sal_Int32 nIndex )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                    ::com::sun::star::uno::RuntimeException);

    /** Tests whether the relation set contains a relation matching the
        specified key.

        @param aRelationType
            The type of relation to look for in this set of relations.  This
            has to be one of the constants of
            <type>AccessibleRelationType</type>.

        @return
            Returns <TRUE/> if there is a (at least one) relation of the
            given type and <FALSE/> if there is no such relation in the set.
    */
    virtual sal_Bool SAL_CALL containsRelation( sal_Int16 aRelationType )
        throw (::com::sun::star::uno::RuntimeException);

    /** Retrieve and return the relation with the given relation type.

        @param aRelationType
            The type of the relation to return.  This has to be one of the
            constants of <type>AccessibleRelationType</type>.

        @return
            If a relation with the given type could be found than (a copy
            of) this relation is returned.  Otherwise a relation with the
            type INVALID is returned.
    */
    virtual ::com::sun::star::accessibility::AccessibleRelation SAL_CALL
        getRelationByType( sal_Int16 aRelationType )
            throw (::com::sun::star::uno::RuntimeException);

    void AddRelation(
        const ::com::sun::star::accessibility::AccessibleRelation& rRelation)
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
    AccessibleRelationSetHelperImpl*    mpHelperImpl;
};

//.........................................................................
}
//... namespace utl .......................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
