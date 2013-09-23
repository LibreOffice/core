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
#include <com/sun/star/script/Converter.hpp>

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include <ucbhelper/propertyvalueset.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

namespace ucbhelper_impl
{

//=========================================================================
//
// PropertyValue.
//
//=========================================================================

const sal_uInt32 NO_VALUE_SET               = 0x00000000;
const sal_uInt32 STRING_VALUE_SET           = 0x00000001;
const sal_uInt32 BOOLEAN_VALUE_SET          = 0x00000002;
const sal_uInt32 BYTE_VALUE_SET             = 0x00000004;
const sal_uInt32 SHORT_VALUE_SET            = 0x00000008;
const sal_uInt32 INT_VALUE_SET              = 0x00000010;
const sal_uInt32 LONG_VALUE_SET             = 0x00000020;
const sal_uInt32 FLOAT_VALUE_SET            = 0x00000040;
const sal_uInt32 DOUBLE_VALUE_SET           = 0x00000080;
const sal_uInt32 BYTES_VALUE_SET            = 0x00000100;
const sal_uInt32 DATE_VALUE_SET             = 0x00000200;
const sal_uInt32 TIME_VALUE_SET             = 0x00000400;
const sal_uInt32 TIMESTAMP_VALUE_SET        = 0x00000800;
const sal_uInt32 BINARYSTREAM_VALUE_SET     = 0x00001000;
const sal_uInt32 CHARACTERSTREAM_VALUE_SET  = 0x00002000;
const sal_uInt32 REF_VALUE_SET              = 0x00004000;
const sal_uInt32 BLOB_VALUE_SET             = 0x00008000;
const sal_uInt32 CLOB_VALUE_SET             = 0x00010000;
const sal_uInt32 ARRAY_VALUE_SET            = 0x00020000;
const sal_uInt32 OBJECT_VALUE_SET           = 0x00040000;

struct PropertyValue
{
    OUString
                sPropertyName;

    sal_uInt32  nPropsSet;
    sal_uInt32  nOrigValue;

    OUString    aString;    // getString
    sal_Bool    bBoolean;   // getBoolean
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
        : nPropsSet( NO_VALUE_SET ), nOrigValue( NO_VALUE_SET ),
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

//=========================================================================
//
// class PropertyValues.
//
//=========================================================================

typedef std::vector< ucbhelper_impl::PropertyValue > PropertyValuesVector;

class PropertyValues : public PropertyValuesVector {};

} // namespace ucbhelper

//=========================================================================
//
// Welcome to the macro hell...
//
//=========================================================================

#define GETVALUE_IMPL_TYPE( _type_, _type_name_, _member_name_, _cppu_type_ ) \
                                                                              \
    osl::MutexGuard aGuard( m_aMutex );                                       \
                                                                              \
    _type_ aValue = _type_();   /* default ctor */                            \
                                                                              \
    m_bWasNull = sal_True;                                                    \
                                                                              \
    if ( ( columnIndex < 1 )                                                  \
         || ( columnIndex > sal_Int32( m_pValues->size() ) ) )                \
    {                                                                         \
        OSL_FAIL( "PropertyValueSet - index out of range!" );    \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        ucbhelper_impl::PropertyValue& rValue                                 \
            = (*m_pValues)[ columnIndex - 1 ];                                \
                                                                              \
        if ( rValue.nOrigValue != NO_VALUE_SET )                              \
        {                                                                     \
            if ( rValue.nPropsSet & _type_name_ )                             \
            {                                                                 \
                /* Values is present natively... */                           \
                aValue = rValue._member_name_;                                \
                m_bWasNull = sal_False;                                       \
            }                                                                 \
            else                                                              \
            {                                                                 \
                if ( !(rValue.nPropsSet & OBJECT_VALUE_SET) )                 \
                {                                                             \
                    /* Value is not (yet) available as Any. Create it. */     \
                    getObject( columnIndex, Reference< XNameAccess >() );     \
                }                                                             \
                                                                              \
                if ( rValue.nPropsSet & OBJECT_VALUE_SET )                    \
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
                            m_bWasNull = sal_False;                           \
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
                                                            _cppu_type_ );    \
                                                                              \
                                    if ( aConvAny >>= aValue )                \
                                    {                                         \
                                        rValue._member_name_ = aValue;        \
                                        rValue.nPropsSet |= _type_name_;      \
                                        m_bWasNull = sal_False;               \
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
            }                                                                 \
        }                                                                     \
    }                                                                         \
    return aValue;

#define GETVALUE_IMPL( _type_, _type_name_, _member_name_ )                   \
    GETVALUE_IMPL_TYPE( _type_,                                               \
                        _type_name_,                                          \
                        _member_name_,                                        \
                        getCppuType( static_cast< const _type_ * >( 0 ) ) )

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

//=========================================================================
//=========================================================================
//
// PropertyValueSet Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
PropertyValueSet::PropertyValueSet(
                    const Reference< XComponentContext >& rxContext )
:  m_xContext( rxContext ),
  m_pValues( new PropertyValues ),
  m_bWasNull( sal_False ),
  m_bTriedToGetTypeConverter( sal_False )

{
}

//=========================================================================
// virtual
PropertyValueSet::~PropertyValueSet()
{
    delete m_pValues;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( PropertyValueSet,
                   XTypeProvider,
                   XRow,
                   XColumnLocate );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( PropertyValueSet,
                      XTypeProvider,
                         XRow,
                      XColumnLocate );

//=========================================================================
//
// XRow methods.
//
//=========================================================================

// virtual
sal_Bool SAL_CALL PropertyValueSet::wasNull()
    throw( SQLException, RuntimeException )
{
    // This method can not be implemented correctly!!! Imagine different
    // threads doing a getXYZ - wasNull calling sequence on the same
    // implementation object...
    return m_bWasNull;
}

//=========================================================================
// virtual
OUString SAL_CALL PropertyValueSet::getString( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( OUString, STRING_VALUE_SET, aString );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertyValueSet::getBoolean( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL_TYPE(
            sal_Bool, BOOLEAN_VALUE_SET, bBoolean, getCppuBooleanType() );
}

//=========================================================================
// virtual
sal_Int8 SAL_CALL PropertyValueSet::getByte( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( sal_Int8, BYTE_VALUE_SET, nByte );
}

//=========================================================================
// virtual
sal_Int16 SAL_CALL PropertyValueSet::getShort( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( sal_Int16, SHORT_VALUE_SET, nShort );
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL PropertyValueSet::getInt( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( sal_Int32, INT_VALUE_SET, nInt );
}

//=========================================================================
// virtual
sal_Int64 SAL_CALL PropertyValueSet::getLong( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( sal_Int64, LONG_VALUE_SET, nLong );
}

//=========================================================================
// virtual
float SAL_CALL PropertyValueSet::getFloat( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( float, FLOAT_VALUE_SET, nFloat );
}

//=========================================================================
// virtual
double SAL_CALL PropertyValueSet::getDouble( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( double, DOUBLE_VALUE_SET, nDouble );
}

//=========================================================================
// virtual
Sequence< sal_Int8 > SAL_CALL
PropertyValueSet::getBytes( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Sequence< sal_Int8 >, BYTES_VALUE_SET, aBytes );
}

//=========================================================================
// virtual
Date SAL_CALL PropertyValueSet::getDate( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Date, DATE_VALUE_SET, aDate );
}

//=========================================================================
// virtual
Time SAL_CALL PropertyValueSet::getTime( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Time, TIME_VALUE_SET, aTime );
}

//=========================================================================
// virtual
DateTime SAL_CALL PropertyValueSet::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( DateTime, TIMESTAMP_VALUE_SET, aTimestamp );
}

//=========================================================================
// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL(
        Reference< XInputStream >, BINARYSTREAM_VALUE_SET, xBinaryStream );
}

//=========================================================================
// virtual
Reference< XInputStream > SAL_CALL
PropertyValueSet::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL(
        Reference< XInputStream >, CHARACTERSTREAM_VALUE_SET, xCharacterStream );
}

//=========================================================================
// virtual
Any SAL_CALL PropertyValueSet::getObject(
                                    sal_Int32 columnIndex,
                                         const Reference< XNameAccess >& )
    throw( SQLException, RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    Any aValue;

    m_bWasNull = sal_True;

    if ( ( columnIndex < 1 )
         || ( columnIndex > sal_Int32( m_pValues->size() ) ) )
    {
        OSL_FAIL( "PropertyValueSet - index out of range!" );
    }
    else
    {
        ucbhelper_impl::PropertyValue& rValue
            = (*m_pValues)[ columnIndex - 1 ];

        if ( rValue.nPropsSet & OBJECT_VALUE_SET )
        {
            // Values is present natively...
            aValue = rValue.aObject;
            m_bWasNull = sal_False;
        }
        else
        {
            // Make Any from original value.

            switch ( rValue.nOrigValue )
            {
                case NO_VALUE_SET:
                    break;

                case STRING_VALUE_SET:
                    aValue <<= rValue.aString;
                    break;

                case BOOLEAN_VALUE_SET:
                    aValue <<= rValue.bBoolean;
                    break;

                case BYTE_VALUE_SET:
                    aValue <<= rValue.nByte;
                    break;

                case SHORT_VALUE_SET:
                    aValue <<= rValue.nShort;
                    break;

                case INT_VALUE_SET:
                    aValue <<= rValue.nInt;
                    break;

                case LONG_VALUE_SET:
                    aValue <<= rValue.nLong;
                    break;

                case FLOAT_VALUE_SET:
                    aValue <<= rValue.nFloat;
                    break;

                case DOUBLE_VALUE_SET:
                    aValue <<= rValue.nDouble;
                    break;

                case BYTES_VALUE_SET:
                    aValue <<= rValue.aBytes;
                    break;

                case DATE_VALUE_SET:
                    aValue <<= rValue.aDate;
                    break;

                case TIME_VALUE_SET:
                    aValue <<= rValue.aTime;
                    break;

                case TIMESTAMP_VALUE_SET:
                    aValue <<= rValue.aTimestamp;
                    break;

                case BINARYSTREAM_VALUE_SET:
                    aValue <<= rValue.xBinaryStream;
                    break;

                case CHARACTERSTREAM_VALUE_SET:
                    aValue <<= rValue.xCharacterStream;
                    break;

                case REF_VALUE_SET:
                    aValue <<= rValue.xRef;
                    break;

                case BLOB_VALUE_SET:
                    aValue <<= rValue.xBlob;
                    break;

                case CLOB_VALUE_SET:
                    aValue <<= rValue.xClob;
                    break;

                case ARRAY_VALUE_SET:
                    aValue <<= rValue.xArray;
                    break;

                case OBJECT_VALUE_SET:
                    // Fall-through is intended!
                default:
                    OSL_FAIL( "PropertyValueSet::getObject - "
                                "Wrong original type" );
                    break;
            }

            if ( aValue.hasValue() )
            {
                rValue.aObject = aValue;
                rValue.nPropsSet |= OBJECT_VALUE_SET;
                m_bWasNull = sal_False;
            }
        }
     }

    return aValue;
}

//=========================================================================
// virtual
Reference< XRef > SAL_CALL PropertyValueSet::getRef( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Reference< XRef >, REF_VALUE_SET, xRef );
}

//=========================================================================
// virtual
Reference< XBlob > SAL_CALL PropertyValueSet::getBlob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Reference< XBlob >, BLOB_VALUE_SET, xBlob );
}

//=========================================================================
// virtual
Reference< XClob > SAL_CALL PropertyValueSet::getClob( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Reference< XClob >, CLOB_VALUE_SET, xClob );
}

//=========================================================================
// virtual
Reference< XArray > SAL_CALL PropertyValueSet::getArray( sal_Int32 columnIndex )
    throw( SQLException, RuntimeException )
{
    GETVALUE_IMPL( Reference< XArray >, ARRAY_VALUE_SET, xArray );
}

//=========================================================================
//
// XColumnLocate methods.
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL PropertyValueSet::findColumn( const OUString& columnName )
    throw( SQLException, RuntimeException )
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

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

const Reference< XTypeConverter >& PropertyValueSet::getTypeConverter()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_bTriedToGetTypeConverter && !m_xTypeConverter.is() )
    {
        m_bTriedToGetTypeConverter = sal_True;
        m_xTypeConverter = Converter::create(m_xContext);

        OSL_ENSURE( m_xTypeConverter.is(),
                    "PropertyValueSet::getTypeConverter() - "
                    "Service 'com.sun.star.script.Converter' n/a!" );
    }
    return m_xTypeConverter;
}

//=========================================================================
void PropertyValueSet::appendString( const OUString& rPropName,
                                     const OUString& rValue )
{
    SETVALUE_IMPL( rPropName, STRING_VALUE_SET, aString, rValue );
}

//=========================================================================
void PropertyValueSet::appendBoolean( const OUString& rPropName,
                                      sal_Bool bValue )
{
    SETVALUE_IMPL( rPropName, BOOLEAN_VALUE_SET, bBoolean, bValue );
}

//=========================================================================
void PropertyValueSet::appendLong( const OUString& rPropName,
                                   sal_Int64 nValue )
{
    SETVALUE_IMPL( rPropName, LONG_VALUE_SET, nLong, nValue );
}

//=========================================================================
void PropertyValueSet::appendTimestamp( const OUString& rPropName,
                                        const DateTime& rValue )
{
    SETVALUE_IMPL( rPropName, TIMESTAMP_VALUE_SET, aTimestamp, rValue );
}

//=========================================================================
void PropertyValueSet::appendObject( const OUString& rPropName,
                                     const Any& rValue )
{
    SETVALUE_IMPL( rPropName, OBJECT_VALUE_SET, aObject, rValue );
}

//=========================================================================
void PropertyValueSet::appendVoid( const OUString& rPropName )
{
    SETVALUE_IMPL( rPropName, NO_VALUE_SET, aObject, Any() );
}

//=========================================================================
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

                Sequence< ::com::sun::star::beans::PropertyValue > aPropValues
                    = xPropertyAccess->getPropertyValues();

                const ::com::sun::star::beans::PropertyValue* pPropValues
                      = aPropValues.getConstArray();

                sal_Int32 nValuesCount = aPropValues.getLength();
                for ( sal_Int32 n = 0; n < nValuesCount; ++n )
                {
                    const ::com::sun::star::beans::PropertyValue& rPropValue
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

//=========================================================================
sal_Bool PropertyValueSet::appendPropertySetValue(
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
                return sal_True;
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
    return sal_False;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
