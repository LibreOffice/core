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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <vector>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/macros.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

enum class PropsSet {
    NONE             = 0x00000000,
    String           = 0x00000001,
    Boolean          = 0x00000002,
    Byte             = 0x00000004,
    Short            = 0x00000008,
    Int              = 0x00000010,
    Long             = 0x00000020,
    Float            = 0x00000040,
    Double           = 0x00000080,
    Bytes            = 0x00000100,
    Date             = 0x00000200,
    Time             = 0x00000400,
    Timestamp        = 0x00000800,
    BinaryStream     = 0x00001000,
    CharacterStream  = 0x00002000,
    Ref              = 0x00004000,
    Blob             = 0x00008000,
    Clob             = 0x00010000,
    Array            = 0x00020000,
    Object           = 0x00040000
};
namespace o3tl {
    template<> struct typed_flags<PropsSet> : is_typed_flags<PropsSet, 0x0007ffff> {};
}

namespace ucbhelper_impl
{


struct PropertyValue
{
    OUString    sPropertyName;

    PropsSet    nPropsSet;
    PropsSet    nOrigValue;

    OUString    aString;    // getString
    bool        bBoolean;   // getBoolean
    sal_Int8    nByte;      // getByte
    sal_Int16   nShort;     // getShort
    sal_Int32   nInt;       // getInt
    sal_Int64   nLong;      // getLong
    float       nFloat;     // getFloat
    double      nDouble;    // getDouble

    Sequence< sal_Int8 >        aBytes;             // getBytes
    Date                        aDate;              // getDate
    Time                        aTime;              // getTime
    DateTime                    aTimestamp;         // getTimestamp
    Reference< XInputStream >   xBinaryStream;      // getBinaryStream
    Reference< XInputStream >   xCharacterStream;   // getCharacterStream
    Reference< XRef >           xRef;               // getRef
    Reference< XBlob >          xBlob;              // getBlob
    Reference< XClob >          xClob;              // getClob
    Reference< XArray >         xArray;             // getArray
    Any                         aObject;            // getObject

    PropertyValue()
        : nPropsSet( PropsSet::NONE ), nOrigValue( PropsSet::NONE ),
          bBoolean(false),
          nByte(0),
          nShort(0),
          nInt(0),
          nLong(0),
          nFloat(0.0),
          nDouble(0.0)
        {}
};
} // namespace ucbhelper_impl

using namespace ucbhelper_impl;

namespace ucbhelper
{

// class PropertyValues.
class PropertyValues : public std::vector< ucbhelper_impl::PropertyValue > {};

} // namespace ucbhelper


namespace ucbhelper {


// PropertyValueSet Implementation.


PropertyValueSet::PropertyValueSet(
                    const Reference< XComponentContext >& rxContext )
:  m_xContext( rxContext ),
  m_pValues( new PropertyValues ),
  m_bWasNull( false ),
  m_bTriedToGetTypeConverter( false )

{
}


// virtual
PropertyValueSet::~PropertyValueSet()
{
}


// XInterface methods.
void SAL_CALL PropertyValueSet::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL PropertyValueSet::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL PropertyValueSet::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XTypeProvider* >(this),
                                               static_cast< XRow* >(this),
                                               static_cast< XColumnLocate* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( PropertyValueSet,
                      XTypeProvider,
                         XRow,
                      XColumnLocate );


// XRow methods.


template <class T, T ucbhelper_impl::PropertyValue::*_member_name_>
T PropertyValueSet::getValue(PropsSet nTypeName, sal_Int32 columnIndex)
{
    osl::MutexGuard aGuard( m_aMutex );

    T aValue {};   /* default ctor */

    m_bWasNull = true;

    if ( ( columnIndex < 1 ) || ( columnIndex > sal_Int32( m_pValues->size() ) ) )
    {
        OSL_FAIL( "PropertyValueSet - index out of range!" );
        return aValue;
    }
    ucbhelper_impl::PropertyValue& rValue = (*m_pValues)[ columnIndex - 1 ];

    if ( rValue.nOrigValue == PropsSet::NONE )
        return aValue;

    if ( rValue.nPropsSet & nTypeName )
    {
        /* Values is present natively... */
        aValue = rValue.*_member_name_;
        m_bWasNull = false;
        return aValue;
    }

    if ( !(rValue.nPropsSet & PropsSet::Object) )
    {
        /* Value is not (yet) available as Any. Create it. */
        getObject( columnIndex, Reference< XNameAccess >() );
    }

    if ( rValue.nPropsSet & PropsSet::Object )
    {
        /* Value is available as Any. */

        if ( rValue.aObject.hasValue() )
        {
            /* Try to convert into native value. */
            if ( rValue.aObject >>= aValue )
            {
                rValue.*_member_name_ = aValue;
                rValue.nPropsSet |= nTypeName;
                m_bWasNull = false;
            }
            else
            {
                /* Last chance. Try type converter service... */

                Reference< XTypeConverter > xConverter = getTypeConverter();
                if ( xConverter.is() )
                {
                    try
                    {
                        Any aConvAny = xConverter->convertTo(
                                                 rValue.aObject,
                                                 cppu::UnoType<T>::get() );

                        if ( aConvAny >>= aValue )
                        {
                            rValue.*_member_name_ = aValue;
                            rValue.nPropsSet |= nTypeName;
                            m_bWasNull = false;
                        }
                    }
                    catch (const IllegalArgumentException&)
                    {
                    }
                    catch (const CannotConvertException&)
                    {
                    }
                }
            }
        }
    }

    return aValue;
}


// virtual
sal_Bool SAL_CALL PropertyValueSet::wasNull()
{
    // This method can not be implemented correctly!!! Imagine different
    // threads doing a getXYZ - wasNull calling sequence on the same
    // implementation object...
    return m_bWasNull;
}


// virtual
OUString SAL_CALL PropertyValueSet::getString( sal_Int32 columnIndex )
{
    return getValue<OUString, &ucbhelper_impl::PropertyValue::aString>(PropsSet::String, columnIndex);
}


// virtual
sal_Bool SAL_CALL PropertyValueSet::getBoolean( sal_Int32 columnIndex )
{
    return getValue<bool, &ucbhelper_impl::PropertyValue::bBoolean>(PropsSet::Boolean, columnIndex);
}


// virtual
sal_Int8 SAL_CALL PropertyValueSet::getByte( sal_Int32 columnIndex )
{
    return getValue<sal_Int8, &ucbhelper_impl::PropertyValue::nByte>(PropsSet::Byte, columnIndex);
}


// virtual
sal_Int16 SAL_CALL PropertyValueSet::getShort( sal_Int32 columnIndex )
{
    return getValue<sal_Int16, &ucbhelper_impl::PropertyValue::nShort>(PropsSet::Short, columnIndex);
}


// virtual
sal_Int32 SAL_CALL PropertyValueSet::getInt( sal_Int32 columnIndex )
{
    return getValue<sal_Int32, &ucbhelper_impl::PropertyValue::nInt>(PropsSet::Int, columnIndex);
}


// virtual
sal_Int64 SAL_CALL PropertyValueSet::getLong( sal_Int32 columnIndex )
{
    return getValue<sal_Int64, &ucbhelper_impl::PropertyValue::nLong>(PropsSet::Long, columnIndex);
}


// virtual
float SAL_CALL PropertyValueSet::getFloat( sal_Int32 columnIndex )
{
    return getValue<float, &ucbhelper_impl::PropertyValue::nFloat>(PropsSet::Float, columnIndex);
}


// virtual
double SAL_CALL PropertyValueSet::getDouble( sal_Int32 columnIndex )
{
    return getValue<double, &ucbhelper_impl::PropertyValue::nDouble>(PropsSet::Double, columnIndex);
}


// virtual
Sequence< sal_Int8 > SAL_CALL
PropertyValueSet::getBytes( sal_Int32 columnIndex )
{
    return getValue<Sequence< sal_Int8 >, &ucbhelper_impl::PropertyValue::aBytes>(PropsSet::Bytes, columnIndex);
}


// virtual
Date SAL_CALL PropertyValueSet::getDate( sal_Int32 columnIndex )
{
    return getValue<Date, &ucbhelper_impl::PropertyValue::aDate>(PropsSet::Date, columnIndex);
}


// virtual
Time SAL_CALL PropertyValueSet::getTime( sal_Int32 columnIndex )
{
    return getValue<Time, &ucbhelper_impl::PropertyValue::aTime>(PropsSet::Time, columnIndex);
}


// virtual
DateTime SAL_CALL PropertyValueSet::getTimestamp( sal_Int32 columnIndex )
{
    return getValue<DateTime, &ucbhelper_impl::PropertyValue::aTimestamp>(PropsSet::Timestamp, columnIndex);
}


// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getBinaryStream( sal_Int32 columnIndex )
{
    return getValue<Reference< XInputStream >, &ucbhelper_impl::PropertyValue::xBinaryStream>(PropsSet::BinaryStream, columnIndex);
}


// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getCharacterStream( sal_Int32 columnIndex )
{
    return getValue<Reference< XInputStream >, &ucbhelper_impl::PropertyValue::xCharacterStream>(PropsSet::CharacterStream, columnIndex);
}


// virtual
Any SAL_CALL PropertyValueSet::getObject(
                                    sal_Int32 columnIndex,
                                         const Reference< XNameAccess >& )
{
    osl::MutexGuard aGuard( m_aMutex );

    Any aValue;

    m_bWasNull = true;

    if ( ( columnIndex < 1 )
         || ( columnIndex > sal_Int32( m_pValues->size() ) ) )
    {
        OSL_FAIL( "PropertyValueSet - index out of range!" );
    }
    else
    {
        ucbhelper_impl::PropertyValue& rValue
            = (*m_pValues)[ columnIndex - 1 ];

        if ( rValue.nPropsSet & PropsSet::Object )
        {
            // Values is present natively...
            aValue = rValue.aObject;
            m_bWasNull = false;
        }
        else
        {
            // Make Any from original value.

            switch ( rValue.nOrigValue )
            {
                case PropsSet::NONE:
                    break;

                case PropsSet::String:
                    aValue <<= rValue.aString;
                    break;

                case PropsSet::Boolean:
                    aValue <<= rValue.bBoolean;
                    break;

                case PropsSet::Byte:
                    aValue <<= rValue.nByte;
                    break;

                case PropsSet::Short:
                    aValue <<= rValue.nShort;
                    break;

                case PropsSet::Int:
                    aValue <<= rValue.nInt;
                    break;

                case PropsSet::Long:
                    aValue <<= rValue.nLong;
                    break;

                case PropsSet::Float:
                    aValue <<= rValue.nFloat;
                    break;

                case PropsSet::Double:
                    aValue <<= rValue.nDouble;
                    break;

                case PropsSet::Bytes:
                    aValue <<= rValue.aBytes;
                    break;

                case PropsSet::Date:
                    aValue <<= rValue.aDate;
                    break;

                case PropsSet::Time:
                    aValue <<= rValue.aTime;
                    break;

                case PropsSet::Timestamp:
                    aValue <<= rValue.aTimestamp;
                    break;

                case PropsSet::BinaryStream:
                    aValue <<= rValue.xBinaryStream;
                    break;

                case PropsSet::CharacterStream:
                    aValue <<= rValue.xCharacterStream;
                    break;

                case PropsSet::Ref:
                    aValue <<= rValue.xRef;
                    break;

                case PropsSet::Blob:
                    aValue <<= rValue.xBlob;
                    break;

                case PropsSet::Clob:
                    aValue <<= rValue.xClob;
                    break;

                case PropsSet::Array:
                    aValue <<= rValue.xArray;
                    break;

                case PropsSet::Object:
                    // Fall-through is intended!
                default:
                    OSL_FAIL( "PropertyValueSet::getObject - "
                                "Wrong original type" );
                    break;
            }

            if ( aValue.hasValue() )
            {
                rValue.aObject = aValue;
                rValue.nPropsSet |= PropsSet::Object;
                m_bWasNull = false;
            }
        }
     }

    return aValue;
}


// virtual
Reference< XRef > SAL_CALL PropertyValueSet::getRef( sal_Int32 columnIndex )
{
    return getValue<Reference< XRef >, &ucbhelper_impl::PropertyValue::xRef>(PropsSet::Ref, columnIndex);
}


// virtual
Reference< XBlob > SAL_CALL PropertyValueSet::getBlob( sal_Int32 columnIndex )
{
    return getValue<Reference< XBlob >, &ucbhelper_impl::PropertyValue::xBlob>(PropsSet::Blob, columnIndex);
}


// virtual
Reference< XClob > SAL_CALL PropertyValueSet::getClob( sal_Int32 columnIndex )
{
    return getValue<Reference< XClob >, &ucbhelper_impl::PropertyValue::xClob>(PropsSet::Clob, columnIndex);
}


// virtual
Reference< XArray > SAL_CALL PropertyValueSet::getArray( sal_Int32 columnIndex )
{
    return getValue<Reference< XArray >, &ucbhelper_impl::PropertyValue::xArray>(PropsSet::Array, columnIndex);
}


// XColumnLocate methods.


// virtual
sal_Int32 SAL_CALL PropertyValueSet::findColumn( const OUString& columnName )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !columnName.isEmpty() )
    {
        sal_Int32 nCount = m_pValues->size();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            if ( (*m_pValues)[ n ].sPropertyName == columnName  )
                return n + 1; // Index is 1-based.
        }
    }
    return 0;
}


// Non-interface methods.


const Reference< XTypeConverter >& PropertyValueSet::getTypeConverter()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_bTriedToGetTypeConverter && !m_xTypeConverter.is() )
    {
        m_bTriedToGetTypeConverter = true;
        m_xTypeConverter = Converter::create(m_xContext);

        OSL_ENSURE( m_xTypeConverter.is(),
                    "PropertyValueSet::getTypeConverter() - "
                    "Service 'com.sun.star.script.Converter' n/a!" );
    }
    return m_xTypeConverter;
}


template <class T, T ucbhelper_impl::PropertyValue::*_member_name_>
void PropertyValueSet::appendValue(const OUString& rPropName, PropsSet nTypeName, const T& rValue)
{
    osl::MutexGuard aGuard( m_aMutex );

    ucbhelper_impl::PropertyValue aNewValue;
    aNewValue.sPropertyName = rPropName;
    aNewValue.nPropsSet     = nTypeName;
    aNewValue.nOrigValue    = nTypeName;
    aNewValue.*_member_name_ = rValue;

    m_pValues->push_back( aNewValue );
}


void PropertyValueSet::appendString( const OUString& rPropName,
                                     const OUString& rValue )
{
    appendValue<OUString, &ucbhelper_impl::PropertyValue::aString>(rPropName, PropsSet::String, rValue);
}


void PropertyValueSet::appendBoolean( const OUString& rPropName,
                                      bool bValue )
{
    appendValue<bool, &ucbhelper_impl::PropertyValue::bBoolean>(rPropName, PropsSet::Boolean, bValue);
}


void PropertyValueSet::appendLong( const OUString& rPropName,
                                   sal_Int64 nValue )
{
    appendValue<sal_Int64, &ucbhelper_impl::PropertyValue::nLong>(rPropName, PropsSet::Long, nValue);
}


void PropertyValueSet::appendTimestamp( const OUString& rPropName,
                                        const DateTime& rValue )
{
    appendValue<DateTime, &ucbhelper_impl::PropertyValue::aTimestamp>(rPropName, PropsSet::Timestamp, rValue);
}


void PropertyValueSet::appendObject( const OUString& rPropName,
                                     const Any& rValue )
{
    appendValue<Any, &ucbhelper_impl::PropertyValue::aObject>(rPropName, PropsSet::Object, rValue);
}


void PropertyValueSet::appendVoid( const OUString& rPropName )
{
    appendValue<Any, &ucbhelper_impl::PropertyValue::aObject>(rPropName, PropsSet::NONE, Any());
}


void PropertyValueSet::appendPropertySet(
                                const Reference< XPropertySet >& rxSet )
{
    if ( rxSet.is() )
    {
        Reference< XPropertySetInfo > xInfo = rxSet->getPropertySetInfo();
        if ( xInfo.is() )
        {
            Sequence< Property > aProps      = xInfo->getProperties();

            Reference< XPropertyAccess > xPropertyAccess( rxSet, UNO_QUERY );
            if ( xPropertyAccess.is() )
            {
                // Efficient: Get all prop values with one ( remote) call.

                Sequence< css::beans::PropertyValue > aPropValues
                    = xPropertyAccess->getPropertyValues();

                for ( const css::beans::PropertyValue& rPropValue : aPropValues )
                {
                    // Find info for current property value.
                    auto pProp = std::find_if(aProps.begin(), aProps.end(),
                        [&rPropValue](const Property& rProp) { return rProp.Name == rPropValue.Name; });
                    if (pProp != aProps.end())
                    {
                        // Found!
                        appendObject( *pProp, rPropValue.Value );
                    }
                }
            }
            else
            {
                // Get every single prop value with one ( remote) call.

                for ( const Property& rProp : aProps )
                {
                    try
                    {
                        Any aValue = rxSet->getPropertyValue( rProp.Name );

                        if ( aValue.hasValue() )
                            appendObject( rProp, aValue );
                    }
                    catch (const UnknownPropertyException&)
                    {
                    }
                    catch (const WrappedTargetException&)
                    {
                    }
                }
            }
        }
    }
}


bool PropertyValueSet::appendPropertySetValue(
                                const Reference< XPropertySet >& rxSet,
                                const Property& rProperty )
{
    if ( rxSet.is() )
    {
        try
        {
            Any aValue = rxSet->getPropertyValue( rProperty.Name );
            if ( aValue.hasValue() )
            {
                appendObject( rProperty, aValue );
                return true;
            }
        }
        catch (const UnknownPropertyException&)
        {
        }
        catch (const WrappedTargetException&)
        {
        }
    }

    // Error.
    return false;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
