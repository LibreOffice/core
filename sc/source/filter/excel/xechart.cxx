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


#include "xechart.hxx"

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XColorScheme.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>

#include <vcl/outdev.hxx>
#include <filter/msfilter/escherex.hxx>

#include "document.hxx"
#include "rangelst.hxx"
#include "rangeutl.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"
#include "token.hxx"
#include "xeescher.hxx"
#include "xeformula.hxx"
#include "xehelper.hxx"
#include "xepage.hxx"
#include "xestyle.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::i18n::XBreakIterator;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;

using ::com::sun::star::chart2::IncrementData;
using ::com::sun::star::chart2::RelativePosition;
using ::com::sun::star::chart2::RelativeSize;
using ::com::sun::star::chart2::ScaleData;
using ::com::sun::star::chart2::SubIncrement;
using ::com::sun::star::chart2::XAxis;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::XChartTypeContainer;
using ::com::sun::star::chart2::XColorScheme;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XCoordinateSystemContainer;
using ::com::sun::star::chart2::XChartType;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XDataSeriesContainer;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XFormattedString;
using ::com::sun::star::chart2::XLegend;
using ::com::sun::star::chart2::XRegressionCurve;
using ::com::sun::star::chart2::XRegressionCurveContainer;
using ::com::sun::star::chart2::XScaling;
using ::com::sun::star::chart2::XTitle;
using ::com::sun::star::chart2::XTitled;

using ::com::sun::star::chart2::data::XDataSequence;
using ::com::sun::star::chart2::data::XDataSource;
using ::com::sun::star::chart2::data::XLabeledDataSequence;

using ::formula::FormulaGrammar;
using ::formula::FormulaToken;

namespace cssc = ::com::sun::star::chart;
namespace cssc2 = ::com::sun::star::chart2;

// Helpers ====================================================================

namespace {

XclExpStream& operator<<( XclExpStream& rStrm, const XclChRectangle& rRect )
{
    return rStrm << rRect.mnX << rRect.mnY << rRect.mnWidth << rRect.mnHeight;
}

inline void lclSaveRecord( XclExpStream& rStrm, XclExpRecordRef xRec )
{
    if( xRec )
        xRec->Save( rStrm );
}

/** Saves the passed record (group) together with a leading value record. */
template< typename Type >
void lclSaveRecord( XclExpStream& rStrm, XclExpRecordRef xRec, sal_uInt16 nRecId, Type nValue )
{
    if( xRec )
    {
        XclExpValueRecord< Type >( nRecId, nValue ).Save( rStrm );
        xRec->Save( rStrm );
    }
}

template<typename ValueType, typename KeyType>
void lclSaveRecord(XclExpStream& rStrm, ValueType* pRec, sal_uInt16 nRecId, KeyType nValue)
{
    if (pRec)
    {
        XclExpValueRecord<KeyType>(nRecId, nValue).Save(rStrm);
        pRec->Save(rStrm);
    }
}

void lclWriteChFrBlockRecord( XclExpStream& rStrm, const XclChFrBlock& rFrBlock, bool bBegin )
{
    sal_uInt16 nRecId = bBegin ? EXC_ID_CHFRBLOCKBEGIN : EXC_ID_CHFRBLOCKEND;
    rStrm.StartRecord( nRecId, 12 );
    rStrm << nRecId << EXC_FUTUREREC_EMPTYFLAGS << rFrBlock.mnType << rFrBlock.mnContext << rFrBlock.mnValue1 << rFrBlock.mnValue2;
    rStrm.EndRecord();
}

template< typename Type >
inline bool lclIsAutoAnyOrGetValue( Type& rValue, const Any& rAny )
{
    return !rAny.hasValue() || !(rAny >>= rValue);
}

bool lclIsAutoAnyOrGetScaledValue( double& rfValue, const Any& rAny, bool bLogScale )
{
    bool bIsAuto = lclIsAutoAnyOrGetValue( rfValue, rAny );
    if( !bIsAuto && bLogScale )
        rfValue = log( rfValue ) / log( 10.0 );
    return bIsAuto;
}

sal_uInt16 lclGetTimeValue( const XclExpRoot& rRoot, double fSerialDate, sal_uInt16 nTimeUnit )
{
    DateTime aDateTime = rRoot.GetDateTimeFromDouble( fSerialDate );
    switch( nTimeUnit )
    {
        case EXC_CHDATERANGE_DAYS:
            return ::limit_cast< sal_uInt16, double >( fSerialDate, 0, SAL_MAX_UINT16 );
        case EXC_CHDATERANGE_MONTHS:
            return ::limit_cast< sal_uInt16, sal_uInt16 >( 12 * (aDateTime.GetYear() - rRoot.GetBaseYear()) + aDateTime.GetMonth() - 1, 0, SAL_MAX_INT16 );
        case EXC_CHDATERANGE_YEARS:
            return ::limit_cast< sal_uInt16, sal_uInt16 >( aDateTime.GetYear() - rRoot.GetBaseYear(), 0, SAL_MAX_INT16 );
        default:
            OSL_ENSURE( false, "lclGetTimeValue - unexpected time unit" );
    }
    return ::limit_cast< sal_uInt16, double >( fSerialDate, 0, SAL_MAX_UINT16 );
}

bool lclConvertTimeValue( const XclExpRoot& rRoot, sal_uInt16& rnValue, const Any& rAny, sal_uInt16 nTimeUnit )
{
    double fSerialDate = 0;
    bool bAuto = lclIsAutoAnyOrGetValue( fSerialDate, rAny );
    if( !bAuto )
        rnValue = lclGetTimeValue( rRoot, fSerialDate, nTimeUnit );
    return bAuto;
}

sal_uInt16 lclGetTimeUnit( sal_Int32 nApiTimeUnit )
{
    switch( nApiTimeUnit )
    {
        case cssc::TimeUnit::DAY:   return EXC_CHDATERANGE_DAYS;
        case cssc::TimeUnit::MONTH: return EXC_CHDATERANGE_MONTHS;
        case cssc::TimeUnit::YEAR:  return EXC_CHDATERANGE_YEARS;
        default:                    OSL_ENSURE( false, "lclGetTimeUnit - unexpected time unit" );
    }
    return EXC_CHDATERANGE_DAYS;
}

bool lclConvertTimeInterval( sal_uInt16& rnValue, sal_uInt16& rnTimeUnit, const Any& rAny )
{
    cssc::TimeInterval aInterval;
    bool bAuto = lclIsAutoAnyOrGetValue( aInterval, rAny );
    if( !bAuto )
    {
        rnValue = ::limit_cast< sal_uInt16, sal_Int32 >( aInterval.Number, 1, SAL_MAX_UINT16 );
        rnTimeUnit = lclGetTimeUnit( aInterval.TimeUnit );
    }
    return bAuto;
}

} // namespace

// Common =====================================================================

/** Stores global data needed in various classes of the Chart export filter. */
struct XclExpChRootData : public XclChRootData
{
    typedef ::std::vector< XclChFrBlock > XclChFrBlockVector;

    XclExpChChart&      mrChartData;            /// The chart data object.
    XclChFrBlockVector  maWrittenFrBlocks;      /// Stack of future record levels already written out.
    XclChFrBlockVector  maUnwrittenFrBlocks;    /// Stack of future record levels not yet written out.

    inline explicit     XclExpChRootData( XclExpChChart& rChartData ) : mrChartData( rChartData ) {}

    /** Registers a new future record level. */
    void                RegisterFutureRecBlock( const XclChFrBlock& rFrBlock );
    /** Initializes the current future record level (writes all unwritten CHFRBLOCKBEGIN records). */
    void                InitializeFutureRecBlock( XclExpStream& rStrm );
    /** Finalizes the current future record level (writes CHFRBLOCKEND record if needed). */
    void                FinalizeFutureRecBlock( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

void XclExpChRootData::RegisterFutureRecBlock( const XclChFrBlock& rFrBlock )
{
    maUnwrittenFrBlocks.push_back( rFrBlock );
}

void XclExpChRootData::InitializeFutureRecBlock( XclExpStream& rStrm )
{
    // first call from a future record writes all missing CHFRBLOCKBEGIN records
    if( !maUnwrittenFrBlocks.empty() )
    {
        // write the leading CHFRINFO record
        if( maWrittenFrBlocks.empty() )
        {
            rStrm.StartRecord( EXC_ID_CHFRINFO, 20 );
            rStrm << EXC_ID_CHFRINFO << EXC_FUTUREREC_EMPTYFLAGS << EXC_CHFRINFO_EXCELXP2003 << EXC_CHFRINFO_EXCELXP2003 << sal_uInt16( 3 );
            rStrm << sal_uInt16( 0x0850 ) << sal_uInt16( 0x085A ) << sal_uInt16( 0x0861 ) << sal_uInt16( 0x0861 ) << sal_uInt16( 0x086A ) << sal_uInt16( 0x086B );
            rStrm.EndRecord();
        }
        // write all unwritten CHFRBLOCKBEGIN records
        for( XclChFrBlockVector::const_iterator aIt = maUnwrittenFrBlocks.begin(), aEnd = maUnwrittenFrBlocks.end(); aIt != aEnd; ++aIt )
        {
            OSL_ENSURE( aIt->mnType != EXC_CHFRBLOCK_TYPE_UNKNOWN, "XclExpChRootData::InitializeFutureRecBlock - unknown future record block type" );
            lclWriteChFrBlockRecord( rStrm, *aIt, true );
        }
        // move all record infos to vector of written blocks
        maWrittenFrBlocks.insert( maWrittenFrBlocks.end(), maUnwrittenFrBlocks.begin(), maUnwrittenFrBlocks.end() );
        maUnwrittenFrBlocks.clear();
    }
}

void XclExpChRootData::FinalizeFutureRecBlock( XclExpStream& rStrm )
{
    OSL_ENSURE( !maUnwrittenFrBlocks.empty() || !maWrittenFrBlocks.empty(), "XclExpChRootData::FinalizeFutureRecBlock - no future record level found" );
    if( !maUnwrittenFrBlocks.empty() )
    {
        // no future record has been written, just forget the topmost level
        maUnwrittenFrBlocks.pop_back();
    }
    else if( !maWrittenFrBlocks.empty() )
    {
        // write the CHFRBLOCKEND record for the topmost block and delete it
        lclWriteChFrBlockRecord( rStrm, maWrittenFrBlocks.back(), false );
        maWrittenFrBlocks.pop_back();
    }
}

// ----------------------------------------------------------------------------

XclExpChRoot::XclExpChRoot( const XclExpRoot& rRoot, XclExpChChart& rChartData ) :
    XclExpRoot( rRoot ),
    mxChData( new XclExpChRootData( rChartData ) )
{
}

XclExpChRoot::~XclExpChRoot()
{
}

Reference< XChartDocument > XclExpChRoot::GetChartDocument() const
{
    return mxChData->mxChartDoc;
}

XclExpChChart& XclExpChRoot::GetChartData() const
{
    return mxChData->mrChartData;
}

const XclChTypeInfo& XclExpChRoot::GetChartTypeInfo( XclChTypeId eType ) const
{
    return mxChData->mxTypeInfoProv->GetTypeInfo( eType );
}

const XclChTypeInfo& XclExpChRoot::GetChartTypeInfo( const OUString& rServiceName ) const
{
    return mxChData->mxTypeInfoProv->GetTypeInfoFromService( rServiceName );
}

const XclChFormatInfo& XclExpChRoot::GetFormatInfo( XclChObjectType eObjType ) const
{
    return mxChData->mxFmtInfoProv->GetFormatInfo( eObjType );
}

void XclExpChRoot::InitConversion( XChartDocRef xChartDoc, const Rectangle& rChartRect ) const
{
    mxChData->InitConversion( GetRoot(), xChartDoc, rChartRect );
}

void XclExpChRoot::FinishConversion() const
{
    mxChData->FinishConversion();
}

bool XclExpChRoot::IsSystemColor( const Color& rColor, sal_uInt16 nSysColorIdx ) const
{
    XclExpPalette& rPal = GetPalette();
    return rPal.IsSystemColor( nSysColorIdx ) && (rColor == rPal.GetDefColor( nSysColorIdx ));
}

void XclExpChRoot::SetSystemColor( Color& rColor, sal_uInt32& rnColorId, sal_uInt16 nSysColorIdx ) const
{
    OSL_ENSURE( GetPalette().IsSystemColor( nSysColorIdx ), "XclExpChRoot::SetSystemColor - invalid color index" );
    rColor = GetPalette().GetDefColor( nSysColorIdx );
    rnColorId = XclExpPalette::GetColorIdFromIndex( nSysColorIdx );
}

sal_Int32 XclExpChRoot::CalcChartXFromHmm( sal_Int32 nPosX ) const
{
    return ::limit_cast< sal_Int32, double >( (nPosX - mxChData->mnBorderGapX) / mxChData->mfUnitSizeX, 0, EXC_CHART_TOTALUNITS );
}

sal_Int32 XclExpChRoot::CalcChartYFromHmm( sal_Int32 nPosY ) const
{
    return ::limit_cast< sal_Int32, double >( (nPosY - mxChData->mnBorderGapY) / mxChData->mfUnitSizeY, 0, EXC_CHART_TOTALUNITS );
}

XclChRectangle XclExpChRoot::CalcChartRectFromHmm( const ::com::sun::star::awt::Rectangle& rRect ) const
{
    XclChRectangle aRect;
    aRect.mnX = CalcChartXFromHmm( rRect.X );
    aRect.mnY = CalcChartYFromHmm( rRect.Y );
    aRect.mnWidth = CalcChartXFromHmm( rRect.Width );
    aRect.mnHeight = CalcChartYFromHmm( rRect.Height );
    return aRect;
}

void XclExpChRoot::ConvertLineFormat( XclChLineFormat& rLineFmt,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().ReadLineProperties(
        rLineFmt, *mxChData->mxLineDashTable, rPropSet, ePropMode );
}

bool XclExpChRoot::ConvertAreaFormat( XclChAreaFormat& rAreaFmt,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode ) const
{
    return GetChartPropSetHelper().ReadAreaProperties( rAreaFmt, rPropSet, ePropMode );
}

void XclExpChRoot::ConvertEscherFormat(
        XclChEscherFormat& rEscherFmt, XclChPicFormat& rPicFmt,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().ReadEscherProperties( rEscherFmt, rPicFmt,
        *mxChData->mxGradientTable, *mxChData->mxHatchTable, *mxChData->mxBitmapTable, rPropSet, ePropMode );
}

sal_uInt16 XclExpChRoot::ConvertFont( const ScfPropertySet& rPropSet, sal_Int16 nScript ) const
{
    XclFontData aFontData;
    GetFontPropSetHelper().ReadFontProperties( aFontData, rPropSet, EXC_FONTPROPSET_CHART, nScript );
    return GetFontBuffer().Insert( aFontData, EXC_COLOR_CHARTTEXT );
}

sal_uInt16 XclExpChRoot::ConvertPieRotation( const ScfPropertySet& rPropSet )
{
    sal_Int32 nApiRot = 0;
    rPropSet.GetProperty( nApiRot, EXC_CHPROP_STARTINGANGLE );
    return static_cast< sal_uInt16 >( (450 - (nApiRot % 360)) % 360 );
}

void XclExpChRoot::RegisterFutureRecBlock( const XclChFrBlock& rFrBlock )
{
    mxChData->RegisterFutureRecBlock( rFrBlock );
}

void XclExpChRoot::InitializeFutureRecBlock( XclExpStream& rStrm )
{
    mxChData->InitializeFutureRecBlock( rStrm );
}

void XclExpChRoot::FinalizeFutureRecBlock( XclExpStream& rStrm )
{
    mxChData->FinalizeFutureRecBlock( rStrm );
}

// ----------------------------------------------------------------------------

XclExpChGroupBase::XclExpChGroupBase( const XclExpChRoot& rRoot,
        sal_uInt16 nFrType, sal_uInt16 nRecId, sal_Size nRecSize ) :
    XclExpRecord( nRecId, nRecSize ),
    XclExpChRoot( rRoot ),
    maFrBlock( nFrType )
{
}

XclExpChGroupBase::~XclExpChGroupBase()
{
}

void XclExpChGroupBase::Save( XclExpStream& rStrm )
{
    // header record
    XclExpRecord::Save( rStrm );
    // group records
    if( HasSubRecords() )
    {
        // register the future record context corresponding to this record group
        RegisterFutureRecBlock( maFrBlock );
        // CHBEGIN record
        XclExpEmptyRecord( EXC_ID_CHBEGIN ).Save( rStrm );
        // embedded records
        WriteSubRecords( rStrm );
        // finalize the future records, must be done before the closing CHEND
        FinalizeFutureRecBlock( rStrm );
        // CHEND record
        XclExpEmptyRecord( EXC_ID_CHEND ).Save( rStrm );
    }
}

bool XclExpChGroupBase::HasSubRecords() const
{
    return true;
}

void XclExpChGroupBase::SetFutureRecordContext( sal_uInt16 nFrContext, sal_uInt16 nFrValue1, sal_uInt16 nFrValue2 )
{
    maFrBlock.mnContext = nFrContext;
    maFrBlock.mnValue1  = nFrValue1;
    maFrBlock.mnValue2  = nFrValue2;
}

// ----------------------------------------------------------------------------

XclExpChFutureRecordBase::XclExpChFutureRecordBase( const XclExpChRoot& rRoot,
        XclFutureRecType eRecType, sal_uInt16 nRecId, sal_Size nRecSize ) :
    XclExpFutureRecord( eRecType, nRecId, nRecSize ),
    XclExpChRoot( rRoot )
{
}

void XclExpChFutureRecordBase::Save( XclExpStream& rStrm )
{
    InitializeFutureRecBlock( rStrm );
    XclExpFutureRecord::Save( rStrm );
}

// Frame formatting ===========================================================

XclExpChFramePos::XclExpChFramePos( sal_uInt16 nTLMode, sal_uInt16 nBRMode ) :
    XclExpRecord( EXC_ID_CHFRAMEPOS, 20 )
{
    maData.mnTLMode = nTLMode;
    maData.mnBRMode = nBRMode;
}

void XclExpChFramePos::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnTLMode << maData.mnBRMode << maData.maRect;
}

// ----------------------------------------------------------------------------

XclExpChLineFormat::XclExpChLineFormat( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHLINEFORMAT, (rRoot.GetBiff() == EXC_BIFF8) ? 12 : 10 ),
    mnColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) )
{
}

void XclExpChLineFormat::SetDefault( XclChFrameType eDefFrameType )
{
    switch( eDefFrameType )
    {
        case EXC_CHFRAMETYPE_AUTO:
            SetAuto( true );
        break;
        case EXC_CHFRAMETYPE_INVISIBLE:
            SetAuto( false );
            maData.mnPattern = EXC_CHLINEFORMAT_NONE;
        break;
        default:
            OSL_FAIL( "XclExpChLineFormat::SetDefault - unknown frame type" );
    }
}

void XclExpChLineFormat::Convert( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    rRoot.ConvertLineFormat( maData, rPropSet, rFmtInfo.mePropMode );
    if( HasLine() )
    {
        // detect system color, set color identifier (TODO: detect automatic series line)
        if( (eObjType != EXC_CHOBJTYPE_LINEARSERIES) && rRoot.IsSystemColor( maData.maColor, rFmtInfo.mnAutoLineColorIdx ) )
        {
            // store color index from automatic format data
            mnColorId = XclExpPalette::GetColorIdFromIndex( rFmtInfo.mnAutoLineColorIdx );
            // try to set automatic mode
            bool bAuto = (maData.mnPattern == EXC_CHLINEFORMAT_SOLID) && (maData.mnWeight == rFmtInfo.mnAutoLineWeight);
            ::set_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO, bAuto );
        }
        else
        {
            // user defined color - register in palette
            mnColorId = rRoot.GetPalette().InsertColor( maData.maColor, EXC_COLOR_CHARTLINE );
        }
    }
    else
    {
        // no line - set default system color
        rRoot.SetSystemColor( maData.maColor, mnColorId, EXC_COLOR_CHWINDOWTEXT );
    }
}

bool XclExpChLineFormat::IsDefault( XclChFrameType eDefFrameType ) const
{
    return
        ((eDefFrameType == EXC_CHFRAMETYPE_INVISIBLE) && !HasLine()) ||
        ((eDefFrameType == EXC_CHFRAMETYPE_AUTO) && IsAuto());
}

void XclExpChLineFormat::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.maColor << maData.mnPattern << maData.mnWeight << maData.mnFlags;
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
        rStrm << rStrm.GetRoot().GetPalette().GetColorIndex( mnColorId );
}

namespace {

/** Creates a CHLINEFORMAT record from the passed property set. */
XclExpChLineFormatRef lclCreateLineFormat( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    XclExpChLineFormatRef xLineFmt( new XclExpChLineFormat( rRoot ) );
    xLineFmt->Convert( rRoot, rPropSet, eObjType );
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    if( rFmtInfo.mbDeleteDefFrame && xLineFmt->IsDefault( rFmtInfo.meDefFrameType ) )
        xLineFmt.reset();
    return xLineFmt;
}

} // namespace

// ----------------------------------------------------------------------------

XclExpChAreaFormat::XclExpChAreaFormat( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHAREAFORMAT, (rRoot.GetBiff() == EXC_BIFF8) ? 16 : 12 ),
    mnPattColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) ),
    mnBackColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) )
{
}

bool XclExpChAreaFormat::Convert( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    bool bComplexFill = rRoot.ConvertAreaFormat( maData, rPropSet, rFmtInfo.mePropMode );
    if( HasArea() )
    {
        bool bSolid = maData.mnPattern == EXC_PATT_SOLID;
        // detect system color, set color identifier (TODO: detect automatic series area)
        if( (eObjType != EXC_CHOBJTYPE_FILLEDSERIES) && rRoot.IsSystemColor( maData.maPattColor, rFmtInfo.mnAutoPattColorIdx ) )
        {
            // store color index from automatic format data
            mnPattColorId = XclExpPalette::GetColorIdFromIndex( rFmtInfo.mnAutoPattColorIdx );
            // set automatic mode
            ::set_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO, bSolid );
        }
        else
        {
            // user defined color - register color in palette
            mnPattColorId = rRoot.GetPalette().InsertColor( maData.maPattColor, EXC_COLOR_CHARTAREA );
        }
        // background color (default system color for solid fills)
        if( bSolid )
            rRoot.SetSystemColor( maData.maBackColor, mnBackColorId, EXC_COLOR_CHWINDOWTEXT );
        else
            mnBackColorId = rRoot.GetPalette().InsertColor( maData.maBackColor, EXC_COLOR_CHARTAREA );
    }
    else
    {
        // no area - set default system colors
        rRoot.SetSystemColor( maData.maPattColor, mnPattColorId, EXC_COLOR_CHWINDOWBACK );
        rRoot.SetSystemColor( maData.maBackColor, mnBackColorId, EXC_COLOR_CHWINDOWTEXT );
    }
    return bComplexFill;
}

void XclExpChAreaFormat::SetDefault( XclChFrameType eDefFrameType )
{
    switch( eDefFrameType )
    {
        case EXC_CHFRAMETYPE_AUTO:
            SetAuto( true );
        break;
        case EXC_CHFRAMETYPE_INVISIBLE:
            SetAuto( false );
            maData.mnPattern = EXC_PATT_NONE;
        break;
        default:
            OSL_FAIL( "XclExpChAreaFormat::SetDefault - unknown frame type" );
    }
}

bool XclExpChAreaFormat::IsDefault( XclChFrameType eDefFrameType ) const
{
    return
        ((eDefFrameType == EXC_CHFRAMETYPE_INVISIBLE) && !HasArea()) ||
        ((eDefFrameType == EXC_CHFRAMETYPE_AUTO) && IsAuto());
}

void XclExpChAreaFormat::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.maPattColor << maData.maBackColor << maData.mnPattern << maData.mnFlags;
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
    {
        const XclExpPalette& rPal = rStrm.GetRoot().GetPalette();
        rStrm << rPal.GetColorIndex( mnPattColorId ) << rPal.GetColorIndex( mnBackColorId );
    }
}

// ----------------------------------------------------------------------------

XclExpChEscherFormat::XclExpChEscherFormat( const XclExpChRoot& rRoot ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_UNKNOWN, EXC_ID_CHESCHERFORMAT ),
    mnColor1Id( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) ),
    mnColor2Id( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) )
{
    OSL_ENSURE_BIFF( GetBiff() == EXC_BIFF8 );
}

void XclExpChEscherFormat::Convert( const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    const XclChFormatInfo& rFmtInfo = GetFormatInfo( eObjType );
    ConvertEscherFormat( maData, maPicFmt, rPropSet, rFmtInfo.mePropMode );
    // register colors in palette
    mnColor1Id = RegisterColor( ESCHER_Prop_fillColor );
    mnColor2Id = RegisterColor( ESCHER_Prop_fillBackColor );
}

bool XclExpChEscherFormat::IsValid() const
{
    return maData.mxEscherSet;
}

void XclExpChEscherFormat::Save( XclExpStream& rStrm )
{
    if( maData.mxEscherSet )
    {
        // replace RGB colors with palette indexes in the Escher container
        const XclExpPalette& rPal = GetPalette();
        maData.mxEscherSet->AddOpt( ESCHER_Prop_fillColor, 0x08000000 | rPal.GetColorIndex( mnColor1Id ) );
        maData.mxEscherSet->AddOpt( ESCHER_Prop_fillBackColor, 0x08000000 | rPal.GetColorIndex( mnColor2Id ) );

        // save the record group
        XclExpChGroupBase::Save( rStrm );
    }
}

bool XclExpChEscherFormat::HasSubRecords() const
{
    // no subrecords for gradients
    return maPicFmt.mnBmpMode != EXC_CHPICFORMAT_NONE;
}

void XclExpChEscherFormat::WriteSubRecords( XclExpStream& rStrm )
{
    rStrm.StartRecord( EXC_ID_CHPICFORMAT, 14 );
    rStrm << maPicFmt.mnBmpMode << sal_uInt16( 0 ) << maPicFmt.mnFlags << maPicFmt.mfScale;
    rStrm.EndRecord();
}

sal_uInt32 XclExpChEscherFormat::RegisterColor( sal_uInt16 nPropId )
{
    sal_uInt32 nBGRValue;
    if( maData.mxEscherSet && maData.mxEscherSet->GetOpt( nPropId, nBGRValue ) )
    {
        // swap red and blue
        Color aColor( RGB_COLORDATA(
            COLORDATA_BLUE( nBGRValue ),
            COLORDATA_GREEN( nBGRValue ),
            COLORDATA_RED( nBGRValue ) ) );
        return GetPalette().InsertColor( aColor, EXC_COLOR_CHARTAREA );
    }
    return XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK );
}

void XclExpChEscherFormat::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE( maData.mxEscherSet, "XclExpChEscherFormat::WriteBody - missing property container" );
    // write Escher property container via temporary memory stream
    SvMemoryStream aMemStrm;
    maData.mxEscherSet->Commit( aMemStrm );
    aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aMemStrm );
}

// ----------------------------------------------------------------------------

XclExpChFrameBase::XclExpChFrameBase()
{
}

XclExpChFrameBase::~XclExpChFrameBase()
{
}

void XclExpChFrameBase::ConvertFrameBase( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    // line format
    mxLineFmt.reset( new XclExpChLineFormat( rRoot ) );
    mxLineFmt->Convert( rRoot, rPropSet, eObjType );
    // area format (only for frame objects)
    if( rRoot.GetFormatInfo( eObjType ).mbIsFrame )
    {
        mxAreaFmt.reset( new XclExpChAreaFormat( rRoot ) );
        bool bComplexFill = mxAreaFmt->Convert( rRoot, rPropSet, eObjType );
        if( (rRoot.GetBiff() == EXC_BIFF8) && bComplexFill )
        {
            mxEscherFmt.reset( new XclExpChEscherFormat( rRoot ) );
            mxEscherFmt->Convert( rPropSet, eObjType );
            if( mxEscherFmt->IsValid() )
                mxAreaFmt->SetAuto( false );
            else
                mxEscherFmt.reset();
        }
    }
}

void XclExpChFrameBase::SetDefaultFrameBase( const XclExpChRoot& rRoot,
        XclChFrameType eDefFrameType, bool bIsFrame )
{
    // line format
    mxLineFmt.reset( new XclExpChLineFormat( rRoot ) );
    mxLineFmt->SetDefault( eDefFrameType );
    // area format (only for frame objects)
    if( bIsFrame )
    {
        mxAreaFmt.reset( new XclExpChAreaFormat( rRoot ) );
        mxAreaFmt->SetDefault( eDefFrameType );
        mxEscherFmt.reset();
    }
}

bool XclExpChFrameBase::IsDefaultFrameBase( XclChFrameType eDefFrameType ) const
{
    return
        (!mxLineFmt || mxLineFmt->IsDefault( eDefFrameType )) &&
        (!mxAreaFmt || mxAreaFmt->IsDefault( eDefFrameType ));
}

void XclExpChFrameBase::WriteFrameRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxLineFmt );
    lclSaveRecord( rStrm, mxAreaFmt );
    lclSaveRecord( rStrm, mxEscherFmt );
}

// ----------------------------------------------------------------------------

XclExpChFrame::XclExpChFrame( const XclExpChRoot& rRoot, XclChObjectType eObjType ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_FRAME, EXC_ID_CHFRAME, 4 ),
    meObjType( eObjType )
{
}

void XclExpChFrame::Convert( const ScfPropertySet& rPropSet )
{
    ConvertFrameBase( GetChRoot(), rPropSet, meObjType );
}

void XclExpChFrame::SetAutoFlags( bool bAutoPos, bool bAutoSize )
{
    ::set_flag( maData.mnFlags, EXC_CHFRAME_AUTOPOS, bAutoPos );
    ::set_flag( maData.mnFlags, EXC_CHFRAME_AUTOSIZE, bAutoSize );
}

bool XclExpChFrame::IsDefault() const
{
    return IsDefaultFrameBase( GetFormatInfo( meObjType ).meDefFrameType );
}

bool XclExpChFrame::IsDeleteable() const
{
    return IsDefault() && GetFormatInfo( meObjType ).mbDeleteDefFrame;
}

void XclExpChFrame::Save( XclExpStream& rStrm )
{
    switch( meObjType )
    {
        // wall/floor frame without CHFRAME header record
        case EXC_CHOBJTYPE_WALL3D:
        case EXC_CHOBJTYPE_FLOOR3D:
            WriteFrameRecords( rStrm );
        break;
        default:
            XclExpChGroupBase::Save( rStrm );
    }
}

void XclExpChFrame::WriteSubRecords( XclExpStream& rStrm )
{
    WriteFrameRecords( rStrm );
}

void XclExpChFrame::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnFormat << maData.mnFlags;
}

namespace {

/** Creates a CHFRAME record from the passed property set. */
XclExpChFrameRef lclCreateFrame( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    XclExpChFrameRef xFrame( new XclExpChFrame( rRoot, eObjType ) );
    xFrame->Convert( rPropSet );
    if( xFrame->IsDeleteable() )
        xFrame.reset();
    return xFrame;
}

} // namespace

// Source links ===============================================================

namespace {

void lclAddDoubleRefData(
        ScTokenArray& orArray, const FormulaToken& rToken,
        SCsTAB nScTab1, SCsCOL nScCol1, SCsROW nScRow1,
        SCsTAB nScTab2, SCsCOL nScCol2, SCsROW nScRow2 )
{
    ScComplexRefData aComplexRef;
    aComplexRef.InitFlags();
    aComplexRef.Ref1.SetFlag3D( true );
    aComplexRef.Ref1.nTab = nScTab1;
    aComplexRef.Ref1.nCol = nScCol1;
    aComplexRef.Ref1.nRow = nScRow1;
    aComplexRef.Ref2.nTab = nScTab2;
    aComplexRef.Ref2.nCol = nScCol2;
    aComplexRef.Ref2.nRow = nScRow2;

    if( orArray.GetLen() > 0 )
        orArray.AddOpCode( ocUnion );

    OSL_ENSURE( (rToken.GetType() == ::formula::svDoubleRef) || (rToken.GetType() == ::formula::svExternalDoubleRef),
        "lclAddDoubleRefData - double reference token expected");
    if( rToken.GetType() == ::formula::svExternalDoubleRef )
        orArray.AddExternalDoubleReference( rToken.GetIndex(), rToken.GetString(), aComplexRef );
    else
        orArray.AddDoubleReference( aComplexRef );
}

} // namespace

// ----------------------------------------------------------------------------

XclExpChSourceLink::XclExpChSourceLink( const XclExpChRoot& rRoot, sal_uInt8 nDestType ) :
    XclExpRecord( EXC_ID_CHSOURCELINK ),
    XclExpChRoot( rRoot )
{
    maData.mnDestType = nDestType;
    maData.mnLinkType = EXC_CHSRCLINK_DIRECTLY;
}

sal_uInt16 XclExpChSourceLink::ConvertDataSequence( Reference< XDataSequence > xDataSeq, bool bSplitToColumns, sal_uInt16 nDefCount )
{
    mxLinkFmla.reset();
    maData.mnLinkType = EXC_CHSRCLINK_DEFAULT;

    if( !xDataSeq.is() )
        return nDefCount;

    // Compile the range representation string into token array.  Note that the
    // source range text depends on the current grammar.
    OUString aRangeRepr = xDataSeq->getSourceRangeRepresentation();
    ScCompiler aComp( GetDocPtr(), ScAddress() );
    aComp.SetGrammar( GetDocPtr()->GetGrammar() );
    ScTokenArray* pArray = aComp.CompileString( aRangeRepr );
    if( !pArray )
        return nDefCount;

    ScTokenArray aArray;
    sal_uInt32 nValueCount = 0;
    pArray->Reset();
    for( const FormulaToken* pToken = pArray->First(); pToken; pToken = pArray->Next() )
    {
        switch( pToken->GetType() )
        {
            case ::formula::svSingleRef:
            case ::formula::svExternalSingleRef:
                // for a single ref token, just add it to the new token array as is
                if( aArray.GetLen() > 0 )
                    aArray.AddOpCode( ocUnion );
                aArray.AddToken( *pToken );
                ++nValueCount;
            break;

            case ::formula::svDoubleRef:
            case ::formula::svExternalDoubleRef:
            {
                // split 3-dimensional ranges into single sheets
                const ScComplexRefData& rComplexRef = static_cast< const ScToken* >( pToken )->GetDoubleRef();
                const ScSingleRefData& rRef1 = rComplexRef.Ref1;
                const ScSingleRefData& rRef2 = rComplexRef.Ref2;
                for( SCsTAB nScTab = rRef1.nTab; nScTab <= rRef2.nTab; ++nScTab )
                {
                    // split 2-dimensional ranges into single columns
                    if( bSplitToColumns && (rRef1.nCol < rRef2.nCol) && (rRef1.nRow < rRef2.nRow) )
                        for( SCsCOL nScCol = rRef1.nCol; nScCol <= rRef2.nCol; ++nScCol )
                            lclAddDoubleRefData( aArray, *pToken, nScTab, nScCol, rRef1.nRow, nScTab, nScCol, rRef2.nRow );
                    else
                        lclAddDoubleRefData( aArray, *pToken, nScTab, rRef1.nCol, rRef1.nRow, nScTab, rRef2.nCol, rRef2.nRow );
                }
                sal_uInt32 nTabs = static_cast< sal_uInt32 >( rRef2.nTab - rRef1.nTab + 1 );
                sal_uInt32 nCols = static_cast< sal_uInt32 >( rRef2.nCol - rRef1.nCol + 1 );
                sal_uInt32 nRows = static_cast< sal_uInt32 >( rRef2.nRow - rRef1.nRow + 1 );
                nValueCount += nCols * nRows * nTabs;
            }
            break;

            default:;
        }
    }

    const ScAddress aBaseCell;
    mxLinkFmla = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CHART, aArray, &aBaseCell );
    maData.mnLinkType = EXC_CHSRCLINK_WORKSHEET;
    return ulimit_cast< sal_uInt16 >( nValueCount, EXC_CHDATAFORMAT_MAXPOINTCOUNT );
}

sal_uInt16 XclExpChSourceLink::ConvertStringSequence( const Sequence< Reference< XFormattedString > >& rStringSeq )
{
    mxString.reset();
    sal_uInt16 nFontIdx = EXC_FONT_APP;
    if( rStringSeq.hasElements() )
    {
        mxString = XclExpStringHelper::CreateString( GetRoot(), String::EmptyString(), EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH | EXC_STR_SEPARATEFORMATS );
        Reference< XBreakIterator > xBreakIt = GetDoc().GetBreakIterator();
        namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;

        // convert all formatted string entries from the sequence
        const Reference< XFormattedString >* pBeg = rStringSeq.getConstArray();
        const Reference< XFormattedString >* pEnd = pBeg + rStringSeq.getLength();
        for( const Reference< XFormattedString >* pIt = pBeg; pIt != pEnd; ++pIt )
        {
            if( pIt->is() )
            {
                sal_uInt16 nWstrnFontIdx = EXC_FONT_NOTFOUND;
                sal_uInt16 nAsianFontIdx = EXC_FONT_NOTFOUND;
                sal_uInt16 nCmplxFontIdx = EXC_FONT_NOTFOUND;
                OUString aText = (*pIt)->getString();
                ScfPropertySet aStrProp( *pIt );

                // #i63255# get script type for leading weak characters
                sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( GetRoot(), aText );

                // process all script portions
                sal_Int32 nPortionPos = 0;
                sal_Int32 nTextLen = aText.getLength();
                while( nPortionPos < nTextLen )
                {
                    // get script type and end position of next script portion
                    sal_Int16 nScript = xBreakIt->getScriptType( aText, nPortionPos );
                    sal_Int32 nPortionEnd = xBreakIt->endOfScript( aText, nPortionPos, nScript );

                    // reuse previous script for following weak portions
                    if( nScript == ApiScriptType::WEAK )
                        nScript = nLastScript;

                    // Excel start position of this portion
                    sal_uInt16 nXclPortionStart = mxString->Len();
                    // add portion text to Excel string
                    XclExpStringHelper::AppendString( *mxString, GetRoot(), aText.copy( nPortionPos, nPortionEnd - nPortionPos ) );
                    if( nXclPortionStart < mxString->Len() )
                    {
                        // find font index variable dependent on script type
                        sal_uInt16& rnFontIdx = (nScript == ApiScriptType::COMPLEX) ? nCmplxFontIdx :
                            ((nScript == ApiScriptType::ASIAN) ? nAsianFontIdx : nWstrnFontIdx);

                        // insert font into buffer (if not yet done)
                        if( rnFontIdx == EXC_FONT_NOTFOUND )
                            rnFontIdx = ConvertFont( aStrProp, nScript );

                        // insert font index into format run vector
                        mxString->AppendFormat( nXclPortionStart, rnFontIdx );
                    }

                    // go to next script portion
                    nLastScript = nScript;
                    nPortionPos = nPortionEnd;
                }
            }
        }
        if( !mxString->IsEmpty() )
        {
            // get leading font index
            const XclFormatRunVec& rFormats = mxString->GetFormats();
            OSL_ENSURE( !rFormats.empty() && (rFormats.front().mnChar == 0),
                "XclExpChSourceLink::ConvertStringSequenc - missing leading format" );
            // remove leading format run, if entire string is equally formatted
            if( rFormats.size() == 1 )
                nFontIdx = mxString->RemoveLeadingFont();
            else if( !rFormats.empty() )
                nFontIdx = rFormats.front().mnFontIdx;
            // add trailing format run, if string is rich-formatted
            if( mxString->IsRich() )
                mxString->AppendTrailingFormat( EXC_FONT_APP );
        }
    }
    return nFontIdx;
}

void XclExpChSourceLink::ConvertNumFmt( const ScfPropertySet& rPropSet, bool bPercent )
{
    sal_Int32 nApiNumFmt = 0;
    if( bPercent ? rPropSet.GetProperty( nApiNumFmt, EXC_CHPROP_PERCENTAGENUMFMT ) : rPropSet.GetProperty( nApiNumFmt, EXC_CHPROP_NUMBERFORMAT ) )
    {
        ::set_flag( maData.mnFlags, EXC_CHSRCLINK_NUMFMT );
        maData.mnNumFmtIdx = GetNumFmtBuffer().Insert( static_cast< sal_uInt32 >( nApiNumFmt ) );
    }
}

void XclExpChSourceLink::AppendString( const String& rStr )
{
    if (!mxString)
        return;
    XclExpStringHelper::AppendString( *mxString, GetRoot(), rStr );
}

void XclExpChSourceLink::Save( XclExpStream& rStrm )
{
    // CHFORMATRUNS record
    if( mxString && mxString->IsRich() )
    {
        sal_Size nRecSize = (1 + mxString->GetFormatsCount()) * ((GetBiff() == EXC_BIFF8) ? 2 : 1);
        rStrm.StartRecord( EXC_ID_CHFORMATRUNS, nRecSize );
        mxString->WriteFormats( rStrm, true );
        rStrm.EndRecord();
    }
    // CHSOURCELINK record
    XclExpRecord::Save( rStrm );
    // CHSTRING record
    if( mxString && !mxString->IsEmpty() )
    {
        rStrm.StartRecord( EXC_ID_CHSTRING, 2 + mxString->GetSize() );
        rStrm << sal_uInt16( 0 ) << *mxString;
        rStrm.EndRecord();
    }
}

void XclExpChSourceLink::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnDestType
            << maData.mnLinkType
            << maData.mnFlags
            << maData.mnNumFmtIdx
            << mxLinkFmla;
}

// Text =======================================================================

XclExpChFont::XclExpChFont( sal_uInt16 nFontIdx ) :
    XclExpUInt16Record( EXC_ID_CHFONT, nFontIdx )
{
}

// ----------------------------------------------------------------------------

XclExpChObjectLink::XclExpChObjectLink( sal_uInt16 nLinkTarget, const XclChDataPointPos& rPointPos ) :
    XclExpRecord( EXC_ID_CHOBJECTLINK, 6 )
{
    maData.mnTarget = nLinkTarget;
    maData.maPointPos = rPointPos;
}

void XclExpChObjectLink::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnTarget << maData.maPointPos.mnSeriesIdx << maData.maPointPos.mnPointIdx;
}

// ----------------------------------------------------------------------------

XclExpChFrLabelProps::XclExpChFrLabelProps( const XclExpChRoot& rRoot ) :
    XclExpChFutureRecordBase( rRoot, EXC_FUTUREREC_UNUSEDREF, EXC_ID_CHFRLABELPROPS, 4 )
{
}

void XclExpChFrLabelProps::Convert( const ScfPropertySet& rPropSet, bool bShowSeries,
        bool bShowCateg, bool bShowValue, bool bShowPercent, bool bShowBubble )
{
    // label value flags
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWSERIES,  bShowSeries );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWCATEG,   bShowCateg );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWVALUE,   bShowValue );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWPERCENT, bShowPercent );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWBUBBLE,  bShowBubble );

    // label value separator
    maData.maSeparator = rPropSet.GetStringProperty( EXC_CHPROP_LABELSEPARATOR );
    if( maData.maSeparator.isEmpty() )
        maData.maSeparator = rtl::OUString(' ');
}

void XclExpChFrLabelProps::WriteBody( XclExpStream& rStrm )
{
    XclExpString aXclSep( maData.maSeparator, EXC_STR_FORCEUNICODE | EXC_STR_SMARTFLAGS );
    rStrm << maData.mnFlags << aXclSep;
}

// ----------------------------------------------------------------------------

XclExpChFontBase::~XclExpChFontBase()
{
}

void XclExpChFontBase::ConvertFontBase( const XclExpChRoot& rRoot, sal_uInt16 nFontIdx )
{
    if( const XclExpFont* pFont = rRoot.GetFontBuffer().GetFont( nFontIdx ) )
    {
        XclExpChFontRef xFont( new XclExpChFont( nFontIdx ) );
        SetFont( xFont, pFont->GetFontData().maColor, pFont->GetFontColorId() );
    }
}

void XclExpChFontBase::ConvertFontBase( const XclExpChRoot& rRoot, const ScfPropertySet& rPropSet )
{
    ConvertFontBase( rRoot, rRoot.ConvertFont( rPropSet, rRoot.GetDefApiScript() ) );
}

void XclExpChFontBase::ConvertRotationBase(
        const XclExpChRoot& rRoot, const ScfPropertySet& rPropSet, bool bSupportsStacked )
{
    sal_uInt16 nRotation = rRoot.GetChartPropSetHelper().ReadRotationProperties( rPropSet, bSupportsStacked );
    SetRotation( nRotation );
}

// ----------------------------------------------------------------------------

XclExpChText::XclExpChText( const XclExpChRoot& rRoot ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_TEXT, EXC_ID_CHTEXT, (rRoot.GetBiff() == EXC_BIFF8) ? 32 : 26 ),
    mnTextColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) )
{
}

void XclExpChText::SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId )
{
    mxFont = xFont;
    maData.maTextColor = rColor;
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOCOLOR, rColor == COL_AUTO );
    mnTextColorId = nColorId;
}

void XclExpChText::SetRotation( sal_uInt16 nRotation )
{
    maData.mnRotation = nRotation;
    ::insert_value( maData.mnFlags, XclTools::GetXclOrientFromRot( nRotation ), 8, 3 );
}

void XclExpChText::ConvertTitle( Reference< XTitle > xTitle, sal_uInt16 nTarget, const String* pSubTitle )
{
    switch( nTarget )
    {
        case EXC_CHOBJLINK_TITLE:   SetFutureRecordContext( EXC_CHFRBLOCK_TEXT_TITLE );         break;
        case EXC_CHOBJLINK_YAXIS:   SetFutureRecordContext( EXC_CHFRBLOCK_TEXT_AXISTITLE, 1 );  break;
        case EXC_CHOBJLINK_XAXIS:   SetFutureRecordContext( EXC_CHFRBLOCK_TEXT_AXISTITLE, 0 );  break;
        case EXC_CHOBJLINK_ZAXIS:   SetFutureRecordContext( EXC_CHFRBLOCK_TEXT_AXISTITLE, 2 );  break;
    }

    mxSrcLink.reset();
    mxObjLink.reset( new XclExpChObjectLink( nTarget, XclChDataPointPos( 0, 0 ) ) );

    if( xTitle.is() )
    {
        // title frame formatting
        ScfPropertySet aTitleProp( xTitle );
        mxFrame = lclCreateFrame( GetChRoot(), aTitleProp, EXC_CHOBJTYPE_TEXT );

        // string sequence
        mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
        sal_uInt16 nFontIdx = mxSrcLink->ConvertStringSequence( xTitle->getText() );
        if (pSubTitle)
        {
            // append subtitle as the 2nd line of the title.
            String aSubTitle = rtl::OUString("\n");
            aSubTitle.Append(*pSubTitle);
            mxSrcLink->AppendString(aSubTitle);
        }

        ConvertFontBase( GetChRoot(), nFontIdx );

        // rotation
        ConvertRotationBase( GetChRoot(), aTitleProp, true );

        // manual text position - only for main title
        mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_PARENT, EXC_CHFRAMEPOS_PARENT ) );
        if( nTarget == EXC_CHOBJLINK_TITLE )
        {
            Any aRelPos;
            if( aTitleProp.GetAnyProperty( aRelPos, EXC_CHPROP_RELATIVEPOSITION ) && aRelPos.has< RelativePosition >() ) try
            {
                // calculate absolute position for CHTEXT record
                Reference< cssc::XChartDocument > xChart1Doc( GetChartDocument(), UNO_QUERY_THROW );
                Reference< XShape > xTitleShape( xChart1Doc->getTitle(), UNO_SET_THROW );
                ::com::sun::star::awt::Point aPos = xTitleShape->getPosition();
                ::com::sun::star::awt::Size aSize = xTitleShape->getSize();
                ::com::sun::star::awt::Rectangle aRect( aPos.X, aPos.Y, aSize.Width, aSize.Height );
                maData.maRect = CalcChartRectFromHmm( aRect );
                ::insert_value( maData.mnFlags2, EXC_CHTEXT_POS_MOVED, 0, 4 );
                // manual title position implies manual plot area
                GetChartData().SetManualPlotArea();
                // calculate the default title position in chart units
                sal_Int32 nDefPosX = ::std::max< sal_Int32 >( (EXC_CHART_TOTALUNITS - maData.maRect.mnWidth) / 2, 0 );
                sal_Int32 nDefPosY = 85;
                // set the position relative to the standard position
                XclChRectangle& rFrameRect = mxFramePos->GetFramePosData().maRect;
                rFrameRect.mnX = maData.maRect.mnX - nDefPosX;
                rFrameRect.mnY = maData.maRect.mnY - nDefPosY;
            }
            catch( Exception& )
            {
            }
        }
    }
    else
    {
        ::set_flag( maData.mnFlags, EXC_CHTEXT_DELETED );
    }
}

void XclExpChText::ConvertLegend( const ScfPropertySet& rPropSet )
{
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOTEXT );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOGEN );
    ConvertFontBase( GetChRoot(), rPropSet );
}

bool XclExpChText::ConvertDataLabel( const ScfPropertySet& rPropSet,
        const XclChTypeInfo& rTypeInfo, const XclChDataPointPos& rPointPos )
{
    SetFutureRecordContext( EXC_CHFRBLOCK_TEXT_DATALABEL, rPointPos.mnPointIdx, rPointPos.mnSeriesIdx );

    cssc2::DataPointLabel aPointLabel;
    if( !rPropSet.GetProperty( aPointLabel, EXC_CHPROP_LABEL ) )
        return false;

    // percentage only allowed in pie and donut charts
    bool bIsPie = rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE;
    // bubble sizes only allowed in bubble charts
    bool bIsBubble = rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES;
    OSL_ENSURE( (GetBiff() == EXC_BIFF8) || !bIsBubble, "XclExpChText::ConvertDataLabel - bubble charts only in BIFF8" );

    // raw show flags
    bool bShowValue   = !bIsBubble && aPointLabel.ShowNumber;       // Chart2 uses 'ShowNumber' for bubble size
    bool bShowPercent = bIsPie && aPointLabel.ShowNumberInPercent;  // percentage only in pie/donut charts
    bool bShowCateg   = aPointLabel.ShowCategoryName;
    bool bShowBubble  = bIsBubble && aPointLabel.ShowNumber;        // Chart2 uses 'ShowNumber' for bubble size
    bool bShowAny     = bShowValue || bShowPercent || bShowCateg || bShowBubble;

    // create the CHFRLABELPROPS record for extended settings in BIFF8
    if( bShowAny && (GetBiff() == EXC_BIFF8) )
    {
        mxLabelProps.reset( new XclExpChFrLabelProps( GetChRoot() ) );
        mxLabelProps->Convert( rPropSet, false, bShowCateg, bShowValue, bShowPercent, bShowBubble );
    }

    // restrict to combinations allowed in CHTEXT
    if( bShowPercent ) bShowValue = false;              // percent wins over value
    if( bShowValue ) bShowCateg = false;                // value wins over category
    if( bShowValue || bShowCateg ) bShowBubble = false; // value or category wins over bubble size

    // set all flags
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOTEXT );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWVALUE, bShowValue );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWPERCENT, bShowPercent );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG, bShowCateg );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEGPERC, bShowPercent && bShowCateg );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWBUBBLE, bShowBubble );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWSYMBOL, bShowAny && aPointLabel.ShowLegendSymbol );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_DELETED, !bShowAny );

    if( bShowAny )
    {
        // font settings
        ConvertFontBase( GetChRoot(), rPropSet );
        ConvertRotationBase( GetChRoot(), rPropSet, false );
        // label placement
        sal_Int32 nPlacement = 0;
        sal_uInt16 nLabelPos = EXC_CHTEXT_POS_AUTO;
        if( rPropSet.GetProperty( nPlacement, EXC_CHPROP_LABELPLACEMENT ) )
        {
            using namespace cssc::DataLabelPlacement;
            if( nPlacement == rTypeInfo.mnDefaultLabelPos )
            {
                nLabelPos = EXC_CHTEXT_POS_DEFAULT;
            }
            else switch( nPlacement )
            {
                case AVOID_OVERLAP:     nLabelPos = EXC_CHTEXT_POS_AUTO;    break;
                case CENTER:            nLabelPos = EXC_CHTEXT_POS_CENTER;  break;
                case TOP:               nLabelPos = EXC_CHTEXT_POS_ABOVE;   break;
                case TOP_LEFT:          nLabelPos = EXC_CHTEXT_POS_LEFT;    break;
                case LEFT:              nLabelPos = EXC_CHTEXT_POS_LEFT;    break;
                case BOTTOM_LEFT:       nLabelPos = EXC_CHTEXT_POS_LEFT;    break;
                case BOTTOM:            nLabelPos = EXC_CHTEXT_POS_BELOW;   break;
                case BOTTOM_RIGHT:      nLabelPos = EXC_CHTEXT_POS_RIGHT;   break;
                case RIGHT:             nLabelPos = EXC_CHTEXT_POS_RIGHT;   break;
                case TOP_RIGHT:         nLabelPos = EXC_CHTEXT_POS_RIGHT;   break;
                case INSIDE:            nLabelPos = EXC_CHTEXT_POS_INSIDE;  break;
                case OUTSIDE:           nLabelPos = EXC_CHTEXT_POS_OUTSIDE; break;
                case NEAR_ORIGIN:       nLabelPos = EXC_CHTEXT_POS_AXIS;    break;
                default:                OSL_FAIL( "XclExpChText::ConvertDataLabel - unknown label placement type" );
            }
        }
        ::insert_value( maData.mnFlags2, nLabelPos, 0, 4 );
        // source link (contains number format)
        mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
        if( bShowValue || bShowPercent )
            // percentage format wins over value format
            mxSrcLink->ConvertNumFmt( rPropSet, bShowPercent );
        // object link
        mxObjLink.reset( new XclExpChObjectLink( EXC_CHOBJLINK_DATA, rPointPos ) );
    }

    /*  Return true to indicate valid label settings:
        - for existing labels at entire series
        - for any settings at single data point (to be able to delete a point label) */
    return bShowAny || (rPointPos.mnPointIdx != EXC_CHDATAFORMAT_ALLPOINTS);
}

void XclExpChText::ConvertTrendLineEquation( const ScfPropertySet& rPropSet, const XclChDataPointPos& rPointPos )
{
    // required flags
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOTEXT );
    if( GetBiff() == EXC_BIFF8 )
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG ); // must set this to make equation visible in Excel
    // frame formatting
    mxFrame = lclCreateFrame( GetChRoot(), rPropSet, EXC_CHOBJTYPE_TEXT );
    // font settings
    maData.mnHAlign = EXC_CHTEXT_ALIGN_TOPLEFT;
    maData.mnVAlign = EXC_CHTEXT_ALIGN_TOPLEFT;
    ConvertFontBase( GetChRoot(), rPropSet );
    // source link (contains number format)
    mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
    mxSrcLink->ConvertNumFmt( rPropSet, false );
    // object link
    mxObjLink.reset( new XclExpChObjectLink( EXC_CHOBJLINK_DATA, rPointPos ) );
}

sal_uInt16 XclExpChText::GetAttLabelFlags() const
{
    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_CHATTLABEL_SHOWVALUE,     ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWVALUE ) );
    ::set_flag( nFlags, EXC_CHATTLABEL_SHOWPERCENT,   ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWPERCENT ) );
    ::set_flag( nFlags, EXC_CHATTLABEL_SHOWCATEGPERC, ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEGPERC ) );
    ::set_flag( nFlags, EXC_CHATTLABEL_SHOWCATEG,     ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG ) );
    return nFlags;
}

void XclExpChText::WriteSubRecords( XclExpStream& rStrm )
{
    // CHFRAMEPOS record
    lclSaveRecord( rStrm, mxFramePos );
    // CHFONT record
    lclSaveRecord( rStrm, mxFont );
    // CHSOURCELINK group
    lclSaveRecord( rStrm, mxSrcLink );
    // CHFRAME group
    lclSaveRecord( rStrm, mxFrame );
    // CHOBJECTLINK record
    lclSaveRecord( rStrm, mxObjLink );
    // CHFRLABELPROPS record
    lclSaveRecord( rStrm, mxLabelProps );
}

void XclExpChText::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnHAlign
            << maData.mnVAlign
            << maData.mnBackMode
            << maData.maTextColor
            << maData.maRect
            << maData.mnFlags;

    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm   << GetPalette().GetColorIndex( mnTextColorId )
                << maData.mnFlags2
                << maData.mnRotation;
    }
}

// ----------------------------------------------------------------------------

namespace {

/** Creates and returns an Excel text object from the passed title. */
XclExpChTextRef lclCreateTitle( const XclExpChRoot& rRoot, Reference< XTitled > xTitled, sal_uInt16 nTarget,
                                const String* pSubTitle = NULL )
{
    Reference< XTitle > xTitle;
    if( xTitled.is() )
        xTitle = xTitled->getTitleObject();

    XclExpChTextRef xText( new XclExpChText( rRoot ) );
    xText->ConvertTitle( xTitle, nTarget, pSubTitle );
    /*  Do not delete the CHTEXT group for the main title. A missing CHTEXT
        will be interpreted as auto-generated title showing the series title in
        charts that contain exactly one data series. */
    if( (nTarget != EXC_CHOBJLINK_TITLE) && !xText->HasString() )
        xText.reset();

    return xText;
}

}

// Data series ================================================================

XclExpChMarkerFormat::XclExpChMarkerFormat( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHMARKERFORMAT, (rRoot.GetBiff() == EXC_BIFF8) ? 20 : 12 ),
    mnLineColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) ),
    mnFillColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) )
{
}

void XclExpChMarkerFormat::Convert( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx )
{
    rRoot.GetChartPropSetHelper().ReadMarkerProperties( maData, rPropSet, nFormatIdx );
    /*  Set marker line/fill color to series line color.
        TODO: remove this if OOChart supports own colors in markers. */
    Color aLineColor;
    if( rPropSet.GetColorProperty( aLineColor, EXC_CHPROP_COLOR ) )
        maData.maLineColor = maData.maFillColor = aLineColor;
    // register colors in palette
    RegisterColors( rRoot );
}

void XclExpChMarkerFormat::ConvertStockSymbol( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, bool bCloseSymbol )
{
    // clear the automatic flag
    ::set_flag( maData.mnFlags, EXC_CHMARKERFORMAT_AUTO, false );
    // symbol type and color
    if( bCloseSymbol )
    {
        // set symbol type for the 'close' data series
        maData.mnMarkerType = EXC_CHMARKERFORMAT_DOWJ;
        maData.mnMarkerSize = EXC_CHMARKERFORMAT_DOUBLESIZE;
        // set symbol line/fill color to series line color
        Color aLineColor;
        if( rPropSet.GetColorProperty( aLineColor, EXC_CHPROP_COLOR ) )
        {
            maData.maLineColor = maData.maFillColor = aLineColor;
            RegisterColors( rRoot );
        }
    }
    else
    {
        // set invisible symbol
        maData.mnMarkerType = EXC_CHMARKERFORMAT_NOSYMBOL;
    }
}

void XclExpChMarkerFormat::RegisterColors( const XclExpChRoot& rRoot )
{
    if( HasMarker() )
    {
        if( HasLineColor() )
            mnLineColorId = rRoot.GetPalette().InsertColor( maData.maLineColor, EXC_COLOR_CHARTLINE );
        if( HasFillColor() )
            mnFillColorId = rRoot.GetPalette().InsertColor( maData.maFillColor, EXC_COLOR_CHARTAREA );
    }
}

void XclExpChMarkerFormat::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.maLineColor << maData.maFillColor << maData.mnMarkerType << maData.mnFlags;
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
    {
        const XclExpPalette& rPal = rStrm.GetRoot().GetPalette();
        rStrm << rPal.GetColorIndex( mnLineColorId ) << rPal.GetColorIndex( mnFillColorId ) << maData.mnMarkerSize;
    }
}

// ----------------------------------------------------------------------------

XclExpChPieFormat::XclExpChPieFormat() :
    XclExpUInt16Record( EXC_ID_CHPIEFORMAT, 0 )
{
}

void XclExpChPieFormat::Convert( const ScfPropertySet& rPropSet )
{
    double fApiDist(0.0);
    if( rPropSet.GetProperty( fApiDist, EXC_CHPROP_OFFSET ) )
        SetValue( limit_cast< sal_uInt16 >( fApiDist * 100.0, 0, 100 ) );
}

// ----------------------------------------------------------------------------

XclExpCh3dDataFormat::XclExpCh3dDataFormat() :
    XclExpRecord( EXC_ID_CH3DDATAFORMAT, 2 )
{
}

void XclExpCh3dDataFormat::Convert( const ScfPropertySet& rPropSet )
{
    sal_Int32 nApiType(0);
    if( rPropSet.GetProperty( nApiType, EXC_CHPROP_GEOMETRY3D ) )
    {
        using namespace cssc2::DataPointGeometry3D;
        switch( nApiType )
        {
            case CUBOID:
                maData.mnBase = EXC_CH3DDATAFORMAT_RECT;
                maData.mnTop = EXC_CH3DDATAFORMAT_STRAIGHT;
            break;
            case PYRAMID:
                maData.mnBase = EXC_CH3DDATAFORMAT_RECT;
                maData.mnTop = EXC_CH3DDATAFORMAT_SHARP;
            break;
            case CYLINDER:
                maData.mnBase = EXC_CH3DDATAFORMAT_CIRC;
                maData.mnTop = EXC_CH3DDATAFORMAT_STRAIGHT;
            break;
            case CONE:
                maData.mnBase = EXC_CH3DDATAFORMAT_CIRC;
                maData.mnTop = EXC_CH3DDATAFORMAT_SHARP;
            break;
            default:
                OSL_FAIL( "XclExpCh3dDataFormat::Convert - unknown 3D bar format" );
        }
    }
}

void XclExpCh3dDataFormat::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnBase << maData.mnTop;
}

// ----------------------------------------------------------------------------

XclExpChAttachedLabel::XclExpChAttachedLabel( sal_uInt16 nFlags ) :
    XclExpUInt16Record( EXC_ID_CHATTACHEDLABEL, nFlags )
{
}

// ----------------------------------------------------------------------------

XclExpChDataFormat::XclExpChDataFormat( const XclExpChRoot& rRoot,
        const XclChDataPointPos& rPointPos, sal_uInt16 nFormatIdx ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_DATAFORMAT, EXC_ID_CHDATAFORMAT, 8 )
{
    maData.maPointPos = rPointPos;
    maData.mnFormatIdx = nFormatIdx;
}

void XclExpChDataFormat::ConvertDataSeries( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo )
{
    // line and area formatting
    ConvertFrameBase( GetChRoot(), rPropSet, rTypeInfo.GetSeriesObjectType() );

    // data point symbols
    bool bIsFrame = rTypeInfo.IsSeriesFrameFormat();
    if( !bIsFrame )
    {
        mxMarkerFmt.reset( new XclExpChMarkerFormat( GetChRoot() ) );
        mxMarkerFmt->Convert( GetChRoot(), rPropSet, maData.mnFormatIdx );
    }

    // pie segments
    if( rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE )
    {
        mxPieFmt.reset( new XclExpChPieFormat );
        mxPieFmt->Convert( rPropSet );
    }

    // 3D bars (only allowed for entire series in BIFF8)
    if( IsSeriesFormat() && (GetBiff() == EXC_BIFF8) && rTypeInfo.mb3dChart && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR) )
    {
        mx3dDataFmt.reset( new XclExpCh3dDataFormat );
        mx3dDataFmt->Convert( rPropSet );
    }

    // spline
    if( IsSeriesFormat() && rTypeInfo.mbSpline && !bIsFrame )
        mxSeriesFmt.reset( new XclExpUInt16Record( EXC_ID_CHSERIESFORMAT, EXC_CHSERIESFORMAT_SMOOTHED ) );

    // data point labels
    XclExpChTextRef xLabel( new XclExpChText( GetChRoot() ) );
    if( xLabel->ConvertDataLabel( rPropSet, rTypeInfo, maData.maPointPos ) )
    {
        // CHTEXT groups for data labels are stored in global CHCHART group
        GetChartData().SetDataLabel( xLabel );
        mxAttLabel.reset( new XclExpChAttachedLabel( xLabel->GetAttLabelFlags() ) );
    }
}

void XclExpChDataFormat::ConvertStockSeries( const ScfPropertySet& rPropSet, bool bCloseSymbol )
{
    // set line format to invisible
    SetDefaultFrameBase( GetChRoot(), EXC_CHFRAMETYPE_INVISIBLE, false );
    // set symbols to invisible or to 'close' series symbol
    mxMarkerFmt.reset( new XclExpChMarkerFormat( GetChRoot() ) );
    mxMarkerFmt->ConvertStockSymbol( GetChRoot(), rPropSet, bCloseSymbol );
}

void XclExpChDataFormat::ConvertLine( const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    ConvertFrameBase( GetChRoot(), rPropSet, eObjType );
}

void XclExpChDataFormat::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mx3dDataFmt );
    WriteFrameRecords( rStrm );
    lclSaveRecord( rStrm, mxPieFmt );
    lclSaveRecord( rStrm, mxMarkerFmt );
    lclSaveRecord( rStrm, mxSeriesFmt );
    lclSaveRecord( rStrm, mxAttLabel );
}

void XclExpChDataFormat::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.maPointPos.mnPointIdx
            << maData.maPointPos.mnSeriesIdx
            << maData.mnFormatIdx
            << maData.mnFlags;
}

// ----------------------------------------------------------------------------

XclExpChSerTrendLine::XclExpChSerTrendLine( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHSERTRENDLINE, 28 ),
    XclExpChRoot( rRoot )
{
}

bool XclExpChSerTrendLine::Convert( Reference< XRegressionCurve > xRegCurve, sal_uInt16 nSeriesIdx )
{
    if( !xRegCurve.is() )
        return false;

    // trend line type
    ScfPropertySet aCurveProp( xRegCurve );
    OUString aService = aCurveProp.GetServiceName();
    if( aService == SERVICE_CHART2_LINEARREGCURVE )
    {
        maData.mnLineType = EXC_CHSERTREND_POLYNOMIAL;
        maData.mnOrder = 1;
    }
    else if( aService == SERVICE_CHART2_EXPREGCURVE )
        maData.mnLineType = EXC_CHSERTREND_EXPONENTIAL;
    else if( aService == SERVICE_CHART2_LOGREGCURVE )
        maData.mnLineType = EXC_CHSERTREND_LOGARITHMIC;
    else if( aService == SERVICE_CHART2_POTREGCURVE )
        maData.mnLineType = EXC_CHSERTREND_POWER;
    else
        return false;

    // line formatting
    XclChDataPointPos aPointPos( nSeriesIdx );
    mxDataFmt.reset( new XclExpChDataFormat( GetChRoot(), aPointPos, 0 ) );
    mxDataFmt->ConvertLine( aCurveProp, EXC_CHOBJTYPE_TRENDLINE );

    // #i83100# show equation and correlation coefficient
    ScfPropertySet aEquationProp( xRegCurve->getEquationProperties() );
    maData.mnShowEquation = aEquationProp.GetBoolProperty( EXC_CHPROP_SHOWEQUATION ) ? 1 : 0;
    maData.mnShowRSquared = aEquationProp.GetBoolProperty( EXC_CHPROP_SHOWCORRELATION ) ? 1 : 0;

    // #i83100# formatting of the equation text box
    if( (maData.mnShowEquation != 0) || (maData.mnShowRSquared != 0) )
    {
        mxLabel.reset( new XclExpChText( GetChRoot() ) );
        mxLabel->ConvertTrendLineEquation( aEquationProp, aPointPos );
    }

    // missing features
    // #i20819# polynomial trend lines
    // #i66819# moving average trend lines
    // #i5085# manual trend line size
    // #i34093# manual crossing point
    return true;
}

void XclExpChSerTrendLine::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnLineType
            << maData.mnOrder
            << maData.mfIntercept
            << maData.mnShowEquation
            << maData.mnShowRSquared
            << maData.mfForecastFor
            << maData.mfForecastBack;
}

// ----------------------------------------------------------------------------

XclExpChSerErrorBar::XclExpChSerErrorBar( const XclExpChRoot& rRoot, sal_uInt8 nBarType ) :
    XclExpRecord( EXC_ID_CHSERERRORBAR, 14 ),
    XclExpChRoot( rRoot )
{
    maData.mnBarType = nBarType;
}

bool XclExpChSerErrorBar::Convert( XclExpChSourceLink& rValueLink, sal_uInt16& rnValueCount, const ScfPropertySet& rPropSet )
{
    sal_Int32 nBarStyle = 0;
    bool bOk = rPropSet.GetProperty( nBarStyle, EXC_CHPROP_ERRORBARSTYLE );
    if( bOk )
    {
        switch( nBarStyle )
        {
            case cssc::ErrorBarStyle::ABSOLUTE:
                maData.mnSourceType = EXC_CHSERERR_FIXED;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_POSITIVEERROR );
            break;
            case cssc::ErrorBarStyle::RELATIVE:
                maData.mnSourceType = EXC_CHSERERR_PERCENT;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_POSITIVEERROR );
            break;
            case cssc::ErrorBarStyle::STANDARD_DEVIATION:
                maData.mnSourceType = EXC_CHSERERR_STDDEV;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_WEIGHT );
            break;
            case cssc::ErrorBarStyle::STANDARD_ERROR:
                maData.mnSourceType = EXC_CHSERERR_STDERR;
            break;
            case cssc::ErrorBarStyle::FROM_DATA:
            {
                bOk = false;
                maData.mnSourceType = EXC_CHSERERR_CUSTOM;
                Reference< XDataSource > xDataSource( rPropSet.GetApiPropertySet(), UNO_QUERY );
                if( xDataSource.is() )
                {
                    // find first sequence with current role
                    OUString aRole = XclChartHelper::GetErrorBarValuesRole( maData.mnBarType );
                    Reference< XDataSequence > xValueSeq;

                    Sequence< Reference< XLabeledDataSequence > > aLabeledSeqVec = xDataSource->getDataSequences();
                    const Reference< XLabeledDataSequence >* pBeg = aLabeledSeqVec.getConstArray();
                    const Reference< XLabeledDataSequence >* pEnd = pBeg + aLabeledSeqVec.getLength();
                    for( const Reference< XLabeledDataSequence >* pIt = pBeg; !xValueSeq.is() && (pIt != pEnd); ++pIt )
                    {
                        Reference< XDataSequence > xTmpValueSeq = (*pIt)->getValues();
                        ScfPropertySet aValueProp( xTmpValueSeq );
                        OUString aCurrRole;
                        if( aValueProp.GetProperty( aCurrRole, EXC_CHPROP_ROLE ) && (aCurrRole == aRole) )
                            xValueSeq = xTmpValueSeq;
                    }
                    if( xValueSeq.is() )
                    {
                        // #i86465# pass value count back to series
                        rnValueCount = maData.mnValueCount = rValueLink.ConvertDataSequence( xValueSeq, true );
                        bOk = maData.mnValueCount > 0;
                    }
                }
            }
            break;
            default:
                bOk = false;
        }
    }
    return bOk;
}

void XclExpChSerErrorBar::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnBarType
            << maData.mnSourceType
            << maData.mnLineEnd
            << sal_uInt8( 1 )       // must be 1 to make line visible
            << maData.mfValue
            << maData.mnValueCount;
}

// ----------------------------------------------------------------------------

namespace {

/** Returns the property set of the specified data point. */
ScfPropertySet lclGetPointPropSet( Reference< XDataSeries > xDataSeries, sal_Int32 nPointIdx )
{
    ScfPropertySet aPropSet;
    try
    {
        aPropSet.Set( xDataSeries->getDataPointByIndex( nPointIdx ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "lclGetPointPropSet - no data point property set" );
    }
    return aPropSet;
}

} // namespace

XclExpChSeries::XclExpChSeries( const XclExpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_SERIES, EXC_ID_CHSERIES, (rRoot.GetBiff() == EXC_BIFF8) ? 12 : 8 ),
    mnGroupIdx( EXC_CHSERGROUP_NONE ),
    mnSeriesIdx( nSeriesIdx ),
    mnParentIdx( EXC_CHSERIES_INVALID )
{
    // CHSOURCELINK records are always required, even if unused
    mxTitleLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
    mxValueLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_VALUES ) );
    mxCategLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_CATEGORY ) );
    if( GetBiff() == EXC_BIFF8 )
        mxBubbleLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_BUBBLES ) );
}

bool XclExpChSeries::ConvertDataSeries(
        Reference< XDiagram > xDiagram, Reference< XDataSeries > xDataSeries,
        const XclChExtTypeInfo& rTypeInfo, sal_uInt16 nGroupIdx, sal_uInt16 nFormatIdx )
{
    bool bOk = false;
    Reference< XDataSource > xDataSource( xDataSeries, UNO_QUERY );
    if( xDataSource.is() )
    {
        Reference< XDataSequence > xYValueSeq, xTitleSeq, xXValueSeq, xBubbleSeq;

        // find first sequence with role 'values-y'
        Sequence< Reference< XLabeledDataSequence > > aLabeledSeqVec = xDataSource->getDataSequences();
        const Reference< XLabeledDataSequence >* pBeg = aLabeledSeqVec.getConstArray();
        const Reference< XLabeledDataSequence >* pEnd = pBeg + aLabeledSeqVec.getLength();
        for( const Reference< XLabeledDataSequence >* pIt = pBeg; pIt != pEnd; ++pIt )
        {
            Reference< XDataSequence > xTmpValueSeq = (*pIt)->getValues();
            ScfPropertySet aValueProp( xTmpValueSeq );
            OUString aRole;
            if( aValueProp.GetProperty( aRole, EXC_CHPROP_ROLE ) )
            {
                if( !xYValueSeq.is() && (aRole == EXC_CHPROP_ROLE_YVALUES) )
                {
                    xYValueSeq = xTmpValueSeq;
                    if( !xTitleSeq.is() )
                        xTitleSeq = (*pIt)->getLabel(); // ignore role of label sequence
                }
                else if( !xXValueSeq.is() && !rTypeInfo.mbCategoryAxis && (aRole == EXC_CHPROP_ROLE_XVALUES) )
                {
                    xXValueSeq = xTmpValueSeq;
                }
                else if( !xBubbleSeq.is() && (rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES) && (aRole == EXC_CHPROP_ROLE_SIZEVALUES) )
                {
                    xBubbleSeq = xTmpValueSeq;
                    xTitleSeq = (*pIt)->getLabel();     // ignore role of label sequence
                }
            }
        }

        bOk = xYValueSeq.is();
        if( bOk )
        {
            // chart type group index
            mnGroupIdx = nGroupIdx;

            // convert source links
            maData.mnValueCount = mxValueLink->ConvertDataSequence( xYValueSeq, true );
            mxTitleLink->ConvertDataSequence( xTitleSeq, true );

            // X values of XY charts
            maData.mnCategCount = mxCategLink->ConvertDataSequence( xXValueSeq, false, maData.mnValueCount );

            // size values of bubble charts
            if( mxBubbleLink )
                mxBubbleLink->ConvertDataSequence( xBubbleSeq, false, maData.mnValueCount );

            // series formatting
            XclChDataPointPos aPointPos( mnSeriesIdx );
            ScfPropertySet aSeriesProp( xDataSeries );
            mxSeriesFmt.reset( new XclExpChDataFormat( GetChRoot(), aPointPos, nFormatIdx ) );
            mxSeriesFmt->ConvertDataSeries( aSeriesProp, rTypeInfo );

            // trend lines
            CreateTrendLines( xDataSeries );

            // error bars
            CreateErrorBars( aSeriesProp, EXC_CHPROP_ERRORBARX, EXC_CHSERERR_XPLUS, EXC_CHSERERR_XMINUS );
            CreateErrorBars( aSeriesProp, EXC_CHPROP_ERRORBARY, EXC_CHSERERR_YPLUS, EXC_CHSERERR_YMINUS );

            if( maData.mnValueCount > 0 )
            {
                const sal_Int32 nMaxPointCount = maData.mnValueCount;

                /*  #i91063# Create missing fill properties in pie/doughnut charts.
                    If freshly created (never saved to ODF), these charts show
                    varying point colors but do not return these points via API. */
                if( xDiagram.is() && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE) )
                {
                    Reference< XColorScheme > xColorScheme = xDiagram->getDefaultColorScheme();
                    if( xColorScheme.is() )
                    {
                        const OUString aFillStyleName = "FillStyle";
                        const OUString aColorName = "Color";
                        namespace cssd = ::com::sun::star::drawing;
                        for( sal_Int32 nPointIdx = 0; nPointIdx < nMaxPointCount; ++nPointIdx )
                        {
                            aPointPos.mnPointIdx = static_cast< sal_uInt16 >( nPointIdx );
                            ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, nPointIdx );
                            // test that the point fill style is solid, but no color is set
                            cssd::FillStyle eFillStyle = cssd::FillStyle_NONE;
                            if( aPointProp.GetProperty( eFillStyle, aFillStyleName ) &&
                                (eFillStyle == cssd::FillStyle_SOLID) &&
                                !aPointProp.HasProperty( aColorName ) )
                            {
                                aPointProp.SetProperty( aColorName, xColorScheme->getColorByIndex( nPointIdx ) );
                            }
                        }
                    }
                }

                // data point formatting
                Sequence< sal_Int32 > aPointIndexes;
                if( aSeriesProp.GetProperty( aPointIndexes, EXC_CHPROP_ATTRIBDATAPOINTS ) && aPointIndexes.hasElements() )
                {
                    const sal_Int32* pnBeg = aPointIndexes.getConstArray();
                    const sal_Int32* pnEnd = pnBeg + aPointIndexes.getLength();
                    for( const sal_Int32* pnIt = pnBeg; (pnIt != pnEnd) && (*pnIt < nMaxPointCount); ++pnIt )
                    {
                        aPointPos.mnPointIdx = static_cast< sal_uInt16 >( *pnIt );
                        ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, *pnIt );
                        XclExpChDataFormatRef xPointFmt( new XclExpChDataFormat( GetChRoot(), aPointPos, nFormatIdx ) );
                        xPointFmt->ConvertDataSeries( aPointProp, rTypeInfo );
                        maPointFmts.AppendRecord( xPointFmt );
                    }
                }
            }
        }
    }
    return bOk;
}

bool XclExpChSeries::ConvertStockSeries( XDataSeriesRef xDataSeries,
        const OUString& rValueRole, sal_uInt16 nGroupIdx, sal_uInt16 nFormatIdx, bool bCloseSymbol )
{
    bool bOk = false;
    Reference< XDataSource > xDataSource( xDataSeries, UNO_QUERY );
    if( xDataSource.is() )
    {
        Reference< XDataSequence > xYValueSeq, xTitleSeq;

        // find first sequence with passed role
        Sequence< Reference< XLabeledDataSequence > > aLabeledSeqVec = xDataSource->getDataSequences();
        const Reference< XLabeledDataSequence >* pBeg = aLabeledSeqVec.getConstArray();
        const Reference< XLabeledDataSequence >* pEnd = pBeg + aLabeledSeqVec.getLength();
        for( const Reference< XLabeledDataSequence >* pIt = pBeg; !xYValueSeq.is() && (pIt != pEnd); ++pIt )
        {
            Reference< XDataSequence > xTmpValueSeq = (*pIt)->getValues();
            ScfPropertySet aValueProp( xTmpValueSeq );
            OUString aRole;
            if( aValueProp.GetProperty( aRole, EXC_CHPROP_ROLE ) && (aRole == rValueRole) )
            {
                xYValueSeq = xTmpValueSeq;
                xTitleSeq = (*pIt)->getLabel();     // ignore role of label sequence
            }
        }

        bOk = xYValueSeq.is();
        if( bOk )
        {
            // chart type group index
            mnGroupIdx = nGroupIdx;
            // convert source links
            maData.mnValueCount = mxValueLink->ConvertDataSequence( xYValueSeq, true );
            mxTitleLink->ConvertDataSequence( xTitleSeq, true );
            // series formatting
            ScfPropertySet aSeriesProp( xDataSeries );
            mxSeriesFmt.reset( new XclExpChDataFormat( GetChRoot(), XclChDataPointPos( mnSeriesIdx ), nFormatIdx ) );
            mxSeriesFmt->ConvertStockSeries( aSeriesProp, bCloseSymbol );
        }
    }
    return bOk;
}

bool XclExpChSeries::ConvertTrendLine( const XclExpChSeries& rParent, Reference< XRegressionCurve > xRegCurve )
{
    InitFromParent( rParent );
    mxTrendLine.reset( new XclExpChSerTrendLine( GetChRoot() ) );
    bool bOk = mxTrendLine->Convert( xRegCurve, mnSeriesIdx );
    if( bOk )
    {
        mxSeriesFmt = mxTrendLine->GetDataFormat();
        GetChartData().SetDataLabel( mxTrendLine->GetDataLabel() );
    }
    return bOk;
}

bool XclExpChSeries::ConvertErrorBar( const XclExpChSeries& rParent, const ScfPropertySet& rPropSet, sal_uInt8 nBarId )
{
    InitFromParent( rParent );
    // error bar settings
    mxErrorBar.reset( new XclExpChSerErrorBar( GetChRoot(), nBarId ) );
    bool bOk = mxErrorBar->Convert( *mxValueLink, maData.mnValueCount, rPropSet );
    if( bOk )
    {
        // error bar formatting
        mxSeriesFmt.reset( new XclExpChDataFormat( GetChRoot(), XclChDataPointPos( mnSeriesIdx ), 0 ) );
        mxSeriesFmt->ConvertLine( rPropSet, EXC_CHOBJTYPE_ERRORBAR );
    }
    return bOk;
}

void XclExpChSeries::ConvertCategSequence( Reference< XLabeledDataSequence > xCategSeq )
{
    if( xCategSeq.is() )
        maData.mnCategCount = mxCategLink->ConvertDataSequence( xCategSeq->getValues(), false );
}

void XclExpChSeries::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxTitleLink );
    lclSaveRecord( rStrm, mxValueLink );
    lclSaveRecord( rStrm, mxCategLink );
    lclSaveRecord( rStrm, mxBubbleLink );
    lclSaveRecord( rStrm, mxSeriesFmt );
    maPointFmts.Save( rStrm );
    if( mnGroupIdx != EXC_CHSERGROUP_NONE )
        XclExpUInt16Record( EXC_ID_CHSERGROUP, mnGroupIdx ).Save( rStrm );
    if( mnParentIdx != EXC_CHSERIES_INVALID )
        XclExpUInt16Record( EXC_ID_CHSERPARENT, mnParentIdx ).Save( rStrm );
    lclSaveRecord( rStrm, mxTrendLine );
    lclSaveRecord( rStrm, mxErrorBar );
}

void XclExpChSeries::InitFromParent( const XclExpChSeries& rParent )
{
    // index to parent series is stored 1-based
    mnParentIdx = rParent.mnSeriesIdx + 1;
    /*  #i86465# MSO2007 SP1 expects correct point counts in child series
        (there was no problem in Excel2003 or Excel2007 without SP1...) */
    maData.mnCategCount = rParent.maData.mnCategCount;
    maData.mnValueCount = rParent.maData.mnValueCount;
}

void XclExpChSeries::CreateTrendLines( XDataSeriesRef xDataSeries )
{
    Reference< XRegressionCurveContainer > xRegCurveCont( xDataSeries, UNO_QUERY );
    if( xRegCurveCont.is() )
    {
        Sequence< Reference< XRegressionCurve > > aRegCurveSeq = xRegCurveCont->getRegressionCurves();
        const Reference< XRegressionCurve >* pBeg = aRegCurveSeq.getConstArray();
        const Reference< XRegressionCurve >* pEnd = pBeg + aRegCurveSeq.getLength();
        for( const Reference< XRegressionCurve >* pIt = pBeg; pIt != pEnd; ++pIt )
        {
            XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
            if( xSeries && !xSeries->ConvertTrendLine( *this, *pIt ) )
                GetChartData().RemoveLastSeries();
        }
    }
}

void XclExpChSeries::CreateErrorBars( const ScfPropertySet& rPropSet,
        const OUString& rBarPropName, sal_uInt8 nPosBarId, sal_uInt8 nNegBarId )
{
    Reference< XPropertySet > xErrorBar;
    if( rPropSet.GetProperty( xErrorBar, rBarPropName ) && xErrorBar.is() )
    {
        ScfPropertySet aErrorProp( xErrorBar );
        CreateErrorBar( aErrorProp, EXC_CHPROP_SHOWPOSITIVEERROR, nPosBarId );
        CreateErrorBar( aErrorProp, EXC_CHPROP_SHOWNEGATIVEERROR, nNegBarId );
    }
}

void XclExpChSeries::CreateErrorBar( const ScfPropertySet& rPropSet,
        const OUString& rShowPropName, sal_uInt8 nBarId )
{
    if( rPropSet.GetBoolProperty( rShowPropName ) )
    {
        XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
        if( xSeries && !xSeries->ConvertErrorBar( *this, rPropSet, nBarId ) )
            GetChartData().RemoveLastSeries();
    }
}

void XclExpChSeries::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnCategType << maData.mnValueType << maData.mnCategCount << maData.mnValueCount;
    if( GetBiff() == EXC_BIFF8 )
        rStrm << maData.mnBubbleType << maData.mnBubbleCount;
}

// Chart type groups ==========================================================

XclExpChType::XclExpChType( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHUNKNOWN ),
    XclExpChRoot( rRoot ),
    maTypeInfo( rRoot.GetChartTypeInfo( EXC_CHTYPEID_UNKNOWN ) )
{
}

void XclExpChType::Convert( Reference< XDiagram > xDiagram, Reference< XChartType > xChartType,
        sal_Int32 nApiAxesSetIdx, bool bSwappedAxesSet, bool bHasXLabels )
{
    if( xChartType.is() )
    {
        maTypeInfo = GetChartTypeInfo( xChartType->getChartType() );
        // special handling for some chart types
        switch( maTypeInfo.meTypeCateg )
        {
            case EXC_CHTYPECATEG_BAR:
            {
                maTypeInfo = GetChartTypeInfo( bSwappedAxesSet ? EXC_CHTYPEID_HORBAR : EXC_CHTYPEID_BAR );
                ::set_flag( maData.mnFlags, EXC_CHBAR_HORIZONTAL, bSwappedAxesSet );
                ScfPropertySet aTypeProp( xChartType );
                Sequence< sal_Int32 > aInt32Seq;
                maData.mnOverlap = 0;
                if( aTypeProp.GetProperty( aInt32Seq, EXC_CHPROP_OVERLAPSEQ ) && (nApiAxesSetIdx < aInt32Seq.getLength()) )
                    maData.mnOverlap = limit_cast< sal_Int16 >( -aInt32Seq[ nApiAxesSetIdx ], -100, 100 );
                maData.mnGap = 150;
                if( aTypeProp.GetProperty( aInt32Seq, EXC_CHPROP_GAPWIDTHSEQ ) && (nApiAxesSetIdx < aInt32Seq.getLength()) )
                    maData.mnGap = limit_cast< sal_uInt16 >( aInt32Seq[ nApiAxesSetIdx ], 0, 500 );
            }
            break;
            case EXC_CHTYPECATEG_RADAR:
                ::set_flag( maData.mnFlags, EXC_CHRADAR_AXISLABELS, bHasXLabels );
            break;
            case EXC_CHTYPECATEG_PIE:
            {
                ScfPropertySet aTypeProp( xChartType );
                bool bDonut = aTypeProp.GetBoolProperty( EXC_CHPROP_USERINGS );
                maTypeInfo = GetChartTypeInfo( bDonut ? EXC_CHTYPEID_DONUT : EXC_CHTYPEID_PIE );
                maData.mnPieHole = bDonut ? 50 : 0;
                // #i85166# starting angle of first pie slice
                ScfPropertySet aDiaProp( xDiagram );
                maData.mnRotation = XclExpChRoot::ConvertPieRotation( aDiaProp );
            }
            break;
            case EXC_CHTYPECATEG_SCATTER:
                if( GetBiff() == EXC_BIFF8 )
                    ::set_flag( maData.mnFlags, EXC_CHSCATTER_BUBBLES, maTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES );
            break;
            default:;
        }
        SetRecId( maTypeInfo.mnRecId );
    }
}

void XclExpChType::SetStacked( bool bPercent )
{
    switch( maTypeInfo.meTypeCateg )
    {
        case EXC_CHTYPECATEG_LINE:
            ::set_flag( maData.mnFlags, EXC_CHLINE_STACKED );
            ::set_flag( maData.mnFlags, EXC_CHLINE_PERCENT, bPercent );
        break;
        case EXC_CHTYPECATEG_BAR:
            ::set_flag( maData.mnFlags, EXC_CHBAR_STACKED );
            ::set_flag( maData.mnFlags, EXC_CHBAR_PERCENT, bPercent );
            maData.mnOverlap = -100;
        break;
        default:;
    }
}

void XclExpChType::WriteBody( XclExpStream& rStrm )
{
    switch( GetRecId() )
    {
        case EXC_ID_CHBAR:
            rStrm << maData.mnOverlap << maData.mnGap << maData.mnFlags;
        break;

        case EXC_ID_CHLINE:
        case EXC_ID_CHAREA:
        case EXC_ID_CHRADARLINE:
        case EXC_ID_CHRADARAREA:
            rStrm << maData.mnFlags;
        break;

        case EXC_ID_CHPIE:
            rStrm << maData.mnRotation << maData.mnPieHole;
            if( GetBiff() == EXC_BIFF8 )
                rStrm << maData.mnFlags;
        break;

        case EXC_ID_CHSCATTER:
            if( GetBiff() == EXC_BIFF8 )
                rStrm << maData.mnBubbleSize << maData.mnBubbleType << maData.mnFlags;
        break;

        default:
            OSL_FAIL( "XclExpChType::WriteBody - unknown chart type" );
    }
}

// ----------------------------------------------------------------------------

XclExpChChart3d::XclExpChChart3d() :
    XclExpRecord( EXC_ID_CHCHART3D, 14 )
{
}

void XclExpChChart3d::Convert( const ScfPropertySet& rPropSet, bool b3dWallChart )
{
    sal_Int32 nRotationY = 0;
    rPropSet.GetProperty( nRotationY, EXC_CHPROP_ROTATIONVERTICAL );
    sal_Int32 nRotationX = 0;
    rPropSet.GetProperty( nRotationX, EXC_CHPROP_ROTATIONHORIZONTAL );
    sal_Int32 nPerspective = 15;
    rPropSet.GetProperty( nPerspective, EXC_CHPROP_PERSPECTIVE );

    if( b3dWallChart )
    {
        // Y rotation (Excel [0..359], Chart2 [-179,180])
        if( nRotationY < 0 ) nRotationY += 360;
        maData.mnRotation = static_cast< sal_uInt16 >( nRotationY );
        // X rotation a.k.a. elevation (Excel [-90..90], Chart2 [-179,180])
        maData.mnElevation = limit_cast< sal_Int16 >( nRotationX, -90, 90 );
        // perspective (Excel and Chart2 [0,100])
        maData.mnEyeDist = limit_cast< sal_uInt16 >( nPerspective, 0, 100 );
        // flags
        maData.mnFlags = 0;
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_REAL3D, !rPropSet.GetBoolProperty( EXC_CHPROP_RIGHTANGLEDAXES ) );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_AUTOHEIGHT );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_HASWALLS );
    }
    else
    {
        // Y rotation not used in pie charts, but 'first pie slice angle'
        maData.mnRotation = XclExpChRoot::ConvertPieRotation( rPropSet );
        // X rotation a.k.a. elevation (map Chart2 [-80,-10] to Excel [10..80])
        maData.mnElevation = limit_cast< sal_Int16 >( (nRotationX + 270) % 180, 10, 80 );
        // perspective (Excel and Chart2 [0,100])
        maData.mnEyeDist = limit_cast< sal_uInt16 >( nPerspective, 0, 100 );
        // flags
        maData.mnFlags = 0;
    }
}

void XclExpChChart3d::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnRotation
            << maData.mnElevation
            << maData.mnEyeDist
            << maData.mnRelHeight
            << maData.mnRelDepth
            << maData.mnDepthGap
            << maData.mnFlags;
}

// ----------------------------------------------------------------------------

XclExpChLegend::XclExpChLegend( const XclExpChRoot& rRoot ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_LEGEND, EXC_ID_CHLEGEND, 20 )
{
}

void XclExpChLegend::Convert( const ScfPropertySet& rPropSet )
{
    // frame properties
    mxFrame = lclCreateFrame( GetChRoot(), rPropSet, EXC_CHOBJTYPE_LEGEND );
    // text properties
    mxText.reset( new XclExpChText( GetChRoot() ) );
    mxText->ConvertLegend( rPropSet );

    // legend position and size
    Any aRelPosAny, aRelSizeAny;
    rPropSet.GetAnyProperty( aRelPosAny, EXC_CHPROP_RELATIVEPOSITION );
    rPropSet.GetAnyProperty( aRelSizeAny, EXC_CHPROP_RELATIVESIZE );
    cssc::ChartLegendExpansion eApiExpand = cssc::ChartLegendExpansion_CUSTOM;
    rPropSet.GetProperty( eApiExpand, EXC_CHPROP_EXPANSION );
    if( aRelPosAny.has< RelativePosition >() || ((eApiExpand == cssc::ChartLegendExpansion_CUSTOM) && aRelSizeAny.has< RelativeSize >()) )
    {
        try
        {
            /*  The 'RelativePosition' or 'RelativeSize' properties are used as
                indicator of manually changed legend position/size, but due to
                the different anchor modes used by this property (in the
                RelativePosition.Anchor member) it cannot be used to calculate
                the position easily. For this, the Chart1 API will be used
                instead. */
            Reference< cssc::XChartDocument > xChart1Doc( GetChartDocument(), UNO_QUERY_THROW );
            Reference< XShape > xChart1Legend( xChart1Doc->getLegend(), UNO_SET_THROW );
            // coordinates in CHLEGEND record written but not used by Excel
            mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_CHARTSIZE, EXC_CHFRAMEPOS_PARENT ) );
            XclChFramePos& rFramePos = mxFramePos->GetFramePosData();
            rFramePos.mnTLMode = EXC_CHFRAMEPOS_CHARTSIZE;
            ::com::sun::star::awt::Point aLegendPos = xChart1Legend->getPosition();
            rFramePos.maRect.mnX = maData.maRect.mnX = CalcChartXFromHmm( aLegendPos.X );
            rFramePos.maRect.mnY = maData.maRect.mnY = CalcChartYFromHmm( aLegendPos.Y );
            // legend size, Excel expects points in CHFRAMEPOS record
            rFramePos.mnBRMode = EXC_CHFRAMEPOS_ABSSIZE_POINTS;
            ::com::sun::star::awt::Size aLegendSize = xChart1Legend->getSize();
            rFramePos.maRect.mnWidth = static_cast< sal_uInt16 >( aLegendSize.Width * EXC_POINTS_PER_HMM + 0.5 );
            rFramePos.maRect.mnHeight = static_cast< sal_uInt16 >( aLegendSize.Height * EXC_POINTS_PER_HMM + 0.5 );
            maData.maRect.mnWidth = CalcChartXFromHmm( aLegendSize.Width );
            maData.maRect.mnHeight = CalcChartYFromHmm( aLegendSize.Height );
            eApiExpand = cssc::ChartLegendExpansion_CUSTOM;
            // manual legend position implies manual plot area
            GetChartData().SetManualPlotArea();
            maData.mnDockMode = EXC_CHLEGEND_NOTDOCKED;
            // a CHFRAME record with cleared auto flags is needed
            if( !mxFrame )
                mxFrame.reset( new XclExpChFrame( GetChRoot(), EXC_CHOBJTYPE_LEGEND ) );
            mxFrame->SetAutoFlags( false, false );
        }
        catch( Exception& )
        {
            OSL_FAIL( "XclExpChLegend::Convert - cannot get legend shape" );
            maData.mnDockMode = EXC_CHLEGEND_RIGHT;
            eApiExpand = cssc::ChartLegendExpansion_HIGH;
        }
    }
    else
    {
        cssc2::LegendPosition eApiPos = cssc2::LegendPosition_CUSTOM;
        rPropSet.GetProperty( eApiPos, EXC_CHPROP_ANCHORPOSITION );
        switch( eApiPos )
        {
            case cssc2::LegendPosition_LINE_START:   maData.mnDockMode = EXC_CHLEGEND_LEFT;      break;
            case cssc2::LegendPosition_LINE_END:     maData.mnDockMode = EXC_CHLEGEND_RIGHT;     break;
            case cssc2::LegendPosition_PAGE_START:   maData.mnDockMode = EXC_CHLEGEND_TOP;       break;
            case cssc2::LegendPosition_PAGE_END:     maData.mnDockMode = EXC_CHLEGEND_BOTTOM;    break;
            default:
                OSL_FAIL( "XclExpChLegend::Convert - unrecognized legend position" );
                maData.mnDockMode = EXC_CHLEGEND_RIGHT;
                eApiExpand = cssc::ChartLegendExpansion_HIGH;
        }
    }
    ::set_flag( maData.mnFlags, EXC_CHLEGEND_STACKED, eApiExpand == cssc::ChartLegendExpansion_HIGH );

    // other flags
    ::set_flag( maData.mnFlags, EXC_CHLEGEND_AUTOSERIES );
    const sal_uInt16 nAutoFlags = EXC_CHLEGEND_DOCKED | EXC_CHLEGEND_AUTOPOSX | EXC_CHLEGEND_AUTOPOSY;
    ::set_flag( maData.mnFlags, nAutoFlags, maData.mnDockMode != EXC_CHLEGEND_NOTDOCKED );
}

void XclExpChLegend::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxFramePos );
    lclSaveRecord( rStrm, mxText );
    lclSaveRecord( rStrm, mxFrame );
}

void XclExpChLegend::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.maRect << maData.mnDockMode << maData.mnSpacing << maData.mnFlags;
}

// ----------------------------------------------------------------------------

XclExpChDropBar::XclExpChDropBar( const XclExpChRoot& rRoot, XclChObjectType eObjType ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_DROPBAR, EXC_ID_CHDROPBAR, 2 ),
    meObjType( eObjType ),
    mnBarDist( 100 )
{
}

void XclExpChDropBar::Convert( const ScfPropertySet& rPropSet )
{
    if( rPropSet.Is() )
        ConvertFrameBase( GetChRoot(), rPropSet, meObjType );
    else
        SetDefaultFrameBase( GetChRoot(), EXC_CHFRAMETYPE_INVISIBLE, true );
}

void XclExpChDropBar::WriteSubRecords( XclExpStream& rStrm )
{
    WriteFrameRecords( rStrm );
}

void XclExpChDropBar::WriteBody( XclExpStream& rStrm )
{
    rStrm << mnBarDist;
}

// ----------------------------------------------------------------------------

XclExpChTypeGroup::XclExpChTypeGroup( const XclExpChRoot& rRoot, sal_uInt16 nGroupIdx ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_TYPEGROUP, EXC_ID_CHTYPEGROUP, 20 ),
    maType( rRoot ),
    maTypeInfo( maType.GetTypeInfo() )
{
    maData.mnGroupIdx = nGroupIdx;
}

void XclExpChTypeGroup::ConvertType(
        Reference< XDiagram > xDiagram, Reference< XChartType > xChartType,
        sal_Int32 nApiAxesSetIdx, bool b3dChart, bool bSwappedAxesSet, bool bHasXLabels )
{
    // chart type settings
    maType.Convert( xDiagram, xChartType, nApiAxesSetIdx, bSwappedAxesSet, bHasXLabels );

    // spline - TODO: get from single series (#i66858#)
    ScfPropertySet aTypeProp( xChartType );
    cssc2::CurveStyle eCurveStyle;
    bool bSpline = aTypeProp.GetProperty( eCurveStyle, EXC_CHPROP_CURVESTYLE ) &&
        (eCurveStyle != cssc2::CurveStyle_LINES);

    // extended type info
    maTypeInfo.Set( maType.GetTypeInfo(), b3dChart, bSpline );

    // 3d chart settings
    if( maTypeInfo.mb3dChart )  // only true, if Excel chart supports 3d mode
    {
        mxChart3d.reset( new XclExpChChart3d );
        ScfPropertySet aDiaProp( xDiagram );
        mxChart3d->Convert( aDiaProp, Is3dWallChart() );
    }
}

void XclExpChTypeGroup::ConvertSeries(
        Reference< XDiagram > xDiagram, Reference< XChartType > xChartType,
        sal_Int32 nGroupAxesSetIdx, bool bPercent, bool bConnectBars )
{
    Reference< XDataSeriesContainer > xSeriesCont( xChartType, UNO_QUERY );
    if( xSeriesCont.is() )
    {
        typedef ::std::vector< Reference< XDataSeries > > XDataSeriesVec;
        XDataSeriesVec aSeriesVec;

        // copy data series attached to the current axes set to the vector
        Sequence< Reference< XDataSeries > > aSeriesSeq = xSeriesCont->getDataSeries();
        const Reference< XDataSeries >* pBeg = aSeriesSeq.getConstArray();
        const Reference< XDataSeries >* pEnd = pBeg + aSeriesSeq.getLength();
        for( const Reference< XDataSeries >* pIt = pBeg; pIt != pEnd; ++pIt )
        {
            ScfPropertySet aSeriesProp( *pIt );
            sal_Int32 nSeriesAxesSetIdx(0);
            if( aSeriesProp.GetProperty( nSeriesAxesSetIdx, EXC_CHPROP_ATTAXISINDEX ) && (nSeriesAxesSetIdx == nGroupAxesSetIdx) )
                aSeriesVec.push_back( *pIt );
        }

        // Are there any series in the current axes set?
        if( !aSeriesVec.empty() )
        {
            // stacking direction (stacked/percent/deep 3d) from first series
            ScfPropertySet aSeriesProp( aSeriesVec.front() );
            cssc2::StackingDirection eStacking;
            if( !aSeriesProp.GetProperty( eStacking, EXC_CHPROP_STACKINGDIR ) )
                eStacking = cssc2::StackingDirection_NO_STACKING;

            // stacked or percent chart
            if( maTypeInfo.mbSupportsStacking && (eStacking == cssc2::StackingDirection_Y_STACKING) )
            {
                // percent overrides simple stacking
                maType.SetStacked( bPercent );

                // connected data points (only in stacked bar charts)
                if (bConnectBars && (maTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR))
                {
                    sal_uInt16 nKey = EXC_CHCHARTLINE_CONNECT;
                    maChartLines.insert(nKey, new XclExpChLineFormat(GetChRoot()));
                }
            }
            else
            {
                // reverse series order for some unstacked 2D chart types
                if( maTypeInfo.mbReverseSeries && !Is3dChart() )
                    ::std::reverse( aSeriesVec.begin(), aSeriesVec.end() );
            }

            // deep 3d chart or clustered 3d chart (stacked is not clustered)
            if( (eStacking == cssc2::StackingDirection_NO_STACKING) && Is3dWallChart() )
                mxChart3d->SetClustered();

            // varied point colors
            ::set_flag( maData.mnFlags, EXC_CHTYPEGROUP_VARIEDCOLORS, aSeriesProp.GetBoolProperty( EXC_CHPROP_VARYCOLORSBY ) );

            // process all series
            for( XDataSeriesVec::const_iterator aIt = aSeriesVec.begin(), aEnd = aSeriesVec.end(); aIt != aEnd; ++aIt )
            {
                // create Excel series object, stock charts need special processing
                if( maTypeInfo.meTypeId == EXC_CHTYPEID_STOCK )
                    CreateAllStockSeries( xChartType, *aIt );
                else
                    CreateDataSeries( xDiagram, *aIt );
            }
        }
    }
}

void XclExpChTypeGroup::ConvertCategSequence( Reference< XLabeledDataSequence > xCategSeq )
{
    for( size_t nIdx = 0, nSize = maSeries.GetSize(); nIdx < nSize; ++nIdx )
        maSeries.GetRecord( nIdx )->ConvertCategSequence( xCategSeq );
}

void XclExpChTypeGroup::ConvertLegend( const ScfPropertySet& rPropSet )
{
    if( rPropSet.GetBoolProperty( EXC_CHPROP_SHOW ) )
    {
        mxLegend.reset( new XclExpChLegend( GetChRoot() ) );
        mxLegend->Convert( rPropSet );
    }
}

void XclExpChTypeGroup::WriteSubRecords( XclExpStream& rStrm )
{
    maType.Save( rStrm );
    lclSaveRecord( rStrm, mxChart3d );
    lclSaveRecord( rStrm, mxLegend );
    lclSaveRecord( rStrm, mxUpBar );
    lclSaveRecord( rStrm, mxDownBar );
    for( XclExpChLineFormatMap::iterator aLIt = maChartLines.begin(), aLEnd = maChartLines.end(); aLIt != aLEnd; ++aLIt )
        lclSaveRecord( rStrm, aLIt->second, EXC_ID_CHCHARTLINE, aLIt->first );
}

sal_uInt16 XclExpChTypeGroup::GetFreeFormatIdx() const
{
    return static_cast< sal_uInt16 >( maSeries.GetSize() );
}

void XclExpChTypeGroup::CreateDataSeries(
        Reference< XDiagram > xDiagram, Reference< XDataSeries > xDataSeries )
{
    // let chart create series object with correct series index
    XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
    if( xSeries )
    {
        if( xSeries->ConvertDataSeries( xDiagram, xDataSeries, maTypeInfo, GetGroupIdx(), GetFreeFormatIdx() ) )
            maSeries.AppendRecord( xSeries );
        else
            GetChartData().RemoveLastSeries();
    }
}

void XclExpChTypeGroup::CreateAllStockSeries(
        Reference< XChartType > xChartType, Reference< XDataSeries > xDataSeries )
{
    // create existing series objects
    bool bHasOpen = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_OPENVALUES, false );
    bool bHasHigh = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_HIGHVALUES, false );
    bool bHasLow = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_LOWVALUES, false );
    bool bHasClose = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_CLOSEVALUES, !bHasOpen );

    // formatting of special stock chart elements
    ScfPropertySet aTypeProp( xChartType );
    // hi-lo lines
    if( bHasHigh && bHasLow && aTypeProp.GetBoolProperty( EXC_CHPROP_SHOWHIGHLOW ) )
    {
        ScfPropertySet aSeriesProp( xDataSeries );
        XclExpChLineFormatRef xLineFmt( new XclExpChLineFormat( GetChRoot() ) );
        xLineFmt->Convert( GetChRoot(), aSeriesProp, EXC_CHOBJTYPE_HILOLINE );
        sal_uInt16 nKey = EXC_CHCHARTLINE_HILO;
        maChartLines.insert(nKey, new XclExpChLineFormat(GetChRoot()));
    }
    // dropbars
    if( bHasOpen && bHasClose )
    {
        // dropbar type is dependent on position in the file - always create both
        Reference< XPropertySet > xWhitePropSet, xBlackPropSet;
        // white dropbar format
        aTypeProp.GetProperty( xWhitePropSet, EXC_CHPROP_WHITEDAY );
        ScfPropertySet aWhiteProp( xWhitePropSet );
        mxUpBar.reset( new XclExpChDropBar( GetChRoot(), EXC_CHOBJTYPE_WHITEDROPBAR ) );
        mxUpBar->Convert( aWhiteProp );
        // black dropbar format
        aTypeProp.GetProperty( xBlackPropSet, EXC_CHPROP_BLACKDAY );
        ScfPropertySet aBlackProp( xBlackPropSet );
        mxDownBar.reset( new XclExpChDropBar( GetChRoot(), EXC_CHOBJTYPE_BLACKDROPBAR ) );
        mxDownBar->Convert( aBlackProp );
    }
}

bool XclExpChTypeGroup::CreateStockSeries( Reference< XDataSeries > xDataSeries,
        const OUString& rValueRole, bool bCloseSymbol )
{
    bool bOk = false;
    // let chart create series object with correct series index
    XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
    if( xSeries )
    {
        bOk = xSeries->ConvertStockSeries( xDataSeries,
            rValueRole, GetGroupIdx(), GetFreeFormatIdx(), bCloseSymbol );
        if( bOk )
            maSeries.AppendRecord( xSeries );
        else
            GetChartData().RemoveLastSeries();
    }
    return bOk;
}

void XclExpChTypeGroup::WriteBody( XclExpStream& rStrm )
{
    rStrm.WriteZeroBytes( 16 );
    rStrm << maData.mnFlags << maData.mnGroupIdx;
}

// Axes =======================================================================

XclExpChLabelRange::XclExpChLabelRange( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHLABELRANGE, 8 ),
    XclExpChRoot( rRoot )
{
}

void XclExpChLabelRange::Convert( const ScaleData& rScaleData, const ScfPropertySet& rChart1Axis, bool bMirrorOrient )
{
    /*  Base time unit (using the property 'ExplicitTimeIncrement' from the old
        chart API allows to detect axis type (date axis, if property exists),
        and to receive the base time unit currently used in case the base time
        unit is set to 'automatic'. */
    cssc::TimeIncrement aTimeIncrement;
    if( rChart1Axis.GetProperty( aTimeIncrement, EXC_CHPROP_EXPTIMEINCREMENT ) )
    {
        // property exists -> this is a date axis currently
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_DATEAXIS );

        // automatic base time unit, if the UNO Any 'rScaleData.TimeIncrement.TimeResolution' does not contain a valid value...
        bool bAutoBase = !rScaleData.TimeIncrement.TimeResolution.has< cssc::TimeIncrement >();
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOBASE, bAutoBase );

        // ...but get the current base time unit from the property of the old chart API
        sal_Int32 nApiTimeUnit = 0;
        bool bValidBaseUnit = aTimeIncrement.TimeResolution >>= nApiTimeUnit;
        OSL_ENSURE( bValidBaseUnit, "XclExpChLabelRange::Convert - cannot ghet base time unit" );
        maDateData.mnBaseUnit = bValidBaseUnit ? lclGetTimeUnit( nApiTimeUnit ) : EXC_CHDATERANGE_DAYS;

        /*  Min/max values depend on base time unit, they specify the number of
            days, months, or years starting from null date. */
        bool bAutoMin = lclConvertTimeValue( GetRoot(), maDateData.mnMinDate, rScaleData.Minimum, maDateData.mnBaseUnit );
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMIN, bAutoMin );
        bool bAutoMax = lclConvertTimeValue( GetRoot(), maDateData.mnMaxDate, rScaleData.Maximum, maDateData.mnBaseUnit );
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMAX, bAutoMax );
    }

    // automatic axis type detection
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTODATE, rScaleData.AutoDateAxis );

    // increment
    bool bAutoMajor = lclConvertTimeInterval( maDateData.mnMajorStep, maDateData.mnMajorUnit, rScaleData.TimeIncrement.MajorTimeInterval );
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMAJOR, bAutoMajor );
    bool bAutoMinor = lclConvertTimeInterval( maDateData.mnMinorStep, maDateData.mnMinorUnit, rScaleData.TimeIncrement.MinorTimeInterval );
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMINOR, bAutoMinor );

    // origin
    double fOrigin = 0.0;
    if( !lclIsAutoAnyOrGetValue( fOrigin, rScaleData.Origin ) )
        maLabelData.mnCross = limit_cast< sal_uInt16 >( fOrigin, 1, 31999 );

    // reverse order
    if( (rScaleData.Orientation == cssc2::AxisOrientation_REVERSE) != bMirrorOrient )
        ::set_flag( maLabelData.mnFlags, EXC_CHLABELRANGE_REVERSE );
}

void XclExpChLabelRange::ConvertAxisPosition( const ScfPropertySet& rPropSet )
{
    cssc::ChartAxisPosition eAxisPos = cssc::ChartAxisPosition_VALUE;
    rPropSet.GetProperty( eAxisPos, EXC_CHPROP_CROSSOVERPOSITION );
    double fCrossingPos = 1.0;
    rPropSet.GetProperty( fCrossingPos, EXC_CHPROP_CROSSOVERVALUE );

    bool bDateAxis = ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_DATEAXIS );
    switch( eAxisPos )
    {
        case cssc::ChartAxisPosition_ZERO:
        case cssc::ChartAxisPosition_START:
            maLabelData.mnCross = 1;
            ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOCROSS );
        break;
        case cssc::ChartAxisPosition_END:
            ::set_flag( maLabelData.mnFlags, EXC_CHLABELRANGE_MAXCROSS );
        break;
        case cssc::ChartAxisPosition_VALUE:
            maLabelData.mnCross = limit_cast< sal_uInt16 >( fCrossingPos, 1, 31999 );
            ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOCROSS, false );
            if( bDateAxis )
                maDateData.mnCross = lclGetTimeValue( GetRoot(), fCrossingPos, maDateData.mnBaseUnit );
        break;
        default:
            maLabelData.mnCross = 1;
            ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOCROSS );
    }
}

void XclExpChLabelRange::Save( XclExpStream& rStrm )
{
    // the CHLABELRANGE record
    XclExpRecord::Save( rStrm );

    // the CHDATERANGE record with date axis settings (BIFF8 only)
    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm.StartRecord( EXC_ID_CHDATERANGE, 18 );
        rStrm   << maDateData.mnMinDate
                << maDateData.mnMaxDate
                << maDateData.mnMajorStep
                << maDateData.mnMajorUnit
                << maDateData.mnMinorStep
                << maDateData.mnMinorUnit
                << maDateData.mnBaseUnit
                << maDateData.mnCross
                << maDateData.mnFlags;
        rStrm.EndRecord();
    }
}

void XclExpChLabelRange::WriteBody( XclExpStream& rStrm )
{
    rStrm << maLabelData.mnCross << maLabelData.mnLabelFreq << maLabelData.mnTickFreq << maLabelData.mnFlags;
}

// ----------------------------------------------------------------------------

XclExpChValueRange::XclExpChValueRange( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHVALUERANGE, 42 ),
    XclExpChRoot( rRoot )
{
}

void XclExpChValueRange::Convert( const ScaleData& rScaleData )
{
    // scaling algorithm
    bool bLogScale = ScfApiHelper::GetServiceName( rScaleData.Scaling ) == SERVICE_CHART2_LOGSCALING;
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE, bLogScale );

    // min/max
    bool bAutoMin = lclIsAutoAnyOrGetScaledValue( maData.mfMin, rScaleData.Minimum, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN, bAutoMin );
    bool bAutoMax = lclIsAutoAnyOrGetScaledValue( maData.mfMax, rScaleData.Maximum, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX, bAutoMax );

    // origin
    bool bAutoCross = lclIsAutoAnyOrGetScaledValue( maData.mfCross, rScaleData.Origin, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS, bAutoCross );

    // major increment
    const IncrementData& rIncrementData = rScaleData.IncrementData;
    bool bAutoMajor = lclIsAutoAnyOrGetValue( maData.mfMajorStep, rIncrementData.Distance ) || (maData.mfMajorStep <= 0.0);
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR, bAutoMajor );
    // minor increment
    const Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
    sal_Int32 nCount = 0;
    bool bAutoMinor = bLogScale || bAutoMajor || (rSubIncrementSeq.getLength() < 1) ||
        lclIsAutoAnyOrGetValue( nCount, rSubIncrementSeq[ 0 ].IntervalCount ) || (nCount < 1);
    if( !bAutoMinor )
        maData.mfMinorStep = maData.mfMajorStep / nCount;
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR, bAutoMinor );

    // reverse order
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_REVERSE, rScaleData.Orientation == cssc2::AxisOrientation_REVERSE );
}

void XclExpChValueRange::ConvertAxisPosition( const ScfPropertySet& rPropSet )
{
    cssc::ChartAxisPosition eAxisPos = cssc::ChartAxisPosition_VALUE;
    double fCrossingPos = 0.0;
    if( rPropSet.GetProperty( eAxisPos, EXC_CHPROP_CROSSOVERPOSITION ) && rPropSet.GetProperty( fCrossingPos, EXC_CHPROP_CROSSOVERVALUE ) )
    {
        switch( eAxisPos )
        {
            case cssc::ChartAxisPosition_ZERO:
            case cssc::ChartAxisPosition_START:
                ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS );
            break;
            case cssc::ChartAxisPosition_END:
                ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_MAXCROSS );
            break;
            case cssc::ChartAxisPosition_VALUE:
                ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS, false );
                maData.mfCross = ::get_flagvalue< double >( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE, log( fCrossingPos ) / log( 10.0 ), fCrossingPos );
            break;
            default:
                ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS );
        }
    }
}

void XclExpChValueRange::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mfMin
            << maData.mfMax
            << maData.mfMajorStep
            << maData.mfMinorStep
            << maData.mfCross
            << maData.mnFlags;
}

// ----------------------------------------------------------------------------

namespace {

sal_uInt8 lclGetXclTickPos( sal_Int32 nApiTickmarks )
{
    using namespace cssc2::TickmarkStyle;
    sal_uInt8 nXclTickPos = 0;
    ::set_flag( nXclTickPos, EXC_CHTICK_INSIDE,  ::get_flag( nApiTickmarks, INNER ) );
    ::set_flag( nXclTickPos, EXC_CHTICK_OUTSIDE, ::get_flag( nApiTickmarks, OUTER ) );
    return nXclTickPos;
}

} // namespace

XclExpChTick::XclExpChTick( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHTICK, (rRoot.GetBiff() == EXC_BIFF8) ? 30 : 26 ),
    XclExpChRoot( rRoot ),
    mnTextColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) )
{
}

void XclExpChTick::Convert( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo, sal_uInt16 nAxisType )
{
    // tick mark style
    sal_Int32 nApiTickmarks = 0;
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MAJORTICKS ) )
        maData.mnMajor = lclGetXclTickPos( nApiTickmarks );
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MINORTICKS ) )
        maData.mnMinor = lclGetXclTickPos( nApiTickmarks );

    // axis labels
    if( (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR) && (nAxisType == EXC_CHAXIS_X) )
    {
        /*  Radar charts disable their category labels via chart type, not via
            axis, and axis labels are always 'near axis'. */
        maData.mnLabelPos = EXC_CHTICK_NEXT;
    }
    else if( !rPropSet.GetBoolProperty( EXC_CHPROP_DISPLAYLABELS ) )
    {
        // no labels
        maData.mnLabelPos = EXC_CHTICK_NOLABEL;
    }
    else if( rTypeInfo.mb3dChart && (nAxisType == EXC_CHAXIS_Y) )
    {
        // Excel expects 'near axis' at Y axes in 3D charts
        maData.mnLabelPos = EXC_CHTICK_NEXT;
    }
    else
    {
        cssc::ChartAxisLabelPosition eApiLabelPos = cssc::ChartAxisLabelPosition_NEAR_AXIS;
        rPropSet.GetProperty( eApiLabelPos, EXC_CHPROP_LABELPOSITION );
        switch( eApiLabelPos )
        {
            case cssc::ChartAxisLabelPosition_NEAR_AXIS:
            case cssc::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE: maData.mnLabelPos = EXC_CHTICK_NEXT;    break;
            case cssc::ChartAxisLabelPosition_OUTSIDE_START:        maData.mnLabelPos = EXC_CHTICK_LOW;     break;
            case cssc::ChartAxisLabelPosition_OUTSIDE_END:          maData.mnLabelPos = EXC_CHTICK_HIGH;    break;
            default:                                                maData.mnLabelPos = EXC_CHTICK_NEXT;
        }
    }
}

void XclExpChTick::SetFontColor( const Color& rColor, sal_uInt32 nColorId )
{
    maData.maTextColor = rColor;
    ::set_flag( maData.mnFlags, EXC_CHTICK_AUTOCOLOR, rColor == COL_AUTO );
    mnTextColorId = nColorId;
}

void XclExpChTick::SetRotation( sal_uInt16 nRotation )
{
    maData.mnRotation = nRotation;
    ::set_flag( maData.mnFlags, EXC_CHTICK_AUTOROT, false );
    ::insert_value( maData.mnFlags, XclTools::GetXclOrientFromRot( nRotation ), 2, 3 );
}

void XclExpChTick::WriteBody( XclExpStream& rStrm )
{
    rStrm   << maData.mnMajor
            << maData.mnMinor
            << maData.mnLabelPos
            << maData.mnBackMode;
    rStrm.WriteZeroBytes( 16 );
    rStrm   << maData.maTextColor
            << maData.mnFlags;
    if( GetBiff() == EXC_BIFF8 )
        rStrm << GetPalette().GetColorIndex( mnTextColorId ) << maData.mnRotation;
}

// ----------------------------------------------------------------------------

namespace {

/** Returns an API axis object from the passed coordinate system. */
Reference< XAxis > lclGetApiAxis( Reference< XCoordinateSystem > xCoordSystem,
        sal_Int32 nApiAxisDim, sal_Int32 nApiAxesSetIdx )
{
    Reference< XAxis > xAxis;
    if( (nApiAxisDim >= 0) && xCoordSystem.is() ) try
    {
        xAxis = xCoordSystem->getAxisByDimension( nApiAxisDim, nApiAxesSetIdx );
    }
    catch( Exception& )
    {
    }
    return xAxis;
}

Reference< cssc::XAxis > lclGetApiChart1Axis( Reference< XChartDocument > xChartDoc,
        sal_Int32 nApiAxisDim, sal_Int32 nApiAxesSetIdx )
{
    Reference< cssc::XAxis > xChart1Axis;
    try
    {
        Reference< cssc::XChartDocument > xChart1Doc( xChartDoc, UNO_QUERY_THROW );
        Reference< cssc::XAxisSupplier > xChart1AxisSupp( xChart1Doc->getDiagram(), UNO_QUERY_THROW );
        switch( nApiAxesSetIdx )
        {
            case EXC_CHART_AXESSET_PRIMARY:
                xChart1Axis = xChart1AxisSupp->getAxis( nApiAxisDim );
            break;
            case EXC_CHART_AXESSET_SECONDARY:
                xChart1Axis = xChart1AxisSupp->getSecondaryAxis( nApiAxisDim );
            break;
        }
    }
    catch( Exception& )
    {
    }
    return xChart1Axis;
}

} // namespace

XclExpChAxis::XclExpChAxis( const XclExpChRoot& rRoot, sal_uInt16 nAxisType ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_AXIS, EXC_ID_CHAXIS, 18 ),
    mnNumFmtIdx( EXC_FORMAT_NOTFOUND )
{
    maData.mnType = nAxisType;
}

void XclExpChAxis::SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId )
{
    mxFont = xFont;
    if( mxTick )
        mxTick->SetFontColor( rColor, nColorId );
}

void XclExpChAxis::SetRotation( sal_uInt16 nRotation )
{
    if( mxTick )
        mxTick->SetRotation( nRotation );
}

void XclExpChAxis::Convert( Reference< XAxis > xAxis, Reference< XAxis > xCrossingAxis,
        Reference< cssc::XAxis > xChart1Axis, const XclChExtTypeInfo& rTypeInfo )
{
    ScfPropertySet aAxisProp( xAxis );
    bool bCategoryAxis = ((GetAxisType() == EXC_CHAXIS_X) && rTypeInfo.mbCategoryAxis) || (GetAxisType() == EXC_CHAXIS_Z);

    // axis line format -------------------------------------------------------

    mxAxisLine.reset( new XclExpChLineFormat( GetChRoot() ) );
    mxAxisLine->Convert( GetChRoot(), aAxisProp, EXC_CHOBJTYPE_AXISLINE );
    // #i58688# axis enabled
    mxAxisLine->SetShowAxis( aAxisProp.GetBoolProperty( EXC_CHPROP_SHOW ) );

    // axis scaling and increment ---------------------------------------------

    ScfPropertySet aCrossingProp( xCrossingAxis );
    if( bCategoryAxis )
    {
        mxLabelRange.reset( new XclExpChLabelRange( GetChRoot() ) );
        mxLabelRange->SetTicksBetweenCateg( rTypeInfo.mbTicksBetweenCateg );
        if( xAxis.is() )
        {
            ScfPropertySet aChart1AxisProp( xChart1Axis );
            // #i71684# radar charts have reversed rotation direction
            mxLabelRange->Convert( xAxis->getScaleData(), aChart1AxisProp, (GetAxisType() == EXC_CHAXIS_X) && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR) );
        }
        // get position of crossing axis on this axis from passed axis object
        if( aCrossingProp.Is() )
            mxLabelRange->ConvertAxisPosition( aCrossingProp );
    }
    else
    {
        mxValueRange.reset( new XclExpChValueRange( GetChRoot() ) );
        if( xAxis.is() )
            mxValueRange->Convert( xAxis->getScaleData() );
        // get position of crossing axis on this axis from passed axis object
        if( aCrossingProp.Is() )
            mxValueRange->ConvertAxisPosition( aCrossingProp );
    }

    // axis caption text ------------------------------------------------------

    // axis ticks properties
    mxTick.reset( new XclExpChTick( GetChRoot() ) );
    mxTick->Convert( aAxisProp, rTypeInfo, GetAxisType() );

    // axis label formatting and rotation
    ConvertFontBase( GetChRoot(), aAxisProp );
    ConvertRotationBase( GetChRoot(), aAxisProp, true );

    // axis number format
    sal_Int32 nApiNumFmt = 0;
    if( !bCategoryAxis && aAxisProp.GetProperty( nApiNumFmt, EXC_CHPROP_NUMBERFORMAT ) )
        mnNumFmtIdx = GetNumFmtBuffer().Insert( static_cast< sal_uInt32 >( nApiNumFmt ) );

    // grid -------------------------------------------------------------------

    if( xAxis.is() )
    {
        // main grid
        ScfPropertySet aGridProp( xAxis->getGridProperties() );
        if( aGridProp.GetBoolProperty( EXC_CHPROP_SHOW ) )
            mxMajorGrid = lclCreateLineFormat( GetChRoot(), aGridProp, EXC_CHOBJTYPE_GRIDLINE );
        // sub grid
        Sequence< Reference< XPropertySet > > aSubGridPropSeq = xAxis->getSubGridProperties();
        if( aSubGridPropSeq.hasElements() )
        {
            ScfPropertySet aSubGridProp( aSubGridPropSeq[ 0 ] );
            if( aSubGridProp.GetBoolProperty( EXC_CHPROP_SHOW ) )
                mxMinorGrid = lclCreateLineFormat( GetChRoot(), aSubGridProp, EXC_CHOBJTYPE_GRIDLINE );
        }
    }
}

void XclExpChAxis::ConvertWall( XDiagramRef xDiagram )
{
    if( xDiagram.is() ) switch( GetAxisType() )
    {
        case EXC_CHAXIS_X:
        {
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxWallFrame = lclCreateFrame( GetChRoot(), aWallProp, EXC_CHOBJTYPE_WALL3D );
        }
        break;
        case EXC_CHAXIS_Y:
        {
            ScfPropertySet aFloorProp( xDiagram->getFloor() );
            mxWallFrame = lclCreateFrame( GetChRoot(), aFloorProp, EXC_CHOBJTYPE_FLOOR3D );
        }
        break;
        default:
            mxWallFrame.reset();
    }
}

void XclExpChAxis::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxLabelRange );
    lclSaveRecord( rStrm, mxValueRange );
    if( mnNumFmtIdx != EXC_FORMAT_NOTFOUND )
        XclExpUInt16Record( EXC_ID_CHFORMAT, mnNumFmtIdx ).Save( rStrm );
    lclSaveRecord( rStrm, mxTick );
    lclSaveRecord( rStrm, mxFont );
    lclSaveRecord( rStrm, mxAxisLine, EXC_ID_CHAXISLINE, EXC_CHAXISLINE_AXISLINE );
    lclSaveRecord( rStrm, mxMajorGrid, EXC_ID_CHAXISLINE, EXC_CHAXISLINE_MAJORGRID );
    lclSaveRecord( rStrm, mxMinorGrid, EXC_ID_CHAXISLINE, EXC_CHAXISLINE_MINORGRID );
    lclSaveRecord( rStrm, mxWallFrame, EXC_ID_CHAXISLINE, EXC_CHAXISLINE_WALLS );
}

void XclExpChAxis::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnType;
    rStrm.WriteZeroBytes( 16 );
}

// ----------------------------------------------------------------------------

XclExpChAxesSet::XclExpChAxesSet( const XclExpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_AXESSET, EXC_ID_CHAXESSET, 18 )
{
    maData.mnAxesSetId = nAxesSetId;
    SetFutureRecordContext( 0, nAxesSetId );

    /*  Need to set a reasonable size for the plot area, otherwise Excel will
        move away embedded shapes while auto-sizing the plot area. This is just
        a wild guess, but will be fixed with implementing manual positioning of
        chart elements. */
    maData.maRect.mnX = 262;
    maData.maRect.mnY = 626;
    maData.maRect.mnWidth = 3187;
    maData.maRect.mnHeight = 2633;
}

sal_uInt16 XclExpChAxesSet::Convert( Reference< XDiagram > xDiagram, sal_uInt16 nFirstGroupIdx )
{
    /*  First unused chart type group index is passed to be able to continue
        counting of chart type groups for secondary axes set. */
    sal_uInt16 nGroupIdx = nFirstGroupIdx;
    Reference< XCoordinateSystemContainer > xCoordSysCont( xDiagram, UNO_QUERY );
    if( xCoordSysCont.is() )
    {
        Sequence< Reference< XCoordinateSystem > > aCoordSysSeq = xCoordSysCont->getCoordinateSystems();
        if( aCoordSysSeq.getLength() > 0 )
        {
            /*  Process first coordinate system only. Import filter puts all
                chart types into one coordinate system. */
            Reference< XCoordinateSystem > xCoordSystem = aCoordSysSeq[ 0 ];
            sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();

            // 3d mode
            bool b3dChart = xCoordSystem.is() && (xCoordSystem->getDimension() == 3);

            // percent charts
            namespace ApiAxisType = cssc2::AxisType;
            Reference< XAxis > xApiYAxis = lclGetApiAxis( xCoordSystem, EXC_CHART_AXIS_Y, nApiAxesSetIdx );
            bool bPercent = xApiYAxis.is() && (xApiYAxis->getScaleData().AxisType == ApiAxisType::PERCENT);

            // connector lines in bar charts
            ScfPropertySet aDiaProp( xDiagram );
            bool bConnectBars = aDiaProp.GetBoolProperty( EXC_CHPROP_CONNECTBARS );

            // swapped axes sets
            ScfPropertySet aCoordSysProp( xCoordSystem );
            bool bSwappedAxesSet = aCoordSysProp.GetBoolProperty( EXC_CHPROP_SWAPXANDYAXIS );

            // X axis for later use
            Reference< XAxis > xApiXAxis = lclGetApiAxis( xCoordSystem, EXC_CHART_AXIS_X, nApiAxesSetIdx );
            // X axis labels
            ScfPropertySet aXAxisProp( xApiXAxis );
            bool bHasXLabels = aXAxisProp.GetBoolProperty( EXC_CHPROP_DISPLAYLABELS );

            // process chart types
            Reference< XChartTypeContainer > xChartTypeCont( xCoordSystem, UNO_QUERY );
            if( xChartTypeCont.is() )
            {
                Sequence< Reference< XChartType > > aChartTypeSeq = xChartTypeCont->getChartTypes();
                const Reference< XChartType >* pBeg = aChartTypeSeq.getConstArray();
                const Reference< XChartType >* pEnd = pBeg + aChartTypeSeq.getLength();
                for( const Reference< XChartType >* pIt = pBeg; pIt != pEnd; ++pIt )
                {
                    XclExpChTypeGroupRef xTypeGroup( new XclExpChTypeGroup( GetChRoot(), nGroupIdx ) );
                    xTypeGroup->ConvertType( xDiagram, *pIt, nApiAxesSetIdx, b3dChart, bSwappedAxesSet, bHasXLabels );
                    /*  If new chart type group cannot be inserted into a combination
                        chart with existing type groups, insert all series into last
                        contained chart type group instead of creating a new group. */
                    XclExpChTypeGroupRef xLastGroup = GetLastTypeGroup();
                    if( xLastGroup && !(xTypeGroup->IsCombinable2d() && xLastGroup->IsCombinable2d()) )
                    {
                        xLastGroup->ConvertSeries( xDiagram, *pIt, nApiAxesSetIdx, bPercent, bConnectBars );
                    }
                    else
                    {
                        xTypeGroup->ConvertSeries( xDiagram, *pIt, nApiAxesSetIdx, bPercent, bConnectBars );
                        if( xTypeGroup->IsValidGroup() )
                        {
                            maTypeGroups.AppendRecord( xTypeGroup );
                            ++nGroupIdx;
                        }
                    }
                }
            }

            if( XclExpChTypeGroup* pGroup = GetFirstTypeGroup().get() )
            {
                const XclChExtTypeInfo& rTypeInfo = pGroup->GetTypeInfo();

                // create axes according to chart type (no axes for pie and donut charts)
                if( rTypeInfo.meTypeCateg != EXC_CHTYPECATEG_PIE )
                {
                    ConvertAxis( mxXAxis, EXC_CHAXIS_X, mxXAxisTitle, EXC_CHOBJLINK_XAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_Y );
                    ConvertAxis( mxYAxis, EXC_CHAXIS_Y, mxYAxisTitle, EXC_CHOBJLINK_YAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_X );
                    if( pGroup->Is3dDeepChart() )
                        ConvertAxis( mxZAxis, EXC_CHAXIS_Z, mxZAxisTitle, EXC_CHOBJLINK_ZAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_NONE );
                }

                // X axis category ranges
                if( rTypeInfo.mbCategoryAxis && xApiXAxis.is() )
                {
                    const ScaleData aScaleData = xApiXAxis->getScaleData();
                    for( size_t nIdx = 0, nSize = maTypeGroups.GetSize(); nIdx < nSize; ++nIdx )
                        maTypeGroups.GetRecord( nIdx )->ConvertCategSequence( aScaleData.Categories );
                }

                // legend
                if( xDiagram.is() && (GetAxesSetId() == EXC_CHAXESSET_PRIMARY) )
                {
                    Reference< XLegend > xLegend = xDiagram->getLegend();
                    if( xLegend.is() )
                    {
                        ScfPropertySet aLegendProp( xLegend );
                        pGroup->ConvertLegend( aLegendProp );
                    }
                }
            }
        }
    }

    // wall/floor/diagram frame formatting
    if( xDiagram.is() && (GetAxesSetId() == EXC_CHAXESSET_PRIMARY) )
    {
        XclExpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
        if( xTypeGroup && xTypeGroup->Is3dWallChart() )
        {
            // wall/floor formatting (3D charts)
            if( mxXAxis )
                mxXAxis->ConvertWall( xDiagram );
            if( mxYAxis )
                mxYAxis->ConvertWall( xDiagram );
        }
        else
        {
            // diagram background formatting
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxPlotFrame = lclCreateFrame( GetChRoot(), aWallProp, EXC_CHOBJTYPE_PLOTFRAME );
        }
    }

    // inner and outer plot area position and size
    try
    {
        Reference< cssc::XChartDocument > xChart1Doc( GetChartDocument(), UNO_QUERY_THROW );
        Reference< cssc::XDiagramPositioning > xPositioning( xChart1Doc->getDiagram(), UNO_QUERY_THROW );
        // set manual flag in chart data
        if( !xPositioning->isAutomaticDiagramPositioning() )
            GetChartData().SetManualPlotArea();
        // the CHAXESSET record contains the inner plot area
        maData.maRect = CalcChartRectFromHmm( xPositioning->calculateDiagramPositionExcludingAxes() );
        // the embedded CHFRAMEPOS record contains the outer plot area
        mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_PARENT, EXC_CHFRAMEPOS_PARENT ) );
        // for pie charts, always use inner plot area size to exclude the data labels as Excel does
        const XclExpChTypeGroup* pFirstTypeGroup = GetFirstTypeGroup().get();
        bool bPieChart = pFirstTypeGroup && (pFirstTypeGroup->GetTypeInfo().meTypeCateg == EXC_CHTYPECATEG_PIE);
        mxFramePos->GetFramePosData().maRect = bPieChart ? maData.maRect :
            CalcChartRectFromHmm( xPositioning->calculateDiagramPositionIncludingAxes() );
    }
    catch( Exception& )
    {
    }

    // return first unused chart type group index for next axes set
    return nGroupIdx;
}

bool XclExpChAxesSet::Is3dChart() const
{
    XclExpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
    return xTypeGroup && xTypeGroup->Is3dChart();
}

void XclExpChAxesSet::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxFramePos );
    lclSaveRecord( rStrm, mxXAxis );
    lclSaveRecord( rStrm, mxYAxis );
    lclSaveRecord( rStrm, mxZAxis );
    lclSaveRecord( rStrm, mxXAxisTitle );
    lclSaveRecord( rStrm, mxYAxisTitle );
    lclSaveRecord( rStrm, mxZAxisTitle );
    if( mxPlotFrame )
    {
        XclExpEmptyRecord( EXC_ID_CHPLOTFRAME ).Save( rStrm );
        mxPlotFrame->Save( rStrm );
    }
    maTypeGroups.Save( rStrm );
}

XclExpChTypeGroupRef XclExpChAxesSet::GetFirstTypeGroup() const
{
    return maTypeGroups.GetFirstRecord();
}

XclExpChTypeGroupRef XclExpChAxesSet::GetLastTypeGroup() const
{
    return maTypeGroups.GetLastRecord();
}

void XclExpChAxesSet::ConvertAxis(
        XclExpChAxisRef& rxChAxis, sal_uInt16 nAxisType,
        XclExpChTextRef& rxChAxisTitle, sal_uInt16 nTitleTarget,
        Reference< XCoordinateSystem > xCoordSystem, const XclChExtTypeInfo& rTypeInfo,
        sal_Int32 nCrossingAxisDim )
{
    // create and convert axis object
    rxChAxis.reset( new XclExpChAxis( GetChRoot(), nAxisType ) );
    sal_Int32 nApiAxisDim = rxChAxis->GetApiAxisDimension();
    sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();
    Reference< XAxis > xAxis = lclGetApiAxis( xCoordSystem, nApiAxisDim, nApiAxesSetIdx );
    Reference< XAxis > xCrossingAxis = lclGetApiAxis( xCoordSystem, nCrossingAxisDim, nApiAxesSetIdx );
    Reference< cssc::XAxis > xChart1Axis = lclGetApiChart1Axis( GetChartDocument(), nApiAxisDim, nApiAxesSetIdx );
    rxChAxis->Convert( xAxis, xCrossingAxis, xChart1Axis, rTypeInfo );

    // create and convert axis title
    Reference< XTitled > xTitled( xAxis, UNO_QUERY );
    rxChAxisTitle = lclCreateTitle( GetChRoot(), xTitled, nTitleTarget );
}

void XclExpChAxesSet::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnAxesSetId << maData.maRect;
}

// The chart object ===========================================================

static void lcl_getChartSubTitle(const Reference<XChartDocument>& xChartDoc,
                                 String& rSubTitle)
{
    Reference< ::com::sun::star::chart::XChartDocument > xChartDoc1(xChartDoc, UNO_QUERY);
    if (!xChartDoc1.is())
        return;

    Reference< XPropertySet > xProp(xChartDoc1->getSubTitle(), UNO_QUERY);
    if (!xProp.is())
        return;

    OUString aTitle;
    Any any = xProp->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("String")) );
    if (any >>= aTitle)
        rSubTitle = aTitle;
}

XclExpChChart::XclExpChChart( const XclExpRoot& rRoot,
        Reference< XChartDocument > xChartDoc, const Rectangle& rChartRect ) :
    XclExpChGroupBase( XclExpChRoot( rRoot, *this ), EXC_CHFRBLOCK_TYPE_CHART, EXC_ID_CHCHART, 16 )
{
    Size aPtSize = OutputDevice::LogicToLogic( rChartRect.GetSize(), MapMode( MAP_100TH_MM ), MapMode( MAP_POINT ) );
    // rectangle is stored in 16.16 fixed-point format
    maRect.mnX = maRect.mnY = 0;
    maRect.mnWidth = static_cast< sal_Int32 >( aPtSize.Width() << 16 );
    maRect.mnHeight = static_cast< sal_Int32 >( aPtSize.Height() << 16 );

    // global chart properties (default values)
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_SHOWVISIBLEONLY, false );
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_MANPLOTAREA );
    maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_SKIP;

    // always create both axes set objects
    mxPrimAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_PRIMARY ) );
    mxSecnAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_SECONDARY ) );

    if( xChartDoc.is() )
    {
        Reference< XDiagram > xDiagram = xChartDoc->getFirstDiagram();

        // global chart properties (only 'include hidden cells' attribute for now)
        ScfPropertySet aDiagramProp( xDiagram );
        bool bIncludeHidden = aDiagramProp.GetBoolProperty( EXC_CHPROP_INCLUDEHIDDENCELLS );
        ::set_flag( maProps.mnFlags,  EXC_CHPROPS_SHOWVISIBLEONLY, !bIncludeHidden );

        // initialize API conversion (remembers xChartDoc and rChartRect internally)
        InitConversion( xChartDoc, rChartRect );

        // chart frame
        ScfPropertySet aFrameProp( xChartDoc->getPageBackground() );
        mxFrame = lclCreateFrame( GetChRoot(), aFrameProp, EXC_CHOBJTYPE_BACKGROUND );

        // chart title
        Reference< XTitled > xTitled( xChartDoc, UNO_QUERY );
        String aSubTitle;
        lcl_getChartSubTitle(xChartDoc, aSubTitle);
        mxTitle = lclCreateTitle( GetChRoot(), xTitled, EXC_CHOBJLINK_TITLE,
                                  aSubTitle.Len() ? &aSubTitle : NULL );

        // diagrams (axes sets)
        sal_uInt16 nFreeGroupIdx = mxPrimAxesSet->Convert( xDiagram, 0 );
        if( !mxPrimAxesSet->Is3dChart() )
            mxSecnAxesSet->Convert( xDiagram, nFreeGroupIdx );

        // treatment of missing values
        ScfPropertySet aDiaProp( xDiagram );
        sal_Int32 nMissingValues = 0;
        if( aDiaProp.GetProperty( nMissingValues, EXC_CHPROP_MISSINGVALUETREATMENT ) )
        {
            using namespace cssc::MissingValueTreatment;
            switch( nMissingValues )
            {
                case LEAVE_GAP: maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_SKIP;           break;
                case USE_ZERO:  maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_ZERO;           break;
                case CONTINUE:  maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_INTERPOLATE;    break;
            }
        }

        // finish API conversion
        FinishConversion();
    }
}

XclExpChSeriesRef XclExpChChart::CreateSeries()
{
    XclExpChSeriesRef xSeries;
    sal_uInt16 nSeriesIdx = static_cast< sal_uInt16 >( maSeries.GetSize() );
    if( nSeriesIdx <= EXC_CHSERIES_MAXSERIES )
    {
        xSeries.reset( new XclExpChSeries( GetChRoot(), nSeriesIdx ) );
        maSeries.AppendRecord( xSeries );
    }
    return xSeries;
}

void XclExpChChart::RemoveLastSeries()
{
    if( !maSeries.IsEmpty() )
        maSeries.RemoveRecord( maSeries.GetSize() - 1 );
}

void XclExpChChart::SetDataLabel( XclExpChTextRef xText )
{
    if( xText )
        maLabels.AppendRecord( xText );
}

void XclExpChChart::SetManualPlotArea()
{
    // this flag does not exist in BIFF5
    if( GetBiff() == EXC_BIFF8 )
        ::set_flag( maProps.mnFlags, EXC_CHPROPS_USEMANPLOTAREA );
}

void XclExpChChart::WriteSubRecords( XclExpStream& rStrm )
{
    // background format
    lclSaveRecord( rStrm, mxFrame );

    // data series
    maSeries.Save( rStrm );

    // CHPROPERTIES record
    rStrm.StartRecord( EXC_ID_CHPROPERTIES, 4 );
    rStrm << maProps.mnFlags << maProps.mnEmptyMode << sal_uInt8( 0 );
    rStrm.EndRecord();

    // axes sets (always save primary axes set)
    sal_uInt16 nUsedAxesSets = mxSecnAxesSet->IsValidAxesSet() ? 2 : 1;
    XclExpUInt16Record( EXC_ID_CHUSEDAXESSETS, nUsedAxesSets ).Save( rStrm );
    mxPrimAxesSet->Save( rStrm );
    if( mxSecnAxesSet->IsValidAxesSet() )
        mxSecnAxesSet->Save( rStrm );

    // chart title and data labels
    lclSaveRecord( rStrm, mxTitle );
    maLabels.Save( rStrm );
}

void XclExpChChart::WriteBody( XclExpStream& rStrm )
{
     rStrm << maRect;
}

// ----------------------------------------------------------------------------

XclExpChartDrawing::XclExpChartDrawing( const XclExpRoot& rRoot,
        const Reference< XModel >& rxModel, const Size& rChartSize ) :
    XclExpRoot( rRoot )
{
    if( (rChartSize.Width() > 0) && (rChartSize.Height() > 0) )
    {
        ScfPropertySet aPropSet( rxModel );
        Reference< XShapes > xShapes;
        if( aPropSet.GetProperty( xShapes, EXC_CHPROP_ADDITIONALSHAPES ) && xShapes.is() && (xShapes->getCount() > 0) )
        {
            /*  Create a new independent object manager with own DFF stream for the
                DGCONTAINER, pass global manager as parent for shared usage of
                global DFF data (picture container etc.). */
            mxObjMgr.reset( new XclExpEmbeddedObjectManager( GetObjectManager(), rChartSize, EXC_CHART_TOTALUNITS, EXC_CHART_TOTALUNITS ) );
            // initialize the drawing object list
            mxObjMgr->StartSheet();
            // process the draw page (convert all shapes)
            mxObjRecs = mxObjMgr->ProcessDrawing( xShapes );
            // finalize the DFF stream
            mxObjMgr->EndDocument();
        }
    }
}

XclExpChartDrawing::~XclExpChartDrawing()
{
}

void XclExpChartDrawing::Save( XclExpStream& rStrm )
{
    if( mxObjRecs )
        mxObjRecs->Save( rStrm );
}

// ----------------------------------------------------------------------------

XclExpChart::XclExpChart( const XclExpRoot& rRoot, Reference< XModel > xModel, const Rectangle& rChartRect ) :
    XclExpSubStream( EXC_BOF_CHART ),
    XclExpRoot( rRoot )
{
    AppendNewRecord( new XclExpChartPageSettings( rRoot ) );
    AppendNewRecord( new XclExpBoolRecord( EXC_ID_PROTECT, false ) );
    AppendNewRecord( new XclExpChartDrawing( rRoot, xModel, rChartRect.GetSize() ) );
    AppendNewRecord( new XclExpUInt16Record( EXC_ID_CHUNITS, EXC_CHUNITS_TWIPS ) );

    Reference< XChartDocument > xChartDoc( xModel, UNO_QUERY );
    AppendNewRecord( new XclExpChChart( rRoot, xChartDoc, rChartRect ) );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
