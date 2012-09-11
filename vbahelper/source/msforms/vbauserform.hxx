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
    OUString m_sLibName;
protected:
public:
    ScVbaUserForm( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaUserForm();
    static css::uno::Reference< css::awt::XControl > nestedSearch( const OUString& aPropertyName, css::uno::Reference< css::awt::XControlContainer >& xContainer );
    // XUserForm
    virtual void SAL_CALL RePaint(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Show(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual OUString SAL_CALL getCaption() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const OUString& _caption ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getInnerWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInnerWidth( double fInnerWidth ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getInnerHeight() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInnerHeight( double fInnerHeight ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL Hide(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL UnloadObject(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    // XIntrospection
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const OUString& aName ) throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
