/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:14:02 $
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

#ifndef CONFIGMGR_API_GROUPACCESS_HXX_
#define CONFIGMGR_API_GROUPACCESS_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTY_HPP_
#include <com/sun/star/beans/XProperty.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    using rtl::OUString;

    namespace configapi { class NodeGroupInfoAccess; }

    /** implements the (read-only) interfaces supported by a group node
        within the configuration tree.
        <p> Is an interface adapter around <type scope='configmgr::configapi'>NodeAccess</type>.</p>
    */
    class BasicGroupAccess
    : public ::cppu::ImplHelper6
                < css::container::XNameAccess
                , css::container::XHierarchicalName
                , css::container::XHierarchicalNameAccess
                , css::container::XContainer
                , css::beans::XExactName
                , css::beans::XProperty
                >
    {
    protected:
    // Destructors
        virtual ~BasicGroupAccess() {}

    public:
    // Interface methods

        // XHierarchicalName
        virtual OUString SAL_CALL
            getHierarchicalName(  )
                throw(uno::RuntimeException);

        virtual OUString SAL_CALL
            composeHierarchicalName( const OUString& aRelativeName )
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
            getByName( const OUString& aName )
                throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
                         uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
            getElementNames(  )
                throw( uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasByName( const OUString& aName )
                throw(uno::RuntimeException);

        // XHierarchicalNameAccess
        virtual uno::Any SAL_CALL
            getByHierarchicalName( const OUString& aName )
                throw(css::container::NoSuchElementException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            hasByHierarchicalName( const OUString& aName )
                throw(uno::RuntimeException);

        // XContainer
        virtual void SAL_CALL
            addContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
                throw(uno::RuntimeException);

        virtual void SAL_CALL
            removeContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
                throw(uno::RuntimeException);

        // XExactName
        virtual ::rtl::OUString SAL_CALL
            getExactName( const OUString& aApproximateName )
                throw(uno::RuntimeException);

        // XProperty
        virtual css::beans::Property SAL_CALL
            getAsProperty(  )
                throw(uno::RuntimeException);

    protected:
        virtual configapi::NodeGroupInfoAccess& getNode() = 0;
    };

}
#endif // CONFIGMGR_API_GROUPACCESS_HXX_


