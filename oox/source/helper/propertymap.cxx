/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertymap.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/helper/propertymap.hxx"
#include <osl/mutex.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyVetoException;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::beans::XVetoableChangeListener;

namespace oox {

// ============================================================================

namespace {

/** Thread-save singleton of a vector of all supported property names. */
struct PropertyNamesPool : public ::rtl::Static< PropertyNamesList, PropertyNamesPool > {};

// ----------------------------------------------------------------------------

typedef ::cppu::WeakImplHelper2< XPropertySet, XPropertySetInfo > GenericPropertySetImplBase;

/** This class implements a generic XPropertySet.

    Properties of all names and types can be set and later retrieved.
    TODO: move this to comphelper or better find an existing implementation
 */
class GenericPropertySet : public GenericPropertySetImplBase, private ::osl::Mutex
{
public:
    explicit            GenericPropertySet();
    explicit            GenericPropertySet( const PropertyMap& rPropMap );

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() throw (RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw (RuntimeException);

private:
    typedef ::std::map< OUString, Any > PropertyNameMap;
    PropertyNameMap     maPropMap;
};

// ----------------------------------------------------------------------------

GenericPropertySet::GenericPropertySet()
{
}

GenericPropertySet::GenericPropertySet( const PropertyMap& rPropMap )
{
    const PropertyNamesList& rPropNames = PropertyNamesPool::get();
    for( PropertyMap::const_iterator aIt = rPropMap.begin(), aEnd = rPropMap.end(); aIt != aEnd; ++aIt )
        maPropMap[ rPropNames[ aIt->first ] ] = aIt->second;
}

Reference< XPropertySetInfo > SAL_CALL GenericPropertySet::getPropertySetInfo() throw (RuntimeException)
{
    return this;
}

void SAL_CALL GenericPropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *this );
    maPropMap[ rPropertyName ] = rValue;
}

Any SAL_CALL GenericPropertySet::getPropertyValue( const OUString& rPropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyNameMap::iterator aIt = maPropMap.find( rPropertyName );
    if( aIt == maPropMap.end() )
        throw UnknownPropertyException();
    return aIt->second;
}

// listeners are not supported by this implementation
void SAL_CALL GenericPropertySet::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}

// XPropertySetInfo
Sequence< Property > SAL_CALL GenericPropertySet::getProperties() throw (RuntimeException)
{
    Sequence< Property > aSeq( static_cast< sal_Int32 >( maPropMap.size() ) );
    Property* pProperty = aSeq.getArray();
    for( PropertyNameMap::iterator aIt = maPropMap.begin(), aEnd = maPropMap.end(); aIt != aEnd; ++aIt, ++pProperty )
    {
        pProperty->Name = aIt->first;
        pProperty->Handle = 0;
        pProperty->Type = aIt->second.getValueType();
        pProperty->Attributes = 0;
    }
    return aSeq;
}

Property SAL_CALL GenericPropertySet::getPropertyByName( const OUString& rPropertyName ) throw (UnknownPropertyException, RuntimeException)
{
    PropertyNameMap::iterator aIt = maPropMap.find( rPropertyName );
    if( aIt == maPropMap.end() )
        throw UnknownPropertyException();
    Property aProperty;
    aProperty.Name = aIt->first;
    aProperty.Handle = 0;
    aProperty.Type = aIt->second.getValueType();
    aProperty.Attributes = 0;
    return aProperty;
}

sal_Bool SAL_CALL GenericPropertySet::hasPropertyByName( const OUString& rPropertyName ) throw (RuntimeException)
{
    return maPropMap.find( rPropertyName ) != maPropMap.end();
}

} // namespace

// ============================================================================

const OUString& PropertyMap::getPropertyName( sal_Int32 nPropId )
{
    OSL_ENSURE( (0 <= nPropId) && (nPropId < PROP_COUNT), "PropertyMap::getPropertyName - invalid property identifier" );
    return PropertyNamesPool::get()[ nPropId ];
}

const Any* PropertyMap::getProperty( sal_Int32 nPropId ) const
{
    const_iterator aIt = find( nPropId );
    return (aIt == end()) ? 0 : &aIt->second;
}

Sequence< PropertyValue > PropertyMap::makePropertyValueSequence() const
{
    Sequence< PropertyValue > aSeq( static_cast< sal_Int32 >( size() ) );
    if( !empty() )
    {
        const PropertyNamesList& rPropNames = PropertyNamesPool::get();
        PropertyValue* pValues = aSeq.getArray();
        for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt, ++pValues )
        {
            OSL_ENSURE( (0 <= aIt->first) && (aIt->first < PROP_COUNT), "PropertyMap::makePropertyValueSequence - invalid property identifier" );
            pValues->Name = rPropNames[ aIt->first ];
            pValues->Value = aIt->second;
            pValues->State = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
        }
    }
    return aSeq;
}

void PropertyMap::fillSequences( Sequence< OUString >& rNames, Sequence< Any >& rValues ) const
{
    rNames.realloc( static_cast< sal_Int32 >( size() ) );
    rValues.realloc( static_cast< sal_Int32 >( size() ) );
    if( !empty() )
    {
        const PropertyNamesList& rPropNames = PropertyNamesPool::get();
        OUString* pNames = rNames.getArray();
        Any* pValues = rValues.getArray();
        for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt, ++pNames, ++pValues )
        {
            OSL_ENSURE( (0 <= aIt->first) && (aIt->first < PROP_COUNT), "PropertyMap::fillSequences - invalid property identifier" );
            *pNames = rPropNames[ aIt->first ];
            *pValues = aIt->second;
        }
    }
}

Reference< XPropertySet > PropertyMap::makePropertySet() const
{
    return new GenericPropertySet( *this );
}

// ============================================================================

} // namespace oox

