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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEET_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEET_HXX

#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <ooo/vba/excel/XWorksheet.hpp>
#include <ooo/vba/excel/XComments.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <ooo/vba/excel/XOutline.hpp>
#include <ooo/vba/excel/XPageSetup.hpp>
#include <ooo/vba/excel/XHPageBreaks.hpp>
#include <ooo/vba/excel/XVPageBreaks.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "address.hxx"

namespace ooo { namespace vba { namespace excel {
    class XChartObjects;
    class XHyperlinks;
} } }

class ScVbaSheetObjectsBase;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XWorksheet >  WorksheetImpl_BASE;

class ScVbaWorksheet : public WorksheetImpl_BASE
{
    css::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< ov::excel::XChartObjects > mxCharts;
    css::uno::Reference< ov::excel::XHyperlinks > mxHlinks;
    ::rtl::Reference< ScVbaSheetObjectsBase > mxButtons;
    bool mbVeryHidden;

    css::uno::Reference< ov::excel::XWorksheet > getSheetAtOffset(SCTAB offset) throw (css::uno::RuntimeException);
    css::uno::Reference< ov::excel::XRange > getSheetRange() throw (css::uno::RuntimeException);

    css::uno::Reference< css::container::XNameAccess > getFormControls();
    css::uno::Any getControlShape( const OUString& sName );

public:
    ScVbaWorksheet( const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::sheet::XSpreadsheet >& xSheet,
        const css::uno::Reference< css::frame::XModel >& xModel )throw (css::uno::RuntimeException)  ;
    ScVbaWorksheet( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual ~ScVbaWorksheet();

    css::uno::Reference< css::frame::XModel > getModel()
    { return mxModel; }
    css::uno::Reference< css::sheet::XSpreadsheet > getSheet()
    { return mxSheet; }
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    css::uno::Reference< ov::excel::XWorksheet > createSheetCopyInNewDoc( const OUString& );
    css::uno::Reference< ov::excel::XWorksheet > createSheetCopy(css::uno::Reference< ov::excel::XWorksheet> xSheet, bool bAfter);

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString &rName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVisible( sal_Int32 nVisible ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getStandardWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getStandardHeight() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getProtectionMode() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getProtectContents() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getProtectDrawingObjects() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getProtectScenarios() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getUsedRange() throw (css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL ChartObjects( const css::uno::Any& Index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XOutline > SAL_CALL Outline( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XPageSetup > SAL_CALL PageSetup( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL HPageBreaks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL VPageBreaks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getNext() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getPrevious() throw (css::uno::RuntimeException, std::exception) override;
     virtual sal_Int16 SAL_CALL getIndex() throw (css::uno::RuntimeException, std::exception) override;
     virtual sal_Int32 SAL_CALL getEnableSelection() throw (css::uno::RuntimeException, std::exception) override;
     virtual void SAL_CALL setEnableSelection( sal_Int32 nSelection ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoFilterMode() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoFilterMode( sal_Bool bAutoFilterMode ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Select() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Move( const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException, std::exception) override ;
     virtual void SAL_CALL Copy( const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Paste( const css::uno::Any& Destination, const css::uno::Any& Link ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Protect( const css::uno::Any& Password, const css::uno::Any& DrawingObjects, const css::uno::Any& Contents, const css::uno::Any& Scenarios, const css::uno::Any& UserInterfaceOnly ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Unprotect( const css::uno::Any& Password ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL Calculate(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL CheckSpelling( const css::uno::Any& CustomDictionary,const css::uno::Any& IgnoreUppercase,const css::uno::Any& AlwaysSuggest, const css::uno::Any& SpellingLang ) throw (css::uno::RuntimeException, std::exception) override;
    // Hacks (?)
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Rows(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Columns(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL Evaluate( const OUString& Name ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL PivotTables( const css::uno::Any& Index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Comments( const css::uno::Any& Index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Hyperlinks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL OLEObjects( const css::uno::Any& Index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL Buttons( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL CheckBoxes( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL DropDowns( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL GroupBoxes( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Labels( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL ListBoxes( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL OptionButtons( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL ScrollBars( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Spinners( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setEnableCalculation( sal_Bool EnableCalculation ) throw ( css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getEnableCalculation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ShowDataForm(  ) throw (css::uno::RuntimeException, std::exception) override;
    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;
    // CodeName
    virtual OUString SAL_CALL getCodeName() throw (css::uno::RuntimeException, std::exception) override;
    sal_Int16 getSheetID() throw (css::uno::RuntimeException);

    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName, const css::uno::Any& IgnorePrintAreas ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAWORKSHEET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
