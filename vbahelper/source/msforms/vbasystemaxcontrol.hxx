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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBASYSTEMAXCONTROL_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBASYSTEMAXCONTROL_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/XInvocation.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper< ScVbaControl, css::script::XInvocation > SystemAXControlImpl_BASE;

class VbaSystemAXControl : public SystemAXControlImpl_BASE
{
    css::uno::Reference< css::script::XInvocation > m_xControlInvocation;

public:
    VbaSystemAXControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper  );

    // XInvocation
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams, ::com::sun::star::uno::Sequence< ::sal_Int16 >& aOutParamIndex, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam ) throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
