/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbarange.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:19:41 $
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
#include "helperdecl.hxx"

#include <comphelper/unwrapargs.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XGoalSeek.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/sheet/XCellSeries.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/TableSortField.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/util/CellProtection.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/awt/XDevice.hpp>

//#include <com/sun/star/sheet/CellDeleteMode.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/GeneralFunction.hdl>

#include <org/openoffice/excel/XlPasteSpecialOperation.hpp>
#include <org/openoffice/excel/XlPasteType.hpp>
#include <org/openoffice/excel/Constants.hpp>
#include <org/openoffice/excel/XlFindLookIn.hpp>
#include <org/openoffice/excel/XlLookAt.hpp>
#include <org/openoffice/excel/XlSearchOrder.hpp>
#include <org/openoffice/excel/XlSortOrder.hpp>
#include <org/openoffice/excel/XlYesNoGuess.hpp>
#include <org/openoffice/excel/XlSortOrientation.hpp>
#include <org/openoffice/excel/XlSortMethod.hpp>
#include <org/openoffice/excel/XlDirection.hpp>
#include <org/openoffice/excel/XlSortDataOption.hpp>
#include <org/openoffice/excel/XlDeleteShiftDirection.hpp>
#include <org/openoffice/excel/XlInsertShiftDirection.hpp>
#include <org/openoffice/excel/XlReferenceStyle.hpp>
#include <org/openoffice/excel/XlBordersIndex.hpp>
#include <org/openoffice/excel/XlPageBreak.hpp>
#include <org/openoffice/excel/XlAutoFilterOperator.hpp>
#include <org/openoffice/excel/XlAutoFillType.hpp>
#include <org/openoffice/excel/XlTextParsingType.hpp>
#include <org/openoffice/excel/XlTextQualifier.hpp>
#include <org/openoffice/excel/XlCellType.hpp>
#include <org/openoffice/excel/XlSpecialCellsValue.hpp>
#include <org/openoffice/excel/XlConsolidationFunction.hpp>

#include <scitems.hxx>
#include <svx/srchitem.hxx>
#include <cellsuno.hxx>
#include <dbcolect.hxx>
#include "docfunc.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/itemwrapper.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <unonames.hxx>

#include "vbarange.hxx"
#include "vbafont.hxx"
#include "vbacomment.hxx"
#include "vbainterior.hxx"
#include "vbacharacters.hxx"
#include "vbaborders.hxx"
#include "vbaworksheet.hxx"
#include "vbavalidation.hxx"

#include "tabvwsh.hxx"
#include "rangelst.hxx"
#include "convuno.hxx"
#include "compiler.hxx"
#include "attrib.hxx"
#include "undodat.hxx"
#include "dbdocfun.hxx"
#include "patattr.hxx"
#include <comphelper/anytostring.hxx>

#include <global.hxx>

#include "vbaglobals.hxx"
#include "vbastyle.hxx"
#include <vector>
#include <vbacollectionimpl.hxx>
// begin test includes
#include <com/sun/star/sheet/FunctionArgument.hpp>
// end test includes

#include <org/openoffice/excel/Range.hpp>

using namespace ::org::openoffice;
using namespace ::com::sun::star;
using ::std::vector;

//    * 1 point = 1/72 inch = 20 twips
//    * 1 inch = 72 points = 1440 twips
//    * 1 cm = 567 twips
double lcl_hmmToPoints( double nVal ) { return ( (double)((nVal /1000 ) * 567 ) / 20 ); }
double lcl_pointsToHmm( double nVal ) { return (double)( ( nVal * 20 ) / 567 ) * 1000; }

static const sal_Int16 supportedIndexTable[] = {  excel::XlBordersIndex::xlEdgeLeft, excel::XlBordersIndex::xlEdgeTop, excel::XlBordersIndex::xlEdgeBottom, excel::XlBordersIndex::xlEdgeRight, excel::XlBordersIndex::xlDiagonalDown, excel::XlBordersIndex::xlDiagonalUp, excel::XlBordersIndex::xlInsideVertical, excel::XlBordersIndex::xlInsideHorizontal };

USHORT lcl_pointsToTwips( double nVal )
{
    nVal = nVal * static_cast<double>(20);
    short nTwips = static_cast<short>(nVal);
    return nTwips;
}
double lcl_TwipsToPoints( USHORT nVal )
{
    double nPoints = nVal;
    return nPoints / 20;
}

double lcl_Round2DecPlaces( double nVal )
{
    nVal  = (nVal * (double)100);
    long tmp = static_cast<long>(nVal);
    if ( ( ( nVal - tmp ) >= 0.5 ) )
        ++tmp;
    nVal = tmp;
    nVal = nVal/100;
    return nVal;
}

uno::Any lcl_makeRange( uno::Reference< uno::XComponentContext >& xContext, const uno::Any aAny, bool bIsRows, bool bIsColumns )
{
    uno::Reference< table::XCellRange > xCellRange( aAny, uno::UNO_QUERY_THROW );
    // #FIXME need proper (WorkSheet) parent
    return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext, xCellRange, bIsRows, bIsColumns ) ) );
}

uno::Reference< excel::XRange > lcl_makeXRangeFromSheetCellRanges( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSheetCellRanges >& xLocSheetCellRanges, ScDocShell* pDoc )
{
    uno::Reference< excel::XRange > xRange;
    uno::Sequence< table::CellRangeAddress  > sAddresses = xLocSheetCellRanges->getRangeAddresses();
    ScRangeList aCellRanges;
    sal_Int32 nLen = sAddresses.getLength();
    for ( sal_Int32 index = 0; index < nLen; ++index )
    {
        ScRange refRange;
        ScUnoConversion::FillScRange( refRange, sAddresses[ index ] );
        aCellRanges.Append( refRange );
    }
    // Single range
    if ( aCellRanges.First() == aCellRanges.Last() )
    {
        uno::Reference< table::XCellRange > xTmpRange( new ScCellRangeObj( pDoc, *aCellRanges.First() ) );
        // #FIXME need proper (WorkSheet) parent
        xRange = new ScVbaRange( xParent, xContext, xTmpRange );
    }
    else
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDoc, aCellRanges ) );
        // #FIXME need proper (WorkSheet) parent
        xRange = new ScVbaRange( xParent, xContext, xRanges );
    }
    return xRange;
}

SfxItemSet*  ScVbaRange::getCurrentDataSet( ) throw ( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xIf;
    if ( mxRanges.is() )
        xIf.set( mxRanges, uno::UNO_QUERY_THROW );
    else
        xIf.set( mxRange, uno::UNO_QUERY_THROW );
    ScCellRangeObj* pUnoCellRange = dynamic_cast< ScCellRangeObj* >( xIf.get() );
    SfxItemSet* pDataSet =  pUnoCellRange ? pUnoCellRange->GetCurrentDataSet( true ) : NULL ;

    if ( !pDataSet )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can't access Itemset for range" ) ), uno::Reference< uno::XInterface >() );
    return pDataSet;
}

class SingleRangeEnumeration : public EnumerationHelper_BASE
{
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > mxContext;
    bool bHasMore;
public:

    SingleRangeEnumeration( const uno::Reference< css::uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException ) : m_xRange( xRange ), mxContext( xContext ), bHasMore( true ) { }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException) { return bHasMore; }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !bHasMore )
            throw container::NoSuchElementException();
        bHasMore = false;
        return uno::makeAny( m_xRange );
    }
};

// very simple class to pass to ScVbaCollectionBaseImpl containing
// just one item
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > SingleRange_BASE;

class SingleRangeIndexAccess : public SingleRange_BASE
{
private:
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > mxContext;
    SingleRangeIndexAccess(); // not defined
public:
    SingleRangeIndexAccess( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ):m_xRange( xRange ), mxContext( xContext ) {}
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (::uno::RuntimeException) { return 1; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index != 0 )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( m_xRange );
    }
        // XElementAccess
        virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException){ return table::XCellRange::static_type(0); }

        virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException) { return sal_True; }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException) { return new SingleRangeEnumeration( mxContext, m_xRange ); }

};



class RangesEnumerationImpl : public EnumerationHelperImpl
{
    bool mbIsRows;
    bool mbIsColumns;
public:

    RangesEnumerationImpl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, bool bIsRows, bool bIsColumns ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ), mbIsRows( bIsRows ), mbIsColumns( bIsColumns ) {}
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return lcl_makeRange( m_xContext, m_xEnumeration->nextElement(), mbIsRows, mbIsColumns );
    }
};


class ScVbaRangeAreas : public ScVbaCollectionBaseImpl
{
    bool mbIsRows;
    bool mbIsColumns;
public:
    ScVbaRangeAreas( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess, bool bIsRows, bool bIsColumns ) : ScVbaCollectionBaseImpl( uno::Reference< vba::XHelperInterface >(), xContext, xIndexAccess ), mbIsRows( bIsRows ), mbIsColumns( bIsColumns ) {}

    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException){ return excel::XRange::static_type(0); }

    virtual uno::Any createCollectionObject( const uno::Any& aSource );

    virtual rtl::OUString& getServiceImplName() { static rtl::OUString sDummy; return sDummy; }

    virtual uno::Sequence< rtl::OUString > getServiceNames() { return uno::Sequence< rtl::OUString >(); }

};

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaRangeAreas::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new RangesEnumerationImpl( mxContext, xEnumAccess->createEnumeration(), mbIsRows, mbIsColumns );

}

uno::Any
ScVbaRangeAreas::createCollectionObject( const uno::Any& aSource )
{
    return lcl_makeRange( mxContext, aSource, mbIsRows, mbIsColumns );
}

// assume that xIf is infact a ScCellRangesBase
ScDocShell*
getDocShellFromIf( const uno::Reference< uno::XInterface >& xIf ) throw ( uno::RuntimeException )
{
    ScCellRangesBase* pUno= dynamic_cast< ScCellRangesBase* >( xIf.get() );
    if ( !pUno )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying uno range object" ) ), uno::Reference< uno::XInterface >()  );
    return pUno->GetDocShell();
}

ScDocShell*
getDocShellFromRange( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
    // need the ScCellRangesBase to get docshell
    uno::Reference< uno::XInterface > xIf( xRange, uno::UNO_QUERY_THROW );
    return getDocShellFromIf(xIf );
}

uno::Reference< frame::XModel > getModelFromXIf( const uno::Reference< uno::XInterface >& xIf ) throw ( uno::RuntimeException )
{
    ScDocShell* pDocShell = getDocShellFromIf(xIf );
    return pDocShell->GetModel();
}

uno::Reference< frame::XModel > getModelFromRange( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xIf( xRange, uno::UNO_QUERY_THROW );
    return getModelFromXIf( xIf );
}

ScDocument*
getDocumentFromRange( const uno::Reference< table::XCellRange >& xRange )
{
    ScDocShell* pDocShell = getDocShellFromRange( xRange );
    if ( !pDocShell )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying docshell from uno range object" ) ), uno::Reference< uno::XInterface >() );
    ScDocument* pDoc = pDocShell->GetDocument();
    return pDoc;
}


ScDocument*
ScVbaRange::getScDocument()
{
    if ( mxRanges.is() )
    {
        uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW );
        uno::Reference< table::XCellRange > xRange( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        return getDocumentFromRange( xRange );
    }
    return getDocumentFromRange( mxRange );
}

ScDocShell*
ScVbaRange::getScDocShell()
{
    if ( mxRanges.is() )
    {
        uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW );
        uno::Reference< table::XCellRange > xRange( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        return getDocShellFromRange( xRange );
    }
    return getDocShellFromRange( mxRange );
}

class NumFormatHelper
{
    uno::Reference< util::XNumberFormatsSupplier > mxSupplier;
    uno::Reference< beans::XPropertySet > mxRangeProps;
    uno::Reference< util::XNumberFormats > mxFormats;
public:
    NumFormatHelper( const uno::Reference< table::XCellRange >& xRange )
    {
        mxSupplier.set( getModelFromRange( xRange ), uno::UNO_QUERY_THROW );
        mxRangeProps.set( xRange, uno::UNO_QUERY_THROW);
        mxFormats = mxSupplier->getNumberFormats();
    }
    uno::Reference< beans::XPropertySet > getNumberProps()
    {
        long nIndexKey = 0;
        uno::Any aValue = mxRangeProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat")));
        aValue >>= nIndexKey;

        if ( mxFormats.is() )
            return  mxFormats->getByKey( nIndexKey );
        return  uno::Reference< beans::XPropertySet > ();
    }

    bool isBooleanType()
    {

        if ( getNumberFormat() & util::NumberFormat::LOGICAL )
            return true;
        return false;
    }

    rtl::OUString getNumberFormatString()
    {
        uno::Reference< uno::XInterface > xIf( mxRangeProps, uno::UNO_QUERY_THROW );
        ScCellRangeObj* pUnoCellRange = dynamic_cast<  ScCellRangeObj* >( xIf.get() );
        if ( pUnoCellRange )
        {

            SfxItemSet* pDataSet =  pUnoCellRange->GetCurrentDataSet( true );
            SfxItemState eState = pDataSet->GetItemState( ATTR_VALUE_FORMAT, TRUE, NULL);
            // one of the cells in the range is not like the other ;-)
            // so return a zero length format to indicate that
            if ( eState == SFX_ITEM_DONTCARE )
                return rtl::OUString();
        }


        uno::Reference< beans::XPropertySet > xNumberProps( getNumberProps(), uno::UNO_QUERY_THROW );
        ::rtl::OUString aFormatString;
        uno::Any aString = xNumberProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FormatString")));
        aString >>= aFormatString;
        return aFormatString;
    }

    sal_Int16 getNumberFormat()
    {
        uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps();
        sal_Int16 nType = ::comphelper::getINT16(
            xNumberProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Type" ) ) );
        return nType;
    }

    bool setNumberFormat( const  rtl::OUString& rFormat )
    {
        lang::Locale aLocale;
        uno::Reference< beans::XPropertySet > xNumProps = getNumberProps();
        xNumProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aLocale;
        sal_Int32 nNewIndex = mxFormats->queryKey(rFormat, aLocale, false );
        if ( nNewIndex == -1 ) // format not defined
        {
            nNewIndex = mxFormats->addNew( rFormat, aLocale );
        }
        mxRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat") ), uno::makeAny( nNewIndex ) );
        return true;
    }

    bool setNumberFormat( sal_Int16 nType )
    {
        uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps();
        lang::Locale aLocale;
        xNumberProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aLocale;
        uno::Reference<util::XNumberFormatTypes> xTypes( mxFormats, uno::UNO_QUERY );
        if ( xTypes.is() )
        {
            sal_Int32 nNewIndex = xTypes->getStandardFormat( nType, aLocale );
               mxRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NumberFormat") ), uno::makeAny( nNewIndex ) );
            return true;
        }
        return false;
    }

};

struct CellPos
{
    CellPos():m_nRow(-1), m_nCol(-1), m_nArea(0) {};
    CellPos( sal_Int32 nRow, sal_Int32 nCol, sal_Int32 nArea ):m_nRow(nRow), m_nCol(nCol), m_nArea( nArea ) {};
sal_Int32 m_nRow;
sal_Int32 m_nCol;
sal_Int32 m_nArea;
};

typedef ::cppu::WeakImplHelper1< container::XEnumeration > CellsEnumeration_BASE;
typedef ::std::vector< CellPos > vCellPos;

class CellsEnumeration : public CellsEnumeration_BASE
{
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< vba::XCollection > m_xAreas;
    vCellPos m_CellPositions;
    vCellPos::const_iterator m_it;
    uno::Reference< table::XCellRange > getArea( sal_Int32 nVBAIndex ) throw ( uno::RuntimeException )
    {
        if ( nVBAIndex < 1 || nVBAIndex > m_xAreas->getCount() )
            throw uno::RuntimeException();
        uno::Reference< excel::XRange > xRange( m_xAreas->Item( uno::makeAny(nVBAIndex), uno::Any() ), uno::UNO_QUERY_THROW );
        ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() );
        uno::Reference< table::XCellRange > xCellRange;
        if ( !pRange )
            throw uno::RuntimeException();
        xCellRange.set( pRange->getCellRange(), uno::UNO_QUERY_THROW );;
        return xCellRange;

    }
        void populateArea( sal_Int32 nVBAIndex )
    {
        uno::Reference< table::XCellRange > xRange = getArea( nVBAIndex );
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xRange, uno::UNO_QUERY_THROW );
        sal_Int32 nRowCount =  xColumnRowRange->getRows()->getCount();
        sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
        for ( sal_Int32 i=0; i<nRowCount; ++i )
        {
            for ( sal_Int32 j=0; j<nColCount; ++j )
                m_CellPositions.push_back( CellPos( i,j,nVBAIndex ) );
        }
    }
public:
    CellsEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< vba::XCollection >& xAreas ): mxContext( xContext ), m_xAreas( xAreas )
    {
        sal_Int32 nItems = m_xAreas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
                populateArea( index );
        }
        m_it = m_CellPositions.begin();
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements() throw (::uno::RuntimeException){ return m_it != m_CellPositions.end(); }

    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        CellPos aPos = *(m_it)++;

        uno::Reference< table::XCellRange > xRangeArea = getArea( aPos.m_nArea );
        uno::Reference< table::XCellRange > xCellRange( xRangeArea->getCellByPosition(  aPos.m_nCol, aPos.m_nRow ), uno::UNO_QUERY_THROW );
        // #FIXME need proper (WorkSheet) parent
        return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( uno::Reference< vba::XHelperInterface >(), mxContext, xCellRange ) ) );

    }
};


const static ::rtl::OUString ISVISIBLE(  RTL_CONSTASCII_USTRINGPARAM( "IsVisible"));
const static ::rtl::OUString WIDTH(  RTL_CONSTASCII_USTRINGPARAM( "Width"));
const static ::rtl::OUString HEIGHT(  RTL_CONSTASCII_USTRINGPARAM( "Height"));
const static ::rtl::OUString POSITION(  RTL_CONSTASCII_USTRINGPARAM( "Position"));
const static rtl::OUString EQUALS( RTL_CONSTASCII_USTRINGPARAM("=") );
const static rtl::OUString NOTEQUALS( RTL_CONSTASCII_USTRINGPARAM("<>") );
const static rtl::OUString GREATERTHAN( RTL_CONSTASCII_USTRINGPARAM(">") );
const static rtl::OUString GREATERTHANEQUALS( RTL_CONSTASCII_USTRINGPARAM(">=") );
const static rtl::OUString LESSTHAN( RTL_CONSTASCII_USTRINGPARAM("<") );
const static rtl::OUString LESSTHANEQUALS( RTL_CONSTASCII_USTRINGPARAM("<=") );
const static rtl::OUString CONTS_HEADER( RTL_CONSTASCII_USTRINGPARAM("ContainsHeader" ));
const static rtl::OUString INSERTPAGEBREAKS( RTL_CONSTASCII_USTRINGPARAM("InsertPageBreaks" ));
const static rtl::OUString STR_ERRORMESSAGE_APPLIESTOSINGLERANGEONLY( RTL_CONSTASCII_USTRINGPARAM("The command you chose cannot be performed with multiple selections.\nSelect a single range and click the command again") );
const static rtl::OUString STR_ERRORMESSAGE_NOCELLSWEREFOUND( RTL_CONSTASCII_USTRINGPARAM("No cells were found") );
const static rtl::OUString STR_ERRORMESSAGE_APPLIESTOROWCOLUMNSONLY( RTL_CONSTASCII_USTRINGPARAM("Property only applicable for Columns and Rows") );
const static rtl::OUString CELLSTYLE( RTL_CONSTASCII_USTRINGPARAM("CellStyle") );

class CellValueSetter : public ValueSetter
{
protected:
    uno::Any maValue;
    uno::TypeClass mTypeClass;
public:
    CellValueSetter( const uno::Any& aValue );
    virtual bool processValue( const uno::Any& aValue,  const uno::Reference< table::XCell >& xCell );
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell );

};

CellValueSetter::CellValueSetter( const uno::Any& aValue ): maValue( aValue ), mTypeClass( aValue.getValueTypeClass() ) {}

void
CellValueSetter::visitNode( sal_Int32 /*i*/, sal_Int32 /*j*/, const uno::Reference< table::XCell >& xCell )
{
    processValue( maValue, xCell );
}

bool
CellValueSetter::processValue( const uno::Any& aValue, const uno::Reference< table::XCell >& xCell )
{

    bool isExtracted = false;
    switch ( aValue.getValueTypeClass() )
    {
        case  uno::TypeClass_BOOLEAN:
        {
            sal_Bool bState = sal_False;
            if ( aValue >>= bState   )
            {
                uno::Reference< table::XCellRange > xRange( xCell, uno::UNO_QUERY_THROW );
                if ( bState )
                    xCell->setValue( (double) 1 );
                else
                    xCell->setValue( (double) 0 );
                NumFormatHelper cellNumFormat( xRange );
                cellNumFormat.setNumberFormat( util::NumberFormat::LOGICAL );
            }
            break;
        }
        case uno::TypeClass_STRING:
        {
            rtl::OUString aString;
            if ( aValue >>= aString )
            {
                uno::Reference< text::XTextRange > xTextRange( xCell, uno::UNO_QUERY_THROW );
                xTextRange->setString( aString );
            }
            else
                isExtracted = false;
            break;
        }
        default:
        {
            double nDouble = 0.0;
            if ( aValue >>= nDouble )
                xCell->setValue( nDouble );
            else
                isExtracted = false;
            break;
        }
    }
    return isExtracted;

}


class CellValueGetter : public ValueGetter
{
protected:
    uno::Any maValue;
    uno::TypeClass mTypeClass;
public:
    CellValueGetter() {}
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell );
    virtual void processValue( sal_Int32 x, sal_Int32 y, const uno::Any& aValue );
    const uno::Any& getValue() const { return maValue; }

};

void
CellValueGetter::processValue(  sal_Int32 /*x*/, sal_Int32 /*y*/, const uno::Any& aValue )
{
    maValue = aValue;
}
void CellValueGetter::visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
{
    uno::Any aValue;
    table::CellContentType eType = xCell->getType();
    if( eType == table::CellContentType_VALUE || eType == table::CellContentType_FORMULA )
    {
        if ( eType == table::CellContentType_FORMULA )
        {

            rtl::OUString sFormula = xCell->getFormula();
            if ( sFormula.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("=TRUE()") ) ) )
                aValue <<= sal_True;
            else if ( sFormula.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("=FALSE()") ) ) )
                aValue <<= sal_False;
            else
            {
                uno::Reference< beans::XPropertySet > xProp( xCell, uno::UNO_QUERY_THROW );

                table::CellContentType eFormulaType = table::CellContentType_VALUE;
                // some formulas give textual results
                xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FormulaResultType" ) ) ) >>= eFormulaType;

                if ( eFormulaType == table::CellContentType_TEXT )
                {
                    uno::Reference< text::XTextRange > xTextRange(xCell, ::uno::UNO_QUERY_THROW);
                    aValue <<= xTextRange->getString();
                }
                else
                    aValue <<= xCell->getValue();
            }
        }
        else
        {
            uno::Reference< table::XCellRange > xRange( xCell, uno::UNO_QUERY_THROW );
            NumFormatHelper cellFormat( xRange );
            if ( cellFormat.isBooleanType() )
                aValue = uno::makeAny( ( xCell->getValue() != 0.0 ) );
            else
                aValue <<= xCell->getValue();
        }
    }
    if( eType == table::CellContentType_TEXT )
    {
        uno::Reference< text::XTextRange > xTextRange(xCell, ::uno::UNO_QUERY_THROW);
        aValue <<= xTextRange->getString();
    }
    processValue( x,y,aValue );
}

class CellFormulaValueSetter : public CellValueSetter
{
private:
    ScDocument*  m_pDoc;
    ScAddress::Convention m_eConv;
public:
    CellFormulaValueSetter( const uno::Any& aValue, ScDocument* pDoc, ScAddress::Convention eConv  ):CellValueSetter( aValue ),  m_pDoc( pDoc ), m_eConv( eConv ){}
protected:
    bool processValue( const uno::Any& aValue, const uno::Reference< table::XCell >& xCell )
    {
        rtl::OUString sFormula;
        if ( aValue >>= sFormula )
        {
            // get current convention
            ScAddress::Convention eConv = m_pDoc->GetAddressConvention();
            // only convert/compile 'real' formulas
            if ( eConv != m_eConv && ( sFormula.trim().indexOf('=') == 0 ) )
            {
                uno::Reference< uno::XInterface > xIf( xCell, uno::UNO_QUERY_THROW );
                ScCellRangesBase* pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xIf.get() );
                if ( pUnoRangesBase )
                {
                    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();
                    ScCompiler aCompiler( m_pDoc, aCellRanges.First()->aStart );
                    // compile the string in the format passed in
                    aCompiler.CompileString( sFormula, m_eConv );
                    // set desired convention to that of the document
                    aCompiler.SetRefConvention( eConv );
                    String sConverted;
                    aCompiler.CreateStringFromTokenArray(sConverted);
                    sFormula = EQUALS + sConverted;
                }
            }

            xCell->setFormula( sFormula );
            return true;
        }
        return false;
    }

};

class CellFormulaValueGetter : public CellValueGetter
{
private:
    ScDocument*  m_pDoc;
    ScAddress::Convention m_eConv;
public:
    CellFormulaValueGetter(ScDocument* pDoc, ScAddress::Convention eConv ) : CellValueGetter( ), m_pDoc( pDoc ), m_eConv( eConv ) {}
    virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
    {
        uno::Any aValue;
        aValue <<= xCell->getFormula();
        rtl::OUString sVal;
        aValue >>= sVal;
        uno::Reference< uno::XInterface > xIf( xCell, uno::UNO_QUERY_THROW );
        ScCellRangesBase* pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xIf.get() );
        if ( ( xCell->getType() == table::CellContentType_FORMULA ) &&
            pUnoRangesBase )
        {
            ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();
            ScCompiler aCompiler( m_pDoc, aCellRanges.First()->aStart );
            aCompiler.CompileString( sVal,  ScAddress::CONV_OOO );
            // set desired convention
            aCompiler.SetRefConvention( m_eConv );
            String sConverted;
            aCompiler.CreateStringFromTokenArray(sConverted);
            sVal = EQUALS + sConverted;
            aValue <<= sVal;
        }

        processValue( x,y,aValue );
    }

};


class Dim2ArrayValueGetter : public ArrayVisitor
{
protected:
    uno::Any maValue;
    ValueGetter& mValueGetter;
    virtual void processValue( sal_Int32 x, sal_Int32 y, const uno::Any& aValue )
    {
        uno::Sequence< uno::Sequence< uno::Any > >& aMatrix = *( uno::Sequence< uno::Sequence< uno::Any > >* )( maValue.getValue() );
        aMatrix[x][y] = aValue;
    }

public:
    Dim2ArrayValueGetter(sal_Int32 nRowCount, sal_Int32 nColCount, ValueGetter& rValueGetter ): mValueGetter(rValueGetter)
    {
        uno::Sequence< uno::Sequence< uno::Any > > aMatrix;
        aMatrix.realloc( nRowCount );
        for ( sal_Int32 index = 0; index < nRowCount; ++index )
            aMatrix[index].realloc( nColCount );
        maValue <<= aMatrix;
    }
    void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )

    {
        mValueGetter.visitNode( x, y, xCell );
        processValue( x, y, mValueGetter.getValue() );
    }
    const uno::Any& getValue() const { return maValue; }

};

const static rtl::OUString sNA = rtl::OUString::createFromAscii("#N/A");

class Dim1ArrayValueSetter : public ArrayVisitor
{
    uno::Sequence< uno::Any > aMatrix;
    sal_Int32 nColCount;
    ValueSetter& mCellValueSetter;
public:
    Dim1ArrayValueSetter( const uno::Any& aValue, ValueSetter& rCellValueSetter ):mCellValueSetter( rCellValueSetter )
    {
        aValue >>= aMatrix;
        nColCount = aMatrix.getLength();
    }
    virtual void visitNode( sal_Int32 /*x*/, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
    {
        if ( y < nColCount )
            mCellValueSetter.processValue( aMatrix[ y ], xCell );
        else
            mCellValueSetter.processValue( uno::makeAny( sNA ), xCell );
    }
};



class Dim2ArrayValueSetter : public ArrayVisitor
{
    uno::Sequence< uno::Sequence< uno::Any > > aMatrix;
    ValueSetter& mCellValueSetter;
    sal_Int32 nRowCount;
    sal_Int32 nColCount;
public:
    Dim2ArrayValueSetter( const uno::Any& aValue, ValueSetter& rCellValueSetter ) : mCellValueSetter( rCellValueSetter )
    {
        aValue >>= aMatrix;
        nRowCount = aMatrix.getLength();
        nColCount = aMatrix[0].getLength();
    }

    virtual void visitNode( sal_Int32 x, sal_Int32 y, const uno::Reference< table::XCell >& xCell )
    {
        if ( x < nRowCount && y < nColCount )
            mCellValueSetter.processValue( aMatrix[ x ][ y ], xCell );
        else
            mCellValueSetter.processValue( uno::makeAny( sNA ), xCell );

    }
};

class RangeProcessor
{
public:
    virtual void process( const uno::Reference< excel::XRange >& xRange ) = 0;
};

class RangeValueProcessor : public RangeProcessor
{
    const uno::Any& m_aVal;
public:
    RangeValueProcessor( const uno::Any& rVal ):m_aVal( rVal ) {}
    virtual void process( const uno::Reference< excel::XRange >& xRange )
    {
        xRange->setValue( m_aVal );
    }
};

class RangeFormulaProcessor : public RangeProcessor
{
    const uno::Any& m_aVal;
public:
    RangeFormulaProcessor( const uno::Any& rVal ):m_aVal( rVal ) {}
    virtual void process( const uno::Reference< excel::XRange >& xRange )
    {
        xRange->setFormula( m_aVal );
    }
};

class RangeCountProcessor : public RangeProcessor
{
    sal_Int32 nCount;
public:
    RangeCountProcessor():nCount(0){}
    virtual void process( const uno::Reference< excel::XRange >& xRange )
    {
        nCount = nCount + xRange->getCount();
    }
    sal_Int32 value() { return nCount; }
};
class AreasVisitor
{
private:
    uno::Reference< vba::XCollection > m_Areas;
public:
    AreasVisitor( const uno::Reference< vba::XCollection >& rAreas ):m_Areas( rAreas ){}

    void visit( RangeProcessor& processor )
    {
        if ( m_Areas.is() )
        {
            sal_Int32 nItems = m_Areas->getCount();
            for ( sal_Int32 index=1; index <= nItems; ++index )
            {
                uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
                processor.process( xRange );
            }
        }
    }
};

class RangeHelper
{
    uno::Reference< table::XCellRange > m_xCellRange;

public:
    RangeHelper( const uno::Reference< table::XCellRange >& xCellRange ) throw (uno::RuntimeException) : m_xCellRange( xCellRange )
    {
        if ( !m_xCellRange.is() )
            throw uno::RuntimeException();
    }
    RangeHelper( const uno::Any aCellRange ) throw (uno::RuntimeException)
    {
        m_xCellRange.set( aCellRange, uno::UNO_QUERY_THROW );
    }
    uno::Reference< sheet::XSheetCellRange > getSheetCellRange() throw (uno::RuntimeException)
    {
        return uno::Reference< sheet::XSheetCellRange >(m_xCellRange, uno::UNO_QUERY_THROW);
    }
    uno::Reference< sheet::XSpreadsheet >  getSpreadSheet() throw (uno::RuntimeException)
    {
        return getSheetCellRange()->getSpreadsheet();
    }

    uno::Reference< table::XCellRange > getCellRangeFromSheet() throw (uno::RuntimeException)
    {
        return uno::Reference< table::XCellRange >(getSpreadSheet(), uno::UNO_QUERY_THROW );
    }

    uno::Reference< sheet::XCellRangeAddressable >  getCellRangeAddressable() throw (uno::RuntimeException)
    {
        return uno::Reference< sheet::XCellRangeAddressable >(m_xCellRange, ::uno::UNO_QUERY_THROW);

    }

    uno::Reference< sheet::XSheetCellCursor > getSheetCellCursor() throw ( uno::RuntimeException )
    {
        return  uno::Reference< sheet::XSheetCellCursor >( getSpreadSheet()->createCursorByRange( getSheetCellRange() ), uno::UNO_QUERY_THROW );
    }

    static uno::Reference< excel::XRange > createRangeFromRange( const uno::Reference<uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange, const uno::Reference< sheet::XCellRangeAddressable >& xCellRangeAddressable, sal_Int32 nStartColOffset = 0, sal_Int32 nStartRowOffset = 0,
 sal_Int32 nEndColOffset = 0, sal_Int32 nEndRowOffset = 0 )
    {
        // #FIXME need proper (WorkSheet) parent
        return uno::Reference< excel::XRange >( new ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext,
            xRange->getCellRangeByPosition(
                xCellRangeAddressable->getRangeAddress().StartColumn + nStartColOffset,
                xCellRangeAddressable->getRangeAddress().StartRow + nStartRowOffset,
                xCellRangeAddressable->getRangeAddress().EndColumn + nEndColOffset,
                xCellRangeAddressable->getRangeAddress().EndRow + nEndRowOffset ) ) );
    }

};

bool
getCellRangesForAddress( USHORT& rResFlags, const rtl::OUString& sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, ScAddress::Convention& eConv )
{

    ScDocument* pDoc = NULL;
    if ( pDocSh )
    {
        pDoc = pDocSh->GetDocument();
        String aString(sAddress);
        USHORT nMask = SCA_VALID;
        //USHORT nParse = rCellRanges.Parse( sAddress, pDoc, nMask, ScAddress::CONV_XL_A1 );
        rResFlags = rCellRanges.Parse( sAddress, pDoc, nMask, eConv, 0 );
        if ( rResFlags & SCA_VALID )
        {
            return true;
        }
    }
    return false;
}

bool getScRangeListForAddress( const rtl::OUString& sName, ScDocShell* pDocSh, ScRange& refRange, ScRangeList& aCellRanges, ScAddress::Convention aConv = ScAddress::CONV_XL_A1 ) throw ( uno::RuntimeException )
{
    // see if there is a match with a named range
    uno::Reference< beans::XPropertySet > xProps( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NamedRanges") ) ), uno::UNO_QUERY_THROW );
    // Strangly enough you can have Range( "namedRange1, namedRange2, etc," )
    // loop around each ',' seperated name
    std::vector< rtl::OUString > vNames;
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OUString aToken = sName.getToken( 0, ',', nIndex );
        vNames.push_back( aToken );
    } while ( nIndex >= 0 );

    if ( !vNames.size() )
        vNames.push_back( sName );

    std::vector< rtl::OUString >::iterator it = vNames.begin();
    std::vector< rtl::OUString >::iterator it_end = vNames.end();
    for ( ; it != it_end; ++it )
    {

        ScAddress::Convention eConv = aConv;
        // spaces are illegal ( but the user of course can enter them )
        rtl::OUString sAddress = (*it).trim();
        if ( xNameAccess->hasByName( sAddress ) )
        {
            uno::Reference< sheet::XNamedRange > xNamed( xNameAccess->getByName( sAddress ), uno::UNO_QUERY_THROW );
            sAddress = xNamed->getContent();
            // As the address comes from OOO, the addressing
            // style is may not be XL_A1
            eConv = pDocSh->GetDocument()->GetAddressConvention();
        }

        USHORT nFlags = 0;
        if ( !getCellRangesForAddress( nFlags, sAddress, pDocSh, aCellRanges, eConv ) )
            return false;

        bool bTabFromReferrer = !( nFlags & SCA_TAB_3D );

        for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
        {
            pRange->aStart.SetCol( refRange.aStart.Col() + pRange->aStart.Col() );
            pRange->aStart.SetRow( refRange.aStart.Row() + pRange->aStart.Row() );
            pRange->aStart.SetTab( bTabFromReferrer ? refRange.aStart.Tab()  : pRange->aStart.Tab() );
            pRange->aEnd.SetCol( refRange.aStart.Col() + pRange->aEnd.Col() );
            pRange->aEnd.SetRow( refRange.aStart.Row() + pRange->aEnd.Row() );
            pRange->aEnd.SetTab( bTabFromReferrer ? refRange.aEnd.Tab()  : pRange->aEnd.Tab() );
        }
    }
    return true;
}


ScVbaRange*
getRangeForName( const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sName, ScDocShell* pDocSh, table::CellRangeAddress& pAddr, ScAddress::Convention eConv = ScAddress::CONV_XL_A1 ) throw ( uno::RuntimeException )
{
    ScRangeList aCellRanges;
    ScRange refRange;
    ScUnoConversion::FillScRange( refRange, pAddr );
    if ( !getScRangeListForAddress ( sName, pDocSh, refRange, aCellRanges, eConv ) )
        throw uno::RuntimeException();
    // Single range
    if ( aCellRanges.First() == aCellRanges.Last() )
    {
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pDocSh, *aCellRanges.First() ) );
        // #FIXME need proper (WorkSheet) parent
        return new ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext, xRange );
    }
    uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDocSh, aCellRanges ) );

    // #FIXME need proper (WorkSheet) parent
    return new ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext, xRanges );
}

css::uno::Reference< excel::XRange >
ScVbaRange::getRangeObjectForName( const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sRangeName, ScDocShell* pDocSh, ScAddress::Convention eConv ) throw ( uno::RuntimeException )
{
    table::CellRangeAddress refAddr;
    return getRangeForName( xContext, sRangeName, pDocSh, refAddr, eConv );
}


table::CellRangeAddress getCellRangeAddressForVBARange( const uno::Any& aParam, ScDocShell* pDocSh,  ScAddress::Convention aConv = ScAddress::CONV_XL_A1) throw ( uno::RuntimeException )
{
    uno::Reference< table::XCellRange > xRangeParam;
    switch ( aParam.getValueTypeClass() )
    {
        case uno::TypeClass_STRING:
        {
            rtl::OUString rString;
            aParam >>= rString;
            ScRangeList aCellRanges;
            ScRange refRange;
            if ( getScRangeListForAddress ( rString, pDocSh, refRange, aCellRanges, aConv ) )
            {
                if ( aCellRanges.First() == aCellRanges.Last() )
                {
                    table::CellRangeAddress aRangeAddress;
                    ScUnoConversion::FillApiRange( aRangeAddress, *aCellRanges.First() );
                    return aRangeAddress;
                }
            }
        }
        case uno::TypeClass_INTERFACE:
        {
            uno::Reference< excel::XRange > xRange;
            aParam >>= xRange;
            if ( xRange.is() )
                xRange->getCellRange() >>= xRangeParam;
            break;
        }
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Can't extact CellRangeAddress from type" ) ), uno::Reference< uno::XInterface >() );
    }
    uno::Reference< sheet::XCellRangeAddressable > xAddressable( xRangeParam, uno::UNO_QUERY_THROW );
    return xAddressable->getRangeAddress();

}

uno::Reference< vba::XCollection >
lcl_setupBorders( const uno::Reference< excel::XRange >& xParentRange, const uno::Reference<uno::XComponentContext>& xContext,  const uno::Reference< table::XCellRange >& xRange  ) throw( uno::RuntimeException )
{
    uno::Reference< vba::XHelperInterface > xParent( xParentRange, uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocumentFromRange(xRange);
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );
    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
     uno::Reference< vba::XCollection > borders( new ScVbaBorders( xParent, xContext, xRange, aPalette ) );
    return borders;
}

ScVbaRange::ScVbaRange( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext )  throw ( lang::IllegalArgumentException ) : ScVbaRange_BASE( getXSomethingFromArgs< vba::XHelperInterface >( args, 0 ), xContext, getXSomethingFromArgs< beans::XPropertySet >( args, 1, false ), getModelFromRange( getXSomethingFromArgs< table::XCellRange >( args, 1 ) ), true ), mbIsRows( sal_False ), mbIsColumns( sal_False )
{
    mxRange.set( mxPropertySet, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndex( new SingleRangeIndexAccess( mxContext, mxRange ) );
    m_Areas = new ScVbaRangeAreas( mxContext, xIndex, mbIsRows, mbIsColumns );
}

ScVbaRange::ScVbaRange( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange, sal_Bool bIsRows, sal_Bool bIsColumns ) throw( lang::IllegalArgumentException )
: ScVbaRange_BASE( xParent, xContext, uno::Reference< beans::XPropertySet >( xRange, uno::UNO_QUERY_THROW ), getModelFromRange( xRange), true ), mxRange( xRange ),
                mbIsRows( bIsRows ),
                mbIsColumns( bIsColumns )
{
    if  ( !xContext.is() )
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "context is not set " ) ), uno::Reference< uno::XInterface >() , 1 );
    if  ( !xRange.is() )
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "range is not set " ) ), uno::Reference< uno::XInterface >() , 1 );

    uno::Reference< container::XIndexAccess > xIndex( new SingleRangeIndexAccess( mxContext, xRange ) );
    m_Areas = new ScVbaRangeAreas( mxContext, xIndex, mbIsRows, mbIsColumns );

}

ScVbaRange::ScVbaRange( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges,  sal_Bool bIsRows, sal_Bool bIsColumns  ) throw ( lang::IllegalArgumentException )
: ScVbaRange_BASE( xParent, xContext, uno::Reference< beans::XPropertySet >( xRanges, uno::UNO_QUERY_THROW ), getModelFromXIf( uno::Reference< uno::XInterface >( xRanges, uno::UNO_QUERY_THROW ) ), true ), mxRanges( xRanges ),mbIsRows( bIsRows ), mbIsColumns( bIsColumns )

{
    uno::Reference< container::XIndexAccess >  xIndex( mxRanges, uno::UNO_QUERY_THROW );
    m_Areas  = new ScVbaRangeAreas( mxContext, xIndex, mbIsRows, mbIsColumns );

}

ScVbaRange::~ScVbaRange()
{
}

uno::Reference< vba::XCollection >& ScVbaRange::getBorders()
{
    if ( !m_Borders.is() )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
        m_Borders = lcl_setupBorders( this, mxContext, uno::Reference< table::XCellRange >( xRange->getCellRange(), uno::UNO_QUERY_THROW ) );
    }
    return m_Borders;
}

void
ScVbaRange::visitArray( ArrayVisitor& visitor )
{
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
    sal_Int32 nRowCount = xColumnRowRange->getRows()->getCount();
    sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
    for ( sal_Int32 i=0; i<nRowCount; ++i )
    {
        for ( sal_Int32 j=0; j<nColCount; ++j )
        {
            uno::Reference< table::XCell > xCell( mxRange->getCellByPosition( j, i ), uno::UNO_QUERY_THROW );

            visitor.visitNode( i, j, xCell );
        }
    }
}



uno::Any
ScVbaRange::getValue( ValueGetter& valueGetter) throw (uno::RuntimeException)
{
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
    // single cell range
    if ( isSingleCellRange() )
    {
        visitArray( valueGetter );
        return valueGetter.getValue();
    }
    sal_Int32 nRowCount = xColumnRowRange->getRows()->getCount();
    sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();
    // multi cell range ( return array )
    Dim2ArrayValueGetter arrayGetter( nRowCount, nColCount, valueGetter );
    visitArray( arrayGetter );
    return uno::makeAny( script::ArrayWrapper( sal_False, arrayGetter.getValue() ) );
}

uno::Any SAL_CALL
ScVbaRange::getValue() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getValue();
    }

    CellValueGetter valueGetter;
    return getValue( valueGetter );

}


void
ScVbaRange::setValue(  const uno::Any  &aValue,  ValueSetter& valueSetter ) throw (uno::RuntimeException)
{
    uno::TypeClass aClass = aValue.getValueTypeClass();
    if ( aClass == uno::TypeClass_SEQUENCE )
    {
        uno::Reference< script::XTypeConverter > xConverter = getTypeConverter( mxContext );
        uno::Any aConverted;
        try
        {
            // test for single dimension, could do
            // with a better test than this
            if ( aValue.getValueTypeName().indexOf('[') ==  aValue.getValueTypeName().lastIndexOf('[') )
            {
                aConverted = xConverter->convertTo( aValue, getCppuType((uno::Sequence< uno::Any >*)0) );
                Dim1ArrayValueSetter setter( aConverted, valueSetter );
                visitArray( setter );
            }
            else
            {
                aConverted = xConverter->convertTo( aValue, getCppuType((uno::Sequence< uno::Sequence< uno::Any > >*)0) );
                Dim2ArrayValueSetter setter( aConverted, valueSetter );
                visitArray( setter );
            }
        }
        catch ( uno::Exception& e )
        {
            OSL_TRACE("Bahhh, caught exception %s",
                rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    else
    {
        visitArray( valueSetter );
    }
}

void SAL_CALL
ScVbaRange::setValue( const uno::Any  &aValue ) throw (uno::RuntimeException)
{
    // If this is a multiple selection apply setValue over all areas
    if ( m_Areas->getCount() > 1 )
    {
        AreasVisitor aVisitor( m_Areas );
        RangeValueProcessor valueProcessor( aValue );
        aVisitor.visit( valueProcessor );
        return;
    }
    CellValueSetter valueSetter( aValue );
    setValue( aValue, valueSetter );
}

void
ScVbaRange::Clear() throw (uno::RuntimeException)
{
    sal_Int32 nFlags = sheet::CellFlags::VALUE | sheet::CellFlags::STRING | sheet::CellFlags::HARDATTR | sheet::CellFlags::FORMATTED | sheet::CellFlags::EDITATTR | sheet::CellFlags::FORMULA;
    ClearContents( nFlags );
}

//helper ClearContent
void
ScVbaRange::ClearContents( sal_Int32 nFlags ) throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() );
            if ( pRange )
                pRange->ClearContents( nFlags );
        }
        return;
    }


    uno::Reference< sheet::XSheetOperation > xSheetOperation(mxRange, uno::UNO_QUERY_THROW);
    xSheetOperation->clearContents( nFlags );
}
void
ScVbaRange::ClearComments() throw (uno::RuntimeException)
{
    ClearContents( sheet::CellFlags::ANNOTATION );
}

void
ScVbaRange::ClearContents() throw (uno::RuntimeException)
{
    sal_Int32 nClearFlags = ( sheet::CellFlags::VALUE |
        sheet::CellFlags::STRING |  sheet::CellFlags::DATETIME |
        sheet::CellFlags::FORMULA );
    ClearContents( nClearFlags );
}

void
ScVbaRange::ClearFormats() throw (uno::RuntimeException)
{
    //FIXME: need to check if we need to combine sheet::CellFlags::FORMATTED
    sal_Int32 nClearFlags = sheet::CellFlags::HARDATTR | sheet::CellFlags::FORMATTED | sheet::CellFlags::EDITATTR;
    ClearContents( nClearFlags );
}

void
ScVbaRange::setFormulaValue( const uno::Any& rFormula, ScAddress::Convention eConv ) throw (uno::RuntimeException)
{
    // If this is a multiple selection apply setFormula over all areas
    if ( m_Areas->getCount() > 1 )
    {
        AreasVisitor aVisitor( m_Areas );
        RangeFormulaProcessor valueProcessor( rFormula );
        aVisitor.visit( valueProcessor );
        return;
    }
    CellFormulaValueSetter formulaValueSetter( rFormula, getScDocument(), eConv );
    setValue( rFormula, formulaValueSetter );
}

uno::Any
ScVbaRange::getFormulaValue( ScAddress::Convention eConv) throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getFormula();
    }
    CellFormulaValueGetter valueGetter( getScDocument(), eConv );
    return getValue( valueGetter );

}

void
ScVbaRange::setFormula(const uno::Any &rFormula ) throw (uno::RuntimeException)
{
    // #FIXME converting "=$a$1" e.g. CONV_XL_A1 -> CONV_OOO                            // results in "=$a$1:a1", temporalily disable conversion
    //setFormulaValue( rFormula, ScAddress::CONV_XL_A1 );;
    setFormulaValue( rFormula, ScAddress::CONV_OOO );;
}

uno::Any
ScVbaRange::getFormulaR1C1() throw (::com::sun::star::uno::RuntimeException)
{
    return getFormulaValue( ScAddress::CONV_XL_R1C1 );
}

void
ScVbaRange::setFormulaR1C1(const uno::Any& rFormula ) throw (uno::RuntimeException)
{
    setFormulaValue( rFormula, ScAddress::CONV_XL_R1C1 );
}

uno::Any
ScVbaRange::getFormula() throw (::com::sun::star::uno::RuntimeException)
{
    return getFormulaValue( ScAddress::CONV_XL_A1 );
}

sal_Int32
ScVbaRange::getCount() throw (uno::RuntimeException)
{
    // If this is a multiple selection apply setValue over all areas
    if ( m_Areas->getCount() > 1 )
    {
        AreasVisitor aVisitor( m_Areas );
        RangeCountProcessor valueProcessor;
        aVisitor.visit( valueProcessor );
        return valueProcessor.value();
    }
    sal_Int32 rowCount = 0;
    sal_Int32 colCount = 0;
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );
    rowCount = xColumnRowRange->getRows()->getCount();
    colCount = xColumnRowRange->getColumns()->getCount();

    if( IsRows() )
        return rowCount;
    if( IsColumns() )
        return colCount;
    return rowCount * colCount;
}

sal_Int32
ScVbaRange::getRow() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getRow();
    }
    uno::Reference< sheet::XCellAddressable > xCellAddressable(mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    return xCellAddressable->getCellAddress().Row + 1; // Zero value indexing
}

sal_Int32
ScVbaRange::getColumn() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getColumn();
    }
    uno::Reference< sheet::XCellAddressable > xCellAddressable(mxRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    return xCellAddressable->getCellAddress().Column + 1; // Zero value indexing
}

uno::Any
ScVbaRange::HasFormula() throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        uno::Any aResult = aNULL();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            // if the HasFormula for any area is different to another
            // return null
            if ( index > 1 )
                if ( aResult != xRange->HasFormula() )
                    return aNULL();
            aResult = xRange->HasFormula();
            if ( aNULL() == aResult )
                return aNULL();
        }
        return aResult;
    }
    uno::Reference< uno::XInterface > xIf( mxRange, uno::UNO_QUERY_THROW );
    ScCellRangesBase* pThisRanges = dynamic_cast< ScCellRangesBase * > ( xIf.get() );
    if ( pThisRanges )
    {
        uno::Reference<uno::XInterface>  xRanges( pThisRanges->queryFormulaCells( ( sheet::FormulaResult::ERROR | sheet::FormulaResult::VALUE |  sheet::FormulaResult::STRING ) ), uno::UNO_QUERY_THROW );
        ScCellRangesBase* pFormulaRanges = dynamic_cast< ScCellRangesBase * > ( xRanges.get() );
        // check if there are no formula cell, return false
        if ( pFormulaRanges->GetRangeList().Count() == 0 )
            return uno::makeAny(sal_False);

        // chech if there are holes (where some cells are not formulas)
        // or returned range is not equal to this range
        if ( ( pFormulaRanges->GetRangeList().Count() > 1 )
        || ( pFormulaRanges->GetRangeList().GetObject(0)->aStart != pThisRanges->GetRangeList().GetObject(0)->aStart )
        || ( pFormulaRanges->GetRangeList().GetObject(0)->aEnd != pThisRanges->GetRangeList().GetObject(0)->aEnd ) )
            return aNULL(); // should return aNULL;
    }
    return uno::makeAny( sal_True );
}
void
ScVbaRange::fillSeries( sheet::FillDirection nFillDirection, sheet::FillMode nFillMode, sheet::FillDateMode nFillDateMode, double fStep, double fEndValue ) throw( uno::RuntimeException )
{
    if ( m_Areas->getCount() > 1 )
    {
        // Multi-Area Range
        uno::Reference< vba::XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
        for ( sal_Int32 index = 1; index <= xCollection->getCount(); ++index )
        {
            uno::Reference< excel::XRange > xRange( xCollection->Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
            ScVbaRange* pThisRange = dynamic_cast< ScVbaRange* >( xRange.get() );
            pThisRange->fillSeries( nFillDirection, nFillMode, nFillDateMode, fStep, fEndValue );

        }
        return;
    }

    uno::Reference< sheet::XCellSeries > xCellSeries(mxRange, uno::UNO_QUERY_THROW );
    xCellSeries->fillSeries( nFillDirection, nFillMode, nFillDateMode, fStep, fEndValue );
}

void
ScVbaRange::FillLeft() throw (uno::RuntimeException)
{
    fillSeries(sheet::FillDirection_TO_LEFT,
        sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void
ScVbaRange::FillRight() throw (uno::RuntimeException)
{
    fillSeries(sheet::FillDirection_TO_RIGHT,
        sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void
ScVbaRange::FillUp() throw (uno::RuntimeException)
{
    fillSeries(sheet::FillDirection_TO_TOP,
        sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

void
ScVbaRange::FillDown() throw (uno::RuntimeException)
{
    fillSeries(sheet::FillDirection_TO_BOTTOM,
        sheet::FillMode_SIMPLE, sheet::FillDateMode_FILL_DATE_DAY, 0, 0x7FFFFFFF);
}

::rtl::OUString
ScVbaRange::getText() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getText();
    }
    uno::Reference< text::XTextRange > xTextRange(mxRange->getCellByPosition(0,0), uno::UNO_QUERY_THROW );
    return xTextRange->getString();
}

uno::Reference< excel::XRange >
ScVbaRange::Offset( const ::uno::Any &nRowOff, const uno::Any &nColOff ) throw (uno::RuntimeException)
{
    SCROW nRowOffset = 0;
    SCCOL nColOffset = 0;
    sal_Bool bIsRowOffset = ( nRowOff >>= nRowOffset );
    sal_Bool bIsColumnOffset = ( nColOff >>= nColOffset );
    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();

    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();


    for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
    {
        if ( bIsColumnOffset )
        {
            pRange->aStart.SetCol( pRange->aStart.Col() + nColOffset );
            pRange->aEnd.SetCol( pRange->aEnd.Col() + nColOffset );
        }
        if ( bIsRowOffset )
        {
            pRange->aStart.SetRow( pRange->aStart.Row() + nRowOffset );
            pRange->aEnd.SetRow( pRange->aEnd.Row() + nRowOffset );
        }
    }

    if ( aCellRanges.Count() > 1 ) // Multi-Area
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );
        return new ScVbaRange( getParent(), mxContext, xRanges );
    }
    // normal range
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.First() ) );
    return new ScVbaRange( getParent(), mxContext, xRange  );
}

uno::Reference< excel::XRange >
ScVbaRange::CurrentRegion() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->CurrentRegion();
    }

    RangeHelper helper( mxRange );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor =
        helper.getSheetCellCursor();
    xSheetCellCursor->collapseToCurrentRegion();
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
    return RangeHelper::createRangeFromRange( mxContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );
}

uno::Reference< excel::XRange >
ScVbaRange::CurrentArray() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->CurrentArray();
    }
    RangeHelper helper( mxRange );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor =
        helper.getSheetCellCursor();
    xSheetCellCursor->collapseToCurrentArray();
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
    return RangeHelper::createRangeFromRange( mxContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );
}

uno::Any
ScVbaRange::getFormulaArray() throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->getFormulaArray();
    }

    uno::Reference< sheet::XCellRangeFormula> xCellRangeFormula( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< script::XTypeConverter > xConverter = getTypeConverter( mxContext );
    uno::Any aMatrix;
    aMatrix = xConverter->convertTo( uno::makeAny( xCellRangeFormula->getFormulaArray() ) , getCppuType((uno::Sequence< uno::Sequence< uno::Any > >*)0)  ) ;
    return aMatrix;
}

void
ScVbaRange::setFormulaArray(const uno::Any& rFormula) throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->setFormulaArray( rFormula );
    }
    // #TODO need to distinguish between getFormula and getFormulaArray e.g. (R1C1)
    // but for the moment its just easier to treat them the same for setting

    setFormula( rFormula );
}

::rtl::OUString
ScVbaRange::Characters(const uno::Any& Start, const uno::Any& Length) throw (uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->Characters( Start, Length );
    }

    long nIndex = 0, nCount = 0;
    ::rtl::OUString rString;
    uno::Reference< text::XTextRange > xTextRange(mxRange, ::uno::UNO_QUERY_THROW );
    rString = xTextRange->getString();
    if( !( Start >>= nIndex ) && !( Length >>= nCount ) )
        return rString;
    if(!( Start >>= nIndex ) )
        nIndex = 1;
    if(!( Length >>= nCount ) )
        nIndex = rString.getLength();
    return rString.copy( --nIndex, nCount ); // Zero value indexing
}

::rtl::OUString
ScVbaRange::Address(  const uno::Any& RowAbsolute, const uno::Any& ColumnAbsolute, const uno::Any& ReferenceStyle, const uno::Any& External, const uno::Any& RelativeTo ) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        // Multi-Area Range
        rtl::OUString sAddress;
        uno::Reference< vba::XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
                uno::Any aExternalCopy = External;
        for ( sal_Int32 index = 1; index <= xCollection->getCount(); ++index )
        {
            uno::Reference< excel::XRange > xRange( xCollection->Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
            if ( index > 1 )
            {
                sAddress += rtl::OUString( ',' );
                                // force external to be false
                                // only first address should have the
                                // document and sheet specifications
                                aExternalCopy = uno::makeAny(sal_False);
            }
            sAddress += xRange->Address( RowAbsolute, ColumnAbsolute, ReferenceStyle, aExternalCopy, RelativeTo );
        }
        return sAddress;

    }
    ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
    if ( ReferenceStyle.hasValue() )
    {
        sal_Int32 refStyle = excel::XlReferenceStyle::xlA1;
        ReferenceStyle >>= refStyle;
        if ( refStyle == excel::XlReferenceStyle::xlR1C1 )
            dDetails = ScAddress::Details( ScAddress::CONV_XL_R1C1, 0, 0 );
    }
    USHORT nFlags = SCA_VALID;
    ScDocShell* pDocShell =  getScDocShell();
    ScDocument* pDoc =  pDocShell->GetDocument();

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    ScRange aRange( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCROW >( thisAddress.StartRow ), static_cast< SCTAB >( thisAddress.Sheet ), static_cast< SCCOL >( thisAddress.EndColumn ), static_cast< SCROW >( thisAddress.EndRow ), static_cast< SCTAB >( thisAddress.Sheet ) );
    String sRange;
    USHORT ROW_ABSOLUTE = ( SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE );
    USHORT COL_ABSOLUTE = ( SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE );
    // default
    nFlags |= ( SCA_TAB_ABSOLUTE | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE );
    if ( RowAbsolute.hasValue() )
    {
        sal_Bool bVal = sal_True;
        RowAbsolute >>= bVal;
        if ( !bVal )
            nFlags &= ~ROW_ABSOLUTE;
    }
    if ( ColumnAbsolute.hasValue() )
    {
        sal_Bool bVal = sal_True;
        ColumnAbsolute >>= bVal;
        if ( !bVal )
            nFlags &= ~COL_ABSOLUTE;
    }
    sal_Bool bLocal = sal_False;
    if ( External.hasValue() )
    {
        External >>= bLocal;
        if (  bLocal )
            nFlags |= SCA_TAB_3D | SCA_FORCE_DOC;
    }
    if ( RelativeTo.hasValue() )
    {
        // #TODO should I throw an error if R1C1 is not set?

        table::CellRangeAddress refAddress = getCellRangeAddressForVBARange( RelativeTo, pDocShell );
        dDetails = ScAddress::Details( ScAddress::CONV_XL_R1C1, static_cast< SCROW >( refAddress.StartRow ), static_cast< SCCOL >( refAddress.StartColumn ) );
    }
    aRange.Format( sRange,  nFlags, pDoc, dDetails );
    return sRange;
}

uno::Reference < excel::XFont >
ScVbaRange::Font() throw ( script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY );
    ScDocument* pDoc = getScDocument();
    if ( mxRange.is() )
        xProps.set(mxRange, ::uno::UNO_QUERY );
    else if ( mxRanges.is() )
        xProps.set(mxRanges, ::uno::UNO_QUERY );
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
    SfxItemSet* pSet = NULL;
    try
    {
        pSet = getCurrentDataSet();
    }
    catch( uno::Exception& )
    {
    }
    return  new ScVbaFont( this, mxContext, aPalette, xProps, pSet );
}

uno::Reference< excel::XRange >
ScVbaRange::Cells( const uno::Any &nRowIndex, const uno::Any &nColumnIndex ) throw(uno::RuntimeException)
{
    // #TODO code within the test below "if ( m_Areas.... " can be removed
    // Test is performed only because m_xRange is NOT set to be
    // the first range in m_Areas ( to force failure while
    // the implementations for each method are being updated )
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->Cells( nRowIndex, nColumnIndex );
    }

    sal_Int32 nRow = 0, nColumn = 0;
    sal_Bool bIsIndex = nRowIndex >>= nRow, bIsColumnIndex = nColumnIndex >>= nColumn;

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisRangeAddress =  thisRange.getCellRangeAddressable()->getRangeAddress();
    uno::Reference< table::XCellRange > xSheetRange = thisRange.getCellRangeFromSheet();
    if( !bIsIndex && !bIsColumnIndex ) // .Cells
        // #FIXE needs proper parent ( Worksheet )
        return uno::Reference< excel::XRange >( new ScVbaRange( uno::Reference< vba::XHelperInterface >(), mxContext, mxRange ) );

    sal_Int32 nIndex = --nRow;
    if( bIsIndex && !bIsColumnIndex ) // .Cells(n)
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, ::uno::UNO_QUERY_THROW);
        sal_Int32 nColCount = xColumnRowRange->getColumns()->getCount();

        if ( !nIndex || nIndex < 0 )
            nRow = 0;
        else
            nRow = nIndex / nColCount;
        nColumn = nIndex % nColCount;
    }
    else
        --nColumn;
    nRow = nRow + thisRangeAddress.StartRow;
    nColumn =  nColumn + thisRangeAddress.StartColumn;
    return new ScVbaRange( getParent(), mxContext, xSheetRange->getCellRangeByPosition( nColumn, nRow,                                        nColumn, nRow ) );
}

void
ScVbaRange::Select() throw (uno::RuntimeException)
{
    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    if ( !pUnoRangesBase )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying uno range object" ) ), uno::Reference< uno::XInterface >()  );
    ScDocShell* pShell = pUnoRangesBase->GetDocShell();
    if ( pShell )
    {
        uno::Reference< frame::XModel > xModel( pShell->GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        if ( mxRanges.is() )
            xSelection->select( uno::makeAny( mxRanges ) );
        else
            xSelection->select( uno::makeAny( mxRange ) );
    }
}

bool cellInRange( const table::CellRangeAddress& rAddr, const sal_Int32& nCol, const sal_Int32& nRow )
{
    if ( nCol >= rAddr.StartColumn && nCol <= rAddr.EndColumn &&
        nRow >= rAddr.StartRow && nRow <= rAddr.EndRow )
        return true;
    return false;
}

void setCursor(  const SCCOL& nCol, const SCROW& nRow, bool bInSel = true )
{
    ScTabViewShell* pShell = getCurrentBestViewShell();
    if ( pShell )
    {
        if ( bInSel )
            pShell->SetCursor( nCol, nRow );
        else
            pShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_NONE, FALSE, FALSE, TRUE, FALSE );
    }
}

void
ScVbaRange::Activate() throw (uno::RuntimeException)
{
    // get first cell of current range
    uno::Reference< table::XCellRange > xCellRange;
    if ( mxRanges.is() )
    {
        uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW  );
        xCellRange.set( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    }
    else
        xCellRange.set( mxRange, uno::UNO_QUERY_THROW );

    RangeHelper thisRange( xCellRange );
    uno::Reference< sheet::XCellRangeAddressable > xThisRangeAddress = thisRange.getCellRangeAddressable();
    table::CellRangeAddress thisRangeAddress = xThisRangeAddress->getRangeAddress();

    // get current selection
    uno::Reference< sheet::XCellRangeAddressable > xRange( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);

    uno::Reference< sheet::XSheetCellRanges > xRanges( getCurrentDocument()->getCurrentSelection(), ::uno::UNO_QUERY);

    if ( xRanges.is() )
    {
        uno::Sequence< table::CellRangeAddress > nAddrs = xRanges->getRangeAddresses();
        for ( sal_Int32 index = 0; index < nAddrs.getLength(); ++index )
        {
            if ( cellInRange( nAddrs[index], thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
            {
                setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ) );
                return;
            }

        }
    }

    if ( xRange.is() && cellInRange( xRange->getRangeAddress(), thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
        setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ) );
    else
    {
        // if this range is multi cell select the range other
        // wise just position the cell at this single range position
        if ( isSingleCellRange() )
            // This top-leftmost cell of this Range is not in the current
            // selection so just select this range
            setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), false  );
        else
            Select();
    }

}

uno::Reference< excel::XRange >
ScVbaRange::Rows(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    SCROW nStartRow = 0;
    SCROW nEndRow = 0;

    sal_Int32 nValue = 0;
    rtl::OUString sAddress;

    if ( aIndex.hasValue() )
    {
        ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
        ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

        ScRange aRange = *aCellRanges.First();
        if( aIndex >>= nValue )
        {
            aRange.aStart.SetRow( aRange.aStart.Row() + --nValue );
            aRange.aEnd.SetRow( aRange.aStart.Row() );
        }

        else if ( aIndex >>= sAddress )
        {
            ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
            ScRange tmpRange;
            tmpRange.ParseRows( sAddress, getDocumentFromRange( mxRange ), dDetails );
            nStartRow = tmpRange.aStart.Row();
            nEndRow = tmpRange.aEnd.Row();

            aRange.aStart.SetRow( aRange.aStart.Row() + nStartRow );
            aRange.aEnd.SetRow( aRange.aStart.Row() + ( nEndRow  - nStartRow ));
        }
        else
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Illegal param" ) ), uno::Reference< uno::XInterface >() );

        if ( aRange.aStart.Row() < 0 || aRange.aEnd.Row() < 0 )
            throw uno::RuntimeException( rtl::OUString::createFromAscii("Internal failure, illegal param"), uno::Reference< uno::XInterface >() );
        // return a normal range ( even for multi-selection
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), aRange ) );
        return new ScVbaRange( getParent(), mxContext, xRange, true  );
    }
    // Rows() - no params
    if ( m_Areas->getCount() > 1 )
        return new ScVbaRange(  getParent(), mxContext, mxRanges, true );
    return new ScVbaRange(  getParent(), mxContext, mxRange, true );
}

uno::Reference< excel::XRange >
ScVbaRange::Columns(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    SCCOL nStartCol = 0;
    SCCOL nEndCol = 0;

    sal_Int32 nValue = 0;
    rtl::OUString sAddress;

    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

    ScRange aRange = *aCellRanges.First();
    if ( aIndex.hasValue() )
    {
        if ( aIndex >>= nValue )
        {
            aRange.aStart.SetCol( aRange.aStart.Col() + static_cast< SCCOL > ( --nValue ) );
            aRange.aEnd.SetCol( aRange.aStart.Col() );
        }

        else if ( aIndex >>= sAddress )
        {
            ScAddress::Details dDetails( ScAddress::CONV_XL_A1, 0, 0 );
            ScRange tmpRange;
            tmpRange.ParseCols( sAddress, getDocumentFromRange( mxRange ), dDetails );
            nStartCol = tmpRange.aStart.Col();
            nEndCol = tmpRange.aEnd.Col();

            aRange.aStart.SetCol( aRange.aStart.Col() + nStartCol );
            aRange.aEnd.SetCol( aRange.aStart.Col() + ( nEndCol  - nStartCol ));
        }
        else
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Illegal param" ) ), uno::Reference< uno::XInterface >() );

        if ( aRange.aStart.Col() < 0 || aRange.aEnd.Col() < 0 )
            throw uno::RuntimeException( rtl::OUString::createFromAscii("Internal failure, illegal param"), uno::Reference< uno::XInterface >() );
    }
    // Columns() - no params
    //return new ScVbaRange(  getParent(), mxContext, mxRange, false, true );
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), aRange ) );
    return new ScVbaRange( getParent(), mxContext, xRange, false, true  );
}

void
ScVbaRange::setMergeCells( const uno::Any& aIsMerged ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bIsMerged = sal_False;
    aIsMerged >>= bIsMerged;
    uno::Reference< util::XMergeable > xMerge( mxRange, ::uno::UNO_QUERY_THROW );
    //FIXME need to check whether all the cell contents are retained or lost by popping up a dialog
    xMerge->merge( bIsMerged );
}

uno::Any
ScVbaRange::getMergeCells() throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Int32 nItems = m_Areas->getCount();

    if ( nItems > 1 )
    {
        uno::Any aResult = aNULL();
        for ( sal_Int32 index=1; index != nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            if ( index > 1 )
                if ( aResult != xRange->getMergeCells() )
                    return aNULL();
            aResult = xRange->getMergeCells();
            if ( aNULL() == aResult )
                return aNULL();
        }
        return aResult;

    }
    uno::Reference< util::XMergeable > xMerge( mxRange, ::uno::UNO_QUERY_THROW );
    return uno::makeAny( xMerge->getIsMerged() );
}

void
ScVbaRange::Copy(const ::uno::Any& Destination) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
    if ( Destination.hasValue() )
    {
        uno::Reference< excel::XRange > xRange( Destination, uno::UNO_QUERY_THROW );
        uno::Any aRange = xRange->getCellRange();
        uno::Reference< table::XCellRange > xCellRange;
        aRange >>= xCellRange;
        uno::Reference< sheet::XSheetCellRange > xSheetCellRange(xCellRange, ::uno::UNO_QUERY_THROW);
        uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
        uno::Reference< table::XCellRange > xDest( xSheet, uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XCellRangeMovement > xMover( xSheet, uno::UNO_QUERY_THROW);
        uno::Reference< sheet::XCellAddressable > xDestination( xDest->getCellByPosition(
                                                xRange->getColumn()-1,xRange->getRow()-1), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XCellRangeAddressable > xSource( mxRange, uno::UNO_QUERY);
        xMover->copyRange( xDestination->getCellAddress(), xSource->getRangeAddress() );
    }
    else
    {
        Select();
        implnCopy();
    }
}

void
ScVbaRange::Cut(const ::uno::Any& Destination) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
    if (Destination.hasValue())
    {
        uno::Reference< excel::XRange > xRange( Destination, uno::UNO_QUERY_THROW );
        uno::Reference< table::XCellRange > xCellRange( xRange->getCellRange(), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSheetCellRange > xSheetCellRange(xCellRange, ::uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XSpreadsheet > xSheet = xSheetCellRange->getSpreadsheet();
        uno::Reference< table::XCellRange > xDest( xSheet, uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XCellRangeMovement > xMover( xSheet, uno::UNO_QUERY_THROW);
        uno::Reference< sheet::XCellAddressable > xDestination( xDest->getCellByPosition(
                                                xRange->getColumn()-1,xRange->getRow()-1), uno::UNO_QUERY);
        uno::Reference< sheet::XCellRangeAddressable > xSource( mxRange, uno::UNO_QUERY);
        xMover->moveRange( xDestination->getCellAddress(), xSource->getRangeAddress() );
    }
    {
        Select();
        implnCut();
    }
}

void
ScVbaRange::setNumberFormat( const uno::Any& aFormat ) throw ( script::BasicErrorException, uno::RuntimeException)
{
    rtl::OUString sFormat;
    aFormat >>= sFormat;
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->setNumberFormat( aFormat );
        }
        return;
    }
    NumFormatHelper numFormat( mxRange );
    numFormat.setNumberFormat( sFormat );
}

uno::Any
ScVbaRange::getNumberFormat() throw ( script::BasicErrorException, uno::RuntimeException)
{

    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        uno::Any aResult = aNULL();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            // if the numberformat of one area is different to another
            // return null
            if ( index > 1 )
                if ( aResult != xRange->getNumberFormat() )
                    return aNULL();
            aResult = xRange->getNumberFormat();
            if ( aNULL() == aResult )
                return aNULL();
        }
        return aResult;
    }
    NumFormatHelper numFormat( mxRange );
    rtl::OUString sFormat = numFormat.getNumberFormatString();
    if ( sFormat.getLength() > 0 )
        return uno::makeAny( sFormat );
    return aNULL();
}

uno::Reference< excel::XRange >
ScVbaRange::Resize( const uno::Any &RowSize, const uno::Any &ColumnSize ) throw (uno::RuntimeException)
{
    long nRowSize = 0, nColumnSize = 0;
    sal_Bool bIsRowChanged = ( RowSize >>= nRowSize ), bIsColumnChanged = ( ColumnSize >>= nColumnSize );
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, ::uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellRange > xSheetRange(mxRange, ::uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xCursor( xSheetRange->getSpreadsheet()->createCursorByRange(xSheetRange), ::uno::UNO_QUERY_THROW );

    if( !bIsRowChanged )
        nRowSize = xColumnRowRange->getRows()->getCount();
    if( !bIsColumnChanged )
        nColumnSize = xColumnRowRange->getColumns()->getCount();

    xCursor->collapseToSize( nColumnSize, nRowSize );
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xCursor, ::uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xRange( xSheetRange->getSpreadsheet(), ::uno::UNO_QUERY_THROW );
    return new ScVbaRange( getParent(), mxContext,xRange->getCellRangeByPosition(
                                        xCellRangeAddressable->getRangeAddress().StartColumn,
                                        xCellRangeAddressable->getRangeAddress().StartRow,
                                        xCellRangeAddressable->getRangeAddress().EndColumn,
                                        xCellRangeAddressable->getRangeAddress().EndRow ) );
}

void
ScVbaRange::setWrapText( const uno::Any& aIsWrapped ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        uno::Any aResult;
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->setWrapText( aIsWrapped );
        }
        return;
    }

    uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTextWrapped" ) ), aIsWrapped );
}

uno::Any
ScVbaRange::getWrapText() throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        uno::Any aResult;
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
                uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
                if ( index > 1 )
                if ( aResult != xRange->getWrapText() )
                    return aNULL();
            aResult = xRange->getWrapText();
        }
        return aResult;
    }

    SfxItemSet* pDataSet = getCurrentDataSet();

    SfxItemState eState = pDataSet->GetItemState( ATTR_LINEBREAK, TRUE, NULL);
    if ( eState == SFX_ITEM_DONTCARE )
        return aNULL();

    uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW );
    uno::Any aValue = xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTextWrapped" ) ) );
    return aValue;
}

uno::Reference< excel::XInterior > ScVbaRange::Interior( ) throw ( script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( mxRange, uno::UNO_QUERY_THROW );
        return new ScVbaInterior ( this, mxContext, xProps, getScDocument() );
}
uno::Reference< excel::XRange >
ScVbaRange::Range( const uno::Any &Cell1, const uno::Any &Cell2 ) throw (uno::RuntimeException)
{
    return Range( Cell1, Cell2, false );
}
uno::Reference< excel::XRange >
ScVbaRange::Range( const uno::Any &Cell1, const uno::Any &Cell2, bool bForceUseInpuRangeTab ) throw (uno::RuntimeException)

{
    uno::Reference< table::XCellRange > xCellRange = mxRange;

    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW );
        xCellRange.set( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    }
    else
        xCellRange.set( mxRange );

    RangeHelper thisRange( xCellRange );
    uno::Reference< table::XCellRange > xRanges = thisRange.getCellRangeFromSheet();
    uno::Reference< sheet::XCellRangeAddressable > xAddressable( xRanges, uno::UNO_QUERY_THROW );

    uno::Reference< table::XCellRange > xReferrer =
        xRanges->getCellRangeByPosition( getColumn()-1, getRow()-1,
                xAddressable->getRangeAddress().EndColumn,
                xAddressable->getRangeAddress().EndRow );
    // xAddressable now for this range
    xAddressable.set( xReferrer, uno::UNO_QUERY_THROW );


    if( !Cell1.hasValue() )
        throw uno::RuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " Invalid Argument " ) ),
            uno::Reference< XInterface >() );

    table::CellRangeAddress resultAddress;
    table::CellRangeAddress parentRangeAddress = xAddressable->getRangeAddress();

    ScRange aRange;
    // Cell1 defined only
    if ( !Cell2.hasValue() )
    {
        rtl::OUString sName;
        Cell1 >>= sName;
        RangeHelper referRange( xReferrer );
        table::CellRangeAddress referAddress = referRange.getCellRangeAddressable()->getRangeAddress();
        return getRangeForName( mxContext, sName, getScDocShell(), referAddress );

    }
    else
    {
        table::CellRangeAddress  cell1, cell2;
        cell1 = getCellRangeAddressForVBARange( Cell1, getScDocShell() );
        // Cell1 & Cell2 defined
        // Excel seems to combine the range as the range defined by
        // the combination of Cell1 & Cell2

        cell2 = getCellRangeAddressForVBARange( Cell2, getScDocShell() );

        resultAddress.StartColumn = ( cell1.StartColumn <  cell2.StartColumn ) ? cell1.StartColumn : cell2.StartColumn;
        resultAddress.StartRow = ( cell1.StartRow <  cell2.StartRow ) ? cell1.StartRow : cell2.StartRow;
        resultAddress.EndColumn = ( cell1.EndColumn >  cell2.EndColumn ) ? cell1.EndColumn : cell2.EndColumn;
        resultAddress.EndRow = ( cell1.EndRow >  cell2.EndRow ) ? cell1.EndRow : cell2.EndRow;
        if ( bForceUseInpuRangeTab )
        {
            // this is a call from Application.Range( x,y )
            // its possiblefor x or y to specify a different sheet from
            // the current or active on ( but they must be the same )
            if ( cell1.Sheet != cell2.Sheet )
                throw uno::RuntimeException();
            parentRangeAddress.Sheet = cell1.Sheet;
        }
        else
        {
            // this is not a call from Application.Range( x,y )
            // if a different sheet from this range is specified it's
            // an error
            if ( parentRangeAddress.Sheet != cell1.Sheet
            || parentRangeAddress.Sheet != cell2.Sheet
            )
                throw uno::RuntimeException();

        }
        ScUnoConversion::FillScRange( aRange, resultAddress );
    }
    ScRange parentAddress;
    ScUnoConversion::FillScRange( parentAddress, parentRangeAddress);
    if ( aRange.aStart.Col() >= 0 && aRange.aStart.Row() >= 0 && aRange.aEnd.Col() >= 0 && aRange.aEnd.Row() >= 0 )
    {
        sal_Int32 nStartX = parentAddress.aStart.Col() + aRange.aStart.Col();
        sal_Int32 nStartY = parentAddress.aStart.Row() + aRange.aStart.Row();
        sal_Int32 nEndX = parentAddress.aStart.Col() + aRange.aEnd.Col();
        sal_Int32 nEndY = parentAddress.aStart.Row() + aRange.aEnd.Row();

        if ( nStartX <= nEndX && nEndX <= parentAddress.aEnd.Col() &&
             nStartY <= nEndY && nEndY <= parentAddress.aEnd.Row() )
        {
            ScRange aNew( (SCCOL)nStartX, (SCROW)nStartY, parentAddress.aStart.Tab(),
                          (SCCOL)nEndX, (SCROW)nEndY, parentAddress.aEnd.Tab() );
            xCellRange = new ScCellRangeObj( getScDocShell(), aNew );
        }
    }

    return new ScVbaRange( getParent(), mxContext, xCellRange );

}

// Allow access to underlying openoffice uno api ( useful for debugging
// with openoffice basic )
::com::sun::star::uno::Any SAL_CALL
ScVbaRange::getCellRange(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    if ( mxRanges.is() )
        aAny <<= mxRanges;
    else if ( mxRange.is() )
        aAny <<= mxRange;
    return aAny;
}

static USHORT
getPasteFlags (sal_Int32 Paste)
{
    USHORT nFlags = IDF_NONE;
    switch (Paste) {
        case excel::XlPasteType::xlPasteComments:
        nFlags = IDF_NOTE;break;
        case excel::XlPasteType::xlPasteFormats:
        nFlags = IDF_ATTRIB;break;
        case excel::XlPasteType::xlPasteFormulas:
        nFlags = IDF_FORMULA;break;
        case excel::XlPasteType::xlPasteFormulasAndNumberFormats :
        case excel::XlPasteType::xlPasteValues:
#ifdef VBA_OOBUILD_HACK
        nFlags = ( IDF_VALUE | IDF_DATETIME | IDF_STRING | IDF_SPECIAL_BOOLEAN ); break;
#else
        nFlags = ( IDF_VALUE | IDF_DATETIME | IDF_STRING ); break;
#endif
        case excel::XlPasteType::xlPasteValuesAndNumberFormats:
        nFlags = IDF_VALUE | IDF_ATTRIB; break;
        case excel::XlPasteType::xlPasteColumnWidths:
        case excel::XlPasteType::xlPasteValidation:
        nFlags = IDF_NONE;break;
    case excel::XlPasteType::xlPasteAll:
        case excel::XlPasteType::xlPasteAllExceptBorders:
    default:
        nFlags = IDF_ALL;break;
    };
return nFlags;
}

static USHORT
getPasteFormulaBits( sal_Int32 Operation)
{
    USHORT nFormulaBits = PASTE_NOFUNC ;
    switch (Operation)
    {
    case excel::XlPasteSpecialOperation::xlPasteSpecialOperationAdd:
        nFormulaBits = PASTE_ADD;break;
    case excel::XlPasteSpecialOperation::xlPasteSpecialOperationSubtract:
        nFormulaBits = PASTE_SUB;break;
    case excel::XlPasteSpecialOperation::xlPasteSpecialOperationMultiply:
        nFormulaBits = PASTE_MUL;break;
    case excel::XlPasteSpecialOperation::xlPasteSpecialOperationDivide:
        nFormulaBits = PASTE_DIV;break;

    case excel::XlPasteSpecialOperation::xlPasteSpecialOperationNone:
    default:
        nFormulaBits = PASTE_NOFUNC; break;
    };

return nFormulaBits;
}
void SAL_CALL
ScVbaRange::PasteSpecial( const uno::Any& Paste, const uno::Any& Operation, const uno::Any& SkipBlanks, const uno::Any& Transpose ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );
    uno::Reference< view::XSelectionSupplier > xSelection( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY_THROW );
    // save old selection
    uno::Reference< uno::XInterface > xSel(  getCurrentDocument()->getCurrentSelection() );
    // select this range
    xSelection->select( uno::makeAny( mxRange ) );
    // set up defaults
    sal_Int32 nPaste = excel::XlPasteType::xlPasteAll;
    sal_Int32 nOperation = excel::XlPasteSpecialOperation::xlPasteSpecialOperationNone;
    sal_Bool bTranspose = sal_False;
    sal_Bool bSkipBlanks = sal_False;

    if ( Paste.hasValue() )
        Paste >>= nPaste;
    if ( Operation.hasValue() )
        Operation >>= nOperation;
    if ( SkipBlanks.hasValue() )
        SkipBlanks >>= bSkipBlanks;
    if ( Transpose.hasValue() )
        Transpose >>= bTranspose;

    USHORT nFlags = getPasteFlags(nPaste);
    USHORT nFormulaBits = getPasteFormulaBits(nOperation);
    implnPasteSpecial(nFlags,nFormulaBits,bSkipBlanks,bTranspose);
    // restore selection
    xSelection->select( uno::makeAny( xSel ) );
}

uno::Reference< excel::XRange >
ScVbaRange::getEntireColumnOrRow( bool bColumn ) throw (uno::RuntimeException)
{
    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    // copy the range list
    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

    for ( ScRange* pRange = aCellRanges.First() ; pRange; pRange = aCellRanges.Next() )
    {
        if ( bColumn )
        {
            pRange->aStart.SetRow( 0 );
            pRange->aEnd.SetRow( MAXROW );
        }
        else
        {
            pRange->aStart.SetCol( 0 );
            pRange->aEnd.SetCol( MAXCOL );
        }
    }
    if ( aCellRanges.Count() > 1 ) // Multi-Area
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );

        return new ScVbaRange( getParent(), mxContext, xRanges, !bColumn, bColumn );
    }
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.First() ) );
    return new ScVbaRange( getParent(), mxContext, xRange, !bColumn, bColumn  );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::getEntireRow() throw (uno::RuntimeException)
{
    return getEntireColumnOrRow(false);
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::getEntireColumn() throw (uno::RuntimeException)
{
    return getEntireColumnOrRow();
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaRange::AddComment( const uno::Any& Text ) throw (uno::RuntimeException)
{

    uno::Reference< excel::XComment > xComment( new ScVbaComment( this, mxContext, mxRange ) );
    // if you don't pass a valid text or if there is already a comment
    // associated with the range then return NULL
    if ( !xComment->Text( Text, uno::Any(), uno::Any() ).getLength()
    ||   xComment->Text( uno::Any(), uno::Any(), uno::Any() ).getLength() )
        return NULL;
    return xComment;
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaRange::getComment() throw (uno::RuntimeException)
{
    // intentional behavior to return a null object if no
    // comment defined
    uno::Reference< excel::XComment > xComment( new ScVbaComment( this, mxContext, mxRange ) );
    if ( !xComment->Text( uno::Any(), uno::Any(), uno::Any() ).getLength() )
        return NULL;
    return xComment;

}

uno::Reference< beans::XPropertySet >
getRowOrColumnProps( const uno::Reference< table::XCellRange >& xCellRange, bool bRows ) throw ( uno::RuntimeException )
{
    uno::Reference< table::XColumnRowRange > xColRow( xCellRange, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps;
    if ( bRows )
        xProps.set( xColRow->getRows(), uno::UNO_QUERY_THROW );
    else
        xProps.set( xColRow->getColumns(), uno::UNO_QUERY_THROW );
    return xProps;
}

uno::Any SAL_CALL
ScVbaRange::getHidden() throw (uno::RuntimeException)
{
    // if multi-area result is the result of the
    // first area
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(sal_Int32(1)), uno::Any() ), uno::UNO_QUERY_THROW );
        return xRange->getHidden();
    }
    bool bIsVisible = false;
    try
    {
        uno::Reference< beans::XPropertySet > xProps = getRowOrColumnProps( mxRange, mbIsRows );
        if ( !( xProps->getPropertyValue( ISVISIBLE ) >>= bIsVisible ) )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to get IsVisible property")), uno::Reference< uno::XInterface >() );
    }
    catch( uno::Exception& e )
    {
        throw uno::RuntimeException( e.Message, uno::Reference< uno::XInterface >() );
    }
    return uno::makeAny( !bIsVisible );
}

void SAL_CALL
ScVbaRange::setHidden( const uno::Any& _hidden ) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->setHidden( _hidden );
        }
        return;
    }

    sal_Bool bHidden = sal_False;
    _hidden >>= bHidden;

    try
    {
        uno::Reference< beans::XPropertySet > xProps = getRowOrColumnProps( mxRange, mbIsRows );
        xProps->setPropertyValue( ISVISIBLE, uno::makeAny( !bHidden ) );
    }
    catch( uno::Exception& e )
    {
        throw uno::RuntimeException( e.Message, uno::Reference< uno::XInterface >() );
    }
}

rtl::OUString lcl_replaceAll( const rtl::OUString& rString, rtl::OUString sWhat, rtl::OUString sWith )
{
    rtl::OUString sString( rString );
    sal_Int32 offset = 0;
    sal_Int32 nWithLen = sWith.getLength();
    while ((offset = sString.indexOf(sWhat )) >= 0)
    {
        sString = sString.replaceAt(offset, nWithLen, sWith);
        offset += nWithLen;
    }
    return sString;
}

::sal_Bool SAL_CALL
ScVbaRange::Replace( const ::rtl::OUString& What, const ::rtl::OUString& Replacement, const uno::Any& LookAt, const uno::Any& SearchOrder, const uno::Any& MatchCase, const uno::Any& MatchByte, const uno::Any& SearchFormat, const uno::Any& ReplaceFormat  ) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        for ( sal_Int32 index = 1; index <= m_Areas->getCount(); ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->Replace( What, Replacement,  LookAt, SearchOrder, MatchCase, MatchByte, SearchFormat, ReplaceFormat );
        }
        return sal_True; // seems to return true always ( or at least I haven't found the trick of
    }

    // sanity check required params
    if ( !What.getLength() /*|| !Replacement.getLength()*/ )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, missing params" )) , uno::Reference< uno::XInterface >() );
    rtl::OUString sWhat = VBAToRegexp( What);
    // #TODO #FIXME SearchFormat & ReplacesFormat are not processed
    // What do we do about MatchByte.. we don't seem to support that
    const SvxSearchItem& globalSearchOptions = ScGlobal::GetSearchItem();
    SvxSearchItem newOptions( globalSearchOptions );

    sal_Int16 nLook =  globalSearchOptions.GetWordOnly() ?  excel::XlLookAt::xlPart : excel::XlLookAt::xlWhole;
    sal_Int16 nSearchOrder = globalSearchOptions.GetRowDirection() ? excel::XlSearchOrder::xlByRows : excel::XlSearchOrder::xlByColumns;

    sal_Bool bMatchCase = sal_False;
    uno::Reference< util::XReplaceable > xReplace( mxRange, uno::UNO_QUERY );
    if ( xReplace.is() )
    {
        uno::Reference< util::XReplaceDescriptor > xDescriptor =
            xReplace->createReplaceDescriptor();

        xDescriptor->setSearchString( sWhat);
        xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHREGEXP ) ), uno::makeAny( sal_True ) );
        xDescriptor->setReplaceString( Replacement);
        if ( LookAt.hasValue() )
        {
            // sets SearchWords ( true is Cell match )
            nLook =  ::comphelper::getINT16( LookAt );
            sal_Bool bSearchWords = sal_False;
            if ( nLook == excel::XlLookAt::xlPart )
                bSearchWords = sal_False;
            else if ( nLook == excel::XlLookAt::xlWhole )
                bSearchWords = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, illegal value for LookAt" )) , uno::Reference< uno::XInterface >() );
            // set global search props ( affects the find dialog
            // and of course the defaults for this method
            newOptions.SetWordOnly( bSearchWords );
            xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHWORDS ) ), uno::makeAny( bSearchWords ) );
        }
        // sets SearchByRow ( true for Rows )
        if ( SearchOrder.hasValue() )
        {
            nSearchOrder =  ::comphelper::getINT16( SearchOrder );
            sal_Bool bSearchByRow = sal_False;
            if ( nSearchOrder == excel::XlSearchOrder::xlByColumns )
                bSearchByRow = sal_False;
            else if ( nSearchOrder == excel::XlSearchOrder::xlByRows )
                bSearchByRow = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Range::Replace, illegal value for SearchOrder" )) , uno::Reference< uno::XInterface >() );

            newOptions.SetRowDirection( bSearchByRow );
            xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHBYROW ) ), uno::makeAny( bSearchByRow ) );
        }
        if ( MatchCase.hasValue() )
        {
            // SearchCaseSensitive
            MatchCase >>= bMatchCase;
            xDescriptor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SRCHCASE ) ), uno::makeAny( bMatchCase ) );
        }

        ScGlobal::SetSearchItem( newOptions );
        // ignore MatchByte for the moment, its not supported in
        // OOo.org afaik

        uno::Reference< util::XSearchDescriptor > xSearch( xDescriptor, uno::UNO_QUERY );
        xReplace->replaceAll( xSearch );
    }
    return sal_True; // always
}

uno::Reference< table::XCellRange > processKey( const uno::Any& Key, uno::Reference<  uno::XComponentContext >& xContext, ScDocShell* pDocSh )
{
    uno::Reference< excel::XRange > xKeyRange;
    if ( Key.getValueType() == excel::XRange::static_type() )
    {
        xKeyRange.set( Key, uno::UNO_QUERY_THROW );
    }
    else if ( Key.getValueType() == ::getCppuType( static_cast< const rtl::OUString* >(0) )  )

    {
        rtl::OUString sRangeName = ::comphelper::getString( Key );
        table::CellRangeAddress  aRefAddr;
        if ( !pDocSh )
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort no docshell to calculate key param")), uno::Reference< uno::XInterface >() );
        xKeyRange = getRangeForName( xContext, sRangeName, pDocSh, aRefAddr );
    }
    else
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort illegal type value for key param")), uno::Reference< uno::XInterface >() );
    uno::Reference< table::XCellRange > xKey;
    xKey.set( xKeyRange->getCellRange(), uno::UNO_QUERY_THROW );
    return xKey;
}

// helper method for Sort
sal_Int32 findSortPropertyIndex( const uno::Sequence< beans::PropertyValue >& props,
const rtl::OUString& sPropName ) throw( uno::RuntimeException )
{
    const beans::PropertyValue* pProp = props.getConstArray();
    sal_Int32 nItems = props.getLength();

     sal_Int32 count=0;
    for ( ; count < nItems; ++count, ++pProp )
        if ( pProp->Name.equals( sPropName ) )
            return count;
    if ( count == nItems )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort unknown sort property")), uno::Reference< uno::XInterface >() );
    return -1; //should never reach here ( satisfy compiler )
}

// helper method for Sort
void updateTableSortField( const uno::Reference< table::XCellRange >& xParentRange,
    const uno::Reference< table::XCellRange >& xColRowKey, sal_Int16 nOrder,
    table::TableSortField& aTableField, sal_Bool bIsSortColumn, sal_Bool bMatchCase ) throw ( uno::RuntimeException )
{
        RangeHelper parentRange( xParentRange );
        RangeHelper colRowRange( xColRowKey );

        table::CellRangeAddress parentRangeAddress = parentRange.getCellRangeAddressable()->getRangeAddress();

        table::CellRangeAddress colRowKeyAddress = colRowRange.getCellRangeAddressable()->getRangeAddress();

        // make sure that upper left poing of key range is within the
        // parent range
        if (  colRowKeyAddress.StartColumn >= parentRangeAddress.StartColumn &&
            colRowKeyAddress.StartColumn <= parentRangeAddress.EndColumn  &&
            colRowKeyAddress.StartRow >= parentRangeAddress.StartRow &&
            colRowKeyAddress.StartRow <= parentRangeAddress.EndRow  )
        {
            //determine col/row index
            if ( bIsSortColumn )
                aTableField.Field = colRowKeyAddress.StartRow - parentRangeAddress.StartRow;
            else
                aTableField.Field = colRowKeyAddress.StartColumn - parentRangeAddress.StartColumn;
            aTableField.IsCaseSensitive = bMatchCase;

            if ( nOrder ==  excel::XlSortOrder::xlAscending )
                aTableField.IsAscending = sal_True;
            else
                aTableField.IsAscending = sal_False;
        }
        else
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal Key param" ) ), uno::Reference< uno::XInterface >() );


}

void SAL_CALL
ScVbaRange::Sort( const uno::Any& Key1, const uno::Any& Order1, const uno::Any& Key2, const uno::Any& /*Type*/, const uno::Any& Order2, const uno::Any& Key3, const uno::Any& Order3, const uno::Any& Header, const uno::Any& OrderCustom, const uno::Any& MatchCase, const uno::Any& Orientation, const uno::Any& SortMethod,  const uno::Any& DataOption1, const uno::Any& DataOption2, const uno::Any& DataOption3  ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# can we do something with Type
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("That command cannot be used on multiple selections" ) ), uno::Reference< uno::XInterface >() );

    sal_Int16 nDataOption1 = excel::XlSortDataOption::xlSortNormal;
    sal_Int16 nDataOption2 = excel::XlSortDataOption::xlSortNormal;;
    sal_Int16 nDataOption3 = excel::XlSortDataOption::xlSortNormal;

    ScDocument* pDoc = getScDocument();
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisRangeAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    ScSortParam aSortParam;
    SCTAB nTab = thisRangeAddress.Sheet;
    pDoc->GetSortParam( aSortParam, nTab );

    if ( DataOption1.hasValue() )
        DataOption1 >>= nDataOption1;
    if ( DataOption2.hasValue() )
        DataOption2 >>= nDataOption2;
    if ( DataOption3.hasValue() )
        DataOption3 >>= nDataOption3;

    // 1) #TODO #FIXME need to process DataOption[1..3] not used currently
    // 2) #TODO #FIXME need to refactor this ( below ) into a IsSingleCell() method
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY_THROW );

    // 'Fraid I don't remember what I was trying to achieve here ???
/*
    if (  isSingleCellRange() )
    {
        uno::Reference< vba::XRange > xCurrent = CurrentRegion();
        xCurrent->Sort( Key1, Order1, Key2, Type, Order2, Key3, Order3, Header, OrderCustom, MatchCase, Orientation, SortMethod, DataOption1, DataOption2, DataOption3 );
        return;
    }
*/
    // set up defaults

    sal_Int16 nOrder1 = aSortParam.bAscending[0] ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;
    sal_Int16 nOrder2 = aSortParam.bAscending[1] ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;
    sal_Int16 nOrder3 = aSortParam.bAscending[2] ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;

    sal_Int16 nCustom = aSortParam.nUserIndex;
    sal_Int16 nSortMethod = excel::XlSortMethod::xlPinYin;
    sal_Bool bMatchCase = aSortParam.bCaseSens;

    // seems to work opposite to expected, see below
    sal_Int16 nOrientation = aSortParam.bByRow ?  excel::XlSortOrientation::xlSortColumns :  excel::XlSortOrientation::xlSortRows;

    if ( Orientation.hasValue() )
    {
        // Documentation says xlSortRows is default but that doesn't appear to be
        // the case. Also it appears that xlSortColumns is the default which
        // strangely enought sorts by Row
        nOrientation = ::comphelper::getINT16( Orientation );
        // persist new option to be next calls default
        if ( nOrientation == excel::XlSortOrientation::xlSortRows )
            aSortParam.bByRow = FALSE;
        else
            aSortParam.bByRow = TRUE;

    }

    sal_Bool bIsSortColumns=sal_False; // sort by row

    if ( nOrientation == excel::XlSortOrientation::xlSortRows )
        bIsSortColumns = sal_True;
    sal_Int16 nHeader = 0;
#ifdef VBA_OOBUILD_HACK
    nHeader = aSortParam.nCompatHeader;
#endif
    sal_Bool bContainsHeader = sal_False;

    if ( Header.hasValue() )
    {
        nHeader = ::comphelper::getINT16( Header );
#ifdef VBA_OOBUILD_HACK
        aSortParam.nCompatHeader = nHeader;
#endif
    }

    if ( nHeader == excel::XlYesNoGuess::xlGuess )
    {
        bool bHasColHeader = pDoc->HasColHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ));
        bool bHasRowHeader = pDoc->HasRowHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ) );
        if ( bHasColHeader || bHasRowHeader )
            nHeader =  excel::XlYesNoGuess::xlYes;
        else
            nHeader =  excel::XlYesNoGuess::xlNo;
#ifdef VBA_OOBUILD_HACK
        aSortParam.nCompatHeader = nHeader;
#endif
    }

    if ( nHeader == excel::XlYesNoGuess::xlYes )
        bContainsHeader = sal_True;

    if ( SortMethod.hasValue() )
    {
        nSortMethod = ::comphelper::getINT16( SortMethod );
    }

    if ( OrderCustom.hasValue() )
    {
        OrderCustom >>= nCustom;
        --nCustom; // 0-based in OOo
        aSortParam.nUserIndex = nCustom;
    }

    if ( MatchCase.hasValue() )
    {
        MatchCase >>= bMatchCase;
        aSortParam.bCaseSens = bMatchCase;
    }

    if ( Order1.hasValue() )
    {
        nOrder1 = ::comphelper::getINT16(Order1);
        if (  nOrder1 == excel::XlSortOrder::xlAscending )
            aSortParam.bAscending[0]  = TRUE;
        else
            aSortParam.bAscending[0]  = FALSE;

    }
    if ( Order2.hasValue() )
    {
        nOrder2 = ::comphelper::getINT16(Order2);
        if ( nOrder2 == excel::XlSortOrder::xlAscending )
            aSortParam.bAscending[1]  = TRUE;
        else
            aSortParam.bAscending[1]  = FALSE;
    }
    if ( Order3.hasValue() )
    {
        nOrder3 = ::comphelper::getINT16(Order3);
        if ( nOrder3 == excel::XlSortOrder::xlAscending )
            aSortParam.bAscending[2]  = TRUE;
        else
            aSortParam.bAscending[2]  = FALSE;
    }

    uno::Reference< table::XCellRange > xKey1;
    uno::Reference< table::XCellRange > xKey2;
    uno::Reference< table::XCellRange > xKey3;
    ScDocShell* pDocShell = getScDocShell();
    xKey1 = processKey( Key1, mxContext, pDocShell );
    if ( !xKey1.is() )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Range::Sort needs a key1 param")), uno::Reference< uno::XInterface >() );

    if ( Key2.hasValue() )
        xKey2 = processKey( Key2, mxContext, pDocShell );
    if ( Key3.hasValue() )
        xKey3 = processKey( Key3, mxContext, pDocShell );

    uno::Reference< util::XSortable > xSort( mxRange, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > sortDescriptor = xSort->createSortDescriptor();
    sal_Int32 nTableSortFieldIndex = findSortPropertyIndex( sortDescriptor, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SortFields") ) );

    uno::Sequence< table::TableSortField > sTableFields(1);
    sal_Int32 nTableIndex = 0;
    updateTableSortField(  mxRange, xKey1, nOrder1, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );

    if ( xKey2.is() )
    {
        sTableFields.realloc( sTableFields.getLength() + 1 );
        updateTableSortField(  mxRange, xKey2, nOrder2, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );
    }
    if ( xKey3.is()  )
    {
        sTableFields.realloc( sTableFields.getLength() + 1 );
        updateTableSortField(  mxRange, xKey3, nOrder3, sTableFields[ nTableIndex++ ], bIsSortColumns, bMatchCase );
    }
    sortDescriptor[ nTableSortFieldIndex ].Value <<= sTableFields;

    sal_Int32 nIndex =  findSortPropertyIndex( sortDescriptor,  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSortColumns")) );
    sortDescriptor[ nIndex ].Value <<= bIsSortColumns;

    nIndex =    findSortPropertyIndex( sortDescriptor, CONTS_HEADER );
    sortDescriptor[ nIndex ].Value <<= bContainsHeader;

    pDoc->SetSortParam( aSortParam, nTab );
    xSort->sort( sortDescriptor );

    // #FIXME #TODO
    // The SortMethod param is not processed ( not sure what its all about, need to

}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::End( ::sal_Int32 Direction )  throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( getArea( 0 ), uno::UNO_QUERY_THROW );
        return xRange->End( Direction );
    }


    // #FIXME #TODO
    // euch! found my orig implementation sucked, so
    // trying this even suckier one ( really need to use/expose code in
    // around  ScTabView::MoveCursorArea(), thats the bit that calcutes
    // where the cursor should go )
    // Main problem with this method is the ultra hacky attempt to preserve
    // the ActiveCell, there should be no need to go to these extreems

    // Save ActiveCell pos ( to restore later )
    uno::Any aDft;
    rtl::OUString sActiveCell = ScVbaGlobals::getGlobalsImpl(
                       mxContext )->getApplication()->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

    // position current cell upper left of this range
    Cells( uno::makeAny( (sal_Int32) 1 ), uno::makeAny( (sal_Int32) 1 ) )->Select();

    SfxViewFrame* pViewFrame = getCurrentViewFrame();
    if ( pViewFrame )
    {
        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
        // Hoping this will make sure this slot is called
        // synchronously
        SfxBoolItem sfxAsync( SID_ASYNCHRON, sal_False );
        aArgs.Put( sfxAsync, sfxAsync.Which() );
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();

        USHORT nSID = 0;

        switch( Direction )
        {
            case excel::XlDirection::xlDown:
                nSID = SID_CURSORBLKDOWN;
                break;
            case excel::XlDirection::xlUp:
                nSID = SID_CURSORBLKUP;
                break;
            case excel::XlDirection::xlToLeft:
                nSID = SID_CURSORBLKLEFT;
                break;
            case excel::XlDirection::xlToRight:
                nSID = SID_CURSORBLKRIGHT;
                break;
            default:
                throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": Invalid ColumnIndex" ) ), uno::Reference< uno::XInterface >() );
        }
        if ( pDispatcher )
        {
            pDispatcher->Execute( nSID, (SfxCallMode)SFX_CALLMODE_SYNCHRON, aArgs );
        }
    }

    // result is the ActiveCell
    rtl::OUString sMoved =  ScVbaGlobals::getGlobalsImpl(
                       mxContext )->getApplication()->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

    // restore old ActiveCell
    uno::Any aVoid;
    uno::Reference< excel::XRange > xOldActiveCell( ScVbaGlobals::getGlobalsImpl(
        mxContext )->getActiveSheet()->Range( uno::makeAny( sActiveCell ), aVoid ), uno::UNO_QUERY_THROW );
    xOldActiveCell->Select();

    uno::Reference< excel::XRange > resultCell;
    resultCell.set( ScVbaGlobals::getGlobalsImpl(
        mxContext )->getActiveSheet()->Range( uno::makeAny( sMoved ), aVoid ), uno::UNO_QUERY_THROW );

    // return result

    return resultCell;
}

bool
ScVbaRange::isSingleCellRange()
{
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY);
    if ( xColumnRowRange.is() && xColumnRowRange->getRows()->getCount() == 1 && xColumnRowRange->getColumns()->getCount() == 1 )
        return true;
    return false;
}

uno::Reference< excel::XCharacters > SAL_CALL
ScVbaRange::characters( const uno::Any& Start, const uno::Any& Length ) throw (uno::RuntimeException)
{
    if ( !isSingleCellRange() )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Can't create Characters property for multicell range ") ), uno::Reference< uno::XInterface >() );
    uno::Reference< text::XSimpleText > xSimple(mxRange->getCellByPosition(0,0) , uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocumentFromRange(mxRange);
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );

    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
    return  new ScVbaCharacters( this, mxContext, aPalette, xSimple, Start, Length );
}

 void SAL_CALL
ScVbaRange::Delete( const uno::Any& Shift ) throw (uno::RuntimeException)
{

    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->Delete( Shift );
        }
        return;
    }
    sheet::CellDeleteMode mode = sheet::CellDeleteMode_NONE ;
    if ( Shift.hasValue() )
    {
        sal_Int32 nShift = 0;
        Shift >>= nShift;
        switch ( nShift )
        {
            case excel::XlDeleteShiftDirection::xlShiftUp:
                mode = sheet::CellDeleteMode_UP;
                break;
            case excel::XlDeleteShiftDirection::xlShiftToLeft:
                mode = sheet::CellDeleteMode_LEFT;
                break;
            default:
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("Illegal paramater ") ), uno::Reference< uno::XInterface >() );
        }
    }
    else
        if ( getRow() >  getColumn() )
            mode = sheet::CellDeleteMode_UP;
        else
            mode = sheet::CellDeleteMode_LEFT;
    RangeHelper thisRange( mxRange );
    uno::Reference< sheet::XCellRangeMovement > xCellRangeMove( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
    xCellRangeMove->removeRange( thisRange.getCellRangeAddressable()->getRangeAddress(), mode );

}

//XElementAccess
sal_Bool SAL_CALL
ScVbaRange::hasElements() throw (uno::RuntimeException)
{
    uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY );
    if ( xColumnRowRange.is() )
        if ( xColumnRowRange->getRows()->getCount() ||
            xColumnRowRange->getColumns()->getCount() )
            return sal_True;
    return sal_False;
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL
ScVbaRange::createEnumeration() throw (uno::RuntimeException)
{
    return new CellsEnumeration( mxContext, m_Areas );
}

::rtl::OUString SAL_CALL
ScVbaRange::getDefaultMethodName(  ) throw (uno::RuntimeException)
{
    const static rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM("Cells") );
    return sName;
}


uno::Reference< awt::XDevice >
getDeviceFromDoc( const uno::Reference< frame::XModel >& xModel ) throw( uno::RuntimeException )
{
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XFrame> xFrame( xController->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XDevice > xDevice( xFrame->getComponentWindow(), uno::UNO_QUERY_THROW );
    return xDevice;
}

// returns calc internal col. width ( in points )
double
ScVbaRange::getCalcColWidth( const table::CellRangeAddress& rAddress) throw (uno::RuntimeException)
{
    ScDocument* pDoc = getScDocument();
    USHORT nWidth = pDoc->GetOriginalWidth( static_cast< SCCOL >( rAddress.StartColumn ), static_cast< SCTAB >( rAddress.Sheet ) );
    double nPoints = lcl_TwipsToPoints( nWidth );
    nPoints = lcl_Round2DecPlaces( nPoints );
    return nPoints;
}

double
ScVbaRange::getCalcRowHeight( const table::CellRangeAddress& rAddress ) throw (uno::RuntimeException)
{
    ScDocument* pDoc = getDocumentFromRange( mxRange );
    USHORT nWidth = pDoc->GetOriginalHeight( rAddress.StartRow, rAddress.Sheet );
    double nPoints = lcl_TwipsToPoints( nWidth );
    nPoints = lcl_Round2DecPlaces( nPoints );
    return nPoints;
}

// return Char Width in points
double getDefaultCharWidth( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    const static rtl::OUString sDflt( RTL_CONSTASCII_USTRINGPARAM("Default"));
    const static rtl::OUString sCharFontName( RTL_CONSTASCII_USTRINGPARAM("CharFontName"));
    const static rtl::OUString sPageStyles( RTL_CONSTASCII_USTRINGPARAM("PageStyles"));
    // get the font from the default style
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xStyleSupplier->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess2( xNameAccess->getByName( sPageStyles ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xNameAccess2->getByName( sDflt ), uno::UNO_QUERY_THROW );
    rtl::OUString sFontName;
    xProps->getPropertyValue( sCharFontName ) >>= sFontName;

    uno::Reference< awt::XDevice > xDevice = getDeviceFromDoc( xModel );
    awt::FontDescriptor aDesc;
    aDesc.Name = sFontName;
    uno::Reference< awt::XFont > xFont( xDevice->getFont( aDesc ), uno::UNO_QUERY_THROW );
    double nCharPixelWidth =  xFont->getCharWidth( (sal_Int8)'0' );

    double nPixelsPerMeter = xDevice->getInfo().PixelPerMeterX;
    double nCharWidth = nCharPixelWidth /  nPixelsPerMeter;
    nCharWidth = nCharWidth * (double)56700;// in twips
    return lcl_TwipsToPoints( (USHORT)nCharWidth );
}

uno::Any SAL_CALL
ScVbaRange::getColumnWidth() throw (uno::RuntimeException)
{
    sal_Int32 nLen = m_Areas->getCount();
    if ( nLen > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xRange->getColumnWidth();
    }

    double nColWidth =  0;
    ScDocShell* pShell = getScDocShell();
    if ( pShell )
    {
        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        double defaultCharWidth = getDefaultCharWidth( xModel );
        RangeHelper thisRange( mxRange );
        table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        sal_Int32 nStartCol = thisAddress.StartColumn;
        sal_Int32 nEndCol = thisAddress.EndColumn;
        USHORT nColTwips = 0;
        for( sal_Int32 nCol = nStartCol ; nCol <= nEndCol; ++nCol )
        {
            thisAddress.StartColumn = nCol;
            USHORT nCurTwips = pShell->GetDocument()->GetOriginalWidth( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCTAB >( thisAddress.Sheet ) );
            if ( nCol == nStartCol )
                nColTwips =  nCurTwips;
            if ( nColTwips != nCurTwips )
                return aNULL();
        }
        nColWidth = lcl_Round2DecPlaces( lcl_TwipsToPoints( nColTwips ) );
        if ( xModel.is() )
            nColWidth = nColWidth / defaultCharWidth;
    }
    nColWidth = lcl_Round2DecPlaces( nColWidth );
    return uno::makeAny( nColWidth );
}

void SAL_CALL
ScVbaRange::setColumnWidth( const uno::Any& _columnwidth ) throw (uno::RuntimeException)
{
    sal_Int32 nLen = m_Areas->getCount();
    if ( nLen > 1 )
    {
        for ( sal_Int32 index = 1; index != nLen; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(index) ), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->setColumnWidth( _columnwidth );
        }
        return;
    }
    double nColWidth = 0;
    _columnwidth >>= nColWidth;
    nColWidth = lcl_Round2DecPlaces( nColWidth );
        ScDocShell* pDocShell = getScDocShell();
        if ( pDocShell )
        {
                uno::Reference< frame::XModel > xModel = pDocShell->GetModel();
                if ( xModel.is() )
                {

            nColWidth = ( nColWidth * getDefaultCharWidth( xModel ) );
            RangeHelper thisRange( mxRange );
            table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
            USHORT nTwips = lcl_pointsToTwips( nColWidth );

            ScDocFunc aFunc(*pDocShell);
            SCCOLROW nColArr[2];
            nColArr[0] = thisAddress.StartColumn;
            nColArr[1] = thisAddress.EndColumn;
            aFunc.SetWidthOrHeight( TRUE, 1, nColArr, thisAddress.Sheet, SC_SIZE_ORIGINAL,
                                                                                nTwips, TRUE, TRUE );

        }
    }
}

uno::Any SAL_CALL
ScVbaRange::getWidth() throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xRange->getWidth();
    }
    uno::Reference< table::XColumnRowRange > xColRowRange( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xColRowRange->getColumns(), uno::UNO_QUERY_THROW );
    sal_Int32 nElems = xIndexAccess->getCount();
    double nWidth = 0;
    for ( sal_Int32 index=0; index<nElems; ++index )
    {
        uno::Reference< sheet::XCellRangeAddressable > xAddressable( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        double nTmpWidth = getCalcColWidth( xAddressable->getRangeAddress() );
        nWidth += nTmpWidth;
    }
    return uno::makeAny( nWidth );
}

uno::Any SAL_CALL
ScVbaRange::Areas( const uno::Any& item) throw (uno::RuntimeException)
{
    if ( !item.hasValue() )
        return uno::makeAny( m_Areas );
    return m_Areas->Item( item, uno::Any() );
}

uno::Reference< excel::XRange >
ScVbaRange::getArea( sal_Int32 nIndex ) throw( css::uno::RuntimeException )
{
    if ( !m_Areas.is() )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No areas available")), uno::Reference< uno::XInterface >() );
    uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( ++nIndex ), uno::Any() ), uno::UNO_QUERY_THROW );
    return xRange;
}

uno::Any
ScVbaRange::Borders( const uno::Any& item ) throw( script::BasicErrorException, uno::RuntimeException )
{
    if ( !item.hasValue() )
        return uno::makeAny( getBorders() );
    return getBorders()->Item( item, uno::Any() );
}

uno::Any SAL_CALL
ScVbaRange::BorderAround( const css::uno::Any& LineStyle, const css::uno::Any& Weight,
                const css::uno::Any& ColorIndex, const css::uno::Any& Color ) throw (css::uno::RuntimeException)
{
    sal_Int32 nCount = getBorders()->getCount();

    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        const sal_Int32 nLineType = supportedIndexTable[i];
        switch( nLineType )
        {
            case excel::XlBordersIndex::xlEdgeLeft:
            case excel::XlBordersIndex::xlEdgeTop:
            case excel::XlBordersIndex::xlEdgeBottom:
            case excel::XlBordersIndex::xlEdgeRight:
            {
                uno::Reference< excel::XBorder > xBorder( m_Borders->Item( uno::makeAny( nLineType ), uno::Any() ), uno::UNO_QUERY_THROW );
                if( LineStyle.hasValue() )
                {
                    xBorder->setLineStyle( LineStyle );
                }
                if( Weight.hasValue() )
                {
                    xBorder->setWeight( Weight );
                }
                if( ColorIndex.hasValue() )
                {
                    xBorder->setColorIndex( ColorIndex );
                }
                if( Color.hasValue() )
                {
                    xBorder->setColor( Color );
                }
                break;
            }
            case excel::XlBordersIndex::xlInsideVertical:
            case excel::XlBordersIndex::xlInsideHorizontal:
            case excel::XlBordersIndex::xlDiagonalDown:
            case excel::XlBordersIndex::xlDiagonalUp:
                break;
            default:
                return uno::makeAny( sal_False );
        }
    }
    return uno::makeAny( sal_True );
}

uno::Any SAL_CALL
ScVbaRange::getRowHeight() throw (uno::RuntimeException)
{
    sal_Int32 nLen = m_Areas->getCount();
    if ( nLen > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xRange->getRowHeight();
    }

    // if this range is a 'Rows' range, then if any row's RowHeight in the
    // range is different from any other then return NULL
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();

    sal_Int32 nStartRow = thisAddress.StartRow;
    sal_Int32 nEndRow = thisAddress.EndRow;
    double nHeight = getCalcRowHeight( thisAddress );
    // #TODO probably possible to use the SfxItemSet ( and see if
    //  SFX_ITEM_DONTCARE is set ) to improve performance
    if ( mbIsRows )
    {
        for ( sal_Int32 nRow = nStartRow ; nRow <= nEndRow; ++nRow )
        {
            thisAddress.StartRow = nRow;
            double nCurHeight = getCalcRowHeight( thisAddress );
            if ( nHeight != nCurHeight )
                return aNULL();
        }
    }
    return uno::makeAny( nHeight );
}

void SAL_CALL
ScVbaRange::setRowHeight( const uno::Any& _rowheight) throw (uno::RuntimeException)
{
    sal_Int32 nLen = m_Areas->getCount();
    if ( nLen > 1 )
    {
        for ( sal_Int32 index = 1; index != nLen; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(index) ), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->setRowHeight( _rowheight );
        }
        return;
    }
    double nHeight = 0; // Incomming height is in points
        _rowheight >>= nHeight;
    nHeight = lcl_Round2DecPlaces( nHeight );
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    USHORT nTwips = lcl_pointsToTwips( nHeight );

    ScDocShell* pDocShell = getDocShellFromRange( mxRange );
    ScDocFunc aFunc(*pDocShell);
    SCCOLROW nRowArr[2];
    nRowArr[0] = thisAddress.StartRow;
    nRowArr[1] = thisAddress.EndRow;
    aFunc.SetWidthOrHeight( FALSE, 1, nRowArr, thisAddress.Sheet, SC_SIZE_ORIGINAL,
                                                                        nTwips, TRUE, TRUE );
}

uno::Any SAL_CALL
ScVbaRange::getPageBreak() throw (uno::RuntimeException)
{
    sal_Int32 nPageBreak = excel::XlPageBreak::xlPageBreakNone;
    ScDocShell* pShell = getDocShellFromRange( mxRange );
    if ( pShell )
    {
        RangeHelper thisRange( mxRange );
        table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        BOOL bColumn = FALSE;

        if (thisAddress.StartRow==0)
            bColumn = TRUE;

        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        if ( xModel.is() )
        {
            ScDocument* pDoc =  getDocumentFromRange( mxRange );

            BYTE nFlag = 0;
            if ( !bColumn )
                nFlag = pDoc -> GetRowFlags(thisAddress.StartRow, thisAddress.Sheet);
            else
                nFlag = pDoc -> GetColFlags(static_cast<SCCOL>(thisAddress.StartColumn), thisAddress.Sheet);

            if ( nFlag & CR_PAGEBREAK)
                nPageBreak = excel::XlPageBreak::xlPageBreakAutomatic;

            if ( nFlag & CR_MANUALBREAK)
                nPageBreak = excel::XlPageBreak::xlPageBreakManual;
        }
    }

    return uno::makeAny( nPageBreak );
}

void SAL_CALL
ScVbaRange::setPageBreak( const uno::Any& _pagebreak) throw (uno::RuntimeException)
{
    sal_Int32 nPageBreak = 0;
    _pagebreak >>= nPageBreak;

    ScDocShell* pShell = getDocShellFromRange( mxRange );
    if ( pShell )
    {
        RangeHelper thisRange( mxRange );
        table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        if ((thisAddress.StartColumn==0) && (thisAddress.StartRow==0))
            return;
        BOOL bColumn = FALSE;

        if (thisAddress.StartRow==0)
            bColumn = TRUE;

        ScAddress aAddr( static_cast<SCCOL>(thisAddress.StartColumn), thisAddress.StartRow, thisAddress.Sheet );
        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        if ( xModel.is() )
        {
            ScTabViewShell* pViewShell = getBestViewShell( xModel );
            if ( nPageBreak == excel::XlPageBreak::xlPageBreakManual )
                pViewShell->InsertPageBreak( bColumn, TRUE, &aAddr);
            else if ( nPageBreak == excel::XlPageBreak::xlPageBreakNone )
                pViewShell->DeletePageBreak( bColumn, TRUE, &aAddr);
        }
    }
}

uno::Any SAL_CALL
ScVbaRange::getHeight() throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
        return xRange->getHeight();
    }

    uno::Reference< table::XColumnRowRange > xColRowRange( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xColRowRange->getRows(), uno::UNO_QUERY_THROW );
    sal_Int32 nElems = xIndexAccess->getCount();
    double nHeight = 0;
    for ( sal_Int32 index=0; index<nElems; ++index )
    {
            uno::Reference< sheet::XCellRangeAddressable > xAddressable( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        nHeight += getCalcRowHeight(xAddressable->getRangeAddress() );
    }
    return uno::makeAny( nHeight );
}

awt::Point
ScVbaRange::getPosition() throw ( uno::RuntimeException )
{
        awt::Point aPoint;
    uno::Reference< beans::XPropertySet > xProps;
    if ( mxRange.is() )
        xProps.set( mxRange, uno::UNO_QUERY_THROW );
    else
        xProps.set( mxRanges, uno::UNO_QUERY_THROW );
    xProps->getPropertyValue(POSITION) >>= aPoint;
    return aPoint;
}
uno::Any SAL_CALL
ScVbaRange::getLeft() throw (uno::RuntimeException)
{
    // helperapi returns the first ranges left ( and top below )
    if ( m_Areas->getCount() > 1 )
        return getArea( 0 )->getLeft();
        awt::Point aPoint = getPosition();
    return uno::makeAny( lcl_hmmToPoints( aPoint.X ) );
}


uno::Any SAL_CALL
ScVbaRange::getTop() throw (uno::RuntimeException)
{
    // helperapi returns the first ranges top
    if ( m_Areas->getCount() > 1 )
        return getArea( 0 )->getTop();
        awt::Point aPoint= getPosition();
    return uno::makeAny( lcl_hmmToPoints( aPoint.Y ) );
}

uno::Reference< excel::XWorksheet >
ScVbaRange::getWorksheet() throw (uno::RuntimeException)
{
    // #TODO #FIXME parent should always be set up ( currently thats not
    // the case )
    uno::Reference< excel::XWorksheet > xSheet( getParent(), uno::UNO_QUERY );
    if ( !xSheet.is() )
    {
        uno::Reference< table::XCellRange > xRange = mxRange;

        if ( mxRanges.is() ) // assign xRange to first range
        {
            uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW );
            xRange.set( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        }
        ScDocShell* pDocShell = getDocShellFromRange(xRange);
        RangeHelper rHelper(xRange);
        // parent should be Thisworkbook
           xSheet.set( new ScVbaWorksheet( uno::Reference< vba::XHelperInterface >(), mxContext,rHelper.getSpreadSheet(),pDocShell->GetModel()) );
    }
    return xSheet;
}

ScCellRangesBase*
ScVbaRange::getCellRangesBase() throw( uno::RuntimeException )
{
    ScCellRangesBase* pUnoRangesBase = NULL;
    if ( mxRanges.is() )
    {
        uno::Reference< uno::XInterface > xIf( mxRanges, uno::UNO_QUERY_THROW );
        pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xIf.get() );
    }
    else if ( mxRange.is() )
    {
        uno::Reference< uno::XInterface > xIf( mxRange, uno::UNO_QUERY_THROW );
        pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xIf.get() );
    }
    else
        throw uno::RuntimeException( rtl::OUString::createFromAscii("General Error creating range - Unknown" ), uno::Reference< uno::XInterface >() );
    return pUnoRangesBase;
}

// #TODO remove this ugly application processing
// Process an application Range request e.g. 'Range("a1,b2,a4:b6")
uno::Reference< excel::XRange >
ScVbaRange::ApplicationRange( const uno::Reference< uno::XComponentContext >& xContext, const css::uno::Any &Cell1, const css::uno::Any &Cell2 ) throw (css::uno::RuntimeException)
{
    // Althought the documentation seems clear that Range without a
    // qualifier then its a shortcut for ActiveSheet.Range
    // however, similarly Application.Range is apparently also a
    // shortcut for ActiveSheet.Range
    // The is however a subtle behavioural difference I've come across
    // wrt to named ranges.
    // If a named range "test" exists { Sheet1!$A1 } and the active sheet
    // is Sheet2 then the following will fail
    // msgbox ActiveSheet.Range("test").Address ' failes
    // msgbox WorkSheets("Sheet2").Range("test").Address
    // but !!!
    // msgbox Range("test").Address ' works
    // msgbox Application.Range("test").Address ' works

    // Single param Range
    rtl::OUString sRangeName;
    Cell1 >>= sRangeName;
    if ( Cell1.hasValue() && !Cell2.hasValue() && sRangeName.getLength() )
    {
        const static rtl::OUString sNamedRanges( RTL_CONSTASCII_USTRINGPARAM("NamedRanges"));
        uno::Reference< beans::XPropertySet > xPropSet( getCurrentDocument(), uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameAccess > xNamed( xPropSet->getPropertyValue( sNamedRanges ), uno::UNO_QUERY_THROW );
        uno::Reference< sheet::XCellRangeReferrer > xReferrer;
        try
        {
            xReferrer.set ( xNamed->getByName( sRangeName ), uno::UNO_QUERY );
        }
        catch( uno::Exception& /*e*/ )
        {
            // do nothing
        }
        if ( xReferrer.is() )
        {
            uno::Reference< table::XCellRange > xRange = xReferrer->getReferredCells();
            if ( xRange.is() )
            {
                // #FIXME need proper (WorkSheet) parent
                uno::Reference< excel::XRange > xVbRange =  new  ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext, xRange );
                return xVbRange;
            }
        }
    }
    uno::Reference< sheet::XSpreadsheetView > xView( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY );
    uno::Reference< table::XCellRange > xSheetRange( xView->getActiveSheet(), uno::UNO_QUERY_THROW );
    ScVbaRange* pRange = new ScVbaRange( uno::Reference< vba::XHelperInterface >(), xContext, xSheetRange );
    uno::Reference< excel::XRange > xVbSheetRange( pRange );
    return pRange->Range( Cell1, Cell2, true );
}

uno::Reference< sheet::XDatabaseRanges >
lcl_GetDataBaseRanges( ScDocShell* pShell ) throw ( uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel;
    if ( pShell )
        xModel.set( pShell->GetModel(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xModelProps( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XDatabaseRanges > xDBRanges( xModelProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DatabaseRanges") ) ), uno::UNO_QUERY_THROW );
    return xDBRanges;
}
// returns the XDatabaseRange for the autofilter on sheet (nSheet)
// also populates sName with the name of range
uno::Reference< sheet::XDatabaseRange >
lcl_GetAutoFiltRange( ScDocShell* pShell, sal_Int16 nSheet, rtl::OUString& sName )
{
    uno::Reference< container::XIndexAccess > xIndexAccess( lcl_GetDataBaseRanges( pShell ), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XDatabaseRange > xDataBaseRange;
    table::CellRangeAddress dbAddress;
    for ( sal_Int32 index=0; index < xIndexAccess->getCount(); ++index )
    {
        uno::Reference< sheet::XDatabaseRange > xDBRange( xIndexAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        uno::Reference< container::XNamed > xNamed( xDBRange, uno::UNO_QUERY_THROW );
        // autofilters work weirdly with openoffice, unnamed is the default
        // named range which is used to create an autofilter, but
        // its also possible that another name could be used
        //     this also causes problems when an autofilter is created on
        //     another sheet
        // ( but.. you can use any named range )
        dbAddress = xDBRange->getDataArea();
        if ( dbAddress.Sheet == nSheet )
        {
            sal_Bool bHasAuto = sal_False;
            uno::Reference< beans::XPropertySet > xProps( xDBRange, uno::UNO_QUERY_THROW );
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutoFilter") ) ) >>= bHasAuto;
            if ( bHasAuto )
            {
                sName = xNamed->getName();
                xDataBaseRange=xDBRange;
                break;
            }
        }
    }
    return xDataBaseRange;
}

// Helper functions for AutoFilter
ScDBData* lcl_GetDBData_Impl( ScDocShell* pDocShell, sal_Int16 nSheet )
{
    rtl::OUString sName;
    lcl_GetAutoFiltRange( pDocShell, nSheet, sName );
    OSL_TRACE("lcl_GetDBData_Impl got autofilter range %s for sheet %d",
        rtl::OUStringToOString( sName, RTL_TEXTENCODING_UTF8 ).getStr() , nSheet );
    ScDBData* pRet = NULL;
    if (pDocShell)
    {
        ScDBCollection* pNames = pDocShell->GetDocument()->GetDBCollection();
        if (pNames)
        {
            USHORT nPos = 0;
            if (pNames->SearchName( sName , nPos ))
                pRet = (*pNames)[nPos];
        }
    }
    return pRet;
}

void lcl_SelectAll( ScDocShell* pDocShell, ScQueryParam& aParam )
{
    if ( pDocShell )
    {
        ScViewData* pViewData = pDocShell->GetViewData();
        if ( pViewData )
        {
            OSL_TRACE("Pushing out SelectAll query");
            pViewData->GetView()->Query( aParam, NULL, TRUE );
        }
    }
}

ScQueryParam lcl_GetQueryParam( ScDocShell* pDocShell, sal_Int16 nSheet )
{
    ScDBData* pDBData = lcl_GetDBData_Impl( pDocShell, nSheet );
    ScQueryParam aParam;
    if (pDBData)
    {
        pDBData->GetQueryParam( aParam );
    }
    return aParam;
}

void lcl_SetAllQueryForField( ScQueryParam& aParam, SCCOLROW nField )
{
    bool bFound = false;
    SCSIZE i = 0;
    for (; i<MAXQUERY && !bFound; i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if ( rEntry.nField == nField)
        {
            OSL_TRACE("found at pos %d", i );
            bFound = true;
        }
    }
    if ( bFound )
    {
        OSL_TRACE("field %d to delete at pos %d", nField, ( i - 1 ) );
        aParam.DeleteQuery(--i);
    }
}


void lcl_SetAllQueryForField( ScDocShell* pDocShell, SCCOLROW nField, sal_Int16 nSheet )
{
    ScQueryParam aParam = lcl_GetQueryParam( pDocShell, nSheet );
    lcl_SetAllQueryForField( aParam, nField );
    lcl_SelectAll( pDocShell, aParam );
}

// Modifies sCriteria, and nOp depending on the value of sCriteria
void lcl_setTableFieldsFromCriteria( rtl::OUString& sCriteria1, uno::Reference< beans::XPropertySet >& xDescProps, sheet::TableFilterField& rFilterField )
{
    // #TODO make this more efficient and cycle through
    // sCriteria1 character by character to pick up <,<>,=, * etc.
    // right now I am more concerned with just getting it to work right

    sCriteria1 = sCriteria1.trim();
    // table of translation of criteria text to FilterOperators
    // <>searchtext - NOT_EQUAL
    //  =searchtext - EQUAL
    //  *searchtext - startwith
    //  <>*searchtext - doesn't startwith
    //  *searchtext* - contains
    //  <>*searchtext* - doesn't contain
    // [>|>=|<=|...]searchtext for GREATER_value, GREATER_EQUAL_value etc.
    sal_Int32 nPos = 0;
    bool bIsNumeric = false;
    if ( ( nPos = sCriteria1.indexOf( EQUALS ) ) == 0 )
    {
        if ( sCriteria1.getLength() == EQUALS.getLength() )
            rFilterField.Operator = sheet::FilterOperator_EMPTY;
        else
        {
            rFilterField.Operator = sheet::FilterOperator_EQUAL;
            sCriteria1 = sCriteria1.copy( EQUALS.getLength() );
            sCriteria1 = VBAToRegexp( sCriteria1 );
            // UseRegularExpressions
            if ( xDescProps.is() )
                xDescProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseRegularExpressions" ) ), uno::Any( sal_True ) );
        }

    }
    else if ( ( nPos = sCriteria1.indexOf( NOTEQUALS ) ) == 0 )
    {
        if ( sCriteria1.getLength() == NOTEQUALS.getLength() )
            rFilterField.Operator = sheet::FilterOperator_NOT_EMPTY;
        else
        {
            rFilterField.Operator = sheet::FilterOperator_NOT_EQUAL;
            sCriteria1 = sCriteria1.copy( NOTEQUALS.getLength() );
            sCriteria1 = VBAToRegexp( sCriteria1 );
            // UseRegularExpressions
            if ( xDescProps.is() )
                xDescProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseRegularExpressions" ) ), uno::Any( sal_True ) );
        }
    }
    else if ( ( nPos = sCriteria1.indexOf( GREATERTHAN ) ) == 0 )
    {
        bIsNumeric = true;
        if ( ( nPos = sCriteria1.indexOf( GREATERTHANEQUALS ) ) == 0 )
        {
            sCriteria1 = sCriteria1.copy( GREATERTHANEQUALS.getLength() );
            rFilterField.Operator = sheet::FilterOperator_GREATER_EQUAL;
        }
        else
        {
            sCriteria1 = sCriteria1.copy( GREATERTHAN.getLength() );
            rFilterField.Operator = sheet::FilterOperator_GREATER;
        }

    }
    else if ( ( nPos = sCriteria1.indexOf( LESSTHAN ) ) == 0 )
    {
        bIsNumeric = true;
        if ( ( nPos = sCriteria1.indexOf( LESSTHANEQUALS ) ) == 0 )
        {
            sCriteria1 = sCriteria1.copy( LESSTHANEQUALS.getLength() );
            rFilterField.Operator = sheet::FilterOperator_LESS_EQUAL;
        }
        else
        {
            sCriteria1 = sCriteria1.copy( LESSTHAN.getLength() );
            rFilterField.Operator = sheet::FilterOperator_LESS;
        }

    }
    else
        rFilterField.Operator = sheet::FilterOperator_EQUAL;

    if ( bIsNumeric )
    {
        rFilterField.IsNumeric= sal_True;
        rFilterField.NumericValue = sCriteria1.toDouble();
    }
    rFilterField.StringValue = sCriteria1;
}

void SAL_CALL
ScVbaRange::AutoFilter( const uno::Any& Field, const uno::Any& Criteria1, const uno::Any& Operator, const uno::Any& Criteria2, const uno::Any& VisibleDropDown ) throw (uno::RuntimeException)
{
    // Is there an existing autofilter
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    sal_Int16 nSheet = thisAddress.Sheet;
    ScDocShell* pShell = getScDocShell();
    sal_Bool bHasAuto = sal_False;
    rtl::OUString sAutofiltRangeName;
    uno::Reference< sheet::XDatabaseRange > xDataBaseRange = lcl_GetAutoFiltRange( pShell, nSheet, sAutofiltRangeName );
    if ( xDataBaseRange.is() )
        bHasAuto = true;

    uno::Reference< table::XCellRange > xFilterRange;
    if ( !bHasAuto )
    {
        if (  m_Areas->getCount() > 1 )
            throw uno::RuntimeException( STR_ERRORMESSAGE_APPLIESTOSINGLERANGEONLY, uno::Reference< uno::XInterface >() );

        table::CellRangeAddress autoFiltAddress;
        //CurrentRegion()
        if ( isSingleCellRange() )
        {
            uno::Reference< excel::XRange > xCurrent( CurrentRegion() );
            if ( xCurrent.is() )
            {
                ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xCurrent.get() );
                if ( pRange->isSingleCellRange() )
                    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Can't create AutoFilter") ), uno::Reference< uno::XInterface >() );
                if ( pRange )
                {
                    RangeHelper currentRegion( pRange->mxRange );
                    autoFiltAddress = currentRegion.getCellRangeAddressable()->getRangeAddress();
                }
            }
        }
        else // multi-cell range
        {
            RangeHelper multiCellRange( mxRange );
            autoFiltAddress = multiCellRange.getCellRangeAddressable()->getRangeAddress();
        }

        uno::Reference< sheet::XDatabaseRanges > xDBRanges = lcl_GetDataBaseRanges( pShell );
        if ( xDBRanges.is() )
        {
            rtl::OUString sGenName( RTL_CONSTASCII_USTRINGPARAM("VBA_Autofilter_") );
            sGenName += rtl::OUString::valueOf( static_cast< sal_Int32 >( nSheet ) );
            OSL_TRACE("Going to add new autofilter range.. name %s",
                rtl::OUStringToOString( sGenName, RTL_TEXTENCODING_UTF8 ).getStr() , nSheet );
            if ( !xDBRanges->hasByName( sGenName ) )
                xDBRanges->addNewByName(  sGenName, autoFiltAddress );
            xDataBaseRange.set( xDBRanges->getByName(  sGenName ), uno::UNO_QUERY_THROW );
        }
        if ( !xDataBaseRange.is() )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Failed to find the autofilter placeholder range" ) ), uno::Reference< uno::XInterface >() );

        uno::Reference< beans::XPropertySet > xDBRangeProps( xDataBaseRange, uno::UNO_QUERY_THROW );
        // set autofilt
        xDBRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutoFilter") ), uno::Any(sal_True) );
        // set header
        uno::Reference< beans::XPropertySet > xFiltProps( xDataBaseRange->getFilterDescriptor(), uno::UNO_QUERY_THROW );
        sal_Bool bHasColHeader = sal_False;
        ScDocument* pDoc = pShell ? pShell->GetDocument() : NULL;

        bHasColHeader = pDoc->HasColHeader(  static_cast< SCCOL >( autoFiltAddress.StartColumn ), static_cast< SCROW >( autoFiltAddress.StartRow ), static_cast< SCCOL >( autoFiltAddress.EndColumn ), static_cast< SCROW >( autoFiltAddress.EndRow ), static_cast< SCTAB >( autoFiltAddress.Sheet ) ) ? sal_True : sal_False;
        xFiltProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ContainsHeader") ), uno::Any( bHasColHeader ) );
    }


    sal_Int32 nField = 0; // *IS* 1 based
    rtl::OUString sCriteria1;
    sal_Int32 nOperator = excel::XlAutoFilterOperator::xlAnd;

    sal_Bool bVisible = sal_True;
    bool  bChangeDropDown = false;
    VisibleDropDown >>= bVisible;

    if ( bVisible == bHasAuto ) // dropdown is displayed/notdisplayed as
                                // required
        bVisible = sal_False;
    else
        bChangeDropDown = true;
    sheet::FilterConnection nConn = sheet::FilterConnection_AND;
    double nCriteria1 = 0;

    bool bHasCritValue = Criteria1.hasValue();
    bool bCritHasNumericValue = sal_False; // not sure if a numeric criteria is possible
    if ( bHasCritValue )
        bCritHasNumericValue = ( Criteria1 >>= nCriteria1 );

    if (  !Field.hasValue() && ( Criteria1.hasValue() || Operator.hasValue() || Criteria2.hasValue() ) )
        throw uno::RuntimeException();
    // Use the normal uno api, sometimes e.g. when you want to use ALL as the filter
    // we can't use refresh as the uno interface doesn't have a concept of ALL
    // in this case we just call the core calc functionality -
    bool bAll = false;;
    if ( ( Field >>= nField )  )
    {
        uno::Sequence< sheet::TableFilterField > sTabFilts;
        uno::Reference< sheet::XSheetFilterDescriptor > xDesc = xDataBaseRange->getFilterDescriptor();
        uno::Reference< beans::XPropertySet > xDescProps( xDesc, uno::UNO_QUERY_THROW );
        if ( Criteria1.hasValue() )
        {
            sTabFilts.realloc( 1 );
            sTabFilts[0].Operator = sheet::FilterOperator_EQUAL;// sensible default
            if ( !bCritHasNumericValue )
            {
                Criteria1 >>= sCriteria1;
                sTabFilts[0].IsNumeric = bCritHasNumericValue;
                if ( bHasCritValue && sCriteria1.getLength() )
                    lcl_setTableFieldsFromCriteria( sCriteria1, xDescProps, sTabFilts[0]  );
                else
                    bAll = true;
            }
            else // numeric
            {
                sTabFilts[0].IsNumeric = sal_True;
                sTabFilts[0].NumericValue = nCriteria1;
            }
        }
        else // no value specified
            bAll = true;
        // not sure what the relationship between Criteria1 and Operator is,
        // e.g. can you have a Operator without a Criteria ? in openoffice it
        if ( Operator.hasValue()  && ( Operator >>= nOperator ) )
        {
            // if its a bottom/top Ten(Percent/Value) and there
            // is no value specified for critera1 set it to 10
            if ( !bCritHasNumericValue && !sCriteria1.getLength() && ( nOperator != excel::XlAutoFilterOperator::xlOr ) && ( nOperator != excel::XlAutoFilterOperator::xlAnd ) )
            {
                sTabFilts[0].IsNumeric = sal_True;
                sTabFilts[0].NumericValue = 10;
                bAll = false;
            }
            switch ( nOperator )
            {
                case excel::XlAutoFilterOperator::xlBottom10Items:
                    sTabFilts[0].Operator = sheet::FilterOperator_BOTTOM_VALUES;
                    break;
                case excel::XlAutoFilterOperator::xlBottom10Percent:
                    sTabFilts[0].Operator = sheet::FilterOperator_BOTTOM_PERCENT;
                    break;
                case excel::XlAutoFilterOperator::xlTop10Items:
                    sTabFilts[0].Operator = sheet::FilterOperator_TOP_VALUES;
                    break;
                case excel::XlAutoFilterOperator::xlTop10Percent:
                    sTabFilts[0].Operator = sheet::FilterOperator_TOP_PERCENT;
                    break;
                case excel::XlAutoFilterOperator::xlOr:
                    nConn = sheet::FilterConnection_OR;
                    break;
                case excel::XlAutoFilterOperator::xlAnd:
                    nConn = sheet::FilterConnection_AND;
                    break;
                default:
                    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UnknownOption") ), uno::Reference< uno::XInterface >() );

            }

        }
        if ( !bAll )
        {
            sTabFilts[0].Connection = sheet::FilterConnection_AND;
            sTabFilts[0].Field = (nField - 1);

            rtl::OUString sCriteria2;
            if ( Criteria2.hasValue() ) // there is a Criteria2
            {
                sTabFilts.realloc(2);
                sTabFilts[1].Field = sTabFilts[0].Field;
                sTabFilts[1].Connection = nConn;

                if ( Criteria2 >>= sCriteria2 )
                {
                    if ( sCriteria2.getLength() > 0 )
                    {
                        uno::Reference< beans::XPropertySet > xProps;
                        lcl_setTableFieldsFromCriteria( sCriteria2, xProps,  sTabFilts[1] );
                        sTabFilts[1].IsNumeric = sal_False;
                    }
                }
                else // numeric
                {
                    Criteria2 >>= sTabFilts[1].NumericValue;
                    sTabFilts[1].IsNumeric = sal_True;
                    sTabFilts[1].Operator = sheet::FilterOperator_EQUAL;
                }
            }
        }

        xDesc->setFilterFields( sTabFilts );
        if ( !bAll )
        {
            xDataBaseRange->refresh();
        }
        else
            // was 0 based now seems to be 1
            lcl_SetAllQueryForField( pShell, nField, nSheet );
    }
    else
    {
        // this is just to toggle autofilter on and off ( not to be confused with
        // a VisibleDropDown option combined with a field, in that case just the
        // button should be disabled ) - currently we don't support that
        bChangeDropDown = true;
        uno::Reference< beans::XPropertySet > xDBRangeProps( xDataBaseRange, uno::UNO_QUERY_THROW );
        if ( bHasAuto )
        {
            // find the any field with the query and select all
            ScQueryParam aParam = lcl_GetQueryParam( pShell, nSheet );
            SCSIZE i = 0;
            for (; i<MAXQUERY; i++)
            {
                ScQueryEntry& rEntry = aParam.GetEntry(i);
                if ( rEntry.bDoQuery )
                    lcl_SetAllQueryForField( pShell, rEntry.nField, nSheet );
            }
            // remove exising filters
            xDataBaseRange->getFilterDescriptor()->setFilterFields( uno::Sequence< sheet::TableFilterField >() );
        }
        xDBRangeProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutoFilter") ), uno::Any(!bHasAuto) );

    }
}

void SAL_CALL
ScVbaRange::Insert( const uno::Any& Shift, const uno::Any& /*CopyOrigin*/ ) throw (uno::RuntimeException)
{
    // It appears ( from the web ) that the undocumented CopyOrigin
    // param should contain member of enum XlInsertFormatOrigin
    // which can have values xlFormatFromLeftOrAbove or xlFormatFromRightOrBelow
    // #TODO investigate resultant behaviour using these constants
    // currently just processing Shift

    sheet::CellInsertMode mode = sheet::CellInsertMode_NONE;
    if ( Shift.hasValue() )
    {
        sal_Int32 nShift = 0;
        Shift >>= nShift;
        switch ( nShift )
        {
            case excel::XlInsertShiftDirection::xlShiftToRight:
                mode = sheet::CellInsertMode_RIGHT;
                break;
            case excel::XlInsertShiftDirection::xlShiftDown:
                mode = sheet::CellInsertMode_DOWN;
                break;
            default:
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("Illegal paramater ") ), uno::Reference< uno::XInterface >() );
        }
    }
    else
    {
        if ( getRow() >=  getColumn() )
            mode = sheet::CellInsertMode_DOWN;
        else
            mode = sheet::CellInsertMode_RIGHT;
    }
    RangeHelper thisRange( mxRange );
    uno::Reference< sheet::XCellRangeMovement > xCellRangeMove( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
    xCellRangeMove->insertCells( thisRange.getCellRangeAddressable()->getRangeAddress(), mode );
}

void SAL_CALL
ScVbaRange::Autofit() throw (uno::RuntimeException)
{
    sal_Int32 nLen = m_Areas->getCount();
    if ( nLen > 1 )
    {
        for ( sal_Int32 index = 1; index != nLen; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(index) ), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->Autofit();
        }
        return;
    }
        // if the range is a not a row or column range autofit will
        // throw an error

        if ( !mbIsColumns  )
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
        ScDocShell* pDocShell = getDocShellFromRange( mxRange );
        if ( pDocShell )
        {
            RangeHelper thisRange( mxRange );
            table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();

            ScDocFunc aFunc(*pDocShell);
            SCCOLROW nColArr[2];
            nColArr[0] = thisAddress.StartColumn;
            nColArr[1] = thisAddress.EndColumn;
            BOOL bDirection = TRUE;
            if ( mbIsRows )
            {
                bDirection = FALSE;
                nColArr[0] = thisAddress.StartRow;
                nColArr[1] = thisAddress.EndRow;
            }
            aFunc.SetWidthOrHeight( bDirection, 1, nColArr, thisAddress.Sheet, SC_SIZE_OPTIMAL,
                                                                                0, TRUE, TRUE );

    }
}

/***************************************************************************************
 * interface for text:
 * com.sun.star.text.XText, com.sun.star.table.XCell, com.sun.star.container.XEnumerationAccess
 * com.sun.star.text.XTextRange,
 * the main problem is to recognize the numeric and date, which assosiate with DecimalSeparator, ThousandsSeparator,
 * TrailingMinusNumbers and FieldInfo.
***************************************************************************************/
void SAL_CALL
ScVbaRange::TextToColumns( const css::uno::Any& Destination, const css::uno::Any& DataType, const css::uno::Any& TextQualifier,
        const css::uno::Any& ConsecutinveDelimiter, const css::uno::Any& Tab, const css::uno::Any& Semicolon, const css::uno::Any& Comma,
        const css::uno::Any& Space, const css::uno::Any& Other, const css::uno::Any& OtherChar, const css::uno::Any& /*FieldInfo*/,
        const css::uno::Any& DecimalSeparator, const css::uno::Any& ThousandsSeparator, const css::uno::Any& /*TrailingMinusNumbers*/  ) throw (css::uno::RuntimeException)
{
    uno::Reference< excel::XRange > xRange;
    if( Destination.hasValue() )
    {
        if( !( Destination >>= xRange ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Destination parameter should be a range" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set range\n");
    }
    else
    {
        //set as current
        xRange = this;
        OSL_TRACE("set range as himself\n");
    }

   sal_Int16 xlTextParsingType = excel::XlTextParsingType::xlDelimited;
    if ( DataType.hasValue() )
    {
        if( !( DataType >>= xlTextParsingType ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "DataType parameter should be a short" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Datatype\n" );
    }
    sal_Bool bDilimited = ( xlTextParsingType == excel::XlTextParsingType::xlDelimited );

    sal_Int16 xlTextQualifier = excel::XlTextQualifier::xlTextQualifierDoubleQuote;
    if( TextQualifier.hasValue() )
    {
        if( !( TextQualifier >>= xlTextQualifier ))
             throw uno::RuntimeException( rtl::OUString::createFromAscii( "TextQualifier parameter should be a short" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set TextQualifier\n");
    }

    sal_Bool bConsecutinveDelimiter = sal_False;
    if( ConsecutinveDelimiter.hasValue() )
    {
        if( !( ConsecutinveDelimiter >>= bConsecutinveDelimiter ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "ConsecutinveDelimiter parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set ConsecutinveDelimiter\n");
    }

    sal_Bool bTab = sal_False;
    if( Tab.hasValue() && bDilimited )
    {
        if( !( Tab >>= bTab ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Tab parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Tab\n");
    }

    sal_Bool bSemicolon = sal_False;
    if( Semicolon.hasValue() && bDilimited )
    {
        if( !( Semicolon >>= bSemicolon ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Semicolon parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Semicolon\n");
    }
    sal_Bool bComma = sal_False;
    if( Comma.hasValue() && bDilimited )
    {
        if( !( Comma >>= bComma ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Comma parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Comma\n");
    }
    sal_Bool bSpace = sal_False;
    if( Space.hasValue() && bDilimited )
    {
        if( !( Space >>= bSpace ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Space parameter should be a boolean" ),
                    uno::Reference< uno::XInterface >() );
        OSL_TRACE("set Space\n");
    }
    sal_Bool bOther = sal_False;
    rtl::OUString sOtherChar;
    if( Other.hasValue() && bDilimited )
    {
        if( Other >>= bOther )
        {
            if( OtherChar.hasValue() )
                if( !( OtherChar >>= sOtherChar ) )
                    throw uno::RuntimeException( rtl::OUString::createFromAscii( "OtherChar parameter should be a String" ),
                        uno::Reference< uno::XInterface >() );
        OSL_TRACE("set OtherChar\n" );
        }
     else if( bOther )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "Other parameter should be a True" ),
                    uno::Reference< uno::XInterface >() );
    }
 //TODO* FieldInfo   Optional Variant. An array containing parse information for the individual columns of data. The interpretation depends on the value of DataType. When the data is delimited, this argument is an array of two-element arrays, with each two-element array specifying the conversion options for a particular column. The first element is the column number (1-based), and the second element is one of the xlColumnDataType  constants specifying how the column is parsed.

    rtl::OUString sDecimalSeparator;
    if( DecimalSeparator.hasValue() )
    {
        if( !( DecimalSeparator >>= sDecimalSeparator ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "DecimalSeparator parameter should be a String" ),
                uno::Reference< uno::XInterface >() );
        OSL_TRACE("set DecimalSeparator\n" );
    }
    rtl::OUString sThousandsSeparator;
    if( ThousandsSeparator.hasValue() )
    {
        if( !( ThousandsSeparator >>= sThousandsSeparator ) )
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "ThousandsSeparator parameter should be a String" ),
                uno::Reference< uno::XInterface >() );
        OSL_TRACE("set ThousandsSpeparator\n" );
    }
 //TODO* TrailingMinusNumbers  Optional Variant. Numbers that begin with a minus character.
}

css::uno::Reference< excel::XValidation > SAL_CALL
ScVbaRange::getValidation() throw (css::uno::RuntimeException)
{
    if ( !m_xValidation.is() )
        m_xValidation = new ScVbaValidation( this, mxContext, mxRange );
    return m_xValidation;
}

uno::Any ScVbaRange::getFormulaHidden() throw ( script::BasicErrorException, css::uno::RuntimeException)
{
    SfxItemSet* pDataSet = getCurrentDataSet();
    const ScProtectionAttr& rProtAttr = (const ScProtectionAttr &)
        pDataSet->Get(ATTR_PROTECTION, TRUE);
    SfxItemState eState = pDataSet->GetItemState(ATTR_PROTECTION, TRUE, NULL);
    if(eState == SFX_ITEM_DONTCARE)
        return aNULL();
    return uno::makeAny(rProtAttr.GetHideFormula());

}
void ScVbaRange::setFormulaHidden(const uno::Any& Hidden) throw ( script::BasicErrorException, css::uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW);
    util::CellProtection rCellAttr;
    xProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLPRO))) >>= rCellAttr;
    Hidden >>= rCellAttr.IsFormulaHidden;
    xProps->setPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLPRO)), uno::makeAny(rCellAttr));
}


void SAL_CALL
ScVbaRange::PrintOut( const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& ActivePrinter, const uno::Any& PrintToFile, const uno::Any& Collate, const uno::Any& PrToFileName ) throw (uno::RuntimeException)
{
    ScDocShell* pShell = NULL;

    sal_Int32 nItems = m_Areas->getCount();
    uno::Sequence<  table::CellRangeAddress > printAreas( nItems );
    uno::Reference< sheet::XPrintAreas > xPrintAreas;
    for ( sal_Int32 index=1; index <= nItems; ++index )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );

        RangeHelper thisRange( xRange->getCellRange() );
        table::CellRangeAddress rangeAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        if ( index == 1 )
        {
            ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() );
            // initialise the doc shell and the printareas
            pShell = getDocShellFromRange( pRange->mxRange );
            xPrintAreas.set( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
        }
        printAreas[ index - 1 ] = rangeAddress;
    }
    if ( pShell )
    {
        if ( xPrintAreas.is() )
        {
            xPrintAreas->setPrintAreas( printAreas );
            uno::Reference< frame::XModel > xModel = pShell->GetModel();
            PrintOutHelper( From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, xModel, sal_True );
        }
    }
}

void SAL_CALL
ScVbaRange::AutoFill(  const uno::Reference< excel::XRange >& Destination, const uno::Any& Type ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xDest( Destination, uno::UNO_QUERY_THROW );
    ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xDest.get() );
    RangeHelper destRangeHelper( pRange->mxRange );
    table::CellRangeAddress destAddress = destRangeHelper.getCellRangeAddressable()->getRangeAddress();

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    ScRange sourceRange;
    ScRange destRange;

    ScUnoConversion::FillScRange( destRange, destAddress );
    ScUnoConversion::FillScRange( sourceRange, thisAddress );


    // source is valid
//  if (  !sourceRange.In( destRange ) )
//      throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "source not in destination" ) ), uno::Reference< uno::XInterface >() );

    FillDir eDir = FILL_TO_BOTTOM;
    double fStep = 1.0;

    ScRange aRange( destRange );
    ScRange aSourceRange( destRange );

    // default to include the number of Rows in the source range;
    SCCOLROW nSourceCount = ( sourceRange.aEnd.Row() - sourceRange.aStart.Row() ) + 1;
    SCCOLROW nCount = 0;

    if ( sourceRange != destRange )
    {
        // Find direction of fill, vertical or horizontal
        if ( sourceRange.aStart == destRange.aStart )
        {
            if ( sourceRange.aEnd.Row() == destRange.aEnd.Row() )
            {
                nSourceCount = ( sourceRange.aEnd.Col() - sourceRange.aStart.Col() + 1 );
                aSourceRange.aEnd.SetCol( static_cast<SCCOL>( aSourceRange.aStart.Col() + nSourceCount - 1 ) );
                eDir = FILL_TO_RIGHT;
                nCount = aRange.aEnd.Col() - aSourceRange.aEnd.Col();
            }
            else if ( sourceRange.aEnd.Col() == destRange.aEnd.Col() )
            {
                aSourceRange.aEnd.SetRow( static_cast<SCROW>( aSourceRange.aStart.Row() + nSourceCount ) - 1 );
                nCount = aRange.aEnd.Row() - aSourceRange.aEnd.Row();
                eDir = FILL_TO_BOTTOM;
            }
        }

        else if ( aSourceRange.aEnd == destRange.aEnd )
        {
            if ( sourceRange.aStart.Col() == destRange.aStart.Col() )
            {
                aSourceRange.aStart.SetRow( static_cast<SCROW>( aSourceRange.aEnd.Row() - nSourceCount + 1 ) );
                nCount = aSourceRange.aStart.Row() - aRange.aStart.Row();
                eDir = FILL_TO_TOP;
                fStep = -fStep;
            }
            else if ( sourceRange.aStart.Row() == destRange.aStart.Row() )
            {
                nSourceCount = ( sourceRange.aEnd.Col() - sourceRange.aStart.Col() ) + 1;
                aSourceRange.aStart.SetCol( static_cast<SCCOL>( aSourceRange.aEnd.Col() - nSourceCount + 1 ) );
                nCount = aSourceRange.aStart.Col() - aRange.aStart.Col();
                eDir = FILL_TO_LEFT;
                fStep = -fStep;
            }
        }
    }
    ScDocShell* pDocSh= getDocShellFromRange( mxRange );

    FillCmd eCmd = FILL_AUTO;
    FillDateCmd eDateCmd = FILL_DAY;

#ifdef VBA_OOBUILD_HACK
    double fEndValue =  MAXDOUBLE;
#endif

    if ( Type.hasValue() )
    {
        sal_Int16 nFillType = excel::XlAutoFillType::xlFillDefault;
        Type >>= nFillType;
        switch ( nFillType )
        {
            case excel::XlAutoFillType::xlFillCopy:
                eCmd =  FILL_SIMPLE;
                fStep = 0.0;
                break;
            case excel::XlAutoFillType::xlFillDays:
                eCmd = FILL_DATE;
                break;
            case excel::XlAutoFillType::xlFillMonths:
                eCmd = FILL_DATE;
                eDateCmd = FILL_MONTH;
                break;
            case excel::XlAutoFillType::xlFillWeekdays:
                eCmd = FILL_DATE;
                eDateCmd = FILL_WEEKDAY;
                break;
            case excel::XlAutoFillType::xlFillYears:
                eCmd = FILL_DATE;
                eDateCmd = FILL_YEAR;
                break;
            case excel::XlAutoFillType::xlGrowthTrend:
                eCmd = FILL_GROWTH;
                break;
            case excel::XlAutoFillType::xlFillFormats:
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "xlFillFormat not supported for AutoFill" ) ), uno::Reference< uno::XInterface >() );
            case excel::XlAutoFillType::xlFillValues:
            case excel::XlAutoFillType::xlFillSeries:
            case excel::XlAutoFillType::xlLinearTrend:
                eCmd = FILL_LINEAR;
                break;
            case excel::XlAutoFillType::xlFillDefault:
            default:
                eCmd =  FILL_AUTO;
                break;
        }
    }
    ScDocFunc aFunc(*pDocSh);
#ifdef VBA_OOBUILD_HACK
    aFunc.FillAuto( aSourceRange, NULL, eDir, eCmd, eDateCmd, nCount, fStep, fEndValue, TRUE, TRUE );
#endif
}
sal_Bool SAL_CALL
ScVbaRange::GoalSeek( const uno::Any& Goal, const uno::Reference< excel::XRange >& ChangingCell ) throw (uno::RuntimeException)
{
    ScDocShell* pDocShell = getScDocShell();
    sal_Bool bRes = sal_True;
    ScVbaRange* pRange = static_cast< ScVbaRange* >( ChangingCell.get() );
    if ( pDocShell && pRange )
    {
        uno::Reference< sheet::XGoalSeek > xGoalSeek(  pDocShell->GetModel(), uno::UNO_QUERY_THROW );
        RangeHelper thisRange( mxRange );
        table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        RangeHelper changingCellRange( pRange->mxRange );
        table::CellRangeAddress changingCellAddr = changingCellRange.getCellRangeAddressable()->getRangeAddress();
        rtl::OUString sGoal = getAnyAsString( Goal );
        table::CellAddress thisCell( thisAddress.Sheet, thisAddress.StartColumn, thisAddress.StartRow );
        table::CellAddress changingCell( changingCellAddr.Sheet, changingCellAddr.StartColumn, changingCellAddr.StartRow );
        sheet::GoalResult res = xGoalSeek->seekGoal( thisCell, changingCell, sGoal );
        ChangingCell->setValue( uno::makeAny( res.Result ) );

        // openoffice behaves differently, result is 0 if the divergence is too great
                // but... if it detects 0 is the value it requires then it will use that
        // e.g. divergence & result both = 0.0 does NOT mean there is an error
        if ( ( res.Divergence != 0.0 ) && ( res.Result == 0.0 ) )
            bRes = sal_False;
    }
    else
        bRes = sal_False;
    return bRes;
}

void
ScVbaRange::Calculate(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    getWorksheet()->Calculate();
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::Item( const uno::Any& row, const uno::Any& column ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( mbIsRows || mbIsColumns )
    {
        if ( column.hasValue() )
            DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
        uno::Reference< excel::XRange > xRange;
        if ( mbIsColumns )
            xRange = Columns( row );
        else
            xRange = Rows( row );
        return xRange;
    }
    return Cells( row, column );
}

void
ScVbaRange::AutoOutline(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #TODO #FIXME needs to check for summary row/col ( whatever they are )
    // not valid for multi Area Addresses
    if ( m_Areas->getCount() )
        DebugHelper::exception(SbERR_METHOD_FAILED, STR_ERRORMESSAGE_APPLIESTOSINGLERANGEONLY);
    // So needs to either span an entire Row or a just be a single cell
    // ( that contains a summary RowColumn )
    // also the Single cell cause doesn't seem to be handled specially in
    // this code ( ported from the helperapi RangeImpl.java,
    // RangeRowsImpl.java, RangesImpl.java, RangeSingleCellImpl.java
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();

    if ( isSingleCellRange() || mbIsRows )
    {
        uno::Reference< sheet::XSheetOutline > xSheetOutline( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
                xSheetOutline->autoOutline( thisAddress );
    }
    else
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
}

void SAL_CALL
ScVbaRange:: ClearOutline(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->ClearOutline();
        }
        return;
    }
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    uno::Reference< sheet::XSheetOutline > xSheetOutline( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
        xSheetOutline->clearOutline();
}

void
ScVbaRange::groupUnGroup( bool bUnGroup ) throw ( script::BasicErrorException, uno::RuntimeException )
{
    if ( m_Areas->getCount() > 1 )
         DebugHelper::exception(SbERR_METHOD_FAILED, STR_ERRORMESSAGE_APPLIESTOSINGLERANGEONLY);
    table::TableOrientation nOrient = table::TableOrientation_ROWS;
    if ( mbIsColumns )
        nOrient = table::TableOrientation_COLUMNS;
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    uno::Reference< sheet::XSheetOutline > xSheetOutline( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
    if ( bUnGroup )
            xSheetOutline->ungroup( thisAddress, nOrient );
    else
            xSheetOutline->group( thisAddress, nOrient );
}

void SAL_CALL
ScVbaRange::Group(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    groupUnGroup();
}
void SAL_CALL
ScVbaRange::Ungroup(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    groupUnGroup(true);
}

void lcl_mergeCellsOfRange( const uno::Reference< table::XCellRange >& xCellRange, sal_Bool _bMerge = sal_True ) throw ( uno::RuntimeException )
{
        uno::Reference< util::XMergeable > xMergeable( xCellRange, uno::UNO_QUERY_THROW );
        xMergeable->merge(_bMerge);
}
void SAL_CALL
ScVbaRange::Merge( const uno::Any& Across ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->Merge(Across);
        }
        return;
    }
    uno::Reference< table::XCellRange > oCellRange;
    sal_Bool bAcross = sal_False;
    Across >>= bAcross;
    if ( !bAcross )
        lcl_mergeCellsOfRange( mxRange );
    else
    {
        uno::Reference< excel::XRange > oRangeRowsImpl = Rows( uno::Any() );
        // #TODO #FIXME this seems incredibly lame, this can't be right
        for (sal_Int32 i=1; i <= oRangeRowsImpl->getCount();i++)
        {
                       oRangeRowsImpl->Cells( uno::makeAny( i ), uno::Any() )->Merge( uno::makeAny( sal_False ) );
               }
    }
}

void SAL_CALL
ScVbaRange::UnMerge(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        sal_Int32 nItems = m_Areas->getCount();
        for ( sal_Int32 index=1; index <= nItems; ++index )
        {
            uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
            xRange->UnMerge();
        }
        return;
    }
    lcl_mergeCellsOfRange( mxRange, sal_False);
}

uno::Any SAL_CALL
ScVbaRange::getStyle() throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32( 1 ) ), uno::Any() ), uno::UNO_QUERY_THROW  );
        return xRange->getStyle();
    }
    uno::Reference< beans::XPropertySet > xProps( mxRange, uno::UNO_QUERY_THROW );
    rtl::OUString sStyleName;
    ScDocShell* pShell = getScDocShell();
    uno::Reference< frame::XModel > xModel( pShell->GetModel() );
    uno::Reference< excel::XStyle > xStyle = new ScVbaStyle( this, mxContext,  sStyleName, xModel );
    return uno::makeAny( xStyle );
}
void SAL_CALL
ScVbaRange::setStyle( const uno::Any& _style ) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32( 1 ) ), uno::Any() ), uno::UNO_QUERY_THROW );
        xRange->setStyle( _style );
        return;
    }
    uno::Reference< beans::XPropertySet > xProps( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XStyle > xStyle;
    _style >>= xStyle;
    xProps->setPropertyValue(CELLSTYLE, uno::makeAny(xStyle->getName()));
}

uno::Reference< excel::XRange >
ScVbaRange::PreviousNext( bool bIsPrevious )
{
    ScMarkData markedRange;
    ScRange refRange;
    RangeHelper thisRange( mxRange );

    ScUnoConversion::FillScRange( refRange, thisRange.getCellRangeAddressable()->getRangeAddress());
    markedRange. SetMarkArea( refRange );
    short nMove = bIsPrevious ? -1 : 1;

    SCCOL nNewX = refRange.aStart.Col();
    SCROW nNewY = refRange.aStart.Row();
    SCTAB nTab = refRange.aStart.Tab();

    ScDocument* pDoc = getScDocument();
    pDoc->GetNextPos( nNewX,nNewY, nTab, nMove,0, TRUE,TRUE, markedRange );
    refRange.aStart.SetCol( nNewX );
    refRange.aStart.SetRow( nNewY );
    refRange.aStart.SetTab( nTab );
    refRange.aEnd.SetCol( nNewX );
    refRange.aEnd.SetRow( nNewY );
    refRange.aEnd.SetTab( nTab );

    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getScDocShell() , refRange ) );

    return new ScVbaRange( getParent(), mxContext, xRange );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::Next() throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32( 1 ) ), uno::Any() ) , uno::UNO_QUERY_THROW  );
        return xRange->Next();
    }
    return PreviousNext( false );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::Previous() throw (script::BasicErrorException, uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
    {
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32( 1 ) ), uno::Any() ), uno::UNO_QUERY_THROW  );
        return xRange->Previous();
    }
    return PreviousNext( true );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::SpecialCells( const uno::Any& _oType, const uno::Any& _oValue) throw ( script::BasicErrorException )
{
    bool bIsSingleCell = isSingleCellRange();
    bool bIsMultiArea = ( m_Areas->getCount() > 1 );
    ScVbaRange* pRangeToUse = this;
    sal_Int32 nType = 0;
    if ( !( _oType >>= nType ) )
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    switch(nType)
    {
        case excel::XlCellType::xlCellTypeSameFormatConditions:
        case excel::XlCellType::xlCellTypeAllValidation:
        case excel::XlCellType::xlCellTypeSameValidation:
            DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
            break;
        case excel::XlCellType::xlCellTypeBlanks:
        case excel::XlCellType::xlCellTypeComments:
        case excel::XlCellType::xlCellTypeConstants:
        case excel::XlCellType::xlCellTypeFormulas:
        case excel::XlCellType::xlCellTypeVisible:
        {
            if ( bIsMultiArea )
            {
                // need to process each area, gather the results and
                // create a new range from those
                std::vector< table::CellRangeAddress > rangeResults;
                sal_Int32 nItems = ( m_Areas->getCount() + 1 );
                for ( sal_Int32 index=1; index <= nItems; ++index )
                {
                    uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny(index), uno::Any() ), uno::UNO_QUERY_THROW );
                    xRange = xRange->SpecialCells( _oType,  _oValue);
                    ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() );
                    if ( xRange.is() && pRange )
                    {
                        sal_Int32 nElems = ( pRange->m_Areas->getCount() + 1 );
                        for ( sal_Int32 nArea = 1; nArea < nElems; ++nArea )
                        {
                            uno::Reference< excel::XRange > xTmpRange( m_Areas->Item( uno::makeAny( nArea ), uno::Any() ), uno::UNO_QUERY_THROW );
                            RangeHelper rHelper( xTmpRange->getCellRange() );
                            rangeResults.push_back( rHelper.getCellRangeAddressable()->getRangeAddress() );
                        }
                    }
                }
                ScRangeList aCellRanges;
                std::vector< table::CellRangeAddress >::iterator it = rangeResults.begin();
                std::vector< table::CellRangeAddress >::iterator it_end = rangeResults.end();
                for ( ; it != it_end; ++ it )
                {
                    ScRange refRange;
                    ScUnoConversion::FillScRange( refRange, *it );
                    aCellRanges.Append( refRange );
                }
                // Single range
                if ( aCellRanges.First() == aCellRanges.Last() )
                {
                    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getScDocShell(), *aCellRanges.First() ) );
                // #FIXME need proper (WorkSheet) parent
                    return new ScVbaRange( getParent(), mxContext, xRange );
                }
                uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( getScDocShell(), aCellRanges ) );

                // #FIXME need proper (WorkSheet) parent
                return new ScVbaRange( getParent(), mxContext, xRanges );
            }
            else if ( bIsSingleCell )
            {
                uno::Reference< excel::XRange > xUsedRange = getWorksheet()->getUsedRange();
                pRangeToUse = static_cast< ScVbaRange* >( xUsedRange.get() );
            }

            break;
        }
        default:
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
            break;
    }
    if ( !pRangeToUse )
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    return pRangeToUse->SpecialCellsImpl( nType, _oValue );
}

sal_Int32 lcl_getFormulaResultFlags(const uno::Any& aType) throw ( script::BasicErrorException )
{
    sal_Int32 nType = excel::XlSpecialCellsValue::xlNumbers;
    aType >>= nType;
    sal_Int32 nRes = sheet::FormulaResult::VALUE;

    switch(nType)
    {
        case excel::XlSpecialCellsValue::xlErrors:
            nRes= sheet::FormulaResult::ERROR;
            break;
        case excel::XlSpecialCellsValue::xlLogical:
            //TODO bc93774: ask NN if this is really an appropriate substitute
            nRes = sheet::FormulaResult::VALUE;
            break;
        case excel::XlSpecialCellsValue::xlNumbers:
            nRes = sheet::FormulaResult::VALUE;
            break;
        case excel::XlSpecialCellsValue::xlTextValues:
            nRes = sheet::FormulaResult::STRING;
            break;
        default:
            DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }
    return nRes;
}

uno::Reference< excel::XRange >
ScVbaRange::SpecialCellsImpl( sal_Int32 nType, const uno::Any& _oValue) throw ( script::BasicErrorException )
{
    uno::Reference< excel::XRange > xRange;
    try
    {
        uno::Reference< sheet::XCellRangesQuery > xQuery( mxRange, uno::UNO_QUERY_THROW );
        uno::Reference< excel::XRange > oLocRangeImpl;
        uno::Reference< sheet::XSheetCellRanges > xLocSheetCellRanges;
        switch(nType)
        {
            case excel::XlCellType::xlCellTypeAllFormatConditions:
            case excel::XlCellType::xlCellTypeSameFormatConditions:
            case excel::XlCellType::xlCellTypeAllValidation:
            case excel::XlCellType::xlCellTypeSameValidation:
                // Shouldn't get here ( should be filtered out by
                // ScVbaRange::SpecialCells()
                DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
                break;
            case excel::XlCellType::xlCellTypeBlanks:
                xLocSheetCellRanges = xQuery->queryEmptyCells();
                break;
            case excel::XlCellType::xlCellTypeComments:
                xLocSheetCellRanges = xQuery->queryContentCells(sheet::CellFlags::ANNOTATION);
                break;
            case excel::XlCellType::xlCellTypeConstants:
                xLocSheetCellRanges = xQuery->queryContentCells(23);
                break;
            case excel::XlCellType::xlCellTypeFormulas:
            {
                sal_Int32 nFormulaResult = lcl_getFormulaResultFlags(_oValue);
                xLocSheetCellRanges = xQuery->queryFormulaCells(nFormulaResult);
                break;
            }
            case excel::XlCellType::xlCellTypeLastCell:
                xRange = Cells( uno::makeAny( getCount() ), uno::Any() );
            case excel::XlCellType::xlCellTypeVisible:
                xLocSheetCellRanges = xQuery->queryVisibleCells();
                break;
            default:
                DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
                break;
        }
        if (xLocSheetCellRanges.is())
        {
            xRange = lcl_makeXRangeFromSheetCellRanges( getParent(), mxContext, xLocSheetCellRanges, getScDocShell() );
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, STR_ERRORMESSAGE_NOCELLSWEREFOUND);
    }
    return xRange;
}

void SAL_CALL
ScVbaRange::RemoveSubtotal(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< sheet::XSubTotalCalculatable > xSub( mxRange, uno::UNO_QUERY_THROW );
    xSub->removeSubTotals();
}

void SAL_CALL
ScVbaRange::Subtotal( ::sal_Int32 _nGroupBy, ::sal_Int32 _nFunction, const uno::Sequence< ::sal_Int32 >& _nTotalList, const uno::Any& aReplace, const uno::Any& PageBreaks, const uno::Any& /*SummaryBelowData*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Bool bDoReplace = sal_False;
        aReplace >>= bDoReplace;
        sal_Bool bAddPageBreaks = sal_False;
        PageBreaks >>= bAddPageBreaks;

        uno::Reference< sheet::XSubTotalCalculatable> xSub;
        uno::Reference< sheet::XSubTotalDescriptor > xSubDesc = xSub->createSubTotalDescriptor(sal_True);
        uno::Reference< beans::XPropertySet > xSubDescPropertySet( xSubDesc, uno::UNO_QUERY_THROW );
        xSubDescPropertySet->setPropertyValue(INSERTPAGEBREAKS, uno::makeAny( bAddPageBreaks));
        sal_Int32 nLen = _nTotalList.getLength();
        uno::Sequence< sheet::SubTotalColumn > aColumns( nLen );
        for (int i = 0; i < nLen; i++)
        {
            aColumns[i].Column = _nTotalList[i] - 1;
            switch (_nFunction)
            {
                case excel::XlConsolidationFunction::xlAverage:
                    aColumns[i].Function = sheet::GeneralFunction_AVERAGE;
                    break;
                case excel::XlConsolidationFunction::xlCount:
                    aColumns[i].Function = sheet::GeneralFunction_COUNT;
                    break;
                case excel::XlConsolidationFunction::xlCountNums:
                    aColumns[i].Function = sheet::GeneralFunction_COUNTNUMS;
                    break;
                case excel::XlConsolidationFunction::xlMax:
                    aColumns[i].Function = sheet::GeneralFunction_MAX;
                    break;
                case excel::XlConsolidationFunction::xlMin:
                    aColumns[i].Function = sheet::GeneralFunction_MIN;
                    break;
                case excel::XlConsolidationFunction::xlProduct:
                    aColumns[i].Function = sheet::GeneralFunction_PRODUCT;
                    break;
                case excel::XlConsolidationFunction::xlStDev:
                    aColumns[i].Function = sheet::GeneralFunction_STDEV;
                    break;
                case excel::XlConsolidationFunction::xlStDevP:
                    aColumns[i].Function = sheet::GeneralFunction_STDEVP;
                    break;
                case excel::XlConsolidationFunction::xlSum:
                    aColumns[i].Function = sheet::GeneralFunction_SUM;
                    break;
                case excel::XlConsolidationFunction::xlUnknown:
                    aColumns[i].Function = sheet::GeneralFunction_NONE;
                    break;
                case excel::XlConsolidationFunction::xlVar:
                    aColumns[i].Function = sheet::GeneralFunction_VAR;
                    break;
                case excel::XlConsolidationFunction::xlVarP:
                    aColumns[i].Function = sheet::GeneralFunction_VARP;
                    break;
                default:
                    DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString()) ;
                    return;
            }
        }
        xSubDesc->addNew(aColumns, _nGroupBy - 1);
        xSub->applySubTotals(xSubDesc, bDoReplace);
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

uno::Reference< excel::XRange >
ScVbaRange::intersect( const css::uno::Reference< oo::excel::XRange >& xRange ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< excel::XRange > xResult;
    try
    {
        uno::Reference< sheet::XCellRangesQuery > xQuery( mxRange, uno::UNO_QUERY_THROW );
        RangeHelper aRange( xRange->getCellRange() );
        table::CellRangeAddress aAddress = aRange.getCellRangeAddressable()->getRangeAddress();
        uno::Reference< sheet::XSheetCellRanges > xIntersectRanges = xQuery->queryIntersection( aAddress );
        xResult = lcl_makeXRangeFromSheetCellRanges( getParent(), mxContext, xIntersectRanges, getScDocShell() );


    }
    catch( uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return xResult;
}

rtl::OUString&
ScVbaRange::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaRange") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaRange::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Range" ) );
    }
    return aServiceNames;
}

namespace range
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaRange, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SvVbaRange",
    "org.openoffice.excel.Range" );
}
