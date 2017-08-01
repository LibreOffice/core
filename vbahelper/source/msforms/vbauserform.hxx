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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAUSERFORM_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBAUSERFORM_HXX

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/msforms/XUserForm.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbacontrol.hxx"

//typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XUserForm > ScVbaUserForm_BASE;
typedef cppu::ImplInheritanceHelper< ScVbaControl, ov::msforms::XUserForm > ScVbaUserForm_BASE;

class ScVbaUserForm : public ScVbaUserForm_BASE
{
private:
    css::uno::Reference< css::awt::XDialog > m_xDialog;
    bool mbDispose;
    OUString m_sLibName;
protected:
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaUserForm( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~ScVbaUserForm() override;
    static css::uno::Reference< css::awt::XControl > nestedSearch( const OUString& aPropertyName, css::uno::Reference< css::awt::XControlContainer > const & xContainer );
    // XUserForm
    virtual void SAL_CALL RePaint(  ) override;
    virtual void SAL_CALL Show(  ) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual OUString SAL_CALL getCaption() override;
    virtual void SAL_CALL setCaption( const OUString& _caption ) override;
    virtual double SAL_CALL getInnerWidth() override;
    virtual void SAL_CALL setInnerWidth( double fInnerWidth ) override;
    virtual double SAL_CALL getInnerHeight() override;
    virtual void SAL_CALL setInnerHeight( double fInnerHeight ) override;
    virtual void SAL_CALL Hide(  ) override;
    virtual void SAL_CALL UnloadObject(  ) override;
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& index ) override;
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool bVis ) override;
    // XIntrospection
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;
    virtual css::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
