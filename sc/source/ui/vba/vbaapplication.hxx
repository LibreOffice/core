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
#ifndef SC_VBA_APPLICATION_HXX
#define SC_VBA_APPLICATION_HXX


#include <ooo/vba/excel/XWorksheetFunction.hpp>
#include <ooo/vba/excel/XApplication.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase1.hxx>

typedef cppu::ImplInheritanceHelper1< VbaApplicationBase, ov::excel::XApplication > ScVbaApplication_BASE;

struct ScVbaAppSettings;

class ScVbaApplication : public ScVbaApplication_BASE
{
private:
    // note: member variables moved to struct "ScVbaAppSettings", see cxx file, to be shared by all application instances
    ScVbaAppSettings& mrAppSettings;

    rtl::OUString getOfficePath( const rtl::OUString& sPath ) throw ( css::uno::RuntimeException );

protected:
    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() throw (css::uno::RuntimeException);

public:
    ScVbaApplication( const css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~ScVbaApplication();

    /** Returns true, if VBA document events are enabled. */
    static bool getDocumentEventsEnabled();

    virtual SfxObjectShell* GetDocShell( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);

    // XExactName
    virtual ::rtl::OUString SAL_CALL getExactName( const ::rtl::OUString& aApproximateName ) throw (css::uno::RuntimeException);

    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL invoke(const rtl::OUString& FunctionName, const css::uno::Sequence< css::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< css::uno::Any >& OutParam) throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void  SAL_CALL setValue(const rtl::OUString& PropertyName, const css::uno::Any& Value) throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL getValue(const rtl::OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasMethod(const rtl::OUString& Name)  throw(css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasProperty(const rtl::OUString& Name)  throw(css::uno::RuntimeException);

    // XApplication
    virtual void SAL_CALL setDefaultFilePath( const ::rtl::OUString& DefaultFilePath ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDefaultFilePath() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPathSeparator() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLibraryPath() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTemplatesPath() throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayAlerts() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayAlerts( sal_Bool displayAlerts ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCalculation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSelection() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getActiveCell() throw ( css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWindow > SAL_CALL getActiveWindow() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayFormulaBar() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setDisplayFormulaBar( ::sal_Bool _displayformulabar ) throw ( css::uno::RuntimeException );

    virtual css::uno::Reference< ov::XAssistant > SAL_CALL getAssistant() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getThisWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Workbooks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL WorksheetFunction( ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Evaluate( const ::rtl::OUString& Name ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& DialogIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getCutCopyMode() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCutCopyMode( const css::uno::Any& _cutcopymode ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStatusBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStatusBar( const css::uno::Any& _statusbar ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCursor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCursor( ::sal_Int32 _cursor ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL OnKey( const ::rtl::OUString& Key, const css::uno::Any& Procedure ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getEnableEvents() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnableEvents( sal_Bool bEnable ) throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getDisplayFullScreen() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayFullScreen( sal_Bool bSet ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayScrollBars() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayScrollBars( sal_Bool bSet ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayExcel4Menus() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayExcel4Menus( sal_Bool bSet ) throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getDisplayNoteIndicator() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayNoteIndicator( sal_Bool bSet ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getShowWindowsInTaskbar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowWindowsInTaskbar( sal_Bool bSet ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL wait( double time ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL GoTo( const css::uno::Any& Reference, const css::uno::Any& Scroll ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Calculate() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Volatile( const css::uno::Any& Volatile ) throw (css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL MenuBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Caller( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL GetOpenFilename( const css::uno::Any& rFileFilter, const css::uno::Any& rFilterIndex, const css::uno::Any& rTitle, const css::uno::Any& rButtonText, const css::uno::Any& rMultiSelect ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL GetSaveAsFilename( const css::uno::Any& rInitialFileName, const css::uno::Any& rFileFilter, const css::uno::Any& rFilterIndex, const css::uno::Any& rTitle, const css::uno::Any& rButtonText ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_APPLICATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
