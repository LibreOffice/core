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


#ifndef SC_VBA_USERFORM_HXX
#define SC_VBA_USERFORM_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XUserForm.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbacontrol.hxx"

//typedef InheritedHelperInterfaceImpl1< ov::msforms::XUserForm > ScVbaUserForm_BASE;
typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XUserForm > ScVbaUserForm_BASE;

class ScVbaUserForm : public ScVbaUserForm_BASE
{
private:
    css::uno::Reference< css::awt::XDialog > m_xDialog;
    bool mbDispose;
protected:
public:
    ScVbaUserForm( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaUserForm();
    // XUserForm
    virtual void SAL_CALL RePaint(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Show(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCaption() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getInnerWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInnerWidth( double fInnerWidth ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getInnerHeight() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInnerHeight( double fInnerHeight ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL Hide(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL UnloadObject(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    // XIntrospection
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
