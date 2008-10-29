/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updateimpl.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_API_BASEUPDATEIMPL_HXX_
#define CONFIGMGR_API_BASEUPDATEIMPL_HXX_

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertyWithState.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

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
        void implReplaceByName(NodeGroupAccess& rNode, const rtl::OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implReplaceByName(NodeTreeSetAccess& rNode, const rtl::OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implReplaceByName(NodeValueSetAccess& rNode, const rtl::OUString& rName, const uno::Any& rElement )
            throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XNameContainer
        //---------------------------------------------------------------------
        void implInsertByName(NodeTreeSetAccess& rNode, const rtl::OUString& rName, const uno::Any& rElement)
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implInsertByName(NodeValueSetAccess& rNode, const rtl::OUString& rName, const uno::Any& rElement)
            throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveByName(NodeTreeSetAccess& rNode, const rtl::OUString& rName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveByName(NodeValueSetAccess& rNode, const rtl::OUString& rName )
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


