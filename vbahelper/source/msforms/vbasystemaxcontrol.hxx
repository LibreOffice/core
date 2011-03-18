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
 * ( a copy is included in the LICENSE file that accompanied this code ).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
