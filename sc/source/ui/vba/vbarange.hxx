/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbarange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-14 12:41:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_RANGE_HXX
#define SC_VBA_RANGE_HXX

#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <org/openoffice/excel/XRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <org/openoffice/excel/XFont.hpp>
#include <org/openoffice/excel/XComment.hpp>
#include <org/openoffice/vba/XCollection.hpp>
#include <org/openoffice/excel/XlPasteType.hdl>
#include <org/openoffice/excel/XlPasteSpecialOperation.hdl>

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

//#include "vbahelperinterface.hxx"
#include "vbaformat.hxx"

class ScTableSheetsObj;
class ScCellRangesBase;

//typedef InheritedHelperInterfaceImpl1< oo::excel::XRange >  ScVbaRange_BASE;
typedef ScVbaFormat< oo::excel::XRange > ScVbaRange_BASE;

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
    css::uno::Reference< oo::vba::XCollection > m_Areas;
    css::uno::Reference< oo::vba::XCollection > m_Borders;
    css::uno::Reference< css::table::XCellRange > mxRange;
    css::uno::Reference< css::sheet::XSheetCellRangeContainer > mxRanges;
    sal_Bool mbIsRows;
    sal_Bool mbIsColumns;
    css::uno::Reference< oo::excel::XValidation > m_xValidation;
    double getCalcColWidth( const css::table::CellRangeAddress& ) throw (css::uno::RuntimeException);
    double getCalcRowHeight( const css::table::CellRangeAddress& ) throw (css::uno::RuntimeException);
    void visitArray( ArrayVisitor& vistor );

    css::uno::Reference< oo::excel::XRange > getEntireColumnOrRow( bool bColumn = true ) throw( css::uno::RuntimeException );

    void fillSeries(  css::sheet::FillDirection nFillDirection, css::sheet::FillMode nFillMode, css::sheet::FillDateMode nFillDateMode, double fStep, double fEndValue ) throw( css::uno::RuntimeException );

    void ClearContents( sal_Int32 nFlags ) throw (css::uno::RuntimeException);
    virtual void   setValue( const css::uno::Any& aValue, ValueSetter& setter) throw ( css::uno::RuntimeException);
    virtual css::uno::Any getValue( ValueGetter& rValueGetter ) throw (css::uno::RuntimeException);
    virtual css::uno::Any getFormulaValue( ScAddress::Convention ) throw (css::uno::RuntimeException);
    virtual void   setFormulaValue( const css::uno::Any& aValue, ScAddress::Convention ) throw ( css::uno::RuntimeException);
    css::uno::Reference< oo::excel::XRange > getArea( sal_Int32 nIndex  ) throw( css::uno::RuntimeException );
    ScCellRangesBase* getCellRangesBase() throw ( css::uno::RuntimeException );
    SfxItemSet* getCurrentDataSet( )  throw ( css::uno::RuntimeException );
    css::uno::Reference< oo::vba::XCollection >& getBorders();
    void groupUnGroup( bool bUnGroup = false ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    css::uno::Reference< oo::excel::XRange > PreviousNext( bool bIsPrevious );
    css::uno::Reference< oo::excel::XRange > SpecialCellsImpl( sal_Int32 nType, const css::uno::Any& _oValue) throw ( css::script::BasicErrorException );
    css::awt::Point getPosition() throw ( css::uno::RuntimeException );
public:
    ScVbaRange( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange, sal_Bool bIsRows = false, sal_Bool bIsColumns = false ) throw ( css::lang::IllegalArgumentException );
    ScVbaRange( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XSheetCellRangeContainer >& xRanges, sal_Bool bIsRows = false, sal_Bool bIsColumns = false ) throw ( css::lang::IllegalArgumentException );
    ScVbaRange( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext ) throw ( css::lang::IllegalArgumentException );

    ScDocument* getScDocument();
    ScDocShell* getScDocShell();

    virtual ~ScVbaRange();
    virtual css::uno::Reference< oo::vba::XHelperInterface > thisHelperIface() { return this; }
    bool isSingleCellRange();
    css::uno::Reference< oo::excel::XRange > intersect( const css::uno::Reference< oo::excel::XRange >& xRange ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
        static css::uno::Reference< oo::excel::XRange > getRangeObjectForName( const css::uno::Reference< css::uno::XComponentContext >& xContext, const rtl::OUString& sRangeName, ScDocShell* pDocSh, ScAddress::Convention eConv = ScAddress::CONV_XL_A1  ) throw ( css::uno::RuntimeException );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void   SAL_CALL setValue( const css::uno::Any& aValue ) throw ( css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormula() throw (css::uno::RuntimeException);
    virtual void   SAL_CALL setFormula( const css::uno::Any& rFormula ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormulaArray() throw (css::uno::RuntimeException);
    virtual void   SAL_CALL setFormulaArray(const css::uno::Any& rFormula) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormulaR1C1() throw (css::uno::RuntimeException);
    virtual void   SAL_CALL setFormulaR1C1( const css::uno::Any &rFormula ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRow() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumn() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    using ScVbaRange_BASE::setNumberFormat;
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& rNumberFormat ) throw ( css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNumberFormat() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMergeCells( const css::uno::Any& bMerge ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getMergeCells() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setWrapText( const css::uno::Any& bIsWrapped ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getWrapText() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getEntireRow() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getEntireColumn() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XComment > SAL_CALL getComment() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getHidden() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHidden( const css::uno::Any& _hidden ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColumnWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setColumnWidth( const css::uno::Any& _columnwidth ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getRowHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRowHeight( const css::uno::Any& _rowheight ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getLeft() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getWorksheet() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getPageBreak() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPageBreak( const css::uno::Any& _pagebreak ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XValidation > SAL_CALL getValidation() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormulaHidden() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setFormulaHidden(const css::uno::Any& aHidden) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // Methods
    sal_Bool IsRows() { return mbIsRows; }
    sal_Bool IsColumns() { return mbIsColumns; }
    virtual css::uno::Reference< oo::excel::XComment > SAL_CALL AddComment( const css::uno::Any& Text ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Clear() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ClearComments() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ClearContents() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ClearFormats() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL HasFormula() throw (css::uno::RuntimeException);
    virtual void SAL_CALL FillLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL FillRight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL FillUp() throw (css::uno::RuntimeException);
    virtual void SAL_CALL FillDown() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Offset( const css::uno::Any &nRowOffset, const css::uno::Any &nColOffset )
                                                           throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL CurrentRegion() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL CurrentArray() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL Characters( const css::uno::Any& nIndex, const css::uno::Any& nCount )
                                                 throw (css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL Address( const css::uno::Any& RowAbsolute, const css::uno::Any& ColumnAbsolute, const css::uno::Any& ReferenceStyle, const css::uno::Any& External, const css::uno::Any& RelativeTo ) throw (css::uno::RuntimeException);

    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Cells( const css::uno::Any &nRow, const css::uno::Any &nCol )
                                                          throw (css::uno::RuntimeException);
    virtual void SAL_CALL Select() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange >  SAL_CALL Rows( const css::uno::Any& nIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange >  SAL_CALL Columns( const css::uno::Any &nIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Copy( const css::uno::Any& Destination ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Cut( const css::uno::Any& Destination ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Resize( const css::uno::Any& RowSize, const css::uno::Any& ColumnSize )
                                                           throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XFont > SAL_CALL Font() throw ( css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XInterior > SAL_CALL Interior(  ) throw ( css::script::BasicErrorException, css::uno::RuntimeException) ;
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > Range( const css::uno::Any &Cell1, const css::uno::Any &Cell2, bool bForceUseInpuRangeTab ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getCellRange(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL PasteSpecial( const css::uno::Any& Paste, const css::uno::Any& Operation, const css::uno::Any& SkipBlanks, const css::uno::Any& Transpose ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL Replace( const ::rtl::OUString& What, const ::rtl::OUString& Replacement, const css::uno::Any& LookAt, const css::uno::Any& SearchOrder, const css::uno::Any& MatchCase, const css::uno::Any& MatchByte, const css::uno::Any& SearchFormat, const css::uno::Any& ReplaceFormat ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Sort( const css::uno::Any& Key1, const css::uno::Any& Order1, const css::uno::Any& Key2, const css::uno::Any& Type, const css::uno::Any& Order2, const css::uno::Any& Key3, const css::uno::Any& Order3, const css::uno::Any& Header, const css::uno::Any& OrderCustom, const css::uno::Any& MatchCase, const css::uno::Any& Orientation, const css::uno::Any& SortMethod,  const css::uno::Any& DataOption1, const css::uno::Any& DataOption2, const css::uno::Any& DataOption3 ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL End( ::sal_Int32 Direction )  throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XCharacters > SAL_CALL characters( const css::uno::Any& Start, const css::uno::Any& Length ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete( const css::uno::Any& Shift ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Areas( const css::uno::Any& ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& ) throw ( css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL BorderAround( const css::uno::Any& LineStyle,
                const css::uno::Any& Weight, const css::uno::Any& ColorIndex, const css::uno::Any& Color ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL TextToColumns( const css::uno::Any& Destination, const css::uno::Any& DataType, const css::uno::Any& TextQualifier,
                const css::uno::Any& ConsecutinveDelimiter, const css::uno::Any& Tab, const css::uno::Any& Semicolon, const css::uno::Any& Comma,
                const css::uno::Any& Space, const css::uno::Any& Other, const css::uno::Any& OtherChar, const css::uno::Any& FieldInfo,
                const css::uno::Any& DecimalSeparator, const css::uno::Any& ThousandsSeparator, const css::uno::Any& TrailingMinusNumbers ) throw (css::uno::RuntimeException);

    virtual void SAL_CALL AutoFilter( const css::uno::Any& Field, const css::uno::Any& Criteria1, const css::uno::Any& Operator, const css::uno::Any& Criteria2, const css::uno::Any& VisibleDropDown ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Insert( const css::uno::Any& Shift, const css::uno::Any& CopyOrigin ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Autofit() throw (css::uno::RuntimeException);
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL AutoFill( const css::uno::Reference< oo::excel::XRange >& Destination, const css::uno::Any& Type ) throw (css::uno::RuntimeException) ;
     void SAL_CALL Calculate(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL AutoOutline(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Item( const ::css::uno::Any& row, const css::uno::Any& column ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL ClearOutline(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Ungroup(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Group(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Merge( const css::uno::Any& Across ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL UnMerge(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const css::uno::Any& _style ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Next() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Previous() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL RemoveSubtotal(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Subtotal( ::sal_Int32 GroupBy, ::sal_Int32 Function, const css::uno::Sequence< ::sal_Int32 >& TotalList, const css::uno::Any& Replace, const css::uno::Any& PageBreaks, const css::uno::Any& SummaryBelowData ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException)
    {
        return oo::excel::XRange::static_type(0);

    }
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);
    // XDefaultMethod
    ::rtl::OUString SAL_CALL getDefaultMethodName(  ) throw (css::uno::RuntimeException);
        // XDefaultProperty
        ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }


// #TODO completely rewrite ScVbaRange, its become a hackfest
// it needs to be closer to ScCellRangeBase in that the underlying
// object model should probably be a ScRangelst.
//     * would be nice to be able to construct a range from an addres only
//     * or a list of address ( multi-area )
//     * object should be a lightweight as possible
//     * we shouldn't need hacks like this below
    static css::uno::Reference< oo::excel::XRange > ApplicationRange( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL GoalSeek( const css::uno::Any& Goal, const css::uno::Reference< oo::excel::XRange >& ChangingCell ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL SpecialCells( const css::uno::Any& _oType, const css::uno::Any& _oValue) throw ( css::script::BasicErrorException );
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_RANGE_HXX */

