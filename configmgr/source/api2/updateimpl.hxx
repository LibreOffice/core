/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:22:45 $
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

#ifndef CONFIGMGR_API_BASEUPDATEIMPL_HXX_
#define CONFIGMGR_API_BASEUPDATEIMPL_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYWITHSTATE_HPP_
#include <com/sun/star/beans/XPropertyWithState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    using rtl::OUString;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (updating operation)
    */
    namespace configapi
    {
        class NodeSetAccess;
        class NodeTreeSetAccess;
        class NodeValueSetAccess;
        class NodeGroupAccess;

        // XNameReplace
        //---------------------------------------------------------------------
        void implReplaceByName(NodeGroupAccess& rNode, const OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implReplaceByName(NodeTreeSetAccess& rNode, const OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implReplaceByName(NodeValueSetAccess& rNode, const OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XNameContainer
        //---------------------------------------------------------------------
        void implInsertByName(NodeTreeSetAccess& rNode, const OUString& rName, const uno::Any& rElement)
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implInsertByName(NodeValueSetAccess& rNode, const OUString& rName, const uno::Any& rElement)
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveByName(NodeTreeSetAccess& rNode, const OUString& rName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveByName(NodeValueSetAccess& rNode, const OUString& rName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XPropertyWithState - updating operation only
        //---------------------------------------------------------------------
        void implSetToDefaultAsProperty(NodeSetAccess& rNode)
            throw (css::lang::WrappedTargetException, uno::RuntimeException);

        // XSingleServiceFactory
        //---------------------------------------------------------------------
        uno::Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode )
            throw(uno::Exception, uno::RuntimeException);

        uno::Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode, const uno::Sequence< uno::Any >& aArguments )
            throw(uno::Exception, uno::RuntimeException);

        //---------------------------------------------------------------------
    }

}
#endif // CONFIGMGR_API_BASEUPDATEIMPL_HXX_


