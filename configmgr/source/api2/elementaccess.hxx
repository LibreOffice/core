/*************************************************************************
 *
 *  $RCSfile: elementaccess.hxx,v $
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

#ifndef CONFIGMGR_API_ELEMENTACCESS_HXX_
#define CONFIGMGR_API_ELEMENTACCESS_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATEINSTANCE_HPP_
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif

#ifndef CONFIGMGR_APITYPES_HXX_
#include "apitypes.hxx"
#endif

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
    typedef  ::cppu::WeakImplHelper2
                < css::container::XChild
                , css::container::XNamed
//              , css::lang::XComponent
//              , css::lang::XServiceInfo
                >
                InnerElementImplHelper;

    class BasicInnerElement : public InnerElementImplHelper
    {
        typedef InnerElementImplHelper BaseImplHelper;
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
        virtual OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // not supported (! - missing exception)
            setName( const OUString& aName )
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
    typedef ::cppu::WeakImplHelper7
                < css::container::XChild
                , css::container::XNamed
                , css::util::XChangesNotifier
                , css::lang::XComponent
                , css::lang::XServiceInfo
                , css::configuration::XTemplateInstance
                , css::lang::XUnoTunnel
                >
                SetElementImplHelper;

    class BasicSetElement : public SetElementImplHelper
    {
        typedef SetElementImplHelper BaseImplHelper;
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
        virtual OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const OUString& aName )
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
        virtual OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

        // XTemplateInstance
        virtual OUString SAL_CALL
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
    typedef ::cppu::WeakImplHelper5
                < css::container::XNamed
                , css::util::XChangesNotifier
                , css::lang::XComponent
                , css::lang::XServiceInfo
                , css::lang::XLocalizable
                >
                RootElementImplHelper;

    class BasicRootElement : public RootElementImplHelper
    {
        typedef RootElementImplHelper BaseImplHelper;
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
        virtual OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const OUString& aName )
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
        virtual OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
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
    typedef ::cppu::WeakImplHelper6
                < css::container::XNamed
                , css::util::XChangesNotifier
                , css::lang::XComponent
                , css::lang::XServiceInfo
                , css::lang::XLocalizable
                , css::util::XChangesBatch
                >
                UpdateElementImplHelper;

    class BasicUpdateElement : public UpdateElementImplHelper
    {
        typedef UpdateElementImplHelper BaseImplHelper;
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
        virtual OUString SAL_CALL
            getName(  )
                throw(uno::RuntimeException);

        virtual void SAL_CALL  // generally not supported (! - missing exception)
            setName( const OUString& aName )
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
        virtual OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
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


