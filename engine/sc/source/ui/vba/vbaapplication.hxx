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

#include <vector>

#include <ooo/vba/XSinkCaller.hpp>
#include <ooo/vba/excel/XApplication.hpp>

#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <docuno.hxx>

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
    virtual ScModelObj* getCurrentDocument() override;

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
    virtual cpo::uno::Any  SAL_CALL invoke(const OUString& FunctionName, const css::uno::Sequence< cpo::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< cpo::uno::Any >& OutParam) override;
    virtual void  SAL_CALL setValue(const OUString& PropertyName, const cpo::uno::Any& Value) override;
    virtual cpo::uno::Any  SAL_CALL getValue(const OUString& PropertyName) override;
    virtual bool  SAL_CALL hasMethod(const OUString& Name) override;
    virtual bool  SAL_CALL hasProperty(const OUString& Name) override;

    // XApplication
    virtual void SAL_CALL setDefaultFilePath( const OUString& DefaultFilePath ) override;
    virtual OUString SAL_CALL getDefaultFilePath() override;
    virtual OUString SAL_CALL getPathSeparator() override;
    virtual OUString SAL_CALL getLibraryPath() override;
    virtual OUString SAL_CALL getTemplatesPath() override;
    virtual OUString SAL_CALL getOperatingSystem() override;

    virtual OUString SAL_CALL getName() override;
    virtual bool SAL_CALL getDisplayAlerts() override;
    virtual void SAL_CALL setDisplayAlerts( bool displayAlerts ) override;
    virtual ::sal_Int32 SAL_CALL getCalculation() override;
    virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) override;
    virtual cpo::uno::Any SAL_CALL getSelection() override;
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getActiveCell() override;
    virtual css::uno::Reference< ov::excel::XWindow > SAL_CALL getActiveWindow() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() override;
    virtual bool SAL_CALL getDisplayFormulaBar() override;
    virtual void SAL_CALL setDisplayFormulaBar(bool _displayformulabar) override;

    virtual css::uno::Reference< ov::XAssistant > SAL_CALL getAssistant() override;
    virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getThisWorkbook() override;

    virtual cpo::uno::Any SAL_CALL GetOpenFilename(const cpo::uno::Any& FileFilter, const cpo::uno::Any& FilterIndex, const cpo::uno::Any& Title, const cpo::uno::Any& ButtonText, const cpo::uno::Any& MultiSelect) override;
    virtual cpo::uno::Any SAL_CALL International( sal_Int32 Index ) override;
    virtual cpo::uno::Any SAL_CALL FileDialog( const cpo::uno::Any& DialogType ) override;
    virtual cpo::uno::Any SAL_CALL Workbooks( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Worksheets( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL WorksheetFunction( ) override;
    virtual cpo::uno::Any SAL_CALL Evaluate( const OUString& Name ) override;
    virtual cpo::uno::Any SAL_CALL Dialogs( const cpo::uno::Any& DialogIndex ) override;
    virtual cpo::uno::Any SAL_CALL getCutCopyMode() override;
    virtual void SAL_CALL setCutCopyMode( const cpo::uno::Any& _cutcopymode ) override;
    virtual cpo::uno::Any SAL_CALL getStatusBar() override;
    virtual void SAL_CALL setStatusBar( const cpo::uno::Any& _statusbar ) override;
    virtual cpo::uno::Any SAL_CALL getWindowState() override;
    virtual void SAL_CALL setWindowState(const cpo::uno::Any& rWindowState) override;
    virtual ::sal_Int32 SAL_CALL getCursor() override;
    virtual void SAL_CALL setCursor( ::sal_Int32 _cursor ) override;
    virtual void SAL_CALL OnKey( const OUString& Key, const cpo::uno::Any& Procedure ) override;
    virtual void SAL_CALL setScreenUpdating( bool bUpdate ) override;
    virtual bool SAL_CALL getEnableEvents() override;
    virtual void SAL_CALL setEnableEvents( bool bEnable ) override;
    virtual bool SAL_CALL getEnableCancelKey() override;
    virtual void SAL_CALL setEnableCancelKey( bool bEnable ) override;

    virtual bool SAL_CALL getDisplayFullScreen() override;
    virtual void SAL_CALL setDisplayFullScreen( bool bSet ) override;
    virtual bool SAL_CALL getDisplayScrollBars() override;
    virtual void SAL_CALL setDisplayScrollBars( bool bSet ) override;
    virtual bool SAL_CALL getDisplayExcel4Menus() override;
    virtual void SAL_CALL setDisplayExcel4Menus( bool bSet ) override;

    virtual bool SAL_CALL getDisplayNoteIndicator() override;
    virtual void SAL_CALL setDisplayNoteIndicator( bool bSet ) override;
    virtual bool SAL_CALL getShowWindowsInTaskbar() override;
    virtual void SAL_CALL setShowWindowsInTaskbar( bool bSet ) override;
    virtual bool SAL_CALL getIteration() override;
    virtual void SAL_CALL setIteration( bool bSet ) override;

    virtual cpo::uno::Any SAL_CALL Windows( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL wait( double time ) override;
    virtual cpo::uno::Any SAL_CALL Range( const cpo::uno::Any& Cell1, const cpo::uno::Any& Cell2 ) override;
    virtual cpo::uno::Any SAL_CALL Names( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL GoTo( const cpo::uno::Any& Reference, const cpo::uno::Any& Scroll ) override;
    virtual void SAL_CALL Calculate() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual double SAL_CALL CentimetersToPoints( double Centimeters ) override;
    virtual double SAL_CALL InchesToPoints( double Inches ) override;
    virtual void SAL_CALL Volatile( const cpo::uno::Any& Volatile ) override;
    virtual cpo::uno::Any SAL_CALL MenuBars( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Rows( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Caller( const cpo::uno::Any& aIndex ) override;
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
    virtual void SAL_CALL CallSinks( const OUString& Method, css::uno::Sequence< cpo::uno::Any >& Arguments ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
