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
#ifndef SW_VBA_SECTION_HXX
#define SW_VBA_SECTION_HXX

#include <ooo/vba/word/XSection.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSection > SwVbaSection_BASE;

class SwVbaSection : public SwVbaSection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;

public:
    SwVbaSection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaSection();

    // Attributes
    virtual ::sal_Bool SAL_CALL getProtectedForForms() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setProtectedForForms( ::sal_Bool _protectedforforms ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Headers( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Footers( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SECTION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
