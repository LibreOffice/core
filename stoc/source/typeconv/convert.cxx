/*************************************************************************
 *
 *  $RCSfile: convert.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:35 $
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

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
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
using namespace rtl;

#define SERVICENAME "com.sun.star.script.Converter"
#define IMPLNAME    "com.sun.star.comp.stoc.TypeConverter"

namespace stoc_tcv
{

const double MIN_DOUBLE     = -DBL_MAX;
const double MAX_DOUBLE     = DBL_MAX;
const double MIN_FLOAT      = -FLT_MAX;
const double MAX_FLOAT      = FLT_MAX;

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
            sal_Int64 nRet = aHexRest.toInt64( 16 );

            if (nRet == 0)
            {
                for ( sal_Int32 nPos = aHexRest.getLength(); nPos--; )
                {
                    if (aHexRest[nPos] != '0')
                        return sal_False;
                }
            }

            rfVal = (bNeg ? -nRet : nRet);
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
class TypeConverter_Impl : public WeakImplHelper2< XTypeConverter, XServiceInfo >
{
    // ...misc helpers...
    sal_Int64 toHyper( const Any& rAny, sal_Int64 min = 0x8000000000000000, sal_uInt64 max = 0xffffffffffffffff )
        throw( CannotConvertException );
    double toDouble( const Any& rAny, double min = MIN_DOUBLE, double max = MAX_DOUBLE ) const
        throw( CannotConvertException );

public:
    static OUString getImplementationName_Static(void);
    static Sequence< OUString > getSupportedServiceNames_Static(void) throw( RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( RuntimeException );
    virtual  Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( RuntimeException );

    // XTypeConverter
    virtual Any SAL_CALL convertTo( const Any& aFrom, const Type& DestinationType ) throw( IllegalArgumentException, CannotConvertException, RuntimeException);
    virtual Any SAL_CALL convertToSimpleType( const Any& aFrom, TypeClass aDestinationType ) throw( IllegalArgumentException, CannotConvertException, RuntimeException);
};

// XServiceInfo
OUString TypeConverter_Impl::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

// XServiceInfo Helper
OUString TypeConverter_Impl::getImplementationName_Static(void)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
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
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > TypeConverter_Impl::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );
    return aSNS;
}

//--------------------------------------------------------------------------------------------------
sal_Int64 TypeConverter_Impl::toHyper( const Any& rAny, sal_Int64 min, sal_uInt64 max )
    throw( CannotConvertException )
{
    sal_Int64 nRet;
    TypeClass aDestinationClass = rAny.getValueTypeClass();

    switch (aDestinationClass)
    {
    // ANY
//      case TypeClass_ANY: // any sollte schon vorher entschachtelt sein...
//          break;
    // ENUM
    case TypeClass_ENUM:
        nRet = *(int *)rAny.getValue();
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("UNSIGNED HYPER out of range!") ),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }

    // FLOAT, DOUBLE
    case TypeClass_FLOAT:
    {
        double fVal = round( *(float *)rAny.getValue() );
        // implementationsabh. cast von unsigned nach signed!
#ifdef SAL_W32 // conversion from unsigned __int64 to double not impl
        nRet = (fVal > 0x7fffffffffffffff ? (__int64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= (__int64)(max & 0x7fffffffffffffff))
#else
        nRet = (fVal > 0x7fffffffffffffff ? (sal_uInt64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= max)
#endif
        {
            return nRet;
        }
        throw CannotConvertException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("FLOAT out of range!") ),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }
    case TypeClass_DOUBLE:
    {
        double fVal = round( *(double *)rAny.getValue() );
        // implementationsabh. cast von unsigned nach signed!
#ifdef SAL_W32 // conversion from unsigned __int64 to double not impl
        nRet = (fVal > 0x7fffffffffffffff ? (__int64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= (__int64)(max & 0x7fffffffffffffff))
#else
        nRet = (fVal > 0x7fffffffffffffff ? (sal_uInt64)fVal : (sal_Int64)fVal);
        if (fVal >= min && fVal <= max)
#endif
        {
            return nRet;
        }
        throw CannotConvertException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("DOUBLE out of range!") ),
            Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    }

    // STRING
    case TypeClass_STRING:
    {
        double fVal;
        if (! getNumericValue( fVal, *(OUString *)rAny.getValue() ))
        {
            throw CannotConvertException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid STRING value!") ),
                Reference<XInterface>(), aDestinationClass, FailReason::IS_NOT_NUMBER, 0 );
        }
        else
        {
            // implementationsabh. cast von unsigned nach signed!
            nRet = (fVal > 0x7fffffffffffffff ? (sal_uInt64)fVal : (sal_Int64)fVal);
#ifdef SAL_W32 // conversion from unsigned __int64 to double not impl
            if (fVal >= min && fVal <= (__int64)(max & 0x7fffffffffffffff))
#else
            if (fVal >= min && fVal <= max)
#endif
                return nRet;
            throw CannotConvertException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("STRING value out of range!") ),
                Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
        }
    }

    default:
        throw CannotConvertException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("TYPE is not supported!") ),
            Reference<XInterface>(), aDestinationClass, FailReason::TYPE_NOT_SUPPORTED, 0 );
    }

    if (nRet >= min && (nRet < 0 || (sal_uInt64)nRet <= max))
        return nRet;
    throw CannotConvertException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("VALUE is out of range!") ),
        Reference<XInterface>(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    return 0; // dummy
}

//--------------------------------------------------------------------------------------------------
double TypeConverter_Impl::toDouble( const Any& rAny, double min, double max ) const
    throw( CannotConvertException )
{
    double fRet;
    TypeClass aDestinationClass = rAny.getValueTypeClass();

    switch (aDestinationClass)
    {
    // ANY
//      case TypeClass_ANY: // any sollte schon vorher entschachtelt sein...
//          break;
    // ENUM
    case TypeClass_ENUM:
        fRet = *(int *)rAny.getValue();
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
        fRet = *(sal_Int64 *)rAny.getValue();
        break;
    // UNSIGNED HYPER
    case TypeClass_UNSIGNED_HYPER:
#ifdef SAL_W32
        fRet = *(__int64 *)rAny.getValue();
#else
        fRet = *(sal_uInt64 *)rAny.getValue();
#endif
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
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid STRING value!") ),
                Reference<XInterface>(), aDestinationClass, FailReason::IS_NOT_NUMBER, 0 );
        }
        break;
    }

    default:
        throw CannotConvertException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("TYPE is not supported!") ),
            Reference< XInterface >(), aDestinationClass, FailReason::TYPE_NOT_SUPPORTED, 0 );
    }

    if (fRet >= min && fRet <= max)
        return fRet;
    throw CannotConvertException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("VALUE is out of range!") ),
        Reference< XInterface >(), aDestinationClass, FailReason::OUT_OF_RANGE, 0 );
    return 0.0; // dummy
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
//      case TypeClass_UNION: // todo
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
                OUString( RTL_CONSTASCII_USTRINGPARAM("value is not of same or derived type!") ),
                Reference< XInterface >(), aDestinationClass, FailReason::SOURCE_IS_NO_DERIVED_TYPE, 0 );
        }
        break;
    }
    // --- to INTERFACE -------------------------------------------------------------------------
    case TypeClass_INTERFACE:
    {
        if (rVal.getValueTypeClass() != TypeClass_INTERFACE ||
            !*(XInterface * const *)rVal.getValue())
        {
            throw CannotConvertException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("value is no interface!") ),
                Reference< XInterface >(), aDestinationClass, FailReason::NO_SUCH_INTERFACE, 0 );
        }
        if (! (aRet = (*(XInterface * const *)rVal.getValue())->queryInterface( aDestType )).hasValue())
        {
            throw CannotConvertException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("value has no such interface!") ),
                Reference< XInterface >(), aDestinationClass, FailReason::NO_SUCH_INTERFACE, 0 );
        }
        break;
    }
    // --- to SEQUENCE --------------------------------------------------------------------------
    case TypeClass_SEQUENCE:
    {
        if (aSourceClass==TypeClass_SEQUENCE)
        {
            // wenn beide Sequences vom gleichen Typ sind
            if( aSourceType == aDestType )
                return rVal;

            TypeDescription aSourceTD( aSourceType );
            TypeDescription aDestTD( aDestType );
            typelib_TypeDescription * pSourceElementTD = 0;
            TYPELIB_DANGER_GET( &pSourceElementTD, ((typelib_IndirectTypeDescription *)aSourceTD.get())->pType );
            typelib_TypeDescription * pDestElementTD = 0;
            TYPELIB_DANGER_GET( &pDestElementTD, ((typelib_IndirectTypeDescription *)aDestTD.get())->pType );

            sal_uInt32 nPos = (*(const uno_Sequence * const *)rVal.getValue())->nElements;
            uno_Sequence * pRet = 0;
            uno_sequence_construct( &pRet, aDestTD.get(), 0, nPos, cpp_acquire );
            aRet.setValue( &pRet, aDestTD.get() );
            uno_destructData( &pRet, aDestTD.get(), cpp_release ); // decr ref count

            char * pDestElements = (*(uno_Sequence * const *)aRet.getValue())->elements;
            const char * pSourceElements = (*(const uno_Sequence * const *)rVal.getValue())->elements;

            while (nPos--)
            {
                char * pDestPos = pDestElements + (nPos * pDestElementTD->nSize);
                const char * pSourcePos = pSourceElements + (nPos * pSourceElementTD->nSize);

                Any aElement( convertTo( Any( pSourcePos, pSourceElementTD ), pDestElementTD->pWeakRef ) );

                sal_Bool bSucc = uno_assignData(
                    pDestPos, pDestElementTD,
                    (pDestElementTD->eTypeClass == typelib_TypeClass_ANY
                     ? &aElement : const_cast< void * >( aElement.getValue() )),
                    pDestElementTD, cpp_queryInterface, cpp_acquire, cpp_release );
                OSL_ASSERT( bSucc );
            }
            TYPELIB_DANGER_RELEASE( pDestElementTD );
            TYPELIB_DANGER_RELEASE( pSourceElementTD );
        }
    }
    break;
    // --- to ENUM ------------------------------------------------------------------------------
    case TypeClass_ENUM:
    {
        TypeDescription aEnumTD( aDestType );
        sal_Int32 nPos = -1;

        if (aSourceClass==TypeClass_STRING)
        {
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (((const OUString *)rVal.getValue())->equalsIgnoreCase( ((typelib_EnumTypeDescription *)aEnumTD.get())->ppEnumNames[nPos] ))
                    break;
            }
        }
        else if (aSourceClass!=TypeClass_ENUM && // exclude some unwanted types for toHyper()
                 aSourceClass!=TypeClass_BOOLEAN &&
                 aSourceClass!=TypeClass_CHAR)
        {
            sal_Int32 nEnumValue = toHyper( rVal, 0, 0xffffffff );
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (nEnumValue == ((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos])
                    break;
            }
        }

        if (nPos >= 0)
        {
            aRet.setValue( &((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos], aEnumTD.get() );
        }
        else
        {
            throw CannotConvertException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("value cannot be converted to demanded ENUM!") ),
                Reference< XInterface >(), aDestinationClass, FailReason::IS_NOT_ENUM, 0 );
        }
        break;
    }

    default:
        // else simple type conversion possible?
        try
        {
            aRet = convertToSimpleType( rVal, aDestinationClass );  // CannotConvertException darf durchfliegen
        }
        catch (IllegalArgumentException &)
        {
            // ...FailReason::INVALID fliegt
        }
    }

    if (aRet.getValueTypeClass() != TypeClass_VOID)
        return aRet;

    throw CannotConvertException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("conversion not possible!") ),
        Reference< XInterface >(), aDestinationClass, FailReason::INVALID, 0 );
    return Any(); // dummy
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
        throw IllegalArgumentException();
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
            if (aStr.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("0") ) ||
                aStr.equalsIgnoreCase( OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) ))
            {
                sal_Bool bFalse = sal_False;
                aRet.setValue( &bFalse, getCppuBooleanType() );
            }
            else if (aStr.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("1") ) ||
                     aStr.equalsIgnoreCase( OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) ))
            {
                sal_Bool bTrue = sal_True;
                aRet.setValue( &bTrue, getCppuBooleanType() );
            }
            else
            {
                throw CannotConvertException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("STRING has no boolean value!") ),
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
        aRet <<= toHyper( rVal, -(sal_Int64)0x8000000000000000, 0x7fffffffffffffff );
        break;
    case TypeClass_UNSIGNED_HYPER:
        aRet <<= (sal_uInt64)( toHyper( rVal, 0, 0xffffffffffffffff ) );
        break;

    // --- to FLOAT, DOUBLE ---------------------------------------------------------------------
    case TypeClass_FLOAT:
        aRet <<= (float)( toDouble( rVal, MIN_FLOAT, MAX_FLOAT ) );
        break;
    case TypeClass_DOUBLE:
        aRet <<= (double)( toDouble( rVal, MIN_DOUBLE, MAX_DOUBLE ) );
        break;

    // --- to STRING ----------------------------------------------------------------------------
    case TypeClass_STRING:
        switch (aSourceClass)
        {
        case TypeClass_ENUM:
        {
            TypeDescription aEnumTD( aSourceType );
            sal_Int32 nPos;
            sal_Int32 nEnumValue = *(int *)rVal.getValue();
            for ( nPos = ((typelib_EnumTypeDescription *)aEnumTD.get())->nEnumValues; nPos--; )
            {
                if (nEnumValue == ((typelib_EnumTypeDescription *)aEnumTD.get())->pEnumValues[nPos])
                    break;
            }
            if (nPos >= 0)
            {
                aRet.setValue( &((typelib_EnumTypeDescription *)aEnumTD.get())->ppEnumNames[nPos], ::getCppuType( (const OUString *)0 ) );
            }
            else
            {
                throw CannotConvertException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("value is not ENUM!") ),
                    Reference< XInterface >(), aDestinationClass, FailReason::IS_NOT_ENUM, 0 );
            }
            break;
        }

        case TypeClass_BOOLEAN:
            aRet <<= OUString::createFromAscii( (*(sal_Bool *)rVal.getValue() ? "true" : "false") );
            break;
        case TypeClass_CHAR:
            aRet <<= OUString( (sal_Unicode *)rVal.getValue(), 1 );
            break;

        case TypeClass_LONG:
            {
                sal_Int32 nInt(0);
                rVal >>= nInt;
                aRet <<= OUString::valueOf( nInt );
            }
            break;

        default:
            aRet <<= OUString::valueOf( toDouble( rVal ) );
        }
    }

    if (aRet.getValueTypeClass() != TypeClass_VOID)
        return aRet;

    throw CannotConvertException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("conversion not possible!") ),
        Reference< XInterface >(), aDestinationClass, FailReason::INVALID, 0 );
    return Any(); // dummy
}

//*************************************************************************
Reference< XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
    const Reference< XMultiServiceFactory > & rSMgr )
    throw( RuntimeException )
{
    return (XWeak *)(OWeakObject *)new TypeConverter_Impl();
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                stoc_tcv::TypeConverter_Impl::getSupportedServiceNames_Static();

            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            stoc_tcv::TypeConverter_Impl_CreateInstance,
            stoc_tcv::TypeConverter_Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


