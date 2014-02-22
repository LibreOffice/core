/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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

#include <tools/gen.hxx>
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

#include <boost/scoped_ptr.hpp>

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

} 



/** Stores global data needed in various classes of the Chart export filter. */
struct XclExpChRootData : public XclChRootData
{
    typedef ::std::vector< XclChFrBlock > XclChFrBlockVector;

    XclExpChChart&      mrChartData;            
    XclChFrBlockVector  maWrittenFrBlocks;      
    XclChFrBlockVector  maUnwrittenFrBlocks;    

    inline explicit     XclExpChRootData( XclExpChChart& rChartData ) : mrChartData( rChartData ) {}

    /** Registers a new future record level. */
    void                RegisterFutureRecBlock( const XclChFrBlock& rFrBlock );
    /** Initializes the current future record level (writes all unwritten CHFRBLOCKBEGIN records). */
    void                InitializeFutureRecBlock( XclExpStream& rStrm );
    /** Finalizes the current future record level (writes CHFRBLOCKEND record if needed). */
    void                FinalizeFutureRecBlock( XclExpStream& rStrm );
};



void XclExpChRootData::RegisterFutureRecBlock( const XclChFrBlock& rFrBlock )
{
    maUnwrittenFrBlocks.push_back( rFrBlock );
}

void XclExpChRootData::InitializeFutureRecBlock( XclExpStream& rStrm )
{
    
    if( !maUnwrittenFrBlocks.empty() )
    {
        
        if( maWrittenFrBlocks.empty() )
        {
            rStrm.StartRecord( EXC_ID_CHFRINFO, 20 );
            rStrm << EXC_ID_CHFRINFO << EXC_FUTUREREC_EMPTYFLAGS << EXC_CHFRINFO_EXCELXP2003 << EXC_CHFRINFO_EXCELXP2003 << sal_uInt16( 3 );
            rStrm << sal_uInt16( 0x0850 ) << sal_uInt16( 0x085A ) << sal_uInt16( 0x0861 ) << sal_uInt16( 0x0861 ) << sal_uInt16( 0x086A ) << sal_uInt16( 0x086B );
            rStrm.EndRecord();
        }
        
        for( XclChFrBlockVector::const_iterator aIt = maUnwrittenFrBlocks.begin(), aEnd = maUnwrittenFrBlocks.end(); aIt != aEnd; ++aIt )
        {
            OSL_ENSURE( aIt->mnType != EXC_CHFRBLOCK_TYPE_UNKNOWN, "XclExpChRootData::InitializeFutureRecBlock - unknown future record block type" );
            lclWriteChFrBlockRecord( rStrm, *aIt, true );
        }
        
        maWrittenFrBlocks.insert( maWrittenFrBlocks.end(), maUnwrittenFrBlocks.begin(), maUnwrittenFrBlocks.end() );
        maUnwrittenFrBlocks.clear();
    }
}

void XclExpChRootData::FinalizeFutureRecBlock( XclExpStream& rStrm )
{
    OSL_ENSURE( !maUnwrittenFrBlocks.empty() || !maWrittenFrBlocks.empty(), "XclExpChRootData::FinalizeFutureRecBlock - no future record level found" );
    if( !maUnwrittenFrBlocks.empty() )
    {
        
        maUnwrittenFrBlocks.pop_back();
    }
    else if( !maWrittenFrBlocks.empty() )
    {
        
        lclWriteChFrBlockRecord( rStrm, maWrittenFrBlocks.back(), false );
        maWrittenFrBlocks.pop_back();
    }
}



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
    
    XclExpRecord::Save( rStrm );
    
    if( HasSubRecords() )
    {
        
        RegisterFutureRecBlock( maFrBlock );
        
        XclExpEmptyRecord( EXC_ID_CHBEGIN ).Save( rStrm );
        
        WriteSubRecords( rStrm );
        
        FinalizeFutureRecBlock( rStrm );
        
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
        
        if( (eObjType != EXC_CHOBJTYPE_LINEARSERIES) && rRoot.IsSystemColor( maData.maColor, rFmtInfo.mnAutoLineColorIdx ) )
        {
            
            mnColorId = XclExpPalette::GetColorIdFromIndex( rFmtInfo.mnAutoLineColorIdx );
            
            bool bAuto = (maData.mnPattern == EXC_CHLINEFORMAT_SOLID) && (maData.mnWeight == rFmtInfo.mnAutoLineWeight);
            ::set_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO, bAuto );
        }
        else
        {
            
            mnColorId = rRoot.GetPalette().InsertColor( maData.maColor, EXC_COLOR_CHARTLINE );
        }
    }
    else
    {
        
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

} 



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
        
        if( (eObjType != EXC_CHOBJTYPE_FILLEDSERIES) && rRoot.IsSystemColor( maData.maPattColor, rFmtInfo.mnAutoPattColorIdx ) )
        {
            
            mnPattColorId = XclExpPalette::GetColorIdFromIndex( rFmtInfo.mnAutoPattColorIdx );
            
            ::set_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO, bSolid );
        }
        else
        {
            
            mnPattColorId = rRoot.GetPalette().InsertColor( maData.maPattColor, EXC_COLOR_CHARTAREA );
        }
        
        if( bSolid )
            rRoot.SetSystemColor( maData.maBackColor, mnBackColorId, EXC_COLOR_CHWINDOWTEXT );
        else
            mnBackColorId = rRoot.GetPalette().InsertColor( maData.maBackColor, EXC_COLOR_CHARTAREA );
    }
    else
    {
        
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
    
    mnColor1Id = RegisterColor( ESCHER_Prop_fillColor );
    mnColor2Id = RegisterColor( ESCHER_Prop_fillBackColor );
}

bool XclExpChEscherFormat::IsValid() const
{
    return static_cast< bool >(maData.mxEscherSet);
}

void XclExpChEscherFormat::Save( XclExpStream& rStrm )
{
    if( maData.mxEscherSet )
    {
        
        const XclExpPalette& rPal = GetPalette();
        maData.mxEscherSet->AddOpt( ESCHER_Prop_fillColor, 0x08000000 | rPal.GetColorIndex( mnColor1Id ) );
        maData.mxEscherSet->AddOpt( ESCHER_Prop_fillBackColor, 0x08000000 | rPal.GetColorIndex( mnColor2Id ) );

        
        XclExpChGroupBase::Save( rStrm );
    }
}

bool XclExpChEscherFormat::HasSubRecords() const
{
    
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
    
    SvMemoryStream aMemStrm;
    maData.mxEscherSet->Commit( aMemStrm );
    aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aMemStrm );
}



XclExpChFrameBase::XclExpChFrameBase()
{
}

XclExpChFrameBase::~XclExpChFrameBase()
{
}

void XclExpChFrameBase::ConvertFrameBase( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    
    mxLineFmt.reset( new XclExpChLineFormat( rRoot ) );
    mxLineFmt->Convert( rRoot, rPropSet, eObjType );
    
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
    
    mxLineFmt.reset( new XclExpChLineFormat( rRoot ) );
    mxLineFmt->SetDefault( eDefFrameType );
    
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

} 



namespace {

void lclAddDoubleRefData(
        ScTokenArray& orArray, const FormulaToken& rToken,
        SCsTAB nScTab1, SCsCOL nScCol1, SCsROW nScRow1,
        SCsTAB nScTab2, SCsCOL nScCol2, SCsROW nScRow2 )
{
    ScComplexRefData aComplexRef;
    aComplexRef.InitRange(ScRange(nScCol1,nScRow1,nScTab1,nScCol2,nScRow2,nScTab2));
    aComplexRef.Ref1.SetFlag3D( true );

    if( orArray.GetLen() > 0 )
        orArray.AddOpCode( ocUnion );

    OSL_ENSURE( (rToken.GetType() == ::formula::svDoubleRef) || (rToken.GetType() == ::formula::svExternalDoubleRef),
        "lclAddDoubleRefData - double reference token expected");
    if( rToken.GetType() == ::formula::svExternalDoubleRef )
        orArray.AddExternalDoubleReference(
            rToken.GetIndex(), rToken.GetString().getString(), aComplexRef);
    else
        orArray.AddDoubleReference( aComplexRef );
}

} 



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

    
    
    OUString aRangeRepr = xDataSeq->getSourceRangeRepresentation();
    ScCompiler aComp( GetDocPtr(), ScAddress() );
    aComp.SetGrammar( GetDocPtr()->GetGrammar() );
    boost::scoped_ptr<ScTokenArray> pArray(aComp.CompileString(aRangeRepr));
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
                
                if( aArray.GetLen() > 0 )
                    aArray.AddOpCode( ocUnion );
                aArray.AddToken( *pToken );
                ++nValueCount;
            break;

            case ::formula::svDoubleRef:
            case ::formula::svExternalDoubleRef:
            {
                
                const ScComplexRefData& rComplexRef = static_cast< const ScToken* >( pToken )->GetDoubleRef();
                ScAddress aAbs1 = rComplexRef.Ref1.toAbs(ScAddress());
                ScAddress aAbs2 = rComplexRef.Ref2.toAbs(ScAddress());
                for (SCsTAB nScTab = aAbs1.Tab(); nScTab <= aAbs2.Tab(); ++nScTab)
                {
                    
                    if (bSplitToColumns && (aAbs1.Col() < aAbs2.Col()) && (aAbs1.Row() < aAbs2.Row()))
                        for (SCCOL nScCol = aAbs1.Col(); nScCol <= aAbs2.Col(); ++nScCol)
                            lclAddDoubleRefData(aArray, *pToken, nScTab, nScCol, aAbs1.Row(), nScTab, nScCol, aAbs2.Row());
                    else
                        lclAddDoubleRefData(aArray, *pToken, nScTab, aAbs1.Col(), aAbs1.Row(), nScTab, aAbs2.Col(), aAbs2.Row());
                }
                sal_uInt32 nTabs = static_cast<sal_uInt32>(aAbs2.Tab() - aAbs1.Tab() + 1);
                sal_uInt32 nCols = static_cast<sal_uInt32>(aAbs2.Col() - aAbs1.Col() + 1);
                sal_uInt32 nRows = static_cast<sal_uInt32>(aAbs2.Row() - aAbs1.Row() + 1);
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

void XclExpChSourceLink::ConvertString( OUString aString )
{
    mxString = XclExpStringHelper::CreateString( GetRoot(), aString, EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH | EXC_STR_SEPARATEFORMATS );
}

sal_uInt16 XclExpChSourceLink::ConvertStringSequence( const Sequence< Reference< XFormattedString > >& rStringSeq )
{
    mxString.reset();
    sal_uInt16 nFontIdx = EXC_FONT_APP;
    if( rStringSeq.hasElements() )
    {
        mxString = XclExpStringHelper::CreateString( GetRoot(), OUString(), EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH | EXC_STR_SEPARATEFORMATS );
        Reference< XBreakIterator > xBreakIt = GetDoc().GetBreakIterator();
        namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;

        
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

                
                sal_Int16 nLastScript = XclExpStringHelper::GetLeadingScriptType( GetRoot(), aText );

                
                sal_Int32 nPortionPos = 0;
                sal_Int32 nTextLen = aText.getLength();
                while( nPortionPos < nTextLen )
                {
                    
                    sal_Int16 nScript = xBreakIt->getScriptType( aText, nPortionPos );
                    sal_Int32 nPortionEnd = xBreakIt->endOfScript( aText, nPortionPos, nScript );

                    
                    if( nScript == ApiScriptType::WEAK )
                        nScript = nLastScript;

                    
                    sal_uInt16 nXclPortionStart = mxString->Len();
                    
                    XclExpStringHelper::AppendString( *mxString, GetRoot(), aText.copy( nPortionPos, nPortionEnd - nPortionPos ) );
                    if( nXclPortionStart < mxString->Len() )
                    {
                        
                        sal_uInt16& rnFontIdx = (nScript == ApiScriptType::COMPLEX) ? nCmplxFontIdx :
                            ((nScript == ApiScriptType::ASIAN) ? nAsianFontIdx : nWstrnFontIdx);

                        
                        if( rnFontIdx == EXC_FONT_NOTFOUND )
                            rnFontIdx = ConvertFont( aStrProp, nScript );

                        
                        mxString->AppendFormat( nXclPortionStart, rnFontIdx );
                    }

                    
                    nLastScript = nScript;
                    nPortionPos = nPortionEnd;
                }
            }
        }
        if( !mxString->IsEmpty() )
        {
            
            const XclFormatRunVec& rFormats = mxString->GetFormats();
            OSL_ENSURE( !rFormats.empty() && (rFormats.front().mnChar == 0),
                "XclExpChSourceLink::ConvertStringSequenc - missing leading format" );
            
            if( rFormats.size() == 1 )
                nFontIdx = mxString->RemoveLeadingFont();
            else if( !rFormats.empty() )
                nFontIdx = rFormats.front().mnFontIdx;
            
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

void XclExpChSourceLink::AppendString( const OUString& rStr )
{
    if (!mxString)
        return;
    XclExpStringHelper::AppendString( *mxString, GetRoot(), rStr );
}

void XclExpChSourceLink::Save( XclExpStream& rStrm )
{
    
    if( mxString && mxString->IsRich() )
    {
        sal_Size nRecSize = (1 + mxString->GetFormatsCount()) * ((GetBiff() == EXC_BIFF8) ? 2 : 1);
        rStrm.StartRecord( EXC_ID_CHFORMATRUNS, nRecSize );
        mxString->WriteFormats( rStrm, true );
        rStrm.EndRecord();
    }
    
    XclExpRecord::Save( rStrm );
    
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



XclExpChFont::XclExpChFont( sal_uInt16 nFontIdx ) :
    XclExpUInt16Record( EXC_ID_CHFONT, nFontIdx )
{
}



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



XclExpChFrLabelProps::XclExpChFrLabelProps( const XclExpChRoot& rRoot ) :
    XclExpChFutureRecordBase( rRoot, EXC_FUTUREREC_UNUSEDREF, EXC_ID_CHFRLABELPROPS, 4 )
{
}

void XclExpChFrLabelProps::Convert( const ScfPropertySet& rPropSet, bool bShowSeries,
        bool bShowCateg, bool bShowValue, bool bShowPercent, bool bShowBubble )
{
    
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWSERIES,  bShowSeries );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWCATEG,   bShowCateg );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWVALUE,   bShowValue );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWPERCENT, bShowPercent );
    ::set_flag( maData.mnFlags, EXC_CHFRLABELPROPS_SHOWBUBBLE,  bShowBubble );

    
    maData.maSeparator = rPropSet.GetStringProperty( EXC_CHPROP_LABELSEPARATOR );
    if( maData.maSeparator.isEmpty() )
        maData.maSeparator = OUString(' ');
}

void XclExpChFrLabelProps::WriteBody( XclExpStream& rStrm )
{
    XclExpString aXclSep( maData.maSeparator, EXC_STR_FORCEUNICODE | EXC_STR_SMARTFLAGS );
    rStrm << maData.mnFlags << aXclSep;
}



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

void XclExpChText::ConvertTitle( Reference< XTitle > xTitle, sal_uInt16 nTarget, const OUString* pSubTitle )
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
        
        ScfPropertySet aTitleProp( xTitle );
        mxFrame = lclCreateFrame( GetChRoot(), aTitleProp, EXC_CHOBJTYPE_TEXT );

        
        mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
        sal_uInt16 nFontIdx = mxSrcLink->ConvertStringSequence( xTitle->getText() );
        if (pSubTitle)
        {
            
            OUString aSubTitle("\n");
            aSubTitle += *pSubTitle;
            mxSrcLink->AppendString(aSubTitle);
        }

        ConvertFontBase( GetChRoot(), nFontIdx );

        
        ConvertRotationBase( GetChRoot(), aTitleProp, true );

        
        mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_PARENT, EXC_CHFRAMEPOS_PARENT ) );
        if( nTarget == EXC_CHOBJLINK_TITLE )
        {
            Any aRelPos;
            if( aTitleProp.GetAnyProperty( aRelPos, EXC_CHPROP_RELATIVEPOSITION ) && aRelPos.has< RelativePosition >() ) try
            {
                
                Reference< cssc::XChartDocument > xChart1Doc( GetChartDocument(), UNO_QUERY_THROW );
                Reference< XShape > xTitleShape( xChart1Doc->getTitle(), UNO_SET_THROW );
                ::com::sun::star::awt::Point aPos = xTitleShape->getPosition();
                ::com::sun::star::awt::Size aSize = xTitleShape->getSize();
                ::com::sun::star::awt::Rectangle aRect( aPos.X, aPos.Y, aSize.Width, aSize.Height );
                maData.maRect = CalcChartRectFromHmm( aRect );
                ::insert_value( maData.mnFlags2, EXC_CHTEXT_POS_MOVED, 0, 4 );
                
                GetChartData().SetManualPlotArea();
                
                sal_Int32 nDefPosX = ::std::max< sal_Int32 >( (EXC_CHART_TOTALUNITS - maData.maRect.mnWidth) / 2, 0 );
                sal_Int32 nDefPosY = 85;
                
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

    
    bool bIsPie = rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE;
    
    bool bIsBubble = rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES;
    OSL_ENSURE( (GetBiff() == EXC_BIFF8) || !bIsBubble, "XclExpChText::ConvertDataLabel - bubble charts only in BIFF8" );

    
    bool bShowValue   = !bIsBubble && aPointLabel.ShowNumber;       
    bool bShowPercent = bIsPie && aPointLabel.ShowNumberInPercent;  
    bool bShowCateg   = aPointLabel.ShowCategoryName;
    bool bShowBubble  = bIsBubble && aPointLabel.ShowNumber;        
    bool bShowAny     = bShowValue || bShowPercent || bShowCateg || bShowBubble;

    
    if( bShowAny && (GetBiff() == EXC_BIFF8) )
    {
        mxLabelProps.reset( new XclExpChFrLabelProps( GetChRoot() ) );
        mxLabelProps->Convert( rPropSet, false, bShowCateg, bShowValue, bShowPercent, bShowBubble );
    }

    
    if( bShowPercent ) bShowValue = false;              
    if( bShowValue ) bShowCateg = false;                
    if( bShowValue || bShowCateg ) bShowBubble = false; 

    
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
        
        ConvertFontBase( GetChRoot(), rPropSet );
        ConvertRotationBase( GetChRoot(), rPropSet, false );
        
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
        
        mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
        if( bShowValue || bShowPercent )
            
            mxSrcLink->ConvertNumFmt( rPropSet, bShowPercent );
        
        mxObjLink.reset( new XclExpChObjectLink( EXC_CHOBJLINK_DATA, rPointPos ) );
    }

    /*  Return true to indicate valid label settings:
        - for existing labels at entire series
        - for any settings at single data point (to be able to delete a point label) */
    return bShowAny || (rPointPos.mnPointIdx != EXC_CHDATAFORMAT_ALLPOINTS);
}

void XclExpChText::ConvertTrendLineEquation( const ScfPropertySet& rPropSet, const XclChDataPointPos& rPointPos )
{
    
    ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOTEXT );
    if( GetBiff() == EXC_BIFF8 )
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG ); 
    
    mxFrame = lclCreateFrame( GetChRoot(), rPropSet, EXC_CHOBJTYPE_TEXT );
    
    maData.mnHAlign = EXC_CHTEXT_ALIGN_TOPLEFT;
    maData.mnVAlign = EXC_CHTEXT_ALIGN_TOPLEFT;
    ConvertFontBase( GetChRoot(), rPropSet );
    
    mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
    mxSrcLink->ConvertNumFmt( rPropSet, false );
    
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
    
    lclSaveRecord( rStrm, mxFramePos );
    
    lclSaveRecord( rStrm, mxFont );
    
    lclSaveRecord( rStrm, mxSrcLink );
    
    lclSaveRecord( rStrm, mxFrame );
    
    lclSaveRecord( rStrm, mxObjLink );
    
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



namespace {

/** Creates and returns an Excel text object from the passed title. */
XclExpChTextRef lclCreateTitle( const XclExpChRoot& rRoot, Reference< XTitled > xTitled, sal_uInt16 nTarget,
                                const OUString* pSubTitle = NULL )
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
    
    RegisterColors( rRoot );
}

void XclExpChMarkerFormat::ConvertStockSymbol( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, bool bCloseSymbol )
{
    
    ::set_flag( maData.mnFlags, EXC_CHMARKERFORMAT_AUTO, false );
    
    if( bCloseSymbol )
    {
        
        maData.mnMarkerType = EXC_CHMARKERFORMAT_DOWJ;
        maData.mnMarkerSize = EXC_CHMARKERFORMAT_DOUBLESIZE;
        
        Color aLineColor;
        if( rPropSet.GetColorProperty( aLineColor, EXC_CHPROP_COLOR ) )
        {
            maData.maLineColor = maData.maFillColor = aLineColor;
            RegisterColors( rRoot );
        }
    }
    else
    {
        
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



XclExpChAttachedLabel::XclExpChAttachedLabel( sal_uInt16 nFlags ) :
    XclExpUInt16Record( EXC_ID_CHATTACHEDLABEL, nFlags )
{
}



XclExpChDataFormat::XclExpChDataFormat( const XclExpChRoot& rRoot,
        const XclChDataPointPos& rPointPos, sal_uInt16 nFormatIdx ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_DATAFORMAT, EXC_ID_CHDATAFORMAT, 8 )
{
    maData.maPointPos = rPointPos;
    maData.mnFormatIdx = nFormatIdx;
}

void XclExpChDataFormat::ConvertDataSeries( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo )
{
    
    ConvertFrameBase( GetChRoot(), rPropSet, rTypeInfo.GetSeriesObjectType() );

    
    bool bIsFrame = rTypeInfo.IsSeriesFrameFormat();
    if( !bIsFrame )
    {
        mxMarkerFmt.reset( new XclExpChMarkerFormat( GetChRoot() ) );
        mxMarkerFmt->Convert( GetChRoot(), rPropSet, maData.mnFormatIdx );
    }

    
    if( rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE )
    {
        mxPieFmt.reset( new XclExpChPieFormat );
        mxPieFmt->Convert( rPropSet );
    }

    
    if( IsSeriesFormat() && (GetBiff() == EXC_BIFF8) && rTypeInfo.mb3dChart && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR) )
    {
        mx3dDataFmt.reset( new XclExpCh3dDataFormat );
        mx3dDataFmt->Convert( rPropSet );
    }

    
    if( IsSeriesFormat() && rTypeInfo.mbSpline && !bIsFrame )
        mxSeriesFmt.reset( new XclExpUInt16Record( EXC_ID_CHSERIESFORMAT, EXC_CHSERIESFORMAT_SMOOTHED ) );

    
    XclExpChTextRef xLabel( new XclExpChText( GetChRoot() ) );
    if( xLabel->ConvertDataLabel( rPropSet, rTypeInfo, maData.maPointPos ) )
    {
        
        GetChartData().SetDataLabel( xLabel );
        mxAttLabel.reset( new XclExpChAttachedLabel( xLabel->GetAttLabelFlags() ) );
    }
}

void XclExpChDataFormat::ConvertStockSeries( const ScfPropertySet& rPropSet, bool bCloseSymbol )
{
    
    SetDefaultFrameBase( GetChRoot(), EXC_CHFRAMETYPE_INVISIBLE, false );
    
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



XclExpChSerTrendLine::XclExpChSerTrendLine( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHSERTRENDLINE, 28 ),
    XclExpChRoot( rRoot )
{
}

bool XclExpChSerTrendLine::Convert( Reference< XRegressionCurve > xRegCurve, sal_uInt16 nSeriesIdx )
{
    if( !xRegCurve.is() )
        return false;

    
    ScfPropertySet aCurveProp( xRegCurve );

    OUString aService = aCurveProp.GetServiceName();
    if( aService == "com.sun.star.chart2.LinearRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_POLYNOMIAL;
        maData.mnOrder = 1;
    }
    else if( aService == "com.sun.star.chart2.ExponentialRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_EXPONENTIAL;
    }
    else if( aService == "com.sun.star.chart2.LogarithmicRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_LOGARITHMIC;
    }
    else if( aService == "com.sun.star.chart2.PotentialRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_POWER;
    }
    else if( aService == "com.sun.star.chart2.PolynomialRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_POLYNOMIAL;
        sal_Int32 aDegree;
        aCurveProp.GetProperty(aDegree, EXC_CHPROP_POLYNOMIAL_DEGREE);
        maData.mnOrder = static_cast<sal_uInt8> (aDegree);
    }
    else if( aService == "com.sun.star.chart2.MovingAverageRegressionCurve" )
    {
        maData.mnLineType = EXC_CHSERTREND_MOVING_AVG;
        sal_Int32 aPeriod;
        aCurveProp.GetProperty(aPeriod, EXC_CHPROP_MOVING_AVERAGE_PERIOD);
        maData.mnOrder = static_cast<sal_uInt8> (aPeriod);
    }
    else
    {
        return false;
    }

    aCurveProp.GetProperty(maData.mfForecastFor,  EXC_CHPROP_EXTRAPOLATE_FORWARD);
    aCurveProp.GetProperty(maData.mfForecastBack, EXC_CHPROP_EXTRAPOLATE_BACKWARD);
    sal_Bool bIsForceIntercept = false;
    aCurveProp.GetProperty(bIsForceIntercept,  EXC_CHPROP_FORCE_INTERCEPT);
    if (bIsForceIntercept)
        aCurveProp.GetProperty(maData.mfIntercept, EXC_CHPROP_INTERCEPT_VALUE);


    
    XclChDataPointPos aPointPos( nSeriesIdx );
    mxDataFmt.reset( new XclExpChDataFormat( GetChRoot(), aPointPos, 0 ) );
    mxDataFmt->ConvertLine( aCurveProp, EXC_CHOBJTYPE_TRENDLINE );

    
    ScfPropertySet aEquationProp( xRegCurve->getEquationProperties() );
    maData.mnShowEquation = aEquationProp.GetBoolProperty( EXC_CHPROP_SHOWEQUATION ) ? 1 : 0;
    maData.mnShowRSquared = aEquationProp.GetBoolProperty( EXC_CHPROP_SHOWCORRELATION ) ? 1 : 0;

    
    if( (maData.mnShowEquation != 0) || (maData.mnShowRSquared != 0) )
    {
        mxLabel.reset( new XclExpChText( GetChRoot() ) );
        mxLabel->ConvertTrendLineEquation( aEquationProp, aPointPos );
    }

    
    
    
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
            << sal_uInt8( 1 )       
            << maData.mfValue
            << maData.mnValueCount;
}



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

} 

XclExpChSeries::XclExpChSeries( const XclExpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_SERIES, EXC_ID_CHSERIES, (rRoot.GetBiff() == EXC_BIFF8) ? 12 : 8 ),
    mnGroupIdx( EXC_CHSERGROUP_NONE ),
    mnSeriesIdx( nSeriesIdx ),
    mnParentIdx( EXC_CHSERIES_INVALID )
{
    
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
                        xTitleSeq = (*pIt)->getLabel(); 
                }
                else if( !xXValueSeq.is() && !rTypeInfo.mbCategoryAxis && (aRole == EXC_CHPROP_ROLE_XVALUES) )
                {
                    xXValueSeq = xTmpValueSeq;
                }
                else if( !xBubbleSeq.is() && (rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES) && (aRole == EXC_CHPROP_ROLE_SIZEVALUES) )
                {
                    xBubbleSeq = xTmpValueSeq;
                    xTitleSeq = (*pIt)->getLabel();     
                }
            }
        }

        bOk = xYValueSeq.is();
        if( bOk )
        {
            
            mnGroupIdx = nGroupIdx;

            
            maData.mnValueCount = mxValueLink->ConvertDataSequence( xYValueSeq, true );
            mxTitleLink->ConvertDataSequence( xTitleSeq, true );

            
            maData.mnCategCount = mxCategLink->ConvertDataSequence( xXValueSeq, false, maData.mnValueCount );

            
            if( mxBubbleLink )
                mxBubbleLink->ConvertDataSequence( xBubbleSeq, false, maData.mnValueCount );

            
            XclChDataPointPos aPointPos( mnSeriesIdx );
            ScfPropertySet aSeriesProp( xDataSeries );
            mxSeriesFmt.reset( new XclExpChDataFormat( GetChRoot(), aPointPos, nFormatIdx ) );
            mxSeriesFmt->ConvertDataSeries( aSeriesProp, rTypeInfo );

            
            CreateTrendLines( xDataSeries );

            
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
                xTitleSeq = (*pIt)->getLabel();     
            }
        }

        bOk = xYValueSeq.is();
        if( bOk )
        {
            
            mnGroupIdx = nGroupIdx;
            
            maData.mnValueCount = mxValueLink->ConvertDataSequence( xYValueSeq, true );
            mxTitleLink->ConvertDataSequence( xTitleSeq, true );
            
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
        OUString aName;
        ScfPropertySet aProperties( xRegCurve );
        aProperties.GetProperty(aName, EXC_CHPROP_CURVENAME);
        mxTitleLink->ConvertString(aName);

        mxSeriesFmt = mxTrendLine->GetDataFormat();
        GetChartData().SetDataLabel( mxTrendLine->GetDataLabel() );
    }
    return bOk;
}

bool XclExpChSeries::ConvertErrorBar( const XclExpChSeries& rParent, const ScfPropertySet& rPropSet, sal_uInt8 nBarId )
{
    InitFromParent( rParent );
    
    mxErrorBar.reset( new XclExpChSerErrorBar( GetChRoot(), nBarId ) );
    bool bOk = mxErrorBar->Convert( *mxValueLink, maData.mnValueCount, rPropSet );
    if( bOk )
    {
        
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
        
        if( nRotationY < 0 ) nRotationY += 360;
        maData.mnRotation = static_cast< sal_uInt16 >( nRotationY );
        
        maData.mnElevation = limit_cast< sal_Int16 >( nRotationX, -90, 90 );
        
        maData.mnEyeDist = limit_cast< sal_uInt16 >( nPerspective, 0, 100 );
        
        maData.mnFlags = 0;
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_REAL3D, !rPropSet.GetBoolProperty( EXC_CHPROP_RIGHTANGLEDAXES ) );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_AUTOHEIGHT );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_HASWALLS );
    }
    else
    {
        
        maData.mnRotation = XclExpChRoot::ConvertPieRotation( rPropSet );
        
        maData.mnElevation = limit_cast< sal_Int16 >( (nRotationX + 270) % 180, 10, 80 );
        
        maData.mnEyeDist = limit_cast< sal_uInt16 >( nPerspective, 0, 100 );
        
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



XclExpChLegend::XclExpChLegend( const XclExpChRoot& rRoot ) :
    XclExpChGroupBase( rRoot, EXC_CHFRBLOCK_TYPE_LEGEND, EXC_ID_CHLEGEND, 20 )
{
}

void XclExpChLegend::Convert( const ScfPropertySet& rPropSet )
{
    
    mxFrame = lclCreateFrame( GetChRoot(), rPropSet, EXC_CHOBJTYPE_LEGEND );
    
    mxText.reset( new XclExpChText( GetChRoot() ) );
    mxText->ConvertLegend( rPropSet );

    
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
            
            mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_CHARTSIZE, EXC_CHFRAMEPOS_PARENT ) );
            XclChFramePos& rFramePos = mxFramePos->GetFramePosData();
            rFramePos.mnTLMode = EXC_CHFRAMEPOS_CHARTSIZE;
            ::com::sun::star::awt::Point aLegendPos = xChart1Legend->getPosition();
            rFramePos.maRect.mnX = maData.maRect.mnX = CalcChartXFromHmm( aLegendPos.X );
            rFramePos.maRect.mnY = maData.maRect.mnY = CalcChartYFromHmm( aLegendPos.Y );
            
            rFramePos.mnBRMode = EXC_CHFRAMEPOS_ABSSIZE_POINTS;
            ::com::sun::star::awt::Size aLegendSize = xChart1Legend->getSize();
            rFramePos.maRect.mnWidth = static_cast< sal_uInt16 >( aLegendSize.Width * EXC_POINTS_PER_HMM + 0.5 );
            rFramePos.maRect.mnHeight = static_cast< sal_uInt16 >( aLegendSize.Height * EXC_POINTS_PER_HMM + 0.5 );
            maData.maRect.mnWidth = CalcChartXFromHmm( aLegendSize.Width );
            maData.maRect.mnHeight = CalcChartYFromHmm( aLegendSize.Height );
            eApiExpand = cssc::ChartLegendExpansion_CUSTOM;
            
            GetChartData().SetManualPlotArea();
            maData.mnDockMode = EXC_CHLEGEND_NOTDOCKED;
            
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
    
    maType.Convert( xDiagram, xChartType, nApiAxesSetIdx, bSwappedAxesSet, bHasXLabels );

    
    ScfPropertySet aTypeProp( xChartType );
    cssc2::CurveStyle eCurveStyle;
    bool bSpline = aTypeProp.GetProperty( eCurveStyle, EXC_CHPROP_CURVESTYLE ) &&
        (eCurveStyle != cssc2::CurveStyle_LINES);

    
    maTypeInfo.Set( maType.GetTypeInfo(), b3dChart, bSpline );

    
    if( maTypeInfo.mb3dChart )  
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

        
        if( !aSeriesVec.empty() )
        {
            
            ScfPropertySet aSeriesProp( aSeriesVec.front() );
            cssc2::StackingDirection eStacking;
            if( !aSeriesProp.GetProperty( eStacking, EXC_CHPROP_STACKINGDIR ) )
                eStacking = cssc2::StackingDirection_NO_STACKING;

            
            if( maTypeInfo.mbSupportsStacking && (eStacking == cssc2::StackingDirection_Y_STACKING) )
            {
                
                maType.SetStacked( bPercent );

                
                if (bConnectBars && (maTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR))
                {
                    sal_uInt16 nKey = EXC_CHCHARTLINE_CONNECT;
                    maChartLines.insert(nKey, new XclExpChLineFormat(GetChRoot()));
                }
            }
            else
            {
                
                if( maTypeInfo.mbReverseSeries && !Is3dChart() )
                    ::std::reverse( aSeriesVec.begin(), aSeriesVec.end() );
            }

            
            if( (eStacking == cssc2::StackingDirection_NO_STACKING) && Is3dWallChart() )
                mxChart3d->SetClustered();

            
            ::set_flag( maData.mnFlags, EXC_CHTYPEGROUP_VARIEDCOLORS, aSeriesProp.GetBoolProperty( EXC_CHPROP_VARYCOLORSBY ) );

            
            for( XDataSeriesVec::const_iterator aIt = aSeriesVec.begin(), aEnd = aSeriesVec.end(); aIt != aEnd; ++aIt )
            {
                
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
    
    bool bHasOpen = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_OPENVALUES, false );
    bool bHasHigh = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_HIGHVALUES, false );
    bool bHasLow = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_LOWVALUES, false );
    bool bHasClose = CreateStockSeries( xDataSeries, EXC_CHPROP_ROLE_CLOSEVALUES, !bHasOpen );

    
    ScfPropertySet aTypeProp( xChartType );
    
    if( bHasHigh && bHasLow && aTypeProp.GetBoolProperty( EXC_CHPROP_SHOWHIGHLOW ) )
    {
        ScfPropertySet aSeriesProp( xDataSeries );
        XclExpChLineFormatRef xLineFmt( new XclExpChLineFormat( GetChRoot() ) );
        xLineFmt->Convert( GetChRoot(), aSeriesProp, EXC_CHOBJTYPE_HILOLINE );
        sal_uInt16 nKey = EXC_CHCHARTLINE_HILO;
        maChartLines.insert(nKey, new XclExpChLineFormat(GetChRoot()));
    }
    
    if( bHasOpen && bHasClose )
    {
        
        Reference< XPropertySet > xWhitePropSet, xBlackPropSet;
        
        aTypeProp.GetProperty( xWhitePropSet, EXC_CHPROP_WHITEDAY );
        ScfPropertySet aWhiteProp( xWhitePropSet );
        mxUpBar.reset( new XclExpChDropBar( GetChRoot(), EXC_CHOBJTYPE_WHITEDROPBAR ) );
        mxUpBar->Convert( aWhiteProp );
        
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
        
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_DATEAXIS );

        
        bool bAutoBase = !rScaleData.TimeIncrement.TimeResolution.has< cssc::TimeIncrement >();
        ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOBASE, bAutoBase );

        
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

    
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTODATE, rScaleData.AutoDateAxis );

    
    bool bAutoMajor = lclConvertTimeInterval( maDateData.mnMajorStep, maDateData.mnMajorUnit, rScaleData.TimeIncrement.MajorTimeInterval );
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMAJOR, bAutoMajor );
    bool bAutoMinor = lclConvertTimeInterval( maDateData.mnMinorStep, maDateData.mnMinorUnit, rScaleData.TimeIncrement.MinorTimeInterval );
    ::set_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMINOR, bAutoMinor );

    
    double fOrigin = 0.0;
    if( !lclIsAutoAnyOrGetValue( fOrigin, rScaleData.Origin ) )
        maLabelData.mnCross = limit_cast< sal_uInt16 >( fOrigin, 1, 31999 );

    
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
    
    XclExpRecord::Save( rStrm );

    
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



XclExpChValueRange::XclExpChValueRange( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHVALUERANGE, 42 ),
    XclExpChRoot( rRoot )
{
}

void XclExpChValueRange::Convert( const ScaleData& rScaleData )
{
    
    bool bLogScale = ScfApiHelper::GetServiceName( rScaleData.Scaling ) == "com.sun.star.chart2.LogarithmicScaling";
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE, bLogScale );

    
    bool bAutoMin = lclIsAutoAnyOrGetScaledValue( maData.mfMin, rScaleData.Minimum, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN, bAutoMin );
    bool bAutoMax = lclIsAutoAnyOrGetScaledValue( maData.mfMax, rScaleData.Maximum, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX, bAutoMax );

    
    bool bAutoCross = lclIsAutoAnyOrGetScaledValue( maData.mfCross, rScaleData.Origin, bLogScale );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS, bAutoCross );

    
    const IncrementData& rIncrementData = rScaleData.IncrementData;
    bool bAutoMajor = lclIsAutoAnyOrGetValue( maData.mfMajorStep, rIncrementData.Distance ) || (maData.mfMajorStep <= 0.0);
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR, bAutoMajor );
    
    const Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
    sal_Int32 nCount = 0;
    bool bAutoMinor = bLogScale || bAutoMajor || (rSubIncrementSeq.getLength() < 1) ||
        lclIsAutoAnyOrGetValue( nCount, rSubIncrementSeq[ 0 ].IntervalCount ) || (nCount < 1);
    if( !bAutoMinor )
        maData.mfMinorStep = maData.mfMajorStep / nCount;
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR, bAutoMinor );

    
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



namespace {

sal_uInt8 lclGetXclTickPos( sal_Int32 nApiTickmarks )
{
    using namespace cssc2::TickmarkStyle;
    sal_uInt8 nXclTickPos = 0;
    ::set_flag( nXclTickPos, EXC_CHTICK_INSIDE,  ::get_flag( nApiTickmarks, INNER ) );
    ::set_flag( nXclTickPos, EXC_CHTICK_OUTSIDE, ::get_flag( nApiTickmarks, OUTER ) );
    return nXclTickPos;
}

} 

XclExpChTick::XclExpChTick( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHTICK, (rRoot.GetBiff() == EXC_BIFF8) ? 30 : 26 ),
    XclExpChRoot( rRoot ),
    mnTextColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWTEXT ) )
{
}

void XclExpChTick::Convert( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo, sal_uInt16 nAxisType )
{
    
    sal_Int32 nApiTickmarks = 0;
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MAJORTICKS ) )
        maData.mnMajor = lclGetXclTickPos( nApiTickmarks );
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MINORTICKS ) )
        maData.mnMinor = lclGetXclTickPos( nApiTickmarks );

    
    if( (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR) && (nAxisType == EXC_CHAXIS_X) )
    {
        /*  Radar charts disable their category labels via chart type, not via
            axis, and axis labels are always 'near axis'. */
        maData.mnLabelPos = EXC_CHTICK_NEXT;
    }
    else if( !rPropSet.GetBoolProperty( EXC_CHPROP_DISPLAYLABELS ) )
    {
        
        maData.mnLabelPos = EXC_CHTICK_NOLABEL;
    }
    else if( rTypeInfo.mb3dChart && (nAxisType == EXC_CHAXIS_Y) )
    {
        
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

} 

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

    

    mxAxisLine.reset( new XclExpChLineFormat( GetChRoot() ) );
    mxAxisLine->Convert( GetChRoot(), aAxisProp, EXC_CHOBJTYPE_AXISLINE );
    
    mxAxisLine->SetShowAxis( aAxisProp.GetBoolProperty( EXC_CHPROP_SHOW ) );

    

    ScfPropertySet aCrossingProp( xCrossingAxis );
    if( bCategoryAxis )
    {
        mxLabelRange.reset( new XclExpChLabelRange( GetChRoot() ) );
        mxLabelRange->SetTicksBetweenCateg( rTypeInfo.mbTicksBetweenCateg );
        if( xAxis.is() )
        {
            ScfPropertySet aChart1AxisProp( xChart1Axis );
            
            mxLabelRange->Convert( xAxis->getScaleData(), aChart1AxisProp, (GetAxisType() == EXC_CHAXIS_X) && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR) );
        }
        
        if( aCrossingProp.Is() )
            mxLabelRange->ConvertAxisPosition( aCrossingProp );
    }
    else
    {
        mxValueRange.reset( new XclExpChValueRange( GetChRoot() ) );
        if( xAxis.is() )
            mxValueRange->Convert( xAxis->getScaleData() );
        
        if( aCrossingProp.Is() )
            mxValueRange->ConvertAxisPosition( aCrossingProp );
    }

    

    
    mxTick.reset( new XclExpChTick( GetChRoot() ) );
    mxTick->Convert( aAxisProp, rTypeInfo, GetAxisType() );

    
    ConvertFontBase( GetChRoot(), aAxisProp );
    ConvertRotationBase( GetChRoot(), aAxisProp, true );

    
    sal_Int32 nApiNumFmt = 0;
    if( !bCategoryAxis && aAxisProp.GetProperty( nApiNumFmt, EXC_CHPROP_NUMBERFORMAT ) )
        mnNumFmtIdx = GetNumFmtBuffer().Insert( static_cast< sal_uInt32 >( nApiNumFmt ) );

    

    if( xAxis.is() )
    {
        
        ScfPropertySet aGridProp( xAxis->getGridProperties() );
        if( aGridProp.GetBoolProperty( EXC_CHPROP_SHOW ) )
            mxMajorGrid = lclCreateLineFormat( GetChRoot(), aGridProp, EXC_CHOBJTYPE_GRIDLINE );
        
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

            
            bool b3dChart = xCoordSystem.is() && (xCoordSystem->getDimension() == 3);

            
            namespace ApiAxisType = cssc2::AxisType;
            Reference< XAxis > xApiYAxis = lclGetApiAxis( xCoordSystem, EXC_CHART_AXIS_Y, nApiAxesSetIdx );
            bool bPercent = xApiYAxis.is() && (xApiYAxis->getScaleData().AxisType == ApiAxisType::PERCENT);

            
            ScfPropertySet aDiaProp( xDiagram );
            bool bConnectBars = aDiaProp.GetBoolProperty( EXC_CHPROP_CONNECTBARS );

            
            ScfPropertySet aCoordSysProp( xCoordSystem );
            bool bSwappedAxesSet = aCoordSysProp.GetBoolProperty( EXC_CHPROP_SWAPXANDYAXIS );

            
            Reference< XAxis > xApiXAxis = lclGetApiAxis( xCoordSystem, EXC_CHART_AXIS_X, nApiAxesSetIdx );
            
            ScfPropertySet aXAxisProp( xApiXAxis );
            bool bHasXLabels = aXAxisProp.GetBoolProperty( EXC_CHPROP_DISPLAYLABELS );

            
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

                
                if( rTypeInfo.meTypeCateg != EXC_CHTYPECATEG_PIE )
                {
                    ConvertAxis( mxXAxis, EXC_CHAXIS_X, mxXAxisTitle, EXC_CHOBJLINK_XAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_Y );
                    ConvertAxis( mxYAxis, EXC_CHAXIS_Y, mxYAxisTitle, EXC_CHOBJLINK_YAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_X );
                    if( pGroup->Is3dDeepChart() )
                        ConvertAxis( mxZAxis, EXC_CHAXIS_Z, mxZAxisTitle, EXC_CHOBJLINK_ZAXIS, xCoordSystem, rTypeInfo, EXC_CHART_AXIS_NONE );
                }

                
                if( rTypeInfo.mbCategoryAxis && xApiXAxis.is() )
                {
                    const ScaleData aScaleData = xApiXAxis->getScaleData();
                    for( size_t nIdx = 0, nSize = maTypeGroups.GetSize(); nIdx < nSize; ++nIdx )
                        maTypeGroups.GetRecord( nIdx )->ConvertCategSequence( aScaleData.Categories );
                }

                
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

    
    if( xDiagram.is() && (GetAxesSetId() == EXC_CHAXESSET_PRIMARY) )
    {
        XclExpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
        if( xTypeGroup && xTypeGroup->Is3dWallChart() )
        {
            
            if( mxXAxis )
                mxXAxis->ConvertWall( xDiagram );
            if( mxYAxis )
                mxYAxis->ConvertWall( xDiagram );
        }
        else
        {
            
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxPlotFrame = lclCreateFrame( GetChRoot(), aWallProp, EXC_CHOBJTYPE_PLOTFRAME );
        }
    }

    
    try
    {
        Reference< cssc::XChartDocument > xChart1Doc( GetChartDocument(), UNO_QUERY_THROW );
        Reference< cssc::XDiagramPositioning > xPositioning( xChart1Doc->getDiagram(), UNO_QUERY_THROW );
        
        if( !xPositioning->isAutomaticDiagramPositioning() )
            GetChartData().SetManualPlotArea();
        
        maData.maRect = CalcChartRectFromHmm( xPositioning->calculateDiagramPositionExcludingAxes() );
        
        mxFramePos.reset( new XclExpChFramePos( EXC_CHFRAMEPOS_PARENT, EXC_CHFRAMEPOS_PARENT ) );
        
        const XclExpChTypeGroup* pFirstTypeGroup = GetFirstTypeGroup().get();
        bool bPieChart = pFirstTypeGroup && (pFirstTypeGroup->GetTypeInfo().meTypeCateg == EXC_CHTYPECATEG_PIE);
        mxFramePos->GetFramePosData().maRect = bPieChart ? maData.maRect :
            CalcChartRectFromHmm( xPositioning->calculateDiagramPositionIncludingAxes() );
    }
    catch( Exception& )
    {
    }

    
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
    
    rxChAxis.reset( new XclExpChAxis( GetChRoot(), nAxisType ) );
    sal_Int32 nApiAxisDim = rxChAxis->GetApiAxisDimension();
    sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();
    Reference< XAxis > xAxis = lclGetApiAxis( xCoordSystem, nApiAxisDim, nApiAxesSetIdx );
    Reference< XAxis > xCrossingAxis = lclGetApiAxis( xCoordSystem, nCrossingAxisDim, nApiAxesSetIdx );
    Reference< cssc::XAxis > xChart1Axis = lclGetApiChart1Axis( GetChartDocument(), nApiAxisDim, nApiAxesSetIdx );
    rxChAxis->Convert( xAxis, xCrossingAxis, xChart1Axis, rTypeInfo );

    
    Reference< XTitled > xTitled( xAxis, UNO_QUERY );
    rxChAxisTitle = lclCreateTitle( GetChRoot(), xTitled, nTitleTarget );
}

void XclExpChAxesSet::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnAxesSetId << maData.maRect;
}



static void lcl_getChartSubTitle(const Reference<XChartDocument>& xChartDoc,
                                 OUString& rSubTitle)
{
    Reference< ::com::sun::star::chart::XChartDocument > xChartDoc1(xChartDoc, UNO_QUERY);
    if (!xChartDoc1.is())
        return;

    Reference< XPropertySet > xProp(xChartDoc1->getSubTitle(), UNO_QUERY);
    if (!xProp.is())
        return;

    OUString aTitle;
    Any any = xProp->getPropertyValue("String");
    if (any >>= aTitle)
        rSubTitle = aTitle;
}

XclExpChChart::XclExpChChart( const XclExpRoot& rRoot,
        Reference< XChartDocument > xChartDoc, const Rectangle& rChartRect ) :
    XclExpChGroupBase( XclExpChRoot( rRoot, *this ), EXC_CHFRBLOCK_TYPE_CHART, EXC_ID_CHCHART, 16 )
{
    Size aPtSize = OutputDevice::LogicToLogic( rChartRect.GetSize(), MapMode( MAP_100TH_MM ), MapMode( MAP_POINT ) );
    
    maRect.mnX = maRect.mnY = 0;
    maRect.mnWidth = static_cast< sal_Int32 >( aPtSize.Width() << 16 );
    maRect.mnHeight = static_cast< sal_Int32 >( aPtSize.Height() << 16 );

    
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_SHOWVISIBLEONLY, false );
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_MANPLOTAREA );
    maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_SKIP;

    
    mxPrimAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_PRIMARY ) );
    mxSecnAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_SECONDARY ) );

    if( xChartDoc.is() )
    {
        Reference< XDiagram > xDiagram = xChartDoc->getFirstDiagram();

        
        ScfPropertySet aDiagramProp( xDiagram );
        bool bIncludeHidden = aDiagramProp.GetBoolProperty( EXC_CHPROP_INCLUDEHIDDENCELLS );
        ::set_flag( maProps.mnFlags,  EXC_CHPROPS_SHOWVISIBLEONLY, !bIncludeHidden );

        
        InitConversion( xChartDoc, rChartRect );

        
        ScfPropertySet aFrameProp( xChartDoc->getPageBackground() );
        mxFrame = lclCreateFrame( GetChRoot(), aFrameProp, EXC_CHOBJTYPE_BACKGROUND );

        
        Reference< XTitled > xTitled( xChartDoc, UNO_QUERY );
        OUString aSubTitle;
        lcl_getChartSubTitle(xChartDoc, aSubTitle);
        mxTitle = lclCreateTitle( GetChRoot(), xTitled, EXC_CHOBJLINK_TITLE,
                                  !aSubTitle.isEmpty() ? &aSubTitle : NULL );

        
        sal_uInt16 nFreeGroupIdx = mxPrimAxesSet->Convert( xDiagram, 0 );
        if( !mxPrimAxesSet->Is3dChart() )
            mxSecnAxesSet->Convert( xDiagram, nFreeGroupIdx );

        
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
    
    if( GetBiff() == EXC_BIFF8 )
        ::set_flag( maProps.mnFlags, EXC_CHPROPS_USEMANPLOTAREA );
}

void XclExpChChart::WriteSubRecords( XclExpStream& rStrm )
{
    
    lclSaveRecord( rStrm, mxFrame );

    
    maSeries.Save( rStrm );

    
    rStrm.StartRecord( EXC_ID_CHPROPERTIES, 4 );
    rStrm << maProps.mnFlags << maProps.mnEmptyMode << sal_uInt8( 0 );
    rStrm.EndRecord();

    
    sal_uInt16 nUsedAxesSets = mxSecnAxesSet->IsValidAxesSet() ? 2 : 1;
    XclExpUInt16Record( EXC_ID_CHUSEDAXESSETS, nUsedAxesSets ).Save( rStrm );
    mxPrimAxesSet->Save( rStrm );
    if( mxSecnAxesSet->IsValidAxesSet() )
        mxSecnAxesSet->Save( rStrm );

    
    lclSaveRecord( rStrm, mxTitle );
    maLabels.Save( rStrm );
}

void XclExpChChart::WriteBody( XclExpStream& rStrm )
{
     rStrm << maRect;
}



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
            
            mxObjMgr->StartSheet();
            
            mxObjRecs = mxObjMgr->ProcessDrawing( xShapes );
            
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
