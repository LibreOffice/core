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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAVALIDATION_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAVALIDATION_HXX
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/excel/XValidation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl<ov::excel::XValidation > ValidationImpl_BASE;

class ScVbaValidation : public ValidationImpl_BASE
{
    css::uno::Reference< css::table::XCellRange > m_xRange;

public:
    ScVbaValidation( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange ) : ValidationImpl_BASE( xParent, xContext ), m_xRange( xRange) {}
    // Attributes
    virtual sal_Bool SAL_CALL getIgnoreBlank() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setIgnoreBlank( sal_Bool _ignoreblank ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getInCellDropdown() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setInCellDropdown( sal_Bool _incelldropdown ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getShowInput() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShowInput( sal_Bool _showinput ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getShowError() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShowError( sal_Bool _showerror ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getInputTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setInputTitle( const OUString& _inputtitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getErrorTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setErrorTitle( const OUString& _errortitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getInputMessage() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setInputMessage( const OUString& _inputmessage ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getErrorMessage() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setErrorMessage( const OUString& _errormessage ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getFormula1() throw (css::uno::RuntimeException, std::exception) override ;
    virtual OUString SAL_CALL getFormula2() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) override;
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& AlertStyle, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
