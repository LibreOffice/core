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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWSFUNCTION_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWSFUNCTION_HXX

#include <ooo/vba/excel/XWorksheetFunction.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "vbarange.hxx"

#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XWorksheetFunction > ScVbaWSFunction_BASE;

class ScVbaWSFunction :  public ScVbaWSFunction_BASE
{
public:
    ScVbaWSFunction( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~ScVbaWSFunction(){}

    virtual css::uno::Reference< css::beans::XIntrospectionAccess >  SAL_CALL getIntrospection()  throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any  SAL_CALL invoke(const OUString& FunctionName, const css::uno::Sequence< css::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< css::uno::Any >& OutParam) throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void  SAL_CALL setValue(const OUString& PropertyName, const css::uno::Any& Value) throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any  SAL_CALL getValue(const OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool  SAL_CALL hasMethod(const OUString& Name)  throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool  SAL_CALL hasProperty(const OUString& Name)  throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getExactName( const OUString& aApproximateName ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
