/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

