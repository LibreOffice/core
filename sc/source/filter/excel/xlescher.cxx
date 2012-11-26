/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "xlescher.hxx"

#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <svx/unoapi.hxx>
#include "document.hxx"
#include "xestream.hxx"
#include "xistream.hxx"
#include "xlroot.hxx"
#include "xltools.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::script::ScriptEventDescriptor;

// Structs and classes ========================================================

XclObjId::XclObjId() :
    mnScTab( SCTAB_INVALID ),
    mnObjId( EXC_OBJ_INVALID_ID )
{
}

XclObjId::XclObjId( SCTAB nScTab, sal_uInt16 nObjId ) :
    mnScTab( nScTab ),
    mnObjId( nObjId )
{
}

bool operator==( const XclObjId& rL, const XclObjId& rR )
{
    return (rL.mnScTab == rR.mnScTab) && (rL.mnObjId == rR.mnObjId);
}

bool operator<( const XclObjId& rL, const XclObjId& rR )
{
    return (rL.mnScTab < rR.mnScTab) || ((rL.mnScTab == rR.mnScTab) && (rL.mnObjId < rR.mnObjId));
}

// ----------------------------------------------------------------------------

namespace {

/** Returns the scaling factor to calculate coordinates from twips. */
double lclGetTwipsScale( MapUnit eMapUnit )
{
    /*  #111027# We cannot use OutputDevice::LogicToLogic() or the XclTools
        conversion functions to calculate drawing layer coordinates due to
        Calc's strange definition of a point (1 inch == 72.27 points, instead
        of 72 points). */
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MAP_TWIP:      fScale = 72 / POINTS_PER_INCH;  break;  // Calc twips <-> real twips
        case MAP_100TH_MM:  fScale = HMM_PER_TWIPS;         break;  // Calc twips <-> 1/100mm
        default:            DBG_ERRORFILE( "lclGetTwipsScale - map unit not implemented" );
    }
    return fScale;
}

/** Calculates a drawing layer X position (in twips) from an object column position. */
long lclGetXFromCol( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclCol, sal_uInt16 nOffset, double fScale )
{
    SCCOL nScCol = static_cast< SCCOL >( nXclCol );
    return static_cast< long >( fScale * (rDoc.GetColOffset( nScCol, nScTab ) +
        ::std::min( nOffset / 1024.0, 1.0 ) * rDoc.GetColWidth( nScCol, nScTab )) + 0.5 );
}

/** Calculates a drawing layer Y position (in twips) from an object row position. */
long lclGetYFromRow( ScDocument& rDoc, SCTAB nScTab, sal_uInt16 nXclRow, sal_uInt16 nOffset, double fScale )
{
    SCROW nScRow = static_cast< SCROW >( nXclRow );
    return static_cast< long >( fScale * (rDoc.GetRowOffset( nScRow, nScTab ) +
        ::std::min( nOffset / 256.0, 1.0 ) * rDoc.GetRowHeight( nScRow, nScTab )) + 0.5 );
}

/** Calculates an object column position from a drawing layer X position (in twips). */
void lclGetColFromX(
        ScDocument& rDoc, SCTAB nScTab, sal_uInt16& rnXclCol,
        sal_uInt16& rnOffset,
        sal_uInt16 nXclStartCol,
        sal_uInt16 nXclMaxCol,
        long& rnStartW,
        double fX,
        double fScale )
{
    // rnStartW in conjunction with nXclStartCol is used as buffer for previously calculated width
    long nTwipsX = static_cast< long >( fX / fScale + 0.5 );
    long nColW = 0;
    for( rnXclCol = nXclStartCol; rnXclCol <= nXclMaxCol; ++rnXclCol )
    {
        nColW = rDoc.GetColWidth( static_cast< SCCOL >( rnXclCol ), nScTab );
        if( rnStartW + nColW > nTwipsX )
            break;
        rnStartW += nColW;
    }
    rnOffset = nColW ? static_cast< sal_uInt16 >( (nTwipsX - rnStartW) * 1024.0 / nColW + 0.5 ) : 0;
}

/** Calculates an object row position from a drawing layer Y position (in twips). */
void lclGetRowFromY(
        ScDocument& rDoc,
        SCTAB nScTab,
        sal_uInt16& rnXclRow,
        sal_uInt16& rnOffset,
        sal_uInt16 nXclStartRow,
        sal_uInt16 nXclMaxRow,
        long& rnStartH,
        double fY,
        double fScale )
{
    // rnStartH in conjunction with nXclStartRow is used as buffer for previously calculated height
    long nTwipsY = static_cast< long >( fY / fScale + 0.5 );
    long nRowH = 0;
    bool bFound = false;
    for( SCROW nRow = static_cast< SCROW >( nXclStartRow ); nRow <= nXclMaxRow; ++nRow )
    {
        nRowH = rDoc.GetRowHeight( nRow, nScTab );
        if( rnStartH + nRowH > nTwipsY )
        {
            rnXclRow = static_cast< sal_uInt16 >( nRow );
            bFound = true;
            break;
        }
        rnStartH += nRowH;
    }
    if( !bFound )
        rnXclRow = nXclMaxRow;
    rnOffset = static_cast< sal_uInt16 >( nRowH ? ((nTwipsY - rnStartH) * 256.0 / nRowH + 0.5) : 0 );
}

/** Mirrors a rectangle (from LTR to RTL layout or vice versa). */
void lclMirrorRange( basegfx::B2DRange& rRange )
{
    rRange = basegfx::B2DRange(
        -rRange.getMaxX(), rRange.getMinY(),
        -rRange.getMinX(), rRange.getMaxY());
}

sal_uInt16 lclGetEmbeddedScale( double fPageSize, double fPageScale, double fPos, double fPosScale )
{
    return static_cast< sal_uInt16 >( fPos * fPosScale / fPageSize * fPageScale + 0.5 );
}

} // namespace

// ----------------------------------------------------------------------------

XclObjAnchor::XclObjAnchor() :
    mnLX( 0 ),
    mnTY( 0 ),
    mnRX( 0 ),
    mnBY( 0 )
{
}

basegfx::B2DRange XclObjAnchor::GetRangeFromAnchor( const XclRoot& rRoot, SCTAB nScTab, MapUnit eMapUnit ) const
{
    ScDocument& rDoc = rRoot.GetDoc();
    const double fScale(lclGetTwipsScale(eMapUnit));
    basegfx::B2DRange aRange(
        lclGetXFromCol( rDoc, nScTab, maFirst.mnCol, mnLX, fScale ),
        lclGetYFromRow( rDoc, nScTab, maFirst.mnRow, mnTY, fScale ),
        lclGetXFromCol( rDoc, nScTab, maLast.mnCol,  mnRX + 1, fScale ),
        lclGetYFromRow( rDoc, nScTab, maLast.mnRow,  mnBY, fScale ) );

    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( nScTab ) )
    {
        lclMirrorRange( aRange );
    }

    return aRange;
}

void XclObjAnchor::SetRangeAtAnchor(
    const XclRoot& rRoot,
    SCTAB nScTab,
    const basegfx::B2DRange& rObjectRange,
    MapUnit eMapUnit )
{
    basegfx::B2DRange aObjectRange(rObjectRange);
    ScDocument& rDoc = rRoot.GetDoc();
    sal_uInt16 nXclMaxCol = rRoot.GetXclMaxPos().Col();
    sal_uInt16 nXclMaxRow = static_cast<sal_uInt16>( rRoot.GetXclMaxPos().Row());

    // #106948# adjust coordinates in mirrored sheets
    if( rDoc.IsLayoutRTL( nScTab ) )
    {
        aObjectRange = basegfx::B2DRange(
            -aObjectRange.getMinX(), aObjectRange.getMinY(),
            -aObjectRange.getMaxX(), aObjectRange.getMaxY());
    }

    const double fScale(lclGetTwipsScale( eMapUnit ));
    long nDummy = 0;
    lclGetColFromX( rDoc, nScTab, maFirst.mnCol, mnLX, 0,             nXclMaxCol, nDummy, aObjectRange.getMinX(), fScale );
    lclGetColFromX( rDoc, nScTab, maLast.mnCol,  mnRX, maFirst.mnCol, nXclMaxCol, nDummy, aObjectRange.getMaxX(), fScale );
    nDummy = 0;
    lclGetRowFromY( rDoc, nScTab, maFirst.mnRow, mnTY, 0,             nXclMaxRow, nDummy, aObjectRange.getMinY(), fScale );
    lclGetRowFromY( rDoc, nScTab, maLast.mnRow,  mnBY, maFirst.mnRow, nXclMaxRow, nDummy, aObjectRange.getMaxY(), fScale );
}

void XclObjAnchor::SetRangeAtAnchor(
    const basegfx::B2DVector& rPageScale,
    double fScaleX,
    double fScaleY,
    const basegfx::B2DRange& rObjectRange,
    MapUnit eMapUnit,
    bool bDffAnchor )
{
    double fScale = 1.0;
    switch( eMapUnit )
    {
        case MAP_TWIP:      fScale = HMM_PER_TWIPS; break;  // Calc twips -> 1/100mm
        case MAP_100TH_MM:  fScale = 1.0;           break;  // Calc 1/100mm -> 1/100mm
        default:            DBG_ERRORFILE( "XclObjAnchor::SetRangeAtAnchor - map unit not implemented" );
    }

    /*  In objects with DFF client anchor, the position of the shape is stored
        in the cell address components of the client anchor. In old BIFF3-BIFF5
        objects, the position is stored in the offset components of the anchor. */
    (bDffAnchor ? maFirst.mnCol : mnLX) = lclGetEmbeddedScale( rPageScale.getX(), fScaleX, rObjectRange.getMinX(), fScale );
    (bDffAnchor ? maFirst.mnRow : mnTY) = lclGetEmbeddedScale( rPageScale.getY(), fScaleY, rObjectRange.getMinY(), fScale );
    (bDffAnchor ? maLast.mnCol  : mnRX) = lclGetEmbeddedScale( rPageScale.getX(), fScaleX, rObjectRange.getMaxX(), fScale );
    (bDffAnchor ? maLast.mnRow  : mnBY) = lclGetEmbeddedScale( rPageScale.getY(), fScaleY, rObjectRange.getMaxY(), fScale );

    // for safety, clear the other members
    if( bDffAnchor )
        mnLX = mnTY = mnRX = mnBY = 0;
    else
        Set( 0, 0, 0, 0 );
}

// ----------------------------------------------------------------------------

XclObjLineData::XclObjLineData() :
    mnColorIdx( EXC_OBJ_LINE_AUTOCOLOR ),
    mnStyle( EXC_OBJ_LINE_SOLID ),
    mnWidth( EXC_OBJ_LINE_HAIR ),
    mnAuto( EXC_OBJ_LINE_AUTO )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclObjLineData& rLineData )
{
    return rStrm
        >> rLineData.mnColorIdx
        >> rLineData.mnStyle
        >> rLineData.mnWidth
        >> rLineData.mnAuto;
}

// ----------------------------------------------------------------------------

XclObjFillData::XclObjFillData() :
    mnBackColorIdx( EXC_OBJ_LINE_AUTOCOLOR ),
    mnPattColorIdx( EXC_OBJ_FILL_AUTOCOLOR ),
    mnPattern( EXC_PATT_SOLID ),
    mnAuto( EXC_OBJ_FILL_AUTO )
{
}

XclImpStream& operator>>( XclImpStream& rStrm, XclObjFillData& rFillData )
{
    return rStrm
        >> rFillData.mnBackColorIdx
        >> rFillData.mnPattColorIdx
        >> rFillData.mnPattern
        >> rFillData.mnAuto;
}

// ----------------------------------------------------------------------------

XclObjTextData::XclObjTextData() :
    mnTextLen( 0 ),
    mnFormatSize( 0 ),
    mnLinkSize( 0 ),
    mnDefFontIdx( EXC_FONT_APP ),
    mnFlags( 0 ),
    mnOrient( EXC_OBJ_ORIENT_NONE ),
    mnButtonFlags( 0 ),
    mnShortcut( 0 ),
    mnShortcutEA( 0 )
{
}

void XclObjTextData::ReadObj3( XclImpStream& rStrm )
{
    rStrm >> mnTextLen;
    rStrm.Ignore( 2 );
    rStrm >> mnFormatSize >> mnDefFontIdx;
    rStrm.Ignore( 2 );
    rStrm >> mnFlags >> mnOrient;
    rStrm.Ignore( 8 );
}

void XclObjTextData::ReadObj5( XclImpStream& rStrm )
{
    rStrm >> mnTextLen;
    rStrm.Ignore( 2 );
    rStrm >> mnFormatSize >> mnDefFontIdx;
    rStrm.Ignore( 2 );
    rStrm >> mnFlags >> mnOrient;
    rStrm.Ignore( 2 );
    rStrm >> mnLinkSize;
    rStrm.Ignore( 2 );
    rStrm >> mnButtonFlags >> mnShortcut >> mnShortcutEA;
}

void XclObjTextData::ReadTxo8( XclImpStream& rStrm )
{
    rStrm >> mnFlags >> mnOrient >> mnButtonFlags >> mnShortcut >> mnShortcutEA >> mnTextLen >> mnFormatSize;
}

// ============================================================================

Reference< XControlModel > XclControlHelper::GetControlModel( Reference< XShape > xShape )
{
    Reference< XControlModel > xCtrlModel;
    Reference< XControlShape > xCtrlShape( xShape, UNO_QUERY );
    if( xCtrlShape.is() )
        xCtrlModel = xCtrlShape->getControl();
    return xCtrlModel;
}

namespace {

static const struct
{
    const sal_Char*     mpcListenerType;
    const sal_Char*     mpcEventMethod;
}
spTbxListenerData[] =
{
    // Attention: MUST be in order of the XclTbxEventType enum!
    /*EXC_TBX_EVENT_ACTION*/    { "XActionListener",     "actionPerformed"        },
    /*EXC_TBX_EVENT_MOUSE*/     { "XMouseListener",      "mouseReleased"          },
    /*EXC_TBX_EVENT_TEXT*/      { "XTextListener",       "textChanged"            },
    /*EXC_TBX_EVENT_VALUE*/     { "XAdjustmentListener", "adjustmentValueChanged" },
    /*EXC_TBX_EVENT_CHANGE*/    { "XChangeListener",     "changed"                }
};

} // namespace

bool XclControlHelper::FillMacroDescriptor( ScriptEventDescriptor& rDescriptor,
        XclTbxEventType eEventType, const String& rXclMacroName, SfxObjectShell* pDocShell )
{
    if( rXclMacroName.Len() > 0 )
    {
        rDescriptor.ListenerType = OUString::createFromAscii( spTbxListenerData[ eEventType ].mpcListenerType );
        rDescriptor.EventMethod = OUString::createFromAscii( spTbxListenerData[ eEventType ].mpcEventMethod );
        rDescriptor.ScriptType = CREATE_OUSTRING( "Script" );
        rDescriptor.ScriptCode = XclTools::GetSbMacroUrl( rXclMacroName, pDocShell );
        return true;
    }
    return false;
}

String XclControlHelper::ExtractFromMacroDescriptor(
        const ScriptEventDescriptor& rDescriptor, XclTbxEventType eEventType )
{
    if( (rDescriptor.ScriptCode.getLength() > 0) &&
            rDescriptor.ScriptType.equalsIgnoreAsciiCaseAscii( "Script" ) &&
            rDescriptor.ListenerType.equalsAscii( spTbxListenerData[ eEventType ].mpcListenerType ) &&
            rDescriptor.EventMethod.equalsAscii( spTbxListenerData[ eEventType ].mpcEventMethod ) )
        return XclTools::GetXclMacroName( rDescriptor.ScriptCode );
    return String::EmptyString();
}

// ============================================================================
