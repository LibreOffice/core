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


#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase2.hxx>

#include <typelib/typedescription.hxx>
#include <uno/data.h>

#ifdef WNT
#include <cmath>
#else
#include <math.h>
#endif
#include <float.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;
#define SERVICENAME "com.sun.star.script.Converter"
#define IMPLNAME    "com.sun.star.comp.stoc.TypeConverter"

namespace stoc_services
{
Sequence< OUString > tcv_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(SERVICENAME);
    return seqNames;
}

OUString tcv_getImplementationName()
{
    return OUString(IMPLNAME);
}
}

namespace stoc_tcv
{

static const sal_uInt64 SAL_UINT64_MAX =
    ((((sal_uInt64)0xffffffff) << 32) | (sal_uInt64)0xffffffff);
static const sal_Int64 SAL_INT64_MAX =
    (sal_Int64)((((sal_uInt64)0x7fffffff) << 32) | (sal_uInt64)0xffffffff);
static const sal_Int64 SAL_INT64_MIN =
    (sal_Int64)(((sal_uInt64)0x80000000) << 32);

/* MS Visual C++ no conversion from unsigned __int64 to double */
#ifdef _MSC_VER
static const double DOUBLE_SAL_UINT64_MAX = ((((double)SAL_INT64_MAX) * 2) + 1);

static inline double unsigned_int64_to_double( sal_uInt64 n ) SAL_THROW(())
{
    sal_uInt64 n2 = (n / 3);
    n -= (2 * n2);
    return (((double)(sal_Int64)n2) * 2.0) + ((double)(sal_Int64)n);
}
#else
static const double DOUBLE_SAL_UINT64_MAX =
    (double)((((sal_uInt64)0xffffffff) << 32) | (sal_uInt64)0xffffffff);

static inline double unsigned_int64_to_double( sal_uInt64 n ) SAL_THROW(())
{
    return (double)n;
}
#endif


//--------------------------------------------------------------------------------------------------
static inline double round( double aVal )
{
    sal_Bool bPos   = (aVal >= 0.0);    //
    aVal            = ::fabs( aVal );
    double aUpper   = ::ceil( aVal );

    aVal            = ((aUpper-aVal) <= 0.5) ? aUpper : (aUpper - 1.0);
    return (bPos ? aVal : -aVal);
}

//--------------------------------------------------------------------------------------------------
static sal_Bool getNumericValue( double & rfVal, const OUString & rStr )
{
    double fRet = rStr.toDouble();
    if (fRet == 0.0)
    {
        sal_Int32 nLen = rStr.getLength();
        if (!nLen || (nLen == 1 && rStr[0] == '0')) // common case
        {
            rfVal = 0.0;
            return sal_True;
        }

        OUString trim( rStr.trim() );

        // try hex
        sal_Int32 nX = trim.indexOf( 'x' );
        if (nX < 0)
            nX = trim.indexOf( 'X' );

        if (nX > 0 && trim[nX-1] == '0') // 0x
        {
            sal_Bool bNeg = sal_False;
            switch (nX)
            {
            case 2: // (+|-)0x...
                if (trim[0] == '-')
                    bNeg = sal_True;
                else if (trim[0] != '+')
                    return sal_False;
            case 1: // 0x...
                break;
            default:
                return sal_False;
            }

            OUString aHexRest( trim.copy( nX+1 ) );
            sal_uInt64 nRet = aHexRest.toUInt64( 16 );

            if (nRet == 0)
            {
                for ( sal_Int32 nPos = aHexRest.getLength(); nPos--; )
                {
                    if (aHexRest[nPos] != '0')
                        return sal_False;
                }
            }

            rfVal = (bNeg ? -(double)nRet : (double)nRet);
            return sal_True;
        }

        nLen = trim.getLength();
        sal_Int32 nPos = 0;

        // skip +/-
        if (nLen && (trim[0] == '-' || trim[0] == '+'))
            ++nPos;

        while (nPos < nLen) // skip leading zeros
        {
            if (trim[nPos] != '0')
            {
                if (trim[nPos] != '.')
                    return sal_False;
                ++nPos;
                while (nPos < nLen) // skip trailing zeros
                {
                    if (trim[nPos] != '0')
                        return sal_False;
                    ++nPos;
                }
                break;
            }
            ++nPos;
        }
    }
    rfVal = fRet;
    return sal_True;
}

//==================================================================================================
static sal_Bool getHyperValue( sal_Int64 & rnVal, const OUString & rStr )
{
    sal_Int32 nLen = rStr.getLength();
    if (!nLen || (nLen == 1 && rStr[0] == '0')) // common case
    {
        rnVal = 0;
        return sal_True;
    }

    OUString trim( rStr.trim() );

    // try hex
    sal_Int32 nX = trim.indexOf( 'x' );
    if (nX < 0)
        nX = trim.indexOf( 'X' );

    if (nX >= 0)
    {
        if (nX > 0 && trim[nX-1] == '0') // 0x
        {
            sal_Bool bNeg = sal_False;
            switch (nX)
            {
            case 2: // (+|-)0x...
                if (trim[0] == '-')
                    bNeg = sal_True;
                else if (trim[0] != '+')
                    return sal_False;
            case 1: // 0x...
                break;
            default:
                return sal_False;
            }

            OUString aHexRest( trim.copy( nX+1 ) );
            sal_uInt64 nRet = aHexRest.toUInt64( 16 );

            if (nRet == 0)
            {
                for ( sal_Int32 nPos = aHexRest.getLength(); nPos--; )
                {
                    if (aHexRest[nPos] != '0')
                        return sal_False;
                }
            }

            rnVal = (bNeg ? -static_cast<sal_Int64>(nRet) : nRet);
            return sal_True;
        }
        return sal_False;
    }

    double fVal;
    if (getNumericValue( fVal, rStr ) &&
        fVal >= (double)SAL_INT64_MIN &&
        fVal <= DOUBLE_SAL_UINT64_MAX)
    {
        rnVal = (sal_Int64)round( fVal );
        return sal_True;
    }
    return sal_False;
}

//==================================================================================================
class TypeConverter_Impl : public WeakImplHelper2< XTypeConverter, XServiceInfo >
{
    // ...misc helpers...
    sal_Int64 toHyper(
        const Any& rAny, sal_Int64 min = SAL_INT64_MIN, sal_uInt64 max = SAL_UINT64_MAX )
        throw( CannotConvertException );
    double toDouble( const Any& rAny, double min = -DBL_MAX, double max = DBL_MAX ) const
        throw( CannotConvertException );

public:
    TypeConverter_Impl();
    virtual ~TypeConverter_Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( RuntimeException );
    virtual  Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw( RuntimeException );

    // XTypeConverter
    virtual Any SAL_CALL convertTo( const Any& aFrom, const Type& DestinationType )
        throw( IllegalArgumentException, CannotConvertException, RuntimeException);
    virtual Any SAL_CALL convertToSimpleType( const Any& aFrom, TypeClass aDestinationType )
        throw( IllegalArgumentException, CannotConvertException, RuntimeException);
};

TypeConverter_Impl::TypeConverter_Impl() {}

TypeConverter_Impl::~TypeConverter_Impl() {}

// XServiceInfo
OUString TypeConverter_Impl::getImplementationName() throw( RuntimeException )
{
    return stoc_services::tcv_getImplementationName();
}

// XServiceInfo
sal_Bool TypeConverter_Impl::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > TypeConverter_Impl::getSupportedServiceNames(void) throw( RuntimeException )
{
    return stoc_services::tcv_getSupportedServiceNames();
}

//--------------------------------------------------------------------------------------------------
sal_Int64 TypeConverter_Impl::toHyper( const Any& rAny, sal_Int64 min, sal_uInt64 max )
    throw( CannotConvertException )
{
    sal_Int64 nRet;
    TypeClass aDestinationClass = rAny.getValueTypeClass();

    switch (aDestinationClass)
    {
    // ENUM
    case TypeClass_ENUM:
        nRet = *(sal_Int32 *)rAny.getValue();
        break;
    // BOOL
    case TypeClass_BOOLEAN:
        nRet = (*(sal_Bool*)rAny.getValue() ? 1 : 0);
        break;
    // CHAR, BYTE
    case TypeClass_CHAR:
        nRet = *(sal_Unicode *)rAny.getValue();
        break;
    case TypeClass_BYTE:
        nRet = *(sal_Int8 *)rAny.getValue();
        break;
    // SHORT
    case TypeClass_SHORT:
        nRet = *(sal_Int16 *)rAny.getValue();
        break;
    // UNSIGNED SHORT
    case TypeClass_UNSIGNED_SHORT:
        nRet = *(sal_uInt16 *)rAny.getValue();
        break;
    // LONG
    case TypeClass_LONG:
        nRet = *(sal_Int32 *)rAny.getValue();
        break;
    // UNSIGNED LONG
    case TypeClass_UNSIGNED_LONG:
        nRet = *(sal_uInt32 *)rAny.getValue();
        break;
    // HYPER
    case TypeClass_HYPER:
        nRet = *(sal_Int64 *)rAny.getValue();
        break;
    // UNSIGNED HYPER
    case TypeClass_UNSIGNED_HYPER:
    {
        nRet = *(sal_Int64 *)rAny.getValue();
        if ((min < 0 || (sal_uInt64)nRet >= (sal_uInt64)min) && // lower bound
            (sal_uInt64)nRet <= max)                            // upper bound
        {
            return nRet;
        }
        throw CannotConvertException(
            OUString("UNSIGNED HYPER out of range!"),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }

    // FLOAT, DOUBLE
    case TypeClass_FLOAT:
    {
        double fVal = round( *(float *)rAny.getValue() );
        nRet = (fVal > SAL_INT64_MAX ? (sal_Int64)(sal_uInt64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= unsigned_int64_to_double( max ))
        {
            return nRet;
        }
        throw CannotConvertException(
            OUString("FLOAT out of range!"),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }
    case TypeClass_DOUBLE:
    {
        double fVal = round( *(double *)rAny.getValue() );
        nRet = (fVal > SAL_INT64_MAX ? (sal_Int64)(sal_uInt64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= unsigned_int64_to_double( max ))
        {
            return nRet;
        }
        throw CannotConvertException(
            OUString("DOUBLE out of range!"),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }

    // STRING
    case TypeClass_STRING:
    {
        sal_Int64 fVal = SAL_CONST_INT64(0);
        if (! getHyperValue( fVal, *(OUString const *)rAny.getValue() ))
        {
            throw CannotConvertException(
                OUString("invalid STRING value!"),
                Reference<XInterface>(), aDestinationClass, FailReason::IS_NOT_NUMBER, 0 );
        }
        nRet = (fVal > SAL_INT64_MAX ? (sal_Int64)(sal_uInt64)fVal : (sal_Int64)fVal);
        if (fVal >= min && (fVal < 0 || ((sal_uInt64)fVal) <= max))
            return nRet;
        throw CannotConvertException(
            OUString("STRING value out of range!"),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }

    default:
        throw CannotConvertException(
            OUString("TYPE is not supported!"),
            Reference<XInterface>(), aDestinationClass, FailReason::TYPE_NOT_SUPPORTED, 0 );
    }

    if (nRet >= min && (nRet < 0 || (sal_uInt64)nRet <= max))
        return nRet;
    throw CannotConvertException(
        OUString("VALUE is out of range!"),
        Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
}

//--------------------------------------------------------------------------------------------------
double TypeConverter_Impl::toDouble( const Any& rAny, double min, double max ) const
    throw( CannotConvertException )
{
    double fRet;
    TypeClass aDestinationClass = rAny.getValueTypeClass();

    switch (aDestinationClass)
    {
    // ENUM
    case TypeClass_ENUM:
        fRet = *(sal_Int32 *)rAny.getValue();
        break;
    // BOOL
    case TypeClass_BOOLEAN:
        fRet = (*(sal_Bool*)rAny.getValue() ? 1.0 : 0.0);
        break;
    // CHAR, BYTE
    case TypeClass_CHAR:
        fRet = *(sal_Unicode *)rAny.getValue();
        break;
    case TypeClass_BYTE:
        fRet = *(sal_Int8 *)rAny.getValue();
        break;
    // SHORT
    case TypeClass_SHORT:
        fRet = *(sal_Int16 *)rAny.getValue();
        break;
    // UNSIGNED SHORT
    case TypeClass_UNSIGNED_SHORT:
        fRet = *(sal_uInt16 *)rAny.getValue();
        break;
    // LONG
    case TypeClass_LONG:
        fRet = *(sal_Int32 *)rAny.getValue();
        break;
    // UNSIGNED LONG
    case TypeClass_UNSIGNED_LONG:
        fRet = *(sal_uInt32 *)rAny.getValue();
        break;
    // HYPER
    case TypeClass_HYPER:
        fRet = (double)*(sal_Int64 *)rAny.getValue();
        break;
    // UNSIGNED HYPER
    case TypeClass_UNSIGNED_HYPER:
        fRet = unsigned_int64_to_double( *(sal_uInt64 const *)rAny.getValue() );
        break;
    // FLOAT, DOUBLE
    case TypeClass_FLOAT:
        fRet = *(float *)rAny.getValue();
        break;
    case TypeClass_DOUBLE:
        fRet = *(double *)rAny.getValue();
        break;

    // STRING
    case TypeClass_STRING:
    {
        if (! getNumericValue( fRet, *(OUString *)rAny.getValue() ))
        {
            throw CannotConvertException(
                OUString("invalid STRING value!"),
                Reference<XInterface>(), aDestinationClass, FailReason::IS_NOT_NUMBER, 0 );
        }
        break;
    }

    default:
        throw CannotConvertException(
            OUString("TYPE is not supported!"),
            Reference< XInterface >(), aDestinationClass, FailReason::TYPE_NOT_SUPPORTED, 0 );
    }

    if (fRet >= min && fRet <= max)
        return fRet;
    throw CannotConvertException(
        OUString("VALUE is out of range!"),
        Reference< XInterface >(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
}

//--------------------------------------------------------------------------------------------------
Any SAL_CALL TypeConverter_Impl::convertTo( const Any& rVal, const Type& aDestType )
    throw( IllegalArgumentException, CannotConvertException, RuntimeException)
{
    Type aSourceType = rVal.getValueType();
    if (aSourceType == aDestType)
        return rVal;

    TypeClass aSourceClass = aSourceType.getTypeClass();
    TypeClass aDestinationClass = aDestType.getTypeClass();

    Any aRet;

    // convert to...
    switch (aDestinationClass)
    {
    // --- to VOID ------------------------------------------------------------------------------
    case TypeClass_VOID:
        return Any();
    // --- to ANY -------------------------------------------------------------------------------
    case TypeClass_ANY:
        return rVal;

    // --- to STRUCT, UNION, EXCEPTION ----------------------------------------------------------
    case TypeClass_STRUCT:
//      case TypeClass_UNION: // xxx todo
    case TypeClass_EXCEPTION:
    {
        // same types or destination type is derived source type?
        TypeDescription aSourceTD( aSourceType );
        TypeDescription aDestTD( aDestType );
        if (typelib_typedescription_isAssignableFrom( aDestTD.get(), aSourceTD.get() ))
        {
            aRet.setValue( rVal.getValue(), aDestTD.get() ); // evtl. .uP.cAsT.
        }
        else
        {
            throw CannotConvertException(
                OUString("value is not of same or derived type!"),
                Reference< XInterface >(), aDestinationClass,
                FailReason::SOURCE_IS_NO_DERIVED_TYPE, 0 );
        }
        break;
    }
    // --- to INTERFACE -------------------------------------------------------------------------
    case TypeClass_INTERFACE:
    {
        if (! rVal.hasValue())
        {
            // void -> interface (null)
            void * null_ref = 0;
            aRet.setValue( &null_ref, aDestType );
            break;
        }

        if (rVal.getValueTypeClass() != TypeClass_INTERFACE ||
            !*(XInterface * const *)rVal.getValue())
        {
            throw CannotConvertException(
                OUString("value is no interface!"),
                Reference< XInterface >(), aDestinationClass, FailReason::NO_SUCH_INTERFACE, 0 );
        }
        if (! (aRet = (*(XInterface * const *)rVal.getValue())->queryInterface(
                   aDestType )).hasValue())
        {
            throw CannotConvertException(
                OUString("value has no such interface!"),
                Reference< XInterface >(), aDestinationClass, FailReason::NO_SUCH_INTERFACE, 0 );
        }
        break;
    }
    // --- to SEQUENCE --------------------------------------------------------------------------
    case TypeClass_SEQUENCE:
    {
        if (aSourceClass==TypeClass_SEQUENCE)
        {
            if( aSourceType == aDestType )
                return rVal;

            TypeDescription aSourceTD( aSourceType );
            TypeDescription aDestTD( aDestType );
            typelib_TypeDescription * pSourceElementTD = 0;
            TYPELIB_DANGER_GET(
                &pSourceElementTD,
                ((typelib_IndirectTypeDescription *)aSourceTD.get())->pType );
            typelib_TypeDescription * pDestElementTD = 0;
            TYPELIB_DANGER_GET(
                &pDestElementTD,
                ((typelib_IndirectTypeDescription *)aDestTD.get())->pType );

            sal_uInt32 nPos = (*(const uno_Sequence * const *)rVal.getValue())->nElements;
            uno_Sequence * pRet = 0;
            uno_sequence_construct(
                &pRet, aDestTD.get(), 0, nPos,
                reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
            aRet.setValue( &pRet, aDestTD.get() );
            uno_destructData(
                &pRet, aDestTD.get(),
                reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
                // decr ref count

            char * pDestElements = (*(uno_Sequence * const *)aRet.getValue())->elements;
            const char * pSourceElements =
                (*(const uno_Sequence * const *)rVal.getValue())->elements;

            while (nPos--)
            {
                char * pDestPos = pDestElements + (nPos * pDestElementTD->nSize);
                const char * pSourcePos = pSourceElements + (nPos * pSourceElementTD->nSize);

                Any aElement(
                    convertTo( Any( pSourcePos, pSourceElementTD ), pDestElementTD->pWeakRef ) );

                if (!uno_assignData(
                        pDestPos, pDestElementTD,
                        (pDestElementTD->eTypeClass == typelib_TypeClass_ANY
                         ? &aElement
                         : const_cast< void * >( aElement.getValue() )),
                        pDestElementTD,
                        reinterpret_cast< uno_QueryInterfaceFunc >(
                            cpp_queryInterface),
                        reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                        reinterpret_cast< uno_ReleaseFunc >(cpp_release) ))
                {
                    OSL_ASSERT( false );
                }
            }
            TYPELIB_DANGER_RELEASE( pDestElementTD );
            TYPELIB_DANGER_RELEASE( pSourceElementTD );
        }
        break;
    }
    // --- to ENUM ------------------------------------------------------------------------------
    case TypeClass_ENUM:
    {
        TypeDescription aEnumTD( aDestType );
        aEnumTD.makeComplete();
        sal_Int32 nPos = -1;

        if (aSourceClass==TypeClass_STRING)
        {
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (((const OUString *)rVal.getValue())->equalsIgnoreAsciiCase(
                        ((typelib_EnumTypeDescription *)aEnumTD.get())->ppEnumNames[nPos] ))
                    break;
            }
        }
        else if (aSourceClass!=TypeClass_ENUM && // exclude some unwanted types for toHyper()
                 aSourceClass!=TypeClass_BOOLEAN &&
                 aSourceClass!=TypeClass_CHAR)
        {
            sal_Int32 nEnumValue = (sal_Int32)toHyper( rVal, -(sal_Int64)0x80000000, 0x7fffffff );
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (nEnumValue == ((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos])
                    break;
            }
        }

        if (nPos >= 0)
        {
            aRet.setValue(
                &((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos],
                aEnumTD.get() );
        }
        else
        {
            throw CannotConvertException(
                OUString("value cannot be converted to demanded ENUM!"),
                Reference< XInterface >(), aDestinationClass, FailReason::IS_NOT_ENUM, 0 );
        }
        break;
    }

    default:
        // else simple type conversion possible?
        try
        {
            aRet = convertToSimpleType( rVal, aDestinationClass );
        }
        catch (IllegalArgumentException &)
        {
            // ...FailReason::INVALID is thrown
        }
    }

    if (aRet.hasValue())
        return aRet;

    throw CannotConvertException(
        OUString("conversion not possible!"),
        Reference< XInterface >(), aDestinationClass, FailReason::INVALID, 0 );
}

//--------------------------------------------------------------------------------------------------
Any TypeConverter_Impl::convertToSimpleType( const Any& rVal, TypeClass aDestinationClass )
    throw( IllegalArgumentException, CannotConvertException, RuntimeException )
{
    switch (aDestinationClass)
    {
        // only simple Conversion of _simple_ types
    case TypeClass_INTERFACE:
    case TypeClass_SERVICE:
    case TypeClass_STRUCT:
    case TypeClass_TYPEDEF:
    case TypeClass_UNION:
    case TypeClass_EXCEPTION:
    case TypeClass_ARRAY:
    case TypeClass_SEQUENCE:
    case TypeClass_ENUM:
    case TypeClass_UNKNOWN:
    case TypeClass_MODULE:
        throw IllegalArgumentException(
            OUString("destination type is not simple!"),
            Reference< XInterface >(), (sal_Int16) 1 );
    default:
        break;
    }

    Type aSourceType = rVal.getValueType();
    TypeClass aSourceClass = aSourceType.getTypeClass();
    if (aDestinationClass == aSourceClass)
        return rVal;

    Any aRet;

    // Convert to...
    switch (aDestinationClass)
    {
    // --- to VOID ------------------------------------------------------------------------------
    case TypeClass_VOID:
        return Any();

    // --- to ANY -------------------------------------------------------------------------------
    case TypeClass_ANY:
        return rVal;

    // --- to BOOL ------------------------------------------------------------------------------
    case TypeClass_BOOLEAN:
        switch (aSourceClass)
        {
        default:
        {
            sal_Bool bTmp = (toDouble( rVal ) != 0.0);
            aRet.setValue( &bTmp, getBooleanCppuType() );
        }
        case TypeClass_ENUM:  // exclude enums
            break;

        case TypeClass_STRING:
        {
            const OUString & aStr = *(const OUString *)rVal.getValue();
            if ( aStr == "0" || aStr.equalsIgnoreAsciiCase( OUString("false") ))
            {
                sal_Bool bFalse = sal_False;
                aRet.setValue( &bFalse, getCppuBooleanType() );
            }
            else if ( aStr == "1" || aStr.equalsIgnoreAsciiCase( OUString("true") ))
            {
                sal_Bool bTrue = sal_True;
                aRet.setValue( &bTrue, getCppuBooleanType() );
            }
            else
            {
                throw CannotConvertException(
                    OUString("STRING has no boolean value!"),
                    Reference< XInterface >(), aDestinationClass, FailReason::IS_NOT_BOOL, 0 );
            }
        }
        }
        break;

    // --- to CHAR, BYTE ------------------------------------------------------------------------
    case TypeClass_CHAR:
    {
        if (aSourceClass==TypeClass_STRING)
        {
            if ((*(const OUString *)rVal.getValue()).getLength() == 1)      // single char
                aRet.setValue( (*(const OUString *)rVal.getValue()).getStr(), ::getCharCppuType() );
        }
        else if (aSourceClass!=TypeClass_ENUM &&        // exclude enums, chars
                 aSourceClass!=TypeClass_CHAR)
        {
             sal_Unicode cRet = (sal_Unicode)toHyper( rVal, 0, 0xffff );    // range
            aRet.setValue( &cRet, ::getCharCppuType() );
        }
        break;
    }
    case TypeClass_BYTE:
        aRet <<= (sal_Int8)( toHyper( rVal, -(sal_Int64)0x80, 0x7f ) );
        break;

    // --- to SHORT, UNSIGNED SHORT -------------------------------------------------------------
    case TypeClass_SHORT:
        aRet <<= (sal_Int16)( toHyper( rVal, -(sal_Int64)0x8000, 0x7fff ) );
        break;
    case TypeClass_UNSIGNED_SHORT:
        aRet <<= (sal_uInt16)( toHyper( rVal, 0, 0xffff ) );
        break;

    // --- to LONG, UNSIGNED LONG ---------------------------------------------------------------
    case TypeClass_LONG:
        aRet <<= (sal_Int32)( toHyper( rVal, -(sal_Int64)0x80000000, 0x7fffffff ) );
        break;
    case TypeClass_UNSIGNED_LONG:
        aRet <<= (sal_uInt32)( toHyper( rVal, 0, 0xffffffff ) );
        break;

    // --- to HYPER, UNSIGNED HYPER--------------------------------------------
    case TypeClass_HYPER:
        aRet <<= toHyper( rVal, SAL_INT64_MIN, SAL_INT64_MAX );
        break;
    case TypeClass_UNSIGNED_HYPER:
        aRet <<= (sal_uInt64)( toHyper( rVal, 0, SAL_UINT64_MAX ) );
        break;

    // --- to FLOAT, DOUBLE ---------------------------------------------------------------------
    case TypeClass_FLOAT:
        aRet <<= (float)( toDouble( rVal, -FLT_MAX, FLT_MAX ) );
        break;
    case TypeClass_DOUBLE:
        aRet <<= (double)( toDouble( rVal, -DBL_MAX, DBL_MAX ) );
        break;

    // --- to STRING ----------------------------------------------------------------------------
    case TypeClass_STRING:
        switch (aSourceClass)
        {
        case TypeClass_ENUM:
        {
            TypeDescription aEnumTD( aSourceType );
            aEnumTD.makeComplete();
            sal_Int32 nPos;
            sal_Int32 nEnumValue = *(sal_Int32 *)rVal.getValue();
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (nEnumValue == ((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos])
                    break;
            }
            if (nPos >= 0)
            {
                aRet.setValue(
                    &((typelib_EnumTypeDescription *)aEnumTD.get())->ppEnumNames[nPos],
                    ::getCppuType( (const OUString *)0 ) );
            }
            else
            {
                throw CannotConvertException(
                    OUString("value is not ENUM!"),
                    Reference< XInterface >(), aDestinationClass, FailReason::IS_NOT_ENUM, 0 );
            }
            break;
        }

        case TypeClass_BOOLEAN:
            aRet <<= (*(sal_Bool *)rVal.getValue()) ?
                OUString("true") :
                OUString("false");
            break;
        case TypeClass_CHAR:
            aRet <<= OUString( (sal_Unicode *)rVal.getValue(), 1 );
            break;

        case TypeClass_BYTE:
            aRet <<= OUString::number( *(sal_Int8 const *)rVal.getValue() );
            break;
        case TypeClass_SHORT:
            aRet <<= OUString::number( *(sal_Int16 const *)rVal.getValue() );
            break;
        case TypeClass_UNSIGNED_SHORT:
            aRet <<= OUString::number( *(sal_uInt16 const *)rVal.getValue() );
            break;
        case TypeClass_LONG:
            aRet <<= OUString::number( *(sal_Int32 const *)rVal.getValue() );
            break;
        case TypeClass_UNSIGNED_LONG:
            aRet <<= OUString::number( *(sal_uInt32 const *)rVal.getValue() );
            break;
        case TypeClass_HYPER:
            aRet <<= OUString::number( *(sal_Int64 const *)rVal.getValue() );
            break;
//      case TypeClass_UNSIGNED_HYPER:
//             aRet <<= OUString::valueOf( (sal_Int64)*(sal_uInt64 const *)rVal.getValue() );
//          break;
            // handle unsigned hyper like double

        default:
            aRet <<= OUString::number( toDouble( rVal ) );
        }
        break;

    default:
        OSL_ASSERT(false);
        break;
    }

    if (aRet.hasValue())
        return aRet;

    throw CannotConvertException(
        OUString("conversion not possible!"),
        Reference< XInterface >(), aDestinationClass, FailReason::INVALID, 0 );
}
}

namespace stoc_services
{
//*************************************************************************
Reference< XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext > & )
    throw( RuntimeException )
{
    static Reference< XInterface > s_ref( (OWeakObject *) new stoc_tcv::TypeConverter_Impl() );
    return s_ref;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
