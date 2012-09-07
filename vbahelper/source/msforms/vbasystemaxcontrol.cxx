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
uno::Any SAL_CALL VbaSystemAXControl::invoke( const OUString& aFunctionName, const uno::Sequence< uno::Any >& aParams, uno::Sequence< ::sal_Int16 >& aOutParamIndex, uno::Sequence< uno::Any >& aOutParam )
    throw ( lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException )
{
    return m_xControlInvocation->invoke( aFunctionName, aParams, aOutParamIndex, aOutParam );
}

//----------------------------------------------------------
void SAL_CALL VbaSystemAXControl::setValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw ( beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException )
{
    m_xControlInvocation->setValue( aPropertyName, aValue );
}

//----------------------------------------------------------
uno::Any SAL_CALL VbaSystemAXControl::getValue( const OUString& aPropertyName )
    throw ( beans::UnknownPropertyException, uno::RuntimeException )
{
    return m_xControlInvocation->getValue( aPropertyName );
}

//----------------------------------------------------------
::sal_Bool SAL_CALL VbaSystemAXControl::hasMethod( const OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xControlInvocation->hasMethod( aName );
}

//----------------------------------------------------------
::sal_Bool SAL_CALL VbaSystemAXControl::hasProperty( const OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xControlInvocation->hasProperty( aName );
}

//----------------------------------------------------------
OUString
VbaSystemAXControl::getServiceImplName()
{
    return OUString( "VbaSystemAXControl" );
}

//----------------------------------------------------------
uno::Sequence< OUString >
VbaSystemAXControl::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.Frame";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
