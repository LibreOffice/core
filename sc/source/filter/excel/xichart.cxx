/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xichart.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 14:51:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "xichart.hxx"

#include <algorithm>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/AxisPosition.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <sfx2/objsh.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "rangeutl.hxx"
#include "fprogressbar.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xiformula.hxx"
#include "xistyle.hxx"
#include "xipage.hxx"
#include "xiview.hxx"
#include "xiescher.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::drawing::CameraGeometry;
using ::com::sun::star::drawing::Direction3D;
using ::com::sun::star::drawing::HomogenMatrix;
using ::com::sun::star::drawing::HomogenMatrixLine;
using ::com::sun::star::drawing::Position3D;

using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XCoordinateSystemContainer;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XChartTypeContainer;
using ::com::sun::star::chart2::XChartType;
using ::com::sun::star::chart2::XDataSeriesContainer;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XRegressionCurve;
using ::com::sun::star::chart2::XRegressionCurveContainer;
using ::com::sun::star::chart2::XAxis;
using ::com::sun::star::chart2::XScaling;
using ::com::sun::star::chart2::ScaleData;
using ::com::sun::star::chart2::IncrementData;
using ::com::sun::star::chart2::SubIncrement;
using ::com::sun::star::chart2::XLegend;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::XTitle;
using ::com::sun::star::chart2::XFormattedString;

using ::com::sun::star::chart2::data::XDataProvider;
using ::com::sun::star::chart2::data::XDataReceiver;
using ::com::sun::star::chart2::data::XDataSink;
using ::com::sun::star::chart2::data::XLabeledDataSequence;
using ::com::sun::star::chart2::data::XDataSequence;

// Helpers ====================================================================

namespace {

XclImpStream& operator>>( XclImpStream& rStrm, XclChRectangle& rRect )
{
    return rStrm >> rRect.mnX >> rRect.mnY >> rRect.mnWidth >> rRect.mnHeight;
}

template< typename Type >
void lclSetValueOrClearAny( Any& rAny, const Type& rValue, bool bClear )
{
    if( bClear )
        rAny.clear();
    else
        rAny <<= rValue;
}

void lclSetScaledValueOrClearAny( Any& rAny, double fValue, Reference< XScaling > xScaling, bool bClear )
{
    if( !bClear && xScaling.is() )
        fValue = xScaling->doScaling( fValue );
    lclSetValueOrClearAny( rAny, fValue, bClear );
}

} // namespace

// Common =====================================================================

/** Stores global data needed in various classes of the Chart import filter. */
class XclImpChRootData : public XclChRootData
{
public:
    explicit            XclImpChRootData( XclImpChChart* pChartData );

    /** Returns a reference to the parent chart data object. */
    inline XclImpChChart& GetChartData() const { return *mpChartData; }

private:
    XclImpChChart*      mpChartData;            /// Pointer to the chart data object.
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
    return mxChData->GetChartData();
}

const XclChTypeInfo& XclImpChRoot::GetChartTypeInfo( XclChTypeId eType ) const
{
    return mxChData->GetTypeInfoProvider().GetTypeInfo( eType );
}

const XclChTypeInfo& XclImpChRoot::GetChartTypeInfo( sal_uInt16 nRecId ) const
{
    return mxChData->GetTypeInfoProvider().GetTypeInfoFromRecId( nRecId );
}

const XclChFormatInfo& XclImpChRoot::GetFormatInfo( XclChObjectType eObjType ) const
{
    return mxChData->GetFormatInfoProvider().GetFormatInfo( eObjType );
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

void XclImpChRoot::InitConversion( Reference< XChartDocument > xChartDoc ) const
{
    // create formatting object tables
    mxChData->InitConversion( xChartDoc );

    // lock the model to suppress any internal updates
    Reference< XModel > xModel( xChartDoc, UNO_QUERY );
    if( xModel.is() )
        xModel->lockControllers();

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

void XclImpChRoot::FinishConversion( ScfProgressBar& rProgress ) const
{
    rProgress.Progress( EXC_CHART_PROGRESS_SIZE );
    // unlock the model
    Reference< XModel > xModel( mxChData->GetChartDoc(), UNO_QUERY );
    if( xModel.is() )
        xModel->unlockControllers();
    rProgress.Progress( EXC_CHART_PROGRESS_SIZE );

    mxChData->FinishConversion();
}

Reference< XDataProvider > XclImpChRoot::GetDataProvider() const
{
    return mxChData->GetChartDoc()->getDataProvider();
}

void XclImpChRoot::ConvertLineFormat( ScfPropertySet& rPropSet,
        const XclChLineFormat& rLineFmt, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteLineProperties(
        rPropSet, mxChData->GetLineDashTable(), rLineFmt, ePropMode );
}

void XclImpChRoot::ConvertAreaFormat( ScfPropertySet& rPropSet,
        const XclChAreaFormat& rAreaFmt, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteAreaProperties( rPropSet, rAreaFmt, ePropMode );
}

void XclImpChRoot::ConvertEscherFormat( ScfPropertySet& rPropSet,
        const XclChEscherFormat& rEscherFmt, const XclChPicFormat& rPicFmt,
        XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().WriteEscherProperties( rPropSet,
        mxChData->GetGradientTable(), mxChData->GetBitmapTable(), rEscherFmt, rPicFmt, ePropMode );
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

// Frame formatting ===========================================================

void XclImpChFramePos::ReadChFramePos( XclImpStream& rStrm )
{
    rStrm >> maData.mnObjType >> maData.mnSizeMode >> maData.maRect;
}

// ----------------------------------------------------------------------------

void XclImpChLineFormat::ReadChLineFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maColor >> maData.mnPattern >> maData.mnWeight >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
        // #116397# BIFF8: index into palette used instead of RGB data
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

// ----------------------------------------------------------------------------

void XclImpChAreaFormat::ReadChAreaFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maPattColor >> maData.maBackColor >> maData.mnPattern >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        // #116397# BIFF8: index into palette used instead of RGB data
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

// ----------------------------------------------------------------------------

XclImpChEscherFormat::XclImpChEscherFormat( const XclImpRoot& rRoot )
{
    maData.mxItemSet.reset(
        new SfxItemSet( rRoot.GetDoc().GetDrawLayer()->GetItemPool() ) );
}

void XclImpChEscherFormat::ReadHeaderRecord( XclImpStream& rStrm )
{
    // read from stream - CHESCHERFORMAT uses own ID for record continuation
    XclImpEscherPropSet aPropSet( rStrm.GetRoot() );
    rStrm.ResetRecord( true, rStrm.GetRecId() );
    rStrm >> aPropSet;
    // get the data
    aPropSet.FillToItemSet( *maData.mxItemSet );
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

void XclImpChEscherFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType ) const
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    rRoot.ConvertEscherFormat( rPropSet, maData, maPicFmt, rFmtInfo.mePropMode );
}

// ----------------------------------------------------------------------------

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
            DBG_ERRORFILE( "XclImpChFrameBase::XclImpChFrameBase - unknown frame type" );
    }
}

XclImpChFrameBase::XclImpChFrameBase(
        const XclChLineFormat& rLineFmt, const XclChAreaFormat& rAreaFmt ) :
    mxLineFmt( new XclImpChLineFormat( rLineFmt ) ),
    mxAreaFmt( new XclImpChAreaFormat( rAreaFmt ) )
{
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

void XclImpChFrameBase::ConvertFrameBase( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, XclChObjectType eObjType, sal_uInt16 nFormatIdx ) const
{
    // line format
    if( mxLineFmt.is() )
        mxLineFmt->Convert( rRoot, rPropSet, eObjType, nFormatIdx );
    // area format (only for frame objects)
    if( rRoot.GetFormatInfo( eObjType ).mbIsFrame )
    {
        // CHESCHERFORMAT overrides CHAREAFORMAT (even if it is auto)
        if( mxEscherFmt.is() )
            mxEscherFmt->Convert( rRoot, rPropSet, eObjType );
        else if( mxAreaFmt.is() )
            mxAreaFmt->Convert( rRoot, rPropSet, eObjType, nFormatIdx );
    }
}

// ----------------------------------------------------------------------------

XclImpChFrame::XclImpChFrame( const XclImpChRoot& rRoot, XclChObjectType eObjType ) :
    XclImpChFrameBase( rRoot.GetFormatInfo( eObjType ) ),
    XclImpChRoot( rRoot ),
    meObjType( eObjType )
{
}

XclImpChFrame::XclImpChFrame( const XclImpChRoot& rRoot,
        const XclChLineFormat& rLineFmt, const XclChAreaFormat& rAreaFmt,
        XclChObjectType eObjType ) :
    XclImpChFrameBase( rLineFmt, rAreaFmt ),
    XclImpChRoot( rRoot ),
    meObjType( eObjType )
{
}

void XclImpChFrame::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.mnFormat >> maData.mnFlags;
}

void XclImpChFrame::Convert( ScfPropertySet& rPropSet ) const
{
    ConvertFrameBase( GetChRoot(), rPropSet, meObjType );
}

// Source links ===============================================================

XclImpChSourceLink::XclImpChSourceLink( const XclImpChRoot& rRoot ) :
    XclImpChRoot( rRoot )
{
}

void XclImpChSourceLink::ReadChSourceLink( XclImpStream& rStrm )
{
    rStrm   >> maData.mnDestType
            >> maData.mnLinkType
            >> maData.mnFlags
            >> maData.mnNumFmtIdx;

    maScRanges.Clear();

    if( GetLinkType() == EXC_CHSRCLINK_WORKSHEET )
    {
        // read token array
        XclTokenArray aXclTokArr;
        rStrm >> aXclTokArr;
        // convert token array to range list
        // FIXME: JEG : This is wrong.  It should be a formula
        GetFormulaCompiler().CreateRangeList( maScRanges, EXC_FMLATYPE_CHART, aXclTokArr, rStrm );
    }

    // try to read a following CHSTRING record
    if( (rStrm.GetNextRecId() == EXC_ID_CHSTRING) && rStrm.StartNextRecord() )
    {
        mxString.reset( new XclImpString );
        rStrm.Ignore( 2 );
        mxString->Read( rStrm, EXC_STR_8BITLENGTH | EXC_STR_SEPARATEFORMATS );
    }
}

void XclImpChSourceLink::SetString( const String& rString )
{
    if( !mxString )
        mxString.reset( new XclImpString );
    mxString->SetText( rString );
}

void XclImpChSourceLink::SetTextFormats( const XclFormatRunVec& rFormats )
{
    if( mxString.is() )
        mxString->SetFormats( rFormats );
}

void XclImpChSourceLink::ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const
{
    if( ::get_flag( maData.mnFlags, EXC_CHSRCLINK_NUMFMT ) )
    {
        sal_uInt32 nScNumFmt = GetNumFmtBuffer().GetScFormat( maData.mnNumFmtIdx );
        if( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32 nApiNumFmt = static_cast< sal_Int32 >( nScNumFmt );
            if( bPercent )
                rPropSet.SetProperty( EXC_CHPROP_PERCENTAGENUMFMT, nApiNumFmt );
            else
                rPropSet.SetProperty( EXC_CHPROP_NUMBERFORMAT, nApiNumFmt );
        }
    }
}

Reference< XDataSequence > XclImpChSourceLink::CreateDataSequence( const OUString& rRole ) const
{
    Reference< XDataSequence > xDataSeq;
    Reference< XDataProvider > xDataProv = GetDataProvider();
    if( xDataProv.is() )
    {
        // create the string representation of the range list
        OUString aRangeRep;
        ScRangeStringConverter::GetStringFromRangeList( aRangeRep, &maScRanges, GetDocPtr(), ';' );

        // create the data sequence
        try
        {
            xDataSeq = xDataProv->createDataSequenceByRangeRepresentation( aRangeRep );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "XclImpChSourceLink::CreateDataSequence - cannot create data sequence" );
        }

        // set sequence role
        ScfPropertySet aSeqProp( xDataSeq );
        aSeqProp.SetProperty( EXC_CHPROP_ROLE, rRole );
    }
    return xDataSeq;
}

Sequence< Reference< XFormattedString > > XclImpChSourceLink::CreateStringSequence(
        const XclImpChRoot& rRoot, sal_uInt16 nLeadFontIdx, const Color& rLeadFontColor ) const
{
    ::std::vector< Reference< XFormattedString > > aStringVec;
    if( mxString.is() )
    {
        for( XclImpStringIterator aIt( *mxString ); aIt.Is(); ++aIt )
        {
            Reference< XFormattedString > xFmtStr(
                ScfApiHelper::CreateInstance( SERVICE_CHART2_FORMATTEDSTRING ), UNO_QUERY );
            if( xFmtStr.is() )
            {
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
                aStringVec.push_back( xFmtStr );
            }
        }
    }
    return ScfApiHelper::VectorToSequence( aStringVec );
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

void XclImpChFontBase::ConvertRotationBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const
{
    rRoot.GetChartPropSetHelper().WriteRotationProperties( rPropSet, GetRotation() );
}

// ----------------------------------------------------------------------------

XclImpChFont::XclImpChFont() :
    mnFontIdx( EXC_FONT_NOTFOUND )
{
}

void XclImpChFont::ReadChFont( XclImpStream& rStrm )
{
    rStrm >> mnFontIdx;
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
        // #116397# BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( rStrm.ReaduInt16() );
        // placement and rotation
        rStrm >> maData.mnPlacement >> maData.mnRotation;
        // lower 4 bits used for placement, other bits contain garbage
        maData.mnPlacement &= 0x000F;
    }
    else
    {
        // BIFF2-BIFF7: get rotation from text orientation
        sal_uInt8 nOrient = 0;
        ::extract_value( nOrient, maData.mnFlags, 8, 3 );
        maData.mnRotation = XclTools::GetXclRotFromOrient( nOrient );
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
            rStrm >> maObjLink.mnTarget >> maObjLink.maPointPos.mnSeriesIdx >> maObjLink.maPointPos.mnPointIdx;
        break;
        case EXC_ID_CHEND:
            if( mxSrcLink.is() && !maFormats.empty() )
                mxSrcLink->SetTextFormats( maFormats );
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

sal_uInt16 XclImpChText::GetRotation() const
{
    return maData.mnRotation;
}

void XclImpChText::SetString( const String& rString )
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

void XclImpChText::ConvertFrame( ScfPropertySet& rPropSet ) const
{
    if( mxFrame.is() )
        mxFrame->Convert( rPropSet );
}

void XclImpChText::ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const
{
    if( mxSrcLink.is() )
        mxSrcLink->ConvertNumFmt( rPropSet, bPercent );
}

void XclImpChText::ConvertDataLabel( ScfPropertySet& rPropSet, const XclChTypeInfo& rTypeInfo ) const
{
    const sal_uInt16 EXC_CHTEXT_SHOWANYCATEG   = EXC_CHTEXT_SHOWCATEGPERC | EXC_CHTEXT_SHOWCATEG;
    const sal_uInt16 EXC_CHTEXT_SHOWANYVALUE   = EXC_CHTEXT_SHOWVALUE;
    const sal_uInt16 EXC_CHTEXT_SHOWANYPERCENT = EXC_CHTEXT_SHOWPERCENT | EXC_CHTEXT_SHOWCATEGPERC;

    bool bIsPie       = rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE;
    bool bShowNone    = IsDeleted();
    bool bShowCateg   = !bShowNone && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYCATEG );
    bool bShowPercent = !bShowNone && bIsPie && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYPERCENT );
    bool bShowValue   = !bShowNone && !bShowPercent && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWANYVALUE );
    bool bShowAny     = bShowValue || bShowPercent || bShowCateg;
    bool bShowSymbol  = bShowAny && ::get_flag( maData.mnFlags, EXC_CHTEXT_SHOWSYMBOL );

    // type of attached label
    namespace cssc = ::com::sun::star::chart2;
    cssc::DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol );
    rPropSet.SetProperty( EXC_CHPROP_LABEL, aPointLabel );

    // text properties of attached label
    if( bShowAny )
    {
        ConvertFont( rPropSet );
        // label placement
        using namespace ::com::sun::star::chart::DataLabelPlacement;
        sal_Int32 nPlacement = rTypeInfo.mnDefaultLabelPos;
        switch( maData.mnPlacement )
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
        rPropSet.SetProperty( EXC_CHPROP_LABELPLACEMENT, nPlacement );
        // label number format (percentage format wins over value format)
        if( bShowPercent || bShowValue )
            ConvertNumFmt( rPropSet, bShowPercent );
    }
}

Reference< XTitle > XclImpChText::CreateTitle() const
{
    Reference< XTitle > xTitle;
    if( mxSrcLink.is() && mxSrcLink->HasString() )
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
                ConvertRotationBase( GetChRoot(), aTitleProp );
            }
        }
    }
    return xTitle;
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

// Data series ================================================================

void XclImpChMarkerFormat::ReadChMarkerFormat( XclImpStream& rStrm )
{
    rStrm >> maData.maLineColor >> maData.maFillColor >> maData.mnMarkerType >> maData.mnFlags;

    const XclImpRoot& rRoot = rStrm.GetRoot();
    if( rRoot.GetBiff() == EXC_BIFF8 )
    {
        // #116397# BIFF8: index into palette used instead of RGB data
        const XclImpPalette& rPal = rRoot.GetPalette();
        maData.maLineColor = rPal.GetColor( rStrm.ReaduInt16() );
        maData.maFillColor = rPal.GetColor( rStrm.ReaduInt16() );
        // marker size
        rStrm >> maData.mnMarkerSize;
    }
}

void XclImpChMarkerFormat::Convert( const XclImpChRoot& rRoot,
        ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, sal_Int16 nLineWeight ) const
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
        aMarkerFmt.mnMarkerType = XclChartHelper::GetAutoMarkerType( nFormatIdx );
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
    rPropSet.SetColorProperty( EXC_CHPROP_COLOR, aLineColor );
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
    double fApiDist = ::std::min< double >( mnPieDist / 100.0, 1.0 );
    rPropSet.SetProperty( EXC_CHPROP_OFFSET, fApiDist );
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
    using namespace ::com::sun::star::chart2::DataPointGeometry3D;
    sal_Int32 nApiType = (maData.mnBase == EXC_CH3DDATAFORMAT_RECT) ?
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? CUBOID : PYRAMID) :
        ((maData.mnTop == EXC_CH3DDATAFORMAT_STRAIGHT) ? CYLINDER : CONE);
    rPropSet.SetProperty( EXC_CHPROP_GEOMETRY3D, nApiType );
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
    rStrm   >> maData.maPointPos.mnPointIdx
            >> maData.maPointPos.mnSeriesIdx
            >> maData.mnFormatIdx
            >> maData.mnFlags;
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
    UpdateDataLabel( 0 );
}

void XclImpChDataFormat::Convert( ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo ) const
{
    // line and area format
    ConvertFrameBase( GetChRoot(), rPropSet, rTypeInfo.GetSeriesObjectType(), maData.mnFormatIdx );
#if EXC_CHART2_3DBAR_HAIRLINES_ONLY
    // #i83151# only hair lines in 3D charts with filled data points
    if( rTypeInfo.mb3dChart && rTypeInfo.IsSeriesFrameFormat() && mxLineFmt.is() && mxLineFmt->HasLine() )
        rPropSet.SetProperty< sal_Int32 >( CREATE_OUSTRING( "BorderWidth" ), 0 );
#endif

    // other formatting
    if( mxMarkerFmt.is() )
        mxMarkerFmt->Convert( GetChRoot(), rPropSet, maData.mnFormatIdx, GetLineWeight() );
    if( mxPieFmt.is() )
        mxPieFmt->Convert( rPropSet );
    if( mx3dDataFmt.is() )
        mx3dDataFmt->Convert( rPropSet );
    if( mxLabel.is() )
        mxLabel->ConvertDataLabel( rPropSet, rTypeInfo );

    // 3D settings
    rPropSet.SetProperty< sal_Int16 >( EXC_CHPROP_PERCENTDIAGONAL, 0 );

    /*  Special case: set marker color as line color, if series line is not
        visible. This makes the color visible in the marker area.
        TODO: remove this if OOChart supports own colors in markers. */
    if( !rTypeInfo.IsSeriesFrameFormat() && !HasLine() && mxMarkerFmt.is() )
        mxMarkerFmt->ConvertColor( GetChRoot(), rPropSet, maData.mnFormatIdx );
}

void XclImpChDataFormat::ConvertVarPoint( ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const
{
    if( !mxEscherFmt && mxAreaFmt.is() && IsAutoArea() )
        mxAreaFmt->Convert( GetChRoot(), rPropSet, EXC_CHOBJTYPE_FILLEDSERIES, nFormatIdx );
}

void XclImpChDataFormat::ConvertLine( ScfPropertySet& rPropSet, XclChObjectType eObjType ) const
{
    if( mxLineFmt.is() )
        mxLineFmt->Convert( GetChRoot(), rPropSet, eObjType );
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
    XclImpChTextRef xDefText;
    if( pParentFmt )
        xDefText = pParentFmt->GetDataLabel();
    if( !xDefText )
        xDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_DATALABEL );
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

Reference< XRegressionCurve > XclImpChSerTrendLine::CreateRegressionCurve() const
{
    // trend line type
    OUString aService;
    switch( maData.mnLineType )
    {
        case EXC_CHSERTREND_POLYNOMIAL:
            // TODO: only linear trend lines are supported by OOChart (#i20819#)
            if( maData.mnOrder == 1 )
                aService = SERVICE_CHART2_LINEARREGCURVE;
        break;
        case EXC_CHSERTREND_EXPONENTIAL:
            aService = SERVICE_CHART2_EXPREGCURVE;
        break;
        case EXC_CHSERTREND_LOGARITHMIC:
            aService = SERVICE_CHART2_LOGREGCURVE;
        break;
        case EXC_CHSERTREND_POWER:
            aService = SERVICE_CHART2_POTREGCURVE;
        break;
    }
    Reference< XRegressionCurve > xRegCurve;
    if( aService.getLength() > 0 )
        xRegCurve.set( ScfApiHelper::CreateInstance( aService ), UNO_QUERY );

    // trend line formatting
    if( xRegCurve.is() && mxDataFmt.is() )
    {
        ScfPropertySet aPropSet( xRegCurve );
        mxDataFmt->ConvertLine( aPropSet, EXC_CHOBJTYPE_TRENDLINE );

        // #i83100# show equation and correlation coefficient
        ScfPropertySet aLabelProp( xRegCurve->getEquationProperties() );
        aLabelProp.SetBoolProperty( EXC_CHPROP_SHOWEQUATION, maData.mnShowEquation != 0 );
        aLabelProp.SetBoolProperty( EXC_CHPROP_SHOWCORRELATION, maData.mnShowRSquared != 0 );

        // #i83100# formatting of the equation text box
        if( const XclImpChText* pLabel = mxDataFmt->GetDataLabel().get() )
        {
            pLabel->ConvertFont( aLabelProp );
            pLabel->ConvertFrame( aLabelProp );
            pLabel->ConvertNumFmt( aLabelProp, false );
        }
    }

    // missing features
    // #i20819# polynomial trend lines
    // #i66819# moving average trend lines
    // #i5085# manual trend line size
    // #i34093# manual crossing point

    return xRegCurve;
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

Reference< XPropertySet > XclImpChSerErrorBar::CreateErrorBar( bool bPosBar, bool bNegBar ) const
{
    Reference< XPropertySet > xErrorBar( ScfApiHelper::CreateInstance( SERVICE_CHART2_ERRORBAR ), UNO_QUERY );
    ScfPropertySet aBarProp( xErrorBar );

    // plus/minus bars visible?
    aBarProp.SetBoolProperty( EXC_CHPROP_SHOWPOSITIVEERROR, bPosBar );
    aBarProp.SetBoolProperty( EXC_CHPROP_SHOWNEGATIVEERROR, bNegBar );

    // type of displayed error
    namespace cssc = ::com::sun::star::chart2;
    switch( maData.mnSourceType )
    {
        case EXC_CHSERERR_PERCENT:
            aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle_RELATIVE );
            aBarProp.SetProperty( EXC_CHPROP_POSITIVEERROR, maData.mfValue );
            aBarProp.SetProperty( EXC_CHPROP_NEGATIVEERROR, maData.mfValue );
        break;
        case EXC_CHSERERR_FIXED:
            aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle_ABSOLUTE );
            aBarProp.SetProperty( EXC_CHPROP_POSITIVEERROR, maData.mfValue );
            aBarProp.SetProperty( EXC_CHPROP_NEGATIVEERROR, maData.mfValue );
        break;
        case EXC_CHSERERR_STDDEV:
            aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle_STANDARD_DEVIATION );
            aBarProp.SetProperty( EXC_CHPROP_WEIGHT, maData.mfValue );
        break;
        case EXC_CHSERERR_STDERR:
            aBarProp.SetProperty( EXC_CHPROP_ERRORBARSTYLE, cssc::ErrorBarStyle_STANDARD_ERROR );
        break;
        default:
            xErrorBar.clear();
    }

    // error bar formatting
    if( mxDataFmt.is() && xErrorBar.is() )
        mxDataFmt->ConvertLine( aBarProp, EXC_CHOBJTYPE_ERRORBAR );

    return xErrorBar;
}

// ----------------------------------------------------------------------------

XclImpChSeries::XclImpChSeries( const XclImpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclImpChRoot( rRoot ),
    mnGroupIdx( EXC_CHSERGROUP_NONE ),
    mnSeriesIdx( nSeriesIdx ),
    mnParentIdx( EXC_CHSERIES_INVALID )
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
            // #i51639# register series format index at chart type group
            if( (pxDataFmt == &mxSeriesFmt) && !HasParentSeries() )
                if( XclImpChTypeGroup* pTypeGroup = GetChartData().GetTypeGroup( mnGroupIdx ).get() )
                    pTypeGroup->SetUsedFormatIndex( xDataFmt->GetFormatIdx() );
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

void XclImpChSeries::AddChildSeries( const XclImpChSeries& rSeries )
{
    DBG_ASSERT( !HasParentSeries(), "XclImpChSeries::AddChildSeries - not allowed for child series" );

    /*  In Excel, trend lines and error bars are stored as own series. In Calc,
        these are properties of the parent series. This function adds the
        settings of the passed series to this series. */
    maTrendLines.insert( maTrendLines.end(), rSeries.maTrendLines.begin(), rSeries.maTrendLines.end() );
    maErrorBars.insert( rSeries.maErrorBars.begin(), rSeries.maErrorBars.end() );
}

void XclImpChSeries::FinalizeDataFormats()
{
    if( HasParentSeries() )
    {
        // *** series is a child series, e.g. trend line or error bar ***

        // create missing series format
        if( !mxSeriesFmt )
            mxSeriesFmt = CreateDataFormat( EXC_CHDATAFORMAT_ALLPOINTS, 0 );

        if( mxSeriesFmt.is() )
        {
            // #i83100# set text label format, e.g. for trend line equations
            mxSeriesFmt->SetDataLabel( maLabels.get( EXC_CHDATAFORMAT_ALLPOINTS ) );
            // create missing automatic formats
            mxSeriesFmt->UpdateTrendLineFormat();
        }

        // copy series formatting to child objects
        for( XclImpChSerTrendLineList::iterator aLIt = maTrendLines.begin(), aLEnd = maTrendLines.end(); aLIt != aLEnd; ++aLIt )
            (*aLIt)->SetDataFormat( mxSeriesFmt );
        for( XclImpChSerErrorBarMap::iterator aMIt = maErrorBars.begin(), aMEnd = maErrorBars.end(); aMIt != aMEnd; ++aMIt )
            aMIt->second->SetDataFormat( mxSeriesFmt );
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
        if( mxSeriesFmt.is() )
            mxSeriesFmt->UpdateSeriesFormat( pTypeGroup->GetTypeInfo(), pTypeGroup->GetGroupFormat().get() );

        // update data point formats (removes unchanged automatic formatting)
        for( XclImpChDataFormatMap::iterator aFIt = maPointFmts.begin(), aFEnd = maPointFmts.end(); aFIt != aFEnd; ++aFIt )
            aFIt->second->UpdatePointFormat( pTypeGroup->GetTypeInfo(), mxSeriesFmt.get() );
    }
}

namespace {

/** Creates a labeled data sequence object, adds link for series title if present. */
Reference< XLabeledDataSequence > lclCreateLabeledDataSequence(
        XclImpChSourceLinkRef xValueLink, const OUString& rValueRole,
        const XclImpChSourceLink* pTitleLink = 0 )
{
    // create data sequence for values and title
    Reference< XDataSequence > xValueSeq;
    if( xValueLink.is() )
        xValueSeq = xValueLink->CreateDataSequence( rValueRole );
    Reference< XDataSequence > xTitleSeq;
    if( pTitleLink )
        xTitleSeq = pTitleLink->CreateDataSequence( EXC_CHPROP_ROLE_LABEL );

    // create the labeled data sequence, if values or title are present
    Reference< XLabeledDataSequence > xLabeledSeq;
    if( xValueSeq.is() || xTitleSeq.is() )
        xLabeledSeq.set( ScfApiHelper::CreateInstance( SERVICE_CHART2_LABELEDDATASEQ ), UNO_QUERY );
    if( xLabeledSeq.is() )
    {
        if( xValueSeq.is() )
            xLabeledSeq->setValues( xValueSeq );
        if( xTitleSeq.is() )
            xLabeledSeq->setLabel( xTitleSeq );
    }
    return xLabeledSeq;
}

/** Returns the property set of the specified data point. */
ScfPropertySet lclGetPointPropSet( Reference< XDataSeries > xDataSeries, sal_uInt16 nPointIdx )
{
    ScfPropertySet aPropSet;
    try
    {
        aPropSet.Set( xDataSeries->getDataPointByIndex( static_cast< sal_Int32 >( nPointIdx ) ) );
    }
    catch( Exception& )
    {
        DBG_ERRORFILE( "lclGetPointPropSet - no data point property set" );
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
            }
            // attach labeled data sequences to series
            if( !aLabeledSeqVec.empty() )
                xDataSink->setData( ScfApiHelper::VectorToSequence( aLabeledSeqVec ) );
        }

        // series formatting
        ScfPropertySet aSeriesProp( xDataSeries );
        if( mxSeriesFmt.is() )
            mxSeriesFmt->Convert( aSeriesProp, rTypeInfo );

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
#if EXC_CHART2_VARYCOLORSBY_PROP
        aSeriesProp.SetBoolProperty( EXC_CHPROP_VARYCOLORSBY, bVarPointFmt );
#else
        aSeriesProp.SetBoolProperty( EXC_CHPROP_VARYCOLORSBY, rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE );
#endif
        if( bVarPointFmt && mxSeriesFmt.is() && mxSeriesFmt->IsAutoArea() )
        {
            sal_uInt16 nPointCount = mxValueLink->GetCellCount();
            for( sal_uInt16 nPointIdx = 0; nPointIdx < nPointCount; ++nPointIdx )
            {
                ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, nPointIdx );
                // nPointIdx is used as automatic format index
                mxSeriesFmt->ConvertVarPoint( aPointProp, nPointIdx );
            }
        }

        // data point formatting
        for( XclImpChDataFormatMap::const_iterator aIt = maPointFmts.begin(), aEnd = maPointFmts.end(); aIt != aEnd; ++aIt )
        {
            ScfPropertySet aPointProp = lclGetPointPropSet( xDataSeries, aIt->first );
            aIt->second->Convert( aPointProp, rTypeInfo );
        }
    }
    return xDataSeries;
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
    XclImpChSerTrendLineRef xTrendLine( new XclImpChSerTrendLine( GetChRoot() ) );
    xTrendLine->ReadChSerTrendLine( rStrm );
    maTrendLines.push_back( xTrendLine );
}

void XclImpChSeries::ReadChSerErrorBar( XclImpStream& rStrm )
{
    XclImpChSerErrorBarRef xErrorBar( new XclImpChSerErrorBar( GetChRoot() ) );
    xErrorBar->ReadChSerErrorBar( rStrm );
    maErrorBars[ xErrorBar->GetBarType() ] = xErrorBar;
}

XclImpChDataFormatRef XclImpChSeries::CreateDataFormat( sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx )
{
    XclImpChDataFormatRef xDataFmt( new XclImpChDataFormat( GetChRoot() ) );
    xDataFmt->SetPointPos( XclChDataPointPos( mnSeriesIdx, nPointIdx ), nFormatIdx );
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

void XclImpChSeries::ConvertTrendLines( Reference< XDataSeries > xDataSeries ) const
{
    Reference< XRegressionCurveContainer > xRegCurveCont( xDataSeries, UNO_QUERY );
    if( xRegCurveCont.is() )
    {
        for( XclImpChSerTrendLineList::const_iterator aIt = maTrendLines.begin(), aEnd = maTrendLines.end(); aIt != aEnd; ++aIt )
        {
            try
            {
                Reference< XRegressionCurve > xRegCurve = (*aIt)->CreateRegressionCurve();
                if( xRegCurve.is() )
                    xRegCurveCont->addRegressionCurve( xRegCurve );
            }
            catch( Exception& )
            {
                DBG_ERRORFILE( "XclImpChSeries::ConvertTrendLines - cannot add regression curve" );
            }
        }
    }
}

Reference< XPropertySet > XclImpChSeries::CreateErrorBar( sal_uInt8 nPosBarId, sal_uInt8 nNegBarId ) const
{
    Reference< XPropertySet > xErrorBar;
    XclImpChSerErrorBarRef xPosBar = maErrorBars.get( nPosBarId );
    XclImpChSerErrorBarRef xNegBar = maErrorBars.get( nNegBarId );
    if( const XclImpChSerErrorBar* pPrimaryBar = xPosBar.is() ? xPosBar.get() : xNegBar.get() )
        xErrorBar = pPrimaryBar->CreateErrorBar( xPosBar.is(), xNegBar.is() );
    return xErrorBar;
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
            if( GetBiff() == EXC_BIFF8 )
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
            if( GetBiff() == EXC_BIFF8 )
                rStrm >> maData.mnBubbleSize >> maData.mnBubbleType >> maData.mnFlags;
            else
                maData.mnFlags = 0;
        break;

        case EXC_ID_CHSURFACE:
            rStrm >> maData.mnFlags;
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
    // service name
    OUString aCoordSysService;
    if( maTypeInfo.mbPolarCoordSystem )
    {
        if( b3dChart )
            aCoordSysService = SERVICE_CHART2_POLARCOORDSYS3D;
        else
            aCoordSysService = SERVICE_CHART2_POLARCOORDSYS2D;
    }
    else
    {
        if( b3dChart )
            aCoordSysService = SERVICE_CHART2_CARTESIANCOORDSYS3D;
        else
            aCoordSysService = SERVICE_CHART2_CARTESIANCOORDSYS2D;
    }

    // create the coordinate system object
    Reference< XCoordinateSystem > xCoordSystem( ScfApiHelper::CreateInstance( aCoordSysService ), UNO_QUERY );

    // swap X and Y axis
    if( maTypeInfo.mbSwappedAxesSet )
    {
        ScfPropertySet aCoordSysProp( xCoordSystem );
        aCoordSysProp.SetBoolProperty( EXC_CHPROP_SWAPXANDYAXIS, true );
    }

    return xCoordSystem;
}

Reference< XChartType > XclImpChType::CreateChartType( Reference< XDiagram > xDiagram ) const
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
            // #i85166# starting angle of first pie slice
            ScfPropertySet aDiaProp( xDiagram );
            sal_Int32 nApiRot = (450 - (maData.mnRotation % 360)) % 360;
            aDiaProp.SetProperty( EXC_CHPROP_STARTINGANGLE, nApiRot );
        }
        break;
        default:;
    }

    return xChartType;
}

// ----------------------------------------------------------------------------

void XclImpChChart3d::ReadChChart3d( XclImpStream& rStrm )
{
    rStrm   >> maData.mnRotation
            >> maData.mnElevation
            >> maData.mnEyeDist
            >> maData.mnRelHeight
            >> maData.mnRelDepth
            >> maData.mnDepthGap
            >> maData.mnFlags;
}

void XclImpChChart3d::Convert( ScfPropertySet& rPropSet, const XclChTypeInfo& rTypeInfo ) const
{
    // do not set right-angled axes for pie charts
    bool bRightAngled = rTypeInfo.mb3dWalls && !::get_flag( maData.mnFlags, EXC_CHCHART3D_REAL3D );
    rPropSet.SetBoolProperty( EXC_CHPROP_RIGHTANGLEDAXES, bRightAngled );
    rPropSet.SetColorProperty( EXC_CHPROP_D3DSCENEAMBIENTCOLOR, Color( RGB_COLORDATA( 204, 204, 204 ) ) );
    rPropSet.SetBoolProperty( EXC_CHPROP_D3DSCENELIGHTON1, false );
    rPropSet.SetBoolProperty( EXC_CHPROP_D3DSCENELIGHTON2, true );
    rPropSet.SetColorProperty( EXC_CHPROP_D3DSCENELIGHTCOLOR2, Color( RGB_COLORDATA( 102, 102, 102 ) ) );
    rPropSet.SetProperty( EXC_CHPROP_D3DSCENELIGHTDIR2, Direction3D( 0.2, 0.4, 1.0 ) );
    rPropSet.SetProperty( EXC_CHPROP_D3DSCENESHADEMODE, ::com::sun::star::drawing::ShadeMode_FLAT );

    // change 3D view of pie charts to Excel's default view
    if( rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE )
    {
        static const HomogenMatrix saMatrix(
            HomogenMatrixLine( 1.0,  0.0,  0.0,  0.0 ),
            HomogenMatrixLine( 0.0,  0.5,  1.5,  0.0 ),
            HomogenMatrixLine( 0.0, -1.5,  0.5,  0.0 ),
            HomogenMatrixLine( 0.0,  0.0,  0.0,  1.0 ) );
        rPropSet.SetProperty( EXC_CHPROP_D3DTRANSFORMMATRIX, saMatrix );
        static const CameraGeometry saCamera(
            Position3D(   0.0,  0.0, 85000.0 ),
            Direction3D(  0.0,  0.0,     1.0 ),
            Direction3D(  0.0,  1.0,     0.0 ) );
        rPropSet.SetProperty( EXC_CHPROP_D3DCAMERAGEOMETRY, saCamera );
    }
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

        // frame properties
        if( mxFrame.is() )
            mxFrame->Convert( aLegendProp );
        // text properties
        if( mxText.is() )
            mxText->ConvertFont( aLegendProp );
        // special legend properties
        GetChartPropSetHelper().WriteLegendProperties( aLegendProp, maData );
    }
    return xLegend;
}

// ----------------------------------------------------------------------------

XclImpChDropBar::XclImpChDropBar( sal_uInt16 nDropBar ) :
    mnDropBar( nDropBar ),
    mnBarDist( 0 )
{
}

void XclImpChDropBar::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> mnBarDist;
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

// ----------------------------------------------------------------------------

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
    rStrm >> maData.maRect >> maData.mnFlags >> maData.mnGroupIdx;
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
        HasHiLoLine() &&                                // must contain hi-lo lines
        (maSeries.size() == static_cast<XclImpChSeriesVec::size_type>(HasDropBars() ? 4 : 3));   // correct series count
    maType.Finalize( bStockChart );

    // extended type info
    maTypeInfo.Set( maType.GetTypeInfo(), mxChart3d.is(), false );

    // reverse series order for some unstacked 2D chart types
    if( maTypeInfo.mbReverseSeries && !Is3dChart() && !maType.IsStacked() && !maType.IsPercent() )
        ::std::reverse( maSeries.begin(), maSeries.end() );

    // update chart type group format, may depend on chart type finalized above
    if( mxGroupFmt.is() )
        mxGroupFmt->UpdateGroupFormat( maTypeInfo );
}

void XclImpChTypeGroup::AddSeries( XclImpChSeriesRef xSeries )
{
    if( xSeries.is() )
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
    DBG_ASSERT( !maUnusedFormats.empty(), "XclImpChTypeGroup::PopUnusedFormatIndex - no more format indexes available" );
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
    bool bAnyStacked = maType.IsStacked() || maType.IsPercent();
    XclImpChLineFormatRef xConnLine = maChartLines.get( EXC_CHCHARTLINE_CONNECT );
    return bAnyStacked && (maTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR) && xConnLine.is() && xConnLine->HasLine();
}

const String& XclImpChTypeGroup::GetSingleSeriesTitle() const
{
    // no automatic title for series with trendlines or error bars
    // pie charts always show an automatic title, even if more series exist
    return (mxFirstSeries.is() && !mxFirstSeries->HasChildSeries() && (maTypeInfo.mbSingleSeriesVis || (maSeries.size() == 1))) ?
        mxFirstSeries->GetTitle() : String::EmptyString();
}

void XclImpChTypeGroup::ConvertChart3d( ScfPropertySet& rPropSet ) const
{
    if( mxChart3d.is() )
        mxChart3d->Convert( rPropSet, maTypeInfo );
}

Reference< XCoordinateSystem > XclImpChTypeGroup::CreateCoordSystem() const
{
    return maType.CreateCoordSystem( Is3dChart() );
}

Reference< XChartType > XclImpChTypeGroup::CreateChartType( Reference< XDiagram > xDiagram, sal_Int32 nApiAxesSetIdx ) const
{
    DBG_ASSERT( IsValidGroup(), "XclImpChTypeGroup::CreateChartType - type group without series" );

    // create the chart type object
    Reference< XChartType > xChartType = maType.CreateChartType( xDiagram );

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
    if( mxFirstSeries.is() )
        xLabeledSeq = mxFirstSeries->CreateCategSequence( EXC_CHPROP_ROLE_CATEG );
    return xLabeledSeq;
}

void XclImpChTypeGroup::ReadChDropBar( XclImpStream& rStrm )
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

void XclImpChTypeGroup::ReadChChartLine( XclImpStream& rStrm )
{
    sal_uInt16 nLineId = rStrm.ReaduInt16();
    if( (rStrm.GetNextRecId() == EXC_ID_CHLINEFORMAT) && rStrm.StartNextRecord() )
    {
        XclImpChLineFormatRef xLineFmt( new XclImpChLineFormat );
        xLineFmt->ReadChLineFormat( rStrm );
        maChartLines[ nLineId ] = xLineFmt;
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


void XclImpChTypeGroup::InsertDataSeries( Reference< XChartType > xChartType,
        Reference< XDataSeries > xSeries, sal_Int32 nApiAxesSetIdx ) const
{
    Reference< XDataSeriesContainer > xSeriesCont( xChartType, UNO_QUERY );
    if( xSeriesCont.is() && xSeries.is() )
    {
        // series stacking mode
        namespace cssc = ::com::sun::star::chart2;
        cssc::StackingDirection eStacking = cssc::StackingDirection_NO_STACKING;
        // stacked overrides deep-3d
        if( maType.IsStacked() || maType.IsPercent() )
            eStacking = cssc::StackingDirection_Y_STACKING;
        else if( Is3dDeepChart() )
            eStacking = cssc::StackingDirection_Z_STACKING;

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
            DBG_ERRORFILE( "XclImpChTypeGroup::InsertDataSeries - cannot add data series" );
        }
    }
}

void XclImpChTypeGroup::CreateDataSeries( Reference< XChartType > xChartType, sal_Int32 nApiAxesSetIdx ) const
{
    bool bSpline = false;
    for( XclImpChSeriesVec::const_iterator aIt = maSeries.begin(), aEnd = maSeries.end(); aIt != aEnd; ++aIt )
    {
        Reference< XDataSeries > xDataSeries = (*aIt)->CreateDataSeries();
        InsertDataSeries( xChartType, xDataSeries, nApiAxesSetIdx );
        bSpline |= (*aIt)->HasSpline();
    }
    // spline - TODO: set at single series (#i66858#)
    if( bSpline && !maTypeInfo.IsSeriesFrameFormat() && (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_RADAR) )
    {
        ScfPropertySet aTypeProp( xChartType );
        aTypeProp.SetProperty( EXC_CHPROP_CURVESTYLE, ::com::sun::star::chart2::CurveStyle_CUBIC_SPLINES );
    }
}

void XclImpChTypeGroup::CreateStockSeries( Reference< XChartType > xChartType, sal_Int32 nApiAxesSetIdx ) const
{
    // create the data series object
    Reference< XDataSeries > xDataSeries( ScfApiHelper::CreateInstance( SERVICE_CHART2_DATASERIES ), UNO_QUERY );
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        // create a list of data sequences from all series
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        DBG_ASSERT( maSeries.size() >= 3, "XclImpChTypeGroup::CreateChartType - missing stock series" );
        int nRoleIdx = (maSeries.size() == 3) ? 1 : 0;
        for( XclImpChSeriesVec::const_iterator aIt = maSeries.begin(), aEnd = maSeries.end();
                (nRoleIdx < 4) && (aIt != aEnd); ++nRoleIdx, ++aIt )
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
            Reference< XLabeledDataSequence > xDataSeq = (*aIt)->CreateValueSequence( aRole );
            if( xDataSeq.is() )
                aLabeledSeqVec.push_back( xDataSeq );
        }

        // attach labeled data sequences to series and insert series into chart type
        xDataSink->setData( ScfApiHelper::VectorToSequence( aLabeledSeqVec ) );

        // formatting of special stock chart elements
        ScfPropertySet aTypeProp( xChartType );
        aTypeProp.SetBoolProperty( EXC_CHPROP_JAPANESE, HasDropBars() );
        aTypeProp.SetBoolProperty( EXC_CHPROP_SHOWFIRST, HasDropBars() );
        aTypeProp.SetBoolProperty( EXC_CHPROP_SHOWHIGHLOW, true );
        // hi-lo line format
        XclImpChLineFormatRef xHiLoLine = maChartLines.get( EXC_CHCHARTLINE_HILO );
        if( xHiLoLine.is() )
        {
            ScfPropertySet aSeriesProp( xDataSeries );
            xHiLoLine->Convert( GetChRoot(), aSeriesProp, EXC_CHOBJTYPE_HILOLINE );
        }
        // white dropbar format
        XclImpChDropBarRef xUpBar = maDropBars.get( EXC_CHDROPBAR_UP );
        Reference< XPropertySet > xWhitePropSet;
        if( xUpBar.is() && aTypeProp.GetProperty( xWhitePropSet, EXC_CHPROP_WHITEDAY ) )
        {
            ScfPropertySet aBarProp( xWhitePropSet );
            xUpBar->Convert( GetChRoot(), aBarProp );
        }
        // black dropbar format
        XclImpChDropBarRef xDownBar = maDropBars.get( EXC_CHDROPBAR_DOWN );
        Reference< XPropertySet > xBlackPropSet;
        if( xDownBar.is() && aTypeProp.GetProperty( xBlackPropSet, EXC_CHPROP_BLACKDAY ) )
        {
            ScfPropertySet aBarProp( xBlackPropSet );
            xDownBar->Convert( GetChRoot(), aBarProp );
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
    rStrm >> maData.mnCross >> maData.mnLabelFreq >> maData.mnTickFreq >> maData.mnFlags;
}

void XclImpChLabelRange::Convert( ScfPropertySet& rPropSet, ScaleData& rScaleData ) const
{
    namespace cssc = ::com::sun::star::chart2;

    // do not overlap text
    rPropSet.SetBoolProperty( EXC_CHPROP_TEXTOVERLAP, false );
    // do not break text into several lines
    rPropSet.SetBoolProperty( EXC_CHPROP_TEXTBREAK, false );

    // origin (max-cross not supported, fall back to auto-cross)
    bool bMaxCross = ::get_flag( maData.mnFlags, EXC_CHLABELRANGE_MAXCROSS );
    lclSetValueOrClearAny( rScaleData.Origin, static_cast< double >( maData.mnCross ), bMaxCross );

    // reverse order
    bool bReverse = ::get_flag( maData.mnFlags, EXC_CHLABELRANGE_REVERSE );
    rScaleData.Orientation = bReverse ? cssc::AxisOrientation_REVERSE : cssc::AxisOrientation_MATHEMATICAL;

    //! TODO #i58731# show n-th category
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

void XclImpChValueRange::Convert( ScaleData& rScaleData ) const
{
    // scaling algorithm
    bool bLogScale = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_LOGSCALE );
    OUString aScalingService = bLogScale ? SERVICE_CHART2_LOGSCALING : SERVICE_CHART2_LINEARSCALING;
    Reference< XScaling > xScaling( ScfApiHelper::CreateInstance( aScalingService ), UNO_QUERY );
    Reference< XScaling > xLogScaling;
    if( xScaling.is() )
    {
        rScaleData.Scaling = xScaling;
        if( bLogScale )
            xLogScaling = xScaling->getInverseScaling();
    }

    // min/max
    lclSetScaledValueOrClearAny( rScaleData.Minimum, maData.mfMin, xLogScaling, ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN ) );
    lclSetScaledValueOrClearAny( rScaleData.Maximum, maData.mfMax, xLogScaling, ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX ) );

    // origin (max-cross overrides crossing settings, fall back to auto-cross)
    const sal_uInt16 nAutoCrossFlags = EXC_CHVALUERANGE_AUTOCROSS | EXC_CHVALUERANGE_MAXCROSS;
    lclSetScaledValueOrClearAny( rScaleData.Origin, maData.mfCross, xLogScaling, ::get_flag( maData.mnFlags, nAutoCrossFlags ) );

    // increment
    bool bAutoMajor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR );
    bool bAutoMinor = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR );
    // major increment
    IncrementData& rIncrementData = rScaleData.IncrementData;
    lclSetValueOrClearAny( rIncrementData.Distance, maData.mfMajorStep, bAutoMajor );
    // minor increment
    Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
    rSubIncrementSeq.realloc( 1 );
    sal_Int32 nCount = 0;
    if( !bAutoMajor && !bAutoMinor && (0.0 < maData.mfMinorStep) && (maData.mfMinorStep <= maData.mfMajorStep) )
    {
        double fCount = maData.mfMajorStep / maData.mfMinorStep + 0.5;
        if( fCount < 1001.0 )
            nCount = static_cast< sal_Int32 >( fCount );
    }
    lclSetValueOrClearAny( rSubIncrementSeq[ 0 ].IntervalCount, nCount, nCount == 0 );

    // reverse order
    namespace cssc = ::com::sun::star::chart2;
    bool bReverse = ::get_flag( maData.mnFlags, EXC_CHVALUERANGE_REVERSE );
    rScaleData.Orientation = bReverse ? cssc::AxisOrientation_REVERSE : cssc::AxisOrientation_MATHEMATICAL;
}

// ----------------------------------------------------------------------------

namespace {

sal_Int32 lclGetApiTickmarks( sal_uInt8 nXclTickPos )
{
    using namespace ::com::sun::star::chart2::TickmarkStyle;
    sal_Int32 nApiTickmarks = NONE;
    ::set_flag( nApiTickmarks, INNER, ::get_flag( nXclTickPos, EXC_CHTICK_INSIDE ) );
    ::set_flag( nApiTickmarks, OUTER, ::get_flag( nXclTickPos, EXC_CHTICK_OUTSIDE ) );
    return nApiTickmarks;
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
        // #116397# BIFF8: index into palette used instead of RGB data
        maData.maTextColor = GetPalette().GetColor( rStrm.ReaduInt16() );
        // rotation
        rStrm >> maData.mnRotation;
    }
    else
    {
        // BIFF2-BIFF7: get rotation from text orientation
        sal_uInt8 nOrient = 0;
        ::extract_value( nOrient, maData.mnFlags, 2, 3 );
        maData.mnRotation = XclTools::GetXclRotFromOrient( nOrient );
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
    rPropSet.SetProperty( EXC_CHPROP_MAJORTICKS, lclGetApiTickmarks( maData.mnMajor ) );
    rPropSet.SetProperty( EXC_CHPROP_MINORTICKS, lclGetApiTickmarks( maData.mnMinor ) );
}

// ----------------------------------------------------------------------------

XclImpChAxis::XclImpChAxis( const XclImpChRoot& rRoot, sal_uInt16 nAxisType ) :
    XclImpChRoot( rRoot ),
    mnNumFmtIdx( EXC_FORMAT_NOTFOUND )
{
    maData.mnType = nAxisType;
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
            rStrm >> mnNumFmtIdx;
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

Reference< XAxis > XclImpChAxis::CreateAxis( const XclImpChTypeGroup& rTypeGroup, bool bPrimary ) const
{
    // create the axis object (always)
    Reference< XAxis > xAxis( ScfApiHelper::CreateInstance( SERVICE_CHART2_AXIS ), UNO_QUERY );
    if( xAxis.is() )
    {
        ScfPropertySet aAxisProp( xAxis );
        // #i58688# axis enabled
        aAxisProp.SetBoolProperty( EXC_CHPROP_SHOW, IsActivated() );

        // axis line properties
        if( mxAxisLine.is() )
            mxAxisLine->Convert( GetChRoot(), aAxisProp, EXC_CHOBJTYPE_AXISLINE );
        // axis ticks properties
        if( mxTick.is() )
            mxTick->Convert( aAxisProp );

        // axis caption text --------------------------------------------------

        // radar charts disable their category labels via chart type, not via axis
        bool bHasLabels = HasLabels() &&
            ((GetAxisType() != EXC_CHAXIS_X) || rTypeGroup.HasCategoryLabels());
        aAxisProp.SetBoolProperty( EXC_CHPROP_DISPLAYLABELS, bHasLabels );
        if( bHasLabels )
        {
            // font settings from CHFONT record or from default text
            if( mxFont.is() )
                ConvertFontBase( GetChRoot(), aAxisProp );
            else if( const XclImpChText* pDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_AXISLABEL ).get() )
                pDefText->ConvertFont( aAxisProp );
            // label text rotation
            ConvertRotationBase( GetChRoot(), aAxisProp );
            // number format
            sal_uInt32 nScNumFmt = GetNumFmtBuffer().GetScFormat( mnNumFmtIdx );
            if( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
                aAxisProp.SetProperty( EXC_CHPROP_NUMBERFORMAT, static_cast< sal_Int32 >( nScNumFmt ) );
        }

        // axis scaling and increment -----------------------------------------

        const XclChExtTypeInfo& rTypeInfo = rTypeGroup.GetTypeInfo();
        ScaleData aScaleData = xAxis->getScaleData();
        // set axis type
        namespace ApiAxisType = ::com::sun::star::chart2::AxisType;
        switch( GetAxisType() )
        {
            case EXC_CHAXIS_X:
                if( rTypeInfo.mbCategoryAxis )
                {
                    aScaleData.AxisType = ApiAxisType::CATEGORY;
                    aScaleData.Categories = rTypeGroup.CreateCategSequence();
                }
                else
                    aScaleData.AxisType = ApiAxisType::REALNUMBER;
            break;
            case EXC_CHAXIS_Y:
                aScaleData.AxisType = rTypeGroup.IsPercent() ?
                    ApiAxisType::PERCENT : ApiAxisType::REALNUMBER;
            break;
            case EXC_CHAXIS_Z:
                aScaleData.AxisType = ApiAxisType::SERIES;
            break;
        }
        // axis scaling settings, dependent on axis type
        switch( aScaleData.AxisType )
        {
            case ApiAxisType::CATEGORY:
            case ApiAxisType::SERIES:
                if( mxLabelRange.is() )
                    mxLabelRange->Convert( aAxisProp, aScaleData );
            break;
            case ApiAxisType::REALNUMBER:
            case ApiAxisType::PERCENT:
                if( mxValueRange.is() )
                    mxValueRange->Convert( aScaleData );
            break;
            default:
                DBG_ERRORFILE( "XclImpChAxis::CreateAxis - unknown axis type" );
        }
        // #i85167# pie/donut charts need hard reverse attribute at Y axis (after mxValueRange->Convert())
        if( (GetAxisType() == EXC_CHAXIS_Y) && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE) )
            aScaleData.Orientation = ::com::sun::star::chart2::AxisOrientation_REVERSE;
        // write back
        xAxis->setScaleData( aScaleData );

        // grid ---------------------------------------------------------------

        // main grid
        ScfPropertySet aGridProp( xAxis->getGridProperties() );
        aGridProp.SetBoolProperty( EXC_CHPROP_SHOW, HasMajorGrid() );
        if( mxMajorGrid.is() )
            mxMajorGrid->Convert( GetChRoot(), aGridProp, EXC_CHOBJTYPE_GRIDLINE );
        // sub grid
        Sequence< Reference< XPropertySet > > aSubGridPropSeq = xAxis->getSubGridProperties();
        if( aSubGridPropSeq.hasElements() )
        {
            ScfPropertySet aSubGridProp( aSubGridPropSeq[ 0 ] );
            aSubGridProp.SetBoolProperty( EXC_CHPROP_SHOW, HasMinorGrid() );
            if( mxMinorGrid.is() )
                mxMinorGrid->Convert( GetChRoot(), aSubGridProp, EXC_CHOBJTYPE_GRIDLINE );
        }

        // axis position ------------------------------------------------------

        namespace ApiAxisPosition = ::com::sun::star::chart2::AxisPosition;
        aAxisProp.SetProperty( EXC_CHPROP_AXISPOSITION, bPrimary ? ApiAxisPosition::MAIN : ApiAxisPosition::SECONDARY );
    }
    return xAxis;
}

void XclImpChAxis::ConvertWall( ScfPropertySet& rPropSet ) const
{
    if( mxWallFrame.is() )
        mxWallFrame->Convert( rPropSet );
}

void XclImpChAxis::ReadChAxisLine( XclImpStream& rStrm )
{
    XclImpChLineFormatRef* pxLineFmt = 0;
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
            else if( bWallFrame && mxWallFrame.is() )
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

// ----------------------------------------------------------------------------

XclImpChAxesSet::XclImpChAxesSet( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclImpChRoot( rRoot )
{
    maData.mnAxesSetId = nAxesSetId;
}

void XclImpChAxesSet::ReadHeaderRecord( XclImpStream& rStrm )
{
    rStrm >> maData.mnAxesSetId >> maData.maRect;
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
        for( XclImpChTypeGroupMap::const_iterator aIt = maTypeGroups.begin(), aEnd = maTypeGroups.end(); aIt != aEnd; ++aIt )
        {
            XclImpChTypeGroupRef xTypeGroup = aIt->second;
            xTypeGroup->Finalize();
            if( xTypeGroup->IsValidGroup() )
                aValidGroups[ aIt->first ] = xTypeGroup;
        }
        maTypeGroups.swap( aValidGroups );
    }

    // invalid chart type groups are deleted now, check again with IsValidAxesSet()
    if( IsValidAxesSet() )
    {
        // create dummy X axis for charts supporting category ranges
        XclImpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
        if( !mxXAxis && xTypeGroup.is() && xTypeGroup->GetTypeInfo().mbCategoryAxis )
            mxXAxis.reset( new XclImpChAxis( GetChRoot(), EXC_CHAXIS_X ) );
        // create dummy Y axis for pie/doughnut charts
        if( !mxYAxis && xTypeGroup.is() && (xTypeGroup->GetTypeInfo().meTypeCateg == EXC_CHTYPECATEG_PIE) )
            mxYAxis.reset( new XclImpChAxis( GetChRoot(), EXC_CHAXIS_Y ) );

        // finalize axes
        if( mxXAxis.is() ) mxXAxis->Finalize();
        if( mxYAxis.is() ) mxYAxis->Finalize();
        if( mxZAxis.is() ) mxZAxis->Finalize();

        // finalize axis titles
        XclImpChTextRef xDefText = GetChartData().GetDefaultText( EXC_CHTEXTTYPE_AXISTITLE );
        lclFinalizeTitle( mxXAxisTitle, xDefText );
        lclFinalizeTitle( mxYAxisTitle, xDefText );
        lclFinalizeTitle( mxZAxisTitle, xDefText );

        // #i47745# missing plot frame -> invisible border and area
        if( !mxPlotFrame )
            mxPlotFrame.reset( new XclImpChFrame( GetChRoot(), EXC_CHOBJTYPE_PLOTFRAME ) );
    }
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
    for( XclImpChTypeGroupMap::const_iterator aIt = maTypeGroups.begin(), aEnd = maTypeGroups.end(); !xLegend && (aIt != aEnd); ++aIt )
        xLegend = aIt->second->GetLegend();
    return xLegend;
}

const String& XclImpChAxesSet::GetSingleSeriesTitle() const
{
    return (maTypeGroups.size() == 1) ? maTypeGroups.begin()->second->GetSingleSeriesTitle() : String::EmptyString();
}

void XclImpChAxesSet::Convert( Reference< XDiagram > xDiagram ) const
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
                DBG_ERRORFILE( "XclImpChAxesSet::Convert - cannot insert coordinate system" );
            }

            // create the axes with grids and axis titles and insert them into the diagram
            ConvertAxis( mxXAxis, mxXAxisTitle, xCoordSystem );
            ConvertAxis( mxYAxis, mxYAxisTitle, xCoordSystem );
            ConvertAxis( mxZAxis, mxZAxisTitle, xCoordSystem );
        }
    }
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
    maTypeGroups[ xTypeGroup->GetGroupIdx() ] = xTypeGroup;
}

Reference< XCoordinateSystem > XclImpChAxesSet::CreateCoordSystem( Reference< XDiagram > xDiagram ) const
{
    Reference< XCoordinateSystem > xCoordSystem;

    /*  Try to get existing ccordinate system. For now, all series from primary
        and secondary axes sets are inserted into one coordinate system. Later,
        this should be changed to use one coordinate system for each axes set. */
    Reference< XCoordinateSystemContainer > xCoordSystemCont( xDiagram, UNO_QUERY );
    if( xCoordSystemCont.is() )
    {
        Sequence< Reference< XCoordinateSystem > > aCoordSystems = xCoordSystemCont->getCoordinateSystems();
        DBG_ASSERT( aCoordSystems.getLength() <= 1, "XclImpChAxesSet::CreateCoordSystem - too many existing coordinate systems" );
        if( aCoordSystems.getLength() > 0 )
            xCoordSystem = aCoordSystems[ 0 ];
    }

    // create the coordinate system according to the first chart type
    if( !xCoordSystem.is() )
    {
        XclImpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
        if( xTypeGroup.is() )
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
        for( XclImpChTypeGroupMap::const_iterator aIt = maTypeGroups.begin(), aEnd = maTypeGroups.end(); aIt != aEnd; ++aIt )
        {
            try
            {
                Reference< XChartType > xChartType = aIt->second->CreateChartType( xDiagram, nApiAxesSetIdx );
                if( xChartType.is() )
                    xChartTypeCont->addChartType( xChartType );
            }
            catch( Exception& )
            {
                DBG_ERRORFILE( "XclImpChAxesSet::CreateCoordSystem - cannot add chart type" );
            }
        }
    }

    return xCoordSystem;
}

void XclImpChAxesSet::ConvertAxis(
        XclImpChAxisRef xChAxis, XclImpChTextRef xChAxisTitle,
        Reference< XCoordinateSystem > xCoordSystem ) const
{
    if( xChAxis.is() )
    {
        // create and attach the axis object
        Reference< XAxis > xAxis = CreateAxis( *xChAxis );
        if( xAxis.is() )
        {
            // create and attach the axis title
            if( xChAxisTitle.is() )
            {
                Reference< XTitled > xTitled( xAxis, UNO_QUERY );
                if( xTitled.is() )
                    xTitled->setTitleObject( xChAxisTitle->CreateTitle() );
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
                DBG_ERRORFILE( "XclImpChAxesSet::ConvertAxis - cannot set axis" );
            }
        }
    }
}

Reference< XAxis > XclImpChAxesSet::CreateAxis( const XclImpChAxis& rChAxis ) const
{
    Reference< XAxis > xAxis;
    if( const XclImpChTypeGroup* pTypeGroup = GetFirstTypeGroup().get() )
        xAxis = rChAxis.CreateAxis( *pTypeGroup, GetAxesSetId() == EXC_CHAXESSET_PRIMARY );
    return xAxis;
}

void XclImpChAxesSet::ConvertBackground( Reference< XDiagram > xDiagram ) const
{
    XclImpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
    if( xTypeGroup.is() && xTypeGroup->Is3dWallChart() )
    {
        // wall/floor formatting (3D charts)
        if( mxXAxis.is() )
        {
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxXAxis->ConvertWall( aWallProp );
        }
        if( mxYAxis.is() )
        {
            ScfPropertySet aFloorProp( xDiagram->getFloor() );
            mxYAxis->ConvertWall( aFloorProp );
        }
    }
    else if( mxPlotFrame.is() )
    {
        // diagram background formatting
        ScfPropertySet aWallProp( xDiagram->getWall() );
        mxPlotFrame->Convert( aWallProp );
    }
}

// The chart object ===========================================================

XclImpChChart::XclImpChChart( const XclImpRoot& rRoot ) :
    XclImpChRoot( rRoot, this )
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
            Finalize();     // finalize the entire chart object
        break;
    }
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

void XclImpChChart::SetChartFrameFormat( const XclChLineFormat& rLineFmt, const XclChAreaFormat& rAreaFmt )
{
    mxFrame.reset( new XclImpChFrame( GetChRoot(), rLineFmt, rAreaFmt, EXC_CHOBJTYPE_BACKGROUND ) );
}

XclImpChTypeGroupRef XclImpChChart::GetTypeGroup( sal_uInt16 nGroupIdx ) const
{
    XclImpChTypeGroupRef xTypeGroup = mxPrimAxesSet->GetTypeGroup( nGroupIdx );
    if( !xTypeGroup ) xTypeGroup = mxSecnAxesSet->GetTypeGroup( nGroupIdx );
    if( !xTypeGroup ) xTypeGroup = mxPrimAxesSet->GetFirstTypeGroup();
    return xTypeGroup;
}

XclImpChTextRef XclImpChChart::GetDefaultText( XclChTextType eTextType ) const
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
    return maDefTexts.get( nDefTextId );
}

void XclImpChChart::Convert( Reference< XChartDocument > xChartDoc, ScfProgressBar& rProgress ) const
{
    // initialize conversion (locks the model to suppress any internal updates)
    InitConversion( xChartDoc );

    // chart frame and title
    if( mxFrame.is() )
    {
        ScfPropertySet aFrameProp( xChartDoc->getPageBackground() );
        mxFrame->Convert( aFrameProp );
    }
    if( mxTitle.is() )
    {
        Reference< XTitled > xTitled( xChartDoc, UNO_QUERY );
        Reference< XTitle > xTitle = mxTitle->CreateTitle();
        if( xTitled.is() && xTitle.is() )
            xTitled->setTitleObject( xTitle );
    }

    /*  Create the diagram object and attach it to the chart document. Currently,
        one diagram is used to carry all coordinate systems and data series. */
    Reference< XDiagram > xDiagram( ScfApiHelper::CreateInstance( SERVICE_CHART2_DIAGRAM ), UNO_QUERY );
    xChartDoc->setFirstDiagram( xDiagram );

    // coordinate systems and chart types, convert axis settings
    mxPrimAxesSet->Convert( xDiagram );
    mxSecnAxesSet->Convert( xDiagram );

    // legend
    if( xDiagram.is() && mxLegend.is() )
        xDiagram->setLegend( mxLegend->CreateLegend() );

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
    if( mxLegend.is() )
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

void XclImpChChart::FinalizeTitle()
{
    if( (!mxTitle || (!mxTitle->IsDeleted() && !mxTitle->HasString())) && !mxSecnAxesSet->IsValidAxesSet() )
    {
        /*  Chart title is auto-generated from series title, if there is only
            one series with title in the chart. */
        const String& rSerTitle = mxPrimAxesSet->GetSingleSeriesTitle();
        if( rSerTitle.Len() > 0 )
        {
            if( !mxTitle )
                mxTitle.reset( new XclImpChText( GetChRoot() ) );
            mxTitle->SetString( rSerTitle );
        }
    }

    // will reset mxTitle, if it does not contain a string
    lclFinalizeTitle( mxTitle, GetDefaultText( EXC_CHTEXTTYPE_TITLE ) );
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

void XclImpChart::SetChartFrameFormat( const XclChLineFormat& rLineFmt, const XclChAreaFormat& rAreaFmt )
{
    if( !mxChartData )
        mxChartData.reset( new XclImpChChart( GetRoot() ) );
    mxChartData->SetChartFrameFormat( rLineFmt, rAreaFmt );
}

sal_Size XclImpChart::GetProgressSize() const
{
    return mxChartData.is() ? mxChartData->GetProgressSize() : 0;
}

void XclImpChart::Convert( Reference< XModel > xModel, ScfProgressBar& rProgress ) const
{
    Reference< XChartDocument > xChartDoc( xModel, UNO_QUERY );
    if( mxChartData.is() && xChartDoc.is() )
        mxChartData->Convert( xChartDoc, rProgress );
}

void XclImpChart::ReadChChart( XclImpStream& rStrm )
{
    mxChartData.reset( new XclImpChChart( GetRoot() ) );
    mxChartData->ReadRecordGroup( rStrm );
}

// ============================================================================

