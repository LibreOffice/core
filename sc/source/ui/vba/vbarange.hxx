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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBARANGE_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBARANGE_HXX

#include <ooo/vba/excel/XRange.hpp>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/sheet/FillDateMode.hpp>
#include <com/sun/star/sheet/FillMode.hpp>
#include <com/sun/star/sheet/FillDirection.hpp>

#include "vbaformat.hxx"
#include <address.hxx>
#include <formula/grammar.hxx>

namespace com::sun::star::sheet { class XSheetCellRangeContainer; }
namespace com::sun::star::table { class XCell; }
namespace com::sun::star::table { class XCellRange; }
namespace com::sun::star::table { struct CellRangeAddress; }
namespace com::sun::star::lang { class XServiceInfo; }
namespace ooo::vba { class XCollection; }
namespace ooo::vba::excel { class XComment; }
namespace ooo::vba::excel { class XFont; }

class SfxItemSet;
class ScCellRangesBase;
class ScCellRangeObj;
class ScDocShell;
class ScDocument;
class ScRangeList;

class ArrayVisitor
{
public:
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const css::uno::Reference< css::table::XCell >& xCell ) = 0;
    virtual ~ArrayVisitor(){}
};

class ValueSetter : public ArrayVisitor
{
public:
    virtual bool processValue( const css::uno::Any& aValue, const css::uno::Reference< css::table::XCell >& xCell ) = 0;

};

class ValueGetter : public ArrayVisitor
{

public:
    virtual void processValue( const css::uno::Any& aValue ) = 0;
    virtual const css::uno::Any& getValue() const = 0;
};

typedef ScVbaFormat< ov::excel::XRange > ScVbaRange_BASE;

class ScVbaRange : public ScVbaRange_BASE
{
    css::uno::Reference< ov::XCollection > m_Areas;
    css::uno::Reference< ov::XCollection > m_Borders;
    css::uno::Reference< css::table::XCellRange > mxRange;
    css::uno::Reference< css::sheet::XSheetCellRangeContainer > mxRanges;
    bool mbIsRows;
    bool mbIsColumns;
    css::uno::Reference< ov::excel::XValidation > m_xValidation;
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
    css::uno::Any getValue( ValueGetter& rValueGetter );
    /// @throws css::uno::RuntimeException
    void setValue( const css::uno::Any& aValue, ValueSetter& setter );

    /// @throws css::uno::RuntimeException
    css::uno::Any getFormulaValue( formula::FormulaGrammar::Grammar );
    /// @throws css::uno::RuntimeException
    void setFormulaValue( const css::uno::Any& aValue, formula::FormulaGrammar::Grammar );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > getArea( sal_Int32 nIndex  );
    /// @throws css::uno::RuntimeException
    ScCellRangeObj* getCellRangeObj( );
    css::uno::Reference< ov::XCollection >& getBorders();
    /// @throws css::uno::RuntimeException
    void groupUnGroup( bool bUnGroup );
     css::uno::Reference< ov::excel::XRange > PreviousNext( bool bIsPrevious );
     /// @throws css::script::BasicErrorException
     css::uno::Reference< ov::excel::XRange > SpecialCellsImpl( sal_Int32 nType, const css::uno::Any& _oValue);
    /// @throws css::uno::RuntimeException
    css::awt::Point getPosition() const;

    /** Fires a Worksheet_Change event for this range or range list. */
    void fireChangeEvent();

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
    ScVbaRange( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

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
        const css::uno::Any &nRowIndex, const css::uno::Any &nColumnIndex );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void   SAL_CALL setValue( const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getFormula() override;
    virtual void   SAL_CALL setFormula( const css::uno::Any& rFormula ) override;
    virtual css::uno::Any SAL_CALL getFormulaArray() override;
    virtual void   SAL_CALL setFormulaArray(const css::uno::Any& rFormula) override;
    virtual css::uno::Any SAL_CALL getFormulaR1C1() override;
    virtual void   SAL_CALL setFormulaR1C1( const css::uno::Any &rFormula ) override;
    virtual ::sal_Int32 SAL_CALL getCount() override;
    virtual ::sal_Int32 SAL_CALL getRow() override;
    virtual ::sal_Int32 SAL_CALL getColumn() override;
    virtual OUString SAL_CALL getText() override;
    using ScVbaRange_BASE::setNumberFormat;
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& rNumberFormat ) override;
    virtual css::uno::Any SAL_CALL getNumberFormat() override;
    virtual void SAL_CALL setMergeCells( const css::uno::Any& bMerge ) override;
    virtual css::uno::Any SAL_CALL getMergeCells() override;
    virtual void SAL_CALL setWrapText( const css::uno::Any& bIsWrapped ) override;
    virtual css::uno::Any SAL_CALL getWrapText() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireRow() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireColumn() override;
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL getComment() override;
    virtual css::uno::Any SAL_CALL getHidden() override;
    virtual void SAL_CALL setHidden( const css::uno::Any& _hidden ) override;
    virtual css::uno::Any SAL_CALL getColumnWidth() override;
    virtual void SAL_CALL setColumnWidth( const css::uno::Any& _columnwidth ) override;
    virtual css::uno::Any SAL_CALL getRowHeight() override;
    virtual void SAL_CALL setRowHeight( const css::uno::Any& _rowheight ) override;
    virtual css::uno::Any SAL_CALL getWidth() override;
    virtual css::uno::Any SAL_CALL getHeight() override;
    virtual css::uno::Any SAL_CALL getTop() override;
    virtual css::uno::Any SAL_CALL getLeft() override;

    virtual css::uno::Reference< ov::excel::XName > SAL_CALL getName() override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getWorksheet() override;
    virtual css::uno::Any SAL_CALL getPageBreak() override;
    virtual void SAL_CALL setPageBreak( const css::uno::Any& _pagebreak ) override;
    virtual css::uno::Reference< ov::excel::XValidation > SAL_CALL getValidation() override;
    virtual css::uno::Any SAL_CALL getPrefixCharacter() override;
    virtual css::uno::Any SAL_CALL getShowDetail() override;
    virtual void SAL_CALL setShowDetail(const css::uno::Any& aShowDetail) override;
    // Methods
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL AddComment( const css::uno::Any& Text ) override;
    virtual void SAL_CALL Clear() override;
    virtual void SAL_CALL ClearComments() override;
    virtual void SAL_CALL ClearContents() override;
    virtual void SAL_CALL ClearFormats() override;
    virtual css::uno::Any SAL_CALL HasFormula() override;
    virtual void SAL_CALL FillLeft() override;
    virtual void SAL_CALL FillRight() override;
    virtual void SAL_CALL FillUp() override;
    virtual void SAL_CALL FillDown() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Offset( const css::uno::Any &nRowOffset, const css::uno::Any &nColOffset ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentRegion() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentArray() override;
    virtual OUString SAL_CALL Characters( const css::uno::Any& nIndex, const css::uno::Any& nCount ) override;

    virtual OUString SAL_CALL Address( const css::uno::Any& RowAbsolute, const css::uno::Any& ColumnAbsolute, const css::uno::Any& ReferenceStyle, const css::uno::Any& External, const css::uno::Any& RelativeTo ) override;

    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol ) override;
    virtual void SAL_CALL Select() override;
    virtual void SAL_CALL Activate() override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Rows( const css::uno::Any& nIndex ) override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Columns( const css::uno::Any &nIndex ) override;
    virtual void SAL_CALL Copy( const css::uno::Any& Destination ) override;
    virtual void SAL_CALL Cut( const css::uno::Any& Destination ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Resize( const css::uno::Any& RowSize, const css::uno::Any& ColumnSize ) override;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL Font() override;
    virtual css::uno::Reference< ov::excel::XInterior > SAL_CALL Interior(  ) override ;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) override;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XRange > Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2, bool bForceUseInpuRangeTab );
    virtual css::uno::Any SAL_CALL getCellRange(  ) override;
    /// @throws css::uno::RuntimeException
    static css::uno::Any getCellRange( const css::uno::Reference< ov::excel::XRange >& rxRange );
    virtual void SAL_CALL PasteSpecial( const css::uno::Any& Paste, const css::uno::Any& Operation, const css::uno::Any& SkipBlanks, const css::uno::Any& Transpose ) override;
    virtual sal_Bool SAL_CALL Replace( const OUString& What, const OUString& Replacement, const css::uno::Any& LookAt, const css::uno::Any& SearchOrder, const css::uno::Any& MatchCase, const css::uno::Any& MatchByte, const css::uno::Any& SearchFormat, const css::uno::Any& ReplaceFormat ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Find( const css::uno::Any& What, const css::uno::Any& After, const css::uno::Any& LookIn, const css::uno::Any& LookAt, const css::uno::Any& SearchOrder, const css::uno::Any& SearchDirection, const css::uno::Any& MatchCase, const css::uno::Any& MatchByte, const css::uno::Any& SearchFormat ) override;
    virtual void SAL_CALL Sort( const css::uno::Any& Key1, const css::uno::Any& Order1, const css::uno::Any& Key2, const css::uno::Any& Type, const css::uno::Any& Order2, const css::uno::Any& Key3, const css::uno::Any& Order3, const css::uno::Any& Header, const css::uno::Any& OrderCustom, const css::uno::Any& MatchCase, const css::uno::Any& Orientation, const css::uno::Any& SortMethod,  const css::uno::Any& DataOption1, const css::uno::Any& DataOption2, const css::uno::Any& DataOption3 ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL End( ::sal_Int32 Direction ) override;
    virtual css::uno::Reference< ov::excel::XCharacters > SAL_CALL characters( const css::uno::Any& Start, const css::uno::Any& Length ) override;
    virtual void SAL_CALL Delete( const css::uno::Any& Shift ) override;
    virtual css::uno::Any SAL_CALL Areas( const css::uno::Any& ) override;
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& ) override;
    virtual css::uno::Any SAL_CALL BorderAround( const css::uno::Any& LineStyle,
                const css::uno::Any& Weight, const css::uno::Any& ColorIndex, const css::uno::Any& Color ) override;
    virtual css::uno::Any SAL_CALL Hyperlinks( const css::uno::Any& aIndex ) override;

    virtual void SAL_CALL AutoFilter( const css::uno::Any& Field, const css::uno::Any& Criteria1, const css::uno::Any& Operator, const css::uno::Any& Criteria2, const css::uno::Any& VisibleDropDown ) override;
    virtual void SAL_CALL Insert( const css::uno::Any& Shift, const css::uno::Any& CopyOrigin ) override;
    virtual void SAL_CALL Autofit() override;
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) override;
    virtual void SAL_CALL AutoFill( const css::uno::Reference< ov::excel::XRange >& Destination, const css::uno::Any& Type ) override ;
     void SAL_CALL Calculate(  ) override;
    virtual void SAL_CALL AutoOutline(  ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Item( const css::uno::Any& row, const css::uno::Any& column ) override;
    virtual void SAL_CALL ClearOutline(  ) override;
    virtual void SAL_CALL Ungroup(  ) override;
    virtual void SAL_CALL Group(  ) override;
    virtual void SAL_CALL Merge( const css::uno::Any& Across ) override;
    virtual void SAL_CALL UnMerge(  ) override;
    virtual css::uno::Any SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _style ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Next() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Previous() override;
    virtual void SAL_CALL RemoveSubtotal(  ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL MergeArea() override;
    virtual void SAL_CALL Subtotal( ::sal_Int32 GroupBy, ::sal_Int32 Function, const css::uno::Sequence< ::sal_Int32 >& TotalList, const css::uno::Any& Replace, const css::uno::Any& PageBreaks, const css::uno::Any& SummaryBelowData ) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<ov::excel::XRange>::get();

    }
    virtual sal_Bool SAL_CALL hasElements() override;
    // XDefaultMethod
    OUString SAL_CALL getDefaultMethodName(  ) override;
        // XDefaultProperty
        OUString SAL_CALL getDefaultPropertyName(  ) override { return "Value"; }

// #TODO completely rewrite ScVbaRange, it's become a hackfest
// it needs to be closer to ScCellRangeBase in that the underlying
// object model should probably be a ScRangelst.
//     * would be nice to be able to construct a range from an address only
//     * or a list of address ( multi-area )
//     * object should be a lightweight as possible
//     * we shouldn't need hacks like this below
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< ov::excel::XRange > ApplicationRange( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Any &Cell1, const css::uno::Any &Cell2 );
    static bool getCellRangesForAddress(ScRefFlags &rResFlags, const OUString& sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, formula::FormulaGrammar::AddressConvention eConv, char cDelimiter );
    virtual sal_Bool SAL_CALL GoalSeek( const css::uno::Any& Goal, const css::uno::Reference< ov::excel::XRange >& ChangingCell ) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL SpecialCells( const css::uno::Any& _oType, const css::uno::Any& _oValue) override;
    // XErrorQuery
    virtual sal_Bool SAL_CALL hasError(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/// @throws css::uno::RuntimeException
bool getScRangeListForAddress( const OUString& sName, ScDocShell* pDocSh, const ScRange& refRange,
                               ScRangeList& aCellRanges,
                               formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 );

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBARANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
