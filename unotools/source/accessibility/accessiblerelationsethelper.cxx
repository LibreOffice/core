/*************************************************************************
 *
 *  $RCSfile: accessiblerelationsethelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:47:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "unotools/accessiblerelationsethelper.hxx"

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#include <vector>
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

class AccessibleRelationSetHelperImpl
{
public:
    AccessibleRelationSetHelperImpl();
    AccessibleRelationSetHelperImpl(const AccessibleRelationSetHelperImpl& rImpl);
    ~AccessibleRelationSetHelperImpl();

    sal_Int32 getRelationCount(  )
        throw (uno::RuntimeException);
    AccessibleRelation getRelation( sal_Int32 nIndex )
            throw (lang::IndexOutOfBoundsException,
                    uno::RuntimeException);
    sal_Bool containsRelation( sal_Int16 aRelationType )
        throw (uno::RuntimeException);
    AccessibleRelation getRelationByType( sal_Int16 aRelationType )
            throw (uno::RuntimeException);
    void AddRelation(const AccessibleRelation& rRelation)
            throw (uno::RuntimeException);

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

AccessibleRelationSetHelperImpl::~AccessibleRelationSetHelperImpl()
{
}

sal_Int32 AccessibleRelationSetHelperImpl::getRelationCount(  )
    throw (uno::RuntimeException)
{
    return maRelations.size();
}

AccessibleRelation AccessibleRelationSetHelperImpl::getRelation( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    if ((nIndex < 0) || (static_cast<sal_uInt32>(nIndex) >= maRelations.size()))
        throw lang::IndexOutOfBoundsException();
    return maRelations[nIndex];
}

sal_Bool AccessibleRelationSetHelperImpl::containsRelation( sal_Int16 aRelationType )
    throw (uno::RuntimeException)
{
    sal_Int32 nCount(getRelationCount());
    sal_Int32 i(0);
    sal_Bool bFound(sal_False);
    while ((i < nCount) && !bFound)
    {
        if (maRelations[i].RelationType == aRelationType)
            bFound = sal_True;
        else
            i++;
    }
    return bFound;
}

AccessibleRelation AccessibleRelationSetHelperImpl::getRelationByType( sal_Int16 aRelationType )
    throw (uno::RuntimeException)
{
    sal_Int32 nCount(getRelationCount());
    sal_Int32 i(0);
    sal_Bool bFound(sal_False);
    while ((i < nCount) && !bFound)
    {
        if (maRelations[i].RelationType == aRelationType)
            bFound = sal_True;
        else
            i++;
    }
    return maRelations[i];
}

void AccessibleRelationSetHelperImpl::AddRelation(const AccessibleRelation& rRelation)
    throw (uno::RuntimeException)
{
    sal_Int32 nCount(getRelationCount());
    sal_Int32 i(0);
    sal_Bool bFound(sal_False);
    while ((i < nCount) && !bFound)
    {
        if (maRelations[i].RelationType == rRelation.RelationType)
            bFound = sal_True;
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
    : mpHelperImpl(NULL)
{
    mpHelperImpl = new AccessibleRelationSetHelperImpl();
}

AccessibleRelationSetHelper::AccessibleRelationSetHelper (const AccessibleRelationSetHelper& rHelper)
    : mpHelperImpl(NULL)
{
    if (rHelper.mpHelperImpl)
        mpHelperImpl = new AccessibleRelationSetHelperImpl(*rHelper.mpHelperImpl);
    else
        mpHelperImpl = new AccessibleRelationSetHelperImpl();
}

AccessibleRelationSetHelper::~AccessibleRelationSetHelper(void)
{
    delete mpHelperImpl;
}

//=====  XAccessibleRelationSet  ==============================================

    /** Returns the number of relations in this relation set.

        @return
            Returns the number of relations or zero if there are none.
    */
sal_Int32 SAL_CALL
    AccessibleRelationSetHelper::getRelationCount(  )
        throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
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
            throw (lang::IndexOutOfBoundsException,
                    uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
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
        throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
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
            throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    return mpHelperImpl->getRelationByType(aRelationType);
}

void AccessibleRelationSetHelper::AddRelation(const AccessibleRelation& rRelation)
            throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    mpHelperImpl->AddRelation(rRelation);
}

//=====  XTypeProvider  =======================================================

uno::Sequence< ::com::sun::star::uno::Type>
    AccessibleRelationSetHelper::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    const ::com::sun::star::uno::Type aTypeList[] = {
        ::getCppuType((const uno::Reference<
            XAccessibleRelationSet>*)0),
        ::getCppuType((const uno::Reference<
            lang::XTypeProvider>*)0)
        };
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>
        aTypeSequence (aTypeList, 2);
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL
    AccessibleRelationSetHelper::getImplementationId (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard (maMutex);
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ::vos::OGuard aGuard (maMutex);
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
    }
    return aId;
}
