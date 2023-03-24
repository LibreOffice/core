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
#include <osl/diagnose.h>
#include <sal/log.hxx>

#if OSL_DEBUG_LEVEL > 0
    #include <cppuhelper/exc_hlp.hxx>
    #include <com/sun/star/lang/XServiceInfo.hpp>
    #include <typeinfo>
#endif
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/ustrbuf.hxx>
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

    const Sequence< Property > aSourceProps = xSourceProps->getProperties();
    Property aDestProp;
    for (const Property& rSourceProp : aSourceProps)
    {
        if ( xDestProps->hasPropertyByName(rSourceProp.Name) )
        {
            try
            {
                aDestProp = xDestProps->getPropertyByName(rSourceProp.Name);
                if (0 == (aDestProp.Attributes & PropertyAttribute::READONLY) )
                {
                    const Any aSourceValue = _rxSource->getPropertyValue(rSourceProp.Name);
                    if ( 0 != (aDestProp.Attributes & PropertyAttribute::MAYBEVOID) || aSourceValue.hasValue() )
                        _rxDest->setPropertyValue(rSourceProp.Name, aSourceValue);
                }
            }
            catch (Exception&)
            {
#if OSL_DEBUG_LEVEL > 0
                OUStringBuffer aBuffer(
                        "::comphelper::copyProperties: could not copy property '"
                        + rSourceProp.Name
                        + "' to the destination set (a '" );

                Reference< XServiceInfo > xSI( _rxDest, UNO_QUERY );
                if ( xSI.is() )
                {
                    aBuffer.append( xSI->getImplementationName() );
                }
                else
                {
                    aBuffer.appendAscii( typeid( *_rxDest ).name() );
                }
                aBuffer.append( "' implementation).\n" );

                Any aException( ::cppu::getCaughtException() );
                aBuffer.append( "Caught an exception of type '"
                        + aException.getValueTypeName()
                        + "'" );

                Exception aBaseException;
                if ( ( aException >>= aBaseException ) && !aBaseException.Message.isEmpty() )
                {
                    aBuffer.append( ", saying '"
                        + aBaseException.Message
                        + "'" );
                }
                aBuffer.append( "." );

                SAL_WARN( "comphelper", aBuffer.makeStringAndClear() );
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
    // binary search
    Property aNameProp(_rPropName, 0, Type(), 0);
    const Property* pResult = std::lower_bound(std::cbegin(_rProps), std::cend(_rProps), aNameProp, PropertyCompareByName());

    if ( pResult != std::cend(_rProps) && pResult->Name == _rPropName)
    {
        removeElementAt(_rProps, pResult - std::cbegin(_rProps));
    }
}


void ModifyPropertyAttributes(Sequence<Property>& seqProps, const OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    // binary search
    auto [begin, end] = asNonConstRange(seqProps);
    Property aNameProp(sPropName, 0, Type(), 0);
    Property* pResult = std::lower_bound(begin, end, aNameProp, PropertyCompareByName());

    if ( (pResult != end) && (pResult->Name == sPropName) )
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
            _rConvertedValue = Any( nullptr, _rExpectedType.getTypeLibType() );

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
