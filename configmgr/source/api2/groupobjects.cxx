/*************************************************************************
 *
 *  $RCSfile: groupobjects.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-13 12:14:15 $
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


NodeAccess& OInnerGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& OInnerGroupInfo::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess* OInnerGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

InnerElement& OInnerGroupInfo::getElementClass()
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


NodeAccess& OInnerGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& OInnerGroupUpdate::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess*    OInnerGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

InnerElement& OInnerGroupUpdate::getElementClass()
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


NodeAccess& OSetElementGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& OSetElementGroupInfo::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess* OSetElementGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

SetElement& OSetElementGroupInfo::getElementClass()
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


NodeAccess& OSetElementGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& OSetElementGroupUpdate::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess* OSetElementGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

SetElement& OSetElementGroupUpdate::getElementClass()
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


NodeAccess& ORootElementGroupInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& ORootElementGroupInfo::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess* ORootElementGroupInfo::maybeGetUpdateAccess()
{
    return 0;
}

RootElement& ORootElementGroupInfo::getElementClass()
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


NodeAccess& ORootElementGroupUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeGroupInfoAccess& ORootElementGroupUpdate::getNode()
{
    return m_aAccessElement;
}

NodeGroupAccess*    ORootElementGroupUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

UpdateRootElement& ORootElementGroupUpdate::getElementClass()
{
    return m_aAccessElement;
}

//........................................................................
} // namespace configmgr
//........................................................................

