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

#include <vector>

#include <ooo/vba/XSinkCaller.hpp>
#include <ooo/vba/excel/XApplication.hpp>

#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace ooo::vba { class XSink; }
namespace ooo::vba::excel { class XFileDialog; }

typedef cppu::ImplInheritanceHelper< VbaApplicationBase, ov::excel::XApplication, ov::XSinkCaller > ScVbaApplication_BASE;

struct ScVbaAppSettings;

class ScVbaApplication : public ScVbaApplication_BASE
{
private:
    // note: member variables moved to struct "ScVbaAppSettings", see cxx file, to be shared by all application instances
    ScVbaAppSettings& mrAppSettings;

    // must be stored in order to get result paths from the same instance
    css::uno::Reference< ov::excel::XFileDialog > m_xFileDialog;
    sal_Int32 m_nDialogType;

    /// @throws css::uno::RuntimeException
    OUString getOfficePath( const OUString& sPath );

    std::vector<css::uno::Reference< ooo::vba::XSink >> mvSinks;

protected:
    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() override;

public:
    explicit ScVbaApplication( const css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~ScVbaApplication() override;

    /** Returns true, if VBA document events are enabled. */
    static bool getDocumentEventsEnabled();

    sal_uInt32 AddSink( const css::uno::Reference< ooo::vba::XSink >& xSink );
    void RemoveSink( sal_uInt32 nNumber );

    // XExactName
    virtual OUString SAL_CALL getExactName( const OUString& aApproximateName ) override;

    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess >  SAL_CALL getIntrospection() override;
    virtual css::uno::Any  SAL_CALL invoke(const OUString& FunctionName, const css::uno::Sequence< css::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< css::uno::Any >& OutParam) override;
    virtual void  SAL_CALL setValue(const OUString& PropertyName, const css::uno::Any& Value) override;
    virtual css::uno::Any  SAL_CALL getValue(const OUString& PropertyName) override;
    virtual sal_Bool  SAL_CALL hasMethod(const OUString& Name) override;
    virtual sal_Bool  SAL_CALL hasProperty(const OUString& Name) override;

    // XApplication
    virtual void SAL_CALL setDefaultFilePath( const OUString& DefaultFilePath ) override;
    virtual OUString SAL_CALL getDefaultFilePath() override;
    virtual OUString SAL_CALL getPathSeparator() override;
    virtual OUString SAL_CALL getLibraryPath() override;
    virtual OUString SAL_CALL getTemplatesPath() override;
    virtual OUString SAL_CALL getOperatingSystem() override;

    virtual OUString SAL_CALL getName() override;
    virtual sal_Bool SAL_CALL getDisplayAlerts() override;
    virtual void SAL_CALL setDisplayAlerts( sal_Bool displayAlerts ) override;
    virtual ::sal_Int32 SAL_CALL getCalculation() override;
    virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) override;
    virtual css::uno::Any SAL_CALL getSelection() override;
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getActiveCell() override;
    virtual css::uno::Reference< ov::excel::XWindow > SAL_CALL getActiveWindow() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() override;
    virtual sal_Bool SAL_CALL getDisplayFormulaBar() override;
    virtual void SAL_CALL setDisplayFormulaBar(sal_Bool _displayformulabar) override;

    virtual css::uno::Reference< ov::XAssistant > SAL_CALL getAssistant() override;
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getThisWorkbook() override;
    virtual css::uno::Any SAL_CALL International( sal_Int32 Index ) override;
    virtual css::uno::Any SAL_CALL FileDialog( const css::uno::Any& DialogType ) override;
    virtual css::uno::Any SAL_CALL Workbooks( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL WorksheetFunction( ) override;
    virtual css::uno::Any SAL_CALL Evaluate( const OUString& Name ) override;
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& DialogIndex ) override;
    virtual css::uno::Any SAL_CALL getCutCopyMode() override;
    virtual void SAL_CALL setCutCopyMode( const css::uno::Any& _cutcopymode ) override;
    virtual css::uno::Any SAL_CALL getStatusBar() override;
    virtual void SAL_CALL setStatusBar( const css::uno::Any& _statusbar ) override;
    virtual ::sal_Int32 SAL_CALL getCursor() override;
    virtual void SAL_CALL setCursor( ::sal_Int32 _cursor ) override;
    virtual void SAL_CALL OnKey( const OUString& Key, const css::uno::Any& Procedure ) override;
    virtual void SAL_CALL setScreenUpdating( sal_Bool bUpdate ) override;
    virtual sal_Bool SAL_CALL getEnableEvents() override;
    virtual void SAL_CALL setEnableEvents( sal_Bool bEnable ) override;
    virtual sal_Bool SAL_CALL getEnableCancelKey() override;
    virtual void SAL_CALL setEnableCancelKey( sal_Bool bEnable ) override;

    virtual sal_Bool SAL_CALL getDisplayFullScreen() override;
    virtual void SAL_CALL setDisplayFullScreen( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL getDisplayScrollBars() override;
    virtual void SAL_CALL setDisplayScrollBars( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL getDisplayExcel4Menus() override;
    virtual void SAL_CALL setDisplayExcel4Menus( sal_Bool bSet ) override;

    virtual sal_Bool SAL_CALL getDisplayNoteIndicator() override;
    virtual void SAL_CALL setDisplayNoteIndicator( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL getShowWindowsInTaskbar() override;
    virtual void SAL_CALL setShowWindowsInTaskbar( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL getIteration() override;
    virtual void SAL_CALL setIteration( sal_Bool bSet ) override;

    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL wait( double time ) override;
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) override;
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL GoTo( const css::uno::Any& Reference, const css::uno::Any& Scroll ) override;
    virtual void SAL_CALL Calculate() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) override;
    virtual double SAL_CALL InchesToPoints( double InchesToPoints ) override;
    virtual void SAL_CALL Volatile( const css::uno::Any& Volatile ) override;
    virtual css::uno::Any SAL_CALL MenuBars( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Rows( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Caller( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL Undo() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XInterfaceWithIID
    virtual OUString SAL_CALL getIID() override;

    // XConnectable
    virtual OUString SAL_CALL GetIIDForClassItselfNotCoclass() override;
    virtual ov::TypeAndIID SAL_CALL GetConnectionPoint() override;
    virtual css::uno::Reference<ov::XConnectionPoint> SAL_CALL FindConnectionPoint() override;

    // XSinkCaller
    virtual void SAL_CALL CallSinks( const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
