/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#ifndef SW_VBA_FORMFIELD_HXX
#define SW_VBA_FORMFIELD_HXX

#include <ooo/vba/word/XFormField.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XFormField.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XFormField > SwVbaFormField_BASE;

class SwVbaFormField : public SwVbaFormField_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XFormField > mxFormField;

public:
    SwVbaFormField( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel, const css::uno::Reference< css::text::XFormField >& xFormField ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaFormField();

    // Methods
    rtl::OUString SAL_CALL getResult() throw ( css::uno::RuntimeException );
    void SAL_CALL setResult( const rtl::OUString& result ) throw ( css::uno::RuntimeException );
    sal_Bool SAL_CALL getEnabled() throw ( css::uno::RuntimeException );
    void SAL_CALL setEnabled( sal_Bool enabled ) throw ( css::uno::RuntimeException );
    css::uno::Any SAL_CALL CheckBox() throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_FORMFIELD_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
