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
 * ( a copy is included in the LICENSE file that accompanied this code ).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "vbasystemaxcontrol.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

//----------------------------------------------------------
VbaSystemAXControl::VbaSystemAXControl(  const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper )
: SystemAXControlImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
, m_xControlInvocation( xControl, uno::UNO_QUERY_THROW )
{
}

//----------------------------------------------------------
uno::Reference< beans::XIntrospectionAccess > SAL_CALL VbaSystemAXControl::getIntrospection()
    throw ( uno::RuntimeException )
{
    return m_xControlInvocation->getIntrospection();
}

//----------------------------------------------------------
uno::Any SAL_CALL VbaSystemAXControl::invoke( const ::rtl::OUString& aFunctionName, const uno::Sequence< uno::Any >& aParams, uno::Sequence< ::sal_Int16 >& aOutParamIndex, uno::Sequence< uno::Any >& aOutParam )
    throw ( lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException )
{
    return m_xControlInvocation->invoke( aFunctionName, aParams, aOutParamIndex, aOutParam );
}

//----------------------------------------------------------
void SAL_CALL VbaSystemAXControl::setValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw ( beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException )
{
    m_xControlInvocation->setValue( aPropertyName, aValue );
}

//----------------------------------------------------------
uno::Any SAL_CALL VbaSystemAXControl::getValue( const ::rtl::OUString& aPropertyName )
    throw ( beans::UnknownPropertyException, uno::RuntimeException )
{
    return m_xControlInvocation->getValue( aPropertyName );
}

//----------------------------------------------------------
::sal_Bool SAL_CALL VbaSystemAXControl::hasMethod( const ::rtl::OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xControlInvocation->hasMethod( aName );
}

//----------------------------------------------------------
::sal_Bool SAL_CALL VbaSystemAXControl::hasProperty( const ::rtl::OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xControlInvocation->hasProperty( aName );
}

//----------------------------------------------------------
rtl::OUString&
VbaSystemAXControl::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM( "VbaSystemAXControl" ) );
    return sImplName;
}

//----------------------------------------------------------
uno::Sequence< rtl::OUString >
VbaSystemAXControl::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.msforms.Frame" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
