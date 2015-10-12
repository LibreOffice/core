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

#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <ooo/vba/excel/XFont.hpp>
#include <ooo/vba/excel/XComment.hpp>
#include <ooo/vba/XCollection.hpp>
#include <ooo/vba/excel/XlPasteType.hpp>
#include <ooo/vba/excel/XlPasteSpecialOperation.hpp>

#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/sheet/FillDateMode.hpp>
#include <com/sun/star/sheet/FillMode.hpp>
#include <com/sun/star/sheet/FillDirection.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>

#include "vbaformat.hxx"
#include <formula/grammar.hxx>

class ScCellRangesBase;
class ScCellRangeObj;
class ScDocShell;
class ScDocument;
class ScRangeList;
class ScRange;

typedef ScVbaFormat< ov::excel::XRange > ScVbaRange_BASE;

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
    virtual void processValue( sal_Int32 x, sal_Int32 y, const css::uno::Any& aValue ) = 0;
    virtual const css::uno::Any& getValue() const = 0;
};

class ScVbaRange : public ScVbaRange_BASE
{
    css::uno::Reference< ov::XCollection > m_Areas;
    css::uno::Reference< ov::XCollection > m_Borders;
    css::uno::Reference< css::table::XCellRange > mxRange;
    css::uno::Reference< css::sheet::XSheetCellRangeContainer > mxRanges;
    bool mbIsRows;
    bool mbIsColumns;
    css::uno::Reference< ov::excel::XValidation > m_xValidation;
    double getCalcColWidth(const css::table::CellRangeAddress&)
        throw (css::uno::RuntimeException, std::exception);
    double getCalcRowHeight(const css::table::CellRangeAddress&)
        throw (css::uno::RuntimeException, std::exception);
    void visitArray( ArrayVisitor& vistor );

    css::uno::Reference< ov::excel::XRange > getEntireColumnOrRow( bool bColumn = true ) throw( css::uno::RuntimeException );

    void fillSeries(  css::sheet::FillDirection nFillDirection, css::sheet::FillMode nFillMode, css::sheet::FillDateMode nFillDateMode, double fStep, double fEndValue ) throw( css::uno::RuntimeException );

    void ClearContents( sal_Int32 nFlags, bool bFireEvent ) throw (css::uno::RuntimeException);

    css::uno::Any getValue( ValueGetter& rValueGetter ) throw (css::uno::RuntimeException);
    void setValue( const css::uno::Any& aValue, ValueSetter& setter, bool bFireEvent ) throw ( css::uno::RuntimeException);

    css::uno::Any getFormulaValue( formula::FormulaGrammar::Grammar ) throw (css::uno::RuntimeException);
    void setFormulaValue( const css::uno::Any& aValue, formula::FormulaGrammar::Grammar, bool bFireEvent ) throw ( css::uno::RuntimeException);

    css::uno::Reference< ov::excel::XRange > getArea( sal_Int32 nIndex  ) throw( css::uno::RuntimeException );
    ScCellRangeObj* getCellRangeObj( )  throw ( css::uno::RuntimeException );
    css::uno::Reference< ov::XCollection >& getBorders();
    void groupUnGroup( bool bUnGroup = false ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
     css::uno::Reference< ov::excel::XRange > PreviousNext( bool bIsPrevious );
     css::uno::Reference< ov::excel::XRange > SpecialCellsImpl( sal_Int32 nType, const css::uno::Any& _oValue) throw ( css::script::BasicErrorException );
    css::awt::Point getPosition() throw ( css::uno::RuntimeException );

    /** Fires a Worksheet_Change event for this range or range list. */
    void fireChangeEvent();

protected:
    virtual ScCellRangesBase* getCellRangesBase() throw ( css::uno::RuntimeException ) override;
    SfxItemSet* getCurrentDataSet( )  throw ( css::uno::RuntimeException );
public:
    ScVbaRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange, bool bIsRows = false, bool bIsColumns = false ) throw ( css::lang::IllegalArgumentException );
    ScVbaRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges, bool bIsRows = false, bool bIsColumns = false ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );
    ScVbaRange( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    ScDocument& getScDocument() throw (css::uno::RuntimeException);
    ScDocShell* getScDocShell() throw (css::uno::RuntimeException);

    /** Returns the ScVbaRange implementation object for the passed VBA Range object. */
    static ScVbaRange* getImplementation( const css::uno::Reference< ov::excel::XRange >& rxRange );

    css::uno::Reference< css::frame::XModel > getUnoModel() throw (css::uno::RuntimeException);
    static css::uno::Reference< css::frame::XModel > getUnoModel( const css::uno::Reference< ov::excel::XRange >& rxRange ) throw (css::uno::RuntimeException);

    const ScRangeList& getScRangeList() throw (css::uno::RuntimeException);
    static const ScRangeList& getScRangeList( const css::uno::Reference< ov::excel::XRange >& rxRange ) throw (css::uno::RuntimeException);

    virtual ~ScVbaRange();
     virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() override { return this; }
    bool isSingleCellRange();

    static css::uno::Reference< ov::excel::XRange > getRangeObjectForName(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const OUString& sRangeName, ScDocShell* pDocSh,
        formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_XL_A1  ) throw ( css::uno::RuntimeException, std::exception );

    static css::uno::Reference< ov::excel::XRange > CellsHelper(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::table::XCellRange >& xRange,
        const css::uno::Any &nRowIndex, const css::uno::Any &nColumnIndex ) throw(css::uno::RuntimeException);

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual void   SAL_CALL setValue( const css::uno::Any& aValue ) throw ( css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getFormula() throw (css::uno::RuntimeException, std::exception) override;
    virtual void   SAL_CALL setFormula( const css::uno::Any& rFormula ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getFormulaArray() throw (css::uno::RuntimeException, std::exception) override;
    virtual void   SAL_CALL setFormulaArray(const css::uno::Any& rFormula) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getFormulaR1C1() throw (css::uno::RuntimeException, std::exception) override;
    virtual void   SAL_CALL setFormulaR1C1( const css::uno::Any &rFormula ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getRow() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getColumn() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    using ScVbaRange_BASE::setNumberFormat;
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& rNumberFormat ) throw ( css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL getNumberFormat() throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual void SAL_CALL setMergeCells( const css::uno::Any& bMerge ) throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL getMergeCells() throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual void SAL_CALL setWrapText( const css::uno::Any& bIsWrapped ) throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL getWrapText() throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireRow() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getEntireColumn() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL getComment() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getHidden() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHidden( const css::uno::Any& _hidden ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColumnWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumnWidth( const css::uno::Any& _columnwidth ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getRowHeight()
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowHeight( const css::uno::Any& _rowheight ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getHeight()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getTop() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getLeft() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< ov::excel::XName > SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getWorksheet() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPageBreak() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPageBreak( const css::uno::Any& _pagebreak ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XValidation > SAL_CALL getValidation() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPrefixCharacter() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getShowDetail() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShowDetail(const css::uno::Any& aShowDetail) throw (css::uno::RuntimeException, std::exception) override;
    // Methods
    bool IsRows() const { return mbIsRows; }
    bool IsColumns() const { return mbIsColumns; }
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL AddComment( const css::uno::Any& Text ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Clear() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ClearComments() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ClearContents() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ClearFormats() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL HasFormula() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL FillLeft() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL FillRight() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL FillUp() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL FillDown() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Offset( const css::uno::Any &nRowOffset, const css::uno::Any &nColOffset )
                                                           throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentRegion() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL CurrentArray() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL Characters( const css::uno::Any& nIndex, const css::uno::Any& nCount )
                                                 throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL Address( const css::uno::Any& RowAbsolute, const css::uno::Any& ColumnAbsolute, const css::uno::Any& ReferenceStyle, const css::uno::Any& External, const css::uno::Any& RelativeTo ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol )
                                                          throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Select() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Rows( const css::uno::Any& nIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange >  SAL_CALL Columns( const css::uno::Any &nIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Copy( const css::uno::Any& Destination ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Cut( const css::uno::Any& Destination ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Resize( const css::uno::Any& RowSize, const css::uno::Any& ColumnSize )
                                                           throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL Font() throw ( css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Reference< ov::excel::XInterior > SAL_CALL Interior(  ) throw ( css::script::BasicErrorException, css::uno::RuntimeException) override ;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< ov::excel::XRange > Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2, bool bForceUseInpuRangeTab ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL getCellRange(  ) throw (css::uno::RuntimeException, std::exception) override;
    static css::uno::Any getCellRange( const css::uno::Reference< ov::excel::XRange >& rxRange ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL PasteSpecial( const css::uno::Any& Paste, const css::uno::Any& Operation, const css::uno::Any& SkipBlanks, const css::uno::Any& Transpose ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL Replace( const OUString& What, const OUString& Replacement, const css::uno::Any& LookAt, const css::uno::Any& SearchOrder, const css::uno::Any& MatchCase, const css::uno::Any& MatchByte, const css::uno::Any& SearchFormat, const css::uno::Any& ReplaceFormat ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Find( const css::uno::Any& What, const css::uno::Any& After, const css::uno::Any& LookIn, const css::uno::Any& LookAt, const css::uno::Any& SearchOrder, const css::uno::Any& SearchDirection, const css::uno::Any& MatchCase, const css::uno::Any& MatchByte, const css::uno::Any& SearchFormat ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Sort( const css::uno::Any& Key1, const css::uno::Any& Order1, const css::uno::Any& Key2, const css::uno::Any& Type, const css::uno::Any& Order2, const css::uno::Any& Key3, const css::uno::Any& Order3, const css::uno::Any& Header, const css::uno::Any& OrderCustom, const css::uno::Any& MatchCase, const css::uno::Any& Orientation, const css::uno::Any& SortMethod,  const css::uno::Any& DataOption1, const css::uno::Any& DataOption2, const css::uno::Any& DataOption3 ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL End( ::sal_Int32 Direction )  throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XCharacters > SAL_CALL characters( const css::uno::Any& Start, const css::uno::Any& Length ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Delete( const css::uno::Any& Shift ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Areas( const css::uno::Any& ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& ) throw ( css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL BorderAround( const css::uno::Any& LineStyle,
                const css::uno::Any& Weight, const css::uno::Any& ColorIndex, const css::uno::Any& Color ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Hyperlinks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL AutoFilter( const css::uno::Any& Field, const css::uno::Any& Criteria1, const css::uno::Any& Operator, const css::uno::Any& Criteria2, const css::uno::Any& VisibleDropDown )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Insert( const css::uno::Any& Shift, const css::uno::Any& CopyOrigin ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Autofit() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL AutoFill( const css::uno::Reference< ov::excel::XRange >& Destination, const css::uno::Any& Type ) throw (css::uno::RuntimeException, std::exception) override ;
     void SAL_CALL Calculate(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL AutoOutline(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Item( const css::uno::Any& row, const css::uno::Any& column ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ClearOutline(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Ungroup(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Group(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Merge( const css::uno::Any& Across ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL UnMerge(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getStyle() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStyle( const css::uno::Any& _style ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Next() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Previous() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL RemoveSubtotal(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL MergeArea() throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Subtotal( ::sal_Int32 GroupBy, ::sal_Int32 Function, const css::uno::Sequence< ::sal_Int32 >& TotalList, const css::uno::Any& Replace, const css::uno::Any& PageBreaks, const css::uno::Any& SummaryBelowData ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException, std::exception) override;
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::UnoType<ov::excel::XRange>::get();

    }
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;
    // XDefaultMethod
    OUString SAL_CALL getDefaultMethodName(  ) throw (css::uno::RuntimeException, std::exception) override;
        // XDefaultProperty
        OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException, std::exception) override { return OUString("Value"); }

// #TODO completely rewrite ScVbaRange, it's become a hackfest
// it needs to be closer to ScCellRangeBase in that the underlying
// object model should probably be a ScRangelst.
//     * would be nice to be able to construct a range from an address only
//     * or a list of address ( multi-area )
//     * object should be a lightweight as possible
//     * we shouldn't need hacks like this below
    static css::uno::Reference< ov::excel::XRange > ApplicationRange( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException, std::exception);
    static bool getCellRangesForAddress( sal_uInt16& rResFlags, const OUString& sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, formula::FormulaGrammar::AddressConvention& eConv, char cDelimiter = 0 );
    virtual sal_Bool SAL_CALL GoalSeek( const css::uno::Any& Goal, const css::uno::Reference< ov::excel::XRange >& ChangingCell ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL SpecialCells( const css::uno::Any& _oType, const css::uno::Any& _oValue)
        throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    // XErrorQuery
    virtual sal_Bool SAL_CALL hasError(  ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

bool getScRangeListForAddress( const OUString& sName, ScDocShell* pDocSh, ScRange& refRange,
                               ScRangeList& aCellRanges,
                               formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 )
    throw ( css::uno::RuntimeException );

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBARANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
