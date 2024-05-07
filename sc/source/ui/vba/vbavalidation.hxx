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
#pragma once

#include <ooo/vba/excel/XValidation.hpp>
#include <utility>
#include <vbahelper/vbahelperinterface.hxx>

namespace com::sun::star::table { class XCellRange; }
namespace com::sun::star::uno { class XComponentContext; }

typedef InheritedHelperInterfaceWeakImpl<ov::excel::XValidation > ValidationImpl_BASE;

class ScVbaValidation : public ValidationImpl_BASE
{
    css::uno::Reference< css::table::XCellRange > m_xRange;

public:
    ScVbaValidation( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::table::XCellRange > xRange ) : ValidationImpl_BASE( xParent, xContext ), m_xRange(std::move( xRange)) {}
    // Attributes
    virtual sal_Bool SAL_CALL getIgnoreBlank() override;
    virtual void SAL_CALL setIgnoreBlank( sal_Bool _ignoreblank ) override;
    virtual sal_Bool SAL_CALL getCaseSensitive() override;
    virtual void SAL_CALL setCaseSensitive( sal_Bool _bCase ) override;
    virtual sal_Bool SAL_CALL getInCellDropdown() override;
    virtual void SAL_CALL setInCellDropdown( sal_Bool _incelldropdown ) override;
    virtual sal_Bool SAL_CALL getShowInput() override;
    virtual void SAL_CALL setShowInput( sal_Bool _showinput ) override;
    virtual sal_Bool SAL_CALL getShowError() override;
    virtual void SAL_CALL setShowError( sal_Bool _showerror ) override;
    virtual OUString SAL_CALL getInputTitle() override;
    virtual void SAL_CALL setInputTitle( const OUString& _inputtitle ) override;
    virtual OUString SAL_CALL getErrorTitle() override;
    virtual void SAL_CALL setErrorTitle( const OUString& _errortitle ) override;
    virtual OUString SAL_CALL getInputMessage() override;
    virtual void SAL_CALL setInputMessage( const OUString& _inputmessage ) override;
    virtual OUString SAL_CALL getErrorMessage() override;
    virtual void SAL_CALL setErrorMessage( const OUString& _errormessage ) override;
    virtual OUString SAL_CALL getFormula1() override ;
    virtual OUString SAL_CALL getFormula2() override;
    virtual sal_Int32 SAL_CALL getType() override;
    // Methods
    virtual void SAL_CALL Delete(  ) override;
    virtual void SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& AlertStyle, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
