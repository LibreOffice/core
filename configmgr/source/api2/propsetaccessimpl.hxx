/*************************************************************************
 *
 *  $RCSfile: propsetaccessimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-21 19:19:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_PROPERTYSETIMPL_HXX_
#define CONFIGMGR_API_PROPERTYSETIMPL_HXX_

#ifndef CONFIGMGR_APITYPES_HXX_
#include "apitypes.hxx"
#endif

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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
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
        beans::PropertyState implGetPropertyState( NodeAccess& rNode, const OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        uno::Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const uno::Sequence< OUString >& aPropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        void implSetPropertyToDefault( NodeGroupAccess& rNode, const OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

        uno::Any implGetPropertyDefault( NodeAccess& rNode, const OUString& aPropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    }

}
#endif // CONFIGMGR_API_PROPERTYSETIMPL_HXX_


