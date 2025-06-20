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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <o3tl/safeint.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <comphelper/sequence.hxx>

using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace
{
    AccessibleRelation lcl_getRelationByType( std::vector<AccessibleRelation>& raRelations, AccessibleRelationType eRelationType)
    {
        for (const auto& aRelation: raRelations)
        {
            if (aRelation.RelationType == eRelationType)
                return aRelation;
        }
        return AccessibleRelation();
    }
}

AccessibleRelationSetHelper::AccessibleRelationSetHelper ()
{
}

AccessibleRelationSetHelper::AccessibleRelationSetHelper (const AccessibleRelationSetHelper& rHelper)
    : cppu::WeakImplHelper<XAccessibleRelationSet>(rHelper),
      maRelations(rHelper.maRelations)
{
}

css::uno::Reference<css::accessibility::XAccessibleRelationSet> AccessibleRelationSetHelper::Clone() const
{
    std::scoped_lock aGuard (maMutex);
    return new AccessibleRelationSetHelper(*this);
}

AccessibleRelationSetHelper::~AccessibleRelationSetHelper()
{
}

//=====  XAccessibleRelationSet  ==============================================

    /** Returns the number of relations in this relation set.

        @return
            Returns the number of relations or zero if there are none.
    */
sal_Int32 SAL_CALL
    AccessibleRelationSetHelper::getRelationCount(  )
{
    std::scoped_lock aGuard (maMutex);

    return maRelations.size();
}

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
 AccessibleRelation SAL_CALL
        AccessibleRelationSetHelper::getRelation( sal_Int32 nIndex )
{
    std::scoped_lock aGuard (maMutex);

    if ((nIndex < 0) || (o3tl::make_unsigned(nIndex) >= maRelations.size()))
        throw lang::IndexOutOfBoundsException();

    return maRelations[nIndex];
}

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
sal_Bool SAL_CALL
    AccessibleRelationSetHelper::containsRelation(AccessibleRelationType eRelationType)
{
    std::scoped_lock aGuard (maMutex);

    AccessibleRelation defaultRelation; // default is INVALID
    AccessibleRelation relationByType = lcl_getRelationByType(maRelations, eRelationType);
    return relationByType.RelationType != defaultRelation.RelationType;
}

    /** Retrieve and return the relation with the given relation type.

        @param eRelationType
            The type of the relation to return.

        @return
            If a relation with the given type could be found, then (a copy
            of) this relation is returned.  Otherwise a relation with the
            type INVALID is returned.
    */
AccessibleRelation SAL_CALL
        AccessibleRelationSetHelper::getRelationByType(AccessibleRelationType eRelationType)
{
    std::scoped_lock aGuard (maMutex);

    return lcl_getRelationByType(maRelations, eRelationType);
}

void AccessibleRelationSetHelper::AddRelation(const AccessibleRelation& rRelation)
{
    std::scoped_lock aGuard (maMutex);

    for (auto& aRelation: maRelations)
    {
        if (aRelation.RelationType == rRelation.RelationType)
        {
            aRelation.TargetSet = comphelper::concatSequences(aRelation.TargetSet, rRelation.TargetSet);
            return;
        }
    }
    maRelations.push_back(rRelation);
}

//=====  XTypeProvider  =======================================================

uno::Sequence< css::uno::Type> AccessibleRelationSetHelper::getTypes()
{
    static const uno::Sequence< css::uno::Type> aTypes {
        cppu::UnoType<XAccessibleRelationSet>::get(),
        cppu::UnoType<lang::XTypeProvider>::get()
    };
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL AccessibleRelationSetHelper::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
