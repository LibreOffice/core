/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: groupobjects.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "groupobjects.hxx"
#include "comphelper/sequence.hxx"

//........................................................................
namespace configmgr
{

//==========================================================================
//= OInnerGroupInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL OInnerGroupInfo::acquire( ) throw ()
{
    BasicInnerElement::acquire();
}

void SAL_CALL OInnerGroupInfo::release( ) throw ()
{
    BasicInnerElement::release();
}

// XInterface joining
uno::Any SAL_CALL OInnerGroupInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicInnerElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroupAccess::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OInnerGroupInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicInnerElement::getTypes(), BasicGroupAccess::getTypes(), BasicPropertySet::getTypes());
}

uno::Sequence< sal_Int8 > SAL_CALL OInnerGroupInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicInnerElement::getImplementationId();
}


configapi::NodeAccess&  OInnerGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& OInnerGroupInfo::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* OInnerGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

configapi::InnerElement& OInnerGroupInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OInnerGroupUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OInnerGroupUpdate::acquire( ) throw ()
{
    BasicInnerElement::acquire();
}

void SAL_CALL OInnerGroupUpdate::release( ) throw ()
{
    BasicInnerElement::release();
}

// XInterface joining
uno::Any SAL_CALL OInnerGroupUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicInnerElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroup::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OInnerGroupUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicInnerElement::getTypes(), BasicGroup::getTypes(), BasicPropertySet::getTypes());
}

uno::Sequence< sal_Int8 > SAL_CALL OInnerGroupUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicInnerElement::getImplementationId();
}


configapi::NodeAccess&  OInnerGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& OInnerGroupUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* OInnerGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::InnerElement& OInnerGroupUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OSetElementGroupInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL OSetElementGroupInfo::acquire( ) throw ()
{
    BasicSetElement::acquire();
}

void SAL_CALL OSetElementGroupInfo::release( ) throw ()
{
    BasicSetElement::release();
}

// XInterface joining
uno::Any SAL_CALL OSetElementGroupInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroupAccess::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OSetElementGroupInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicSetElement::getTypes(), BasicGroupAccess::getTypes(), BasicPropertySet::getTypes() );
}

uno::Sequence< sal_Int8 > SAL_CALL OSetElementGroupInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicSetElement::getImplementationId();
}


configapi::NodeAccess&  OSetElementGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& OSetElementGroupInfo::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* OSetElementGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

configapi::SetElement& OSetElementGroupInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OSetElementGroupUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OSetElementGroupUpdate::acquire( ) throw ()
{
    BasicSetElement::acquire();
}

void SAL_CALL OSetElementGroupUpdate::release( ) throw ()
{
    BasicSetElement::release();
}

// XInterface joining
uno::Any SAL_CALL OSetElementGroupUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroup::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OSetElementGroupUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicSetElement::getTypes(), BasicGroup::getTypes(), BasicPropertySet::getTypes());
}

uno::Sequence< sal_Int8 > SAL_CALL OSetElementGroupUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicSetElement::getImplementationId();
}


configapi::NodeAccess&  OSetElementGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& OSetElementGroupUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* OSetElementGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::SetElement& OSetElementGroupUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= ORootElementGroupInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL ORootElementGroupInfo::acquire( ) throw ()
{
    BasicRootElement::acquire();
}

void SAL_CALL ORootElementGroupInfo::release( ) throw ()
{
    BasicRootElement::release();
}

// XInterface joining
uno::Any SAL_CALL ORootElementGroupInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicRootElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroupAccess::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL ORootElementGroupInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicRootElement::getTypes(), BasicGroupAccess::getTypes(), BasicPropertySet::getTypes());
}

uno::Sequence< sal_Int8 > SAL_CALL ORootElementGroupInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicRootElement::getImplementationId();
}


configapi::NodeAccess&  ORootElementGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& ORootElementGroupInfo::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* ORootElementGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

configapi::RootElement& ORootElementGroupInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= ORootElementGroupUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL ORootElementGroupUpdate::acquire( ) throw ()
{
    BasicUpdateElement::acquire();
}

void SAL_CALL ORootElementGroupUpdate::release( ) throw ()
{
    BasicUpdateElement::release();
}

// XInterface joining
uno::Any SAL_CALL ORootElementGroupUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicUpdateElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicGroup::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicPropertySet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL ORootElementGroupUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicUpdateElement::getTypes(),BasicGroup::getTypes(), BasicPropertySet::getTypes());
}

uno::Sequence< sal_Int8 > SAL_CALL ORootElementGroupUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicUpdateElement::getImplementationId();
}


configapi::NodeAccess&  ORootElementGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeGroupInfoAccess& ORootElementGroupUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeGroupAccess* ORootElementGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::UpdateRootElement& ORootElementGroupUpdate::getElementClass()
{
    return m_aAccessElement;
}

//........................................................................
} // namespace configmgr
//........................................................................

