/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xichart.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:34:46 $
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

#ifndef SC_XICHART_HXX
#include "xichart.hxx"
#endif

#include <algorithm>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XDIAGRAM_HPP_
#include <com/sun/star/chart/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_X3DDISPLAY_HPP_
#include <com/sun/star/chart/X3DDisplay.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XSTATISTICDISPLAY_HPP_
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XAXISXSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISZSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISASSIGN_HPP_
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSYMBOLTYPE_HPP_
#include <com/sun/star/chart/ChartSymbolType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSOLIDTYPE_HPP_
#include <com/sun/star/chart/ChartSolidType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATACAPTION_HPP_
#include <com/sun/star/chart/ChartDataCaption.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISMARKS_HPP_
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTLEGENDPOSITION_HPP_
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTREGRESSIONCURVETYPE_HPP_
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORCATEGORY_HPP_
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XIFORMULA_HXX
#include "xiformula.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XIVIEW_HXX
#include "xiview.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::util::XNumberFormatsSupplier;

using ::com::sun::star::chart::XChartDocument;
using ::com::sun::star::chart::XDiagram;
using ::com::sun::star::chart::X3DDisplay;
using ::com::sun::star::chart::XStatisticDisplay;
using ::com::sun::star::chart::XAxisXSupplier;
using ::com::sun::star::chart::XAxisYSupplier;
using ::com::sun::star::chart::XTwoAxisYSupplier;
using ::com::sun::star::chart::XAxisZSupplier;

typedef ::com::sun::star::util::Date ApiDate;

// Helpers ====================================================================

namespace {

XclImpStream& operator>>( XclImpStream& rStrm, XclChRectangle& rRect )
{
    return rStrm >> rRect.mnX >> rRect.mnY >> rRect.mnWidth >> rRect.mnHeight;
}

} // namespace

// Common =====================================================================

/** Stores global data needed in various classes of the Chart import filter. */
struct XclImpChRootData
{
    typedef ScfRef< XclChObjectTable >      XclChObjectTableRef;

    Reference< XChartDocument > mxChartDoc;     /// The destination chart document.
    XclChObjectTableRef mxLineDashTable;        /// Container for line dash styles.
    XclChObjectTableRef mxGradientTable;        /// Container for gradient fill styles.
    XclChObjectTableRef mxBitmapTable;          /// Container for bitmap fill styles.
    XclImpChChart*      mpChartData;            /// Pointer to the chart data object.

    explicit            XclImpChRootData( XclImpChChart* pChartData );
};

XclImpChRootData::XclImpChRootData( XclImpChChart* pChartData ) :
    mpChartData( pChartData )
{
}

// ----------------------------------------------------------------------------

XclImpChRoot::XclImpChRoot( const XclImpRoot& rRoot, XclImpChChart* pChartData ) :
    XclImpRoot( rRoot ),
    mxChData( new XclImpChRootData( pChartData ) )
{
}

XclImpChRoot::~XclImpChRoot()
{
}

XclImpChChart& XclImpChRoot::GetChartData() const
{
    return *mxChData->mpChartData;
}

Color XclImpChRoot::GetLineAutoColor() const
{
    return GetPalette().GetColor( EXC_COLOR_CHBORDERAUTO );
}

Color XclImpChRoot::GetFillAutoColor() const
{
    return GetPalette().GetColor( EXC_COLOR_CHWINDOWBACK );
}

Color XclImpChRoot::GetFontAutoColor() const
{
    return GetPalette().GetColor( EXC_COLOR_CHWINDOWTEXT );
}

Color XclImpChRoot::GetSeriesLineAutoColor( sal_uInt16 nFormatIdx ) const
{
    static const sal_uInt16 spnLineColors[] =
    {
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62,  8,
         9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 63
    };
    return GetPalette().GetColor( spnLineColors[ nFormatIdx % STATIC_TABLE_SIZE( spnLineColors ) ] );
}

Color XclImpChRoot::GetSeriesFillAutoColor( sal_uInt16 nFormatIdx ) const
{
    static const sal_uInt16 spnFillColors[] =
    {
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63,
         8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23
    };
    static const sal_uInt16 spnTrans[] = { 0x0000, 0x4000, 0x2000, 0x6000, 0x7000 };

    sal_uInt16 nColorIdx = spnFillColors[ nFormatIdx % STATIC_TABLE_SIZE( spnFillColors ) ];
    Color aColor = GetPalette().GetColor( nColorIdx );
    int nCycle = nFormatIdx / STATIC_TABLE_SIZE( spnFillColors );
    sal_uInt16 nTrans = spnTrans[ nCycle % STATIC_TABLE_SIZE( spnTrans ) ];
    return ScfTools::GetMixedColor( aColor, GetFillAutoColor(), nTrans );
}

void XclImpChRoot::InitConversion( Reference< XChartDocument > xChartDoc ) const
{
    // remember chart document reference
    mxChData->mxChartDoc = xChartDoc;

    // create object tables
    Reference< XMultiServiceFactory > xFactory( xChartDoc, UNO_QUERY );
    mxChData->mxLineDashTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_DASHTABLE, CREATE_OUSTRING( "Excel line dash " ) ) );
    mxChData->mxGradientTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_GRADIENTTABLE, CREATE_OUSTRING( "Excel gradient " ) ) );
    mxChData->mxBitmapTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_BITMAPTABLE, CREATE_OUSTRING( "Excel bitmap " ) ) );

    // lock the model to suppress any internal updates
    Reference< XModel > xModel( xChartDoc, UNO_QUERY );
    if( xModel.is() )
        xModel->lockControllers();
}

void XclImpChRoot::FinishConversion( ScfProgressBar& rProgress ) const
{
    // unlock the model
    Reference< XModel > xModel( mxChData->mxChartDoc, UNO_QUERY );
    if( xModel.is() )
        xModel->unlockControllers();
    rProgress.Progress( EXC_CHART_PROGRESS_SIZE );
    // forget chart document reference
    mxChData->mxChartDoc.clear();
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

void XclImpChRoot::ConvertEscherForrmat( ScfPropertySet& rPropSet,
        const SfxItemSet& rItemSet, const XclChPicFormat& rPicFmt ) const
{
    GetChartPropSetHelper().WriteEscherProperties( rPropSet,
        *mxChData->mxGradientTable, *mxChData->mxBitmapTable, rItemSet, rPicFmt );
}

void XclImpChRoot::ConvertFont( ScfPropertySet& rPropSet,
        sal_uInt16 nFontIdx, const Color* pFontColor ) const
{
    GetFontBuffer().WriteFontProperties( rPropSet, EXC_FONTPROPSET_CHART, nFontIdx, pFontColor );
}

// ----------------------------------------------------------------------------

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
    DBG_ASSERT( rStrm.GetRecId() == EXC_ID_CHBEGIN, "XclImpChGroupBase::SkipBlock - no CHBEGIN record" );
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

// Formatting =================================================================

void XclImpChFramePos::ReadChFramePos( XclImpStream& rStrm )
{
    rStrm >> maData.mnObjType >> maData.mnSizeMode >> maData.maRect;
}

// ----------------------------------------------------------------------------

XclImpChLineFormat::XclImpChLineFormat( sal_uInt16 nLinkTarget )
{
    switch( nLinkTarget )
    {
        case EXC_CHOBJLINK_BACKGROUND:
        case EXC_CHOBJLINK_DIAGRAM:
            ::set_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO, false );
            maData.mnPattern = EXC_CHLINEFORMAT_NONE;
        break;
    }
}

void XclImpChLineFormat::ReadChLineFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maColor >> maData.mnPattern >> maData.mnWeight >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        rStrm >> maData.mnColorIdx;
        // #116397# BIFF8: index into palette used instead of RGB data
        maData.maColor = rRoot.GetPalette().GetColor( maData.mnColorIdx );
    }
}

void XclImpChLineFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nAutoWeight ) const
{
    if( IsAuto() )
    {
        XclChLineFormat aLineFmt;
        aLineFmt.maColor = rRoot.GetLineAutoColor();
        aLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
        aLineFmt.mnWeight = nAutoWeight;
        rRoot.ConvertLineFormat( rPropSet, aLineFmt, EXC_CHPROPMODE_COMMON );
    }
    else
    {
        rRoot.ConvertLineFormat( rPropSet, maData, EXC_CHPROPMODE_COMMON );
    }
}

void XclImpChLineFormat::ConvertSeries( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, bool bLinear2dType ) const
{
    // property names depend on chart type
    XclChPropertyMode ePropMode = bLinear2dType ? EXC_CHPROPMODE_LINEARSERIES : EXC_CHPROPMODE_FILLEDSERIES;

    if( IsAuto() )
    {
        XclChLineFormat aLineFmt;
        aLineFmt.maColor = bLinear2dType ?
            rRoot.GetSeriesLineAutoColor( nFormatIdx ) : rRoot.GetLineAutoColor();
        aLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
        aLineFmt.mnWeight = EXC_CHLINEFORMAT_SINGLE;
        rRoot.ConvertLineFormat( rPropSet, aLineFmt, ePropMode );
    }
    else
    {
        rRoot.ConvertLineFormat( rPropSet, maData, ePropMode );
    }
}

// ----------------------------------------------------------------------------

XclImpChAreaFormat::XclImpChAreaFormat( sal_uInt16 nLinkTarget )
{
    switch( nLinkTarget )
    {
        case EXC_CHOBJLINK_BACKGROUND:
        case EXC_CHOBJLINK_DIAGRAM:
            ::set_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO, false );
            maData.mnPattern = EXC_CHAREAFORMAT_NONE;
        break;
    }
}

void XclImpChAreaFormat::ReadChAreaFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maForeColor >> maData.maBackColor >> maData.mnPattern >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        rStrm >> maData.mnForeColorIdx >> maData.mnBackColorIdx;
        // #116397# BIFF8: index into palette used instead of RGB data
        const XclImpPalette& rPal = rRoot.GetPalette();
        maData.maForeColor = rPal.GetColor( maData.mnForeColorIdx );
        maData.maBackColor = rPal.GetColor( maData.mnBackColorIdx );
    }
}

void XclImpChAreaFormat::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    if( IsAuto() )
    {
        XclChAreaFormat aAreaFmt;
        aAreaFmt.maForeColor = rRoot.GetFillAutoColor();
        aAreaFmt.mnPattern = EXC_CHAREAFORMAT_SOLID;
        rRoot.ConvertAreaFormat( rPropSet, aAreaFmt, EXC_CHPROPMODE_COMMON );
    }
    else
    {
        rRoot.ConvertAreaFormat( rPropSet, maData, EXC_CHPROPMODE_COMMON );
    }
}

void XclImpChAreaFormat::ConvertSeries( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    if( IsAuto() )
    {
        XclChAreaFormat aAreaFmt;
        aAreaFmt.maForeColor = rRoot.GetSeriesFillAutoColor( nFormatIdx );
        aAreaFmt.mnPattern = EXC_CHAREAFORMAT_SOLID;
        rRoot.ConvertAreaFormat( rPropSet, aAreaFmt, EXC_CHPROPMODE_FILLEDSERIES );
    }
    else
    {
        rRoot.ConvertAreaFormat( rPropSet, maData, EXC_CHPROPMODE_FILLEDSERIES );
    }
}

// ----------------------------------------------------------------------------

XclImpChEscherFormat::XclImpChEscherFormat( const XclImpRoot& rRoot ) :
    maItemSet( rRoot.GetDoc().GetDrawLayer()->GetItemPool() )
{
}

void XclImpChEscherFormat::ReadHeaderRecord( XclImpStream& rStrm )
{
    const XclImpRoot& rRoot = rStrm.GetRoot();
    // use temporary object manager, not the one of the root data
    XclImpObjectManager aObjManager( rRoot );
    XclImpEscherPropSet aPropSet( aObjManager.GetDffManager() );
    // read from stream - CHESCHERFORMAT uses own ID for record continuation
    rStrm.ResetRecord( true, rStrm.GetRecId() );
    rStrm >> aPropSet;
    // get the data
    aPropSet.FillToItemSet( maItemSet );
    // get bitmap mode from Escher item set
    sal_uInt32 nType = aPropSet.GetPropertyValue( DFF_Prop_fillType, mso_fillSolid );
    maPicFmt.mnBmpMode = (nType == mso_fillPicture) ? EXC_CHPICFORMAT_STRETCH : EXC_CHPICFORMAT_STACK;
}

void XclImpChEscherFormat::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHPICFORMAT:
            rStrm >> maPicFmt.mnBmpMode >> maPicFmt.mnFormat >> maPicFmt.mnFlags >> maPicFmt.mfScale;
        break;
    }
}

void XclImpChEscherFormat::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    rRoot.ConvertEscherForrmat( rPropSet, maItemSet, maPicFmt );
}

// ----------------------------------------------------------------------------

XclImpChFrameBase::XclImpChFrameBase( sal_uInt16 nLinkTarget )
{
    switch( nLinkTarget )
    {
        case EXC_CHOBJLINK_BACKGROUND:
        case EXC_CHOBJLINK_DIAGRAM:
        case EXC_CHOBJLINK_LEGEND:
            // legend, plot frame, and chart page need existing objects due to different defaults
            mxLineFmt.reset( new XclImpChLineFormat( nLinkTarget ) );
            mxAreaFmt.reset( new XclImpChAreaFormat( nLinkTarget ) );
        break;
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
            // pass unique object name to escher format object
            mxEscherFmt.reset( new XclImpChEscherFormat( rStrm.GetRoot() ) );
            mxEscherFmt->ReadRecordGroup( rStrm );
        break;
    }
}

void XclImpChFrameBase::ConvertFrameBase(
        const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    // line format
    if( mxLineFmt.is() )
        mxLineFmt->Convert( rRoot, rPropSet );
    // area format - CHESCHERFORMAT overrides CHAREAFORMAT
    if( mxEscherFmt.is() )
        mxEscherFmt->Convert( rRoot, rPropSet );
    else if( mxAreaFmt.is() )
        mxAreaFmt->Convert( rRoot, rPropSet );
}

void XclImpChFrameBase::ConvertSeriesFrameBase( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, bool bLinear2dType ) const
{
    // line format
    if( mxLineFmt.is() )
        mxLineFmt->ConvertSeries( rRoot, rPropSet, nFormatIdx, bLinear2dType );
    // area format
    if( !bLinear2dType )
    {
        // CHESCHERFORMAT overrides CHAREAFORMAT
        if( mxEscherFmt.is() )
            mxEscherFmt->Convert( rRoot, rPropSet );
        else if( mxAreaFmt.is() )
            mxAreaFmt->ConvertSeries( rRoot, rPropSet, nFormatIdx );
    }
}

// ----------------------------------------------------------------------------

XclImpChFrame::XclImpChFrame( sal_uInt16 nLinkTarget ) :
    XclImpChFrameBase( nLinkTarget )
{
}

void XclImpChFrame::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.mnFormat >> maData.mnFlags;
}

void XclImpChFrame::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    ConvertFrameBase( rRoot, rPropSet );
}

// ----------------------------------------------------------------------------

void XclImpChMarkerFormat::ReadChMarkerFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maLineColor >> maData.maFillColor >> maData.mnMarkerType >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        rStrm >> maData.mnLineColorIdx >> maData.mnFillColorIdx >> maData.mnMarkerSize;
        // #116397# BIFF8: index into palette used instead of RGB data
        const XclImpPalette& rPal = rRoot.GetPalette();
        maData.maLineColor = rPal.GetColor( maData.mnLineColorIdx );
        maData.maFillColor = rPal.GetColor( maData.mnFillColorIdx );
    }
}

void XclImpChMarkerFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, sal_uInt16 nLineWeight ) const
{
    if( IsAuto() )
    {
        XclChMarkerFormat aMarkerFmt;
        // line and fill color of the symbol are equal to series line color
        //! TODO: Excel sets no fill color for specific symbols (e.g. cross)
        aMarkerFmt.maLineColor = aMarkerFmt.maFillColor = rRoot.GetSeriesLineAutoColor( nFormatIdx );
        switch( nLineWeight )
        {
            case EXC_CHLINEFORMAT_HAIR:     aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_HAIRSIZE;      break;
            case EXC_CHLINEFORMAT_SINGLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_SINGLESIZE;    break;
            case EXC_CHLINEFORMAT_DOUBLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_DOUBLESIZE;    break;
            case EXC_CHLINEFORMAT_TRIPLE:   aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_TRIPLESIZE;    break;
            default:                        aMarkerFmt.mnMarkerSize = EXC_CHMARKERFORMAT_SINGLESIZE;
        }
        switch( nFormatIdx % EXC_CHMARKERFORMAT_SYMBOLCOUNT )
        {
            case 0:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_DIAMOND;     break;
            case 1:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_SQUARE;      break;
            case 2:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_TRIANGLE;    break;
            case 3:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_CROSS;       break;
            case 4:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_STAR;        break;
            case 5:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_CIRCLE;      break;
            case 6:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_PLUS;        break;
            case 7:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_DOWJ;        break;
            case 8:     aMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_STDDEV;      break;
            default:    DBG_ERRORFILE( "XclImpChMarkerFormat::Convert - missing symbol type" );
        }
        aMarkerFmt.mnFlags = 0;
        rRoot.GetChartPropSetHelper().WriteMarkerProperties( rPropSet, aMarkerFmt );
    }
    else
    {
        rRoot.GetChartPropSetHelper().WriteMarkerProperties( rPropSet, maData );
    }
}

void XclImpChMarkerFormat::ConvertColor( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    Color aLineColor = IsAuto() ? rRoot.GetSeriesLineAutoColor( nFormatIdx ) : maData.maFillColor;
    rPropSet.SetColorProperty( EXC_CHPROP_LINECOLOR, aLineColor );
}

// ----------------------------------------------------------------------------

XclImpChPieFormat::XclImpChPieFormat() :
    mnPieDist( 0 )
{
}

void XclImpChPieFormat::ReadChPieFormat( XclImpStream& rStrm )
{
    rStrm >> mnPieDist;
}

void XclImpChPieFormat::Convert( ScfPropertySet& rPropSet ) const
{
    sal_Int32 nApiDist = ::std::min< sal_Int32 >(
        sal::static_int_cast< sal_Int32 >(mnPieDist), 100 );
    rPropSet.SetProperty( EXC_CHPROP_SEGMENTOFFSET, nApiDist );
}

// ----------------------------------------------------------------------------

XclImpChSeriesFormat::XclImpChSeriesFormat() :
    mnFlags( 0 )
{
}

void XclImpChSeriesFormat::ReadChSeriesFormat( XclImpStream& rStrm )
{
    rStrm >> mnFlags;
}

// ----------------------------------------------------------------------------

void XclImpCh3dDataFormat::ReadCh3dDataFormat( XclImpStream& rStrm )
{
    rStrm >> maData.mnBase >> maData.mnTop;
}

void XclImpCh3dDataFormat::Convert( ScfPropertySet& rPropSet ) const
{
    using namespace ::com::sun::star::chart::ChartSolidType;
    sal_Int32 nApiType = (maData.mnBase == EXC_CH3DDATAFORMAT_RECT) ?
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? RECTANGULAR_SOLID : PYRAMID) :
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? CYLINDER : CONE);
    rPropSet.SetProperty( EXC_CHPROP_SOLIDTYPE, nApiType );
}

// Text =======================================================================

XclImpChFont::XclImpChFont() :
    mnFontIdx( EXC_FONT_NOTFOUND )
{
}

void XclImpChFont::ReadChFont( XclImpStream& rStrm )
{
    rStrm >> mnFontIdx;
}

// ----------------------------------------------------------------------------

XclImpChFormat::XclImpChFormat() :
    mnNumFmtIdx( EXC_FORMAT_NOTFOUND )
{
}

void XclImpChFormat::ReadChFormat( XclImpStream& rStrm )
{
    rStrm >> mnNumFmtIdx;
}

void XclImpChFormat::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    ULONG nScNumFmt = rRoot.GetNumFmtBuffer().GetScFormat( mnNumFmtIdx );
    if( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        rPropSet.SetBoolProperty( EXC_CHPROP_LINKNUMFMT, false );
        rPropSet.SetProperty( EXC_CHPROP_NUMFMT, static_cast< sal_Int32 >( nScNumFmt ) );
    }
}

// ----------------------------------------------------------------------------

void XclImpChString::ReadChString( XclImpStream& rStrm )
{
    rStrm.Ignore( 2 );
    maData.Read( rStrm, EXC_STR_8BITLENGTH | EXC_STR_KEEPFORMATS );
}

void XclImpChString::ReadChFormatRuns( XclImpStream& rStrm )
{
    rStrm.GetRoot().GetTracer().TraceChartTextFormatting();
    maData.ReadFormats( rStrm );
}

void XclImpChString::Convert( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetStringProperty( EXC_CHPROP_STRING, maData.GetText() );
}

// ----------------------------------------------------------------------------

XclImpChFontBase::~XclImpChFontBase()
{
}

void XclImpChFontBase::ConvertFontBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    Color aFontColor = GetFontColor();
    rRoot.ConvertFont( rPropSet, GetFontIndex(), &aFontColor );
}

// ----------------------------------------------------------------------------

XclImpChRotationBase::~XclImpChRotationBase()
{
}

// ----------------------------------------------------------------------------

XclImpChText::XclImpChText( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChText::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm   >> maData.mnHAlign
            >> maData.mnVAlign
            >> maData.mnBackMode
            >> maData.maTextColor
            >> maData.maRect
            >> maData.mnFlags;

    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm >> maData.mnTextColorIdx >> maData.mnPlacement >> maData.mnRotation;
        // #116397# BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( maData.mnTextColorIdx );
    }
}

void XclImpChText::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFONT:
            mxFont.reset( new XclImpChFont );
            mxFont->ReadChFont( rStrm );
        break;
        case EXC_ID_CHFORMATRUNS:
            if( !mxString )
                mxString.reset( new XclImpChString );
            mxString->ReadChFormatRuns( rStrm );
        break;
        case EXC_ID_CHSTRING:
            if( !mxString )
               mxString.reset( new XclImpChString );
            mxString->ReadChString( rStrm );
        break;
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame );
            mxFrame->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHOBJECTLINK:
            rStrm >> maObjLink.mnTarget >> maObjLink.maPointPos.mnSeriesIdx >> maObjLink.maPointPos.mnPointIdx;
        break;
    }
}

sal_uInt16 XclImpChText::GetFontIndex() const
{
    return mxFont.is() ? mxFont->GetFontIndex() : EXC_FONT_NOTFOUND;
}

Color XclImpChText::GetFontColor() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTEXT_AUTOCOLOR ) ? GetFontAutoColor() : maData.maTextColor;
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

void XclImpChText::ConvertFrame( ScfPropertySet& rPropSet ) const
{
    if( mxFrame.is() )
        mxFrame->Convert( GetChRoot(), rPropSet );
}

void XclImpChText::ConvertRotation( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetProperty( EXC_CHPROP_TEXTROTATION, XclTools::GetScRotation( maData.mnRotation, 27000 ) );
}

void XclImpChText::ConvertString( ScfPropertySet& rPropSet ) const
{
    if( mxString.is() )
        mxString->Convert( rPropSet );
}

void XclImpChText::ConvertDataLabel( ScfPropertySet& rPropSet, sal_uInt16 nTypeId ) const
{
    const sal_uInt16 EXC_CHTEXT_SHOWANYCATEG   = EXC_CHTEXT_SHOWCATEGPERC | EXC_CHTEXT_SHOWCATEG;
    const sal_uInt16 EXC_CHTEXT_SHOWANYVALUE   = EXC_CHTEXT_SHOWVALUE;
    const sal_uInt16 EXC_CHTEXT_SHOWANYPERCENT = EXC_CHTEXT_SHOWPERCENT | EXC_CHTEXT_SHOWCATEGPERC;

    bool bIsPie       = XclChartHelper::IsPieChartType( nTypeId );
    bool bShowNone    = ::get_flag( maData.mnFlags, EXC_CHTEXT_DELETED );
    bool bShowCateg   = !bShowNone && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYCATEG );
    bool bShowPercent = !bShowNone && bIsPie && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYPERCENT );
    bool bShowValue   = !bShowNone && !bShowPercent && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYVALUE );
    bool bShowAny     = bShowValue || bShowPercent || bShowCateg;
    bool bShowSymbol  = bShowAny && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWSYMBOL );

    // type of attached label
    using namespace ::com::sun::star::chart::ChartDataCaption;
    sal_Int32 nApiCaption = NONE;
    ::set_flag( nApiCaption, VALUE,   bShowValue );
    ::set_flag( nApiCaption, PERCENT, bShowPercent );
    ::set_flag( nApiCaption, TEXT,    bShowCateg );
    ::set_flag( nApiCaption, SYMBOL,  bShowSymbol );
    rPropSet.SetProperty( EXC_CHPROP_DATACAPTION, nApiCaption );

    // text properties of attached label
    if( bShowAny )
        ConvertFont( rPropSet );
}

void XclImpChText::ConvertTitle( ScfPropertySet& rPropSet ) const
{
    ConvertFont( rPropSet );
    ConvertRotation( rPropSet );
    // frame must be converted before setting the title string, otherwise it will be deleted
    ConvertFrame( rPropSet );
    ConvertString( rPropSet );
}

namespace {

void lclUpdateText( XclImpChTextRef& rxText, XclImpChTextRef xDefText )
{
    if( rxText.is() )
        rxText->UpdateText( xDefText.get() );
    else
        rxText = xDefText;
}

void lclFinalizeTitle( XclImpChTextRef& rxTitle, XclImpChTextRef xDefText )
{
    /*  Do not update a title, if it is not visible (if rxTitle is null).
        Existing reference indicates enabled title. */
    if( rxTitle.is() )
    {
        if( rxTitle->HasString() )
            rxTitle->UpdateText( xDefText.get() );
        else
            rxTitle.reset();
    }
}

} // namespace

// Linked source data =========================================================

namespace {

XclChOrientation lclGetOrientation( const ScRange& rScRange )
{
    SCCOL nWidth = rScRange.aEnd.Col() - rScRange.aStart.Col();
    SCROW nHeight = rScRange.aEnd.Row() - rScRange.aStart.Row();
    SCTAB nDepth = rScRange.aEnd.Tab() - rScRange.aStart.Tab();
    return (nDepth == 0) ?
                ((nWidth == 0) ?
                    ((nHeight == 0) ? EXC_CHORIENT_SINGLE : EXC_CHORIENT_VERTICAL) :
                    ((nHeight == 0) ? EXC_CHORIENT_HORIZONTAL : EXC_CHORIENT_COMPLEX)) :
                EXC_CHORIENT_COMPLEX;
}

inline bool lclIsVerHorOrient( XclChOrientation eOrient )
{
    return (eOrient == EXC_CHORIENT_VERTICAL) || (eOrient == EXC_CHORIENT_HORIZONTAL);
}

} // namespace

// ----------------------------------------------------------------------------

XclImpChSourceLink::XclImpChSourceLink( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    meOrient( EXC_CHORIENT_EMPTY ),
    mnScCol( -1 ),
    mnScRow( -1 ),
    mnCells( 0 )
{
}

void XclImpChSourceLink::ReadChSourceLink( XclImpStream& rStrm )
{
    rStrm   >> maData.mnDestType
            >> maData.mnLinkType
            >> maData.mnFlags
            >> maData.mnNumFmtIdx;

    // read range list formula (linked data in Calc document)
    maIntervals.clear();
    meOrient = EXC_CHORIENT_EMPTY;
    maMainPos.mnScTab = -1;
    maMainPos.mnScPos = -1;
    mnScCol = -1;
    mnScRow = -1;
    mnCells = 0;

    if( GetLinkType() == EXC_CHSRCLINK_WORKSHEET )
    {
        // read token array
        XclTokenArray aXclTokArr;
        rStrm >> aXclTokArr;
        // convert token array to range list
        ScRangeList aScRanges;
        GetFormulaCompiler().CreateRangeList( aScRanges, EXC_FMLATYPE_CHART, aXclTokArr, rStrm );
        // process range list
        CalcOrientation( aScRanges );
    }

    // try to read a following CHSTRING record
    if( (rStrm.GetNextRecId() == EXC_ID_CHSTRING) && rStrm.StartNextRecord() )
    {
        mxString.reset( new XclImpChString );
        mxString->ReadChString( rStrm );
    }
}

void XclImpChSourceLink::SetFinalOrientation( XclChOrientation eOrient )
{
    DBG_ASSERT( meOrient == EXC_CHORIENT_SINGLE, "XclImpChSourceLink::SetFinalOrientation - no single source link" );
    if( meOrient == EXC_CHORIENT_SINGLE ) switch( eOrient )
    {
        case EXC_CHORIENT_VERTICAL:
            meOrient = eOrient;
            maMainPos.mnScPos = mnScCol;
            AppendInterval( mnScRow, mnScRow );
        break;
        case EXC_CHORIENT_HORIZONTAL:
            meOrient = eOrient;
            maMainPos.mnScPos = mnScRow;
            AppendInterval( mnScCol, mnScCol );
        break;
        default:
            DBG_ERRORFILE( "XclImpChSourceLink::SetFinalOrientation - invalid final orientation" );
    }
}

void XclImpChSourceLink::InsertTitlePos( SCCOLROW nScPos )
{
    DBG_ASSERT( lclIsVerHorOrient( meOrient ), "XclImpChSourceLink::InsertTitlePos - invalid orientation" );
    if( !maIntervals.empty() && (nScPos < maIntervals.front().mnScPos1) )
    {
        // extend first interval if possible
        if( nScPos + 1 == maIntervals.front().mnScPos1 )
            maIntervals.front().mnScPos1 = nScPos;
        else
            maIntervals.insert( maIntervals.begin(), XclImpChInterval( nScPos, nScPos ) );
    }
}

bool XclImpChSourceLink::HasValidOrientation() const
{
    return lclIsVerHorOrient( meOrient ) || (meOrient == EXC_CHORIENT_SINGLE);
}

bool XclImpChSourceLink::IsEqualLink( const XclImpChSourceLink& rSrcLink ) const
{
    bool bEqual = (meOrient == rSrcLink.meOrient) && (maMainPos.mnScTab == rSrcLink.maMainPos.mnScTab);
    if( bEqual ) switch( meOrient )
    {
        case EXC_CHORIENT_SINGLE:
            bEqual = (mnScCol == rSrcLink.mnScCol) && (mnScRow == rSrcLink.mnScRow);
        break;
        case EXC_CHORIENT_VERTICAL:
        case EXC_CHORIENT_HORIZONTAL:
            bEqual = (maIntervals == rSrcLink.maIntervals);
        break;
        default:
            bEqual = meOrient == EXC_CHORIENT_EMPTY;
    }
    return bEqual;
}

bool XclImpChSourceLink::IsValidValueLink( const XclImpChSourceLink& rValueLink ) const
{
    bool bValid = false;
    if( meOrient == rValueLink.meOrient ) switch( meOrient )
    {
        case EXC_CHORIENT_SINGLE:
            bValid = (rValueLink.mnScRow == mnScRow) || (rValueLink.mnScCol == mnScCol);
        break;
        case EXC_CHORIENT_VERTICAL:
        case EXC_CHORIENT_HORIZONTAL:
            bValid = rValueLink.maIntervals == maIntervals;
        break;
    }
    return bValid;
}

bool XclImpChSourceLink::IsValidCategLink( const XclImpChSourceLink& rCategLink ) const
{
    bool bValid = false;
    if( meOrient == rCategLink.meOrient ) switch( meOrient )
    {
        case EXC_CHORIENT_SINGLE:
            bValid =
                ((XclImpChMainPos( rCategLink.maMainPos.mnScTab, rCategLink.mnScCol ) < XclImpChMainPos( maMainPos.mnScTab, mnScCol )) && (rCategLink.mnScRow == mnScRow)) ||
                ((XclImpChMainPos( rCategLink.maMainPos.mnScTab, rCategLink.mnScRow ) < XclImpChMainPos( maMainPos.mnScTab, mnScRow )) && (rCategLink.mnScCol == mnScCol));
        break;
        case EXC_CHORIENT_VERTICAL:
        case EXC_CHORIENT_HORIZONTAL:
            bValid = (rCategLink.maMainPos < maMainPos) && (rCategLink.maIntervals == maIntervals);
        break;
    }
    return bValid;
}

bool XclImpChSourceLink::IsValidTitleLink( const XclImpChSourceLink& rTitleLink ) const
{
    bool bValid = false;
    if( rTitleLink.GetOrientation() != EXC_CHORIENT_EMPTY )
    {
        /*  Always use top-left cell of title, regardless of remaining linked
            cells. Title cell must be located on the same sheet. */
        bool bValidTab = rTitleLink.maMainPos.mnScTab == maMainPos.mnScTab;
        bool bValidVer = bValidTab && (rTitleLink.mnScRow < mnScRow) && (rTitleLink.mnScCol == mnScCol);
        bool bValidHor = bValidTab && (rTitleLink.mnScCol < mnScCol) && (rTitleLink.mnScRow == mnScRow);
        switch( meOrient )
        {
            case EXC_CHORIENT_SINGLE:       bValid = bValidVer || bValidHor;    break;
            case EXC_CHORIENT_VERTICAL:     bValid = bValidVer;                 break;
            case EXC_CHORIENT_HORIZONTAL:   bValid = bValidHor;                 break;
        }
    }
    return bValid;
}

void XclImpChSourceLink::JoinRanges( ScRangeList& rScRanges ) const
{
    switch( meOrient )
    {
        case EXC_CHORIENT_SINGLE:
            rScRanges.Join( ScRange( mnScCol, mnScRow, maMainPos.mnScTab ) );
        break;
        case EXC_CHORIENT_VERTICAL:
        {
            SCCOL nScCol = static_cast< SCCOL >( maMainPos.mnScPos );
            for( XclImpChIntervalVec::const_iterator aIt = maIntervals.begin(), aEnd = maIntervals.end(); aIt != aEnd; ++aIt )
                rScRanges.Join( ScRange(
                    nScCol, static_cast< SCROW >( aIt->mnScPos1 ), maMainPos.mnScTab,
                    nScCol, static_cast< SCROW >( aIt->mnScPos2 ), maMainPos.mnScTab ) );
        }
        break;
        case EXC_CHORIENT_HORIZONTAL:
        {
            SCROW nScRow = static_cast< SCROW >( maMainPos.mnScPos );
            for( XclImpChIntervalVec::const_iterator aIt = maIntervals.begin(), aEnd = maIntervals.end(); aIt != aEnd; ++aIt )
                rScRanges.Join( ScRange(
                    static_cast< SCCOL >( aIt->mnScPos1 ), nScRow, maMainPos.mnScTab,
                    static_cast< SCCOL >( aIt->mnScPos2 ), nScRow, maMainPos.mnScTab ) );
        }
        break;
    }
}

// private --------------------------------------------------------------------

void XclImpChSourceLink::CalcOrientation( const ScRangeList& rScRanges )
{
    ULONG nCount = rScRanges.Count();
    if( nCount > 0 )
    {
        // initialize values from first range
        const ScRange& rFirstScRange = *rScRanges.GetObject( 0 );
        meOrient = lclGetOrientation( rFirstScRange );
        mnScCol = rFirstScRange.aStart.Col();
        mnScRow = rFirstScRange.aStart.Row();
        // main position
        maMainPos.mnScTab = rFirstScRange.aStart.Tab();
        switch( meOrient )
        {
            case EXC_CHORIENT_VERTICAL:
                maMainPos.mnScPos = mnScCol;
                AppendInterval( mnScRow, rFirstScRange.aEnd.Row() );
            break;
            case EXC_CHORIENT_HORIZONTAL:
                maMainPos.mnScPos = mnScRow;
                AppendInterval( mnScCol, rFirstScRange.aEnd.Col() );
            break;
        }

        // update with following ranges
        for( ULONG nIdx = 1; (meOrient != EXC_CHORIENT_COMPLEX) && (nIdx < nCount); ++nIdx )
        {
            const ScRange& rScRange = *rScRanges.GetObject( nIdx );
            // additional ranges must be located on same sheet
            if( maMainPos.mnScTab == rScRange.aStart.Tab() )
            {
                SCCOL nScCol = rScRange.aStart.Col();
                SCROW nScRow = rScRange.aStart.Row();
                switch( lclGetOrientation( rScRange ) )
                {
                    case EXC_CHORIENT_SINGLE:
                        AppendSingleCell( nScCol, nScRow );
                    break;
                    case EXC_CHORIENT_VERTICAL:
                        AppendColumnRange( nScCol, nScRow, rScRange.aEnd.Row() );
                    break;
                    case EXC_CHORIENT_HORIZONTAL:
                        AppendRowRange( nScCol, rScRange.aEnd.Col(), nScRow );
                    break;
                    default:
                        meOrient = EXC_CHORIENT_COMPLEX;
                }
            }
            else
                meOrient = EXC_CHORIENT_COMPLEX;
        }
    }
}

void XclImpChSourceLink::AppendSingleCell( SCCOL nScCol, SCROW nScRow )
{
    if( (mnScCol == nScCol) && (mnScRow < nScRow) )
        AppendColumnRange( nScCol, nScRow, nScRow );
    else if( (mnScRow == nScRow) && (mnScCol < nScCol) )
        AppendRowRange( nScCol, nScCol, nScRow );
    else
        meOrient = EXC_CHORIENT_COMPLEX;
}

void XclImpChSourceLink::AppendColumnRange( SCCOL nScCol, SCROW nScRow1, SCROW nScRow2 )
{
    switch( meOrient )
    {
        // Append column to single cell: Column indexes must be equal.
        case EXC_CHORIENT_SINGLE:
            if( (mnScCol == nScCol) && (mnScRow < nScRow1) )
                UpdateOrientation( EXC_CHORIENT_VERTICAL, nScRow1, nScRow2 );
            else
                meOrient = EXC_CHORIENT_COMPLEX;
        break;
        // Append column to column: Column indexes must be equal.
        case EXC_CHORIENT_VERTICAL:
            if( (maMainPos.mnScPos == nScCol) && (maIntervals.back().mnScPos2 < nScRow1) )
                AppendInterval( nScRow1, nScRow2 );
            else
                meOrient = EXC_CHORIENT_COMPLEX;
        break;
        // Append column to row: Give up.
        case EXC_CHORIENT_HORIZONTAL:
            meOrient = EXC_CHORIENT_COMPLEX;
        break;
    }
}

void XclImpChSourceLink::AppendRowRange( SCCOL nScCol1, SCCOL nScCol2, SCROW nScRow )
{
    switch( meOrient )
    {
        // Append row to single cell: Row indexes must be equal.
        case EXC_CHORIENT_SINGLE:
            if( (mnScRow == nScRow) && (mnScCol < nScCol1) )
                UpdateOrientation( EXC_CHORIENT_HORIZONTAL, nScCol1, nScCol2 );
            else
                meOrient = EXC_CHORIENT_COMPLEX;
        break;
        // Append row to column: Give up.
        case EXC_CHORIENT_VERTICAL:
            meOrient = EXC_CHORIENT_COMPLEX;
        break;
        // Append row to row: Row indexes must be equal.
        case EXC_CHORIENT_HORIZONTAL:
            if( (maMainPos.mnScPos == nScRow) && (maIntervals.back().mnScPos2 < nScCol1) )
                AppendInterval( nScCol1, nScCol2 );
            else
                meOrient = EXC_CHORIENT_COMPLEX;
        break;
    }
}

void XclImpChSourceLink::UpdateOrientation( XclChOrientation eOrient, SCCOLROW nScPos1, SCCOLROW nScPos2 )
{
    SetFinalOrientation( eOrient );
    AppendInterval( nScPos1, nScPos2 );
}

void XclImpChSourceLink::AppendInterval( SCCOLROW nScPos1, SCCOLROW nScPos2 )
{
    DBG_ASSERT( nScPos1 <= nScPos2, "XclImpChSourceLink::AppendInterval - invalid interval" );
    if( maIntervals.empty() )
    {
        // empty interval list: insert the new interval
        maIntervals.push_back( XclImpChInterval( nScPos1, nScPos2 ) );
    }
    else if( maIntervals.back().mnScPos2 < nScPos1 )
    {
        // extend last interval if possible
        if( maIntervals.back().mnScPos2 + 1 == nScPos1 )
            maIntervals.back().mnScPos2 = nScPos2;
        else
            maIntervals.push_back( XclImpChInterval( nScPos1, nScPos2 ) );
    }
    else
        DBG_ERRORFILE( "XclImpChSourceLink::AppendInterval - invalid interval" );

    // count number of cells
    mnCells += static_cast< sal_uInt16 >( nScPos2 - nScPos1 + 1 );
}

// ----------------------------------------------------------------------------

XclImpChAttachedLabel::XclImpChAttachedLabel( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    mnFlags( 0 )
{
}

void XclImpChAttachedLabel::ReadChAttachedLabel( XclImpStream& rStrm )
{
    rStrm >> mnFlags;
}

XclImpChTextRef XclImpChAttachedLabel::CreateDataLabel( XclImpChTextRef xParent ) const
{
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYVALUE = EXC_CHATTLABEL_SHOWVALUE;
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYPERCENT = EXC_CHATTLABEL_SHOWPERCENT | EXC_CHATTLABEL_SHOWCATEGPERC;
    const sal_uInt16 EXC_CHATTLABEL_SHOWANYCATEG = EXC_CHATTLABEL_SHOWCATEG | EXC_CHATTLABEL_SHOWCATEGPERC;

    XclImpChTextRef xLabel( xParent.is() ? new XclImpChText( *xParent ) : new XclImpChText( GetChRoot() ) );
    xLabel->UpdateDataLabel(
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYCATEG ),
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYVALUE ),
        ::get_flag( mnFlags, EXC_CHATTLABEL_SHOWANYPERCENT ) );
    return xLabel;
}

// ----------------------------------------------------------------------------

XclImpChDataFormat::XclImpChDataFormat( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChDataFormat::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.maPointPos.mnPointIdx >> maData.maPointPos.mnSeriesIdx >> maData.mnFormatIdx >> maData.mnFlags;
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

void XclImpChDataFormat::SetPointPos( sal_uInt16 nSeriesIdx, sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx )
{
    maData.maPointPos.mnSeriesIdx = nSeriesIdx;
    maData.maPointPos.mnPointIdx = nPointIdx;
    maData.mnFormatIdx = nFormatIdx;
}

void XclImpChDataFormat::UpdateGroupFormat( const XclImpChChartGroup& rChartGroup )
{
    // remove formats not used for the current chart type
    RemoveUnusedFormats( rChartGroup );
}

void XclImpChDataFormat::UpdateSeriesFormat( const XclImpChChartGroup& rChartGroup )
{
    const XclImpChDataFormat* pGroupFmt = rChartGroup.GetGroupFormat().get();

    // update missing formats from passed chart group format
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
    RemoveUnusedFormats( rChartGroup );
    // update data label
    UpdateDataLabel( pGroupFmt );
}

void XclImpChDataFormat::UpdatePointFormat( const XclImpChChartGroup& rChartGroup, const XclImpChDataFormat* pSeriesFmt )
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
    // remove point line formats for linear 2D chart types, TODO: implement in OOChart
    if( rChartGroup.IsLinear2dChart() )
        mxLineFmt.reset();

    // remove formats not used for the current chart type
    RemoveUnusedFormats( rChartGroup );
    // update data label
    UpdateDataLabel( pSeriesFmt );
}

void XclImpChDataFormat::Convert( ScfPropertySet& rPropSet, const XclImpChChartGroup& rChartGroup ) const
{
    bool bLinear2dType = rChartGroup.IsLinear2dChart();
    // line and area format
    ConvertSeriesFrameBase( GetChRoot(), rPropSet, maData.mnFormatIdx, bLinear2dType );
    // other formatting
    if( mxMarkerFmt.is() )
        mxMarkerFmt->Convert( GetChRoot(), rPropSet, maData.mnFormatIdx, GetLineWeight() );
    if( mxPieFmt.is() )
        mxPieFmt->Convert( rPropSet );
    if( mx3dDataFmt.is() )
        mx3dDataFmt->Convert( rPropSet );
    if( mxLabel.is() )
        mxLabel->ConvertDataLabel( rPropSet, rChartGroup.GetChartTypeId() );

    /*  Special case: set marker color as line color, if series line is not
        visible. This makes the color visible in the marker area.
        TODO: remove this if OOChart supports own colors in markers. */
    if( bLinear2dType && !HasLine() && mxMarkerFmt.is() )
        mxMarkerFmt->ConvertColor( GetChRoot(), rPropSet, maData.mnFormatIdx );
}

void XclImpChDataFormat::ConvertVarPoint( ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    if( mxAreaFmt.is() && IsAutoArea() )
        mxAreaFmt->ConvertSeries( GetChRoot(), rPropSet, nFormatIdx );
}

void XclImpChDataFormat::ConvertLine( ScfPropertySet& rPropSet, sal_uInt16 nAutoWeight ) const
{
    if( mxLineFmt.is() )
        mxLineFmt->Convert( GetChRoot(), rPropSet, nAutoWeight );
}

void XclImpChDataFormat::RemoveUnusedFormats( const XclImpChChartGroup& rChartGroup )
{
    sal_uInt16 nTypeId = rChartGroup.GetChartTypeId();
    // point marker only in linear 2D charts
    if( !rChartGroup.IsLinear2dChart() )
        mxMarkerFmt.reset();
    // pie format only in pie charts (not supported in OOChart donut charts)
    if( nTypeId != EXC_ID_CHPIE )
        mxPieFmt.reset();
    // 3D format only in 3D bar charts
    if( !GetChartData().Is3dChart() || !XclChartHelper::IsBarChartType( nTypeId ) )
        mx3dDataFmt.reset();
}

void XclImpChDataFormat::UpdateDataLabel( const XclImpChDataFormat* pParentFmt )
{
    /*  CHTEXT groups linked to data labels override existing CHATTACHEDLABEL
        records. Only if there is a CHATTACHEDLABEL record without a CHTEXT
        group, the contents of the CHATTACHEDLABEL record are used. In this
        case a new CHTEXT group is created and filled with the settings from
        the CHATTACHEDLABEL record. */
    XclImpChTextRef xDefText;
    if( pParentFmt )
        xDefText = pParentFmt->GetDataLabel();
    if( !xDefText )
        xDefText = GetChartData().GetDefaultText( EXC_CHDEFTEXT_LABEL );
    if( mxLabel.is() )
        mxLabel->UpdateText( xDefText.get() );
    else if( mxAttLabel.is() )
        mxLabel = mxAttLabel->CreateDataLabel( xDefText );
}

// ----------------------------------------------------------------------------

XclImpChSerTrendLine::XclImpChSerTrendLine( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChSerTrendLine::ReadChSerTrendLine( XclImpStream& rStrm )
{
    rStrm   >> maData.mnLineType
            >> maData.mnOrder
            >> maData.mfIntercept
            >> maData.mnShowEquation
            >> maData.mnShowRSquared
            >> maData.mfForecastFor
            >> maData.mfForecastBack;
}

bool XclImpChSerTrendLine::IsValidType() const
{
    return
        ((maData.mnLineType == EXC_CHSERTREND_POLYNOMIAL) && (maData.mnOrder == 1)) ||
        (maData.mnLineType == EXC_CHSERTREND_EXPONENTIAL) ||
        (maData.mnLineType == EXC_CHSERTREND_LOGARITHMIC) ||
        (maData.mnLineType == EXC_CHSERTREND_POWER);
}

void XclImpChSerTrendLine::Convert( ScfPropertySet& rPropSet ) const
{
    namespace cssc = ::com::sun::star::chart;

    // trend line type
    cssc::ChartRegressionCurveType eApiType = cssc::ChartRegressionCurveType_NONE;
    switch( maData.mnLineType )
    {
        case EXC_CHSERTREND_POLYNOMIAL:
            // only linear trend lines are supported by OOChart
            if( maData.mnOrder == 1 )
                eApiType = cssc::ChartRegressionCurveType_LINEAR;
        break;
        case EXC_CHSERTREND_EXPONENTIAL:
            eApiType = cssc::ChartRegressionCurveType_EXPONENTIAL;
        break;
        case EXC_CHSERTREND_LOGARITHMIC:
            eApiType = cssc::ChartRegressionCurveType_LOGARITHM;
        break;
        case EXC_CHSERTREND_POWER:
            /*  WORKAROUND for #i47673#. Bug in OOChart API:
                com.sun.star.chart.ChartRegressionCurveType.POLYNOMIAL is
                mapped to "power" regression line type, and POWER is mapped to
                nothing. If the bug is fixed, set type POWER here. */
            eApiType = cssc::ChartRegressionCurveType_POLYNOMIAL;
        break;
    }
    rPropSet.SetProperty( EXC_CHPROP_REGRESSIONCURVES, eApiType );

    // trend line formatting
    if( mxLineFmt.is() )
    {
        Reference< XPropertySet > xLineProp;
        if( rPropSet.GetProperty( xLineProp, EXC_CHPROP_DATAREGRESSIONPROPS ) )
        {
            ScfPropertySet aLineProp( xLineProp );
            mxLineFmt->ConvertLine( aLineProp, EXC_CHLINEFORMAT_DOUBLE );
        }
    }
}

// ----------------------------------------------------------------------------

XclImpChSerErrorBar::XclImpChSerErrorBar( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChSerErrorBar::ReadChSerErrorBar( XclImpStream& rStrm )
{
    rStrm >> maData.mnBarType >> maData.mnSourceType >> maData.mnLineEnd;
    rStrm.Ignore( 1 );
    rStrm >> maData.mfValue >> maData.mnValueCount;
}

void XclImpChSerErrorBar::AddErrorBar( const XclImpChSerErrorBar& rErrorBar )
{
    // add the error bar type
    sal_uInt8 nBarType = rErrorBar.maData.mnBarType;
    switch( nBarType )
    {
        case EXC_CHSERERR_XPLUS:
        case EXC_CHSERERR_XMINUS:
        case EXC_CHSERERR_XBOTH:
            DBG_ASSERT( IsXErrorBar(), "XclImpChSerErrorBar::AddErrorBar - wrong orientation" );
            if( maData.mnBarType != nBarType )
                maData.mnBarType = EXC_CHSERERR_XBOTH;
        break;
        case EXC_CHSERERR_YPLUS:
        case EXC_CHSERERR_YMINUS:
        case EXC_CHSERERR_YBOTH:
            DBG_ASSERT( IsYErrorBar(), "XclImpChSerErrorBar::AddErrorBar - wrong orientation" );
            if( maData.mnBarType != nBarType )
                maData.mnBarType = EXC_CHSERERR_YBOTH;
        break;
        default:
            DBG_ERROR1( "XclImpChSerErrorBar::AddErrorBar - unknown error bar type: %d", nBarType );
    }

    // add error bar formatting settings
    if( !mxBarFmt )
        mxBarFmt = rErrorBar.mxBarFmt;
}

bool XclImpChSerErrorBar::IsXErrorBar() const
{
    return (maData.mnBarType == EXC_CHSERERR_XPLUS) || (maData.mnBarType == EXC_CHSERERR_XMINUS) || (maData.mnBarType == EXC_CHSERERR_XBOTH);
}

bool XclImpChSerErrorBar::IsYErrorBar() const
{
    return (maData.mnBarType == EXC_CHSERERR_YPLUS) || (maData.mnBarType == EXC_CHSERERR_YMINUS) || (maData.mnBarType == EXC_CHSERERR_YBOTH);
}

void XclImpChSerErrorBar::Convert( ScfPropertySet& rPropSet ) const
{
    namespace cssc = ::com::sun::star::chart;

    // type (direction) of the error bars
    cssc::ChartErrorIndicatorType eApiBarType = cssc::ChartErrorIndicatorType_NONE;
    switch( maData.mnBarType )
    {
        case EXC_CHSERERR_YPLUS:    eApiBarType = cssc::ChartErrorIndicatorType_UPPER;          break;
        case EXC_CHSERERR_YMINUS:   eApiBarType = cssc::ChartErrorIndicatorType_LOWER;          break;
        case EXC_CHSERERR_YBOTH:    eApiBarType = cssc::ChartErrorIndicatorType_TOP_AND_BOTTOM; break;
        default:    DBG_ERRORFILE( "XclImpChSerErrorBar::Convert - invalid error bar type" );
    }
    rPropSet.SetProperty( EXC_CHPROP_ERRORINDICATOR, eApiBarType );

    // type of displayed error
    switch( maData.mnSourceType )
    {
        case EXC_CHSERERR_PERCENT:
            rPropSet.SetProperty( EXC_CHPROP_ERRORCATEGORY, cssc::ChartErrorCategory_PERCENT );
            rPropSet.SetProperty( EXC_CHPROP_PERCENTAGEERROR, maData.mfValue );
        break;
        case EXC_CHSERERR_FIXED:
            rPropSet.SetProperty( EXC_CHPROP_ERRORCATEGORY, cssc::ChartErrorCategory_CONSTANT_VALUE );
            rPropSet.SetProperty( EXC_CHPROP_CONSTANTERRORLOW, maData.mfValue );
            rPropSet.SetProperty( EXC_CHPROP_CONSTANTERRORHIGH, maData.mfValue );
        break;
        case EXC_CHSERERR_STDDEV:
            rPropSet.SetProperty( EXC_CHPROP_ERRORCATEGORY, cssc::ChartErrorCategory_STANDARD_DEVIATION );
        break;
        // Standard error not supported
//        case EXC_CHSERERR_STDERR:
//        break;
    }

    // error bar formatting
    if( mxBarFmt.is() )
    {
        Reference< XPropertySet > xErrorProp;
        if( rPropSet.GetProperty( xErrorProp, EXC_CHPROP_DATAERRORPROPS ) )
        {
            ScfPropertySet aErrorProp( xErrorProp );
            mxBarFmt->ConvertLine( aErrorProp, EXC_CHLINEFORMAT_SINGLE );
        }
    }
}

// ----------------------------------------------------------------------------

XclImpChSeries::XclImpChSeries( const XclImpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclImpChRoot( rRoot ),
    meOrient( EXC_CHORIENT_EMPTY ),
    mnGroupIdx( EXC_CHSERGROUP_NONE ),
    mnSeriesIdx( nSeriesIdx ),
    mnParentIdx( EXC_CHSERIES_INVALID ),
    mbHasValue( false ),
    mbHasCateg( false ),
    mbHasTitle( false )
{
}

void XclImpChSeries::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.mnCategType >> maData.mnValueType >> maData.mnCategCount >> maData.mnValueCount;
    if( GetBiff() == EXC_BIFF8 )
        rStrm >> maData.mnBubbleType >> maData.mnBubbleCount;
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
            rStrm >> mnGroupIdx;
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
        case EXC_ID_CHEND:
            ReadChEnd( rStrm );
        break;
    }
}

bool XclImpChSeries::CheckAndUpdateOrientation( const XclImpChSeries& rSeries )
{
    /*  IsValidValueLink() checks only the interval positions of the links. It
        returns false for invalid orientation (e.g. COMPLEX). Main position of the
        series (e.g. two vertical series in the same column) are checked later. */
    bool bValid = mbHasValue && rSeries.mbHasValue && mxValueLink->IsValidValueLink( *rSeries.mxValueLink );
    if( !bValid )
        GetTracer().TraceChartRange();

    // if orientation of own values is single, update it with position of passed series
    if( bValid && (mxValueLink->GetOrientation() == EXC_CHORIENT_SINGLE) )
    {
        if( mxValueLink->GetFirstScRow() == rSeries.mxValueLink->GetFirstScRow() )
            meOrient = EXC_CHORIENT_VERTICAL;
        else if( mxValueLink->GetFirstScCol() == rSeries.mxValueLink->GetFirstScCol() )
            meOrient = EXC_CHORIENT_HORIZONTAL;
        // orientation should be final now...
        DBG_ASSERT( meOrient != EXC_CHORIENT_SINGLE, "XclImpChSeries::CheckAndUpdateOrientation - orientation not found" );
    }
    return bValid;
}

void XclImpChSeries::SetFinalOrientation( XclChOrientation eOrient )
{
    if( mbHasValue && (mxValueLink->GetOrientation() == EXC_CHORIENT_SINGLE) )
    {
        // update orientation for values
        mxValueLink->SetFinalOrientation( eOrient );
        // update orientation for categories
        if( mbHasCateg )
        {
            // if value link is a single cell, then a valid category is a single cell too
            mxCategLink->SetFinalOrientation( eOrient );
            /*  Categories may be invalid now. This happens, if e.g. a single
                value cell has a single category cell to the left, which makes
                this series vertical. If the final orientation is horizontal,
                the category range cannot be used. */
            mbHasCateg = mxValueLink->IsValidCategLink( *mxCategLink );
        }
        // update title
        if( mbHasTitle )
        {
            /*  Do not update title orientation, we always use the first cell
                only. But check the validity of the title position according to
                the new orientation of the value ranges. */
            mbHasTitle = mxValueLink->IsValidTitleLink( *mxTitleLink );
        }
        // remember the new orientation
        meOrient = eOrient;
    }
}

void XclImpChSeries::UpdateCategories( const XclImpChSeries& rSeries )
{
    // remove own categories, if they are not valid for the values of the passed series
    if( mbHasCateg && rSeries.mbHasValue && !rSeries.mxValueLink->IsValidCategLink( *mxCategLink ) )
    {
        mxCategLink.reset();
        mbHasCateg = false;
    }
    // take categories of passed series, if they are valid for own values
    if( mbHasValue && !mbHasCateg && rSeries.mbHasCateg && mxValueLink->IsValidCategLink( *rSeries.mxCategLink ) )
    {
        mxCategLink = rSeries.mxCategLink;
        mbHasCateg = true;
    }
}

void XclImpChSeries::UpdateTitle( const XclImpChSeries& rSeries )
{
    if( mbHasTitle )
    {
        if( rSeries.mbHasTitle ) switch( meOrient )
        {
            case EXC_CHORIENT_VERTICAL:     mbHasTitle = mxTitleLink->GetFirstScRow() == rSeries.mxTitleLink->GetFirstScRow();  break;
            case EXC_CHORIENT_HORIZONTAL:   mbHasTitle = mxTitleLink->GetFirstScCol() == rSeries.mxTitleLink->GetFirstScCol();  break;
            default:                        mbHasTitle = false;
        }
        else
            mbHasTitle = false;
    }
}

void XclImpChSeries::AddTitleToValues()
{
    DBG_ASSERT( mbHasTitle, "XclImpChSeries::AddTitleToValues - no valid series title found" );
    if( !mbHasTitle ) return;

    SCCOLROW nScPos = -1;
    switch( meOrient )
    {
        case EXC_CHORIENT_VERTICAL:     nScPos = mxTitleLink->GetFirstScRow();  break;
        case EXC_CHORIENT_HORIZONTAL:   nScPos = mxTitleLink->GetFirstScCol();  break;
    }
    if( nScPos >= 0 )
    {
        if( mbHasValue )
            mxValueLink->InsertTitlePos( nScPos );
        // #i1508# chart needs title for category range (to get a rectangular range)
        if( mbHasCateg )
            mxCategLink->InsertTitlePos( nScPos );
    }
}

void XclImpChSeries::SetDataFormat( XclImpChDataFormatRef xDataFmt )
{
    if( xDataFmt.is() )
    {
        XclImpChDataFormatRef* pxDataFmt = GetDataFormatRef( xDataFmt->GetPointPos().mnPointIdx );
        // do not overwrite existing data format
        if( pxDataFmt && !*pxDataFmt )
        {
            *pxDataFmt = xDataFmt;
            // #i51639# register series format index at chart group
            if( pxDataFmt == &mxSeriesFmt )
                if( XclImpChChartGroup* pChartGroup = GetChartGroup() )
                    pChartGroup->SetUsedFormatIndex( xDataFmt->GetFormatIdx() );
        }
    }
}

void XclImpChSeries::SetDataLabel( XclImpChTextRef xLabel )
{
    if( xLabel.is() )
    {
        XclImpChTextRef* pxLabel = GetDataLabelRef( xLabel->GetPointPos().mnPointIdx );
        if( pxLabel && !*pxLabel )
            *pxLabel = xLabel;
    }
}

namespace {

void lclAddErrorBar( XclImpChSerErrorBarRef& rxOldErrorBar, XclImpChSerErrorBarRef xNewErrorBar )
{
    if( xNewErrorBar.is() )
    {
        if( rxOldErrorBar.is() )
            rxOldErrorBar->AddErrorBar( *xNewErrorBar );
        else
            rxOldErrorBar = xNewErrorBar;
    }
}

} // namespace

void XclImpChSeries::AddChildSeries( const XclImpChSeries& rSeries )
{
    /*  In Excel, trend lines and error bars are stored as own series. In Calc,
        these are properties of the parent series. This function adds the
        settings of the passed series to this series. */
    if( !mxTrendLine )
        mxTrendLine = rSeries.mxTrendLine;
    lclAddErrorBar( mxXErrorBar, rSeries.mxXErrorBar );
    lclAddErrorBar( mxYErrorBar, rSeries.mxYErrorBar );
}

void XclImpChSeries::FinalizeDataFormats()
{
    XclImpChChartGroup* pChartGroup = &GetChartData().GetDefChartGroup( mnGroupIdx );

    // create missing series format
    if( pChartGroup && !mxSeriesFmt && !HasParentSeries() )
    {
        // #i51639# use a new unused format index to create series default format
        sal_uInt16 nFormatIdx = pChartGroup->PopUnusedFormatIndex();
        mxSeriesFmt = CreateDataFormat( EXC_CHDATAFORMAT_ALLPOINTS, nFormatIdx );
    }

    // set text labels to data formats
    for( XclImpChTextMap::iterator aTIt = maLabels.begin(), aTEnd = maLabels.end(); aTIt != aTEnd; ++aTIt )
    {
        if( XclImpChDataFormatRef* pxDataFmt = GetDataFormatRef( aTIt->first ) )
        {
            if( !*pxDataFmt )
                *pxDataFmt = CreateDataFormat( aTIt->first, EXC_CHDATAFORMAT_DEFAULT );
            (*pxDataFmt)->SetDataLabel( aTIt->second );
        }
    }

    // update series format (copy missing formatting from group default format)
    if( pChartGroup && mxSeriesFmt.is() )
        mxSeriesFmt->UpdateSeriesFormat( *pChartGroup );

    // copy series formatting to trend lines and error bars
    if( mxTrendLine.is() )
        mxTrendLine->SetLineFormat( mxSeriesFmt );
    if( mxXErrorBar.is() )
        mxXErrorBar->SetBarFormat( mxSeriesFmt );
    if( mxYErrorBar.is() )
        mxYErrorBar->SetBarFormat( mxSeriesFmt );

    // update data point formats (removes unchanged automatic formatting)
    if( pChartGroup )
        for( XclImpChDataFormatMap::iterator aFIt = maPointFmts.begin(), aFEnd = maPointFmts.end(); aFIt != aFEnd; ++aFIt )
            aFIt->second->UpdatePointFormat( *pChartGroup, mxSeriesFmt.get() );
}

sal_uInt16 XclImpChSeries::GetChartTypeId() const
{
    return GetChartData().GetChartTypeId();
}

bool XclImpChSeries::HasEqualCategories( const XclImpChSeries& rSeries ) const
{
    return mbHasCateg && rSeries.mbHasCateg && mxCategLink->IsEqualLink( *rSeries.mxCategLink );
}

const XclImpChMainPos& XclImpChSeries::GetMainPos() const
{
    static const XclImpChMainPos aDummyPos;
    DBG_ASSERT( mbHasValue, "XclImpChSeries::GetMainPos - no value links" );
    return mbHasValue ? mxValueLink->GetMainPos() : aDummyPos;
}

bool XclImpChSeries::HasMarker() const
{
    bool bHasMarker = mxSeriesFmt.is() && mxSeriesFmt->HasMarker();
    for( XclImpChDataFormatMap::const_iterator aIt = maPointFmts.begin(), aEnd = maPointFmts.end(); !bHasMarker && (aIt != aEnd); ++aIt )
        bHasMarker = aIt->second->HasMarker();
    return bHasMarker;
}

void XclImpChSeries::JoinValueRanges( ScRangeList& rScRanges ) const
{
    if( mbHasValue )
        mxValueLink->JoinRanges( rScRanges );
}

void XclImpChSeries::JoinCategoryRanges( ScRangeList& rScRanges ) const
{
    if( mbHasCateg )
        mxCategLink->JoinRanges( rScRanges );
}

namespace {

/** Returns the property set of the specified series. */
ScfPropertySet lclGetSeriesPropSet( Reference< XDiagram > xDiagram, sal_uInt16 nTypeId, sal_uInt16 nSeriesIdx )
{
    ScfPropertySet aPropSet;
    try
    {
        // XY charts: series 0 is used for X values, series 1 is first series for Y values
        if( nTypeId == EXC_ID_CHSCATTER )
            ++nSeriesIdx;
        aPropSet.Set( xDiagram->getDataRowProperties( nSeriesIdx ) );
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "lclGetSeriesPropSet - no series property set" );
    }
    return aPropSet;
}

/** Returns the property set of the specified data point. */
ScfPropertySet lclGetPointPropSet( Reference< XDiagram > xDiagram,
    sal_uInt16 nTypeId, sal_uInt16 nSeriesIdx, sal_uInt16 nPointIdx )
{
    ScfPropertySet aPropSet;
    try
    {
        // XY charts: series 0 is used for X values, series 1 is first series for Y values
        if( nTypeId == EXC_ID_CHSCATTER )
            ++nSeriesIdx;
        // series index and data point index exchanged in OOChart donut charts
        if( nTypeId == EXC_ID_CHDONUT )
            aPropSet.Set( xDiagram->getDataPointProperties( nSeriesIdx, nPointIdx ) );
        else
            aPropSet.Set( xDiagram->getDataPointProperties( nPointIdx, nSeriesIdx ) );
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "lclGetPointPropSet - no data point property set" );
    }
    return aPropSet;
}

} // namespace

void XclImpChSeries::Convert( Reference< XChartDocument > xChartDoc, size_t nSeriesIdx ) const
{
    Reference< XDiagram > xDiagram = xChartDoc->getDiagram();
    DBG_ASSERT( xDiagram.is(), "XclImpChSeries::Convert - no diagram" );
    if( !xDiagram.is() )
        return;

    // chart type and number of series on the axes set
    const XclImpChChartGroup& rChartGroup = GetChartData().GetDefChartGroup( mnGroupIdx );
    sal_uInt16 nTypeId = rChartGroup.GetChartTypeId();
    sal_uInt16 nSeriesCount = GetChartData().GetChartGroupSeriesCount( mnGroupIdx );
    sal_uInt16 nPointCount = GetPointCount();

    // don't set any formatting on additional pie series (may change formats of first series)
    if( (nTypeId == EXC_ID_CHPIE) && (nSeriesIdx > 0) )
        return;

    // ------------------------------------------------------------------------
    // write global series formatting and assign series to correct axes set
    if( (nTypeId == EXC_ID_CHPIE) || (nTypeId == EXC_ID_CHDONUT) )
    {
        /*  Series of pie charts and donut charts have different internal
            orientation. Therefore every data point of a series must be set.
            Additional pitfall: points on pie charts and points on donut
            charts have to be accessed with different indexes (on donut
            charts, series index and point index are exchanged). */
        if( mxSeriesFmt.is() )
        {
            for( sal_uInt16 nPointIdx = 0; nPointIdx < nPointCount; ++nPointIdx )
            {
                // get the data point property set from the chart
                ScfPropertySet aPointProp( lclGetPointPropSet( xDiagram, nTypeId, nSeriesIdx, nPointIdx ) );
                // set the series formatting
                mxSeriesFmt->Convert( aPointProp, rChartGroup );
                // set additional formatting for automatic point colors
                if( rChartGroup.IsVarPointFormat() && mxSeriesFmt->IsAutoArea() )
                    mxSeriesFmt->ConvertVarPoint( aPointProp, nPointIdx );
            }
        }
    }
    else
    {
        // get the series property set from the chart
        ScfPropertySet aSeriesProp( lclGetSeriesPropSet( xDiagram, nTypeId, nSeriesIdx ) );
        // attach series to correct axes set
        using namespace ::com::sun::star::chart::ChartAxisAssign;
        bool bSecondary = GetChartData().GetChartGroupAxesSetId( mnGroupIdx ) == EXC_CHAXESSET_SECONDARY;
        aSeriesProp.SetProperty( EXC_CHPROP_AXIS, bSecondary ? SECONDARY_Y : PRIMARY_Y );
        // set the series formatting
        if( mxSeriesFmt.is() )
            mxSeriesFmt->Convert( aSeriesProp, rChartGroup );
        // trend line, error bars
        if( mxTrendLine.is() )
            mxTrendLine->Convert( aSeriesProp );
        if( mxYErrorBar.is() )
            mxYErrorBar->Convert( aSeriesProp );
        // own area formatting for every data point
        if( rChartGroup.IsVarPointFormat() && (nSeriesCount == 1) && mxSeriesFmt.is() && mxSeriesFmt->IsAutoArea() )
        {
            for( sal_uInt16 nPointIdx = 0; nPointIdx < nPointCount; ++nPointIdx )
            {
                // get the data point property set from the chart
                ScfPropertySet aPointProp( lclGetPointPropSet( xDiagram, nTypeId, nSeriesIdx, nPointIdx ) );
                // set the data point formatting, nPointIdx used as automatic format index
                mxSeriesFmt->ConvertVarPoint( aPointProp, nPointIdx );
            }
        }
    }

    // ------------------------------------------------------------------------
    // write separate data point formatting
    for( XclImpChDataFormatMap::const_iterator aIt = maPointFmts.begin(), aEnd = maPointFmts.end();
            (aIt != aEnd) && (aIt->first < nPointCount); ++aIt )
    {
        // get the data point property set from the chart
        ScfPropertySet aPointProp( lclGetPointPropSet( xDiagram, nTypeId, nSeriesIdx, aIt->first ) );
        // set data point formatting
        aIt->second->Convert( aPointProp, rChartGroup );
    }
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
    rStrm >> mnParentIdx;
    // index to parent series is 1-based, convert it to 0-based
    if( mnParentIdx > 0 )
        --mnParentIdx;
    else
        mnParentIdx = EXC_CHSERIES_INVALID;
}

void XclImpChSeries::ReadChSerTrendLine( XclImpStream& rStrm )
{
    mxTrendLine.reset( new XclImpChSerTrendLine( GetChRoot() ) );
    mxTrendLine->ReadChSerTrendLine( rStrm );
}

void XclImpChSeries::ReadChSerErrorBar( XclImpStream& rStrm )
{
    XclImpChSerErrorBarRef xErrorBar( new XclImpChSerErrorBar( GetChRoot() ) );
    xErrorBar->ReadChSerErrorBar( rStrm );
    if( xErrorBar->IsXErrorBar() )
        lclAddErrorBar( mxXErrorBar, xErrorBar );
    else if( xErrorBar->IsYErrorBar() )
        lclAddErrorBar( mxYErrorBar, xErrorBar );
}

void XclImpChSeries::ReadChEnd( XclImpStream& rStrm )
{
    // calculate the orientation of this series
    CalcOrientation();

    // remove invalid trend line
    if( mxTrendLine.is() && !mxTrendLine->IsValidType() )
        mxTrendLine.reset();
}

void XclImpChSeries::CalcOrientation()
{
    meOrient = EXC_CHORIENT_EMPTY;
    mbHasValue = mbHasCateg = mbHasTitle = false;

    // range for values (ignore child series, e.g. trend lines and error bars)
    if( !HasParentSeries() && mxValueLink.is() && mxValueLink->HasValidOrientation() )
    {
        meOrient = mxValueLink->GetOrientation();
        mbHasValue = true;

        // category range
        if( mxCategLink.is() )
        {
            mbHasCateg = mxValueLink->IsValidCategLink( *mxCategLink );
            // single value and single category: try to get final orientation
            if( mbHasCateg && (meOrient == EXC_CHORIENT_SINGLE) )
                meOrient = (mxCategLink->GetFirstScRow() == mxValueLink->GetFirstScRow()) ?
                    EXC_CHORIENT_VERTICAL : EXC_CHORIENT_HORIZONTAL;
        }

        // series title
        if( mxTitleLink.is() && (mxTitleLink->GetOrientation() != EXC_CHORIENT_EMPTY) )
        {
            mbHasTitle = mxValueLink->IsValidTitleLink( *mxTitleLink );
            // single value and single title: try to get final orientation
            if( mbHasTitle && (meOrient == EXC_CHORIENT_SINGLE) )
                meOrient = (mxTitleLink->GetFirstScCol() == mxValueLink->GetFirstScCol()) ?
                    EXC_CHORIENT_VERTICAL : EXC_CHORIENT_HORIZONTAL;
        }
    }

    if( mxValueLink.is() && (mxValueLink->GetOrientation() == EXC_CHORIENT_COMPLEX) )
        GetTracer().TraceChartRange();
}

XclImpChChartGroup* XclImpChSeries::GetChartGroup() const
{
    return &GetChartData().GetDefChartGroup( mnGroupIdx );
}

XclImpChDataFormatRef XclImpChSeries::CreateDataFormat( sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx )
{
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->SetPointPos( mnSeriesIdx, nPointIdx, nFormatIdx );
    return xDataFmt;
}

XclImpChDataFormatRef* XclImpChSeries::GetDataFormatRef( sal_uInt16 nPointIdx )
{
    if( nPointIdx == EXC_CHDATAFORMAT_ALLPOINTS )
        return &mxSeriesFmt;
    if( nPointIdx <= EXC_CHDATAFORMAT_MAXPOINT )
        return &maPointFmts[ nPointIdx ];
    return 0;
}

XclImpChTextRef* XclImpChSeries::GetDataLabelRef( sal_uInt16 nPointIdx )
{
    if( (nPointIdx == EXC_CHDATAFORMAT_ALLPOINTS) || (nPointIdx <= EXC_CHDATAFORMAT_MAXPOINT) )
        return &maLabels[ nPointIdx ];
    return 0;
}

// ----------------------------------------------------------------------------

/** Binary predicate to compare two series by main position.
    @descr  Returns true, if first series is before second. */
struct XclImpChSeriesPred_Order
{
    inline bool         operator()( const XclImpChSeriesRef& rxSer1, const XclImpChSeriesRef& rxSer2 ) const
                            { return rxSer1->GetMainPos() < rxSer2->GetMainPos(); }
};

// Chart structure ============================================================

XclImpChType::XclImpChType( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    mnTypeId( EXC_ID_CHUNKNOWN ),
    mnGroupIdx( EXC_CHSERGROUP_NONE )
{
}

void XclImpChType::ReadChType( XclImpStream& rStrm )
{
    sal_uInt16 nRecId = rStrm.GetRecId();
    XclBiff eBiff = rStrm.GetRoot().GetBiff();
    bool bKnownType = true;

    switch( nRecId )
    {
        case EXC_ID_CHBAR:
            rStrm >> maData.mnOverlap >> maData.mnGap >> maData.mnFlags;
        break;

        case EXC_ID_CHLINE:
        case EXC_ID_CHAREA:
        case EXC_ID_CHRADARLINE:
        case EXC_ID_CHRADARAREA:
            rStrm >> maData.mnFlags;
        break;

        case EXC_ID_CHPIE:
            rStrm >> maData.mnRotation >> maData.mnPieHole;
            if( eBiff == EXC_BIFF8 )
                rStrm >> maData.mnFlags;
            else
                maData.mnFlags = 0;
        break;

        case EXC_ID_CHPIEEXT:
            maData.mnRotation = 0;
            maData.mnPieHole = 0;
            maData.mnFlags = 0;
        break;

        case EXC_ID_CHSCATTER:
            if( eBiff == EXC_BIFF8 )
                rStrm >> maData.mnBubbleSize >> maData.mnBubbleType >> maData.mnFlags;
            else
                maData.mnFlags = 0;
        break;

        default:
            bKnownType = false;
    }

    if( bKnownType )
        mnTypeId = nRecId;
}

void XclImpChType::Finalize()
{
    bool bIsPrimary = GetChartData().GetChartGroupAxesSetId( mnGroupIdx ) == EXC_CHAXESSET_PRIMARY;
    switch( mnTypeId )
    {
        case EXC_ID_CHLINE:
        case EXC_ID_CHBAR:
            // import uses different type identifier for stock charts
            if( GetChartData().IsStockChart() )
                mnTypeId = EXC_ID_CHSTOCK;
        break;
        case EXC_ID_CHPIE:
            // import uses different type identifier for donut charts
            if( maData.mnPieHole > 0 )
                mnTypeId = EXC_ID_CHDONUT;
        break;
        case EXC_ID_CHPIEEXT:
            // #i49241# pie-to-pie and pie-to-bar not supported - convert to pie chart
            mnTypeId = EXC_ID_CHPIE;
        break;
        case EXC_ID_CHSCATTER:
            // bubble chart - imported as XY chart
            if( ::get_flag( maData.mnFlags, EXC_CHSCATTER_BUBBLES ) )
                GetTracer().TraceChartUnKnownType();
            // #92909# no X values -> create line chart -- #94149# only for XY charts
            if( bIsPrimary && !GetChartData().UpdateScatterXRanges() )
            {
                mnTypeId = EXC_ID_CHLINE;
                maData.mnFlags = 0;
                GetTracer().TraceChartInvalidXY();
            }
        break;
        case EXC_ID_CHUNKNOWN:
            mnTypeId = EXC_ID_CHBAR;
            maData.mnFlags = 0;
            GetTracer().TraceChartUnKnownType();
        break;
    }
}

bool XclImpChType::IsStackedChart() const
{
    bool bStacked = false;
    switch( mnTypeId )
    {
        case EXC_ID_CHLINE:
        case EXC_ID_CHAREA:
        case EXC_ID_CHSTOCK:
            bStacked = ::get_flag( maData.mnFlags, EXC_CHLINE_STACKED );
        break;
        case EXC_ID_CHBAR:
        case EXC_ID_CHCOLUMN:
            bStacked = ::get_flag( maData.mnFlags, EXC_CHBAR_STACKED );
        break;
    }
    return bStacked;
}

bool XclImpChType::IsPercentChart() const
{
    bool bPercent = false;
    switch( mnTypeId )
    {
        case EXC_ID_CHLINE:
        case EXC_ID_CHAREA:
        case EXC_ID_CHSTOCK:
            bPercent = ::get_flag( maData.mnFlags, EXC_CHLINE_PERCENT );
        break;
        case EXC_ID_CHBAR:
        case EXC_ID_CHCOLUMN:
            bPercent = ::get_flag( maData.mnFlags, EXC_CHBAR_PERCENT );
        break;
    }
    return bPercent && IsStackedChart();
}

void XclImpChType::CreateDiagram( Reference< XChartDocument > xChartDoc ) const
{
    bool bIsPrimary = GetChartData().GetChartGroupAxesSetId( mnGroupIdx ) == EXC_CHAXESSET_PRIMARY;
    DBG_ASSERT( bIsPrimary, "XclImpChType::CreateDiagram - not allowed for secondary axes" );
    if( bIsPrimary )
    {
        OUString aService;
        switch( mnTypeId )
        {
            case EXC_ID_CHLINE:         aService = SERVICE_CHART_LINE;      break;
            case EXC_ID_CHAREA:         aService = SERVICE_CHART_AREA;      break;
            case EXC_ID_CHSTOCK:        aService = SERVICE_CHART_STOCK;     break;
            case EXC_ID_CHBAR:          aService = SERVICE_CHART_BAR;       break;
            case EXC_ID_CHPIE:          aService = SERVICE_CHART_PIE;       break;
            case EXC_ID_CHDONUT:        aService = SERVICE_CHART_DONUT;     break;
            case EXC_ID_CHRADARLINE:
            case EXC_ID_CHRADARAREA:    aService = SERVICE_CHART_NET;       break;
            case EXC_ID_CHSCATTER:      aService = SERVICE_CHART_XY;        break;
            default:
                DBG_ERRORFILE( "XclImpChType::CreateDiagram - unknown chart type" );
                aService = SERVICE_CHART_BAR;
        }

        Reference< XMultiServiceFactory > xFactory( xChartDoc, UNO_QUERY );
        Reference< XDiagram > xDiagram( ScfApiHelper::CreateInstance( xFactory, aService ), UNO_QUERY );
        if( xDiagram.is() )
            xChartDoc->setDiagram( xDiagram );
    }
}

void XclImpChType::Convert( Reference< XChartDocument > xChartDoc ) const
{
    bool bIsPrimary = GetChartData().GetChartGroupAxesSetId( mnGroupIdx ) == EXC_CHAXESSET_PRIMARY;
    Reference< XDiagram > xDiagram( xChartDoc->getDiagram() );
    ScfPropertySet aDiaProp( xDiagram );

    if( bIsPrimary )
    {
        bool bStacked = false;
        bool bPercent = false;

        switch( mnTypeId )
        {
            case EXC_ID_CHLINE:
            case EXC_ID_CHAREA:
            case EXC_ID_CHSTOCK:
                bStacked = ::get_flag( maData.mnFlags, EXC_CHLINE_STACKED );
                bPercent = ::get_flag( maData.mnFlags, EXC_CHLINE_PERCENT );
            break;
            case EXC_ID_CHBAR:
                aDiaProp.SetBoolProperty( EXC_CHPROP_VERTICAL, ::get_flag( maData.mnFlags, EXC_CHBAR_HORIZONTAL ) );
                bStacked = ::get_flag( maData.mnFlags, EXC_CHBAR_STACKED );
                bPercent = ::get_flag( maData.mnFlags, EXC_CHBAR_PERCENT );
            break;
            case EXC_ID_CHSCATTER:
                // #i19559# set existence of lines for correct display of chart type
                aDiaProp.SetBoolProperty( EXC_CHPROP_LINES, GetChartData().HasAnySeriesLine() );
            break;
        }

        /*  #86200# don't set the Boolean values for 'Stacked' and 'Percent'
            directly but use 'if' statements. Disabling 'Percent' in the second
            step resets 'Stacked' even if it was set before. */
        if( bStacked )
            aDiaProp.SetBoolProperty( EXC_CHPROP_STACKED, true );
        if( bPercent )
            aDiaProp.SetBoolProperty( EXC_CHPROP_PERCENT, true );
    }

    // additional axis properties (for primary or secondary axes)
    switch( mnTypeId )
    {
        case EXC_ID_CHBAR:
        {
            // gap width, overlap (at Y axis)
            Reference< XTwoAxisYSupplier > xAxisSup( xDiagram, UNO_QUERY );
            if( xAxisSup.is() )
            {
                ScfPropertySet aAxisProp( bIsPrimary ? xAxisSup->getYAxis() : xAxisSup->getSecondaryYAxis() );
                aAxisProp.SetProperty< sal_Int32 >( EXC_CHPROP_OVERLAP, -maData.mnOverlap );
                aAxisProp.SetProperty< sal_Int32 >( EXC_CHPROP_GAPWIDTH, maData.mnGap );
            }
        }
        break;
    }
}

// ----------------------------------------------------------------------------

void XclImpChChart3d::ReadChChart3d( XclImpStream& rStrm )
{
    rStrm   >> maData.mnRotation
            >> maData.mnElevation
            >> maData.mnDist
            >> maData.mnHeight
            >> maData.mnDepth
            >> maData.mnGap
            >> maData.mnFlags;
}

void XclImpChChart3d::Convert( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetBoolProperty( EXC_CHPROP_DIM3D, true );
    rPropSet.SetBoolProperty( EXC_CHPROP_DEEP, !::get_flag( maData.mnFlags, EXC_CHCHART3D_CLUSTER ) );
}

// ----------------------------------------------------------------------------

XclImpChLegend::XclImpChLegend( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChLegend::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.maRect >> maData.mnDockMode >> maData.mnSpacing >> maData.mnFlags;

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
        case EXC_ID_CHTEXT:
            mxText.reset( new XclImpChText( GetChRoot() ) );
            mxText->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_LEGEND ) );
            mxFrame->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHEND:
            // legend default formatting differs in OOChart and Excel
            if( !mxFrame )
                mxFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_LEGEND ) );
        break;
    }
}

void XclImpChLegend::Finalize()
{
    // Update text formatting. If mxText is empty, the passed default text is used.
    lclUpdateText( mxText, GetChartData().GetDefaultText( EXC_CHDEFTEXT_LEGEND ) );
}

void XclImpChLegend::Convert( ScfPropertySet& rPropSet ) const
{
    // frame properties
    if( mxFrame.is() )
        mxFrame->Convert( GetChRoot(), rPropSet );
    // text properties
    if( mxText.is() )
        mxText->ConvertFont( rPropSet );

    // legend position
    namespace cssc = ::com::sun::star::chart;
    cssc::ChartLegendPosition eApiPos;
    switch( maData.mnDockMode )
    {
        case EXC_CHLEGEND_BOTTOM:   eApiPos = cssc::ChartLegendPosition_BOTTOM; break;
        case EXC_CHLEGEND_TOP:      eApiPos = cssc::ChartLegendPosition_TOP;    break;
        case EXC_CHLEGEND_RIGHT:    eApiPos = cssc::ChartLegendPosition_RIGHT;  break;
        case EXC_CHLEGEND_LEFT:     eApiPos = cssc::ChartLegendPosition_LEFT;   break;
        default:
            eApiPos = ::get_flag( maData.mnFlags, EXC_CHLEGEND_STACKED ) ?
                cssc::ChartLegendPosition_RIGHT : cssc::ChartLegendPosition_BOTTOM;
    }
    rPropSet.SetProperty( EXC_CHPROP_ALIGNMENT, eApiPos );

    // auto-generated legend entries may differ in OOChart and Excel
    if( GetTracer().IsEnabled() && !GetChartData().HasValidTitles() )
    {
        sal_uInt16 nTypeId = GetChartData().GetChartTypeId();
        if( (nTypeId != EXC_ID_CHPIE) && (nTypeId != EXC_ID_CHDONUT) )
            GetTracer().TraceChartDSName();
    }
}

// ----------------------------------------------------------------------------

XclImpChDropBar::XclImpChDropBar( sal_uInt16 nDropBar ) :
    mnBarDist( 0 )
{
}

void XclImpChDropBar::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> mnBarDist;
}

void XclImpChDropBar::Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    ConvertFrameBase( rRoot, rPropSet );
}

// ----------------------------------------------------------------------------

XclImpChChartGroup::XclImpChChartGroup( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot ),
    maType( rRoot )
{
    maType.SetGroupIndex( maData.mnGroupIdx );

    // Initialize unused format indexes set. At this time, all formats are unused.
    for( sal_uInt16 nFormatIdx = 0; nFormatIdx <= EXC_CHSERIES_MAXSERIES; ++nFormatIdx )
        maUnusedFormats.insert( maUnusedFormats.end(), nFormatIdx );
}

void XclImpChChartGroup::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.maRect >> maData.mnFlags >> maData.mnGroupIdx;
    maType.SetGroupIndex( maData.mnGroupIdx );
}

void XclImpChChartGroup::ReadSubRecord( XclImpStream& rStrm )
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

void XclImpChChartGroup::Finalize()
{
    // check and set valid chart type
    maType.Finalize();
    // update chart group format, may depend on chart type finalized above
    if( mxGroupFmt.is() )
        mxGroupFmt->UpdateGroupFormat( *this );
}

void XclImpChChartGroup::SetUsedFormatIndex( sal_uInt16 nFormatIdx )
{
    maUnusedFormats.erase( nFormatIdx );
}

sal_uInt16 XclImpChChartGroup::PopUnusedFormatIndex()
{
    DBG_ASSERT( !maUnusedFormats.empty(), "XclImpChChartGroup::PopUnusedFormatIndex - no more format indexes available" );
    sal_uInt16 nFormatIdx = maUnusedFormats.empty() ? 0 : *maUnusedFormats.begin();
    SetUsedFormatIndex( nFormatIdx );
    return nFormatIdx;
}

void XclImpChChartGroup::CreateDiagram( Reference< XChartDocument > xChartDoc ) const
{
    // chart type (must be done before first getDiagram() call)
    maType.CreateDiagram( xChartDoc );

    // 3d mode
    if( mxChart3d.is() )
    {
        ScfPropertySet aDiaProp( xChartDoc->getDiagram() );
        mxChart3d->Convert( aDiaProp );
    }
}

void XclImpChChartGroup::Convert( Reference< XChartDocument > xChartDoc ) const
{
    // type dependent settings
    maType.Convert( xChartDoc );

    // stock chart properties
    if( (GetChartData().GetChartTypeId() == EXC_ID_CHSTOCK) && HasHiLoLine() )
    {
        Reference< XDiagram > xDiagram( xChartDoc->getDiagram() );

        ScfPropertySet aDiaProp( xDiagram );
        aDiaProp.SetBoolProperty( EXC_CHPROP_UPDOWN, !maDropBars.empty() );
        aDiaProp.SetBoolProperty( EXC_CHPROP_VOLUME, GetChartData().HasAxesSet( EXC_CHAXESSET_SECONDARY ) );

        Reference< XStatisticDisplay > xStatDisplay( xDiagram, UNO_QUERY );
        if( xStatDisplay.is() )
        {
            XclImpChLineFormatRef xHiLoLine = maChartLines.get( EXC_CHCHARTLINE_HILO );
            if( xHiLoLine.is() )
            {
                ScfPropertySet aLineProp( xStatDisplay->getMinMaxLine() );
                xHiLoLine->Convert( GetChRoot(), aLineProp );
            }
            XclImpChDropBarRef xUpBar = maDropBars.get( EXC_CHDROPBAR_UP );
            if( xUpBar.is() )
            {
                ScfPropertySet aBarProp( xStatDisplay->getUpBar() );
                xUpBar->Convert( GetChRoot(), aBarProp );
            }
            XclImpChDropBarRef xDownBar = maDropBars.get( EXC_CHDROPBAR_DOWN );
            if( xDownBar.is() )
            {
                ScfPropertySet aBarProp( xStatDisplay->getDownBar() );
                xDownBar->Convert( GetChRoot(), aBarProp );
            }
        }
    }
}

void XclImpChChartGroup::ReadChDropBar( XclImpStream& rStrm )
{
    sal_uInt16 nDropBar = EXC_CHDROPBAR_NONE;
    if( !maDropBars.has( EXC_CHDROPBAR_UP ) )
        nDropBar = EXC_CHDROPBAR_UP;
    else if( !maDropBars.has( EXC_CHDROPBAR_DOWN ) )
        nDropBar = EXC_CHDROPBAR_DOWN;

    if( nDropBar != EXC_CHDROPBAR_NONE )
    {
        XclImpChDropBarRef xDropBar( new XclImpChDropBar( nDropBar ) );
        xDropBar->ReadRecordGroup( rStrm );
        maDropBars[ nDropBar ] = xDropBar;
    }
}

void XclImpChChartGroup::ReadChChartLine( XclImpStream& rStrm )
{
    sal_uInt16 nLineId = rStrm.ReaduInt16();
    if( (rStrm.GetNextRecId() == EXC_ID_CHLINEFORMAT) && rStrm.StartNextRecord() )
    {
        XclImpChLineFormatRef xLineFmt( new XclImpChLineFormat );
        xLineFmt->ReadChLineFormat( rStrm );
        maChartLines[ nLineId ] = xLineFmt;
    }
}

void XclImpChChartGroup::ReadChDataFormat( XclImpStream& rStrm )
{
    // global series and data point format
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->ReadRecordGroup( rStrm );
    const XclChDataPointPos& rPos = xDataFmt->GetPointPos();
    if( (rPos.mnSeriesIdx == 0) && (rPos.mnPointIdx == 0) &&
            (xDataFmt->GetFormatIdx() == EXC_CHDATAFORMAT_DEFAULT) )
        mxGroupFmt = xDataFmt;
}

// Axes =======================================================================

XclImpChLabelRange::XclImpChLabelRange( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChLabelRange::ReadChLabelRange( XclImpStream& rStrm )
{
    rStrm >> maData.mnCross >> maData.mnLabelFreq >> maData.mnTickFreq >> maData.mnFlags;
}

void XclImpChLabelRange::Convert( ScfPropertySet& rPropSet ) const
{
    // never overlap text
    rPropSet.SetBoolProperty( EXC_CHPROP_TEXTOVERLAP, false );
    // break text into several lines, if all labels are shown
    rPropSet.SetBoolProperty( EXC_CHPROP_TEXTBREAK, maData.mnLabelFreq == 1 );
}

// ----------------------------------------------------------------------------

XclImpChValueRange::XclImpChValueRange( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChValueRange::ReadChValueRange( XclImpStream& rStrm )
{
    rStrm   >> maData.mfMin
            >> maData.mfMax
            >> maData.mfMajorStep
            >> maData.mfMinorStep
            >> maData.mfCross
            >> maData.mnFlags;
}

void XclImpChValueRange::Convert( ScfPropertySet& rPropSet ) const
{
    bool bAutoMin   = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN );
    bool bAutoMax   = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX );
    bool bAutoMajor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR );
    bool bAutoMinor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR );
    bool bAutoCross = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS );
    bool bLog       = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE );
    double fMin     = maData.mfMin;
    double fMax     = maData.mfMax;
    double fCross   = maData.mfCross;

    if( bAutoCross )
    {
        /*  #60146# special processing for automatic origin in automatic axis range.
            If minimum or maximum are set to automatic, Excel uses 0.0 for the origin.
            Chart would use real minimum or maximum in that case. */
        sal_uInt16 nTypeId = GetChartData().GetChartTypeId();
        bool bAdjustAutoOrigin =
            (nTypeId == EXC_ID_CHLINE) || (nTypeId == EXC_ID_CHAREA) ||
            (nTypeId == EXC_ID_CHSTOCK) || (nTypeId == EXC_ID_CHBAR);

        if( bAdjustAutoOrigin &&
            (bAutoMin && bAutoMax) ||                       // minimum and maximum automatic
            (bAutoMin && !bAutoMax && (fMax > 0.0)) ||      // minimum automatic, maximum > 0
            (!bAutoMin && (fMin < 0.0) && bAutoMax) )       // minimum < 0, maximum automatic
        {
            bAutoCross = false;
            fCross = 0.0;
            GetTracer().TraceChartAxisAutoCross();
        }
    }
    else
    {
        // Adjust manual origin, if it is outside the manual minimum or maximum.
        if( !bAutoMin && (fCross < fMin) )
            fCross = fMin;
        else if( !bAutoMax && (fCross > fMax) )
            fCross = fMax;
        if( fCross != maData.mfCross )
            GetTracer().TraceChartAxisManualCross();
    }

    // axis origin
    rPropSet.SetBoolProperty( EXC_CHPROP_AUTOORIG, bAutoCross );
    if( !bAutoCross )
        rPropSet.SetProperty( EXC_CHPROP_ORIGIN, fCross );
    // logarithmic scale
    if( bLog )
        rPropSet.SetBoolProperty( EXC_CHPROP_LOG, true );
    // minimum
    rPropSet.SetBoolProperty( EXC_CHPROP_AUTOMIN, bAutoMin );
    if( !bAutoMin )
        rPropSet.SetProperty( EXC_CHPROP_MIN, fMin );
    // maximum
    rPropSet.SetBoolProperty( EXC_CHPROP_AUTOMAX, bAutoMax );
    if( !bAutoMax )
        rPropSet.SetProperty( EXC_CHPROP_MAX, fMax );
    // major grid distance
    rPropSet.SetBoolProperty( EXC_CHPROP_AUTOSTMAIN, bAutoMajor );
    if( !bAutoMajor )
        rPropSet.SetProperty( EXC_CHPROP_STEPMAIN, maData.mfMajorStep );
    // minor grid distance
    rPropSet.SetBoolProperty( EXC_CHPROP_AUTOSTHELP, bAutoMinor );
    if( !bAutoMinor )
        rPropSet.SetProperty( EXC_CHPROP_STEPHELP, maData.mfMinorStep );
}

// ----------------------------------------------------------------------------

namespace {

sal_Int32 lclGetApiAxisMarks( sal_uInt8 nXclTickPos )
{
    using namespace ::com::sun::star::chart::ChartAxisMarks;
    sal_Int32 nApiMarks = NONE;
    ::set_flag( nApiMarks, INNER, ::get_flag( nXclTickPos, EXC_CHTICK_INSIDE ) );
    ::set_flag( nApiMarks, OUTER, ::get_flag( nXclTickPos, EXC_CHTICK_OUTSIDE ) );
    return nApiMarks;
}

} // namespace

XclImpChTick::XclImpChTick( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChTick::ReadChTick( XclImpStream& rStrm )
{
    rStrm   >> maData.mnMajor
            >> maData.mnMinor
            >> maData.mnLabelPos
            >> maData.mnBackMode
            >> maData.maRect
            >> maData.maTextColor
            >> maData.mnFlags;

    if( GetBiff() == EXC_BIFF8 )
    {
        rStrm >> maData.mnTextColorIdx >> maData.mnRotation;
        // #116397# BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( maData.mnTextColorIdx );
    }
}

Color XclImpChTick::GetFontColor() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTICK_AUTOCOLOR ) ? GetFontAutoColor() : maData.maTextColor;
}

sal_uInt16 XclImpChTick::GetRotation() const
{
    return ::get_flag( maData.mnFlags, EXC_CHTICK_AUTOROT ) ? EXC_CHART_AUTOROTATION : maData.mnRotation;
}

void XclImpChTick::Convert( ScfPropertySet& rPropSet ) const
{
    // tick marks
    rPropSet.SetProperty( EXC_CHPROP_MARKS,     lclGetApiAxisMarks( maData.mnMajor ) );
    rPropSet.SetProperty( EXC_CHPROP_HELPMARKS, lclGetApiAxisMarks( maData.mnMinor ) );
    // axis labels rotation
    if( !::get_flag( maData.mnFlags, EXC_CHTICK_AUTOROT ) )
        rPropSet.SetProperty( EXC_CHPROP_TEXTROTATION, XclTools::GetScRotation( maData.mnRotation, 27000 ) );
}

void XclImpChTick::ConvertFontColor( ScfPropertySet& rPropSet ) const
{
    rPropSet.SetColorProperty( EXC_CHPROP_CHARCOLOR, GetFontColor() );
}

// ----------------------------------------------------------------------------

XclImpChAxis::XclImpChAxis( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclImpChRoot( rRoot ),
    mnAxesSetId( nAxesSetId )
{
}

void XclImpChAxis::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.mnType >> maData.maRect;
}

void XclImpChAxis::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHLABELRANGE:
            mxLabelRange.reset( new XclImpChLabelRange( GetChRoot() ) );
            mxLabelRange->ReadChLabelRange( rStrm );
        break;
        case EXC_ID_CHVALUERANGE:
            mxValueRange.reset( new XclImpChValueRange( GetChRoot() ) );
            mxValueRange->ReadChValueRange( rStrm );
        break;
        case EXC_ID_CHFORMAT:
            mxFormat.reset( new XclImpChFormat );
            mxFormat->ReadChFormat( rStrm );
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
    // remove invisible grid lines completely
    if( mxMajorGrid.is() && !mxMajorGrid->HasLine() )
        mxMajorGrid.reset();
    if( mxMinorGrid.is() && !mxMinorGrid->HasLine() )
        mxMinorGrid.reset();
}

sal_uInt16 XclImpChAxis::GetFontIndex() const
{
    return mxFont.is() ? mxFont->GetFontIndex() : EXC_FONT_NOTFOUND;
}

Color XclImpChAxis::GetFontColor() const
{
    return mxTick.is() ? mxTick->GetFontColor() : GetFontAutoColor();
}

sal_uInt16 XclImpChAxis::GetRotation() const
{
    return mxTick.is() ? mxTick->GetRotation() : EXC_CHART_AUTOROTATION;
}

void XclImpChAxis::Convert( ScfPropertySet& rAxisProp, ScfPropertySet& rMajorProp, ScfPropertySet& rMinorProp ) const
{
    /*  Axis caption text. Must be set before axis scaling settings. Otherwise
        the secondary Y axis returns to min=0, max=1. For no obvious reason
        the primary Y axis does not do this... */
    if( HasLabels() )
    {
        // font settings from CHFONT record or from default text
        if( mxFont.is() )
            ConvertFontBase( GetChRoot(), rAxisProp );
        else if( const XclImpChText* pDefText = GetChartData().GetDefaultText( EXC_CHDEFTEXT_AXIS ).get() )
            pDefText->ConvertFont( rAxisProp );
    }

    // number format
    if( mxFormat.is() )
        mxFormat->Convert( GetChRoot(), rAxisProp );
    // category axis scaling
    if( mxLabelRange.is() && ((GetAxisType() == EXC_CHAXIS_X) || (GetAxisType() == EXC_CHAXIS_Z)) )
        mxLabelRange->Convert( rAxisProp );
    // value axis scaling
    if( mxValueRange.is() && ((GetAxisType() == EXC_CHAXIS_X) || (GetAxisType() == EXC_CHAXIS_Y)) )
        mxValueRange->Convert( rAxisProp );
    // axis line properties
    if( mxAxisLine.is() )
        mxAxisLine->Convert( GetChRoot(), rAxisProp );
    // axis ticks properties
    if( mxTick.is() )
        mxTick->Convert( rAxisProp );

    // grid lines
    if( mnAxesSetId == EXC_CHAXESSET_PRIMARY )
    {
        if( mxMajorGrid.is() )
            mxMajorGrid->Convert( GetChRoot(), rMajorProp );
        if( mxMinorGrid.is() )
            mxMinorGrid->Convert( GetChRoot(), rMinorProp );
    }
}

void XclImpChAxis::ConvertWall( ScfPropertySet& rPropSet ) const
{
    if( (mnAxesSetId == EXC_CHAXESSET_PRIMARY) && mxWallFrame.is() )
        mxWallFrame->Convert( GetChRoot(), rPropSet );
}

void XclImpChAxis::ReadChAxisLine( XclImpStream& rStrm )
{
    XclImpChLineFormatRef* pxLineFmt = 0;
    XclImpChFrameRef* pxFrame = 0;

    switch( rStrm.ReaduInt16() )
    {
        case EXC_CHAXISLINE_AXISLINE:   pxLineFmt = &mxAxisLine;    break;
        case EXC_CHAXISLINE_MAJORGRID:  pxLineFmt = &mxMajorGrid;   break;
        case EXC_CHAXISLINE_MINORGRID:  pxLineFmt = &mxMinorGrid;   break;
        case EXC_CHAXISLINE_WALLS:      pxFrame   = &mxWallFrame;   break;
    }

    if( pxFrame )
        pxFrame->reset( new XclImpChFrame( EXC_CHOBJLINK_DIAGRAM ) );

    bool bLoop = pxLineFmt || pxFrame;
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
            else if( pxFrame )
            {
                (*pxFrame)->ReadSubRecord( rStrm );
            }
        }
    }
}

// ----------------------------------------------------------------------------

namespace {

inline void lclWriteBoolProperty( ScfPropertySet& rPropSet, const OUString& rPropName, bool bValue )
{
    if( rPropName.getLength() )
        rPropSet.SetBoolProperty( rPropName, bValue );
}

} // namespace

XclImpChAxisHelper::XclImpChAxisHelper(
        XclImpChAxisRef xAxis, XclImpChTextRef xTitle, Reference< XDiagram > xDiagram ) :
    mxAxis( xAxis ),
    mxTitle( xTitle ),
    maDiaProp( xDiagram )
{
}

void XclImpChAxisHelper::Convert( const XclImpChRoot& rRoot )
{
    // axis title
    if( mxTitle.is() )
        mxTitle->ConvertTitle( maTitleProp );

    // axis
    lclWriteBoolProperty( maDiaProp, maNameHasAxis,   mxAxis.is() );
    lclWriteBoolProperty( maDiaProp, maNameHasLabels, mxAxis.is() && mxAxis->HasLabels() );
    lclWriteBoolProperty( maDiaProp, maNameHasMajor,  mxAxis.is() && mxAxis->HasMajorGrid() );
    lclWriteBoolProperty( maDiaProp, maNameHasMinor,  mxAxis.is() && mxAxis->HasMinorGrid() );
    if( mxAxis.is() )
        mxAxis->Convert( maAxisProp, maMajorProp, maMinorProp );
}

// ----------------------------------------------------------------------------

XclImpChAxesSet::XclImpChAxesSet( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclImpChRoot( rRoot ),
    mbAlive( false )
{
    maData.mnAxesSetId = nAxesSetId;
    // insert a default chart format, used to always have a filled map
    XclImpChChartGroupRef xDefFmt( new XclImpChChartGroup( GetChRoot() ) );
    maChartGroups[ xDefFmt->GetGroupIdx() ] = xDefFmt;
}

void XclImpChAxesSet::ReadHeaderRecord( XclImpStream& rStrm )
{
    DBG_ASSERT( !mbAlive, "XclImpChAxesSet::ReadHeaderRecord - axes set read multiple times" );
    rStrm >> maData.mnAxesSetId >> maData.maRect;
    mbAlive = true;
}

void XclImpChAxesSet::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAMEPOS:
            mxPos.reset( new XclImpChFramePos );
            mxPos->ReadChFramePos( rStrm );
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
        case EXC_ID_CHCHARTGROUP:
            ReadChChartGroup( rStrm );
        break;
        case EXC_ID_CHEND:
            // #i47745# missing plot frame -> invisible border and area
            if( !mxPlotFrame )
                mxPlotFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_DIAGRAM ) );
        break;
    }
}

void XclImpChAxesSet::Finalize()
{
    // finalize chart format (checks and sets valid chart type)
    if( GetAxesSetId() == EXC_CHAXESSET_PRIMARY )
        maChartGroups.begin()->second->Finalize();

    // finalize axes
    if( mxXAxis.is() ) mxXAxis->Finalize();
    if( mxYAxis.is() ) mxYAxis->Finalize();
    if( mxZAxis.is() ) mxZAxis->Finalize();

    // finalize axis titles
    XclImpChTextRef xDefText = GetChartData().GetDefaultText( EXC_CHDEFTEXT_AXIS );
    lclFinalizeTitle( mxXAxisTitle, xDefText );
    lclFinalizeTitle( mxYAxisTitle, xDefText );
    lclFinalizeTitle( mxZAxisTitle, xDefText );
}

XclImpChLegendRef XclImpChAxesSet::GetLegend() const
{
    XclImpChLegendRef xLegend;
    for( XclImpChChartGroupMap::const_iterator aIt = maChartGroups.begin(), aEnd = maChartGroups.end(); !xLegend && (aIt != aEnd); ++aIt )
        xLegend = aIt->second->GetLegend();
    return xLegend;
}

void XclImpChAxesSet::CreateDiagram( Reference< XChartDocument > xChartDoc ) const
{
    // create diagram type from first chart group in primary axes set
    if( GetAxesSetId() == EXC_CHAXESSET_PRIMARY )
    {
        // create the diagram object (chart type)
        GetDefChartGroup().CreateDiagram( xChartDoc );
        // axis titles on/off
        ScfPropertySet aDiaProp( xChartDoc->getDiagram() );
        aDiaProp.SetBoolProperty( EXC_CHPROP_HASXAXISTIT, mxXAxisTitle.is() );
        aDiaProp.SetBoolProperty( EXC_CHPROP_HASYAXISTIT, mxYAxisTitle.is() );
        aDiaProp.SetBoolProperty( EXC_CHPROP_HASZAXISTIT, mxZAxisTitle.is() );
    }
}

void XclImpChAxesSet::Convert( Reference< XChartDocument > xChartDoc ) const
{
    Reference< XDiagram > xDiagram( xChartDoc->getDiagram() );
    switch( GetAxesSetId() )
    {
        case EXC_CHAXESSET_PRIMARY:
        {
            // properties of all axes
            ConvertXAxis( xDiagram );
            ConvertYAxis( xDiagram );
            ConvertZAxis( xDiagram );
            // background area formatting
            ConvertBackground( xDiagram );
            // global chart format
            GetDefChartGroup().Convert( xChartDoc );
        }
        break;

        case EXC_CHAXESSET_SECONDARY:
        {
            ConvertSecYAxis( xDiagram );
            // global chart format
            GetDefChartGroup().Convert( xChartDoc );
        }
        break;
    }
}

void XclImpChAxesSet::ReadChAxis( XclImpStream& rStrm )
{
    XclImpChAxisRef xAxis( new XclImpChAxis( GetChRoot(), GetAxesSetId() ) );
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
        mxPlotFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_DIAGRAM ) );
        mxPlotFrame->ReadRecordGroup( rStrm );
    }
}

void XclImpChAxesSet::ReadChChartGroup( XclImpStream& rStrm )
{
    XclImpChChartGroupRef xChartGroup( new XclImpChChartGroup( GetChRoot() ) );
    xChartGroup->ReadRecordGroup( rStrm );
    maChartGroups[ xChartGroup->GetGroupIdx() ] = xChartGroup;
}

void XclImpChAxesSet::ConvertXAxis( Reference< XDiagram > xDiagram ) const
{
    XclImpChAxisHelper aHelper( mxXAxis, mxXAxisTitle, xDiagram );
    Reference< XAxisXSupplier > xAxisSup( xDiagram, UNO_QUERY );
    if( xAxisSup.is() )
    {
        aHelper.maAxisProp.Set( xAxisSup->getXAxis() );
        aHelper.maTitleProp.Set( xAxisSup->getXAxisTitle() );
        aHelper.maMajorProp.Set( xAxisSup->getXMainGrid() );
        aHelper.maMinorProp.Set( xAxisSup->getXHelpGrid() );
        aHelper.maNameHasAxis = EXC_CHPROP_HASXAXIS;
        aHelper.maNameHasLabels = EXC_CHPROP_HASXAXISDESCR;
        aHelper.maNameHasMajor = EXC_CHPROP_HASXAXISGRID;
        aHelper.maNameHasMinor = EXC_CHPROP_HASXAXISHELPGRID;

        // set null date from number formatter to X axis
        if( const Date* pDate = GetFormatter().GetNullDate() )
        {
            Reference< XNumberFormatsSupplier > xNumFmtsSup( xAxisSup->getXAxis(), UNO_QUERY );
            if( xNumFmtsSup.is() )
            {
                ApiDate aApiDate( pDate->GetDay(), pDate->GetMonth(), pDate->GetYear() );
                ScfPropertySet aPropSet( xNumFmtsSup->getNumberFormatSettings() );
                aPropSet.SetProperty( EXC_CHPROP_NULLDATE, aApiDate );
            }
        }
    }
    aHelper.Convert( GetChRoot() );
}

void XclImpChAxesSet::ConvertYAxis( Reference< XDiagram > xDiagram ) const
{
    XclImpChAxisHelper aHelper( mxYAxis, mxYAxisTitle, xDiagram );
    Reference< XAxisYSupplier > xAxisSup( xDiagram, UNO_QUERY );
    if( xAxisSup.is() )
    {
        aHelper.maAxisProp.Set( xAxisSup->getYAxis() );
        aHelper.maTitleProp.Set( xAxisSup->getYAxisTitle() );
        aHelper.maMajorProp.Set( xAxisSup->getYMainGrid() );
        aHelper.maMinorProp.Set( xAxisSup->getYHelpGrid() );
        aHelper.maNameHasAxis = EXC_CHPROP_HASYAXIS;
        aHelper.maNameHasLabels = EXC_CHPROP_HASYAXISDESCR;
        aHelper.maNameHasMajor = EXC_CHPROP_HASYAXISGRID;
        aHelper.maNameHasMinor = EXC_CHPROP_HASYAXISHELPGRID;
    }
    aHelper.Convert( GetChRoot() );
}

void XclImpChAxesSet::ConvertSecYAxis( Reference< XDiagram > xDiagram ) const
{
    XclImpChAxisHelper aHelper( mxYAxis, mxYAxisTitle, xDiagram );
    Reference< XTwoAxisYSupplier > xAxisSup( xDiagram, UNO_QUERY );
    if( xAxisSup.is() )
    {
        aHelper.maAxisProp.Set( xAxisSup->getSecondaryYAxis() );
        aHelper.maNameHasAxis = EXC_CHPROP_HASSECYAXIS;
        aHelper.maNameHasLabels = EXC_CHPROP_HASSECYAXISDESCR;
    }
    aHelper.Convert( GetChRoot() );
}

void XclImpChAxesSet::ConvertZAxis( Reference< XDiagram > xDiagram ) const
{
    XclImpChAxisHelper aHelper( mxZAxis, mxZAxisTitle, xDiagram );
    Reference< XAxisZSupplier > xAxisSup( xDiagram, UNO_QUERY );
    if( xAxisSup.is() )
    {
        aHelper.maAxisProp.Set( xAxisSup->getZAxis() );
        aHelper.maTitleProp.Set( xAxisSup->getZAxisTitle() );
        aHelper.maMajorProp.Set( xAxisSup->getZMainGrid() );
        aHelper.maMinorProp.Set( xAxisSup->getZHelpGrid() );
        aHelper.maNameHasAxis = EXC_CHPROP_HASZAXIS;
        aHelper.maNameHasLabels = EXC_CHPROP_HASZAXISDESCR;
        aHelper.maNameHasMajor = EXC_CHPROP_HASZAXISGRID;
        aHelper.maNameHasMinor = EXC_CHPROP_HASZAXISHELPGRID;
    }
    aHelper.Convert( GetChRoot() );
}

void XclImpChAxesSet::ConvertBackground( Reference< XDiagram > xDiagram ) const
{
    Reference< X3DDisplay > x3DDisplay( xDiagram, UNO_QUERY );
    if( x3DDisplay.is() )
    {
        ScfPropertySet aWallProp( x3DDisplay->getWall() );
        ScfPropertySet aFloorProp( x3DDisplay->getFloor() );

        if( GetDefChartGroup().Is3dChart() )
        {
            // wall/floor formatting (3D charts)
            if( mxXAxis.is() )
                mxXAxis->ConvertWall( aWallProp );
            if( mxYAxis.is() )
                mxYAxis->ConvertWall( aFloorProp );
        }
        else
        {
            // background formatting (2D charts)
            if( mxPlotFrame.is() )
                mxPlotFrame->Convert( GetChRoot(), aWallProp );
        }
    }
}

// The chart object ===========================================================

XclImpChChart::XclImpChChart( const XclImpRoot& rRoot ) :
    XclImpChRoot( rRoot, this ),
    meOrient( EXC_CHORIENT_EMPTY ),
    mbHasValue( false ),
    mbHasCateg( false ),
    mbHasTitle( false )
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
    meOrient = EXC_CHORIENT_EMPTY;
}

void XclImpChChart::ReadSubRecord( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_CHFRAME:
            mxFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_BACKGROUND ) );
            mxFrame->ReadRecordGroup( rStrm );
        break;
        case EXC_ID_CHSERIES:
            ReadChSeries( rStrm );
        break;
        case EXC_ID_CHPROPERTIES:
            rStrm >> maProps.mnFlags >> maProps.mnEmptyMode;
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
            Finalize();
        break;
    }
}

void XclImpChChart::ReadChDataFormat( XclImpStream& rStrm )
{
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->ReadRecordGroup( rStrm );
    if( xDataFmt->GetPointPos().mnSeriesIdx <= EXC_CHSERIES_MAXSERIES )
    {
        XclImpChDataFormatRef& rxMapFmt = maDataFmts[ xDataFmt->GetPointPos() ];
        /*  Do not overwrite existing data format group, Excel always uses the
            first data format group occuring in any CHSERIES group. */
        if( !rxMapFmt )
            rxMapFmt = xDataFmt;
    }
}

bool XclImpChChart::UpdateScatterXRanges()
{
    bool bValid = mbHasValue && mbHasCateg;

    /*  In XY charts, all series must have valid categories (X values),
        and all categories must be equal. Otherwise this chart becomes
        a line chart later (#92909#). */
    if( bValid && !maValidSeries.empty() )
    {
        XclImpChSeriesVec::iterator aBeg = maValidSeries.begin(), aEnd = maValidSeries.end(), aIt;
        for( aIt = aBeg + 1; bValid && (aIt != aEnd); ++aIt )
            bValid = (*aBeg)->HasEqualCategories( **aIt );

        /*  If ranges for X values are valid, reset mbValidCateg to indicate
            the chart does not contain categoties. Otherwise the resulting
            XY chart contains a category range *and* a range for X values. */
        if( bValid )
            mbHasCateg = false;
    }

    return bValid;
}

ScRangeListRef XclImpChChart::GetSourceData() const
{
    ScRangeListRef xScRanges( new ScRangeList );
    if( !maValidSeries.empty() )
    {
        // start with category ranges
        maValidSeries.front()->JoinCategoryRanges( *xScRanges );
        // join all value ranges with titles, if enabled
        for( XclImpChSeriesVec::const_iterator aIt = maValidSeries.begin(), aEnd = maValidSeries.end(); aIt != aEnd; ++aIt )
            (*aIt)->JoinValueRanges( *xScRanges );
    }
    return xScRanges;
}

bool XclImpChChart::HasHeaderColumn() const
{
    return
        (mbHasCateg && (meOrient == EXC_CHORIENT_VERTICAL)) ||
        (mbHasTitle && (meOrient == EXC_CHORIENT_HORIZONTAL));
}

bool XclImpChChart::HasHeaderRow() const
{
    return
        (mbHasTitle && (meOrient == EXC_CHORIENT_VERTICAL)) ||
        (mbHasCateg && (meOrient == EXC_CHORIENT_HORIZONTAL));
}

bool XclImpChChart::Is3dChart() const
{
    return mxPrimAxesSet->GetDefChartGroup().Is3dChart();
}

bool XclImpChChart::IsStockChart() const
{
    const XclImpChChartGroup& rPrimGroup = mxPrimAxesSet->GetDefChartGroup();
    const XclImpChChartGroup& rSecnGroup = mxSecnAxesSet->GetDefChartGroup();

    bool bHasPrim = mxPrimAxesSet->IsAlive();
    bool bPrimHiLo = bHasPrim && (rPrimGroup.GetChartTypeId() == EXC_ID_CHLINE) && rPrimGroup.HasHiLoLine();
    bool bPrimDrop = bHasPrim && (rPrimGroup.GetChartTypeId() == EXC_ID_CHLINE) && rPrimGroup.HasDropBars();
    bool bPrimBar = bHasPrim && (rPrimGroup.GetChartTypeId() == EXC_ID_CHBAR);

    bool bHasSecn = mxSecnAxesSet->IsAlive();
    bool bSecnHiLo = bHasSecn && (rSecnGroup.GetChartTypeId() == EXC_ID_CHLINE) && rSecnGroup.HasHiLoLine();
    bool bSecnDrop = bHasSecn && (rSecnGroup.GetChartTypeId() == EXC_ID_CHLINE) && rSecnGroup.HasDropBars();
    bool bSecnBar = bHasSecn && (rSecnGroup.GetChartTypeId() == EXC_ID_CHBAR);

    size_t nSeriesCnt = maValidSeries.size();

    return
        (bPrimHiLo && (nSeriesCnt == (bSecnBar ? (bPrimDrop ? 5 : 4) : (bPrimDrop ? 4 : 3)))) ||
        (bSecnHiLo && (nSeriesCnt == (bPrimBar ? (bSecnDrop ? 5 : 4) : (bSecnDrop ? 4 : 3))));
}

bool XclImpChChart::HasAxesSet( sal_uInt16 nAxesSetId ) const
{
    return
        ((nAxesSetId == EXC_CHAXESSET_PRIMARY) && mxPrimAxesSet->IsAlive()) ||
        ((nAxesSetId == EXC_CHAXESSET_SECONDARY) && mxSecnAxesSet->IsAlive());
}

XclImpChAxesSet& XclImpChChart::GetChartGroupAxesSet( sal_uInt16 nGroupIdx ) const
{
    return mxSecnAxesSet->HasChartGroup( nGroupIdx ) ? *mxSecnAxesSet : *mxPrimAxesSet;
}

sal_uInt16 XclImpChChart::GetChartGroupAxesSetId( sal_uInt16 nGroupIdx ) const
{
    return GetChartGroupAxesSet( nGroupIdx ).GetAxesSetId();
}

XclImpChChartGroupRef XclImpChChart::GetChartGroup( sal_uInt16 nGroupIdx ) const
{
    return GetChartGroupAxesSet( nGroupIdx ).GetChartGroup( nGroupIdx );
}

XclImpChChartGroup& XclImpChChart::GetDefChartGroup( sal_uInt16 nGroupIdx ) const
{
    return GetChartGroupAxesSet( nGroupIdx ).GetDefChartGroup();
}

sal_uInt16 XclImpChChart::GetChartGroupSeriesCount( sal_uInt16 nGroupIdx ) const
{
    /*  Count all series that are contained in the same axes set that contains
        the chart group with the passed index nGroupIdx.
        Example: How many series for chart group 2? Primary axes set may
        contain chart groups 0 and 1, secondary axes set may contain chart
        groups 2 and 3. Chart group 2 is contained in secondary axes set, so
        this function will return the number of series contained in secondary
        axes set, i.e. all series with group index 2 and 3. */
    const XclImpChAxesSet& rAxesSet = GetChartGroupAxesSet( nGroupIdx );
    sal_uInt16 nSeriesCnt = 0;
    for( XclImpChSeriesVec::const_iterator aIt = maValidSeries.begin(), aEnd = maValidSeries.end(); aIt != aEnd; ++aIt )
        if( rAxesSet.HasChartGroup( (*aIt)->GetGroupIdx() ) )
            ++nSeriesCnt;
    return nSeriesCnt;
}

XclImpChTextRef XclImpChChart::GetDefaultText( sal_uInt16 nTextId ) const
{
    return maDefTexts.get( nTextId );
}

bool XclImpChChart::HasAnySeriesLine() const
{
    bool bHasLine = false;
    for( XclImpChSeriesVec::const_iterator aIt = maSeries.begin(), aEnd = maSeries.end(); !bHasLine && (aIt != aEnd); ++aIt )
        bHasLine = (*aIt)->HasLine();
    return bHasLine;
}

void XclImpChChart::Convert( Reference< XChartDocument > xChartDoc, ScfProgressBar& rProgress ) const
{
    // initialize conversion (locks the model to suppress any internal updates)
    InitConversion( xChartDoc );

    /*  chart type, creates the XDiagram in the chart document (must be called
        first), 3d mode, axes on/off, axis titles on/off */
    mxPrimAxesSet->CreateDiagram( xChartDoc );

    // chart title on/off, legend on/off
    ScfPropertySet aChartProp( xChartDoc );
    aChartProp.SetBoolProperty( EXC_CHPROP_HASMAINTIT, mxTitle.is() );
    aChartProp.SetBoolProperty( EXC_CHPROP_HASLEGEND, mxLegend.is() );

    // source data orientation
    if( mbHasValue )
    {
        namespace cssc = ::com::sun::star::chart;
        cssc::ChartDataRowSource eSource = (meOrient == EXC_CHORIENT_VERTICAL) ?
            cssc::ChartDataRowSource_COLUMNS : cssc::ChartDataRowSource_ROWS;
        ScfPropertySet aDiaProp( xChartDoc->getDiagram() );
        aDiaProp.SetProperty( EXC_CHPROP_DATAROWSOURCE, eSource );
    }

    /*  Unlock/lock the model once. This will create internal objects (e.g.
        axes, legend, titles) to be able to set their properties. */
    FinishConversion( rProgress );
    InitConversion( xChartDoc );

    // axes settings
    mxPrimAxesSet->Convert( xChartDoc );
    mxSecnAxesSet->Convert( xChartDoc );

    // legend properties
    if( mxLegend.is() )
    {
        ScfPropertySet aLegProp( xChartDoc->getLegend() );
        mxLegend->Convert( aLegProp );
    }

    // series formatting and order
    ConvertSeries( xChartDoc );
    ConvertSeriesOrder( xChartDoc );

    // chart frame and title
    if( mxFrame.is() )
    {
        ScfPropertySet aFrameProp( xChartDoc->getArea() );
        mxFrame->Convert( GetChRoot(), aFrameProp );
    }
    if( mxTitle.is() )
    {
        ScfPropertySet aTitleProp( xChartDoc->getTitle() );
        mxTitle->ConvertTitle( aTitleProp );
    }

    // unlock the model
    FinishConversion( rProgress );
}

void XclImpChChart::ReadChSeries( XclImpStream& rStrm )
{
    sal_uInt16 nNewSeriesIdx = static_cast< sal_uInt16 >( maSeries.size() );
    XclImpChSeriesRef xSeries( new XclImpChSeries( GetChRoot(), nNewSeriesIdx ) );
    xSeries->ReadRecordGroup( rStrm );
    maSeries.push_back( xSeries );
}

void XclImpChChart::ReadChDefaultText( XclImpStream& rStrm )
{
    sal_uInt16 nTextId = rStrm.ReaduInt16();
    if( (rStrm.GetNextRecId() == EXC_ID_CHTEXT) && rStrm.StartNextRecord() )
    {
        XclImpChTextRef xText( new XclImpChText( GetChRoot() ) );
        xText->ReadRecordGroup( rStrm );
        maDefTexts[ nTextId ] = xText;
    }
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
    // axes sets, updates chart group default formats -> must be called before FinalizeDataFormats()
    mxPrimAxesSet->Finalize();
    mxSecnAxesSet->Finalize();
    // formatting of all series
    FinalizeDataFormats();
    // #i49218# legend may be attached to primary or secondary axes set
    mxLegend = mxPrimAxesSet->GetLegend();
    if( !mxLegend )
        mxLegend = mxSecnAxesSet->GetLegend();
    if( mxLegend.is() )
        mxLegend->Finalize();
    // #i47745# missing frame -> invisible border and area
    if( !mxFrame )
        mxFrame.reset( new XclImpChFrame( EXC_CHOBJLINK_BACKGROUND ) );
    // chart title (may reset mxTitle, if it does not contain a string)
    lclFinalizeTitle( mxTitle, GetDefaultText( EXC_CHDEFTEXT_TITLE ) );
}

void XclImpChChart::FinalizeSeries()
{
    maValidSeries.clear();

    /*  Check series with valid value range. If a series has valid value
        ranges, it will be compared with the value ranges of the first valid
        series. The CheckAndUpdateOrientation() function updates the orientaion
        of the first series, if it is EXC_CHORIENT_SINGLE. After the loop, the
        first series will contain the final orientation of the chart source
        data. Store all valid series in aUsedSeries. It will be used to fill
        maValidSeries after additional checks (e.g. double main position). */
    XclImpChSeriesVec aUsedSeries;
    for( XclImpChSeriesVec::iterator aSIt = maSeries.begin(), aSEnd = maSeries.end(); aSIt != aSEnd; ++aSIt )
    {
        XclImpChSeriesRef xSeries = *aSIt;
        if( xSeries->HasParentSeries() )
        {
            /*  Process child series (trend lines and error bars). Data of
                child series will be set at the connected parent series. */
            if( xSeries->GetParentIdx() < maSeries.size() )
                maSeries[ xSeries->GetParentIdx() ]->AddChildSeries( *xSeries );
        }
        else if( xSeries->HasValidValues() )
        {
            /*  Compare following valid series with first valid series, push
                all valid series into the temporary vector. */
            if( aUsedSeries.empty() || aUsedSeries.front()->CheckAndUpdateOrientation( *xSeries ) )
                aUsedSeries.push_back( xSeries );
        }
    }

    // nothing to do, if no valid series found
    if( !aUsedSeries.empty() )
    {
        /*  Get orientation from first series in the source list. The loop
            above has updated it according to all valid series in the chart. */
        meOrient = aUsedSeries.front()->GetOrientation();

        /*  Still single orientation? This happens only, if the chart contains
            only one series with exactly one value, and without title and
            category. Just change it to vertical in this case. */
        if( meOrient == EXC_CHORIENT_SINGLE )
            meOrient = EXC_CHORIENT_VERTICAL;

        /*  This set will contain all used main positions of valid series. It
            will be used to find series that are based on the same row or
            column data (this is not allowed in OOChart). */
        typedef ::std::set< XclImpChMainPos > XclImpChMainPosSet;
        XclImpChMainPosSet aUsedPosSet;

        /*  Update orientation of all series, remove invalid titles and
            categories. Find and remove series that use the same main position. */
        for( XclImpChSeriesVec::iterator aUIt = aUsedSeries.begin(), aUEnd = aUsedSeries.end(); aUIt != aUEnd; ++aUIt )
        {
            XclImpChSeriesRef xSeries = *aUIt;
            // set final orientation, update main position and intervals
            xSeries->SetFinalOrientation( meOrient );
            // check if main position is already used by another series
            if( aUsedPosSet.insert( xSeries->GetMainPos() ).second )
                maValidSeries.push_back( xSeries );
        }

        // nothing to do, if no valid series found
        if( !maValidSeries.empty() )
        {
            // first series really used in the OOChart
            XclImpChSeries& rFirstSeries = *maValidSeries.front();
            // vector iterators
            XclImpChSeriesVec::iterator aVBeg = maValidSeries.begin(), aVEnd = maValidSeries.end(), aVIt;

            /*  Find a category that is located before the leftmost/topmost
                series. Categories in OOChart must be located in the very first
                row or column. The UpdateCategories() function will set the
                first valid category range at the first series. rFirstSeries is
                the first series in the Excel list of series, but it may not be
                the leftmost or topmost series. If it contains categories that
                are not valid for the leftmost/topmost series, these categories
                have to be removed. Therefore loop over *all* series. */
            for( aVIt = aVBeg + 1; aVIt != aVEnd; ++aVIt )
                rFirstSeries.UpdateCategories( **aVIt );

            /*  Look at titles of all series. If they do not match the title of
                the first series, remove its title (first series is used as
                indicator for existing titles later). */
            for( aVIt = aVBeg + 1; rFirstSeries.HasValidTitle() && (aVIt != aVEnd); ++aVIt )
                rFirstSeries.UpdateTitle( **aVIt );

            /*  If series have titles, add them to the value ranges and to the
                category range (#i1508#) to get a rectangular source range. */
            if( rFirstSeries.HasValidTitle() )
                for( aVIt = aVBeg; aVIt != aVEnd; ++aVIt )
                    (*aVIt)->AddTitleToValues();

            // store valid components of source data
            mbHasValue = true;
            mbHasCateg = rFirstSeries.HasValidCategories();
            mbHasTitle = rFirstSeries.HasValidTitle();
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
    for( XclImpChDataFormatMap::const_iterator aMIt = maDataFmts.begin(), aMEnd = maDataFmts.end(); aMIt != aMEnd; ++aMIt )
    {
        sal_uInt16 nSeriesIdx = aMIt->first.mnSeriesIdx;
        if( nSeriesIdx < maSeries.size() )
            maSeries[ nSeriesIdx ]->SetDataFormat( aMIt->second );
    }

    /*  #i51639# (part 2): Finalize data formats of all series. This adds for
        example missing CHDATAFORMAT groups for entire series that are needed
        for automatic colors of lines and areas. */
    for( XclImpChSeriesVec::iterator aVIt = maSeries.begin(), aVEnd = maSeries.end(); aVIt != aVEnd; ++aVIt )
        (*aVIt)->FinalizeDataFormats();
}

void XclImpChChart::ConvertSeries( Reference< XChartDocument > xChartDoc ) const
{
    if( !maValidSeries.empty() )
    {
        XclImpChSeriesVec::const_iterator aVBeg = maValidSeries.begin(), aVEnd = maValidSeries.end(), aVIt;

        // additional diagram properties, must be set before series formatting
        ScfPropertySet aDiaProp( xChartDoc->getDiagram() );

        bool bHasMarker = false;
        bool bHasSpline = false;
        for( aVIt = aVBeg; aVIt != aVEnd; ++aVIt )
        {
            const XclImpChSeries& rSeries = **aVIt;
            bHasMarker |= rSeries.HasMarker();
            bHasSpline |= rSeries.HasSpline();
        }

        // #i19559# set existence of symbols for correct display of chart type
        using namespace ::com::sun::star::chart::ChartSymbolType;
        aDiaProp.SetProperty( EXC_CHPROP_SYMBOLTYPE, bHasMarker ? AUTO : NONE );

        // set spline type at diagram (not possible to be set for single series)
        sal_Int32 nApiSpline = bHasSpline ? 1 : 0;  // no constants available...
        aDiaProp.SetProperty( EXC_CHPROP_SPLINETYPE, nApiSpline );

        // series formatting
        for( aVIt = aVBeg; aVIt != aVEnd; ++aVIt )
            (*aVIt)->Convert( xChartDoc, aVIt - aVBeg );
    }
}

void XclImpChChart::ConvertSeriesOrder( Reference< XChartDocument > xChartDoc ) const
{
    /*  Reorder the series according to source data from Excel. The properties
        'TranslatedColumns' or 'TranslatedRows' only reorder the source data,
        but leave the series formatting unchanged. Therefore the previous part
        of the filter has used maValidSeries to set the series formatting.
        maValidSeries contains the formatting in the same order displayed by
        Excel and Calc.
        Example: An Excel chart contains 3 series:
        - Series 1: Source data in column D, formatted red.
        - Series 2: Source data in column B, formatted green.
        - Series 3: Source data in column C, formatted blue.
        The import filter creates rectangular source range from column B to D.
        - Series 1 (which currently corresponds to column B) is set to red.
        - Series 2 (which currently corresponds to column C) is set to green.
        - Series 3 (which currently corresponds to column D) is set to blue.
        Now this function reorders the series to the order D, B, C, according
        to the Excel source data. But the OOChart does not move the series
        formatting. So the first visible series will be based on column D but
        still will be formatted red, and so on.
     */
    if( !maValidSeries.empty() )
    {
        // find correct property name
        OUString aPropName;
        switch( meOrient )
        {
            case EXC_CHORIENT_VERTICAL:     aPropName = EXC_CHPROP_TRANSLATEDCOLS;  break;
            case EXC_CHORIENT_HORIZONTAL:   aPropName = EXC_CHPROP_TRANSLATEDROWS;  break;
        }

        if( aPropName.getLength() )
        {
            XclImpChSeriesVec aOrderedSeries( maValidSeries );
            /*  Sort aOrderedSeries. It will be ordered by main position (first
                by sheet index, then all columns from left to right in vertical
                series, or all rows from top to bottom in horizontal series). */
            ::std::sort( aOrderedSeries.begin(), aOrderedSeries.end(), XclImpChSeriesPred_Order() );
            XclImpChSeriesVec::const_iterator aOBeg = aOrderedSeries.begin();
            XclImpChSeriesVec::const_iterator aOEnd = aOrderedSeries.end();

            /*  Go through the original series maValidSeries and search all
                valid series in the ordered vector aOrderedSeries. Remember
                the vector index of each series. The resulting sequence
                specifies the drawing order of the series. */
            ScfInt32Vec aOrderedIndexes;
            aOrderedIndexes.reserve( aOrderedSeries.size() );
            for( XclImpChSeriesVec::const_iterator aVIt = maValidSeries.begin(), aVEnd = maValidSeries.end(); aVIt != aVEnd; ++aVIt )
            {
                // find the series in the aOrderedSeries vector
                XclImpChSeriesVec::const_iterator aOIt = ::std::find( aOBeg, aOEnd, *aVIt );
                if( aOIt != aOEnd )
                    aOrderedIndexes.push_back( static_cast< sal_Int32 >( aOIt - aOBeg ) );
            }
            DBG_ASSERT( aOrderedIndexes.size() == aOrderedSeries.size(),
                "XclImpChChart::ConvertSeriesOrder - error while calculating series drawing order" );
            if( aOrderedIndexes.size() == aOrderedSeries.size() )
            {
                // is any series out of order?
                ScfInt32Vec::const_iterator aOIdxBeg = aOrderedIndexes.begin();
                ScfInt32Vec::const_iterator aOIdxEnd = aOrderedIndexes.end();
                if( ::std::adjacent_find( aOIdxBeg, aOIdxEnd, ::std::greater_equal< sal_Int32 >() ) != aOIdxEnd )
                {
                    // create the UNO sequence and set the property
                    ScfPropertySet aChartProp( xChartDoc );
                    aChartProp.SetProperty( aPropName, ScfApiHelper::VectorToSequence( aOrderedIndexes ) );
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------

XclImpChart::XclImpChart( const XclImpRoot& rRoot, bool bOwnTab ) :
    XclImpRoot( rRoot ),
    mbOwnTab( bOwnTab )
{
}

void XclImpChart::ReadChartSubStream( XclImpStream& rStrm )
{
    XclImpPageSettings& rPageSett = GetPageSettings();
    XclImpTabViewSettings& rTabViewSett = GetTabViewSettings();

    bool bLoop = true;
    while( bLoop && rStrm.StartNextRecord() )
    {
        sal_uInt16 nRecId = rStrm.GetRecId();
        bLoop = nRecId != EXC_ID_EOF;

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
            case EXC_ID_BITMAP:         rPageSett.ReadBitmap( rStrm );          break;

            case EXC_ID_WINDOW2:        rTabViewSett.ReadWindow2( rStrm, true );break;
            case EXC_ID_SCL:            rTabViewSett.ReadScl( rStrm );          break;
        }

        switch( rStrm.GetRecId() )
        {
            // #i31882# ignore embedded chart objects
            case EXC_ID2_BOF:
            case EXC_ID3_BOF:
            case EXC_ID4_BOF:
            case EXC_ID5_BOF:           XclTools::SkipSubStream( rStrm );       break;

            case EXC_ID_CHCHART:        ReadChChart( rStrm );                   break;
            case EXC_ID_OBJ:            GetTracer().TraceChartEmbeddedObj();    break;
            case EXC_ID8_CHPIVOTREF:    GetTracer().TracePivotChartExists();    break;
        }
    }
}

ScRangeListRef XclImpChart::GetSourceData() const
{
    if( mxChartData.is() )
        return mxChartData->GetSourceData();
    return ScRangeListRef();
}

bool XclImpChart::HasHeaderColumn() const
{
    return mxChartData.is() && mxChartData->HasHeaderColumn();
}

bool XclImpChart::HasHeaderRow() const
{
    return mxChartData.is() && mxChartData->HasHeaderRow();
}

sal_Size XclImpChart::GetProgressSize() const
{
    return mxChartData.is() ? mxChartData->GetProgressSize() : 0;
}

void XclImpChart::Convert( Reference< XInterface > xComp, ScfProgressBar& rProgress ) const
{
    Reference< XChartDocument > xChartDoc( xComp, UNO_QUERY );
    if( mxChartData.is() && xChartDoc.is() )
        mxChartData->Convert( xChartDoc, rProgress );
}

void XclImpChart::ReadChChart( XclImpStream& rStrm )
{
    mxChartData.reset( new XclImpChChart( GetRoot() ) );
    mxChartData->ReadRecordGroup( rStrm );
}

// ============================================================================

