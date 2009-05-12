/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setaccess.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_API_SETACCESS_HXX_
#define CONFIGMGR_API_SETACCESS_HXX_

#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/util/XStringEscape.hpp>
#include <cppuhelper/implbase9.hxx>

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

    namespace configapi { class NodeSetInfoAccess; }

    /** implements the (read-only) interfaces supported by a set node
        within the configuration tree.
        <p> Is an interface adapter around <type scope='configmgr::configapi'>NodeAccess</type>.</p>
    */
    class BasicSetAccess
    : public ::cppu::ImplHelper9
                < css::container::XNameAccess
                , css::container::XHierarchicalName
                , css::container::XHierarchicalNameAccess
                , css::container::XContainer
                , css::beans::XExactName
                , css::beans::XProperty
                , css::beans::XPropertySetInfo
                , css::configuration::XTemplateContainer
                , css::util::XStringEscape
                >
    {
    protected:
    // Destructors
        virtual ~BasicSetAccess() {}

    public:
    // Interface methods
        // XHierarchicalName
        virtual rtl::OUString SAL_CALL
            getHierarchicalName(  )
                throw(uno::RuntimeException);

        virtual rtl::OUString SAL_CALL
            composeHierarchicalName( const rtl::OUString& aRelativeName )
                throw(css::lang::IllegalArgumentException, css::lang::NoSupportException,
                        uno::RuntimeException);

        // XElementAccess, base class of XNameAccess
        virtual uno::Type SAL_CALL
            getElementType(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasElements(  )
                throw(uno::RuntimeException);

        // XNameAccess
        virtual uno::Any SAL_CALL
            getByName( const rtl::OUString& aName )
                throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
                         uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getElementNames(  )
                throw( uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasByName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XHierarchicalNameAccess
        virtual uno::Any SAL_CALL
            getByHierarchicalName( const rtl::OUString& aName )
                throw(css::container::NoSuchElementException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasByHierarchicalName( const rtl::OUString& aName )
                throw(uno::RuntimeException);

        // XContainer
        virtual void SAL_CALL
            addContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
                throw(uno::RuntimeException);

        // XExactName
        virtual rtl::OUString SAL_CALL
            getExactName( const rtl::OUString& aApproximateName )
                throw(uno::RuntimeException);

        // XProperty
        virtual css::beans::Property SAL_CALL
            getAsProperty(  )
                throw(uno::RuntimeException);

        // XPropertySetInfo
        virtual uno::Sequence< css::beans::Property > SAL_CALL
            getProperties(  )
                throw (uno::RuntimeException);

        virtual css::beans::Property SAL_CALL
            getPropertyByName( const rtl::OUString& aName )
                throw (css::beans::UnknownPropertyException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasPropertyByName( const rtl::OUString& name )
                throw (uno::RuntimeException);

        // XTemplateContainer
        rtl::OUString SAL_CALL
            getElementTemplateName(  )
                throw(uno::RuntimeException);

        // XStringEscape
        rtl::OUString SAL_CALL
            escapeString( const rtl::OUString& aString )
                throw(css::lang::IllegalArgumentException, uno::RuntimeException);

        rtl::OUString SAL_CALL
            unescapeString( const rtl::OUString& aEscapedString )
                throw(css::lang::IllegalArgumentException, uno::RuntimeException);

    protected:
        virtual configapi::NodeSetInfoAccess& getNode() = 0;
    };

}
#endif // CONFIGMGR_API_SETACCESS_HXX_


