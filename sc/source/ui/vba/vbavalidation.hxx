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
#ifndef SC_VBA_VALIDATION_HXX
#define SC_VBA_VALIDATION_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/excel/XValidation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1<ov::excel::XValidation > ValidationImpl_BASE;

class ScVbaValidation : public ValidationImpl_BASE
{
    css::uno::Reference< css::table::XCellRange > m_xRange;

public:
    ScVbaValidation( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange ) : ValidationImpl_BASE( xParent, xContext ), m_xRange( xRange) {}
    // Attributes
    virtual ::sal_Bool SAL_CALL getIgnoreBlank() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setIgnoreBlank( ::sal_Bool _ignoreblank ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getInCellDropdown() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInCellDropdown( ::sal_Bool _incelldropdown ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getShowInput() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowInput( ::sal_Bool _showinput ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getShowError() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowError( ::sal_Bool _showerror ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputTitle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInputTitle( const ::rtl::OUString& _inputtitle ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getErrorTitle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setErrorTitle( const ::rtl::OUString& _errortitle ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputMessage() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInputMessage( const ::rtl::OUString& _inputmessage ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getErrorMessage() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setErrorMessage( const ::rtl::OUString& _errormessage ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula1() throw (css::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getFormula2() throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& AlertStyle, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
