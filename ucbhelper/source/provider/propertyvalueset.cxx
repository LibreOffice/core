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

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include <ucbhelper/propertyvalueset.hxx>
#include <o3tl/typed_flags_set.hxx>

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

    inline PropertyValue()
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


// Welcome to the macro hell...


#define GETVALUE_IMPL( _type_, _type_name_, _member_name_ ) \
                                                                  \
    osl::MutexGuard aGuard( m_aMutex );                           \
                                                                  \
    _type_ aValue = _type_();   /* default ctor */                \
                                                                  \
    m_bWasNull = true;                                            \
                                                                  \
    if ( ( columnIndex < 1 )                                      \
         || ( columnIndex > sal_Int32( m_pValues->size() ) ) )    \
    {                                                             \
        OSL_FAIL( "PropertyValueSet - index out of range!" );     \
        return aValue;                                            \
    }                                                             \
    ucbhelper_impl::PropertyValue& rValue                         \
        = (*m_pValues)[ columnIndex - 1 ];                        \
                                                                  \
    if ( rValue.nOrigValue == PropsSet::NONE )                    \
        return aValue;                                            \
                                                                  \
    if ( rValue.nPropsSet & _type_name_ )                         \
    {                                                             \
        /* Values is present natively... */                       \
        aValue = rValue._member_name_;                            \
        m_bWasNull = false;                                       \
        return aValue;                                            \
    }                                                             \
                                                                  \
    if ( !(rValue.nPropsSet & PropsSet::Object) )                 \
    {                                                             \
        /* Value is not (yet) available as Any. Create it. */     \
        getObject( columnIndex, Reference< XNameAccess >() );     \
    }                                                             \
                                                                  \
    if ( rValue.nPropsSet & PropsSet::Object )                    \
    {                                                             \
        /* Value is available as Any. */                          \
                                                                  \
        if ( rValue.aObject.hasValue() )                          \
        {                                                         \
            /* Try to convert into native value. */               \
            if ( rValue.aObject >>= aValue )                      \
            {                                                     \
                rValue._member_name_ = aValue;                    \
                rValue.nPropsSet |= _type_name_;                  \
                m_bWasNull = false;                               \
            }                                                     \
            else                                                  \
            {                                                     \
                /* Last chance. Try type converter service... */  \
                                                                  \
                Reference< XTypeConverter > xConverter            \
                                        = getTypeConverter();     \
                if ( xConverter.is() )                            \
                {                                                 \
                    try                                           \
                    {                                             \
                        Any aConvAny = xConverter->convertTo(     \
                                                 rValue.aObject,      \
                                                 cppu::UnoType<_type_>::get() );    \
                                                                  \
                        if ( aConvAny >>= aValue )                \
                        {                                         \
                            rValue._member_name_ = aValue;        \
                            rValue.nPropsSet |= _type_name_;      \
                            m_bWasNull = false;               \
                        }                                         \
                    }                                             \
                    catch (const IllegalArgumentException&)       \
                    {                                             \
                    }                                             \
                    catch (const CannotConvertException&)         \
                    {                                             \
                    }                                             \
                }                                                 \
            }                                                     \
        }                                                         \
    }                                                             \
    return aValue;

#define SETVALUE_IMPL( _prop_name_, _type_name_, _member_name_, _value_ )     \
                                                                              \
    osl::MutexGuard aGuard( m_aMutex );                                       \
                                                                              \
    ucbhelper_impl::PropertyValue aNewValue;                                  \
    aNewValue.sPropertyName = _prop_name_;                                    \
    aNewValue.nPropsSet     = _type_name_;                                    \
    aNewValue.nOrigValue    = _type_name_;                                    \
    aNewValue._member_name_ = _value_;                                        \
                                                                              \
    m_pValues->push_back( aNewValue );

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
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XRow* >(this)),
                                               (static_cast< XColumnLocate* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( PropertyValueSet,
                      XTypeProvider,
                         XRow,
                      XColumnLocate );


// XRow methods.


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
    GETVALUE_IMPL( OUString, PropsSet::String, aString );
}


// virtual
sal_Bool SAL_CALL PropertyValueSet::getBoolean( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( bool, PropsSet::Boolean, bBoolean );
}


// virtual
sal_Int8 SAL_CALL PropertyValueSet::getByte( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( sal_Int8, PropsSet::Byte, nByte );
}


// virtual
sal_Int16 SAL_CALL PropertyValueSet::getShort( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( sal_Int16, PropsSet::Short, nShort );
}


// virtual
sal_Int32 SAL_CALL PropertyValueSet::getInt( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( sal_Int32, PropsSet::Int, nInt );
}


// virtual
sal_Int64 SAL_CALL PropertyValueSet::getLong( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( sal_Int64, PropsSet::Long, nLong );
}


// virtual
float SAL_CALL PropertyValueSet::getFloat( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( float, PropsSet::Float, nFloat );
}


// virtual
double SAL_CALL PropertyValueSet::getDouble( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( double, PropsSet::Double, nDouble );
}


// virtual
Sequence< sal_Int8 > SAL_CALL
PropertyValueSet::getBytes( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Sequence< sal_Int8 >, PropsSet::Bytes, aBytes );
}


// virtual
Date SAL_CALL PropertyValueSet::getDate( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Date, PropsSet::Date, aDate );
}


// virtual
Time SAL_CALL PropertyValueSet::getTime( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Time, PropsSet::Time, aTime );
}


// virtual
DateTime SAL_CALL PropertyValueSet::getTimestamp( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( DateTime, PropsSet::Timestamp, aTimestamp );
}


// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getBinaryStream( sal_Int32 columnIndex )
{
    GETVALUE_IMPL(
        Reference< XInputStream >, PropsSet::BinaryStream, xBinaryStream );
}


// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getCharacterStream( sal_Int32 columnIndex )
{
    GETVALUE_IMPL(
        Reference< XInputStream >, PropsSet::CharacterStream, xCharacterStream );
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
    GETVALUE_IMPL( Reference< XRef >, PropsSet::Ref, xRef );
}


// virtual
Reference< XBlob > SAL_CALL PropertyValueSet::getBlob( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Reference< XBlob >, PropsSet::Blob, xBlob );
}


// virtual
Reference< XClob > SAL_CALL PropertyValueSet::getClob( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Reference< XClob >, PropsSet::Clob, xClob );
}


// virtual
Reference< XArray > SAL_CALL PropertyValueSet::getArray( sal_Int32 columnIndex )
{
    GETVALUE_IMPL( Reference< XArray >, PropsSet::Array, xArray );
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
            if ( (*m_pValues)[ n ].sPropertyName.equals( columnName ) )
                return sal_Int32( n + 1 ); // Index is 1-based.
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


void PropertyValueSet::appendString( const OUString& rPropName,
                                     const OUString& rValue )
{
    SETVALUE_IMPL( rPropName, PropsSet::String, aString, rValue );
}


void PropertyValueSet::appendBoolean( const OUString& rPropName,
                                      bool bValue )
{
    SETVALUE_IMPL( rPropName, PropsSet::Boolean, bBoolean, bValue );
}


void PropertyValueSet::appendLong( const OUString& rPropName,
                                   sal_Int64 nValue )
{
    SETVALUE_IMPL( rPropName, PropsSet::Long, nLong, nValue );
}


void PropertyValueSet::appendTimestamp( const OUString& rPropName,
                                        const DateTime& rValue )
{
    SETVALUE_IMPL( rPropName, PropsSet::Timestamp, aTimestamp, rValue );
}


void PropertyValueSet::appendObject( const OUString& rPropName,
                                     const Any& rValue )
{
    SETVALUE_IMPL( rPropName, PropsSet::Object, aObject, rValue );
}


void PropertyValueSet::appendVoid( const OUString& rPropName )
{
    SETVALUE_IMPL( rPropName, PropsSet::NONE, aObject, Any() );
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
            const Property*      pProps      = aProps.getConstArray();
            sal_Int32            nPropsCount = aProps.getLength();

            Reference< XPropertyAccess > xPropertyAccess( rxSet, UNO_QUERY );
            if ( xPropertyAccess.is() )
            {
                // Efficient: Get all prop values with one ( remote) call.

                Sequence< css::beans::PropertyValue > aPropValues
                    = xPropertyAccess->getPropertyValues();

                const css::beans::PropertyValue* pPropValues
                      = aPropValues.getConstArray();

                sal_Int32 nValuesCount = aPropValues.getLength();
                for ( sal_Int32 n = 0; n < nValuesCount; ++n )
                {
                    const css::beans::PropertyValue& rPropValue
                        = pPropValues[ n ];

                    // Find info for current property value.
                    for ( sal_Int32 m = 0; m < nPropsCount; ++m )
                    {
                        const Property& rProp = pProps[ m ];
                        if ( rProp.Name == rPropValue.Name )
                        {
                            // Found!
                            appendObject( rProp, rPropValue.Value );
                            break;
                        }
                    }
                }
            }
            else
            {
                // Get every single prop value with one ( remote) call.

                for ( sal_Int32 n = 0; n < nPropsCount; ++n )
                {
                    const Property& rProp = pProps[ n ];

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
