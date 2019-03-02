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
#include <unotools/accessiblerelationsethelper.hxx>
#include <vector>
#include <comphelper/sequence.hxx>

using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

class AccessibleRelationSetHelperImpl
{
public:
    AccessibleRelationSetHelperImpl();
    AccessibleRelationSetHelperImpl(const AccessibleRelationSetHelperImpl& rImpl);

    /// @throws uno::RuntimeException
    sal_Int32 getRelationCount() const;
    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    AccessibleRelation const & getRelation( sal_Int32 nIndex ) const;
    /// @throws uno::RuntimeException
    bool containsRelation( sal_Int16 aRelationType ) const;
    /// @throws uno::RuntimeException
    AccessibleRelation getRelationByType( sal_Int16 aRelationType ) const;
    /// @throws uno::RuntimeException
    void AddRelation(const AccessibleRelation& rRelation);

private:
    std::vector<AccessibleRelation> maRelations;
};

AccessibleRelationSetHelperImpl::AccessibleRelationSetHelperImpl()
{
}

AccessibleRelationSetHelperImpl::AccessibleRelationSetHelperImpl(const AccessibleRelationSetHelperImpl& rImpl)
    : maRelations(rImpl.maRelations)
{
}

sal_Int32 AccessibleRelationSetHelperImpl::getRelationCount() const
{
    return maRelations.size();
}

AccessibleRelation const & AccessibleRelationSetHelperImpl::getRelation( sal_Int32 nIndex ) const
{
    if ((nIndex < 0) || (static_cast<sal_uInt32>(nIndex) >= maRelations.size()))
        throw lang::IndexOutOfBoundsException();
    return maRelations[nIndex];
}

bool AccessibleRelationSetHelperImpl::containsRelation( sal_Int16 aRelationType ) const
{
    AccessibleRelation defaultRelation; // default is INVALID
    AccessibleRelation relationByType = getRelationByType(aRelationType);
    return relationByType.RelationType != defaultRelation.RelationType;
}

AccessibleRelation AccessibleRelationSetHelperImpl::getRelationByType( sal_Int16 aRelationType ) const
{
    sal_Int32 nCount(getRelationCount());
    sal_Int32 i(0);
    while (i < nCount)
    {
        if (maRelations[i].RelationType == aRelationType)
            return maRelations[i];
        i++;
    }
    return AccessibleRelation();
}

void AccessibleRelationSetHelperImpl::AddRelation(const AccessibleRelation& rRelation)
{
    sal_Int32 nCount(getRelationCount());
    sal_Int32 i(0);
    bool bFound(false);
    while ((i < nCount) && !bFound)
    {
        if (maRelations[i].RelationType == rRelation.RelationType)
            bFound = true;
        else
            i++;
    }
    if (bFound)
        maRelations[i].TargetSet = comphelper::concatSequences(maRelations[i].TargetSet, rRelation.TargetSet);
    else
        maRelations.push_back(rRelation);
}

//=====  internal  ============================================================

AccessibleRelationSetHelper::AccessibleRelationSetHelper ()
    : mpHelperImpl(new AccessibleRelationSetHelperImpl)
{
}

AccessibleRelationSetHelper::AccessibleRelationSetHelper (const AccessibleRelationSetHelper& rHelper)
    : cppu::WeakImplHelper<XAccessibleRelationSet>(rHelper)
{
    if (rHelper.mpHelperImpl)
        mpHelperImpl.reset(new AccessibleRelationSetHelperImpl(*rHelper.mpHelperImpl));
    else
        mpHelperImpl.reset(new AccessibleRelationSetHelperImpl());
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
    osl::MutexGuard aGuard (maMutex);
    return mpHelperImpl->getRelationCount();
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
    osl::MutexGuard aGuard (maMutex);
    return mpHelperImpl->getRelation(nIndex);
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
    AccessibleRelationSetHelper::containsRelation( sal_Int16 aRelationType )
{
    osl::MutexGuard aGuard (maMutex);
    return mpHelperImpl->containsRelation(aRelationType);
}

    /** Retrieve and return the relation with the given relation type.

        @param aRelationType
            The type of the relation to return.  This has to be one of the
            constants of <type>AccessibleRelationType</type>.

        @return
            If a relation with the given type could be found than (a copy
            of) this relation is returned.  Otherwise a relation with the
            type INVALID is returned.
    */
AccessibleRelation SAL_CALL
        AccessibleRelationSetHelper::getRelationByType( sal_Int16 aRelationType )
{
    osl::MutexGuard aGuard (maMutex);
    return mpHelperImpl->getRelationByType(aRelationType);
}

void AccessibleRelationSetHelper::AddRelation(const AccessibleRelation& rRelation)
{
    osl::MutexGuard aGuard (maMutex);
    mpHelperImpl->AddRelation(rRelation);
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
