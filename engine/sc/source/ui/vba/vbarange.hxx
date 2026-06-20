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

#include <ooo/vba/excel/XRange.hpp>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/sheet/FillDateMode.hpp>
#include <com/sun/star/sheet/FillMode.hpp>
#include <com/sun/star/sheet/FillDirection.hpp>
#include <rtl/ref.hxx>
#include "vbaformat.hxx"
#include <address.hxx>
#include <formula/grammar.hxx>

namespace com::sun::star::sheet { class XSheetCellRangeContainer; }
namespace com::sun::star::table { class XCell; }
namespace com::sun::star::table { class XCellRange; }
namespace com::sun::star::table { struct CellRangeAddress; }
namespace ooo::vba { class XCollection; }

class SfxItemSet;
class ScCellRangeObj;
class ScDocShell;
class ScDocument;
class ScRangeList;
class ScVbaRangeAreas;
class ScVbaValidation;

class ArrayVisitor
{
public:
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const css::uno::Reference< css::table::XCell >& xCell ) = 0;
    virtual ~ArrayVisitor(){}
};

class ValueSetter : public ArrayVisitor
{
public:
    virtual bool processValue( const cpo::uno::Any& aValue, const css::uno::Reference< css::table::XCell >& xCell ) = 0;

};

class ValueGetter : public ArrayVisitor
{

public:
    virtual void processValue( const cpo::uno::Any& aValue ) = 0;
    virtual const cpo::uno::Any& getValue() const = 0;
};

typedef ScVbaFormat< ov::excel::XRange > ScVbaRange_BASE;

enum class RangeValueType { value, value2 };

class ScVbaRange : public ScVbaRange_BASE
{
    rtl::Reference< ScVbaRangeAreas > m_Areas;
    css::uno::Reference< ov::XCollection > m_Borders;
    css::uno::Reference< css::table::XCellRange > mxRange;
    css::uno::Reference< css::sheet::XSheetCellRangeContainer > mxRanges;
    bool mbIsRows;
    bool mbIsColumns;
    rtl::Reference< ScVbaValidation > m_xValidation;
    /// @throws css::uno::RuntimeException
    double getCalcColWidth(const css::table::CellRangeAddress&);
    /// @throws css::uno::RuntimeException
    double getCalcRowHeight(const css::table::CellRangeAddress&);
    void visitArray( ArrayVisitor& visitor );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > getEntireColumnOrRow( bool bColumn );

    /// @throws css::uno::RuntimeException
    void fillSeries(  css::sheet::FillDirection nFillDirection, css::sheet::FillMode nFillMode, css::sheet::FillDateMode nFillDateMode, double fStep, double fEndValue );

    /// @throws css::uno::RuntimeException
    void ClearContents( sal_Int32 nFlags, bool bFireEvent );

    /// @throws css::uno::RuntimeException
    cpo::uno::Any getValue( ValueGetter& rValueGetter );
    cpo::uno::Any DoGetValue( RangeValueType eValueType );
    /// @throws css::uno::RuntimeException
    void setValue( const cpo::uno::Any& aValue, ValueSetter& setter );

    /// @throws css::uno::RuntimeException
    cpo::uno::Any getFormulaValue( formula::FormulaGrammar::Grammar );
    /// @throws css::uno::RuntimeException
    void setFormulaValue( const cpo::uno::Any& aValue, formula::FormulaGrammar::Grammar );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > getArea( sal_Int32 nIndex  );
    /// @throws css::uno::RuntimeException
    ScCellRangeObj* getCellRangeObj( );
    css::uno::Reference< ov::XCollection >& getBorders();
    /// @throws css::uno::RuntimeException
    void groupUnGroup( bool bUnGroup );
     css::uno::Reference< ov::excel::XRange > PreviousNext( bool bIsPrevious );
     /// @throws css::script::BasicErrorException
     css::uno::Reference< ov::excel::XRange > SpecialCellsImpl( sal_Int32 nType, const cpo::uno::Any& _oValue);
    /// @throws css::uno::RuntimeException
    css::awt::Point getPosition() const;

    /** Fires a Worksheet_Change event for this range or range list. */
    void fireChangeEvent();

    /// @throws css::uno::RuntimeException
    ScRange obtainRangeEvenIfRangeListIsEmpty( const ScRangeList& rCellRanges ) const;

protected:
    virtual ScCellRangesBase* getCellRangesBase() override;
    /// @throws css::uno::RuntimeException
    SfxItemSet* getCurrentDataSet();
public:
    /// @throws css::lang::IllegalArgumentException
    ScVbaRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange, bool bIsRows = false, bool bIsColumns = false );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges, bool bIsRows = false, bool bIsColumns = false );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaRange( css::uno::Sequence< cpo::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    /// @throws css::uno::RuntimeException
    ScDocument& getScDocument();
    /// @throws css::uno::RuntimeException
    ScDocShell* getScDocShell();

    /** Returns the ScVbaRange implementation object for the passed VBA Range object. */
    static ScVbaRange* getImplementation( const css::uno::Reference< ov::excel::XRange >& rxRange );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::frame::XModel > getUnoModel();
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< css::frame::XModel > getUnoModel( const css::uno::Reference< ov::excel::XRange >& rxRange );

    /// @throws css::uno::RuntimeException
    const ScRangeList& getScRangeList();
    /// @throws css::uno::RuntimeException
    static const ScRangeList& getScRangeList( const css::uno::Reference< ov::excel::XRange >& rxRange );

    virtual ~ScVbaRange() override;
     virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() override { return this; }
    bool isSingleCellRange() const;

    /// @throws css::uno::RuntimeException
    static css::uno::Reference< ov::excel::XRange > getRangeObjectForName(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const OUString& sRangeName, ScDocShell* pDocSh,
        formula::FormulaGrammar::AddressConvention eConv  );

    /// @throws css::uno::RuntimeException
    static css::uno::Reference< ov::excel::XRange > CellsHelper(
        const ScDocument& rDoc,
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::table::XCellRange >& xRange,
        const cpo::uno::Any &nRowIndex, const cpo::uno::Any &nColumnIndex );

    // Attributes
    virtual cpo::uno::Any SAL_CALL getValue() override;
    virtual cpo::uno::Any SAL_CALL getValue2() override;
    virtual void   SAL_CALL setValue( const cpo::uno::Any& aValue ) override;
    virtual void   SAL_CALL setValue2( const cpo::uno::Any& aValue2 ) override;
    virtual cpo::uno::Any SAL_CALL getFormula() override;
    virtual void   SAL_CALL setFormula( const cpo::uno::Any& rFormula ) override;
    virtual cpo::uno::Any SAL_CALL getFormulaArray() override;
    virtual void   SAL_CALL setFormulaArray(const cpo::uno::Any& rFormula) override;
    virtual cpo::uno::Any SAL_CALL getFormulaR1C1() override;
    virtual void   SAL_CALL setFormulaR1C1( const cpo::uno::Any &rFormula ) override;
    virtual cpo::uno::Any SAL_CALL getFormulaLocal() override;
    virtual void   SAL_CALL setFormulaLocal( const cpo::uno::Any &rFormula ) override;
    virtual cpo::uno::Any SAL_CALL getFormulaR1C1Local() override;
    virtual void   SAL_CALL setFormulaR1C1Local( const cpo::uno::Any &rFormula ) override;
    virtual ::sal_Int32 SAL_CALL getCount() override;
    virtual ::sal_Int32 SAL_CALL getRow() override;
    virtual ::sal_Int32 SAL_CALL getColumn() override;
    virtual OUString SAL_CALL getText() override;
    using ScVbaRange_BASE::setNumberFormat;
    virtual void SAL_CALL setNumberFormat( const cpo::uno::Any& rNumberFormat ) override;
    virtual cpo::uno::Any SAL_CALL getNumberFormat() override;
    virtual void SAL_CALL setMergeCells( const cpo::uno::Any& bMerge ) override;
    virtual cpo::uno::Any SAL_CALL getMergeCells() override;
    virtual void SAL_CALL setWrapText( const cpo::uno::Any& bIsWrapped ) override;
    virtual cpo::uno::Any SAL_CALL getWrapText() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireRow() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireColumn() override;
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL getComment() override;
    virtual cpo::uno::Any SAL_CALL getHidden() override;
    virtual void SAL_CALL setHidden( const cpo::uno::Any& _hidden ) override;
    virtual cpo::uno::Any SAL_CALL getColumnWidth() override;
    virtual void SAL_CALL setColumnWidth( const cpo::uno::Any& _columnwidth ) override;
    virtual cpo::uno::Any SAL_CALL getRowHeight() override;
    virtual void SAL_CALL setRowHeight( const cpo::uno::Any& _rowheight ) override;
    virtual cpo::uno::Any SAL_CALL getWidth() override;
    virtual cpo::uno::Any SAL_CALL getHeight() override;
    virtual cpo::uno::Any SAL_CALL getTop() override;
    virtual cpo::uno::Any SAL_CALL getLeft() override;

    virtual css::uno::Reference< ov::excel::XName > SAL_CALL getName() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getWorksheet() override;
    virtual cpo::uno::Any SAL_CALL getPageBreak() override;
    virtual void SAL_CALL setPageBreak( const cpo::uno::Any& _pagebreak ) override;
    virtual css::uno::Reference< ov::excel::XValidation > SAL_CALL getValidation() override;
    virtual cpo::uno::Any SAL_CALL getPrefixCharacter() override;
    virtual cpo::uno::Any SAL_CALL getShowDetail() override;
    virtual void SAL_CALL setShowDetail(const cpo::uno::Any& aShowDetail) override;
    // Methods
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL AddComment( const cpo::uno::Any& Text ) override;
    virtual void SAL_CALL Clear() override;
    virtual void SAL_CALL ClearComments() override;
    virtual void SAL_CALL ClearContents() override;
    virtual void SAL_CALL ClearFormats() override;
    virtual cpo::uno::Any SAL_CALL HasFormula() override;
    virtual void SAL_CALL FillLeft() override;
    virtual void SAL_CALL FillRight() override;
    virtual void SAL_CALL FillUp() override;
    virtual void SAL_CALL FillDown() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Offset( const cpo::uno::Any &nRowOffset, const cpo::uno::Any &nColOffset ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentRegion() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentArray() override;
    virtual OUString SAL_CALL Characters( const cpo::uno::Any& nIndex, const cpo::uno::Any& nCount ) override;

    virtual OUString SAL_CALL Address( const cpo::uno::Any& RowAbsolute, const cpo::uno::Any& ColumnAbsolute, const cpo::uno::Any& ReferenceStyle, const cpo::uno::Any& External, const cpo::uno::Any& RelativeTo ) override;

    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const cpo::uno::Any &nRow, const cpo::uno::Any &nCol ) override;
    virtual void SAL_CALL Select() override;
    virtual void SAL_CALL Activate() override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Rows( const cpo::uno::Any& nIndex ) override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Columns( const cpo::uno::Any &nIndex ) override;
    virtual void SAL_CALL Copy( const cpo::uno::Any& Destination ) override;
    virtual void SAL_CALL Cut( const cpo::uno::Any& Destination ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Resize( const cpo::uno::Any& RowSize, const cpo::uno::Any& ColumnSize ) override;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL Font() override;
    virtual css::uno::Reference< ov::excel::XInterior > SAL_CALL Interior(  ) override ;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range( const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2 ) override;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > Range( const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2, bool bForceUseInpuRangeTab );
    virtual cpo::uno::Any SAL_CALL getCellRange(  ) override;
    /// @throws css::uno::RuntimeException
    static cpo::uno::Any getCellRange( const css::uno::Reference< ov::excel::XRange >& rxRange );
    virtual void SAL_CALL PasteSpecial( const cpo::uno::Any& Paste, const cpo::uno::Any& Operation, const cpo::uno::Any& SkipBlanks, const cpo::uno::Any& Transpose ) override;
    virtual bool SAL_CALL Replace( const OUString& What, const OUString& Replacement, const cpo::uno::Any& LookAt, const cpo::uno::Any& SearchOrder, const cpo::uno::Any& MatchCase, const cpo::uno::Any& MatchByte, const cpo::uno::Any& SearchFormat, const cpo::uno::Any& ReplaceFormat ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Find( const cpo::uno::Any& What, const cpo::uno::Any& After, const cpo::uno::Any& LookIn, const cpo::uno::Any& LookAt, const cpo::uno::Any& SearchOrder, const cpo::uno::Any& SearchDirection, const cpo::uno::Any& MatchCase, const cpo::uno::Any& MatchByte, const cpo::uno::Any& SearchFormat ) override;
    virtual void SAL_CALL Sort( const cpo::uno::Any& Key1, const cpo::uno::Any& Order1, const cpo::uno::Any& Key2, const cpo::uno::Any& Type, const cpo::uno::Any& Order2, const cpo::uno::Any& Key3, const cpo::uno::Any& Order3, const cpo::uno::Any& Header, const cpo::uno::Any& OrderCustom, const cpo::uno::Any& MatchCase, const cpo::uno::Any& Orientation, const cpo::uno::Any& SortMethod,  const cpo::uno::Any& DataOption1, const cpo::uno::Any& DataOption2, const cpo::uno::Any& DataOption3 ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL End( ::sal_Int32 Direction ) override;
    virtual css::uno::Reference< ov::excel::XCharacters > SAL_CALL characters( const cpo::uno::Any& Start, const cpo::uno::Any& Length ) override;
    virtual void SAL_CALL Delete( const cpo::uno::Any& Shift ) override;
    virtual cpo::uno::Any SAL_CALL Areas( const cpo::uno::Any& ) override;
    virtual cpo::uno::Any SAL_CALL Borders( const cpo::uno::Any& ) override;
    virtual cpo::uno::Any SAL_CALL BorderAround( const cpo::uno::Any& LineStyle,
                const cpo::uno::Any& Weight, const cpo::uno::Any& ColorIndex, const cpo::uno::Any& Color ) override;
    virtual cpo::uno::Any SAL_CALL Hyperlinks( const cpo::uno::Any& aIndex ) override;

    virtual void SAL_CALL AutoFilter( const cpo::uno::Any& Field, const cpo::uno::Any& Criteria1, const cpo::uno::Any& Operator, const cpo::uno::Any& Criteria2, const cpo::uno::Any& VisibleDropDown ) override;
    virtual void SAL_CALL Insert( const cpo::uno::Any& Shift, const cpo::uno::Any& CopyOrigin ) override;
    virtual void SAL_CALL Autofit() override;
    virtual void SAL_CALL PrintOut( const cpo::uno::Any& From, const cpo::uno::Any& To, const cpo::uno::Any& Copies, const cpo::uno::Any& Preview, const cpo::uno::Any& ActivePrinter, const cpo::uno::Any& PrintToFile, const cpo::uno::Any& Collate, const cpo::uno::Any& PrToFileName ) override;
    virtual void SAL_CALL AutoFill( const css::uno::Reference< ov::excel::XRange >& Destination, const cpo::uno::Any& Type ) override ;
     void SAL_CALL Calculate(  ) override;
    virtual void SAL_CALL AutoOutline(  ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Item( const cpo::uno::Any& row, const cpo::uno::Any& column ) override;
    virtual void SAL_CALL ClearOutline(  ) override;
    virtual void SAL_CALL Ungroup(  ) override;
    virtual void SAL_CALL Group(  ) override;
    virtual void SAL_CALL Merge( const cpo::uno::Any& Across ) override;
    virtual void SAL_CALL UnMerge(  ) override;
    virtual cpo::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const cpo::uno::Any& _style ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Next() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Previous() override;
    virtual void SAL_CALL RemoveSubtotal(  ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL MergeArea() override;
    virtual void SAL_CALL Subtotal( ::sal_Int32 GroupBy, ::sal_Int32 Function, const css::uno::Sequence< ::sal_Int32 >& TotalList, const cpo::uno::Any& Replace, const cpo::uno::Any& PageBreaks, const cpo::uno::Any& SummaryBelowData ) override;
    virtual void SAL_CALL ExportAsFixedFormat(const cpo::uno::Any& Type, const cpo::uno::Any& FileName, const cpo::uno::Any& Quality,
        const cpo::uno::Any& IncludeDocProperties, const cpo::uno::Any& IgnorePrintAreas, const cpo::uno::Any& From,
        const cpo::uno::Any& To, const cpo::uno::Any& OpenAfterPublish, const cpo::uno::Any& FixedFormatExtClassPtr) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<ov::excel::XRange>::get();

    }
    virtual bool SAL_CALL hasElements() override;
    // XDefaultMethod
    OUString SAL_CALL getDefaultMethodName(  ) override;
        // XDefaultProperty
        OUString SAL_CALL getDefaultPropertyName(  ) override { return u"Value"_ustr; }

// #TODO completely rewrite ScVbaRange, it's become a hackfest
// it needs to be closer to ScCellRangeBase in that the underlying
// object model should probably be a ScRangelst.
//     * would be nice to be able to construct a range from an address only
//     * or a list of address ( multi-area )
//     * object should be a lightweight as possible
//     * we shouldn't need hacks like this below
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< ov::excel::XRange > ApplicationRange( const css::uno::Reference< css::uno::XComponentContext >& xContext, const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2 );
    static bool getCellRangesForAddress(ScRefFlags &rResFlags, std::u16string_view sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, formula::FormulaGrammar::AddressConvention eConv, char cDelimiter );
    virtual bool SAL_CALL GoalSeek( const cpo::uno::Any& Goal, const css::uno::Reference< ov::excel::XRange >& ChangingCell ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL SpecialCells( const cpo::uno::Any& _oType, const cpo::uno::Any& _oValue) override;
    // XErrorQuery
    virtual bool SAL_CALL hasError(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/// @throws css::uno::RuntimeException
bool getScRangeListForAddress( const OUString& sName, ScDocShell* pDocSh, const ScRange& refRange,
                               ScRangeList& aCellRanges,
                               formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
