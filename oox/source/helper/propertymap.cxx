/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include "oox/token/propertynames.hxx"

namespace oox {
using ::com::sun::star::container::XIndexReplace;

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using ::com::sun::star::drawing::TextHorizontalAdjust;
using ::com::sun::star::drawing::TextVerticalAdjust;

using ::rtl::OString;
using ::rtl::OUString;

// ============================================================================

namespace {

typedef ::cppu::WeakImplHelper2< XPropertySet, XPropertySetInfo > GenericPropertySetBase;

/** This class implements a generic XPropertySet.

    Properties of all names and types can be set and later retrieved.
    TODO: move this to comphelper or better find an existing implementation
 */
class GenericPropertySet : public GenericPropertySetBase, private ::osl::Mutex
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
    const PropertyNameVector& rPropNames = StaticPropertyNameVector::get();
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

PropertyMap::PropertyMap() :
    mpPropNames( &StaticPropertyNameVector::get() ) // pointer instead reference to get compiler generated copy c'tor and operator=
{
}

PropertyMap::~PropertyMap()
{
}

/*static*/ const OUString& PropertyMap::getPropertyName( sal_Int32 nPropId )
{
    OSL_ENSURE( (0 <= nPropId) && (nPropId < PROP_COUNT), "PropertyMap::getPropertyName - invalid property identifier" );
    return StaticPropertyNameVector::get()[ nPropId ];
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
        PropertyValue* pValues = aSeq.getArray();
        for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt, ++pValues )
        {
            OSL_ENSURE( (0 <= aIt->first) && (aIt->first < PROP_COUNT), "PropertyMap::makePropertyValueSequence - invalid property identifier" );
            pValues->Name = (*mpPropNames)[ aIt->first ];
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
        OUString* pNames = rNames.getArray();
        Any* pValues = rValues.getArray();
        for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt, ++pNames, ++pValues )
        {
            OSL_ENSURE( (0 <= aIt->first) && (aIt->first < PROP_COUNT), "PropertyMap::fillSequences - invalid property identifier" );
            *pNames = (*mpPropNames)[ aIt->first ];
            *pValues = aIt->second;
        }
    }
}

Reference< XPropertySet > PropertyMap::makePropertySet() const
{
    return new GenericPropertySet( *this );
}

#if OSL_DEBUG_LEVEL > 0
static void lclDumpAnyValue( Any value)
{
        OUString strValue;
        sal_Int32 intValue = 0;
        sal_uInt32 uintValue = 0;
        sal_Int16 int16Value = 0;
        sal_uInt16 uint16Value = 0;
        float floatValue = 0;
        bool boolValue = false;
    LineSpacing spacing;
//         RectanglePoint pointValue;
    WritingMode aWritingMode;
    TextVerticalAdjust aTextVertAdj;
    TextHorizontalAdjust aTextHorizAdj;
    Reference< XIndexReplace > xNumRule;

        if( value >>= strValue )
            fprintf (stderr,"\"%s\"\n", USS( strValue ) );
        else if( value >>= intValue )
            fprintf (stderr,"%"SAL_PRIdINT32"            (hex: %"SAL_PRIxUINT32")\n", intValue, intValue);
        else if( value >>= uintValue )
            fprintf (stderr,"%"SAL_PRIdINT32"            (hex: %"SAL_PRIxUINT32")\n", uintValue, uintValue);
        else if( value >>= int16Value )
            fprintf (stderr,"%d            (hex: %x)\n", int16Value, int16Value);
        else if( value >>= uint16Value )
            fprintf (stderr,"%d            (hex: %x)\n", uint16Value, uint16Value);
        else if( value >>= floatValue )
            fprintf (stderr,"%f\n", floatValue);
        else if( value >>= boolValue )
            fprintf (stderr,"%d            (bool)\n", boolValue);
        else if( value >>= xNumRule ) {
            fprintf (stderr, "XIndexReplace\n");
            for (int k=0; k<xNumRule->getCount(); k++) {
                Sequence< PropertyValue > aBulletPropSeq;
                fprintf (stderr, "level %d\n", k);
                if (xNumRule->getByIndex (k) >>= aBulletPropSeq) {
                    for (int j=0; j<aBulletPropSeq.getLength(); j++) {
                        fprintf(stderr, "%46s = ", USS (aBulletPropSeq[j].Name));
                        lclDumpAnyValue (aBulletPropSeq[j].Value);
                    }
                }
            }
        } else if( value >>= aWritingMode )
            fprintf (stderr, "%d writing mode\n", aWritingMode);
        else if( value >>= aTextVertAdj ) {
            const char* s = "uknown";
            switch( aTextVertAdj ) {
            case TextVerticalAdjust_TOP:
                s = "top";
                break;
            case TextVerticalAdjust_CENTER:
                s = "center";
                break;
            case TextVerticalAdjust_BOTTOM:
                s = "bottom";
                break;
            case TextVerticalAdjust_BLOCK:
                s = "block";
                break;
            case TextVerticalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
        }
        fprintf (stderr, "%s\n", s);
    } else if( value >>= aTextHorizAdj ) {
        const char* s = "uknown";
        switch( aTextHorizAdj ) {
            case TextHorizontalAdjust_LEFT:
                s = "left";
                break;
            case TextHorizontalAdjust_CENTER:
                s = "center";
                break;
            case TextHorizontalAdjust_RIGHT:
                s = "right";
                break;
            case TextHorizontalAdjust_BLOCK:
                s = "block";
                break;
            case TextHorizontalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
        }
        fprintf (stderr, "%s\n", s);
    } else if( value >>= spacing ) {
        fprintf (stderr, "mode: %d value: %d\n", spacing.Mode, spacing.Height);
    } else if( value.isExtractableTo(::getCppuType((const sal_Int32*)0))) {
        fprintf (stderr,"is extractable to int32\n");
    }
//         else if( value >>= pointValue )
//             fprintf (stderr,"%d            (RectanglePoint)\n", pointValue);
        else
      fprintf (stderr,"???           <unhandled type %s>\n", USS(value.getValueTypeName()));
}

void PropertyMap::dump( Reference< XPropertySet > rXPropSet )
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo ();
    Sequence< beans::Property > props = info->getProperties ();

    OSL_TRACE("dump props, len: %d", props.getLength ());

    for (int i=0; i < props.getLength (); i++) {
        OString name = OUStringToOString( props [i].Name, RTL_TEXTENCODING_UTF8);
        fprintf (stderr,"%30s = ", name.getStr() );

        try {
            lclDumpAnyValue (rXPropSet->getPropertyValue( props [i].Name ));
        } catch(Exception e) {
            fprintf (stderr,"unable to get '%s' value\n", USS(props [i].Name));
        }
    }
}

void PropertyMap::dump()
{
    dump( Reference< XPropertySet >( makePropertySet(), UNO_QUERY ) );
}
#endif

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
