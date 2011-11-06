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


#ifndef VBA_SYSTEMAXCONTROL_HXX
#define VBA_SYSTEMAXCONTROL_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/script/XInvocation.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, css::script::XInvocation > SystemAXControlImpl_BASE;

class VbaSystemAXControl : public SystemAXControlImpl_BASE
{
    css::uno::Reference< css::script::XInvocation > m_xControlInvocation;

public:
    VbaSystemAXControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper  );

    // XInvocation
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams, ::com::sun::star::uno::Sequence< ::sal_Int16 >& aOutParamIndex, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam ) throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw ( ::com::sun::star::uno::RuntimeException );

    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
