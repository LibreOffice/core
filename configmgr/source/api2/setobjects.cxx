/*************************************************************************
 *
 *  $RCSfile: setobjects.cxx,v $
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


NodeAccess& OInnerSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OInnerSetInfo::getNode()
{
    return m_aAccessElement;
}

InnerElement& OInnerSetInfo::getElementClass()
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


NodeAccess& OInnerTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OInnerTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeTreeSetAccess*  OInnerTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

InnerElement& OInnerTreeSetUpdate::getElementClass()
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


NodeAccess& OInnerValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OInnerValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeValueSetAccess* OInnerValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

InnerElement& OInnerValueSetUpdate::getElementClass()
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


NodeAccess& OSetElementSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OSetElementSetInfo::getNode()
{
    return m_aAccessElement;
}

SetElement& OSetElementSetInfo::getElementClass()
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


NodeAccess& OSetElementTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OSetElementTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeTreeSetAccess*  OSetElementTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

SetElement& OSetElementTreeSetUpdate::getElementClass()
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


NodeAccess& OSetElementValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& OSetElementValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeValueSetAccess* OSetElementValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

SetElement& OSetElementValueSetUpdate::getElementClass()
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


NodeAccess& ORootElementSetInfo::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& ORootElementSetInfo::getNode()
{
    return m_aAccessElement;
}

RootElement& ORootElementSetInfo::getElementClass()
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


NodeAccess& ORootElementTreeSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& ORootElementTreeSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeTreeSetAccess*  ORootElementTreeSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

UpdateRootElement& ORootElementTreeSetUpdate::getElementClass()
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


NodeAccess& ORootElementValueSetUpdate::getNodeAccess()
{
    return m_aAccessElement;
}

NodeSetInfoAccess& ORootElementValueSetUpdate::getNode()
{
    return m_aAccessElement;
}

NodeValueSetAccess* ORootElementValueSetUpdate::maybeGetUpdateAccess()
{
    return &m_aAccessElement;
}

UpdateRootElement& ORootElementValueSetUpdate::getElementClass()
{
    return m_aAccessElement;
}
//........................................................................
} // namespace configmgr
//........................................................................

