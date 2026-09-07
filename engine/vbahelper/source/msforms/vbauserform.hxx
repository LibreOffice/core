/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    /// @throws cpo::uno::RuntimeException
    ScVbaUserForm( cpo::uno::Sequence< cpo::uno::Any > const& aArgs, css::uno::Reference< cpo::uno::XComponentContext >const& xContext );
    virtual ~ScVbaUserForm() override;
    static css::uno::Reference< css::awt::XControl > nestedSearch( const OUString& aPropertyName, css::uno::Reference< css::awt::XControlContainer > const & xContainer );
    // XUserForm
    virtual void RePaint(  ) override;
    virtual void Show(  ) override;
    virtual void setValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getValue( const OUString& aPropertyName ) override;
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& _caption ) override;
    virtual double getInnerWidth() override;
    virtual void setInnerWidth( double fInnerWidth ) override;
    virtual double getInnerHeight() override;
    virtual void setInnerHeight( double fInnerHeight ) override;
    virtual void Hide(  ) override;
    virtual void UnloadObject(  ) override;
    virtual cpo::uno::Any Controls( const cpo::uno::Any& index ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool bVis ) override;
    // XIntrospection
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > getIntrospection(  ) override;
    virtual cpo::uno::Any invoke( const OUString& aFunctionName, const cpo::uno::Sequence< cpo::uno::Any >& aParams, cpo::uno::Sequence< ::sal_Int16 >& aOutParamIndex, cpo::uno::Sequence< cpo::uno::Any >& aOutParam ) override;
    virtual bool hasMethod( const OUString& aName ) override;
    virtual bool hasProperty( const OUString& aName ) override;
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
