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
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const cpo::uno::Reference< css::table::XCell >& xCell ) = 0;
    virtual ~ArrayVisitor(){}
};

class ValueSetter : public ArrayVisitor
{
public:
    virtual bool processValue( const cpo::uno::Any& aValue, const cpo::uno::Reference< css::table::XCell >& xCell ) = 0;

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
    cpo::uno::Reference< ov::XCollection > m_Borders;
    cpo::uno::Reference< css::table::XCellRange > mxRange;
    cpo::uno::Reference< css::sheet::XSheetCellRangeContainer > mxRanges;
    bool mbIsRows;
    bool mbIsColumns;
    rtl::Reference< ScVbaValidation > m_xValidation;
    /// @throws cpo::uno::RuntimeException
    double getCalcColWidth(const css::table::CellRangeAddress&);
    /// @throws cpo::uno::RuntimeException
    double getCalcRowHeight(const css::table::CellRangeAddress&);
    void visitArray( ArrayVisitor& visitor );

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XRange > getEntireColumnOrRow( bool bColumn );

    /// @throws cpo::uno::RuntimeException
    void fillSeries(  css::sheet::FillDirection nFillDirection, css::sheet::FillMode nFillMode, css::sheet::FillDateMode nFillDateMode, double fStep, double fEndValue );

    /// @throws cpo::uno::RuntimeException
    void ClearContents( sal_Int32 nFlags, bool bFireEvent );

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Any getValue( ValueGetter& rValueGetter );
    cpo::uno::Any DoGetValue( RangeValueType eValueType );
    /// @throws cpo::uno::RuntimeException
    void setValue( const cpo::uno::Any& aValue, ValueSetter& setter );

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Any getFormulaValue( formula::FormulaGrammar::Grammar );
    /// @throws cpo::uno::RuntimeException
    void setFormulaValue( const cpo::uno::Any& aValue, formula::FormulaGrammar::Grammar );

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XRange > getArea( sal_Int32 nIndex  );
    /// @throws cpo::uno::RuntimeException
    ScCellRangeObj* getCellRangeObj( );
    cpo::uno::Reference< ov::XCollection >& getBorders();
    /// @throws cpo::uno::RuntimeException
    void groupUnGroup( bool bUnGroup );
     cpo::uno::Reference< ov::excel::XRange > PreviousNext( bool bIsPrevious );
     /// @throws css::script::BasicErrorException
     cpo::uno::Reference< ov::excel::XRange > SpecialCellsImpl( sal_Int32 nType, const cpo::uno::Any& _oValue);
    /// @throws cpo::uno::RuntimeException
    css::awt::Point getPosition() const;

    /** Fires a Worksheet_Change event for this range or range list. */
    void fireChangeEvent();

    /// @throws cpo::uno::RuntimeException
    ScRange obtainRangeEvenIfRangeListIsEmpty( const ScRangeList& rCellRanges ) const;

protected:
    virtual ScCellRangesBase* getCellRangesBase() override;
    /// @throws cpo::uno::RuntimeException
    SfxItemSet* getCurrentDataSet();
public:
    /// @throws css::lang::IllegalArgumentException
    ScVbaRange( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, const cpo::uno::Reference< css::table::XCellRange >& xRange, bool bIsRows = false, bool bIsColumns = false );
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaRange( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, const cpo::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges, bool bIsRows = false, bool bIsColumns = false );
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaRange( cpo::uno::Sequence< cpo::uno::Any > const& aArgs, cpo::uno::Reference< cpo::uno::XComponentContext >const& xContext );

    /// @throws cpo::uno::RuntimeException
    ScDocument& getScDocument();
    /// @throws cpo::uno::RuntimeException
    ScDocShell* getScDocShell();

    /** Returns the ScVbaRange implementation object for the passed VBA Range object. */
    static ScVbaRange* getImplementation( const cpo::uno::Reference< ov::excel::XRange >& rxRange );

    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::frame::XModel > getUnoModel();
    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Reference< css::frame::XModel > getUnoModel( const cpo::uno::Reference< ov::excel::XRange >& rxRange );

    /// @throws cpo::uno::RuntimeException
    const ScRangeList& getScRangeList();
    /// @throws cpo::uno::RuntimeException
    static const ScRangeList& getScRangeList( const cpo::uno::Reference< ov::excel::XRange >& rxRange );

    virtual ~ScVbaRange() override;
     virtual cpo::uno::Reference< ov::XHelperInterface > thisHelperIface() override { return this; }
    bool isSingleCellRange() const;

    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Reference< ov::excel::XRange > getRangeObjectForName(
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const OUString& sRangeName, ScDocShell* pDocSh,
        formula::FormulaGrammar::AddressConvention eConv  );

    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Reference< ov::excel::XRange > CellsHelper(
        const ScDocument& rDoc,
        const cpo::uno::Reference< ov::XHelperInterface >& xParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const cpo::uno::Reference< css::table::XCellRange >& xRange,
        const cpo::uno::Any &nRowIndex, const cpo::uno::Any &nColumnIndex );

    // Attributes
    virtual cpo::uno::Any getValue() override;
    virtual cpo::uno::Any getValue2() override;
    virtual void   setValue( const cpo::uno::Any& aValue ) override;
    virtual void   setValue2( const cpo::uno::Any& aValue2 ) override;
    virtual cpo::uno::Any getFormula() override;
    virtual void   setFormula( const cpo::uno::Any& rFormula ) override;
    virtual cpo::uno::Any getFormulaArray() override;
    virtual void   setFormulaArray(const cpo::uno::Any& rFormula) override;
    virtual cpo::uno::Any getFormulaR1C1() override;
    virtual void   setFormulaR1C1( const cpo::uno::Any &rFormula ) override;
    virtual cpo::uno::Any getFormulaLocal() override;
    virtual void   setFormulaLocal( const cpo::uno::Any &rFormula ) override;
    virtual cpo::uno::Any getFormulaR1C1Local() override;
    virtual void   setFormulaR1C1Local( const cpo::uno::Any &rFormula ) override;
    virtual ::sal_Int32 getCount() override;
    virtual ::sal_Int32 getRow() override;
    virtual ::sal_Int32 getColumn() override;
    virtual OUString getText() override;
    using ScVbaRange_BASE::setNumberFormat;
    virtual void setNumberFormat( const cpo::uno::Any& rNumberFormat ) override;
    virtual cpo::uno::Any getNumberFormat() override;
    virtual void setMergeCells( const cpo::uno::Any& bMerge ) override;
    virtual cpo::uno::Any getMergeCells() override;
    virtual void setWrapText( const cpo::uno::Any& bIsWrapped ) override;
    virtual cpo::uno::Any getWrapText() override;
    virtual cpo::uno::Reference< ov::excel::XRange > getEntireRow() override;
    virtual cpo::uno::Reference< ov::excel::XRange > getEntireColumn() override;
    virtual cpo::uno::Reference< ov::excel::XComment > getComment() override;
    virtual cpo::uno::Any getHidden() override;
    virtual void setHidden( const cpo::uno::Any& _hidden ) override;
    virtual cpo::uno::Any getColumnWidth() override;
    virtual void setColumnWidth( const cpo::uno::Any& _columnwidth ) override;
    virtual cpo::uno::Any getRowHeight() override;
    virtual void setRowHeight( const cpo::uno::Any& _rowheight ) override;
    virtual cpo::uno::Any getWidth() override;
    virtual cpo::uno::Any getHeight() override;
    virtual cpo::uno::Any getTop() override;
    virtual cpo::uno::Any getLeft() override;

    virtual cpo::uno::Reference< ov::excel::XName > getName() override;
    virtual cpo::uno::Reference< ov::excel::XWorksheet > getWorksheet() override;
    virtual cpo::uno::Any getPageBreak() override;
    virtual void setPageBreak( const cpo::uno::Any& _pagebreak ) override;
    virtual cpo::uno::Reference< ov::excel::XValidation > getValidation() override;
    virtual cpo::uno::Any getPrefixCharacter() override;
    virtual cpo::uno::Any getShowDetail() override;
    virtual void setShowDetail(const cpo::uno::Any& aShowDetail) override;
    // Methods
    virtual cpo::uno::Reference< ov::excel::XComment > AddComment( const cpo::uno::Any& Text ) override;
    virtual void Clear() override;
    virtual void ClearComments() override;
    virtual void ClearContents() override;
    virtual void ClearFormats() override;
    virtual cpo::uno::Any HasFormula() override;
    virtual void FillLeft() override;
    virtual void FillRight() override;
    virtual void FillUp() override;
    virtual void FillDown() override;
    virtual cpo::uno::Reference< ov::excel::XRange > Offset( const cpo::uno::Any &nRowOffset, const cpo::uno::Any &nColOffset ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > CurrentRegion() override;
    virtual cpo::uno::Reference< ov::excel::XRange > CurrentArray() override;
    virtual OUString Characters( const cpo::uno::Any& nIndex, const cpo::uno::Any& nCount ) override;

    virtual OUString Address( const cpo::uno::Any& RowAbsolute, const cpo::uno::Any& ColumnAbsolute, const cpo::uno::Any& ReferenceStyle, const cpo::uno::Any& External, const cpo::uno::Any& RelativeTo ) override;

    virtual cpo::uno::Reference< ov::excel::XRange > Cells( const cpo::uno::Any &nRow, const cpo::uno::Any &nCol ) override;
    virtual void Select() override;
    virtual void Activate() override;
    virtual cpo::uno::Reference< ov::excel::XRange >  Rows( const cpo::uno::Any& nIndex ) override;
    virtual cpo::uno::Reference< ov::excel::XRange >  Columns( const cpo::uno::Any &nIndex ) override;
    virtual void Copy( const cpo::uno::Any& Destination ) override;
    virtual void Cut( const cpo::uno::Any& Destination ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Resize( const cpo::uno::Any& RowSize, const cpo::uno::Any& ColumnSize ) override;
    virtual cpo::uno::Reference< ov::excel::XFont > Font() override;
    virtual cpo::uno::Reference< ov::excel::XInterior > Interior(  ) override ;
    virtual cpo::uno::Reference< ov::excel::XRange > Range( const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2 ) override;
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XRange > Range( const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2, bool bForceUseInpuRangeTab );
    virtual cpo::uno::Any getCellRange(  ) override;
    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Any getCellRange( const cpo::uno::Reference< ov::excel::XRange >& rxRange );
    virtual void PasteSpecial( const cpo::uno::Any& Paste, const cpo::uno::Any& Operation, const cpo::uno::Any& SkipBlanks, const cpo::uno::Any& Transpose ) override;
    virtual bool Replace( const OUString& What, const OUString& Replacement, const cpo::uno::Any& LookAt, const cpo::uno::Any& SearchOrder, const cpo::uno::Any& MatchCase, const cpo::uno::Any& MatchByte, const cpo::uno::Any& SearchFormat, const cpo::uno::Any& ReplaceFormat ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Find( const cpo::uno::Any& What, const cpo::uno::Any& After, const cpo::uno::Any& LookIn, const cpo::uno::Any& LookAt, const cpo::uno::Any& SearchOrder, const cpo::uno::Any& SearchDirection, const cpo::uno::Any& MatchCase, const cpo::uno::Any& MatchByte, const cpo::uno::Any& SearchFormat ) override;
    virtual void Sort( const cpo::uno::Any& Key1, const cpo::uno::Any& Order1, const cpo::uno::Any& Key2, const cpo::uno::Any& Type, const cpo::uno::Any& Order2, const cpo::uno::Any& Key3, const cpo::uno::Any& Order3, const cpo::uno::Any& Header, const cpo::uno::Any& OrderCustom, const cpo::uno::Any& MatchCase, const cpo::uno::Any& Orientation, const cpo::uno::Any& SortMethod,  const cpo::uno::Any& DataOption1, const cpo::uno::Any& DataOption2, const cpo::uno::Any& DataOption3 ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > End( ::sal_Int32 Direction ) override;
    virtual cpo::uno::Reference< ov::excel::XCharacters > characters( const cpo::uno::Any& Start, const cpo::uno::Any& Length ) override;
    virtual void Delete( const cpo::uno::Any& Shift ) override;
    virtual cpo::uno::Any Areas( const cpo::uno::Any& ) override;
    virtual cpo::uno::Any Borders( const cpo::uno::Any& ) override;
    virtual cpo::uno::Any BorderAround( const cpo::uno::Any& LineStyle,
                const cpo::uno::Any& Weight, const cpo::uno::Any& ColorIndex, const cpo::uno::Any& Color ) override;
    virtual cpo::uno::Any Hyperlinks( const cpo::uno::Any& aIndex ) override;

    virtual void AutoFilter( const cpo::uno::Any& Field, const cpo::uno::Any& Criteria1, const cpo::uno::Any& Operator, const cpo::uno::Any& Criteria2, const cpo::uno::Any& VisibleDropDown ) override;
    virtual void Insert( const cpo::uno::Any& Shift, const cpo::uno::Any& CopyOrigin ) override;
    virtual void Autofit() override;
    virtual void PrintOut( const cpo::uno::Any& From, const cpo::uno::Any& To, const cpo::uno::Any& Copies, const cpo::uno::Any& Preview, const cpo::uno::Any& ActivePrinter, const cpo::uno::Any& PrintToFile, const cpo::uno::Any& Collate, const cpo::uno::Any& PrToFileName ) override;
    virtual void AutoFill( const cpo::uno::Reference< ov::excel::XRange >& Destination, const cpo::uno::Any& Type ) override ;
     void Calculate(  ) override;
    virtual void AutoOutline(  ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Item( const cpo::uno::Any& row, const cpo::uno::Any& column ) override;
    virtual void ClearOutline(  ) override;
    virtual void Ungroup(  ) override;
    virtual void Group(  ) override;
    virtual void Merge( const cpo::uno::Any& Across ) override;
    virtual void UnMerge(  ) override;
    virtual cpo::uno::Any getStyle() override;
    virtual void setStyle( const cpo::uno::Any& _style ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > Next() override;
    virtual cpo::uno::Reference< ov::excel::XRange > Previous() override;
    virtual void RemoveSubtotal(  ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > MergeArea() override;
    virtual void Subtotal( ::sal_Int32 GroupBy, ::sal_Int32 Function, const cpo::uno::Sequence< ::sal_Int32 >& TotalList, const cpo::uno::Any& Replace, const cpo::uno::Any& PageBreaks, const cpo::uno::Any& SummaryBelowData ) override;
    virtual void ExportAsFixedFormat(const cpo::uno::Any& Type, const cpo::uno::Any& FileName, const cpo::uno::Any& Quality,
        const cpo::uno::Any& IncludeDocProperties, const cpo::uno::Any& IgnorePrintAreas, const cpo::uno::Any& From,
        const cpo::uno::Any& To, const cpo::uno::Any& OpenAfterPublish, const cpo::uno::Any& FixedFormatExtClassPtr) override;

    // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration > createEnumeration() override;
    // XElementAccess
    virtual cpo::uno::Type getElementType() override
    {
        return cppu::UnoType<ov::excel::XRange>::get();

    }
    virtual bool hasElements() override;
    // XDefaultMethod
    OUString getDefaultMethodName(  ) override;
        // XDefaultProperty
        OUString getDefaultPropertyName(  ) override { return u"Value"_ustr; }

// #TODO completely rewrite ScVbaRange, it's become a hackfest
// it needs to be closer to ScCellRangeBase in that the underlying
// object model should probably be a ScRangelst.
//     * would be nice to be able to construct a range from an address only
//     * or a list of address ( multi-area )
//     * object should be a lightweight as possible
//     * we shouldn't need hacks like this below
    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Reference< ov::excel::XRange > ApplicationRange( const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, const cpo::uno::Any &Cell1, const cpo::uno::Any &Cell2 );
    static bool getCellRangesForAddress(ScRefFlags &rResFlags, std::u16string_view sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, formula::FormulaGrammar::AddressConvention eConv, char cDelimiter );
    virtual bool GoalSeek( const cpo::uno::Any& Goal, const cpo::uno::Reference< ov::excel::XRange >& ChangingCell ) override;
    virtual cpo::uno::Reference< ov::excel::XRange > SpecialCells( const cpo::uno::Any& _oType, const cpo::uno::Any& _oValue) override;
    // XErrorQuery
    virtual bool hasError(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/// @throws cpo::uno::RuntimeException
bool getScRangeListForAddress( const OUString& sName, ScDocShell* pDocSh, const ScRange& refRange,
                               ScRangeList& aCellRanges,
                               formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
