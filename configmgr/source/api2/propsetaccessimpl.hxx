/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propsetaccessimpl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:18:45 $
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

#ifndef CONFIGMGR_API_PROPERTYSETIMPL_HXX_
#define CONFIGMGR_API_PROPERTYSETIMPL_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XHIERARCHICALPROPERTYSET_HPP_
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIHIERARCHICALPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#endif

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
    using rtl::OUString;

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
        void implSetPropertyValue( NodeGroupAccess& rNode, const OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                      lang::WrappedTargetException, uno::RuntimeException);

         // XMultiPropertySet
       void implSetPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                      lang::WrappedTargetException, uno::RuntimeException);

        // XHierarchicalPropertySet
        void implSetHierarchicalPropertyValue( NodeGroupAccess& rNode, const OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

         // XMultiHierarchicalPropertySet
       void implSetHierarchicalPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

    // getting values
        // XPropertySet
        uno::Any implGetPropertyValue( NodeGroupInfoAccess& rNode,const OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

         // XMultiPropertySet
        uno::Sequence< uno::Any > implGetPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< OUString >& aPropertyNames )
            throw(uno::RuntimeException);

        // XHierarchicalPropertySet
        uno::Any implGetHierarchicalPropertyValue( NodeGroupInfoAccess& rNode, const OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

         // XMultiHierarchicalPropertySet
        uno::Sequence< uno::Any > implGetHierarchicalPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< OUString >& aPropertyNames )
            throw(uno::RuntimeException);

    // SPECIAL: firePropertiesChangeEvent
         // XMultiPropertySet
        void implFirePropertiesChangeEvent( NodeGroupInfoAccess& rNode, const uno::Sequence< OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

    // XPropertyState
        beans::PropertyState implGetPropertyState( NodeAccess& rNode, const OUString& sPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetPropertyToDefault( NodeGroupAccess& rNode, const OUString& sPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        uno::Any implGetPropertyDefault( NodeGroupInfoAccess& rNode, const OUString& sPropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    // XMultiPropertyState
        uno::Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const uno::Sequence< OUString >& aPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetPropertiesToDefault( NodeGroupAccess& rNode, const uno::Sequence< OUString >& aPropertyNames )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetAllPropertiesToDefault( NodeGroupAccess& rNode )
            throw(uno::RuntimeException);

        uno::Sequence< uno::Any > implGetPropertyDefaults( NodeGroupInfoAccess& rNode, const uno::Sequence< OUString >& aPropertyNames )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    }

}
#endif // CONFIGMGR_API_PROPERTYSETIMPL_HXX_


