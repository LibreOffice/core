/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementaccess.cxx,v $
 * $Revision: 1.8 $
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

#include "elementaccess.hxx"

#include "elementimpl.hxx"
#include "apinotifierimpl.hxx"

#include "apitreeaccess.hxx"

//-----------------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------------

    namespace uno = com::sun::star::uno;

//-----------------------------------------------------------------------------------
// XInterface (but not method queryInterface)
//-----------------------------------------------------------------------------------

// acuire doesn't really do anything but forward. OTOH it should always be overridden when release() is
void SAL_CALL BasicInnerElement ::acquire() throw() { cppu::WeakImplHelper3< css::container::XChild, css::container::XNamed, css::lang::XServiceInfo >::acquire(); }
void SAL_CALL BasicSetElement   ::acquire() throw() { cppu::WeakImplHelper6< css::container::XChild, css::container::XNamed, css::lang::XComponent, css::lang::XServiceInfo, css::configuration::XTemplateInstance, css::lang::XUnoTunnel >::acquire(); }
void SAL_CALL BasicRootElement  ::acquire() throw() { cppu::WeakImplHelper5< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable >::acquire(); }
void SAL_CALL BasicUpdateElement::acquire() throw() { cppu::WeakImplHelper6< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable, css::util::XChangesBatch >::acquire(); }
//-----------------------------------------------------------------------------------

void SAL_CALL BasicInnerElement::release() throw()
{
    // FIXME: this looks highly flaky wrt. weak-refs etc.
    bool bLastRef = (1 == m_refCount);
    if (bLastRef)
    {
        UnoApiLock::acquire();
        configapi::implDisposeObject( getNodeAccess(), getElementClass() );
    }
    cppu::WeakImplHelper3< css::container::XChild, css::container::XNamed, css::lang::XServiceInfo >::release();
    if (bLastRef)
        UnoApiLock::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::release() throw()
{
    // FIXME: this looks highly flaky wrt. weak-refs etc.
    bool bLastRef = (1 == m_refCount);
    if (bLastRef)
    {
        UnoApiLock::acquire();
        configapi::implDisposeObject( getNodeAccess(), getElementClass() );
    }
    cppu::WeakImplHelper6< css::container::XChild, css::container::XNamed, css::lang::XComponent, css::lang::XServiceInfo, css::configuration::XTemplateInstance, css::lang::XUnoTunnel >::release();
    if (bLastRef)
        UnoApiLock::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::release() throw()
{
    // FIXME: this looks highly flaky wrt. weak-refs etc.
    bool bLastRef = (1 == m_refCount);
    if (bLastRef)
    {
        UnoApiLock::acquire();
        configapi::implDisposeObject( getNodeAccess(), getElementClass() );
    }
    cppu::WeakImplHelper5< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable >::release();
    if (bLastRef)
        UnoApiLock::release();
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::release() throw()
{
    // FIXME: this looks highly flaky wrt. weak-refs etc.
    bool bLastRef = (1 == m_refCount);
    if (bLastRef)
    {
        UnoApiLock::acquire();
        configapi::implDisposeObject( getNodeAccess(), getElementClass() );
    }
    cppu::WeakImplHelper6< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable, css::util::XChangesBatch >::release();
    if (bLastRef)
        UnoApiLock::release();
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XTypeProvider (but not method getTypes)
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicInnerElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicSetElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicRootElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence<sal_Int8> SAL_CALL BasicUpdateElement::getImplementationId(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationId( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChild (not for root elements)
//-----------------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL BasicInnerElement::getParent(  ) throw(uno::RuntimeException)
{
    return configapi::implGetParent( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL BasicSetElement::getParent(  ) throw(uno::RuntimeException)
{
    return configapi::implGetParent( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicInnerElement::setParent( const uno::Reference< uno::XInterface >& xParent )
    throw(css::lang::NoSupportException, uno::RuntimeException)
{
    configapi::implSetParent( getNodeAccess(), getElementClass(), xParent );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::setParent( const uno::Reference< uno::XInterface >& xParent )
    throw(css::lang::NoSupportException, uno::RuntimeException)
{
    configapi::implSetParent( getNodeAccess(), getElementClass(), xParent );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XNamed
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicInnerElement::getName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicSetElement::getName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicRootElement::getName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicUpdateElement::getName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicInnerElement::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    configapi::implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    configapi::implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    configapi::implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    configapi::implSetName( getNodeAccess(), getElementClass(), aName );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChangesNotifier
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::addChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::addChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicRootElement::removeChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::removeChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XComponent
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetElement::dispose(  ) throw(uno::RuntimeException)
{
    configapi::implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::dispose(  ) throw(uno::RuntimeException)
{
    configapi::implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::dispose(  ) throw(uno::RuntimeException)
{
    configapi::implDispose( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicSetElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicSetElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicRootElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::removeEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNodeAccess(), xListener );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XServiceInfo
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicInnerElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicSetElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicRootElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicUpdateElement::getImplementationName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetImplementationName( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicInnerElement::supportsService( const rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return configapi::implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicSetElement::supportsService( const rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return configapi::implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicRootElement::supportsService( const rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return configapi::implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicUpdateElement::supportsService( const rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return configapi::implSupportsService( getNodeAccess(), getElementClass(), ServiceName );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Sequence< rtl::OUString > SAL_CALL     BasicInnerElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return configapi::implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL     BasicSetElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return configapi::implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL     BasicRootElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return configapi::implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL     BasicUpdateElement::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return configapi::implGetSupportedServiceNames( getNodeAccess(), getElementClass() );
}
//-----------------------------------------------------------------------------------

// ONLY set elements
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XTemplateInstance
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicSetElement::getTemplateName( ) throw(uno::RuntimeException)
{
    return configapi::implGetTemplateName( getElementClass() );
}

//-----------------------------------------------------------------------------------
// XUnoTunnel
//-----------------------------------------------------------------------------------

sal_Int64 SAL_CALL BasicSetElement::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    return configapi::implGetSomething( getElementClass(), aIdentifier);
}

//-----------------------------------------------------------------------------------

// ONLY root elements
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XLocalizable
//-----------------------------------------------------------------------------------

css::lang::Locale SAL_CALL  BasicRootElement::getLocale(  ) throw(uno::RuntimeException)
{
    return configapi::implGetLocale( getElementClass() );
}
//-----------------------------------------------------------------------------------

css::lang::Locale SAL_CALL  BasicUpdateElement::getLocale(  ) throw(uno::RuntimeException)
{
    return configapi::implGetLocale( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void SAL_CALL BasicRootElement::setLocale( const css::lang::Locale& eLocale ) throw(uno::RuntimeException)
{
    configapi::implSetLocale( getElementClass(), eLocale );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::setLocale( const css::lang::Locale& eLocale ) throw(uno::RuntimeException)
{
    configapi::implSetLocale( getElementClass(), eLocale );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// XChangesBatch (only on update root)
//-----------------------------------------------------------------------------------

void SAL_CALL BasicUpdateElement::commitChanges(  ) throw(css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implCommitChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicUpdateElement::hasPendingChanges(  ) throw(uno::RuntimeException)
{
    return configapi::implHasPendingChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

uno::Sequence< css::util::ElementChange > SAL_CALL BasicUpdateElement::getPendingChanges(  ) throw(uno::RuntimeException)
{
    return configapi::implGetPendingChanges( getElementClass() );
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
} // namespace configmgr


