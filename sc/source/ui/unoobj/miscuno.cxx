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
#include "precompiled_sc.hxx"



#include <tools/debug.hxx>

#include "miscuno.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

//------------------------------------------------------------------------

//UNUSED2008-05  SC_SIMPLE_SERVICE_INFO( ScEmptyEnumeration, "ScEmptyEnumeration", "stardiv.unknown" )
//UNUSED2008-05  SC_SIMPLE_SERVICE_INFO( ScEmptyEnumerationAccess, "ScEmptyEnumerationAccess", "stardiv.unknown" )
//UNUSED2008-05  SC_SIMPLE_SERVICE_INFO( ScIndexEnumeration, "ScIndexEnumeration", "stardiv.unknown" )
//UNUSED2008-05  SC_SIMPLE_SERVICE_INFO( ScPrintSettingsObj, "ScPrintSettingsObj", "stardiv.unknown" )

SC_SIMPLE_SERVICE_INFO( ScNameToIndexAccess, "ScNameToIndexAccess", "stardiv.unknown" )

//------------------------------------------------------------------------

//  static
uno::Reference<uno::XInterface> ScUnoHelpFunctions::AnyToInterface( const uno::Any& rAny )
{
    if ( rAny.getValueTypeClass() == uno::TypeClass_INTERFACE )
    {
        return uno::Reference<uno::XInterface>(rAny, uno::UNO_QUERY);
    }
    return uno::Reference<uno::XInterface>();   //! Exception?
}

//  static
sal_Bool ScUnoHelpFunctions::GetBoolProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const rtl::OUString& rName, sal_Bool bDefault )
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

//  static
sal_Int32 ScUnoHelpFunctions::GetLongProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const rtl::OUString& rName, long nDefault )
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

//  static
sal_Int32 ScUnoHelpFunctions::GetEnumProperty( const uno::Reference<beans::XPropertySet>& xProp,
                                            const rtl::OUString& rName, long nDefault )
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

// static
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

//  static
sal_Bool ScUnoHelpFunctions::GetBoolFromAny( const uno::Any& aAny )
{
    if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        return *(sal_Bool*)aAny.getValue();
    return sal_False;
}

//  static
sal_Int16 ScUnoHelpFunctions::GetInt16FromAny( const uno::Any& aAny )
{
    sal_Int16 nRet = 0;
    if ( aAny >>= nRet )
        return nRet;
    return 0;
}

//  static
sal_Int32 ScUnoHelpFunctions::GetInt32FromAny( const uno::Any& aAny )
{
    sal_Int32 nRet = 0;
    if ( aAny >>= nRet )
        return nRet;
    return 0;
}

//  static
sal_Int32 ScUnoHelpFunctions::GetEnumFromAny( const uno::Any& aAny )
{
    sal_Int32 nRet = 0;
    if ( aAny.getValueTypeClass() == uno::TypeClass_ENUM )
        nRet = *(sal_Int32*)aAny.getValue();
    else
        aAny >>= nRet;
    return nRet;
}

//  static
void ScUnoHelpFunctions::SetBoolInAny( uno::Any& rAny, sal_Bool bValue )
{
    rAny.setValue( &bValue, getBooleanCppuType() );
}

//  static
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
                                       const rtl::OUString& rServiceName) :
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
    ScUnoGuard aGuard;
    return ( nPos < xIndex->getCount() );
}

uno::Any SAL_CALL ScIndexEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
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

::rtl::OUString SAL_CALL ScIndexEnumeration::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii("ScIndexEnumeration");
}

sal_Bool SAL_CALL ScIndexEnumeration::supportsService( const ::rtl::OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return sServiceName == ServiceName;
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
    SAL_CALL ScIndexEnumeration::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    return aRet;
}

//------------------------------------------------------------------------

//UNUSED2008-05  ScEmptyEnumerationAccess::ScEmptyEnumerationAccess()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  ScEmptyEnumerationAccess::~ScEmptyEnumerationAccess()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  // XEnumerationAccess
//UNUSED2008-05
//UNUSED2008-05  uno::Reference<container::XEnumeration> SAL_CALL ScEmptyEnumerationAccess::createEnumeration()
//UNUSED2008-05                                                      throw(uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      ScUnoGuard aGuard;
//UNUSED2008-05      return new ScEmptyEnumeration;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  uno::Type SAL_CALL ScEmptyEnumerationAccess::getElementType() throw(uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      ScUnoGuard aGuard;
//UNUSED2008-05      return getCppuType((uno::Reference<uno::XInterface>*)0);    // or what?
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  sal_Bool SAL_CALL ScEmptyEnumerationAccess::hasElements() throw(uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }

//------------------------------------------------------------------------

//UNUSED2008-05  ScEmptyEnumeration::ScEmptyEnumeration()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  ScEmptyEnumeration::~ScEmptyEnumeration()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  // XEnumeration
//UNUSED2008-05
//UNUSED2008-05  sal_Bool SAL_CALL ScEmptyEnumeration::hasMoreElements() throw(uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      ScUnoGuard aGuard;
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  uno::Any SAL_CALL ScEmptyEnumeration::nextElement() throw(container::NoSuchElementException,
//UNUSED2008-05                                          lang::WrappedTargetException, uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      ScUnoGuard aGuard;
//UNUSED2008-05      return uno::Any();
//UNUSED2008-05  }

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
//    return uno::Any();
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

//UNUSED2008-05  ScPrintSettingsObj::ScPrintSettingsObj()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  ScPrintSettingsObj::~ScPrintSettingsObj()
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  // XPropertySet
//UNUSED2008-05
//UNUSED2008-05  uno::Reference<beans::XPropertySetInfo> SAL_CALL ScPrintSettingsObj::getPropertySetInfo()
//UNUSED2008-05                                                          throw(uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      return NULL;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  void SAL_CALL ScPrintSettingsObj::setPropertyValue(
//UNUSED2008-05                          const rtl::OUString& /* aPropertyName */, const uno::Any& /* aValue */ )
//UNUSED2008-05                  throw(beans::UnknownPropertyException, beans::PropertyVetoException,
//UNUSED2008-05                          lang::IllegalArgumentException, lang::WrappedTargetException,
//UNUSED2008-05                          uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      //! later...
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  uno::Any SAL_CALL ScPrintSettingsObj::getPropertyValue( const rtl::OUString& /* aPropertyName */ )
//UNUSED2008-05                  throw(beans::UnknownPropertyException, lang::WrappedTargetException,
//UNUSED2008-05                          uno::RuntimeException)
//UNUSED2008-05  {
//UNUSED2008-05      //! later...
//UNUSED2008-05      return uno::Any();
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  SC_IMPL_DUMMY_PROPERTY_LISTENER( ScPrintSettingsObj )


//------------------------------------------------------------------------



