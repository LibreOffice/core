/*************************************************************************
 *
 *  $RCSfile: elementaccess.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include "elementaccess.hxx"

#include "elementimpl.hxx"
#include "apinotifierimpl.hxx"

#include "apitreeaccess.hxx"

//-----------------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------------
    using namespace configapi;

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

//-----------------------------------------------------------------------------------
// XInterface (but not method queryInterface)
//-----------------------------------------------------------------------------------

// acuire doesn't really do anything but forward. OTOH it should always be overridden when release() is
void SAL_CALL BasicInnerElement ::acquire() throw() { BaseImplHelper::acquire(); }
void SAL_CALL BasicSetElement   ::acquire() throw() { BaseImplHelper::acquire(); }
void SAL_CALL BasicRootElement  ::acquire() throw() { BaseImplHelper::acquire(); }
void SAL_CALL BasicUpdateElement::acquire() throw() { BaseImplHelper::acquire(); }
//-----------------------------------------------------------------------------------

void SAL_CALL BasicInnerElement::release() throw()
{
    if (1 == m_refCount)
    {
        implDisposeObject( getNodeAccess(), getElementClass() );
    }
    BaseImplHelper::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::release() throw()
{
    if (1 == m_refCount)
    {
        implDisposeObject( getNodeAccess(), getElementClass() );
    }
    BaseImplHelper::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::release() throw()
{
    if (1 == m_refCount)
    {
        implDisposeObject( getNodeAccess(), getElementClass() );
    }
    BaseImplHelper::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::release() throw()
{
    if (1 == m_refCount)
    {
        implDisposeObject( getNodeAccess(), getElementClass() );
    }
    BaseImplHelper::release();
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XTypeProvider (but not method getTypes)
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicInnerElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicSetElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicRootElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicUpdateElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChild (not for root elements)
//-----------------------------------------------------------------------------------

Reference< uno::XInterface > SAL_CALL BasicInnerElement::getParent(  ) throw(RuntimeException)
{
    return implGetParent( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

Reference< uno::XInterface > SAL_CALL BasicSetElement::getParent(  ) throw(RuntimeException)
{
    return implGetParent( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicInnerElement::setParent( const Reference< uno::XInterface >& xParent )
    throw(css::lang::NoSupportException, RuntimeException)
{
    implSetParent( getNodeAccess(), getElementClass(), xParent );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::setParent( const Reference< uno::XInterface >& xParent )
    throw(css::lang::NoSupportException, RuntimeException)
{
    implSetParent( getNodeAccess(), getElementClass(), xParent );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XNamed
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicInnerElement::getName(  ) throw(RuntimeException)
{
    return implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicSetElement::getName(  ) throw(RuntimeException)
{
    return implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicRootElement::getName(  ) throw(RuntimeException)
{
    return implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicUpdateElement::getName(  ) throw(RuntimeException)
{
    return implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicInnerElement::setName( const OUString& aName ) throw(RuntimeException)
{
    implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::setName( const OUString& aName ) throw(RuntimeException)
{
    implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::setName( const OUString& aName ) throw(RuntimeException)
{
    implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::setName( const OUString& aName ) throw(RuntimeException)
{
    implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChangesNotifier
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::addChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::addChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::addChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicSetElement::removeChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::removeChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::removeChangesListener( const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XComponent
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::dispose(  ) throw(uno::RuntimeException)
{
    implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::dispose(  ) throw(uno::RuntimeException)
{
    implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::dispose(  ) throw(uno::RuntimeException)
{
    implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicSetElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicSetElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XServiceInfo
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicSetElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicRootElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicUpdateElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicSetElement::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicRootElement::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicUpdateElement::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL  BasicSetElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL  BasicRootElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL  BasicUpdateElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

// ONLY set elements
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XTemplateInstance
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicSetElement::getTemplateName( ) throw(uno::RuntimeException)
{
    return implGetTemplateName( getElementClass() );
}

//-----------------------------------------------------------------------------------
// XUnoTunnel
//-----------------------------------------------------------------------------------

sal_Int64 SAL_CALL BasicSetElement::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    return implGetSomething( getElementClass(), aIdentifier);
}

//-----------------------------------------------------------------------------------

// ONLY root elements
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XLocalizable
//-----------------------------------------------------------------------------------

css::lang::Locale SAL_CALL  BasicRootElement::getLocale(  ) throw(uno::RuntimeException)
{
    return implGetLocale( getElementClass() );
}
//-----------------------------------------------------------------------------------

css::lang::Locale SAL_CALL  BasicUpdateElement::getLocale(  ) throw(uno::RuntimeException)
{
    return implGetLocale( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicRootElement::setLocale( const css::lang::Locale& eLocale ) throw(uno::RuntimeException)
{
    implSetLocale( getElementClass(), eLocale );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::setLocale( const css::lang::Locale& eLocale ) throw(uno::RuntimeException)
{
    implSetLocale( getElementClass(), eLocale );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChangesBatch (only on update root)
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::commitChanges(  ) throw(css::lang::WrappedTargetException, uno::RuntimeException)
{
    implCommitChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicUpdateElement::hasPendingChanges(  ) throw(uno::RuntimeException)
{
    return implHasPendingChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< css::util::ElementChange > SAL_CALL BasicUpdateElement::getPendingChanges(  ) throw(uno::RuntimeException)
{
    return implGetPendingChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
} // namespace configmgr


