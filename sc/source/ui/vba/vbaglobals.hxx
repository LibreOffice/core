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
#ifndef SC_VBA_GLOBALS
#define SC_VBA_GLOBALS

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/excel/XGlobals.hpp>
#include <ooo/vba/excel/XApplication.hpp>

#include <cppuhelper/implbase1.hxx>
#include "excelvbahelper.hxx"

#include <vbahelper/vbaglobalbase.hxx>
    // =============================================================================
    // class ScVbaGlobals
    // =============================================================================

typedef ::cppu::ImplInheritanceHelper1< VbaGlobalsBase, ov::excel::XGlobals > ScVbaGlobals_BASE;

    class ScVbaGlobals : public ScVbaGlobals_BASE
    {
        css::uno::Reference< ov::excel::XApplication > mxApplication;
        virtual css::uno::Reference<
 ov::excel::XApplication > SAL_CALL getApplication()
                        throw (css::uno::RuntimeException);
    public:

        ScVbaGlobals( css::uno::Sequence< css::uno::Any > const& aArgs,
            css::uno::Reference< css::uno::XComponentContext >const& rxContext );
        //ScVbaGlobals(
        //    css::uno::Reference< css::uno::XComponentContext >const& rxContext,  );
        virtual ~ScVbaGlobals();

        // XGlobals
        virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
        virtual css::uno::Reference< ov::excel::XWindow > SAL_CALL getActiveWindow() throw (css::uno::RuntimeException);
        virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::XAssistant > SAL_CALL getAssistant() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Calculate(  ) throw (::com::sun::star::script::BasicErrorException, ::com::sun::star::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getSelection() throw (css::uno::RuntimeException);
        virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getActiveCell() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getThisWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const css::uno::Any& RowIndex, const css::uno::Any& ColumnIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Columns( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Evaluate( const ::rtl::OUString& Name ) throw (css::uno::RuntimeException);

        virtual css::uno::Any SAL_CALL WorkSheets(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL WorkBooks(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL WorksheetFunction(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XRange > SAL_CALL Rows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XApplication > SAL_CALL getExcel() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getDebug() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL MenuBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);


    // XMultiServiceFactory
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    };
#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
