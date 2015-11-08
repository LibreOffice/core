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


namespace comphelper
{

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

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;


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
                OStringBuffer aBuffer;
                aBuffer.append( "::comphelper::copyProperties: could not copy property '" );
                aBuffer.append( OString( pSourceProps->Name.getStr(), pSourceProps->Name.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "' to the destination set (a '" );

                Reference< XServiceInfo > xSI( _rxDest, UNO_QUERY );
                if ( xSI.is() )
                {
                    aBuffer.append( OUStringToOString( xSI->getImplementationName(), osl_getThreadTextEncoding() ) );
                }
                else
                {
                    aBuffer.append( typeid( *_rxDest.get() ).name() );
                }
                aBuffer.append( "' implementation).\n" );

                Any aException( ::cppu::getCaughtException() );
                aBuffer.append( "Caught an exception of type '" );
                OUString sExceptionType( aException.getValueTypeName() );
                aBuffer.append( OString( sExceptionType.getStr(), sExceptionType.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "'" );

                Exception aBaseException;
                if ( ( aException >>= aBaseException ) && !aBaseException.Message.isEmpty() )
                {
                    aBuffer.append( ", saying '" );
                    aBuffer.append( OString( aBaseException.Message.getStr(), aBaseException.Message.getLength(), osl_getThreadTextEncoding() ) );
                    aBuffer.append( "'" );
                }
                aBuffer.append( "." );

                OSL_FAIL( aBuffer.getStr() );
#endif
            }
        }
    }
}


bool hasProperty(const OUString& _rName, const Reference<XPropertySet>& _rxSet)
{
    if (_rxSet.is())
    {
        //  XPropertySetInfoRef xInfo(rxSet->getPropertySetInfo());
        return _rxSet->getPropertySetInfo()->hasPropertyByName(_rName);
    }
    return false;
}


void RemoveProperty(Sequence<Property>& _rProps, const OUString& _rPropName)
{
    sal_Int32 nLen = _rProps.getLength();

    // binaere Suche
    const Property* pProperties = _rProps.getConstArray();
    Property aNameProp(_rPropName, 0, Type(), 0);
    const Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, aNameProp, PropertyCompareByName());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == _rPropName) )
    {
        OSL_ENSURE(pResult->Name.equals(_rPropName), "::RemoveProperty Properties nicht sortiert");
        removeElementAt(_rProps, pResult - pProperties);
    }
}


void ModifyPropertyAttributes(Sequence<Property>& seqProps, const OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    Property* pProperties = seqProps.getArray();
    Property aNameProp(sPropName, 0, Type(), 0);
    Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, aNameProp, PropertyCompareByName());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == sPropName) )
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}


bool tryPropertyValue(Any& _rConvertedValue, Any& _rOldValue, const Any& _rValueToSet, const Any& _rCurrentValue, const Type& _rExpectedType)
{
    bool bModified(false);
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
                throw css::lang::IllegalArgumentException();
        }
        else
            _rConvertedValue = _rValueToSet;

        if ( _rCurrentValue != _rConvertedValue )
        {
            _rOldValue = _rCurrentValue;
            bModified = true;
        }
    }
    return bModified;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
