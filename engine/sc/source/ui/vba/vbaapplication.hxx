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

namespace cpo::uno { class XComponentContext; }
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
    cpo::uno::Reference< ov::excel::XFileDialog > m_xFileDialog;
    sal_Int32 m_nDialogType;

    /// @throws cpo::uno::RuntimeException
    OUString getOfficePath( const OUString& sPath );

    std::vector<cpo::uno::Reference< ooo::vba::XSink >> mvSinks;

protected:
    virtual ScModelObj* getCurrentDocument() override;

public:
    explicit ScVbaApplication( const cpo::uno::Reference< cpo::uno::XComponentContext >& m_xContext );
    virtual ~ScVbaApplication() override;

    /** Returns true, if VBA document events are enabled. */
    static bool getDocumentEventsEnabled();

    sal_uInt32 AddSink( const cpo::uno::Reference< ooo::vba::XSink >& xSink );
    void RemoveSink( sal_uInt32 nNumber );

    // XExactName
    virtual OUString getExactName( const OUString& aApproximateName ) override;

    // XInvocation
    virtual cpo::uno::Reference< css::beans::XIntrospectionAccess >  getIntrospection() override;
    virtual cpo::uno::Any  invoke(const OUString& FunctionName, const cpo::uno::Sequence< cpo::uno::Any >& Params, cpo::uno::Sequence< sal_Int16 >& OutParamIndex, cpo::uno::Sequence< cpo::uno::Any >& OutParam) override;
    virtual void  setValue(const OUString& PropertyName, const cpo::uno::Any& Value) override;
    virtual cpo::uno::Any  getValue(const OUString& PropertyName) override;
    virtual bool  hasMethod(const OUString& Name) override;
    virtual bool  hasProperty(const OUString& Name) override;

    // XApplication
    virtual void setDefaultFilePath( const OUString& DefaultFilePath ) override;
    virtual OUString getDefaultFilePath() override;
    virtual OUString getPathSeparator() override;
    virtual OUString getLibraryPath() override;
    virtual OUString getTemplatesPath() override;
    virtual OUString getOperatingSystem() override;

    virtual OUString getName() override;
    virtual bool getDisplayAlerts() override;
    virtual void setDisplayAlerts( bool displayAlerts ) override;
    virtual ::sal_Int32 getCalculation() override;
    virtual void setCalculation( ::sal_Int32 _calculation ) override;
    virtual cpo::uno::Any getSelection() override;
    virtual cpo::uno::Reference< ov::excel::XWorkbook > getActiveWorkbook() override;
    virtual cpo::uno::Reference< ov::excel::XRange > getActiveCell() override;
    virtual cpo::uno::Reference< ov::excel::XWindow > getActiveWindow() override;
    virtual cpo::uno::Reference< ov::excel::XWorksheet > getActiveSheet() override;
    virtual bool getDisplayFormulaBar() override;
    virtual void setDisplayFormulaBar(bool _displayformulabar) override;

    virtual cpo::uno::Reference< ov::XAssistant > getAssistant() override;
    virtual cpo::uno::Reference< ov::excel::XWorkbook > getThisWorkbook() override;

    virtual cpo::uno::Any GetOpenFilename(const cpo::uno::Any& FileFilter, const cpo::uno::Any& FilterIndex, const cpo::uno::Any& Title, const cpo::uno::Any& ButtonText, const cpo::uno::Any& MultiSelect) override;
    virtual cpo::uno::Any International( sal_Int32 Index ) override;
    virtual cpo::uno::Any FileDialog( const cpo::uno::Any& DialogType ) override;
    virtual cpo::uno::Any Workbooks( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any Worksheets( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any WorksheetFunction( ) override;
    virtual cpo::uno::Any Evaluate( const OUString& Name ) override;
    virtual cpo::uno::Any Dialogs( const cpo::uno::Any& DialogIndex ) override;
    virtual cpo::uno::Any getCutCopyMode() override;
    virtual void setCutCopyMode( const cpo::uno::Any& _cutcopymode ) override;
    virtual cpo::uno::Any getStatusBar() override;
    virtual void setStatusBar( const cpo::uno::Any& _statusbar ) override;
    virtual cpo::uno::Any getWindowState() override;
    virtual void setWindowState(const cpo::uno::Any& rWindowState) override;
    virtual ::sal_Int32 getCursor() override;
    virtual void setCursor( ::sal_Int32 _cursor ) override;
    virtual void OnKey( const OUString& Key, const cpo::uno::Any& Procedure ) override;
    virtual void setScreenUpdating( bool bUpdate ) override;
    virtual bool getEnableEvents() override;
    virtual void setEnableEvents( bool bEnable ) override;
    virtual bool getEnableCancelKey() override;
    virtual void setEnableCancelKey( bool bEnable ) override;

    virtual bool getDisplayFullScreen() override;
    virtual void setDisplayFullScreen( bool bSet ) override;
    virtual bool getDisplayScrollBars() override;
    virtual void setDisplayScrollBars( bool bSet ) override;
    virtual bool getDisplayExcel4Menus() override;
    virtual void setDisplayExcel4Menus( bool bSet ) override;

    virtual bool getDisplayNoteIndicator() override;
    virtual void setDisplayNoteIndicator( bool bSet ) override;
    virtual bool getShowWindowsInTaskbar() override;
    virtual void setShowWindowsInTaskbar( bool bSet ) override;
    virtual bool getIteration() override;
    virtual void setIteration( bool bSet ) override;

    virtual cpo::uno::Any Windows( const cpo::uno::Any& aIndex ) override;
    virtual void wait( double time ) override;
    virtual cpo::uno::Any Range( const cpo::uno::Any& Cell1, const cpo::uno::Any& Cell2 ) override;
    virtual cpo::uno::Any Names( const cpo::uno::Any& aIndex ) override;
    virtual void GoTo( const cpo::uno::Any& Reference, const cpo::uno::Any& Scroll ) override;
    virtual void Calculate() override;
    virtual cpo::uno::Reference< ov::excel::XRange > Intersect( const cpo::uno::Reference< ov::excel::XRange >& Arg1, const cpo::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Union( const cpo::uno::Reference< ov::excel::XRange >& Arg1, const cpo::uno::Reference< ov::excel::XRange >& Arg2, const cpo::uno::Any& Arg3, const cpo::uno::Any& Arg4, const cpo::uno::Any& Arg5, const cpo::uno::Any& Arg6, const cpo::uno::Any& Arg7, const cpo::uno::Any& Arg8, const cpo::uno::Any& Arg9, const cpo::uno::Any& Arg10, const cpo::uno::Any& Arg11, const cpo::uno::Any& Arg12, const cpo::uno::Any& Arg13, const cpo::uno::Any& Arg14, const cpo::uno::Any& Arg15, const cpo::uno::Any& Arg16, const cpo::uno::Any& Arg17, const cpo::uno::Any& Arg18, const cpo::uno::Any& Arg19, const cpo::uno::Any& Arg20, const cpo::uno::Any& Arg21, const cpo::uno::Any& Arg22, const cpo::uno::Any& Arg23, const cpo::uno::Any& Arg24, const cpo::uno::Any& Arg25, const cpo::uno::Any& Arg26, const cpo::uno::Any& Arg27, const cpo::uno::Any& Arg28, const cpo::uno::Any& Arg29, const cpo::uno::Any& Arg30 ) override;
    virtual double CentimetersToPoints( double Centimeters ) override;
    virtual double InchesToPoints( double Inches ) override;
    virtual void Volatile( const cpo::uno::Any& Volatile ) override;
    virtual cpo::uno::Any MenuBars( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any Rows( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any Caller( const cpo::uno::Any& aIndex ) override;
    virtual void Undo() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;

    // XInterfaceWithIID
    virtual OUString getIID() override;

    // XConnectable
    virtual OUString GetIIDForClassItselfNotCoclass() override;
    virtual ov::TypeAndIID GetConnectionPoint() override;
    virtual cpo::uno::Reference<ov::XConnectionPoint> FindConnectionPoint() override;

    // XSinkCaller
    virtual void CallSinks( const OUString& Method, cpo::uno::Sequence< cpo::uno::Any >& Arguments ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
