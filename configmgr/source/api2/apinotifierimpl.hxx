/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apinotifierimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:08:24 $
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

#ifndef CONFIGMGR_API_NOTIFIERIMPL_HXX_
#define CONFIGMGR_API_NOTIFIERIMPL_HXX_

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    using rtl::OUString;

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
        void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XVetoableChangeListener >& xListener, const OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XVetoableChangeListener >& xListener, const OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XPropertySet
        void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XPropertyChangeListener >& xListener, const OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

         void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< css::beans::XPropertyChangeListener >& xListener, const OUString& sPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XMultiPropertySet
        void implAddListener( NodeAccess& rNode, const uno::Reference< css::beans::XPropertiesChangeListener >& xListener, const uno::Sequence< OUString >& sPropertyNames )
            throw(uno::RuntimeException);

        void implRemoveListener( NodeAccess& rNode, const uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_NOTIFIERIMPL_HXX_


