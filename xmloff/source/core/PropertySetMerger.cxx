/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/XPropertyState.hpp>
#include <PropertySetMerger.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase3.hxx>

namespace {

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
    PropertySetMergerImpl( const Reference< XPropertySet > & rPropSet1, const Reference< XPropertySet > & rPropSet2 );

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) override;
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) override;

    // XPropertyState
    virtual PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual Sequence< PropertyState > SAL_CALL getPropertyStates( const Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  ) override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;
};

}

// Interface implementation

PropertySetMergerImpl::PropertySetMergerImpl( Reference< XPropertySet > const & rPropSet1, Reference< XPropertySet > const & rPropSet2 )
: mxPropSet1( rPropSet1 )
, mxPropSet1State( rPropSet1, UNO_QUERY )
, mxPropSet1Info( rPropSet1->getPropertySetInfo() )
, mxPropSet2( rPropSet2 )
, mxPropSet2State( rPropSet2, UNO_QUERY )
, mxPropSet2Info( rPropSet2->getPropertySetInfo() )
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetMergerImpl::getPropertySetInfo(  )
{
    return this;
}

void SAL_CALL PropertySetMergerImpl::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
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

Any SAL_CALL PropertySetMergerImpl::getPropertyValue( const OUString& PropertyName )
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

void SAL_CALL PropertySetMergerImpl::addPropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ )
{
}

void SAL_CALL PropertySetMergerImpl::removePropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*aListener*/ )
{
}

void SAL_CALL PropertySetMergerImpl::addVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
{
}

void SAL_CALL PropertySetMergerImpl::removeVetoableChangeListener( const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ )
{
}

// XPropertyState
PropertyState SAL_CALL PropertySetMergerImpl::getPropertyState( const OUString& PropertyName )
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

Sequence< PropertyState > SAL_CALL PropertySetMergerImpl::getPropertyStates( const Sequence< OUString >& aPropertyName )
{
    const sal_Int32 nCount = aPropertyName.getLength();
    Sequence< PropertyState > aPropStates( nCount );

    std::transform(aPropertyName.begin(), aPropertyName.end(), aPropStates.begin(),
        [this](const OUString& rPropName) -> PropertyState { return getPropertyState(rPropName); });

    return aPropStates;
}

void SAL_CALL PropertySetMergerImpl::setPropertyToDefault( const OUString& PropertyName )
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

Any SAL_CALL PropertySetMergerImpl::getPropertyDefault( const OUString& aPropertyName )
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
Sequence< Property > SAL_CALL PropertySetMergerImpl::getProperties()
{
    Sequence< Property > aProps1( mxPropSet1Info->getProperties() );
    Sequence< Property > aProps2( mxPropSet2Info->getProperties() );

    return comphelper::concatSequences(aProps1, aProps2);
}

Property SAL_CALL PropertySetMergerImpl::getPropertyByName( const OUString& aName )
{
    if( mxPropSet1Info->hasPropertyByName( aName ) )
        return mxPropSet1Info->getPropertyByName( aName );

    return mxPropSet2Info->getPropertyByName( aName );
}

sal_Bool SAL_CALL PropertySetMergerImpl::hasPropertyByName( const OUString& Name )
{
    if(mxPropSet1Info->hasPropertyByName( Name ) )
        return true;

    return mxPropSet2Info->hasPropertyByName( Name );
}

Reference< XPropertySet > PropertySetMerger_CreateInstance( const Reference< XPropertySet >& rPropSet1, const Reference< XPropertySet >& rPropSet2 ) noexcept
{
    return new PropertySetMergerImpl( rPropSet1, rPropSet2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
