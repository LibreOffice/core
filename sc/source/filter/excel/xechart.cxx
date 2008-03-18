/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xechart.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 14:50:51 $
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

#include "xechart.hxx"

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <vcl/outdev.hxx>
#include <svx/escherex.hxx>

#include "document.hxx"
#include "rangelst.hxx"
#include "rangeutl.hxx"
#include "xeformula.hxx"
#include "xehelper.hxx"
#include "xepage.hxx"
#include "xestyle.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::i18n::XBreakIterator;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XCoordinateSystemContainer;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XChartTypeContainer;
using ::com::sun::star::chart2::XChartType;
using ::com::sun::star::chart2::XDataSeriesContainer;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XRegressionCurveContainer;
using ::com::sun::star::chart2::XRegressionCurve;
using ::com::sun::star::chart2::XAxis;
using ::com::sun::star::chart2::XScaling;
using ::com::sun::star::chart2::ScaleData;
using ::com::sun::star::chart2::IncrementData;
using ::com::sun::star::chart2::SubIncrement;
using ::com::sun::star::chart2::XLegend;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::XTitle;
using ::com::sun::star::chart2::XFormattedString;

using ::com::sun::star::chart2::data::XDataSource;
using ::com::sun::star::chart2::data::XLabeledDataSequence;
using ::com::sun::star::chart2::data::XDataSequence;

// Helpers ====================================================================

namespace {

XclExpStream& operator<<( XclExpStream& rStrm, const XclChRectangle& rRect )
{
    return rStrm << rRect.mnX << rRect.mnY << rRect.mnWidth << rRect.mnHeight;
}

inline void lclSaveRecord( XclExpStream& rStrm, XclExpRecordRef xRec )
{
    if( xRec.is() )
        xRec->Save( rStrm );
}

/** Saves the passed record (group) together with a leading value record. */
template< typename Type >
void lclSaveRecord( XclExpStream& rStrm, XclExpRecordRef xRec, sal_uInt16 nRecId, Type nValue )
{
    if( xRec.is() )
    {
        XclExpValueRecord< Type >( nRecId, nValue ).Save( rStrm );
        xRec->Save( rStrm );
    }
}

template< typename Type >
inline bool lclIsAutoAnyOrGetValue( Type& rValue, const Any& rAny )
{
    return !rAny.hasValue() || !(rAny >>= rValue);
}

bool lclIsAutoAnyOrGetScaledValue( double& rfValue, const Any& rAny, Reference< XScaling > xScaling )
{
    bool bIsAuto = lclIsAutoAnyOrGetValue( rfValue, rAny );
    if( !bIsAuto && xScaling.is() )
        rfValue = xScaling->doScaling( rfValue );
    return bIsAuto;
}

} // namespace

// Common =====================================================================

/** Stores global data needed in various classes of the Chart export filter. */
class XclExpChRootData : public XclChRootData
{
public:
    explicit            XclExpChRootData( XclExpChChart* pChartData );

    /** Returns a reference to the parent chart data object. */
    inline XclExpChChart& GetChartData() const { return *mpChartData; }

private:
    XclExpChChart*      mpChartData;            /// Pointer to the chart data object.
};

XclExpChRootData::XclExpChRootData( XclExpChChart* pChartData ) :
    mpChartData( pChartData )
{
}

// ----------------------------------------------------------------------------

XclExpChRoot::XclExpChRoot( const XclExpRoot& rRoot, XclExpChChart* pChartData ) :
    XclExpRoot( rRoot ),
    mxChData( new XclExpChRootData( pChartData ) )
{
}

XclExpChRoot::~XclExpChRoot()
{
}

XclExpChChart& XclExpChRoot::GetChartData() const
{
    return mxChData->GetChartData();
}

const XclChTypeInfo& XclExpChRoot::GetChartTypeInfo( XclChTypeId eType ) const
{
    return mxChData->GetTypeInfoProvider().GetTypeInfo( eType );
}

const XclChTypeInfo& XclExpChRoot::GetChartTypeInfo( const OUString& rServiceName ) const
{
    return mxChData->GetTypeInfoProvider().GetTypeInfoFromService( rServiceName );
}

const XclChFormatInfo& XclExpChRoot::GetFormatInfo( XclChObjectType eObjType ) const
{
    return mxChData->GetFormatInfoProvider().GetFormatInfo( eObjType );
}

void XclExpChRoot::InitConversion( XChartDocRef xChartDoc ) const
{
    mxChData->InitConversion( xChartDoc );
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
    DBG_ASSERT( GetPalette().IsSystemColor( nSysColorIdx ), "XclExpChRoot::SetSystemColor - invalid color index" );
    rColor = GetPalette().GetDefColor( nSysColorIdx );
    rnColorId = XclExpPalette::GetColorIdFromIndex( nSysColorIdx );
}

void XclExpChRoot::ConvertLineFormat( XclChLineFormat& rLineFmt,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode ) const
{
    GetChartPropSetHelper().ReadLineProperties(
        rLineFmt, mxChData->GetLineDashTable(), rPropSet, ePropMode );
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
        mxChData->GetGradientTable(), mxChData->GetBitmapTable(), rPropSet, ePropMode );
}

sal_uInt16 XclExpChRoot::ConvertFont( const ScfPropertySet& rPropSet, sal_Int16 nScript ) const
{
    XclFontData aFontData;
    GetFontPropSetHelper().ReadFontProperties( aFontData, rPropSet, EXC_FONTPROPSET_CHART, nScript );
    return GetFontBuffer().Insert( aFontData, EXC_COLOR_CHARTTEXT );
}

// ----------------------------------------------------------------------------

XclExpChGroupBase::XclExpChGroupBase( sal_uInt16 nRecId, sal_Size nRecSize ) :
    XclExpRecord( nRecId, nRecSize )
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
        // CHBEGIN record
        XclExpEmptyRecord( EXC_ID_CHBEGIN ).Save( rStrm );
        // embedded records
        WriteSubRecords( rStrm );
        // CHEND record
        XclExpEmptyRecord( EXC_ID_CHEND ).Save( rStrm );
    }
}

bool XclExpChGroupBase::HasSubRecords() const
{
    return true;
}

// Frame formatting ===========================================================

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
            DBG_ERRORFILE( "XclExpChLineFormat::SetDefault - unknown frame type" );
    }
}

void XclExpChLineFormat::Convert( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    rRoot.ConvertLineFormat( maData, rPropSet, rFmtInfo.mePropMode );
    if( HasLine() )
    {
        // detect system color, set color identifier
        if( rRoot.IsSystemColor( maData.maColor, rFmtInfo.mnAutoLineColorIdx ) )
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
        // detect system color, set color identifier
        if( rRoot.IsSystemColor( maData.maPattColor, rFmtInfo.mnAutoPattColorIdx ) )
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
            DBG_ERRORFILE( "XclExpChAreaFormat::SetDefault - unknown frame type" );
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
    XclExpChGroupBase( EXC_ID_CHESCHERFORMAT ),
    mnColor1Id( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) ),
    mnColor2Id( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK ) )
{
    DBG_ASSERT_BIFF( rRoot.GetBiff() == EXC_BIFF8 );
    (void)rRoot;
}

void XclExpChEscherFormat::Convert( const XclExpChRoot& rRoot,
        const ScfPropertySet& rPropSet, XclChObjectType eObjType )
{
    const XclChFormatInfo& rFmtInfo = rRoot.GetFormatInfo( eObjType );
    rRoot.ConvertEscherFormat( maData, maPicFmt, rPropSet, rFmtInfo.mePropMode );
    // register colors in palette
    mnColor1Id = RegisterColor( rRoot, ESCHER_Prop_fillColor );
    mnColor2Id = RegisterColor( rRoot, ESCHER_Prop_fillBackColor );
}

bool XclExpChEscherFormat::IsValid() const
{
    return maData.mxEscherSet.is();
}

void XclExpChEscherFormat::Save( XclExpStream& rStrm )
{
    if( maData.mxEscherSet.is() )
    {
        // replace RGB colors with palette indexes in the Escher container
        const XclExpPalette& rPal = rStrm.GetRoot().GetPalette();
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
    rStrm << maPicFmt.mnBmpMode << maPicFmt.mnFormat << maPicFmt.mnFlags << maPicFmt.mfScale;
    rStrm.EndRecord();
}

sal_uInt32 XclExpChEscherFormat::RegisterColor( const XclExpChRoot& rRoot, sal_uInt16 nPropId )
{
    sal_uInt32 nBGRValue;
    if( maData.mxEscherSet.is() && maData.mxEscherSet->GetOpt( nPropId, nBGRValue ) )
    {
        // swap red and blue
        Color aColor( RGB_COLORDATA(
            COLORDATA_BLUE( nBGRValue ),
            COLORDATA_GREEN( nBGRValue ),
            COLORDATA_RED( nBGRValue ) ) );
        return rRoot.GetPalette().InsertColor( aColor, EXC_COLOR_CHARTAREA );
    }
    return XclExpPalette::GetColorIdFromIndex( EXC_COLOR_CHWINDOWBACK );
}

void XclExpChEscherFormat::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( maData.mxEscherSet.is(), "XclExpChEscherFormat::WriteBody - missing property container" );
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
            mxEscherFmt->Convert( rRoot, rPropSet, eObjType );
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
    XclExpChGroupBase( EXC_ID_CHFRAME, 4 ),
    XclExpChRoot( rRoot ),
    meObjType( eObjType )
{
}

void XclExpChFrame::Convert( const ScfPropertySet& rPropSet )
{
    ConvertFrameBase( GetChRoot(), rPropSet, meObjType );
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

XclExpChSourceLink::XclExpChSourceLink( const XclExpChRoot& rRoot, sal_uInt8 nDestType ) :
    XclExpRecord( EXC_ID_CHSOURCELINK ),
    XclExpChRoot( rRoot )
{
    maData.mnDestType = nDestType;
}

sal_uInt16 XclExpChSourceLink::ConvertDataSequence( Reference< XDataSequence > xDataSeq, bool bSplitToColumns )
{
    mxLinkFmla.reset();
    maData.mnLinkType = EXC_CHSRCLINK_DEFAULT;
    sal_uInt16 nValueCount = 0;

    if( xDataSeq.is() )
    {
        OUString aRangeRepr = xDataSeq->getSourceRangeRepresentation();
        ScRangeList aScRanges;
        if( ScRangeStringConverter::GetRangeListFromString( aScRanges, aRangeRepr, GetDocPtr(), ';' ) )
        {
            // split 3-dimensional ranges into single sheets
            ScRangeList aNewScRanges;
            for( const ScRange* pScRange = aScRanges.First(); pScRange; pScRange = aScRanges.Next() )
            {
                SCCOL nScCol1 = pScRange->aStart.Col();
                SCROW nScRow1 = pScRange->aStart.Row();
                SCCOL nScCol2 = pScRange->aEnd.Col();
                SCROW nScRow2 = pScRange->aEnd.Row();
                for( SCTAB nScTab = pScRange->aStart.Tab(); nScTab <= pScRange->aEnd.Tab(); ++nScTab )
                {
                    // split 2-dimensional ranges into single columns
                    if( bSplitToColumns && (nScRow1 != nScRow2) )
                        for( SCCOL nScCol = nScCol1; nScCol <= nScCol2; ++nScCol )
                            aNewScRanges.Append( ScRange( nScCol, nScRow1, nScTab, nScCol, nScRow2, nScTab ) );
                    else
                        aNewScRanges.Append( ScRange( nScCol1, nScRow1, nScTab, nScCol2, nScRow2, nScTab ) );
                }
            }
            mxLinkFmla = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CHART, aNewScRanges );
            maData.mnLinkType = EXC_CHSRCLINK_WORKSHEET;
            nValueCount = limit_cast< sal_uInt16 >( aScRanges.GetCellCount() );
        }
    }
    return nValueCount;
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
            DBG_ASSERT( !rFormats.empty() && (rFormats.front().mnChar == 0),
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

void XclExpChSourceLink::Save( XclExpStream& rStrm )
{
    // CHFORMATRUNS record
    if( mxString.is() && mxString->IsRich() )
    {
        sal_Size nRecSize = (1 + mxString->GetFormatsCount()) * ((GetBiff() == EXC_BIFF8) ? 2 : 1);
        rStrm.StartRecord( EXC_ID_CHFORMATRUNS, nRecSize );
        mxString->WriteFormats( rStrm, true );
        rStrm.EndRecord();
    }
    // CHSOURCELINK record
    XclExpRecord::Save( rStrm );
    // CHSTRING record
    if( mxString.is() && !mxString->IsEmpty() )
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
        const XclExpChRoot& rRoot, const ScfPropertySet& rPropSet )
{
    sal_uInt16 nRotation = rRoot.GetChartPropSetHelper().ReadRotationProperties( rPropSet );
    SetRotation( nRotation );
}

// ----------------------------------------------------------------------------

XclExpChText::XclExpChText( const XclExpChRoot& rRoot ) :
    XclExpChGroupBase( EXC_ID_CHTEXT, (rRoot.GetBiff() == EXC_BIFF8) ? 32 : 26 ),
    XclExpChRoot( rRoot ),
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

void XclExpChText::ConvertTitle( Reference< XTitle > xTitle, sal_uInt16 nTarget )
{
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
        ConvertFontBase( GetChRoot(), nFontIdx );

        // rotation
        ConvertRotationBase( GetChRoot(), aTitleProp );
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
    namespace cssc = ::com::sun::star::chart2;
    cssc::DataPointLabel aPointLabel;
    if( rPropSet.GetProperty( aPointLabel, EXC_CHPROP_LABEL ) )
    {
        //! TODO: make value and percent independent
        bool bIsPie       = rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_PIE;
        bool bShowValue   = aPointLabel.ShowNumber;
        bool bShowPercent = bIsPie && !bShowValue && aPointLabel.ShowNumberInPercent;
        bool bShowCateg   = !bShowValue && aPointLabel.ShowCategoryName;
        bool bShowAny     = bShowValue || bShowPercent || bShowCateg;
        bool bShowSymbol  = bShowAny && aPointLabel.ShowLegendSymbol;

        ::set_flag( maData.mnFlags, EXC_CHTEXT_AUTOTEXT );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWVALUE, bShowValue );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWPERCENT, bShowPercent );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEG, bShowCateg );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWCATEGPERC, bShowPercent && bShowCateg );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_SHOWSYMBOL, bShowSymbol );
        ::set_flag( maData.mnFlags, EXC_CHTEXT_DELETED, !bShowAny );

        if( bShowAny )
        {
            // font settings
            ConvertFontBase( GetChRoot(), rPropSet );
            // label placement
            sal_Int32 nPlacement = 0;
            if( rPropSet.GetProperty( nPlacement, EXC_CHPROP_LABELPLACEMENT ) )
            {
                using namespace ::com::sun::star::chart::DataLabelPlacement;
                if( nPlacement == rTypeInfo.mnDefaultLabelPos )
                {
                    maData.mnPlacement = EXC_CHTEXT_POS_DEFAULT;
                }
                else switch( nPlacement )
                {
                    case AVOID_OVERLAP:     maData.mnPlacement = EXC_CHTEXT_POS_AUTO;       break;
                    case CENTER:            maData.mnPlacement = EXC_CHTEXT_POS_CENTER;     break;
                    case TOP:               maData.mnPlacement = EXC_CHTEXT_POS_ABOVE;      break;
                    case TOP_LEFT:          maData.mnPlacement = EXC_CHTEXT_POS_LEFT;       break;
                    case LEFT:              maData.mnPlacement = EXC_CHTEXT_POS_LEFT;       break;
                    case BOTTOM_LEFT:       maData.mnPlacement = EXC_CHTEXT_POS_LEFT;       break;
                    case BOTTOM:            maData.mnPlacement = EXC_CHTEXT_POS_BELOW;      break;
                    case BOTTOM_RIGHT:      maData.mnPlacement = EXC_CHTEXT_POS_RIGHT;      break;
                    case RIGHT:             maData.mnPlacement = EXC_CHTEXT_POS_RIGHT;      break;
                    case TOP_RIGHT:         maData.mnPlacement = EXC_CHTEXT_POS_RIGHT;      break;
                    case INSIDE:            maData.mnPlacement = EXC_CHTEXT_POS_INSIDE;     break;
                    case OUTSIDE:           maData.mnPlacement = EXC_CHTEXT_POS_OUTSIDE;    break;
                    case NEAR_ORIGIN:       maData.mnPlacement = EXC_CHTEXT_POS_AXIS;       break;
                    default:                DBG_ERRORFILE( "XclExpChText::ConvertDataLabel - unknown label placement type" );
                }
            }
            // source link (contains number format)
            mxSrcLink.reset( new XclExpChSourceLink( GetChRoot(), EXC_CHSRCLINK_TITLE ) );
            if( bShowValue || bShowPercent )
                // percentage format wins over value format
                mxSrcLink->ConvertNumFmt( rPropSet, bShowPercent );
            // object link
            mxObjLink.reset( new XclExpChObjectLink( EXC_CHOBJLINK_DATA, rPointPos ) );
            // return true to indicate existing label
            return true;
        }
    }
    return false;
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
    // CHFONT record
    lclSaveRecord( rStrm, mxFont );
    // CHSOURCELINK group
    lclSaveRecord( rStrm, mxSrcLink );
    // CHFRAME group
    lclSaveRecord( rStrm, mxFrame );
    // CHOBJECTLINK record
    lclSaveRecord( rStrm, mxObjLink );
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
                << maData.mnPlacement
                << maData.mnRotation;
    }
}

// ----------------------------------------------------------------------------

namespace {

/** Creates and returns an Excel text object from the passed title. */
XclExpChTextRef lclCreateTitle( const XclExpChRoot& rRoot, Reference< XTitled > xTitled, sal_uInt16 nTarget )
{
    Reference< XTitle > xTitle;
    if( xTitled.is() )
        xTitle = xTitled->getTitleObject();

    XclExpChTextRef xText( new XclExpChText( rRoot ) );
    xText->ConvertTitle( xTitle, nTarget );
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
        using namespace ::com::sun::star::chart2::DataPointGeometry3D;
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
                DBG_ERRORFILE( "XclExpCh3dDataFormat::Convert - unknown 3D bar format" );
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
    XclExpChGroupBase( EXC_ID_CHDATAFORMAT, 8 ),
    XclExpChRoot( rRoot )
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

bool XclExpChSerErrorBar::Convert( const ScfPropertySet& rPropSet )
{
    namespace cssc = ::com::sun::star::chart2;
    cssc::ErrorBarStyle eBarStyle;
    bool bOk = rPropSet.GetProperty( eBarStyle, EXC_CHPROP_ERRORBARSTYLE );
    if( bOk )
    {
        switch( eBarStyle )
        {
            case cssc::ErrorBarStyle_ABSOLUTE:
                maData.mnSourceType = EXC_CHSERERR_FIXED;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_POSITIVEERROR );
            break;
            case cssc::ErrorBarStyle_RELATIVE:
                maData.mnSourceType = EXC_CHSERERR_PERCENT;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_POSITIVEERROR );
            break;
            case cssc::ErrorBarStyle_STANDARD_DEVIATION:
                maData.mnSourceType = EXC_CHSERERR_STDDEV;
                rPropSet.GetProperty( maData.mfValue, EXC_CHPROP_WEIGHT );
            break;
            case cssc::ErrorBarStyle_STANDARD_ERROR:
                maData.mnSourceType = EXC_CHSERERR_STDERR;
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
        DBG_ERRORFILE( "lclGetPointPropSet - no data point property set" );
    }
    return aPropSet;
}

} // namespace

XclExpChSeries::XclExpChSeries( const XclExpChRoot& rRoot, sal_uInt16 nSeriesIdx ) :
    XclExpChGroupBase( EXC_ID_CHSERIES, (rRoot.GetBiff() == EXC_BIFF8) ? 12 : 8 ),
    XclExpChRoot( rRoot ),
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

bool XclExpChSeries::ConvertDataSeries( Reference< XDataSeries > xDataSeries,
        const XclChExtTypeInfo& rTypeInfo, sal_uInt16 nGroupIdx, sal_uInt16 nFormatIdx )
{
    bool bOk = false;
    Reference< XDataSource > xDataSource( xDataSeries, UNO_QUERY );
    if( xDataSource.is() )
    {
        Reference< XDataSequence > xYValueSeq, xTitleSeq, xXValueSeq;

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
                    xTitleSeq = (*pIt)->getLabel();     // ignore role of label sequence
                }
                else if( !xXValueSeq.is() && !rTypeInfo.mbCategoryAxis && (aRole == EXC_CHPROP_ROLE_XVALUES) )
                {
                    xXValueSeq = xTmpValueSeq;
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
            if( xXValueSeq.is() )
                maData.mnCategCount = mxCategLink->ConvertDataSequence( xXValueSeq, false );
            else
                maData.mnCategCount = maData.mnValueCount;

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

            // data point formatting
            Sequence< sal_Int32 > aPointIndexes;
            if( (maData.mnValueCount > 0) && aSeriesProp.GetProperty( aPointIndexes, EXC_CHPROP_ATTRIBDATAPOINTS ) )
            {
                const sal_Int32* pnBeg = aPointIndexes.getConstArray();
                const sal_Int32* pnEnd = pnBeg + aPointIndexes.getLength();
                const sal_Int32 nMaxPointIdx = ::std::min< sal_Int32 >( maData.mnValueCount - 1, EXC_CHDATAFORMAT_MAXPOINT );
                for( const sal_Int32* pnIt = pnBeg; (pnIt != pnEnd) && (*pnIt <= nMaxPointIdx); ++pnIt )
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

bool XclExpChSeries::ConvertTrendLine( Reference< XRegressionCurve > xRegCurve, sal_uInt16 nParentIdx )
{
    mxTrendLine.reset( new XclExpChSerTrendLine( GetChRoot() ) );
    bool bOk = mxTrendLine->Convert( xRegCurve, mnSeriesIdx );
    if( bOk )
    {
        mxSeriesFmt = mxTrendLine->GetDataFormat();
        GetChartData().SetDataLabel( mxTrendLine->GetDataLabel() );
        // index to parent series is stored 1-based
        mnParentIdx = nParentIdx + 1;
    }
    return bOk;
}

bool XclExpChSeries::ConvertErrorBar( const ScfPropertySet& rPropSet, sal_uInt16 nParentIdx, sal_uInt8 nBarId )
{
    // error bar settings
    mxErrorBar.reset( new XclExpChSerErrorBar( GetChRoot(), nBarId ) );
    bool bOk = mxErrorBar->Convert( rPropSet );
    if( bOk )
    {
        // error bar formatting
        mxSeriesFmt.reset( new XclExpChDataFormat( GetChRoot(), XclChDataPointPos( mnSeriesIdx ), 0 ) );
        mxSeriesFmt->ConvertLine( rPropSet, EXC_CHOBJTYPE_ERRORBAR );
        // index to parent series is stored 1-based
        mnParentIdx = nParentIdx + 1;
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
            if( xSeries.is() && !xSeries->ConvertTrendLine( *pIt, mnSeriesIdx ) )
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
        if( xSeries.is() && !xSeries->ConvertErrorBar( rPropSet, mnSeriesIdx, nBarId ) )
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
                sal_Int32 nApiRot;
                if( aDiaProp.GetProperty( nApiRot, EXC_CHPROP_STARTINGANGLE ) )
                    maData.mnRotation = static_cast< sal_uInt16 >( (450 - (nApiRot % 360)) % 360 );
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
            DBG_ERRORFILE( "XclExpChType::WriteBody - unknown chart type" );
    }
}

// ----------------------------------------------------------------------------

XclExpChChart3d::XclExpChChart3d() :
    XclExpRecord( EXC_ID_CHCHART3D, 14 )
{
}

void XclExpChChart3d::Convert( const ScfPropertySet& rPropSet, const XclChTypeInfo& rTypeInfo )
{
    if( rTypeInfo.mb3dWalls )
    {
        maData.mnRotation = 20;
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_REAL3D, !rPropSet.GetBoolProperty( EXC_CHPROP_RIGHTANGLEDAXES ) );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_AUTOHEIGHT );
        ::set_flag( maData.mnFlags, EXC_CHCHART3D_BIT4 );
    }
    else
    {
        maData.mnRotation = 0;
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
    XclExpChGroupBase( EXC_ID_CHLEGEND, 20 ),
    XclExpChRoot( rRoot )
{
}

void XclExpChLegend::Convert( const ScfPropertySet& rPropSet )
{
    // frame properties
    mxFrame = lclCreateFrame( GetChRoot(), rPropSet, EXC_CHOBJTYPE_LEGEND );
    // text properties
    mxText.reset( new XclExpChText( GetChRoot() ) );
    mxText->ConvertLegend( rPropSet );
    // special legend properties
    GetChartPropSetHelper().ReadLegendProperties( maData, rPropSet );
}

void XclExpChLegend::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxText );
    lclSaveRecord( rStrm, mxFrame );
}

void XclExpChLegend::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.maRect << maData.mnDockMode << maData.mnSpacing << maData.mnFlags;
}

// ----------------------------------------------------------------------------

XclExpChDropBar::XclExpChDropBar( XclChObjectType eObjType ) :
    XclExpChGroupBase( EXC_ID_CHDROPBAR, 2 ),
    meObjType( eObjType ),
    mnBarDist( 100 )
{
}

void XclExpChDropBar::Convert( const XclExpChRoot& rRoot, const ScfPropertySet& rPropSet )
{
    if( rPropSet.Is() )
        ConvertFrameBase( rRoot, rPropSet, meObjType );
    else
        SetDefaultFrameBase( rRoot, EXC_CHFRAMETYPE_INVISIBLE, true );
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
    XclExpChGroupBase( EXC_ID_CHTYPEGROUP, 20 ),
    XclExpChRoot( rRoot ),
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
    ::com::sun::star::chart2::CurveStyle eCurveStyle;
    bool bSpline = aTypeProp.GetProperty( eCurveStyle, EXC_CHPROP_CURVESTYLE ) &&
        (eCurveStyle != ::com::sun::star::chart2::CurveStyle_LINES);

    // extended type info
    maTypeInfo.Set( maType.GetTypeInfo(), b3dChart, bSpline );

    // 3d chart settings
    if( maTypeInfo.mb3dChart )  // only true, if Excel chart supports 3d mode
    {
        mxChart3d.reset( new XclExpChChart3d );
        ScfPropertySet aDiaProp( xDiagram );
        mxChart3d->Convert( aDiaProp, maTypeInfo );
    }
}

void XclExpChTypeGroup::ConvertSeries( Reference< XChartType > xChartType,
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
            namespace cssc = ::com::sun::star::chart2;
            cssc::StackingDirection eStacking;
            if( !aSeriesProp.GetProperty( eStacking, EXC_CHPROP_STACKINGDIR ) )
                eStacking = cssc::StackingDirection_NO_STACKING;

            // stacked or percent chart
            if( maTypeInfo.mbSupportsStacking && (eStacking == cssc::StackingDirection_Y_STACKING) )
            {
                // percent overrides simple stacking
                maType.SetStacked( bPercent );

                // connected data points (only in stacked bar charts)
                if( bConnectBars && (maTypeInfo.meTypeCateg == EXC_CHTYPECATEG_BAR) )
                    maChartLines[ EXC_CHCHARTLINE_CONNECT ].reset( new XclExpChLineFormat( GetChRoot() ) );
            }
            else
            {
                // reverse series order for some unstacked 2D chart types
                if( maTypeInfo.mbReverseSeries && !Is3dChart() )
                    ::std::reverse( aSeriesVec.begin(), aSeriesVec.end() );
            }

            // deep 3d chart or clustered 3d chart (stacked is not clustered)
            if( (eStacking == cssc::StackingDirection_NO_STACKING) && Is3dWallChart() )
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
                    CreateDataSeries( *aIt );
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

void XclExpChTypeGroup::CreateDataSeries( Reference< XDataSeries > xDataSeries )
{
    // let chart create series object with correct series index
    XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
    if( xSeries.is() )
    {
        if( xSeries->ConvertDataSeries( xDataSeries, maTypeInfo, GetGroupIdx(), GetFreeFormatIdx() ) )
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
        maChartLines[ EXC_CHCHARTLINE_HILO ] = xLineFmt;
    }
    // dropbars
    if( bHasOpen && bHasClose )
    {
        // dropbar type is dependent on position in the file - always create both
        Reference< XPropertySet > xWhitePropSet, xBlackPropSet;
        // white dropbar format
        aTypeProp.GetProperty( xWhitePropSet, EXC_CHPROP_WHITEDAY );
        ScfPropertySet aWhiteProp( xWhitePropSet );
        mxUpBar.reset( new XclExpChDropBar( EXC_CHOBJTYPE_WHITEDROPBAR ) );
        mxUpBar->Convert( GetChRoot(), aWhiteProp );
        // black dropbar format
        aTypeProp.GetProperty( xBlackPropSet, EXC_CHPROP_BLACKDAY );
        ScfPropertySet aBlackProp( xBlackPropSet );
        mxDownBar.reset( new XclExpChDropBar( EXC_CHOBJTYPE_BLACKDROPBAR ) );
        mxDownBar->Convert( GetChRoot(), aBlackProp );
    }
}

bool XclExpChTypeGroup::CreateStockSeries( Reference< XDataSeries > xDataSeries,
        const OUString& rValueRole, bool bCloseSymbol )
{
    bool bOk = false;
    // let chart create series object with correct series index
    XclExpChSeriesRef xSeries = GetChartData().CreateSeries();
    if( xSeries.is() )
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
    rStrm << maData.maRect << maData.mnFlags << maData.mnGroupIdx;
}

// Axes =======================================================================

XclExpChLabelRange::XclExpChLabelRange( const XclExpChRoot& rRoot ) :
    XclExpRecord( EXC_ID_CHLABELRANGE, 8 ),
    XclExpChRoot( rRoot )
{
}

void XclExpChLabelRange::Convert( const ScaleData& rScaleData )
{
    // origin
    double fOrigin = 0.0;
    if( !lclIsAutoAnyOrGetValue( fOrigin, rScaleData.Origin ) )
        maData.mnCross = limit_cast< sal_uInt16 >( fOrigin, 1, 32767 );

    // reverse order
    if( rScaleData.Orientation == ::com::sun::star::chart2::AxisOrientation_REVERSE )
    {
        ::set_flag( maData.mnFlags, EXC_CHLABELRANGE_REVERSE );
        SwapAxisMaxCross();
    }
}

void XclExpChLabelRange::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnCross << maData.mnLabelFreq << maData.mnTickFreq << maData.mnFlags;
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
    Reference< XScaling > xLogScaling;
    if( bLogScale )
        xLogScaling = rScaleData.Scaling;

    // min/max
    bool bAutoMin = lclIsAutoAnyOrGetScaledValue( maData.mfMin, rScaleData.Minimum, xLogScaling );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMIN, bAutoMin );
    bool bAutoMax = lclIsAutoAnyOrGetScaledValue( maData.mfMax, rScaleData.Maximum, xLogScaling );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAX, bAutoMax );

    // origin
    bool bAutoCross = lclIsAutoAnyOrGetScaledValue( maData.mfCross, rScaleData.Origin, xLogScaling );
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOCROSS, bAutoCross );

    // major increment
    const IncrementData& rIncrementData = rScaleData.IncrementData;
    bool bAutoMajor = lclIsAutoAnyOrGetValue( maData.mfMajorStep, rIncrementData.Distance ) || (maData.mfMajorStep <= 0.0);
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMAJOR, bAutoMajor );
    // minor increment
    const Sequence< SubIncrement >& rSubIncrementSeq = rIncrementData.SubIncrements;
    sal_Int32 nCount = 0;
    bool bAutoMinor = bAutoMajor || (rSubIncrementSeq.getLength() < 1) ||
        lclIsAutoAnyOrGetValue( nCount, rSubIncrementSeq[ 0 ].IntervalCount ) || (nCount < 1);
    if( !bAutoMinor )
        maData.mfMinorStep = maData.mfMajorStep / nCount;
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_AUTOMINOR, bAutoMinor );

    // reverse order
    namespace cssc = ::com::sun::star::chart2;
    ::set_flag( maData.mnFlags, EXC_CHVALUERANGE_REVERSE, rScaleData.Orientation == cssc::AxisOrientation_REVERSE );
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
    using namespace ::com::sun::star::chart2::TickmarkStyle;
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

void XclExpChTick::Convert( const ScfPropertySet& rPropSet )
{
    // tick mark style
    sal_Int32 nApiTickmarks(0);
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MAJORTICKS ) )
        maData.mnMajor = lclGetXclTickPos( nApiTickmarks );
    if( rPropSet.GetProperty( nApiTickmarks, EXC_CHPROP_MINORTICKS ) )
        maData.mnMinor = lclGetXclTickPos( nApiTickmarks );
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
            << maData.mnBackMode
            << maData.maRect
            << maData.maTextColor
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
    try
    {
        if( xCoordSystem.is() )
            xAxis = xCoordSystem->getAxisByDimension( nApiAxisDim, nApiAxesSetIdx );
    }
    catch( Exception& )
    {
    }
    return xAxis;
}

} // namespace

XclExpChAxis::XclExpChAxis( const XclExpChRoot& rRoot, sal_uInt16 nAxisType ) :
    XclExpChGroupBase( EXC_ID_CHAXIS, 18 ),
    XclExpChRoot( rRoot ),
    mnNumFmtIdx( EXC_FORMAT_NOTFOUND )
{
    maData.mnType = nAxisType;
}

void XclExpChAxis::SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId )
{
    mxFont = xFont;
    if( mxTick.is() )
        mxTick->SetFontColor( rColor, nColorId );
}

void XclExpChAxis::SetRotation( sal_uInt16 nRotation )
{
    if( mxTick.is() )
        mxTick->SetRotation( nRotation );
}

void XclExpChAxis::Convert( Reference< XAxis > xAxis, const XclChExtTypeInfo& rTypeInfo, sal_Int32 nApiAxesSetIdx )
{
    ScfPropertySet aAxisProp( xAxis );
    bool bCategoryAxis = ((GetAxisType() == EXC_CHAXIS_X) && rTypeInfo.mbCategoryAxis) || (GetAxisType() == EXC_CHAXIS_Z);

    // axis line format -------------------------------------------------------

    mxAxisLine.reset( new XclExpChLineFormat( GetChRoot() ) );
    mxAxisLine->Convert( GetChRoot(), aAxisProp, EXC_CHOBJTYPE_AXISLINE );
    // #i58688# axis enabled
    mxAxisLine->SetShowAxis( aAxisProp.GetBoolProperty( EXC_CHPROP_SHOW ) );

    // axis scaling and increment ---------------------------------------------

    if( bCategoryAxis )
    {
        mxLabelRange.reset( new XclExpChLabelRange( GetChRoot() ) );
        mxLabelRange->SetTicksBetweenCateg( rTypeInfo.mbTicksBetweenCateg );
        if( xAxis.is() )
            mxLabelRange->Convert( xAxis->getScaleData() );
        // swap max-cross flag for secondary X axis to have the secondary Y axis at right side
        if( (GetAxisType() == EXC_CHAXIS_X) && (nApiAxesSetIdx == EXC_CHAXESSET_SECONDARY) )
            mxLabelRange->SwapAxisMaxCross();
    }
    else
    {
        mxValueRange.reset( new XclExpChValueRange( GetChRoot() ) );
        if( xAxis.is() )
            mxValueRange->Convert( xAxis->getScaleData() );
    }

    // axis caption text ------------------------------------------------------

    // axis ticks properties
    mxTick.reset( new XclExpChTick( GetChRoot() ) );
    mxTick->Convert( aAxisProp );

    // existence and position of axis labels
    sal_uInt8 nLabelPos = EXC_CHTICK_NOLABEL;
    bool bHasLabels = aAxisProp.GetBoolProperty( EXC_CHPROP_DISPLAYLABELS );
    // radar charts disable their category labels via chart type, not via axis
    if( bHasLabels || (bCategoryAxis && (rTypeInfo.meTypeCateg == EXC_CHTYPECATEG_RADAR)) )
        // category axes in 3d charts have default position 'low', all other 'next to axis'
        nLabelPos = (rTypeInfo.mb3dChart && bCategoryAxis) ? EXC_CHTICK_LOW : EXC_CHTICK_NEXT;
    mxTick->SetLabelPos( nLabelPos );

    // axis label formatting and rotation
    ConvertFontBase( GetChRoot(), aAxisProp );
    ConvertRotationBase( GetChRoot(), aAxisProp );

    // axis number format
    sal_Int32 nApiNumFmt = 0;
    if( !bCategoryAxis && aAxisProp.GetProperty( nApiNumFmt, EXC_CHPROP_NUMBERFORMAT ) )
        mnNumFmtIdx = GetNumFmtBuffer().Insert( static_cast< sal_uInt32 >( nApiNumFmt ) );

    // grid ---------------------------------------------------------------

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
    rStrm << maData.mnType << maData.maRect;
}

// ----------------------------------------------------------------------------

XclExpChAxesSet::XclExpChAxesSet( const XclExpChRoot& rRoot, sal_uInt16 nAxesSetId ) :
    XclExpChGroupBase( EXC_ID_CHAXESSET, 18 ),
    XclExpChRoot( rRoot )
{
    maData.mnAxesSetId = nAxesSetId;
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
            namespace ApiAxisType = ::com::sun::star::chart2::AxisType;
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
                    if( xLastGroup.is() && !(xTypeGroup->IsCombinable2d() && xLastGroup->IsCombinable2d()) )
                    {
                        xLastGroup->ConvertSeries( *pIt, nApiAxesSetIdx, bPercent, bConnectBars );
                    }
                    else
                    {
                        xTypeGroup->ConvertSeries( *pIt, nApiAxesSetIdx, bPercent, bConnectBars );
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
                    ConvertAxis( mxXAxis, EXC_CHAXIS_X, mxXAxisTitle, EXC_CHOBJLINK_XAXIS, xCoordSystem, rTypeInfo );
                    ConvertAxis( mxYAxis, EXC_CHAXIS_Y, mxYAxisTitle, EXC_CHOBJLINK_YAXIS, xCoordSystem, rTypeInfo );
                    if( pGroup->Is3dDeepChart() )
                        ConvertAxis( mxZAxis, EXC_CHAXIS_Z, mxZAxisTitle, EXC_CHOBJLINK_ZAXIS, xCoordSystem, rTypeInfo );
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
        if( xTypeGroup.is() && xTypeGroup->Is3dWallChart() )
        {
            // wall/floor formatting (3D charts)
            if( mxXAxis.is() )
                mxXAxis->ConvertWall( xDiagram );
            if( mxYAxis.is() )
                mxYAxis->ConvertWall( xDiagram );
        }
        else
        {
            // diagram background formatting
            ScfPropertySet aWallProp( xDiagram->getWall() );
            mxPlotFrame = lclCreateFrame( GetChRoot(), aWallProp, EXC_CHOBJTYPE_PLOTFRAME );
        }
    }

    // return first unused chart type group index for next axes set
    return nGroupIdx;
}

bool XclExpChAxesSet::Is3dChart() const
{
    XclExpChTypeGroupRef xTypeGroup = GetFirstTypeGroup();
    return xTypeGroup.is() && xTypeGroup->Is3dChart();
}

void XclExpChAxesSet::WriteSubRecords( XclExpStream& rStrm )
{
    lclSaveRecord( rStrm, mxXAxis );
    lclSaveRecord( rStrm, mxYAxis );
    lclSaveRecord( rStrm, mxZAxis );
    lclSaveRecord( rStrm, mxXAxisTitle );
    lclSaveRecord( rStrm, mxYAxisTitle );
    lclSaveRecord( rStrm, mxZAxisTitle );
    if( mxPlotFrame.is() )
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
        Reference< XCoordinateSystem > xCoordSystem, const XclChExtTypeInfo& rTypeInfo )
{
    // create and convert axis object
    rxChAxis.reset( new XclExpChAxis( GetChRoot(), nAxisType ) );
    sal_Int32 nApiAxisDim = rxChAxis->GetApiAxisDimension();
    sal_Int32 nApiAxesSetIdx = GetApiAxesSetIndex();
    Reference< XAxis > xAxis = lclGetApiAxis( xCoordSystem, nApiAxisDim, nApiAxesSetIdx );
    rxChAxis->Convert( xAxis, rTypeInfo, nApiAxesSetIdx );

    // create and convert axis title
    Reference< XTitled > xTitled( xAxis, UNO_QUERY );
    rxChAxisTitle = lclCreateTitle( GetChRoot(), xTitled, nTitleTarget );
}

void XclExpChAxesSet::WriteBody( XclExpStream& rStrm )
{
    rStrm << maData.mnAxesSetId << maData.maRect;
}

// The chart object ===========================================================

XclExpChChart::XclExpChChart( const XclExpRoot& rRoot,
        Reference< XChartDocument > xChartDoc, const Size& rSize ) :
    XclExpChGroupBase( EXC_ID_CHCHART, 16 ),
    XclExpChRoot( rRoot, this )
{
    Size aPtSize = OutputDevice::LogicToLogic( rSize, MapMode( MAP_100TH_MM ), MapMode( MAP_POINT ) );
    // rectangle is stored in 16.16 fixed-point format
    maRect.mnX = maRect.mnY = 0;
    maRect.mnWidth = static_cast< sal_Int32 >( aPtSize.Width() << 16 );
    maRect.mnHeight = static_cast< sal_Int32 >( aPtSize.Height() << 16 );

    // global chart properties
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_MANSERIES );
    ::set_flag( maProps.mnFlags, EXC_CHPROPS_SHOWVISCELLS, false );
    maProps.mnEmptyMode = EXC_CHPROPS_EMPTY_SKIP;

    // always create both axes set objects
    mxPrimAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_PRIMARY ) );
    mxSecnAxesSet.reset( new XclExpChAxesSet( GetChRoot(), EXC_CHAXESSET_SECONDARY ) );

    if( xChartDoc.is() )
    {
        // initialize API conversion (remembers xChartDoc internally)
        InitConversion( xChartDoc );

        // chart frame
        ScfPropertySet aFrameProp( xChartDoc->getPageBackground() );
        mxFrame = lclCreateFrame( GetChRoot(), aFrameProp, EXC_CHOBJTYPE_BACKGROUND );

        // chart title
        Reference< XTitled > xTitled( xChartDoc, UNO_QUERY );
        mxTitle = lclCreateTitle( GetChRoot(), xTitled, EXC_CHOBJLINK_TITLE );

        // diagrams (axes sets)
        Reference< XDiagram > xDiagram = xChartDoc->getFirstDiagram();
        sal_uInt16 nFreeGroupIdx = mxPrimAxesSet->Convert( xDiagram, 0 );
        if( !mxPrimAxesSet->Is3dChart() )
            mxSecnAxesSet->Convert( xDiagram, nFreeGroupIdx );

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
    if( xText.is() )
        maLabels.AppendRecord( xText );
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

XclExpChart::XclExpChart( const XclExpRoot& rRoot, Reference< XModel > xModel, const Size& rSize ) :
    XclExpSubStream( EXC_BOF_CHART ),
    XclExpRoot( rRoot )
{
    AppendNewRecord( new XclExpChartPageSettings( rRoot ) );
    AppendNewRecord( new XclExpBoolRecord( EXC_ID_PROTECT, false ) );
    AppendNewRecord( new XclExpUInt16Record( EXC_ID_CHUNITS, EXC_CHUNITS_TWIPS ) );

    Reference< XChartDocument > xChartDoc( xModel, UNO_QUERY );
    AppendNewRecord( new XclExpChChart( rRoot, xChartDoc, rSize ) );
}

// ============================================================================

