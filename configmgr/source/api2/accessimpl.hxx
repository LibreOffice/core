/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessimpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:04:58 $
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

#ifndef CONFIGMGR_API_BASEACCESSIMPL_HXX_
#define CONFIGMGR_API_BASEACCESSIMPL_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATEINSTANCE_HPP_
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATECONTAINER_HPP_
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTY_HPP_
#include <com/sun/star/beans/XProperty.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYWITHSTATE_HPP_
#include <com/sun/star/beans/XPropertyWithState.hpp>
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
        class NodeAccess;
        class NodeSetInfoAccess;
        class NodeGroupInfoAccess;

        // XHierarchicalName
        OUString implGetHierarchicalName(NodeAccess& rNode)
            throw(uno::RuntimeException);

        OUString implComposeHierarchicalName(NodeGroupInfoAccess& rNode, const OUString& aRelativeName )
            throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, uno::RuntimeException);

        OUString implComposeHierarchicalName(NodeSetInfoAccess& rNode, const OUString& aRelativeName )
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
        uno::Any implGetByName(NodeAccess& rNode, const OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        uno::Sequence< OUString > implGetElementNames(NodeAccess& rNode)
            throw( uno::RuntimeException);

        sal_Bool implHasByName(NodeAccess& rNode, const OUString& aName )
            throw(uno::RuntimeException);

        // XHierarchicalNameAccess
        uno::Any implGetByHierarchicalName(NodeAccess& rNode, const OUString& aName )
            throw(css::container::NoSuchElementException, uno::RuntimeException);

        sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const OUString& aName )
            throw(uno::RuntimeException);

        // XExactName
        OUString implGetExactName(NodeGroupInfoAccess& rNode, const OUString& aApproximateName )
            throw(uno::RuntimeException);

        OUString implGetExactName(NodeSetInfoAccess& rNode, const OUString& aApproximateName )
            throw(uno::RuntimeException);

        // XProperty
        css::beans::Property implGetAsProperty(NodeAccess& rNode)
            throw(uno::RuntimeException);

        // XPropertySetInfo
        uno::Sequence< css::beans::Property > implGetProperties( NodeAccess& rNode )
            throw (uno::RuntimeException);

        css::beans::Property implGetPropertyByName( NodeAccess& rNode, const OUString& aName )
            throw (css::beans::UnknownPropertyException, uno::RuntimeException);

        sal_Bool implHasPropertyByName( NodeAccess& rNode, const OUString& Name )
            throw (uno::RuntimeException);


        // XPropertyWithState
        css::beans::PropertyState implGetStateAsProperty(NodeAccess& rNode)
            throw (uno::RuntimeException);

        void implSetToDefaultAsProperty(NodeAccess& rNode)
            throw (css::lang::WrappedTargetException, uno::RuntimeException);

        uno::Reference< uno::XInterface > implGetDefaultAsProperty(NodeAccess& rNode)
            throw (css::lang::WrappedTargetException, uno::RuntimeException);

        // set-specific interfaces

        // XTemplateContainer
        OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


