/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "vbarange.hxx"

#include <vbahelper/helperdecl.hxx>

#include <comphelper/unwrapargs.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/sheet/XGoalSeek.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/sheet/XCellSeries.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
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
#include <com/sun/star/sheet/FilterOperator2.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/TableFilterField2.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor2.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/util/TriState.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/awt/XDevice.hpp>

#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>

#include <ooo/vba/excel/XlPasteSpecialOperation.hpp>
#include <ooo/vba/excel/XlPasteType.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <ooo/vba/excel/XlFindLookIn.hpp>
#include <ooo/vba/excel/XlLookAt.hpp>
#include <ooo/vba/excel/XlSearchOrder.hpp>
#include <ooo/vba/excel/XlSortOrder.hpp>
#include <ooo/vba/excel/XlYesNoGuess.hpp>
#include <ooo/vba/excel/XlSortOrientation.hpp>
#include <ooo/vba/excel/XlSortMethod.hpp>
#include <ooo/vba/excel/XlDirection.hpp>
#include <ooo/vba/excel/XlSortDataOption.hpp>
#include <ooo/vba/excel/XlDeleteShiftDirection.hpp>
#include <ooo/vba/excel/XlInsertShiftDirection.hpp>
#include <ooo/vba/excel/XlReferenceStyle.hpp>
#include <ooo/vba/excel/XlBordersIndex.hpp>
#include <ooo/vba/excel/XlPageBreak.hpp>
#include <ooo/vba/excel/XlAutoFilterOperator.hpp>
#include <ooo/vba/excel/XlAutoFillType.hpp>
#include <ooo/vba/excel/XlTextParsingType.hpp>
#include <ooo/vba/excel/XlTextQualifier.hpp>
#include <ooo/vba/excel/XlCellType.hpp>
#include <ooo/vba/excel/XlSpecialCellsValue.hpp>
#include <ooo/vba/excel/XlConsolidationFunction.hpp>
#include <ooo/vba/excel/XlSearchDirection.hpp>

#include <scitems.hxx>
#include <svl/srchitem.hxx>
#include <cellsuno.hxx>
#include <dbdata.hxx>
#include "docfunc.hxx"
#include <docuno.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/itemwrapper.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <unonames.hxx>

#include "vbaapplication.hxx"
#include "vbafont.hxx"
#include "vbacomment.hxx"
#include "vbainterior.hxx"
#include "vbacharacters.hxx"
#include "vbaborders.hxx"
#include "vbaworksheet.hxx"
#include "vbavalidation.hxx"
#include "vbahyperlinks.hxx"

#include "tabvwsh.hxx"
#include "rangelst.hxx"
#include "convuno.hxx"
#include "compiler.hxx"
#include "attrib.hxx"
#include "undodat.hxx"
#include "dbdocfun.hxx"
#include "patattr.hxx"
#include "olinetab.hxx"
#include "transobj.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include <comphelper/anytostring.hxx>

#include <global.hxx>

#include "vbaglobals.hxx"
#include "vbastyle.hxx"
#include <vector>
#include <vbahelper/vbacollectionimpl.hxx>
// begin test includes
#include <com/sun/star/sheet/FunctionArgument.hpp>
// end test includes

#include <ooo/vba/excel/Range.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using ::std::vector;

// difference between VBA and file format width, in character units
const double fExtraWidth = 182.0 / 256.0;

//    * 1 point = 1/72 inch = 20 twips
//    * 1 inch = 72 points = 1440 twips
//    * 1 cm = 567 twips
double lcl_hmmToPoints( double nVal ) { return ( (double)((nVal /1000 ) * 567 ) / 20 ); }

static const sal_Int16 supportedIndexTable[] = {  excel::XlBordersIndex::xlEdgeLeft, excel::XlBordersIndex::xlEdgeTop, excel::XlBordersIndex::xlEdgeBottom, excel::XlBordersIndex::xlEdgeRight, excel::XlBordersIndex::xlDiagonalDown, excel::XlBordersIndex::xlDiagonalUp, excel::XlBordersIndex::xlInsideVertical, excel::XlBordersIndex::xlInsideHorizontal };

sal_uInt16 lcl_pointsToTwips( double nVal )
{
    nVal = nVal * static_cast<double>(20);
    short nTwips = static_cast<short>(nVal);
    return nTwips;
}
double lcl_TwipsToPoints( sal_uInt16 nVal )
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

uno::Any lcl_makeRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Any aAny, bool bIsRows, bool bIsColumns )
{
    uno::Reference< table::XCellRange > xCellRange( aAny, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( xParent, xContext, xCellRange, bIsRows, bIsColumns ) ) );
}

uno::Reference< excel::XRange > lcl_makeXRangeFromSheetCellRanges( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSheetCellRanges >& xLocSheetCellRanges, ScDocShell* pDoc )
{
    uno::Reference< excel::XRange > xRange;
    uno::Sequence< table::CellRangeAddress  > sAddresses = xLocSheetCellRanges->getRangeAddresses();
    ScRangeList aCellRanges;
    sal_Int32 nLen = sAddresses.getLength();
    if ( nLen )
           {
    for ( sal_Int32 index = 0; index < nLen; ++index )
    {
        ScRange refRange;
        ScUnoConversion::FillScRange( refRange, sAddresses[ index ] );
        aCellRanges.Append( refRange );
    }
    // Single range
    if ( aCellRanges.size() == 1 )
    {
        uno::Reference< table::XCellRange > xTmpRange( new ScCellRangeObj( pDoc, *aCellRanges.front() ) );
        xRange = new ScVbaRange( xParent, xContext, xTmpRange );
    }
    else
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDoc, aCellRanges ) );
        xRange = new ScVbaRange( xParent, xContext, xRanges );
    }
    }
    return xRange;
}

ScCellRangesBase* ScVbaRange::getCellRangesBase() throw ( uno::RuntimeException )
{
    if( mxRanges.is() )
        return ScCellRangesBase::getImplementation( mxRanges );
    if( mxRange.is() )
        return ScCellRangesBase::getImplementation( mxRange );
    throw uno::RuntimeException( rtl::OUString("General Error creating range - Unknown" ), uno::Reference< uno::XInterface >() );
}

ScCellRangeObj* ScVbaRange::getCellRangeObj() throw ( uno::RuntimeException )
{
    return dynamic_cast< ScCellRangeObj* >( getCellRangesBase() );
}

SfxItemSet*  ScVbaRange::getCurrentDataSet( ) throw ( uno::RuntimeException )
{
    SfxItemSet* pDataSet = excel::ScVbaCellRangeAccess::GetDataSet( getCellRangesBase() );
    if ( !pDataSet )
        throw uno::RuntimeException( rtl::OUString(  "Can't access Itemset for range"  ), uno::Reference< uno::XInterface >() );
    return pDataSet;
}

void ScVbaRange::fireChangeEvent()
{
    if( ScVbaApplication::getDocumentEventsEnabled() )
    {
        if( ScDocument* pDoc = getScDocument() )
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVBAEvents = pDoc->GetVbaEventProcessor();
            if( xVBAEvents.is() ) try
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[ 0 ] <<= uno::Reference< excel::XRange >( this );
                xVBAEvents->processVbaEvent( script::vba::VBAEventId::WORKSHEET_CHANGE, aArgs );
            }
            catch( uno::Exception& )
            {
            }
        }
    }
}

class SingleRangeEnumeration : public EnumerationHelper_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > mxContext;
    bool bHasMore;
public:

    SingleRangeEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException ) : m_xParent( xParent ), m_xRange( xRange ), mxContext( xContext ), bHasMore( true ) { }
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
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > mxContext;
    SingleRangeIndexAccess(); // not defined
public:
    SingleRangeIndexAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange ):mxParent( xParent ), m_xRange( xRange ), mxContext( xContext ) {}
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
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException) { return new SingleRangeEnumeration( mxParent, mxContext, m_xRange ); }

};



class RangesEnumerationImpl : public EnumerationHelperImpl
{
    bool mbIsRows;
    bool mbIsColumns;
public:

    RangesEnumerationImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, bool bIsRows, bool bIsColumns ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mbIsRows( bIsRows ), mbIsColumns( bIsColumns ) {}
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return lcl_makeRange( m_xParent, m_xContext, m_xEnumeration->nextElement(), mbIsRows, mbIsColumns );
    }
};


class ScVbaRangeAreas : public ScVbaCollectionBaseImpl
{
    bool mbIsRows;
    bool mbIsColumns;
public:
    ScVbaRangeAreas( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess, bool bIsRows, bool bIsColumns ) : ScVbaCollectionBaseImpl( xParent, xContext, xIndexAccess ), mbIsRows( bIsRows ), mbIsColumns( bIsColumns ) {}

    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration() throw (uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException){ return excel::XRange::static_type(0); }

    virtual uno::Any createCollectionObject( const uno::Any& aSource );

    virtual rtl::OUString getServiceImplName() { return rtl::OUString(); }

    virtual uno::Sequence< rtl::OUString > getServiceNames() { return uno::Sequence< rtl::OUString >(); }

};

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaRangeAreas::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new RangesEnumerationImpl( mxParent, mxContext, xEnumAccess->createEnumeration(), mbIsRows, mbIsColumns );
}

uno::Any
ScVbaRangeAreas::createCollectionObject( const uno::Any& aSource )
{
    return lcl_makeRange( mxParent, mxContext, aSource, mbIsRows, mbIsColumns );
}

// assume that xIf is infact a ScCellRangesBase
ScDocShell*
getDocShellFromIf( const uno::Reference< uno::XInterface >& xIf ) throw ( uno::RuntimeException )
{
	ScCellRangesBase* pUno = ScCellRangesBase::getImplementation( xIf );
	if ( !pUno )
		throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying uno range object" ) ), uno::Reference< uno::XInterface >()  );
	return pUno->GetDocShell();
}

ScDocShell*
getDocShellFromRange( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
	// need the ScCellRangesBase to get docshell
    uno::Reference< uno::XInterface > xIf( xRange );
	return getDocShellFromIf(xIf );
}

ScDocShell*
getDocShellFromRanges( const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges ) throw ( uno::RuntimeException )
{
	// need the ScCellRangesBase to get docshell
    uno::Reference< uno::XInterface > xIf( xRanges );
	return getDocShellFromIf(xIf );
}

uno::Reference< frame::XModel > getModelFromXIf( const uno::Reference< uno::XInterface >& xIf ) throw ( uno::RuntimeException )
{
	ScDocShell* pDocShell = getDocShellFromIf(xIf );
	return pDocShell->GetModel();
}

uno::Reference< frame::XModel > getModelFromRange( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
    // the XInterface for getImplementation can be any derived interface, no need for queryInterface
    uno::Reference< uno::XInterface > xIf( xRange );
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
ScVbaRange::getScDocument() throw (uno::RuntimeException)
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
ScVbaRange::getScDocShell() throw (uno::RuntimeException)
{
    if ( mxRanges.is() )
    {
        uno::Reference< container::XIndexAccess > xIndex( mxRanges, uno::UNO_QUERY_THROW );
        uno::Reference< table::XCellRange > xRange( xIndex->getByIndex( 0 ), uno::UNO_QUERY_THROW );
        return getDocShellFromRange( xRange );
    }
    return getDocShellFromRange( mxRange );
}

ScVbaRange* ScVbaRange::getImplementation( const uno::Reference< excel::XRange >& rxRange )
{
    // FIXME: always save to use dynamic_cast? Or better to (implement and) use XTunnel?
    return dynamic_cast< ScVbaRange* >( rxRange.get() );
}

uno::Reference< frame::XModel > ScVbaRange::getUnoModel() throw (uno::RuntimeException)
{
    if( ScDocShell* pDocShell = getScDocShell() )
        return pDocShell->GetModel();
    throw uno::RuntimeException();
}

uno::Reference< frame::XModel > ScVbaRange::getUnoModel( const uno::Reference< excel::XRange >& rxRange ) throw (uno::RuntimeException)
{
    if( ScVbaRange* pScVbaRange = getImplementation( rxRange ) )
        return pScVbaRange->getUnoModel();
    throw uno::RuntimeException();
}

const ScRangeList& ScVbaRange::getScRangeList() throw (uno::RuntimeException)
{
    if( ScCellRangesBase* pScRangesBase = getCellRangesBase() )
        return pScRangesBase->GetRangeList();
    throw uno::RuntimeException( ::rtl::OUString(  "Cannot obtain UNO range implementation object"  ), uno::Reference< uno::XInterface >() );
}

const ScRangeList& ScVbaRange::getScRangeList( const uno::Reference< excel::XRange >& rxRange ) throw (uno::RuntimeException)
{
    if( ScVbaRange* pScVbaRange = getImplementation( rxRange ) )
        return pScVbaRange->getScRangeList();
    throw uno::RuntimeException( ::rtl::OUString(  "Cannot obtain VBA range implementation object"  ), uno::Reference< uno::XInterface >() );
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
        uno::Any aValue = mxRangeProps->getPropertyValue(rtl::OUString( "NumberFormat"));
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

    bool isDateType()
    {
        sal_Int16 nType = getNumberFormat();
        if(( nType & util::NumberFormat::DATETIME ))
        {
            return true;
        }
        return false;
    }

    rtl::OUString getNumberFormatString()
    {
        uno::Reference< uno::XInterface > xIf( mxRangeProps, uno::UNO_QUERY_THROW );
        ScCellRangesBase* pUnoCellRange = ScCellRangesBase::getImplementation( xIf );
        if ( pUnoCellRange )
        {

            SfxItemSet* pDataSet =  excel::ScVbaCellRangeAccess::GetDataSet( pUnoCellRange );
            SfxItemState eState = pDataSet->GetItemState( ATTR_VALUE_FORMAT, true, NULL);
            // one of the cells in the range is not like the other ;-)
            // so return a zero length format to indicate that
            if ( eState == SFX_ITEM_DONTCARE )
                return rtl::OUString();
        }


        uno::Reference< beans::XPropertySet > xNumberProps( getNumberProps(), uno::UNO_QUERY_THROW );
        ::rtl::OUString aFormatString;
        uno::Any aString = xNumberProps->getPropertyValue(rtl::OUString( "FormatString"));
        aString >>= aFormatString;
        return aFormatString;
    }

    sal_Int16 getNumberFormat()
    {
        uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps();
        sal_Int16 nType = ::comphelper::getINT16(
            xNumberProps->getPropertyValue( ::rtl::OUString( "Type" ) ) );
        return nType;
    }

    bool setNumberFormat( const rtl::OUString& rFormat )
    {
        // #163288# treat "General" as "Standard" format
        sal_Int32 nNewIndex = 0;
        if( !rFormat.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "General" ) ) )
        {
            lang::Locale aLocale;
            uno::Reference< beans::XPropertySet > xNumProps = getNumberProps();
            xNumProps->getPropertyValue( ::rtl::OUString(  "Locale"  ) ) >>= aLocale;
            nNewIndex = mxFormats->queryKey( rFormat, aLocale, false );
            if ( nNewIndex == -1 ) // format not defined
                nNewIndex = mxFormats->addNew( rFormat, aLocale );
        }
        mxRangeProps->setPropertyValue( rtl::OUString( "NumberFormat" ), uno::makeAny( nNewIndex ) );
        return true;
    }

    bool setNumberFormat( sal_Int16 nType )
    {
        uno::Reference< beans::XPropertySet > xNumberProps = getNumberProps();
        lang::Locale aLocale;
        xNumberProps->getPropertyValue( ::rtl::OUString( "Locale" ) ) >>= aLocale;
        uno::Reference<util::XNumberFormatTypes> xTypes( mxFormats, uno::UNO_QUERY );
        if ( xTypes.is() )
        {
            sal_Int32 nNewIndex = xTypes->getStandardFormat( nType, aLocale );
               mxRangeProps->setPropertyValue( rtl::OUString( "NumberFormat" ), uno::makeAny( nNewIndex ) );
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

// #FIXME - QUICK
// we could probably could and should modify CellsEnumeration below
// to handle rows and columns ( but I do this seperately for now
// and.. this class only handles singe areas ( does it have to handle
// multi area ranges?? )
class ColumnsRowEnumeration: public CellsEnumeration_BASE
{
    uno::Reference< uno::XComponentContext > mxContext;
        uno::Reference< excel::XRange > mxRange;
    sal_Int32 mMaxElems;
    sal_Int32 mCurElem;

public:
    ColumnsRowEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< excel::XRange >& xRange, sal_Int32 nElems ) : mxContext( xContext ), mxRange( xRange ), mMaxElems( nElems ), mCurElem( 0 )
        {
    }

    virtual ::sal_Bool SAL_CALL hasMoreElements() throw (::uno::RuntimeException){ return mCurElem < mMaxElems; }

    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        sal_Int32 vbaIndex = 1 + mCurElem++;
        return uno::makeAny( mxRange->Item( uno::makeAny( vbaIndex ), uno::Any() ) );
    }
};

class CellsEnumeration : public CellsEnumeration_BASE
{
    uno::WeakReference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< XCollection > m_xAreas;
    vCellPos m_CellPositions;
    vCellPos::const_iterator m_it;

    uno::Reference< table::XCellRange > getArea( sal_Int32 nVBAIndex ) throw ( uno::RuntimeException )
    {
        if ( nVBAIndex < 1 || nVBAIndex > m_xAreas->getCount() )
            throw uno::RuntimeException();
        uno::Reference< excel::XRange > xRange( m_xAreas->Item( uno::makeAny(nVBAIndex), uno::Any() ), uno::UNO_QUERY_THROW );
        uno::Reference< table::XCellRange > xCellRange( ScVbaRange::getCellRange( xRange ), uno::UNO_QUERY_THROW );
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
    CellsEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< XCollection >& xAreas ): mxParent( xParent ), mxContext( xContext ), m_xAreas( xAreas )
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
        return uno::makeAny( uno::Reference< excel::XRange >( new ScVbaRange( mxParent, mxContext, xCellRange ) ) );

    }
};


const static ::rtl::OUString ISVISIBLE(   "IsVisible");
const static ::rtl::OUString POSITION(   "Position");
const static rtl::OUString EQUALS( "=" );
const static rtl::OUString NOTEQUALS( "<>" );
const static rtl::OUString GREATERTHAN( ">" );
const static rtl::OUString GREATERTHANEQUALS( ">=" );
const static rtl::OUString LESSTHAN( "<" );
const static rtl::OUString LESSTHANEQUALS( "<=" );
const static rtl::OUString CONTS_HEADER( "ContainsHeader" );
const static rtl::OUString INSERTPAGEBREAKS( "InsertPageBreaks" );
const static rtl::OUString STR_ERRORMESSAGE_APPLIESTOSINGLERANGEONLY( "The command you chose cannot be performed with multiple selections.\nSelect a single range and click the command again" );
const static rtl::OUString STR_ERRORMESSAGE_NOCELLSWEREFOUND( "No cells were found" );
const static rtl::OUString CELLSTYLE( "CellStyle" );

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
            sal_Bool bState = false;
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
                // The required behavior for a string value is:
                // 1. If the first character is a single quote, use the rest as a string cell, regardless of the cell's number format.
                // 2. Otherwise, if the cell's number format is "text", use the string value as a string cell.
                // 3. Otherwise, parse the string value in English locale, and apply a corresponding number format with the cell's locale
                //    if the cell's number format was "General".
                // Case 1 is handled here, the rest in ScCellObj::InputEnglishString

                if ( aString.toChar() == '\'' )     // case 1 - handle with XTextRange
                {
                    rtl::OUString aRemainder( aString.copy(1) );    // strip the quote
                    uno::Reference< text::XTextRange > xTextRange( xCell, uno::UNO_QUERY_THROW );
                    xTextRange->setString( aRemainder );
                }
                else
                {
                    // call implementation method InputEnglishString
                    ScCellObj* pCellObj = dynamic_cast< ScCellObj* >( xCell.get() );
                    if ( pCellObj )
                        pCellObj->InputEnglishString( aString );
                }
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
            if ( sFormula == "=TRUE()" )
                aValue <<= sal_True;
            else if ( sFormula == "=FALSE()" )
                aValue <<= false;
            else
            {
                uno::Reference< beans::XPropertySet > xProp( xCell, uno::UNO_QUERY_THROW );

                table::CellContentType eFormulaType = table::CellContentType_VALUE;
                // some formulas give textual results
                xProp->getPropertyValue( rtl::OUString( "FormulaResultType"  ) ) >>= eFormulaType;

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
            else if ( cellFormat.isDateType() )
                aValue = uno::makeAny( bridge::oleautomation::Date( xCell->getValue() ) );
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
    formula::FormulaGrammar::Grammar m_eGrammar;
public:
    CellFormulaValueSetter( const uno::Any& aValue, ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram ):CellValueSetter( aValue ),  m_pDoc( pDoc ), m_eGrammar( eGram ){}
protected:
    bool processValue( const uno::Any& aValue, const uno::Reference< table::XCell >& xCell )
    {
        rtl::OUString sFormula;
        double aDblValue = 0.0;
        if ( aValue >>= sFormula )
        {
            // convert to GRAM_PODF_A1 style grammar because XCell::setFormula
            // always compile it in that grammar. Perhaps
            // css.sheet.FormulaParser should be used in future to directly
            // pass formula tokens when that API stabilizes.
            if ( m_eGrammar != formula::FormulaGrammar::GRAM_PODF_A1 && ( sFormula.trim().indexOf('=') == 0 ) )
            {
                uno::Reference< uno::XInterface > xIf( xCell, uno::UNO_QUERY_THROW );
                ScCellRangesBase* pUnoRangesBase = dynamic_cast< ScCellRangesBase* >( xIf.get() );
                if ( pUnoRangesBase )
                {
                    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();
                    ScCompiler aCompiler( m_pDoc, aCellRanges.front()->aStart );
                    aCompiler.SetGrammar(m_eGrammar);
                    // compile the string in the format passed in
                    aCompiler.CompileString( sFormula );
                    // set desired convention to that of the document
                    aCompiler.SetGrammar( formula::FormulaGrammar::GRAM_PODF_A1 );
                    String sConverted;
                    aCompiler.CreateStringFromTokenArray(sConverted);
                    sFormula = EQUALS + sConverted;
                }
            }

            xCell->setFormula( sFormula );
            return true;
        }
        else if ( aValue >>= aDblValue )
        {
            xCell->setValue( aDblValue );
            return true;
        }
        return false;
    }

};

class CellFormulaValueGetter : public CellValueGetter
{
private:
    ScDocument*  m_pDoc;
    formula::FormulaGrammar::Grammar m_eGrammar;
public:
    CellFormulaValueGetter(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram ) : CellValueGetter( ), m_pDoc( pDoc ), m_eGrammar( eGram ) {}
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
            ScCompiler aCompiler( m_pDoc, aCellRanges.front()->aStart );
            aCompiler.SetGrammar(formula::FormulaGrammar::GRAM_DEFAULT);
            aCompiler.CompileString( sVal );
            // set desired convention
            aCompiler.SetGrammar( m_eGrammar );
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

const static rtl::OUString sNA("#N/A");

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

protected:
    ~RangeProcessor() {}
};

class RangeValueProcessor : public RangeProcessor
{
    const uno::Any& m_aVal;
public:
    RangeValueProcessor( const uno::Any& rVal ):m_aVal( rVal ) {}
    virtual ~RangeValueProcessor() {}
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
    virtual ~RangeFormulaProcessor() {}
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
    virtual ~RangeCountProcessor() {}
    virtual void process( const uno::Reference< excel::XRange >& xRange )
    {
        nCount = nCount + xRange->getCount();
    }
    sal_Int32 value() { return nCount; }
};
class AreasVisitor
{
private:
    uno::Reference< XCollection > m_Areas;
public:
    AreasVisitor( const uno::Reference< XCollection >& rAreas ):m_Areas( rAreas ){}

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

    static uno::Reference< excel::XRange > createRangeFromRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference<uno::XComponentContext >& xContext,
        const uno::Reference< table::XCellRange >& xRange, const uno::Reference< sheet::XCellRangeAddressable >& xCellRangeAddressable,
        sal_Int32 nStartColOffset = 0, sal_Int32 nStartRowOffset = 0, sal_Int32 nEndColOffset = 0, sal_Int32 nEndRowOffset = 0 )
    {
        return uno::Reference< excel::XRange >( new ScVbaRange( xParent, xContext,
            xRange->getCellRangeByPosition(
                xCellRangeAddressable->getRangeAddress().StartColumn + nStartColOffset,
                xCellRangeAddressable->getRangeAddress().StartRow + nStartRowOffset,
                xCellRangeAddressable->getRangeAddress().EndColumn + nEndColOffset,
                xCellRangeAddressable->getRangeAddress().EndRow + nEndRowOffset ) ) );
    }

};

bool
getCellRangesForAddress( sal_uInt16& rResFlags, const rtl::OUString& sAddress, ScDocShell* pDocSh, ScRangeList& rCellRanges, formula::FormulaGrammar::AddressConvention& eConv, char cDelimiter = 0 )
{

    ScDocument* pDoc = NULL;
    if ( pDocSh )
    {
        pDoc = pDocSh->GetDocument();
        String aString(sAddress);
        sal_uInt16 nMask = SCA_VALID;
        rResFlags = rCellRanges.Parse( sAddress, pDoc, nMask, eConv, cDelimiter );
        if ( rResFlags & SCA_VALID )
        {
            return true;
        }
    }
    return false;
}

bool getScRangeListForAddress( const rtl::OUString& sName, ScDocShell* pDocSh, ScRange& refRange, ScRangeList& aCellRanges, formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1 ) throw ( uno::RuntimeException )
{
    // see if there is a match with a named range
    uno::Reference< beans::XPropertySet > xProps( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xProps->getPropertyValue( rtl::OUString( "NamedRanges" ) ), uno::UNO_QUERY_THROW );
    // Strangly enough you can have Range( "namedRange1, namedRange2, etc," )
    // loop around each ',' seperated name
    std::vector< rtl::OUString > vNames;
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OUString aToken = sName.getToken( 0, ',', nIndex );
        vNames.push_back( aToken );
    } while ( nIndex >= 0 );

    if ( vNames.empty() )
        vNames.push_back( sName );

    std::vector< rtl::OUString >::iterator it = vNames.begin();
    std::vector< rtl::OUString >::iterator it_end = vNames.end();
    for ( ; it != it_end; ++it )
    {

        formula::FormulaGrammar::AddressConvention eConv = aConv;
        // spaces are illegal ( but the user of course can enter them )
        rtl::OUString sAddress = (*it).trim();
        // if a local name ( on the active sheet ) exists this will
        // take precedence over a global with the same name
        bool bLocalName = false;
        if ( !xNameAccess->hasByName( sAddress ) && pDocSh )
        {
            // try a local name
            ScDocument* pDoc = pDocSh->GetDocument();
            if ( pDoc )
            {
                SCTAB nCurTab = pDocSh->GetCurTab();
                ScRangeName* pRangeName = pDoc->GetRangeName(nCurTab);
                if (pRangeName)
                {
                    bLocalName = pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(sAddress)) != NULL;
                    // TODO: Handle local names correctly.
                    (void)bLocalName;
                }
            }
        }
        char aChar = 0;
        if ( xNameAccess->hasByName( sAddress ) )
        {
            uno::Reference< sheet::XNamedRange > xNamed( xNameAccess->getByName( sAddress ), uno::UNO_QUERY_THROW );
            sAddress = xNamed->getContent();
            // As the address comes from OOO, the addressing
            // style is may not be XL_A1
            eConv = pDocSh->GetDocument()->GetAddressConvention();
            aChar = ';';
        }

        sal_uInt16 nFlags = 0;
        if ( !getCellRangesForAddress( nFlags, sAddress, pDocSh, aCellRanges, eConv, aChar ) )
            return false;

        bool bTabFromReferrer = !( nFlags & SCA_TAB_3D );

        for ( size_t i = 0, nRanges = aCellRanges.size(); i < nRanges; ++i )
        {
            ScRange* pRange = aCellRanges[ i ];
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
getRangeForName( const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sName, ScDocShell* pDocSh, table::CellRangeAddress& pAddr, formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_XL_A1 ) throw ( uno::RuntimeException )
{
    ScRangeList aCellRanges;
    ScRange refRange;
    ScUnoConversion::FillScRange( refRange, pAddr );
    if ( !getScRangeListForAddress ( sName, pDocSh, refRange, aCellRanges, eConv ) )
        throw uno::RuntimeException();
    // Single range
    if ( aCellRanges.size() == 1 )
    {
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pDocSh, *aCellRanges.front() ) );
        uno::Reference< XHelperInterface > xFixThisParent = excel::getUnoSheetModuleObj( xRange );
        return new ScVbaRange( xFixThisParent, xContext, xRange );
    }
    uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDocSh, aCellRanges ) );

    uno::Reference< XHelperInterface > xFixThisParent = excel::getUnoSheetModuleObj( xRanges );
    return new ScVbaRange( xFixThisParent, xContext, xRanges );
}

// ----------------------------------------------------------------------------

namespace {

template< typename RangeType >
inline table::CellRangeAddress lclGetRangeAddress( const uno::Reference< RangeType >& rxCellRange ) throw (uno::RuntimeException)
{
    return uno::Reference< sheet::XCellRangeAddressable >( rxCellRange, uno::UNO_QUERY_THROW )->getRangeAddress();
}

void lclClearRange( const uno::Reference< table::XCellRange >& rxCellRange ) throw (uno::RuntimeException)
{
    using namespace ::com::sun::star::sheet::CellFlags;
    sal_Int32 nFlags = VALUE | DATETIME | STRING | ANNOTATION | FORMULA | HARDATTR | STYLES | EDITATTR | FORMATTED;
    uno::Reference< sheet::XSheetOperation > xSheetOperation( rxCellRange, uno::UNO_QUERY_THROW );
    xSheetOperation->clearContents( nFlags );
}

uno::Reference< sheet::XSheetCellRange > lclExpandToMerged( const uno::Reference< table::XCellRange >& rxCellRange, bool bRecursive ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetCellRange > xNewCellRange( rxCellRange, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xNewCellRange->getSpreadsheet(), uno::UNO_SET_THROW );
    table::CellRangeAddress aNewAddress = lclGetRangeAddress( xNewCellRange );
    table::CellRangeAddress aOldAddress;
    // expand as long as there are new merged ranges included
    do
    {
        aOldAddress = aNewAddress;
        uno::Reference< sheet::XSheetCellCursor > xCursor( xSheet->createCursorByRange( xNewCellRange ), uno::UNO_SET_THROW );
        xCursor->collapseToMergedArea();
        xNewCellRange.set( xCursor, uno::UNO_QUERY_THROW );
        aNewAddress = lclGetRangeAddress( xNewCellRange );
    }
    while( bRecursive && (aOldAddress != aNewAddress) );
    return xNewCellRange;
}

uno::Reference< sheet::XSheetCellRangeContainer > lclExpandToMerged( const uno::Reference< sheet::XSheetCellRangeContainer >& rxCellRanges, bool bRecursive ) throw (uno::RuntimeException)
{
    if( !rxCellRanges.is() )
        throw uno::RuntimeException( rtl::OUString(  "Missing cell ranges object"  ), uno::Reference< uno::XInterface >() );
    sal_Int32 nCount = rxCellRanges->getCount();
    if( nCount < 1 )
        throw uno::RuntimeException( rtl::OUString(  "Missing cell ranges object"  ), uno::Reference< uno::XInterface >() );

    ScRangeList aScRanges;
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        uno::Reference< table::XCellRange > xRange( rxCellRanges->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
        table::CellRangeAddress aRangeAddr = lclGetRangeAddress( lclExpandToMerged( xRange, bRecursive ) );
        ScRange aScRange;
        ScUnoConversion::FillScRange( aScRange, aRangeAddr );
        aScRanges.Append( aScRange );
    }
    return new ScCellRangesObj( getDocShellFromRanges( rxCellRanges ), aScRanges );
}

void lclExpandAndMerge( const uno::Reference< table::XCellRange >& rxCellRange, bool bMerge ) throw (uno::RuntimeException)
{
    uno::Reference< util::XMergeable > xMerge( lclExpandToMerged( rxCellRange, true ), uno::UNO_QUERY_THROW );
    // Calc cannot merge over merged ranges, always unmerge first
    xMerge->merge( false );
    if( bMerge )
    {
        // clear all contents of the covered cells (not the top-left cell)
        table::CellRangeAddress aRangeAddr = lclGetRangeAddress( rxCellRange );
        sal_Int32 nLastColIdx = aRangeAddr.EndColumn - aRangeAddr.StartColumn;
        sal_Int32 nLastRowIdx = aRangeAddr.EndRow - aRangeAddr.StartRow;
        // clear cells of top row, right of top-left cell
        if( nLastColIdx > 0 )
            lclClearRange( rxCellRange->getCellRangeByPosition( 1, 0, nLastColIdx, 0 ) );
        // clear all rows below top row
        if( nLastRowIdx > 0 )
            lclClearRange( rxCellRange->getCellRangeByPosition( 0, 1, nLastColIdx, nLastRowIdx ) );
        // merge the range
        xMerge->merge( sal_True );
    }
}

util::TriState lclGetMergedState( const uno::Reference< table::XCellRange >& rxCellRange ) throw (uno::RuntimeException)
{
    /*  1) Check if range is completely inside one single merged range. To do
        this, try to extend from top-left cell only (not from entire range).
        This will exclude cases where this range consists of several merged
        ranges (or parts of them). */
    table::CellRangeAddress aRangeAddr = lclGetRangeAddress( rxCellRange );
    uno::Reference< table::XCellRange > xTopLeft( rxCellRange->getCellRangeByPosition( 0, 0, 0, 0 ), uno::UNO_SET_THROW );
    uno::Reference< sheet::XSheetCellRange > xExpanded( lclExpandToMerged( xTopLeft, false ), uno::UNO_SET_THROW );
    table::CellRangeAddress aExpAddr = lclGetRangeAddress( xExpanded );
    // check that expanded range has more than one cell (really merged)
    if( ((aExpAddr.StartColumn < aExpAddr.EndColumn) || (aExpAddr.StartRow < aExpAddr.EndRow)) && ScUnoConversion::Contains( aExpAddr, aRangeAddr ) )
        return util::TriState_YES;

    /*  2) Check if this range contains any merged cells (completely or
        partly). This seems to be hardly possible via API, as
        XMergeable::getIsMerged() returns only true, if the top-left cell of a
        merged range is part of this range, so cases where just the lower part
        of a merged range is part of this range are not covered. */
    ScRange aScRange;
    ScUnoConversion::FillScRange( aScRange, aRangeAddr );
    bool bHasMerged = getDocumentFromRange( rxCellRange )->HasAttrib( aScRange, HASATTR_MERGED | HASATTR_OVERLAPPED );
    return bHasMerged ? util::TriState_INDETERMINATE : util::TriState_NO;
}

} // namespace

// ----------------------------------------------------------------------------

css::uno::Reference< excel::XRange >
ScVbaRange::getRangeObjectForName(
        const uno::Reference< uno::XComponentContext >& xContext, const rtl::OUString& sRangeName,
        ScDocShell* pDocSh, formula::FormulaGrammar::AddressConvention eConv ) throw ( uno::RuntimeException )
{
    table::CellRangeAddress refAddr;
    return getRangeForName( xContext, sRangeName, pDocSh, refAddr, eConv );
}

table::CellRangeAddress getCellRangeAddressForVBARange( const uno::Any& aParam, ScDocShell* pDocSh,  formula::FormulaGrammar::AddressConvention aConv = formula::FormulaGrammar::CONV_XL_A1) throw ( uno::RuntimeException )
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
                if ( aCellRanges.size() == 1 )
                {
                    table::CellRangeAddress aRangeAddress;
                    ScUnoConversion::FillApiRange( aRangeAddress, *aCellRanges.front() );
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
            throw uno::RuntimeException( rtl::OUString( "Can't extact CellRangeAddress from type"  ), uno::Reference< uno::XInterface >() );
    }
    return lclGetRangeAddress( xRangeParam );
}

uno::Reference< XCollection >
lcl_setupBorders( const uno::Reference< excel::XRange >& xParentRange, const uno::Reference<uno::XComponentContext>& xContext,  const uno::Reference< table::XCellRange >& xRange  ) throw( uno::RuntimeException )
{
    uno::Reference< XHelperInterface > xParent( xParentRange, uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocumentFromRange(xRange);
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString(  "Failed to access document from shell"  ), uno::Reference< uno::XInterface >() );
    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
     uno::Reference< XCollection > borders( new ScVbaBorders( xParent, xContext, xRange, aPalette ) );
    return borders;
}

ScVbaRange::ScVbaRange( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext )  throw ( lang::IllegalArgumentException ) : ScVbaRange_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext, getXSomethingFromArgs< beans::XPropertySet >( args, 1, false ), getModelFromXIf( getXSomethingFromArgs< uno::XInterface >( args, 1 ) ), true ), mbIsRows( false ), mbIsColumns( false )
{
    mxRange.set( mxPropertySet, uno::UNO_QUERY );
    mxRanges.set( mxPropertySet, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >  xIndex;
    if ( mxRange.is() )
    {
        xIndex = new SingleRangeIndexAccess( mxParent, mxContext, mxRange );
    }
    else if ( mxRanges.is() )
    {
        xIndex.set( mxRanges, uno::UNO_QUERY_THROW );
    }
    m_Areas = new ScVbaRangeAreas( mxParent, mxContext, xIndex, mbIsRows, mbIsColumns );
}

ScVbaRange::ScVbaRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< table::XCellRange >& xRange, sal_Bool bIsRows, sal_Bool bIsColumns ) throw( lang::IllegalArgumentException )
: ScVbaRange_BASE( xParent, xContext, uno::Reference< beans::XPropertySet >( xRange, uno::UNO_QUERY_THROW ), getModelFromRange( xRange), true ), mxRange( xRange ),
                mbIsRows( bIsRows ),
                mbIsColumns( bIsColumns )
{
    if  ( !xContext.is() )
        throw lang::IllegalArgumentException( rtl::OUString(  "context is not set "  ), uno::Reference< uno::XInterface >() , 1 );
    if  ( !xRange.is() )
        throw lang::IllegalArgumentException( rtl::OUString(  "range is not set "  ), uno::Reference< uno::XInterface >() , 1 );

    uno::Reference< container::XIndexAccess > xIndex( new SingleRangeIndexAccess( mxParent, mxContext, xRange ) );
    m_Areas = new ScVbaRangeAreas( mxParent, mxContext, xIndex, mbIsRows, mbIsColumns );

}

ScVbaRange::ScVbaRange( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges,  sal_Bool bIsRows, sal_Bool bIsColumns  ) throw ( lang::IllegalArgumentException )
: ScVbaRange_BASE( xParent, xContext, uno::Reference< beans::XPropertySet >( xRanges, uno::UNO_QUERY_THROW ), getModelFromXIf( uno::Reference< uno::XInterface >( xRanges, uno::UNO_QUERY_THROW ) ), true ), mxRanges( xRanges ),mbIsRows( bIsRows ), mbIsColumns( bIsColumns )

{
    uno::Reference< container::XIndexAccess >  xIndex( mxRanges, uno::UNO_QUERY_THROW );
    m_Areas  = new ScVbaRangeAreas( xParent, mxContext, xIndex, mbIsRows, mbIsColumns );

}

ScVbaRange::~ScVbaRange()
{
}

uno::Reference< XCollection >& ScVbaRange::getBorders()
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
    return uno::makeAny( script::ArrayWrapper( false, arrayGetter.getValue() ) );
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
ScVbaRange::setValue( const uno::Any& aValue, ValueSetter& valueSetter, bool bFireEvent ) throw (uno::RuntimeException)
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
        catch ( const uno::Exception& e )
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
    if( bFireEvent ) fireChangeEvent();
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
    setValue( aValue, valueSetter, true );
}

void SAL_CALL
ScVbaRange::Clear() throw (uno::RuntimeException)
{
    using namespace ::com::sun::star::sheet::CellFlags;
    sal_Int32 nFlags = VALUE | DATETIME | STRING | FORMULA | HARDATTR | EDITATTR | FORMATTED;
    ClearContents( nFlags, true );
}

//helper ClearContent
void
ScVbaRange::ClearContents( sal_Int32 nFlags, bool bFireEvent ) throw (uno::RuntimeException)
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
            ScVbaRange* pRange = getImplementation( xRange );
            if ( pRange )
                pRange->ClearContents( nFlags, false ); // do not fire for single ranges
        }
        // fire change event for the entire range list
        if( bFireEvent ) fireChangeEvent();
        return;
    }


    uno::Reference< sheet::XSheetOperation > xSheetOperation(mxRange, uno::UNO_QUERY_THROW);
    xSheetOperation->clearContents( nFlags );
    if( bFireEvent ) fireChangeEvent();
}

void SAL_CALL
ScVbaRange::ClearComments() throw (uno::RuntimeException)
{
    ClearContents( sheet::CellFlags::ANNOTATION, false );
}

void SAL_CALL
ScVbaRange::ClearContents() throw (uno::RuntimeException)
{
    sal_Int32 nClearFlags = ( sheet::CellFlags::VALUE |
        sheet::CellFlags::STRING |  sheet::CellFlags::DATETIME |
        sheet::CellFlags::FORMULA );
    ClearContents( nClearFlags, true );
}

void SAL_CALL
ScVbaRange::ClearFormats() throw (uno::RuntimeException)
{
    //FIXME: need to check if we need to combine sheet::CellFlags::FORMATTED
    sal_Int32 nClearFlags = sheet::CellFlags::HARDATTR | sheet::CellFlags::FORMATTED | sheet::CellFlags::EDITATTR;
    ClearContents( nClearFlags, false );
}

void
ScVbaRange::setFormulaValue( const uno::Any& rFormula, formula::FormulaGrammar::Grammar eGram, bool bFireEvent ) throw (uno::RuntimeException)
{
    // If this is a multiple selection apply setFormula over all areas
    if ( m_Areas->getCount() > 1 )
    {
        AreasVisitor aVisitor( m_Areas );
        RangeFormulaProcessor valueProcessor( rFormula );
        aVisitor.visit( valueProcessor );
        return;
    }
    CellFormulaValueSetter formulaValueSetter( rFormula, getScDocument(), eGram );
    setValue( rFormula, formulaValueSetter, bFireEvent );
}

uno::Any
ScVbaRange::getFormulaValue( formula::FormulaGrammar::Grammar eGram ) throw (uno::RuntimeException)
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
    CellFormulaValueGetter valueGetter( getScDocument(), eGram );
    return getValue( valueGetter );

}

void
ScVbaRange::setFormula(const uno::Any &rFormula ) throw (uno::RuntimeException)
{
    // #FIXME converting "=$a$1" e.g. CONV_XL_A1 -> CONV_OOO                            // results in "=$a$1:a1", temporalily disable conversion
    setFormulaValue( rFormula,formula::FormulaGrammar::GRAM_NATIVE_XL_A1, true );
}

uno::Any
ScVbaRange::getFormulaR1C1() throw (::com::sun::star::uno::RuntimeException)
{
    return getFormulaValue( formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1 );
}

void
ScVbaRange::setFormulaR1C1(const uno::Any& rFormula ) throw (uno::RuntimeException)
{
    setFormulaValue( rFormula,formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1, true );
}

uno::Any
ScVbaRange::getFormula() throw (::com::sun::star::uno::RuntimeException)
{
    return getFormulaValue( formula::FormulaGrammar::GRAM_NATIVE_XL_A1 );
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
        if ( pFormulaRanges->GetRangeList().empty() )
            return uno::makeAny(sal_False);

        // chech if there are holes (where some cells are not formulas)
        // or returned range is not equal to this range
        if (  ( pFormulaRanges->GetRangeList().size() > 1 )
           || ( pFormulaRanges->GetRangeList().front()->aStart != pThisRanges->GetRangeList().front()->aStart )
           || ( pFormulaRanges->GetRangeList().front()->aEnd   != pThisRanges->GetRangeList().front()->aEnd   )
           )
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
        uno::Reference< XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
        for ( sal_Int32 index = 1; index <= xCollection->getCount(); ++index )
        {
            uno::Reference< excel::XRange > xRange( xCollection->Item( uno::makeAny( index ), uno::Any() ), uno::UNO_QUERY_THROW );
            ScVbaRange* pThisRange = getImplementation( xRange );
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


    for ( size_t i = 0, nRanges = aCellRanges.size(); i < nRanges; ++i )
    {
        ScRange* pRange = aCellRanges[ i ];
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

    if ( aCellRanges.size() > 1 ) // Multi-Area
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );
        return new ScVbaRange( mxParent, mxContext, xRanges );
    }
    // normal range
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.front() ) );
    return new ScVbaRange( mxParent, mxContext, xRange  );
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
    return RangeHelper::createRangeFromRange( mxParent, mxContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );
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
    return RangeHelper::createRangeFromRange( mxParent, mxContext, helper.getCellRangeFromSheet(), xCellRangeAddressable );
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
        uno::Reference< XCollection > xCollection( m_Areas, uno::UNO_QUERY_THROW );
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
    ScAddress::Details dDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
    if ( ReferenceStyle.hasValue() )
    {
        sal_Int32 refStyle = excel::XlReferenceStyle::xlA1;
        ReferenceStyle >>= refStyle;
        if ( refStyle == excel::XlReferenceStyle::xlR1C1 )
            dDetails = ScAddress::Details( formula::FormulaGrammar::CONV_XL_R1C1, 0, 0 );
    }
    sal_uInt16 nFlags = SCA_VALID;
    ScDocShell* pDocShell =  getScDocShell();
    ScDocument* pDoc =  pDocShell->GetDocument();

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    ScRange aRange( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCROW >( thisAddress.StartRow ), static_cast< SCTAB >( thisAddress.Sheet ), static_cast< SCCOL >( thisAddress.EndColumn ), static_cast< SCROW >( thisAddress.EndRow ), static_cast< SCTAB >( thisAddress.Sheet ) );
    String sRange;
    sal_uInt16 ROW_ABSOLUTE = ( SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE );
    sal_uInt16 COL_ABSOLUTE = ( SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE );
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
    sal_Bool bLocal = false;
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
        dDetails = ScAddress::Details( formula::FormulaGrammar::CONV_XL_R1C1, static_cast< SCROW >( refAddress.StartRow ), static_cast< SCCOL >( refAddress.StartColumn ) );
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
        throw uno::RuntimeException( rtl::OUString(  "Failed to access document from shell"  ), uno::Reference< uno::XInterface >() );

    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
    ScCellRangeObj* pRangeObj = NULL;
    try
    {
        pRangeObj = getCellRangeObj();
    }
    catch( uno::Exception& )
    {
    }
    return  new ScVbaFont( this, mxContext, aPalette, xProps, pRangeObj );
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

    // Performance: Use a common helper method for ScVbaRange::Cells and ScVbaWorksheet::Cells,
    // instead of creating a new ScVbaRange object in often-called ScVbaWorksheet::Cells
    return CellsHelper( mxParent, mxContext, mxRange, nRowIndex, nColumnIndex );
}

// static
uno::Reference< excel::XRange >
ScVbaRange::CellsHelper( const uno::Reference< ov::XHelperInterface >& xParent,
                         const uno::Reference< uno::XComponentContext >& xContext,
                         const uno::Reference< css::table::XCellRange >& xRange,
                         const uno::Any &nRowIndex, const uno::Any &nColumnIndex ) throw(uno::RuntimeException)
{
    sal_Int32 nRow = 0, nColumn = 0;

    sal_Bool bIsIndex = nRowIndex.hasValue();
    sal_Bool bIsColumnIndex = nColumnIndex.hasValue();

    // Sometimes we might get a float or a double or whatever
    // set in the Any, we should convert as appropriate
    // #FIXME - perhaps worth turning this into some sort of
    // convertion routine e.g. bSuccess = getValueFromAny( nRow, nRowIndex, getCppuType((sal_Int32*)0) )
    uno::Any aRowIndexAny = nRowIndex;
    if ( aRowIndexAny.hasValue() && !( aRowIndexAny >>= nRow ) )
    {
        uno::Reference< script::XTypeConverter > xConverter = getTypeConverter( xContext );
        uno::Any aConverted;
        try
        {
            aConverted = xConverter->convertTo( aRowIndexAny, getCppuType((sal_Int32*)0) );
            bIsIndex = ( aConverted >>= nRow );
        }
        catch( uno::Exception& ) {} // silence any errors
    }

    uno::Any aColumnAny = nColumnIndex;

    if ( bIsColumnIndex )
    {
         // Column index can be a col address e.g Cells( 1, "B" ) etc.
        rtl::OUString sCol;
        if ( nColumnIndex >>= sCol )
        {
            ScAddress::Details dDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
            ScRange tmpRange;
            sal_uInt16 flags = tmpRange.ParseCols( sCol, getDocumentFromRange( xRange ), dDetails );
            if ( ( flags & 0x200 ) != 0x200 )
               throw uno::RuntimeException();
            nColumn = tmpRange.aStart.Col() + 1;
        }
        else
        {
            if ( !( aColumnAny >>= nColumn ) )
            {
                uno::Reference< script::XTypeConverter > xConverter = getTypeConverter( xContext );
                uno::Any aConverted;
                try
                {
                    aConverted = xConverter->convertTo( aColumnAny, getCppuType((sal_Int32*)0) );
                    bIsColumnIndex = ( aConverted >>= nColumn );
                }
                catch( uno::Exception& ) {} // silence any errors
            }
       }
    }
    RangeHelper thisRange( xRange );
    table::CellRangeAddress thisRangeAddress =  thisRange.getCellRangeAddressable()->getRangeAddress();
    uno::Reference< table::XCellRange > xSheetRange = thisRange.getCellRangeFromSheet();
    if( !bIsIndex && !bIsColumnIndex ) // .Cells
        // #FIXE needs proper parent ( Worksheet )
        return uno::Reference< excel::XRange >( new ScVbaRange( xParent, xContext, xRange ) );

    sal_Int32 nIndex = --nRow;
    if( bIsIndex && !bIsColumnIndex ) // .Cells(n)
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xRange, ::uno::UNO_QUERY_THROW);
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
    return new ScVbaRange( xParent, xContext, xSheetRange->getCellRangeByPosition( nColumn, nRow, nColumn, nRow ) );
}

void
ScVbaRange::Select() throw (uno::RuntimeException)
{
    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    if ( !pUnoRangesBase )
        throw uno::RuntimeException( rtl::OUString(  "Failed to access underlying uno range object"  ), uno::Reference< uno::XInterface >()  );
    ScDocShell* pShell = pUnoRangesBase->GetDocShell();
    if ( pShell )
    {
        uno::Reference< frame::XModel > xModel( pShell->GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        if ( mxRanges.is() )
            xSelection->select( uno::Any( lclExpandToMerged( mxRanges, true ) ) );
        else
            xSelection->select( uno::Any( lclExpandToMerged( mxRange, true ) ) );
        // set focus on document e.g.
        // ThisComponent.CurrentController.Frame.getContainerWindow.SetFocus
        try
        {
            uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
            uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_QUERY_THROW );
            uno::Reference< awt::XWindow > xWin( xFrame->getContainerWindow(), uno::UNO_QUERY_THROW );
            xWin->setFocus();
        }
        catch( uno::Exception& )
        {
        }
    }
}

bool cellInRange( const table::CellRangeAddress& rAddr, const sal_Int32& nCol, const sal_Int32& nRow )
{
    if ( nCol >= rAddr.StartColumn && nCol <= rAddr.EndColumn &&
        nRow >= rAddr.StartRow && nRow <= rAddr.EndRow )
        return true;
    return false;
}

void setCursor(  const SCCOL& nCol, const SCROW& nRow, const uno::Reference< frame::XModel >& xModel,  bool bInSel = true )
{
    ScTabViewShell* pShell = excel::getBestViewShell( xModel );
    if ( pShell )
    {
        if ( bInSel )
            pShell->SetCursor( nCol, nRow );
        else
            pShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_NONE, false, false, sal_True, false );
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
        uno::Reference< frame::XModel > xModel;
        ScDocShell* pShell = getScDocShell();

        if ( pShell )
            xModel = pShell->GetModel();

        if ( !xModel.is() )
            throw uno::RuntimeException();

    // get current selection
    uno::Reference< sheet::XCellRangeAddressable > xRange( xModel->getCurrentSelection(), ::uno::UNO_QUERY);

    uno::Reference< sheet::XSheetCellRanges > xRanges( xModel->getCurrentSelection(), ::uno::UNO_QUERY);

    if ( xRanges.is() )
    {
        uno::Sequence< table::CellRangeAddress > nAddrs = xRanges->getRangeAddresses();
        for ( sal_Int32 index = 0; index < nAddrs.getLength(); ++index )
        {
            if ( cellInRange( nAddrs[index], thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
            {
                setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), xModel );
                return;
            }

        }
    }

    if ( xRange.is() && cellInRange( xRange->getRangeAddress(), thisRangeAddress.StartColumn, thisRangeAddress.StartRow ) )
        setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), xModel );
    else
    {
        // if this range is multi cell select the range other
        // wise just position the cell at this single range position
        if ( isSingleCellRange() )
            // This top-leftmost cell of this Range is not in the current
            // selection so just select this range
            setCursor( static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), xModel, false  );
        else
            Select();
    }

}

uno::Reference< excel::XRange >
ScVbaRange::Rows(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    rtl::OUString sAddress;

    if ( aIndex.hasValue() )
    {
        sal_Int32 nValue = 0;
        ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
        ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

        ScRange aRange = *aCellRanges.front();
        if( aIndex >>= nValue )
        {
            aRange.aStart.SetRow( aRange.aStart.Row() + --nValue );
            aRange.aEnd.SetRow( aRange.aStart.Row() );
        }
        else if ( aIndex >>= sAddress )
        {
            ScAddress::Details dDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
            ScRange tmpRange;
            tmpRange.ParseRows( sAddress, getDocumentFromRange( mxRange ), dDetails );
            SCROW nStartRow = tmpRange.aStart.Row();
            SCROW nEndRow = tmpRange.aEnd.Row();

            aRange.aStart.SetRow( aRange.aStart.Row() + nStartRow );
            aRange.aEnd.SetRow( aRange.aStart.Row() + ( nEndRow  - nStartRow ));
        }
        else
            throw uno::RuntimeException( rtl::OUString( "Illegal param"  ), uno::Reference< uno::XInterface >() );

        if ( aRange.aStart.Row() < 0 || aRange.aEnd.Row() < 0 )
            throw uno::RuntimeException( rtl::OUString("Internal failure, illegal param"), uno::Reference< uno::XInterface >() );
        // return a normal range ( even for multi-selection
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), aRange ) );
        return new ScVbaRange( mxParent, mxContext, xRange, true  );
    }
    // Rows() - no params
    if ( m_Areas->getCount() > 1 )
        return new ScVbaRange(  mxParent, mxContext, mxRanges, true );
    return new ScVbaRange(  mxParent, mxContext, mxRange, true );
}

uno::Reference< excel::XRange >
ScVbaRange::Columns(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    rtl::OUString sAddress;

    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

    ScRange aRange = *aCellRanges.front();
    if ( aIndex.hasValue() )
    {
        sal_Int32 nValue = 0;
        if ( aIndex >>= nValue )
        {
            aRange.aStart.SetCol( aRange.aStart.Col() + static_cast< SCCOL > ( --nValue ) );
            aRange.aEnd.SetCol( aRange.aStart.Col() );
        }

        else if ( aIndex >>= sAddress )
        {
            ScAddress::Details dDetails( formula::FormulaGrammar::CONV_XL_A1, 0, 0 );
            ScRange tmpRange;
            tmpRange.ParseCols( sAddress, getDocumentFromRange( mxRange ), dDetails );
            SCCOL nStartCol = tmpRange.aStart.Col();
            SCCOL nEndCol = tmpRange.aEnd.Col();

            aRange.aStart.SetCol( aRange.aStart.Col() + nStartCol );
            aRange.aEnd.SetCol( aRange.aStart.Col() + ( nEndCol  - nStartCol ));
        }
        else
            throw uno::RuntimeException( rtl::OUString( "Illegal param"  ), uno::Reference< uno::XInterface >() );

        if ( aRange.aStart.Col() < 0 || aRange.aEnd.Col() < 0 )
            throw uno::RuntimeException( rtl::OUString("Internal failure, illegal param"), uno::Reference< uno::XInterface >() );
    }
    // Columns() - no params
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), aRange ) );
    return new ScVbaRange( mxParent, mxContext, xRange, false, true  );
}

void
ScVbaRange::setMergeCells( const uno::Any& aIsMerged ) throw (script::BasicErrorException, uno::RuntimeException)
{
    bool bMerge = extractBoolFromAny( aIsMerged );

    if( mxRanges.is() )
    {
        sal_Int32 nCount = mxRanges->getCount();

        // VBA does nothing (no error) if the own ranges overlap somehow
        ::std::vector< table::CellRangeAddress > aList;
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
        {
            uno::Reference< sheet::XCellRangeAddressable > xRangeAddr( mxRanges->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
            table::CellRangeAddress aAddress = xRangeAddr->getRangeAddress();
            for( ::std::vector< table::CellRangeAddress >::const_iterator aIt = aList.begin(), aEnd = aList.end(); aIt != aEnd; ++aIt )
                if( ScUnoConversion::Intersects( *aIt, aAddress ) )
                    return;
            aList.push_back( aAddress );
        }

        // (un)merge every range after it has been extended to intersecting merged ranges from sheet
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
        {
            uno::Reference< table::XCellRange > xRange( mxRanges->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
            lclExpandAndMerge( xRange, bMerge );
        }
        return;
    }

    // otherwise, merge single range
    lclExpandAndMerge( mxRange, bMerge );
}

uno::Any
ScVbaRange::getMergeCells() throw (script::BasicErrorException, uno::RuntimeException)
{
    if( mxRanges.is() )
    {
        sal_Int32 nCount = mxRanges->getCount();
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
        {
            uno::Reference< table::XCellRange > xRange( mxRanges->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
            util::TriState eMerged = lclGetMergedState( xRange );
            /*  Excel always returns NULL, if one range of the range list is
                partly or completely merged. Even if all ranges are completely
                merged, the return value is still NULL. */
            if( eMerged != util::TriState_NO )
                return aNULL();
        }
        // no range is merged anyhow, return false
        return uno::Any( false );
    }

    // otherwise, check single range
    switch( lclGetMergedState( mxRange ) )
    {
        case util::TriState_YES:    return uno::Any( true );
        case util::TriState_NO:     return uno::Any( false );
        default:                    return aNULL();
    }
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
        uno::Reference< frame::XModel > xModel = getModelFromRange( mxRange );
        Select();
        excel::implnCopy( xModel );
    }
}

void
ScVbaRange::Cut(const ::uno::Any& Destination) throw (uno::RuntimeException)
{
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( "That command cannot be used on multiple selections"  ), uno::Reference< uno::XInterface >() );
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
        uno::Reference< frame::XModel > xModel = getModelFromRange( mxRange );
        Select();
        excel::implnCut( xModel );
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
    if ( !sFormat.isEmpty() )
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
    return new ScVbaRange( mxParent, mxContext,xRange->getCellRangeByPosition(
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
    bool bIsWrapped = extractBoolFromAny( aIsWrapped );
    xProps->setPropertyValue( rtl::OUString(  "IsTextWrapped"  ), uno::Any( bIsWrapped ) );
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

    SfxItemState eState = pDataSet->GetItemState( ATTR_LINEBREAK, true, NULL);
    if ( eState == SFX_ITEM_DONTCARE )
        return aNULL();

    uno::Reference< beans::XPropertySet > xProps(mxRange, ::uno::UNO_QUERY_THROW );
    uno::Any aValue = xProps->getPropertyValue( rtl::OUString(  "IsTextWrapped"  ) );
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
            rtl::OUString(  " Invalid Argument "  ),
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

    return new ScVbaRange( mxParent, mxContext, xCellRange );

}

// Allow access to underlying openoffice uno api ( useful for debugging
// with openoffice basic )
uno::Any SAL_CALL ScVbaRange::getCellRange(  ) throw (uno::RuntimeException)
{
    uno::Any aAny;
    if ( mxRanges.is() )
        aAny <<= mxRanges;
    else if ( mxRange.is() )
        aAny <<= mxRange;
    return aAny;
}

uno::Any ScVbaRange::getCellRange( const uno::Reference< excel::XRange >& rxRange ) throw (uno::RuntimeException)
{
    if( ScVbaRange* pVbaRange = getImplementation( rxRange ) )
        return pVbaRange->getCellRange();
    throw uno::RuntimeException();
}

static sal_uInt16
getPasteFlags (sal_Int32 Paste)
{
    sal_uInt16 nFlags = IDF_NONE;
    switch (Paste) {
        case excel::XlPasteType::xlPasteComments:
        nFlags = IDF_NOTE;break;
        case excel::XlPasteType::xlPasteFormats:
        nFlags = IDF_ATTRIB;break;
        case excel::XlPasteType::xlPasteFormulas:
        nFlags = IDF_FORMULA;break;
        case excel::XlPasteType::xlPasteFormulasAndNumberFormats :
        case excel::XlPasteType::xlPasteValues:
        nFlags = ( IDF_VALUE | IDF_DATETIME | IDF_STRING | IDF_SPECIAL_BOOLEAN ); break;
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

static sal_uInt16
getPasteFormulaBits( sal_Int32 Operation)
{
    sal_uInt16 nFormulaBits = PASTE_NOFUNC ;
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
        throw uno::RuntimeException( rtl::OUString( "That command cannot be used on multiple selections"  ), uno::Reference< uno::XInterface >() );
        ScDocShell* pShell = getScDocShell();

        uno::Reference< frame::XModel > xModel( ( pShell ? pShell->GetModel() : NULL ), uno::UNO_QUERY_THROW );
    uno::Reference< view::XSelectionSupplier > xSelection( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    // select this range
    xSelection->select( uno::makeAny( mxRange ) );
    // set up defaults
    sal_Int32 nPaste = excel::XlPasteType::xlPasteAll;
    sal_Int32 nOperation = excel::XlPasteSpecialOperation::xlPasteSpecialOperationNone;
    sal_Bool bTranspose = false;
    sal_Bool bSkipBlanks = false;

    if ( Paste.hasValue() )
        Paste >>= nPaste;
    if ( Operation.hasValue() )
        Operation >>= nOperation;
    if ( SkipBlanks.hasValue() )
        SkipBlanks >>= bSkipBlanks;
    if ( Transpose.hasValue() )
        Transpose >>= bTranspose;

    sal_uInt16 nFlags = getPasteFlags(nPaste);
    sal_uInt16 nFormulaBits = getPasteFormulaBits(nOperation);
    excel::implnPasteSpecial(pShell->GetModel(), nFlags,nFormulaBits,bSkipBlanks,bTranspose);
}

uno::Reference< excel::XRange >
ScVbaRange::getEntireColumnOrRow( bool bColumn ) throw (uno::RuntimeException)
{
    ScCellRangesBase* pUnoRangesBase = getCellRangesBase();
    // copy the range list
    ScRangeList aCellRanges = pUnoRangesBase->GetRangeList();

    for ( size_t i = 0, nRanges = aCellRanges.size(); i < nRanges; ++i )
    {
        ScRange* pRange = aCellRanges[ i ];
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
    if ( aCellRanges.size() > 1 ) // Multi-Area
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pUnoRangesBase->GetDocShell(), aCellRanges ) );

        return new ScVbaRange( mxParent, mxContext, xRanges, !bColumn, bColumn );
    }
    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( pUnoRangesBase->GetDocShell(), *aCellRanges.front() ) );
    return new ScVbaRange( mxParent, mxContext, xRange, !bColumn, bColumn  );
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
    // if there is already a comment in the top-left cell then throw
    if( getComment().is() )
        throw uno::RuntimeException();

    // workaround: Excel allows to create empty comment, Calc does not
    ::rtl::OUString aNoteText;
    if( Text.hasValue() && !(Text >>= aNoteText) )
        throw uno::RuntimeException();
    if( aNoteText.isEmpty() )
        aNoteText = ::rtl::OUString( sal_Unicode( ' ' ) );

    // try to create a new annotation
    table::CellRangeAddress aRangePos = lclGetRangeAddress( mxRange );
    table::CellAddress aNotePos( aRangePos.Sheet, aRangePos.StartColumn, aRangePos.StartRow );
    uno::Reference< sheet::XSheetCellRange > xCellRange( mxRange, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnosSupp( xCellRange->getSpreadsheet(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetAnnotations > xAnnos( xAnnosSupp->getAnnotations(), uno::UNO_SET_THROW );
    xAnnos->insertNew( aNotePos, aNoteText );
    return new ScVbaComment( this, mxContext, getUnoModel(), mxRange );
}

uno::Reference< excel::XComment > SAL_CALL
ScVbaRange::getComment() throw (uno::RuntimeException)
{
    // intentional behavior to return a null object if no
    // comment defined
    uno::Reference< excel::XComment > xComment( new ScVbaComment( this, mxContext, getUnoModel(), mxRange ) );
    if ( xComment->Text( uno::Any(), uno::Any(), uno::Any() ).isEmpty() )
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
            throw uno::RuntimeException( rtl::OUString(  "Failed to get IsVisible property"), uno::Reference< uno::XInterface >() );
    }
    catch( const uno::Exception& e )
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

    bool bHidden = extractBoolFromAny( _hidden );
    try
    {
        uno::Reference< beans::XPropertySet > xProps = getRowOrColumnProps( mxRange, mbIsRows );
        xProps->setPropertyValue( ISVISIBLE, uno::Any( !bHidden ) );
    }
    catch( const uno::Exception& e )
    {
        throw uno::RuntimeException( e.Message, uno::Reference< uno::XInterface >() );
    }
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
    if ( What.isEmpty()  )
        throw uno::RuntimeException( rtl::OUString( "Range::Replace, missing params" ) , uno::Reference< uno::XInterface >() );
    rtl::OUString sWhat = VBAToRegexp( What);
    // #TODO #FIXME SearchFormat & ReplacesFormat are not processed
    // What do we do about MatchByte.. we don't seem to support that
    const SvxSearchItem& globalSearchOptions = ScGlobal::GetSearchItem();
    SvxSearchItem newOptions( globalSearchOptions );

    sal_Int16 nLook =  globalSearchOptions.GetWordOnly() ?  excel::XlLookAt::xlPart : excel::XlLookAt::xlWhole;
    sal_Int16 nSearchOrder = globalSearchOptions.GetRowDirection() ? excel::XlSearchOrder::xlByRows : excel::XlSearchOrder::xlByColumns;

    sal_Bool bMatchCase = false;
    uno::Reference< util::XReplaceable > xReplace( mxRange, uno::UNO_QUERY );
    if ( xReplace.is() )
    {
        uno::Reference< util::XReplaceDescriptor > xDescriptor =
            xReplace->createReplaceDescriptor();

        xDescriptor->setSearchString( sWhat);
        xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHREGEXP  ), uno::makeAny( sal_True ) );
        xDescriptor->setReplaceString( Replacement);
        if ( LookAt.hasValue() )
        {
            // sets SearchWords ( true is Cell match )
            nLook =  ::comphelper::getINT16( LookAt );
            sal_Bool bSearchWords = false;
            if ( nLook == excel::XlLookAt::xlPart )
                bSearchWords = false;
            else if ( nLook == excel::XlLookAt::xlWhole )
                bSearchWords = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for LookAt" ) , uno::Reference< uno::XInterface >() );
            // set global search props ( affects the find dialog
            // and of course the defaults for this method
            newOptions.SetWordOnly( bSearchWords );
            xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHWORDS  ), uno::makeAny( bSearchWords ) );
        }
        // sets SearchByRow ( true for Rows )
        if ( SearchOrder.hasValue() )
        {
            nSearchOrder =  ::comphelper::getINT16( SearchOrder );
            sal_Bool bSearchByRow = false;
            if ( nSearchOrder == excel::XlSearchOrder::xlByColumns )
                bSearchByRow = false;
            else if ( nSearchOrder == excel::XlSearchOrder::xlByRows )
                bSearchByRow = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for SearchOrder" ) , uno::Reference< uno::XInterface >() );

            newOptions.SetRowDirection( bSearchByRow );
            xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHBYROW  ), uno::makeAny( bSearchByRow ) );
        }
        if ( MatchCase.hasValue() )
        {
            // SearchCaseSensitive
            MatchCase >>= bMatchCase;
            xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHCASE  ), uno::makeAny( bMatchCase ) );
        }

        ScGlobal::SetSearchItem( newOptions );
        // ignore MatchByte for the moment, its not supported in
        // OOo.org afaik

        uno::Reference< util::XSearchDescriptor > xSearch( xDescriptor, uno::UNO_QUERY );
        xReplace->replaceAll( xSearch );
    }
    return sal_True; // always
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::Find( const uno::Any& What, const uno::Any& After, const uno::Any& LookIn, const uno::Any& LookAt, const uno::Any& SearchOrder, const uno::Any& SearchDirection, const uno::Any& MatchCase, const uno::Any& /*MatchByte*/, const uno::Any& /*SearchFormat*/ ) throw (uno::RuntimeException)
{
    // return a Range object that represents the first cell where that information is found.
    rtl::OUString sWhat;
    sal_Int32 nWhat = 0;
    double fWhat = 0.0;

    // string.
    if( What >>= sWhat )
    {
        if( sWhat.isEmpty() )
            throw uno::RuntimeException( rtl::OUString( "Range::Find, missing params" ) , uno::Reference< uno::XInterface >() );
    }
    else if( What >>= nWhat )
    {
        sWhat = rtl::OUString::valueOf( nWhat );
    }
    else if( What >>= fWhat )
    {
        sWhat = rtl::OUString::valueOf( fWhat );
    }
    else
        throw uno::RuntimeException( rtl::OUString( "Range::Find, missing params" ) , uno::Reference< uno::XInterface >() );

    rtl::OUString sSearch = VBAToRegexp( sWhat );

    const SvxSearchItem& globalSearchOptions = ScGlobal::GetSearchItem();
    SvxSearchItem newOptions( globalSearchOptions );

    sal_Int16 nLookAt =  globalSearchOptions.GetWordOnly() ?  excel::XlLookAt::xlPart : excel::XlLookAt::xlWhole;
    sal_Int16 nSearchOrder = globalSearchOptions.GetRowDirection() ? excel::XlSearchOrder::xlByRows : excel::XlSearchOrder::xlByColumns;

    uno::Reference< util::XSearchable > xSearch( mxRange, uno::UNO_QUERY );
    if( xSearch.is() )
    {
        uno::Reference< util::XSearchDescriptor > xDescriptor = xSearch->createSearchDescriptor();
        xDescriptor->setSearchString( sSearch );
        xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHREGEXP  ), uno::Any( true ) );

        uno::Reference< excel::XRange > xAfterRange;
        uno::Reference< table::XCellRange > xStartCell;
        if( After >>= xAfterRange )
        {
            // After must be a single cell in the range
            if( xAfterRange->getCount() > 1 )
                throw uno::RuntimeException( rtl::OUString( "After must be a single cell." ) , uno::Reference< uno::XInterface >() );
            uno::Reference< excel::XRange > xCell( Cells( uno::makeAny( xAfterRange->getRow() ), uno::makeAny( xAfterRange->getColumn() ) ), uno::UNO_QUERY );
            if( !xCell.is() )
                throw uno::RuntimeException( rtl::OUString( "After must be in range." ) , uno::Reference< uno::XInterface >() );
            xStartCell.set( xAfterRange->getCellRange(), uno::UNO_QUERY_THROW );
        }

        // LookIn
        if( LookIn.hasValue() )
        {
            sal_Int32 nLookIn = 0;
            if( LookIn >>= nLookIn )
            {
                sal_Int16 nSearchType = 0;
                switch( nLookIn )
                {
                    case excel::XlFindLookIn::xlComments :
                        nSearchType = SVX_SEARCHIN_NOTE; // Notes
                    break;
                    case excel::XlFindLookIn::xlFormulas :
                        nSearchType = SVX_SEARCHIN_FORMULA;
                    break;
                    case excel::XlFindLookIn::xlValues :
                        nSearchType = SVX_SEARCHIN_VALUE;
                    break;
                    default:
                        throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for LookIn." ) , uno::Reference< uno::XInterface >() );
                }
                newOptions.SetCellType( nSearchType );
                xDescriptor->setPropertyValue( rtl::OUString( "SearchType" ), uno::makeAny( nSearchType ) );
            }
        }

        // LookAt
        if ( LookAt.hasValue() )
        {
            nLookAt =  ::comphelper::getINT16( LookAt );
            sal_Bool bSearchWords = false;
            if ( nLookAt == excel::XlLookAt::xlPart )
                bSearchWords = false;
            else if ( nLookAt == excel::XlLookAt::xlWhole )
                bSearchWords = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for LookAt" ) , uno::Reference< uno::XInterface >() );
            newOptions.SetWordOnly( bSearchWords );
            xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHWORDS  ), uno::makeAny( bSearchWords ) );
        }

        // SearchOrder
        if ( SearchOrder.hasValue() )
        {
            nSearchOrder =  ::comphelper::getINT16( SearchOrder );
            sal_Bool bSearchByRow = false;
            if ( nSearchOrder == excel::XlSearchOrder::xlByColumns )
                bSearchByRow = false;
            else if ( nSearchOrder == excel::XlSearchOrder::xlByRows )
                bSearchByRow = sal_True;
            else
                throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for SearchOrder" ) , uno::Reference< uno::XInterface >() );

            newOptions.SetRowDirection( bSearchByRow );
            xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHBYROW  ), uno::makeAny( bSearchByRow ) );
        }

        // SearchDirection
        if ( SearchDirection.hasValue() )
        {
            sal_Int32 nSearchDirection = 0;
            if( SearchDirection >>= nSearchDirection )
            {
                sal_Bool bSearchBackwards = false;
                if ( nSearchDirection == excel::XlSearchDirection::xlNext )
                    bSearchBackwards = false;
                else if( nSearchDirection == excel::XlSearchDirection::xlPrevious )
                    bSearchBackwards = sal_True;
                else
                    throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for SearchDirection" ) , uno::Reference< uno::XInterface >() );
                newOptions.SetBackward( bSearchBackwards );
                xDescriptor->setPropertyValue( rtl::OUString( "SearchBackwards" ), uno::makeAny( bSearchBackwards ) );
            }
        }

        // MatchCase
        sal_Bool bMatchCase = false;
        if ( MatchCase.hasValue() )
        {
            // SearchCaseSensitive
            if( !( MatchCase >>= bMatchCase ) )
                throw uno::RuntimeException( rtl::OUString( "Range::Replace, illegal value for MatchCase" ) , uno::Reference< uno::XInterface >() );
        }
        xDescriptor->setPropertyValue( rtl::OUString(  SC_UNO_SRCHCASE  ), uno::makeAny( bMatchCase ) );

        // MatchByte
        // SearchFormat
        // ignore

        ScGlobal::SetSearchItem( newOptions );

        uno::Reference< uno::XInterface > xInterface = xStartCell.is() ? xSearch->findNext( xStartCell, xDescriptor) : xSearch->findFirst( xDescriptor );
        uno::Reference< table::XCellRange > xCellRange( xInterface, uno::UNO_QUERY );
        // if we are searching from a starting cell and failed to find a match
        // then try from the begining
        if ( !xCellRange.is() && xStartCell.is() )
        {
            xInterface = xSearch->findFirst( xDescriptor );
            xCellRange.set( xInterface, uno::UNO_QUERY );
        }
        if ( xCellRange.is() )
        {
            uno::Reference< excel::XRange > xResultRange = new ScVbaRange( mxParent, mxContext, xCellRange );
            if( xResultRange.is() )
            {
                xResultRange->Select();
                return xResultRange;
            }
        }

    }

    return uno::Reference< excel::XRange >();
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
            throw uno::RuntimeException( rtl::OUString("Range::Sort no docshell to calculate key param"), uno::Reference< uno::XInterface >() );
        xKeyRange = getRangeForName( xContext, sRangeName, pDocSh, aRefAddr );
    }
    else
        throw uno::RuntimeException( rtl::OUString("Range::Sort illegal type value for key param"), uno::Reference< uno::XInterface >() );
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
        throw uno::RuntimeException( rtl::OUString("Range::Sort unknown sort property"), uno::Reference< uno::XInterface >() );
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
        if (  ( !bIsSortColumn && colRowKeyAddress.StartColumn >= parentRangeAddress.StartColumn &&
            colRowKeyAddress.StartColumn <= parentRangeAddress.EndColumn ) || ( bIsSortColumn &&
            colRowKeyAddress.StartRow >= parentRangeAddress.StartRow &&
            colRowKeyAddress.StartRow <= parentRangeAddress.EndRow  ) )
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
                aTableField.IsAscending = false;
        }
        else
            throw uno::RuntimeException( rtl::OUString(  "Illegal Key param"  ), uno::Reference< uno::XInterface >() );


}

void SAL_CALL
ScVbaRange::Sort( const uno::Any& Key1, const uno::Any& Order1, const uno::Any& Key2, const uno::Any& /*Type*/, const uno::Any& Order2, const uno::Any& Key3, const uno::Any& Order3, const uno::Any& Header, const uno::Any& OrderCustom, const uno::Any& MatchCase, const uno::Any& Orientation, const uno::Any& SortMethod,  const uno::Any& DataOption1, const uno::Any& DataOption2, const uno::Any& DataOption3  ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# can we do something with Type
    if ( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( "That command cannot be used on multiple selections"  ), uno::Reference< uno::XInterface >() );

    sal_Int16 nDataOption1 = excel::XlSortDataOption::xlSortNormal;
    sal_Int16 nDataOption2 = excel::XlSortDataOption::xlSortNormal;
    sal_Int16 nDataOption3 = excel::XlSortDataOption::xlSortNormal;

    ScDocument* pDoc = getScDocument();
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString(  "Failed to access document from shell"  ), uno::Reference< uno::XInterface >() );

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

    // set up defaults

    sal_Int16 nOrder1 = aSortParam.maKeyState[1].bAscending ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;
    sal_Int16 nOrder2 = aSortParam.maKeyState[2].bAscending ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;
    sal_Int16 nOrder3 = aSortParam.maKeyState[3].bAscending ? excel::XlSortOrder::xlAscending : excel::XlSortOrder::xlDescending;

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
            aSortParam.bByRow = false;
        else
            aSortParam.bByRow = sal_True;

    }

    sal_Bool bIsSortColumns=false; // sort by row

    if ( nOrientation == excel::XlSortOrientation::xlSortRows )
        bIsSortColumns = sal_True;
    sal_Int16 nHeader = 0;
    nHeader = aSortParam.nCompatHeader;
    sal_Bool bContainsHeader = false;

    if ( Header.hasValue() )
    {
        nHeader = ::comphelper::getINT16( Header );
        aSortParam.nCompatHeader = nHeader;
    }

    if ( nHeader == excel::XlYesNoGuess::xlGuess )
    {
        bool bHasColHeader = pDoc->HasColHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ));
        bool bHasRowHeader = pDoc->HasRowHeader(  static_cast< SCCOL >( thisRangeAddress.StartColumn ), static_cast< SCROW >( thisRangeAddress.StartRow ), static_cast< SCCOL >( thisRangeAddress.EndColumn ), static_cast< SCROW >( thisRangeAddress.EndRow ), static_cast< SCTAB >( thisRangeAddress.Sheet ) );
        if ( bHasColHeader || bHasRowHeader )
            nHeader =  excel::XlYesNoGuess::xlYes;
        else
            nHeader =  excel::XlYesNoGuess::xlNo;
        aSortParam.nCompatHeader = nHeader;
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
            aSortParam.maKeyState[0].bAscending  = true;
        else
            aSortParam.maKeyState[0].bAscending  = false;

    }
    if ( Order2.hasValue() )
    {
        nOrder2 = ::comphelper::getINT16(Order2);
        if ( nOrder2 == excel::XlSortOrder::xlAscending )
            aSortParam.maKeyState[1].bAscending  = true;
        else
            aSortParam.maKeyState[1].bAscending  = false;
    }
    if ( Order3.hasValue() )
    {
        nOrder3 = ::comphelper::getINT16(Order3);
        if ( nOrder3 == excel::XlSortOrder::xlAscending )
            aSortParam.maKeyState[2].bAscending  = true;
        else
            aSortParam.maKeyState[2].bAscending  = false;
    }

    uno::Reference< table::XCellRange > xKey1;
    uno::Reference< table::XCellRange > xKey2;
    uno::Reference< table::XCellRange > xKey3;
    ScDocShell* pDocShell = getScDocShell();
    xKey1 = processKey( Key1, mxContext, pDocShell );
    if ( !xKey1.is() )
        throw uno::RuntimeException( rtl::OUString("Range::Sort needs a key1 param"), uno::Reference< uno::XInterface >() );

    if ( Key2.hasValue() )
        xKey2 = processKey( Key2, mxContext, pDocShell );
    if ( Key3.hasValue() )
        xKey3 = processKey( Key3, mxContext, pDocShell );

    uno::Reference< util::XSortable > xSort( mxRange, uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > sortDescriptor = xSort->createSortDescriptor();
    sal_Int32 nTableSortFieldIndex = findSortPropertyIndex( sortDescriptor, rtl::OUString( "SortFields" ) );

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

    sal_Int32 nIndex =  findSortPropertyIndex( sortDescriptor,  rtl::OUString("IsSortColumns") );
    sortDescriptor[ nIndex ].Value <<= bIsSortColumns;

    nIndex =    findSortPropertyIndex( sortDescriptor, CONTS_HEADER );
    sortDescriptor[ nIndex ].Value <<= bContainsHeader;

    pDoc->SetSortParam( aSortParam, nTab );
    xSort->sort( sortDescriptor );

    // #FIXME #TODO
    // The SortMethod param is not processed ( not sure what its all about, need to
    (void)nSortMethod;
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
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    rtl::OUString sActiveCell = xApplication->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

    // position current cell upper left of this range
    Cells( uno::makeAny( (sal_Int32) 1 ), uno::makeAny( (sal_Int32) 1 ) )->Select();

        uno::Reference< frame::XModel > xModel = getModelFromRange( mxRange );

    SfxViewFrame* pViewFrame = excel::getViewFrame( xModel );
    if ( pViewFrame )
    {
        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
        // Hoping this will make sure this slot is called
        // synchronously
        SfxBoolItem sfxAsync( SID_ASYNCHRON, sal_False );
        aArgs.Put( sfxAsync, sfxAsync.Which() );
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();

        sal_uInt16 nSID = 0;

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
    rtl::OUString sMoved =    xApplication->getActiveCell()->Address(aDft, aDft, aDft, aDft, aDft );

    // restore old ActiveCell
    uno::Any aVoid;

    uno::Reference< excel::XRange > xOldActiveCell( xApplication->getActiveSheet()->Range( uno::makeAny( sActiveCell ), aVoid ), uno::UNO_QUERY_THROW );
    xOldActiveCell->Select();

    uno::Reference< excel::XRange > resultCell;
    resultCell.set( xApplication->getActiveSheet()->Range( uno::makeAny( sMoved ), aVoid ), uno::UNO_QUERY_THROW );

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
        throw uno::RuntimeException( rtl::OUString( "Can't create Characters property for multicell range " ), uno::Reference< uno::XInterface >() );
    uno::Reference< text::XSimpleText > xSimple(mxRange->getCellByPosition(0,0) , uno::UNO_QUERY_THROW );
    ScDocument* pDoc = getDocumentFromRange(mxRange);
    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString(  "Failed to access document from shell"  ), uno::Reference< uno::XInterface >() );

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
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
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
                throw uno::RuntimeException( rtl::OUString( "Illegal paramater " ), uno::Reference< uno::XInterface >() );
        }
    }
    else
        {
        bool bFullRow = ( thisAddress.StartColumn == 0 && thisAddress.EndColumn == MAXCOL );
            sal_Int32 nCols = thisAddress.EndColumn - thisAddress.StartColumn;
            sal_Int32 nRows = thisAddress.EndRow - thisAddress.StartRow;
        if ( mbIsRows || bFullRow || ( nCols >=  nRows ) )
            mode = sheet::CellDeleteMode_UP;
        else
            mode = sheet::CellDeleteMode_LEFT;
    }
    uno::Reference< sheet::XCellRangeMovement > xCellRangeMove( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
    xCellRangeMove->removeRange( thisAddress, mode );

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
    return false;
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL
ScVbaRange::createEnumeration() throw (uno::RuntimeException)
{
    if ( mbIsColumns || mbIsRows )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(mxRange, uno::UNO_QUERY );
        uno::Reference< excel::XRange > xRange( m_Areas->Item( uno::makeAny( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
                sal_Int32 nElems = 0;
        if ( mbIsColumns )
            nElems = xColumnRowRange->getColumns()->getCount();
        else
            nElems = xColumnRowRange->getRows()->getCount();
                return new ColumnsRowEnumeration( mxContext, xRange, nElems );

    }
    return new CellsEnumeration( mxParent, mxContext, m_Areas );
}

::rtl::OUString SAL_CALL
ScVbaRange::getDefaultMethodName(  ) throw (uno::RuntimeException)
{
    const static rtl::OUString sName( "Item" );
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
    sal_uInt16 nWidth = pDoc->GetOriginalWidth( static_cast< SCCOL >( rAddress.StartColumn ), static_cast< SCTAB >( rAddress.Sheet ) );
    double nPoints = lcl_TwipsToPoints( nWidth );
    nPoints = lcl_Round2DecPlaces( nPoints );
    return nPoints;
}

double
ScVbaRange::getCalcRowHeight( const table::CellRangeAddress& rAddress ) throw (uno::RuntimeException)
{
    ScDocument* pDoc = getDocumentFromRange( mxRange );
    sal_uInt16 nWidth = pDoc->GetOriginalHeight( rAddress.StartRow, rAddress.Sheet );
    double nPoints = lcl_TwipsToPoints( nWidth );
    nPoints = lcl_Round2DecPlaces( nPoints );
    return nPoints;
}

// return Char Width in points
double getDefaultCharWidth( const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException )
{
    const static rtl::OUString sDflt( "Default");
    const static rtl::OUString sCharFontName( "CharFontName");
    const static rtl::OUString sPageStyles( "PageStyles");
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
    return lcl_TwipsToPoints( (sal_uInt16)nCharWidth );
}

double getDefaultCharWidth( ScDocShell* pDocShell )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    OutputDevice* pRefDevice = pDoc->GetRefDevice();
    ScPatternAttr* pAttr = pDoc->GetDefPattern();
    ::Font aDefFont;
    pAttr->GetFont( aDefFont, SC_AUTOCOL_BLACK, pRefDevice );
    pRefDevice->SetFont( aDefFont );
    long nCharWidth = pRefDevice->GetTextWidth( rtl::OUString('0') );        // 1/100th mm
    return lcl_hmmToPoints( nCharWidth );
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
        sal_uInt16 nColTwips = 0;
        for( sal_Int32 nCol = nStartCol ; nCol <= nEndCol; ++nCol )
        {
            thisAddress.StartColumn = nCol;
            sal_uInt16 nCurTwips = pShell->GetDocument()->GetOriginalWidth( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCTAB >( thisAddress.Sheet ) );
            if ( nCol == nStartCol )
                nColTwips =  nCurTwips;
            if ( nColTwips != nCurTwips )
                return aNULL();
        }
        nColWidth = lcl_TwipsToPoints( nColTwips );
        if ( nColWidth != 0.0 )
            nColWidth = ( nColWidth / defaultCharWidth ) - fExtraWidth;
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
        if ( nColWidth != 0.0 )
            nColWidth = ( nColWidth + fExtraWidth ) * getDefaultCharWidth( pDocShell );
        RangeHelper thisRange( mxRange );
        table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
        sal_uInt16 nTwips = lcl_pointsToTwips( nColWidth );

        SCCOLROW nColArr[2];
        nColArr[0] = thisAddress.StartColumn;
        nColArr[1] = thisAddress.EndColumn;
        // #163561# use mode SC_SIZE_DIRECT: hide for width 0, show for other values
        pDocShell->GetDocFunc().SetWidthOrHeight( true, 1, nColArr, thisAddress.Sheet,
                                                  SC_SIZE_DIRECT, nTwips, true, true );

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
        throw uno::RuntimeException( rtl::OUString("No areas available"), uno::Reference< uno::XInterface >() );
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
                return uno::makeAny( false );
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

    // if any row's RowHeight in the
    // range is different from any other then return NULL
    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();

    sal_Int32 nStartRow = thisAddress.StartRow;
    sal_Int32 nEndRow = thisAddress.EndRow;
        sal_uInt16 nRowTwips = 0;
    // #TODO probably possible to use the SfxItemSet ( and see if
    //  SFX_ITEM_DONTCARE is set ) to improve performance
// #CHECKME looks like this is general behaviour not just row Range specific
//  if ( mbIsRows )
    ScDocShell* pShell = getScDocShell();
    if ( pShell )
    {
        for ( sal_Int32 nRow = nStartRow ; nRow <= nEndRow; ++nRow )
        {
            thisAddress.StartRow = nRow;
            sal_uInt16 nCurTwips = pShell->GetDocument()->GetOriginalHeight( thisAddress.StartRow, thisAddress.Sheet );
            if ( nRow == nStartRow )
                nRowTwips = nCurTwips;
            if ( nRowTwips != nCurTwips )
                return aNULL();
        }
    }
    double nHeight = lcl_Round2DecPlaces( lcl_TwipsToPoints( nRowTwips ) );
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
    sal_uInt16 nTwips = lcl_pointsToTwips( nHeight );

    ScDocShell* pDocShell = getDocShellFromRange( mxRange );
    SCCOLROW nRowArr[2];
    nRowArr[0] = thisAddress.StartRow;
    nRowArr[1] = thisAddress.EndRow;
    pDocShell->GetDocFunc().SetWidthOrHeight( false, 1, nRowArr, thisAddress.Sheet, SC_SIZE_ORIGINAL,
                                                                        nTwips, true, true );
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
        sal_Bool bColumn = false;

        if (thisAddress.StartRow==0)
            bColumn = true;

        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        if ( xModel.is() )
        {
            ScDocument* pDoc = getDocumentFromRange( mxRange );

            ScBreakType nBreak = BREAK_NONE;
            if ( !bColumn )
                nBreak = pDoc->HasRowBreak(thisAddress.StartRow, thisAddress.Sheet);
            else
                nBreak = pDoc->HasColBreak(thisAddress.StartColumn, thisAddress.Sheet);

            if (nBreak & BREAK_PAGE)
                nPageBreak = excel::XlPageBreak::xlPageBreakAutomatic;

            if (nBreak & BREAK_MANUAL)
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
        sal_Bool bColumn = false;

        if (thisAddress.StartRow==0)
            bColumn = true;

        ScAddress aAddr( static_cast<SCCOL>(thisAddress.StartColumn), thisAddress.StartRow, thisAddress.Sheet );
        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        if ( xModel.is() )
        {
            ScTabViewShell* pViewShell = excel::getBestViewShell( xModel );
            if ( nPageBreak == excel::XlPageBreak::xlPageBreakManual )
                pViewShell->InsertPageBreak( bColumn, sal_True, &aAddr);
            else if ( nPageBreak == excel::XlPageBreak::xlPageBreakNone )
                pViewShell->DeletePageBreak( bColumn, sal_True, &aAddr);
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
           xSheet.set( new ScVbaWorksheet( uno::Reference< XHelperInterface >(), mxContext,rHelper.getSpreadSheet(),pDocShell->GetModel()) );
    }
    return xSheet;
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
    if ( Cell1.hasValue() && !Cell2.hasValue() && !sRangeName.isEmpty() )
    {
        const static rtl::OUString sNamedRanges( "NamedRanges");
        uno::Reference< beans::XPropertySet > xPropSet( getCurrentExcelDoc(xContext), uno::UNO_QUERY_THROW );

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
                uno::Reference< excel::XRange > xVbRange =  new ScVbaRange( excel::getUnoSheetModuleObj( xRange ), xContext, xRange );
                return xVbRange;
            }
        }
    }

    uno::Reference< sheet::XSpreadsheetView > xView( getCurrentExcelDoc(xContext)->getCurrentController(), uno::UNO_QUERY );
    uno::Reference< table::XCellRange > xSheetRange( xView->getActiveSheet(), uno::UNO_QUERY_THROW );
    ScVbaRange* pRange = new ScVbaRange( excel::getUnoSheetModuleObj( xSheetRange ), xContext, xSheetRange );
    uno::Reference< excel::XRange > xVbSheetRange( pRange );
    return pRange->Range( Cell1, Cell2, true );
}

// Helper functions for AutoFilter
ScDBData* lcl_GetDBData_Impl( ScDocShell* pDocShell, sal_Int16 nSheet )
{
    ScDBData* pRet = NULL;
    if (pDocShell)
    {
        pRet = pDocShell->GetDocument()->GetAnonymousDBData(nSheet);
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
            pViewData->GetView()->Query( aParam, NULL, sal_True );
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

void lcl_SetAllQueryForField( ScDocShell* pDocShell, SCCOLROW nField, sal_Int16 nSheet )
{
    ScQueryParam aParam = lcl_GetQueryParam( pDocShell, nSheet );
    aParam.RemoveEntryByField(nField);
    lcl_SelectAll( pDocShell, aParam );
}

// Modifies sCriteria, and nOp depending on the value of sCriteria
void lcl_setTableFieldsFromCriteria( rtl::OUString& sCriteria1, uno::Reference< beans::XPropertySet >& xDescProps, sheet::TableFilterField2& rFilterField )
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
            rFilterField.Operator = sheet::FilterOperator2::EMPTY;
        else
        {
            rFilterField.Operator = sheet::FilterOperator2::EQUAL;
            sCriteria1 = sCriteria1.copy( EQUALS.getLength() );
            sCriteria1 = VBAToRegexp( sCriteria1 );
            // UseRegularExpressions
            if ( xDescProps.is() )
                xDescProps->setPropertyValue( rtl::OUString(  "UseRegularExpressions"  ), uno::Any( sal_True ) );
        }

    }
    else if ( ( nPos = sCriteria1.indexOf( NOTEQUALS ) ) == 0 )
    {
        if ( sCriteria1.getLength() == NOTEQUALS.getLength() )
            rFilterField.Operator = sheet::FilterOperator2::NOT_EMPTY;
        else
        {
            rFilterField.Operator = sheet::FilterOperator2::NOT_EQUAL;
            sCriteria1 = sCriteria1.copy( NOTEQUALS.getLength() );
            sCriteria1 = VBAToRegexp( sCriteria1 );
            // UseRegularExpressions
            if ( xDescProps.is() )
                xDescProps->setPropertyValue( rtl::OUString(  "UseRegularExpressions"  ), uno::Any( sal_True ) );
        }
    }
    else if ( ( nPos = sCriteria1.indexOf( GREATERTHAN ) ) == 0 )
    {
        bIsNumeric = true;
        if ( ( nPos = sCriteria1.indexOf( GREATERTHANEQUALS ) ) == 0 )
        {
            sCriteria1 = sCriteria1.copy( GREATERTHANEQUALS.getLength() );
            rFilterField.Operator = sheet::FilterOperator2::GREATER_EQUAL;
        }
        else
        {
            sCriteria1 = sCriteria1.copy( GREATERTHAN.getLength() );
            rFilterField.Operator = sheet::FilterOperator2::GREATER;
        }

    }
    else if ( ( nPos = sCriteria1.indexOf( LESSTHAN ) ) == 0 )
    {
        bIsNumeric = true;
        if ( ( nPos = sCriteria1.indexOf( LESSTHANEQUALS ) ) == 0 )
        {
            sCriteria1 = sCriteria1.copy( LESSTHANEQUALS.getLength() );
            rFilterField.Operator = sheet::FilterOperator2::LESS_EQUAL;
        }
        else
        {
            sCriteria1 = sCriteria1.copy( LESSTHAN.getLength() );
            rFilterField.Operator = sheet::FilterOperator2::LESS;
        }

    }
    else
        rFilterField.Operator = sheet::FilterOperator2::EQUAL;

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
    sal_Bool bHasAuto = false;
    uno::Reference< sheet::XDatabaseRange > xDataBaseRange = excel::GetAutoFiltRange( pShell, nSheet );
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
                ScVbaRange* pRange = getImplementation( xCurrent );
                if ( pRange )
                {
                    if ( pRange->isSingleCellRange() )
                       throw uno::RuntimeException( rtl::OUString( "Can't create AutoFilter" ), uno::Reference< uno::XInterface >() );
                    RangeHelper currentRegion( pRange->mxRange );
                    autoFiltAddress = currentRegion.getCellRangeAddressable()->getRangeAddress();
                }
            }
        }
        else // multi-cell range
        {
            RangeHelper multiCellRange( mxRange );
            autoFiltAddress = multiCellRange.getCellRangeAddressable()->getRangeAddress();
            // #163530# Filter box shows only entry of first row
            ScDocument* pDocument = ( pShell ? pShell->GetDocument() : NULL );
            if ( pDocument )
            {
                SCCOL nStartCol = autoFiltAddress.StartColumn;
                SCROW nStartRow = autoFiltAddress.StartRow;
                SCCOL nEndCol = autoFiltAddress.EndColumn;
                SCROW nEndRow = autoFiltAddress.EndRow;
                pDocument->GetDataArea( autoFiltAddress.Sheet, nStartCol, nStartRow, nEndCol, nEndRow, true, true );
                autoFiltAddress.StartColumn = nStartCol;
                autoFiltAddress.StartRow = nStartRow;
                autoFiltAddress.EndColumn = nEndCol;
                autoFiltAddress.EndRow = nEndRow;
            }
        }

        uno::Reference< sheet::XUnnamedDatabaseRanges > xDBRanges = excel::GetUnnamedDataBaseRanges( pShell );
        if ( xDBRanges.is() )
        {
            OSL_TRACE("Going to add new autofilter range.. sheet %i", nSheet );
            if ( !xDBRanges->hasByTable( nSheet ) )
                xDBRanges->setByTable( autoFiltAddress );
            xDataBaseRange.set( xDBRanges->getByTable(nSheet ), uno::UNO_QUERY_THROW );
        }
        if ( !xDataBaseRange.is() )
            throw uno::RuntimeException( rtl::OUString( "Failed to find the autofilter placeholder range"  ), uno::Reference< uno::XInterface >() );

        uno::Reference< beans::XPropertySet > xDBRangeProps( xDataBaseRange, uno::UNO_QUERY_THROW );
        // set autofilt
        xDBRangeProps->setPropertyValue( rtl::OUString( "AutoFilter" ), uno::Any(sal_True) );
        // set header
        uno::Reference< beans::XPropertySet > xFiltProps( xDataBaseRange->getFilterDescriptor(), uno::UNO_QUERY_THROW );
        sal_Bool bHasColHeader = false;
        ScDocument* pDoc = pShell ? pShell->GetDocument() : NULL;

        bHasColHeader = pDoc->HasColHeader(  static_cast< SCCOL >( autoFiltAddress.StartColumn ), static_cast< SCROW >( autoFiltAddress.StartRow ), static_cast< SCCOL >( autoFiltAddress.EndColumn ), static_cast< SCROW >( autoFiltAddress.EndRow ), static_cast< SCTAB >( autoFiltAddress.Sheet ) ) ? sal_True : false;
        xFiltProps->setPropertyValue( rtl::OUString( "ContainsHeader" ), uno::Any( bHasColHeader ) );
    }


    sal_Int32 nField = 0; // *IS* 1 based
    rtl::OUString sCriteria1;
    sal_Int32 nOperator = excel::XlAutoFilterOperator::xlAnd;

    sal_Bool bVisible = sal_True;
    VisibleDropDown >>= bVisible;

    if ( bVisible == bHasAuto ) // dropdown is displayed/notdisplayed as required
        bVisible = false;
    sheet::FilterConnection nConn = sheet::FilterConnection_AND;
    double nCriteria1 = 0;

    bool bHasCritValue = Criteria1.hasValue();
    bool bCritHasNumericValue = false; // not sure if a numeric criteria is possible
    if ( bHasCritValue )
        bCritHasNumericValue = ( Criteria1 >>= nCriteria1 );

    if (  !Field.hasValue() && ( Criteria1.hasValue() || Operator.hasValue() || Criteria2.hasValue() ) )
        throw uno::RuntimeException();
    // Use the normal uno api, sometimes e.g. when you want to use ALL as the filter
    // we can't use refresh as the uno interface doesn't have a concept of ALL
    // in this case we just call the core calc functionality -
    bool bAll = false;
    if ( ( Field >>= nField )  )
    {
        uno::Reference< sheet::XSheetFilterDescriptor2 > xDesc(
                xDataBaseRange->getFilterDescriptor(), uno::UNO_QUERY );
        if ( xDesc.is() )
        {
            uno::Sequence< sheet::TableFilterField2 > sTabFilts;
            uno::Reference< beans::XPropertySet > xDescProps( xDesc, uno::UNO_QUERY_THROW );
        if ( Criteria1.hasValue() )
        {
            sTabFilts.realloc( 1 );
            sTabFilts[0].Operator = sheet::FilterOperator2::EQUAL;// sensible default
            if ( !bCritHasNumericValue )
            {
                Criteria1 >>= sCriteria1;
                sTabFilts[0].IsNumeric = bCritHasNumericValue;
                if ( bHasCritValue && !sCriteria1.isEmpty() )
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
            if ( !bCritHasNumericValue && sCriteria1.isEmpty() && ( nOperator != excel::XlAutoFilterOperator::xlOr ) && ( nOperator != excel::XlAutoFilterOperator::xlAnd ) )
            {
                sTabFilts[0].IsNumeric = sal_True;
                sTabFilts[0].NumericValue = 10;
                bAll = false;
            }
            switch ( nOperator )
            {
                case excel::XlAutoFilterOperator::xlBottom10Items:
                    sTabFilts[0].Operator = sheet::FilterOperator2::BOTTOM_VALUES;
                    break;
                case excel::XlAutoFilterOperator::xlBottom10Percent:
                    sTabFilts[0].Operator = sheet::FilterOperator2::BOTTOM_PERCENT;
                    break;
                case excel::XlAutoFilterOperator::xlTop10Items:
                    sTabFilts[0].Operator = sheet::FilterOperator2::TOP_VALUES;
                    break;
                case excel::XlAutoFilterOperator::xlTop10Percent:
                    sTabFilts[0].Operator = sheet::FilterOperator2::TOP_PERCENT;
                    break;
                case excel::XlAutoFilterOperator::xlOr:
                    nConn = sheet::FilterConnection_OR;
                    break;
                case excel::XlAutoFilterOperator::xlAnd:
                    nConn = sheet::FilterConnection_AND;
                    break;
                default:
                    throw uno::RuntimeException( rtl::OUString( "UnknownOption" ), uno::Reference< uno::XInterface >() );

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
                    if ( !sCriteria2.isEmpty() )
                    {
                        uno::Reference< beans::XPropertySet > xProps;
                        lcl_setTableFieldsFromCriteria( sCriteria2, xProps,  sTabFilts[1] );
                        sTabFilts[1].IsNumeric = false;
                    }
                }
                else // numeric
                {
                    Criteria2 >>= sTabFilts[1].NumericValue;
                    sTabFilts[1].IsNumeric = sal_True;
                    sTabFilts[1].Operator = sheet::FilterOperator2::EQUAL;
                }
            }
        }

        xDesc->setFilterFields2( sTabFilts );
        if ( !bAll )
        {
            xDataBaseRange->refresh();
        }
        else
            // was 0 based now seems to be 1
            lcl_SetAllQueryForField( pShell, nField, nSheet );
        }
    }
    else
    {
        // this is just to toggle autofilter on and off ( not to be confused with
        // a VisibleDropDown option combined with a field, in that case just the
        // button should be disabled ) - currently we don't support that
        uno::Reference< beans::XPropertySet > xDBRangeProps( xDataBaseRange, uno::UNO_QUERY_THROW );
        if ( bHasAuto )
        {
            // find the any field with the query and select all
            ScQueryParam aParam = lcl_GetQueryParam( pShell, nSheet );
            for (SCSIZE i = 0; i< aParam.GetEntryCount(); ++i)
            {
                ScQueryEntry& rEntry = aParam.GetEntry(i);
                if ( rEntry.bDoQuery )
                    lcl_SetAllQueryForField( pShell, rEntry.nField, nSheet );
            }
            // remove exising filters
            uno::Reference< sheet::XSheetFilterDescriptor2 > xSheetFilterDescriptor(
                    xDataBaseRange->getFilterDescriptor(), uno::UNO_QUERY );
            if( xSheetFilterDescriptor.is() )
                xSheetFilterDescriptor->setFilterFields2( uno::Sequence< sheet::TableFilterField2 >() );
        }
        xDBRangeProps->setPropertyValue( rtl::OUString( "AutoFilter" ), uno::Any(!bHasAuto) );

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
                throw uno::RuntimeException( rtl::OUString( "Illegal paramater " ), uno::Reference< uno::XInterface >() );
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
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    uno::Reference< sheet::XCellRangeMovement > xCellRangeMove( thisRange.getSpreadSheet(), uno::UNO_QUERY_THROW );
    xCellRangeMove->insertCells( thisAddress, mode );

    // Paste from clipboard only if the clipboard content was copied via VBA, and not already pasted via VBA again.
    // "Insert" behavior should not depend on random clipboard content previously copied by the user.
    ScTransferObj* pClipObj = ScTransferObj::GetOwnClipboard( NULL );
    if ( pClipObj && pClipObj->GetUseInApi() )
    {
        // After the insert ( this range ) actually has moved
        ScRange aRange( static_cast< SCCOL >( thisAddress.StartColumn ), static_cast< SCROW >( thisAddress.StartRow ), static_cast< SCTAB >( thisAddress.Sheet ), static_cast< SCCOL >( thisAddress.EndColumn ), static_cast< SCROW >( thisAddress.EndRow ), static_cast< SCTAB >( thisAddress.Sheet ) );
        uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getDocShellFromRange( mxRange ) , aRange ) );
        uno::Reference< excel::XRange > xVbaRange( new ScVbaRange( mxParent, mxContext, xRange, mbIsRows, mbIsColumns ) );
        xVbaRange->PasteSpecial( uno::Any(), uno::Any(), uno::Any(), uno::Any() );
    }
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

        if ( !( mbIsColumns || mbIsRows ) )
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
        ScDocShell* pDocShell = getDocShellFromRange( mxRange );
        if ( pDocShell )
        {
            RangeHelper thisRange( mxRange );
            table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();

            SCCOLROW nColArr[2];
            nColArr[0] = thisAddress.StartColumn;
            nColArr[1] = thisAddress.EndColumn;
            sal_Bool bDirection = sal_True;
            if ( mbIsRows )
            {
                bDirection = false;
                nColArr[0] = thisAddress.StartRow;
                nColArr[1] = thisAddress.EndRow;
            }
            pDocShell->GetDocFunc().SetWidthOrHeight( bDirection, 1, nColArr, thisAddress.Sheet,
                                                      SC_SIZE_OPTIMAL, 0, true, true );
    }
}

uno::Any SAL_CALL
ScVbaRange::Hyperlinks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    /*  The range object always returns a new Hyperlinks object containing a
        fixed list of existing hyperlinks in the range.
        See vbahyperlinks.hxx for more details. */

    // get the global hyperlink object of the sheet (sheet should always be the parent of a Range object)
    uno::Reference< excel::XWorksheet > xWorksheet( getParent(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XHyperlinks > xSheetHlinks( xWorksheet->Hyperlinks( uno::Any() ), uno::UNO_QUERY_THROW );
    ScVbaHyperlinksRef xScSheetHlinks( dynamic_cast< ScVbaHyperlinks* >( xSheetHlinks.get() ) );
    if( !xScSheetHlinks.is() )
        throw uno::RuntimeException( ::rtl::OUString(  "Cannot obtain hyperlinks implementation object"  ), uno::Reference< uno::XInterface >() );

    // create a new local hyperlinks object based on the sheet hyperlinks
    ScVbaHyperlinksRef xHlinks( new ScVbaHyperlinks( getParent(), mxContext, xScSheetHlinks, getScRangeList() ) );
    if( aIndex.hasValue() )
        return xHlinks->Item( aIndex, uno::Any() );
    return uno::Any( uno::Reference< excel::XHyperlinks >( xHlinks.get() ) );
}

css::uno::Reference< excel::XValidation > SAL_CALL
ScVbaRange::getValidation() throw (css::uno::RuntimeException)
{
    if ( !m_xValidation.is() )
        m_xValidation = new ScVbaValidation( this, mxContext, mxRange );
    return m_xValidation;
}

namespace {

sal_Unicode lclGetPrefixChar( const uno::Reference< table::XCell >& rxCell ) throw (uno::RuntimeException)
{
    /*  TODO/FIXME: We need an apostroph-prefix property at the cell to
        implement this correctly. For now, return an apostroph for every text
        cell.

        TODO/FIXME: When Application.TransitionNavigKeys is supported and true,
        this function needs to inspect the cell formatting and return different
        prefixes according to the horizontal cell alignment.
     */
    return (rxCell->getType() == table::CellContentType_TEXT) ? '\'' : 0;
}

sal_Unicode lclGetPrefixChar( const uno::Reference< table::XCellRange >& rxRange ) throw (uno::RuntimeException)
{
    /*  This implementation is able to handle different prefixes (needed if
        Application.TransitionNavigKeys is true). The function lclGetPrefixChar
        for single cells called from here may return any prefix. If that
        function returns an empty prefix (NUL character) or different non-empty
        prefixes for two cells, this function returns 0.
     */
    sal_Unicode cCurrPrefix = 0;
    table::CellRangeAddress aRangeAddr = lclGetRangeAddress( rxRange );
    sal_Int32 nEndCol = aRangeAddr.EndColumn - aRangeAddr.StartColumn;
    sal_Int32 nEndRow = aRangeAddr.EndRow - aRangeAddr.StartRow;
    for( sal_Int32 nRow = 0; nRow <= nEndRow; ++nRow )
    {
        for( sal_Int32 nCol = 0; nCol <= nEndCol; ++nCol )
        {
            uno::Reference< table::XCell > xCell( rxRange->getCellByPosition( nCol, nRow ), uno::UNO_SET_THROW );
            sal_Unicode cNewPrefix = lclGetPrefixChar( xCell );
            if( (cNewPrefix == 0) || ((cCurrPrefix != 0) && (cNewPrefix != cCurrPrefix)) )
                return 0;
            cCurrPrefix = cNewPrefix;
        }
    }
    // all cells contain the same prefix - return it
    return cCurrPrefix;
}

sal_Unicode lclGetPrefixChar( const uno::Reference< sheet::XSheetCellRangeContainer >& rxRanges ) throw (uno::RuntimeException)
{
    sal_Unicode cCurrPrefix = 0;
    uno::Reference< container::XEnumerationAccess > xRangesEA( rxRanges, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xRangesEnum( xRangesEA->createEnumeration(), uno::UNO_SET_THROW );
    while( xRangesEnum->hasMoreElements() )
    {
        uno::Reference< table::XCellRange > xRange( xRangesEnum->nextElement(), uno::UNO_QUERY_THROW );
        sal_Unicode cNewPrefix = lclGetPrefixChar( xRange );
        if( (cNewPrefix == 0) || ((cCurrPrefix != 0) && (cNewPrefix != cCurrPrefix)) )
            return 0;
        cCurrPrefix = cNewPrefix;
    }
    // all ranges contain the same prefix - return it
    return cCurrPrefix;
}

inline uno::Any lclGetPrefixVariant( sal_Unicode cPrefixChar )
{
    return uno::Any( (cPrefixChar == 0) ? ::rtl::OUString() : ::rtl::OUString( cPrefixChar ) );
}

} // namespace

uno::Any SAL_CALL ScVbaRange::getPrefixCharacter() throw (uno::RuntimeException)
{
    /*  (1) If Application.TransitionNavigKeys is false, this function returns
        an apostroph character if the text cell begins with an apostroph
        character (formula return values are not taken into account); otherwise
        an empty string.

        (2) If Application.TransitionNavigKeys is true, this function returns
        an apostroph character, if the cell is left-aligned; a double-quote
        character, if the cell is right-aligned; a circumflex character, if the
        cell is centered; a backslash character, if the cell is set to filled;
        or an empty string, if nothing of the above.

        If a range or a list of ranges contains texts with leading apostroph
        character as well as other cells, this function returns an empty
        string.
     */

    if( mxRange.is() )
        return lclGetPrefixVariant( lclGetPrefixChar( mxRange ) );
    if( mxRanges.is() )
        return lclGetPrefixVariant( lclGetPrefixChar( mxRanges ) );
    throw uno::RuntimeException( ::rtl::OUString(  "Unexpected empty Range object"  ), uno::Reference< uno::XInterface >() );
}

uno::Any ScVbaRange::getShowDetail() throw ( css::uno::RuntimeException)
{
    // #FIXME, If the specified range is in a PivotTable report

    // In MSO VBA, the specified range must be a single summary column or row in an outline. otherwise throw exception
    if( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( "Can not get Range.ShowDetail attribute "), uno::Reference< uno::XInterface >() );

    sal_Bool bShowDetail = false;

    RangeHelper helper( mxRange );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = helper.getSheetCellCursor();
    xSheetCellCursor->collapseToCurrentRegion();
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
    table::CellRangeAddress aOutlineAddress = xCellRangeAddressable->getRangeAddress();

    // check if the specified range is a single summary column or row.
    table::CellRangeAddress thisAddress = helper.getCellRangeAddressable()->getRangeAddress();
    if( (thisAddress.StartRow == thisAddress.EndRow &&  thisAddress.EndRow == aOutlineAddress.EndRow ) ||
        (thisAddress.StartColumn == thisAddress.EndColumn && thisAddress.EndColumn == aOutlineAddress.EndColumn ))
    {
        sal_Bool bColumn =thisAddress.StartRow == thisAddress.EndRow ? false:sal_True;
        ScDocument* pDoc = getDocumentFromRange( mxRange );
        ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable(static_cast<SCTAB>(thisAddress.Sheet), sal_True);
        const ScOutlineArray* pOutlineArray =  bColumn ? pOutlineTable->GetColArray(): pOutlineTable->GetRowArray();
        if( pOutlineArray )
        {
            SCCOLROW nPos = bColumn ? (SCCOLROW)(thisAddress.EndColumn-1):(SCCOLROW)(thisAddress.EndRow-1);
            const ScOutlineEntry* pEntry = pOutlineArray->GetEntryByPos( 0, nPos );
            if( pEntry )
            {
                bShowDetail = !pEntry->IsHidden();
                return uno::makeAny( bShowDetail );
            }
        }
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( "Can not set Range.ShowDetail attribute "), uno::Reference< uno::XInterface >() );
    }
    return aNULL();
}

void ScVbaRange::setShowDetail(const uno::Any& aShowDetail) throw ( css::uno::RuntimeException)
{
    // #FIXME, If the specified range is in a PivotTable report

    // In MSO VBA, the specified range must be a single summary column or row in an outline. otherwise throw exception
    if( m_Areas->getCount() > 1 )
        throw uno::RuntimeException( rtl::OUString( "Can not set Range.ShowDetail attribute "), uno::Reference< uno::XInterface >() );

    bool bShowDetail = extractBoolFromAny( aShowDetail );

    RangeHelper helper( mxRange );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = helper.getSheetCellCursor();
    xSheetCellCursor->collapseToCurrentRegion();
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable(xSheetCellCursor, uno::UNO_QUERY_THROW);
    table::CellRangeAddress aOutlineAddress = xCellRangeAddressable->getRangeAddress();

    // check if the specified range is a single summary column or row.
    table::CellRangeAddress thisAddress = helper.getCellRangeAddressable()->getRangeAddress();
    if( (thisAddress.StartRow == thisAddress.EndRow &&  thisAddress.EndRow == aOutlineAddress.EndRow ) ||
        (thisAddress.StartColumn == thisAddress.EndColumn && thisAddress.EndColumn == aOutlineAddress.EndColumn ))
    {
        // #FIXME, seems there is a different behavior between MSO and OOo.
        //  In OOo, the showDetail will show all the level entrys, while only show the first level entry in MSO
        uno::Reference< sheet::XSheetOutline > xSheetOutline( helper.getSpreadSheet(), uno::UNO_QUERY_THROW );
        if( bShowDetail )
            xSheetOutline->showDetail( aOutlineAddress );
        else
            xSheetOutline->hideDetail( aOutlineAddress );
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( "Can not set Range.ShowDetail attribute "), uno::Reference< uno::XInterface >() );
    }
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaRange::MergeArea() throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< sheet::XSheetCellRange > xMergeShellCellRange(mxRange->getCellRangeByPosition(0,0,0,0), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xMergeSheetCursor(xMergeShellCellRange->getSpreadsheet()->createCursorByRange( xMergeShellCellRange ), uno::UNO_QUERY_THROW);
    if( xMergeSheetCursor.is() )
    {
        xMergeSheetCursor->collapseToMergedArea();
        uno::Reference<sheet::XCellRangeAddressable> xMergeCellAddress(xMergeSheetCursor, uno::UNO_QUERY_THROW);
        if( xMergeCellAddress.is() )
        {
            table::CellRangeAddress aCellAddress = xMergeCellAddress->getRangeAddress();
            if( aCellAddress.StartColumn ==0 && aCellAddress.EndColumn==0 &&
                aCellAddress.StartRow==0 && aCellAddress.EndRow==0)
            {
                return new ScVbaRange( mxParent,mxContext,mxRange );
            }
            else
            {
                ScRange refRange( static_cast< SCCOL >( aCellAddress.StartColumn ), static_cast< SCROW >( aCellAddress.StartRow ), static_cast< SCTAB >( aCellAddress.Sheet ),
                                  static_cast< SCCOL >( aCellAddress.EndColumn ), static_cast< SCROW >( aCellAddress.EndRow ), static_cast< SCTAB >( aCellAddress.Sheet ) );
                uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getScDocShell() , refRange ) );
                return new ScVbaRange( mxParent, mxContext,xRange );
            }
        }
    }
    return new ScVbaRange( mxParent, mxContext, mxRange );
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
            ScVbaRange* pRange = getImplementation( xRange );
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
            PrintOutHelper( excel::getBestViewShell( xModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, sal_True );
        }
    }
}

void SAL_CALL
ScVbaRange::AutoFill(  const uno::Reference< excel::XRange >& Destination, const uno::Any& Type ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xDest( Destination, uno::UNO_QUERY_THROW );
    ScVbaRange* pRange = getImplementation( xDest );
    RangeHelper destRangeHelper( pRange->mxRange );
    table::CellRangeAddress destAddress = destRangeHelper.getCellRangeAddressable()->getRangeAddress();

    RangeHelper thisRange( mxRange );
    table::CellRangeAddress thisAddress = thisRange.getCellRangeAddressable()->getRangeAddress();
    ScRange sourceRange;
    ScRange destRange;

    ScUnoConversion::FillScRange( destRange, destAddress );
    ScUnoConversion::FillScRange( sourceRange, thisAddress );

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

    FillCmd eCmd = FILL_AUTO;
    FillDateCmd eDateCmd = FILL_DAY;

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
                throw uno::RuntimeException( rtl::OUString(  "xlFillFormat not supported for AutoFill"  ), uno::Reference< uno::XInterface >() );
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
    double fEndValue =  MAXDOUBLE;
    ScDocShell* pDocSh = getDocShellFromRange( mxRange );
    pDocSh->GetDocFunc().FillAuto( aSourceRange, NULL, eDir, eCmd, eDateCmd,
                                   nCount, fStep, fEndValue, sal_True, sal_True );
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
            bRes = false;
    }
    else
        bRes = false;
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
    if ( m_Areas->getCount() > 1 )
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
    sal_Bool bAcross = false;
    Across >>= bAcross;
    if ( !bAcross )
        lcl_mergeCellsOfRange( mxRange );
    else
    {
        uno::Reference< excel::XRange > oRangeRowsImpl = Rows( uno::Any() );
        // #TODO #FIXME this seems incredibly lame, this can't be right
        for (sal_Int32 i=1; i <= oRangeRowsImpl->getCount();i++)
        {
            oRangeRowsImpl->Cells( uno::makeAny( i ), uno::Any() )->Merge( uno::makeAny( false ) );
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
    lcl_mergeCellsOfRange( mxRange, false);
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
    xProps->getPropertyValue(CELLSTYLE) >>= sStyleName;
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
    pDoc->GetNextPos( nNewX,nNewY, nTab, nMove,0, true,true, markedRange );
    refRange.aStart.SetCol( nNewX );
    refRange.aStart.SetRow( nNewY );
    refRange.aStart.SetTab( nTab );
    refRange.aEnd.SetCol( nNewX );
    refRange.aEnd.SetRow( nNewY );
    refRange.aEnd.SetTab( nTab );

    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getScDocShell() , refRange ) );

    return new ScVbaRange( mxParent, mxContext, xRange );
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
    uno::Reference< excel::XRange > xUsedRange( getWorksheet()->getUsedRange() );
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
        case excel::XlCellType::xlCellTypeLastCell:
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
                    ScVbaRange* pRange = getImplementation( xRange );
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
                if ( aCellRanges.size() == 1 )
                {
                    uno::Reference< table::XCellRange > xRange( new ScCellRangeObj( getScDocShell(), *aCellRanges.front() ) );
                    return new ScVbaRange( mxParent, mxContext, xRange );
                }
                uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( getScDocShell(), aCellRanges ) );

                return new ScVbaRange( mxParent, mxContext, xRanges );
            }
            else if ( bIsSingleCell )
            {
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
        sal_Bool bDoReplace = false;
        aReplace >>= bDoReplace;
        sal_Bool bAddPageBreaks = false;
        PageBreaks >>= bAddPageBreaks;

        uno::Reference< sheet::XSubTotalCalculatable> xSub(mxRange, uno::UNO_QUERY_THROW );
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

rtl::OUString
ScVbaRange::getServiceImplName()
{
    return rtl::OUString("ScVbaRange");
}

uno::Sequence< rtl::OUString >
ScVbaRange::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( "ooo.vba.excel.Range"  );
    }
    return aServiceNames;
}

sal_Bool SAL_CALL
ScVbaRange::hasError() throw (uno::RuntimeException)
{
    double dResult = false;
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    uno::Reference< script::XInvocation > xInvoc( xApplication->WorksheetFunction(), uno::UNO_QUERY_THROW );

    static rtl::OUString FunctionName( "IsError" );
    uno::Sequence< uno::Any > Params(1);
    uno::Reference< excel::XRange > aRange( this );
    Params[0] = uno::makeAny( aRange );
    uno::Sequence< sal_Int16 > OutParamIndex;
    uno::Sequence< uno::Any > OutParam;
    xInvoc->invoke( FunctionName, Params, OutParamIndex, OutParam ) >>= dResult;
    if ( dResult > 0.0 )
         return sal_True;
    return false;
}

namespace range
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaRange, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SvVbaRange",
    "ooo.vba.excel.Range" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
