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


#include "comphelper/propertysetinfo.hxx"
#include "comphelper/propertysethelper.hxx"

///////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper
{
class PropertySetHelperImpl
{
public:
    PropertyMapEntry* find( const OUString& aName ) const throw();

    PropertySetInfo* mpInfo;
};
}

PropertyMapEntry* PropertySetHelperImpl::find( const OUString& aName ) const throw()
{
    PropertyMap::const_iterator aIter = mpInfo->getPropertyMap()->find( aName );

    if( mpInfo->getPropertyMap()->end() != aIter )
    {
        return (*aIter).second;
    }
    else
    {
        return NULL;
    }
}

///////////////////////////////////////////////////////////////////////

PropertySetHelper::PropertySetHelper( comphelper::PropertySetInfo* pInfo ) throw()
{
    mp = new PropertySetHelperImpl;
    mp->mpInfo = pInfo;
    pInfo->acquire();
}

PropertySetHelper::PropertySetHelper( comphelper::PropertySetInfo* pInfo, __sal_NoAcquire ) throw()
{
    mp = new PropertySetHelperImpl;
    mp->mpInfo = pInfo;
}

PropertySetHelper::~PropertySetHelper() throw()
{
    mp->mpInfo->release();
    delete mp;
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo(  ) throw(RuntimeException)
{
    return mp->mpInfo;
}

void SAL_CALL PropertySetHelper::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];
    aEntries[0] = mp->find( aPropertyName );

    if( NULL == aEntries[0] )
        throw UnknownPropertyException( aPropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = NULL;

    _setPropertyValues( (const PropertyMapEntry**)aEntries, &aValue );
}

Any SAL_CALL PropertySetHelper::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];
    aEntries[0] = mp->find( PropertyName );

    if( NULL == aEntries[0] )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = NULL;

    Any aAny;
    _getPropertyValues( (const PropertyMapEntry**)aEntries, &aAny );

    return aAny;
}

void SAL_CALL PropertySetHelper::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL PropertySetHelper::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();

    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
        pEntries[nCount] = NULL;
        const OUString* pNames = aPropertyNames.getConstArray();

        sal_Bool bUnknown = sal_False;
        sal_Int32 n;
        for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        if( !bUnknown )
            _setPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getConstArray() );

        delete[] pEntries;

        if( bUnknown )
            throw UnknownPropertyException( *pNames, static_cast< XPropertySet* >( this ) );
    }
}

Sequence< Any > SAL_CALL PropertySetHelper::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw(RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence< Any > aValues;
    if( nCount )
    {
        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
        pEntries[nCount] = NULL;
        const OUString* pNames = aPropertyNames.getConstArray();

        sal_Bool bUnknown = sal_False;
        sal_Int32 n;
        for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        if( !bUnknown )
        {
            aValues.realloc(nCount);
            _getPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getArray() );
        }

        delete[] pEntries;

        if( bUnknown )
            throw UnknownPropertyException( *pNames, static_cast< XPropertySet* >( this ) );
    }

    return aValues;
}

void SAL_CALL PropertySetHelper::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw(RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& ) throw(RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw(RuntimeException)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL PropertySetHelper::getPropertyState( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];

    aEntries[0] = mp->find( PropertyName );
    if( aEntries[0] == NULL )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    aEntries[1] = NULL;

    PropertyState aState;
    _getPropertyStates( (const PropertyMapEntry**)aEntries, &aState );

    return aState;
}

Sequence< PropertyState > SAL_CALL PropertySetHelper::getPropertyStates( const Sequence< OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();

    Sequence< PropertyState > aStates( nCount );

    if( nCount )
    {
        const OUString* pNames = aPropertyName.getConstArray();

        sal_Bool bUnknown = sal_False;

        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];

        sal_Int32 n;
        for( n = 0; !bUnknown && (n < nCount); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        pEntries[nCount] = NULL;

        if( !bUnknown )
            _getPropertyStates( (const PropertyMapEntry**)pEntries, aStates.getArray() );

        delete[] pEntries;

        if( bUnknown )
            throw UnknownPropertyException( *pNames, static_cast< XPropertySet* >( this ) );
    }

    return aStates;
}

void SAL_CALL PropertySetHelper::setPropertyToDefault( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    PropertyMapEntry *pEntry  = mp->find( PropertyName );
    if( NULL == pEntry )
        throw UnknownPropertyException( PropertyName, static_cast< XPropertySet* >( this ) );

    _setPropertyToDefault( pEntry );
}

Any SAL_CALL PropertySetHelper::getPropertyDefault( const OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* pEntry = mp->find( aPropertyName );
    if( NULL == pEntry )
        throw UnknownPropertyException( aPropertyName, static_cast< XPropertySet* >( this ) );

    return _getPropertyDefault( pEntry );
}

void PropertySetHelper::_getPropertyStates(
        const comphelper::PropertyMapEntry**, PropertyState*)
throw (UnknownPropertyException, RuntimeException)
{
    OSL_FAIL( "you have to implement this yourself!");
}

void
PropertySetHelper::_setPropertyToDefault(const comphelper::PropertyMapEntry*)
throw (UnknownPropertyException, RuntimeException)
{
    OSL_FAIL( "you have to implement this yourself!");
}

Any PropertySetHelper::_getPropertyDefault(const comphelper::PropertyMapEntry*)
throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "you have to implement this yourself!");

    Any aAny;
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
