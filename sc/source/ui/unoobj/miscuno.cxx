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

#include <vcl/svapp.hxx>

#include "miscuno.hxx"

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScNameToIndexAccess, "ScNameToIndexAccess", "stardiv.unknown" )

//------------------------------------------------------------------------

uno::Reference<uno::XInterface> ScUnoHelpFunctions::AnyToInterface( const uno::Any& rAny )
{
    if ( rAny.getValueTypeClass() == uno::TypeClass_INTERFACE )
    {
        return uno::Reference<uno::XInterface>(rAny, uno::UNO_QUERY);
    }
    return uno::Reference<uno::XInterface>();   //! Exception?
}

sal_Bool ScUnoHelpFunctions::GetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const OUString& rName, sal_Bool bDefault )
{
    sal_Bool bRet = bDefault;
    if ( xProp.is() )
    {
        try
        {
            uno::Any aAny(xProp->getPropertyValue( rName ));
            //! type conversion???
            //  operator >>= shouldn't be used for bool (?)
            if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
            {
                //! safe way to get bool value from any???
                bRet = *(sal_Bool*)aAny.getValue();
            }
        }
        catch(uno::Exception&)
        {
            // keep default
        }
    }
    return bRet;
}

sal_Int32 ScUnoHelpFunctions::GetLongProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const OUString& rName, long nDefault )
{
    sal_Int32 nRet = nDefault;
    if ( xProp.is() )
    {
        try
        {
            //! type conversion???
            xProp->getPropertyValue( rName ) >>= nRet;
        }
        catch(uno::Exception&)
        {
            // keep default
        }
    }
    return nRet;
}

sal_Int32 ScUnoHelpFunctions::GetEnumProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const OUString& rName, long nDefault )
{
    sal_Int32 nRet = nDefault;
    if ( xProp.is() )
    {
        try
        {
            uno::Any aAny(xProp->getPropertyValue( rName ));

            if ( aAny.getValueTypeClass() == uno::TypeClass_ENUM )
            {
                //! get enum value from any???
                nRet = *(sal_Int32*)aAny.getValue();
            }
            else
            {
                //! type conversion???
                aAny >>= nRet;
            }
        }
        catch(uno::Exception&)
        {
            // keep default
        }
    }
    return nRet;
}

OUString ScUnoHelpFunctions::GetStringProperty(
    const Reference<beans::XPropertySet>& xProp, const OUString& rName, const OUString& rDefault )
{
    OUString aRet = rDefault;
    if (!xProp.is())
        return aRet;

    try
    {
        Any any = xProp->getPropertyValue(rName);
        any >>= aRet;
    }
    catch (const uno::Exception&)
    {
    }

    return aRet;
}

sal_Bool ScUnoHelpFunctions::GetBoolFromAny( const uno::Any& aAny )
{
    if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        return *(sal_Bool*)aAny.getValue();
    return false;
}

sal_Int16 ScUnoHelpFunctions::GetInt16FromAny( const uno::Any& aAny )
{
    sal_Int16 nRet = 0;
    if ( aAny >>= nRet )
        return nRet;
    return 0;
}

sal_Int32 ScUnoHelpFunctions::GetInt32FromAny( const uno::Any& aAny )
{
    sal_Int32 nRet = 0;
    if ( aAny >>= nRet )
        return nRet;
    return 0;
}

sal_Int32 ScUnoHelpFunctions::GetEnumFromAny( const uno::Any& aAny )
{
    sal_Int32 nRet = 0;
    if ( aAny.getValueTypeClass() == uno::TypeClass_ENUM )
        nRet = *(sal_Int32*)aAny.getValue();
    else
        aAny >>= nRet;
    return nRet;
}

void ScUnoHelpFunctions::SetBoolInAny( uno::Any& rAny, sal_Bool bValue )
{
    rAny.setValue( &bValue, getBooleanCppuType() );
}

void ScUnoHelpFunctions::SetOptionalPropertyValue(
    Reference<beans::XPropertySet>& rPropSet, const sal_Char* pPropName, const Any& rVal )
{
    try
    {
        rPropSet->setPropertyValue(OUString::createFromAscii(pPropName), rVal);
    }
    catch (const beans::UnknownPropertyException&)
    {
        // ignored - not supported.
    }
}

//------------------------------------------------------------------------

ScIndexEnumeration::ScIndexEnumeration(const uno::Reference<container::XIndexAccess>& rInd,
                                       const OUString& rServiceName) :
    xIndex( rInd ),
    sServiceName(rServiceName),
    nPos( 0 )
{
}

ScIndexEnumeration::~ScIndexEnumeration()
{
}

// XEnumeration

sal_Bool SAL_CALL ScIndexEnumeration::hasMoreElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( nPos < xIndex->getCount() );
}

uno::Any SAL_CALL ScIndexEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aReturn;
    try
    {
        aReturn = xIndex->getByIndex(nPos++);
    }
    catch (lang::IndexOutOfBoundsException&)
    {
        throw container::NoSuchElementException();
    }
    return aReturn;
}

OUString SAL_CALL ScIndexEnumeration::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString("ScIndexEnumeration");
}

sal_Bool SAL_CALL ScIndexEnumeration::supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return sServiceName == ServiceName;
}

::com::sun::star::uno::Sequence< OUString >
    SAL_CALL ScIndexEnumeration::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    return aRet;
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------

ScNameToIndexAccess::ScNameToIndexAccess( const com::sun::star::uno::Reference<
                                            com::sun::star::container::XNameAccess>& rNameObj ) :
    xNameAccess( rNameObj )
{
    //! test for XIndexAccess interface at rNameObj, use that instead!

    if ( xNameAccess.is() )
        aNames = xNameAccess->getElementNames();
}

ScNameToIndexAccess::~ScNameToIndexAccess()
{
}

// XIndexAccess

sal_Int32 SAL_CALL ScNameToIndexAccess::getCount(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return aNames.getLength();
}

::com::sun::star::uno::Any SAL_CALL ScNameToIndexAccess::getByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException)
{
    if ( xNameAccess.is() && nIndex >= 0 && nIndex < aNames.getLength() )
        return xNameAccess->getByName( aNames.getConstArray()[nIndex] );

    throw lang::IndexOutOfBoundsException();
}

// XElementAccess

::com::sun::star::uno::Type SAL_CALL ScNameToIndexAccess::getElementType(  )
                                throw(::com::sun::star::uno::RuntimeException)
{
    if ( xNameAccess.is() )
        return xNameAccess->getElementType();
    else
        return uno::Type();
}

sal_Bool SAL_CALL ScNameToIndexAccess::hasElements(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getCount() > 0;
}

//------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
