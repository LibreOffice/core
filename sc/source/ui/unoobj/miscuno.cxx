/*************************************************************************
 *
 *  $RCSfile: miscuno.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 10:37:49 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>

#include "miscuno.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScEmptyEnumeration, "ScEmptyEnumeration", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScEmptyEnumerationAccess, "ScEmptyEnumerationAccess", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScIndexEnumeration, "ScIndexEnumeration", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScPrintSettingsObj, "ScPrintSettingsObj", "stardiv.unknown" )

SC_SIMPLE_SERVICE_INFO( ScNameToIndexAccess, "ScNameToIndexAccess", "stardiv.unknown" )

//------------------------------------------------------------------------

//  static
uno::Reference<uno::XInterface> ScUnoHelpFunctions::AnyToInterface( const uno::Any& rAny )
{
    if ( rAny.getValueTypeClass() == uno::TypeClass_INTERFACE )
    {
        uno::Reference<uno::XInterface> xInterface;
        rAny >>= xInterface;
        return xInterface;
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
            uno::Any aAny = xProp->getPropertyValue( rName );
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
            uno::Any aAny = xProp->getPropertyValue( rName );
            //! type conversion???
            aAny >>= nRet;
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
            uno::Any aAny = xProp->getPropertyValue( rName );

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

//  static
sal_Bool ScUnoHelpFunctions::GetBoolFromAny( const uno::Any& aAny )
{
    if ( aAny.getValueTypeClass() == uno::TypeClass_BOOLEAN )
        return *(sal_Bool*)aAny.getValue();
    return FALSE;
}

//  static
sal_Int16 ScUnoHelpFunctions::GetInt16FromAny( const uno::Any& aAny )
{
    sal_Int16 nRet;
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

//------------------------------------------------------------------------

ScIndexEnumeration::ScIndexEnumeration(const uno::Reference<container::XIndexAccess>& rInd) :
    xIndex( rInd ),
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
    return xIndex->getByIndex(nPos++);
}

//------------------------------------------------------------------------

ScEmptyEnumerationAccess::ScEmptyEnumerationAccess()
{
}

ScEmptyEnumerationAccess::~ScEmptyEnumerationAccess()
{
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScEmptyEnumerationAccess::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScEmptyEnumeration;
}

uno::Type SAL_CALL ScEmptyEnumerationAccess::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<uno::XInterface>*)0);    // or what?
}

sal_Bool SAL_CALL ScEmptyEnumerationAccess::hasElements() throw(uno::RuntimeException)
{
    return FALSE;
}

//------------------------------------------------------------------------

ScEmptyEnumeration::ScEmptyEnumeration()
{
}

ScEmptyEnumeration::~ScEmptyEnumeration()
{
}

// XEnumeration

sal_Bool SAL_CALL ScEmptyEnumeration::hasMoreElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return FALSE;
}

uno::Any SAL_CALL ScEmptyEnumeration::nextElement() throw(container::NoSuchElementException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return uno::Any();
}

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
    return uno::Any();
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

ScPrintSettingsObj::ScPrintSettingsObj()
{
}

ScPrintSettingsObj::~ScPrintSettingsObj()
{
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScPrintSettingsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    return NULL;
}

void SAL_CALL ScPrintSettingsObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //! later...
}

uno::Any SAL_CALL ScPrintSettingsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //! later...
    return uno::Any();
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScPrintSettingsObj )


//------------------------------------------------------------------------



