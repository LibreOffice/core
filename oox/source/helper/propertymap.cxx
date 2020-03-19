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

#include <oox/helper/propertymap.hxx>

#if OSL_DEBUG_LEVEL > 0
# include <cstdio>
# include <com/sun/star/style/LineSpacing.hpp>
# include <com/sun/star/text/WritingMode.hpp>
using ::com::sun::star::style::LineSpacing;
using ::com::sun::star::text::WritingMode;
#include <comphelper/anytostring.hxx>
#include <iostream>
#endif

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <rtl/strbuf.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/propertynames.hxx>
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::beans::XVetoableChangeListener;
using ::com::sun::star::container::XIndexReplace;

#if OSL_DEBUG_LEVEL > 0
#define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using ::com::sun::star::style::LineSpacing;
using ::com::sun::star::text::WritingMode;
using ::com::sun::star::drawing::TextHorizontalAdjust;
using ::com::sun::star::drawing::TextVerticalAdjust;
#endif

namespace oox {
using ::com::sun::star::container::XIndexReplace;

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using ::com::sun::star::drawing::TextHorizontalAdjust;
using ::com::sun::star::drawing::TextVerticalAdjust;

namespace {

/** This class implements a generic XPropertySet.

    Properties of all names and types can be set and later retrieved.
    TODO: move this to comphelper or better find an existing implementation
 */
class GenericPropertySet : public ::cppu::WeakImplHelper< XPropertySet, XPropertySetInfo >
{
public:
    explicit            GenericPropertySet( const PropertyMap& rPropMap );

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) override;
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) override;

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

private:
    osl::Mutex mMutex;
    PropertyNameMap     maPropMap;
};

GenericPropertySet::GenericPropertySet( const PropertyMap& rPropMap )
{
    rPropMap.fillPropertyNameMap(maPropMap);
}

Reference< XPropertySetInfo > SAL_CALL GenericPropertySet::getPropertySetInfo()
{
    return this;
}

void SAL_CALL GenericPropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    ::osl::MutexGuard aGuard( mMutex );
    maPropMap[ rPropertyName ] = rValue;
}

Any SAL_CALL GenericPropertySet::getPropertyValue( const OUString& rPropertyName )
{
    PropertyNameMap::iterator aIt = maPropMap.find( rPropertyName );
    if( aIt == maPropMap.end() )
        throw UnknownPropertyException(rPropertyName);
    return aIt->second;
}

// listeners are not supported by this implementation
void SAL_CALL GenericPropertySet::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >& ) {}
void SAL_CALL GenericPropertySet::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) {}
void SAL_CALL GenericPropertySet::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) {}
void SAL_CALL GenericPropertySet::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) {}

// XPropertySetInfo
Sequence< Property > SAL_CALL GenericPropertySet::getProperties()
{
    Sequence< Property > aSeq( static_cast< sal_Int32 >( maPropMap.size() ) );
    Property* pProperty = aSeq.getArray();
    for (auto const& prop : maPropMap)
    {
        pProperty->Name = prop.first;
        pProperty->Handle = 0;
        pProperty->Type = prop.second.getValueType();
        pProperty->Attributes = 0;
        ++pProperty;
    }
    return aSeq;
}

Property SAL_CALL GenericPropertySet::getPropertyByName( const OUString& rPropertyName )
{
    PropertyNameMap::iterator aIt = maPropMap.find( rPropertyName );
    if( aIt == maPropMap.end() )
        throw UnknownPropertyException(rPropertyName);
    Property aProperty;
    aProperty.Name = aIt->first;
    aProperty.Handle = 0;
    aProperty.Type = aIt->second.getValueType();
    aProperty.Attributes = 0;
    return aProperty;
}

sal_Bool SAL_CALL GenericPropertySet::hasPropertyByName( const OUString& rPropertyName )
{
    return maPropMap.find( rPropertyName ) != maPropMap.end();
}

} // namespace

PropertyMap::PropertyMap() :
    mpPropNames( &StaticPropertyNameVector::get() ) // pointer instead reference to get compiler generated copy c'tor and operator=
{
}

bool PropertyMap::hasProperty( sal_Int32 nPropId ) const
{
    return maProperties.find( nPropId ) != maProperties.end();
}

bool PropertyMap::setAnyProperty( sal_Int32 nPropId, const Any& rValue )
{
    if( nPropId < 0 )
        return false;

    maProperties[ nPropId ] = rValue;
    return true;
}

Any PropertyMap::getProperty( sal_Int32 nPropId )
{
    return maProperties[ nPropId ];
}

void PropertyMap::erase( sal_Int32 nPropId )
{
    maProperties.erase(nPropId);
}

bool PropertyMap::empty() const
{
    return maProperties.empty();
}

void PropertyMap::assignUsed( const PropertyMap& rPropMap )
{
    maProperties.insert(rPropMap.maProperties.begin(), rPropMap.maProperties.end());
}

const OUString& PropertyMap::getPropertyName( sal_Int32 nPropId )
{
    OSL_ENSURE( (0 <= nPropId) && (nPropId < PROP_COUNT), "PropertyMap::getPropertyName - invalid property identifier" );
    return StaticPropertyNameVector::get()[ nPropId ];
}

void PropertyMap::assignAll( const PropertyMap& rPropMap )
{
    for (auto const& prop : rPropMap.maProperties)
        maProperties[prop.first] = prop.second;
}

Sequence< PropertyValue > PropertyMap::makePropertyValueSequence() const
{
    Sequence< PropertyValue > aSeq( static_cast< sal_Int32 >( maProperties.size() ) );
    PropertyValue* pValues = aSeq.getArray();
    for (auto const& prop : maProperties)
    {
        OSL_ENSURE( (0 <= prop.first) && (prop.first < PROP_COUNT), "PropertyMap::makePropertyValueSequence - invalid property identifier" );
        pValues->Name = (*mpPropNames)[ prop.first ];
        pValues->Value = prop.second;
        pValues->State = PropertyState_DIRECT_VALUE;
        ++pValues;
    }
    return aSeq;
}

void PropertyMap::fillSequences( Sequence< OUString >& rNames, Sequence< Any >& rValues ) const
{
    rNames.realloc( static_cast< sal_Int32 >( maProperties.size() ) );
    rValues.realloc( static_cast< sal_Int32 >( maProperties.size() ) );
    if( maProperties.empty() )
        return;

    OUString* pNames = rNames.getArray();
    Any* pValues = rValues.getArray();
    for (auto const& prop : maProperties)
    {
        OSL_ENSURE( (0 <= prop.first) && (prop.first < PROP_COUNT), "PropertyMap::fillSequences - invalid property identifier" );
        *pNames = (*mpPropNames)[ prop.first ];
        *pValues = prop.second;
        ++pNames;
        ++pValues;
    }
}

void PropertyMap::fillPropertyNameMap(PropertyNameMap& rMap) const
{
    for (auto const& prop : maProperties)
    {
        rMap.insert(std::pair<OUString, Any>((*mpPropNames)[prop.first], prop.second));
    }
}

Reference< XPropertySet > PropertyMap::makePropertySet() const
{
    return new GenericPropertySet( *this );
}

#if OSL_DEBUG_LEVEL > 0
static void lclDumpAnyValue(const Any& value, std::string prefix)
{
    OUString strValue;
    Sequence< OUString > strArray;
    Sequence< Any > anyArray;
    Sequence< PropertyValue > propArray;
    Sequence< Sequence< PropertyValue > > propArrayArray;
    Sequence< EnhancedCustomShapeAdjustmentValue > adjArray;
    Sequence< EnhancedCustomShapeSegment > segArray;
    Sequence< EnhancedCustomShapeParameterPair > ppArray;
    EnhancedCustomShapeSegment segment;
    EnhancedCustomShapeParameterPair pp;
    EnhancedCustomShapeParameter par;
    HomogenMatrix3 aMatrix;
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
            SAL_INFO("oox", prefix
                    << "\"" << USS(strValue) << "\"");
    else if( value >>= strArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<strArray.getLength(); i++ )
                    SAL_INFO("oox", "[" << std::setw(3) << i << "] \""
                            << USS(strArray[i]) << "\"");
    } else if( value >>= propArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<propArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] "
                        << USS(propArray[i].Name) << " ("
                        << USS(propArray[i].Value.getValueTypeName())
                        << ")";
                    lclDumpAnyValue(propArray[i].Value, oss.str());
            }
    } else if( value >>= propArrayArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<propArrayArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] ";
                    lclDumpAnyValue(makeAny(propArrayArray[i]), oss.str());
            }
    } else if( value >>= anyArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<anyArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] ("
                        << USS(value.getValueTypeName())
                        << ")";
                    lclDumpAnyValue(anyArray[i], oss.str());
            }
    } else if( value >>= adjArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<adjArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] ("
                        << USS(adjArray[i].Value.getValueTypeName())
                        << ")";
                    lclDumpAnyValue(adjArray[i].Value, oss.str());
            }
    } else if( value >>= segArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<segArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] ";
                    lclDumpAnyValue(makeAny(segArray[i]), oss.str());
            }
    } else if( value >>= ppArray ) {
            SAL_INFO("oox", prefix
                    << USS(value.getValueTypeName()));
            for( int i=0; i<ppArray.getLength(); i++ ) {
                    std::ostringstream oss;
                    oss << "[" << std::setw(3) << i << "] ";
                    lclDumpAnyValue(makeAny(ppArray[i]), oss.str());
            }
    } else if( value >>= segment ) {
            SAL_INFO("oox", prefix
                    << "Command: " << segment.Command
                    << " Count: " << segment.Count);
    } else if( value >>= pp ) {
            { std::ostringstream oss;
              oss << prefix << "First: ";
              lclDumpAnyValue(makeAny(pp.First), oss.str()); }
            { std::ostringstream oss;
              oss << prefix << "Second: ";
              lclDumpAnyValue(makeAny(pp.Second), oss.str()); }
    } else if( value >>= par ) {
            std::ostringstream oss;
            oss << prefix
                << "Parameter (" << USS(par.Value.getValueTypeName()) << "): ";
            lclDumpAnyValue(par.Value, oss.str());
    } else if( value >>= aMatrix ) {
            SAL_INFO("oox", prefix << "Matrix");
            SAL_INFO("oox", aMatrix.Line1.Column1 << " "
                    << aMatrix.Line1.Column2 << " "
                    << aMatrix.Line1.Column3);
            SAL_INFO("oox", aMatrix.Line2.Column1 << " "
                    << aMatrix.Line2.Column2 << " "
                    << aMatrix.Line2.Column3);
            SAL_INFO("oox", aMatrix.Line3.Column1 << " "
                    << aMatrix.Line3.Column2 << " "
                    << aMatrix.Line3.Column3);
    } else if( value >>= intValue )
            SAL_INFO("oox", prefix
                    << std::setw(10) << std::left << intValue
                    << "  (hex: " << std::hex << intValue << ").");
    else if( value >>= uintValue )
            SAL_INFO("oox", prefix
                    << std::setw(10) << std::left << uintValue
                    << "  (hex: " << std::hex << uintValue << ").");
    else if( value >>= int16Value )
            SAL_INFO("oox", prefix
                    << std::setw(10) << std::left << int16Value
                    << "  (hex: " << std::hex << int16Value << ").");
    else if( value >>= uint16Value )
            SAL_INFO("oox", prefix
                    << std::setw(10) << std::left << uint16Value
                    << "  (hex: " << std::hex << uint16Value << ").");
    else if( value >>= floatValue )
            SAL_INFO("oox", prefix << floatValue);
    else if( value >>= boolValue )
            SAL_INFO("oox", prefix
                    << std::setw(10) << std::left << boolValue);
    else if( value >>= xNumRule ) {
            SAL_INFO("oox", prefix << "XIndexReplace.");
            if (xNumRule.is()) {
                for (int k=0; k<xNumRule->getCount(); k++) {
                    Sequence< PropertyValue > aBulletPropSeq;
                    SAL_INFO("oox", "level " << k);
                    if (xNumRule->getByIndex (k) >>= aBulletPropSeq) {
                        for (int j=0; j<aBulletPropSeq.getLength(); j++) {
                            std::ostringstream oss;
                            oss << std::setw(46) << " = " << USS(aBulletPropSeq[j].Name);
                            lclDumpAnyValue(aBulletPropSeq[j].Value, oss.str());
                        }
                    }
                }
            } else {
                SAL_INFO("oox", "empty reference.");
            }
    } else if( value >>= aWritingMode )
            SAL_INFO("oox", prefix
                    << static_cast<int>(aWritingMode) << " writing mode.");
    else if( value >>= aTextVertAdj ) {
            const char* s = "unknown";
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
            case TextVerticalAdjust::TextVerticalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
            }
            SAL_INFO("oox", prefix << s);
    } else if( value >>= aTextHorizAdj ) {
        const char* s = "unknown";
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
            case TextHorizontalAdjust::TextHorizontalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
        }
        SAL_INFO("oox", prefix << s);
    } else if( value >>= spacing ) {
        SAL_INFO("oox", prefix
                << "mode: " << spacing.Mode
                << " value: " << spacing.Height);
    } else if( value.isExtractableTo(::cppu::UnoType<sal_Int32>::get())) {
        SAL_INFO("oox", prefix << "is extractable to int32.");
    }
//         else if( value >>= pointValue )
//             fprintf (stderr,"%d            (RectanglePoint)\n", pointValue);
    else
        SAL_INFO("oox", prefix
                << "???           <unhandled type "
                << USS(value.getValueTypeName())
                << ">.");
}

#ifdef DBG_UTIL
void PropertyMap::dump( const Reference< XPropertySet >& rXPropSet )
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo ();
    Sequence< Property > props = info->getProperties ();

    SAL_INFO("oox", "dump props, len: " << props.getLength ());

    for (int i=0; i < props.getLength (); i++) {
        std::ostringstream oss;
        oss << std::setw(30)
            << OUStringToOString(
                    props[i].Name, RTL_TEXTENCODING_UTF8).getStr()
            << " = ";

        try {
            lclDumpAnyValue(rXPropSet->getPropertyValue(props[i].Name), oss.str());
        } catch (const Exception&) {
            SAL_WARN("oox", "unable to get '" << USS(props[i].Name) << "' value.");
        }
    }
}
#endif

static OString getLevelOString(int level)
{
    OStringBuffer buffer;
    for (int i=0; i<level; i++)
        buffer.append("    ");
    return buffer.toString();
}

static const char *lclGetEnhancedParameterType( sal_uInt16 nType )
{
    const char* type;
    switch (nType) {
    case EnhancedCustomShapeParameterType::NORMAL:
        type = "EnhancedCustomShapeParameterType::NORMAL";
        break;
    case EnhancedCustomShapeParameterType::EQUATION:
        type = "EnhancedCustomShapeParameterType::EQUATION";
        break;
    case EnhancedCustomShapeParameterType::ADJUSTMENT:
        type = "EnhancedCustomShapeParameterType::ADJUSTMENT";
        break;
    case EnhancedCustomShapeParameterType::LEFT:
        type = "EnhancedCustomShapeParameterType::LEFT";
        break;
    case EnhancedCustomShapeParameterType::TOP:
        type = "EnhancedCustomShapeParameterType::TOP";
        break;
    case EnhancedCustomShapeParameterType::RIGHT:
        type = "EnhancedCustomShapeParameterType::RIGHT";
        break;
    case EnhancedCustomShapeParameterType::BOTTOM:
        type = "EnhancedCustomShapeParameterType::BOTTOM";
        break;
    case EnhancedCustomShapeParameterType::XSTRETCH:
        type = "EnhancedCustomShapeParameterType::XSTRETCH";
        break;
    case EnhancedCustomShapeParameterType::YSTRETCH:
        type = "EnhancedCustomShapeParameterType::YSTRETCH";
        break;
    case EnhancedCustomShapeParameterType::HASSTROKE:
        type = "EnhancedCustomShapeParameterType::HASSTROKE";
        break;
    case EnhancedCustomShapeParameterType::HASFILL:
        type = "EnhancedCustomShapeParameterType::HASFILL";
        break;
    case EnhancedCustomShapeParameterType::WIDTH:
        type = "EnhancedCustomShapeParameterType::WIDTH";
        break;
    case EnhancedCustomShapeParameterType::HEIGHT:
        type = "EnhancedCustomShapeParameterType::HEIGHT";
        break;
    case EnhancedCustomShapeParameterType::LOGWIDTH:
        type = "EnhancedCustomShapeParameterType::LOGWIDTH";
        break;
    case EnhancedCustomShapeParameterType::LOGHEIGHT:
        type = "EnhancedCustomShapeParameterType::LOGHEIGHT";
        break;
    default:
        type = "unknown";
        break;
    }
    return type;
}

static void printParameterPairData(int level, EnhancedCustomShapeParameterPair const &pp)
{
    // These are always sal_Int32s so lets depend on that for our packing ...
    sal_Int32 nFirstValue = {};
    sal_Int32 nSecondValue = {}; // spurious -Werror=maybe-uninitialized
    if (!(pp.First.Value >>= nFirstValue))
        assert (false);
    if (!(pp.Second.Value >>= nSecondValue))
        assert (false);

    OString levelString = getLevelOString(level);
    OString nextLevelString = getLevelOString(level + 1);

    SAL_INFO("oox", levelString.getStr()
            << "{");
    SAL_INFO("oox", nextLevelString.getStr()
            << lclGetEnhancedParameterType(pp.First.Type) << ",");
    SAL_INFO("oox", nextLevelString.getStr()
            << lclGetEnhancedParameterType(pp.Second.Type) << ",");
    SAL_INFO("oox", nextLevelString.getStr()
            << static_cast<int>(nFirstValue) << ", "
            << static_cast<int>(nSecondValue));
    SAL_INFO("oox", levelString.getStr()
            << "}");
}

static const char* lclDumpAnyValueCode( const Any& value, int level)
{
    OUString strValue;
    Sequence< OUString > strArray;
    Sequence< Any > anyArray;
    Sequence< awt::Size > sizeArray;
    Sequence< PropertyValue > propArray;
    Sequence< Sequence< PropertyValue > > propArrayArray;
    Sequence< EnhancedCustomShapeAdjustmentValue > adjArray;
    Sequence< EnhancedCustomShapeTextFrame > segTextFrame;
    Sequence< EnhancedCustomShapeSegment > segArray;
    Sequence< EnhancedCustomShapeParameterPair > ppArray;
    EnhancedCustomShapeSegment segment;
    EnhancedCustomShapeTextFrame textFrame;
    EnhancedCustomShapeParameterPair pp;
    EnhancedCustomShapeParameter par;
    awt::Rectangle rect;
    awt::Size size;
    sal_Int32 intValue;
    sal_uInt32 uintValue;
    sal_Int16 int16Value;
    sal_uInt16 uint16Value;
    long longValue;
    float floatValue = 0;
    bool boolValue;
    LineSpacing spacing;
//         RectanglePoint pointValue;
    WritingMode aWritingMode;
    TextVerticalAdjust aTextVertAdj;
    TextHorizontalAdjust aTextHorizAdj;
    Reference< XIndexReplace > xNumRule;

    OString levelString = getLevelOString(level);

    if( value >>= strValue )
    {
        SAL_INFO("oox", levelString.getStr()
                << "OUString str = \"" << USS(strValue) << "\";");
        return "Any (str)";
    }
    else if( value >>= strArray )
    {
        if (strArray.getLength() == 0)
            return "Sequence< OUString >(0)";

        SAL_INFO("oox", levelString.getStr() << "static const char *aStrings[] = {");
        for( int i=0; i<strArray.getLength(); i++ ) {
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << "\"" << USS(strArray[i]) << "\""
                    << (i < strArray.getLength() - 1 ? "," : "" ));
        }
        SAL_INFO("oox", levelString.getStr() << "};");
        return "createStringSequence( SAL_N_ELEMENTS( aStrings ), aStrings )";
    }
    else if( value >>= propArray )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Sequence< PropertyValue > aPropSequence ("
                << propArray.getLength() << ");");
        OString nextLevelString = getLevelOString(level + 1);
        for( int i=0; i<propArray.getLength(); i++ ) {
            SAL_INFO("oox", levelString.getStr()
                    << "{");
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aPropSequence [" << i << "].Name = \"" << USS(propArray[i].Name) << "\";");
            const char *var = lclDumpAnyValueCode(propArray[i].Value, level + 1);
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aPropSequence [" << i << "].Value = makeAny (" << var << ");");
            SAL_INFO("oox", levelString.getStr()
                    << "}");
        }
        return "aPropSequence";
    }
    else if( value >>= sizeArray )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Sequence< awt::Size > aSizeSequence (" << sizeArray.getLength() << ");");
        for( int i=0; i<sizeArray.getLength(); i++ ) {
            SAL_INFO("oox", levelString.getStr() << "{");
            const char *var = lclDumpAnyValueCode (makeAny (sizeArray[i]), level + 1);
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << "aSizeSequence [" << i << "] = " << var << ";");
            SAL_INFO("oox", levelString.getStr()
                    << "}");
        }
        return "aSizeSequence";
    }
    else if( value >>= propArrayArray )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Sequence< Sequence < PropertyValue > > aPropSequenceSequence ("
                << propArrayArray.getLength() << ");");
        for( int i=0; i<propArrayArray.getLength(); i++ ) {
            SAL_INFO("oox", levelString.getStr() << "{");
            const char *var = lclDumpAnyValueCode( makeAny (propArrayArray[i]), level + 1 );
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << "aPropSequenceSequence [" << i << "] = " << var << ";");
            SAL_INFO("oox", levelString.getStr()
                    << "}");
        }
        return "aPropSequenceSequence";
    }
    else if( value >>= anyArray )
    {
        SAL_INFO("oox", USS(value.getValueTypeName()));
        for( int i=0; i<anyArray.getLength(); i++ ) {
            std::ostringstream oss;
            oss << "[" << std::setw(3) << i << "] ("
                << USS(value.getValueTypeName()) << ") ";
            lclDumpAnyValue(anyArray[i], oss.str());
        }
    }
    else if( value >>= adjArray )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Sequence< EnhancedCustomShapeAdjustmentValue > aAdjSequence ("
                << adjArray.getLength() << ");");
        OString nextLevelString = getLevelOString(level + 1);
        for( int i=0; i<adjArray.getLength(); i++ ) {
            SAL_INFO("oox", levelString.getStr()
                    << "{");
            const char *var = lclDumpAnyValueCode( adjArray[i].Value, level + 1 );
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aAdjSequence [" << i << "].Value = " << var << ";");
            if (adjArray[i].Name.getLength() > 0) {
                SAL_INFO("oox", nextLevelString.getStr()
                        << "aAdjSequence [" << i << "].Name = \""
                        << USS(adjArray[i].Name) << "\";");
            }
            SAL_INFO("oox", levelString.getStr()
                    << "}");
        }
        return "aAdjSequence";
    }
    else if( value >>= segArray )
    {
        if (segArray.getLength() == 0)
            return "Sequence< EnhancedCustomShapeSegment >(0)";

        SAL_INFO("oox", levelString.getStr()
                << "static const sal_uInt16 nValues[] = {");
        SAL_INFO("oox", levelString.getStr()
                << "// Command, Count");
        for( int i = 0; i < segArray.getLength(); i++ ) {
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << segArray[i].Command << ","
                    << segArray[i].Count
                    << (i < segArray.getLength() - 1 ? "," : ""));
        }
        SAL_INFO("oox", levelString.getStr()
                << "};");
        return "createSegmentSequence( SAL_N_ELEMENTS( nValues ), nValues )";
    }
    else if( value >>= segTextFrame )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Sequence< EnhancedCustomShapeTextFrame > aTextFrameSeq ("
                << segTextFrame.getLength() << ");");
        for( int i=0; i<segTextFrame.getLength(); i++ ) {
            SAL_INFO("oox", levelString.getStr()
                    << "{");
            const char *var = lclDumpAnyValueCode (makeAny (segTextFrame[i]), level + 1);
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << "aTextFrameSeq [" << i << "] = " << var << ";");
            SAL_INFO("oox", levelString.getStr() << "}");
        }
        return "aTextFrameSeq";
    }
    else if( value >>= ppArray )
    {
        if (ppArray.getLength() == 0)
            return "Sequence< EnhancedCustomShapeParameterPair >(0)";

        SAL_INFO("oox", levelString.getStr()
                << "static const CustomShapeProvider::ParameterPairData aData[] = {");
        for( int i = 0; i < ppArray.getLength(); i++ ) {
            printParameterPairData(level + 1, ppArray[i]);
            SAL_INFO("oox", getLevelOString(level + 1).getStr()
                    << (i < ppArray.getLength() - 1 ? "," : ""));
        }
        SAL_INFO("oox", levelString.getStr()
                << "};");

        return "createParameterPairSequence(SAL_N_ELEMENTS(aData), aData)";
    }
    else if( value >>= segment )
    {
        SAL_INFO("oox", levelString.getStr()
                << "EnhancedCustomShapeSegment aSegment;");
        // TODO: use EnhancedCustomShapeSegmentCommand constants
        SAL_INFO("oox", levelString.getStr()
                << "aSegment.Command = " << segment.Command << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aSegment.Count = " << segment.Count << ";");
        return "aSegment";
    }
    else if( value >>= textFrame )
    {
        OString nextLevelString = getLevelOString(level + 1);

        SAL_INFO("oox", levelString.getStr()
                << "EnhancedCustomShapeTextFrame aTextFrame;");
        SAL_INFO("oox", levelString.getStr()
                << "{");
        {
            const char* var = lclDumpAnyValueCode( makeAny (textFrame.TopLeft), level + 1 );
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aTextFrame.TopLeft = " << var << ";");
        }
        SAL_INFO("oox", levelString.getStr()
                << "}");

        SAL_INFO("oox", levelString.getStr()
                << "{");
        {
            const char* var = lclDumpAnyValueCode( makeAny (textFrame.BottomRight), level + 1 );
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aTextFrame.BottomRight = " << var << ";");
        }
        SAL_INFO("oox", levelString.getStr()
                << "}");

        return "aTextFrame";
    }
    else if( value >>= pp )
    {
        SAL_INFO("oox", levelString.getStr()
                << "static const CustomShapeProvider::ParameterPairData aData =");
        printParameterPairData(level, pp);
        SAL_INFO("oox", levelString.getStr()
                << ";");

        return "createParameterPair(&aData)";
    }
    else if( value >>= par )
    {
        SAL_INFO("oox", levelString.getStr()
                << "EnhancedCustomShapeParameter aParameter;");
        const char* var = lclDumpAnyValueCode( par.Value, level );
        SAL_INFO("oox", levelString.getStr()
                << "aParameter.Value = " << var << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aParameter.Type = " << lclGetEnhancedParameterType(par.Type) << ";");
        return "aParameter";
    }
    else if( value >>= longValue )
    {
        SAL_INFO("oox", levelString.getStr()
                << "Any aAny ((sal_Int32) " << longValue << ");");
        return "aAny";
    }
    else if( value >>= intValue )
        SAL_INFO("oox", intValue << "            (hex: " << std::hex << intValue << ")");
    else if( value >>= uintValue )
        SAL_INFO("oox", uintValue << "            (hex: " << std::hex << uintValue << ")");
    else if( value >>= int16Value )
        SAL_INFO("oox", int16Value << "            (hex: " << std::hex << int16Value << ")");
    else if( value >>= uint16Value )
        SAL_INFO("oox", uint16Value << "            (hex: " << std::hex << uint16Value << ")");
    else if( value >>= floatValue )
        SAL_INFO("oox", floatValue);
    else if( value >>= boolValue ) {
        if (boolValue)
            return "(sal_Bool) sal_True";
        else
            return "(sal_Bool) sal_False";
    }
    else if( value >>= xNumRule ) {
        SAL_INFO("oox", "XIndexReplace.");
        for (int k=0; k<xNumRule->getCount(); k++) {
            Sequence< PropertyValue > aBulletPropSeq;
            SAL_INFO("oox", "level " << k);
            if (xNumRule->getByIndex (k) >>= aBulletPropSeq) {
                for (int j=0; j<aBulletPropSeq.getLength(); j++) {
                    std::ostringstream oss;
                    oss << std::setw(46) << USS(aBulletPropSeq[j].Name) << " = ";
                    lclDumpAnyValue(aBulletPropSeq[j].Value, oss.str());
                }
            }
        }
    }
    else if( value >>= aWritingMode )
        SAL_INFO("oox", static_cast<int>(aWritingMode) << " writing mode.");
    else if( value >>= aTextVertAdj ) {
        const char* s = "unknown";
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
            case TextVerticalAdjust::TextVerticalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
        }
        SAL_INFO("oox", s);
    }
    else if( value >>= aTextHorizAdj ) {
        const char* s = "unknown";
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
            case TextHorizontalAdjust::TextHorizontalAdjust_MAKE_FIXED_SIZE:
                s = "make_fixed_size";
                break;
        }
        SAL_INFO("oox", s);
    }
    else if( value >>= spacing ) {
        SAL_INFO("oox", "mode: " << spacing.Mode << " value: " << spacing.Height);
    }
    else if( value >>= rect ) {
        SAL_INFO("oox", levelString.getStr()
                << "awt::Rectangle aRectangle;");
        SAL_INFO("oox", levelString.getStr()
                << "aRectangle.X = " << rect.X << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aRectangle.Y = " << rect.Y << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aRectangle.Width = " << rect.Width << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aRectangle.Height = " << rect.Height << ";");
        return "aRectangle";
    }
    else if( value >>= size ) {
        SAL_INFO("oox", levelString.getStr()
                << "awt::Size aSize;");
        SAL_INFO("oox", levelString.getStr()
                << "aSize.Width = " << size.Width << ";");
        SAL_INFO("oox", levelString.getStr()
                << "aSize.Height = " << size.Height << ";");
        return "aSize";
    }
    else if( value.isExtractableTo(::cppu::UnoType<sal_Int32>::get())) {
        SAL_INFO("oox", "is extractable to int32.");
    }
    else
        SAL_INFO("oox", "???           <unhandled type "
                << USS(value.getValueTypeName()) << ">");

    return "";
}

void PropertyMap::dumpCode( const Reference< XPropertySet >& rXPropSet )
{
    Reference< XPropertySetInfo > info = rXPropSet->getPropertySetInfo ();
    Sequence< Property > props = info->getProperties ();
    const OUString sType = "Type";

    for (int i=0; i < props.getLength (); i++) {

        // ignore Type, it is set elsewhere
        if (props[i].Name == sType)
            continue;

        OString name = OUStringToOString( props [i].Name, RTL_TEXTENCODING_UTF8);

        int level = 1;
        OString levelString = getLevelOString(level);
        OString nextLevelString = getLevelOString(level + 1);

        try {
            SAL_INFO("oox", levelString.getStr()
                    << "{");
            const char* var = lclDumpAnyValueCode (rXPropSet->getPropertyValue (props [i].Name), level + 1);
            SAL_INFO("oox", nextLevelString.getStr()
                    << "aPropertyMap.setProperty(PROP_"
                    << name.getStr() << ", " << var << ");");
            SAL_INFO("oox", levelString.getStr()
                    << "}");
        } catch (const Exception&) {
            SAL_INFO("oox", "unable to get '" << USS(props[i].Name) << "' value.");
        }
    }
}

void PropertyMap::dumpData(const Reference<XPropertySet>& xPropertySet)
{
    Reference<XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
    Sequence<Property> aProperties = xPropertySetInfo->getProperties();

    for (int i = 0; i < aProperties.getLength(); ++i)
    {
        std::cerr << aProperties[i].Name << std::endl;
        std::cerr << comphelper::anyToString(xPropertySet->getPropertyValue(aProperties[i].Name)) << std::endl;
    }
}

#endif

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
