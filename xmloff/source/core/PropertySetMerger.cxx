/*************************************************************************
 *
 *  $RCSfile: PropertySetMerger.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-08 11:30:57 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class SvXMLAttrContainerItem_Impl;

class PropertySetMergerImpl : public ::cppu::WeakAggImplHelper3< XPropertySet, XPropertyState, XPropertySetInfo >
{
private:
    Reference< XPropertySet > mxPropSet1;
    Reference< XPropertyState > mxPropSet1State;
    Reference< XPropertySetInfo > mxPropSet1Info;

    Reference< XPropertySet > mxPropSet2;
    Reference< XPropertyState > mxPropSet2State;
    Reference< XPropertySetInfo > mxPropSet2Info;

public:
    PropertySetMergerImpl( const Reference< XPropertySet > rPropSet1, const Reference< XPropertySet > rPropSet2 );
    virtual ~PropertySetMergerImpl();

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertyState
    virtual PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException);
    virtual Sequence< PropertyState > SAL_CALL getPropertyStates( const Sequence< OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException);
    virtual Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  ) throw(RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw(RuntimeException);
};

// --------------------------------------------------------------------
// Interface implementation
// --------------------------------------------------------------------

PropertySetMergerImpl::PropertySetMergerImpl( Reference< XPropertySet > rPropSet1, Reference< XPropertySet > rPropSet2 )
: mxPropSet1( rPropSet1 ), mxPropSet2( rPropSet2 ),
  mxPropSet1State( rPropSet1, UNO_QUERY ), mxPropSet2State( rPropSet2, UNO_QUERY ),
  mxPropSet1Info( rPropSet1->getPropertySetInfo() ), mxPropSet2Info( rPropSet2->getPropertySetInfo() )
{
}

PropertySetMergerImpl::~PropertySetMergerImpl()
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetMergerImpl::getPropertySetInfo(  ) throw(RuntimeException)
{
    return this;
}

void SAL_CALL PropertySetMergerImpl::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if( mxPropSet1Info->hasPropertyByName( aPropertyName ) )
    {
        mxPropSet1->setPropertyValue( aPropertyName, aValue );
    }
    else
    {
        mxPropSet2->setPropertyValue( aPropertyName, aValue );
    }
}

Any SAL_CALL PropertySetMergerImpl::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if( mxPropSet1Info->hasPropertyByName( PropertyName ) )
    {
        return mxPropSet1->getPropertyValue( PropertyName );
    }
    else
    {
        return mxPropSet2->getPropertyValue( PropertyName );
    }
}

void SAL_CALL PropertySetMergerImpl::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL PropertySetMergerImpl::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL PropertySetMergerImpl::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL PropertySetMergerImpl::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// XPropertyState
PropertyState SAL_CALL PropertySetMergerImpl::getPropertyState( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    if( mxPropSet1Info->hasPropertyByName( PropertyName ) )
    {
        if( mxPropSet1State.is() )
        {
            return mxPropSet1State->getPropertyState( PropertyName );
        }
        else
        {
            return PropertyState_DIRECT_VALUE;
        }
    }
    else
    {
        if( mxPropSet2State.is() )
        {
            return mxPropSet2State->getPropertyState( PropertyName );
        }
        else
        {
            return PropertyState_DIRECT_VALUE;
        }
    }
}

Sequence< PropertyState > SAL_CALL PropertySetMergerImpl::getPropertyStates( const Sequence< OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();
    Sequence< PropertyState > aPropStates( nCount );
    PropertyState* pPropStates = aPropStates.getArray();
    const OUString* pPropNames = aPropertyName.getConstArray();

    sal_Int32 nIndex;
    for( nIndex = 0; nIndex < nCount; nIndex++ )
        *pPropStates++ = getPropertyState( *pPropNames++ );

    return aPropStates;
}

void SAL_CALL PropertySetMergerImpl::setPropertyToDefault( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    if( mxPropSet1State.is() && mxPropSet1Info->hasPropertyByName( PropertyName ) )
    {
        mxPropSet1State->setPropertyToDefault( PropertyName );
    }
    else
    {
        if( mxPropSet2State.is() )
        {
            mxPropSet2State->setPropertyToDefault( PropertyName );
        }
    }
}

Any SAL_CALL PropertySetMergerImpl::getPropertyDefault( const OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if( mxPropSet1State.is() && mxPropSet1Info->hasPropertyByName( aPropertyName ) )
    {
        return mxPropSet1State->getPropertyDefault( aPropertyName );
    }
    else
    {
        if( mxPropSet2State.is() )
        {
            return mxPropSet2State->getPropertyDefault( aPropertyName );
        }
        else
        {
            Any aAny;
            return aAny;
        }
    }
}

// XPropertySetInfo
Sequence< Property > SAL_CALL PropertySetMergerImpl::getProperties() throw(RuntimeException)
{
    Sequence< Property > aProps1( mxPropSet1Info->getProperties() );
    const Property* pProps1 = aProps1.getArray();
    const sal_Int32 nCount1 = aProps1.getLength();

    Sequence< Property > aProps2( mxPropSet1Info->getProperties() );
    const Property* pProps2 = aProps2.getArray();
    const sal_Int32 nCount2 = aProps2.getLength();

    Sequence< Property > aProperties( nCount1 + nCount2 );

    sal_Int32 nIndex;

    Property* pProperties;

    for( nIndex = 0; nIndex < nCount1; nIndex++ )
        *pProperties++ = *pProps1++;

    for( nIndex = 0; nIndex < nCount2; nIndex++ )
        *pProperties++ = *pProps2++;

    return aProperties;
}

Property SAL_CALL PropertySetMergerImpl::getPropertyByName( const OUString& aName ) throw(UnknownPropertyException, RuntimeException)
{
    if( mxPropSet1Info->hasPropertyByName( aName ) )
        return mxPropSet1Info->getPropertyByName( aName );

    return mxPropSet2Info->getPropertyByName( aName );
}

sal_Bool SAL_CALL PropertySetMergerImpl::hasPropertyByName( const OUString& Name ) throw(RuntimeException)
{
    if(mxPropSet1Info->hasPropertyByName( Name ) )
        return sal_True;

    return mxPropSet2Info->hasPropertyByName( Name );
}

Reference< XPropertySet > PropertySetMerger_CreateInstance( Reference< XPropertySet > rPropSet1, Reference< XPropertySet > rPropSet2 ) throw()
{
    return new PropertySetMergerImpl( rPropSet1, rPropSet2 );
}
