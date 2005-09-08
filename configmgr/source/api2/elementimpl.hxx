/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementimpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:13:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_ELEMENTIMPL_HXX_
#define CONFIGMGR_API_ELEMENTIMPL_HXX_

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
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    using rtl::OUString;

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
        OUString implGetName(NodeAccess& rNode, NodeElement& rElement)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, NodeElement& rElement, const OUString& aName ) // generally not supported (! - missing exception)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, SetElement& rElement, const OUString& aName ) // maybe supported
            throw(uno::RuntimeException);

        // XServiceInfo
        //---------------------------------------------------------------------
        OUString implGetImplementationName( NodeAccess& rNode, NodeElement& rElement )
            throw(uno::RuntimeException);

        sal_Bool implSupportsService( NodeAccess& rNode, NodeElement& rElement, const OUString& ServiceName )
            throw(uno::RuntimeException);

        uno::Sequence< OUString > implGetSupportedServiceNames( NodeAccess& rNode, NodeElement& rElement )
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
        OUString implGetTemplateName(SetElement& rElement)
            throw(uno::RuntimeException);

        // XUnoTunnel
        //---------------------------------------------------------------------
        sal_Int64 implGetSomething(SetElement& rElement, const uno::Sequence< sal_Int8 >& aIdentifier )
            throw(uno::RuntimeException);

    //-------------------------------------------------------------------------
    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


