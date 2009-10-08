/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementimpl.hxx,v $
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

#ifndef CONFIGMGR_API_ELEMENTIMPL_HXX_
#define CONFIGMGR_API_ELEMENTIMPL_HXX_

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

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
    //-------------------------------------------------------------------------
        class NodeAccess;
        class NodeElement;
        class InnerElement;
        class TreeElement;
        class SetElement;
        class RootElement;
        class UpdateRootElement;
    //-------------------------------------------------------------------------

        // XComponent and XInterface (EOL handling)
        //---------------------------------------------------------------------
        void implDispose( SetElement& rElement)
            throw(uno::RuntimeException);

        void implDispose( RootElement& rElement)
            throw(uno::RuntimeException);

        void implDisposeObject( NodeAccess& aNode, SetElement& rElement)
            throw(uno::RuntimeException);

        void implDisposeObject( NodeAccess& aNode, RootElement& rElement)
            throw(uno::RuntimeException);

        void implDisposeObject( NodeAccess& aNode, InnerElement& rElement)
            throw(uno::RuntimeException);

        // XTypeProvider
        //---------------------------------------------------------------------
        uno::Sequence<sal_Int8> implGetImplementationId(NodeAccess& rNode,  NodeElement& rElement ) throw(uno::RuntimeException);

        // XChild
        //---------------------------------------------------------------------
        uno::Reference< uno::XInterface > implGetParent(NodeAccess& rNode, InnerElement& rElement)
            throw(uno::RuntimeException);

        uno::Reference< uno::XInterface > implGetParent(NodeAccess& rNode, SetElement& rElement)
            throw(uno::RuntimeException);

        void implSetParent(NodeAccess& rNode, InnerElement& rElement, const uno::Reference< uno::XInterface >& Parent ) // generally not supported
            throw(css::lang::NoSupportException, css::uno::RuntimeException);

        void implSetParent(NodeAccess& rNode, SetElement& rElement, const uno::Reference< uno::XInterface >& Parent ) // maybe supported
            throw(css::lang::NoSupportException, css::uno::RuntimeException);

        // XNamed
        //---------------------------------------------------------------------
        rtl::OUString implGetName(NodeAccess& rNode, NodeElement& rElement)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, NodeElement& rElement, const rtl::OUString& aName ) // generally not supported (! - missing exception)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, SetElement& rElement, const rtl::OUString& aName ) // maybe supported
            throw(uno::RuntimeException);

        // XServiceInfo
        //---------------------------------------------------------------------
        rtl::OUString implGetImplementationName( NodeAccess& rNode, NodeElement& rElement )
            throw(uno::RuntimeException);

        sal_Bool implSupportsService( NodeAccess& rNode, NodeElement& rElement, const rtl::OUString& ServiceName )
            throw(uno::RuntimeException);

        uno::Sequence< rtl::OUString > implGetSupportedServiceNames( NodeAccess& rNode, NodeElement& rElement )
            throw(uno::RuntimeException);

    // Root only only
    //-------------------------------------------------------------------------

        // XLocalizable
        //---------------------------------------------------------------------

        css::lang::Locale implGetLocale( RootElement& rElement )
                throw(uno::RuntimeException);

        void implSetLocale( RootElement& rElement, const css::lang::Locale& eLocale )
                throw(uno::RuntimeException);

        // XChangesBatch
        //---------------------------------------------------------------------

        void implCommitChanges( UpdateRootElement& rElement )
                throw(css::lang::WrappedTargetException, uno::RuntimeException);

        sal_Bool implHasPendingChanges( RootElement& rElement )
                throw(uno::RuntimeException);

        uno::Sequence< css::util::ElementChange > implGetPendingChanges( RootElement& rElement )
                throw(uno::RuntimeException);

    // Set only
    //-------------------------------------------------------------------------
        // XTemplateInstance
        //---------------------------------------------------------------------
        rtl::OUString implGetTemplateName(SetElement& rElement)
            throw(uno::RuntimeException);

        // XUnoTunnel
        //---------------------------------------------------------------------
        sal_Int64 implGetSomething(SetElement& rElement, const uno::Sequence< sal_Int8 >& aIdentifier )
            throw(uno::RuntimeException);

    //-------------------------------------------------------------------------
    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


