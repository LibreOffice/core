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
#ifndef SW_VBA_VARIABLE_HXX
#define SW_VBA_VARIABLE_HXX

#include <ooo/vba/word/XVariable.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyAccess.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XVariable > SwVbaVariable_BASE;

class SwVbaVariable : public SwVbaVariable_BASE
{
private:
    css::uno::Reference< css::beans::XPropertyAccess > mxUserDefined;
    rtl::OUString maName;

public:
    SwVbaVariable( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::beans::XPropertyAccess >& rUserDefined, const rtl::OUString& rName ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaVariable();

   // XVariable
    virtual rtl::OUString SAL_CALL getName() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setName( const rtl::OUString& ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getValue() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setValue( const css::uno::Any& rValue ) throw ( css::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getIndex() throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_VARIABLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
