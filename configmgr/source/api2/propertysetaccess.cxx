/*************************************************************************
 *
 *  $RCSfile: propertysetaccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-21 19:13:57 $
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
 *  Source License Version 1.1 (the "License")  You may not use this file
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
 *  Contributor(s); _______________________________________
 *
 *
 ************************************************************************/

#include "propertysetaccess.hxx"
#include "propsetaccessimpl.hxx"
#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"

namespace configmgr
{
//////////////////////////////////////////////////////////////////////////////////
    using namespace configapi;

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

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
    return implGetPropertySetInfo( getNode(), NULL != maybeGetUpdateAccess() );
}

// XHierarchicalPropertySet & XHierarchicalMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Reference< beans::XHierarchicalPropertySetInfo > SAL_CALL BasicPropertySet::getHierarchicalPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    return implGetHierarchicalPropertySetInfo( getNode() );
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: INTERNAL VETO - Write operation invoked on a read-only node access")),
                static_cast< beans::XPropertySet * >(this)
            );
    }
    return *pAccess;
}

// XPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    implSetPropertyValue( getGroupNode(), aPropertyName, aValue );
}

// XMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setPropertyValues( const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    implSetPropertyValues( getGroupNode(), PropertyNames, Values );
}

// XHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setHierarchicalPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    implSetHierarchicalPropertyValue( getGroupNode(), aPropertyName, aValue );
}

// XMultiHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::setHierarchicalPropertyValues( const uno::Sequence< OUString >& PropertyNames, const uno::Sequence< uno::Any >& Values )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    implSetHierarchicalPropertyValues( getGroupNode(), PropertyNames, Values );
}


//////////////////////////////////////////////////////////////////////////////////
// getting values
//////////////////////////////////////////////////////////////////////////////////
// XPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicPropertySet::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return implGetPropertyValue( getNode(), aPropertyName );
}

// XMultiPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Any > SAL_CALL BasicPropertySet::getPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    return implGetPropertyValues( getNode(), aPropertyNames );
}

// XHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicPropertySet::getHierarchicalPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return implGetHierarchicalPropertyValue( getNode(), aPropertyName );
}

// XMultiHierarchicalPropertySet
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Any > SAL_CALL BasicPropertySet::getHierarchicalPropertyValues( const uno::Sequence< OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    return implGetHierarchicalPropertyValues( getNode(), aPropertyNames );
}

//////////////////////////////////////////////////////////////////////////////////
// adding/removing listeners
//////////////////////////////////////////////////////////////////////////////////

void SAL_CALL BasicPropertySet::addVetoableChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XVetoableChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    implAddListener( getNode(), xListener, aPropertyName );
}

void SAL_CALL BasicPropertySet::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    implAddListener( getNode(), xListener, aPropertyName );
}

void SAL_CALL BasicPropertySet::addPropertiesChangeListener( const uno::Sequence< OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    implAddListener( getNode(), xListener, aPropertyNames );
}

//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::removeVetoableChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XVetoableChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    implRemoveListener( getNode(), xListener, aPropertyName );
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    implRemoveListener( getNode(), xListener, aPropertyName );
}
//---------------------------------------------------------------------------------

void SAL_CALL BasicPropertySet::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    implRemoveListener( getNode(), xListener );
}
//---------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////////
// SPECIAL: XMultiPropertySet::firePropertiesChangeEvent
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicPropertySet::firePropertiesChangeEvent( const uno::Sequence< OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    implFirePropertiesChangeEvent( getNode(),aPropertyNames , xListener );
}

//////////////////////////////////////////////////////////////////////////////////
// XPropertyState
//////////////////////////////////////////////////////////////////////////////////
beans::PropertyState SAL_CALL BasicPropertySet::getPropertyState( const OUString& sPropertyName )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return implGetPropertyState( getNode(), sPropertyName);
}

uno::Sequence< beans::PropertyState > SAL_CALL BasicPropertySet::getPropertyStates( const uno::Sequence< OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return implGetPropertyStates( getNode(), aPropertyNames );
}

void SAL_CALL BasicPropertySet::setPropertyToDefault( const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    implSetPropertyToDefault( getGroupNode(), sPropertyName);
}

uno::Any SAL_CALL BasicPropertySet::getPropertyDefault( const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)

{
    return implGetPropertyDefault( getNode(), sPropertyName);
}

//-----------------------------------------------------------------------------------
} // namespace configmgr


