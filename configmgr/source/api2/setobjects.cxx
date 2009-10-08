/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setobjects.cxx,v $
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
#include "setobjects.hxx"
#include "comphelper/sequence.hxx"

//........................................................................
namespace configmgr
{

//==========================================================================
//= OInnerSetInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL OInnerSetInfo::acquire( ) throw ()
{
    BasicInnerElement::acquire();
}

void SAL_CALL OInnerSetInfo::release( ) throw ()
{
    BasicInnerElement::release();
}

// XInterface joining
uno::Any SAL_CALL OInnerSetInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicInnerElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSetAccess::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OInnerSetInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicInnerElement::getTypes(),BasicSetAccess::getTypes() );
}

uno::Sequence< sal_Int8 > SAL_CALL OInnerSetInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicInnerElement::getImplementationId();
}


configapi::NodeAccess&  OInnerSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OInnerSetInfo::getNode()
{
    return m_aAccessElement;
}

configapi::InnerElement& OInnerSetInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OInnerTreeSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OInnerTreeSetUpdate::acquire( ) throw ()
{
    BasicInnerElement::acquire();
}

void SAL_CALL OInnerTreeSetUpdate::release( ) throw ()
{
    BasicInnerElement::release();
}

// XInterface joining
uno::Any SAL_CALL OInnerTreeSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicInnerElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OInnerTreeSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicInnerElement::getTypes(),BasicSet::getTypes()   );
}

uno::Sequence< sal_Int8 > SAL_CALL OInnerTreeSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicInnerElement::getImplementationId();
}


configapi::NodeAccess&  OInnerTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OInnerTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeTreeSetAccess*   OInnerTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::InnerElement& OInnerTreeSetUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OInnerValueSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OInnerValueSetUpdate::acquire( ) throw ()
{
    BasicInnerElement::acquire();
}

void SAL_CALL OInnerValueSetUpdate::release( ) throw ()
{
    BasicInnerElement::release();
}

// XInterface joining
uno::Any SAL_CALL OInnerValueSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicInnerElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicValueSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OInnerValueSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicInnerElement::getTypes(),BasicValueSet::getTypes()  );
}

uno::Sequence< sal_Int8 > SAL_CALL OInnerValueSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicInnerElement::getImplementationId();
}


configapi::NodeAccess&  OInnerValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OInnerValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeValueSetAccess*  OInnerValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::InnerElement& OInnerValueSetUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OSetElementSetInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL OSetElementSetInfo::acquire( ) throw ()
{
    BasicSetElement::acquire();
}

void SAL_CALL OSetElementSetInfo::release( ) throw ()
{
    BasicSetElement::release();
}

// XInterface joining
uno::Any SAL_CALL OSetElementSetInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSetAccess::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OSetElementSetInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicSetElement::getTypes(),BasicSetAccess::getTypes()   );
}

uno::Sequence< sal_Int8 > SAL_CALL OSetElementSetInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicSetElement::getImplementationId();
}


configapi::NodeAccess&  OSetElementSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OSetElementSetInfo::getNode()
{
    return m_aAccessElement;
}

configapi::SetElement& OSetElementSetInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OSetElementTreeSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OSetElementTreeSetUpdate::acquire( ) throw ()
{
    BasicSetElement::acquire();
}

void SAL_CALL OSetElementTreeSetUpdate::release( ) throw ()
{
    BasicSetElement::release();
}

// XInterface joining
uno::Any SAL_CALL OSetElementTreeSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OSetElementTreeSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicSetElement::getTypes(),BasicSet::getTypes() );
}

uno::Sequence< sal_Int8 > SAL_CALL OSetElementTreeSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicSetElement::getImplementationId();
}


configapi::NodeAccess&  OSetElementTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OSetElementTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeTreeSetAccess*   OSetElementTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::SetElement& OSetElementTreeSetUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= OSetElementValueSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL OSetElementValueSetUpdate::acquire( ) throw ()
{
    BasicSetElement::acquire();
}

void SAL_CALL OSetElementValueSetUpdate::release( ) throw ()
{
    BasicSetElement::release();
}

// XInterface joining
uno::Any SAL_CALL OSetElementValueSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicValueSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL OSetElementValueSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicSetElement::getTypes(),BasicValueSet::getTypes()    );
}

uno::Sequence< sal_Int8 > SAL_CALL OSetElementValueSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicSetElement::getImplementationId();
}


configapi::NodeAccess&  OSetElementValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& OSetElementValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeValueSetAccess*  OSetElementValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::SetElement& OSetElementValueSetUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= ORootElementSetInfo
//==========================================================================

// XInterface refcounting
void SAL_CALL ORootElementSetInfo::acquire( ) throw ()
{
    BasicRootElement::acquire();
}

void SAL_CALL ORootElementSetInfo::release( ) throw ()
{
    BasicRootElement::release();
}

// XInterface joining
uno::Any SAL_CALL ORootElementSetInfo::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicRootElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSetAccess::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL ORootElementSetInfo::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicRootElement::getTypes(),BasicSetAccess::getTypes()  );
}

uno::Sequence< sal_Int8 > SAL_CALL ORootElementSetInfo::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicRootElement::getImplementationId();
}


configapi::NodeAccess&  ORootElementSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& ORootElementSetInfo::getNode()
{
    return m_aAccessElement;
}

configapi::RootElement& ORootElementSetInfo::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= ORootElementTreeSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL ORootElementTreeSetUpdate::acquire( ) throw ()
{
    BasicUpdateElement::acquire();
}

void SAL_CALL ORootElementTreeSetUpdate::release( ) throw ()
{
    BasicUpdateElement::release();
}

// XInterface joining
uno::Any SAL_CALL ORootElementTreeSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicUpdateElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL ORootElementTreeSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicUpdateElement::getTypes(),BasicSet::getTypes()  );
}

uno::Sequence< sal_Int8 > SAL_CALL ORootElementTreeSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicUpdateElement::getImplementationId();
}


configapi::NodeAccess&  ORootElementTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& ORootElementTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeTreeSetAccess*   ORootElementTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::UpdateRootElement& ORootElementTreeSetUpdate::getElementClass()
{
    return m_aAccessElement;
}

//==========================================================================
//= ORootElementValueSetUpdate
//==========================================================================

// XInterface refcounting
void SAL_CALL ORootElementValueSetUpdate::acquire( ) throw ()
{
    BasicUpdateElement::acquire();
}

void SAL_CALL ORootElementValueSetUpdate::release( ) throw ()
{
    BasicUpdateElement::release();
}

// XInterface joining
uno::Any SAL_CALL ORootElementValueSetUpdate::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicUpdateElement::queryInterface(rType);

    if (!aRet.hasValue())
        aRet = BasicValueSet::queryInterface(rType);

    return aRet;
}

// XTypeProvider joining
uno::Sequence< uno::Type > SAL_CALL ORootElementValueSetUpdate::getTypes( ) throw (uno::RuntimeException )
{
    return comphelper::concatSequences(BasicUpdateElement::getTypes(),BasicValueSet::getTypes() );
}

uno::Sequence< sal_Int8 > SAL_CALL ORootElementValueSetUpdate::getImplementationId( ) throw (uno::RuntimeException )
{
    return BasicUpdateElement::getImplementationId();
}


configapi::NodeAccess&  ORootElementValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

configapi::NodeSetInfoAccess& ORootElementValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

configapi::NodeValueSetAccess*  ORootElementValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

configapi::UpdateRootElement& ORootElementValueSetUpdate::getElementClass()
{
    return m_aAccessElement;
}
//........................................................................
} // namespace configmgr
//........................................................................

