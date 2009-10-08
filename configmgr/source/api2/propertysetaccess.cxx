/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysetaccess.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "propertysetaccess.hxx"
#include "propsetaccessimpl.hxx"
#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"

namespace configmgr
{
//////////////////////////////////////////////////////////////////////////////////
// class BasicPropertySet
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// getting Property Metadata
//////////////////////////////////////////////////////////////////////////////////

// XPropertySet & XMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Reference< beans::XPropertySetInfo > SAL_CALL BasicPropertySet::getPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    return configapi::implGetPropertySetInfo( getNode(), NULL != maybeGetUpdateAccess() );
}

// XHierarchicalPropertySet & XHierarchicalMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Reference< beans::XHierarchicalPropertySetInfo > SAL_CALL BasicPropertySet::getHierarchicalPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    return configapi::implGetHierarchicalPropertySetInfo( getNode() );
}

//////////////////////////////////////////////////////////////////////////////////
// setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
//////////////////////////////////////////////////////////////////////////////////

/// get the access for updating, check that it is present
configapi::NodeGroupAccess& BasicPropertySet::getGroupNode()
{
    configapi::NodeGroupAccess* pAccess = maybeGetUpdateAccess();
    OSL_ENSURE(pAccess, "Write operation invoked on a read-only node access - failing with PropertyVetoException");

    if (!pAccess)
    {
        throw beans::PropertyVetoException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: INTERNAL VETO - Write operation invoked on a read-only node access")),
                static_cast< beans::XPropertySet * >(this)
            );
    }
    return *pAccess;
}

// XPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetPropertyValue( getGroupNode(), aPropertyName, aValue );
}

// XMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setPropertyValues( const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetPropertyValues( getGroupNode(), PropertyNames, Values );
}

// XHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setHierarchicalPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetHierarchicalPropertyValue( getGroupNode(), aPropertyName, aValue );
}

// XMultiHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setHierarchicalPropertyValues( const uno::Sequence< rtl::OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetHierarchicalPropertyValues( getGroupNode(), PropertyNames, Values );
}


//////////////////////////////////////////////////////////////////////////////////
// getting values
//////////////////////////////////////////////////////////////////////////////////
// XPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicPropertySet::getPropertyValue( const rtl::OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetPropertyValue( getNode(), aPropertyName );
}

// XMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Any > SAL_CALL BasicPropertySet::getPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    return configapi::implGetPropertyValues( getNode(), aPropertyNames );
}

// XHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicPropertySet::getHierarchicalPropertyValue( const rtl::OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetHierarchicalPropertyValue( getNode(), aPropertyName );
}

// XMultiHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Any > SAL_CALL BasicPropertySet::getHierarchicalPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    return configapi::implGetHierarchicalPropertyValues( getNode(), aPropertyNames );
}

//////////////////////////////////////////////////////////////////////////////////
// adding/removing listeners
//////////////////////////////////////////////////////////////////////////////////

void SAL_CALL BasicPropertySet::addVetoableChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XVetoableChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implAddListener( getNode(), xListener, aPropertyName );
}

void SAL_CALL BasicPropertySet::addPropertyChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implAddListener( getNode(), xListener, aPropertyName );
}

void SAL_CALL BasicPropertySet::addPropertiesChangeListener( const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNode(), xListener, aPropertyNames );
}

//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::removeVetoableChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XVetoableChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener, aPropertyName );
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::removePropertyChangeListener( const rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener, aPropertyName );
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener );
}
//---------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////////
// SPECIAL: XMultiPropertySet::firePropertiesChangeEvent
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::firePropertiesChangeEvent( const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implFirePropertiesChangeEvent( getNode(),aPropertyNames , xListener );
}

//////////////////////////////////////////////////////////////////////////////////
// XPropertyState / XMultiPropertyStates
//////////////////////////////////////////////////////////////////////////////////

// getting property states
//////////////////////////////////////////////////////////////////////////////////

beans::PropertyState SAL_CALL BasicPropertySet::getPropertyState( const rtl::OUString& sPropertyName )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return configapi::implGetPropertyState( getNode(), sPropertyName);
}
//---------------------------------------------------------------------------------

uno::Sequence< beans::PropertyState > SAL_CALL BasicPropertySet::getPropertyStates( const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return configapi::implGetPropertyStates( getNode(), aPropertyNames );
}
//---------------------------------------------------------------------------------

// setting to default state
//////////////////////////////////////////////////////////////////////////////////

void SAL_CALL BasicPropertySet::setPropertyToDefault( const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    configapi::implSetPropertyToDefault( getGroupNode(), sPropertyName);
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::setPropertiesToDefault( const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    configapi::implSetPropertiesToDefault( getGroupNode(), aPropertyNames);
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::setAllPropertiesToDefault(  )
    throw (uno::RuntimeException)
{
    configapi::implSetAllPropertiesToDefault( getGroupNode() );
}
//---------------------------------------------------------------------------------

// getting defaults
//////////////////////////////////////////////////////////////////////////////////

uno::Any SAL_CALL BasicPropertySet::getPropertyDefault( const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)

{
    return configapi::implGetPropertyDefault( getNode(), sPropertyName);
}
//---------------------------------------------------------------------------------

uno::Sequence< uno::Any > SAL_CALL BasicPropertySet::getPropertyDefaults( const uno::Sequence< rtl::OUString >& aPropertyNames )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetPropertyDefaults( getNode(), aPropertyNames);
}
//---------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
} // namespace configmgr


