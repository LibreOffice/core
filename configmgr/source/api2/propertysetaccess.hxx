/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysetaccess.hxx,v $
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

#ifndef CONFIGMGR_API_PROPERTYSET_HXX_
#define CONFIGMGR_API_PROPERTYSET_HXX_

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <cppuhelper/implbase6.hxx>

namespace configmgr
{
/////////////////////////////////////////////////////////////////////////////////////////////
    namespace css   = ::com::sun::star;
    namespace uno       = css::uno;
    namespace lang      = css::lang;
    namespace beans     = css::beans;

    namespace configapi
    {
        class NodeGroupAccess;
        class NodeGroupInfoAccess;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

/** implements the interfaces supported by a group node, viewed as a property set
    within the configuration tree.
    <p> Is an interface adapter around <type scope='configmgr::configapi'>NodeGroup(Info)Access</type>.</p>
*/
class BasicPropertySet
: public ::cppu::ImplHelper6
            <   beans::XPropertySet
            ,   beans::XMultiPropertySet
            ,   beans::XHierarchicalPropertySet
            ,   beans::XMultiHierarchicalPropertySet
            ,   beans::XPropertyState
            ,   beans::XMultiPropertyStates
            >
{
protected:
// Constructors & Destructors
    virtual ~BasicPropertySet() {}

public:
// getting Property Metadata
    // XPropertySet & XMultiPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(  )
            throw(uno::RuntimeException);

    // XHierarchicalPropertySet & XHierarchicalMultiPropertySet
    virtual uno::Reference< beans::XHierarchicalPropertySetInfo > SAL_CALL
        getHierarchicalPropertySetInfo(  )
            throw(uno::RuntimeException);

// setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
    // XPropertySet
    virtual void SAL_CALL
        setPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
   virtual void SAL_CALL
        setPropertyValues( const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

    // XHierarchicalPropertySet
    virtual void SAL_CALL
        setHierarchicalPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

     // XMultiHierarchicalPropertySet
   virtual void SAL_CALL
        setHierarchicalPropertyValues( const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

// getting values
    // XPropertySet
    virtual uno::Any SAL_CALL
        getPropertyValue( const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual uno::Sequence< uno::Any > SAL_CALL
        getPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(uno::RuntimeException);

    // XHierarchicalPropertySet
    virtual uno::Any SAL_CALL
        getHierarchicalPropertyValue( const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiHierarchicalPropertySet
    virtual uno::Sequence< uno::Any > SAL_CALL
        getHierarchicalPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw(uno::RuntimeException);

// adding listeners
    // XPropertySet
    virtual void SAL_CALL
        addPropertyChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual void SAL_CALL
        addPropertiesChangeListener( const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

// removing listeners
    // XPropertySet
    virtual void SAL_CALL
        removePropertyChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual void SAL_CALL
        removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& Listener )
            throw(uno::RuntimeException);

// SPECIAL: support for VetoableChangeListeners
    // XPropertySet
    virtual void SAL_CALL
        addVetoableChangeListener( const rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    virtual void SAL_CALL
        removeVetoableChangeListener( const rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

// SPECIAL: firePropertiesChangeEvent
     // XMultiPropertySet
    virtual void SAL_CALL
        firePropertiesChangeEvent( const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

// XPropertyState
    virtual beans::PropertyState SAL_CALL
        getPropertyState( const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

    // see below:
    // virtual uno::Sequence< beans::PropertyState > SAL_CALL
    //    getPropertyStates( const uno::Sequence< rtl::OUString >& aPropertyName )
    //      throw(beans::UnknownPropertyException, uno::RuntimeException);

    virtual void SAL_CALL
        setPropertyToDefault( const rtl::OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

    virtual uno::Any SAL_CALL
        getPropertyDefault( const rtl::OUString& aPropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

// XMultiPropertyStates
    virtual uno::Sequence< beans::PropertyState > SAL_CALL
        getPropertyStates( const uno::Sequence< rtl::OUString >& aPropertyName )
            throw (beans::UnknownPropertyException, uno::RuntimeException);

    virtual void SAL_CALL
        setAllPropertiesToDefault(  )
            throw (uno::RuntimeException);

    virtual void SAL_CALL
        setPropertiesToDefault( const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw (beans::UnknownPropertyException, uno::RuntimeException);

    virtual uno::Sequence< uno::Any > SAL_CALL
        getPropertyDefaults( const uno::Sequence< rtl::OUString >& aPropertyNames )
            throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

protected:
    virtual configapi::NodeGroupInfoAccess&         getNode() = 0;
            configapi::NodeGroupAccess&             getGroupNode();
    virtual configapi::NodeGroupAccess*             maybeGetUpdateAccess() = 0;
};

//--------------------------------------------------------------------------

}
#endif // CONFIGMGR_API_PROPERTYSET_HXX_


