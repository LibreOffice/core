/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propsetaccessimpl.hxx,v $
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

#ifndef CONFIGMGR_API_PROPERTYSETIMPL_HXX_
#define CONFIGMGR_API_PROPERTYSETIMPL_HXX_

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>

#ifndef CONFIGMGR_NO_PROPERTYSTATE
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (read-only operation)
    */
    namespace configapi
    {
        class NodeAccess;
        class NodeGroupInfoAccess;
        class NodeGroupAccess;

    // getting Property Metadata
        // XPropertySet & XMultiPropertySet
        uno::Reference< beans::XPropertySetInfo > implGetPropertySetInfo( NodeGroupInfoAccess& rNode, sal_Bool _bWriteable )
            throw(uno::RuntimeException);

        // XHierarchicalPropertySet & XHierarchicalMultiPropertySet
        uno::Reference< beans::XHierarchicalPropertySetInfo > implGetHierarchicalPropertySetInfo( NodeGroupInfoAccess& rNode )
            throw(uno::RuntimeException);

    // setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
        // XPropertySet
        void implSetPropertyValue( NodeGroupAccess& rNode, const rtl::OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                      lang::WrappedTargetException, uno::RuntimeException);

         // XMultiPropertySet
       void implSetPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                      lang::WrappedTargetException, uno::RuntimeException);

        // XHierarchicalPropertySet
        void implSetHierarchicalPropertyValue( NodeGroupAccess& rNode, const rtl::OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

         // XMultiHierarchicalPropertySet
       void implSetHierarchicalPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

    // getting values
        // XPropertySet
        uno::Any implGetPropertyValue( NodeGroupInfoAccess& rNode,const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

         // XMultiPropertySet
        uno::Sequence< uno::Any > implGetPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(uno::RuntimeException);

        // XHierarchicalPropertySet
        uno::Any implGetHierarchicalPropertyValue( NodeGroupInfoAccess& rNode, const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

         // XMultiHierarchicalPropertySet
        uno::Sequence< uno::Any > implGetHierarchicalPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(uno::RuntimeException);

    // SPECIAL: firePropertiesChangeEvent
         // XMultiPropertySet
        void implFirePropertiesChangeEvent( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

    // XPropertyState
        beans::PropertyState implGetPropertyState( NodeAccess& rNode, const rtl::OUString& sPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetPropertyToDefault( NodeGroupAccess& rNode, const rtl::OUString& sPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        uno::Any implGetPropertyDefault( NodeGroupInfoAccess& rNode, const rtl::OUString& sPropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    // XMultiPropertyState
        uno::Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetPropertiesToDefault( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetAllPropertiesToDefault( NodeGroupAccess& rNode )
            throw(uno::RuntimeException);

        uno::Sequence< uno::Any > implGetPropertyDefaults( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    }

}
#endif // CONFIGMGR_API_PROPERTYSETIMPL_HXX_


