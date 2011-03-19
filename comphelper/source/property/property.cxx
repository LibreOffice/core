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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"


#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <osl/diagnose.h>

#if OSL_DEBUG_LEVEL > 0
    #include <rtl/strbuf.hxx>
    #include <cppuhelper/exc_hlp.hxx>
    #include <osl/thread.h>
    #include <com/sun/star/lang/XServiceInfo.hpp>
    #include <typeinfo>
#endif
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/genfunc.h>

#include <algorithm>
#include <boost/bind.hpp>

//.........................................................................
namespace comphelper
{

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::uno::cpp_acquire;
    using ::com::sun::star::uno::cpp_release;
#if OSL_DEBUG_LEVEL > 0
    using ::com::sun::star::lang::XServiceInfo;
#endif
    using ::com::sun::star::uno::UNO_QUERY;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

//------------------------------------------------------------------
void copyProperties(const Reference<XPropertySet>& _rxSource,
                    const Reference<XPropertySet>& _rxDest)
{
    if (!_rxSource.is() || !_rxDest.is())
    {
        OSL_FAIL("copyProperties: invalid arguments !");
        return;
    }

    Reference< XPropertySetInfo > xSourceProps = _rxSource->getPropertySetInfo();
    Reference< XPropertySetInfo > xDestProps = _rxDest->getPropertySetInfo();

    Sequence< Property > aSourceProps = xSourceProps->getProperties();
    const Property* pSourceProps = aSourceProps.getConstArray();
    Property aDestProp;
    for (sal_Int32 i=0; i<aSourceProps.getLength(); ++i, ++pSourceProps)
    {
        if ( xDestProps->hasPropertyByName(pSourceProps->Name) )
        {
            try
            {
                aDestProp = xDestProps->getPropertyByName(pSourceProps->Name);
                if (0 == (aDestProp.Attributes & PropertyAttribute::READONLY) )
                {
                    const Any aSourceValue = _rxSource->getPropertyValue(pSourceProps->Name);
                    if ( 0 != (aDestProp.Attributes & PropertyAttribute::MAYBEVOID) || aSourceValue.hasValue() )
                        _rxDest->setPropertyValue(pSourceProps->Name, aSourceValue);
                }
            }
            catch (Exception&)
            {
#if OSL_DEBUG_LEVEL > 0
                ::rtl::OStringBuffer aBuffer;
                aBuffer.append( "::comphelper::copyProperties: could not copy property '" );
                aBuffer.append( ::rtl::OString( pSourceProps->Name.getStr(), pSourceProps->Name.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "' to the destination set (a '" );

                Reference< XServiceInfo > xSI( _rxDest, UNO_QUERY );
                if ( xSI.is() )
                {
                    aBuffer.append( ::rtl::OUStringToOString( xSI->getImplementationName(), osl_getThreadTextEncoding() ) );
                }
                else
                {
                    aBuffer.append( typeid( *_rxDest.get() ).name() );
                }
                aBuffer.append( "' implementation).\n" );

                Any aException( ::cppu::getCaughtException() );
                aBuffer.append( "Caught an exception of type '" );
                ::rtl::OUString sExceptionType( aException.getValueTypeName() );
                aBuffer.append( ::rtl::OString( sExceptionType.getStr(), sExceptionType.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "'" );

                Exception aBaseException;
                if ( ( aException >>= aBaseException ) && aBaseException.Message.getLength() )
                {
                    aBuffer.append( ", saying '" );
                    aBuffer.append( ::rtl::OString( aBaseException.Message.getStr(), aBaseException.Message.getLength(), osl_getThreadTextEncoding() ) );
                    aBuffer.append( "'" );
                }
                aBuffer.append( "." );

                OSL_FAIL( aBuffer.getStr() );
#endif
            }
        }
    }
}

//------------------------------------------------------------------
sal_Bool hasProperty(const rtl::OUString& _rName, const Reference<XPropertySet>& _rxSet)
{
    if (_rxSet.is())
    {
        //  XPropertySetInfoRef xInfo(rxSet->getPropertySetInfo());
        return _rxSet->getPropertySetInfo()->hasPropertyByName(_rName);
    }
    return sal_False;
}

//------------------------------------------------------------------
bool findProperty(Property&              o_rProp,
                  Sequence<Property>&    i_seqProps,
                  const ::rtl::OUString& i_rPropName)
{
    const Property* pAry(i_seqProps.getConstArray());
    const sal_Int32 nLen(i_seqProps.getLength());
    const Property* pRes(
        std::find_if(pAry,pAry+nLen,
                     boost::bind(PropertyStringEqualFunctor(),
                                 _1,
                                 boost::cref(i_rPropName))));
    if( pRes == pAry+nLen )
        return false;

    o_rProp = *pRes;
    return true;
}

//------------------------------------------------------------------
void RemoveProperty(Sequence<Property>& _rProps, const rtl::OUString& _rPropName)
{
    sal_Int32 nLen = _rProps.getLength();

    // binaere Suche
    const Property* pProperties = _rProps.getConstArray();
    const Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, _rPropName,PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == _rPropName) )
    {
        OSL_ENSURE(pResult->Name.equals(_rPropName), "::RemoveProperty Properties nicht sortiert");
        removeElementAt(_rProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
void ModifyPropertyAttributes(Sequence<Property>& seqProps, const ::rtl::OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    Property* pProperties = seqProps.getArray();
    Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen,sPropName, PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == sPropName) )
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}

//------------------------------------------------------------------
sal_Bool tryPropertyValue(Any& _rConvertedValue, Any& _rOldValue, const Any& _rValueToSet, const Any& _rCurrentValue, const Type& _rExpectedType)
{
    sal_Bool bModified(sal_False);
    if (_rCurrentValue.getValue() != _rValueToSet.getValue())
    {
        if ( _rValueToSet.hasValue() && ( !_rExpectedType.equals( _rValueToSet.getValueType() ) ) )
        {
            _rConvertedValue = Any( NULL, _rExpectedType.getTypeLibType() );

            if  ( !uno_type_assignData(
                    const_cast< void* >( _rConvertedValue.getValue() ), _rConvertedValue.getValueType().getTypeLibType(),
                    const_cast< void* >( _rValueToSet.getValue() ), _rValueToSet.getValueType().getTypeLibType(),
                    reinterpret_cast< uno_QueryInterfaceFunc >(
                        cpp_queryInterface),
                    reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                    reinterpret_cast< uno_ReleaseFunc >(cpp_release)
                  )
                )
                throw starlang::IllegalArgumentException();
        }
        else
            _rConvertedValue = _rValueToSet;

        if ( _rCurrentValue != _rConvertedValue )
        {
            _rOldValue = _rCurrentValue;
            bModified = sal_True;
        }
    }
    return bModified;
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
