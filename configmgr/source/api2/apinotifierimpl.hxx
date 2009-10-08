/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinotifierimpl.hxx,v $
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

#ifndef CONFIGMGR_API_NOTIFIERIMPL_HXX_
#define CONFIGMGR_API_NOTIFIERIMPL_HXX_

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

    /* implementations of the event notification interfaces
        supported by a node within the configuration tree.
    */
    namespace configapi
    {
        class NodeAccess;
        class NodeSetInfoAccess;
        class NodeGroupInfoAccess;

        // Notification support

        // XComponent
        void implAddListener(NodeAccess& rNode, const uno::Reference< css::lang::XEventListener >& xListener )
            throw(uno::RuntimeException);

        void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::lang::XEventListener >& xListener )
            throw(uno::RuntimeException);

        // XContainer
        void implAddListener(NodeAccess& rNode, const uno::Reference< css::container::XContainerListener >& xListener )
            throw(uno::RuntimeException);

        void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::container::XContainerListener >& xListener )
            throw(uno::RuntimeException);

        // XChangesNotifier
        void implAddListener(NodeAccess& rNode, const uno::Reference< css::util::XChangesListener >& xListener )
            throw(uno::RuntimeException);

        void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::util::XChangesListener >& xListener )
            throw(uno::RuntimeException);

        // XPropertySet - VetoableChangeListeners
        void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XVetoableChangeListener >& xListener, const rtl::OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XVetoableChangeListener >& xListener, const rtl::OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XPropertySet
        void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XPropertyChangeListener >& xListener, const rtl::OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

         void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XPropertyChangeListener >& xListener, const rtl::OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XMultiPropertySet
        void implAddListener( NodeAccess& rNode, const uno::Reference< css::beans::XPropertiesChangeListener >& xListener, const uno::Sequence< rtl::OUString >& sPropertyNames )
            throw(uno::RuntimeException);

        void implRemoveListener( NodeAccess& rNode, const uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_NOTIFIERIMPL_HXX_


