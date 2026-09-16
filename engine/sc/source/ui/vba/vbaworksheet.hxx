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

#include <ooo/vba/excel/XWorksheet.hpp>
#include <rtl/ref.hxx>

#include <vbahelper/vbahelperinterface.hxx>
#include <types.hxx>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::sheet { class XSpreadsheet; }
namespace cpo::uno { class XComponentContext; }
namespace ooo::vba::excel { class XRange; }

namespace ooo::vba::excel {
    class XHyperlinks;
}

class ScVbaSheetObjectsBase;
class ScVbaChartObjects;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XWorksheet >  WorksheetImpl_BASE;

class ScVbaWorksheet : public WorksheetImpl_BASE
{
    cpo::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
    cpo::uno::Reference< css::frame::XModel > mxModel;
    rtl::Reference<ScVbaChartObjects> mxCharts;
    cpo::uno::Reference< ov::excel::XHyperlinks > mxHlinks;
    ::rtl::Reference< ScVbaSheetObjectsBase > mxButtons[2];
    bool mbVeryHidden;

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XWorksheet > getSheetAtOffset(SCTAB offset);
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XRange > getSheetRange();

    cpo::uno::Reference< css::container::XNameAccess > getFormControls() const;
    cpo::uno::Any getControlShape( std::u16string_view sName );

    cpo::uno::Any getButtons( const cpo::uno::Any &rIndex, bool bOptionButtons );

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaWorksheet( const cpo::uno::Reference< ov::XHelperInterface >& xParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        cpo::uno::Reference< css::sheet::XSpreadsheet > xSheet,
        cpo::uno::Reference< css::frame::XModel > xModel )  ;
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaWorksheet( cpo::uno::Sequence< cpo::uno::Any > const& aArgs, cpo::uno::Reference< cpo::uno::XComponentContext >const& xContext );

    virtual ~ScVbaWorksheet() override;

    const cpo::uno::Reference< css::frame::XModel >& getModel() const
    { return mxModel; }
    const cpo::uno::Reference< css::sheet::XSpreadsheet >& getSheet() const
    { return mxSheet; }
    static const cpo::uno::Sequence<sal_Int8>& getUnoTunnelId();
    cpo::uno::Reference< ov::excel::XWorksheet > createSheetCopyInNewDoc( const OUString& );
    cpo::uno::Reference< ov::excel::XWorksheet > createSheetCopy(cpo::uno::Reference< ov::excel::XWorksheet> const & xSheet, bool bAfter);

    // Attributes
    virtual OUString getName() override;
    virtual void setName( const OUString &rName ) override;
    virtual sal_Int32 getVisible() override;
    virtual void setVisible( sal_Int32 nVisible ) override;
    virtual ::sal_Int32 getStandardWidth() override;
    virtual ::sal_Int32 getStandardHeight() override;
    virtual bool getProtectionMode() override;
    virtual bool getProtectContents() override;
    virtual bool getProtectDrawingObjects() override;
    virtual bool getProtectScenarios() override;
    virtual cpo::uno::Reference< ov::excel::XRange > getUsedRange() override ;
    virtual cpo::uno::Any ChartObjects( const cpo::uno::Any& Index ) override;
    virtual cpo::uno::Reference< ov::excel::XOutline > Outline( ) override;
    virtual cpo::uno::Reference< ov::excel::XPageSetup > PageSetup( ) override;
    virtual cpo::uno::Any HPageBreaks( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any VPageBreaks( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Reference< ov::excel::XWorksheet > getNext() override;
    virtual cpo::uno::Reference< ov::excel::XWorksheet > getPrevious() override;
     virtual sal_Int16 getIndex() override;
     virtual sal_Int32 getEnableSelection() override;
     virtual void setEnableSelection( sal_Int32 nSelection ) override;
    virtual bool getAutoFilterMode() override;
    virtual void setAutoFilterMode( bool bAutoFilterMode ) override;

    // Methods
    virtual void Activate() override;
    virtual void Select() override;
    virtual cpo::uno::Reference< ov::excel::XRange > Range( const cpo::uno::Any& Cell1, const cpo::uno::Any& Cell2 ) override;
    virtual void Move( const cpo::uno::Any& Before, const cpo::uno::Any& After ) override ;
     virtual void Copy( const cpo::uno::Any& Before, const cpo::uno::Any& After ) override;
    virtual void Paste( const cpo::uno::Any& Destination, const cpo::uno::Any& Link ) override;
    virtual void Delete(  ) override;
    virtual void Protect( const cpo::uno::Any& Password, const cpo::uno::Any& DrawingObjects, const cpo::uno::Any& Contents, const cpo::uno::Any& Scenarios, const cpo::uno::Any& UserInterfaceOnly ) override;
    virtual void Unprotect( const cpo::uno::Any& Password ) override;

    virtual void Calculate(  ) override;
    virtual void CheckSpelling( const cpo::uno::Any& CustomDictionary,const cpo::uno::Any& IgnoreUppercase,const cpo::uno::Any& AlwaysSuggest, const cpo::uno::Any& SpellingLang ) override;
    // Hacks (?)
    virtual cpo::uno::Reference< ov::excel::XRange > Cells( const cpo::uno::Any &nRow, const cpo::uno::Any &nCol ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Rows(const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Columns(const cpo::uno::Any& aIndex ) override;

    virtual cpo::uno::Any Evaluate( const OUString& Name ) override;
    virtual cpo::uno::Any PivotTables( const cpo::uno::Any& Index ) override;
    virtual cpo::uno::Any Comments( const cpo::uno::Any& Index ) override;
    virtual cpo::uno::Any Hyperlinks( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any Names( const cpo::uno::Any& aIndex ) override;

    virtual cpo::uno::Any OLEObjects( const cpo::uno::Any& Index ) override;
    virtual cpo::uno::Any Shapes( const cpo::uno::Any& aIndex ) override;

    virtual cpo::uno::Any Buttons( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any CheckBoxes( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any DropDowns( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any GroupBoxes( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any Labels( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any ListBoxes( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any OptionButtons( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any ScrollBars( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any Spinners( const cpo::uno::Any& rIndex ) override;

    virtual void setEnableCalculation( bool EnableCalculation ) override;
    virtual bool getEnableCalculation(  ) override;
    virtual void ShowDataForm(  ) override;
    // XInvocation
    virtual cpo::uno::Reference< css::beans::XIntrospectionAccess > getIntrospection(  ) override;
    virtual cpo::uno::Any invoke( const OUString& aFunctionName, const cpo::uno::Sequence< cpo::uno::Any >& aParams, cpo::uno::Sequence< ::sal_Int16 >& aOutParamIndex, cpo::uno::Sequence< cpo::uno::Any >& aOutParam ) override;
    virtual void setValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getValue( const OUString& aPropertyName ) override;
    virtual bool hasMethod( const OUString& aName ) override;
    virtual bool hasProperty( const OUString& aName ) override;
    // CodeName
    virtual OUString getCodeName() override;
    /// @throws cpo::uno::RuntimeException
    sal_Int16 getSheetID() const;

    virtual void PrintOut( const cpo::uno::Any& From, const cpo::uno::Any& To, const cpo::uno::Any& Copies, const cpo::uno::Any& Preview, const cpo::uno::Any& ActivePrinter, const cpo::uno::Any& PrintToFile, const cpo::uno::Any& Collate, const cpo::uno::Any& PrToFileName, const cpo::uno::Any& IgnorePrintAreas ) override;
    virtual void ExportAsFixedFormat(const cpo::uno::Any& Type, const cpo::uno::Any& FileName, const cpo::uno::Any& Quality,
        const cpo::uno::Any& IncludeDocProperties, const cpo::uno::Any& IgnorePrintAreas, const cpo::uno::Any& From,
        const cpo::uno::Any& To, const cpo::uno::Any& OpenAfterPublish, const cpo::uno::Any& FixedFormatExtClassPtr) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
    // XUnoTunnel
    virtual ::sal_Int64 getSomething(const cpo::uno::Sequence<sal_Int8 >& rId ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
