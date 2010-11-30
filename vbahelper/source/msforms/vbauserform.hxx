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
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_VBA_USERFORM_HXX
#define SC_VBA_USERFORM_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XUserForm.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
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
    rtl::OUString m_sLibName;
protected:
public:
    ScVbaUserForm( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaUserForm();
    static css::uno::Reference< css::awt::XControl > nestedSearch( const rtl::OUString& aPropertyName, css::uno::Reference< css::awt::XControlContainer >& xContainer );
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    // XUserForm
    virtual void SAL_CALL RePaint(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Show(  ) throw (css::uno::RuntimeException);
    // XIntrospection
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCaption() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL Hide(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL UnloadObject(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
