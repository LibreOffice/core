/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessimpl.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_API_BASEACCESSIMPL_HXX_
#define CONFIGMGR_API_BASEACCESSIMPL_HXX_

#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyWithState.hpp>

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (read-only operation)
    */
    namespace configapi
    {
        class NodeAccess;
        class NodeSetInfoAccess;
        class NodeGroupInfoAccess;

        // XHierarchicalName
        rtl::OUString implGetHierarchicalName(NodeAccess& rNode)
            throw(uno::RuntimeException);

        rtl::OUString implComposeHierarchicalName(NodeGroupInfoAccess& rNode, const rtl::OUString& aRelativeName )
            throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, uno::RuntimeException);

        rtl::OUString implComposeHierarchicalName(NodeSetInfoAccess& rNode, const rtl::OUString& aRelativeName )
            throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, uno::RuntimeException);

        // XElementAccess, base class of XNameAccess
        uno::Type implGetElementType(NodeGroupInfoAccess& rNode)
            throw(uno::RuntimeException);

        uno::Type implGetElementType(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

        sal_Bool implHasElements(NodeGroupInfoAccess& rNode)
            throw(uno::RuntimeException);

        sal_Bool implHasElements(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

        // XNameAccess
        uno::Any implGetByName(NodeAccess& rNode, const rtl::OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        uno::Sequence< rtl::OUString > implGetElementNames(NodeAccess& rNode)
            throw( uno::RuntimeException);

        sal_Bool implHasByName(NodeAccess& rNode, const rtl::OUString& aName )
            throw(uno::RuntimeException);

        // XHierarchicalNameAccess
        uno::Any implGetByHierarchicalName(NodeAccess& rNode, const rtl::OUString& aName )
            throw(css::container::NoSuchElementException, uno::RuntimeException);

        sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const rtl::OUString& aName )
            throw(uno::RuntimeException);

        // XExactName
        rtl::OUString implGetExactName(NodeGroupInfoAccess& rNode, const rtl::OUString& aApproximateName )
            throw(uno::RuntimeException);

        rtl::OUString implGetExactName(NodeSetInfoAccess& rNode, const rtl::OUString& aApproximateName )
            throw(uno::RuntimeException);

        // XProperty
        css::beans::Property implGetAsProperty(NodeAccess& rNode)
            throw(uno::RuntimeException);

        // XPropertySetInfo
        uno::Sequence< css::beans::Property > implGetProperties( NodeAccess& rNode )
            throw (uno::RuntimeException);

        css::beans::Property implGetPropertyByName( NodeAccess& rNode, const rtl::OUString& aName )
            throw (css::beans::UnknownPropertyException, uno::RuntimeException);

        sal_Bool implHasPropertyByName( NodeAccess& rNode, const rtl::OUString& name )
            throw (uno::RuntimeException);


        // XPropertyWithState
        css::beans::PropertyState implGetStateAsProperty(NodeAccess& rNode)
            throw (uno::RuntimeException);

        uno::Reference< uno::XInterface > implGetDefaultAsProperty(NodeAccess& rNode)
            throw (css::lang::WrappedTargetException, uno::RuntimeException);

        // set-specific interfaces

        // XTemplateContainer
        rtl::OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


