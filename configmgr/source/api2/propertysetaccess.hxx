/*************************************************************************
 *
 *  $RCSfile: propertysetaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:35 $
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

#ifndef CONFIGMGR_API_PROPERTYSET_HXX_
#define CONFIGMGR_API_PROPERTYSET_HXX_

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
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSTATES_HPP_
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

namespace configmgr
{
/////////////////////////////////////////////////////////////////////////////////////////////
    namespace css   = ::com::sun::star;
    namespace uno       = css::uno;
    namespace lang      = css::lang;
    namespace beans     = css::beans;
    using rtl::OUString;

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
    ~BasicPropertySet() {}

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
        setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
   virtual void SAL_CALL
        setPropertyValues( const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

    // XHierarchicalPropertySet
    virtual void SAL_CALL
        setHierarchicalPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
            throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

     // XMultiHierarchicalPropertySet
   virtual void SAL_CALL
        setHierarchicalPropertyValues( const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                  lang::WrappedTargetException, uno::RuntimeException);

// getting values
    // XPropertySet
    virtual uno::Any SAL_CALL
        getPropertyValue( const OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual uno::Sequence< uno::Any > SAL_CALL
        getPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
            throw(uno::RuntimeException);

    // XHierarchicalPropertySet
    virtual uno::Any SAL_CALL
        getHierarchicalPropertyValue( const OUString& PropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiHierarchicalPropertySet
    virtual uno::Sequence< uno::Any > SAL_CALL
        getHierarchicalPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
            throw(uno::RuntimeException);

// adding listeners
    // XPropertySet
    virtual void SAL_CALL
        addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual void SAL_CALL
        addPropertiesChangeListener( const uno::Sequence< OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

// removing listeners
    // XPropertySet
    virtual void SAL_CALL
        removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

     // XMultiPropertySet
    virtual void SAL_CALL
        removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& Listener )
            throw(uno::RuntimeException);

// SPECIAL: support for VetoableChangeListeners
    // XPropertySet
    virtual void SAL_CALL
        addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    virtual void SAL_CALL
        removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

// SPECIAL: firePropertiesChangeEvent
     // XMultiPropertySet
    virtual void SAL_CALL
        firePropertiesChangeEvent( const uno::Sequence< OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
            throw(uno::RuntimeException);

// XPropertyState
    virtual beans::PropertyState SAL_CALL
        getPropertyState( const OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

    // see below:
    // virtual uno::Sequence< beans::PropertyState > SAL_CALL
    //    getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    //      throw(beans::UnknownPropertyException, uno::RuntimeException);

    virtual void SAL_CALL
        setPropertyToDefault( const OUString& PropertyName )
            throw(beans::UnknownPropertyException, uno::RuntimeException);

    virtual uno::Any SAL_CALL
        getPropertyDefault( const OUString& aPropertyName )
            throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

// XMultiPropertyStates
    virtual uno::Sequence< beans::PropertyState > SAL_CALL
        getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
            throw (beans::UnknownPropertyException, uno::RuntimeException);

    virtual void SAL_CALL
        setAllPropertiesToDefault(  )
            throw (uno::RuntimeException);

    virtual void SAL_CALL
        setPropertiesToDefault( const uno::Sequence< OUString >& aPropertyNames )
            throw (beans::UnknownPropertyException, uno::RuntimeException);

    virtual uno::Sequence< uno::Any > SAL_CALL
        getPropertyDefaults( const uno::Sequence< OUString >& aPropertyNames )
            throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

protected:
    virtual configapi::NodeGroupInfoAccess&         getNode() = 0;
            configapi::NodeGroupAccess&             getGroupNode();
    virtual configapi::NodeGroupAccess*             maybeGetUpdateAccess() = 0;
};

//--------------------------------------------------------------------------

}
#endif // CONFIGMGR_API_PROPERTYSET_HXX_


