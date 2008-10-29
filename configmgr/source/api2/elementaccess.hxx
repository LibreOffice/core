/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementaccess.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_API_ELEMENTACCESS_HXX_
#define CONFIGMGR_API_ELEMENTACCESS_HXX_

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

//-----------------------------------------------------------------------------
    namespace configapi
    {
        class NodeAccess;
        class InnerElement;
        class SetElement;
        class RootElement;
        class UpdateRootElement;
    }
//-----------------------------------------------------------------------------

    /** implements the interfaces supported by a inner node
        within the configuration tree that is a plain node (group member).
        <p> Is an interface adapter around
            <type scope='configmgr::configapi'>NodeAccess</type> and
            <type scope='configmgr::configapi'>InnerElement</type>.</p>
    */
    class BasicInnerElement : public cppu::WeakImplHelper3< css::container::XChild, css::container::XNamed, css::lang::XServiceInfo >
    {
    protected:
    // Destructors
        virtual ~BasicInnerElement() {}

    public:
    // Interface methods
        // XInterface ('dispose' management) - partial implementation
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider (from implhelper) - partial implementation
        virtual uno::Sequence<sal_Int8> SAL_CALL
            getImplementationId(  )
                throw(uno::RuntimeException);

        // XChild
        virtual uno::Reference< uno::XInterface > SAL_CALL
            getParent(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL // not supported
            setParent( const uno::Reference< uno::XInterface >& Parent )
                throw(css::lang::NoSupportException, css::uno::RuntimeException);

        // XNamed
        virtual rtl::OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // not supported (! - missing exception)
            setName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

    protected:
        virtual configapi::InnerElement& getElementClass() = 0;
        virtual configapi::NodeAccess& getNodeAccess() = 0;
    };
//-----------------------------------------------------------------------------

    /** implements the interfaces supported by a node
        within the configuration tree that is a set element (or floating free).
        <p> Is an interface adapter around
            <type scope='configmgr::configapi'>NodeAccess</type> and
            <type scope='configmgr::configapi'>SetElement</type>.</p>
    */
    class BasicSetElement : public cppu::WeakImplHelper6< css::container::XChild, css::container::XNamed, css::lang::XComponent, css::lang::XServiceInfo, css::configuration::XTemplateInstance, css::lang::XUnoTunnel >
    {
    protected:
    // Destructors
        virtual ~BasicSetElement() {}

    public:
    // Interface methods
        // XInterface ('dispose' management) - partial implementation
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider (from implhelper) - partial implementation
        virtual uno::Sequence<sal_Int8> SAL_CALL
            getImplementationId(  )
                throw(uno::RuntimeException);

        // XChild
        virtual uno::Reference< uno::XInterface > SAL_CALL
            getParent(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL // currently not supported
            setParent( const uno::Reference< uno::XInterface >& Parent )
                throw(css::lang::NoSupportException, css::uno::RuntimeException);

        // XNamed
        virtual rtl::OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL
            dispose(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeEventListener( const uno::Reference< css::lang::XEventListener >& aListener )
                throw(uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

        // XTemplateInstance
        virtual rtl::OUString SAL_CALL
            getTemplateName( )
                throw(uno::RuntimeException);

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL
            getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
                throw(uno::RuntimeException);

    protected:
        virtual configapi::SetElement& getElementClass() = 0;
        virtual configapi::NodeAccess& getNodeAccess() = 0;
    };
//-----------------------------------------------------------------------------

    /** implements the interfaces supported by a node
        within the configuration that is the root of a read-only access
        <p> Is an interface adapter around
            <type scope='configmgr::configapi'>NodeAccess</type> and
            <type scope='configmgr::configapi'>SetElement</type>.</p>
    */
    class BasicRootElement : public cppu::WeakImplHelper5< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable >
    {
    protected:
    // Destructors
        virtual ~BasicRootElement() {}

    public:
    // Interface methods
        // XInterface ('dispose' management) - partial implementation
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider (from implhelper) - partial implementation
        virtual uno::Sequence<sal_Int8> SAL_CALL
            getImplementationId(  )
                throw(uno::RuntimeException);

        // XNamed
        virtual rtl::OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XChangesNotifier
        virtual void SAL_CALL
            addChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
                throw(uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL
            dispose(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeEventListener( const uno::Reference< css::lang::XEventListener >& aListener )
                throw(uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

        // XLocalizable
        virtual css::lang::Locale SAL_CALL
            getLocale(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL   // maybe not supported (! - missing exception)
            setLocale( const css::lang::Locale& eLocale )
                throw(uno::RuntimeException);

    protected:
        virtual configapi::RootElement& getElementClass() = 0;
        virtual configapi::NodeAccess& getNodeAccess() = 0;
    };
//-----------------------------------------------------------------------------

    /** implements the interfaces supported by a node
        within the configuration that is the root of an update access
        <p> Is an interface adapter around
            <type scope='configmgr::configapi'>NodeAccess</type> and
            <type scope='configmgr::configapi'>SetElement</type>.</p>
    */
    class BasicUpdateElement : public cppu::WeakImplHelper6< css::container::XNamed, css::util::XChangesNotifier, css::lang::XComponent, css::lang::XServiceInfo, css::lang::XLocalizable, css::util::XChangesBatch >
    {
    protected:
    // Destructors
        virtual ~BasicUpdateElement() {}

    public:
    // Interface methods
        // XInterface ('dispose' management) - partial implementation
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider (from implhelper) - partial implementation
        virtual uno::Sequence<sal_Int8> SAL_CALL
            getImplementationId(  )
                throw(uno::RuntimeException);

        // XNamed
        virtual rtl::OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XChangesNotifier
        virtual void SAL_CALL
            addChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeChangesListener( const uno::Reference< css::util::XChangesListener >& xListener )
                throw(uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL
            dispose(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            addEventListener( const uno::Reference< css::lang::XEventListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeEventListener( const uno::Reference< css::lang::XEventListener >& aListener )
                throw(uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

        // XChangesBatch
        virtual void SAL_CALL
            commitChanges(  )
                throw(css::lang::WrappedTargetException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasPendingChanges(  )
                throw(uno::RuntimeException);

        virtual uno::Sequence< css::util::ElementChange > SAL_CALL
            getPendingChanges(  )
                throw(uno::RuntimeException);

        // XLocalizable
        virtual css::lang::Locale SAL_CALL
            getLocale(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL    // maybe not supported (! - missing exception)
            setLocale( const css::lang::Locale& eLocale )
                throw(uno::RuntimeException);

    protected:
        virtual configapi::UpdateRootElement& getElementClass() = 0;
        virtual configapi::NodeAccess& getNodeAccess() = 0;
    };
//-----------------------------------------------------------------------------
}
#endif // CONFIGMGR_API_ELEMENTACCESS_HXX_


