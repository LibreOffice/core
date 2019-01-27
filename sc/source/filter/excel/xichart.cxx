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

#include <xichart.hxx>

#include <algorithm>
#include <memory>
#include <utility>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/LinearRegressionCurve.hpp>
#include <com/sun/star/chart2/ExponentialRegressionCurve.hpp>
#include <com/sun/star/chart2/LogarithmicRegressionCurve.hpp>
#include <com/sun/star/chart2/PotentialRegressionCurve.hpp>
#include <com/sun/star/chart2/PolynomialRegressionCurve.hpp>
#include <com/sun/star/chart2/MovingAverageRegressionCurve.hpp>
#include <com/sun/star/chart2/CartesianCoordinateSystem2d.hpp>
#include <com/sun/star/chart2/CartesianCoordinateSystem3d.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/LogarithmicScaling.hpp>
#include <com/sun/star/chart2/LinearScaling.hpp>
#include <com/sun/star/chart2/PolarCoordinateSystem2d.hpp>
#include <com/sun/star/chart2/PolarCoordinateSystem3d.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <comphelper/processfactory.hxx>
#include <o3tl/numeric.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <sal/log.hxx>
#include <tools/helpers.hxx>

#include <document.hxx>
#include <drwlayer.hxx>
#include <tokenarray.hxx>
#include <compiler.hxx>
#include <reftokenhelper.hxx>
#include <chartlis.hxx>
#include <xltracer.hxx>
#include <xltools.hxx>
#include <xistream.hxx>
#include <xiformula.hxx>
#include <xistyle.hxx>
#include <xipage.hxx>
#include <xiview.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::drawing::XShape;

using namespace ::com::sun::star::chart2;

using ::com::sun::star::chart2::data::XDataProvider;
using ::com::sun::star::chart2::data::XDataReceiver;
using ::com::sun::star::chart2::data::XDataSequence;
using ::com::sun::star::chart2::data::XDataSink;
using ::com::sun::star::chart2::data::XLabeledDataSequence;
using ::com::sun::star::chart2::data::LabeledDataSequence;

using ::formula::FormulaToken;
using ::formula::FormulaTokenArrayPlainIterator;
using ::std::unique_ptr;

namespace cssc = ::com::sun::star::chart;
namespace cssc2 = ::com::sun::star::chart2;

// Helpers ====================================================================

namespace {

XclImpStream& operator>>( XclImpStream& rStrm, XclChRectangle& rRect )
{
    rRect.mnX = rStrm.ReadInt32();
    rRect.mnY = rStrm.ReadInt32();
    rRect.mnWidth = rStrm.ReadInt32();
    rRect.mnHeight = rStrm.ReadInt32();
    return rStrm;
}

void lclSetValueOrClearAny( Any& rAny, double fValue, bool bClear )
{
    if( bClear )
        rAny.clear();
    else
        rAny <<= fValue;
}

void lclSetExpValueOrClearAny( Any& rAny, double fValue, bool bLogScale, bool bClear )
{
    if( !bClear && bLogScale )
        fValue = pow( 10.0, fValue );
    lclSetValueOrClearAny( rAny, fValue, bClear );
}

double lclGetSerialDay( const XclImpRoot& rRoot, sal_uInt16 nValue, sal_uInt16 nTimeUnit )
{
    switch( nTimeUnit )
    {
        case EXC_CHDATERANGE_DAYS:
            return nValue;
        case EXC_CHDATERANGE_MONTHS:
            return rRoot.GetDoubleFromDateTime( Date( 1, static_cast< sal_uInt16 >( 1 + nValue % 12 ), static_cast< sal_uInt16 >( rRoot.GetBaseYear() + nValue / 12 ) ) );
        case EXC_CHDATERANGE_YEARS:
            return rRoot.GetDoubleFromDateTime( Date( 1, 1, static_cast< sal_uInt16 >( rRoot.GetBaseYear() + nValue ) ) );
        default:
            OSL_ENSURE( false, "lclGetSerialDay - unexpected time unit" );
    }
    return nValue;
}

void lclConvertTimeValue( const XclImpRoot& rRoot, Any& rAny, sal_uInt16 nValue, bool bAuto, sal_uInt16 nTimeUnit )
{
    if( bAuto )
        rAny.clear();
    else
        rAny <<= lclGetSerialDay( rRoot, nValue, nTimeUnit );
}

sal_Int32 lclGetApiTimeUnit( sal_uInt16 nTimeUnit )
{
    switch( nTimeUnit )
    {
        case EXC_CHDATERANGE_DAYS:      return cssc::TimeUnit::DAY;
        case EXC_CHDATERANGE_MONTHS:    return cssc::TimeUnit::MONTH;
        case EXC_CHDATERANGE_YEARS:     return cssc::TimeUnit::YEAR;
        default:                        OSL_ENSURE( false, "lclGetApiTimeUnit - unexpected time unit" );
    }
    return cssc::TimeUnit::DAY;
}

void lclConvertTimeInterval( Any& rInterval, sal_uInt16 nValue, bool bAuto, sal_uInt16 nTimeUnit )
{
    if( bAuto || (nValue == 0) )
        rInterval.clear();
    else
        rInterval <<= cssc::TimeInterval( nValue, lclGetApiTimeUnit( nTimeUnit ) );
}

} // namespace

// Common =====================================================================

/** Stores global data needed in various classes of the Chart import filter. */
struct XclImpChRootData : public XclChRootData
{
    XclImpChChart&      mrChartData;            /// The chart data object.

    explicit     XclImpChRootData( XclImpChChart& rChartData ) : mrChartData( rChartData ) {}
};

XclImpChRoot::XclImpChRoot( const XclImpRoot& rRoot, XclImpChChart& rChartData ) :
    XclImpRoot( rRoot ),
    mxChData( new XclImpChRootData( rChartData ) )
{
}

XclImpChRoot::~XclImpChRoot()
{
}

XclImpChChart& XclImpChRoot::GetChartData() const
{
    return mxChData->mrChartData;
}

const XclChTypeInfo& XclImpChRoot::GetChartTypeInfo( XclChTypeId eType ) const
{
    return mxChData->mxTypeInfoProv->GetTypeInfo( eType );
}

const XclChTypeInfo& XclImpChRoot::GetChartTypeInfo( sal_uInt16 nRecId ) const
{
    return mxChData->mxTypeInfoProv->GetTypeInfoFromRecId( nRecId );
}

const XclChFormatInfo& XclImpChRoot::GetFormatInfo( XclChObjectType eObjType ) const
{
    return mxChData->mxFmtInfoProv->GetFormatInfo( eObjType );
}

Color XclImpChRoot::GetFontAutoColor() const
{
    return GetPalette().GetColor( EXC_COLOR_CHWINDOWTEXT );
}

Color XclImpChRoot::GetSeriesLineAutoColor( sal_uInt16 nFormatIdx ) const
{
    return GetPalette().GetColor( XclChartHelper::GetSeriesLineAutoColorIdx( nFormatIdx ) );
}

Color XclImpChRoot::GetSeriesFillAutoColor( sal_uInt16 nFormatIdx ) const
{
    const XclImpPalette& rPal = GetPalette();
    Color aColor = rPal.GetColor( XclChartHelper::GetSeriesFillAutoColorIdx( nFormatIdx ) );
    sal_uInt8 nTrans = XclChartHelper::GetSeriesFillAutoTransp( nFormatIdx );
    return ScfTools::GetMixedColor( aColor, rPal.GetColor( EXC_COLOR_CHWINDOWBACK ), nTrans );
}

void XclImpChRoot::InitConversion( const Reference<XChartDocument>& xChartDoc, const tools::Rectangle& rChartRect ) const
{
    // create formatting object tables
    mxChData->InitConversion( GetRoot(), xChartDoc, rChartRect );

    // lock the model to suppress any internal updates
    if( xChartDoc.is() )
        xChartDoc->lockControllers();

    SfxObjectShell* pDocShell = GetDocShell();
    Reference< XDataReceiver > xDataRec( xChartDoc, UNO_QUERY );
    if( pDocShell && xDataRec.is() )
    {
        // create and register a data provider
        Reference< XDataProvider > xDataProv(
            ScfApiHelper::CreateInstance( pDocShell, SERVICE_CHART2_DATAPROVIDER ), UNO_QUERY );
        if( xDataProv.is() )
            xDataRec->attachDataProvider( xDataProv );
        // attach the number formatter
        Reference< XNumberFormatsSupplier > xNumFmtSupp( pDocShell->GetModel(), UNO_QUERY );
        if( xNumFmtSupp.is() )
            xDataRec->attachNumberFormatsSupplier( xNumFmtSupp );
    }
}

void XclImpChRoot::FinishConversion( XclImpDffConverter& rDffConv ) const
{
    rDffConv.Progress( EXC_CHART_PROGRESS_SIZE );
    // unlock the model
    Reference< XModel > xModel( mxChData->mxChartDoc, UNO_QUERY );
    if( xModel.is() )
        xModel->unlockControllers();
    rDffConv.Progress( EXC_CHART_PROGRESS_SIZE );

    mxChData->FinishConversion();
}

Reference< XDataProvider > XclImpChRoot::GetDataProvider() const
{
    return mxChData->mxChartDoc->getDataProvider();
}

Reference< XShape > XclImpChRoot::GetTitleShape( const XclChTextKey& rTitleKey ) const
{
    return mxChData->GetTitleShape( rTitleKey );
}

sal_Int32 XclImpChRoot::CalcHmmFromChartX( sal_Int32 nPosX ) const
{
    return static_cast< sal_Int32 >( mxChData->mfUnitSizeX * nPosX + mxChData->mnBorderGapX + 0.5 );
}

sal_Int32 XclImpChRoot::CalcHmmFromChartY( sal_Int32 nPosY ) const
{
    return static_cast< sal_Int32 >( mxChData->mfUnitSizeY * nPosY + mxChData->mnBorderGapY + 0.5 );
}

css::awt::Rectangle XclImpChRoot::CalcHmmFromChartRect( const XclChRectangle& rRect ) const
{
    return css::awt::Rectangle(
        CalcHmmFromChartX( rRect.mnX ),
        CalcHmmFromChartY( rRect.mnY ),
        CalcHmmFromChartX( rRect.mnWidth ),
        CalcHmmFromChartY( rRect.mnHeight ) );
}

double XclImpChRoot::CalcRelativeFromHmmX( sal_Int32 nPosX ) const
{
    const long nWidth = mxChData->maChartRect.GetWidth();
    if (!nWidth)
        throw o3tl::divide_by_zero();
    return static_cast<double>(nPosX) / nWidth;
}

double XclImpChRoot::CalcRelativeFromHmmY( sal_Int32 nPosY ) const
{
    const long nHeight = mxChData->maChartRect.GetHeight();
    if (!nHeight)
        throw o3tl::divide_by_zero();
    return static_cast<double >(nPosY) / nHeight;
}

double XclImpChRoot::CalcRelativeFromChartX( sal_Int32 nPosX ) const
{
    return CalcRelativeFromHmmX( CalcHmmFromChartX( nPosX ) );
}

double XclImpChRoot::CalcRelativeFromChartY( sal_Int32 nPosY ) const
{
    return CalcRelativeFromHmmY( CalcHmmFromChartY( nPosY ) );
}

void XclImpChRoot::ConvertLineFormat( ScfPropertySet& rPropSet,
        const XclChLineFormat& rLineFmt, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteLineProperties(
        rPropSet, *mxChData->mxLineDashTable, rLineFmt, ePropMode );
}

void XclImpChRoot::ConvertAreaFormat( ScfPropertySet& rPropSet,
        const XclChAreaFormat& rAreaFmt, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteAreaProperties( rPropSet, rAreaFmt, ePropMode );
}

void XclImpChRoot::ConvertEscherFormat( ScfPropertySet& rPropSet,
        const XclChEscherFormat& rEscherFmt, const XclChPicFormat* pPicFmt,
        sal_uInt32 nDffFillType, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteEscherProperties( rPropSet,
        *mxChData->mxGradientTable, *mxChData->mxBitmapTable,
        rEscherFmt, pPicFmt, nDffFillType, ePropMode );
}

void XclImpChRoot::ConvertFont( ScfPropertySet& rPropSet,
        sal_uInt16 nFontIdx, const Color* pFontColor ) const
{
    GetFontBuffer().WriteFontProperties( rPropSet, EXC_FONTPROPSET_CHART, nFontIdx, pFontColor );
}

void XclImpChRoot::ConvertPieRotation( ScfPropertySet& rPropSet, sal_uInt16 nAngle )
{
    sal_Int32 nApiRot = (450 - (nAngle % 360)) % 360;
    rPropSet.SetProperty( EXC_CHPROP_STARTINGANGLE, nApiRot );
}

XclImpChGroupBase::~XclImpChGroupBase()
{
}

void XclImpChGroupBase::ReadRecordGroup( XclImpStream& rStrm )
{
    // read contents of the header record
    ReadHeaderRecord( rStrm );

    // only read sub records, if the next record is a CHBEGIN
    if( rStrm.GetNextRecId() == EXC_ID_CHBEGIN )
    {
        // read the CHBEGIN record, may be used for special initial processing
        rStrm.StartNextRecord();
        ReadSubRecord( rStrm );

        // read the nested records
        bool bLoop = true;
        while( bLoop && rStrm.StartNextRecord() )
        {
            sal_uInt16 nRecId = rStrm.GetRecId();
            bLoop = nRecId != EXC_ID_CHEND;
            // skip unsupported nested blocks
            if( nRecId == EXC_ID_CHBEGIN )
                SkipBlock( rStrm );
            else
                ReadSubRecord( rStrm );
        }
    }
    /*  Returns with current CHEND record or unchanged stream, if no record
        group present. In every case another call to StartNextRecord() will go
        to next record of interest. */
}

void XclImpChGroupBase::SkipBlock( XclImpStream& rStrm )
{
    OSL_ENSURE( rStrm.GetRecId() == EXC_ID_CHBEGIN, "XclImpChGroupBase::SkipBlock - no CHBEGIN record" );
    // do nothing if current record is not CHBEGIN
    bool bLoop = rStrm.GetRecId() == EXC_ID_CHBEGIN;
    while( bLoop && rStrm.StartNextRecord() )
    {
        sal_uInt16 nRecId = rStrm.GetRecId();
        bLoop = nRecId != EXC_ID_CHEND;
        // skip nested record groups
        if( nRecId == EXC_ID_CHBEGIN )
            SkipBlock( rStrm );
    }
}

// Frame formatting ===========================================================

void XclImpChFramePos::ReadChFramePos( XclImpStream& rStrm )
{
    maData.mnTLMode = rStrm.ReaduInt16();
    maData.mnBRMode = rStrm.ReaduInt16();
    /*  According to the spec, the upper 16 bits of all members in the
        rectangle are unused and may contain garbage. */
    maData.maRect.mnX = rStrm.ReadInt16(); rStrm.Ignore( 2 );
    maData.maRect.mnY = rStrm.ReadInt16(); rStrm.Ignore( 2 );
    maData.maRect.mnWidth = rStrm.ReadInt16(); rStrm.Ignore( 2 );
    maData.maRect.mnHeight = rStrm.ReadInt16(); rStrm.Ignore( 2 );
}

void XclImpChLineFormat::ReadChLineFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maColor;
    maData.mnPattern = rStrm.ReaduInt16();
    maData.mnWeight = rStrm.ReadInt16();
    maData.mnFlags = rStrm.ReaduInt16();

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
        // BIFF8: index into palette used instead of RGB data
        maData.maColor = rRoot.GetPalette().GetColor( rStrm.ReaduInt16() );
}

void XclImpChLineFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx ) const
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    if( IsAuto() )
    {
        XclChLineFormat aLineFmt;
        aLineFmt.maColor = (eObjType == EXC_CHOBJTYPE_LINEARSERIES) ?
            rRoot.GetSeriesLineAutoColor( nFormatIdx ) :
            rRoot.GetPalette().GetColor( rFmtInfo.mnAutoLineColorIdx );
        aLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
        aLineFmt.mnWeight = rFmtInfo.mnAutoLineWeight;
        rRoot.ConvertLineFormat( rPropSet, aLineFmt, rFmtInfo.mePropMode );
    }
    else
    {
        rRoot.ConvertLineFormat( rPropSet, maData, rFmtInfo.mePropMode );
    }
}

void XclImpChAreaFormat::ReadChAreaFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maPattColor >> maData.maBackColor;
    maData.mnPattern = rStrm.ReaduInt16();
    maData.mnFlags = rStrm.ReaduInt16();

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        // BIFF8: index into palette used instead of RGB data
        const XclImpPalette& rPal = rRoot.GetPalette();
        maData.maPattColor = rPal.GetColor( rStrm.ReaduInt16() );
        maData.maBackColor = rPal.GetColor( rStrm.ReaduInt16());
    }
}

void XclImpChAreaFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx ) const
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    if( IsAuto() )
    {
        XclChAreaFormat aAreaFmt;
        aAreaFmt.maPattColor = (eObjType == EXC_CHOBJTYPE_FILLEDSERIES) ?
            rRoot.GetSeriesFillAutoColor( nFormatIdx ) :
            rRoot.GetPalette().GetColor( rFmtInfo.mnAutoPattColorIdx );
        aAreaFmt.mnPattern = EXC_PATT_SOLID;
        rRoot.ConvertAreaFormat( rPropSet, aAreaFmt, rFmtInfo.mePropMode );
    }
    else
    {
        rRoot.ConvertAreaFormat( rPropSet, maData, rFmtInfo.mePropMode );
    }
}

XclImpChEscherFormat::XclImpChEscherFormat( const XclImpRoot& rRoot ) :
    mnDffFillType( mso_fillSolid )
{
    maData.mxItemSet.reset(
        new SfxItemSet( rRoot.GetDoc().GetDrawLayer()->GetItemPool() ) );
}

void XclImpChEscherFormat::ReadHeaderRecord( XclImpStream& rStrm )
{
    // read from stream - CHESCHERFORMAT uses own ID for record continuation
    XclImpDffPropSet aPropSet( rStrm.GetRoot() );
    rStrm.ResetRecord( true, rStrm.GetRecId() );
    rStrm >> aPropSet;
    // get the data
    aPropSet.FillToItemSet( *maData.mxItemSet );
    // get fill type from DFF property set
    mnDffFillType = aPropSet.GetPropertyValue( DFF_Prop_fillType );
}

void XclImpChEscherFormat::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHPICFORMAT:
            maPicFmt.mnBmpMode = rStrm.ReaduInt16();
            rStrm.Ignore( 2 );
            maPicFmt.mnFlags = rStrm.ReaduInt16();
            maPicFmt.mfScale = rStrm.ReadDouble();
        break;
    }
}

void XclImpChEscherFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, bool bUsePicFmt ) const
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    rRoot.ConvertEscherFormat( rPropSet, maData, bUsePicFmt ? &maPicFmt : nullptr, mnDffFillType, rFmtInfo.mePropMode );
}

XclImpChFrameBase::XclImpChFrameBase( const XclChFormatInfo& rFmtInfo )
{
    if( rFmtInfo.mbCreateDefFrame ) switch( rFmtInfo.meDefFrameType )
    {
        case EXC_CHFRAMETYPE_AUTO:
            mxLineFmt.reset( new XclImpChLineFormat );
            if( rFmtInfo.mbIsFrame )
                mxAreaFmt.reset( new XclImpChAreaFormat );
        break;
        case EXC_CHFRAMETYPE_INVISIBLE:
        {
            XclChLineFormat aLineFmt;
            ::set_flag( aLineFmt.mnFlags, EXC_CHLINEFORMAT_AUTO, false );
            aLineFmt.mnPattern = EXC_CHLINEFORMAT_NONE;
            mxLineFmt.reset( new XclImpChLineFormat( aLineFmt ) );
            if( rFmtInfo.mbIsFrame )
            {
                XclChAreaFormat aAreaFmt;
                ::set_flag( aAreaFmt.mnFlags, EXC_CHAREAFORMAT_AUTO, false );
                aAreaFmt.mnPattern = EXC_PATT_NONE;
                mxAreaFmt.reset( new XclImpChAreaFormat( aAreaFmt ) );
            }
        }
        break;
        default:
            OSL_FAIL( "XclImpChFrameBase::XclImpChFrameBase - unknown frame type" );
    }
}

void XclImpChFrameBase::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHLINEFORMAT:
            mxLineFmt.reset( new XclImpChLineFormat );
            mxLineFmt->ReadChLineFormat( rStrm );
        break;
        case EXC_ID_CHAREAFORMAT:
            mxAreaFmt.reset( new XclImpChAreaFormat );
            mxAreaFmt->ReadChAreaFormat( rStrm );
        break;
        case EXC_ID_CHESCHERFORMAT:
            mxEscherFmt.reset( new XclImpChEscherFormat( rStrm.GetRoot() ) );
            mxEscherFmt->ReadRecordGroup( rStrm );
        break;
    }
}

void XclImpChFrameBase::ConvertLineBase( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx ) const
{
    if( mxLineFmt )
        mxLineFmt->Convert( rRoot, rPropSet, eObjType, nFormatIdx );
}

void XclImpChFrameBase::ConvertAreaBase( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx, bool bUsePicFmt ) const
{
    if( rRoot.GetFormatInfo( eObjType ).mbIsFrame )
    {
        // CHESCHERFORMAT overrides CHAREAFORMAT (even if it is auto)
        if( mxEscherFmt )
            mxEscherFmt->Convert( rRoot, rPropSet, eObjType, bUsePicFmt );
        else if( mxAreaFmt )
            mxAreaFmt->Convert( rRoot, rPropSet, eObjType, nFormatIdx );
    }
}

void XclImpChFrameBase::ConvertFrameBase( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx, bool bUsePicFmt ) const
{
    ConvertLineBase( rRoot, rPropSet, eObjType, nFormatIdx );
    ConvertAreaBase( rRoot, rPropSet, eObjType, nFormatIdx, bUsePicFmt );
}

XclImpChFrame::XclImpChFrame( const XclImpChRoot& rRoot, XclChObjectType eObjType ) :
    XclImpChFrameBase( rRoot.GetFormatInfo( eObjType ) ),
    XclImpChRoot( rRoot ),
    meObjType( eObjType )
{
}

void XclImpChFrame::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.mnFormat = rStrm.ReaduInt16();
    maData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChFrame::UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData )
{
    const XclImpPalette& rPal = GetPalette();

    if( rLineData.IsVisible() && (!mxLineFmt || !mxLineFmt->HasLine()) )
    {
        // line formatting
        XclChLineFormat aLineFmt;
        aLineFmt.maColor = rPal.GetColor( rLineData.mnColorIdx );
        switch( rLineData.mnStyle )
        {
            case EXC_OBJ_LINE_SOLID:        aLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;        break;
            case EXC_OBJ_LINE_DASH:         aLineFmt.mnPattern = EXC_CHLINEFORMAT_DASH;         break;
            case EXC_OBJ_LINE_DOT:          aLineFmt.mnPattern = EXC_CHLINEFORMAT_DOT;          break;
            case EXC_OBJ_LINE_DASHDOT:      aLineFmt.mnPattern = EXC_CHLINEFORMAT_DASHDOT;      break;
            case EXC_OBJ_LINE_DASHDOTDOT:   aLineFmt.mnPattern = EXC_CHLINEFORMAT_DASHDOTDOT;   break;
            case EXC_OBJ_LINE_MEDTRANS:     aLineFmt.mnPattern = EXC_CHLINEFORMAT_MEDTRANS;     break;
            case EXC_OBJ_LINE_DARKTRANS:    aLineFmt.mnPattern = EXC_CHLINEFORMAT_DARKTRANS;    break;
            case EXC_OBJ_LINE_LIGHTTRANS:   aLineFmt.mnPattern = EXC_CHLINEFORMAT_LIGHTTRANS;   break;
            case EXC_OBJ_LINE_NONE:         aLineFmt.mnPattern = EXC_CHLINEFORMAT_NONE;         break;
            default:                        aLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
        }
        switch( rLineData.mnWidth )
        {
            case EXC_OBJ_LINE_HAIR:     aLineFmt.mnWeight = EXC_CHLINEFORMAT_HAIR;      break;
            case EXC_OBJ_LINE_THIN:     aLineFmt.mnWeight = EXC_CHLINEFORMAT_SINGLE;    break;
            case EXC_OBJ_LINE_MEDIUM:   aLineFmt.mnWeight = EXC_CHLINEFORMAT_DOUBLE;    break;
            case EXC_OBJ_LINE_THICK:    aLineFmt.mnWeight = EXC_CHLINEFORMAT_TRIPLE;    break;
            default:                    aLineFmt.mnWeight = EXC_CHLINEFORMAT_HAIR;
        }
        ::set_flag( aLineFmt.mnFlags, EXC_CHLINEFORMAT_AUTO, rLineData.IsAuto() );
        mxLineFmt.reset( new XclImpChLineFormat( aLineFmt ) );
    }

    if( rFillData.IsFilled() && (!mxAreaFmt || !mxAreaFmt->HasArea()) && !mxEscherFmt )
    {
        // area formatting
        XclChAreaFormat aAreaFmt;
        aAreaFmt.maPattColor = rPal.GetColor( rFillData.mnPattColorIdx );
        aAreaFmt.maBackColor = rPal.GetColor( rFillData.mnBackColorIdx );
        aAreaFmt.mnPattern = rFillData.mnPattern;
        ::set_flag( aAreaFmt.mnFlags, EXC_CHAREAFORMAT_AUTO, rFillData.IsAuto() );
        mxAreaFmt.reset( new XclImpChAreaFormat( aAreaFmt ) );
    }
}

void XclImpChFrame::Convert( ScfPropertySet& rPropSet, bool bUsePicFmt ) const
{
    ConvertFrameBase( GetChRoot(), rPropSet, meObjType, EXC_CHDATAFORMAT_UNKNOWN, bUsePicFmt );
}

// Source links ===============================================================

namespace {

/** Creates a labeled data sequence object, adds link for series title if present. */
Reference< XLabeledDataSequence > lclCreateLabeledDataSequence(
        const XclImpChSourceLinkRef& xValueLink, const OUString& rValueRole,
        const XclImpChSourceLink* pTitleLink = nullptr )
{
    // create data sequence for values and title
    Reference< XDataSequence > xValueSeq;
    if( xValueLink )
        xValueSeq = xValueLink->CreateDataSequence( rValueRole );
    Reference< XDataSequence > xTitleSeq;
    if( pTitleLink )
        xTitleSeq = pTitleLink->CreateDataSequence( EXC_CHPROP_ROLE_LABEL );

    // create the labeled data sequence, if values or title are present
    Reference< XLabeledDataSequence > xLabeledSeq;
    if( xValueSeq.is() || xTitleSeq.is() )
        xLabeledSeq.set( LabeledDataSequence::create(comphelper::getProcessComponentContext()), UNO_QUERY );
    if( xLabeledSeq.is() )
    {
        if( xValueSeq.is() )
            xLabeledSeq->setValues( xValueSeq );
        if( xTitleSeq.is() )
            xLabeledSeq->setLabel( xTitleSeq );
    }
    return xLabeledSeq;
}

} // namespace

XclImpChSourceLink::XclImpChSourceLink( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

XclImpChSourceLink::~XclImpChSourceLink()
{
}

void XclImpChSourceLink::ReadChSourceLink( XclImpStream& rStrm )
{
    maData.mnDestType = rStrm.ReaduInt8();
    maData.mnLinkType = rStrm.ReaduInt8();
    maData.mnFlags = rStrm.ReaduInt16();
    maData.mnNumFmtIdx = rStrm.ReaduInt16();

    mxTokenArray.reset();
    if( GetLinkType() == EXC_CHSRCLINK_WORKSHEET )
    {
        // read token array
        XclTokenArray aXclTokArr;
        rStrm >> aXclTokArr;

        // convert BIFF formula tokens to Calc token array
        if( std::unique_ptr<ScTokenArray> pTokens = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CHART, aXclTokArr ) )
            mxTokenArray = std::move( pTokens );
    }

    // try to read a following CHSTRING record
    if( (rStrm.GetNextRecId() == EXC_ID_CHSTRING) && rStrm.StartNextRecord() )
    {
        mxString.reset( new XclImpString );
        rStrm.Ignore( 2 );
        mxString->Read( rStrm, XclStrFlags::EightBitLength | XclStrFlags::SeparateFormats );
    }
}

void XclImpChSourceLink::SetString( const OUString& rString )
{
    if( !mxString )
        mxString.reset( new XclImpString );
    mxString->SetText( rString );
}

void XclImpChSourceLink::SetTextFormats( const XclFormatRunVec& rFormats )
{
    if( mxString )
        mxString->SetFormats( rFormats );
}

sal_uInt16 XclImpChSourceLink::GetCellCount() const
{
    sal_uInt32 nCellCount = 0;
    if( mxTokenArray )
    {
        FormulaTokenArrayPlainIterator aIter(*mxTokenArray);
        for( const FormulaToken* pToken = aIter.First(); pToken; pToken = aIter.Next() )
        {
            switch( pToken->GetType() )
            {
                case ::formula::svSingleRef:
                case ::formula::svExternalSingleRef:
                    // single cell
                    ++nCellCount;
                break;
                case ::formula::svDoubleRef:
                case ::formula::svExternalDoubleRef:
                {
                    // cell range
                    const ScComplexRefData& rComplexRef = *pToken->GetDoubleRef();
                    ScAddress aAbs1 = rComplexRef.Ref1.toAbs(ScAddress());
                    ScAddress aAbs2 = rComplexRef.Ref2.toAbs(ScAddress());
                    sal_uInt32 nTabs = static_cast<sal_uInt32>(aAbs2.Tab() - aAbs1.Tab() + 1);
                    sal_uInt32 nCols = static_cast<sal_uInt32>(aAbs2.Col() - aAbs1.Col() + 1);
                    sal_uInt32 nRows = static_cast<sal_uInt32>(aAbs2.Row() - aAbs1.Row() + 1);
                    nCellCount += nCols * nRows * nTabs;
                }
                break;
                default: ;
            }
        }
    }
    return limit_cast< sal_uInt16 >( nCellCount );
}

void XclImpChSourceLink::ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const
{
    bool bLinkToSource = ::get_flag( maData.mnFlags, EXC_CHSRCLINK_NUMFMT );
    sal_uInt32 nScNumFmt = bLinkToSource ? GetNumFmtBuffer().GetScFormat( maData.mnNumFmtIdx ) : NUMBERFORMAT_ENTRY_NOT_FOUND;
    OUString aPropName = bPercent ? OUString( EXC_CHPROP_PERCENTAGENUMFMT ) : OUString( EXC_CHPROP_NUMBERFORMAT );
    if( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
        rPropSet.SetProperty( aPropName, static_cast< sal_Int32 >( nScNumFmt ) );
    else
        // restore 'link to source' at data point (series may contain manual number format)
        rPropSet.SetAnyProperty( aPropName, Any() );
}

Reference< XDataSequence > XclImpChSourceLink::CreateDataSequence( const OUString& rRole ) const
{
    Reference< XDataSequence > xDataSeq;
    Reference< XDataProvider > xDataProv = GetDataProvider();
    if( xDataProv.is() )
    {
        if ( mxTokenArray )
        {
            ScCompiler aComp( &GetDocRef(), ScAddress(), *mxTokenArray, GetDoc().GetGrammar() );
            OUStringBuffer aRangeRep;
            aComp.CreateStringFromTokenArray( aRangeRep );
            try
            {
                xDataSeq = xDataProv->createDataSequenceByRangeRepresentation( aRangeRep.makeStringAndClear() );
                // set sequence role
                ScfPropertySet aSeqProp( xDataSeq );
                aSeqProp.SetProperty( EXC_CHPROP_ROLE, rRole );
            }
            catch( Exception& )
            {
    //            OSL_FAIL( "XclImpChSourceLink::CreateDataSequence - cannot create data sequence" );
            }
        }
        else if( rRole == EXC_CHPROP_ROLE_LABEL && mxString && !mxString->GetText().isEmpty() )
        {
            try
            {
                OUString aString("\"");
                xDataSeq = xDataProv->createDataSequenceByRangeRepresentation( aString + mxString->GetText() + aString );
                // set sequence role
                ScfPropertySet aSeqProp( xDataSeq );
                aSeqProp.SetProperty( EXC_CHPROP_ROLE, rRole );
            }
            catch( Exception& ) { }
        }
    }
    return xDataSeq;
}

Sequence< Reference< XFormattedString > > XclImpChSourceLink::CreateStringSequence(
        const XclImpChRoot& rRoot, sal_uInt16 nLeadFontIdx, const Color& rLeadFontColor ) const
{
    ::std::vector< Reference< XFormattedString > > aStringVec;
    if( mxString )
    {
        for( XclImpStringIterator aIt( *mxString ); aIt.Is(); ++aIt )
        {
            Reference< css::chart2::XFormattedString2 > xFmtStr = css::chart2::FormattedString::create( comphelper::getProcessComponentContext() );
            // set text data
            xFmtStr->setString( aIt.GetPortionText() );

            // set font formatting and font color
            ScfPropertySet aStringProp( xFmtStr );
            sal_uInt16 nFontIdx = aIt.GetPortionFont();
            if( (nFontIdx == EXC_FONT_NOTFOUND) && (aIt.GetPortionIndex() == 0) )
                // leading unformatted portion - use passed font settings
                rRoot.ConvertFont( aStringProp, nLeadFontIdx, &rLeadFontColor );
            else
                rRoot.ConvertFont( aStringProp, nFontIdx );

            // add string to vector of strings
            aStringVec.emplace_back(xFmtStr );
        }
    }
    return ScfApiHelper::VectorToSequence( aStringVec );
}

void XclImpChSourceLink::FillSourceLink( ::std::vector< ScTokenRef >& rTokens ) const
{
    if( !mxTokenArray )
        // no links to fill.
        return;

    FormulaTokenArrayPlainIterator aIter(*mxTokenArray);
    for (FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        ScTokenRef pToken(p->Clone());
        if (ScRefTokenHelper::isRef(pToken))
            // This is a reference token.  Store it.
            ScRefTokenHelper::join(rTokens, pToken, ScAddress());
    }
}

// Text =======================================================================

XclImpChFontBase::~XclImpChFontBase()
{
}

void XclImpChFontBase::ConvertFontBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    Color aFontColor = GetFontColor();
    rRoot.ConvertFont( rPropSet, GetFontIndex(), &aFontColor );
}

void XclImpChFontBase::ConvertRotationBase( ScfPropertySet& rPropSet, bool bSupportsStacked ) const
{
    XclChPropSetHelper::WriteRotationProperties( rPropSet, GetRotation(), bSupportsStacked );
}

XclImpChFont::XclImpChFont() :
    mnFontIdx( EXC_FONT_NOTFOUND )
{
}

void XclImpChFont::ReadChFont( XclImpStream& rStrm )
{
    mnFontIdx = rStrm.ReaduInt16();
}

XclImpChText::XclImpChText( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChText::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.mnHAlign = rStrm.ReaduInt8();
    maData.mnVAlign = rStrm.ReaduInt8();
    maData.mnBackMode = rStrm.ReaduInt16();
    rStrm   >> maData.maTextColor
            >> maData.maRect;
    maData.mnFlags = rStrm.ReaduInt16();

    if( GetBiff() == EXC_BIFF8 )
    {
        // BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( rStrm.ReaduInt16() );
        // placement and rotation
        maData.mnFlags2 = rStrm.ReaduInt16();
        maData.mnRotation = rStrm.ReaduInt16();
    }
    else
    {
        // BIFF2-BIFF7: get rotation from text orientation
        sal_uInt8 nOrient = ::extract_value< sal_uInt8 >( maData.mnFlags, 8, 3 );
        maData.mnRotation = XclTools::GetXclRotFromOrient( nOrient );
    }
}

void XclImpChText::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAMEPOS:
            mxFramePos.reset( new XclImpChFramePos );
            mxFramePos->ReadChFramePos( rStrm );
        break;
        case EXC_ID_CHFONT:
            mxFont.reset( new XclImpChFont );
            mxFont->ReadChFont( rStrm );
        break;
        case EXC_ID_CHFORMATRUNS:
            if( GetBiff() == EXC_BIFF8 )
                XclImpString::ReadFormats( rStrm, maFormats );
        break;
        case EXC_ID_CHSOURCELINK:
            mxSrcLink.reset( new XclImpChSourceLink( GetChRoot() ) );
            mxSrcLink->ReadChSourceLink( rStrm );
        break;
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_TEXT ) );
            mxFrame->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHOBJECTLINK:
            maObjLink.mnTarget = rStrm.ReaduInt16();
            maObjLink.maPointPos.mnSeriesIdx = rStrm.ReaduInt16();
            maObjLink.maPointPos.mnPointIdx = rStrm.ReaduInt16();
        break;
        case EXC_ID_CHFRLABELPROPS:
            ReadChFrLabelProps( rStrm );
        break;
        case EXC_ID_CHEND:
            if( mxSrcLink && !maFormats.empty() )
                mxSrcLink->SetTextFormats( maFormats );
        break;
    }
}

sal_uInt16 XclImpChText::GetFontIndex() const
{
    return mxFont ? mxFont->GetFontIndex() : EXC_FONT_NOTFOUND;
}

Color XclImpChText::GetFontColor() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTEXT_AUTOCOLOR ) ? GetFontAutoColor() : maData.maTextColor;
}

sal_uInt16 XclImpChText::GetRotation() const
{
    return maData.mnRotation;
}

void XclImpChText::SetString( const OUString& rString )
{
    if( !mxSrcLink )
        mxSrcLink.reset( new XclImpChSourceLink( GetChRoot() ) );
    mxSrcLink->SetString( rString );
}

void XclImpChText::UpdateText( const XclImpChText* pParentText )
{
    if( pParentText )
    {
        // update missing members
        if( !mxFrame )
            mxFrame = pParentText->mxFrame;
        if( !mxFont )
        {
            mxFont = pParentText->mxFont;
            // text color is taken from CHTEXT record, not from font in CHFONT
            ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOCOLOR, ::get_flag( pParentText->maData.mnFlags, EXC_CHTEXT_AUTOCOLOR ) );
            maData.maTextColor = pParentText->maData.maTextColor;
        }
    }
}

void XclImpChText::UpdateDataLabel( bool bCateg, bool bValue, bool bPercent )
{
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG,     bCateg );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWVALUE,     bValue );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWPERCENT,   bPercent );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEGPERC, bCateg && bPercent );
    ::set_flag( maData.mnFlags, EXC_CHTEXT_DELETED,       !bCateg && !bValue && !bPercent );
}

void XclImpChText::ConvertFont( ScfPropertySet& rPropSet ) const
{
    ConvertFontBase( GetChRoot(), rPropSet );
}

void XclImpChText::ConvertRotation( ScfPropertySet& rPropSet, bool bSupportsStacked ) const
{
    ConvertRotationBase( rPropSet, bSupportsStacked );
}

void XclImpChText::ConvertFrame( ScfPropertySet& rPropSet ) const
{
    if( mxFrame )
        mxFrame->Convert( rPropSet );
}

void XclImpChText::ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const
{
    if( mxSrcLink )
        mxSrcLink->ConvertNumFmt( rPropSet, bPercent );
}

void XclImpChText::ConvertDataLabel( ScfPropertySet& rPropSet, const XclChTypeInfo& rTypeInfo, const ScfPropertySet* pGlobalPropSet ) const
{
    // existing CHFRLABELPROPS record wins over flags from CHTEXT
    sal_uInt16 nShowFlags = mxLabelProps ? mxLabelProps->mnFlags : maData.mnFlags;
    sal_uInt16 SHOWANYCATEG   = mxLabelProps ? EXC_CHFRLABELPROPS_SHOWCATEG : (EXC_CHTEXT_SHOWCATEGPERC | EXC_CHTEXT_SHOWCATEG);
    sal_uInt16 SHOWANYVALUE   = mxLabelProps ? EXC_CHFRLABELPROPS_SHOWVALUE : EXC_CHTEXT_SHOWVALUE;
    sal_uInt16 SHOWANYPERCENT = mxLabelProps ? EXC_CHFRLABELPROPS_SHOWPERCENT : (EXC_CHTEXT_SHOWPERCENT | EXC_CHTEXT_SHOWCATEGPERC);
    sal_uInt16 SHOWANYBUBBLE  = mxLabelProps ? EXC_CHFRLABELPROPS_SHOWBUBBLE : EXC_CHTEXT_SHOWBUBBLE;

    // get raw flags for label values
    bool bShowNone    = IsDeleted();
    bool bShowCateg   = !bShowNone && ::get_flag( nShowFlags, SHOWANYCATEG );
    bool bShowPercent = !bShowNone && ::get_flag( nShowFlags, SHOWANYPERCENT );
    bool bShowValue   = !bShowNone && ::get_flag( nShowFlags, SHOWANYVALUE );
    bool bShowBubble  = !bShowNone && ::get_flag( nShowFlags, SHOWANYBUBBLE );

    // adjust to Chart2 behaviour
    if( rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES )
         bShowValue = bShowBubble;  // Chart2 bubble charts show bubble size if 'ShowValue' is set

    // other flags
    bool bShowAny = bShowValue || bShowPercent || bShowCateg;
    bool bShowSymbol = bShowAny && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWSYMBOL );

    // create API struct for label values, set API label separator
    cssc2::DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol );
    rPropSet.SetProperty( EXC_CHPROP_LABEL, aPointLabel );
    OUString aSep = mxLabelProps ? mxLabelProps->maSeparator : OUString('\n');
    if( aSep.isEmpty() )
        aSep = "; ";
    rPropSet.SetStringProperty( EXC_CHPROP_LABELSEPARATOR, aSep );

    // text properties of attached label
    if( bShowAny )
    {
        ConvertFont( rPropSet );
        ConvertRotation( rPropSet, false );
        // label placement
        using namespace cssc::DataLabelPlacement;
        sal_Int32 nPlacement = rTypeInfo.mnDefaultLabelPos;
        switch( ::extract_value< sal_uInt16 >( maData.mnFlags2, 0, 4 ) )
        {
            case EXC_CHTEXT_POS_DEFAULT:    nPlacement = rTypeInfo.mnDefaultLabelPos;   break;
            case EXC_CHTEXT_POS_OUTSIDE:    nPlacement = OUTSIDE;                       break;
            case EXC_CHTEXT_POS_INSIDE:     nPlacement = INSIDE;                        break;
            case EXC_CHTEXT_POS_CENTER:     nPlacement = CENTER;                        break;
            case EXC_CHTEXT_POS_AXIS:       nPlacement = NEAR_ORIGIN;                   break;
            case EXC_CHTEXT_POS_ABOVE:      nPlacement = TOP;                           break;
            case EXC_CHTEXT_POS_BELOW:      nPlacement = BOTTOM;                        break;
            case EXC_CHTEXT_POS_LEFT:       nPlacement = LEFT;                          break;
            case EXC_CHTEXT_POS_RIGHT:      nPlacement = RIGHT;                         break;
            case EXC_CHTEXT_POS_AUTO:       nPlacement = AVOID_OVERLAP;                 break;
        }
        sal_Int32 nGlobalPlacement = 0;
        if ( ( nPlacement == rTypeInfo.mnDefaultLabelPos ) && pGlobalPropSet &&
             pGlobalPropSet->GetProperty( nGlobalPlacement, EXC_CHPROP_LABELPLACEMENT ) )
            nPlacement = nGlobalPlacement;

        rPropSet.SetProperty( EXC_CHPROP_LABELPLACEMENT, nPlacement );
        // label number format (percentage format wins over value format)
        if( bShowPercent || bShowValue )
            ConvertNumFmt( rPropSet, bShowPercent );
    }
}

Reference< XTitle > XclImpChText::CreateTitle() const
{
    Reference< XTitle > xTitle;
    if( mxSrcLink && mxSrcLink->HasString() )
    {
        // create the formatted strings
        Sequence< Reference< XFormattedString > > aStringSeq(
            mxSrcLink->CreateStringSequence( GetChRoot(), GetFontIndex(), GetFontColor() ) );
        if( aStringSeq.hasElements() )
        {
            // create the title object
            xTitle.set( ScfApiHelper::CreateInstance( SERVICE_CHART2_TITLE ), UNO_QUERY );
            if( xTitle.is() )
            {
                // set the formatted strings
                xTitle->setText( aStringSeq );
                // more title formatting properties
                ScfPropertySet aTitleProp( xTitle );
                ConvertFrame( aTitleProp );
                ConvertRotation( aTitleProp, true );
            }
        }
    }
    return xTitle;
}

void XclImpChText::ConvertTitlePosition( const XclChTextKey& rTitleKey ) const
{
    if( !mxFramePos ) return;

    const XclChFramePos& rPosData = mxFramePos->GetFramePosData();
    OSL_ENSURE( (rPosData.mnTLMode == EXC_CHFRAMEPOS_PARENT) && (rPosData.mnBRMode == EXC_CHFRAMEPOS_PARENT),
        "XclImpChText::ConvertTitlePosition - unexpected frame position mode" );

    /*  Check if title is moved manually. To get the actual position of the
        title, we do some kind of hack and use the values from the CHTEXT
        record, effectively ignoring the contents of the CHFRAMEPOS record
        which contains the position relative to the default title position
        (according to the spec, the CHFRAMEPOS supersedes the CHTEXT record).
        Especially when it comes to axis titles, things would become very
        complicated here, because the relative title position is stored in a
        measurement unit that is dependent on the size of the inner plot area,
        the interpretation of the X and Y coordinate is dependent on the
        direction of the axis, and in 3D charts, and the title default
        positions are dependent on the 3D view settings (rotation, elevation,
        and perspective). Thus, it is easier to assume that the creator has
        written out the correct absolute position and size of the title in the
        CHTEXT record. This is assured by checking that the shape size stored
        in the CHTEXT record is non-zero. */
    if( (rPosData.mnTLMode == EXC_CHFRAMEPOS_PARENT) &&
        ((rPosData.maRect.mnX != 0) || (rPosData.maRect.mnY != 0)) &&
        (maData.maRect.mnWidth > 0) && (maData.maRect.mnHeight > 0) ) try
    {
        Reference< XShape > xTitleShape( GetTitleShape( rTitleKey ), UNO_SET_THROW );
        // the call to XShape.getSize() may recalc the chart view
        css::awt::Size aTitleSize = xTitleShape->getSize();
        // rotated titles need special handling...
        sal_Int32 nScRot = XclTools::GetScRotation( GetRotation(), 0 );
        double fRad = nScRot * F_PI18000;
        double fSin = fabs( sin( fRad ) );
        // calculate the title position from the values in the CHTEXT record
        css::awt::Point aTitlePos(
            CalcHmmFromChartX( maData.maRect.mnX ),
            CalcHmmFromChartY( maData.maRect.mnY ) );
        // add part of height to X direction, if title is rotated down (clockwise)
        if( nScRot > 18000 )
            aTitlePos.X += static_cast< sal_Int32 >( fSin * aTitleSize.Height + 0.5 );
        // add part of width to Y direction, if title is rotated up (counterclockwise)
        else if( nScRot > 0 )
            aTitlePos.Y += static_cast< sal_Int32 >( fSin * aTitleSize.Width + 0.5 );
        // set the resulting position at the title shape
        xTitleShape->setPosition( aTitlePos );
    }
    catch( Exception& )
    {
    }
}

void XclImpChText::ReadChFrLabelProps( XclImpStream& rStrm )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        mxLabelProps.reset( new XclChFrLabelProps );
        sal_uInt16 nSepLen;
        rStrm.Ignore( 12 );
        mxLabelProps->mnFlags = rStrm.ReaduInt16();
        nSepLen = rStrm.ReaduInt16();
        if( nSepLen > 0 )
            mxLabelProps->maSeparator = rStrm.ReadUniString( nSepLen );
    }
}

namespace {

void lclUpdateText( XclImpChTextRef& rxText, const XclImpChText* xDefText )
{
    if (rxText)
        rxText->UpdateText( xDefText );
    else if (xDefText)
    {
        XclImpChTextRef xNew(new XclImpChText(*xDefText));
        rxText = xNew;
    }
}

void lclFinalizeTitle( XclImpChTextRef& rxTitle, const XclImpChText* pDefText, const OUString& rAutoTitle )
{
    /*  Do not update a title, if it is not visible (if rxTitle is null).
        Existing reference indicates enabled title. */
    if( rxTitle )
    {
        if( !rxTitle->HasString() )
            rxTitle->SetString( rAutoTitle );
        if( rxTitle->HasString() )
            rxTitle->UpdateText(pDefText);
        else
            rxTitle.reset();
    }
}

} // namespace

// Data series ================================================================

void XclImpChMarkerFormat::ReadChMarkerFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maLineColor >> maData.maFillColor;
    maData.mnMarkerType = rStrm.ReaduInt16();
    maData.mnFlags = rStrm.ReaduInt16();

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        // BIFF8: index into palette used instead of RGB data
        const XclImpPalette& rPal = rRoot.GetPalette();
        maData.maLineColor = rPal.GetColor( rStrm.ReaduInt16() );
        maData.maFillColor = rPal.GetColor( rStrm.ReaduInt16() );
        // marker size
        maData.mnMarkerSize = rStrm.ReaduInt32();
    }
}

void XclImpChMarkerFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, sal_Int16 nLineWeight ) const
{
    if( IsAuto() )
    {
        XclChMarkerFormat aMarkerFmt;
        // line and fill color of the symbol are equal to series line color
        //TODO: Excel sets no fill color for specific symbols (e.g. cross)
        aMarkerFmt.maLineColor = aMarkerFmt.maFillColor = rRoot.GetSeriesLineAutoColor( nFormatIdx );
        switch( nLineWeight )
        {
            case EXC_CHLINEFORMAT_HAIR:     aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_HAIRSIZE;      break;
            case EXC_CHLINEFORMAT_SINGLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_SINGLESIZE;    break;
            case EXC_CHLINEFORMAT_DOUBLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_DOUBLESIZE;    break;
            case EXC_CHLINEFORMAT_TRIPLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_TRIPLESIZE;    break;
            default:                        aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_SINGLESIZE;
        }
        aMarkerFmt.mnMarkerType = XclChartHelper::GetAutoMarkerType( nFormatIdx );
        XclChPropSetHelper::WriteMarkerProperties( rPropSet, aMarkerFmt );
    }
    else
    {
        XclChPropSetHelper::WriteMarkerProperties( rPropSet, maData );
    }
}

void XclImpChMarkerFormat::ConvertColor( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    Color aLineColor = IsAuto() ? rRoot.GetSeriesLineAutoColor( nFormatIdx ) : maData.maFillColor;
    rPropSet.SetColorProperty( EXC_CHPROP_COLOR, aLineColor );
}

XclImpChPieFormat::XclImpChPieFormat() :
    mnPieDist( 0 )
{
}

void XclImpChPieFormat::ReadChPieFormat( XclImpStream& rStrm )
{
    mnPieDist = rStrm.ReaduInt16();
}

void XclImpChPieFormat::Convert( ScfPropertySet& rPropSet ) const
{
    double fApiDist = ::std::min< double >( mnPieDist / 100.0, 1.0 );
    rPropSet.SetProperty( EXC_CHPROP_OFFSET, fApiDist );
}

XclImpChSeriesFormat::XclImpChSeriesFormat() :
    mnFlags( 0 )
{
}

void XclImpChSeriesFormat::ReadChSeriesFormat( XclImpStream& rStrm )
{
    mnFlags = rStrm.ReaduInt16();
}

void XclImpCh3dDataFormat::ReadCh3dDataFormat( XclImpStream& rStrm )
{
    maData.mnBase = rStrm.ReaduInt8();
    maData.mnTop = rStrm.ReaduInt8();
}

void XclImpCh3dDataFormat::Convert( ScfPropertySet& rPropSet ) const
{
    using namespace ::com::sun::star::chart2::DataPointGeometry3D;
    sal_Int32 nApiType = (maData.mnBase == EXC_CH3DDATAFORMAT_RECT) ?
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? CUBOID : PYRAMID) :
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? CYLINDER : CONE);
    rPropSet.SetProperty( EXC_CHPROP_GEOMETRY3D, nApiType );
}

XclImpChAttachedLabel::XclImpChAttachedLabel( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    mnFlags( 0 )
{
}

void XclImpChAttachedLabel::ReadChAttachedLabel( XclImpStream& rStrm )
{
    mnFlags = rStrm.ReaduInt16();
}

XclImpChTextRef XclImpChAttachedLabel::CreateDataLabel( const XclImpChText* pParent ) const
{
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYVALUE = EXC_CHATTLABEL_SHOWVALUE;
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYPERCENT = EXC_CHATTLABEL_SHOWPERCENT | EXC_CHATTLABEL_SHOWCATEGPERC;
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYCATEG = EXC_CHATTLABEL_SHOWCATEG | EXC_CHATTLABEL_SHOWCATEGPERC;

    XclImpChTextRef xLabel( pParent ? new XclImpChText( *pParent ) : new XclImpChText( GetChRoot() ) );
    xLabel->UpdateDataLabel(
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYCATEG ),
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYVALUE ),
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYPERCENT ) );
    return xLabel;
}

XclImpChDataFormat::XclImpChDataFormat( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChDataFormat::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.maPointPos.mnPointIdx = rStrm.ReaduInt16();
    maData.maPointPos.mnSeriesIdx = rStrm.ReaduInt16();
    maData.mnFormatIdx = rStrm.ReaduInt16();
    maData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChDataFormat::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHMARKERFORMAT:
            mxMarkerFmt.reset( new XclImpChMarkerFormat );
            mxMarkerFmt->ReadChMarkerFormat( rStrm );
        break;
        case EXC_ID_CHPIEFORMAT:
            mxPieFmt.reset( new XclImpChPieFormat );
            mxPieFmt->ReadChPieFormat( rStrm );
        break;
        case EXC_ID_CHSERIESFORMAT:
            mxSeriesFmt.reset( new XclImpChSeriesFormat );
            mxSeriesFmt->ReadChSeriesFormat( rStrm );
        break;
        case EXC_ID_CH3DDATAFORMAT:
            mx3dDataFmt.reset( new XclImpCh3dDataFormat );
            mx3dDataFmt->ReadCh3dDataFormat( rStrm );
        break;
        case EXC_ID_CHATTACHEDLABEL:
            mxAttLabel.reset( new XclImpChAttachedLabel( GetChRoot() ) );
            mxAttLabel->ReadChAttachedLabel( rStrm );
        break;
        default:
            XclImpChFrameBase::ReadSubRecord( rStrm );
    }
}

void XclImpChDataFormat::SetPointPos( const XclChDataPointPos& rPointPos, sal_uInt16 nFormatIdx )
{
    maData.maPointPos = rPointPos;
    maData.mnFormatIdx = nFormatIdx;
}

void XclImpChDataFormat::UpdateGroupFormat( const XclChExtTypeInfo& rTypeInfo )
{
    // remove formats not used for the current chart type
    RemoveUnusedFormats( rTypeInfo );
}

void XclImpChDataFormat::UpdateSeriesFormat( const XclChExtTypeInfo& rTypeInfo, const XclImpChDataFormat* pGroupFmt )
{
    // update missing formats from passed chart type group format
    if( pGroupFmt )
    {
        if( !mxLineFmt )
            mxLineFmt = pGroupFmt->mxLineFmt;
        if( !mxAreaFmt && !mxEscherFmt )
        {
            mxAreaFmt = pGroupFmt->mxAreaFmt;
            mxEscherFmt = pGroupFmt->mxEscherFmt;
        }
        if( !mxMarkerFmt )
            mxMarkerFmt = pGroupFmt->mxMarkerFmt;
        if( !mxPieFmt )
            mxPieFmt = pGroupFmt->mxPieFmt;
        if( !mxSeriesFmt )
            mxSeriesFmt = pGroupFmt->mxSeriesFmt;
        if( !mx3dDataFmt )
            mx3dDataFmt = pGroupFmt->mx3dDataFmt;
        if( !mxAttLabel )
            mxAttLabel = pGroupFmt->mxAttLabel;
    }

    /*  Create missing but required formats. Existing line, area, and marker
        format objects are needed to create automatic series formatting. */
    if( !mxLineFmt )
        mxLineFmt.reset( new XclImpChLineFormat );
    if( !mxAreaFmt && !mxEscherFmt )
        mxAreaFmt.reset( new XclImpChAreaFormat );
    if( !mxMarkerFmt )
        mxMarkerFmt.reset( new XclImpChMarkerFormat );

    // remove formats not used for the current chart type
    RemoveUnusedFormats( rTypeInfo );
    // update data label
    UpdateDataLabel( pGroupFmt );
}

void XclImpChDataFormat::UpdatePointFormat( const XclChExtTypeInfo& rTypeInfo, const XclImpChDataFormat* pSeriesFmt )
{
    // remove formats if they are automatic in this and in the passed series format
    if( pSeriesFmt )
    {
        if( IsAutoLine() && pSeriesFmt->IsAutoLine() )
            mxLineFmt.reset();
        if( IsAutoArea() && pSeriesFmt->IsAutoArea() )
            mxAreaFmt.reset();
        if( IsAutoMarker() && pSeriesFmt->IsAutoMarker() )
            mxMarkerFmt.reset();
        mxSeriesFmt.reset();
    }

    // Excel ignores 3D bar format for single data points
    mx3dDataFmt.reset();
    // remove point line formats for linear chart types, TODO: implement in OOChart
    if( !rTypeInfo.IsSeriesFrameFormat() )
        mxLineFmt.reset();

    // remove formats not used for the current chart type
    RemoveUnusedFormats( rTypeInfo );
    // update data label
    UpdateDataLabel( pSeriesFmt );
}

void XclImpChDataFormat::UpdateTrendLineFormat()
{
    if( !mxLineFmt )
        mxLineFmt.reset( new XclImpChLineFormat );
    mxAreaFmt.reset();
    mxEscherFmt.reset();
    mxMarkerFmt.reset();
    mxPieFmt.reset();
    mxSeriesFmt.reset();
    mx3dDataFmt.reset();
    mxAttLabel.reset();
    // update data label
    UpdateDataLabel( nullptr );
}

void XclImpChDataFormat::Convert( ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo, const ScfPropertySet* pGlobalPropSet ) const
{
    /*  Line and area format.
        #i71810# If the data points are filled with bitmaps, textures, or
        patterns, then only bar charts will use the CHPICFORMAT record to
        determine stacking/stretching mode. All other chart types ignore this
        record and always use the property 'fill-type' from the DFF property
        set (stretched for bitmaps, and stacked for textures and patterns). */
    bool bUsePicFmt = rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR;
    ConvertFrameBase( GetChRoot(), rPropSet, rTypeInfo.GetSeriesObjectType(), maData.mnFormatIdx, bUsePicFmt );

    // #i83151# only hair lines in 3D charts with filled data points
    if( rTypeInfo.mb3dChart && rTypeInfo.IsSeriesFrameFormat() && mxLineFmt && mxLineFmt->HasLine() )
        rPropSet.SetProperty< sal_Int32 >( "BorderWidth", 0 );

    // other formatting
    if( mxMarkerFmt )
        mxMarkerFmt->Convert( GetChRoot(), rPropSet, maData.mnFormatIdx, GetLineWeight() );
    if( mxPieFmt )
        mxPieFmt->Convert( rPropSet );
    if( mx3dDataFmt )
        mx3dDataFmt->Convert( rPropSet );
    if( mxLabel )
        mxLabel->ConvertDataLabel( rPropSet, rTypeInfo, pGlobalPropSet );

    // 3D settings
    rPropSet.SetProperty< sal_Int16 >( EXC_CHPROP_PERCENTDIAGONAL, 0 );

    /*  Special case: set marker color as line color, if series line is not
        visible. This makes the color visible in the marker area.
        TODO: remove this if OOChart supports own colors in markers. */
    if( !rTypeInfo.IsSeriesFrameFormat() && !HasLine() && mxMarkerFmt )
        mxMarkerFmt->ConvertColor( GetChRoot(), rPropSet, maData.mnFormatIdx );
}

void XclImpChDataFormat::ConvertLine( ScfPropertySet& rPropSet, XclChObjectType eObjType ) const
{
    ConvertLineBase( GetChRoot(), rPropSet, eObjType );
}

void XclImpChDataFormat::ConvertArea( ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    ConvertAreaBase( GetChRoot(), rPropSet, EXC_CHOBJTYPE_FILLEDSERIES, nFormatIdx );
}

void XclImpChDataFormat::RemoveUnusedFormats( const XclChExtTypeInfo& rTypeInfo )
{
    // data point marker only in linear 2D charts
    if( rTypeInfo.IsSeriesFrameFormat() )
        mxMarkerFmt.reset();
    // pie format only in pie/donut charts
    if( rTypeInfo.meTypeCateg != EXC_CHTYPECATEG_PIE )
        mxPieFmt.reset();
    // 3D format only in 3D bar charts
    if( !rTypeInfo.mb3dChart || (rTypeInfo.meTypeCateg != EXC_CHTYPECATEG_BAR) )
        mx3dDataFmt.reset();
}

void XclImpChDataFormat::UpdateDataLabel( const XclImpChDataFormat* pParentFmt )
{
    /*  CHTEXT groups linked to data labels override existing CHATTACHEDLABEL
        records. Only if there is a CHATTACHEDLABEL record without a CHTEXT
        group, the contents of the CHATTACHEDLABEL record are used. In this
        case a new CHTEXT group is created and filled with the settings from
        the CHATTACHEDLABEL record. */
    const XclImpChText* pDefText = nullptr;
    if (pParentFmt)
        pDefText = pParentFmt->GetDataLabel();
    if (!pDefText)
        pDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_DATALABEL );
    if (mxLabel)
        mxLabel->UpdateText(pDefText);
    else if (mxAttLabel)
        mxLabel = mxAttLabel->CreateDataLabel( pDefText );
}

XclImpChSerTrendLine::XclImpChSerTrendLine( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChSerTrendLine::ReadChSerTrendLine( XclImpStream& rStrm )
{
    maData.mnLineType = rStrm.ReaduInt8();
    maData.mnOrder = rStrm.ReaduInt8();
    maData.mfIntercept = rStrm.ReadDouble();
    maData.mnShowEquation = rStrm.ReaduInt8();
    maData.mnShowRSquared = rStrm.ReaduInt8();
    maData.mfForecastFor = rStrm.ReadDouble();
    maData.mfForecastBack = rStrm.ReadDouble();
}

Reference< XRegressionCurve > XclImpChSerTrendLine::CreateRegressionCurve() const
{
    // trend line type
    Reference< XRegressionCurve > xRegCurve;
    switch( maData.mnLineType )
    {
        case EXC_CHSERTREND_POLYNOMIAL:
            if( maData.mnOrder == 1 )
            {
                xRegCurve = LinearRegressionCurve::create( comphelper::getProcessComponentContext() );
            } else {
                xRegCurve = PolynomialRegressionCurve::create( comphelper::getProcessComponentContext() );
            }
        break;
        case EXC_CHSERTREND_EXPONENTIAL:
            xRegCurve = ExponentialRegressionCurve::create( comphelper::getProcessComponentContext() );
        break;
        case EXC_CHSERTREND_LOGARITHMIC:
            xRegCurve = LogarithmicRegressionCurve::create( comphelper::getProcessComponentContext() );
        break;
        case EXC_CHSERTREND_POWER:
            xRegCurve = PotentialRegressionCurve::create( comphelper::getProcessComponentContext() );
        break;
        case EXC_CHSERTREND_MOVING_AVG:
            xRegCurve = MovingAverageRegressionCurve::create( comphelper::getProcessComponentContext() );
        break;
    }

    // trend line formatting
    if( xRegCurve.is() && mxDataFmt )
    {
        ScfPropertySet aPropSet( xRegCurve );
        mxDataFmt->ConvertLine( aPropSet, EXC_CHOBJTYPE_TRENDLINE );

        aPropSet.SetProperty(EXC_CHPROP_CURVENAME, maTrendLineName);
        aPropSet.SetProperty(EXC_CHPROP_POLYNOMIAL_DEGREE, static_cast<sal_Int32> (maData.mnOrder) );
        aPropSet.SetProperty(EXC_CHPROP_MOVING_AVERAGE_PERIOD, static_cast<sal_Int32> (maData.mnOrder) );
        aPropSet.SetProperty(EXC_CHPROP_EXTRAPOLATE_FORWARD, maData.mfForecastFor);
        aPropSet.SetProperty(EXC_CHPROP_EXTRAPOLATE_BACKWARD, maData.mfForecastBack);

        bool bForceIntercept = rtl::math::isFinite(maData.mfIntercept);
        aPropSet.SetProperty(EXC_CHPROP_FORCE_INTERCEPT, bForceIntercept);
        if (bForceIntercept)
        {
            aPropSet.SetProperty(EXC_CHPROP_INTERCEPT_VALUE, maData.mfIntercept);
        }

        // #i83100# show equation and correlation coefficient
        ScfPropertySet aLabelProp( xRegCurve->getEquationProperties() );
        aLabelProp.SetBoolProperty( EXC_CHPROP_SHOWEQUATION, maData.mnShowEquation != 0 );
        aLabelProp.SetBoolProperty( EXC_CHPROP_SHOWCORRELATION, maData.mnShowRSquared != 0 );

        // #i83100# formatting of the equation text box
        if (const XclImpChText* pLabel = mxDataFmt->GetDataLabel())
        {
            pLabel->ConvertFont( aLabelProp );
            pLabel->ConvertFrame( aLabelProp );
            pLabel->ConvertNumFmt( aLabelProp, false );
        }
    }

    return xRegCurve;
}

XclImpChSerErrorBar::XclImpChSerErrorBar( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChSerErrorBar::ReadChSerErrorBar( XclImpStream& rStrm )
{
    maData.mnBarType = rStrm.ReaduInt8();
    maData.mnSourceType = rStrm.ReaduInt8();
    maData.mnLineEnd = rStrm.ReaduInt8();
    rStrm.Ignore( 1 );
    maData.mfValue = rStrm.ReadDouble();
    maData.mnValueCount = rStrm.ReaduInt16();
}

void XclImpChSerErrorBar::SetSeriesData( XclImpChSourceLinkRef const & xValueLink, XclImpChDataFormatRef const & xDataFmt )
{
    mxValueLink = xValueLink;
    mxDataFmt = xDataFmt;
}

Reference< XLabeledDataSequence > XclImpChSerErrorBar::CreateValueSequence() const
{
    return lclCreateLabeledDataSequence( mxValueLink, XclChartHelper::GetErrorBarValuesRole( maData.mnBarType ) );
}

Reference< XPropertySet > XclImpChSerErrorBar::CreateErrorBar( const XclImpChSerErrorBar* pPosBar, const XclImpChSerErrorBar* pNegBar )
{
    Reference< XPropertySet > xErrorBar;

    if( const XclImpChSerErrorBar* pPrimaryBar = pPosBar ? pPosBar : pNegBar )
    {
        xErrorBar.set( ScfApiHelper::CreateInstance( SERVICE_CHART2_ERRORBAR ), UNO_QUERY );
        ScfPropertySet aBarProp( xErrorBar );

        // plus/minus bars visible?
        aBarProp.SetBoolProperty( EXC_CHPROP_SHOWPOSITIVEERROR, pPosBar != nullptr );
        aBarProp.SetBoolProperty( EXC_CHPROP_SHOWNEGATIVEERROR, pNegBar != nullptr );

        // type of displayed error
        switch( pPrimaryBar->maData.mnSourceType )
        {
            case EXC_CHSERERR_PERCENT:
                aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle::RELATIVE );
                aBarProp.SetProperty( EXC_CHPROP_POSITIVEERROR, pPrimaryBar->maData.mfValue );
                aBarProp.SetProperty( EXC_CHPROP_NEGATIVEERROR, pPrimaryBar->maData.mfValue );
            break;
            case EXC_CHSERERR_FIXED:
                aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle::ABSOLUTE );
                aBarProp.SetProperty( EXC_CHPROP_POSITIVEERROR, pPrimaryBar->maData.mfValue );
                aBarProp.SetProperty( EXC_CHPROP_NEGATIVEERROR, pPrimaryBar->maData.mfValue );
            break;
            case EXC_CHSERERR_STDDEV:
                aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle::STANDARD_DEVIATION );
                aBarProp.SetProperty( EXC_CHPROP_WEIGHT, pPrimaryBar->maData.mfValue );
            break;
            case EXC_CHSERERR_STDERR:
                aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle::STANDARD_ERROR );
            break;
            case EXC_CHSERERR_CUSTOM:
            {
                aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle::FROM_DATA );
                // attach data sequences to error bar
                Reference< XDataSink > xDataSink( xErrorBar, UNO_QUERY );
                if( xDataSink.is() )
                {
                    // create vector of all value sequences
                    ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
                    // add positive values
                    if( pPosBar )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = pPosBar->CreateValueSequence();
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    // add negative values
                    if( pNegBar )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = pNegBar->CreateValueSequence();
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    // attach labeled data sequences to series
                    if( aLabeledSeqVec.empty() )
                        xErrorBar.clear();
                    else
                        xDataSink->setData( ScfApiHelper::VectorToSequence( aLabeledSeqVec ) );
                }
            }
            break;
            default:
                xErrorBar.clear();
        }

        // error bar formatting
        if( pPrimaryBar->mxDataFmt && xErrorBar.is() )
            pPrimaryBar->mxDataFmt->ConvertLine( aBarProp, EXC_CHOBJTYPE_ERRORBAR );
    }

    return xErrorBar;
}

XclImpChSeries::XclImpChSeries( const XclImpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclImpChRoot( rRoot ),
    mnGroupIdx( EXC_CHSERGROUP_NONE ),
    mnSeriesIdx( nSeriesIdx ),
    mnParentIdx( EXC_CHSERIES_INVALID ),
    mbLabelDeleted( false )
{
}

void XclImpChSeries::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.mnCategType = rStrm.ReaduInt16();
    maData.mnValueType = rStrm.ReaduInt16();
    maData.mnCategCount = rStrm.ReaduInt16();
    maData.mnValueCount = rStrm.ReaduInt16();
    if( GetBiff() == EXC_BIFF8 )
    {
        maData.mnBubbleType = rStrm.ReaduInt16();
        maData.mnBubbleCount = rStrm.ReaduInt16();
    }
}

void XclImpChSeries::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHSOURCELINK:
            ReadChSourceLink( rStrm );
        break;
        case EXC_ID_CHDATAFORMAT:
            ReadChDataFormat( rStrm );
        break;
        case EXC_ID_CHSERGROUP:
            mnGroupIdx = rStrm.ReaduInt16();
        break;
        case EXC_ID_CHSERPARENT:
            ReadChSerParent( rStrm );
        break;
        case EXC_ID_CHSERTRENDLINE:
            ReadChSerTrendLine( rStrm );
        break;
        case EXC_ID_CHSERERRORBAR:
            ReadChSerErrorBar( rStrm );
        break;
        case EXC_ID_CHLEGENDEXCEPTION:
            ReadChLegendException( rStrm );
        break;
    }
}

void XclImpChSeries::SetDataFormat( const XclImpChDataFormatRef& xDataFmt )
{
    if (!xDataFmt)
        return;

    sal_uInt16 nPointIdx = xDataFmt->GetPointPos().mnPointIdx;
    if (nPointIdx == EXC_CHDATAFORMAT_ALLPOINTS)
    {
        if (mxSeriesFmt)
            // Don't overwrite the existing format.
            return;

        mxSeriesFmt = xDataFmt;
        if (HasParentSeries())
            return;

        XclImpChTypeGroupRef pTypeGroup = GetChartData().GetTypeGroup(mnGroupIdx);
        if (pTypeGroup)
            pTypeGroup->SetUsedFormatIndex(xDataFmt->GetFormatIdx());

        return;
    }

    if (nPointIdx >= EXC_CHDATAFORMAT_MAXPOINTCOUNT)
        // Above the max point count.  Bail out.
        return;

    XclImpChDataFormatMap::iterator itr = maPointFmts.lower_bound(nPointIdx);
    if (itr == maPointFmts.end() || maPointFmts.key_comp()(nPointIdx, itr->first))
    {
        // No object exists at this point index position.  Insert it.
        itr = maPointFmts.insert(itr, XclImpChDataFormatMap::value_type(nPointIdx, xDataFmt));
    }
}

void XclImpChSeries::SetDataLabel( const XclImpChTextRef& xLabel )
{
    if (!xLabel)
        return;

    sal_uInt16 nPointIdx = xLabel->GetPointPos().mnPointIdx;
    if ((nPointIdx != EXC_CHDATAFORMAT_ALLPOINTS) && (nPointIdx >= EXC_CHDATAFORMAT_MAXPOINTCOUNT))
        // Above the maximum allowed data points. Bail out.
        return;

    XclImpChTextMap::iterator itr = maLabels.lower_bound(nPointIdx);
    if (itr == maLabels.end() || maLabels.key_comp()(nPointIdx, itr->first))
    {
        // No object exists at this point index position.  Insert it.
        itr = maLabels.insert(itr, XclImpChTextMap::value_type(nPointIdx, xLabel));
    }
}

void XclImpChSeries::AddChildSeries( const XclImpChSeries& rSeries )
{
    OSL_ENSURE( !HasParentSeries(), "XclImpChSeries::AddChildSeries - not allowed for child series" );
    if (&rSeries == this)
    {
        SAL_WARN("sc.filter", "self add attempt");
        return;
    }

    /*  In Excel, trend lines and error bars are stored as own series. In Calc,
        these are properties of the parent series. This function adds the
        settings of the passed series to this series. */
    maTrendLines.insert( maTrendLines.end(), rSeries.maTrendLines.begin(), rSeries.maTrendLines.end() );
    for (auto const& it : rSeries.m_ErrorBars)
    {
        m_ErrorBars.insert(std::make_pair(it.first, std::make_unique<XclImpChSerErrorBar>(*it.second)));
    }
}

void XclImpChSeries::FinalizeDataFormats()
{
    if( HasParentSeries() )
    {
        // *** series is a child series, e.g. trend line or error bar ***

        // create missing series format
        if( !mxSeriesFmt )
            mxSeriesFmt = CreateDataFormat( EXC_CHDATAFORMAT_ALLPOINTS, 0 );

        if( mxSeriesFmt )
        {
            // #i83100# set text label format, e.g. for trend line equations
            XclImpChTextRef xLabel;
            XclImpChTextMap::iterator itr = maLabels.find(EXC_CHDATAFORMAT_ALLPOINTS);
            if (itr != maLabels.end())
                xLabel = itr->second;
            mxSeriesFmt->SetDataLabel(xLabel);
            // create missing automatic formats
            mxSeriesFmt->UpdateTrendLineFormat();
        }

        // copy series formatting to child objects
        for (auto const& trendLine : maTrendLines)
        {
            trendLine->SetDataFormat(mxSeriesFmt);
            if (mxTitleLink && mxTitleLink->HasString())
            {
                trendLine->SetTrendlineName(mxTitleLink->GetString());
            }
        }
        for (auto const& it : m_ErrorBars)
        {
            it.second->SetSeriesData( mxValueLink, mxSeriesFmt );
        }
    }
    else if( XclImpChTypeGroup* pTypeGroup = GetChartData().GetTypeGroup( mnGroupIdx ).get() )
    {
        // *** series is a regular data series ***

        // create missing series format
        if( !mxSeriesFmt )
        {
            // #i51639# use a new unused format index to create series default format
            sal_uInt16 nFormatIdx = pTypeGroup->PopUnusedFormatIndex();
            mxSeriesFmt = CreateDataFormat( EXC_CHDATAFORMAT_ALLPOINTS, nFormatIdx );
        }

        // set text labels to data formats
        for (auto const& label : maLabels)
        {
            sal_uInt16 nPointIdx = label.first;
            if (nPointIdx == EXC_CHDATAFORMAT_ALLPOINTS)
            {
                if (!mxSeriesFmt)
                    mxSeriesFmt = CreateDataFormat(nPointIdx, EXC_CHDATAFORMAT_DEFAULT);
                mxSeriesFmt->SetDataLabel(label.second);
            }
            else if (nPointIdx < EXC_CHDATAFORMAT_MAXPOINTCOUNT)
            {
                XclImpChDataFormatRef p;
                XclImpChDataFormatMap::iterator itr = maPointFmts.lower_bound(nPointIdx);
                if (itr == maPointFmts.end() || maPointFmts.key_comp()(nPointIdx, itr->first))
                {
                    // No object exists at this point index position.  Insert
                    // a new one.
                    p = CreateDataFormat(nPointIdx, EXC_CHDATAFORMAT_DEFAULT);
                    itr = maPointFmts.insert(
                        itr, XclImpChDataFormatMap::value_type(nPointIdx, p));
                }
                else
                    p = itr->second;
                p->SetDataLabel(label.second);
            }
        }

        // update series format (copy missing formatting from group default format)
        if( mxSeriesFmt )
            mxSeriesFmt->UpdateSeriesFormat( pTypeGroup->GetTypeInfo(), pTypeGroup->GetGroupFormat().get() );

        // update data point formats (removes unchanged automatic formatting)
        for (auto const& pointFormat : maPointFmts)
            pointFormat.second->UpdatePointFormat( pTypeGroup->GetTypeInfo(), mxSeriesFmt.get() );
    }
}

namespace {

/** Returns the property set of the specified data point. */
ScfPropertySet lclGetPointPropSet( Reference< XDataSeries > const & xDataSeries, sal_uInt16 nPointIdx )
{
    ScfPropertySet aPropSet;
    try
    {
        aPropSet.Set( xDataSeries->getDataPointByIndex( static_cast< sal_Int32 >( nPointIdx ) ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "lclGetPointPropSet - no data point property set" );
    }
    return aPropSet;
}

} // namespace

Reference< XLabeledDataSequence > XclImpChSeries::CreateValueSequence( const OUString& rValueRole ) const
{
    return lclCreateLabeledDataSequence( mxValueLink, rValueRole, mxTitleLink.get() );
}

Reference< XLabeledDataSequence > XclImpChSeries::CreateCategSequence( const OUString& rCategRole ) const
{
    return lclCreateLabeledDataSequence( mxCategLink, rCategRole );
}

Reference< XDataSeries > XclImpChSeries::CreateDataSeries() const
{
    Reference< XDataSeries > xDataSeries;
    if( const XclImpChTypeGroup* pTypeGroup = GetChartData().GetTypeGroup( mnGroupIdx ).get() )
    {
        const XclChExtTypeInfo& rTypeInfo = pTypeGroup->GetTypeInfo();

        // create the data series object
        xDataSeries.set( ScfApiHelper::CreateInstance( SERVICE_CHART2_DATASERIES ), UNO_QUERY );

        // attach data and title sequences to series
        Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
        if( xDataSink.is() )
        {
            // create vector of all value sequences
            ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
            // add Y values
            Reference< XLabeledDataSequence > xYValueSeq =
                CreateValueSequence( EXC_CHPROP_ROLE_YVALUES );
            if( xYValueSeq.is() )
                aLabeledSeqVec.push_back( xYValueSeq );
            // add X values
            if( !rTypeInfo.mbCategoryAxis )
            {
                Reference< XLabeledDataSequence > xXValueSeq =
                    CreateCategSequence( EXC_CHPROP_ROLE_XVALUES );
                if( xXValueSeq.is() )
                    aLabeledSeqVec.push_back( xXValueSeq );
                // add size values of bubble charts
                if( rTypeInfo.meTypeId == EXC_CHTYPEID_BUBBLES )
                {
                    Reference< XLabeledDataSequence > xSizeValueSeq =
                        lclCreateLabeledDataSequence( mxBubbleLink, EXC_CHPROP_ROLE_SIZEVALUES, mxTitleLink.get() );
                    if( xSizeValueSeq.is() )
                        aLabeledSeqVec.push_back( xSizeValueSeq );
                }
            }
            // attach labeled data sequences to series
            if( !aLabeledSeqVec.empty() )
                xDataSink->setData( ScfApiHelper::VectorToSequence( aLabeledSeqVec ) );
        }

        // series formatting
        ScfPropertySet aSeriesProp( xDataSeries );
        if( mxSeriesFmt )
            mxSeriesFmt->Convert( aSeriesProp, rTypeInfo );

        if (mbLabelDeleted)
            aSeriesProp.SetProperty(EXC_CHPROP_SHOWLEGENDENTRY, false);

        // trend lines
        ConvertTrendLines( xDataSeries );

        // error bars
        Reference< XPropertySet > xErrorBarX = CreateErrorBar( EXC_CHSERERR_XPLUS, EXC_CHSERERR_XMINUS );
        if( xErrorBarX.is() )
            aSeriesProp.SetProperty( EXC_CHPROP_ERRORBARX, xErrorBarX );
        Reference< XPropertySet > xErrorBarY = CreateErrorBar( EXC_CHSERERR_YPLUS, EXC_CHSERERR_YMINUS );
        if( xErrorBarY.is() )
            aSeriesProp.SetProperty( EXC_CHPROP_ERRORBARY, xErrorBarY );

        // own area formatting for every data point (TODO: varying line color not supported)
        bool bVarPointFmt = pTypeGroup->HasVarPointFormat() && rTypeInfo.IsSeriesFrameFormat();
        aSeriesProp.SetBoolProperty( EXC_CHPROP_VARYCOLORSBY, rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE );
        // #i91271# always set area formatting for every point in pie/doughnut charts
        if (mxSeriesFmt && mxValueLink && ((bVarPointFmt && mxSeriesFmt->IsAutoArea()) || (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE)))
        {
            for( sal_uInt16 nPointIdx = 0, nPointCount = mxValueLink->GetCellCount(); nPointIdx < nPointCount; ++nPointIdx )
            {
                ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, nPointIdx );
                mxSeriesFmt->ConvertArea( aPointProp, bVarPointFmt ? nPointIdx : mnSeriesIdx );
            }
        }

        // data point formatting
        for (auto const& pointFormat : maPointFmts)
        {
            ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, pointFormat.first );
            pointFormat.second->Convert( aPointProp, rTypeInfo, &aSeriesProp );
        }
    }
    return xDataSeries;
}

void XclImpChSeries::FillAllSourceLinks( ::std::vector< ScTokenRef >& rTokens ) const
{
    if( mxValueLink )
        mxValueLink->FillSourceLink( rTokens );
    if( mxCategLink )
        mxCategLink->FillSourceLink( rTokens );
    if( mxTitleLink )
        mxTitleLink->FillSourceLink( rTokens );
    if( mxBubbleLink )
        mxBubbleLink->FillSourceLink( rTokens );
}

void XclImpChSeries::ReadChSourceLink( XclImpStream& rStrm )
{
    XclImpChSourceLinkRef xSrcLink( new XclImpChSourceLink( GetChRoot() ) );
    xSrcLink->ReadChSourceLink( rStrm );
    switch( xSrcLink->GetDestType() )
    {
        case EXC_CHSRCLINK_TITLE:       mxTitleLink = xSrcLink;     break;
        case EXC_CHSRCLINK_VALUES:      mxValueLink = xSrcLink;     break;
        case EXC_CHSRCLINK_CATEGORY:    mxCategLink = xSrcLink;     break;
        case EXC_CHSRCLINK_BUBBLES:     mxBubbleLink = xSrcLink;    break;
    }
}

void XclImpChSeries::ReadChDataFormat( XclImpStream& rStrm )
{
    // #i51639# chart stores all data formats and assigns them later to the series
    GetChartData().ReadChDataFormat( rStrm );
}

void XclImpChSeries::ReadChSerParent( XclImpStream& rStrm )
{
    mnParentIdx = rStrm.ReaduInt16();
    // index to parent series is 1-based, convert it to 0-based
    if( mnParentIdx > 0 )
        --mnParentIdx;
    else
        mnParentIdx = EXC_CHSERIES_INVALID;
}

void XclImpChSeries::ReadChSerTrendLine( XclImpStream& rStrm )
{
    XclImpChSerTrendLineRef xTrendLine( new XclImpChSerTrendLine( GetChRoot() ) );
    xTrendLine->ReadChSerTrendLine( rStrm );
    maTrendLines.push_back( xTrendLine );
}

void XclImpChSeries::ReadChSerErrorBar( XclImpStream& rStrm )
{
    unique_ptr<XclImpChSerErrorBar> pErrorBar(new XclImpChSerErrorBar(GetChRoot()));
    pErrorBar->ReadChSerErrorBar(rStrm);
    sal_uInt8 nBarType = pErrorBar->GetBarType();
    m_ErrorBars.insert(std::make_pair(nBarType, std::move(pErrorBar)));
}

XclImpChDataFormatRef XclImpChSeries::CreateDataFormat( sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx )
{
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->SetPointPos( XclChDataPointPos( mnSeriesIdx, nPointIdx ), nFormatIdx );
    return xDataFmt;
}

void XclImpChSeries::ConvertTrendLines( Reference< XDataSeries > const & xDataSeries ) const
{
    Reference< XRegressionCurveContainer > xRegCurveCont( xDataSeries, UNO_QUERY );
    if( xRegCurveCont.is() )
    {
        for (auto const& trendLine : maTrendLines)
        {
            try
            {
                Reference< XRegressionCurve > xRegCurve = trendLine->CreateRegressionCurve();
                if( xRegCurve.is() )
                {
                    xRegCurveCont->addRegressionCurve( xRegCurve );
                }
            }
            catch( Exception& )
            {
                OSL_FAIL( "XclImpChSeries::ConvertTrendLines - cannot add regression curve" );
            }
        }
    }
}

Reference< XPropertySet > XclImpChSeries::CreateErrorBar( sal_uInt8 nPosBarId, sal_uInt8 nNegBarId ) const
{
    XclImpChSerErrorBarMap::const_iterator itrPosBar = m_ErrorBars.find(nPosBarId);
    XclImpChSerErrorBarMap::const_iterator itrNegBar = m_ErrorBars.find(nNegBarId);
    XclImpChSerErrorBarMap::const_iterator itrEnd = m_ErrorBars.end();
    if (itrPosBar == itrEnd || itrNegBar == itrEnd)
        return Reference<XPropertySet>();

    return XclImpChSerErrorBar::CreateErrorBar(itrPosBar->second.get(), itrNegBar->second.get());
}

void XclImpChSeries::ReadChLegendException(XclImpStream& rStrm)
{
    rStrm.Ignore(2);
    sal_uInt16 nFlags = rStrm.ReaduInt16();
    mbLabelDeleted = (nFlags & EXC_CHLEGENDEXCEPTION_DELETED);
}

// Chart type groups ==========================================================

XclImpChType::XclImpChType( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    mnRecId( EXC_ID_CHUNKNOWN ),
    maTypeInfo( rRoot.GetChartTypeInfo( EXC_CHTYPEID_UNKNOWN ) )
{
}

void XclImpChType::ReadChType( XclImpStream& rStrm )
{
    sal_uInt16 nRecId = rStrm.GetRecId();
    bool bKnownType = true;

    switch( nRecId )
    {
        case EXC_ID_CHBAR:
            maData.mnOverlap = rStrm.ReadInt16();
            maData.mnGap = rStrm.ReadInt16();
            maData.mnFlags = rStrm.ReaduInt16();
        break;

        case EXC_ID_CHLINE:
        case EXC_ID_CHAREA:
        case EXC_ID_CHRADARLINE:
        case EXC_ID_CHRADARAREA:
            maData.mnFlags = rStrm.ReaduInt16();
        break;

        case EXC_ID_CHPIE:
            maData.mnRotation = rStrm.ReaduInt16();
            maData.mnPieHole = rStrm.ReaduInt16();
            if( GetBiff() == EXC_BIFF8 )
                maData.mnFlags = rStrm.ReaduInt16();
            else
                maData.mnFlags = 0;
        break;

        case EXC_ID_CHPIEEXT:
            maData.mnRotation = 0;
            maData.mnPieHole = 0;
            maData.mnFlags = 0;
        break;

        case EXC_ID_CHSCATTER:
            if( GetBiff() == EXC_BIFF8 )
            {
                maData.mnBubbleSize = rStrm.ReaduInt16();
                maData.mnBubbleType = rStrm.ReaduInt16();
                maData.mnFlags = rStrm.ReaduInt16();
            }
            else
                maData.mnFlags = 0;
        break;

        case EXC_ID_CHSURFACE:
            maData.mnFlags = rStrm.ReaduInt16();
        break;

        default:
            bKnownType = false;
    }

    if( bKnownType )
        mnRecId = nRecId;
}

void XclImpChType::Finalize( bool bStockChart )
{
    switch( mnRecId )
    {
        case EXC_ID_CHLINE:
            maTypeInfo = GetChartTypeInfo( bStockChart ?
                EXC_CHTYPEID_STOCK : EXC_CHTYPEID_LINE );
        break;
        case EXC_ID_CHBAR:
            maTypeInfo = GetChartTypeInfo( ::get_flagvalue(
                maData.mnFlags, EXC_CHBAR_HORIZONTAL,
                EXC_CHTYPEID_HORBAR, EXC_CHTYPEID_BAR ) );
        break;
        case EXC_ID_CHPIE:
            maTypeInfo = GetChartTypeInfo( (maData.mnPieHole > 0) ?
                EXC_CHTYPEID_DONUT : EXC_CHTYPEID_PIE );
        break;
        case EXC_ID_CHSCATTER:
            maTypeInfo = GetChartTypeInfo( ::get_flagvalue(
                maData.mnFlags, EXC_CHSCATTER_BUBBLES,
                EXC_CHTYPEID_BUBBLES, EXC_CHTYPEID_SCATTER ) );
        break;
        default:
            maTypeInfo = GetChartTypeInfo( mnRecId );
    }

    switch( maTypeInfo.meTypeId )
    {
        case EXC_CHTYPEID_PIEEXT:
        case EXC_CHTYPEID_BUBBLES:
        case EXC_CHTYPEID_SURFACE:
        case EXC_CHTYPEID_UNKNOWN:
            GetTracer().TraceChartUnKnownType();
        break;
        default:;
    }
}

bool XclImpChType::IsStacked() const
{
    bool bStacked = false;
    if( maTypeInfo.mbSupportsStacking ) switch( maTypeInfo.meTypeCateg )
    {
        case EXC_CHTYPECATEG_LINE:
            bStacked =
                ::get_flag( maData.mnFlags, EXC_CHLINE_STACKED ) &&
                !::get_flag( maData.mnFlags, EXC_CHLINE_PERCENT );
        break;
        case EXC_CHTYPECATEG_BAR:
            bStacked =
                ::get_flag( maData.mnFlags, EXC_CHBAR_STACKED ) &&
                !::get_flag( maData.mnFlags, EXC_CHBAR_PERCENT );
        break;
        default:;
    }
    return bStacked;
}

bool XclImpChType::IsPercent() const
{
    bool bPercent = false;
    if( maTypeInfo.mbSupportsStacking ) switch( maTypeInfo.meTypeCateg )
    {
        case EXC_CHTYPECATEG_LINE:
            bPercent =
                ::get_flag( maData.mnFlags, EXC_CHLINE_STACKED ) &&
                ::get_flag( maData.mnFlags, EXC_CHLINE_PERCENT );
        break;
        case EXC_CHTYPECATEG_BAR:
            bPercent =
                ::get_flag( maData.mnFlags, EXC_CHBAR_STACKED ) &&
                ::get_flag( maData.mnFlags, EXC_CHBAR_PERCENT );
        break;
        default:;
    }
    return bPercent;
}

bool XclImpChType::HasCategoryLabels() const
{
    // radar charts disable category labels in chart type, not in CHTICK of X axis
    return (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_RADAR) || ::get_flag( maData.mnFlags, EXC_CHRADAR_AXISLABELS );
}

Reference< XCoordinateSystem > XclImpChType::CreateCoordSystem( bool b3dChart ) const
{
    // create the coordinate system object
    Reference< css::uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< XCoordinateSystem > xCoordSystem;
    if( maTypeInfo.mbPolarCoordSystem )
    {
        if( b3dChart )
            xCoordSystem = css::chart2::PolarCoordinateSystem3d::create(xContext);
        else
            xCoordSystem = css::chart2::PolarCoordinateSystem2d::create(xContext);
    }
    else
    {
        if( b3dChart )
            xCoordSystem = css::chart2::CartesianCoordinateSystem3d::create(xContext);
        else
            xCoordSystem = css::chart2::CartesianCoordinateSystem2d::create(xContext);
    }

    // swap X and Y axis
    if( maTypeInfo.mbSwappedAxesSet )
    {
        ScfPropertySet aCoordSysProp( xCoordSystem );
        aCoordSysProp.SetBoolProperty( EXC_CHPROP_SWAPXANDYAXIS, true );
    }

    return xCoordSystem;
}

Reference< XChartType > XclImpChType::CreateChartType( Reference< XDiagram > const & xDiagram, bool b3dChart ) const
{
    OUString aService = OUString::createFromAscii( maTypeInfo.mpcServiceName );
    Reference< XChartType > xChartType( ScfApiHelper::CreateInstance( aService ), UNO_QUERY );

    // additional properties
    switch( maTypeInfo.meTypeCateg )
    {
        case EXC_CHTYPECATEG_BAR:
        {
            ScfPropertySet aTypeProp( xChartType );
            Sequence< sal_Int32 > aInt32Seq( 2 );
            aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = -maData.mnOverlap;
            aTypeProp.SetProperty( EXC_CHPROP_OVERLAPSEQ, aInt32Seq );
            aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = maData.mnGap;
            aTypeProp.SetProperty( EXC_CHPROP_GAPWIDTHSEQ, aInt32Seq );
        }
        break;
        case EXC_CHTYPECATEG_PIE:
        {
            ScfPropertySet aTypeProp( xChartType );
            aTypeProp.SetBoolProperty( EXC_CHPROP_USERINGS, maTypeInfo.meTypeId == EXC_CHTYPEID_DONUT );
            /*  #i85166# starting angle of first pie slice. 3D pie charts use Y
                rotation setting in view3D element. Of-pie charts do not
                support pie rotation. */
            if( !b3dChart && (maTypeInfo.meTypeId != EXC_CHTYPEID_PIEEXT) )
            {
                ScfPropertySet aDiaProp( xDiagram );
                XclImpChRoot::ConvertPieRotation( aDiaProp, maData.mnRotation );
            }
        }
        break;
        default:;
    }

    return xChartType;
}

void XclImpChChart3d::ReadChChart3d( XclImpStream& rStrm )
{
    maData.mnRotation = rStrm.ReaduInt16();
    maData.mnElevation = rStrm.ReadInt16();
    maData.mnEyeDist = rStrm.ReaduInt16();
    maData.mnRelHeight = rStrm.ReaduInt16();
    maData.mnRelDepth = rStrm.ReaduInt16();
    maData.mnDepthGap = rStrm.ReaduInt16();
    maData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChChart3d::Convert( ScfPropertySet& rPropSet, bool b3dWallChart ) const
{
    namespace cssd = ::com::sun::star::drawing;

//    #i104057# do not assert this, written by broken external generators
//    OSL_ENSURE( ::get_flag( maData.mnFlags, EXC_CHCHART3D_HASWALLS ) == b3dWallChart, "XclImpChChart3d::Convert - wrong wall flag" );

    sal_Int32 nRotationY = 0;
    sal_Int32 nRotationX = 0;
    sal_Int32 nPerspective = 15;
    bool bRightAngled = false;
    cssd::ProjectionMode eProjMode = cssd::ProjectionMode_PERSPECTIVE;
    Color aAmbientColor, aLightColor;

    if( b3dWallChart )
    {
        // Y rotation (Excel [0..359], Chart2 [-179,180])
        nRotationY = NormAngle180<sal_Int32>(maData.mnRotation);
        // X rotation a.k.a. elevation (Excel [-90..90], Chart2 [-179,180])
        nRotationX = limit_cast< sal_Int32, sal_Int32 >( maData.mnElevation, -90, 90 );
        // perspective (Excel and Chart2 [0,100])
        nPerspective = limit_cast< sal_Int32, sal_Int32 >( maData.mnEyeDist, 0, 100 );
        // right-angled axes
        bRightAngled = !::get_flag( maData.mnFlags, EXC_CHCHART3D_REAL3D );
        // projection mode (parallel axes, if right-angled, #i90360# or if perspective is at 0%)
        bool bParallel = bRightAngled || (nPerspective == 0);
        eProjMode = bParallel ? cssd::ProjectionMode_PARALLEL : cssd::ProjectionMode_PERSPECTIVE;
        // ambient color (Gray 20%)
        aAmbientColor = Color( 204, 204, 204 );
        // light color (Gray 60%)
        aLightColor = Color( 102, 102, 102 );
    }
    else
    {
        // Y rotation not used in pie charts, but 'first pie slice angle'
        nRotationY = 0;
        XclImpChRoot::ConvertPieRotation( rPropSet, maData.mnRotation );
        // X rotation a.k.a. elevation (map Excel [10..80] to Chart2 [-80,-10])
        nRotationX = limit_cast< sal_Int32, sal_Int32 >( maData.mnElevation, 10, 80 ) - 90;
        // perspective (Excel and Chart2 [0,100])
        nPerspective = limit_cast< sal_Int32, sal_Int32 >( maData.mnEyeDist, 0, 100 );
        // no right-angled axes in pie charts, but parallel projection
        bRightAngled = false;
        eProjMode = cssd::ProjectionMode_PARALLEL;
        // ambient color (Gray 30%)
        aAmbientColor = Color( 179, 179, 179 );
        // light color (Gray 70%)
        aLightColor = Color( 76, 76, 76 );
    }

    // properties
    rPropSet.SetProperty( EXC_CHPROP_3DRELATIVEHEIGHT, static_cast<sal_Int32>(maData.mnRelHeight / 2)); // seems to be 200%, change to 100%
    rPropSet.SetProperty( EXC_CHPROP_ROTATIONVERTICAL, nRotationY );
    rPropSet.SetProperty( EXC_CHPROP_ROTATIONHORIZONTAL, nRotationX );
    rPropSet.SetProperty( EXC_CHPROP_PERSPECTIVE, nPerspective );
    rPropSet.SetBoolProperty( EXC_CHPROP_RIGHTANGLEDAXES, bRightAngled );
    rPropSet.SetProperty( EXC_CHPROP_D3DSCENEPERSPECTIVE, eProjMode );

    // light settings
    rPropSet.SetProperty( EXC_CHPROP_D3DSCENESHADEMODE, cssd::ShadeMode_FLAT );
    rPropSet.SetColorProperty( EXC_CHPROP_D3DSCENEAMBIENTCOLOR, aAmbientColor );
    rPropSet.SetBoolProperty( EXC_CHPROP_D3DSCENELIGHTON1, false );
    rPropSet.SetBoolProperty( EXC_CHPROP_D3DSCENELIGHTON2, true );
    rPropSet.SetColorProperty( EXC_CHPROP_D3DSCENELIGHTCOLOR2, aLightColor );
    rPropSet.SetProperty( EXC_CHPROP_D3DSCENELIGHTDIR2, cssd::Direction3D( 0.2, 0.4, 1.0 ) );
}

XclImpChLegend::XclImpChLegend( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChLegend::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.maRect;
    maData.mnDockMode = rStrm.ReaduInt8();
    maData.mnSpacing = rStrm.ReaduInt8();
    maData.mnFlags = rStrm.ReaduInt16();

    // trace unsupported features
    if( GetTracer().IsEnabled() )
    {
        if( maData.mnDockMode == EXC_CHLEGEND_NOTDOCKED )
            GetTracer().TraceChartLegendPosition();
        if( ::get_flag( maData.mnFlags, EXC_CHLEGEND_DATATABLE ) )
            GetTracer().TraceChartDataTable();
    }
}

void XclImpChLegend::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAMEPOS:
            mxFramePos.reset( new XclImpChFramePos );
            mxFramePos->ReadChFramePos( rStrm );
        break;
        case EXC_ID_CHTEXT:
            mxText.reset( new XclImpChText( GetChRoot() ) );
            mxText->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_LEGEND ) );
            mxFrame->ReadRecordGroup( rStrm );
        break;
    }
}

void XclImpChLegend::Finalize()
{
    // legend default formatting differs in OOChart and Excel, missing frame means automatic
    if( !mxFrame )
        mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_LEGEND ) );
    // Update text formatting. If mxText is empty, the passed default text is used.
    lclUpdateText( mxText, GetChartData().GetDefaultText( EXC_CHTEXTTYPE_LEGEND ) );
}

Reference< XLegend > XclImpChLegend::CreateLegend() const
{
    Reference< XLegend > xLegend( ScfApiHelper::CreateInstance( SERVICE_CHART2_LEGEND ), UNO_QUERY );
    if( xLegend.is() )
    {
        ScfPropertySet aLegendProp( xLegend );
        aLegendProp.SetBoolProperty( EXC_CHPROP_SHOW, true );

        // frame properties
        if( mxFrame )
            mxFrame->Convert( aLegendProp );
        // text properties
        if( mxText )
            mxText->ConvertFont( aLegendProp );

        /*  Legend position and size. Default positions are used only if the
            plot area is positioned automatically (Excel sets the plot area to
            manual mode, if the legend is moved or resized). With manual plot
            areas, Excel ignores the value in maData.mnDockMode completely. */
        cssc2::LegendPosition eApiPos = cssc2::LegendPosition_CUSTOM;
        cssc::ChartLegendExpansion eApiExpand = cssc::ChartLegendExpansion_CUSTOM;
        if( !GetChartData().IsManualPlotArea() ) switch( maData.mnDockMode )
        {
            case EXC_CHLEGEND_LEFT:
                eApiPos = cssc2::LegendPosition_LINE_START;
                eApiExpand = cssc::ChartLegendExpansion_HIGH;
            break;
            case EXC_CHLEGEND_RIGHT:
            // top-right not supported
            case EXC_CHLEGEND_CORNER:
                eApiPos = cssc2::LegendPosition_LINE_END;
                eApiExpand = cssc::ChartLegendExpansion_HIGH;
            break;
            case EXC_CHLEGEND_TOP:
                eApiPos = cssc2::LegendPosition_PAGE_START;
                eApiExpand = cssc::ChartLegendExpansion_WIDE;
            break;
            case EXC_CHLEGEND_BOTTOM:
                eApiPos = cssc2::LegendPosition_PAGE_END;
                eApiExpand = cssc::ChartLegendExpansion_WIDE;
            break;
        }

        // no automatic position/size: try to find the correct position and size
        if( eApiPos == cssc2::LegendPosition_CUSTOM )
        {
            const XclChFramePos* pFramePos = mxFramePos ? &mxFramePos->GetFramePosData() : nullptr;

            /*  Legend position. Only the settings from the CHFRAMEPOS record
                are used by Excel, the position in the CHLEGEND record will be
                ignored. */
            if( pFramePos )
            {
                RelativePosition aRelPos(
                    CalcRelativeFromChartX( pFramePos->maRect.mnX ),
                    CalcRelativeFromChartY( pFramePos->maRect.mnY ),
                    css::drawing::Alignment_TOP_LEFT );
                aLegendProp.SetProperty( EXC_CHPROP_RELATIVEPOSITION, aRelPos );
            }
            else
            {
                // no manual position/size found, just go for the default
                eApiPos = cssc2::LegendPosition_LINE_END;
            }

            /*  Legend size. The member mnBRMode specifies whether size is
                automatic or changes manually. Manual size is given in points,
                not in chart units. */
            if( pFramePos && (pFramePos->mnBRMode == EXC_CHFRAMEPOS_ABSSIZE_POINTS) &&
                (pFramePos->maRect.mnWidth > 0) && (pFramePos->maRect.mnHeight > 0) )
            {
                eApiExpand = cssc::ChartLegendExpansion_CUSTOM;
                sal_Int32 nWidthHmm = static_cast< sal_Int32 >( pFramePos->maRect.mnWidth / EXC_POINTS_PER_HMM );
                sal_Int32 nHeightHmm = static_cast< sal_Int32 >( pFramePos->maRect.mnHeight / EXC_POINTS_PER_HMM );
                RelativeSize aRelSize( CalcRelativeFromHmmX( nWidthHmm ), CalcRelativeFromHmmY( nHeightHmm ) );
                aLegendProp.SetProperty( EXC_CHPROP_RELATIVESIZE, aRelSize );
            }
            else
            {
                // automatic size: determine entry direction from flags
                eApiExpand = ::get_flagvalue( maData.mnFlags, EXC_CHLEGEND_STACKED,
                    cssc::ChartLegendExpansion_HIGH, cssc::ChartLegendExpansion_WIDE );
            }
        }
        aLegendProp.SetProperty( EXC_CHPROP_ANCHORPOSITION, eApiPos );
        aLegendProp.SetProperty( EXC_CHPROP_EXPANSION, eApiExpand );
    }
    return xLegend;
}

XclImpChDropBar::XclImpChDropBar( sal_uInt16 nDropBar ) :
    mnDropBar( nDropBar ),
    mnBarDist( 0 )
{
}

void XclImpChDropBar::ReadHeaderRecord( XclImpStream& rStrm )
{
    mnBarDist = rStrm.ReaduInt16();
}

void XclImpChDropBar::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    XclChObjectType eObjType = EXC_CHOBJTYPE_BACKGROUND;
    switch( mnDropBar )
    {
        case EXC_CHDROPBAR_UP:      eObjType = EXC_CHOBJTYPE_WHITEDROPBAR;  break;
        case EXC_CHDROPBAR_DOWN:    eObjType = EXC_CHOBJTYPE_BLACKDROPBAR;  break;
    }
    ConvertFrameBase( rRoot, rPropSet, eObjType );
}

XclImpChTypeGroup::XclImpChTypeGroup( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    maType( rRoot ),
    maTypeInfo( maType.GetTypeInfo() )
{
    // Initialize unused format indexes set. At this time, all formats are unused.
    for( sal_uInt16 nFormatIdx = 0; nFormatIdx <= EXC_CHSERIES_MAXSERIES; ++nFormatIdx )
        maUnusedFormats.insert( maUnusedFormats.end(), nFormatIdx );
}

void XclImpChTypeGroup::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm.Ignore( 16 );
    maData.mnFlags = rStrm.ReaduInt16();
    maData.mnGroupIdx = rStrm.ReaduInt16();
}

void XclImpChTypeGroup::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHCHART3D:
            mxChart3d.reset( new XclImpChChart3d );
            mxChart3d->ReadChChart3d( rStrm );
        break;
        case EXC_ID_CHLEGEND:
            mxLegend.reset( new XclImpChLegend( GetChRoot() ) );
            mxLegend->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHDEFAULTTEXT:
            GetChartData().ReadChDefaultText( rStrm );
        break;
        case EXC_ID_CHDROPBAR:
            ReadChDropBar( rStrm );
        break;
        case EXC_ID_CHCHARTLINE:
            ReadChChartLine( rStrm );
        break;
        case EXC_ID_CHDATAFORMAT:
            ReadChDataFormat( rStrm );
        break;
        default:
            maType.ReadChType( rStrm );
    }
}

void XclImpChTypeGroup::Finalize()
{
    // check and set valid chart type
    bool bStockChart =
        (maType.GetRecId() == EXC_ID_CHLINE) &&         // must be a line chart
        !mxChart3d &&                                   // must be a 2d chart
        m_ChartLines.find(EXC_CHCHARTLINE_HILO) != m_ChartLines.end() && // must contain hi-lo lines
        (maSeries.size() == static_cast<XclImpChSeriesVec::size_type>(HasDropBars() ? 4 : 3));   // correct series count
    maType.Finalize( bStockChart );

    // extended type info
    maTypeInfo.Set( maType.GetTypeInfo(), static_cast< bool >(mxChart3d), false );

    // reverse series order for some unstacked 2D chart types
    if( maTypeInfo.mbReverseSeries && !Is3dChart() && !maType.IsStacked() && !maType.IsPercent() )
        ::std::reverse( maSeries.begin(), maSeries.end() );

    // update chart type group format, may depend on chart type finalized above
    if( mxGroupFmt )
        mxGroupFmt->UpdateGroupFormat( maTypeInfo );
}

void XclImpChTypeGroup::AddSeries( XclImpChSeriesRef const & xSeries )
{
    if( xSeries )
        maSeries.push_back( xSeries );
    // store first inserted series separately, series order may be reversed later
    if( !mxFirstSeries )
        mxFirstSeries = xSeries;
}

void XclImpChTypeGroup::SetUsedFormatIndex( sal_uInt16 nFormatIdx )
{
    maUnusedFormats.erase( nFormatIdx );
}

sal_uInt16 XclImpChTypeGroup::PopUnusedFormatIndex()
{
    OSL_ENSURE( !maUnusedFormats.empty(), "XclImpChTypeGroup::PopUnusedFormatIndex - no more format indexes available" );
    sal_uInt16 nFormatIdx = maUnusedFormats.empty() ? 0 : *maUnusedFormats.begin();
    SetUsedFormatIndex( nFormatIdx );
    return nFormatIdx;
}

bool XclImpChTypeGroup::HasVarPointFormat() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTYPEGROUP_VARIEDCOLORS ) &&
        ((maTypeInfo.meVarPointMode == EXC_CHVARPOINT_MULTI) ||         // multiple series allowed
            ((maTypeInfo.meVarPointMode == EXC_CHVARPOINT_SINGLE) &&    // or exactly 1 series?
                (maSeries.size() == 1)));
}

bool XclImpChTypeGroup::HasConnectorLines() const
{
    // existence of connector lines (only in stacked bar charts)
    if ( !(maType.IsStacked() || maType.IsPercent()) || (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_BAR) )
        return false;
    XclImpChLineFormatMap::const_iterator aConLine = m_ChartLines.find(EXC_CHCHARTLINE_CONNECT);
    return (aConLine != m_ChartLines.end() && aConLine->second.HasLine());
}

OUString XclImpChTypeGroup::GetSingleSeriesTitle() const
{
    // no automatic title for series with trendlines or error bars
    // pie charts always show an automatic title, even if more series exist
    return (mxFirstSeries && !mxFirstSeries->HasChildSeries() && (maTypeInfo.mbSingleSeriesVis || (maSeries.size() == 1))) ?
        mxFirstSeries->GetTitle() : OUString();
}

void XclImpChTypeGroup::ConvertChart3d( ScfPropertySet& rPropSet ) const
{
    if( mxChart3d )
        mxChart3d->Convert( rPropSet, Is3dWallChart() );
}

Reference< XCoordinateSystem > XclImpChTypeGroup::CreateCoordSystem() const
{
    return maType.CreateCoordSystem( Is3dChart() );
}

Reference< XChartType > XclImpChTypeGroup::CreateChartType( Reference< XDiagram > const & xDiagram, sal_Int32 nApiAxesSetIdx ) const
{
    OSL_ENSURE( IsValidGroup(), "XclImpChTypeGroup::CreateChartType - type group without series" );

    // create the chart type object
    Reference< XChartType > xChartType = maType.CreateChartType( xDiagram, Is3dChart() );

    // bar chart connector lines
    if( HasConnectorLines() )
    {
        ScfPropertySet aDiaProp( xDiagram );
        aDiaProp.SetBoolProperty( EXC_CHPROP_CONNECTBARS, true );
    }

    /*  Stock chart needs special processing. Create one 'big' series with
        data sequences of different roles. */
    if( maTypeInfo.meTypeId == EXC_CHTYPEID_STOCK )
        CreateStockSeries( xChartType, nApiAxesSetIdx );
    else
        CreateDataSeries( xChartType, nApiAxesSetIdx );

    return xChartType;
}

Reference< XLabeledDataSequence > XclImpChTypeGroup::CreateCategSequence() const
{
    Reference< XLabeledDataSequence > xLabeledSeq;
    // create category sequence from first visible series
    if( mxFirstSeries )
        xLabeledSeq = mxFirstSeries->CreateCategSequence( EXC_CHPROP_ROLE_CATEG );
    return xLabeledSeq;
}

void XclImpChTypeGroup::ReadChDropBar( XclImpStream& rStrm )
{
    if (m_DropBars.find(EXC_CHDROPBAR_UP) == m_DropBars.end())
    {
        unique_ptr<XclImpChDropBar> p(new XclImpChDropBar(EXC_CHDROPBAR_UP));
        p->ReadRecordGroup(rStrm);
        m_DropBars.insert(std::make_pair(EXC_CHDROPBAR_UP, std::move(p)));
    }
    else if (m_DropBars.find(EXC_CHDROPBAR_DOWN) == m_DropBars.end())
    {
        unique_ptr<XclImpChDropBar> p(new XclImpChDropBar(EXC_CHDROPBAR_DOWN));
        p->ReadRecordGroup(rStrm);
        m_DropBars.insert(std::make_pair(EXC_CHDROPBAR_DOWN, std::move(p)));
    }
}

void XclImpChTypeGroup::ReadChChartLine( XclImpStream& rStrm )
{
    sal_uInt16 nLineId = rStrm.ReaduInt16();
    if( (rStrm.GetNextRecId() == EXC_ID_CHLINEFORMAT) && rStrm.StartNextRecord() )
    {
        XclImpChLineFormat aLineFmt;
        aLineFmt.ReadChLineFormat( rStrm );
        m_ChartLines[ nLineId ] = aLineFmt;
    }
}

void XclImpChTypeGroup::ReadChDataFormat( XclImpStream& rStrm )
{
    // global series and data point format
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->ReadRecordGroup( rStrm );
    const XclChDataPointPos& rPos = xDataFmt->GetPointPos();
    if( (rPos.mnSeriesIdx == 0) && (rPos.mnPointIdx == 0) &&
            (xDataFmt->GetFormatIdx() == EXC_CHDATAFORMAT_DEFAULT) )
        mxGroupFmt = xDataFmt;
}

void XclImpChTypeGroup::InsertDataSeries( Reference< XChartType > const & xChartType,
        Reference< XDataSeries > const & xSeries, sal_Int32 nApiAxesSetIdx ) const
{
    Reference< XDataSeriesContainer > xSeriesCont( xChartType, UNO_QUERY );
    if( xSeriesCont.is() && xSeries.is() )
    {
        // series stacking mode
        cssc2::StackingDirection eStacking = cssc2::StackingDirection_NO_STACKING;
        // stacked overrides deep-3d
        if( maType.IsStacked() || maType.IsPercent() )
            eStacking = cssc2::StackingDirection_Y_STACKING;
        else if( Is3dDeepChart() )
            eStacking = cssc2::StackingDirection_Z_STACKING;

        // additional series properties
        ScfPropertySet aSeriesProp( xSeries );
        aSeriesProp.SetProperty( EXC_CHPROP_STACKINGDIR, eStacking );
        aSeriesProp.SetProperty( EXC_CHPROP_ATTAXISINDEX, nApiAxesSetIdx );

        // insert series into container
        try
        {
            xSeriesCont->addDataSeries( xSeries );
        }
        catch( Exception& )
        {
            OSL_FAIL( "XclImpChTypeGroup::InsertDataSeries - cannot add data series" );
        }
    }
}

void XclImpChTypeGroup::CreateDataSeries( Reference< XChartType > const & xChartType, sal_Int32 nApiAxesSetIdx ) const
{
    bool bSpline = false;
    for (auto const& elem : maSeries)
    {
        Reference< XDataSeries > xDataSeries = elem->CreateDataSeries();
        InsertDataSeries( xChartType, xDataSeries, nApiAxesSetIdx );
        bSpline |= elem->HasSpline();
    }
    // spline - TODO: set at single series (#i66858#)
    if( bSpline && !maTypeInfo.IsSeriesFrameFormat() && (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_RADAR) )
    {
        ScfPropertySet aTypeProp( xChartType );
        aTypeProp.SetProperty( EXC_CHPROP_CURVESTYLE, css::chart2::CurveStyle_CUBIC_SPLINES );
    }
}

void XclImpChTypeGroup::CreateStockSeries( Reference< XChartType > const & xChartType, sal_Int32 nApiAxesSetIdx ) const
{
    // create the data series object
    Reference< XDataSeries > xDataSeries( ScfApiHelper::CreateInstance( SERVICE_CHART2_DATASERIES ), UNO_QUERY );
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        // create a list of data sequences from all series
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        OSL_ENSURE( maSeries.size() >= 3, "XclImpChTypeGroup::CreateChartType - missing stock series" );
        int nRoleIdx = (maSeries.size() == 3) ? 1 : 0;
        for( const auto& rxSeries : maSeries )
        {
            // create a data sequence with a specific role
            OUString aRole;
            switch( nRoleIdx )
            {
                case 0: aRole = EXC_CHPROP_ROLE_OPENVALUES;     break;
                case 1: aRole = EXC_CHPROP_ROLE_HIGHVALUES;     break;
                case 2: aRole = EXC_CHPROP_ROLE_LOWVALUES;      break;
                case 3: aRole = EXC_CHPROP_ROLE_CLOSEVALUES;    break;
            }
            Reference< XLabeledDataSequence > xDataSeq = rxSeries->CreateValueSequence( aRole );
            if( xDataSeq.is() )
                aLabeledSeqVec.push_back( xDataSeq );
            ++nRoleIdx;
            if (nRoleIdx >= 4)
                break;
        }

        // attach labeled data sequences to series and insert series into chart type
        xDataSink->setData( ScfApiHelper::VectorToSequence( aLabeledSeqVec ) );

        // formatting of special stock chart elements
        ScfPropertySet aTypeProp( xChartType );
        aTypeProp.SetBoolProperty( EXC_CHPROP_JAPANESE, HasDropBars() );
        aTypeProp.SetBoolProperty( EXC_CHPROP_SHOWFIRST, HasDropBars() );
        aTypeProp.SetBoolProperty( EXC_CHPROP_SHOWHIGHLOW, true );
        // hi-lo line format
        XclImpChLineFormatMap::const_iterator aHiLoLine = m_ChartLines.find( EXC_CHCHARTLINE_HILO );
        if (aHiLoLine != m_ChartLines.end())
        {
            ScfPropertySet aSeriesProp( xDataSeries );
            aHiLoLine->second.Convert( GetChRoot(), aSeriesProp, EXC_CHOBJTYPE_HILOLINE );
        }
        // white dropbar format
        XclImpChDropBarMap::const_iterator itr = m_DropBars.find(EXC_CHDROPBAR_UP);
        Reference<XPropertySet> xWhitePropSet;
        if (itr != m_DropBars.end() && aTypeProp.GetProperty(xWhitePropSet, EXC_CHPROP_WHITEDAY))
        {
            ScfPropertySet aBarProp( xWhitePropSet );
            itr->second->Convert(GetChRoot(), aBarProp);
        }
        // black dropbar format
        itr = m_DropBars.find(EXC_CHDROPBAR_DOWN);
        Reference<XPropertySet> xBlackPropSet;
        if (itr != m_DropBars.end() && aTypeProp.GetProperty(xBlackPropSet, EXC_CHPROP_BLACKDAY))
        {
            ScfPropertySet aBarProp( xBlackPropSet );
            itr->second->Convert(GetChRoot(), aBarProp);
        }

        // insert the series into the chart type object
        InsertDataSeries( xChartType, xDataSeries, nApiAxesSetIdx );
    }
}

// Axes =======================================================================

XclImpChLabelRange::XclImpChLabelRange( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChLabelRange::ReadChLabelRange( XclImpStream& rStrm )
{
    maLabelData.mnCross = rStrm.ReaduInt16();
    maLabelData.mnLabelFreq = rStrm.ReaduInt16();
    maLabelData.mnTickFreq = rStrm.ReaduInt16();
    maLabelData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChLabelRange::ReadChDateRange( XclImpStream& rStrm )
{
    maDateData.mnMinDate = rStrm.ReaduInt16();
    maDateData.mnMaxDate = rStrm.ReaduInt16();
    maDateData.mnMajorStep = rStrm.ReaduInt16();
    maDateData.mnMajorUnit = rStrm.ReaduInt16();
    maDateData.mnMinorStep = rStrm.ReaduInt16();
    maDateData.mnMinorUnit = rStrm.ReaduInt16();
    maDateData.mnBaseUnit = rStrm.ReaduInt16();
    maDateData.mnCross = rStrm.ReaduInt16();
    maDateData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChLabelRange::Convert( ScfPropertySet& rPropSet, ScaleData& rScaleData, bool bMirrorOrient ) const
{
    // automatic axis type detection
    rScaleData.AutoDateAxis = ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTODATE );

    // the flag EXC_CHDATERANGE_DATEAXIS specifies whether this is a date axis
    if( ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_DATEAXIS ) )
    {
        /*  Chart2 requires axis type CATEGORY for automatic category/date axis
            (even if it is a date axis currently). */
        rScaleData.AxisType = rScaleData.AutoDateAxis ? cssc2::AxisType::CATEGORY : cssc2::AxisType::DATE;
        rScaleData.Scaling = css::chart2::LinearScaling::create( comphelper::getProcessComponentContext() );
        /*  Min/max values depend on base time unit, they specify the number of
            days, months, or years starting from null date. */
        lclConvertTimeValue( GetRoot(), rScaleData.Minimum, maDateData.mnMinDate, ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMIN ), maDateData.mnBaseUnit );
        lclConvertTimeValue( GetRoot(), rScaleData.Maximum, maDateData.mnMaxDate, ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMAX ), maDateData.mnBaseUnit );
        // increment
        cssc::TimeIncrement& rTimeIncrement = rScaleData.TimeIncrement;
        lclConvertTimeInterval( rTimeIncrement.MajorTimeInterval, maDateData.mnMajorStep, ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMAJOR ), maDateData.mnMajorUnit );
        lclConvertTimeInterval( rTimeIncrement.MinorTimeInterval, maDateData.mnMinorStep, ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOMINOR ), maDateData.mnMinorUnit );
        // base unit
        if( ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOBASE ) )
            rTimeIncrement.TimeResolution.clear();
        else
            rTimeIncrement.TimeResolution <<= lclGetApiTimeUnit( maDateData.mnBaseUnit );
    }
    else
    {
        // do not overlap text unless all labels are visible
        rPropSet.SetBoolProperty( EXC_CHPROP_TEXTOVERLAP, maLabelData.mnLabelFreq == 1 );
        // do not break text into several lines unless all labels are visible
        rPropSet.SetBoolProperty( EXC_CHPROP_TEXTBREAK, maLabelData.mnLabelFreq == 1 );
        // do not stagger labels in two lines
        rPropSet.SetProperty( EXC_CHPROP_ARRANGEORDER, cssc::ChartAxisArrangeOrderType_SIDE_BY_SIDE );
    }

    // reverse order
    bool bReverse = ::get_flag( maLabelData.mnFlags, EXC_CHLABELRANGE_REVERSE ) != bMirrorOrient;
    rScaleData.Orientation = bReverse ? cssc2::AxisOrientation_REVERSE : cssc2::AxisOrientation_MATHEMATICAL;

    //TODO #i58731# show n-th category
}

void XclImpChLabelRange::ConvertAxisPosition( ScfPropertySet& rPropSet, bool b3dChart ) const
{
    /*  Crossing mode (max-cross flag overrides other crossing settings). Excel
        does not move the Y axis in 3D charts, regardless of actual settings.
        But: the Y axis has to be moved to "end", if the X axis is mirrored,
        to keep it at the left end of the chart. */
    bool bMaxCross = ::get_flag( maLabelData.mnFlags, b3dChart ? EXC_CHLABELRANGE_REVERSE : EXC_CHLABELRANGE_MAXCROSS );
    cssc::ChartAxisPosition eAxisPos = bMaxCross ? cssc::ChartAxisPosition_END : cssc::ChartAxisPosition_VALUE;
    rPropSet.SetProperty( EXC_CHPROP_CROSSOVERPOSITION, eAxisPos );

    // crossing position (depending on axis type text/date)
    if( ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_DATEAXIS ) )
    {
        bool bAutoCross = ::get_flag( maDateData.mnFlags, EXC_CHDATERANGE_AUTOCROSS );
        /*  Crossing position value depends on base time unit, it specifies the
            number of days, months, or years from null date. Note that Excel
            2007/2010 write broken BIFF8 files, they always stores the number
            of days regardless of the base time unit (and they are reading it
            the same way, thus wrongly displaying files written by Excel
            97-2003). This filter sticks to the correct behaviour of Excel
            97-2003. */
        double fCrossingPos = bAutoCross ? 1.0 : lclGetSerialDay( GetRoot(), maDateData.mnCross, maDateData.mnBaseUnit );
        rPropSet.SetProperty( EXC_CHPROP_CROSSOVERVALUE, fCrossingPos );
    }
    else
    {
        double fCrossingPos = b3dChart ? 1.0 : maLabelData.mnCross;
        rPropSet.SetProperty( EXC_CHPROP_CROSSOVERVALUE, fCrossingPos );
    }
}

XclImpChValueRange::XclImpChValueRange( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChValueRange::ReadChValueRange( XclImpStream& rStrm )
{
    maData.mfMin = rStrm.ReadDouble();
    maData.mfMax = rStrm.ReadDouble();
    maData.mfMajorStep = rStrm.ReadDouble();
    maData.mfMinorStep = rStrm.ReadDouble();
    maData.mfCross = rStrm.ReadDouble();
    maData.mnFlags = rStrm.ReaduInt16();
}

void XclImpChValueRange::Convert( ScaleData& rScaleData, bool bMirrorOrient ) const
{
    // scaling algorithm
    const bool bLogScale = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE );
    if( bLogScale )
        rScaleData.Scaling = css::chart2::LogarithmicScaling::create( comphelper::getProcessComponentContext() );
    else
        rScaleData.Scaling = css::chart2::LinearScaling::create( comphelper::getProcessComponentContext() );

    // min/max
    lclSetExpValueOrClearAny( rScaleData.Minimum, maData.mfMin, bLogScale, ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN ) );
    lclSetExpValueOrClearAny( rScaleData.Maximum, maData.mfMax, bLogScale, ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX ) );

    // increment
    bool bAutoMajor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR );
    bool bAutoMinor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR );
    // major increment
    IncrementData& rIncrementData = rScaleData.IncrementData;
    lclSetValueOrClearAny( rIncrementData.Distance, maData.mfMajorStep, bAutoMajor );
    // minor increment
    Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
    rSubIncrementSeq.realloc( 1 );
    Any& rIntervalCount = rSubIncrementSeq[ 0 ].IntervalCount;
    rIntervalCount.clear();
    if( bLogScale )
    {
        if( !bAutoMinor )
            rIntervalCount <<= sal_Int32( 9 );
    }
    else if( !bAutoMajor && !bAutoMinor && (0.0 < maData.mfMinorStep) && (maData.mfMinorStep <= maData.mfMajorStep) )
    {
        double fCount = maData.mfMajorStep / maData.mfMinorStep + 0.5;
        if( (1.0 <= fCount) && (fCount < 1001.0) )
            rIntervalCount <<= static_cast< sal_Int32 >( fCount );
    }
    else if( bAutoMinor )
    {
            // tdf#114168 If minor unit is not set then set interval to 5, as MS Excel do.
            rIntervalCount <<= static_cast< sal_Int32 >( 5 );
    }

    // reverse order
    bool bReverse = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_REVERSE ) != bMirrorOrient;
    rScaleData.Orientation = bReverse ? cssc2::AxisOrientation_REVERSE : cssc2::AxisOrientation_MATHEMATICAL;
}

void XclImpChValueRange::ConvertAxisPosition( ScfPropertySet& rPropSet ) const
{
    bool bMaxCross = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_MAXCROSS );
    bool bAutoCross = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS );
    bool bLogScale = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE );

    // crossing mode (max-cross flag overrides other crossing settings)
    cssc::ChartAxisPosition eAxisPos = bMaxCross ? cssc::ChartAxisPosition_END : cssc::ChartAxisPosition_VALUE;
    rPropSet.SetProperty( EXC_CHPROP_CROSSOVERPOSITION, eAxisPos );

    // crossing position
    double fCrossingPos = bAutoCross ? 0.0 : maData.mfCross;
    if( bLogScale ) fCrossingPos = pow( 10.0, fCrossingPos );
    rPropSet.SetProperty( EXC_CHPROP_CROSSOVERVALUE, fCrossingPos );
}

namespace {

sal_Int32 lclGetApiTickmarks( sal_uInt8 nXclTickPos )
{
    using namespace ::com::sun::star::chart2::TickmarkStyle;
    sal_Int32 nApiTickmarks = css::chart2::TickmarkStyle::NONE;
    ::set_flag( nApiTickmarks, INNER, ::get_flag( nXclTickPos, EXC_CHTICK_INSIDE ) );
    ::set_flag( nApiTickmarks, OUTER, ::get_flag( nXclTickPos, EXC_CHTICK_OUTSIDE ) );
    return nApiTickmarks;
}

cssc::ChartAxisLabelPosition lclGetApiLabelPosition( sal_Int8 nXclLabelPos )
{
    using namespace ::com::sun::star::chart;
    switch( nXclLabelPos )
    {
        case EXC_CHTICK_LOW:    return ChartAxisLabelPosition_OUTSIDE_START;
        case EXC_CHTICK_HIGH:   return ChartAxisLabelPosition_OUTSIDE_END;
        case EXC_CHTICK_NEXT:   return ChartAxisLabelPosition_NEAR_AXIS;
    }
    return ChartAxisLabelPosition_NEAR_AXIS;
}

} // namespace

XclImpChTick::XclImpChTick( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChTick::ReadChTick( XclImpStream& rStrm )
{
    maData.mnMajor = rStrm.ReaduInt8();
    maData.mnMinor = rStrm.ReaduInt8();
    maData.mnLabelPos = rStrm.ReaduInt8();
    maData.mnBackMode = rStrm.ReaduInt8();
    rStrm.Ignore( 16 );
    rStrm >> maData.maTextColor;
    maData.mnFlags = rStrm.ReaduInt16();

    if( GetBiff() == EXC_BIFF8 )
    {
        // BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( rStrm.ReaduInt16() );
        // rotation
        maData.mnRotation = rStrm.ReaduInt16();
    }
    else
    {
        // BIFF2-BIFF7: get rotation from text orientation
        sal_uInt8 nOrient = ::extract_value< sal_uInt8 >( maData.mnFlags, 2, 3 );
        maData.mnRotation = XclTools::GetXclRotFromOrient( nOrient );
    }
}

Color XclImpChTick::GetFontColor() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTICK_AUTOCOLOR ) ? GetFontAutoColor() : maData.maTextColor;
}

sal_uInt16 XclImpChTick::GetRotation() const
{
    /* n#720443: Ignore auto-rotation if there is a suggested rotation.
     * Better fix would be to improve our axis auto rotation algorithm.
     */
    if( maData.mnRotation != EXC_ROT_NONE )
        return maData.mnRotation;
    return ::get_flag( maData.mnFlags, EXC_CHTICK_AUTOROT ) ? EXC_CHART_AUTOROTATION : maData.mnRotation;
}

void XclImpChTick::Convert( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetProperty( EXC_CHPROP_MAJORTICKS, lclGetApiTickmarks( maData.mnMajor ) );
    rPropSet.SetProperty( EXC_CHPROP_MINORTICKS, lclGetApiTickmarks( maData.mnMinor ) );
    rPropSet.SetProperty( EXC_CHPROP_LABELPOSITION, lclGetApiLabelPosition( maData.mnLabelPos ) );
    rPropSet.SetProperty( EXC_CHPROP_MARKPOSITION, cssc::ChartAxisMarkPosition_AT_AXIS );
}

XclImpChAxis::XclImpChAxis( const XclImpChRoot& rRoot, sal_uInt16 nAxisType ) :
    XclImpChRoot( rRoot ),
    mnNumFmtIdx( EXC_FORMAT_NOTFOUND )
{
    maData.mnType = nAxisType;
}

void XclImpChAxis::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.mnType = rStrm.ReaduInt16();
}

void XclImpChAxis::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHLABELRANGE:
            mxLabelRange.reset( new XclImpChLabelRange( GetChRoot() ) );
            mxLabelRange->ReadChLabelRange( rStrm );
        break;
        case EXC_ID_CHDATERANGE:
            if( !mxLabelRange )
                mxLabelRange.reset( new XclImpChLabelRange( GetChRoot() ) );
            mxLabelRange->ReadChDateRange( rStrm );
        break;
        case EXC_ID_CHVALUERANGE:
            mxValueRange.reset( new XclImpChValueRange( GetChRoot() ) );
            mxValueRange->ReadChValueRange( rStrm );
        break;
        case EXC_ID_CHFORMAT:
            mnNumFmtIdx = rStrm.ReaduInt16();
        break;
        case EXC_ID_CHTICK:
            mxTick.reset( new XclImpChTick( GetChRoot() ) );
            mxTick->ReadChTick( rStrm );
        break;
        case EXC_ID_CHFONT:
            mxFont.reset( new XclImpChFont );
            mxFont->ReadChFont( rStrm );
        break;
        case EXC_ID_CHAXISLINE:
            ReadChAxisLine( rStrm );
        break;
    }
}

void XclImpChAxis::Finalize()
{
    // add default scaling, needed e.g. to adjust rotation direction of pie and radar charts
    if( !mxLabelRange )
        mxLabelRange.reset( new XclImpChLabelRange( GetChRoot() ) );
    if( !mxValueRange )
        mxValueRange.reset( new XclImpChValueRange( GetChRoot() ) );
    // remove invisible grid lines completely
    if( mxMajorGrid && !mxMajorGrid->HasLine() )
        mxMajorGrid.reset();
    if( mxMinorGrid && !mxMinorGrid->HasLine() )
        mxMinorGrid.reset();
    // default tick settings different in OOChart and Excel
    if( !mxTick )
        mxTick.reset( new XclImpChTick( GetChRoot() ) );
    // #i4140# different default axis line color
    if( !mxAxisLine )
    {
        XclChLineFormat aLineFmt;
        // set "show axis" flag, default if line format record is missing
        ::set_flag( aLineFmt.mnFlags, EXC_CHLINEFORMAT_SHOWAXIS );
        mxAxisLine.reset( new XclImpChLineFormat( aLineFmt ) );
    }
    // add wall/floor frame for 3d charts
    if( !mxWallFrame )
        CreateWallFrame();
}

sal_uInt16 XclImpChAxis::GetFontIndex() const
{
    return mxFont ? mxFont->GetFontIndex() : EXC_FONT_NOTFOUND;
}

Color XclImpChAxis::GetFontColor() const
{
    return mxTick ? mxTick->GetFontColor() : GetFontAutoColor();
}

sal_uInt16 XclImpChAxis::GetRotation() const
{
    return mxTick ? mxTick->GetRotation() : EXC_CHART_AUTOROTATION;
}

Reference< XAxis > XclImpChAxis::CreateAxis( const XclImpChTypeGroup& rTypeGroup, const XclImpChAxis* pCrossingAxis ) const
{
    // create the axis object (always)
    Reference< XAxis > xAxis( ScfApiHelper::CreateInstance( SERVICE_CHART2_AXIS ), UNO_QUERY );
    if( xAxis.is() )
    {
        ScfPropertySet aAxisProp( xAxis );
        // #i58688# axis enabled
        aAxisProp.SetBoolProperty( EXC_CHPROP_SHOW, !mxAxisLine || mxAxisLine->IsShowAxis() );

        // axis line properties
        if( mxAxisLine )
            mxAxisLine->Convert( GetChRoot(), aAxisProp, EXC_CHOBJTYPE_AXISLINE );
        // axis ticks properties
        if( mxTick )
            mxTick->Convert( aAxisProp );

        // axis caption text --------------------------------------------------

        // radar charts disable their category labels via chart type, not via axis
        bool bHasLabels = (!mxTick || mxTick->HasLabels()) &&
                          ((GetAxisType() != EXC_CHAXIS_X) || rTypeGroup.HasCategoryLabels());
        aAxisProp.SetBoolProperty( EXC_CHPROP_DISPLAYLABELS, bHasLabels );
        if( bHasLabels )
        {
            // font settings from CHFONT record or from default text
            if( mxFont )
                ConvertFontBase( GetChRoot(), aAxisProp );
            else if( const XclImpChText* pDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_AXISLABEL ) )
                pDefText->ConvertFont( aAxisProp );
            // label text rotation
            ConvertRotationBase( aAxisProp, true );
            // number format
            bool bLinkNumberFmtToSource = true;
            if ( mnNumFmtIdx != EXC_FORMAT_NOTFOUND )
            {
                sal_uInt32 nScNumFmt = GetNumFmtBuffer().GetScFormat( mnNumFmtIdx );
                if( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
                {
                    aAxisProp.SetProperty( EXC_CHPROP_NUMBERFORMAT, static_cast< sal_Int32 >( nScNumFmt ) );
                    bLinkNumberFmtToSource = false;
                }
            }

            aAxisProp.SetProperty( EXC_CHPROP_NUMBERFORMAT_LINKSRC, bLinkNumberFmtToSource );
        }

        // axis scaling and increment -----------------------------------------

        const XclChExtTypeInfo& rTypeInfo = rTypeGroup.GetTypeInfo();
        ScaleData aScaleData = xAxis->getScaleData();
        // set axis type
        switch( GetAxisType() )
        {
            case EXC_CHAXIS_X:
                if( rTypeInfo.mbCategoryAxis )
                {
                    aScaleData.AxisType = cssc2::AxisType::CATEGORY;
                    aScaleData.Categories = rTypeGroup.CreateCategSequence();
                }
                else
                    aScaleData.AxisType = cssc2::AxisType::REALNUMBER;
            break;
            case EXC_CHAXIS_Y:
                aScaleData.AxisType = rTypeGroup.IsPercent() ?
                    cssc2::AxisType::PERCENT : cssc2::AxisType::REALNUMBER;
            break;
            case EXC_CHAXIS_Z:
                aScaleData.AxisType = cssc2::AxisType::SERIES;
            break;
        }
        // axis scaling settings, dependent on axis type
        switch( aScaleData.AxisType )
        {
            case cssc2::AxisType::CATEGORY:
            case cssc2::AxisType::SERIES:
                // #i71684# radar charts have reversed rotation direction
                if (mxLabelRange)
                    mxLabelRange->Convert( aAxisProp, aScaleData, rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR );
                else
                    SAL_WARN("sc.filter", "missing LabelRange");
            break;
            case cssc2::AxisType::REALNUMBER:
            case cssc2::AxisType::PERCENT:
                // #i85167# pie/donut charts have reversed rotation direction (at Y axis!)
                if (mxValueRange)
                    mxValueRange->Convert( aScaleData, rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE );
                else
                    SAL_WARN("sc.filter", "missing ValueRange");
            break;
            default:
                OSL_FAIL( "XclImpChAxis::CreateAxis - unknown axis type" );
        }

        /*  Do not set a value to the Origin member anymore (will be done via
            new axis properties 'CrossoverPosition' and 'CrossoverValue'). */
        aScaleData.Origin.clear();

        // write back
        xAxis->setScaleData( aScaleData );

        // grid ---------------------------------------------------------------

        // main grid
        ScfPropertySet aGridProp( xAxis->getGridProperties() );
        aGridProp.SetBoolProperty( EXC_CHPROP_SHOW, static_cast<bool>(mxMajorGrid) );
        if( mxMajorGrid )
            mxMajorGrid->Convert( GetChRoot(), aGridProp, EXC_CHOBJTYPE_GRIDLINE );
        // sub grid
        Sequence< Reference< XPropertySet > > aSubGridPropSeq = xAxis->getSubGridProperties();
        if( aSubGridPropSeq.hasElements() )
        {
            ScfPropertySet aSubGridProp( aSubGridPropSeq[ 0 ] );
            aSubGridProp.SetBoolProperty( EXC_CHPROP_SHOW, static_cast<bool>(mxMinorGrid) );
            if( mxMinorGrid )
                mxMinorGrid->Convert( GetChRoot(), aSubGridProp, EXC_CHOBJTYPE_GRIDLINE );
        }

        // position of crossing axis ------------------------------------------

        if( pCrossingAxis )
            pCrossingAxis->ConvertAxisPosition( aAxisProp, rTypeGroup );
    }
    return xAxis;
}

void XclImpChAxis::ConvertWall( ScfPropertySet& rPropSet ) const
{
    // #i71810# walls and floor in 3D charts use the CHPICFORMAT record for bitmap mode
    if( mxWallFrame )
        mxWallFrame->Convert( rPropSet, true );
}

void XclImpChAxis::ConvertAxisPosition( ScfPropertySet& rPropSet, const XclImpChTypeGroup& rTypeGroup ) const
{
    if( ((GetAxisType() == EXC_CHAXIS_X) && rTypeGroup.GetTypeInfo().mbCategoryAxis) || (GetAxisType() == EXC_CHAXIS_Z) )
    {
        if (mxLabelRange)
            mxLabelRange->ConvertAxisPosition( rPropSet, rTypeGroup.Is3dChart() );
        else
            SAL_WARN("sc.filter", "missing LabelRange");
    }
    else
    {
        if (mxValueRange)
            mxValueRange->ConvertAxisPosition( rPropSet );
        else
            SAL_WARN("sc.filter", "missing ValueRange");
    }
}

void XclImpChAxis::ReadChAxisLine( XclImpStream& rStrm )
{
    XclImpChLineFormatRef* pxLineFmt = nullptr;
    bool bWallFrame = false;
    switch( rStrm.ReaduInt16() )
    {
        case EXC_CHAXISLINE_AXISLINE:   pxLineFmt = &mxAxisLine;    break;
        case EXC_CHAXISLINE_MAJORGRID:  pxLineFmt = &mxMajorGrid;   break;
        case EXC_CHAXISLINE_MINORGRID:  pxLineFmt = &mxMinorGrid;   break;
        case EXC_CHAXISLINE_WALLS:      bWallFrame = true;          break;
    }
    if( bWallFrame )
        CreateWallFrame();

    bool bLoop = pxLineFmt || bWallFrame;
    while( bLoop )
    {
        sal_uInt16 nRecId = rStrm.GetNextRecId();
        bLoop = ((nRecId == EXC_ID_CHLINEFORMAT) ||
                 (nRecId == EXC_ID_CHAREAFORMAT) ||
                 (nRecId == EXC_ID_CHESCHERFORMAT))
                 && rStrm.StartNextRecord();
        if( bLoop )
        {
            if( pxLineFmt && (nRecId == EXC_ID_CHLINEFORMAT) )
            {
                pxLineFmt->reset( new XclImpChLineFormat );
                (*pxLineFmt)->ReadChLineFormat( rStrm );
            }
            else if( bWallFrame && mxWallFrame )
            {
                mxWallFrame->ReadSubRecord( rStrm );
            }
        }
    }
}

void XclImpChAxis::CreateWallFrame()
{
    switch( GetAxisType() )
    {
        case EXC_CHAXIS_X:
            mxWallFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_WALL3D ) );
        break;
        case EXC_CHAXIS_Y:
            mxWallFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_FLOOR3D ) );
        break;
        default:
            mxWallFrame.reset();
    }
}

XclImpChAxesSet::XclImpChAxesSet( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclImpChRoot( rRoot )
{
    maData.mnAxesSetId = nAxesSetId;
}

void XclImpChAxesSet::ReadHeaderRecord( XclImpStream& rStrm )
{
    maData.mnAxesSetId = rStrm.ReaduInt16();
    rStrm >> maData.maRect;
}

void XclImpChAxesSet::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAMEPOS:
            mxFramePos.reset( new XclImpChFramePos );
            mxFramePos->ReadChFramePos( rStrm );
        break;
        case EXC_ID_CHAXIS:
            ReadChAxis( rStrm );
        break;
        case EXC_ID_CHTEXT:
            ReadChText( rStrm );
        break;
        case EXC_ID_CHPLOTFRAME:
            ReadChPlotFrame( rStrm );
        break;
        case EXC_ID_CHTYPEGROUP:
            ReadChTypeGroup( rStrm );
        break;
    }
}

void XclImpChAxesSet::Finalize()
{
    if( IsValidAxesSet() )
    {
        // finalize chart type groups, erase empty groups without series
        XclImpChTypeGroupMap aValidGroups;
        for (auto const& typeGroup : maTypeGroups)
        {
            XclImpChTypeGroupRef xTypeGroup = typeGroup.second;
            xTypeGroup->Finalize();
            if( xTypeGroup->IsValidGroup() )
                aValidGroups.emplace(typeGroup.first, xTypeGroup);
        }
        maTypeGroups.swap( aValidGroups );
    }

    // invalid chart type groups are deleted now, check again with IsValidAxesSet()
    if( IsValidAxesSet() )
    {
        // always create missing axis objects
        if( !mxXAxis )
            mxXAxis.reset( new XclImpChAxis( GetChRoot(), EXC_CHAXIS_X ) );
        if( !mxYAxis )
            mxYAxis.reset( new XclImpChAxis( GetChRoot(), EXC_CHAXIS_Y ) );
        if( !mxZAxis && GetFirstTypeGroup()->Is3dDeepChart() )
            mxZAxis.reset( new XclImpChAxis( GetChRoot(), EXC_CHAXIS_Z ) );

        // finalize axes
        if( mxXAxis ) mxXAxis->Finalize();
        if( mxYAxis ) mxYAxis->Finalize();
        if( mxZAxis ) mxZAxis->Finalize();

        // finalize axis titles
        const XclImpChText* pDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_AXISTITLE );
        OUString aAutoTitle("Axis Title");
        lclFinalizeTitle( mxXAxisTitle, pDefText, aAutoTitle );
        lclFinalizeTitle( mxYAxisTitle, pDefText, aAutoTitle );
        lclFinalizeTitle( mxZAxisTitle, pDefText, aAutoTitle );

        // #i47745# missing plot frame -> invisible border and area
        if( !mxPlotFrame )
            mxPlotFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_PLOTFRAME ) );
    }
}

XclImpChTypeGroupRef XclImpChAxesSet::GetTypeGroup( sal_uInt16 nGroupIdx ) const
{
    XclImpChTypeGroupMap::const_iterator itr = maTypeGroups.find(nGroupIdx);
    return itr == maTypeGroups.end() ? XclImpChTypeGroupRef() : itr->second;
}

XclImpChTypeGroupRef XclImpChAxesSet::GetFirstTypeGroup() const
{
    XclImpChTypeGroupRef xTypeGroup;
    if( !maTypeGroups.empty() )
        xTypeGroup = maTypeGroups.begin()->second;
    return xTypeGroup;
}

XclImpChLegendRef XclImpChAxesSet::GetLegend() const
{
    XclImpChLegendRef xLegend;
    for( const auto& rEntry : maTypeGroups )
    {
        xLegend = rEntry.second->GetLegend();
        if (xLegend)
            break;
    }
    return xLegend;
}

OUString XclImpChAxesSet::GetSingleSeriesTitle() const
{
    return (maTypeGroups.size() == 1) ? maTypeGroups.begin()->second->GetSingleSeriesTitle() : OUString();
}

void XclImpChAxesSet::Convert( Reference< XDiagram > const & xDiagram ) const
{
    if( IsValidAxesSet() && xDiagram.is() )
    {
        // diagram background formatting
        if( GetAxesSetId() == EXC_CHAXESSET_PRIMARY )
            ConvertBackground( xDiagram );

        // create the coordinate system, this inserts all chart types and series
        Reference< XCoordinateSystem > xCoordSystem = CreateCoordSystem( xDiagram );
        if( xCoordSystem.is() )
        {
            // insert coordinate system, if not already done
            try
            {
                Reference< XCoordinateSystemContainer > xCoordSystemCont( xDiagram, UNO_QUERY_THROW );
                Sequence< Reference< XCoordinateSystem > > aCoordSystems = xCoordSystemCont->getCoordinateSystems();
                if( aCoordSystems.getLength() == 0 )
                    xCoordSystemCont->addCoordinateSystem( xCoordSystem );
            }
            catch( Exception& )
            {
                OSL_FAIL( "XclImpChAxesSet::Convert - cannot insert coordinate system" );
            }

            // create the axes with grids and axis titles and insert them into the diagram
            ConvertAxis( mxXAxis, mxXAxisTitle, xCoordSystem, mxYAxis.get() );
            ConvertAxis( mxYAxis, mxYAxisTitle, xCoordSystem, mxXAxis.get() );
            ConvertAxis( mxZAxis, mxZAxisTitle, xCoordSystem, nullptr );
        }
    }
}

void XclImpChAxesSet::ConvertTitlePositions() const
{
    if( mxXAxisTitle )
        mxXAxisTitle->ConvertTitlePosition( XclChTextKey( EXC_CHTEXTTYPE_AXISTITLE, maData.mnAxesSetId, EXC_CHAXIS_X ) );
    if( mxYAxisTitle )
        mxYAxisTitle->ConvertTitlePosition( XclChTextKey( EXC_CHTEXTTYPE_AXISTITLE, maData.mnAxesSetId, EXC_CHAXIS_Y ) );
    if( mxZAxisTitle )
        mxZAxisTitle->ConvertTitlePosition( XclChTextKey( EXC_CHTEXTTYPE_AXISTITLE, maData.mnAxesSetId, EXC_CHAXIS_Z ) );
}

void XclImpChAxesSet::ReadChAxis( XclImpStream& rStrm )
{
    XclImpChAxisRef xAxis( new XclImpChAxis( GetChRoot() ) );
    xAxis->ReadRecordGroup( rStrm );

    switch( xAxis->GetAxisType() )
    {
        case EXC_CHAXIS_X:  mxXAxis = xAxis;    break;
        case EXC_CHAXIS_Y:  mxYAxis = xAxis;    break;
        case EXC_CHAXIS_Z:  mxZAxis = xAxis;    break;
    }
}

void XclImpChAxesSet::ReadChText( XclImpStream& rStrm )
{
    XclImpChTextRef xText( new XclImpChText( GetChRoot() ) );
    xText->ReadRecordGroup( rStrm );

    switch( xText->GetLinkTarget() )
    {
        case EXC_CHOBJLINK_XAXIS:   mxXAxisTitle = xText;   break;
        case EXC_CHOBJLINK_YAXIS:   mxYAxisTitle = xText;   break;
        case EXC_CHOBJLINK_ZAXIS:   mxZAxisTitle = xText;   break;
    }
}

void XclImpChAxesSet::ReadChPlotFrame( XclImpStream& rStrm )
{
    if( (rStrm.GetNextRecId() == EXC_ID_CHFRAME) && rStrm.StartNextRecord() )
    {
        mxPlotFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_PLOTFRAME ) );
        mxPlotFrame->ReadRecordGroup( rStrm );
    }
}

void XclImpChAxesSet::ReadChTypeGroup( XclImpStream& rStrm )
{
    XclImpChTypeGroupRef xTypeGroup( new XclImpChTypeGroup( GetChRoot() ) );
    xTypeGroup->ReadRecordGroup( rStrm );
    sal_uInt16 nGroupIdx = xTypeGroup->GetGroupIdx();
    XclImpChTypeGroupMap::iterator itr = maTypeGroups.lower_bound(nGroupIdx);
    if (itr != maTypeGroups.end() && !maTypeGroups.key_comp()(nGroupIdx, itr->first))
        // Overwrite the existing element.
        itr->second = xTypeGroup;
    else
        maTypeGroups.insert(
            itr, XclImpChTypeGroupMap::value_type(nGroupIdx, xTypeGroup));
}

Reference< XCoordinateSystem > XclImpChAxesSet::CreateCoordSystem( Reference< XDiagram > const & xDiagram ) const
{
    Reference< XCoordinateSystem > xCoordSystem;

    /*  Try to get existing coordinate system. For now, all series from primary
        and secondary axes sets are inserted into one coordinate system. Later,
        this should be changed to use one coordinate system for each axes set. */
    Reference< XCoordinateSystemContainer > xCoordSystemCont( xDiagram, UNO_QUERY );
    if( xCoordSystemCont.is() )
    {
        Sequence< Reference< XCoordinateSystem > > aCoordSystems = xCoordSystemCont->getCoordinateSystems();
        OSL_ENSURE( aCoordSystems.getLength() <= 1, "XclImpChAxesSet::CreateCoordSystem - too many existing coordinate systems" );
        if( aCoordSystems.getLength() > 0 )
            xCoordSystem = aCoordSystems[ 0 ];
    }

    // create the coordinate system according to the first chart type
    if( !xCoordSystem.is() )
    {
        XclImpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
        if( xTypeGroup )
        {
            xCoordSystem = xTypeGroup->CreateCoordSystem();
            // convert 3d chart settings
            ScfPropertySet aDiaProp( xDiagram );
            xTypeGroup->ConvertChart3d( aDiaProp );
        }
    }

    /*  Create XChartType objects for all chart type groups. Each group will
        add its series to the data provider attached to the chart document. */
    Reference< XChartTypeContainer > xChartTypeCont( xCoordSystem, UNO_QUERY );
    if( xChartTypeCont.is() )
    {
        sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();
        for( const auto& rEntry : maTypeGroups )
        {
            try
            {
                Reference< XChartType > xChartType = rEntry.second->CreateChartType( xDiagram, nApiAxesSetIdx );
                if( xChartType.is() )
                    xChartTypeCont->addChartType( xChartType );
            }
            catch( Exception& )
            {
                OSL_FAIL( "XclImpChAxesSet::CreateCoordSystem - cannot add chart type" );
            }
        }
    }

    return xCoordSystem;
}

void XclImpChAxesSet::ConvertAxis(
        XclImpChAxisRef const & xChAxis, XclImpChTextRef const & xChAxisTitle,
        Reference< XCoordinateSystem > const & xCoordSystem, const XclImpChAxis* pCrossingAxis ) const
{
    if( xChAxis )
    {
        // create and attach the axis object
        Reference< XAxis > xAxis = CreateAxis( *xChAxis, pCrossingAxis );
        if( xAxis.is() )
        {
            // create and attach the axis title
            if( xChAxisTitle ) try
            {
                Reference< XTitled > xTitled( xAxis, UNO_QUERY_THROW );
                Reference< XTitle > xTitle( xChAxisTitle->CreateTitle(), UNO_SET_THROW );
                xTitled->setTitleObject( xTitle );
            }
            catch( Exception& )
            {
                OSL_FAIL( "XclImpChAxesSet::ConvertAxis - cannot set axis title" );
            }

            // insert axis into coordinate system
            try
            {
                sal_Int32 nApiAxisDim = xChAxis->GetApiAxisDimension();
                sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();
                xCoordSystem->setAxisByDimension( nApiAxisDim, xAxis, nApiAxesSetIdx );
            }
            catch( Exception& )
            {
                OSL_FAIL( "XclImpChAxesSet::ConvertAxis - cannot set axis" );
            }
        }
    }
}

Reference< XAxis > XclImpChAxesSet::CreateAxis( const XclImpChAxis& rChAxis, const XclImpChAxis* pCrossingAxis ) const
{
    Reference< XAxis > xAxis;
    if( const XclImpChTypeGroup* pTypeGroup = GetFirstTypeGroup().get() )
        xAxis = rChAxis.CreateAxis( *pTypeGroup, pCrossingAxis );
    return xAxis;
}

void XclImpChAxesSet::ConvertBackground( Reference< XDiagram > const & xDiagram ) const
{
    XclImpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
    if( xTypeGroup && xTypeGroup->Is3dWallChart() )
    {
        // wall/floor formatting (3D charts)
        if( mxXAxis )
        {
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxXAxis->ConvertWall( aWallProp );
        }
        if( mxYAxis )
        {
            ScfPropertySet aFloorProp( xDiagram->getFloor() );
            mxYAxis->ConvertWall( aFloorProp );
        }
    }
    else if( mxPlotFrame )
    {
        // diagram background formatting
        ScfPropertySet aWallProp( xDiagram->getWall() );
        mxPlotFrame->Convert( aWallProp );
    }
}

// The chart object ===========================================================

XclImpChChart::XclImpChChart( const XclImpRoot& rRoot ) :
    XclImpChRoot( rRoot, *this )
{
    mxPrimAxesSet.reset( new XclImpChAxesSet( GetChRoot(), EXC_CHAXESSET_PRIMARY ) );
    mxSecnAxesSet.reset( new XclImpChAxesSet( GetChRoot(), EXC_CHAXESSET_SECONDARY ) );
}

XclImpChChart::~XclImpChChart()
{
}

void XclImpChChart::ReadHeaderRecord( XclImpStream& rStrm )
{
    // coordinates are stored as 16.16 fixed point
    rStrm >> maRect;
}

void XclImpChChart::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_BACKGROUND ) );
            mxFrame->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHSERIES:
            ReadChSeries( rStrm );
        break;
        case EXC_ID_CHPROPERTIES:
            ReadChProperties( rStrm );
        break;
        case EXC_ID_CHDEFAULTTEXT:
            ReadChDefaultText( rStrm );
        break;
        case EXC_ID_CHAXESSET:
            ReadChAxesSet( rStrm );
        break;
        case EXC_ID_CHTEXT:
            ReadChText( rStrm );
        break;
        case EXC_ID_CHEND:
            Finalize();     // finalize the entire chart object
        break;
    }
}

void XclImpChChart::ReadChDefaultText( XclImpStream& rStrm )
{
    sal_uInt16 nTextId = rStrm.ReaduInt16();
    if( (rStrm.GetNextRecId() == EXC_ID_CHTEXT) && rStrm.StartNextRecord() )
    {
        unique_ptr<XclImpChText> pText(new XclImpChText(GetChRoot()));
        pText->ReadRecordGroup(rStrm);
        m_DefTexts.insert(std::make_pair(nTextId, std::move(pText)));
    }
}

void XclImpChChart::ReadChDataFormat( XclImpStream& rStrm )
{
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->ReadRecordGroup( rStrm );
    if( xDataFmt->GetPointPos().mnSeriesIdx <= EXC_CHSERIES_MAXSERIES )
    {
        const XclChDataPointPos& rPos = xDataFmt->GetPointPos();
        XclImpChDataFormatMap::iterator itr = maDataFmts.lower_bound(rPos);
        if (itr == maDataFmts.end() || maDataFmts.key_comp()(rPos, itr->first))
            // No element exists for this data point.  Insert it.
            maDataFmts.insert(
                itr, XclImpChDataFormatMap::value_type(rPos, xDataFmt));

        /*  Do not overwrite existing data format group, Excel always uses the
            first data format group occurring in any CHSERIES group. */
    }
}

void XclImpChChart::UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData )
{
    if( !mxFrame )
        mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_BACKGROUND ) );
    mxFrame->UpdateObjFrame( rLineData, rFillData );
}

XclImpChTypeGroupRef XclImpChChart::GetTypeGroup( sal_uInt16 nGroupIdx ) const
{
    XclImpChTypeGroupRef xTypeGroup = mxPrimAxesSet->GetTypeGroup( nGroupIdx );
    if( !xTypeGroup ) xTypeGroup = mxSecnAxesSet->GetTypeGroup( nGroupIdx );
    if( !xTypeGroup ) xTypeGroup = mxPrimAxesSet->GetFirstTypeGroup();
    return xTypeGroup;
}

const XclImpChText* XclImpChChart::GetDefaultText( XclChTextType eTextType ) const
{
    sal_uInt16 nDefTextId = EXC_CHDEFTEXT_GLOBAL;
    bool bBiff8 = GetBiff() == EXC_BIFF8;
    switch( eTextType )
    {
        case EXC_CHTEXTTYPE_TITLE:      nDefTextId = EXC_CHDEFTEXT_GLOBAL;                                  break;
        case EXC_CHTEXTTYPE_LEGEND:     nDefTextId = EXC_CHDEFTEXT_GLOBAL;                                  break;
        case EXC_CHTEXTTYPE_AXISTITLE:  nDefTextId = bBiff8 ? EXC_CHDEFTEXT_AXESSET : EXC_CHDEFTEXT_GLOBAL; break;
        case EXC_CHTEXTTYPE_AXISLABEL:  nDefTextId = bBiff8 ? EXC_CHDEFTEXT_AXESSET : EXC_CHDEFTEXT_GLOBAL; break;
        case EXC_CHTEXTTYPE_DATALABEL:  nDefTextId = bBiff8 ? EXC_CHDEFTEXT_AXESSET : EXC_CHDEFTEXT_GLOBAL; break;
    }

    XclImpChTextMap::const_iterator const itr = m_DefTexts.find(nDefTextId);
    return itr == m_DefTexts.end() ? nullptr : itr->second.get();
}

bool XclImpChChart::IsManualPlotArea() const
{
    // there is no real automatic mode in BIFF5 charts
    return (GetBiff() <= EXC_BIFF5) || ::get_flag( maProps.mnFlags, EXC_CHPROPS_USEMANPLOTAREA );
}

void XclImpChChart::Convert( const Reference<XChartDocument>& xChartDoc,
        XclImpDffConverter& rDffConv, const OUString& rObjName, const tools::Rectangle& rChartRect ) const
{
    // initialize conversion (locks the model to suppress any internal updates)
    InitConversion( xChartDoc, rChartRect );

    // chart frame formatting
    if( mxFrame )
    {
        ScfPropertySet aFrameProp( xChartDoc->getPageBackground() );
        mxFrame->Convert( aFrameProp );
    }

    // chart title
    if( mxTitle ) try
    {
        Reference< XTitled > xTitled( xChartDoc, UNO_QUERY_THROW );
        Reference< XTitle > xTitle( mxTitle->CreateTitle(), UNO_SET_THROW );
        xTitled->setTitleObject( xTitle );
    }
    catch( Exception& )
    {
    }

    /*  Create the diagram object and attach it to the chart document. Currently,
        one diagram is used to carry all coordinate systems and data series. */
    Reference< XDiagram > xDiagram = CreateDiagram();
    xChartDoc->setFirstDiagram( xDiagram );

    // coordinate systems and chart types, convert axis settings
    mxPrimAxesSet->Convert( xDiagram );
    mxSecnAxesSet->Convert( xDiagram );

    // legend
    if( xDiagram.is() && mxLegend )
        xDiagram->setLegend( mxLegend->CreateLegend() );

    /*  Following all conversions needing the old Chart1 API that involves full
        initialization of the chart view. */
    Reference< cssc::XChartDocument > xChart1Doc( xChartDoc, UNO_QUERY );
    if( xChart1Doc.is() )
    {
        Reference< cssc::XDiagram > xDiagram1 = xChart1Doc->getDiagram();

        /*  Set the 'IncludeHiddenCells' property via the old API as only this
            ensures that the data provider and all created sequences get this
            flag correctly. */
        ScfPropertySet aDiaProp( xDiagram1 );
        bool bShowVisCells = ::get_flag( maProps.mnFlags, EXC_CHPROPS_SHOWVISIBLEONLY );
        aDiaProp.SetBoolProperty( EXC_CHPROP_INCLUDEHIDDENCELLS, !bShowVisCells );

        // plot area position and size (there is no real automatic mode in BIFF5 charts)
        XclImpChFramePosRef xPlotAreaPos = mxPrimAxesSet->GetPlotAreaFramePos();
        if( IsManualPlotArea() && xPlotAreaPos ) try
        {
            const XclChFramePos& rFramePos = xPlotAreaPos->GetFramePosData();
            if( (rFramePos.mnTLMode == EXC_CHFRAMEPOS_PARENT) && (rFramePos.mnBRMode == EXC_CHFRAMEPOS_PARENT) )
            {
                Reference< cssc::XDiagramPositioning > xPositioning( xDiagram1, UNO_QUERY_THROW );
                css::awt::Rectangle aDiagramRect = CalcHmmFromChartRect( rFramePos.maRect );
                // for pie charts, always set inner plot area size to exclude the data labels as Excel does
                const XclImpChTypeGroup* pFirstTypeGroup = mxPrimAxesSet->GetFirstTypeGroup().get();
                if( pFirstTypeGroup && (pFirstTypeGroup->GetTypeInfo().meTypeCateg == EXC_CHTYPECATEG_PIE) )
                    xPositioning->setDiagramPositionExcludingAxes( aDiagramRect );
                else if( pFirstTypeGroup && pFirstTypeGroup->Is3dChart() )
                    xPositioning->setDiagramPositionIncludingAxesAndAxisTitles( aDiagramRect );
                else
                    xPositioning->setDiagramPositionIncludingAxes( aDiagramRect );
            }
        }
        catch( Exception& )
        {
        }

        // positions of all title objects
        if( mxTitle )
            mxTitle->ConvertTitlePosition( XclChTextKey( EXC_CHTEXTTYPE_TITLE ) );
        mxPrimAxesSet->ConvertTitlePositions();
        mxSecnAxesSet->ConvertTitlePositions();
    }

    // unlock the model
    FinishConversion( rDffConv );

    // start listening to this chart
    ScDocument& rDoc = GetRoot().GetDoc();
    if( ScChartListenerCollection* pChartCollection = rDoc.GetChartListenerCollection() )
    {
        ::std::unique_ptr< ::std::vector< ScTokenRef > > xRefTokens( new ::std::vector< ScTokenRef > );
        for( const auto& rxSeries : maSeries )
            rxSeries->FillAllSourceLinks( *xRefTokens );
        if( !xRefTokens->empty() )
        {
            ::std::unique_ptr< ScChartListener > xListener( new ScChartListener( rObjName, &rDoc, std::move(xRefTokens) ) );
            xListener->SetUsed( true );
            xListener->StartListeningTo();
            pChartCollection->insert( xListener.release() );
        }
    }
}

void XclImpChChart::ReadChSeries( XclImpStream& rStrm )
{
    sal_uInt16 nNewSeriesIdx = static_cast< sal_uInt16 >( maSeries.size() );
    XclImpChSeriesRef xSeries( new XclImpChSeries( GetChRoot(), nNewSeriesIdx ) );
    xSeries->ReadRecordGroup( rStrm );
    maSeries.push_back( xSeries );
}

void XclImpChChart::ReadChProperties( XclImpStream& rStrm )
{
    maProps.mnFlags = rStrm.ReaduInt16();
    maProps.mnEmptyMode = rStrm.ReaduInt8();
}

void XclImpChChart::ReadChAxesSet( XclImpStream& rStrm )
{
    XclImpChAxesSetRef xAxesSet( new XclImpChAxesSet( GetChRoot(), EXC_CHAXESSET_NONE ) );
    xAxesSet->ReadRecordGroup( rStrm );
    switch( xAxesSet->GetAxesSetId() )
    {
        case EXC_CHAXESSET_PRIMARY:     mxPrimAxesSet = xAxesSet;   break;
        case EXC_CHAXESSET_SECONDARY:   mxSecnAxesSet = xAxesSet;   break;
    }
}

void XclImpChChart::ReadChText( XclImpStream& rStrm )
{
    XclImpChTextRef xText( new XclImpChText( GetChRoot() ) );
    xText->ReadRecordGroup( rStrm );
    switch( xText->GetLinkTarget() )
    {
        case EXC_CHOBJLINK_TITLE:
            mxTitle = xText;
        break;
        case EXC_CHOBJLINK_DATA:
        {
            sal_uInt16 nSeriesIdx = xText->GetPointPos().mnSeriesIdx;
            if( nSeriesIdx < maSeries.size() )
                maSeries[ nSeriesIdx ]->SetDataLabel( xText );
        }
        break;
    }
}

void XclImpChChart::Finalize()
{
    // finalize series (must be done first)
    FinalizeSeries();
    // #i49218# legend may be attached to primary or secondary axes set
    mxLegend = mxPrimAxesSet->GetLegend();
    if( !mxLegend )
        mxLegend = mxSecnAxesSet->GetLegend();
    if( mxLegend )
        mxLegend->Finalize();
    // axes sets, updates chart type group default formats -> must be called before FinalizeDataFormats()
    mxPrimAxesSet->Finalize();
    mxSecnAxesSet->Finalize();
    // formatting of all series
    FinalizeDataFormats();
    // #i47745# missing frame -> invisible border and area
    if( !mxFrame )
        mxFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_BACKGROUND ) );
    // chart title
    FinalizeTitle();
}

void XclImpChChart::FinalizeSeries()
{
    for( const XclImpChSeriesRef& xSeries : maSeries )
    {
        if( xSeries->HasParentSeries() )
        {
            /*  Process child series (trend lines and error bars). Data of
                child series will be set at the connected parent series. */
            if( xSeries->GetParentIdx() < maSeries.size() )
                maSeries[ xSeries->GetParentIdx() ]->AddChildSeries( *xSeries );
        }
        else
        {
            // insert the series into the related chart type group
            if( XclImpChTypeGroup* pTypeGroup = GetTypeGroup( xSeries->GetGroupIdx() ).get() )
                pTypeGroup->AddSeries( xSeries );
        }
    }
}

void XclImpChChart::FinalizeDataFormats()
{
    /*  #i51639# (part 1): CHDATAFORMAT groups are part of CHSERIES groups.
        Each CHDATAFORMAT group specifies the series and data point it is
        assigned to. This makes it possible to have a data format that is
        related to another series, e.g. a CHDATAFORMAT group for series 2 is
        part of a CHSERIES group that describes series 1. Therefore the chart
        itself has collected all CHDATAFORMAT groups to be able to store data
        format groups for series that have not been imported at that time. This
        loop finally assigns these groups to the related series. */
    for( const auto& [rPos, rDataFmt] : maDataFmts )
    {
        sal_uInt16 nSeriesIdx = rPos.mnSeriesIdx;
        if( nSeriesIdx < maSeries.size() )
            maSeries[ nSeriesIdx ]->SetDataFormat( rDataFmt );
    }

    /*  #i51639# (part 2): Finalize data formats of all series. This adds for
        example missing CHDATAFORMAT groups for entire series that are needed
        for automatic colors of lines and areas. */
    for( auto& rxSeries : maSeries )
        rxSeries->FinalizeDataFormats();
}

void XclImpChChart::FinalizeTitle()
{
    // special handling for auto-generated title
    OUString aAutoTitle;
    if( !mxTitle || (!mxTitle->IsDeleted() && !mxTitle->HasString()) )
    {
        // automatic title from first series name (if there are no series on secondary axes set)
        if( !mxSecnAxesSet->IsValidAxesSet() )
            aAutoTitle = mxPrimAxesSet->GetSingleSeriesTitle();
        if( mxTitle || (!aAutoTitle.isEmpty()) )
        {
            if( !mxTitle )
                mxTitle.reset( new XclImpChText( GetChRoot() ) );
            if( aAutoTitle.isEmpty() )
                aAutoTitle = "Chart Title";
        }
    }

    // will reset mxTitle, if it does not contain a string and no auto title exists
    lclFinalizeTitle( mxTitle, GetDefaultText( EXC_CHTEXTTYPE_TITLE ), aAutoTitle );
}

Reference< XDiagram > XclImpChChart::CreateDiagram() const
{
    // create a diagram object
    Reference< XDiagram > xDiagram( ScfApiHelper::CreateInstance( SERVICE_CHART2_DIAGRAM ), UNO_QUERY );

    // convert global chart settings
    ScfPropertySet aDiaProp( xDiagram );

    // treatment of missing values
    using namespace cssc::MissingValueTreatment;
    sal_Int32 nMissingValues = LEAVE_GAP;
    switch( maProps.mnEmptyMode )
    {
        case EXC_CHPROPS_EMPTY_SKIP:        nMissingValues = LEAVE_GAP; break;
        case EXC_CHPROPS_EMPTY_ZERO:        nMissingValues = USE_ZERO;  break;
        case EXC_CHPROPS_EMPTY_INTERPOLATE: nMissingValues = CONTINUE;  break;
    }
    aDiaProp.SetProperty( EXC_CHPROP_MISSINGVALUETREATMENT, nMissingValues );

    return xDiagram;
}

XclImpChartDrawing::XclImpChartDrawing( const XclImpRoot& rRoot, bool bOwnTab ) :
    XclImpDrawing( rRoot, bOwnTab ), // sheet charts may contain OLE objects
    mnScTab( rRoot.GetCurrScTab() ),
    mbOwnTab( bOwnTab )
{
}

void XclImpChartDrawing::ConvertObjects( XclImpDffConverter& rDffConv,
        const Reference< XModel >& rxModel, const tools::Rectangle& rChartRect )
{
    maChartRect = rChartRect;   // needed in CalcAnchorRect() callback

    SdrModel* pSdrModel = nullptr;
    SdrPage* pSdrPage = nullptr;
    if( mbOwnTab )
    {
        // chart sheet: insert all shapes into the sheet, not into the chart object
        pSdrModel = GetDoc().GetDrawLayer();
        pSdrPage = GetSdrPage( mnScTab );
    }
    else
    {
        // embedded chart object: insert all shapes into the chart
        try
        {
            Reference< XDrawPageSupplier > xDrawPageSupp( rxModel, UNO_QUERY_THROW );
            Reference< XDrawPage > xDrawPage( xDrawPageSupp->getDrawPage(), UNO_SET_THROW );
            pSdrPage = ::GetSdrPageFromXDrawPage( xDrawPage );
            pSdrModel = pSdrPage ? &pSdrPage->getSdrModelFromSdrPage() : nullptr;
        }
        catch( Exception& )
        {
        }
    }

    if( pSdrModel && pSdrPage )
        ImplConvertObjects( rDffConv, *pSdrModel, *pSdrPage );
}

tools::Rectangle XclImpChartDrawing::CalcAnchorRect( const XclObjAnchor& rAnchor, bool bDffAnchor ) const
{
    /*  In objects with DFF client anchor, the position of the shape is stored
        in the cell address components of the client anchor. In old BIFF3-BIFF5
        objects, the position is stored in the offset components of the anchor. */
    tools::Rectangle aRect(
        static_cast< long >( static_cast< double >( bDffAnchor ? rAnchor.maFirst.mnCol : rAnchor.mnLX ) / EXC_CHART_TOTALUNITS * maChartRect.GetWidth()  + 0.5 ),
        static_cast< long >( static_cast< double >( bDffAnchor ? rAnchor.maFirst.mnRow : rAnchor.mnTY ) / EXC_CHART_TOTALUNITS * maChartRect.GetHeight() + 0.5 ),
        static_cast< long >( static_cast< double >( bDffAnchor ? rAnchor.maLast.mnCol  : rAnchor.mnRX ) / EXC_CHART_TOTALUNITS * maChartRect.GetWidth()  + 0.5 ),
        static_cast< long >( static_cast< double >( bDffAnchor ? rAnchor.maLast.mnRow  : rAnchor.mnBY ) / EXC_CHART_TOTALUNITS * maChartRect.GetHeight() + 0.5 ) );
    aRect.Justify();
    // move shapes into chart area for sheet charts
    if( mbOwnTab )
        aRect.Move( maChartRect.Left(), maChartRect.Top() );
    return aRect;
}

void XclImpChartDrawing::OnObjectInserted( const XclImpDrawObjBase& )
{
}

XclImpChart::XclImpChart( const XclImpRoot& rRoot, bool bOwnTab ) :
    XclImpRoot( rRoot ),
    mbOwnTab( bOwnTab ),
    mbIsPivotChart( false )
{
}

XclImpChart::~XclImpChart()
{
}

void XclImpChart::ReadChartSubStream( XclImpStream& rStrm )
{
    XclImpPageSettings& rPageSett = GetPageSettings();
    XclImpTabViewSettings& rTabViewSett = GetTabViewSettings();

    bool bLoop = true;
    while( bLoop && rStrm.StartNextRecord() )
    {
        // page settings - only for charts in entire sheet
        if( mbOwnTab ) switch( rStrm.GetRecId() )
        {
            case EXC_ID_HORPAGEBREAKS:
            case EXC_ID_VERPAGEBREAKS:  rPageSett.ReadPageBreaks( rStrm );      break;
            case EXC_ID_HEADER:
            case EXC_ID_FOOTER:         rPageSett.ReadHeaderFooter( rStrm );    break;
            case EXC_ID_LEFTMARGIN:
            case EXC_ID_RIGHTMARGIN:
            case EXC_ID_TOPMARGIN:
            case EXC_ID_BOTTOMMARGIN:   rPageSett.ReadMargin( rStrm );          break;
            case EXC_ID_PRINTHEADERS:   rPageSett.ReadPrintHeaders( rStrm );    break;
            case EXC_ID_PRINTGRIDLINES: rPageSett.ReadPrintGridLines( rStrm );  break;
            case EXC_ID_HCENTER:
            case EXC_ID_VCENTER:        rPageSett.ReadCenter( rStrm );          break;
            case EXC_ID_SETUP:          rPageSett.ReadSetup( rStrm );           break;
            case EXC_ID8_IMGDATA:       rPageSett.ReadImgData( rStrm );         break;

            case EXC_ID_WINDOW2:        rTabViewSett.ReadWindow2( rStrm, true );break;
            case EXC_ID_SCL:            rTabViewSett.ReadScl( rStrm );          break;

            case EXC_ID_SHEETEXT: //0x0862
            {
                // FIXME: do not need to pass palette, XclImpTabVieSettings is derived from root
                XclImpPalette& rPal = GetPalette();
                rTabViewSett.ReadTabBgColor( rStrm,  rPal);
            }
            break;

            case EXC_ID_CODENAME:       ReadCodeName( rStrm, false );           break;
        }

        // common records
        switch( rStrm.GetRecId() )
        {
            case EXC_ID_EOF:            bLoop = false;                          break;

            // #i31882# ignore embedded chart objects
            case EXC_ID2_BOF:
            case EXC_ID3_BOF:
            case EXC_ID4_BOF:
            case EXC_ID5_BOF:           XclTools::SkipSubStream( rStrm );       break;

            case EXC_ID_CHCHART:        ReadChChart( rStrm );                   break;

            case EXC_ID8_CHPIVOTREF:
                GetTracer().TracePivotChartExists();
                mbIsPivotChart = true;
            break;

            // BIFF specific records
            default: switch( GetBiff() )
            {
                case EXC_BIFF5: switch( rStrm.GetRecId() )
                {
                    case EXC_ID_OBJ:        GetChartDrawing().ReadObj( rStrm );         break;
                }
                break;
                case EXC_BIFF8: switch( rStrm.GetRecId() )
                {
                    case EXC_ID_MSODRAWING: GetChartDrawing().ReadMsoDrawing( rStrm );  break;
                    // #i61786# weird documents: OBJ without MSODRAWING -> read in BIFF5 format
                    case EXC_ID_OBJ:        GetChartDrawing().ReadObj( rStrm );         break;
                }
                break;
                default:;
            }
        }
    }
}

void XclImpChart::UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData )
{
    if( !mxChartData )
        mxChartData.reset( new XclImpChChart( GetRoot() ) );
    mxChartData->UpdateObjFrame( rLineData, rFillData );
}

std::size_t XclImpChart::GetProgressSize() const
{
    return
        (mxChartData ? XclImpChChart::GetProgressSize() : 0) +
        (mxChartDrawing ? mxChartDrawing->GetProgressSize() : 0);
}

void XclImpChart::Convert( Reference< XModel > const & xModel, XclImpDffConverter& rDffConv, const OUString& rObjName, const tools::Rectangle& rChartRect ) const
{
    Reference< XChartDocument > xChartDoc( xModel, UNO_QUERY );
    if( xChartDoc.is() )
    {
        if( mxChartData )
            mxChartData->Convert( xChartDoc, rDffConv, rObjName, rChartRect );
        if( mxChartDrawing )
            mxChartDrawing->ConvertObjects( rDffConv, xModel, rChartRect );
    }
}

XclImpChartDrawing& XclImpChart::GetChartDrawing()
{
    if( !mxChartDrawing )
        mxChartDrawing.reset( new XclImpChartDrawing( GetRoot(), mbOwnTab ) );
    return *mxChartDrawing;
}

void XclImpChart::ReadChChart( XclImpStream& rStrm )
{
    mxChartData.reset( new XclImpChChart( GetRoot() ) );
    mxChartData->ReadRecordGroup( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
