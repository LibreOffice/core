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

#include <ooo/vba/excel/XWorksheet.hpp>
#include <rtl/ref.hxx>

#include <vbahelper/vbahelperinterface.hxx>
#include <types.hxx>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::sheet { class XSpreadsheet; }
namespace com::sun::star::uno { class XComponentContext; }
namespace ooo::vba::excel { class XOutline; }
namespace ooo::vba::excel { class XPageSetup; }
namespace ooo::vba::excel { class XRange; }

namespace ooo::vba::excel {
    class XChartObjects;
    class XHyperlinks;
}

class ScVbaSheetObjectsBase;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XWorksheet >  WorksheetImpl_BASE;

class ScVbaWorksheet : public WorksheetImpl_BASE
{
    css::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< ov::excel::XChartObjects > mxCharts;
    css::uno::Reference< ov::excel::XHyperlinks > mxHlinks;
    ::rtl::Reference< ScVbaSheetObjectsBase > mxButtons[2];
    bool mbVeryHidden;

    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XWorksheet > getSheetAtOffset(SCTAB offset);
    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > getSheetRange();

    css::uno::Reference< css::container::XNameAccess > getFormControls() const;
    css::uno::Any getControlShape( std::u16string_view sName );

    css::uno::Any getButtons( const css::uno::Any &rIndex, bool bOptionButtons );

public:
    /// @throws css::uno::RuntimeException
    ScVbaWorksheet( const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::sheet::XSpreadsheet >& xSheet,
        const css::uno::Reference< css::frame::XModel >& xModel )  ;
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaWorksheet( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    virtual ~ScVbaWorksheet() override;

    const css::uno::Reference< css::frame::XModel >& getModel() const
    { return mxModel; }
    const css::uno::Reference< css::sheet::XSpreadsheet >& getSheet() const
    { return mxSheet; }
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    css::uno::Reference< ov::excel::XWorksheet > createSheetCopyInNewDoc( const OUString& );
    css::uno::Reference< ov::excel::XWorksheet > createSheetCopy(css::uno::Reference< ov::excel::XWorksheet> const & xSheet, bool bAfter);

    // Attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString &rName ) override;
    virtual sal_Int32 SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Int32 nVisible ) override;
    virtual ::sal_Int32 SAL_CALL getStandardWidth() override;
    virtual ::sal_Int32 SAL_CALL getStandardHeight() override;
    virtual sal_Bool SAL_CALL getProtectionMode() override;
    virtual sal_Bool SAL_CALL getProtectContents() override;
    virtual sal_Bool SAL_CALL getProtectDrawingObjects() override;
    virtual sal_Bool SAL_CALL getProtectScenarios() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getUsedRange() override ;
    virtual css::uno::Any SAL_CALL ChartObjects( const css::uno::Any& Index ) override;
    virtual css::uno::Reference< ov::excel::XOutline > SAL_CALL Outline( ) override;
    virtual css::uno::Reference< ov::excel::XPageSetup > SAL_CALL PageSetup( ) override;
    virtual css::uno::Any SAL_CALL HPageBreaks( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL VPageBreaks( const css::uno::Any& aIndex ) override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getNext() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getPrevious() override;
     virtual sal_Int16 SAL_CALL getIndex() override;
     virtual sal_Int32 SAL_CALL getEnableSelection() override;
     virtual void SAL_CALL setEnableSelection( sal_Int32 nSelection ) override;
    virtual sal_Bool SAL_CALL getAutoFilterMode() override;
    virtual void SAL_CALL setAutoFilterMode( sal_Bool bAutoFilterMode ) override;

    // Methods
    virtual void SAL_CALL Activate() override;
    virtual void SAL_CALL Select() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) override;
    virtual void SAL_CALL Move( const css::uno::Any& Before, const css::uno::Any& After ) override ;
     virtual void SAL_CALL Copy( const css::uno::Any& Before, const css::uno::Any& After ) override;
    virtual void SAL_CALL Paste( const css::uno::Any& Destination, const css::uno::Any& Link ) override;
    virtual void SAL_CALL Delete(  ) override;
    virtual void SAL_CALL Protect( const css::uno::Any& Password, const css::uno::Any& DrawingObjects, const css::uno::Any& Contents, const css::uno::Any& Scenarios, const css::uno::Any& UserInterfaceOnly ) override;
    virtual void SAL_CALL Unprotect( const css::uno::Any& Password ) override;

    virtual void SAL_CALL Calculate(  ) override;
    virtual void SAL_CALL CheckSpelling( const css::uno::Any& CustomDictionary,const css::uno::Any& IgnoreUppercase,const css::uno::Any& AlwaysSuggest, const css::uno::Any& SpellingLang ) override;
    // Hacks (?)
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Rows(const css::uno::Any& aIndex ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Columns(const css::uno::Any& aIndex ) override;

    virtual css::uno::Any SAL_CALL Evaluate( const OUString& Name ) override;
    virtual css::uno::Any SAL_CALL PivotTables( const css::uno::Any& Index ) override;
    virtual css::uno::Any SAL_CALL Comments( const css::uno::Any& Index ) override;
    virtual css::uno::Any SAL_CALL Hyperlinks( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) override;

    virtual css::uno::Any SAL_CALL OLEObjects( const css::uno::Any& Index ) override;
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) override;

    virtual css::uno::Any SAL_CALL Buttons( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL CheckBoxes( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL DropDowns( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL GroupBoxes( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL Labels( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL ListBoxes( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL OptionButtons( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL ScrollBars( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL Spinners( const css::uno::Any& rIndex ) override;

    virtual void SAL_CALL setEnableCalculation( sal_Bool EnableCalculation ) override;
    virtual sal_Bool SAL_CALL getEnableCalculation(  ) override;
    virtual void SAL_CALL ShowDataForm(  ) override;
    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;
    virtual css::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;
    // CodeName
    virtual OUString SAL_CALL getCodeName() override;
    /// @throws css::uno::RuntimeException
    sal_Int16 getSheetID() const;

    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName, const css::uno::Any& IgnorePrintAreas ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8 >& rId ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
