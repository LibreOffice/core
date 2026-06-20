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
#pragma once

#include <ooo/vba/excel/XGlobals.hpp>

#include <cppuhelper/implbase.hxx>

#include <vbahelper/vbaglobalbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace ooo::vba::excel { class XApplication; }


typedef ::cppu::ImplInheritanceHelper< VbaGlobalsBase, ov::excel::XGlobals > ScVbaGlobals_BASE;

class ScVbaGlobals : public ScVbaGlobals_BASE
{
    css::uno::Reference< ov::excel::XApplication > mxApplication;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XApplication > const & getApplication();
public:

    ScVbaGlobals( css::uno::Sequence< cpo::uno::Any > const& aArgs,
                 css::uno::Reference< css::uno::XComponentContext >const& rxContext );
    virtual ~ScVbaGlobals() override;

    // XGlobals
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() override;
    virtual css::uno::Reference< ov::excel::XWindow > SAL_CALL getActiveWindow() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() override;
    virtual css::uno::Reference< ov::XAssistant > SAL_CALL getAssistant() override;
    virtual void SAL_CALL Calculate(  ) override;

    virtual cpo::uno::Any SAL_CALL getSelection() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getActiveCell() override;
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getThisWorkbook() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const cpo::uno::Any& RowIndex, const cpo::uno::Any& ColumnIndex ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Columns( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL CommandBars( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Evaluate( const OUString& Name ) override;

    virtual cpo::uno::Any SAL_CALL WorkSheets(const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL WorkBooks(const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL WorksheetFunction(  ) override;
    virtual cpo::uno::Any SAL_CALL Windows( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Sheets( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Range( const cpo::uno::Any& Cell1, const cpo::uno::Any& Cell2 ) override;
    virtual css::uno::Reference< ::ooo::vba::excel::XRange > SAL_CALL Rows( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Names( const cpo::uno::Any& aIndex ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual css::uno::Reference< ov::excel::XApplication > SAL_CALL getExcel() override;
    virtual cpo::uno::Any SAL_CALL getDebug() override;
    virtual cpo::uno::Any SAL_CALL MenuBars( const cpo::uno::Any& aIndex ) override;

    // XMultiServiceFactory
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
